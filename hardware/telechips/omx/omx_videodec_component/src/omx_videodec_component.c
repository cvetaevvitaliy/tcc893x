/**
  @file src/components/ffmpeg/omx_videodec_component.c

  This component implements H.264 / MPEG-4 AVC video decoder.
  The H.264 / MPEG-4 AVC Video decoder is based on the FFmpeg software library.

  Copyright (C) 2007-2008 STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies)
  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

  $Date: 2009/03/10 13:33:28 $
  Revision $Rev: 557 $
  Author $Author: B060934 $
  Android revised by ZzaU.
*/

#include <omxcore.h>
#include <omx_base_video_port.h>
#include <omx_videodec_component.h>
#ifdef RING_BUFFER_MODULE_ENABLE
#include <omx_vpudec_component.h>	//RINGBUFF
#endif
#include <OMX_Video.h>
#ifdef OPENMAX1_2
#include <OMX_VideoExt.h>
#endif

#include <lcd_resolution.h>

#include <vdec.h>
#include <cdk.h>
#include "cdk_android.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "tcc_video_config_data.h"

#include <mach/tccfb_ioctrl.h>
#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"OMX_TCC_VIDEO_DEC"
#include <utils/Log.h>

#include <cutils/properties.h>
#include "TCCStagefrightDefine.h"
#ifdef MOVE_HW_OPERATION
#ifdef USE_WMIXER_FOR_COPY
#include <mach/tcc_wmixer_ioctrl.h>
#include <mach/vioc_global.h>
#define COPY_DEVICE         "/dev/wmixer"
#else
#include <mach/tcc_scaler_ioctrl.h>
#define COPY_DEVICE         "/dev/scaler1"
#endif
#include <mach/tcc_mem_ioctl.h>

#include <fcntl.h>
#include <sys/poll.h>
#endif
#define TMEM_DEVICE			"/dev/tmem"
#define FB0_DEVICE          "/dev/graphics/fb0"

#ifdef DIVX_DRM5
#include <divx_drm5_Ex.h>
#endif

#define RESTORE_DECODE_ERR
#define CHECK_SEQHEADER_WITH_SYNCFRAME

/* Option for TS H.264 seek tuning */
#define ENABLE_DECODE_ONLY_MODE_AVC

#define LOGMSG(x...)
#define LOGERR(x...)
#define LOGINFO(x...)

#include <cutils/properties.h>
#include <mach/tcc_video_private.h>

#define MAX_DECODE_ONLY_FRAME_NUM	(3 * VPU_BUFF_COUNT)
#define MAX_DEC_ONLY_ERR_THRESHOLD	60

#define TIMESTAMP_CORRECTION

static int DEBUG_ON = 0;
#define DBUG_MSG(msg...)	if (DEBUG_ON) { ALOGD( ": " msg);/* sleep(1);*/}
#define DPRINTF_DEC(msg...) //ALOGI( ": " msg);
#define DPRINTF_DEC_STATUS(msg...) //ALOGD( ": " msg);
#define DPRINTF_VBUFFER_MANAGE(msg...) //ALOGD( ": " msg);

#define LOGD(x...)    ALOGD(x)
#define LOGE(x...)    ALOGE(x)
#define LOGI(x...)    ALOGI(x)
#define LOGW(x...)    ALOGW(x)

#define HARDWARE_CODEC	1
#ifdef ANDROID_USE_GRALLOC_BUFFER
#define USE_EXTERNAL_BUFFER 1

static int GRALLOC_DEBUG_ON = 0;
#define GBUG_MSG(msg...)	if (GRALLOC_DEBUG_ON) { LOGD( ": " msg);/* sleep(1);*/}

#define MVC_PROCESS

//#define VPU_FRAME_DUMP
//#define UMP_COPIED_FRAME_DUMP

#ifndef UMP_COPIED_FRAME_DUMP
#define USE_YUV420SP_DEFAULT
#endif

//#define COMPARE_TIME_LOG
#ifdef COMPARE_TIME_LOG
#include "time.h"
static unsigned int dec_time[30] = {0,};
static unsigned int time_cnt = 0;
static unsigned int total_dec_time = 0;
#endif
static unsigned int total_frm = 0;
#else
#define USE_EXTERNAL_BUFFER 0
#endif

//for WFD
#define CONFIG_DATA_SIZE 	(1024*1024)
#define MIN_NAL_STARTCODE_LEN 	    3
#define MAX_NAL_STARTCODE_LEN 	    4

#define AVC_IDR_PICTURE_SEARCH_COUNT	20     // 10 EA

//static unsigned int 	Display_index[31] ={0,};
//static unsigned char 	out_index, in_index;
//static unsigned int 	max_fifo_cnt = VPU_BUFF_COUNT;

#define OMX_BUFF_OFFSET_UNASSIGNED	0xFFFFFFFF
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
#define MARK_BUFF_NOT_USED	0xFFFFFFFF
#define MAX_CHECK_COUNT_FOR_CLEAR	100

//static unsigned int 	Display_Buff_ID[31] ={0,};
//static unsigned int 	buffer_unique_id;
//static unsigned int		used_fifo_count;
//static int g_hFb = -1 ;
//video display mode related with vsync
//static int iVsyncMode;
#endif
//static unsigned int 	video_dec_idx = 0;

//static OMX_BOOL gHDMIOutput = OMX_FALSE;
#endif // HAVE_ANDROID_OS

typedef struct VIDEO_PROFILE_LEVEL
{
    OMX_S32  nProfile;
    OMX_S32  nLevel;
} VIDEO_PROFILE_LEVEL_TYPE;

/* H.263 Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedH263ProfileLevels[] = {
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileISWV2, OMX_VIDEO_H263Level70},
  {-1, -1}};

/* MPEG4 Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedMPEG4ProfileLevels[] ={
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0b},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level1},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level2},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level3},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4a},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level5},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level0},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level0b},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level1},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level2},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level3},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level4},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level5},
  {-1,-1}};

/* AVC Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedAVCProfileLevels[] ={
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel51},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel51},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51},
  {-1,-1}};

/* MPEG2 Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedMPEG2ProfileLevels[] = {
  {OMX_VIDEO_MPEG2ProfileSimple, OMX_VIDEO_MPEG2LevelLL},
  {OMX_VIDEO_MPEG2ProfileSimple, OMX_VIDEO_MPEG2LevelML},
  {OMX_VIDEO_MPEG2ProfileSimple, OMX_VIDEO_MPEG2LevelH14},
  {OMX_VIDEO_MPEG2ProfileSimple, OMX_VIDEO_MPEG2LevelHL},
  {OMX_VIDEO_MPEG2ProfileMain, OMX_VIDEO_MPEG2LevelLL},
  {OMX_VIDEO_MPEG2ProfileMain, OMX_VIDEO_MPEG2LevelML},
  {OMX_VIDEO_MPEG2ProfileMain, OMX_VIDEO_MPEG2LevelHL},
  {OMX_VIDEO_MPEG2ProfileMain, OMX_VIDEO_MPEG2LevelHL},
  {-1, -1}};

/* VPX Supported Levels & profiles */
VIDEO_PROFILE_LEVEL_TYPE SupportedVPXProfileLevels[] = {
  {-1, -1}};

//static cdk_func_t* gspfVDec;
static cdk_func_t* gspfVDecList[VCODEC_MAX] =
{
	vdec_vpu //STD_AVC
	,vdec_vpu //STD_VC1
	,vdec_vpu //STD_MPEG2
	,vdec_vpu //STD_MPEG4
	,vdec_vpu //STD_H263
	,vdec_vpu //STD_DIV3
	,vdec_vpu //STD_RV
	,vdec_vpu //STD_AVS
#ifdef INCLUDE_WMV78_DEC
	,vdec_WMV78 //STD_WMV78
#else
	,0
#endif
#ifdef INCLUDE_SORENSON263_DEC
	,vdec_sorensonH263dec	//STD_SORENSON263
#else
	,vdec_vpu //STD_SH263
#endif
#ifdef JPEG_DECODE_FOR_MJPEG
	,vdec_jpeg
#else
    #ifdef INCLUDE_JPU_MJPEG_DEC
	,vdec_mjpeg_jpu //STD_MJPG
    #else
	,vdec_vpu //STD_MJPG
    #endif
#endif
	,vdec_vpu //STD_VP8
	,vdec_vpu //STD_THEORA
	,vdec_vpu //???
	,vdec_vpu //STD_MVC
};

#define EXT_V_DECODER_TR_TEST

/*
typedef struct dec_disp_info_ctrl_t {
	int		m_iTimeStampType;	//! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS)
	int		m_iStdType;			//! STD type
	int		m_iFmtType;			//! Formater Type

	int		m_iUsedIdxPTS;		//! total number of decoded index for PTS
	int		m_iRegIdxPTS[32];	//! decoded index for PTS
	void	*m_pRegInfoPTS[32];	//! side information of the decoded index for PTS

	int		m_iDecodeIdxDTS;	//! stored DTS index of decoded frame
	int		m_iDispIdxDTS;		//! display DTS index of DTS array
	int		m_iDTS[32];			//! Decode Timestamp (decoding order)

	int		m_Reserved;
} dec_disp_info_ctrl_t;

typedef struct dec_disp_info_t {
	int m_iFrameType;			//! Frame Type

	int m_iTimeStamp;			//! Time Stamp
	int m_iextTimeStamp;			//! TR(RV)

	int m_iPicStructure;		//! PictureStructure
	int m_iM2vFieldSequence;	//! Field sequence(MPEG2)
	int m_iFrameDuration;		//! MPEG2 Frame Duration

	int m_iFrameSize;			//! Frame size
} dec_disp_info_t;

typedef struct dec_disp_info_input_t {
	int m_iFrameIdx;			//! Display frame buffer index for CVDEC_DISP_INFO_UPDATE command
								//! Decoded frame buffer index for CVDEC_DISP_INFO_GET command
	int m_iStdType;				//! STD type for CVDEC_DISP_INFO_INIT
	int m_iTimeStampType;		//! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS) for CVDEC_DISP_INFO_INIT
	int m_iFmtType;				//! Formater Type specification
	int m_iFrameRate;
} dec_disp_info_input_t;
*/
//dec_disp_info_ctrl_t dec_disp_info_ctrl;
//dec_disp_info_t dec_disp_info[32];
//dec_disp_info_input_t	dec_disp_info_input;
/*
#ifdef TIMESTAMP_CORRECTION
typedef struct pts_ctrl{
	int m_iLatestPTS;
	int m_iPTSInterval;
	int m_iRamainingDuration;
} pts_ctrl;
//pts_ctrl gsPtsInfo;
#endif
*/
//OMX_U32 gVideo_FrameRate = 30;

static OMX_ERRORTYPE omx_videodec_component_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes);

static OMX_ERRORTYPE omx_videodec_component_UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer);

static OMX_ERRORTYPE omx_videodec_component_FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);

static int init_stBuffer_Management(omx_videodec_component_PrivateType* omx_private);

static OMX_BOOL isSWCodec(OMX_S32 format);

/** internal function to set codec related parameters in the private type structure
  */
static void SetInternalVideoParameters(OMX_COMPONENTTYPE *openmaxStandComp) {

	omx_videodec_component_PrivateType* omx_private;
	omx_base_video_PortType *inPort ;

	omx_private = openmaxStandComp->pComponentPrivate;;
	OMX_U32  video_coding_type = omx_private->pVideoDecodInstance.video_coding_type;

	if (video_coding_type == OMX_VIDEO_CodingRV) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/rv");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingRV;

		setHeader(&omx_private->pVideoRv, sizeof(OMX_VIDEO_PARAM_RVTYPE));
#if 0
		omx_private->pVideoRv.nSize = 0;
		omx_private->pVideoRv.nPortIndex = 0;
		omx_private->pVideoRv.nPFrames = 0;
		omx_private->pVideoRv.nBFrames = 0;
		omx_private->pVideoRv.eProfile = OMX_VIDEO_MPEG2ProfileSimple;
		omx_private->pVideoRv.eLevel = OMX_VIDEO_RVLevelLL;
#endif

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingRV;
	}
#ifdef OPENMAX1_2
	else if (video_coding_type == OMX_VIDEO_CodingSorenson) {
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingSorenson;
#else
	else if (video_coding_type == OMX_VIDEO_CodingFLV1) {
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/x-flv");
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingFLV1;
#endif

		setHeader(&omx_private->pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
		#if 0
		omx_private->pVideoH263.nPortIndex = 0x0;
		omx_private->pVideoH263.eProfile = OMX_VIDEO_H263ProfileBaseline; //OMX_VIDEO_MPEG4ProfileCore
		omx_private->pVideoH263.eLevel = OMX_VIDEO_H263Level45;
		omx_private->pVideoH263.bPLUSPTYPEAllowed = OMX_FALSE;
		omx_private->pVideoH263.nAllowedPictureTypes = 0;
		omx_private->pVideoH263.bForceRoundingTypeToZero = OMX_TRUE;
		omx_private->pVideoH263.nPictureHeaderRepetition = 0;
		omx_private->pVideoH263.nGOBHeaderInterval = 0;
		omx_private->pVideoH263.nPFrames = 0;
		omx_private->pVideoH263.nBFrames = 0;
	#endif

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
#ifdef OPENMAX1_2
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingSorenson;
#else
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingFLV1;
#endif
	}
	else if (video_coding_type == OMX_VIDEO_CodingAVS) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/avs-video");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVS;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingAVS;
	}
	else if (video_coding_type == OMX_VIDEO_CodingH263) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/h263");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;

		setHeader(&omx_private->pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
		omx_private->pVideoH263.nPortIndex = 0x0;
		omx_private->pVideoH263.eProfile = OMX_VIDEO_H263ProfileBaseline; //OMX_VIDEO_MPEG4ProfileCore
		omx_private->pVideoH263.eLevel = OMX_VIDEO_H263Level45;
		omx_private->pVideoH263.bPLUSPTYPEAllowed = OMX_FALSE;
		omx_private->pVideoH263.nAllowedPictureTypes = 0;
		omx_private->pVideoH263.bForceRoundingTypeToZero = OMX_TRUE;
		omx_private->pVideoH263.nPictureHeaderRepetition = 0;
		omx_private->pVideoH263.nGOBHeaderInterval = 0;
		omx_private->pVideoH263.nPFrames = 0;
		omx_private->pVideoH263.nBFrames = 0;


		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingH263;
	}
	else if (video_coding_type == OMX_VIDEO_CodingAVC) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/avc(h264)");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;

		setHeader(&omx_private->pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
		omx_private->pVideoAvc.nPortIndex = 0;
		omx_private->pVideoAvc.nSliceHeaderSpacing = 0;
		omx_private->pVideoAvc.bUseHadamard = OMX_FALSE;
		omx_private->pVideoAvc.nRefFrames = 2;
		omx_private->pVideoAvc.nPFrames = 0;
		omx_private->pVideoAvc.nBFrames = 0;
		omx_private->pVideoAvc.bUseHadamard = OMX_FALSE;
		omx_private->pVideoAvc.nRefFrames = 2;
		omx_private->pVideoAvc.eProfile = OMX_VIDEO_AVCProfileBaseline;
		omx_private->pVideoAvc.eLevel = OMX_VIDEO_AVCLevel1;
		omx_private->pVideoAvc.nAllowedPictureTypes = 0;
		omx_private->pVideoAvc.bFrameMBsOnly = OMX_FALSE;
		omx_private->pVideoAvc.nRefIdx10ActiveMinus1 = 0;
		omx_private->pVideoAvc.nRefIdx11ActiveMinus1 = 0;
		omx_private->pVideoAvc.bEnableUEP = OMX_FALSE;
		omx_private->pVideoAvc.bEnableFMO = OMX_FALSE;
		omx_private->pVideoAvc.bEnableASO = OMX_FALSE;
		omx_private->pVideoAvc.bEnableRS = OMX_FALSE;

		omx_private->pVideoAvc.bMBAFF = OMX_FALSE;
		omx_private->pVideoAvc.bEntropyCodingCABAC = OMX_FALSE;
		omx_private->pVideoAvc.bWeightedPPrediction = OMX_FALSE;
		omx_private->pVideoAvc.nWeightedBipredicitonMode = 0;
		omx_private->pVideoAvc.bconstIpred = OMX_FALSE;
		omx_private->pVideoAvc.bDirect8x8Inference = OMX_FALSE;
		omx_private->pVideoAvc.bDirectSpatialTemporal = OMX_FALSE;
		omx_private->pVideoAvc.nCabacInitIdc = 0;
		omx_private->pVideoAvc.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterDisable;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingAVC;
	}
	else if (video_coding_type == OMX_VIDEO_CodingMPEG4) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/mpeg4");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;

		setHeader(&omx_private->pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
		omx_private->pVideoMpeg4.nPortIndex = 0;
		omx_private->pVideoMpeg4.nSliceHeaderSpacing = 0;
		omx_private->pVideoMpeg4.bSVH = OMX_FALSE;
		omx_private->pVideoMpeg4.bGov = OMX_FALSE;
		omx_private->pVideoMpeg4.nPFrames = 0;
		omx_private->pVideoMpeg4.nBFrames = 0;
		omx_private->pVideoMpeg4.nIDCVLCThreshold = 0;
		omx_private->pVideoMpeg4.bACPred = OMX_FALSE;
		omx_private->pVideoMpeg4.nMaxPacketSize = 0;
		omx_private->pVideoMpeg4.nTimeIncRes = 0;
		omx_private->pVideoMpeg4.eProfile = OMX_VIDEO_MPEG4ProfileSimple;
		omx_private->pVideoMpeg4.eLevel = OMX_VIDEO_MPEG4Level0;
		omx_private->pVideoMpeg4.nAllowedPictureTypes = 0;
		omx_private->pVideoMpeg4.nHeaderExtension = 0;
		omx_private->pVideoMpeg4.bReversibleVLC = OMX_FALSE;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingMPEG4;
	}
	else if (video_coding_type == OMX_VIDEO_CodingWMV) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/wmv");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingWMV;

		setHeader(&omx_private->pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
		omx_private->pVideoWmv.nPortIndex = 0;
		omx_private->pVideoWmv.eFormat = OMX_VIDEO_WMVFormat9;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingWMV;
	}
	else if (video_coding_type == OMX_VIDEO_CodingWMV_1_2) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/x-wmv-1-2");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingWMV_1_2;

		setHeader(&omx_private->pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
		omx_private->pVideoWmv.nPortIndex = 0;
		omx_private->pVideoWmv.eFormat = OMX_VIDEO_WMVFormat9;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingWMV_1_2;
	}
	else if (video_coding_type == OMX_VIDEO_CodingDIVX) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/divx");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingDIVX;

		//setHeader(&omx_private->pVideoWmv, sizeof(OMX_VIDEO_PARAM_DIVXTYPE));
		//omx_private->pVideoWmv.nPortIndex = 0;
		//omx_private->pVideoWmv.eFormat = ;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingDIVX;
	}
	else if (video_coding_type == OMX_VIDEO_CodingMPEG2) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/mpeg2");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG2;

		setHeader(&omx_private->pVideoWmv, sizeof(OMX_VIDEO_PARAM_MPEG2TYPE));
		omx_private->pVideoMpeg4.nPFrames = 0;
		omx_private->pVideoMpeg4.nBFrames = 0;
		omx_private->pVideoMpeg4.eProfile = OMX_VIDEO_MPEG2ProfileSimple;
		omx_private->pVideoMpeg4.eLevel = OMX_VIDEO_MPEG2LevelHL;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingMPEG2;
	}
	else if (video_coding_type == OMX_VIDEO_CodingMJPEG) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/x-jpeg");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMJPEG;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingMJPEG;
	}
	else if (video_coding_type == OMX_VIDEO_CodingVP8) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/x-vnd.on2.vp8");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingVP8;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingVP8;
	}
	else if (video_coding_type == OMX_VIDEO_CodingMVC) {
#ifndef OPENMAX1_2
		strcpy(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.cMIMEType,"video/x-mvc");
#endif
		omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMVC;

		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		inPort->sVideoParam.eCompressionFormat = OMX_VIDEO_CodingMVC;
	}

}

OMX_ERRORTYPE omx_videodec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp) {
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	OMX_U32  video_coding_type = omx_private->pVideoDecodInstance.video_coding_type;
	if(video_coding_type == OMX_VIDEO_CodingRV) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_RV_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingH263) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_H263_NAME));
	}
#ifdef OPENMAX1_2
	else if(video_coding_type == OMX_VIDEO_CodingSorenson) {
#else
	else if(video_coding_type == OMX_VIDEO_CodingFLV1) {
#endif
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_SORENSON_H263_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingAVC) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_H264_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingMPEG4) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_MPEG4_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingWMV) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_WMV_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingWMV_1_2) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_WMV12_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingDIVX) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_DIVX_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingMPEG2) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_MPEG2_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingMJPEG) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_MJPEG_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingAVS) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_AVS_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingVP8) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_VP8_NAME));
	}
	else if(video_coding_type == OMX_VIDEO_CodingMVC) {
		return (omx_videodec_component_Constructor(openmaxStandComp, VIDEO_DEC_MVC_NAME));
	}
	return OMX_ErrorComponentNotFound;
}
/** The Constructor of the video decoder component
  * @param openmaxStandComp the component handle to be constructed
  * @param cComponentName is the name of the constructed component
  */
OMX_ERRORTYPE omx_videodec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName) {

	OMX_ERRORTYPE eError = OMX_ErrorNone;
	omx_videodec_component_PrivateType* omx_private;
	omx_base_video_PortType *inPort,*outPort;
	OMX_U32 i;

#ifdef HAVE_ANDROID_OS
		if (1)
		{
				DBUG_MSG("In %s, allocating component\n", __func__);
				openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_videodec_component_PrivateType));
				if(openmaxStandComp->pComponentPrivate == NULL)
				{
					return OMX_ErrorInsufficientResources;
				}
				memset(openmaxStandComp->pComponentPrivate, 0x00, sizeof(omx_videodec_component_PrivateType));
		}
		else
#else
		if(!openmaxStandComp->pComponentPrivate)
		{
				DBUG_MSG("In %s, allocating component\n", __func__);
				openmaxStandComp->pComponentPrivate = TCC_calloc(1, sizeof(omx_videodec_component_PrivateType));
				if(openmaxStandComp->pComponentPrivate == NULL)
				{
					return OMX_ErrorInsufficientResources;
				}
		}
		else
#endif
		{
			DBUG_MSG("In %s, Error Component %x Already Allocated\n", __func__, (int)openmaxStandComp->pComponentPrivate);
		}

		omx_private = openmaxStandComp->pComponentPrivate;
		omx_private->pVideoDecodInstance.isVPUClosed = OMX_TRUE;
		omx_private->ports = NULL;
		omx_private->gralloc_info.fd_copy = omx_private->mTmem_fd = -1;
		omx_private->gralloc_info.nTimestamp = 0;

		omx_private->BufferType = 0;

#if defined(TCC_88XX_INCLUDE)
		omx_private->g_hFb = -1;
#endif
		eError = omx_base_filter_Constructor(openmaxStandComp, cComponentName);

		omx_private->sPortTypesParam[OMX_PortDomainVideo].nStartPortNumber = 0;
		omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts = 2;

		/** Allocate Ports and call port constructor. */
		if (omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts && !omx_private->ports) {
			omx_private->ports = TCC_calloc(omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts, sizeof(omx_base_PortType *));
			if (!omx_private->ports) {
				eError = OMX_ErrorInsufficientResources;
				goto Error;
			}
			for (i=0; i < omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++) {
				omx_private->ports[i] = TCC_calloc(1, sizeof(omx_base_video_PortType));
				if (!omx_private->ports[i]) {
					eError = OMX_ErrorInsufficientResources;
					goto Error;
				}
			}
		}

		base_video_port_Constructor(openmaxStandComp, &omx_private->ports[0], 0, OMX_TRUE);
		base_video_port_Constructor(openmaxStandComp, &omx_private->ports[1], 1, OMX_FALSE);

		memset(&omx_private->pVideoDecodInstance.gsVDecUserInfo, 0x00, sizeof(vdec_user_info_t));
		memset(&omx_private->pVideoDecodInstance.gsVDecInput, 0x00, sizeof (vdec_input_t));
		memset(&omx_private->pVideoDecodInstance.gsVDecOutput, 0x00, sizeof (vdec_output_t));
		memset(&omx_private->pVideoDecodInstance.gsVDecInit, 0x00, sizeof (vdec_init_t));

		omx_private->pVideoDecodInstance.pVdec_Instance = vdec_alloc_instance();
		if(omx_private->pVideoDecodInstance.pVdec_Instance == NULL)
		{
			LOGE("vdec_alloc_instance() = null");
			eError = OMX_ErrorComponentNotFound;
			goto Error;
		}
		omx_private->pVideoDecodInstance.nVdec_Instance = vdec_get_instance_index(omx_private->pVideoDecodInstance.pVdec_Instance);

		omx_private->pVideoDecodInstance.gspfVDec = gspfVDecList[omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat];
		if (omx_private->pVideoDecodInstance.gspfVDec == 0)
		{
			eError = OMX_ErrorComponentNotFound;
			goto Error;
		}
		/** now it's time to know the video coding type of the component */
		if(!strcmp(cComponentName, VIDEO_DEC_RV_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingRV;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_RV;
		}else if(!strcmp(cComponentName, VIDEO_DEC_H263_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingH263;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_H263;
		}
		#ifdef INCLUDE_SORENSON263_DEC
		else if(!strcmp(cComponentName, VIDEO_DEC_SORENSON_H263_NAME)) {
#ifdef OPENMAX1_2
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingSorenson;
#else
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingFLV1;
#endif
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_SORENSON263;
		}
		#endif
		#if defined(SUPPORT_SORENSON_SPARK_H_263)
		else if(!strcmp(cComponentName, VIDEO_DEC_SORENSON_H263_NAME)) {
#ifdef OPENMAX1_2
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingSorenson;
#else
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingFLV1;
#endif
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_SH263;
		}
		#endif
		else if(!strcmp(cComponentName, VIDEO_DEC_H264_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingAVC;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_AVC;
		}else if(!strcmp(cComponentName, VIDEO_DEC_MPEG4_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMPEG4;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_MPEG4;
		}else if(!strcmp(cComponentName, VIDEO_DEC_WMV_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingWMV;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_VC1;
#ifdef INCLUDE_WMV78_DEC
		}else if(!strcmp(cComponentName, VIDEO_DEC_WMV12_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingWMV_1_2;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_WMV78;
#endif
		}else if (!strcmp(cComponentName, VIDEO_DEC_DIVX_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingDIVX;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_DIV3;
		}else if(!strcmp(cComponentName, VIDEO_DEC_MPEG2_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMPEG2;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_MPEG2;
		}else if(!strcmp(cComponentName, VIDEO_DEC_MJPEG_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMJPEG;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_MJPG;
		}else if(!strcmp(cComponentName, VIDEO_DEC_AVS_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingAVS;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_AVS;
		}else if(!strcmp(cComponentName, VIDEO_DEC_VP8_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingVP8;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_VP8;
		}else if(!strcmp(cComponentName, VIDEO_DEC_MVC_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMVC;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat	= STD_MVC;
		}else if (!strcmp(cComponentName, VIDEO_DEC_BASE_NAME)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingUnused;
		} else {
			// IL client specified an invalid component name
			eError = OMX_ErrorInvalidComponentName;
			goto Error;
		}

		omx_private->pVideoDecodInstance.gspfVDec = gspfVDecList[omx_private->gsVDecInit.m_iBitstreamFormat];

		/** here we can override whatever defaults the base_component constructor set
		* e.g. we can override the function pointers in the private struct
		*/

		/** Domain specific section for the ports.
		* first we set the parameter common to both formats
		*/
		//common parameters related to input port.
		inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
		//ZzaU:: change input buffer size because DEFAULT_OUT_BUFFER_SIZE increased.
		inPort->sPortParam.nBufferSize = (OMX_U32) VIDEO_DEC_IN_BUFFER_SIZE;//DEFAULT_OUT_BUFFER_SIZE;
		inPort->sPortParam.format.video.xFramerate = 30;
		inPort->sPortParam.format.video.nFrameWidth = AVAILABLE_MAX_WIDTH;
		inPort->sPortParam.format.video.nFrameHeight = AVAILABLE_MAX_HEIGHT;
		inPort->sPortParam.nBufferCountMin = kNumInputBuffers;
		inPort->sPortParam.nBufferCountActual = inPort->sPortParam.nBufferCountMin;


		//common parameters related to output port
		outPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
		outPort->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
		outPort->sPortParam.format.video.nFrameWidth = AVAILABLE_MAX_WIDTH;
		outPort->sPortParam.format.video.nFrameHeight = AVAILABLE_MAX_HEIGHT;
		outPort->sPortParam.nBufferSize =  (OMX_U32) (AVAILABLE_MAX_WIDTH*AVAILABLE_MAX_HEIGHT*3/2);
		outPort->sPortParam.format.video.xFramerate = 30;

#ifdef HAVE_ANDROID_OS
		{
			char value[PROPERTY_VALUE_MAX], Rplayerstream[PROPERTY_VALUE_MAX];

            omx_private->bVSyncMode = omx_private->blocalPlaybackMode = OMX_FALSE;
            property_get("tcc.video.lplayback.mode", value, "");
            if( atoi(value) != 0) {
		        omx_private->blocalPlaybackMode = 1;
			}

			omx_private->bVSyncMode = 0;
			outPort->sPortParam.nBufferCountMin = kNumOutputBuffers;
		}

		outPort->sPortParam.nBufferCountActual = outPort->sPortParam.nBufferCountMin + kNumNativeMinBuffers;

		property_set("tcc.video.multidecoding.check", "0");
#endif

		{

			char wfdmode[PROPERTY_VALUE_MAX];
			property_get("tcc.media.wfd.sink.run", wfdmode, "0");
			if(atoi(wfdmode))
			{
				ALOGI("WFD Sink Play....");
				omx_private->bWFD_mode = OMX_TRUE;
			}
			else
				omx_private->bWFD_mode = OMX_FALSE;

			omx_private->mWFD_PrevVPUPOC = 0;

			//WFD HDCP2 Enable check
			char value[PROPERTY_VALUE_MAX];
			property_get("tcc.hdcp2.session.started", value, "0");
			omx_private->bTEEEnable = atoi(value) == 1 ? OMX_TRUE : OMX_FALSE;
			if( omx_private->bTEEEnable == OMX_TRUE ){
				ALOGI(" HDCP enable for SINK");
			}

			property_get("tcc.tee.session.started", value, "0");
			omx_private->bTEEEnable = atoi(value) == 1 ? OMX_TRUE : omx_private->bTEEEnable;
			if( omx_private->bTEEEnable == OMX_TRUE ){
				ALOGI("Secure memory copy is needed");
			}

			//Display Level check
			omx_private->bWFDErrorFrame = OMX_FALSE;
			property_get("tcc.wfd.display.level.enable", value, "0");
			omx_private->bWFDDisplayEnable = atoi(value);

			property_get("tcc.wfd.display.level", value, "0");
			omx_private->bWFDDisplayLV = atoi(value);

			if(omx_private->bWFD_mode == OMX_TRUE && omx_private->bWFDDisplayEnable != 0) {
				ALOGI("Source Device is support IDR-Request method. Display Level %d", omx_private->bWFDDisplayLV);
			} else if (omx_private->bWFD_mode == OMX_TRUE) {
				ALOGI("Source Device is not support IDR-Request method.");
			}
			property_get("tcc.hdcp2.src.session.started", value, "0");
			omx_private->bWFDHDCP2SrcEnable = atoi(value) == 1 ? OMX_TRUE : OMX_FALSE;
			if( omx_private->bWFDHDCP2SrcEnable == OMX_TRUE ){
				ALOGI(" HDCP enable for SOURCE");
			}
		}

		{
			char value[PROPERTY_VALUE_MAX];
			
	    	property_get("tcc.video.dec.no.bufferdelay", value, "0");
			if( atoi(value) != 0 )
				omx_private->bNoBufferDelay = OMX_TRUE;
			else
				omx_private->bNoBufferDelay = OMX_FALSE;
		}

		/** settings of output port parameter definition */
		if(isSWCodec(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat)
#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
			|| (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMJPEG)
#endif
		)
		{
			outPort->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
			outPort->sVideoParam.eColorFormat = OMX_COLOR_FormatYUV420Planar;
		}
		else
		{
			outPort->sPortParam.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
			outPort->sVideoParam.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
		}

		outPort->sVideoParam.xFramerate = 30;

		if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat >= STD_AVC)
			omx_private->pVideoDecodInstance.gspfVDec = gspfVDecList[omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat];
		else
			omx_private->pVideoDecodInstance.gspfVDec = 0;

		if(omx_private->pVideoDecodInstance.gspfVDec == 0)
		{
			eError = OMX_ErrorComponentNotFound;
			goto Error;
		}

		if(!omx_private->avCodecSyncSem) {
			omx_private->avCodecSyncSem = TCC_malloc(sizeof(tsem_t));
			if(omx_private->avCodecSyncSem == NULL) {
				eError = OMX_ErrorInsufficientResources;
				goto Error;
			}
			tsem_init(omx_private->avCodecSyncSem, 0);
		}

		SetInternalVideoParameters(openmaxStandComp);

		omx_private->eOutFramePixFmt = PIX_FMT_YUV420P;

		if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingRV) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingRV;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingH263) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingH263;
		}
#ifdef OPENMAX1_2
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingSorenson) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingSorenson;
#else
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingFLV1) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingFLV1;
#endif
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG4;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingWMV;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV_1_2) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingWMV_1_2;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingDIVX) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingDIVX;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG2) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMPEG2;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMJPEG) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMJPEG;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVS) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingAVS;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingVP8) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingVP8;
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC) {
			omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat = OMX_VIDEO_CodingMVC;
		}

		/** general configuration irrespective of any video formats
		* setting other parameters of omx_videodec_component_private
		*/
		//  omx_private->avCodec = NULL;
		//  omx_private->avCodecContext= NULL;
		omx_private->pVideoDecodInstance.avcodecReady = OMX_FALSE;
		omx_private->extradata = NULL;
		omx_private->extradata_size = 0;
		omx_private->BufferMgmtCallback = omx_videodec_component_BufferMgmtCallback;

		/** initializing the codec context etc that was done earlier by ffmpeglibinit function */
		omx_private->messageHandler = omx_videodec_component_MessageHandler;
		omx_private->destructor = omx_videodec_component_Destructor;

		openmaxStandComp->SetParameter = omx_videodec_component_SetParameter;
		openmaxStandComp->GetParameter = omx_videodec_component_GetParameter;
		openmaxStandComp->SetConfig    = omx_videodec_component_SetConfig;
		openmaxStandComp->GetConfig    = omx_videodec_component_GetConfig;
		openmaxStandComp->ComponentRoleEnum = omx_videodec_component_ComponentRoleEnum;
		openmaxStandComp->GetExtensionIndex = omx_videodec_component_GetExtensionIndex;

#ifdef HAVE_ANDROID_OS
		openmaxStandComp->AllocateBuffer = omx_videodec_component_AllocateBuffer;
		openmaxStandComp->UseBuffer = omx_videodec_component_UseBuffer;
		openmaxStandComp->FreeBuffer = omx_videodec_component_FreeBuffer;
#endif

		omx_private->pConfigdata = TCC_calloc(1,CONFIG_DATA_SIZE);
		omx_private->szConfigdata = 0;
		omx_private->seq_header_init_error_count = SEQ_HEADER_INIT_ERROR_COUNT;
		omx_private->displaying_error_count = DISPLAYING_FAIL_IN_THUMBNAIL_MODE_ERROR_COUNT;

#ifdef DIVX_DRM5
		if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MPEG4)
		{
			DivxDecryptExInit();
		}
#endif

#if defined(TCC_88XX_INCLUDE)
		if((omx_private->g_hFb = open(FB0_DEVICE, O_RDWR)) < 0)
		{
			LOGE("fb driver open fail reason:'%s'", strerror(errno));
		}
#endif

#ifdef RESTORE_DECODE_ERR
		omx_private->seqHeader_backup = NULL;
		omx_private->seqHeader_len = 0;
		omx_private->cntDecError = 0;
#endif

		omx_private->pVideoDecodInstance.avcodecReady = OMX_FALSE;
		omx_private->pVideoDecodInstance.video_dec_idx = 0;
		omx_private->pVideoDecodInstance.bitrate_mbps = 20; //default 20Mbps

		memset(omx_private->pVideoDecodInstance.dec_disp_info, 0x00, sizeof(dec_disp_info_t)*32);
		memset(&omx_private->pVideoDecodInstance.dec_disp_info_input, 0x00, sizeof(dec_disp_info_input_t));

		init_stBuffer_Management(omx_private);

		omx_private->max_fifo_cnt = VPU_BUFF_COUNT;
		omx_private->gHDMIOutput = OMX_FALSE;

		omx_private->buffer_unique_id = 0;
		omx_private->gVideo_FrameRate = 30;
		omx_private->ConsecutiveBufferFullCnt = 0;

#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
		omx_private->sequence_header_only = NULL;
		omx_private->sequence_header_size = 0;
		omx_private->need_sequence_header_attachment = OMX_FALSE;
#endif
		omx_private->bDetected_res_changed = omx_private->bSet_Except_I_Frm = omx_private->bNoOutput_after_res_changed = OMX_FALSE;
		omx_private->prev_interval = omx_private->prev_timestamp = 0;

		omx_private->bNeed_TS_adjustment = omx_private->bTopDown_adjustmemt = OMX_FALSE;
		omx_private->uTS_last_output = 0;
		omx_private->nST_last_output = 0;
		omx_private->uTS_new_base = 0;
		omx_private->uTS_curr_output = 0;

#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
#ifdef COMPARE_TIME_LOG
		time_cnt = total_dec_time = 0;
#endif
		total_frm = 0;
		omx_private->gralloc_info.m_pDispOut[PA][0] = omx_private->gralloc_info.m_pDispOut[PA][1] = omx_private->gralloc_info.m_pDispOut[PA][2] = NULL;
		omx_private->gralloc_info.m_pDispOut[VA][0] = omx_private->gralloc_info.m_pDispOut[VA][1] = omx_private->gralloc_info.m_pDispOut[VA][2] = NULL;
