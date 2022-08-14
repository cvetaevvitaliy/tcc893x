/**

  Copyright (C) 2007-2008  STMicroelectronics
  Copyright (C) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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

*/

#ifndef _OMX_VIDEODEC_COMPONENT_H_
#define _OMX_VIDEODEC_COMPONENT_H_


#include <omx_base_filter.h>
#include <tcc_video_common.h>
#include "OMX_TCC_Index.h"
#include <vpu_ringbuffer_struct.h>

#define	OMX_DXB_VIDEOCOMPONENT_INPORT_MAIN	0
#define	OMX_DXB_VIDEOCOMPONENT_INPORT_SUB	1
#define	OMX_DXB_VIDEOCOMPONENT_OUTPORT	2
#define	OMX_DXB_VIDEOCOMPONENT_OUTPORT_SUB	3

#define TS_TIMESTAMP_CORRECTION

#define VPU_REQUEST_BUFF_COUNT 	31 // <- RV problem : reference frame limitation!!

#define CDMX_PTS_MODE		0	//! Presentation Timestamp (Display order)
#define CDMX_DTS_MODE		1	//! Decode Timestamp (Decode order)

#define CVDEC_DISP_INFO_INIT	0
#define CVDEC_DISP_INFO_UPDATE	1
#define CVDEC_DISP_INFO_GET		2
#define CVDEC_DISP_INFO_RESET	3

typedef enum
{
	OMX_IndexParamVideoSkipFrames = 0x7E000001,
	OMX_IndexParamVideoSetStart,  			/**< reference: OMX_VIDEO_PARAM_STARTTYPE */	
	OMX_IndexParamVideoSetPause,  			/**< reference: OMX_VIDEO_PARAM_PAUSETYPE */	
	OMX_IndexParamInterlaceInfo,
}TCC_DXBVIDEO_OMX_INDEXVENDORTYPE;

typedef enum
{
	TCC_DXBVIDEO_OMX_Codec_H264 = 0x100,
	TCC_DXBVIDEO_OMX_Codec_MPEG2,
	TCC_DXBVIDEO_OMX_Codec_MAX
}TCC_DXBVIDEO_OMX_CODECTYPE;

typedef enum
{
	TCC_DXBVIDEO_OMX_Dec_None = 0,
	TCC_DXBVIDEO_OMX_Dec_Stop,
	TCC_DXBVIDEO_OMX_Dec_Start,
	TCC_DXBVIDEO_OMX_Dec_Pause,
	TCC_DXBVIDEO_OMX_Dec_MAX
}TCC_DXBVIDEO_OMX_DECTYPE;

typedef enum
{
	TCC_DXBVIDEO_DISPLAY_MAIN =0,
	TCC_DXBVIDEO_DISPLAY_SUB
}TCC_DXBVIDEO_OMX_DISPLAYTYPE;

typedef enum
{
	TCC_DXBVIDEO_ISSUPPORT_JAPAN =0,
	TCC_DXBVIDEO_ISSUPPORT_BRAZIL
}TCC_DXBVIDEO_OMX_COUNTRYCODE;

typedef struct OMX_VIDEO_PARAM_STARTTYPE {
    OMX_U32 nDevID;              
    OMX_U32 nVideoFormat;              
    OMX_BOOL bStartFlag;              
} OMX_VIDEO_PARAM_STARTTYPE;

typedef struct OMX_VIDEO_DEFINITIONTYPE {
    OMX_U32 nFrameWidth;
    OMX_U32 nFrameHeight;
    OMX_U32 nAspectRatio;
    OMX_U32 nFrameRate;
    OMX_U32 bProgressive;
} OMX_VIDEO_DEFINITIONTYPE;

typedef struct OMX_VIDEO_DEFINITIONTYPE_DUAL_DECODE {
    OMX_U32 nFrameWidth;
    OMX_U32 nFrameHeight;
    OMX_U32 nAspectRatio;
    OMX_U32 nFrameRate;
    OMX_U32 MainDecoderID;
    OMX_U32 nSubFrameWidth;
    OMX_U32 nSubFrameHeight;
    OMX_U32 nSubAspectRatio;
    OMX_U32 SubDecoderID;
    OMX_U32 DisplayID;
    OMX_U32 bProgressive;
} OMX_VIDEO_DEFINITIONTYPE_DUAL_DECODE;