#endif

#if defined(ANDROID_USE_GRALLOC_BUFFER) 
		hw_module_t const* module;
		int err;

		err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
		if (err == 0)
		{
			omx_private->gralloc_info.grallocModule = (gralloc_module_t const *)module;
		}
		else
		{
			LOGE("ERROR: can't load gralloc using hw_get_module(%s) => err[0x%x]", GRALLOC_HARDWARE_MODULE_ID, err);
			err = OMX_ErrorInsufficientResources;
		}
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
		int ret =  gralloc_open((hw_module_t const*)omx_private->gralloc_info.grallocModule, &(omx_private->gralloc_info.mAllocDev));
#endif
#endif

		{
			omx_private->mTmem_fd = open(TMEM_DEVICE, O_RDWR);
			if (omx_private->mTmem_fd < 0) {
				LOGE("can't open[%s] '%s'", strerror(errno), TMEM_DEVICE);
			}
		    else {
				omx_private->bDRAM_16bit = OMX_FALSE;
				if( ioctl( omx_private->mTmem_fd, TCC_DRAM_16BIT_USED, 0) > 0 ) {
					omx_private->bDRAM_16bit = OMX_TRUE;
				}

		        pmap_get_info("ump_reserved", &omx_private->mUmpReservedPmap);
				if(omx_private->mUmpReservedPmap.size > 0)
				{
			        if( ( omx_private->mTMapInfo = (void*)mmap(0, omx_private->mUmpReservedPmap.size, PROT_READ | PROT_WRITE, MAP_SHARED, omx_private->mTmem_fd, omx_private->mUmpReservedPmap.base) ) == MAP_FAILED ) {
			            LOGE("%s device's ump_reserved's mmap failed.", TMEM_DEVICE);
			        }
				}

		        pmap_get_info("secured_inbuff", &omx_private->mUmpSecuredInPmap);
				if(omx_private->mUmpSecuredInPmap.size > 0)
				{
			        if( ( omx_private->mSecureInMapInfo = (void*)mmap(0, omx_private->mUmpSecuredInPmap.size, PROT_READ | PROT_WRITE, MAP_SHARED, omx_private->mTmem_fd, omx_private->mUmpSecuredInPmap.base) ) == MAP_FAILED ) {
			            LOGE("%s device's secured_inbuffer's mmap failed.", TMEM_DEVICE);
			        }
				}

				pmap_get_info("video_sbackup", &omx_private->mSeqBackupPmap);
				if(omx_private->mSeqBackupPmap.size > 0)
				{
			        if( ( omx_private->mSeqBackupMapInfo = (void*)mmap(0, omx_private->mSeqBackupPmap.size, PROT_READ | PROT_WRITE, MAP_SHARED, omx_private->mTmem_fd, omx_private->mSeqBackupPmap.base) ) == MAP_FAILED ) {
			            LOGE("%s device's secured_inbuffer's mmap failed.", TMEM_DEVICE);
			        }
				}
		    }
		}

#if defined(TCC_88XX_INCLUDE)
		omx_private->mProxy_fd = omx_private->g_hFb;
#else
		omx_private->mProxy_fd = omx_private->mTmem_fd;
#endif

		omx_private->MVC_Base_addr0=0;
		omx_private->MVC_Base_addr1=0;
		omx_private->MVC_Base_addr2=0;

		omx_private->bThumbnailMode = OMX_FALSE;
		omx_private->bSecuredInBufferMode = omx_private->bWFDSyncMode = OMX_FALSE;

		omx_private->scale.xWidth = 0x10000;
		omx_private->scale.xHeight = 0x10000;

		omx_private->gralloc_info.mDispIndex = -10;
		omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_INPUTPORT_INDEX].BufferType = 0;
		omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType = 0;
Error:
		if( eError != OMX_ErrorNone )
			omx_videodec_component_Destructor(openmaxStandComp);

		return eError;
}


/** The destructor of the video decoder component
  */
OMX_ERRORTYPE omx_videodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp) {

	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	OMX_U32 i;

	//to make sure that VPU close.
	OMX_S32 ret;

	if(omx_private->pVideoDecodInstance.isVPUClosed == OMX_FALSE)
	{
		if( (ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_CLOSE, NULL, NULL, &(omx_private->pVideoDecodInstance.gsVDecOutput), (omx_private->pVideoDecodInstance.pVdec_Instance))) < 0 )
		{
			LOGE( "[VDEC_CLOSE] [Err:%ld] video decoder Deinit", ret );
		}
		omx_private->pVideoDecodInstance.isVPUClosed = OMX_TRUE;
	}

	#ifdef DIVX_DRM5
	if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MPEG4)
	{
		DivxDecryptExDeinit();
	}
	#endif

	if(omx_private->pVideoDecodInstance.pVdec_Instance)
		vdec_release_instance(omx_private->pVideoDecodInstance.pVdec_Instance);

	if(omx_private->extradata) {
		TCC_free(omx_private->extradata);
		omx_private->extradata=NULL;
	}

	if(omx_private->avCodecSyncSem) {
		tsem_deinit(omx_private->avCodecSyncSem);
		TCC_free(omx_private->avCodecSyncSem);
		omx_private->avCodecSyncSem = NULL;
	}

#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
	if(omx_private->sequence_header_only) {
		TCC_free(omx_private->sequence_header_only);
		omx_private->sequence_header_only = NULL;
	}
#endif
	/* frees port/s */
	if (omx_private->ports) {
		for (i=0; i < omx_private->sPortTypesParam[OMX_PortDomainVideo].nPorts; i++) {
		if(omx_private->ports[i])
			omx_private->ports[i]->PortDestructor(omx_private->ports[i]);
		}
		TCC_free(omx_private->ports);
		omx_private->ports=NULL;
	}

	if( omx_private->pThumbnailBuff )
		TCC_free(omx_private->pThumbnailBuff);
	if( omx_private->pConfigdata )
		TCC_free(omx_private->pConfigdata);
#ifdef RESTORE_DECODE_ERR
	if(omx_private->seqHeader_backup != NULL)
		TCC_free(omx_private->seqHeader_backup);
#endif
	DBUG_MSG("Destructor of video decoder component is called\n");

	omx_base_filter_Destructor(openmaxStandComp);
	omx_private->bThumbnailMode = OMX_FALSE;

#if defined(TCC_88XX_INCLUDE)
    if(omx_private->g_hFb > 0)
    {
		if(close( omx_private->g_hFb ) < 0)
		{
			LOGE("fb driver close fail reason:'%s'", strerror(errno));
		}
		omx_private->g_hFb = -1;
    }
#endif

#ifdef MOVE_HW_OPERATION
	if(omx_private->gralloc_info.fd_copy != -1)
		close(omx_private->gralloc_info.fd_copy);
#endif

	if(omx_private->mTmem_fd != -1){
		if( omx_private->mTMapInfo != NULL )
       		munmap((void*)omx_private->mTMapInfo, omx_private->mUmpReservedPmap.size);
		if( omx_private->mSecureInMapInfo != NULL )
        	munmap((void*)omx_private->mSecureInMapInfo, omx_private->mUmpSecuredInPmap.size);
		if( omx_private->mSeqBackupMapInfo != NULL )
			munmap((void*)omx_private->mSeqBackupMapInfo, omx_private->mSeqBackupPmap.size);
		if(close(omx_private->mTmem_fd) < 0)
		{
			LOGE("Error: close(omx_private->mTmem_fd)");
		}
    	omx_private->mTmem_fd = -1;
	}

#if defined(TC_SECURE_MEMORY_COPY)
	if (omx_private->bTEEEnable) {
		TC_SecureMemoryAPI_ServiceEnd();
	}
#endif

#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
	gralloc_close(omx_private->gralloc_info.mAllocDev);
#endif

	LOGI("Video decoder component is destoried.");

	return OMX_ErrorNone;
}


/** It initializates the FFmpeg framework, and opens an FFmpeg videodecoder of type specified by IL client
  */
OMX_ERRORTYPE omx_videodec_component_LibInit(omx_videodec_component_PrivateType* omx_private) {

	OMX_U32 target_codecID;
	char value[PROPERTY_VALUE_MAX];
	OMX_U32 uiHDMIOutputMode = 0;

	tsem_up(omx_private->avCodecSyncSem);

	omx_private->pVideoDecodInstance.avcodecInited = 0;
	omx_private->pVideoDecodInstance.container_type = 0;
//	omx_private->avcodecInited = 0;
//	omx_private->pVideoDecodInstance.container_type = 0;
	omx_private->i_skip_scheme_level = VDEC_SKIP_FRAME_DISABLE;
	omx_private->i_skip_count = TMP_SKIP_OPT_SKIP_INTERVAL;
	omx_private->i_skip_interval = TMP_SKIP_OPT_SKIP_INTERVAL;
//	omx_private->isFirst_Frame = OMX_TRUE;
	omx_private->isFirstSyncFrame = OMX_TRUE;
	omx_private->extractorType = 0;
//	omx_private->bThumbnailMode = OMX_FALSE;

  	omx_private->bDelayedDecodeOut = OMX_FALSE; 
	omx_private->appendable_header_offset = OMX_BUFF_OFFSET_UNASSIGNED;
	omx_private->bSetDecodeOnlyMode = OMX_FALSE;
	omx_private->bUseDecodeOnlyMode = OMX_FALSE;
	omx_private->skipFrameNum = 0;
	omx_private->decodeOnlyErrNum = 0;
	omx_private->numSkipFrame = MAX_DECODE_ONLY_FRAME_NUM;
	omx_private->I_frame_search_mode = AVC_NONIDR_PICTURE_SEARCH_MODE;
	omx_private->IDR_frame_search_count = 0;

	omx_private->bDetectFrameDelimiter = OMX_FALSE;
	omx_private->start_code_with_type = 0xFFFFFFFF;
	omx_private->start_code_header = 0;
	omx_private->start_code_picture = 0;
	omx_private->start_code_picture1 = 0;
	omx_private->frame_delimiter_offset = OMX_BUFF_OFFSET_UNASSIGNED;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	omx_private->buffer_unique_id = 0;
#endif
	init_stBuffer_Management(omx_private);

#ifdef _TCC8900_
	property_get("persist.sys.output_mode", value, "");
	uiHDMIOutputMode = atoi(value);
	if( uiHDMIOutputMode == OUTPUT_HDMI)
#else
	property_get("persist.sys.hdmi_output.enable", value, "");
	if (strcmp(value, "1") == 0)
#endif
	{
		LOGD("HDMI output enabled");
		omx_private->gHDMIOutput = OMX_TRUE;
	}

	omx_private->bAllPortsFlushed = OMX_FALSE;

	return OMX_ErrorNone;
}

/** It Deinitializates the ffmpeg framework, and close the ffmpeg video decoder of selected coding type
  */
void omx_videodec_component_LibDeinit(omx_videodec_component_PrivateType* omx_private)
{
	int ret;

	DBUG_MSG("omx_videodec_component_LibDeinit is called(%d)\n", omx_private->pVideoDecodInstance.isVPUClosed);

	if(omx_private->pVideoDecodInstance.isVPUClosed == OMX_FALSE)
	{
		if( (ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_CLOSE, NULL, NULL, &(omx_private->pVideoDecodInstance.gsVDecOutput), (omx_private->pVideoDecodInstance.pVdec_Instance))) < 0 )
		{
			LOGE( "[VDEC_CLOSE] [Err:%4d] video decoder Deinit", ret );
		}
		omx_private->pVideoDecodInstance.isVPUClosed = OMX_TRUE;
	}
}

/** The Initialization function of the video decoder
  */
OMX_ERRORTYPE omx_videodec_component_Initialize(OMX_COMPONENTTYPE *openmaxStandComp) {

	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	omx_private->ConsecutiveVdecFailCnt = 0; //Reset Consecutive Vdec Fail Counting B060955
    omx_private->maxConsecutiveVdecFailCnt = MAX_CONSECUTIVE_VPU_FAIL_COUNT;

	/** Temporary First Output buffer size */
	omx_private->inputCurrBuffer = NULL;
	omx_private->inputCurrLength = 0;
	omx_private->isFirstBuffer = 1;
	omx_private->isNewBuffer = 1;
	omx_private->isFirstSyncFrame = OMX_TRUE;

  	omx_private->bDelayedDecodeOut = OMX_FALSE; 
	omx_private->appendable_header_offset = OMX_BUFF_OFFSET_UNASSIGNED;
	omx_private->bSetDecodeOnlyMode = OMX_FALSE;
	omx_private->bUseDecodeOnlyMode = OMX_FALSE;
	omx_private->skipFrameNum = 0;
	omx_private->decodeOnlyErrNum = 0;
	omx_private->numSkipFrame = MAX_DECODE_ONLY_FRAME_NUM;
	omx_private->I_frame_search_mode = AVC_NONIDR_PICTURE_SEARCH_MODE;
	omx_private->IDR_frame_search_count = 0;
	omx_private->bPlayDirection = OMX_TRUE; //Normal Direction.
	omx_private->nDelayedOutputBufferCount = 0;

#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
	omx_private->gralloc_info.m_pDispOut[PA][0] = omx_private->gralloc_info.m_pDispOut[PA][1] = omx_private->gralloc_info.m_pDispOut[PA][2] = NULL;
	omx_private->gralloc_info.m_pDispOut[VA][0] = omx_private->gralloc_info.m_pDispOut[VA][1] = omx_private->gralloc_info.m_pDispOut[VA][2] = NULL;
	omx_private->gralloc_info.mDispIndex = -10;
	omx_private->gralloc_info.nTimestamp = 0;
#endif

	omx_private->isNewBuffer = 1;

	return eError;
}

/** The Deinitialization function of the video decoder
  */
OMX_ERRORTYPE omx_videodec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp) {

	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	OMX_ERRORTYPE eError = OMX_ErrorNone;

	DBUG_MSG("omx_videodec_component_Deinit is called(%d)\n", omx_private->pVideoDecodInstance.avcodecReady);

#ifdef RING_BUFFER_MODULE_ENABLE
	omx_vpudec_component_DeinitVpuRingbufferCallback(openmaxStandComp);
#endif

	if (omx_private->pVideoDecodInstance.avcodecReady){
		omx_videodec_component_LibDeinit(omx_private);
		omx_private->pVideoDecodInstance.avcodecReady = OMX_FALSE;
	}

	return eError;
}

/** Executes all the required steps after an output buffer frame-size has changed.
*/
static inline void UpdateFrameSize(OMX_COMPONENTTYPE *openmaxStandComp) {

	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *outPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	omx_base_video_PortType *inPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX];
	outPort->sPortParam.format.video.nFrameWidth =
		inPort->sPortParam.format.video.nFrameWidth + (inPort->sPortParam.format.video.nFrameWidth & 1);
	outPort->sPortParam.format.video.nFrameHeight =
		inPort->sPortParam.format.video.nFrameHeight + (inPort->sPortParam.format.video.nFrameHeight & 1);
	outPort->sPortParam.format.video.xFramerate = inPort->sPortParam.format.video.xFramerate;
	switch(outPort->sVideoParam.eColorFormat) {
		case OMX_COLOR_FormatYUV420Planar:
		case OMX_COLOR_FormatYUV420SemiPlanar:
			if(outPort->sPortParam.format.video.nFrameWidth && outPort->sPortParam.format.video.nFrameHeight) {
				outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3/2;
			}
		break;
		default:
			if(outPort->sPortParam.format.video.nFrameWidth && outPort->sPortParam.format.video.nFrameHeight) {
				outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3;
			}
			break;
	}
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
	if(omx_private->BufferType == DecoderMetadataPtr) {
		outPort->sPortParam.nBufferSize = 8;
	}
#endif
}

#ifdef MOVE_HW_OPERATION
int copy_data_to_grallocbuffer(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BOOL hw_addr, unsigned int width, unsigned int height, unsigned char *YSrc, unsigned char *USrc, unsigned char *VSrc,
							char bSrcYUVInter, OMX_U8 *YDst, OMX_U8 *UDst, OMX_U8 *VDst, COPY_OPER_MODE cmd, OMX_BOOL bOnlyCopy);
#endif

#define COPY_FROM_DECODED_BUFFER	1
#define COPY_FROM_DISPLAY_BUFFER	2
#define COPY_FROM_BLACK_FRAME		3
static void CopyOutputFrame(OMX_COMPONENTTYPE *openmaxStandComp, OMX_U8 *pDestAddress, OMX_U32 *pulCopyedBytes, OMX_S32 lSrcIndex)
{
	omx_videodec_component_PrivateType* p_omx_private = openmaxStandComp->pComponentPrivate;
	OMX_S32 frame_size;
	OMX_S32	y_size, cb_size, cr_size;
	OMX_S32	dst_width, dst_height;
	OMX_S32	y_stride, cbcr_stride;
	OMX_S32 h_off = 0, v_off = 0;
	OMX_U8 *pDstY, *pDstCb, *pDstCr;
	OMX_S32 i, j;
	OMX_U8 *pSrcY = NULL, *pSrcCb = NULL, *pSrcCr = NULL;
#if defined(TC_SECURE_MEMORY_COPY)
	pmap_t mThumbPmap;
	void* mThumbMapInfo = NULL;
#endif

	if(p_omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_AVC || p_omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MVC)
	{
		y_stride = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
		dst_width = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
		dst_width -= p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropLeft;
		dst_width -= p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropRight;

		dst_height = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight;
		dst_height -= p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropBottom;
		dst_height -= p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropTop;

		h_off = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropLeft;
		h_off -= h_off & 1;
		v_off = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropTop;
		v_off -= v_off & 1;
	}
	else
	{
		y_stride = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
		dst_width = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
		dst_height = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight;
	}

	dst_width += dst_width & 1;
	dst_height += dst_height & 1;

	// src size setup
	y_stride = ((y_stride + 15) >> 4) << 4;
	cbcr_stride = y_stride >> 1;

	// dst size setup
	y_size = dst_width * dst_height;
	cb_size = y_size >> 2;
	cr_size = cb_size;
	frame_size = y_size + cb_size + cr_size;

	pDstY = pDestAddress;
	pDstCb = pDstY + y_size;
	pDstCr = pDstCb + cb_size;

	if( lSrcIndex != COPY_FROM_BLACK_FRAME )
	{
#if defined(TC_SECURE_MEMORY_COPY)
		if( p_omx_private->bWFDHDCP2SrcEnable )
		{
			omx_base_video_PortType *outPort = (omx_base_video_PortType *)p_omx_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
			int ret = 0;
			int index = p_omx_private->pVideoDecodInstance.nVdec_Instance;
			unsigned int def_size = 3*1024*1024;
			unsigned int offset = index*def_size;
	
			pmap_get_info("video_thumb", &mThumbPmap);
			if( mThumbPmap.size > 0 ) {
				if( ( mThumbMapInfo = (void*)mmap(0, mThumbPmap.size, PROT_READ | PROT_WRITE, MAP_SHARED, p_omx_private->mTmem_fd, mThumbPmap.base) ) == MAP_FAILED ) {
					LOGE("%s device's secured_inbuffer's mmap failed.", TMEM_DEVICE);
					ret = -1;
				}
			}
			else {
				ret = -1;
			}
	
			if( index > 1 )
				ret = -1;
	
			if( mThumbPmap.size != 0 && mThumbMapInfo != NULL && ret != -1 )
			{
				if( lSrcIndex == COPY_FROM_DECODED_BUFFER ) {
					pSrcY = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][0];
					pSrcCb = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][1];
					pSrcCr = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][2];
				}
				else if( lSrcIndex == COPY_FROM_DISPLAY_BUFFER ) {
					pSrcY = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][0];
					pSrcCb = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][1];
					pSrcCr = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][2];
				}
	
				OMX_U8 *pDestY = mThumbPmap.base + offset;
				OMX_U8 *pDestU = pDestY + (pSrcCb - pSrcY);
				OMX_U8 *pDestV = pDestU + (pSrcCr - pSrcCb);
	
				GBUG_MSG("Thumb copy(%d) :: 0x%x-0x%x-0x%x -> 0x%x-0x%x-0x%x", p_omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode, pSrcY, pSrcCb, pSrcCr, pDestY, pDestU, pDestV);
				ret = copy_data_to_grallocbuffer( openmaxStandComp, OMX_TRUE,
														p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth,
														p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight,
														pSrcY, pSrcCb, pSrcCr, p_omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode,
														pDestY, pDestU, pDestV, SEND_CMD | WAIT_RESPOND, OMX_TRUE);
	
				pSrcY = (OMX_U8*)mThumbMapInfo + offset;
				pSrcCb = pSrcY + (pDestU - pDestY);
				pSrcCr = pSrcCb + (pDestV - pDestU);
			}
	
			if( ret < 0 )
				lSrcIndex = COPY_FROM_BLACK_FRAME;
		}
		else
#endif
		{
			if( lSrcIndex == COPY_FROM_DECODED_BUFFER ) {
				pSrcY = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][0];
				pSrcCb = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][1];
				pSrcCr = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][2];
			}
			else if( lSrcIndex == COPY_FROM_DISPLAY_BUFFER ) {
				pSrcY = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][0];
				pSrcCb = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][1];
				pSrcCr = p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][2];
			}
		}
	}

	if (lSrcIndex == COPY_FROM_BLACK_FRAME)
	{
		memset(pDstY, 0x10, y_size);
		memset(pDstCb, 0x80, cb_size);
		memset(pDstCr, 0x80, cr_size);
	}
	else
	{
		if( p_omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode )
		{
			OMX_U8 *pSrcCbCrTmp;
			OMX_S32 dst_width_cbcr = dst_width >> 1;
			OMX_S32 dst_height_cbcr = dst_height >> 1;

			// cropping
			pSrcY += h_off + y_stride * v_off;
			pSrcCb += ((h_off >> 1) << 1) + (y_stride * (v_off >> 1));

			// luminance
			for(i = 0; i < dst_height; i++)
			{
				memcpy(pDstY, pSrcY, dst_width);
				pDstY += dst_width;
				pSrcY += y_stride;
			}

		#if 1 // In case of ICS, all format is supported.
			// chrominance
			for(i = 0; i < dst_height_cbcr; i++)
			{
				pSrcCbCrTmp = pSrcCb;
				pSrcCb += y_stride;
				memcpy(pDstCb, pSrcCbCrTmp, dst_width_cbcr*2);
				pDstCb += dst_width_cbcr*2;
			}
		#else
			// chrominance
			for(i = 0; i < dst_height_cbcr; i++)
			{
				pSrcCbCrTmp = pSrcCbCr;
				pSrcCbCr += y_stride;
				for(j = 0; j < dst_width_cbcr; j++) {
					*pDstCb++ = *pSrcCbCrTmp++;
					*pDstCr++ = *pSrcCbCrTmp++;
				}
			}
		#endif
		}
		else
		{
			OMX_S32 dst_width_cbcr = dst_width >> 1;

			//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
			if( p_omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MJPG &&
				p_omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1 )
			{
				// cropping
				pSrcY += h_off + y_stride * v_off;
				h_off >>= 1;
				pSrcCb += h_off + (cbcr_stride * v_off);
				pSrcCr += h_off + (cbcr_stride * v_off);

				for(i = 0; i < dst_height; i++)
				{
					// luminance
					memcpy(pDstY, pSrcY, dst_width);
					pDstY += dst_width;
					pSrcY += y_stride;

					// chrominance (4:2:2 to 4:2:0)
					if( i & 1 ) {
						memcpy(pDstCr, pSrcCr, dst_width_cbcr);
						pDstCr += dst_width_cbcr;
					}
					else {
						memcpy(pDstCb, pSrcCb, dst_width_cbcr);
						pDstCb += dst_width_cbcr;
					}
					pSrcCb += cbcr_stride;
					pSrcCr += cbcr_stride;
				}
			}
			else
			{
				OMX_S32 dst_height_cbcr = dst_height >> 1;

				// cropping
				pSrcY += h_off + (y_stride * v_off);
				h_off >>= 1;
				v_off >>= 1;
				pSrcCb += h_off + (cbcr_stride * v_off);
				pSrcCr += h_off + (cbcr_stride * v_off);

				// luminance
				for(i = 0; i < dst_height; i++)
				{
					// luminance
					memcpy(pDstY, pSrcY, dst_width);
					pDstY += dst_width;
					pSrcY += y_stride;
				}

				// chrominance
				for(i = 0; i < dst_height_cbcr; i++)
				{
					memcpy(pDstCb, pSrcCb, dst_width_cbcr);
					pDstCb += dst_width_cbcr;
					pSrcCb += cbcr_stride;
					memcpy(pDstCr, pSrcCr, dst_width_cbcr);
					pDstCr += dst_width_cbcr;
					pSrcCr += cbcr_stride;
				}
			}
		}
	}

#if 0
	{
		FILE *fp;
		OMX_S8 path[256];
		LOGE("Thumbnail frame: %d x %d", dst_width, dst_height);
		sprintf(path, "/sdcard/tflash/thumb_%dx%d.yuv", dst_width, dst_height);
		if( fp = fopen(path, "wb") ) {
			fwrite( p_omx_private->pThumbnailBuff, 1, frame_size, fp);
			fclose(fp);
		}
	}
#endif

	*pulCopyedBytes = frame_size;
#if defined(TC_SECURE_MEMORY_COPY)
	if( mThumbMapInfo != NULL )
	{
        munmap((void*)mThumbMapInfo, mThumbPmap.size);
	}
#endif
}

static int isPortChange(OMX_COMPONENTTYPE *openmaxStandComp)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
    omx_base_video_PortType *outPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	OMX_COLOR_FORMATTYPE colorformat;
	OMX_CONFIG_RECTTYPE rectParm;
	OMX_U32 width, height;
	OMX_U32 bPortChanged, bCropChanged, bScaleChanged;

	bPortChanged = bCropChanged = bScaleChanged = OMX_FALSE;
	int ret = 0;

#ifdef SET_FRAMEBUFFER_INTO_MAX
	if(omx_private->bDetected_res_changed == OMX_TRUE)
	{
		width = omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
		height = omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight;

		bCropChanged = bPortChanged = OMX_TRUE;

		if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_AVC || omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MVC)
		{
			omx_private->rectParm.nLeft		= rectParm.nLeft 	= omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropLeft;
			omx_private->rectParm.nTop		= rectParm.nTop 	= omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropTop;
			omx_private->rectParm.nWidth	= rectParm.nWidth 	= width - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropLeft - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropRight;
			omx_private->rectParm.nHeight	= rectParm.nHeight 	= height - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropBottom - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropTop;
		}
		else
		{
			omx_private->rectParm.nLeft		= rectParm.nLeft 	= 0;
			omx_private->rectParm.nTop		= rectParm.nTop 	= 0;
			omx_private->rectParm.nWidth	= rectParm.nWidth 	= width;
			omx_private->rectParm.nHeight	= rectParm.nHeight 	= height;
		}
	}
	else
#endif
	{
		if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_AVC || omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MVC)
		{
			width = (omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth- omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropLeft - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropRight);
			height = (omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropBottom - omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropTop);

			if((outPort->sPortParam.format.video.nFrameWidth != width) ||
				(outPort->sPortParam.format.video.nFrameHeight != height))
			{
				bPortChanged = OMX_TRUE;
			}

			rectParm.nLeft 		= omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropLeft;
			rectParm.nTop 		= omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iAvcPicCrop.m_iCropTop;
			rectParm.nWidth 	= width;
			rectParm.nHeight 	= height;
		}
		else
		{
			width = omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
			height = omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight;

			if(outPort->sPortParam.format.video.nFrameWidth != width ||
				outPort->sPortParam.format.video.nFrameHeight != height)
			{
				bPortChanged = OMX_TRUE;
			}

			rectParm.nLeft 		= 0;
			rectParm.nTop 		= 0;
			rectParm.nWidth 	= width;
			rectParm.nHeight 	= height;
		}

		if( rectParm.nLeft != omx_private->rectParm.nLeft ||
			rectParm.nTop != omx_private->rectParm.nTop ||
			rectParm.nWidth != omx_private->rectParm.nWidth ||
			rectParm.nHeight != omx_private->rectParm.nHeight)
		{
			omx_private->rectParm.nLeft		= rectParm.nLeft;
			omx_private->rectParm.nTop		= rectParm.nTop;
			omx_private->rectParm.nWidth	= rectParm.nWidth;
			omx_private->rectParm.nHeight	= rectParm.nHeight;
			bCropChanged = OMX_TRUE;
			LOGI("%ldx%ld :: CropInfo Changed %ld,%ld - %ldx%ld", width, height, omx_private->rectParm.nLeft, omx_private->rectParm.nTop, omx_private->rectParm.nWidth, omx_private->rectParm.nHeight);
		}
	}

	if(vdec_getAspectRatio(omx_private->pVideoDecodInstance.pVdec_Instance, &omx_private->scale.xWidth, &omx_private->scale.xHeight, width, height,
							omx_private->pVideoDecodInstance.cdmx_info, omx_private->pVideoDecodInstance.container_type,
							omx_private->extractorType & OMX_BUFFERFLAG_EXTRACTORTYPE_TCC))
	{
		bScaleChanged = OMX_TRUE;
	}

	width += width & 1;
	height += height & 1;

	if(width > AVAILABLE_MAX_WIDTH || ((width *height) > AVAILABLE_MAX_REGION))
	{
		LOGE("%ld x %ld ==> MAX-Resolution(%d x %d) over!!", width, height, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT);
		ret = -1;
	}

	if(width < AVAILABLE_MIN_WIDTH || height < AVAILABLE_MIN_HEIGHT)
	{
		LOGE("%ld x %ld ==> MIN-Resolution(%d x %d) less!!", width, height, AVAILABLE_MIN_WIDTH, AVAILABLE_MIN_HEIGHT);
		ret = -1;
	}