typedef struct OMX_VIDEO_PARAM_PAUSETYPE {
    OMX_BOOL bPauseFlag;              
} OMX_VIDEO_PARAM_PAUSETYPE;

typedef struct dec_disp_info_ctrl_t
{
	int       m_iTimeStampType;							   //! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS)
	int       m_iStdType;								   //! STD type
	int       m_iFmtType;								   //! Formater Type

	int       m_iUsedIdxPTS;							   //! total number of decoded index for PTS
	int       m_iRegIdxPTS[32];							   //! decoded index for PTS
	void     *m_pRegInfoPTS[32];						   //! side information of the decoded index for PTS

	int       m_iDecodeIdxDTS;							   //! stored DTS index of decoded frame
	int       m_iDispIdxDTS;							   //! display DTS index of DTS array
	int       m_iDTS[32];								   //! Decode Timestamp (decoding order)

	int       m_iFrameRate;                                //! Frame Rate
	int       m_iFrameDuration;                            //! frame-duration
	int       m_iFrameDurationFactor;                      //! frame-duration factor
	int       m_iLatestPTS;                                //! last output frame time stamp
	int       m_Reserved;
} dec_disp_info_ctrl_t;

typedef struct dec_disp_info_t
{
	int       m_iFrameType;								   //! Frame Type

	int       m_iTimeStamp;								   //! Time Stamp
	int       m_iextTimeStamp;							   //! TR(RV)

	int       m_iPicStructure;							   //! PictureStructure
	int       m_iM2vFieldSequence;						   //! Field sequence(MPEG2) 
	int       m_iFrameDurationFactor;                      //! frame-duration factor

	int       m_iFrameSize;								   //! Frame size

	int       m_iTopFieldFirst;                            //! Top Field First
	int       m_iIsProgressive;                            //! Interlace information :: 0:interlace, 1:progressive	
	int       m_iWidth;                                    //! Frame Width
	int       m_iHeight;                                   //! Frame Height
	int       m_iFrameRate;                                //! Frame Rate
} dec_disp_info_t;

typedef struct dec_disp_info_input_t
{
	int       m_iFrameIdx;								   //! Display frame buffer index for CVDEC_DISP_INFO_UPDATE command
	//! Decoded frame buffer index for CVDEC_DISP_INFO_GET command
	int       m_iStdType;								   //! STD type for CVDEC_DISP_INFO_INIT
	int       m_iTimeStampType;							   //! TS(Timestamp) type (0: Presentation TS(default), 1:Decode TS) for CVDEC_DISP_INFO_INIT
	int       m_iFmtType;								   //! Formater Type specification
	int       m_iFrameRate;
} dec_disp_info_input_t;

typedef struct mpeg2_pts_ctrl
{
	int       m_iLatestPTS;
	int       m_iPTSInterval;
	int       m_iRamainingDuration;
} mpeg2_pts_ctrl;

typedef struct VideoStartInfo
{
	OMX_U32		nDevID;
	OMX_U32		nState;
	OMX_U32		nFormat;
	pthread_mutex_t mutex;
} VideoStartInfo;

#ifdef TS_TIMESTAMP_CORRECTION
typedef struct ts_pts_ctrl{
	int m_iLatestPTS;
	int m_iPTSInterval; //[usec]
	int m_iRamainingDuration;
	int m_iRealPTS;
	int m_iInterpolationCount;
} ts_pts_ctrl;
//ts_pts_ctrl gsTSPtsInfo;
#endif

typedef struct VideoSubFunctionFlag{
	OMX_BOOL	SupportFieldDecoding;		//for field decoding(ex:IPTV TrickMode )
	OMX_BOOL	SupportIFrameSearchMode;	//for i-frame search mode 
	OMX_BOOL	SupprotUsingErrorMB;		//if ErrorMb is not "0", decoding frame is broken.
	OMX_BOOL	SupprotDirectDsiplay;		//unsig at first display, (Reduce for IPTV Channel change time) 
}VideoSubFunctionFlag ;