#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
	if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MJPG)
	{
		if(omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode != 1)
		{
			//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
			if(omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
				colorformat = OMX_COLOR_FormatYUV422Planar;
			else
				colorformat = OMX_COLOR_FormatYUV420Planar;

			if(outPort->sPortParam.format.video.eColorFormat != colorformat)
			{
				LOGI( "Change ColorFormat!! %d -> %d", outPort->sPortParam.format.video.eColorFormat, colorformat);

				if(omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
					outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 2;
				else
					outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3/2;

				outPort->sPortParam.format.video.eColorFormat = colorformat;
				bPortChanged = 1;
			}
		}
	}
#endif

	if(bPortChanged || bCropChanged || bScaleChanged)
	{
		if(bPortChanged)
			outPort->bIsPortChanged = OMX_TRUE;

		LOGI( "ReSize Needed!! %ld x %ld -> %ld x %ld \n", outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight, width, height);

		outPort->sPortParam.format.video.nFrameWidth = width;
        outPort->sPortParam.format.video.nFrameHeight = height;

        switch(outPort->sVideoParam.eColorFormat) {
                case OMX_COLOR_FormatYUV420Planar:
                case OMX_COLOR_FormatYUV420SemiPlanar:
                        if(outPort->sPortParam.format.video.nFrameWidth && outPort->sPortParam.format.video.nFrameHeight) {
                                outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3/2;
                        }
                break;
                default:
                        if(outPort->sPortParam.format.video.nFrameWidth && outPort->sPortParam.format.video.nFrameHeight) {
                                outPort->sPortParam.nBufferSize = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3;
                        }
                break;
        }

		ret = 1;

		if( bScaleChanged ) {
			(*(omx_private->callbacks->EventHandler))(
								   openmaxStandComp,
								   omx_private->callbackData,
								   OMX_EventPortSettingsChanged,
								   OMX_DirOutput,
								   OMX_IndexConfigCommonScale,
								   NULL);
		}

		if( bCropChanged ) {
			(*(omx_private->callbacks->EventHandler))(
								   openmaxStandComp,
								   omx_private->callbackData,
								   OMX_EventPortSettingsChanged,
								   OMX_DirOutput,
								   OMX_IndexConfigCommonOutputCrop,
								   NULL);
		}

		if( bPortChanged ) {
			(*(omx_private->callbacks->EventHandler))(
								   openmaxStandComp,
								   omx_private->callbackData,
								   OMX_EventPortSettingsChanged,
								   OMX_DirOutput,
								   0,
								   NULL);
		}
	}

	if(ret == 1 || omx_private->gHDMIOutput)
	{
		if(omx_private->gHDMIOutput) {
			vpu_update_sizeinfo(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps, omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT, omx_private->pVideoDecodInstance.pVdec_Instance); //max-clock!!
		}
		else{
			vpu_update_sizeinfo(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps,
								omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate, omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth,
								omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight, omx_private->pVideoDecodInstance.pVdec_Instance);
		}
	}

	return ret;
}


static OMX_BOOL isSWCodec(OMX_S32 format)
{
#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC)
	if(
#ifdef INCLUDE_WMV78_DEC
		format == STD_WMV78
#endif
#ifdef INCLUDE_SORENSON263_DEC
		|| format == STD_SORENSON263
#endif
	){
		return OMX_TRUE;
}
#endif

	return OMX_FALSE;
}

static void
disp_pic_info (int Opcode, void* pParam1, void *pParam2, void *pParam3, omx_videodec_component_PrivateType* omx_private)
{
	int i;
	dec_disp_info_ctrl_t  *pInfoCtrl = (dec_disp_info_ctrl_t*)pParam1;
	dec_disp_info_t 	  *pInfo = (dec_disp_info_t *)pParam2;
	dec_disp_info_input_t *pInfoInput = (dec_disp_info_input_t*)pParam3;

	switch( Opcode )
	{
	case CVDEC_DISP_INFO_INIT:	//init.
			pInfoCtrl->m_iStdType = pInfoInput->m_iStdType;
			pInfoCtrl->m_iFmtType = pInfoInput->m_iFmtType;
			pInfoCtrl->m_iTimeStampType = pInfoInput->m_iTimeStampType;

			#ifdef TIMESTAMP_CORRECTION
			if( pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MPG
			|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_TS
			|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MKV )
			{
				omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS = 0;
				omx_private->pVideoDecodInstance.gsPtsInfo.m_iRamainingDuration = 0;

				if( omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate != 0 )
				{
					omx_private->pVideoDecodInstance.gsPtsInfo.m_iPTSInterval = (((1000 * 1000) << 10) / omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate) >> 10;
				}
				else if(omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate != 0)
				{
					omx_private->pVideoDecodInstance.gsPtsInfo.m_iPTSInterval = (((1000 * 1000) << 10) / omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate) >> 10;
				}
			}
			#endif

	case CVDEC_DISP_INFO_RESET: //reset
			for( i=0 ; i<32 ; i++ )
			{
				pInfoCtrl->m_iRegIdxPTS[i] = -1;	//unused
				pInfoCtrl->m_pRegInfoPTS[i] = (void*)&pInfo[i];
			}
			pInfoCtrl->m_iUsedIdxPTS = 0;
			pInfoCtrl->m_iPrevIdx = -1;

			if( pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE )	//Decode Timestamp (Decode order)
			{
				pInfoCtrl->m_iDecodeIdxDTS = 0;
				pInfoCtrl->m_iDispIdxDTS = 0;
				for( i=0 ; i<32 ; i++ )
				{
					pInfoCtrl->m_iDTS[i] = 0;
				}
			}

			memset(&omx_private->pVideoDecodInstance.gsEXT_F_frame_time, 0, sizeof(EXT_F_frame_time_t));
			omx_private->pVideoDecodInstance.gsextReference_Flag = 1;
			omx_private->pVideoDecodInstance.gsextP_frame_cnt = 0;

			#ifdef TIMESTAMP_CORRECTION
			if( pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MPG
				|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_TS
				|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MKV )
			{
				omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS = 0;
				omx_private->pVideoDecodInstance.gsPtsInfo.m_iRamainingDuration = 0;
			}
			#endif
		break;

	case CVDEC_DISP_INFO_UPDATE: //update
		{
			int iDecodedIdx;
			int usedIdx, startIdx, regIdx;
			dec_disp_info_t * pdec_disp_info;

			iDecodedIdx = pInfoInput->m_iFrameIdx;

			//In case that frame rate is changed...
			#ifdef TIMESTAMP_CORRECTION
			if( pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MPG )
			{
				if(pInfoInput->m_iFrameRate)
				{
					omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate = ((pInfoInput->m_iFrameRate & 0xffff) * 1000) / (((pInfoInput->m_iFrameRate >> 16) + 1)&0xffff);
					if(omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate != 0)
					{
						omx_private->pVideoDecodInstance.gsPtsInfo.m_iPTSInterval = (((1000 * 1000) << 10) / omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate) >> 10;
					}
					else if(omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate != 0)
					{
						omx_private->pVideoDecodInstance.gsPtsInfo.m_iPTSInterval = ((1000 << 10) / omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate) >> 10;
					}

					//LOGD("CVDEC_DISP_INFO_UPDATE m_iPTSInterval %d m_iFrameRate %d input FrameRate %x ",omx_private->gsMPEG2PtsInfo.m_iPTSInterval , omx_private->cdmx_info.m_sVideoInfo.m_iFrameRate,pInfoInput->m_iFrameRate);
				}
			}
			#endif
			//Presentation Timestamp (Display order)
			{
				//sort
				usedIdx=0;
				startIdx = -1;
				for( i=0 ; i<32 ; i++ )
				{
					if( pInfoCtrl->m_iRegIdxPTS[i] > -1 )
					{
						if( startIdx == -1 )
						{
							startIdx = i;
						}
						usedIdx++;
					}
				}

				if( usedIdx > 0 )
				{
					regIdx = 0;
					for( i=startIdx ; i<32 ; i++ )
					{
						if( pInfoCtrl->m_iRegIdxPTS[i] > -1 )
						{
							if( i != regIdx )
							{
								void * pswap;
								int iswap;

								iswap = pInfoCtrl->m_iRegIdxPTS[regIdx];
								pswap = pInfoCtrl->m_pRegInfoPTS[regIdx];

								pInfoCtrl->m_iRegIdxPTS[regIdx] = pInfoCtrl->m_iRegIdxPTS[i];
								pInfoCtrl->m_pRegInfoPTS[regIdx] = pInfoCtrl->m_pRegInfoPTS[i];

								pInfoCtrl->m_iRegIdxPTS[i] = iswap;
								pInfoCtrl->m_pRegInfoPTS[i] = pswap;
							}
							regIdx++;
							if( regIdx == usedIdx )
								break;
						}
					}
				}

				//save the side info.
				pInfoCtrl->m_iRegIdxPTS[usedIdx] = iDecodedIdx;
				pdec_disp_info = (dec_disp_info_t*)pInfoCtrl->m_pRegInfoPTS[usedIdx];

				if(pInfoCtrl->m_iPrevIdx != -1)
				{
					dec_disp_info_t * pdec_disp_info_prev = (dec_disp_info_t*)pInfoCtrl->m_pRegInfoPTS[pInfoCtrl->m_iPrevIdx];
					if(pInfo->m_iTimeStamp == pdec_disp_info_prev->m_iTimeStamp)
						pdec_disp_info->m_iTimeStamp = -1;
					else
						pdec_disp_info->m_iTimeStamp = pInfo->m_iTimeStamp;
				}
				else
				{
					pdec_disp_info->m_iTimeStamp = pInfo->m_iTimeStamp;
				}
				pInfoCtrl->m_iPrevIdx = usedIdx;

				pdec_disp_info->m_iFrameType = pInfo->m_iFrameType;
				pdec_disp_info->m_iPicStructure = pInfo->m_iPicStructure;
				pdec_disp_info->m_iextTimeStamp = pInfo->m_iextTimeStamp;
				pdec_disp_info->m_iM2vFieldSequence = pInfo->m_iM2vFieldSequence;
				pdec_disp_info->m_iFrameDuration = pInfo->m_iFrameDuration;
				pdec_disp_info->m_iFrameSize = pInfo->m_iFrameSize;
				pdec_disp_info->m_bIsMvcDependent = pInfo->m_bIsMvcDependent;
				pdec_disp_info->m_iNumMBError = pInfo->m_iNumMBError;

				if( pInfoCtrl->m_iStdType  == STD_RV )
				{
					//int curTimestamp, ext_Timestamp, ext_FrameType;
					OMX_TICKS curTimestamp, ext_Timestamp, ext_FrameType;

					curTimestamp = pInfo->m_iTimeStamp;
					ext_Timestamp = pInfo->m_iextTimeStamp;
					ext_FrameType = pInfo->m_iFrameType;

					if(omx_private->pVideoDecodInstance.gsextReference_Flag)
					{
						omx_private->pVideoDecodInstance.gsextReference_Flag = 0;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_time_stamp = curTimestamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Previous_TR = ext_Timestamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P2.Current_time_stamp = curTimestamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P2.Current_TR = ext_Timestamp;
					}
					else
					{
						omx_private->pVideoDecodInstance.gsextTRDelta = ext_Timestamp - omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_TR;
						if(omx_private->pVideoDecodInstance.gsextTRDelta < 0)
						{
							omx_private->pVideoDecodInstance.gsextTRDelta += 8192;
						}

						if(ext_FrameType == 2) //B-frame
						{
							curTimestamp = omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_time_stamp + omx_private->pVideoDecodInstance.gsextTRDelta;
						}
						else
						{
							omx_private->pVideoDecodInstance.gsextP_frame_cnt++;
						}
					}

					if( omx_private->pVideoDecodInstance.gsextP_frame_cnt == 1)
					{
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P1.Current_TR = ext_Timestamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P1.Current_time_stamp = curTimestamp;

						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_time_stamp = omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P2.Current_time_stamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_TR = omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P2.Current_TR;
					}
					else if( omx_private->pVideoDecodInstance.gsextP_frame_cnt == 2)
					{
						omx_private->pVideoDecodInstance.gsextP_frame_cnt = 0;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P2.Current_TR = ext_Timestamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P2.Current_time_stamp = curTimestamp;

						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_time_stamp = omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P1.Current_time_stamp;
						omx_private->pVideoDecodInstance.gsEXT_F_frame_time.ref_frame.Current_TR = omx_private->pVideoDecodInstance.gsEXT_F_frame_time.frame_P1.Current_TR;
					}

					pdec_disp_info->m_iextTimeStamp = curTimestamp;
				}

				pInfoCtrl->m_iUsedIdxPTS = usedIdx + 1;
				if( pInfoCtrl->m_iUsedIdxPTS > 31 )
				{
					DBUG_MSG( "[CDK_CORE] disp_pic_info index failed\n" );
					for( i=0 ; i<32 ; i++ )
					{
						pInfoCtrl->m_iRegIdxPTS[i] = -1;
					}
				}
			}

			if( pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE )	//Decode Timestamp (Decode order)
			{
				if( iDecodedIdx >= 0 || ( iDecodedIdx == -2 && pInfoCtrl->m_iStdType  == STD_MPEG4  ) )
				{
					pInfoCtrl->m_iDTS[pInfoCtrl->m_iDecodeIdxDTS] = pInfo->m_iTimeStamp;
					pInfoCtrl->m_iDecodeIdxDTS = ( pInfoCtrl->m_iDecodeIdxDTS + 1 ) & 31;
				}
			}
		}
		break;
	case CVDEC_DISP_INFO_GET:	//display
		{
			dec_disp_info_t **pInfo = (dec_disp_info_t **)pParam2;
			int dispOutIdx = pInfoInput->m_iFrameIdx;

			//Presentation Timestamp (Display order)
			{
				*pInfo = 0;

				for( i=0; i<pInfoCtrl->m_iUsedIdxPTS ; i++ )
				{
					if( dispOutIdx == pInfoCtrl->m_iRegIdxPTS[i] )
					{
						*pInfo = (dec_disp_info_t*)pInfoCtrl->m_pRegInfoPTS[i];

						#ifdef TIMESTAMP_CORRECTION
						if( (pInfoCtrl->m_iFmtType  == CONTAINER_TYPE_MPG
							|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_TS
							|| pInfoCtrl->m_iFmtType == CONTAINER_TYPE_MKV)
							&& (omx_private->bPlayDirection))
						{
							static long prev_pts;
							if( (*pInfo)->m_bIsMvcDependent ) {
								(*pInfo)->m_iTimeStamp = omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS;
							} 
							else {
								if( (*pInfo)->m_iTimeStamp == -1 )
									(*pInfo)->m_iTimeStamp = omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS + ((omx_private->pVideoDecodInstance.gsPtsInfo.m_iPTSInterval * omx_private->pVideoDecodInstance.gsPtsInfo.m_iRamainingDuration) >> 1);
							}
							prev_pts = (*pInfo)->m_iTimeStamp;
							omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS = (*pInfo)->m_iTimeStamp;
							omx_private->pVideoDecodInstance.gsPtsInfo.m_iRamainingDuration = (*pInfo)->m_iFrameDuration;
						}
						#endif

						pInfoCtrl->m_iRegIdxPTS[i] = -1; //unused
						pInfoCtrl->m_iUsedIdxPTS--;
						break;
					}
				}

				if( *pInfo ) {
					void *p_temp;
					for(; i < 31; i++) 
					{
						if( pInfoCtrl->m_iRegIdxPTS[i+1] <= -1 )
							break; 

						pInfoCtrl->m_iRegIdxPTS[i ] = pInfoCtrl->m_iRegIdxPTS[i+1];
						pInfoCtrl->m_iRegIdxPTS[i+1] = -1;

						p_temp = pInfoCtrl->m_pRegInfoPTS[i+1];
						pInfoCtrl->m_pRegInfoPTS[i+1] = pInfoCtrl->m_pRegInfoPTS[i];
						pInfoCtrl->m_pRegInfoPTS[i] = p_temp;
					}
				}
			}

			if( pInfoCtrl->m_iTimeStampType == CDMX_DTS_MODE )	//Decode Timestamp (Decode order)
			{
				if( *pInfo != 0 )
				{
					(*pInfo)->m_iTimeStamp =
					(*pInfo)->m_iextTimeStamp = pInfoCtrl->m_iDTS[pInfoCtrl->m_iDispIdxDTS];
					pInfoCtrl->m_iDispIdxDTS = ( pInfoCtrl->m_iDispIdxDTS + 1 ) & 31;
				}
			}
		}
		break;
	}

	return;
}

static int
get_frame_type_for_frame_skipping(int iStdType, int iPicType, int iPicStructure)
{
	int frameType = 0; //unknown

	switch ( iStdType )
	{
	case STD_VC1 :
		switch( (iPicType>>3) ) //Frame or // FIELD_INTERLACED(TOP FIELD)
		{
		case PIC_TYPE_I:	frameType = 1; break;//I
		case PIC_TYPE_P:	frameType = 2; break;//P
		case 2:				frameType = 3; break;//B //DSTATUS( "BI  :" );
		case 3:				frameType = 3; break;//B //DSTATUS( "B   :" );
		case 4:				frameType = 3; break;//B //DSTATUS( "SKIP:" );
		}
		if( iPicStructure == 3)
		{
			switch( (iPicType&0x7) ) // FIELD_INTERLACED(BOTTOM FIELD)
			{
			case PIC_TYPE_I:	frameType = 1; break;//I
			case PIC_TYPE_P:	frameType = 2; break;//P
			case 2:				frameType = 3; break;//B //DSTATUS( "BI  :" );
			case 3:				frameType = 3; break;//B //DSTATUS( "B   :" );
			case 4:				frameType = 3; break;//B //DSTATUS( "SKIP:" );
			}
		}
		break;
	case STD_MPEG4 :
		switch( iPicType )
		{
		case PIC_TYPE_I:	frameType = 1;	break;//I
		case PIC_TYPE_P:	frameType = 2;	break;//P
		case PIC_TYPE_B:	frameType = 3;	break;//B
		case PIC_TYPE_B_PB: frameType = 4;	break;//B of Packed PB-frame
		}
		break;
	case STD_MPEG2 :
	default:
		switch( iPicType & 0xF )
		{
		case PIC_TYPE_I:	frameType = 1;	break;//I
		case PIC_TYPE_P:	frameType = 2;	break;//P
		case PIC_TYPE_B:	frameType = 3;	break;//B
		}
	}
	return frameType;
}

void print_user_data(unsigned char * pUserData)
{
	unsigned int i, j;
	unsigned char * pTmpPTR;
	unsigned char * pRealData;
	unsigned int nNumUserData;
	unsigned int nTotalSize;
	unsigned int nDataSize;

	pTmpPTR = pUserData;
	nNumUserData = (pTmpPTR[0] << 8) | pTmpPTR[1];
	nTotalSize = (pTmpPTR[2] << 8) | pTmpPTR[3];

	pTmpPTR = pUserData + 8;
	pRealData = pUserData + (8 * 17);

	DBUG_MSG( "\n***User Data Print***\n");
	for(i = 0;i < nNumUserData;i++)
	{
		nDataSize = (pTmpPTR[2] << 8) | pTmpPTR[3];
		DBUG_MSG( "[User Data][Idx : %02d][Size : %05d]", i, nDataSize);
		for(j = 0;j < nDataSize;j++)
		{
			DBUG_MSG( "%02x ", pRealData[j]);
		}
		pTmpPTR += 8;
		pRealData += nDataSize;
	}
}


char*
print_pic_type( int iVideoType, int iPicType, int iPictureStructure )
{
	switch ( iVideoType )
	{
	case STD_MPEG2 :
		if( iPicType == PIC_TYPE_I )
			return "I :";
		else if( iPicType == PIC_TYPE_P )
			return "P :";
		else if( iPicType == PIC_TYPE_B )
			return "B :";
		else
			return "D :"; //D_TYPE
		break;

	case STD_MPEG4 :
		if( iPicType == PIC_TYPE_I )
			return "I :";
		else if( iPicType == PIC_TYPE_P )
			return "P :";
		else if( iPicType == PIC_TYPE_B )
			return "B :";
		else if( iPicType == PIC_TYPE_B_PB ) //MPEG-4 Packed PB-frame
			return "pB:";
		else
			return "S :"; //S_TYPE
		break;

	case STD_VC1 :
		if( iPictureStructure == 3)
		{
			// FIELD_INTERLACED
			if( (iPicType>>3) == PIC_TYPE_I )
				return "TF_I   :";	//TOP_FIELD = I
			else if( (iPicType>>3) == PIC_TYPE_P )
				return "TF_P   :";	//TOP_FIELD = P
			else if( (iPicType>>3) == 2 )
				return "TF_BI  :";	//TOP_FIELD = BI_TYPE
			else if( (iPicType>>3) == 3 )
				return "TF_B   :";	//TOP_FIELD = B_TYPE
			else if( (iPicType>>3) == 4 )
				return "TF_SKIP:";	//TOP_FIELD = SKIP_TYPE
			else
				return "TF_FORBIDDEN :"; //TOP_FIELD = FORBIDDEN

			if( (iPicType&0x7) == PIC_TYPE_I )
				return "BF_I   :";	//BOTTOM_FIELD = I
			else if( (iPicType&0x7) == PIC_TYPE_P )
				return "BF_P   :";	//BOTTOM_FIELD = P
			else if( (iPicType&0x7) == 2 )
				return "BF_BI  :";	//BOTTOM_FIELD = BI_TYPE
			else if( (iPicType&0x7) == 3 )
				return "BF_B   :";	//BOTTOM_FIELD = B_TYPE
			else if( (iPicType&0x7) == 4 )
				return "BF_SKIP:";	//BOTTOM_FIELD = SKIP_TYPE
			else
				return "BF_FORBIDDEN :"; //BOTTOM_FIELD = FORBIDDEN
		}
		else
		{
			iPicType = iPicType>>3;
			if( iPicType == PIC_TYPE_I )
				return "I   :";
			else if( iPicType == PIC_TYPE_P )
				return "P   :";
			else if( iPicType == 2 )
				return "BI  :";
			else if( iPicType == 3 )
				return "B   :";
			else if( iPicType == 4 )
				return "SKIP:";
			else
				return "FORBIDDEN :"; //FORBIDDEN
		}
		break;
	default:
		if( iPicType == PIC_TYPE_I )
			return "I :";
		else if( iPicType == PIC_TYPE_P )
			return "P :";
		else if( iPicType == PIC_TYPE_B )
			return "B :";
		else
			return "U :"; //Unknown
	}
}


OMX_ERRORTYPE omx_videodec_component_AllocateBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE eError;
	OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *port;

	if( omx_private->bSecuredInBufferMode == OMX_TRUE && nPortIndex == OMX_DirInput )
	{
		OMX_U32 nBufferLen;

		port = (omx_base_video_PortType *)omx_private->ports[nPortIndex];
		nBufferLen = omx_private->mUmpSecuredInPmap.size / port->sPortParam.nBufferCountActual;

		if( nBufferLen != port->sPortParam.nBufferSize ){
			ALOGE("Buffer length(0x%x) is different from requested one(0x%x).", (unsigned int)nBufferLen, (unsigned int)port->sPortParam.nBufferSize);
		}

		if (omx_private->bTEEEnable) {
	    	eError = omx_base_component_Allocate_SecuredInBuffer(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes, (OMX_U32)omx_private->mUmpSecuredInPmap.base, nBufferLen);
		} else {
	    	eError = omx_base_component_Allocate_SecuredInBuffer(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes, (OMX_U32)omx_private->mSecureInMapInfo, nBufferLen);
		}
		ALOGI(" Allocate_SecuredInBuffer 0x%x :: 0x%x - 0x%x", (unsigned int)omx_private->mSecureInMapInfo, (unsigned int)(*pBuffer)->pBuffer, (unsigned int)nBufferLen);
	}
	else
	{
	    eError = omx_base_component_AllocateBuffer(hComponent, pBuffer, nPortIndex, pAppPrivate, nSizeBytes);
	}

	if( nPortIndex == OMX_DirOutput && omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr)
	{
		(*pBuffer)->pOutputPortPrivate = TCC_calloc(1, sizeof(TCC_PLATFORM_PRIVATE_PMEM_INFO));
		DBUG_MSG("Allocate PlatformPrivate 0x%x - 0x%x", (unsigned int)(*pBuffer)->pOutputPortPrivate, sizeof(TCC_PLATFORM_PRIVATE_PMEM_INFO));
	}

    return eError;
}

OMX_ERRORTYPE omx_videodec_component_UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OMX_ERRORTYPE eError;
	OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	omx_base_video_PortType *port;

    eError = omx_base_component_UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);

	if( nPortIndex == OMX_DirOutput && omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr)
	{
		(*ppBufferHdr)->pOutputPortPrivate = TCC_calloc(1, sizeof(TCC_PLATFORM_PRIVATE_PMEM_INFO));
		DBUG_MSG("Use OutputPortPrivate 0x%x - 0x%x", (unsigned int)(*ppBufferHdr)->pOutputPortPrivate, sizeof(TCC_PLATFORM_PRIVATE_PMEM_INFO));
	}

    return eError;
}

OMX_ERRORTYPE omx_videodec_component_FreeBuffer(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_IN  OMX_U32 nPortIndex,
    OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
	OMX_ERRORTYPE eError;
	OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

	if (pBuffer->pOutputPortPrivate && nPortIndex == OMX_DirOutput && omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr)
	{
        TCC_free(pBuffer->pOutputPortPrivate);
		DBUG_MSG("Free PlatformPrivate 0x%x", (unsigned int)pBuffer->pOutputPortPrivate);
	}

	if( omx_private->bSecuredInBufferMode == OMX_TRUE && nPortIndex == OMX_DirInput )
	{
	    eError = omx_base_component_Free_SecuredInBuffer(hComponent, nPortIndex, pBuffer);
		ALOGI(" Free_SecuredInBuffer %ld", nPortIndex);
	}
	else
	{
	    eError = omx_base_component_FreeBuffer(hComponent, nPortIndex, pBuffer);
	}

	return eError;
}

///////////////////////////////////////////////////////////////////////////////////
#define CODETYPE_NONE		(0x00000000)
#define CODETYPE_HEADER		(0x00000001)
#define CODETYPE_PICTURE	(0x00000002)
#define CODETYPE_ALL		(CODETYPE_HEADER | CODETYPE_PICTURE)

/* MPEG2 start code */
#define MPEG2_PICTURE_START     0x00000100
#define MPEG2_SEQUENCE_HEADER   0x000001B3
#define MPEG2_SEQUENCE_END      0x000001B7
#define MPEG2_GROUP_OF_PICTURES 0x000001B8

/* WVC1 start code */
#define WVC1_VIDEO_HEADER       0x0000010F
#define WVC1_VIDEO_FRAME        0x0000010D

/* MPEG4 start code */
#define MPEG4_VIDEO_HEADER      0x00000100
#define MPEG4_VIDEO_FRAME       0x000001B6

/* AVS start code */
#define AVS_VIDEO_HEADER        0x000001B0
#define AVS_VIDEO_I_FRAME       0x000001B3
#define AVS_VIDEO_PB_FRAME      0x000001B6

/* H.264/AVC NAL unit type codes for start code and type mask */
#define AVC_NAL_FORBIDDEN_ZERO_BIT_MASK      0x80
#define AVC_NAL_STARTCODE_WITH_TYPE_MASK     0xFFFFFF1F
#define AVC_NAL_STARTCODE_WITH_TYPE_SLICE    0x00000101  // P-frame
#define AVC_NAL_STARTCODE_WITH_TYPE_DPA      0x00000102
#define AVC_NAL_STARTCODE_WITH_TYPE_DPB      0x00000103
#define AVC_NAL_STARTCODE_WITH_TYPE_DPC      0x00000104
#define AVC_NAL_STARTCODE_WITH_TYPE_IDR      0x00000105  // I-frame
#define AVC_NAL_STARTCODE_WITH_TYPE_SEI      0x00000106
#define AVC_NAL_STARTCODE_WITH_TYPE_SPS      0x00000107
#define AVC_NAL_STARTCODE_WITH_TYPE_PPS      0x00000108
#define AVC_NAL_STARTCODE_WITH_TYPE_AUD      0x00000109
#define AVC_NAL_STARTCODE_WITH_TYPE_EOSEQ    0x0000010A
#define AVC_NAL_STARTCODE_WITH_TYPE_EOSTREAM 0x0000010B
#define AVC_NAL_STARTCODE_WITH_TYPE_FILL     0x0000010C

OMX_U32 SearchCodeType_Common(OMX_INOUT void* omx_private_type, OMX_OUT OMX_U32 *input_offset, OMX_IN OMX_U32 search_option)
{
	omx_videodec_component_PrivateType* omx_private = (omx_videodec_component_PrivateType*)omx_private_type;
	OMX_U32 temp_input_offset = *input_offset;
	OMX_U32 code_type = CODETYPE_NONE;

	if(omx_private->bSecuredInBufferMode == OMX_TRUE){
		*input_offset = 0;
		return OMX_TRUE;
	}

    omx_private->start_code_with_type = 0xFFFFFFFF;

    for (; temp_input_offset < omx_private->inputCurrLength; temp_input_offset++)
    {
        omx_private->start_code_with_type = (omx_private->start_code_with_type << 8) | omx_private->inputCurrBuffer[temp_input_offset];

        if ((search_option & CODETYPE_HEADER) && (omx_private->start_code_with_type == omx_private->start_code_header))
        {
            code_type = CODETYPE_HEADER;
            temp_input_offset -= MIN_NAL_STARTCODE_LEN;
            break;
        }
        if ((search_option & CODETYPE_PICTURE) &&
            ((omx_private->start_code_with_type == omx_private->start_code_picture) ||
             (omx_private->start_code_picture1 && omx_private->start_code_with_type == omx_private->start_code_picture1)))
        {
            code_type = CODETYPE_PICTURE;
            temp_input_offset -= MIN_NAL_STARTCODE_LEN;
            break;
        }
    }

	*input_offset = temp_input_offset;

	return code_type;
}

OMX_U32 SearchCodeType_AVC(OMX_INOUT void* omx_private_type, OMX_OUT OMX_U32 *input_offset, OMX_IN OMX_U32 search_option)
{
	omx_videodec_component_PrivateType* omx_private = (omx_videodec_component_PrivateType*)omx_private_type;
	OMX_U32 temp_input_offset = *input_offset;
	OMX_U32 code_type = CODETYPE_NONE;
	omx_private->AVC_naltype_mask = 0;

	if(omx_private->bSecuredInBufferMode == OMX_TRUE){
		*input_offset = 0;
		return OMX_TRUE;
	}

    omx_private->start_code_with_type = 0xFFFFFFFF;

    for (; temp_input_offset < omx_private->inputCurrLength; temp_input_offset++)
    {
        omx_private->start_code_with_type = (omx_private->start_code_with_type << 8) | omx_private->inputCurrBuffer[temp_input_offset];
        omx_private->AVC_naltype_mask = omx_private->start_code_with_type & AVC_NAL_STARTCODE_WITH_TYPE_MASK;

        if (omx_private->AVC_naltype_mask == AVC_NAL_STARTCODE_WITH_TYPE_AUD)
        {
            // Is it first time to detect frame delimiter ?
            if(omx_private->bDetectFrameDelimiter == OMX_FALSE)
            {
                omx_private->bDetectFrameDelimiter = OMX_TRUE;
            }

            omx_private->frame_delimiter_offset = temp_input_offset;
        }
        else if (omx_private->AVC_naltype_mask == AVC_NAL_STARTCODE_WITH_TYPE_SEI)
        {
            omx_private->appendable_header_offset = temp_input_offset;
        }

        if ((omx_private->bDetectFrameDelimiter == OMX_TRUE) && (omx_private->frame_delimiter_offset == OMX_BUFF_OFFSET_UNASSIGNED))
        {
            // Keep searching until AUD is detected.
            continue;
        }

        if ((search_option & CODETYPE_HEADER) && (omx_private->AVC_naltype_mask == AVC_NAL_STARTCODE_WITH_TYPE_SPS))
        {
            if (omx_private->frame_delimiter_offset != OMX_BUFF_OFFSET_UNASSIGNED)
            {
                temp_input_offset = omx_private->frame_delimiter_offset;
                omx_private->frame_delimiter_offset = OMX_BUFF_OFFSET_UNASSIGNED;
            }
            else if (omx_private->appendable_header_offset != OMX_BUFF_OFFSET_UNASSIGNED)
            {
                if(omx_private->appendable_header_offset < temp_input_offset)
                {
                    // assign the target as min. buffer offset
                    temp_input_offset = omx_private->appendable_header_offset;
                    omx_private->appendable_header_offset = OMX_BUFF_OFFSET_UNASSIGNED;
                }
            }
            code_type = CODETYPE_HEADER;
            temp_input_offset -= MIN_NAL_STARTCODE_LEN;
            break;
        }

        if ((search_option & CODETYPE_PICTURE) &&
            ((omx_private->AVC_naltype_mask == AVC_NAL_STARTCODE_WITH_TYPE_SLICE) || (omx_private->AVC_naltype_mask == AVC_NAL_STARTCODE_WITH_TYPE_IDR)))
        {
            if (omx_private->frame_delimiter_offset != OMX_BUFF_OFFSET_UNASSIGNED)
            {
                temp_input_offset = omx_private->frame_delimiter_offset;
                omx_private->frame_delimiter_offset = OMX_BUFF_OFFSET_UNASSIGNED;
            }
            else if (omx_private->appendable_header_offset != OMX_BUFF_OFFSET_UNASSIGNED)
            {
                if(omx_private->appendable_header_offset < temp_input_offset)
                {
                    // assign the target as min. buffer offset
                    temp_input_offset = omx_private->appendable_header_offset;
                    omx_private->appendable_header_offset = OMX_BUFF_OFFSET_UNASSIGNED;
                }
            }
            code_type = CODETYPE_PICTURE;
            temp_input_offset -= MIN_NAL_STARTCODE_LEN;

        #ifdef ENABLE_DECODE_ONLY_MODE_AVC
            if(omx_private->AVC_naltype_mask == AVC_NAL_STARTCODE_WITH_TYPE_IDR)
            {
                if(omx_private->I_frame_search_mode == AVC_NONIDR_PICTURE_SEARCH_MODE &&
                   omx_private->bSetDecodeOnlyMode == OMX_TRUE)
                {
                    LOGMSG("[Seek] Change I-frame search mode : non-IDR -> IDR");
                    omx_private->ConsecutiveVdecFailCnt = 0;
                    omx_private->frameSearchOrSkip_flag = 1;

                    omx_private->I_frame_search_mode = AVC_IDR_PICTURE_SEARCH_MODE;
                    omx_private->IDR_frame_search_count = 0;
                    omx_private->skipFrameNum = 0;
                }
            }
            else
            {
                if((omx_private->frameSearchOrSkip_flag == 1) &&
                   (omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable == AVC_IDR_PICTURE_SEARCH_MODE))
                {
                    if(++omx_private->IDR_frame_search_count >= AVC_IDR_PICTURE_SEARCH_COUNT)
                    {
                        LOGMSG("[Seek] Change I-frame search mode : IDR -> non-IDR");
                        omx_private->I_frame_search_mode = AVC_NONIDR_PICTURE_SEARCH_MODE;
                        omx_private->skipFrameNum = 0;
                    }
                }
            }
        #endif
            break;
        }
    }

	*input_offset = temp_input_offset;

	return code_type;
}
///////////////////////////////////////////////////////////////////////////////////

OMX_BOOL ExtractConfigData(omx_videodec_component_PrivateType* omx_private, OMX_U32 input_offset)
{
	OMX_U8* p = omx_private->inputCurrBuffer;
	unsigned int szInfo_video = sizeof(omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo);
    omx_base_video_PortType *outPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

	if(omx_private->bSecuredInBufferMode == OMX_TRUE)
		return OMX_TRUE;

	if(omx_private->inputCurrLength >= sizeof(TCCVideoConfigData))
	{
		TCCVideoConfigData *config_data = (TCCVideoConfigData*)(p+omx_private->inputCurrLength-sizeof(TCCVideoConfigData));

		if (strncmp(config_data->id, TCC_VIDEO_CONFIG_ID, 9) == 0)
		{
			if(omx_private->extractorType == 0)
			{
				omx_private->extractorType = OMX_BUFFERFLAG_EXTRACTORTYPE_TCC;
			}
			omx_private->pVideoDecodInstance.container_type = config_data->iContainerType;
			omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps = config_data->iBitRate;
			omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate = config_data->iFrameRate;

			memset(&(omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo), 0x00, szInfo_video);
			//sync with parser!!
			if (omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_RV || omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_DIV3 ||
					omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MPEG2 || omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_VC1 ||
					omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_AVC || omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MPEG4
#ifdef INCLUDE_WMV78_DEC
					|| omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_WMV78
#endif
#ifdef INCLUDE_SORENSON263_DEC
					|| omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_SORENSON263
#endif
			   )
			{
				memcpy(&(omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo), (char*)(p+omx_private->inputCurrLength-sizeof(TCCVideoConfigData)-szInfo_video), szInfo_video);
#ifdef INCLUDE_WMV78_DEC
				omx_private->pVideoDecodInstance.gsVDecInit.m_iFourCC = omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFourCC;
#endif
			}

			if(omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iExtraDataLen
					&& omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV_1_2)
			{
				DBUG_MSG("ExtraData = %d", omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iExtraDataLen);
				omx_private->extradata_size = omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iExtraDataLen;
				omx_private->extradata = TCC_calloc(1, omx_private->extradata_size);
				memcpy(omx_private->extradata, omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_pExtraData, omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iExtraDataLen);
			}
			else
			{
				omx_private->extradata_size = 0;
				omx_private->extradata = NULL;
			}

			LOGI("Resolution = %ld x %ld - %d Mbps - %d fps, Container Type = %d, FourCC = 0x%08x[%c%c%c%c]", outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
					omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps, omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate/1000, omx_private->pVideoDecodInstance.container_type, omx_private->pVideoDecodInstance.gsVDecInit.m_iFourCC,
					(char)(omx_private->pVideoDecodInstance.gsVDecInit.m_iFourCC>>0), (char)(omx_private->pVideoDecodInstance.gsVDecInit.m_iFourCC>>8), (char)(omx_private->pVideoDecodInstance.gsVDecInit.m_iFourCC>>16),(char)(omx_private->pVideoDecodInstance.gsVDecInit.m_iFourCC>>24));
		}
		else
		{
			omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps = 0;
			omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate = 0;
		}
	}
	else
	{
		if(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
		{
			LOGE("ExtractConfigData : Error - TCCVideoConfigData is truncated");
			return OMX_FALSE;
		}

		omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps = 0;
		omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate = 0;
	}

	if(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
	{
		if ((CONTAINER_TYPE_TS == omx_private->pVideoDecodInstance.container_type)
		/*|| (CONTAINER_TYPE_MPG == omx_private->pVideoDecodInstance.container_type)*/)
		{
		#if defined (ENABLE_DECODE_ONLY_MODE_AVC)
			char value[PROPERTY_VALUE_MAX];
			memset(value, 0, PROPERTY_VALUE_MAX);
		#endif

			if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG2)
			{
				omx_private->SearchCodeType = SearchCodeType_Common;
				omx_private->start_code_header = MPEG2_SEQUENCE_HEADER;
				omx_private->start_code_picture = MPEG2_PICTURE_START;
			}
			else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC || omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC)
			{
				omx_private->SearchCodeType = SearchCodeType_AVC;

			#ifdef ENABLE_DECODE_ONLY_MODE_AVC
				if(CONTAINER_TYPE_TS == omx_private->pVideoDecodInstance.container_type)
				{
					memset(value, 0, PROPERTY_VALUE_MAX);
					property_get("tcc.avc.seektype", value, "0");

					OMX_U32 seektype = (OMX_U32)atoi(value);
					if(seektype)
					{
						omx_private->I_frame_search_mode = AVC_IDR_PICTURE_SEARCH_MODE;
						omx_private->maxConsecutiveVdecFailCnt = MAX_CONSECUTIVE_VPU_FAIL_COUNT_FOR_IDR;
					}
					else
					{
						memset(value, 0, PROPERTY_VALUE_MAX);
						property_get("tcc.video.skip.disp", value, "1");
						omx_private->bUseDecodeOnlyMode = (OMX_U32)atoi(value) == 0 ? OMX_FALSE : OMX_TRUE;

						if(omx_private->bUseDecodeOnlyMode)
						{
							memset(value, 0, PROPERTY_VALUE_MAX);
							property_get("tcc.video.skip.disp.num", value, "0");
							OMX_U32 numSkipFrame = (OMX_U32)atoi(value);
							if(numSkipFrame > omx_private->max_fifo_cnt)
							{
								omx_private->numSkipFrame = numSkipFrame;
							}
						}
					}
				}
			#endif
			}
			else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV)
			{
				omx_private->SearchCodeType = SearchCodeType_Common;
				omx_private->start_code_header = WVC1_VIDEO_HEADER;
				omx_private->start_code_picture = WVC1_VIDEO_FRAME;
			}
			else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4 || omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingDIVX)
			{
				omx_private->SearchCodeType = SearchCodeType_Common;
				omx_private->start_code_header = MPEG4_VIDEO_HEADER;
				omx_private->start_code_picture = MPEG4_VIDEO_FRAME;
			}
			else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVS)
			{
				omx_private->SearchCodeType = SearchCodeType_Common;
				omx_private->start_code_header = AVS_VIDEO_HEADER;
				omx_private->start_code_picture = AVS_VIDEO_I_FRAME;
				omx_private->start_code_picture1 = AVS_VIDEO_PB_FRAME;
			}
		}
	}
	else
	{
		if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC || omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC)
		{
			const OMX_U8 NAL_UNIT_SPS = 0x07;
			const OMX_U8 NAL_UNIT_PPS = 0x08;

			OMX_U8* p = omx_private->inputCurrBuffer;
			OMX_U8 cNalUnitType = p[MIN_NAL_STARTCODE_LEN + 1] & 0x1F;

			if(cNalUnitType == NAL_UNIT_SPS || cNalUnitType == NAL_UNIT_PPS)
			{
				OMX_U32 uiConfigDataSize = omx_private->inputCurrLength - input_offset;
				memcpy(&omx_private->pConfigdata[omx_private->szConfigdata], p + input_offset, uiConfigDataSize);
				omx_private->szConfigdata += uiConfigDataSize;
			}

		#ifdef ENABLE_DECODE_ONLY_MODE_AVC
			if(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_MPEG2TS
				|| omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_RTSP)
			{
				char value[PROPERTY_VALUE_MAX];
				memset(value, 0, PROPERTY_VALUE_MAX);
				property_get("tcc.avc.seektype", value, "0");

				OMX_U32 seektype = (OMX_U32)atoi(value);
				if(seektype)
				{
					omx_private->I_frame_search_mode = AVC_IDR_PICTURE_SEARCH_MODE;
					omx_private->maxConsecutiveVdecFailCnt = MAX_CONSECUTIVE_VPU_FAIL_COUNT_FOR_IDR;
				}
				else
				{
					memset(value, 0, PROPERTY_VALUE_MAX);
					property_get("tcc.video.skip.disp", value, "1");
					omx_private->bUseDecodeOnlyMode = (OMX_U32)atoi(value) == 0 ? OMX_FALSE : OMX_TRUE;

					if(omx_private->bUseDecodeOnlyMode)
					{
						memset(value, 0, PROPERTY_VALUE_MAX);
						property_get("tcc.video.skip.disp.num", value, "0");
						OMX_U32 numSkipFrame = (OMX_U32)atoi(value);
						if(numSkipFrame > omx_private->max_fifo_cnt)
						{
							omx_private->numSkipFrame = numSkipFrame;
						}
					}
				}
			}
		#endif
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4 ||
				omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG2 )
		{
			OMX_U32 uiConfigDataSize = omx_private->inputCurrLength - input_offset;
			memcpy(&omx_private->pConfigdata[omx_private->szConfigdata], omx_private->inputCurrBuffer+input_offset, uiConfigDataSize);
			omx_private->szConfigdata += uiConfigDataSize;
		}
	}

	return OMX_TRUE;
}

static void VideoDecErrorProcess(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer, int ret)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

    if(omx_private->cntDecError > MAX_CONSECUTIVE_VPU_FAIL_TO_RESTORE_COUNT)
    {
		LOGE("[VDEC-%d] Consecutive decode-cmd failure is occurred", omx_private->pVideoDecodInstance.nVdec_Instance);
    }

	if(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_RTSP) {
		omx_private->cntDecError = 0;
	}

#ifdef RESTORE_DECODE_ERR
	if((ret == -RETCODE_CODEC_EXIT || ret == -RETCODE_MULTI_CODEC_EXIT_TIMEOUT) && omx_private->cntDecError <= MAX_CONSECUTIVE_VPU_FAIL_TO_RESTORE_COUNT)
	{
		if(omx_private->pVideoDecodInstance.avcodecInited && omx_private->seqHeader_len == 0)
			goto Error_Proc;

		if(!omx_private->pVideoDecodInstance.avcodecInited && omx_private->sequence_header_only != NULL)
			omx_private->need_sequence_header_attachment = OMX_TRUE;

		omx_private->isNewBuffer = 1;
		omx_private->cntDecError++;
		omx_private->pVideoDecodInstance.avcodecInited = 0;
		omx_private->ConsecutiveBufferFullCnt = 0;

		init_stBuffer_Management(omx_private);

		omx_private->seq_header_init_error_count = SEQ_HEADER_INIT_ERROR_COUNT;
		if(omx_private->pVideoDecodInstance.isVPUClosed != OMX_TRUE)
		{
			omx_private->pVideoDecodInstance.gspfVDec( VDEC_CLOSE, NULL, NULL, &(omx_private->pVideoDecodInstance.gsVDecOutput), omx_private->pVideoDecodInstance.pVdec_Instance);
			omx_private->pVideoDecodInstance.isVPUClosed = OMX_TRUE;
		}

		LOGE("[VDEC-%d] try to restore decode error", omx_private->pVideoDecodInstance.nVdec_Instance);

		goto Success_Proc;
	}
#endif

Error_Proc:
	{
		if(omx_private->pVideoDecodInstance.isVPUClosed != OMX_TRUE)
		{
			/* close VPU */
			omx_private->pVideoDecodInstance.gspfVDec( VDEC_CLOSE, NULL, NULL, &(omx_private->pVideoDecodInstance.gsVDecOutput), omx_private->pVideoDecodInstance.pVdec_Instance);
			omx_private->pVideoDecodInstance.isVPUClosed = OMX_TRUE;

			/* Report error event */
			(*(omx_private->callbacks->EventHandler))(openmaxStandComp, omx_private->callbackData,
					OMX_EventError, OMX_ErrorHardware,
					0, NULL);
		}
	}

Success_Proc:

#ifdef SET_FRAMEBUFFER_INTO_MAX
	if(omx_private->bDetected_res_changed == OMX_FALSE)