typedef struct BrokenFrameInfo
{
	unsigned int 	broken_frame_cnt;
	unsigned int	broken_frame_index[32];
	unsigned char	broken_frame_iPicType[32];
	unsigned int 	broken_frame_iNumOfErrMBs[32];
} BrokenFrameInfo;

typedef struct BFrameSkipInfo
{
	unsigned int 	bframe_skipcnt;
	unsigned int	bframe_skipindex[32];
} BFrameSkipInfo;

typedef struct FirstFrameDispInfo
{
	unsigned int 	TopFrame_DecStatus;
	unsigned int	BottomFrame_DecStatus;
	unsigned int	FirstFrame_DispStatus;
} FirstFrameDispInfo;

typedef struct IPTVActiveModeInfo
{
	OMX_U32	 IPTV_PLAYMode;
	OMX_U32 IPTV_Activemode;
	OMX_U32 IPTV_Old_Activemode;
	OMX_U32 IPTV_Mode_change_cnt; 
}IPTVActiveModeInfo;

typedef struct _VIDEO_DECOD_INSTANCE_ {
	/** @param avcodecReady boolean flag that is true when the video coded has been initialized */
	OMX_BOOL avcodecReady;
	/** @param video_coding_type Field that indicate the supported video format of video decoder */
	OMX_U32 video_coding_type;
	queue_t *pVPUDisplayedIndexQueue;
	OMX_U8	container_type;
	OMX_U32  bitrate_mbps;
	vdec_input_t gsVDecInput;
	vdec_output_t gsVDecOutput;
	vdec_init_t gsVDecInit;
	/** @param Android Only flag */
	OMX_BOOL avcodecInited;
	OMX_BOOL  isVPUClosed;
	cdmx_info_t cdmx_info;
	OMX_U32 video_dec_idx;
	dec_disp_info_ctrl_t dec_disp_info_ctrl;
	dec_disp_info_t dec_disp_info[32];
	dec_disp_info_input_t dec_disp_info_input;
	OMX_BOOL bVideoStarted;
	OMX_BOOL bVideoPaused;
	VideoStartInfo stVideoStart;
	OMX_U32 guiDisplayBufFullCount;
	OMX_PTR pUserData_List_Array;
	OMX_PTR pVdec_Instance;
#ifdef TS_TIMESTAMP_CORRECTION
	ts_pts_ctrl gsTSPtsInfo;
#endif
	OMX_U32 out_index;
	OMX_U32 in_index;
	OMX_U32 buffer_unique_id;
	OMX_S32 Display_Buff_ID[VPU_BUFF_COUNT];
	OMX_U32 Display_index[VPU_BUFF_COUNT];
	OMX_U32 Send_Buff_ID[VPU_REQUEST_BUFF_COUNT];
	OMX_U32 used_fifo_count;
	OMX_U32 max_fifo_cnt;
	OMX_S32 iDecod_status;

	cdk_func_t *gspfVDec;

	ringbuff_state_t stRingBuffState;
	OMX_U32 ulFlags;
	OMX_U32 ulDecoderUID;
} _VIDEO_DECOD_INSTANCE_;

typedef struct dual_dec_info_t
{
	int	iDecod_Instance;
	int 	iDecod_status;
	int 	iDecod_Support_Country;
	unsigned int uDecod_time;
}dual_dec_info_t;

/** Video Decoder component private structure.
  */
/**
 * Pixel format. Notes:
 *
 * PIX_FMT_RGB32 is handled in an endian-specific manner. A RGBA
 * color is put together as:
 *  (A << 24) | (R << 16) | (G << 8) | B
 * This is stored as BGRA on little endian CPU architectures and ARGB on
 * big endian CPUs.
 *
 * When the pixel format is palettized RGB (PIX_FMT_PAL8), the palettized
 * image data is stored in AVFrame.data[0]. The palette is transported in
 * AVFrame.data[1] and, is 1024 bytes long (256 4-byte entries) and is
 * formatted the same as in PIX_FMT_RGB32 described above (i.e., it is
 * also endian-specific). Note also that the individual RGB palette
 * components stored in AVFrame.data[1] should be in the range 0..255.
 * This is important as many custom PAL8 video codecs that were designed
 * to run on the IBM VGA graphics adapter use 6-bit palette components.
 */
 
DERIVEDCLASS(omx_videodec_component_PrivateType, omx_base_filter_PrivateType)
#define omx_videodec_component_PrivateType_FIELDS omx_base_filter_PrivateType_FIELDS \
  /** @param semaphore for avcodec access syncrhonization */\
  tsem_t* avCodecSyncSem; \
  /** @param inputCurrBuffer Field that stores pointer of the current input buffer position */ \
  OMX_U8* inputCurrBuffer; \
  /** @param inputCurrLength Field that stores current input buffer length in bytes */ \
  OMX_U32 inputCurrLength; \
  /** @param isFirstBuffer Field that the buffer is the first buffer */ \
  OMX_S32 isFirstBuffer; \
  _VIDEO_DECOD_INSTANCE_ *pVideoDecodInstance; \
  mpeg2_pts_ctrl gsMPEG2PtsInfo; \
  OMX_BOOL gHDMIOutput; \
  OMX_U32 gVideo_FrameRate; \
  OMX_U32 guiSkipBframeEnable; \
  OMX_U32 guiSkipBFrameNumber; \
  OMX_S32 iInterlaceInfo; \
  OMX_S32 g_hFb; \
  OMX_S32 iVsyncMode; \
  OMX_S8 iDisplayID; \
  OMX_S8 Resolution_Change; \
  OMX_VIDEO_DEFINITIONTYPE stVideoDefinition;\
  OMX_VIDEO_DEFINITIONTYPE_DUAL_DECODE stVideoDefinition_dual; \
  BrokenFrameInfo stbroken_frame_info;\
  FirstFrameDispInfo stfristframe_dsipInfo;\
  BFrameSkipInfo stbframe_skipinfo;\  
  IPTVActiveModeInfo stIPTVActiveModeInfo;\
  VideoSubFunctionFlag stVideoSubFunFlag;\
  dual_dec_info_t stPreVideoInfo;
ENDCLASS(omx_videodec_component_PrivateType)

/* Component private entry points declaration */
OMX_ERRORTYPE omx_videodec_component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_ring_videodec_component_Constructor(OMX_COMPONENTTYPE *openmaxStandComp,OMX_STRING cComponentName);
OMX_ERRORTYPE omx_ring_videodec_component_Destructor(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_ring_videodec_component_Initialize(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_ring_videodec_component_Deinit(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_ring_videodec_component_MessageHandler(OMX_COMPONENTTYPE*,internalRequestMessageType*);

OMX_ERRORTYPE omx_ring_videodec_MPEG2component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_ring_videodec_H264component_Init(OMX_COMPONENTTYPE *openmaxStandComp);
OMX_ERRORTYPE omx_ring_VideoDec_Default_ComponentInit(OMX_COMPONENTTYPE *openmaxStandComp);

void omx_ring_videodec_component_BufferMgmtCallback(
  OMX_COMPONENTTYPE *openmaxStandComp,
  OMX_BUFFERHEADERTYPE* inputbuffer,
  OMX_BUFFERHEADERTYPE* outputbuffer);

OMX_ERRORTYPE omx_ring_videodec_component_GetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_INOUT OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_ring_videodec_component_SetParameter(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_INDEXTYPE nParamIndex,
  OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE omx_ring_videodec_component_ComponentRoleEnum(
  OMX_IN OMX_HANDLETYPE hComponent,
  OMX_OUT OMX_U8 *cRole,
  OMX_IN OMX_U32 nIndex);

OMX_ERRORTYPE omx_ring_videodec_component_SetConfig(
  OMX_HANDLETYPE hComponent,
  OMX_INDEXTYPE nIndex,
  OMX_PTR pComponentConfigStructure);

OMX_ERRORTYPE omx_ring_videodec_component_GetExtensionIndex(
  OMX_IN  OMX_HANDLETYPE hComponent,
  OMX_IN  OMX_STRING cParameterName,
  OMX_OUT OMX_INDEXTYPE* pIndexType);

#endif