#endif
		pInputBuffer->nFilledLen =0;
	pOutputBuffer->nFilledLen = 0;


#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
	omx_private->gralloc_info.m_pDispOut[PA][0] = omx_private->gralloc_info.m_pDispOut[PA][1] = omx_private->gralloc_info.m_pDispOut[PA][2] = NULL;
	omx_private->gralloc_info.m_pDispOut[VA][0] = omx_private->gralloc_info.m_pDispOut[VA][1] = omx_private->gralloc_info.m_pDispOut[VA][2] = NULL;
#endif

	return;
}

static int clear_vpu_buffer(int buffer_id, omx_videodec_component_PrivateType* omx_private)
{
	int ret =0;
	int cleared_buff_count = 0;
	int loopCount = 0;

	while(buffer_id < omx_private->mBuffVpu[omx_private->out_index].Display_Buff_ID && loopCount < MAX_CHECK_COUNT_FOR_CLEAR)
	{
		usleep(10000);
		if(omx_private->bVSyncMode)
		{
			// Case: normal decoder cerated from player that use video-vsync.
			buffer_id = ioctl(omx_private->mProxy_fd, TCC_LCDC_VIDEO_GET_DISPLAYED, NULL) ;  // TCC_LCDC_HDMI_GET_DISPLAYED
		}
		else
		{
			// Case : decoder created from MooPlayer on Google-TV Platform.
			//		  decoder using hwrenderer without video-vsync.(ex. flash playback with full-view mode.)
			// => Following process (buffer-id management) is needed to find buffer that is possible to be cleared.
			buffer_id = ioctl(omx_private->mProxy_fd, TCC_VIDEO_GET_DISPLAYED_BUFFER_ID, omx_private->pVideoDecodInstance.nVdec_Instance) ;
		}
		loopCount++;
		if(buffer_id == -2)
			return 0; //ZzaU:: in case of seek status, return
		else if(buffer_id < 0)
			break;
	}

	while( buffer_id >= omx_private->mBuffVpu[omx_private->out_index].Display_Buff_ID && omx_private->used_fifo_count > 0
		&& omx_private->mBuffVpu[omx_private->out_index].Display_cleared == 0 )
	{
		ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &(omx_private->mBuffVpu[omx_private->out_index].Display_index), NULL,  (omx_private->pVideoDecodInstance.pVdec_Instance));

		if(ret  < 0 )
		{
			LOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %d", omx_private->mBuffVpu[omx_private->out_index].Display_index, ret );
			return ret;
		}
		///wz// LOGE("### FLAG CLEAR idx(%d), displayed(%d)", omx_private->out_index, tmp ) ;
		omx_private->mBuffVpu[omx_private->out_index].Display_cleared = 1;
		omx_private->out_index = (omx_private->out_index + 1) % omx_private->max_fifo_cnt;
		omx_private->used_fifo_count--;
		cleared_buff_count++;
		break; // ZzaU :: There is no need to clear all buffers eventhough there are too many buffers having smaller id than coressponding one. clear only one.
	}

	if( cleared_buff_count == 0 && omx_private->mBuffVpu[omx_private->out_index].Display_cleared == 0 )
	{
		ioctl(omx_private->mProxy_fd, TCC_LCDC_VIDEO_CLEAR_FRAME, omx_private->mBuffVpu[omx_private->out_index].Display_Buff_ID);
		LOGW("Video Buffer Clear Sync Fail : %d %ld , loopcount(%d), VSYnc(%d) \n", buffer_id, omx_private->mBuffVpu[omx_private->out_index].Display_Buff_ID, loopCount, omx_private->bVSyncMode);
		ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &(omx_private->mBuffVpu[omx_private->out_index].Display_index), NULL, (omx_private->pVideoDecodInstance.pVdec_Instance) );

		if(ret  < 0 )
		{
			LOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %d", omx_private->mBuffVpu[omx_private->out_index].Display_index, ret );
			return ret;
		}
		///wz// LOGE("### FLAG CLEAR idx(%d), displayed(%d)", omx_private->out_index, tmp ) ;
		omx_private->mBuffVpu[omx_private->out_index].Display_cleared = 1;
		omx_private->out_index = (omx_private->out_index + 1) % omx_private->max_fifo_cnt;
		omx_private->used_fifo_count--;
	}

	return 0;
}

static int pull_displayedIndex(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer)
{
	unsigned int start_idx = 0;
	int ret;
	int curr_displaying_idx = -1;
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

#ifdef VPU_BUFFER_MANAGEMENT
	if(!omx_private->bVSyncMode && (omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr))
	{
		// Case: => just in case of players that use gralloc-buffer.
		//		 decoder created from flash player and html5 (both in inline-mode case.)
		//       other decoders except one(using hwrenderer) during multi-playback
		//       decoder created fram Video-Editor (case by case)
		//		 decoder using hwrenderer without video-vsync.(ex. flash playback with full-view mode.)
		// => following process is needed to deal with buffers dropped by SurfaceTexture(BufferQueue management) or Player(for A/V Sync)
		if( !omx_private->frm_clear )
			return -1;

		curr_displaying_idx = ioctl(omx_private->mProxy_fd, TCC_VIDEO_GET_DISPLAYING_IDX, NULL);

		while(omx_private->max_fifo_cnt > start_idx)
		{
			if( omx_private->mBuffVpu[start_idx].Keep_index != -1 && (curr_displaying_idx != -1 && omx_private->mBuffVpu[start_idx].Keep_index != curr_displaying_idx))
			{
				DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: keep DispIdx Clear %ld", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->mBuffVpu[start_idx].Keep_index);
				if( ( ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &(omx_private->mBuffVpu[start_idx].Keep_index), NULL, (omx_private->pVideoDecodInstance.pVdec_Instance)) ) < 0 )
				{
					LOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %d", omx_private->mBuffVpu[start_idx].Keep_index, ret );
					VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
					return -1;
				}
				omx_private->mBuffVpu[start_idx].Keep_index = -1;
			}
			start_idx++;
		}

		start_idx = 0;
		while(omx_private->max_fifo_cnt > start_idx)
		{
			if( omx_private->mBuffVpu[start_idx].Display_out_addr == (OMX_U32)pOutputBuffer->pBuffer ){
				DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: pull_displayedIndex[%ld] :: 0x%x-%d index.", omx_private->pVideoDecodInstance.nVdec_Instance, start_idx, pOutputBuffer->pBuffer, omx_private->mBuffVpu[start_idx].Display_index);

				if(curr_displaying_idx != -1 && curr_displaying_idx == omx_private->mBuffVpu[start_idx].Display_index){
					omx_private->mBuffVpu[start_idx].Keep_index = omx_private->mBuffVpu[start_idx].Display_index;
					DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: pull_displayedIndex[%d] :: can't offer it due to displaying status. 0x%x-%d index.", omx_private->pVideoDecodInstance.nVdec_Instance, start_idx, pOutputBuffer->pBuffer, omx_private->mBuffVpu[start_idx].Display_index);
					return -1;
				}
				else
					return omx_private->mBuffVpu[start_idx].Display_index;
			}
			start_idx++;
		}
	}
	else
#endif
	{
		// Case: normal decoder cerated from player that use video-vsync.
		//		 decoders created from MooPlayer on Google-TV Platform.
		//       decoder created fram Video-Editor (case by case)
		return omx_private->mBuffVpu[omx_private->out_index].Display_index;
	}

	ALOGE("pull_displayedIndex error. 0x%x", (unsigned int)pOutputBuffer->pBuffer);

	return -1;
}

static int push_displayedIndex(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer, OMX_S32 dispOutIdx )
{
	unsigned int start_idx = 0;
	int ret = 0;
	int result = 1; // 1: success in new queue, 0: success in old queue, -1: fail to queue 
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

#ifdef VPU_BUFFER_MANAGEMENT
	if(!omx_private->bVSyncMode && (omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr))
	{
		// Case: => just in case of players that use gralloc-buffer.
		//		 decoder created from flash player and html5 with inline-mode
		//       other decoders except one(video-vsync) during multi-playback
		//       decoder created fram Video-Editor
		// => following process is needed to deal with buffers dropped by SurfaceTexture(BufferQueue management) or Player(for A/V Sync)
		if( omx_private->frm_clear )
		{
			while(omx_private->max_fifo_cnt > start_idx)
			{
				if( omx_private->mBuffVpu[start_idx].Display_out_addr == (OMX_U32)pOutputBuffer->pBuffer )
					break;
				start_idx++;
			}

			DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: push_displayedIndex[%ld]-0x%x :: %ld -> %ld index.", omx_private->pVideoDecodInstance.nVdec_Instance, start_idx, (unsigned int)pOutputBuffer->pBuffer, omx_private->mBuffVpu[start_idx].Display_index, dispOutIdx);

			omx_private->mBuffVpu[start_idx].Display_cleared = 0;
			omx_private->mBuffVpu[start_idx].Display_index = dispOutIdx;
			omx_private->mBuffVpu[start_idx].Display_Buff_ID = omx_private->buffer_unique_id;
			omx_private->mBuffVpu[start_idx].Display_out_addr = (OMX_U32)pOutputBuffer->pBuffer;

			if(omx_private->max_fifo_cnt < start_idx){
				ALOGE("push_displayedIndex error[can't find exact index].");
				return -1;
			}
		}
		else
		{
			OMX_BOOL bSameAddr = OMX_FALSE;
			int pushed_index = omx_private->in_index;

			//check if there is same address.(due to frame-drop)
			while(omx_private->in_index > start_idx)
			{
				if( omx_private->mBuffVpu[start_idx].Display_out_addr == (OMX_U32)pOutputBuffer->pBuffer ){
					bSameAddr = OMX_TRUE;
					pushed_index = start_idx;
					result = 0;
					break;
				}
				start_idx++;
			}

			if(bSameAddr){
				if( ( ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &omx_private->mBuffVpu[pushed_index].Display_index, NULL, omx_private->pVideoDecodInstance.pVdec_Instance) ) < 0 )
				{
					LOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %d", omx_private->mBuffVpu[pushed_index].Display_index, ret );
					VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
					return -1;
				}
				DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: clear and push_displayedIndex[%d]-0x%x :: %ld -> %ld index.", omx_private->pVideoDecodInstance.nVdec_Instance, start_idx, (unsigned int)pOutputBuffer->pBuffer, omx_private->mBuffVpu[pushed_index].Display_index, dispOutIdx);
				omx_private->mBuffVpu[pushed_index].Display_cleared = 1;
			}

			DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: normal push_displayedIndex[%d]-0x%x :: %d index. fifo_count(%d/%d)", omx_private->pVideoDecodInstance.nVdec_Instance, pushed_index, pOutputBuffer->pBuffer, dispOutIdx, omx_private->used_fifo_count, omx_private->max_fifo_cnt);
			omx_private->mBuffVpu[pushed_index].Display_cleared = 0;
			omx_private->mBuffVpu[pushed_index].Display_index = dispOutIdx;
			omx_private->mBuffVpu[pushed_index].Display_Buff_ID = omx_private->buffer_unique_id;
			omx_private->mBuffVpu[pushed_index].Display_out_addr = (OMX_U32)pOutputBuffer->pBuffer;
		}
	}
	else
#endif
	{
		// Case: normal decoder cerated from player that use video-vsync.
		//		 decoders created from MooPlayer on Google-TV Platform.
		//       decoder created fram Video-Editor (case by case)
		omx_private->mBuffVpu[omx_private->in_index].Display_cleared = 0;
		omx_private->mBuffVpu[omx_private->in_index].Display_index = dispOutIdx;
		omx_private->mBuffVpu[omx_private->in_index].Display_Buff_ID = omx_private->buffer_unique_id;
		omx_private->mBuffVpu[omx_private->in_index].Display_out_addr = (OMX_U32)pOutputBuffer->pBuffer;
	}

	return result;
}

static int init_buffer_id_for_kernel(omx_videodec_component_PrivateType* omx_private)
{
	if(omx_private->pVideoDecodInstance.isVPUClosed == OMX_FALSE)
	{
		vbuffer_manager vBuff_Init;

		vBuff_Init.istance_index = omx_private->pVideoDecodInstance.nVdec_Instance;
		vBuff_Init.index = -1;
		ioctl(omx_private->mProxy_fd, TCC_VIDEO_SET_DISPLAYED_BUFFER_ID, &vBuff_Init);
	}

	return 0;
}

static int init_stBuffer_Management(omx_videodec_component_PrivateType* omx_private)
{
	int ret;

	omx_private->in_index = omx_private->out_index = omx_private->frm_clear = 0;
#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	omx_private->used_fifo_count = 0;
#endif
	memset(omx_private->mBuffVpu, 0x00, sizeof(VBUFFER_MANAGEMENT));
	{
		int start_idx = 0;

		while( VPU_BUFF_COUNT*2 > start_idx )
		{
			omx_private->mBuffVpu[start_idx].Display_cleared = 1;
			omx_private->mBuffVpu[start_idx].Display_out_addr = 0x00;
			omx_private->mBuffVpu[start_idx].Keep_index = -1;
			start_idx++;
		}
	}

	init_buffer_id_for_kernel(omx_private);

	return 0;
}

static int clear_all_displayedIndex(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BUFFERHEADERTYPE* pOutputBuffer, OMX_BOOL bSeek)
{
	int ret = 0;
	unsigned int start_idx = 0;
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

	ioctl(omx_private->mProxy_fd, TCC_LCDC_VIDEO_CLEAR_FRAME, omx_private->buffer_unique_id);
	if(bSeek == OMX_TRUE)
		ioctl(omx_private->mProxy_fd, TCC_LCDC_VIDEO_SWAP_VPU_FRAME, omx_private->buffer_unique_id);

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
	LOGD("Used Buffer Count %ld/%ld, buffer-id %d", omx_private->used_fifo_count, omx_private->max_fifo_cnt, omx_private->buffer_unique_id);
#endif

	while(omx_private->max_fifo_cnt > start_idx)
	{
		if( omx_private->mBuffVpu[start_idx].Keep_index != -1)
		{
			LOGD("keep DispIdx Clear %ld", omx_private->mBuffVpu[start_idx].Keep_index);
			if( ( ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &(omx_private->mBuffVpu[start_idx].Keep_index), NULL, (omx_private->pVideoDecodInstance.pVdec_Instance)) ) < 0 )
			{
				LOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %d", omx_private->mBuffVpu[start_idx].Keep_index, ret );
				VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
				return -1;
			}
			omx_private->mBuffVpu[start_idx].Keep_index = -1;
		}
		
		if( omx_private->mBuffVpu[start_idx].Display_cleared == 0 ){
			LOGD("DispIdx Clear %ld", omx_private->mBuffVpu[start_idx].Display_index);
			if( ( ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &(omx_private->mBuffVpu[start_idx].Display_index), NULL, (omx_private->pVideoDecodInstance.pVdec_Instance)) ) < 0 )
			{
				LOGE( "[VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %d", omx_private->mBuffVpu[start_idx].Display_index, ret );
				VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
				return -1;
			}
			omx_private->mBuffVpu[start_idx].Display_cleared = 1;
		}
		else{
			DPRINTF_VBUFFER_MANAGE("DispIdx %ld is already cleared.", omx_private->mBuffVpu[start_idx].Display_index);
		}
		start_idx++;
	}

	init_stBuffer_Management(omx_private);

	return 0;
}

#ifdef MOVE_HW_OPERATION
int copy_data_to_grallocbuffer(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BOOL hw_addr, unsigned int width, unsigned int height, unsigned char *YSrc, unsigned char *USrc, unsigned char *VSrc,
							char bSrcYUVInter, OMX_U8 *YDst, OMX_U8 *UDst, OMX_U8 *VDst, COPY_OPER_MODE cmd, OMX_BOOL bOnlyCopy)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

	unsigned int stride, stride_c;
	unsigned int framesize_Y, framesize_C;
	int ret_val = 0;

	if( bOnlyCopy )
		stride = width;
	else
		stride = ALIGNED_BUFF(width, 16);
    framesize_Y = ALIGNED_BUFF(stride * height, 64);

	stride_c = ALIGNED_BUFF(stride/2, 16);
    framesize_C = ALIGNED_BUFF(stride_c * height/2, 64);

	if(!hw_addr)
	{		
		if(cmd & SEND_CMD)
		{
			unsigned int VSrcY, VSrcU, VSrcV, VDestY, VDestU, VDestV;

			VSrcY = ((unsigned int)vpu_getFrameBufVirtAddr(YSrc, PA, omx_private->pVideoDecodInstance.pVdec_Instance));
			VSrcU = ((unsigned int)vpu_getFrameBufVirtAddr(USrc, PA, omx_private->pVideoDecodInstance.pVdec_Instance));
			VSrcV = ((unsigned int)vpu_getFrameBufVirtAddr(VSrc, PA, omx_private->pVideoDecodInstance.pVdec_Instance));

			VDestY = (unsigned int)YDst;
			VDestU = UDst;
			VDestV = VDst;
			if(VDestU == NULL)
				VDestU = VDestY + framesize_Y;
			if(VDestV== NULL)
				VDestV = VDestU + framesize_C;		

			memcpy((void*)VDestY, (void*)VSrcY, framesize_Y);
			if(bSrcYUVInter)
			{
				memcpy((void*)VDestU, (void*)VSrcU, framesize_C*2);
			}
			else
			{
				memcpy((void*)VDestU, (void*)VSrcU, framesize_C);
				memcpy((void*)VDestV, (void*)VSrcV, framesize_C);
			}
		}
	}
	else
	{
#ifdef USE_WMIXER_FOR_COPY
		WMIXER_INFO_TYPE WmixerInfo;
#else
		SCALER_TYPE ScaleInfo;
#endif
		struct pollfd poll_event[1];

		if(cmd & SEND_CMD)
		{
			if( omx_private->gralloc_info.fd_copy < 0 )
			{
				omx_private->gralloc_info.fd_copy = open(COPY_DEVICE, O_RDWR);
				if (omx_private->gralloc_info.fd_copy < 0) {
					LOGE("can't open[%s] %s", strerror(errno), COPY_DEVICE);
					return -1;
				}
			}

#ifdef VPU_FRAME_DUMP //for frame dump.
			if(total_frm != 0){
				if(total_frm % 150 == 0){
					FILE *pFs;
					unsigned int Y, U, V;

					Y = ((unsigned int)vpu_getFrameBufVirtAddr(YSrc, PA, omx_private->pVideoDecodInstance.pVdec_Instance));
					U = ((unsigned int)vpu_getFrameBufVirtAddr(USrc, PA, omx_private->pVideoDecodInstance.pVdec_Instance));
					V = ((unsigned int)vpu_getFrameBufVirtAddr(VSrc, PA, omx_private->pVideoDecodInstance.pVdec_Instance));

					pFs = fopen("/data/vFrame.yuv", "ab+");
					if (!pFs) {
						LOGE("Cannot open '/data/vFrame.yuv'");
						return 0;
					}
					else
					{
						if(pFs){
							fwrite( Y, stride*height, 1, pFs);
							fwrite( U, stride*height/4, 1, pFs);
							fwrite( V, stride*height/4, 1, pFs);
						}
						fclose(pFs);
					}
				}
			}
#endif

			if(!bSrcYUVInter && !VSrc)
			{
				LOGE("VSrc is null!!");
				return -1;
			}

#ifdef USE_WMIXER_FOR_COPY

			memset(&WmixerInfo, 0x00, sizeof(WmixerInfo));
			WmixerInfo.rsp_type			= WMIXER_INTERRUPT;

			WmixerInfo.src_y_addr		= (unsigned int)YSrc;
			WmixerInfo.src_u_addr		= (unsigned int)USrc;
			WmixerInfo.src_v_addr		= (unsigned int)VSrc;

			if(bSrcYUVInter)
				WmixerInfo.src_fmt		= VIOC_IMG_FMT_YUV420IL0;
			else
			{
#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
				if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MJPG)
				{
					//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
					if(omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
						WmixerInfo.src_fmt		= VIOC_IMG_FMT_YUV422SEP;
					else
						WmixerInfo.src_fmt		= VIOC_IMG_FMT_YUV420SEP;
				}
				else
#endif
					WmixerInfo.src_fmt		= VIOC_IMG_FMT_YUV420SEP;
			}

			WmixerInfo.dst_y_addr		= (unsigned int)YDst;
			WmixerInfo.dst_u_addr 		= (unsigned int)UDst;
			WmixerInfo.dst_v_addr 		= (unsigned int)VDst;

#ifdef USE_YUV420SP_DEFAULT
			if(1)
#else
			if(omx_private->blocalPlaybackMode)
#endif
			{
				if( bOnlyCopy )
					WmixerInfo.dst_fmt = WmixerInfo.src_fmt;
				else
					WmixerInfo.dst_fmt = VIOC_IMG_FMT_YUV420IL0;
				if( WmixerInfo.dst_u_addr == NULL )
					WmixerInfo.dst_u_addr = (unsigned int)((char*)WmixerInfo.dst_y_addr + framesize_Y);
				if( WmixerInfo.dst_v_addr == NULL )
					WmixerInfo.dst_v_addr = (unsigned int)((char*)WmixerInfo.dst_u_addr + framesize_C);
			}
			else
			{
				WmixerInfo.dst_fmt = VIOC_IMG_FMT_YUV420SEP;
				if( WmixerInfo.dst_v_addr == NULL )
					WmixerInfo.dst_v_addr = (unsigned int)((char*)WmixerInfo.dst_y_addr + framesize_Y);
				if( WmixerInfo.dst_u_addr == NULL )
					WmixerInfo.dst_u_addr = (unsigned int)((char*)WmixerInfo.dst_v_addr + framesize_C);
			}

			WmixerInfo.img_width 		= stride;
			WmixerInfo.img_height		= height;

			GBUG_MSG("%s copy :: 0x%x-0x%x-0x%x -> 0x%x(0x%x-0x%x-0x%x)", COPY_DEVICE, WmixerInfo.src_y_addr, WmixerInfo.src_u_addr, WmixerInfo.src_v_addr, (unsigned int)YDst, WmixerInfo.dst_y_addr, WmixerInfo.dst_u_addr, WmixerInfo.dst_v_addr);

			if (ioctl(omx_private->gralloc_info.fd_copy, TCC_WMIXER_IOCTRL, &WmixerInfo) < 0)
			{
				LOGE("%s Out Error!", COPY_DEVICE);
				return -1;
			}
#else

			memset(&ScaleInfo, 0x00, sizeof(ScaleInfo));
			ScaleInfo.src_Yaddr			= (char*)YSrc;
			ScaleInfo.src_Uaddr			= (char*)USrc;
			ScaleInfo.src_Vaddr			= (char*)VSrc;
			ScaleInfo.responsetype 		= SCALER_INTERRUPT;
			if(bSrcYUVInter)
				ScaleInfo.src_fmt		= SCALER_YUV420_inter;
			else
			{
#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
				if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MJPG)
				{
					//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
					if(omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
						ScaleInfo.src_fmt		= SCALER_YUV422_sp;
					else
						ScaleInfo.src_fmt		= SCALER_YUV420_sp;
				}
				else
#endif
					ScaleInfo.src_fmt		= SCALER_YUV420_sp;
			}
			ScaleInfo.src_ImgWidth 		= stride;
			ScaleInfo.src_ImgHeight		= height;
			ScaleInfo.src_winLeft		= 0;
			ScaleInfo.src_winTop		= 0;
			ScaleInfo.src_winRight 		= ScaleInfo.src_winLeft + ScaleInfo.src_ImgWidth;
			ScaleInfo.src_winBottom		= ScaleInfo.src_winTop + ScaleInfo.src_ImgHeight;

			ScaleInfo.dest_Yaddr		= (char*)YDst;
			ScaleInfo.dest_Uaddr 		= (char*)UDst;
			ScaleInfo.dest_Vaddr 		= (char*)VDst;

#ifdef USE_YUV420SP_DEFAULT
			if(1)
#else
			if(omx_private->blocalPlaybackMode)
#endif
			{
				ScaleInfo.dest_fmt 			= SCALER_YUV420_inter;
				if( ScaleInfo.dest_Uaddr == NULL )
					ScaleInfo.dest_Uaddr = (char*)ScaleInfo.dest_Yaddr + framesize_Y;
				if( ScaleInfo.dest_Vaddr == NULL )
					ScaleInfo.dest_Vaddr = (char*)ScaleInfo.dest_Uaddr + framesize_C;
			}
			else
			{
				ScaleInfo.dest_fmt 			= SCALER_YUV420_sp;
				if( ScaleInfo.dest_Vaddr == NULL )
					ScaleInfo.dest_Vaddr = (char*)ScaleInfo.dest_Yaddr + framesize_Y;
				if( ScaleInfo.dest_Uaddr == NULL )
					ScaleInfo.dest_Uaddr = (char*)ScaleInfo.dest_Vaddr + framesize_C;
			}
			GBUG_MSG("%s copy :: 0x%x-0x%x-0x%x -> 0x%x(0x%x-0x%x-0x%x)", COPY_DEVICE, ScaleInfo.src_Yaddr, ScaleInfo.src_Uaddr, ScaleInfo.src_Vaddr, YDst, ScaleInfo.dest_Yaddr, ScaleInfo.dest_Uaddr, ScaleInfo.dest_Vaddr);

			ScaleInfo.dest_ImgWidth		= stride;
			ScaleInfo.dest_ImgHeight	= height;
			ScaleInfo.dest_winLeft 		= 0;
			ScaleInfo.dest_winTop		= 0;
			ScaleInfo.dest_winRight		= ScaleInfo.dest_winLeft + ScaleInfo.dest_ImgWidth;
			ScaleInfo.dest_winBottom	= ScaleInfo.dest_winTop + ScaleInfo.dest_ImgHeight;

			if (ioctl(omx_private->gralloc_info.fd_copy, TCC_SCALER_IOCTRL, &ScaleInfo) < 0)
			{
				LOGE("%s Out Error!", COPY_DEVICE);
				return -1;
			}
#endif

#ifdef UMP_COPIED_FRAME_DUMP //for frame dump. 
			if(total_frm != 0){
				if(total_frm % 200 == 0){
					FILE *pFs;
					unsigned int Y, U, V;

#ifdef USE_WMIXER_FOR_COPY
					Y = ((unsigned int)omx_private->mTMapInfo + (WmixerInfo.dst_y_addr - omx_private->mUmpReservedPmap.base));
					U = ((unsigned int)omx_private->mTMapInfo + (WmixerInfo.dst_u_addr - omx_private->mUmpReservedPmap.base));
					V = ((unsigned int)omx_private->mTMapInfo + (WmixerInfo.dst_v_addr - omx_private->mUmpReservedPmap.base));
#else
					Y = ((unsigned int)omx_private->mTMapInfo + (ScaleInfo.dest_Yaddr - omx_private->mUmpReservedPmap.base));
					U = ((unsigned int)omx_private->mTMapInfo + (ScaleInfo.dest_Uaddr - omx_private->mUmpReservedPmap.base));
					V = ((unsigned int)omx_private->mTMapInfo + (ScaleInfo.dest_Vaddr - omx_private->mUmpReservedPmap.base));
#endif

					pFs = fopen("/data/frame.yuv", "ab+");
					if (!pFs) {
						LOGE("Cannot open '/data/frame.yuv'");
						return 0;
					}
					else
					{
						if(pFs){
							fwrite( Y, stride*height, 1, pFs);
							fwrite( U, stride*height/4, 1, pFs);
							fwrite( V, stride*height/4, 1, pFs);
						}
						fclose(pFs);
					}
				}
			}
#endif
		}

		if(cmd & WAIT_RESPOND)
		{
			int ret;
			memset(poll_event, 0, sizeof(poll_event));
			poll_event[0].fd = omx_private->gralloc_info.fd_copy;
			poll_event[0].events = POLLIN;
			ret = poll((struct pollfd*)poll_event, 1, 100);

			if (ret < 0) {
				LOGE("%s poll error '%s'", COPY_DEVICE, strerror(errno));
				ret_val = -1;
			}else if (ret == 0) {
				LOGE("%s poll timeout", COPY_DEVICE);
				ret_val = -1;
			}else if (ret > 0) {
				if (poll_event[0].revents & POLLERR) {
					LOGE("%s poll POLLERR", COPY_DEVICE);
					ret_val = -1;
				}
			}
		}

		if(ret_val < 0)
		{
			if(omx_private->gralloc_info.fd_copy != -1){
				close(omx_private->gralloc_info.fd_copy);
				omx_private->gralloc_info.fd_copy = -1;
			}
		}
	}

	return ret_val;
}
#endif

OMX_BOOL check_valid_hw_availble(OMX_COMPONENTTYPE *openmaxStandComp, unsigned int fd_val)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;

	if(fd_val < omx_private->mUmpReservedPmap.base)
		return OMX_FALSE;

	if(fd_val > (omx_private->mUmpReservedPmap.base + omx_private->mUmpReservedPmap.size))
		return OMX_FALSE;

	return OMX_TRUE;
}

OMX_BOOL check_copy_toGralloc(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BOOL hw_addr)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	char value[PROPERTY_VALUE_MAX];
	OMX_BOOL isHWrendered = OMX_FALSE;
	OMX_U32 skip_count = 12;
	
    if( hw_addr && omx_private->mTmem_fd <= 0 ) {
        LOGE("[VDEC-%d] %s device is not opened.", omx_private->pVideoDecodInstance.nVdec_Instance, TMEM_DEVICE);
        return OMX_TRUE;
    }

	property_get("tcc.video.hwr.id", value, "0");
	if( atoi(value) == omx_private->mCodecStart_ms){
		DBUG_MSG("[VDEC-%d] hwr is detected? 0x%x =? 0x%x.", omx_private->pVideoDecodInstance.nVdec_Instance, atoi(value), omx_private->mCodecStart_ms);
		vdec_set_rendered_index(omx_private->pVideoDecodInstance.pVdec_Instance);
		isHWrendered = OMX_TRUE;
	}

	if( hw_addr )
	{
		if(omx_private->bWFDSyncMode && omx_private->bTEEEnable)
		{
			return OMX_FALSE;
		}

		skip_count = 12;
		property_get("tcc.sys.output_mode_detected", value, "0");
	    if( !atoi(value) )
		{
#ifdef USE_LCD_VIDEO_VSYNC
			if( omx_private->bDRAM_16bit )
				goto EXCEPTIONAL_PROC;
			else
#endif
				return OMX_TRUE; //LCD mode always copy decoded data into gralloc buffer.
	    }
		else //Output display Mode
		{			
			if( isHWrendered )
				return OMX_FALSE;
			else
				return OMX_TRUE;
		}
	}
	else
	{
		skip_count = 24;
	}

EXCEPTIONAL_PROC:

	if( isHWrendered )
		return OMX_FALSE;
	else{
		if( omx_private->isTry_count_for_copying < skip_count){
			DBUG_MSG("[VDEC-%d] skip frame %ld'th", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->isTry_count_for_copying);
			omx_private->isTry_count_for_copying++;
			return OMX_FALSE;
		}

		return OMX_TRUE;
	}
}

void* get_private_addr(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BOOL hw_addr, unsigned int fd_val, int width, int height)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
    int stride, stride_c, frame_len = 0;

	stride = (width + 15) & ~15;
    stride_c = (stride/2 + 15) & ~15;
	frame_len = height * (stride + stride_c);// + 100;

    if( hw_addr && omx_private->mTmem_fd <= 0 ) {
        LOGE("[VDEC-%d] %s device is not opened.", omx_private->pVideoDecodInstance.nVdec_Instance, TMEM_DEVICE);
        return NULL;
    }

	if(hw_addr)
		return (void*)((unsigned int)omx_private->mTMapInfo + (fd_val - omx_private->mUmpReservedPmap.base) + frame_len);
	else
		return 	(void*)(fd_val + frame_len);
}

#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
static int extract_seqheader(
		const unsigned char	*pbyStreamData,
		long				lStreamDataSize,
		unsigned char		**ppbySeqHeaderData,
		long				*plSeqHeaderSize,
		int					codec_type,
		OMX_COMPONENTTYPE *openmaxStandComp
		)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	long i;

	if( codec_type == OMX_VIDEO_CodingAVC || codec_type == OMX_VIDEO_CodingMVC)
	{
		long l_seq_start_pos = 0, l_seq_end_pos = 0, l_seq_length = 0; // Start Position, End Position, Length of the sequence header
		long l_sps_found = 0;
		long l_pps_found = 0;

		unsigned long ul_read_word_buff;	   	    	            //4 byte temporary buffer
		unsigned long ul_masking_word_seq          = 0x0FFFFFFF;    //Masking Value for finding H.264 sequence header
		unsigned long ul_masking_word_sync         = 0x0FFFFFFF;    //Masking Value for finding sync word of H.264
		unsigned long ul_h264_result_word_seq_SPS  = 0x07010000;    //Masking result should be this value in case of SPS. SPS Sequence header of H.264 must start by "00 00 01 x7"
		unsigned long ul_h264_result_word_seq_PPS  = 0x08010000;    //Masking result should be this value in case of PPS. PPS Sequence header of H.264 must start by "00 00 01 x8"
		unsigned long ul_h264_result_word_sync     = 0x01010000;    //Masking result should be this value. Sequence header of H.264 must start by "00 00 01 x1"
		unsigned long ul_h264_result_word_sync2    = 0x05010000;    //Masking result should be this value. Sequence header of H.264 must start by "00 00 01 x5"

		if ( lStreamDataSize < 4 )
			return 0; // there's no Seq. header in this frame. we need the next frame.

		if ( *plSeqHeaderSize > 0 )
		{
			// we already find the sps, pps in previous frame
			l_sps_found = 1;
			l_pps_found = 1;
			l_seq_start_pos = 0;
		}
		else
		{
			// find the SPS of H.264
			ul_read_word_buff = 0;
			ul_read_word_buff |= (pbyStreamData[0] << 8);
			ul_read_word_buff |= (pbyStreamData[1] << 16);
			ul_read_word_buff |= (pbyStreamData[2] << 24);

			for ( i = 0; i < lStreamDataSize-4; i++ )
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff &= 0x00FFFFFF;
				ul_read_word_buff |= (pbyStreamData[i+3] << 24);

				if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_SPS )
				{
					// SPS Sequence Header has been detected
					l_sps_found = 1;
					l_seq_start_pos = i;          // save the start position of the sequence header

					break;
				}

				// Continue to find the sps in next loop
			}

			if ( l_sps_found == 1 )
			{
				// Now, let's start to find the PPS of the Seq. header.

				i = i + 4;
				ul_read_word_buff = 0;
				ul_read_word_buff |= (pbyStreamData[i] << 8);
				ul_read_word_buff |= (pbyStreamData[i+1] << 16);
				ul_read_word_buff |= (pbyStreamData[i+2] << 24);

				for (  ; i < lStreamDataSize - 4; i++ )
				{
					ul_read_word_buff = ul_read_word_buff >> 8;
					ul_read_word_buff &= 0x00FFFFFF;
					ul_read_word_buff |= (pbyStreamData[i+3] << 24);

					if ( (ul_read_word_buff & ul_masking_word_seq) == ul_h264_result_word_seq_PPS )
					{
						// PPS has been detected.
						l_pps_found = 1;
						break;
					}

					// Continue to find the pps in next loop
				}
			}
		}

		if ( l_pps_found == 1 )
		{
			// Now, let's start to find the next sync word to find the end position of Seq. Header

			if ( *plSeqHeaderSize > 0 )
				i = 0;     // we already find the sps, pps in previous frame
			else
				i = i + 4;
			ul_read_word_buff = 0;
			ul_read_word_buff |= (pbyStreamData[i] << 8);
			ul_read_word_buff |= (pbyStreamData[i+1] << 16);
			ul_read_word_buff |= (pbyStreamData[i+2] << 24);

			for ( ; i < lStreamDataSize - 4; i++ )
			{
				ul_read_word_buff = ul_read_word_buff >> 8;
				ul_read_word_buff &= 0x00FFFFFF;
				ul_read_word_buff |= (pbyStreamData[i+3] << 24);

				if ( ((ul_read_word_buff & ul_masking_word_sync) == ul_h264_result_word_sync) || // 00 00 01 x1
					 ((ul_read_word_buff & ul_masking_word_sync) == ul_h264_result_word_sync2))  // 00 00 01 x5
				{
					long l_cnt_zeros = 0;       // to count extra zeros ahead of "00 00 01"

					// next sync-word has been found.
					l_seq_end_pos = i - 1;      // save the end position of the sequence header (00 00 01 case)

					// any zeros can be added ahead of "00 00 01" sync word by H.264 specification. Count the number of these leading zeros.
					while (1)
					{
						l_cnt_zeros++;

						if(i >= l_cnt_zeros) //ZzaU :: to prevent segmentation fault.
						{
							if ( pbyStreamData[i-l_cnt_zeros] == 0 )
							{
								l_seq_end_pos = l_seq_end_pos -1;    // decrease the end position of Seq. Header by 1.
							}
							else
								break;
						}
						else
							break;
					}

					if ( *plSeqHeaderSize > 0 )
					{
						if ( omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC) {
							// we already find the sps, pps in previous frame
							l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;

							if ( l_seq_length > 0 )
							{
								if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
									return 0;

								*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
								memcpy( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
								*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
							}
						}
						return 1;

					}
					else
					{
						// calculate the length of the sequence header
						l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;

						if ( l_seq_length > 0 )
						{
							*ppbySeqHeaderData = TCC_malloc( l_seq_length );     // allocation memory for sequence header array (must free this at the CLOSE step)
							memcpy( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
							*plSeqHeaderSize = l_seq_length;

							return 1;  // We've found the sequence header successfully
						}
					}
				}

				// Continue to find the sync-word in next loop
			}
		}

		if (l_sps_found == 1 && l_pps_found == 1)
		{
			// we found sps and pps, but we couldn't find the next sync word yet
			l_seq_end_pos = lStreamDataSize - 1;
			l_seq_length = l_seq_end_pos - l_seq_start_pos + 1;        // calculate the length of the sequence header

			if ( *plSeqHeaderSize > 0 )
			{
				if ( omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC) {
					// we already saved the sps, pps in previous frame
					if ( l_seq_length > 0 )
					{
						if ( *plSeqHeaderSize + l_seq_length > MAX_SEQ_HEADER_ALLOC_SIZE )     // check the maximum threshold
							return 0;

						*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + l_seq_length );     // allocate memory for sequence header array (must free this at the CLOSE step)
						memcpy( (unsigned char*) (*ppbySeqHeaderData) + *plSeqHeaderSize , &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
						*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
					}
				}

			}
			else
			{
				*ppbySeqHeaderData = TCC_malloc( l_seq_length );           // allocate memory for sequence header array (must free this at the CLOSE step)
				memcpy( (unsigned char*) (*ppbySeqHeaderData), &pbyStreamData[l_seq_start_pos], l_seq_length);   // save the seq. header to array
				*plSeqHeaderSize = *plSeqHeaderSize + l_seq_length;
			}
		}
	}
	else
	{
		unsigned long syncword = 0xFFFFFFFF;
		int	start_pos = -1;
		int end_pos = -1;
		int i;

		syncword <<= 8;
		syncword |= pbyStreamData[0];
		syncword <<= 8;
		syncword |= pbyStreamData[1];
		syncword <<= 8;
		syncword |= pbyStreamData[2];

		for(i = 3; i < lStreamDataSize; i++) {
			syncword <<= 8;
			syncword |= pbyStreamData[i];

			if( (syncword >> 8) == 1 ) {	// 0x 000001??
				if( syncword >= MPEG4_VOL_STARTCODE_MIN &&
					syncword <= MPEG4_VOL_STARTCODE_MAX )
					start_pos = i-3;
				//else if( start_pos >= 0 || *plSeqHeaderSize > 0 ) {
				else if( start_pos >= 0 && *plSeqHeaderSize > 0 ) {
					if ( syncword == MPEG4_VOP_STARTCODE )
					{
						end_pos = i-3;
						break;
					}
				}
			}
		}

		if (start_pos >= 0 && end_pos == -1) {
			//end_pos = lStreamDataSize - start_pos;
			end_pos = lStreamDataSize;
		}

		if( start_pos >= 0 ) {
			if( end_pos >= 0 ) {
				*plSeqHeaderSize = end_pos-start_pos;
				*ppbySeqHeaderData = TCC_malloc( *plSeqHeaderSize );     // allocate memory for sequence header array
				memcpy(*ppbySeqHeaderData, pbyStreamData + start_pos, *plSeqHeaderSize);
				return 1;
			}
			else {
				*plSeqHeaderSize = lStreamDataSize - start_pos;
				*ppbySeqHeaderData = TCC_malloc( *plSeqHeaderSize );     // allocate memory for sequence header array
				memcpy(*ppbySeqHeaderData, pbyStreamData + start_pos, *plSeqHeaderSize);
				return 0;
			}
		}
		else if( *plSeqHeaderSize > 0 ) {
			if( end_pos < 0 )
				end_pos = lStreamDataSize;

			if ( *plSeqHeaderSize + end_pos > MAX_SEQ_HEADER_ALLOC_SIZE ) // check the maximum threshold
				return 0;

			*ppbySeqHeaderData = TCC_realloc(*ppbySeqHeaderData , *plSeqHeaderSize + end_pos);     // re-allocate memory for sequence header array
			memcpy(*ppbySeqHeaderData + *plSeqHeaderSize, pbyStreamData, end_pos);
			*plSeqHeaderSize += end_pos;
			return 1;
		}
	}

	return 0; // We couldn't find the complete sequence header yet. We need to search the next frame data.
}
#endif

#if defined(TCC_VPU_C7_INCLUDE)
#define VP8_BUFF_SIZE 44  // temporary  buffer for saving start code for VP8

static void put_LE32(unsigned char **pp, unsigned int var)
{
	**pp = (unsigned char)((var)>>0);
	*pp = *pp + 1;
	**pp = (unsigned char)((var)>>8);
	*pp = *pp + 1;
	**pp = (unsigned char)((var)>>16);
	*pp = *pp + 1;
	**pp = (unsigned char)((var)>>24);
	*pp = *pp + 1;
}

static void put_LE16(unsigned char **pp, unsigned int var)
{
	**pp = (unsigned char)((var)>>0);
	*pp = *pp + 1;
	**pp = (unsigned char)((var)>>8);
	*pp = *pp + 1;
}

// Function for attaching start code for VP8 codec. This is only for TCC892x(VPU:CODA960).
static int get_startcode_for_VP8( int for_seqHeader, int frameWidth, int frameHeight, int streamDataSize, unsigned char *startCodeBuffer, int *startCodeBufferSize )
{
    unsigned char* puc_buffer_ptr = startCodeBuffer;

    if(startCodeBuffer == NULL)
        return -1;

    if(for_seqHeader)
    {
        // Make Seq Header for VP8
        // Only for the first frame
        *startCodeBufferSize = 44;

        *puc_buffer_ptr++ = (unsigned char)'D';
        *puc_buffer_ptr++ = (unsigned char)'K';
        *puc_buffer_ptr++ = (unsigned char)'I';
        *puc_buffer_ptr++ = (unsigned char)'F';
        put_LE16(&puc_buffer_ptr, 0x00);
        put_LE16(&puc_buffer_ptr, 0x20);
        *puc_buffer_ptr++ = (unsigned char)'V';
        *puc_buffer_ptr++ = (unsigned char)'P';
        *puc_buffer_ptr++ = (unsigned char)'8';
        *puc_buffer_ptr++ = (unsigned char)'0';
        put_LE16(&puc_buffer_ptr, frameWidth);
        put_LE16(&puc_buffer_ptr, frameHeight);
        put_LE32(&puc_buffer_ptr, 30);
        put_LE32(&puc_buffer_ptr, 1);
        put_LE32(&puc_buffer_ptr, 3000);
        put_LE32(&puc_buffer_ptr, 0);
    }
    else
    {
        // Not the first frame.
        *startCodeBufferSize = 12;
    }

    // Make Pic Header for VP8
    // For all frames
    put_LE32(&puc_buffer_ptr, streamDataSize);
    put_LE32(&puc_buffer_ptr, 0);
    put_LE32(&puc_buffer_ptr, 0);

    return 0;
}

static int check_startcode_for_VP8( unsigned char *streamBuffer, int streamBufferSize, int for_seqHeader )
{
    unsigned char* pBuff = streamBuffer;

    if(streamBuffer == NULL)
        return -1;

    if(for_seqHeader)
    {
        if(*pBuff++ == (unsigned char)'D'
	        && *pBuff++ == (unsigned char)'K'
	        && *pBuff++ == (unsigned char)'I'
	        && *pBuff++ == (unsigned char)'F')
        {
			return 1;
        }
    }
    else
    {
		unsigned char tBuff[12+1];
		unsigned char* ptBuff = tBuff;
	    put_LE32(&ptBuff, streamBufferSize);

		if(*pBuff++ == tBuff[0] && *pBuff++ == tBuff[1] && *pBuff++ == tBuff[2] && *pBuff++ == tBuff[3])
			return 1;
    }

    return 0;
}
#endif

//#define TC_SECURE_MEMORY_COPY
#if defined(TC_SECURE_MEMORY_COPY)
extern int
TC_SecureMemoryCopy(
	unsigned int paTarget,
	unsigned int paSource,
	unsigned int nSize
);
#endif

OMX_U32 get_SecuredBuf_PhysicalAddr(omx_videodec_component_PrivateType* omx_private, OMX_U32 virtAddr)
{
	if( omx_private->bSecuredInBufferMode == OMX_FALSE)
		return OMX_FALSE;

	return (omx_private->mUmpSecuredInPmap.base + ((OMX_U32)virtAddr - (OMX_U32)omx_private->mSecureInMapInfo));
}

OMX_BOOL Secured_inBuffer_Process(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer, OMX_BOOL bSeqHeader)
{
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
	OMX_S32 ret = 0;

	if( omx_private->bSecuredInBufferMode == OMX_FALSE)
		return OMX_FALSE;

	// VPU can't support this yet.
	if(1)// bSeqHeader == OMX_TRUE )
	{
		//To Do :: should copy by TEE.
#if defined(TC_SECURE_MEMORY_COPY)
		if(omx_private->bTEEEnable)
		{
			unsigned int uiTargetAddr = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
			unsigned int uiSourceAddr = pInputBuffer->pBuffer;
			unsigned int uiLength = pInputBuffer->nFilledLen;

			//ALOGI("[SinB] Start : %08X -> %08X (%d)", uiSourceAddr, uiTargetAddr, uiLength);
			ret = TC_SecureMemoryCopy(uiTargetAddr, uiSourceAddr, uiLength);
			//ALOGI("[SinB] End(%d)", ret);
		}
		else
#endif
		{
			//ALOGI("Copy stream for SeqHeader.");
			memcpy((void*)vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance), pInputBuffer->pBuffer, pInputBuffer->nFilledLen);
		}

		omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
		omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);
	}
	else
	{
		omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = (unsigned char*)get_SecuredBuf_PhysicalAddr(omx_private, (OMX_U32)pInputBuffer->pBuffer);
		omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = pInputBuffer->pBuffer;

		//ALOGI("Stream for Decoder :: 0x%x", omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA]);
	}
	omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen = pInputBuffer->nFilledLen;

	if(ret < 0)
	{
		return OMX_FALSE;
	}

	return OMX_TRUE;
}

/** This function is used to process the input buffer and provide one output buffer
  */
void omx_videodec_component_BufferMgmtCallback(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE* pInputBuffer	, OMX_BUFFERHEADERTYPE* pOutputBuffer) {

	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
    char value[PROPERTY_VALUE_MAX], value1[PROPERTY_VALUE_MAX];
	OMX_S32 ret;
	OMX_S32 nOutputFilled = 0;
	OMX_S32 nLen = 0;
	int internalOutputFilled = 0;
	int dispMVCOutIdx=0;

#ifndef HAVE_ANDROID_OS
	OMX_U32 nSize;
#endif
	OMX_U32 output_len = 0;
	OMX_U32 input_offset = 0;
	OMX_U32 code_type = CODETYPE_NONE;
#ifdef ANDROID_USE_GRALLOC_BUFFER
	TCC_PLATFORM_PRIVATE_PMEM_INFO *plat_priv = NULL;
	TCC_PLATFORM_PRIVATE_PMEM_INFO privOutData;
	buffer_handle_t*  grallocHandle = NULL;
	OMX_U8 *pGrallocAddr;
	void *pGrallocPureAddr;
	#ifdef COMPARE_TIME_LOG
	clock_t start, end;
	#endif
	COPY_MODE gralloc_copy_mode = COPY_NONE;
	OMX_BOOL bCopyToGrallocBuff = OMX_FALSE;
	OMX_BOOL bHWaddr = OMX_FALSE;
#endif
	int decode_result;
	int i;
	dec_disp_info_t dec_disp_info_tmp;
	OMX_BOOL bJust_after_sequence_done = OMX_FALSE;
	OMX_BOOL bUse_backup_seqHeader = OMX_FALSE;
	int cleared_index_for_vpu = 0;

	dec_disp_info_tmp.m_bIsMvcDependent = 0;
	dec_disp_info_tmp.m_iTimeStamp = 0;
    omx_base_video_PortType *outPort = (omx_base_video_PortType *)omx_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

	if(omx_private->bWFD_mode == OMX_TRUE)
	{
	    omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs = 0;
	}

	if(omx_private->state != 3) {
	    LOGE("[VDEC-%d] => omx_private->state != 3", omx_private->pVideoDecodInstance.nVdec_Instance);
		return;
	}
#ifdef DIVX_DRM5
	if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MPEG4)
	{
		DivxDecryptEx(pInputBuffer->pBuffer,pInputBuffer->nFilledLen);
	}
#endif

	if(omx_private->bThumbnailMode && omx_private->pThumbnailBuff)
	{
		VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
		return;
	}

#ifdef SET_FRAMEBUFFER_INTO_MAX
	if(omx_private->bDetected_res_changed == OMX_TRUE && omx_private->bNoOutput_after_res_changed == OMX_TRUE)
	{
		VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, -RETCODE_CODEC_EXIT);
		omx_private->cntDecError = 0; // Changing resolution is not error!!
	}
#endif

	// stream discontinuity occurred, all ports were flushed, 
	// data in vpu buffer is invalid
	// so, should reset timestamp
	if (omx_private->bAllPortsFlushed == OMX_TRUE) {
		LOGI("[VDEC-%d] Reset timestamp", omx_private->pVideoDecodInstance.nVdec_Instance);
		omx_private->bAllPortsFlushed = OMX_FALSE;
		pInputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
		omx_private->isNewBuffer = 1;
	}

    /** Fill up the current input buffer when a new buffer has arrived */
	if(omx_private->isNewBuffer)
	{
		if( !(pInputBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) ) {
			if( omx_private->bWaitNewBuffer == OMX_TRUE ) {
				omx_private->bWaitNewBuffer = OMX_FALSE;
				pInputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
				pInputBuffer->nFlags |= OMX_BUFFERFLAG_IFRAME_ONLY;
			}

			//frame-rate update (AVG)
			if( omx_private->bUpdateFPS == OMX_TRUE ) {
				if( pInputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME ) {
					// reset
					omx_private->frameCount = 0;
					omx_private->prevTimestamp = 0;
				}
				else {
					OMX_TICKS prev_pts = omx_private->prevTimestamp;
					OMX_TICKS curr_pts = pInputBuffer->nTimeStamp;
					if( omx_private->frameCount == 0 ) {
						omx_private->prevTimestamp = curr_pts;
					}
					else {
						if( prev_pts == 0 ) {
							omx_private->prevTimestamp = curr_pts;
							omx_private->frameCount = 0;
						}
						else {
							if( prev_pts < curr_pts ) {
								// frame-rate update
								if ( omx_private->frameCount == 1 )
									omx_private->frameDuration = curr_pts - prev_pts;
								else {
									OMX_TICKS time_diff = curr_pts - prev_pts;
									OMX_TICKS frame_due = time_diff / (OMX_TICKS)omx_private->frameCount;
									OMX_S32 frame_rate = (1000000000 / (OMX_S32)frame_due);
									omx_private->frameDuration = frame_due;
									omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate = frame_rate;
									omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameRate = frame_rate;
								}
								omx_private->prevTimestamp = curr_pts;
								omx_private->frameCount = 0;
							} else if( prev_pts == curr_pts ) {
								// pts increase
								if( omx_private->frameDuration )
									pInputBuffer->nTimeStamp += omx_private->frameCount * omx_private->frameDuration;
								else if ( omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate ) {
									OMX_TICKS frame_due = (1000000000 / omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate);
									omx_private->frameDuration = frame_due;
									pInputBuffer->nTimeStamp += omx_private->frameCount * frame_due;
								}
							}
						}
					}
				}
			} //end of "omx_private->bUpdateFPS == OMX_TRUE"
		}

		omx_private->inputCurrBuffer = pInputBuffer->pBuffer;
		omx_private->inputCurrLength = pInputBuffer->nFilledLen;
		omx_private->isNewBuffer = 0;

		DPRINTF_DEC("[VDEC-%d] New Buffer -----> inputCurrLen:%d, PTS:%d, offset:%d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->inputCurrLength, pInputBuffer->nTimeStamp/1000, pInputBuffer->nOffset);
	}

	pOutputBuffer->nFilledLen = 0;
	pOutputBuffer->nOffset = 0;

#if 1//JS Baek
	if(pInputBuffer->nFlags & OMX_BUFFERFLAG_BFRAME_SKIP)
	{
		//Need to boost up so that decode with maximized speed.
		omx_private->i_skip_scheme_level = VDEC_SKIP_FRAME_ONLY_B;
		omx_private->bDecIndexOutput = OMX_FALSE;
		//LOGE("BMC : VDEC_SKIP_FRAME_ONLY_B");
		//vpu_update_sizeinfo(omx_private->gsVDecInit.m_iBitstreamFormat, omx_private->gsVDecUserInfo.bitrate_mbps, omx_private->gsVDecUserInfo.frame_rate, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT); //max-clock!!
	}
	else if(pInputBuffer->nFlags & OMX_BUFFERFLAG_IFRAME_ONLY)
	{
		//omx_private->I_frame_search_mode = AVC_NONIDR_PICTURE_SEARCH_MODE;
		//omx_private->i_skip_scheme_level = VDEC_SKIP_FRAME_EXCEPT_I;
		//LOGE("BMC : VDEC_SKIP_FRAME_EXCEPT_I");
		//To display decoded index directly
		omx_private->bDecIndexOutput = OMX_TRUE;
		//vpu_update_sizeinfo(omx_private->gsVDecInit.m_iBitstreamFormat, omx_private->gsVDecUserInfo.bitrate_mbps, omx_private->gsVDecUserInfo.frame_rate, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT); //max-clock!!
		//vpu_update_sizeinfo(omx_private->gsVDecInit.m_iBitstreamFormat, omx_private->gsVDecUserInfo.bitrate_mbps, omx_private->gsVDecUserInfo.frame_rate, omx_private->gsVDecOutput.m_pInitialInfo->m_iPicWidth, omx_private->gsVDecOutput.m_pInitialInfo->m_iPicHeight);
	}
	else
	{
	#ifdef SET_FRAMEBUFFER_INTO_MAX
		if(omx_private->bSet_Except_I_Frm == OMX_TRUE)
			omx_private->i_skip_scheme_level = VDEC_SKIP_FRAME_EXCEPT_I;
		else
	#endif
			omx_private->i_skip_scheme_level = VDEC_SKIP_FRAME_DISABLE;
		omx_private->bDecIndexOutput = OMX_FALSE;
	}
#endif

	while (!nOutputFilled) {

	    if (omx_private->isFirstBuffer) {
	        tsem_down(omx_private->avCodecSyncSem);
	        omx_private->isFirstBuffer = 0;
	    }

	    //////////////////////////////////////////////////////////////////////////////////////////
	    /* ZzaU :: remove NAL-Start Code when there are double codes. ex) AVI container */
        if ( omx_private->bSecuredInBufferMode == OMX_FALSE &&
			CONTAINER_TYPE_TS != omx_private->pVideoDecodInstance.container_type 
            && (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC || 
				omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC)) {

			OMX_U8* p = omx_private->inputCurrBuffer;
			if (omx_private->inputCurrLength > 8) {
				if (!memcmp("\x00\x00\x00\x01", p, 4)) {
					if (!memcmp("\x00\x00\x00\x01", p+4, 4)) {
						input_offset = 4;
						DBUG_MSG("[VDEC-%d] Double NAL-Start Code!!", omx_private->pVideoDecodInstance.nVdec_Instance);
					} else if (!memcmp("\x00\x00\x01", p+4, 3)) {
						input_offset = 3;
						DBUG_MSG("[VDEC-%d] remove 00 00 01 behind NAL-Start Code!!", omx_private->pVideoDecodInstance.nVdec_Instance);
						pInputBuffer->pBuffer[3] = 0x00;
					}
				}
			} else {
				LOGW("[VDEC-%d] WARNING !! video input length is less than 8 bytes.", omx_private->pVideoDecodInstance.nVdec_Instance);
				// fall through
			}
		}

	    if(pInputBuffer->nFlags & OMX_BUFFERFLAG_CODECCONFIG) {
			DBUG_MSG("[VDEC-%d] Config data IN!!", omx_private->pVideoDecodInstance.nVdec_Instance);

			omx_private->extractorType = (pInputBuffer->nFlags & OMX_BUFFERFLAG_EXTRACTOR_TYPE_FILTER);
			LOGI("[VDEC-%d] omx_private->extractorType = 0x%lx", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->extractorType);

			if(ExtractConfigData(omx_private, input_offset) == OMX_FALSE) {
				VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
				return;
			}

#ifdef RING_BUFFER_MODULE_ENABLE
			if( omx_private->pVpuRingPrivate == NULL && isSWCodec(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat) == OMX_FALSE )
			{
				char value[PROPERTY_VALUE_MAX];
				char value1[PROPERTY_VALUE_MAX];

				property_get("tcc.vpu.ringbuffer.enable", value, "1");

				if( atoi(value) && (omx_private->bWFD_mode == OMX_FALSE) )
				{
					property_get("tcc.vpu.rb.ignore.dmxtype", value, "0");
					property_get("tcc.vpu.rb.mvc.enable", value1, "1");

					if( ( (omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC && (omx_private->pVideoDecodInstance.container_type == CONTAINER_TYPE_MPG || omx_private->pVideoDecodInstance.container_type == CONTAINER_TYPE_TS)) || atoi(value) ) && 
						( omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat != STD_MVC || atoi(value1) ) ) 
					{
						if( omx_vpudec_component_InitVpuRingbufferCallback(openmaxStandComp, OMX_TRUE) == OMX_TRUE ) {
							omx_private->BufferMgmtCallback = omx_vpudec_component_BufferMgmtCallback;
							omx_vpudec_component_BufferMgmtCallback(openmaxStandComp, pInputBuffer, pOutputBuffer);
							return;
						}
					}
				}
			}
#endif

			omx_private->isNewBuffer = 1;
			pOutputBuffer->nFilledLen = 0;
			pInputBuffer->nFilledLen = 0;

			omx_private->bDetectFrameDelimiter = OMX_FALSE;
			omx_private->start_code_with_type = 0xFFFFFFFF;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
			ioctl(omx_private->mProxy_fd, TCC_LCDC_VIDEO_SET_FRAMERATE, omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate ) ;  // TCC_LCDC_HDMI_GET_DISPLAYED
#endif
			return;
		}

		if(!omx_private->pVideoDecodInstance.avcodecInited)
		{
#ifdef RESTORE_DECODE_ERR
			if(omx_private->cntDecError != 0){
				LOGE("[VDEC-%d] start to restore decode error count(%d)", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->cntDecError);
				if(!(pInputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)){
					LOGE("[VDEC-%d] to set forcingly SYNC_FRAME", omx_private->pVideoDecodInstance.nVdec_Instance);
					pInputBuffer->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
				}

				if(omx_private->seqHeader_len != 0){
					omx_private->need_sequence_header_attachment = OMX_TRUE;
				}
			}
			else
#endif
			if ((CONTAINER_TYPE_TS == omx_private->pVideoDecodInstance.container_type)
			/*|| (CONTAINER_TYPE_MPG == omx_private->pVideoDecodInstance.container_type)*/)
			{
				if (0 == omx_private->szConfigdata)
				{
					LOGINFO("[VDEC-%d] [BufMgmtCB] Call CODETYPE_HEADER", omx_private->pVideoDecodInstance.nVdec_Instance);
					omx_private->SearchCodeType(omx_private, &input_offset, CODETYPE_HEADER);
					if (input_offset >= omx_private->inputCurrLength)
					{
						omx_private->isNewBuffer = 1;
						pInputBuffer->nFilledLen = 0;
						LOGW("[VDEC-%d] [BufMgmtCB] header is not included in this input frame", omx_private->pVideoDecodInstance.nVdec_Instance);
						return;
					}

					omx_private->inputCurrBuffer += input_offset;
					omx_private->inputCurrLength -= input_offset;
					pInputBuffer->nFilledLen -= input_offset;
					input_offset = 0;
				}
			}

			omx_private->frameSearchOrSkip_flag = 0;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iPicWidth				= outPort->sPortParam.format.video.nFrameWidth;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iPicHeight			= outPort->sPortParam.format.video.nFrameHeight;
			omx_private->pVideoDecodInstance.gsVDecInit.m_bEnableVideoCache		= 0;//1;	// Richard_20100507 Don't use video cache
			omx_private->pVideoDecodInstance.gsVDecInit.m_bEnableUserData 		= 0;
			omx_private->pVideoDecodInstance.gsVDecInit.m_pExtraData			= omx_private->extradata;
			omx_private->pVideoDecodInstance.gsVDecInit.m_iExtraDataLen			= omx_private->extradata_size;

			omx_private->pVideoDecodInstance.gsVDecInit.m_bM4vDeblk 			= 0;//pCdk->m_bM4vDeblk;
			omx_private->pVideoDecodInstance.gsVDecInit.m_uiDecOptFlags			= 0;
			omx_private->pVideoDecodInstance.gsVDecInit.m_uiMaxResolution 		= 0;//pCdk->m_uiVideoMaxResolution;
			omx_private->pVideoDecodInstance.gsVDecInit.m_bFilePlayEnable 		= 1;

			omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode	= 0;

	#ifndef VPU_FRAME_DUMP
			if(omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr)
			{
				if(isSWCodec(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat)
		#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
					|| (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMJPEG)
		#endif
				)
					omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode	= 0;
				else
					omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode	= 1;

				LOGI("[VDEC-%d] Gralloc :: VPU Format = %d, HAL_PIXEL_FORMAT_YCbCr_420_SP = %d, size = %ld x %ld, YUVinter(%d)", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat,
		#ifdef USE_YUV420SP_DEFAULT
						1,
		#else
						omx_private->blocalPlaybackMode,
		#endif
						outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
						omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode);

			}
			else
			{
#ifdef OPENMAX1_2
				if( !omx_private->bThumbnailMode ) // video-editor(STB x) or MooPlayer
					omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode = 1;
#endif

				LOGI("[VDEC-%d] VPU Format = %d, size = %ld x %ld, YUVinter(%d)", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat,
							outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
							omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode);
			}
	#endif
			{
				omx_private->pVideoDecodInstance.dec_disp_info_input.m_iStdType 			= omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat;
				omx_private->pVideoDecodInstance.dec_disp_info_input.m_iTimeStampType 	= CDMX_PTS_MODE;	// Presentation Timestamp (Display order)
				if(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
				{
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFmtType 		= omx_private->pVideoDecodInstance.container_type;

					if(omx_private->pVideoDecodInstance.container_type == CONTAINER_TYPE_AVI || omx_private->pVideoDecodInstance.container_type == CONTAINER_TYPE_MP4)
					{
						DBUG_MSG("[VDEC-%d] TimeStampType = CDMX_DTS_MODE", omx_private->pVideoDecodInstance.nVdec_Instance);
						omx_private->pVideoDecodInstance.dec_disp_info_input.m_iTimeStampType = CDMX_DTS_MODE;	// Decode Timestamp (Decode order)
					}
					else
					{
						DBUG_MSG("[VDEC-%d] TimeStampType = CDMX_PTS_MODE", omx_private->pVideoDecodInstance.nVdec_Instance);
					}
				}
				else if(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_MPEG4)
				{
					DBUG_MSG("[VDEC-%d] This file comes from Android MP4 parser. TimeStampType = CDMX_DTS_MODE", omx_private->pVideoDecodInstance.nVdec_Instance);
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iTimeStampType = CDMX_PTS_MODE;
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFmtType			= CONTAINER_TYPE_MP4;
				}
				else
				{
					DBUG_MSG("[VDEC-%d] This file comes from Android proprietry or plug-in parser. extractorType(0x%08x)", omx_private->pVideoDecodInstance.nVdec_Instance, (unsigned int)omx_private->extractorType);
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iTimeStampType = CDMX_PTS_MODE;	// Decode Timestamp (Decode order)
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFmtType 		 = CONTAINER_TYPE_TS;
				}

				disp_pic_info ( CVDEC_DISP_INFO_INIT, (void*) &(omx_private->pVideoDecodInstance.dec_disp_info_ctrl), (void*)omx_private->pVideoDecodInstance.dec_disp_info,(void*) &(omx_private->pVideoDecodInstance.dec_disp_info_input), omx_private);
			}

			if(omx_private->seq_header_init_error_count == SEQ_HEADER_INIT_ERROR_COUNT)
			{

				if(omx_private->bThumbnailMode == OMX_FALSE)
				{
					int res_changed = 0;

	#ifdef VPU_BUFFER_MANAGEMENT
					res_changed = LASTFRAME_FOR_CODEC_CHANGE;
	#endif

	#ifdef SET_FRAMEBUFFER_INTO_MAX
					if(omx_private->bDetected_res_changed == OMX_TRUE)
					{
						property_get("tcc.video.vsync.enable", value, "0");
						property_get("tcc.video.hwr.id", value1, "0");
						LOGI("[VDEC-%d] Will wait %lld ~ %lld us to flush output-port before reconfiguration", omx_private->pVideoDecodInstance.nVdec_Instance, (omx_private->prev_interval * 2), (omx_private->prev_interval * 5));
						if(atoi(value) != 0 && atoi(value1) == omx_private->mCodecStart_ms)
							usleep(omx_private->prev_interval * 2);
						else
							usleep(omx_private->prev_interval * 5);

						res_changed = LASTFRAME_FOR_RESOLUTION_CHANGE;
					}
	#endif
					if(0 == vdec_is_rendered_index(omx_private->pVideoDecodInstance.pVdec_Instance)){
						if(ioctl(omx_private->mProxy_fd, TCC_LCDC_HDMI_LASTFRAME,&res_changed) < 0)
						{
	#ifdef SET_FRAMEBUFFER_INTO_MAX
							if(omx_private->bDetected_res_changed == OMX_TRUE && atoi(value) != 0)
								usleep(omx_private->prev_interval * 3);
	#endif
						}
					}
				}
				omx_private->pVideoDecodInstance.gsVDecUserInfo.bMaxfbMode = 0;//omx_private->bDetected_res_changed;
				omx_private->pVideoDecodInstance.gsVDecUserInfo.extFunction = 0x0;
				if( omx_private->bSecuredInBufferMode == OMX_TRUE )
					omx_private->pVideoDecodInstance.gsVDecUserInfo.extFunction |= EXT_FUNC_LOCK_FOR_SINK;
				if( omx_private->bWFDHDCP2SrcEnable == OMX_TRUE )
					omx_private->pVideoDecodInstance.gsVDecUserInfo.extFunction |= EXT_FUNC_LOCK_FOR_SOURCE;
				if(omx_private->bWFD_mode == OMX_TRUE || omx_private->bNoBufferDelay == OMX_TRUE )
					omx_private->pVideoDecodInstance.gsVDecUserInfo.extFunction |= EXT_FUNC_NO_BUFFER_DELAY;

				if( (ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_INIT, NULL, &(omx_private->pVideoDecodInstance.gsVDecInit), &(omx_private->pVideoDecodInstance.gsVDecUserInfo), (omx_private->pVideoDecodInstance.pVdec_Instance))) < 0 )
				{
					LOGE( "[VDEC-%d] [VDEC_INIT] [Err:%ld] video decoder init", omx_private->pVideoDecodInstance.nVdec_Instance, ret );

					if(ret != -VPU_ENV_INIT_ERROR) //to close vpu!!
						omx_private->pVideoDecodInstance.isVPUClosed = OMX_FALSE;

					VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
					return;
				}

				//if(omx_private->bDetected_res_changed != OMX_TRUE){
					omx_private->mCodecStart_ms = (OMX_U32)(systemTime(SYSTEM_TIME_MONOTONIC)/1000000);
					ALOGI("[VDEC-%d] mCodecStart_ms = %ld", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->mCodecStart_ms);
				//}

				vpu_update_sizeinfo(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, omx_private->pVideoDecodInstance.gsVDecUserInfo.bitrate_mbps,
									omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate, omx_private->pVideoDecodInstance.gsVDecInit.m_iPicWidth, omx_private->pVideoDecodInstance.gsVDecInit.m_iPicHeight, omx_private->pVideoDecodInstance.pVdec_Instance);
				omx_private->pVideoDecodInstance.isVPUClosed = OMX_FALSE;

				init_buffer_id_for_kernel(omx_private);

			}

			if(omx_private->pVideoDecodInstance.isVPUClosed == OMX_TRUE)// Following codes should not be worked under vpu-closed status.
			{
				LOGE( "[VDEC-%d] Now VPU has been closed , return ", omx_private->pVideoDecodInstance.nVdec_Instance );
				VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
				return;
			}

			if(omx_private->szConfigdata != 0)
			{
				if (CONTAINER_TYPE_TS == omx_private->pVideoDecodInstance.container_type)
				{
					LOGE("[VDEC-%d] BufMgmtCB: TS fileformat stream can't use szConfigdata !!", omx_private->pVideoDecodInstance.nVdec_Instance);
					VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
					return;
				}

				omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = omx_private->pConfigdata;
				omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen = omx_private->szConfigdata;
			}
			else
			{
                omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = omx_private->inputCurrBuffer + input_offset;
				omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen	= omx_private->inputCurrLength - input_offset;
			}

			omx_private->pVideoDecodInstance.gsVDecInput.m_iIsThumbnail = omx_private->bThumbnailMode;

#ifdef RESTORE_DECODE_ERR
			if(omx_private->cntDecError != 0 && omx_private->seqHeader_len != 0)
			{
				LOGI("[VDEC-%d] Use backup seq_header(%ld) data to restore decode", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->seqHeader_len);
				omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
				omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);
				omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen	= omx_private->seqHeader_len;

	#if defined(TC_SECURE_MEMORY_COPY)
				if( omx_private->bWFDHDCP2SrcEnable )
				{
					if( omx_private->mSeqBackupPmap.size != 0 )
					{
						//memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->mSeqBackupMapInfo, omx_private->seqHeader_len);
						ret = TC_SecureMemoryCopy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA], omx_private->mSeqBackupPmap.base, omx_private->seqHeader_len);
						//ALOGI("[SinB] End(%d)", ret);
					}
					else
					{
						omx_private->seqHeader_len = 0;
						ALOGE("[VDEC-%d] mSeqBackupPmap.size is 0.", omx_private->pVideoDecodInstance.nVdec_Instance);
					}
				}
				else
	#endif
				{
					memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->seqHeader_backup, omx_private->seqHeader_len);
				}
				bUse_backup_seqHeader = OMX_TRUE;
			}
#endif

#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
			if( bUse_backup_seqHeader == OMX_FALSE && omx_private->bSecuredInBufferMode == OMX_FALSE &&
				(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC ||
					omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC ||
					omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4))
			{
				OMX_BOOL bFound_frame = OMX_FALSE;
				DBUG_MSG("[VDEC-%d] sequence header: 0x%x - %ld bytes, frame: 0x%x - %d bytes", omx_private->pVideoDecodInstance.nVdec_Instance, (unsigned int)omx_private->sequence_header_only, omx_private->sequence_header_size,
											(unsigned int)omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);

				{
					unsigned char* ps = (unsigned char*)omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA];
					DBUG_MSG( "[VDEC-%d In %d] \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"
										"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"
										"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", 
										omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen,
										ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
										ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
										ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
										ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63],
										ps[64], ps[65], ps[66], ps[67], ps[68], ps[69], ps[70], ps[71], ps[72], ps[73], ps[74], ps[75], ps[76], ps[77], ps[78], ps[79]);
				}
				if(0 >= extract_seqheader((const unsigned char*)omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen,
													(unsigned char**)&(omx_private->sequence_header_only), (long*)&(omx_private->sequence_header_size),
													omx_private->pVideoDecodInstance.video_coding_type, openmaxStandComp))
				{
					bFound_frame = OMX_FALSE;

					if( (omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] == omx_private->pConfigdata) && omx_private->sequence_header_size > 0)
					{
						unsigned char *input_addr 	= omx_private->inputCurrBuffer + input_offset;
						unsigned int input_size 	= omx_private->inputCurrLength - input_offset;

						DBUG_MSG("[VDEC-%d] Retry :: sequence header extraction (%ld) after checking Configdata", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->sequence_header_size);
						if(0 < extract_seqheader((const unsigned char*)input_addr, input_size,
															(unsigned char**)&(omx_private->sequence_header_only), (long*)&(omx_private->sequence_header_size),
															omx_private->pVideoDecodInstance.video_coding_type, openmaxStandComp))
						{
							bFound_frame = OMX_TRUE;
							omx_private->need_sequence_header_attachment = OMX_TRUE;
							omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
							omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);

							memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->sequence_header_only, omx_private->sequence_header_size);
							memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] + omx_private->sequence_header_size, input_addr, input_size);
							omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen	+= input_size;
						}
					}

					if(!bFound_frame)
					{
						if( omx_private->sequence_header_size > 0 ) {
							omx_private->need_sequence_header_attachment = OMX_TRUE;
						}

						LOGE( "[VDEC-%d] [%ld'th frame with only sequence frame (%ld bytes / %ld bytes)] VPU want sequence_header frame with sync frame!", omx_private->pVideoDecodInstance.nVdec_Instance, SEQ_HEADER_INIT_ERROR_COUNT - omx_private->seq_header_init_error_count,
															omx_private->sequence_header_size, omx_private->inputCurrLength);

						if(--omx_private->seq_header_init_error_count <= 0)
						{
							LOGE( "[VDEC-%d] This contents can't decode because there are no sequence frame.", omx_private->pVideoDecodInstance.nVdec_Instance);
							VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
							return;
						}
						omx_private->isNewBuffer = 1;
						pInputBuffer->nFilledLen = 0;
						pOutputBuffer->nFilledLen = 0;

						return;
					}
				}
				else
				{
					if(omx_private->need_sequence_header_attachment
		#ifdef SET_FRAMEBUFFER_INTO_MAX
						&& (omx_private->bDetected_res_changed == OMX_FALSE)
		#endif
					)
					{
						unsigned char *temp_addr = (unsigned char *)omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA];
						omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
						omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);

						memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->sequence_header_only, omx_private->sequence_header_size);

						if(temp_addr == omx_private->pConfigdata)
						{
							memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] + omx_private->sequence_header_size, omx_private->inputCurrBuffer + input_offset, omx_private->inputCurrLength - input_offset);
							omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen	+= (omx_private->inputCurrLength - input_offset);
						}
						else
						{
							memcpy(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA]+ omx_private->sequence_header_size, temp_addr, omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
							omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen	+= omx_private->sequence_header_size;
						}
					}
					else
					{
						LOGI("[VDEC-%d] Success at one time :: Input frame has sequence(%ld)/%ld + sync frame", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->sequence_header_size, omx_private->inputCurrLength - input_offset);
					}
				}
			}
#endif

#if defined(TCC_VPU_C7_INCLUDE)
			if( bUse_backup_seqHeader == OMX_FALSE && omx_private->bSecuredInBufferMode == OMX_FALSE && omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingVP8)
			{
				if(0 >= check_startcode_for_VP8(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen, 1))
				{
					unsigned char *InStreamBuff;
					unsigned int InStreamSize;

					InStreamBuff = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);

					get_startcode_for_VP8(1, outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
											omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen, (unsigned char*)InStreamBuff, (int*)&InStreamSize);

					memcpy(InStreamBuff+InStreamSize, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
					omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
					omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);
					omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen += InStreamSize;
				}
			}
#endif

			omx_private->pVideoDecodInstance.video_dec_idx = 0;

			#if defined(MVC_PROCESS) && defined(TCC_VPU_C7_INCLUDE)
			omx_private->bMVC_mode = OMX_FALSE;
			
			if( omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat==STD_MVC)
			{
				property_get("tcc.solution.mbox", value, "0");
				pmap_t videoPmap;
				//we have to check video pamp size , in case if 1G ram, video memory is 96M(0x06000000) and if MVC, memory is more used.
				if(atoi(value) && pmap_get_info("video", &videoPmap) && videoPmap.size > 0x07000000 )
				{
					omx_private->bMVC_mode=OMX_TRUE;
				}
			}
			
			if(omx_private->bMVC_mode==OMX_TRUE){
				if(outPort->sPortParam.format.video.nFrameHeight==720)
					omx_private->max_fifo_cnt = outPort->sPortParam.nBufferCountActual+4;
				else
					omx_private->max_fifo_cnt = outPort->sPortParam.nBufferCountActual+6;
				
				LOGI( "[VDEC-%d] MVC max_fifo_cnt %ld !", omx_private->pVideoDecodInstance.nVdec_Instance,	omx_private->max_fifo_cnt );
			}
			else
				omx_private->max_fifo_cnt = outPort->sPortParam.nBufferCountActual;
			
			#else
				omx_private->max_fifo_cnt = outPort->sPortParam.nBufferCountActual;
			#endif

#ifdef VPU_BUFFER_MANAGEMENT
			vpu_set_additional_refframe_count(omx_private->max_fifo_cnt+1, omx_private->pVideoDecodInstance.pVdec_Instance);
#else
			vpu_set_additional_refframe_count(omx_private->max_fifo_cnt, omx_private->pVideoDecodInstance.pVdec_Instance);
#endif

			Secured_inBuffer_Process(openmaxStandComp, pInputBuffer, OMX_TRUE);

			if( (ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_DEC_SEQ_HEADER, NULL, &omx_private->pVideoDecodInstance.gsVDecInput, &omx_private->pVideoDecodInstance.gsVDecOutput, (omx_private->pVideoDecodInstance.pVdec_Instance) )) < 0 )
			{
				if ( (--omx_private->seq_header_init_error_count <= 0) || (ret == -VPU_NOT_ENOUGH_MEM) ||
					 (ret == -RETCODE_INVALID_STRIDE) || (ret == -RETCODE_CODEC_SPECOUT) || (ret == -RETCODE_CODEC_EXIT) || (ret == -RETCODE_MULTI_CODEC_EXIT_TIMEOUT))
				{
					LOGE( "[[VDEC-%d] VDEC_DEC_SEQ_HEADER] [Err:%ld]", omx_private->pVideoDecodInstance.nVdec_Instance, ret );
					VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
					return;
				}
				else
				{
					DBUG_MSG("[VDEC-%d] skip seq header frame, data len %d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
					LOGI( "[VDEC-%d] [VDEC_DEC_SEQ_HEADER] retry %ld using next frame!", omx_private->pVideoDecodInstance.nVdec_Instance,  SEQ_HEADER_INIT_ERROR_COUNT - omx_private->seq_header_init_error_count);

					omx_private->isNewBuffer = 1;
					pInputBuffer->nFilledLen = 0;
					pOutputBuffer->nFilledLen = 0;

					return;
				}
			}
#ifdef RESTORE_DECODE_ERR
			else
			{
				if( omx_private->bSecuredInBufferMode == OMX_FALSE && omx_private->seqHeader_len == 0)
				{
	#if defined(TC_SECURE_MEMORY_COPY)
					if( omx_private->bWFDHDCP2SrcEnable )
					{
						if( omx_private->mSeqBackupPmap.size != 0 )
						{
							//memcpy(omx_private->mSeqBackupMapInfo, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
							ret = TC_SecureMemoryCopy(omx_private->mSeqBackupPmap.base, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
							//ALOGI("[SinB] End(%d)", ret);
							omx_private->seqHeader_len = omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen;
						}
						else
						{
							omx_private->seqHeader_len = 0;
							ALOGE("[VDEC-%d] mSeqBackupPmap.size is 0.", omx_private->pVideoDecodInstance.nVdec_Instance);
						}
					}
					else
	#endif
					{
						omx_private->seqHeader_backup = TCC_calloc(1,omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
						memcpy(omx_private->seqHeader_backup, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
						omx_private->seqHeader_len = omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen;
					}

					omx_private->cntDecError = 0;

					LOGI("[VDEC-%d] backup seq_header(%ld) data to restore decode", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->seqHeader_len);
				}
				LOGI("[VDEC-%d] success seq_header(%ld)", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->seqHeader_len);
				bJust_after_sequence_done = OMX_TRUE;
			}
#endif

			omx_private->pVideoDecodInstance.avcodecInited = 1;

			//frame-rate update (AVG)
			omx_private->frameCount = 0;
			omx_private->prevTimestamp = 0;
			omx_private->frameDuration = 0;
			omx_private->bUpdateFPS = OMX_FALSE;

			if (omx_private->pVideoDecodInstance.container_type == CONTAINER_TYPE_TS)
			{
				if (omx_private->bDelayedDecodeOut == OMX_FALSE) 
				{
					omx_private->code_type = CODETYPE_NONE;

					//frame-rate update (AVG)
					if (omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate != 0)
					{
						omx_private->frameDuration = (OMX_TICKS)1000000000 / omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_iFrameRate;
						omx_private->bUpdateFPS = OMX_TRUE;
					} 
					else
					{
						omx_private->bUpdateFPS = OMX_FALSE;
					}
				}
			}

			// set the flag to disable further processing until Client reacts to this by doing dynamic port reconfiguration
			ret = isPortChange(openmaxStandComp);
			if(ret < 0) {//max-resolution over!!
				VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
				return;
			}

#ifdef SET_FRAMEBUFFER_INTO_MAX
			if(omx_private->bDetected_res_changed == OMX_TRUE){
				omx_private->bDetected_res_changed = omx_private->bNoOutput_after_res_changed = OMX_FALSE;
			}
#endif
			if(ret == 1) //port reconfiguration!!
				return;
		}

		if(pInputBuffer->nFlags & OMX_BUFFERFLAG_SYNCFRAME)
		{
			pInputBuffer->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

			//reset delayed output buffer count
			omx_private->nDelayedOutputBufferCount = 0;

			//reset base timestamp.
			omx_private->prev_timestamp = pInputBuffer->nTimeStamp;

			// reset MVC base-view address
			omx_private->MVC_Base_addr0 = (OMX_U32)NULL;
			omx_private->MVC_Base_addr1 = (OMX_U32)NULL;
			omx_private->MVC_Base_addr2 = (OMX_U32)NULL;

			// reset gralloc copy state
#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
			omx_private->gralloc_info.m_pDispOut[PA][0] = omx_private->gralloc_info.m_pDispOut[PA][1] = omx_private->gralloc_info.m_pDispOut[PA][2] = NULL;
			omx_private->gralloc_info.m_pDispOut[VA][0] = omx_private->gralloc_info.m_pDispOut[VA][1] = omx_private->gralloc_info.m_pDispOut[VA][2] = NULL;
			omx_private->gralloc_info.mDispIndex = -10;
			omx_private->gralloc_info.nTimestamp = 0;
#endif

			//AVG: Some problems have found in special conditions. (temporary disabled)
			if( omx_private->isFirstSyncFrame == OMX_FALSE ) {
				omx_private->bWaitKeyFrameOut = OMX_TRUE;
			}

#ifdef ENABLE_DECODE_ONLY_MODE_AVC
			//if(omx_private->bUseDecodeOnlyMode == OMX_TRUE &&
			//   omx_private->bDecIndexOutput == OMX_FALSE )
			if(omx_private->bUseDecodeOnlyMode == OMX_TRUE )
			{
				/* Playback-start should not be affected by decode only mode. */
				if(omx_private->isFirstSyncFrame == OMX_FALSE)
				{
					omx_private->bSetDecodeOnlyMode = OMX_TRUE;
					omx_private->skipFrameNum = 0;
					omx_private->decodeOnlyErrNum = 0;
				}
			}
#endif

			DPRINTF_DEC_STATUS("[VDEC-%d] [SEEK] I-frame Search Mode enable", omx_private->pVideoDecodInstance.nVdec_Instance);
			omx_private->ConsecutiveVdecFailCnt = 0; //Reset Consecutive Vdec Fail Counting B060955
			omx_private->frameSearchOrSkip_flag = 1;

			if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC || omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC)
			{
#ifdef ENABLE_DECODE_ONLY_MODE_AVC
				if(omx_private->pVideoDecodInstance.container_type == CONTAINER_TYPE_TS
				|| omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_MPEG2TS
				|| omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_RTSP
				)
				{
					if(omx_private->isFirstSyncFrame == OMX_TRUE)
					{
						omx_private->isFirstSyncFrame = OMX_FALSE;
					}
					else
					{
						omx_private->I_frame_search_mode = AVC_IDR_PICTURE_SEARCH_MODE;
						omx_private->IDR_frame_search_count = 0;

						//if( omx_private->bDecIndexOutput == OMX_TRUE )
						//	omx_private->I_frame_search_mode = AVC_NONIDR_PICTURE_SEARCH_MODE;
					}
				}
				else
#endif
				if(!(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC))
				{
					//temporarily disable because this line causes to fail compatibility test.
					//omx_private->frameSearchOrSkip_flag = 0;
				}
			}

			disp_pic_info( CVDEC_DISP_INFO_RESET, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_ctrl),
							(void*)omx_private->pVideoDecodInstance.dec_disp_info,(void*)&(omx_private->pVideoDecodInstance.dec_disp_info_input),
							omx_private);

#ifdef HAVE_ANDROID_OS
			/*ZzaU :: Clear all decoded frame-buffer!!*/
			//if(omx_private->max_fifo_cnt != 0 && omx_private->bDecIndexOutput == OMX_FALSE)
			if(omx_private->max_fifo_cnt != 0)
			{
				if(0 > clear_all_displayedIndex(openmaxStandComp, pInputBuffer, pOutputBuffer, OMX_TRUE))
					return;
			}
#endif
			if( omx_private->isFirstSyncFrame == OMX_TRUE )
				omx_private->isFirstSyncFrame = OMX_FALSE;
		}

#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
        if(omx_private->need_sequence_header_attachment)
        {
            omx_private->need_sequence_header_attachment = OMX_FALSE;
        }
        else
#endif
        {
            omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = omx_private->inputCurrBuffer + input_offset;
            omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen  = omx_private->inputCurrLength - input_offset;
        }

		omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameNum = 0;
		omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable = 0;

		switch(omx_private->i_skip_scheme_level)
		{
			case VDEC_SKIP_FRAME_DISABLE:
			case VDEC_SKIP_FRAME_EXCEPT_I:
				omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = omx_private->i_skip_scheme_level;
				break;
			case VDEC_SKIP_FRAME_ONLY_B:
				if(omx_private->i_skip_count == omx_private->i_skip_interval)
				{
					omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = omx_private->i_skip_scheme_level;
					omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameNum = 1000;
				}
				else
				{
					omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
				}
				break;
			default:
					omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
				break;
		}

		if(omx_private->frameSearchOrSkip_flag == 1 )
		{
			omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameNum = 1;
			omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable = omx_private->I_frame_search_mode;
			omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;

			if( omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable == AVC_IDR_PICTURE_SEARCH_MODE )
			{
				DPRINTF_DEC_STATUS( "[VDEC-%d] [SEEK] I-frame Search Mode(IDR-picture for H.264) Enable!!!", omx_private->pVideoDecodInstance.nVdec_Instance);
				if(++omx_private->IDR_frame_search_count >= AVC_IDR_PICTURE_SEARCH_COUNT)
				{
					LOGMSG("[VDEC-%d] [Seek] Change I-frame search mode : IDR -> non-IDR", omx_private->pVideoDecodInstance.nVdec_Instance);
					omx_private->I_frame_search_mode = AVC_NONIDR_PICTURE_SEARCH_MODE;
					omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable = AVC_NONIDR_PICTURE_SEARCH_MODE;
					omx_private->skipFrameNum = 0;
				}
			}
			else if( omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable == AVC_NONIDR_PICTURE_SEARCH_MODE )
			{
				DPRINTF_DEC_STATUS( "[VDEC-%d] [SEEK] I-frame Search Mode(I-slice for H.264) Enable!!!", omx_private->pVideoDecodInstance.nVdec_Instance);
			}
		}
		else if( omx_private->frameSearchOrSkip_flag == 2 )
		{
			omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameNum = 1;
			omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable = 0;
			omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_ONLY_B;
			DPRINTF_DEC_STATUS("[VDEC-%d] [SEEK] B-frame Skip Mode Enable!!!", omx_private->pVideoDecodInstance.nVdec_Instance);
		}

// FRAME_SKIP_MODE
		if( omx_private->frameSearchOrSkip_flag == 0 ) // video frame skip
		{
			char value[PROPERTY_VALUE_MAX];
			property_get("tcc.video.skip.flag", value, "");
			if (strcmp(value, "1") == 0)
			{
				omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameNum = 1;
				omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable = 0;
				omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_ONLY_B;	//Fix me...
				LOGE("[VDEC-%d] [SEEK] B-frame Skip Mode Enable!!! - Frame late!!! \n", omx_private->pVideoDecodInstance.nVdec_Instance);

				property_set("tcc.video.skip.flag", "0");
			}
		}
		if(omx_private->pVideoDecodInstance.isVPUClosed == OMX_TRUE)
		{
			LOGE( "[VDEC-%d] Now VPU has been closed , return ", omx_private->pVideoDecodInstance.nVdec_Instance );
			VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, 0);
			return;
		}
// FRAME_SKIP_MODE
#ifdef COMPARE_TIME_LOG
		start = clock();
#endif

#if defined(ANDROID_USE_GRALLOC_BUFFER)
	#if defined(MOVE_HW_OPERATION)
		gralloc_copy_mode = COPY_NONE;
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
		if (omx_private->BufferType == DecoderMetadataPtr) {
			VideoDecoderOutputMetaData* metaBuffer = (VideoDecoderOutputMetaData*)pOutputBuffer->pBuffer;
			grallocHandle = metaBuffer->pHandle;
		} else
#endif
			grallocHandle = (buffer_handle_t*)pOutputBuffer->pBuffer;


		if(omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr)
		{
			if(omx_private->gralloc_info.m_pDispOut[PA][0] != NULL && omx_private->gralloc_info.m_pDispOut[PA][1] != NULL)
			{
				omx_private->gralloc_info.grallocModule->lock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule,
													(buffer_handle_t)grallocHandle, GRALLOC_USAGE_HW_RENDER,
													0,0,outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight, &pGrallocPureAddr);

				pGrallocAddr = (OMX_U8 *)pGrallocPureAddr;

				bHWaddr = check_valid_hw_availble(openmaxStandComp, (unsigned int)pGrallocAddr);
				bCopyToGrallocBuff = check_copy_toGralloc(openmaxStandComp, bHWaddr);

				if(!bHWaddr){
					GBUG_MSG("[VDEC-%d] **************** No h/w address", omx_private->pVideoDecodInstance.nVdec_Instance);
					omx_private->gralloc_info.grallocModule->unlock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule, (buffer_handle_t)grallocHandle);
					omx_private->gralloc_info.grallocModule->lock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule,
													(buffer_handle_t)grallocHandle, (GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK),
													0,0,outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight, &pGrallocPureAddr);
					pGrallocAddr = (OMX_U8 *)pGrallocPureAddr;
				}

				GBUG_MSG("[VDEC-%d] handle: %p - addr: %p, 0x%x", omx_private->pVideoDecodInstance.nVdec_Instance, grallocHandle, (void*)pGrallocAddr, (int)pGrallocAddr);
				plat_priv = (TCC_PLATFORM_PRIVATE_PMEM_INFO *)get_private_addr(openmaxStandComp, bHWaddr, (unsigned int)pGrallocAddr, outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight);

				if( bCopyToGrallocBuff )
				{
					GBUG_MSG("[VDEC-%d] G copy", omx_private->pVideoDecodInstance.nVdec_Instance);
					if(0 <= copy_data_to_grallocbuffer(openmaxStandComp, bHWaddr, outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
												omx_private->gralloc_info.m_pDispOut[PA][0], omx_private->gralloc_info.m_pDispOut[PA][1],
												omx_private->gralloc_info.m_pDispOut[PA][2], omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode, pGrallocAddr, NULL, NULL, SEND_CMD, OMX_FALSE))
					{
						gralloc_copy_mode = COPY_START;
					}
					else
					{
						gralloc_copy_mode = COPY_FAILED;
						omx_private->gralloc_info.grallocModule->unlock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule, (buffer_handle_t)grallocHandle);
					}
				}
				else
				{
		#ifdef VPU_FRAME_DUMP //for frame dump.
					if(total_frm != 0){
						if(total_frm == 100){
							FILE *pFs;
							unsigned int Y, U, V;
							unsigned int stride;

							stride = ALIGNED_BUFF(outPort->sPortParam.format.video.nFrameWidth, 16);

							Y = ((unsigned int)vpu_getFrameBufVirtAddr(omx_private->gralloc_info.m_pDispOut[PA][0], PA, omx_private->pVideoDecodInstance.pVdec_Instance));
							U = ((unsigned int)vpu_getFrameBufVirtAddr(omx_private->gralloc_info.m_pDispOut[PA][1], PA, omx_private->pVideoDecodInstance.pVdec_Instance));
							V = ((unsigned int)vpu_getFrameBufVirtAddr(omx_private->gralloc_info.m_pDispOut[PA][2], PA, omx_private->pVideoDecodInstance.pVdec_Instance));

							pFs = fopen("/data/vFrame.yuv", "ab+");
							if (!pFs) {
								LOGE("Cannot open '/data/vFrame.yuv'");
							}
							else
							{
								if(pFs){
									fwrite( Y, stride*outPort->sPortParam.format.video.nFrameHeight, 1, pFs);
									fwrite( U, stride*outPort->sPortParam.format.video.nFrameHeight/4, 1, pFs);
									fwrite( V, stride*outPort->sPortParam.format.video.nFrameHeight/4, 1, pFs);
								}
								fclose(pFs);
							}
						}
					}
		#endif
				}
			}
		}
		else
		{
			if( pOutputBuffer->pOutputPortPrivate != NULL )
				plat_priv = (TCC_PLATFORM_PRIVATE_PMEM_INFO *)pOutputBuffer->pOutputPortPrivate;
			else
			{
				if( !omx_private->bThumbnailMode && omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr )
					plat_priv = &privOutData;
			}
		}
	#else
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
		if (omx_private->BufferType == DecoderMetadataPtr) {
			VideoDecoderOutputMetaData* metaBuffer = (VideoDecoderOutputMetaData*)pOutputBuffer->pBuffer;
			grallocHandle = &metaBuffer->pHandle;
		} else
#endif
			grallocHandle = (buffer_handle_t*)pOutputBuffer->pBuffer;

		gralloc_copy_mode = COPY_FAILED;
	#endif
#endif

#if defined(TCC_VPU_C7_INCLUDE)
		if( omx_private->bSecuredInBufferMode == OMX_FALSE && omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingVP8 )
		{
			if(0 >= check_startcode_for_VP8(omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen, 0))
			{
				unsigned char *InStreamBuff;
				unsigned int InStreamSize;

				InStreamBuff = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);

				get_startcode_for_VP8((omx_private->pVideoDecodInstance.video_dec_idx == 0)?1:0, outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
										omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen, (unsigned char*)InStreamBuff, (int*)&InStreamSize);

				memcpy(InStreamBuff+InStreamSize, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
				omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA] = vpu_getBitstreamBufAddr(PA, omx_private->pVideoDecodInstance.pVdec_Instance);
				omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA] = vpu_getBitstreamBufAddr(VA, omx_private->pVideoDecodInstance.pVdec_Instance);
				omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen += InStreamSize;
			}
		}
#endif

		Secured_inBuffer_Process(openmaxStandComp, pInputBuffer, OMX_FALSE);

		//LOGE("Dec Input %d(%d) ", pInputBuffer->nFilledLen, omx_private->gsVDecInput.m_iInpLen);
		if(omx_private->bDetected_res_changed == OMX_TRUE)
			omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen = 0;

		if( pInputBuffer->nFlags & OMX_BUFFERFLAG_DO_PEND_EOF ) {
			if( pInputBuffer->nFilledLen == 0 && omx_private->nDelayedOutputBufferCount == 1 && (omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr) ) {
				omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;
				omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx = -2;
				omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_SUCCESS;
				omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx = omx_private->gralloc_info.mDispIndex;
				goto skip_decoding_process;
			}
		}

		if( (ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_DECODE, NULL, &omx_private->pVideoDecodInstance.gsVDecInput, &omx_private->pVideoDecodInstance.gsVDecOutput, omx_private->pVideoDecodInstance.pVdec_Instance)) < 0 )
		{
			if(omx_private->bDetected_res_changed == OMX_TRUE)
			{
				omx_private->bNoOutput_after_res_changed = OMX_TRUE;
				return;
			}

			LOGE( "[VDEC-%d] [VDEC_DECODE] [Err:%ld (%d)] video decode", omx_private->pVideoDecodInstance.nVdec_Instance, ret, -RETCODE_CODEC_EXIT );
			VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
			if(gralloc_copy_mode == COPY_START)
				omx_private->gralloc_info.grallocModule->unlock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule, (buffer_handle_t)grallocHandle);
#endif
			return;
		}

skip_decoding_process:

#ifdef RESTORE_DECODE_ERR
		omx_private->cntDecError = 0;
#endif
		//LOGE("Decoded out Idx(%d/%d-0x%x)", omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx,
		//					omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][0]);

#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
		if(gralloc_copy_mode == COPY_START)
		{
			if(0 <= copy_data_to_grallocbuffer(openmaxStandComp, bHWaddr, outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight,
											omx_private->gralloc_info.m_pDispOut[PA][0], omx_private->gralloc_info.m_pDispOut[PA][1],
											omx_private->gralloc_info.m_pDispOut[PA][2], omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode, pGrallocAddr, NULL, NULL, WAIT_RESPOND, OMX_FALSE))
			{
				gralloc_copy_mode = COPY_DONE;
				output_len = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3 / 2;
			}
			else
			{
				gralloc_copy_mode = COPY_FAILED;
			}

			GBUG_MSG("[VDEC-%d] gralloc unlock :: mode = %d, len = %ld", omx_private->pVideoDecodInstance.nVdec_Instance, gralloc_copy_mode, output_len);
			omx_private->gralloc_info.grallocModule->unlock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule, (buffer_handle_t)grallocHandle);
		}
#endif

		if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL)
		{
			// Current input stream should be used next time.
			if(omx_private->ConsecutiveBufferFullCnt++ > MAX_CONSECUTIVE_VPU_BUFFER_FULL_COUNT) {
				LOGE("[VDEC-%d] VPU_DEC_BUF_FULL", omx_private->pVideoDecodInstance.nVdec_Instance);
				omx_private->ConsecutiveBufferFullCnt = 0;
				VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, -RETCODE_CODEC_EXIT);
				return;
			}

			if (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS) {
				decode_result = 0; // display Index : processed.
				omx_private->bDelayedDecodeOut = OMX_TRUE;
			}
			else
			{
				decode_result = 1; // display Index : not processsed.
				omx_private->bDelayedDecodeOut = OMX_FALSE;
			}
		}
		else
		{
			omx_private->bDelayedDecodeOut = OMX_FALSE;
			omx_private->ConsecutiveBufferFullCnt = 0;

			if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
				decode_result = 2; // display Index : proceed.
			else
				decode_result = 3; // display Index : not processsed.
		}

		switch(omx_private->i_skip_scheme_level)
		{
			case VDEC_SKIP_FRAME_DISABLE:
				break;
			case VDEC_SKIP_FRAME_EXCEPT_I:
			case VDEC_SKIP_FRAME_ONLY_B:
				if((omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode != omx_private->i_skip_scheme_level) || (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx == -2))
					omx_private->i_skip_count--;

				if(omx_private->i_skip_count < 0)
					omx_private->i_skip_count = omx_private->i_skip_interval;
				break;
		}

		// resolution change detection
		//if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS ||
		//	omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == 6
		//)
		if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_AVC ||
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MPEG2 ||
			omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_RV)
		{
			OMX_U32 width, height;

			width = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iWidth;
			height = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iHeight;

			if( width > 0 && height > 0 && (width*height < 2048*2048) )
			{
				if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_AVC || omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MVC)
				{
					width -= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft;
					width -= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight;
					height -= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom;
					height -= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop;
				}

				if(outPort->sPortParam.format.video.nFrameWidth != width || outPort->sPortParam.format.video.nFrameHeight != height)
				{
#if 1 // ZzaU :: to play continuously when resolution is changed during playback.
					if( omx_private->curr_width != width || omx_private->curr_height != height)
					{
						omx_private->curr_width = width;
						omx_private->curr_height = height;


		#ifdef SET_FRAMEBUFFER_INTO_MAX
						if(bJust_after_sequence_done == OMX_FALSE)
						{
							LOGI("[VDEC-%d] %d - %d - %d, %d - %d - %d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iWidth, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropLeft, 
											omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropRight, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iHeight, 
											omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropTop, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_CropInfo.m_iCropBottom);
							LOGI("[VDEC-%d] DS %d, Idx(%d/%d), Resolution change detected In(%ld) (%ldx%ld ==> %ldx%ld)", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus, 
											omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx,
											pInputBuffer->nFilledLen, outPort->sPortParam.format.video.nFrameWidth, outPort->sPortParam.format.video.nFrameHeight, width, height);

							total_frm = 0;
							omx_private->bDetected_res_changed = OMX_TRUE;
							omx_private->bNoOutput_after_res_changed = OMX_FALSE;
							omx_private->bCheck_same_index = OMX_FALSE;
				#if 0 //temp
							if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx == omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx)
							{
								omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx = -3;
								omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx = -2;
							}
							omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx = -2;							
				#endif
							omx_private->iDx_last_output = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx;
				#ifdef TS_ADJUSTMENT
							omx_private->bNeed_TS_adjustment = OMX_TRUE;
							omx_private->uTS_last_output = omx_private->prev_timestamp;
							omx_private->nST_last_output = systemTime(SYSTEM_TIME_MONOTONIC);
							LOGE("[VDEC-%d] Ready TS-Adjustment : TS(%d), in_TS(%d)", omx_private->pVideoDecodInstance.nVdec_Instance, (int)(omx_private->uTS_last_output/1000), (int)(pInputBuffer->nTimeStamp/1000));
				#endif

				#ifdef RESTORE_DECODE_ERR
						if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_RV)
						{
								int k = 0;
								unsigned char* tmpbuf = 0;
								unsigned char* seq_hdr_temp;
								unsigned short int   usWidth;
								unsigned short int   usHeight;
								if(omx_private->seqHeader_len != 0)
								{
									seq_hdr_temp = TCC_calloc(1, omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size);
									usWidth = omx_private->curr_width;
									tmpbuf = (unsigned char*)&usWidth;
									usWidth = (unsigned short int)((tmpbuf[0] << 8) | (tmpbuf[1] ));

									tmpbuf = 0;
									usHeight = omx_private->curr_height;
									tmpbuf = (unsigned char*)&usHeight;
									usHeight = (unsigned short int)((tmpbuf[0] << 8) | (tmpbuf[1] ));

						#if defined(TC_SECURE_MEMORY_COPY)
									if( omx_private->bWFDHDCP2SrcEnable )
									{
										if( omx_private->mSeqBackupPmap.size != 0 )
										{
											memcpy(omx_private->mSeqBackupMapInfo + 12, &usWidth , 2);
											memcpy(omx_private->mSeqBackupMapInfo + 14, &usHeight, 2);

											//memcpy(omx_private->mSeqBackupMapInfo + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size, 
											//			omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
											ret = TC_SecureMemoryCopy(omx_private->mSeqBackupPmap.base + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size, 
																		omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);

											omx_private->seqHeader_len = omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size;
										}
										else
										{
											omx_private->seqHeader_len = 0;
											ALOGE("[VDEC-%d] mSeqBackupPmap.size is 0.", omx_private->pVideoDecodInstance.nVdec_Instance);
										}
									}
									else
						#endif
									{
										memcpy(omx_private->seqHeader_backup + 12, &usWidth , 2);
										memcpy(omx_private->seqHeader_backup + 14, &usHeight, 2);
										//memcpy(omx_private->seqHeader_backup + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulOpaqueDataSize, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
										memcpy(seq_hdr_temp, omx_private->seqHeader_backup, 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size);
										memcpy(seq_hdr_temp + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);

										if(omx_private->seqHeader_backup != NULL)
											TCC_free(omx_private->seqHeader_backup);
										omx_private->seqHeader_backup = TCC_calloc(1,omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size);

										memcpy(omx_private->seqHeader_backup, seq_hdr_temp, omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size);
										omx_private->seqHeader_len = omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen + 26 + omx_private->pVideoDecodInstance.cdmx_info.m_sVideoInfo.m_ulEXTV1Size;
									}
									TCC_free(seq_hdr_temp);
									LOGI("[VDEC-%d] Re-backup seq_header(%ld) data to restore decode", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->seqHeader_len);
								}
						}
						else
						{
							if(omx_private->seqHeader_backup != NULL)
								TCC_free(omx_private->seqHeader_backup);

				#if defined(TC_SECURE_MEMORY_COPY)
							if( omx_private->bWFDHDCP2SrcEnable )
							{
								if( omx_private->mSeqBackupPmap.size != 0 )
								{
									//memcpy(omx_private->mSeqBackupMapInfo, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
									ret = TC_SecureMemoryCopy(omx_private->mSeqBackupPmap.base, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[PA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
									//ALOGI("[SinB] End(%d)", ret);
									omx_private->seqHeader_len = omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen;
								}
								else
								{
									omx_private->seqHeader_len = 0;
									ALOGE("[VDEC-%d] mSeqBackupPmap.size is 0.", omx_private->pVideoDecodInstance.nVdec_Instance);
								}
							}
							else
				#endif
							{
								omx_private->seqHeader_backup = TCC_calloc(1,omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
								memcpy(omx_private->seqHeader_backup, omx_private->pVideoDecodInstance.gsVDecInput.m_pInp[VA], omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen);
								omx_private->seqHeader_len = omx_private->pVideoDecodInstance.gsVDecInput.m_iInpLen;
							}
							LOGI("[VDEC-%d] Re-backup seq_header(%ld) data to restore decode", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->seqHeader_len);
						}
						#ifdef CHECK_SEQHEADER_WITH_SYNCFRAME
							if(omx_private->sequence_header_only) {
								TCC_free(omx_private->sequence_header_only);
								omx_private->sequence_header_only = NULL;
								omx_private->sequence_header_size = 0;
								omx_private->need_sequence_header_attachment = OMX_FALSE;
							}
						#endif
				#endif
						}
		#endif
					}

#else // previous scheme
					pInputBuffer->nFilledLen = 0;
					pOutputBuffer->nFilledLen = 0;
					(*(omx_private->callbacks->EventHandler))(
											openmaxStandComp,
											omx_private->callbackData,
										  OMX_EventError,
										  OMX_ErrorStreamCorrupt,
										  0,
										  NULL);
					return ;
#endif
				}
			}
		}

		// Update TimeStamp!!
		if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS
			&& omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
		{
	#ifdef SET_FRAMEBUFFER_INTO_MAX
			if(omx_private->bSet_Except_I_Frm == OMX_TRUE)
			{
				omx_private->bSet_Except_I_Frm = OMX_FALSE;
				LOGE("[VDEC-%d] Disable Except I-Frm.", omx_private->pVideoDecodInstance.nVdec_Instance);
			}
	#endif
			if( omx_private->bDecIndexOutput == OMX_TRUE )
				omx_private->bWaitNewBuffer = OMX_TRUE;

			//frame-rate update (AVG)
			omx_private->frameCount++;

//            dec_disp_info_tmp.m_iTimeStamp 		= (int)(pInputBuffer->nTimeStamp/1000);
            dec_disp_info_tmp.m_iTimeStamp 		    = pInputBuffer->nTimeStamp;
			dec_disp_info_tmp.m_iFrameType 			= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPicType;

#ifndef OPENMAX1_2
			if (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPicType == 0 )
				pOutputBuffer->nFlags |= OMX_BUFFERFLAG_DECODED_PIC_TYPE;
			else
				pOutputBuffer->nFlags &= ~OMX_BUFFERFLAG_DECODED_PIC_TYPE;
#endif

			dec_disp_info_tmp.m_iPicStructure 		= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPictureStructure;
			dec_disp_info_tmp.m_iextTimeStamp 		= 0;
			dec_disp_info_tmp.m_iM2vFieldSequence   = 0;
			dec_disp_info_tmp.m_iFrameSize 			= omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iConsumedBytes;// gsCDmxOutput.m_iPacketSize;
			dec_disp_info_tmp.m_iFrameDuration 		= 2;
			dec_disp_info_tmp.m_iNumMBError         = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs;

			switch( omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat)
			{
				case STD_RV:
					dec_disp_info_tmp.m_iextTimeStamp = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iRvTimestamp;
					if (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS )
					{
						omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
						disp_pic_info( CVDEC_DISP_INFO_UPDATE, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_ctrl),
							(void*)&dec_disp_info_tmp, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_input), omx_private);
					}

					break;

				case STD_MVC:
				case STD_AVC:
					dec_disp_info_tmp.m_iM2vFieldSequence = 0;
					#if defined(TCC_VPU_C7_INCLUDE)
					dec_disp_info_tmp.m_bIsMvcDependent = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded;
					#endif
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
					disp_pic_info( CVDEC_DISP_INFO_UPDATE, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_ctrl), (void*)&dec_disp_info_tmp, (void*)&omx_private->pVideoDecodInstance.dec_disp_info_input, omx_private);
					break;

				case STD_MPEG2:
					if( dec_disp_info_tmp.m_iPicStructure != 3 )
					{
						dec_disp_info_tmp.m_iFrameDuration = 1;
					}
					else if( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveSequence == 1 )
					{
						if( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iRepeatFirstField == 0 )
							dec_disp_info_tmp.m_iFrameDuration = 2;
						else
							dec_disp_info_tmp.m_iFrameDuration = ( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst == 0 )?4:6;
					}
					else
					{
						//pOutputBuffer->nFlags |= OMX_BUFFERFLAG_INTERLACED_FRAME;
						/* interlaced sequence */
						if( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0 )
							dec_disp_info_tmp.m_iFrameDuration = 2;
						else
							dec_disp_info_tmp.m_iFrameDuration = ( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iRepeatFirstField == 0 )?2:3;
					}

					dec_disp_info_tmp.m_iM2vFieldSequence = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vFieldSequence;
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameRate = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vFrameRate;
					disp_pic_info( CVDEC_DISP_INFO_UPDATE, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_ctrl), (void*)&dec_disp_info_tmp, (void*)&omx_private->pVideoDecodInstance.dec_disp_info_input, omx_private);
					break;

				default:
					dec_disp_info_tmp.m_iM2vFieldSequence = 0;
					omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
					disp_pic_info( CVDEC_DISP_INFO_UPDATE, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_ctrl), (void*)&dec_disp_info_tmp, (void*)&omx_private->pVideoDecodInstance.dec_disp_info_input, omx_private);
					break;
			}
			DPRINTF_DEC("[VDEC-%d] IN-Buffer :: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", omx_private->pVideoDecodInstance.nVdec_Instance,
						pInputBuffer->pBuffer[0], pInputBuffer->pBuffer[1], pInputBuffer->pBuffer[2], pInputBuffer->pBuffer[3], pInputBuffer->pBuffer[4],
						pInputBuffer->pBuffer[5], pInputBuffer->pBuffer[6], pInputBuffer->pBuffer[7], pInputBuffer->pBuffer[8], pInputBuffer->pBuffer[9]);
			//current decoded frame info
			DPRINTF_DEC( "[VDEC-%d] [In - %s][N:%4d][LEN:%6d][RT:%8d] [DecoIdx:%2d][DecStat:%d][FieldSeq:%d][TR:%8d] ", omx_private->pVideoDecodInstance.nVdec_Instance,
							print_pic_type(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, dec_disp_info_tmp.m_iFrameType, dec_disp_info_tmp.m_iPicStructure),
							omx_private->pVideoDecodInstance.video_dec_idx, pInputBuffer->nFilledLen,(pInputBuffer->nTimeStamp),
							omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus,
							omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vFieldSequence, dec_disp_info_tmp.m_iextTimeStamp);

			//increase counter of output buffer to be displayed in the near future
			omx_private->nDelayedOutputBufferCount++;
		}
		else
		{
			if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS_FIELD_PICTURE)
			{
#ifdef TS_TIMESTAMP_CORRECTION
				if( ((omx_private->pVideoDecodInstance.dec_disp_info_ctrl).m_iFmtType  == CONTAINER_TYPE_MPG) ||((omx_private->pVideoDecodInstance.dec_disp_info_ctrl).m_iFmtType  == CONTAINER_TYPE_TS) )
#else//TS_TIMESTAMP_CORRECTION
				if( (omx_private->pVideoDecodInstance.dec_disp_info_ctrl).m_iFmtType  == CONTAINER_TYPE_MPG )
#endif//TS_TIMESTAMP_CORRECTION
				{
					if( dec_disp_info_tmp.m_iTimeStamp <= omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS )
						dec_disp_info_tmp.m_iTimeStamp = omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS + ((omx_private->pVideoDecodInstance.gsPtsInfo.m_iPTSInterval * omx_private->pVideoDecodInstance.gsPtsInfo.m_iRamainingDuration) >> 1);
					omx_private->pVideoDecodInstance.gsPtsInfo.m_iLatestPTS = dec_disp_info_tmp.m_iTimeStamp;
					omx_private->pVideoDecodInstance.gsPtsInfo.m_iRamainingDuration = 1;
				}
			}
			DPRINTF_DEC("[VDEC-%d] IN-Buffer :: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x", omx_private->pVideoDecodInstance.nVdec_Instance,
						pInputBuffer->pBuffer[0], pInputBuffer->pBuffer[1], pInputBuffer->pBuffer[2], pInputBuffer->pBuffer[3], pInputBuffer->pBuffer[4],
						pInputBuffer->pBuffer[5], pInputBuffer->pBuffer[6], pInputBuffer->pBuffer[7], pInputBuffer->pBuffer[8], pInputBuffer->pBuffer[9]);
			DPRINTF_DEC( "[VDEC-%d] [Err In - %s][N:%4d][LEN:%6d][RT:%8d] [DecoIdx:%2d][DecStat:%d][FieldSeq:%d][TR:%8d] ", omx_private->pVideoDecodInstance.nVdec_Instance,
							print_pic_type(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, dec_disp_info_tmp.m_iFrameType, dec_disp_info_tmp.m_iPicStructure),
							omx_private->pVideoDecodInstance.video_dec_idx, pInputBuffer->nFilledLen, (int)(pInputBuffer->nTimeStamp/1000),
							omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus,
							omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vFieldSequence, dec_disp_info_tmp.m_iextTimeStamp);
		}


		//if (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
		{
			switch( omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat)
			{
				case STD_MVC:
				case STD_AVC:
					if ( ( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0 && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 0x3 )
						|| (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame )
						|| ((omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPictureStructure  ==1) && (omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iInterlace ==0)) )
					{
						pOutputBuffer->nFlags |= OMX_BUFFERFLAG_INTERLACED_FRAME;
					}

	/*
					if( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst == 0)
					{
						pOutputBuffer->nFlags |= OMX_BUFFERFLAG_ODD_FIRST_FRAME;
					}
	*/
					break;

				case STD_MPEG2:
					if ( ( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iM2vProgressiveFrame == 0 && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPictureStructure == 0x3 )
						|| omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iInterlacedFrame )
					{
						//LOGD("Interlaced Frame!!!");
						pOutputBuffer->nFlags |= OMX_BUFFERFLAG_INTERLACED_FRAME;
					}

					if( omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iTopFieldFirst == 0)
					{
						//LOGD("Odd First Frame!!!");
						pOutputBuffer->nFlags |= OMX_BUFFERFLAG_ODD_FIRST_FRAME;
					}
					break;

				default:
					break;
			}
		}

		if( omx_private->frameSearchOrSkip_flag
			&& omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0
			//&& omx_private->gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS)
			&& (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS || omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS_FIELD_PICTURE))
		{
			// frameType - 0: unknown, 1: I-frame, 2: P-frame, 3:B-frame
			int frameType = get_frame_type_for_frame_skipping( omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat,
															omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPicType,
															omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iPictureStructure );

			if( omx_private->pVideoDecodInstance.gsVDecInput.m_iFrameSearchEnable )
			{
				omx_private->frameSearchOrSkip_flag = 2; //I-frame Search Mode disable and B-frame Skip Mode enable
				DPRINTF_DEC_STATUS("[VDEC-%d] [SEEK] I-frame Search Mode disable and B-frame Skip Mode enable", omx_private->pVideoDecodInstance.nVdec_Instance);
			}
			else if( omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode == VDEC_SKIP_FRAME_ONLY_B )
			{
				if(frameType != 3)
				{
					DPRINTF_DEC_STATUS( "[VDEC-%d] [SEEK] B-frame Skip Mode Disable after P-frame decoding!!!", omx_private->pVideoDecodInstance.nVdec_Instance);
					omx_private->frameSearchOrSkip_flag = 0; //B-frame Skip Mode disable
					omx_private->skipFrameNum = (omx_private->I_frame_search_mode == AVC_IDR_PICTURE_SEARCH_MODE)
					                          ? omx_private->numSkipFrame - 3 : 0;
				}
			}
			else if( omx_private->pVideoDecodInstance.gsVDecInput.m_iSkipFrameMode == VDEC_SKIP_FRAME_EXCEPT_I )
			{
				if(frameType == 1)
				{
					DPRINTF_DEC_STATUS( "[VDEC-%d] [SEEK] B-frame Skip Mode Disable after P-frame decoding!!!", omx_private->pVideoDecodInstance.nVdec_Instance);
					omx_private->frameSearchOrSkip_flag = 0; //B-frame Skip Mode disable
					omx_private->skipFrameNum = (omx_private->I_frame_search_mode == AVC_IDR_PICTURE_SEARCH_MODE)
					                          ? omx_private->numSkipFrame - 3 : 0;
				}
			}
		}

		if (CONTAINER_TYPE_TS != omx_private->pVideoDecodInstance.container_type)
		{
			omx_private->inputCurrBuffer = pInputBuffer->pBuffer;
			omx_private->inputCurrLength = pInputBuffer->nFilledLen;
		}
		//////////////////////////////////////////////////////////////////////////////////////////

#ifdef SET_FRAMEBUFFER_INTO_MAX
		if( ((omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx < 0) || (omx_private->bCheck_same_index == OMX_TRUE && (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx == omx_private ->iDx_last_output))) 
			&& omx_private->bDetected_res_changed == OMX_TRUE)
		{
			LOGE("[VDEC-%d] NO-Display index!! %d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx);
			omx_private->bNoOutput_after_res_changed = OMX_TRUE;
	#ifdef TS_ADJUSTMENT
			omx_private->uTS_last_output = omx_private->prev_timestamp;
			omx_private->nST_last_output = systemTime(SYSTEM_TIME_MONOTONIC);
			LOGE("[VDEC-%d] Ready TS-Adjustment : TS(%d), in_TS(%d)", omx_private->pVideoDecodInstance.nVdec_Instance, (int)(omx_private->uTS_last_output/1000), (int)(pInputBuffer->nTimeStamp/1000));
	#endif
		}

		if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx == omx_private ->iDx_last_output)
			omx_private->bCheck_same_index = OMX_TRUE;

//		LOGE("Input(%d), Display index %d (0x%x), Decoded index %d (0x%x) !!", omx_private->inputCurrLength, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx,
//			(unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][0], omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx, (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][0]);
#endif

		if (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS ||
		    (omx_private->bDecIndexOutput == OMX_TRUE &&
			 omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS &&
			 omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
#ifdef SET_FRAMEBUFFER_INTO_MAX
			|| (omx_private->bDetected_res_changed == OMX_TRUE && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx > 0)
#endif
		)
		{
			int dispOutIdx;
			dec_disp_info_t *pdec_disp_info = NULL;

			omx_private->ConsecutiveVdecFailCnt = 0; //Reset Consecutive Vdec Fail Counting B060955

			if(((omx_private->gralloc_info.mDispIndex == omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx)|| 
               (omx_private->gralloc_info.mDispIndex == omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx))
#ifdef INCLUDE_WMV78_DEC	
               &&(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat != STD_WMV78)
#endif
			)
				DBUG_MSG("[VDEC-%d] ##################### Same Displayed index was copied %d ? (Disp)%d : (Dec)%d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->gralloc_info.mDispIndex, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx);

			if( omx_private->bDecIndexOutput == OMX_TRUE && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
				dispOutIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx;
			else
				dispOutIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx;

			if( NULL != plat_priv )
			{
				GBUG_MSG(": [VDEC-%d] P copied index : %d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->gralloc_info.mDispIndex);
				plat_priv->unique_addr = omx_private->mCodecStart_ms;
				if(gralloc_copy_mode == COPY_DONE)
					plat_priv->copied = 1;
				else
					plat_priv->copied = 0;
				plat_priv->width = outPort->sPortParam.format.video.nFrameWidth;
				plat_priv->height = outPort->sPortParam.format.video.nFrameHeight;
				if(omx_private->pVideoDecodInstance.gsVDecInit.m_bCbCrInterleaveMode)
					plat_priv->format = OMX_COLOR_FormatYUV420SemiPlanar;
				else
				{
			#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
					if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MJPG)
					{
						//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
						if(omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
							plat_priv->format = OMX_COLOR_FormatYUV422Planar;
						else
							plat_priv->format = OMX_COLOR_FormatYUV420Planar;
					}
					else
			#endif
						plat_priv->format = OMX_COLOR_FormatYUV420Planar;
				}

				if( omx_private->bDecIndexOutput == OMX_TRUE && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
				{
					plat_priv->offset[0] = (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][0];
					plat_priv->offset[1] = (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][1];
					plat_priv->offset[2] = (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][2];
					decode_result = 2;
			#if defined(TCC_VPU_C7_INCLUDE)
					dispMVCOutIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded;
			#endif
				}
				else
				{
					plat_priv->offset[0] = (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][0];
					plat_priv->offset[1] = (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][1];
					plat_priv->offset[2] = (unsigned int)omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][2];
			#if defined(TCC_VPU_C7_INCLUDE)
					dispMVCOutIdx = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDisplay;
			#endif
				}

				sprintf((char*)plat_priv->name, "video");
				plat_priv->name[5] = 0;
				plat_priv->optional_info[0] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicWidth;
				plat_priv->optional_info[1] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pInitialInfo->m_iPicHeight;
				plat_priv->optional_info[2] = outPort->sPortParam.format.video.nFrameWidth;
				plat_priv->optional_info[3] = outPort->sPortParam.format.video.nFrameHeight;
				plat_priv->optional_info[4] = 0; //buffer_id // there is no need to put it.
				plat_priv->optional_info[5] = 0; //timeStamp	// there is no need to put it.
				plat_priv->optional_info[6] = 0; //curTime
				plat_priv->optional_info[7] = 0; //flags		// there is no need to put it.
				plat_priv->optional_info[8] = 0; //framerate	// there is no need to put it.
				#if defined(MVC_PROCESS) && defined(TCC_VPU_C7_INCLUDE)
				plat_priv->optional_info[9] = (omx_private->bMVC_mode==OMX_TRUE) ? dispMVCOutIdx : 0; 
				#else
				plat_priv->optional_info[9] = 0;
				#endif
				plat_priv->optional_info[10] = 0; //MVC Base addr0
				plat_priv->optional_info[11] = 0; //MVC Base addr1
				plat_priv->optional_info[12] = 0;//MVC Base addr2
				plat_priv->optional_info[13] = 0;//Vsync enable field	//there is no need to put it. // Player must set '1' to use Vsync with other infos
				plat_priv->optional_info[14] = dispOutIdx;// current vpu index for on-the-fly mode not using video-vsync.
				plat_priv->optional_info[15] = omx_private->pVideoDecodInstance.nVdec_Instance;// current vpu instance-index.

				output_len = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3 / 2;
				GBUG_MSG("[VDEC-%d] copied ? private data %p %dx%d, %d, 0x%x-0x%x-0x%x", omx_private->pVideoDecodInstance.nVdec_Instance, (void*)plat_priv, plat_priv->width, plat_priv->height, plat_priv->format,
							plat_priv->offset[0], plat_priv->offset[1], plat_priv->offset[2]);
			}

#ifdef ANDROID_USE_GRALLOC_BUFFER
			if(omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr)
			{
				if( NULL != plat_priv )
				{
					pOutputBuffer->pOutputPortPrivate = plat_priv;
					omx_private->gralloc_info.grallocModule->unlock((gralloc_module_t const *) omx_private->gralloc_info.grallocModule, (buffer_handle_t)grallocHandle);
				}

		#ifdef SET_FRAMEBUFFER_INTO_MAX
				if(omx_private->bDetected_res_changed == OMX_FALSE || (omx_private->bDetected_res_changed == OMX_TRUE && omx_private->bNoOutput_after_res_changed == OMX_FALSE))
		#endif
				{
					if( omx_private->bDecIndexOutput == OMX_TRUE && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
					{
						GBUG_MSG("[VDEC-%d] VPU address copy 2:: 0x%p-0x%p-0x%p - 0x%p-0x%p-0x%p", omx_private->pVideoDecodInstance.nVdec_Instance,
										omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][0], omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][1], omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][2],
										omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][0], omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][1], omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][2]);

						omx_private->gralloc_info.m_pDispOut[PA][0] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][0];
						omx_private->gralloc_info.m_pDispOut[PA][1] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][1];
						omx_private->gralloc_info.m_pDispOut[PA][2] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[PA][2];

						omx_private->gralloc_info.m_pDispOut[VA][0] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][0];
						omx_private->gralloc_info.m_pDispOut[VA][1] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][1];
						omx_private->gralloc_info.m_pDispOut[VA][2] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pCurrOut[VA][2];
					}
					else
					{
						GBUG_MSG("[VDEC-%d] VPU address copy 1:: 0x%p-0x%p-0x%p - 0x%p-0x%p-0x%p", omx_private->pVideoDecodInstance.nVdec_Instance,
										omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][0], omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][1], omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][2],
										omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][0], omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][1], omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][2]);

						omx_private->gralloc_info.m_pDispOut[PA][0] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][0];
						omx_private->gralloc_info.m_pDispOut[PA][1] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][1];
						omx_private->gralloc_info.m_pDispOut[PA][2] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[PA][2];

						omx_private->gralloc_info.m_pDispOut[VA][0] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][0];
						omx_private->gralloc_info.m_pDispOut[VA][1] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][1];
						omx_private->gralloc_info.m_pDispOut[VA][2] = omx_private->pVideoDecodInstance.gsVDecOutput.m_pDispOut[VA][2];
						omx_private->gralloc_info.mDispIndex = dispOutIdx;
					}
				}
			}
#endif

#ifdef COMPARE_TIME_LOG
			{
				end = clock();

				dec_time[time_cnt] = (end-start)*1000/CLOCKS_PER_SEC;
				total_dec_time += dec_time[time_cnt];
				if(time_cnt != 0 && time_cnt % 29 == 0)
				{
					LOGD("VDEC_TIME %d ms: %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d",
						total_dec_time/total_frm, dec_time[0], dec_time[1], dec_time[2], dec_time[3], dec_time[4], dec_time[5], dec_time[6], dec_time[7], dec_time[8], dec_time[9],
						dec_time[10], dec_time[11], dec_time[12], dec_time[13], dec_time[14], dec_time[15], dec_time[16], dec_time[17], dec_time[18], dec_time[19],
						dec_time[20], dec_time[21], dec_time[22], dec_time[23], dec_time[24], dec_time[25], dec_time[26], dec_time[27], dec_time[28], dec_time[29]);
					time_cnt = 0;
				}
				else{
					time_cnt++;
				}
			}
#endif
			total_frm++;

			//Get TimeStamp!!
#ifdef SET_FRAMEBUFFER_INTO_MAX
			if(omx_private->bDetected_res_changed == OMX_TRUE /*&& dispOutIdx < 0*/)
			{
				pOutputBuffer->nTimeStamp = omx_private->prev_timestamp + omx_private->prev_interval;
			}
			else
#endif
			{
				omx_private->pVideoDecodInstance.dec_disp_info_input.m_iFrameIdx = dispOutIdx;
				disp_pic_info( CVDEC_DISP_INFO_GET, (void*)&(omx_private->pVideoDecodInstance.dec_disp_info_ctrl), (void*)&pdec_disp_info, (void*)&omx_private->pVideoDecodInstance.dec_disp_info_input, omx_private);

				if( pdec_disp_info != (dec_disp_info_t*)0 )
				{
#ifdef JPEG_DECODE_FOR_MJPEG
					if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_MJPG)
					{
						pOutputBuffer->nTimeStamp = pInputBuffer->nTimeStamp;
					}
					else
#endif
					if(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat == STD_RV)
					{
//						pOutputBuffer->nTimeStamp = (OMX_TICKS)pdec_disp_info->m_iextTimeStamp * 1000;
						pOutputBuffer->nTimeStamp = (OMX_TICKS)pdec_disp_info->m_iextTimeStamp;
					}
					else// if(omx_private->gsVDecInit.m_iBitstreamFormat == STD_MPEG2)
					{
//						pOutputBuffer->nTimeStamp = (OMX_TICKS)pdec_disp_info->m_iTimeStamp * 1000; //pdec_disp_info->m_iM2vFieldSequence * 1000;
						pOutputBuffer->nTimeStamp = (OMX_TICKS)pdec_disp_info->m_iTimeStamp; //pdec_disp_info->m_iM2vFieldSequence * 1000;
					}


					if(omx_private->pVideoDecodInstance.gsVDecInit.m_bEnableUserData)
					{
						print_user_data((unsigned char*)omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_UserDataAddress[VA]);
					}

					DPRINTF_DEC( "[VDEC-%d] [Out - %s][N:%4d][LEN:%6d][RT:%8d] [DispIdx:%2d][OutStat:%d][FieldSeq:%d][TR:%8d] ", omx_private->pVideoDecodInstance.nVdec_Instance,
									print_pic_type(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, pdec_disp_info->m_iFrameType, pdec_disp_info->m_iPicStructure),
									omx_private->pVideoDecodInstance.video_dec_idx, pdec_disp_info->m_iFrameSize, pdec_disp_info->m_iTimeStamp,
									omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus,
									pdec_disp_info->m_iM2vFieldSequence, pdec_disp_info->m_iextTimeStamp);
				}
				else
				{
					pOutputBuffer->nTimeStamp = omx_private->prev_timestamp + omx_private->prev_interval;
				}
			}

			omx_private->prev_interval = pOutputBuffer->nTimeStamp - omx_private->prev_timestamp;
			omx_private->prev_timestamp = pOutputBuffer->nTimeStamp;

			if( omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr )
			{
				#ifdef SET_FRAMEBUFFER_INTO_MAX
				if((omx_private->bDetected_res_changed == OMX_FALSE) || (omx_private->bNoOutput_after_res_changed == OMX_FALSE))
				#endif
				{
					OMX_TICKS output_timestamp = omx_private->gralloc_info.nTimestamp;
					omx_private->gralloc_info.nTimestamp = pOutputBuffer->nTimeStamp;
					pOutputBuffer->nTimeStamp = output_timestamp;
				}
			}

			//if( omx_private->bDecIndexOutput == OMX_FALSE && omx_private->bWaitKeyFrameOut == OMX_TRUE )
			if( omx_private->bWaitKeyFrameOut == OMX_TRUE )
			{
				int output_type = 0;
				if( pdec_disp_info )
					output_type = get_frame_type_for_frame_skipping(omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat, pdec_disp_info->m_iFrameType, pdec_disp_info->m_iPicStructure);

				if( output_type == 1 || output_type == 2)	
					omx_private->bWaitKeyFrameOut = OMX_FALSE;
				else
				{
					if( ( ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &dispOutIdx, NULL, omx_private->pVideoDecodInstance.pVdec_Instance) ) < 0 )
					{
						LOGE( "[VDEC-%d] [VDEC_BUF_FLAG_CLEAR] Idx = %d, ret = %ld", omx_private->pVideoDecodInstance.nVdec_Instance, dispOutIdx, ret );
						VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
						return;
					}
					decode_result = 3;

					LOGE("[VDEC-%d] drop frame after seek (idx: %d / type: %d)", omx_private->pVideoDecodInstance.nVdec_Instance, dispOutIdx, output_type);
				}
			}

			if( omx_private->bWaitKeyFrameOut == OMX_FALSE )
			{
#ifdef HAVE_ANDROID_OS

				cleared_index_for_vpu = pull_displayedIndex(openmaxStandComp, pInputBuffer, pOutputBuffer);
				if(omx_private->max_fifo_cnt != 0)
				{
					// to change video output device
					OMX_BOOL before_bVSyncMode	 = omx_private->bVSyncMode;
					char value[PROPERTY_VALUE_MAX];
					property_get("tcc.video.vsync.enable", value, "");					
					int sussess_pushed = 0;
					
					char isMultiDecoding[PROPERTY_VALUE_MAX];
					property_get("tcc.video.multidecoding.check", isMultiDecoding, "0");

					if(atoi(isMultiDecoding) != 0)
					{
						property_get("tcc.video.hwr.id", value1, "0");
						if(atoi(value) != 0 && atoi(value1) == omx_private->mCodecStart_ms)
							omx_private->bVSyncMode = 1;
						else{
							property_get("tcc.solution.mbox", value, "");
							if(atoi(value) && atoi(value1) == omx_private->mCodecStart_ms)
								omx_private->bVSyncMode = 1;
							else
								omx_private->bVSyncMode = 0;
						}
					}
					else
					{
						if(atoi(value) != 0)
							omx_private->bVSyncMode = 1;
						else{
							omx_private->bVSyncMode = 0;
						}
					}

					if(before_bVSyncMode != omx_private->bVSyncMode)
					{
						if(0 > clear_all_displayedIndex(openmaxStandComp, pInputBuffer, pOutputBuffer, OMX_FALSE))
							return;
					}

					if((sussess_pushed = push_displayedIndex(openmaxStandComp, pInputBuffer, pOutputBuffer, dispOutIdx)) < 0){
						ALOGE("[VDEC-%d] return because the result of push_displayedIndex() is %d.", omx_private->pVideoDecodInstance.nVdec_Instance, sussess_pushed);
						return;
					}

					if(sussess_pushed > 0)
					{
						omx_private->in_index = (omx_private->in_index + 1) % omx_private->max_fifo_cnt;
						omx_private->used_fifo_count++;
					}

					if(omx_private->frm_clear || omx_private->used_fifo_count == omx_private->max_fifo_cnt)
					{
						int cleared_buff_id;

						if( !omx_private->frm_clear )
							omx_private->frm_clear = 1;

						if(omx_private->bVSyncMode)
							cleared_buff_id = ioctl(omx_private->mProxy_fd, TCC_LCDC_VIDEO_GET_DISPLAYED, NULL) ;  // TCC_LCDC_HDMI_GET_DISPLAYED
						else if(omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr)
							cleared_buff_id = ioctl(omx_private->mProxy_fd, TCC_VIDEO_GET_DISPLAYED_BUFFER_ID, omx_private->pVideoDecodInstance.nVdec_Instance);
						else
							cleared_buff_id = -1;
						if(cleared_buff_id < 0)
						{
							// Case: decoder created from flash player and html5 (both in inline-mode case.)
							//       other decoders except one(using hwrenderer) during multi-playback => juse in stagefright case.
							//       decoder created fram Video-Editor (case by case)
							DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: Clear %d :: %d", omx_private->pVideoDecodInstance.nVdec_Instance, cleared_buff_id, cleared_index_for_vpu);
							if(cleared_index_for_vpu >= 0)
							{
								ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &cleared_index_for_vpu, NULL, omx_private->pVideoDecodInstance.pVdec_Instance);
								if(ret  < 0 )
								{
									LOGE( "[VDEC-%d] [VDEC_BUF_FLAG_CLEAR] Idx = %d, ret = %ld", omx_private->pVideoDecodInstance.nVdec_Instance, cleared_index_for_vpu, ret );
									VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
									return;
								}

								omx_private->out_index = (omx_private->out_index + 1) % omx_private->max_fifo_cnt;
								omx_private->used_fifo_count-- ;
							}
						}
						else
						{
							// Case: normal decoder cerated from player that use video-vsync.
							//		 decoders created from MooPlayer on Google-TV Platform.
							//		 decoder created fram Video-Editor (case by case)
							//		 decoder using hwrenderer without video-vsync.(ex. flash playback with full-view mode.)
							DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: Clear with kernel %d :: %d", omx_private->pVideoDecodInstance.nVdec_Instance, cleared_buff_id, cleared_index_for_vpu);
							ret = clear_vpu_buffer(cleared_buff_id, omx_private);
							if(ret  < 0 )
							{
								LOGE( "[VDEC-%d] [VDEC_BUF_FLAG_CLEAR] Idx = %ld, ret = %ld", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->mBuffVpu[omx_private->out_index].Display_index, ret );
								VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
								return;
							}
						}
					}
					else
					{
						DPRINTF_VBUFFER_MANAGE("[VDEC-%d] :: Need to queue the displayed buffer index. %d != %d, id : %d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->used_fifo_count, omx_private->max_fifo_cnt, omx_private->buffer_unique_id);
					}
				}
				else
				{
					if( ( ret = omx_private->pVideoDecodInstance.gspfVDec( VDEC_BUF_FLAG_CLEAR, NULL, &dispOutIdx, NULL, omx_private->pVideoDecodInstance.pVdec_Instance) ) < 0 )
					{
						LOGE( "[VDEC-%d] [VDEC_BUF_FLAG_CLEAR] Idx = %d, ret = %ld", omx_private->pVideoDecodInstance.nVdec_Instance, dispOutIdx, ret );
						VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
						return;
					}
				}
			}
#endif
			if( plat_priv != NULL) {
				plat_priv->optional_info[4] = omx_private->buffer_unique_id;
				omx_private->buffer_unique_id++;
			}
			omx_private->pVideoDecodInstance.video_dec_idx++;
		}
		else
		{
			DPRINTF_DEC( "[VDEC-%d] [VDEC_DECODE] NO-OUTPUT!! m_iDispOutIdx = %d, m_iDecodedIdx = %d, m_iOutputStatus = %d, m_iDecodingStatus = %d, m_iNumOfErrMBs = %d", omx_private->pVideoDecodInstance.nVdec_Instance,
											omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx,
											omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus,
											omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs);

			if(omx_private->frameSearchOrSkip_flag == 1)
			{
				omx_private->ConsecutiveVdecFailCnt++;
				if(omx_private->ConsecutiveVdecFailCnt >= omx_private->maxConsecutiveVdecFailCnt)
				{
					LOGE("[VDEC-%d] [VDEC_ERROR]m_iOutputStatus %d %ldtimes!!!\n", omx_private->pVideoDecodInstance.nVdec_Instance,omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iOutputStatus,omx_private->ConsecutiveVdecFailCnt);
					omx_private->ConsecutiveVdecFailCnt = 0; // Reset Consecutive Vdec Fail Counting B060955
					VideoDecErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
					return;
				}
			}

#ifdef ENABLE_DECODE_ONLY_MODE_AVC
			if(omx_private->bSetDecodeOnlyMode == OMX_TRUE && omx_private->I_frame_search_mode == AVC_NONIDR_PICTURE_SEARCH_MODE)
			{
				if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx < 0 && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
				{
					if(omx_private->skipFrameNum > 3) {
						omx_private->skipFrameNum -= 3;
					}
					omx_private->decodeOnlyErrNum++;
				}
			}
#endif

			output_len = 0;
		}

#ifdef ENABLE_DECODE_ONLY_MODE_AVC
		if(omx_private->bSetDecodeOnlyMode == OMX_TRUE)
		{
			LOGINFO("[VDEC-%d] decodeOnlyErrNum = %d, skipFrameNum = %d", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->decodeOnlyErrNum, omx_private->skipFrameNum);
			if(omx_private->skipFrameNum == omx_private->numSkipFrame || omx_private->decodeOnlyErrNum == MAX_DEC_ONLY_ERR_THRESHOLD)
			{
				// Finish decode only mode.
				omx_private->bSetDecodeOnlyMode = OMX_FALSE;
				omx_private->skipFrameNum = 0;
				omx_private->decodeOnlyErrNum = 0;
				LOGMSG("[VDEC-%d] Finish Decode Only frame", omx_private->pVideoDecodInstance.nVdec_Instance);
			}
			else
			{
				if(!(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDispOutIdx < 0 && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx < 0))
				{
					omx_private->skipFrameNum++;
				}

				pInputBuffer->nFlags |= OMX_BUFFERFLAG_DECODEONLY;
				LOGINFO("[VDEC-%d] Decode Only frame - skipFrameNum(%d)", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->skipFrameNum);
			}
		}
#endif

		internalOutputFilled = 1;

#ifdef ANDROID_USE_GRALLOC_BUFFER
		if(omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr)
#endif
		{
			if(omx_private->bThumbnailMode && omx_private->pThumbnailBuff == 0)
			{
				if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_SUCCESS
					&& omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iDecodedIdx >= 0)
				{
					CopyOutputFrame(openmaxStandComp, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_DECODED_BUFFER);
					return;
				}
			}
		}

#ifdef SET_FRAMEBUFFER_INTO_MAX
		if(omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs > 0)
		{
	#ifdef SKIP_ERROR_MB_FRAME
			omx_private->bSet_Except_I_Frm = OMX_TRUE;
			LOGE("[VDEC-%d] Enable Except I-Frame :: ErrorMBs(%d).", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs);
	#endif
		}

		if( omx_private->bWFD_mode == OMX_TRUE ) {
#if defined(TCC_VPU_C7_INCLUDE)
			if( omx_private->bWFDDisplayLV >= 4 &&omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_Reserved[11] != 0 
					&& (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_Reserved[11] - omx_private->mWFD_PrevVPUPOC != 2) )
			{
				ALOGE("==== POC IDR request call");
				property_set("tcc.wfd.IDR.request", "1");
				omx_private->bSet_Except_I_Frm = OMX_TRUE;
				omx_private->bWFDErrorFrame = OMX_TRUE;
			}
			omx_private->mWFD_PrevVPUPOC = omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_Reserved[11];
#endif

			if ( omx_private->bWFDDisplayEnable == OMX_TRUE && omx_private->bWFDDisplayLV >= 3 && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs > 0 ) {
				ALOGI("[WFD] MB error Lv3");
				property_set("tcc.wfd.IDR.request", "1");
				omx_private->bSet_Except_I_Frm = OMX_TRUE;
				omx_private->bWFDErrorFrame = OMX_TRUE;
			}
		}

		if(omx_private->bDetected_res_changed == OMX_TRUE)
		{
			if(omx_private->bNoOutput_after_res_changed == OMX_TRUE)
				pOutputBuffer->nFilledLen = 0;
			else{
				pOutputBuffer->nFilledLen = output_len;
				LOGD("[VDEC-%d] out frame %8d", omx_private->pVideoDecodInstance.nVdec_Instance, (int)(pOutputBuffer->nTimeStamp/1000));
			}
			//LOGI("In %d / Out %d", pInputBuffer->nFilledLen, pOutputBuffer->nFilledLen);
			return;
		}
#endif

		// To process input stream retry or next frame
		switch (decode_result) {
			case 0 :	// Display Output Success, Decode Failed Due to Buffer Full
			case 1 :	// Display Output Failed, Decode Failed Due to Buffer Full
				break;
			case 2 :	// Display Output Success, Decode Success
			case 3 :	// Display Output Failed, Decode Success
#ifdef HAVE_ANDROID_OS
				/* ZzaU:: Process exception to make right image because android framework only send one-frame on thumbnail-mode.
							   We will use this condition Because of mp4/3gp formats using pv-parser.*/
				// If data comes from the TCC parser, doesn't need this routine
				if (!(omx_private->extractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC) && decode_result == 3)
				{
					if(omx_private->bThumbnailMode && omx_private->pThumbnailBuff==0) // SSG
					{
						omx_private->displaying_error_count--;

						if (omx_private->displaying_error_count == 0)
						{
							// we never display this frame
							// so, have to fill the thumbnail image with black
							CopyOutputFrame(openmaxStandComp, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_BLACK_FRAME);
						}
						else
						{
							return;
						}
					}
				}
#endif

				//LOGI("Consumed byte = %d, input_offset = %d, currLength = %d", omx_private->gsVDecOutput.m_DecOutInfo.m_iConsumedBytes,
				//                                                               input_offset, omx_private->inputCurrLength);
                if ((CONTAINER_TYPE_TS == omx_private->pVideoDecodInstance.container_type)
                && (omx_private->inputCurrLength > (omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iConsumedBytes + input_offset)))
                {
                    nLen += omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iConsumedBytes + input_offset;
                }
                else
                {
                    nLen += omx_private->inputCurrLength;
                }

				DPRINTF_DEC("[VDEC-%d] ----------------------> inputCurrLength : %ld, nLen : %ld\n", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->inputCurrLength, nLen);
				if (nLen < 0) {
					LOGE("[VDEC-%d] ----> A general error or simply frame not decoded?\n", omx_private->pVideoDecodInstance.nVdec_Instance);
				}
				if ( nLen >= 0 && internalOutputFilled)
				{
					omx_private->inputCurrBuffer += nLen;
		#ifdef SET_FRAMEBUFFER_INTO_MAX
					if(omx_private->bDetected_res_changed == OMX_FALSE)
		#endif
						omx_private->inputCurrLength -= nLen;
					pInputBuffer->nFilledLen -= nLen;

#if 1 // until vpu bug is fixed
					if((CONTAINER_TYPE_TS == omx_private->pVideoDecodInstance.container_type)
					//&& (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC || omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC))	cvipop debug
					&& (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC
						|| omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMVC
						|| omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG2
						|| omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4
						|| omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV
						|| omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVS))
					{
                        pInputBuffer->nFilledLen = 0;
					}
#endif

					//Buffer is fully consumed. Request for new Input Buffer
					if(pInputBuffer->nFilledLen == 0)
					{
						omx_private->isNewBuffer = 1;
						DPRINTF_DEC("[VDEC-%d] ----------------------> New InputBuffer!!", omx_private->pVideoDecodInstance.nVdec_Instance);
					}
				}
				break;
			default :
				break;
		}

		// To process output stream retry or next frame
		switch (decode_result) {
			case 0 :	// Display Output Success, Decode Failed Due to Buffer Full
			case 2 :	// Display Output Success, Decode Success
				if ( nLen >= 0 && internalOutputFilled)
				{
#ifdef HAVE_ANDROID_OS
	#ifdef ANDROID_USE_GRALLOC_BUFFER
					if(omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr)
					{
						#if defined(MVC_PROCESS) && defined(TCC_VPU_C7_INCLUDE)
						if(omx_private->bMVC_mode==OMX_TRUE && plat_priv != NULL && omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat==STD_MVC)
						{
							//LOGD(" MVC[%s] plat_priv:%x  Y:%x  U:%x  V:%x ",dispMVCOutIdx? "D" : "B",
							//	plat_priv,plat_priv->offset[0],plat_priv->offset[1],plat_priv->offset[2]);
							if(dispMVCOutIdx){
								if( omx_private->MVC_Base_addr0 != (OMX_U32)NULL ) {
									pOutputBuffer->nFilledLen = output_len;
									plat_priv->optional_info[10] = (unsigned int)omx_private->MVC_Base_addr0 ; 
									plat_priv->optional_info[11] = (unsigned int)omx_private->MVC_Base_addr1 ; 
									plat_priv->optional_info[12] = (unsigned int)omx_private->MVC_Base_addr2 ; 
									omx_private->MVC_Base_addr0 = (OMX_U32)NULL;
									omx_private->MVC_Base_addr1 = (OMX_U32)NULL;
									omx_private->MVC_Base_addr2 = (OMX_U32)NULL;
								}
							}
							else{
								pOutputBuffer->nFilledLen = 0;
								omx_private->MVC_Base_addr0 = (OMX_U32)plat_priv->offset[0] ; 
								omx_private->MVC_Base_addr1 = (OMX_U32)plat_priv->offset[1] ; 
								omx_private->MVC_Base_addr2 = (OMX_U32)plat_priv->offset[2] ; 
							}
						}
						else{
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
							if (omx_private->BufferType == DecoderMetadataPtr) {
								pOutputBuffer->nFilledLen = sizeof(VideoDecoderOutputMetaData);
							} else
#endif
							    pOutputBuffer->nFilledLen = output_len;
						}
						#else
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
						if (omx_private->BufferType == DecoderMetadataPtr) {
							pOutputBuffer->nFilledLen = sizeof(VideoDecoderOutputMetaData);
						} else
#endif
							pOutputBuffer->nFilledLen = output_len;
						#endif
					}
					else
					{
			#if !defined(USE_EXTERNAL_BUFFER)
				#ifdef HARDWARE_CODEC 	/*ZzaU :: VPU output is seperated yuv420 include gap.*/
						TCC_PLATFORM_PRIVATE_PMEM_INFO *pmemInfoPtr;

						pmemInfoPtr = (TCC_PLATFORM_PRIVATE_PMEM_INFO *) pOutputBuffer->pOutputPortPrivate;
					#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
						memcpy(pmemInfoPtr->offset, pOutputBuffer->pBuffer, output_len);
					#else
						memcpy(pmemInfoPtr->offset, pOutputBuffer->pBuffer, sizeof(int)*6);
					#endif
				#endif
			#endif
						if ( omx_private->bThumbnailMode ) {
							CopyOutputFrame(openmaxStandComp, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_DECODED_BUFFER);
						}
						else
						{
#if defined(USE_EXTERNAL_BUFFER)
							//To support video-editor!!
	#ifndef OPENMAX1_2
						    CopyOutputFrame(openmaxStandComp, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_DISPLAY_BUFFER);
	#endif
#endif
						    pOutputBuffer->nFilledLen = outPort->sPortParam.format.video.nFrameWidth * outPort->sPortParam.format.video.nFrameHeight * 3/2;

						}
						DBUG_MSG("[VDEC-%d] ----------------------> New OutputBuffer!!", omx_private->pVideoDecodInstance.nVdec_Instance);
					}
	#endif
#else
					nSize = output_len;

					if(pOutputBuffer->nAllocLen < nSize) {
						LOGE( "Ouch!!!! Output buffer Alloc Len %ld less than Frame Size %ld\n",(int)pOutputBuffer->nAllocLen,nSize);
						return;
					}
					pOutputBuffer->nFilledLen += nSize;
#endif
					if(omx_private->bWFD_mode == OMX_TRUE 
                        && ((omx_private->bWFDDisplayLV >= 2 && omx_private->pVideoDecodInstance.gsVDecOutput.m_DecOutInfo.m_iNumOfErrMBs > 0) 
                            || (omx_private->bWFDErrorFrame == OMX_TRUE))) {
						ALOGI("[WFD] MB error Lv2");
						pOutputBuffer->nFilledLen = 0;
						omx_private->bWFDErrorFrame = OMX_FALSE;
					}
				}
				else
				{
					/** Few bytes may be left in the input buffer but can't generate one output frame.
					*  Request for new Input Buffer
					*/
					if(decode_result == 2){
						omx_private->isNewBuffer = 1;
					}
					pOutputBuffer->nFilledLen = 0;
				}
				break;

			case 3 :	// Display Output Failed, Decode Success
			case 1 :	// Display Output Failed, Decode Failed Due to Buffer Full
			default :
				break;
		}

		nOutputFilled = 1;
	}

	if(pOutputBuffer->nFilledLen > 0) {
		//decrease counter of output buffer to be displayed
		omx_private->nDelayedOutputBufferCount--;

		if( !omx_private->bThumbnailMode && omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType != GrallocPtr ){
			plat_priv->optional_info[5] = (unsigned int)(pOutputBuffer->nTimeStamp/1000);
			plat_priv->optional_info[7] = (unsigned int)(pOutputBuffer->nFlags & ~0x000000FF);
			plat_priv->optional_info[8] = omx_private->pVideoDecodInstance.gsVDecUserInfo.frame_rate/1000;
#ifdef OPENMAX1_2
			memcpy( pOutputBuffer->pBuffer, plat_priv, sizeof(TCC_PLATFORM_PRIVATE_PMEM_INFO) );
#endif
		}
//		LOGD("out frame %8d", (int)(pOutputBuffer->nTimeStamp/1000));
	}
	DPRINTF_DEC("[VDEC-%d] ----------------------> OutputBuffer len = %ld, %d us!!", omx_private->pVideoDecodInstance.nVdec_Instance, pOutputBuffer->nFilledLen, (int)(pOutputBuffer->nTimeStamp/1000));

#ifdef TS_ADJUSTMENT
	if(omx_private->bNeed_TS_adjustment && pOutputBuffer->nFilledLen > 0)
	{
		if(omx_private->uTS_new_base == 0)
		{
			nsecs_t nST_gap = systemTime(SYSTEM_TIME_MONOTONIC) - omx_private->nST_last_output;
			omx_private->uTS_new_base = omx_private->uTS_last_output + (OMX_TICKS)(nST_gap/1000);

			if(omx_private->uTS_new_base >= pOutputBuffer->nTimeStamp){
				omx_private->bTopDown_adjustmemt = OMX_TRUE;
//				omx_private->uTS_new_base += (omx_private->prev_interval);
				LOGE("[VDEC-%d] ========> Configure for TS-Adjustment TopDown(%d) : base(%8d)= last(%8d) + gap(%8d) /*+ (inter(%8d)*2)*/", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->bTopDown_adjustmemt,
						(int)(omx_private->uTS_new_base/1000), (int)(omx_private->uTS_last_output/1000), (int)(nST_gap/1000000), (int)(omx_private->prev_interval/1000));
			}
			else{
				omx_private->bTopDown_adjustmemt = OMX_FALSE;
//				omx_private->uTS_new_base -= (omx_private->prev_interval);
				LOGE("[VDEC-%d] ========> Configure for TS-Adjustment TopDown(%d) : base(%8d)= last(%8d) + gap(%8d) /*- (inter(%8d)*2)*/", omx_private->pVideoDecodInstance.nVdec_Instance, omx_private->bTopDown_adjustmemt,
						(int)(omx_private->uTS_new_base/1000), (int)(omx_private->uTS_last_output/1000), (int)(nST_gap/1000000), (int)(omx_private->prev_interval/1000));
			}
			omx_private->uTS_curr_output = omx_private->uTS_new_base;
		}
		else
		{
			if(omx_private->bTopDown_adjustmemt == OMX_TRUE)
				omx_private->uTS_curr_output += (omx_private->prev_interval - omx_private->prev_interval/3);
			else
				omx_private->uTS_curr_output += (omx_private->prev_interval + omx_private->prev_interval/3);
		}

		if(omx_private->bTopDown_adjustmemt == OMX_TRUE && (omx_private->uTS_curr_output < pOutputBuffer->nTimeStamp))
		{
			omx_private->bNeed_TS_adjustment = OMX_FALSE;
			omx_private->uTS_new_base = 0;
//			LOGE("[VDEC-%d] ========> END for TS-Adjustment!!", omx_private->pVideoDecodInstance.nVdec_Instance);
		}
		else if(omx_private->bTopDown_adjustmemt == OMX_FALSE&& (omx_private->uTS_curr_output > pOutputBuffer->nTimeStamp))
		{
			omx_private->bNeed_TS_adjustment = OMX_FALSE;
			omx_private->uTS_new_base = 0;
//			LOGE("[VDEC-%d] ========> END for TS-Adjustment!!", omx_private->pVideoDecodInstance.nVdec_Instance);
		}
		else
		{
			pOutputBuffer->nTimeStamp = omx_private->uTS_curr_output;
//			LOGE("[VDEC-%d] TS-Adjustment %8d", omx_private->pVideoDecodInstance.nVdec_Instance, (int)(pOutputBuffer->nTimeStamp/1000));
		}
	}
#endif

	return;
}

OMX_ERRORTYPE omx_videodec_component_SetParameter(
OMX_IN  OMX_HANDLETYPE hComponent,
OMX_IN  OMX_INDEXTYPE nParamIndex,
OMX_IN  OMX_PTR ComponentParameterStructure) {

  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_U32 portIndex;

  /* Check which structure we are being fed and make control its header */
  OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
	omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
  omx_base_video_PortType *port;
  if (ComponentParameterStructure == NULL) {
    return OMX_ErrorBadParameter;
  }

  DBUG_MSG("   Setting parameter 0x%x", nParamIndex);
  switch(nParamIndex) {
    case OMX_IndexParamPortDefinition:
      {
        eError = omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
        if(eError == OMX_ErrorNone) {
          OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*)ComponentParameterStructure;
          UpdateFrameSize (openmaxStandComp);
          portIndex = pPortDef->nPortIndex;
          port = (omx_base_video_PortType *)omx_private->ports[portIndex];
          port->sVideoParam.eColorFormat = port->sPortParam.format.video.eColorFormat;

		  omx_private->rectParm.nLeft 	= 0;
		  omx_private->rectParm.nTop 	= 0;
		  omx_private->rectParm.nWidth	= port->sPortParam.format.video.nFrameWidth;
		  omx_private->rectParm.nHeight	= port->sPortParam.format.video.nFrameHeight;
		  LOGI(" CropInfo %ld,%ld - %ldx%ld", omx_private->rectParm.nLeft, omx_private->rectParm.nTop, omx_private->rectParm.nWidth, omx_private->rectParm.nHeight);
        }
        break;
      }
    case OMX_IndexParamVideoPortFormat:
      {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
        pVideoPortFormat = ComponentParameterStructure;
        portIndex = pVideoPortFormat->nPortIndex;
        /*Check Structure Header and verify component state*/
        eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
        if(eError!=OMX_ErrorNone) {
          LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
          break;
        }

		if (pVideoPortFormat->nIndex != 0) {
            return OMX_ErrorNoMore;
        }

        if (portIndex <= 1) {
          port = (omx_base_video_PortType *)omx_private->ports[portIndex];
          memcpy(&port->sVideoParam, pVideoPortFormat, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
          omx_private->ports[portIndex]->sPortParam.format.video.eColorFormat = port->sVideoParam.eColorFormat;

          if (portIndex == 1) {
            switch(port->sVideoParam.eColorFormat) {
              case OMX_COLOR_FormatYUV420Planar :
                omx_private->eOutFramePixFmt = PIX_FMT_YUV420P;
                break;
              default:
			  case OMX_COLOR_FormatYUV420SemiPlanar :
				omx_private->eOutFramePixFmt = PIX_FMT_NV12;
                break;
            }
            UpdateFrameSize (openmaxStandComp);
          }
        } else {
          return OMX_ErrorBadPortIndex;
        }
        break;
      }
	case OMX_IndexParamVideoRv:
	{
		OMX_VIDEO_PARAM_RVTYPE *pVideoRv;
		pVideoRv = ComponentParameterStructure;
		portIndex = pVideoRv->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoRv, sizeof(OMX_VIDEO_PARAM_RVTYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		if (pVideoRv->nPortIndex == 0) {
			memcpy(&omx_private->pVideoRv, pVideoRv, sizeof(OMX_VIDEO_PARAM_RVTYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
		break;
	}
    case OMX_IndexParamVideoH263:
	{
		OMX_VIDEO_PARAM_H263TYPE *pVideoH263;
		pVideoH263 = ComponentParameterStructure;
		portIndex = pVideoH263->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		if (pVideoH263->nPortIndex == 0) {
			memcpy(&omx_private->pVideoH263, pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
	break;
	}
    case OMX_IndexParamVideoAvc:
	{
		OMX_VIDEO_PARAM_AVCTYPE *pVideoAvc;
		pVideoAvc = ComponentParameterStructure;
		portIndex = pVideoAvc->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		memcpy(&omx_private->pVideoAvc, pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
		break;
	}
    case OMX_IndexParamVideoMpeg4:
	{
		OMX_VIDEO_PARAM_MPEG4TYPE *pVideoMpeg4;
		pVideoMpeg4 = ComponentParameterStructure;
		portIndex = pVideoMpeg4->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		if (pVideoMpeg4->nPortIndex == 0) {
			memcpy(&omx_private->pVideoMpeg4, pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
		break;
	}
    case OMX_IndexParamVideoWmv:
	{
		OMX_VIDEO_PARAM_WMVTYPE *pVideoWmv;
		pVideoWmv = ComponentParameterStructure;
		portIndex = pVideoWmv->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		if (pVideoWmv->nPortIndex == 0) {
			memcpy(&omx_private->pVideoWmv, pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
		break;
	}
/*
    case OMX_IndexParamVideoDivx:
	{
		OMX_VIDEO_PARAM_DIVXTYPE *pVideoWmv;
		pVideoDivx = ComponentParameterStructure;
		portIndex = pVideoWmv->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoDivx, sizeof(OMX_VIDEO_PARAM_DIVXTYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		if (pVideoDivx->nPortIndex == 0) {
			memcpy(&omx_private->pVideoWmv, pVideoWmv, sizeof(OMX_VIDEO_PARAM_DIVXTYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
		break;
	}
*/
    case OMX_IndexParamVideoMpeg2:
	{
		OMX_VIDEO_PARAM_MPEG2TYPE *pVideoMpeg2;
		pVideoMpeg2 = ComponentParameterStructure;
		portIndex = pVideoMpeg2->nPortIndex;
		eError = omx_base_component_ParameterSanityCheck(hComponent, portIndex, pVideoMpeg2, sizeof(OMX_VIDEO_PARAM_MPEG2TYPE));
		if(eError!=OMX_ErrorNone) {
			LOGE( "In %s Parameter Check Error=%x\n",__func__,eError);
			break;
		}
		if (pVideoMpeg2->nPortIndex == 0) {
			memcpy(&omx_private->pVideoWmv, pVideoMpeg2, sizeof(OMX_VIDEO_PARAM_MPEG2TYPE));
		} else {
			return OMX_ErrorBadPortIndex;
		}
		break;
	}
    case OMX_IndexParamStandardComponentRole:
	{
		OMX_PARAM_COMPONENTROLETYPE *pComponentRole;
		pComponentRole = ComponentParameterStructure;
		if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_RV_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingRV;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_H263_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingH263;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_H264_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingAVC;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_MPEG4_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMPEG4;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_TCC_WMV_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingWMV;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_TCC_WMV12_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingWMV_1_2;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_DIVX_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingDIVX;
		}else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_MPEG2_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMPEG2;
		} else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_SORENSON_H263_ROLE)) {
#ifdef OPENMAX1_2
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingSorenson;
#else
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingFLV1;
#endif
		} else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_MJPEG_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMJPEG;
		} else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_AVS_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingAVS;
		} else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_VP8_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingVP8;
		} else if (!strcmp((char *)pComponentRole->cRole, VIDEO_DEC_MVC_ROLE)) {
			omx_private->pVideoDecodInstance.video_coding_type = OMX_VIDEO_CodingMVC;
		} else {
			return OMX_ErrorBadParameter;
		}
		SetInternalVideoParameters(openmaxStandComp);
		break;
	}
	case OMX_IndexVendorVideoExtraData:
	{
		OMX_VENDOR_EXTRADATATYPE* pExtradata;
		pExtradata = (OMX_VENDOR_EXTRADATATYPE*)ComponentParameterStructure;
		//printf("%s %d Extradata[%d]  \n",__func__,__LINE__,pExtradata->nPortIndex);
		if (pExtradata->nPortIndex <= 1) {
			/** copy the extradata in the codec context private structure */
			//				memcpy(&omx_private->videoinfo, pExtradata->pData, sizeof(rm_video_info));
			//report_rm_videoinfo(&omx_private->videoinfo);
		} else {
			return OMX_ErrorBadPortIndex;
		}
	}
	break;

	case OMX_IndexParamCommonDeblocking:
	{
		break;
	}

#ifdef ANDROID_USE_GRALLOC_BUFFER
	case OMX_IndexUseNativeBuffers:
	{
		EnableAndroidNativeBuffersParams *pParamNativeBuffer = NULL;

		pParamNativeBuffer = (EnableAndroidNativeBuffersParams* )ComponentParameterStructure;
		if(pParamNativeBuffer->enable == OMX_TRUE)
		{
			LOGI("######################## Use GrallocPtr mode #########################");
			omx_private->gralloc_info.PortBuffers[pParamNativeBuffer->nPortIndex].BufferType = GrallocPtr;
			omx_private->gralloc_info.PortBuffers[pParamNativeBuffer->nPortIndex].IsBuffer2D = OMX_TRUE;
		}
	}
	break;
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
	case OMX_IndexDecoderStoreMetadatInBuffers:
	{
		StoreMetaDataInBuffersParams *pStoreMetaData = NULL;

		pStoreMetaData = (StoreMetaDataInBuffersParams* )ComponentParameterStructure;
		if(pStoreMetaData->bStoreMetaData == OMX_TRUE)
		{
			omx_private->BufferType = DecoderMetadataPtr;
			UpdateFrameSize (openmaxStandComp);
			LOGI("######################## Use MetadataPtr mode #########################");
		}
	}
	break;
#endif
#endif

    default: /*Call the base component function*/
      return omx_base_component_SetParameter(hComponent, nParamIndex, ComponentParameterStructure);
  }

  #ifdef RING_BUFFER_MODULE_ENABLE
  omx_vpudec_component_SetParameter(openmaxStandComp, nParamIndex, ComponentParameterStructure);
  #endif

  return eError;
}

OMX_ERRORTYPE omx_videodec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure) {

  omx_base_video_PortType *port;
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  OMX_COMPONENTTYPE *openmaxStandComp = hComponent;
  omx_videodec_component_PrivateType* omx_private = openmaxStandComp->pComponentPrivate;
  if (ComponentParameterStructure == NULL) {
    return OMX_ErrorBadParameter;
  }
  DBUG_MSG("   Getting parameter 0x%x", nParamIndex);
  /* Check which structure we are being fed and fill its header */
  switch(nParamIndex) {
    case OMX_IndexParamVideoInit:
      if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_PORT_PARAM_TYPE))) != OMX_ErrorNone) {
        break;
      }
      memcpy(ComponentParameterStructure, &omx_private->sPortTypesParam[OMX_PortDomainVideo], sizeof(OMX_PORT_PARAM_TYPE));
      break;
    case OMX_IndexParamVideoPortFormat:
      {
        OMX_VIDEO_PARAM_PORTFORMATTYPE *pVideoPortFormat;
        pVideoPortFormat = ComponentParameterStructure;
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE))) != OMX_ErrorNone) {
          break;
        }

		if (pVideoPortFormat->nIndex != 0) {
            return OMX_ErrorNoMore;
        }

        if (pVideoPortFormat->nPortIndex <= 1) {
          port = (omx_base_video_PortType *)omx_private->ports[pVideoPortFormat->nPortIndex];
          memcpy(pVideoPortFormat, &port->sVideoParam, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
        } else {
          return OMX_ErrorBadPortIndex;
        }
        break;
      }
    case OMX_IndexParamVideoRv:
      {
        OMX_VIDEO_PARAM_RVTYPE *pVideoRv;
        pVideoRv = ComponentParameterStructure;
        if (pVideoRv->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_RVTYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoRv, &omx_private->pVideoRv, sizeof(OMX_VIDEO_PARAM_RVTYPE));
        break;
      }
    case OMX_IndexParamVideoH263:
      {
        OMX_VIDEO_PARAM_H263TYPE *pVideoH263;
        pVideoH263 = ComponentParameterStructure;
        if (pVideoH263->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_H263TYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoH263, &omx_private->pVideoH263, sizeof(OMX_VIDEO_PARAM_H263TYPE));
        break;
      }
    case OMX_IndexParamVideoAvc:
      {
        OMX_VIDEO_PARAM_AVCTYPE * pVideoAvc;
        pVideoAvc = ComponentParameterStructure;
        if (pVideoAvc->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_AVCTYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoAvc, &omx_private->pVideoAvc, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
        break;
      }
    case OMX_IndexParamVideoMpeg4:
      {
        OMX_VIDEO_PARAM_MPEG4TYPE *pVideoMpeg4;
        pVideoMpeg4 = ComponentParameterStructure;
        if (pVideoMpeg4->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoMpeg4, &omx_private->pVideoMpeg4, sizeof(OMX_VIDEO_PARAM_MPEG4TYPE));
        break;
      }
    case OMX_IndexParamVideoWmv:
      {
        OMX_VIDEO_PARAM_WMVTYPE *pVideoWmv;
        pVideoWmv = ComponentParameterStructure;
        if (pVideoWmv->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_WMVTYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoWmv, &omx_private->pVideoWmv, sizeof(OMX_VIDEO_PARAM_WMVTYPE));
        break;
      }
/*
    case OMX_IndexParamVideoDivx:
      {
        OMX_VIDEO_PARAM_DIVXTYPE *pVideoDivx;
        pVideoDivx = ComponentParameterStructure;
        if (pVideoDivx->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_DIVXTYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoDivx, &omx_private->pVideoWmv, sizeof(OMX_VIDEO_PARAM_DIVXTYPE));
        break;
      }
*/
    case OMX_IndexParamVideoMpeg2:
      {
        OMX_VIDEO_PARAM_MPEG2TYPE *pVideoMpeg2;
        pVideoMpeg2 = ComponentParameterStructure;
        if (pVideoMpeg2->nPortIndex != 0) {
          return OMX_ErrorBadPortIndex;
        }
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_VIDEO_PARAM_MPEG2TYPE))) != OMX_ErrorNone) {
          break;
        }
        memcpy(pVideoMpeg2, &omx_private->pVideoMpeg2, sizeof(OMX_VIDEO_PARAM_MPEG2TYPE));
        break;
      }
    case OMX_IndexParamStandardComponentRole:
      {
        OMX_PARAM_COMPONENTROLETYPE * pComponentRole;
        pComponentRole = ComponentParameterStructure;
        if ((eError = checkHeader(ComponentParameterStructure, sizeof(OMX_PARAM_COMPONENTROLETYPE))) != OMX_ErrorNone) {
          break;
        }
        if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingRV) {
          strcpy((char *)pComponentRole->cRole, VIDEO_DEC_RV_ROLE);
        }else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingH263) {
          strcpy((char *)pComponentRole->cRole, VIDEO_DEC_H263_ROLE);
	 	}else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC) {
          strcpy((char *)pComponentRole->cRole, VIDEO_DEC_H264_ROLE);
        }else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4) {
          strcpy((char *)pComponentRole->cRole, VIDEO_DEC_MPEG4_ROLE);
        }else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV) {
          strcpy((char *)pComponentRole->cRole, VIDEO_DEC_TCC_WMV_ROLE);
        }else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV_1_2) {
          strcpy((char *)pComponentRole->cRole, VIDEO_DEC_TCC_WMV12_ROLE);
		}else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingDIVX) {
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_DIVX_ROLE);
		}else if (omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG2) {
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_MPEG2_ROLE);
#ifdef OPENMAX1_2
	    } else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingSorenson ) {
#else
	    } else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingFLV1 ) {
#endif
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_SORENSON_H263_ROLE);
		} else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingMJPEG) {
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_MJPEG_ROLE);
		} else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingAVS) {
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_AVS_ROLE);
		} else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingVP8) {
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_VP8_ROLE);
		} else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingMVC) {
			strcpy((char *)pComponentRole->cRole, VIDEO_DEC_MVC_ROLE);
		}else {
          strcpy((char *)pComponentRole->cRole,"\0");
        }
        break;
      }

	case OMX_IndexParamVideoProfileLevelQuerySupported:
	{
		OMX_VIDEO_PARAM_PROFILELEVELTYPE *profileLevel = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *) ComponentParameterStructure;
		VIDEO_PROFILE_LEVEL_TYPE* pProfileLevel = NULL;
		OMX_U32 nNumberOfProfiles = 0;

		if (profileLevel->nPortIndex != 0) {
		    LOGE("Invalid port index: %ld", profileLevel->nPortIndex);
		    return OMX_ErrorUnsupportedIndex;
		}

		/* Choose table based on compression format */
		switch(omx_private->ports[OMX_BASE_FILTER_INPUTPORT_INDEX]->sPortParam.format.video.eCompressionFormat)
		{
			case OMX_VIDEO_CodingH263:
				pProfileLevel = SupportedH263ProfileLevels;
				nNumberOfProfiles = sizeof(SupportedH263ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
				break;
			case OMX_VIDEO_CodingMPEG4:
				pProfileLevel = SupportedMPEG4ProfileLevels;
				nNumberOfProfiles = sizeof(SupportedMPEG4ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
				break;
			case OMX_VIDEO_CodingAVC:
				pProfileLevel = SupportedAVCProfileLevels;
				nNumberOfProfiles = sizeof(SupportedAVCProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
				break;
			case OMX_VIDEO_CodingMPEG2:
				pProfileLevel = SupportedMPEG2ProfileLevels;
				nNumberOfProfiles = sizeof(SupportedMPEG2ProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
				break;
			case OMX_VIDEO_CodingVP8:
				pProfileLevel = SupportedVPXProfileLevels;
				nNumberOfProfiles = sizeof(SupportedVPXProfileLevels) / sizeof (VIDEO_PROFILE_LEVEL_TYPE);
				break;
			default:
				return OMX_ErrorBadParameter;
		}

#ifdef OPENMAX1_2
		if (profileLevel->nIndex >= nNumberOfProfiles - 1) {
		    return OMX_ErrorNoMore;
		}
		profileLevel->eProfile = pProfileLevel[profileLevel->nIndex].nProfile;
		profileLevel->eLevel = pProfileLevel[profileLevel->nIndex].nLevel;
#else
		if (profileLevel->nProfileIndex >= nNumberOfProfiles - 1) {
		    return OMX_ErrorNoMore;
		}
		profileLevel->eProfile = pProfileLevel[profileLevel->nProfileIndex].nProfile;
		profileLevel->eLevel = pProfileLevel[profileLevel->nProfileIndex].nLevel;
#endif

		return OMX_ErrorNone;
	}
	break;

    case OMX_IndexParamCommonDeblocking:
      {
        break;
      }

#ifdef ANDROID_USE_GRALLOC_BUFFER
	case OMX_IndexAndroidNativeBufferUsage:
	{
		GetAndroidNativeBufferUsageParams *pNativeBuffUsage = NULL;

		pNativeBuffUsage = (GetAndroidNativeBufferUsageParams*)ComponentParameterStructure;
		if(omx_private->gralloc_info.PortBuffers[pNativeBuffUsage->nPortIndex].BufferType == GrallocPtr)
		{
			pNativeBuffUsage->nUsage = GRALLOC_USAGE_HW_RENDER;
			eError = OMX_ErrorNone;
		}
	}
	break;
#endif

    default: /*Call the base component function*/
    {
		eError = omx_base_component_GetParameter(hComponent, nParamIndex, ComponentParameterStructure);
#ifdef ANDROID_USE_GRALLOC_BUFFER
		if( nParamIndex == OMX_IndexParamPortDefinition )
		{
			OMX_PARAM_PORTDEFINITIONTYPE *pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*)ComponentParameterStructure;

			pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
			if(omx_private->gralloc_info.PortBuffers[pPortDef->nPortIndex].BufferType == GrallocPtr)
			{
		#ifdef USE_YUV420SP_DEFAULT
				if(1)
		#else
				if(omx_private->blocalPlaybackMode)
		#endif
					pPortDef->format.video.eColorFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;
				else
					pPortDef->format.video.eColorFormat = HAL_PIXEL_FORMAT_YV12;
				DBUG_MSG("pPortDef->format.video.eColorFormat(0x%x)", pPortDef->format.video.eColorFormat);
			}
			else
			{
				pPortDef->format.video.eColorFormat = OMX_COLOR_FormatYUV420Planar;
			}

			if( omx_private->bSecuredInBufferMode == OMX_TRUE && pPortDef->nPortIndex == OMX_DirInput )
			{
				port = (omx_base_video_PortType *)omx_private->ports[pPortDef->nPortIndex];
				if( omx_private->mUmpSecuredInPmap.size > 0){
					port->sPortParam.nBufferSize = pPortDef->nBufferSize = omx_private->mUmpSecuredInPmap.size / pPortDef->nBufferCountActual;
				}

				//For WFD display.
				if( omx_private->bWFDSyncMode) 
                {
					port->sPortParam.nBufferCountActual = port->sPortParam.nBufferCountMin = 1;
					pPortDef->nBufferCountActual = pPortDef->nBufferCountMin = 1;
				}
			}
		}
		else if ( nParamIndex == OMX_IndexParamVideoPortFormat )
		{
	        OMX_VIDEO_PARAM_PORTFORMATTYPE *pPortParam = (OMX_VIDEO_PARAM_PORTFORMATTYPE*)ComponentParameterStructure;

			pPortParam = (OMX_VIDEO_PARAM_PORTFORMATTYPE *)ComponentParameterStructure;
			if(omx_private->gralloc_info.PortBuffers[pPortParam->nPortIndex].BufferType == GrallocPtr)
			{
		#ifdef USE_YUV420SP_DEFAULT
				if(1)
		#else
				if(omx_private->blocalPlaybackMode)
		#endif
					pPortParam->eColorFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;
				else
					pPortParam->eColorFormat = HAL_PIXEL_FORMAT_YV12;
				DBUG_MSG("pPortParam->eColorFormat(0x%x)", pPortParam->eColorFormat);
			}
			else
			{
				pPortParam->eColorFormat = OMX_COLOR_FormatYUV420Planar;
			}
		}
#endif
	}
	break;
  }
  return eError;
}

OMX_ERRORTYPE omx_videodec_component_MessageHandler(OMX_COMPONENTTYPE* openmaxStandComp,internalRequestMessageType *message) {
  omx_videodec_component_PrivateType* omx_private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;
  OMX_ERRORTYPE err;
  OMX_STATETYPE eCurrentState = omx_private->state;

  DBUG_MSG("In %s\n", __func__);

  if (message->messageType == OMX_CommandStateSet){
    if ((message->messageParam == OMX_StateExecuting ) && (omx_private->state == OMX_StateIdle)) {
      if (!omx_private->pVideoDecodInstance.avcodecReady) {
        err = omx_videodec_component_LibInit(omx_private);
        if (err != OMX_ErrorNone) {
          return OMX_ErrorNotReady;
        }
        omx_private->pVideoDecodInstance.avcodecReady = OMX_TRUE;
      }
    }
    else if ((message->messageParam == OMX_StateIdle ) && (omx_private->state == OMX_StateLoaded)) {
      err = omx_videodec_component_Initialize(openmaxStandComp);
      if(err!=OMX_ErrorNone) {
        LOGE( "In %s Video Decoder Init Failed Error=%x\n",__func__,err);
        return err;
      }
    } else if ((message->messageParam == OMX_StateLoaded) && (omx_private->state == OMX_StateIdle)) {
      err = omx_videodec_component_Deinit(openmaxStandComp);
      if(err!=OMX_ErrorNone) {
        LOGE( "In %s Video Decoder Deinit Failed Error=%x\n",__func__,err);
        return err;
      }
    }
  }
  // Execute the base message handling
  err =  omx_base_component_MessageHandler(openmaxStandComp,message);

  if (message->messageType == OMX_CommandStateSet){
   if ((message->messageParam == OMX_StateIdle  ) && (eCurrentState == OMX_StateExecuting || eCurrentState == OMX_StatePause)) {
#ifndef HAVE_ANDROID_OS	// ZzaU:: to sync call-sequence with opencore!!
      if (omx_private->pVideoDecodInstance.avcodecReady) {
        omx_videodec_component_LibDeinit(omx_private);
        omx_private->pVideoDecodInstance.avcodecReady = OMX_FALSE;
      }
#endif
    }
  }

  // flush all ports to start new stream
  if (message->messageType == OMX_CommandFlush && message->messageParam == OMX_ALL) {
	  omx_private->bAllPortsFlushed = OMX_TRUE;
  }

  return err;
}
OMX_ERRORTYPE omx_videodec_component_ComponentRoleEnum(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_U8 *cRole,
  OMX_IN OMX_U32 nIndex) {

	OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_videodec_component_PrivateType* omx_private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;

	if (nIndex == 0) {
		if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingRV) {
			strcpy((char *)cRole, VIDEO_DEC_RV_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingH263) {
			strcpy((char *)cRole, VIDEO_DEC_H263_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingAVC) {
			strcpy((char *)cRole, VIDEO_DEC_H264_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG4) {
			strcpy((char *)cRole, VIDEO_DEC_MPEG4_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV) {
			strcpy((char *)cRole, VIDEO_DEC_TCC_WMV_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingWMV_1_2) {
			strcpy((char *)cRole, VIDEO_DEC_TCC_WMV12_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingDIVX) {
			strcpy((char *)cRole, VIDEO_DEC_DIVX_ROLE);
		}
		else if(omx_private->pVideoDecodInstance.video_coding_type == OMX_VIDEO_CodingMPEG2) {
			strcpy((char *)cRole, VIDEO_DEC_MPEG2_ROLE);
#ifdef OPENMAX1_2
		}else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingSorenson ) {
#else
		}else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingFLV1 ) {
#endif
			strcpy((char *)cRole, VIDEO_DEC_SORENSON_H263_ROLE);
		}else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingMJPEG) {
			strcpy((char *)cRole, VIDEO_DEC_MJPEG_ROLE);
		}else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingAVS) {
			strcpy((char *)cRole, VIDEO_DEC_AVS_ROLE);
		}else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingVP8) {
			strcpy((char *)cRole, VIDEO_DEC_VP8_ROLE);
		}else if (omx_private->pVideoDecodInstance.video_coding_type ==OMX_VIDEO_CodingMVC) {
			strcpy((char *)cRole, VIDEO_DEC_MVC_ROLE);
		}
	} else{
		return OMX_ErrorUnsupportedIndex;
	}

	return OMX_ErrorNone;
}

OMX_ERRORTYPE omx_videodec_component_SetConfig(
  OMX_HANDLETYPE hComponent,
  OMX_INDEXTYPE nIndex,
  OMX_PTR pComponentConfigStructure) {

  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_VENDOR_EXTRADATATYPE* pExtradata;

  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_videodec_component_PrivateType* omx_private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;

  if (pComponentConfigStructure == NULL) {
    return OMX_ErrorBadParameter;
  }
  DBUG_MSG("   Setting configuration %i\n", nIndex);
  /* Check which structure we are being fed and fill its header */
  switch (nIndex) {
    case OMX_IndexVendorVideoExtraData :
		pExtradata = (OMX_VENDOR_EXTRADATATYPE*)pComponentConfigStructure;
		if (pExtradata->nPortIndex <= 1) {
			/** copy the extradata in the codec context private structure */
			omx_private->extradata_size = (OMX_U32)pExtradata->nDataSize;
			if(omx_private->extradata_size > 0)
			{
				if(omx_private->extradata) {
					TCC_free(omx_private->extradata);
				}
				omx_private->extradata = (unsigned char *)TCC_malloc((int)pExtradata->nDataSize*sizeof(char));
				//_memcpy(omx_private->extradata,(unsigned char*)(pExtradata->pData),pExtradata->nDataSize);
			}
			else
			{
				DBUG_MSG("extradata size is 0 !!!\n");
			}
		}
		else
		{
			return OMX_ErrorBadPortIndex;
		}

      break;

#ifndef OPENMAX1_2
    case OMX_IndexConfigVideoPlayDirection :
	  omx_private->bPlayDirection = *(OMX_BOOL*)pComponentConfigStructure;
      break;

	case OMX_IndexConfigVideoOutputKeyFrameOnly:
		omx_private->bDecIndexOutput = *(OMX_BOOL*)pComponentConfigStructure;
		if( omx_private->bDecIndexOutput == OMX_FALSE )
			omx_private->bWaitNewBuffer = OMX_FALSE;
	break;
#endif

	case OMX_IndexVendorThumbnailMode:
		DBUG_MSG("this is thumbnail mode");
		omx_private->bThumbnailMode = *((OMX_BOOL *)pComponentConfigStructure);
		break;

	case OMX_IndexDecoderSecuredInBuffers:
		LOGI("this is SecuredInBuffers mode (size:%d, fd:%d)", omx_private->mUmpSecuredInPmap.size, omx_private->mTmem_fd);
		omx_private->bWFDSyncMode = *((OMX_BOOL *)pComponentConfigStructure);
		omx_private->bSecuredInBufferMode = OMX_TRUE;

		if(omx_private->mUmpSecuredInPmap.size == 0 || omx_private->mTmem_fd <= 0)
			omx_private->bSecuredInBufferMode = OMX_FALSE;
		break;

    default: // delegate to superclass
      return omx_base_component_SetConfig(hComponent, nIndex, pComponentConfigStructure);
  }

  #ifdef RING_BUFFER_MODULE_ENABLE
  omx_vpudec_component_SetConfig(openmaxStandComp, nIndex, pComponentConfigStructure);
  #endif

  return err;
}

OMX_ERRORTYPE omx_videodec_component_GetConfig(
  OMX_HANDLETYPE hComponent,
  OMX_INDEXTYPE nIndex,
  OMX_PTR pComponentConfigStructure) {

  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_VENDOR_EXTRADATATYPE* pExtradata;

  OMX_COMPONENTTYPE *openmaxStandComp = (OMX_COMPONENTTYPE *)hComponent;
	omx_videodec_component_PrivateType* omx_private = (omx_videodec_component_PrivateType*)openmaxStandComp->pComponentPrivate;

  if (pComponentConfigStructure == NULL) {
    return OMX_ErrorBadParameter;
  }
  DBUG_MSG("   Getting configuration %i\n", nIndex);
  /* Check which structure we are being fed and fill its header */
  switch (nIndex) {
    case OMX_IndexConfigCommonOutputCrop:
		{
			OMX_CONFIG_RECTTYPE *rectParams = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;

			if (rectParams->nPortIndex != 1) {
			    return OMX_ErrorUndefined;
			}

			rectParams->nLeft 	= omx_private->rectParm.nLeft;
			rectParams->nTop 	= omx_private->rectParm.nTop;
			rectParams->nWidth 	= omx_private->rectParm.nWidth;
			rectParams->nHeight = omx_private->rectParm.nHeight;
	    }
		break;

	case OMX_IndexConfigCommonScale:
		{
			OMX_CONFIG_SCALEFACTORTYPE *scale = (OMX_CONFIG_SCALEFACTORTYPE *)pComponentConfigStructure;

			if (scale->nPortIndex != 1) {
			    return OMX_ErrorUndefined;
			}

			scale->xWidth  = omx_private->scale.xWidth;
			scale->xHeight = omx_private->scale.xHeight;
			DBUG_MSG("SAR OMX_IndexConfigScale: 0x%lx/0x%lx", scale->xWidth, scale->xHeight);
		}
		break;

    default: // delegate to superclass
		return omx_base_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
  }

  #ifdef RING_BUFFER_MODULE_ENABLE
  omx_vpudec_component_GetConfig(hComponent, nIndex, pComponentConfigStructure);
  #endif

  return err;
}

OMX_ERRORTYPE omx_videodec_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType) {

	DBUG_MSG("In  %s - %s \n",__func__, cParameterName);

	if(strcmp(cParameterName,"OMX.tcc.index.config.videoextradata") == 0) {
		*pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorVideoExtraData;
	}else if(strcmp(cParameterName, "OMX.TCC.index.ThumbnailMode") == 0){
		*pIndexType = (OMX_INDEXTYPE)OMX_IndexVendorThumbnailMode;
	}
#ifdef ANDROID_USE_GRALLOC_BUFFER
	else if(strcmp(cParameterName, "OMX.google.android.index.getAndroidNativeBufferUsage") == 0){
		*pIndexType = (OMX_INDEXTYPE)OMX_IndexAndroidNativeBufferUsage;
	}else if(strcmp(cParameterName, "OMX.google.android.index.enableAndroidNativeBuffers") == 0){
		*pIndexType = (OMX_INDEXTYPE) OMX_IndexUseNativeBuffers;
	}else if(strcmp(cParameterName, "OMX.google.android.index.useAndroidNativeBuffer2") == 0){
		*pIndexType = (OMX_INDEXTYPE) NULL;
	}else if (strcmp(cParameterName, "OMX.google.android.index.storeMetaDataInBuffers") == 0) {
#ifdef ANDROID_USE_STOREMETADATAINBUFFERS
		char useMetaData[PROPERTY_VALUE_MAX];
		property_get("tcc.video.use.metabuffer", useMetaData, "1");

		if (atoi(useMetaData) == 1) {
			*pIndexType = (OMX_INDEXTYPE) OMX_IndexDecoderStoreMetadatInBuffers;
		} else  {
			return OMX_ErrorBadParameter;
		}
#else
		return OMX_ErrorBadParameter;
#endif
	} 
#endif
	else if(strcmp(cParameterName, "OMX.TCC.index.InputBuffer.SecureMode") == 0){
		*pIndexType = (OMX_INDEXTYPE) OMX_IndexDecoderSecuredInBuffers;
	}
	else {
		LOGE("OMX_ErrorBadParameter  %s - %s \n", __func__, cParameterName);
		return OMX_ErrorBadParameter;
	}

	DBUG_MSG("Out(Index = 0x%x)  %s - %s \n", *pIndexType, __func__, cParameterName);
	return OMX_ErrorNone;
}

#ifdef HAVE_ANDROID_OS
OMX_ERRORTYPE OMX_ComponentInit(OMX_HANDLETYPE openmaxStandComp, OMX_STRING cComponentName, OMX_HANDLE_FUNC pHandleFunc)
{
	OMX_ERRORTYPE err = OMX_ErrorNone;

	err = omx_videodec_component_Constructor(openmaxStandComp,cComponentName);
#ifdef ANDROID_USE_GRALLOC_BUFFER
#ifndef ANDROID_USE_STOREMETADATAINBUFFERS

	if(err == OMX_ErrorNone)
	{
		hw_module_t const* module;
  		omx_videodec_component_PrivateType* omx_private = ((OMX_COMPONENTTYPE*)openmaxStandComp)->pComponentPrivate;

        err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &module);
        if (err == 0)
		{
            omx_private->gralloc_info.grallocModule = (gralloc_module_t const *)module;
        }
		else
		{
            LOGE("ERROR: can't load gralloc using hw_get_module(%s) => err[0x%x]", GRALLOC_HARDWARE_MODULE_ID, err);
			err = OMX_ErrorInsufficientResources;
		}
	}
#endif
#endif
	return err;
}

#endif
