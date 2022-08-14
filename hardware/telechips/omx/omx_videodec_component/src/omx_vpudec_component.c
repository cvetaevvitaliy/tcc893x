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
#ifdef RING_BUFFER_MODULE_ENABLE

#include <omxcore.h>
#include <omx_base_video_port.h>
#include <omx_videodec_component.h>
#include <omx_vpudec_component.h>
#include<OMX_Video.h>

#include <lcd_resolution.h>

#include <mach/TCC_VPU_CODEC.h>
#include <vdec.h>
#include <cdk.h>
#include "cdk_android.h"
#include "tcc_video_config_data.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <mach/tccfb_ioctrl.h>
#include <cutils/properties.h>
#include <mach/tcc_video_private.h>

#ifdef MOVE_HW_OPERATION
#include <mach/tcc_mem_ioctl.h>
#endif

#ifdef OPENMAX1_2
#include <OMX_VideoExt.h>
#endif

//#define LOG_NDEBUG 0
#define LOG_TAG	"OMX_VPU_DEC"
#include <utils/Log.h>

#ifdef ANDROID_USE_GRALLOC_BUFFER
#define USE_EXTERNAL_BUFFER 1
#else
#define USE_EXTERNAL_BUFFER 0
#endif

#define USE_YUV420SP_DEFAULT // from omx_videodec_component.c


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Options
//
//

/*----------------------------------------------------------*/
/*        THESE ARE MUST BE '0' WHEN IT IS RELEASED         */
/*----------------------------------------------------------*/
#define DEBUG_MESSAGE_ENABLE            0  // 
#define VPU_PROFILER_ENABLE             0  // VPU profiler on/off
#define VPU_LOGGER_ENABLE               0  // VPU call logger on/off
#define CHECK_RING_STATUS_AFTER_UPDATE  0  // re-checking after ring-buffer status update
#define USE_MAX_VPU_CLOCK               0  // MAX VPU clock
#define TEST_CONDITION_ENABLE           0  // test condition setting
/*------------------------------------------------------------*/

/* build options */
#define AVC_DECODE_ONLY_MODE_ENABLE           1
#define FLUSH_RING_BEFORE_UPDATE_TIME         0
#define UPDATE_WRITE_PTR_WITH_ALIGNED_LENGTH  1
#define WRITE_PTR_ALIGN_BYTES                 512
#define USE_AVAILABLE_SIZE_FROM_VPU           0
#define RESOLUTION_SWITCHING_SUPPORT          1
#define MVC_PROCESS_ENABLE                    1

/* test condition setting */
#if TEST_CONDITION_ENABLE
	#define AVC_CROP_ENABLE                 1 //
	#define SINGLE_FRAME_INPUT_ENABLE       0 //
#else //TEST_CONDITION_ENABLE
	#define AVC_CROP_ENABLE                 1
	#define SINGLE_FRAME_INPUT_ENABLE       0
#endif //TEST_CONDITION_ENABLE


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Debugging tools
//
//
#define CONDITIONAL_LOG_ENABLE        1  // property conditional debug message

#define LOG_INPUT_BUFF_INFO           1  // input buffer information log - LOG_INPUT()
#define LOG_FRAME_SKIP_MODE           1  // VPU skip mode information log - LOG_SKIP()
#define LOG_DECODE_STATUS             1  // decode status log - LOG_DEC()
#define LOG_OUTPUT_STATUS             1  // output status log - LOG_OUT()
#define LOG_INPUT_INFO_QUEUE          1  // input information queue internal log - LOG_IIQUE()
#define LOG_INPUT_INFO_MANAGEMENT     1  // input information management log - LOG_IIMGE()
#define LOG_RING_BUFFER_STATUS        1  // VPU ring-buffer helper internal log - LOG_RING()
#define LOG_FEEDING_STATUS            1  // VPU feeder internal log - LOG_FEED()
#define LOG_DISPLAY_INFO_MANAGER      1  // display information manager log - LOG_DIMGR()
#define LOG_DISPLAY_INDEX_QUEUE       1  // display index queue internal log - LOG_IDXQUE()
#define LOG_BUFFER_CLEAR_STATUS       1  // buffer clear log - LOG_BUFCLR()
#define LOG_GRALLOC_MODE              1  // gralloc mode log - LOG_GRA()

#define CHECK_VPULOG_ENABLE()
#define CREATE_LOG_TAG()
#define REMOVE_LOG_TAG()
#define INFO(...)  ALOGI("[INFO] "__VA_ARGS__)
#define ERROR(...)  ALOGE(__VA_ARGS__)
#define ASSERT(cond)  // assertion
#define DBGM(...)     // debug message
#define DBGL()        // line printing
#define DBGV(v)       // value printing
#define DBGT()        // line/time printing
#define CLOG(TAG, ...)

#define LOG_INPUT(...)
#define LOG_SKIP(...)
#define LOG_DEC(...)
#define LOG_OUT(...)
#define LOG_IIQUE(...)
#define LOG_IIMGE(...)
#define LOG_RING(...)
#define LOG_FEED(...)
#define LOG_DIMGR(...)
#define LOG_IDXQUE(...)
#define LOG_BUFCLR(...)
#define LOG_GRA(...)

#if DEBUG_MESSAGE_ENABLE

static char *gs_pszLogTag[65536];

#undef CREATE_LOG_TAG
#define CREATE_LOG_TAG() {\
	int tid = gettid();\
	if( gs_pszLogTag[tid] == NULL ) {\
		gs_pszLogTag[tid] = (char*)malloc(32);\
		sprintf(gs_pszLogTag[tid], LOG_TAG"_#%d", tid);\
	}\
}

#undef REMOVE_LOG_TAG
#define REMOVE_LOG_TAG() {\
	int tid = gettid(); \
	if( gs_pszLogTag[tid] ) { \
		free(gs_pszLogTag[tid]);\
		gs_pszLogTag[tid] = NULL; \
	}\
}

#define VPU_ALOGV(...)    ((void)ALOG(LOG_VERBOSE, gs_pszLogTag[gettid()], __VA_ARGS__))
#define VPU_ALOGD(...)    ((void)ALOG(LOG_DEBUG,   gs_pszLogTag[gettid()], __VA_ARGS__))
#define VPU_ALOGI(...)    ((void)ALOG(LOG_INFO,    gs_pszLogTag[gettid()], __VA_ARGS__))
#define VPU_ALOGW(...)    ((void)ALOG(LOG_WARN,    gs_pszLogTag[gettid()], __VA_ARGS__))
#define VPU_ALOGE(...)    ((void)ALOG(LOG_ERROR,   gs_pszLogTag[gettid()], __VA_ARGS__))

#undef INFO(...)
#define INFO(...)	VPU_ALOGI("[INFO] "__VA_ARGS__)

#undef ERROR(...)
#define ERROR(...)  VPU_ALOGE(__VA_ARGS__)

#undef DBGM
#define DBGM(...)   VPU_ALOGD("[VPUDBG] "__VA_ARGS__)

#undef DBGL
#define DBGL()      VPU_ALOGE("[DBG_LINE] %d line", __LINE__)

#undef DBGV
#define DBGV(v)     VPU_ALOGE("[DBG_VAL][%4d line] 0x%08X (%d)", __LINE__, v, v)


#if CONDITIONAL_LOG_ENABLE

static int gs_iDbgmsgEnable;

#undef CHECK_VPULOG_ENABLE
#define CHECK_VPULOG_ENABLE() {\
	char prop[PROPERTY_VALUE_MAX];\
	property_get("tcc.vpu.clog.enable", prop, "0");\
	gs_iDbgmsgEnable = (int)(prop[0] - '0');\
}

#define VPU_CLOG(CLOG_TAG, ...) \
if( gs_iDbgmsgEnable ){\
	char value[PROPERTY_VALUE_MAX];\
	property_get("tcc.logctrl.vpu."#CLOG_TAG, value, "0");\
	if( value[0] != '0' ) \
		__android_log_buf_print(LOG_ID_SYSTEM, (int)value[0]-'0', gs_pszLogTag[gettid()], "[LOG_"#CLOG_TAG"] "__VA_ARGS__);\
}

#undef CLOG
#define CLOG(TAG, ...) {\
	char value[PROPERTY_VALUE_MAX];\
	property_get("tcc.logctrl.gp."#TAG, value, "0");\
	if( value[0] != '0' ) __android_log_buf_print(LOG_ID_SYSTEM, (int)value[0]-'0', LOG_TAG, __VA_ARGS__);\
}

#define VPU_LOGV    VPU_CLOG
#define VPU_LOGD    VPU_CLOG
#define VPU_LOGI    VPU_CLOG
#define VPU_LOGW    VPU_CLOG
#define VPU_LOGE    VPU_CLOG

#else //CONDITIONAL_LOG_ENABLE

#define VPU_LOGV(CLOG_TAG, ...)	VPU_ALOGV("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGD(CLOG_TAG, ...)	VPU_ALOGD("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGI(CLOG_TAG, ...)	VPU_ALOGI("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGW(CLOG_TAG, ...)	VPU_ALOGW("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define VPU_LOGE(CLOG_TAG, ...)	VPU_ALOGE("[LOG_"#CLOG_TAG"] "__VA_ARGS__)
#define CLOG(TAG, ...) VPU_ALOGD(["LOG_"#TAG"] "__VA_ARGS__)

#endif


static OMX_S64 g_llPrevLogTime;
#undef DBGT
#define DBGT() {\
	struct timeval time;\
	OMX_S64 curr_time;\
	gettimeofday(&time, NULL);\
	curr_time = (OMX_S64)1000000*time.tv_sec + time.tv_usec;\
	VPU_ALOGD("[DBG_TIME][%4d line] latency: %ll8d us", __LINE__, curr_time-g_llPrevLogTime);\
	g_llPrevLogTime = curr_time;\
}

#if LOG_INPUT_BUFF_INFO
#undef LOG_INPUT
#define LOG_INPUT(...)   VPU_LOGV(INPUT, __VA_ARGS__)
#endif

#if LOG_FRAME_SKIP_MODE
#undef LOG_SKIP
#define LOG_SKIP(...)    VPU_LOGI(SKIP, __VA_ARGS__)
#endif

#if LOG_DECODE_STATUS
#undef LOG_DEC
#define LOG_DEC(...)     VPU_LOGI(DEC, __VA_ARGS__)
#endif

#if LOG_OUTPUT_STATUS
#undef LOG_OUT
#define LOG_OUT(...)     VPU_LOGD(OUT, __VA_ARGS__)
#endif

#if LOG_INPUT_INFO_QUEUE
#undef LOG_IIQUE
#define LOG_IIQUE(...)   VPU_LOGD(IIQUE, __VA_ARGS__)
#endif

#if LOG_INPUT_INFO_MANAGEMENT
#undef LOG_IIMGE
#define LOG_IIMGE(...)   VPU_LOGI(IIMGE, __VA_ARGS__)
#endif

#if LOG_RING_BUFFER_STATUS
#undef LOG_RING
#define LOG_RING(...)    VPU_LOGI(RING, __VA_ARGS__)
#endif

#if LOG_FEEDING_STATUS
#undef LOG_FEED
#define LOG_FEED(...)    VPU_LOGD(FEED, __VA_ARGS__)
#endif

#if LOG_DISPLAY_INFO_MANAGER
#undef LOG_DIMGR
#define LOG_DIMGR(...)   VPU_LOGD(DIMGR, __VA_ARGS__)
#endif

#if LOG_DISPLAY_INDEX_QUEUE
#undef LOG_IDXQUE
#define LOG_IDXQUE(...)  VPU_LOGI(IDXQUE, __VA_ARGS__)
#endif

#if LOG_BUFFER_CLEAR_STATUS
#undef LOG_BUFCLR
#define LOG_BUFCLR(...)  VPU_LOGI(BUFCLR, __VA_ARGS__)
#endif

#if LOG_GRALLOC_MODE
#undef LOG_GRA
#define LOG_GRA(...)     VPU_LOGV(GRALLOC, __VA_ARGS__)
#endif

#undef ASSERT
#define ASSERT(cond) \
	if(!(cond)){\
		ERROR("ASSERTION FAILED!"); \
		ERROR("  - Position: %s (%d line) : %s()", __FILE__, __LINE__, __FUNCTION__); \
		ERROR("  - Expression: " #cond); \
	}


#if VPU_PROFILER_ENABLE
#undef VPU_LOGGER_ENABLE
#define VPU_LOGGER_ENABLE 0
int
profile_vdec_vpu(
    int    nOpCode, int*   pHandle, void*  pParam1, void*  pParam2, void*  pParam3,
    char*  pszProfileTag, int    nCallLine
	)
{
	OMX_S32 ret = 0;
	struct timeval start;
	struct timeval end;

	gettimeofday(&start, NULL);
	ret = vdec_vpu(nOpCode, pHandle, pParam1, pParam2, pParam3);
	gettimeofday(&end, NULL);

	INFO("[PROFILE][%3d][%s] time: %lld us", 
		nCallLine, pszProfileTag,
		((OMX_S64)1000000*end.tv_sec + end.tv_usec)-((OMX_S64)1000000*start.tv_sec + start.tv_usec));

	return ret;
}
#define vdec_vpu(opcode, ptr_handle, ptr_param1, ptr_param2, ptr_param3) \
		profile_vdec_vpu(opcode, ptr_handle, ptr_param1, ptr_param2, ptr_param3, #opcode, __LINE__)
#endif //VPU_PROFILER_ENABLE


#if VPU_LOGGER_ENABLE
#undef VPU_PROFILER_ENABLE
#define VPU_PROFILER_ENABLE 0
int
logger_vdec_vpu(
    int    nOpCode, int*   pHandle, void*  pParam1, void*  pParam2, void*  pParam3,
    char*  pszOpCode, int    nCallLine
	)
{
	OMX_S32 ret = 0;
	struct timeval start;
	struct timeval end;

	ERROR("[VPU_CALL][%3d][%s] - IN", nCallLine, pszOpCode);
	ret = vdec_vpu(nOpCode, pHandle, pParam1, pParam2, pParam3);
	ERROR("[VPU_CALL][%3d][%s] - OUT (ret: %d)", nCallLine, pszOpCode, ret);

	return ret;
}
#define vdec_vpu(opcode, ptr_handle, ptr_param1, ptr_param2, ptr_param3) \
		logger_vdec_vpu(opcode, ptr_handle, ptr_param1, ptr_param2, ptr_param3, #opcode, __LINE__)
#endif

#endif //DEBUG_MESSAGE_ENABLE



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Definitions
//
//
#define SEQ_HEADER_INIT_ERROR_MAX		300
#define SEQ_HEADER_SEARCH_MAX           300
#define VSYNC_DISP_CHECK_COUNT_MAX		100
#define INPUT_INFO_QUEUE_INIT_SIZE		64
#define THUMBNAIL_DECODING_FAIL_MAX		20
#define SEQHEAD_LENGTH_MAX              1024*500
#define EMPTY_SIZE_MIN                  1024*2   // VPU ring-buffer margine between read ptr and write ptr
#define FEED_LIMIT_TIMEDIFF_INIT        300000 // init as 300ms
#define FRAME_RATE_FOR_FEED_LIMIT_MIN   15000  // 15 fps


/* I-frame search */
#define VPU_OUTPUT_FAIL_MAX             500	   // after I frame search
#define VPU_OUTPUT_IDR_FAIL_MAX         20000  // after I frame search


/* AVC decode only mode */
#define DECODE_ONLY_SKIP_MAX            VPU_BUFF_COUNT*3  // decoded frame skip max
#define DECODE_ONLY_ERROR_MAX			60                // decoded frame skip max
#define AVC_IDR_SLICE_SCAN_MAX          20


/* OMX private */
typedef omx_vpudec_component_PrivateType	vpudec_private_t;

#define GET_VIDEODEC_PRIVATE(__p_omx_component) \
		((omx_videodec_component_PrivateType*)(__p_omx_component)->pComponentPrivate)

#define GET_VPUDEC_PRIVATE(__p_omx_component) \
		(omx_vpudec_component_PrivateType*)GET_VIDEODEC_PRIVATE(__p_omx_component)->pVpuRingPrivate


/* state flags */
#define STATE_WAIT_LIB_INIT             0x00000001
#define STATE_VPU_INITIALIZED           0x00000002
#define STATE_WAIT_FIRST_SYNCFRAME      0x00000004
#define STATE_IN_VPU_RESET_PROCESS      0x00000008
#define STATE_CLEAR_DISPLAY_BUFFER      0x00000010
#define STATE_DECODE_BUFFER_OUTPUT      0x00000020
#define STATE_SYNCFRAME_FORCED          0x00000040
#define STATE_DECODE_ONLY_USED          0x00000080
#define STATE_DECODE_ONLY               0x00000100
#define STATE_SEQUENCE_HEADER_FOUND     0x00000200
#define STATE_THUMBNAIL_MODE            0x00000400
#define STATE_FRAME_RATE_UPDATE_ENABLE  0x00000800
#define STATE_IDR_SLICE_SCAN            0x00001000
#define STATE_MVC_OUTPUT_ENABLE         0x00002000
#define STATE_SKIP_OUTPUT_B_FRAME       0x00004000
#define STATE_VPU_BUFFER_PROTECTION     0x00008000
#define STATE_HDMI_OUTPUT               0x00010000
#define STATE_VSYNC_OUTPUT              0x00020000	// !STATE_LCD_OUTPUT
#define STATE_GRALLOC_MODE              0x00040000
#define STATE_GRALLOC_RESET             0x00080000
#define STATE_REMOTE_PLAYER_MODE        0x00100000
#define STATE_LOCAL_FILE_PLAY_MODE      0x00200000
#define STATE_RESOLUTION_CHANGING       0x00400000
#define STATE_READY_TO_RESET            0x00800000
#define STATE_BLACK_FRAME_OUTPUT_ONCE   0x01000000	// not used
#define STATE_16BIT_DRAM_MODE           0x02000000
#define STATE_INSUFFICIENT_BITSTREAM    0x04000000	// 
#define STATE_NOT_DEFINED0              0x08000000	// NOT DEFINED
#define STATE_NOT_DEFINED1              0x10000000	// NOT DEFINED
#define STATE_NOT_DEFINED2              0x20000000	// NOT DEFINED
#define STATE_NOT_DEFINED3              0x40000000	// NOT DEFINED
#define STATE_NOT_DEFINED4              0x80000000	// NOT DEFINED

#define SET_FLAG(__p_vpu_private, __flag)    ((__p_vpu_private)->ulFlags |= (__flag))
#define CLEAR_FLAG(__p_vpu_private, __flag)  ((__p_vpu_private)->ulFlags &= ~(__flag))
#define CHECK_FLAG(__p_vpu_private, __flag)  ((__p_vpu_private)->ulFlags & (__flag))


/* VPU decoding result flags */
#define DECODING_SUCCESS_FRAME    0x0001
#define DECODING_SUCCESS_FIELD    0x0002
#define DECODING_SUCCESS          0x0003
#define DISPLAY_OUTPUT_SUCCESS    0x0010
#define DECODED_OUTPUT_SUCCESS    0x0020
#define OUTPUT_SUCCESS            0x0030
#define DECODING_BUFFER_FULL      0x0100
#define RESOLUTION_CHANGED        0x1000

/* Resolution Change */
#define ERROR_INVALID_BUFFER_STATE      -1001
#define ERROR_INVALID_OUTPUT_FRAME      -1002
#define ERROR_SKIP_OUTPUT_FRAME         -1003


#if defined(TC_SECURE_MEMORY_COPY)
extern int
TC_SecureMemoryCopy(
	unsigned int paTarget,
	unsigned int paSource,
	unsigned int nSize
);

extern int
TC_SecureScanFilterSpace(
	unsigned int paRingBase,   //[IN] physical address
	int           nRingSize,   //[IN] offset
	int           nScanStart,  //[IN] offset
	int           nScanEnd,    //[IN] offset
	int           bIsAVC,      //[IN] boolean
	int          *plSpaceEnd   //[OUT] offset
);
#endif

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Debugging helper
//
//
static
char*
GetFrameTypeString(
	OMX_S32 lVideoType, 
	OMX_S32 lPictureType, 
	OMX_S32 lPictureStructure 
	)
{
	switch ( lVideoType )
	{
	case STD_MPEG2 :
		switch( lPictureType ) {
		case PIC_TYPE_I: return "I";
		case PIC_TYPE_P: return "P";
		case PIC_TYPE_B: return "B";
		default:         return "D"; //D_TYPE
		}

	case STD_MPEG4 :
		switch( lPictureType ) {
		case PIC_TYPE_I:     return "I ";
		case PIC_TYPE_P:     return "P ";
		case PIC_TYPE_B:     return "B ";
		case PIC_TYPE_B_PB:  return "pB"; //MPEG-4 Packed PB-frame
		default:             return "S "; //S_TYPE
		}

	case STD_VC1 :
		if( lPictureStructure == 3 ) {
			OMX_S32 tf_type = lPictureType >> 3;
			OMX_S32 bf_type = lPictureType & 0x7;
			if( tf_type ){
				switch( tf_type ) {
				case PIC_TYPE_I: return "TF_I   ";  //TOP_FIELD = I
				case PIC_TYPE_P: return "TF_P   ";  //TOP_FIELD = P
				case 2:          return "TF_BI  ";  //TOP_FIELD = BI_TYPE
				case 3:          return "TF_B   ";  //TOP_FIELD = B_TYPE
				case 4:          return "TF_SKIP";  //TOP_FIELD = SKIP_TYPE
				default:         return "TF_FORB"; //TOP_FIELD = FORBIDDEN
				}
			}
			if( bf_type ) {
				switch( tf_type ) {
				case PIC_TYPE_I: return "BF_I   ";  //TOP_FIELD = I
				case PIC_TYPE_P: return "BF_P   ";  //TOP_FIELD = P
				case 2:          return "BF_BI  ";  //TOP_FIELD = BI_TYPE
				case 3:          return "BF_B   ";  //TOP_FIELD = B_TYPE
				case 4:          return "BF_SKIP";  //TOP_FIELD = SKIP_TYPE
				default:         return "BF_FORB"; //TOP_FIELD = FORBIDDEN
				}
			}
		}
		else {
			#ifndef TCC_892X_INCLUDE
			lPictureType >>= 3;
			#endif
			switch( lPictureType ) {
			case PIC_TYPE_I:  return "I   ";
			case PIC_TYPE_P:  return "P   ";
			case 2:           return "BI  ";
			case 3:           return "B   ";
			case 4:           return "SKIP";
			default:          return "FORB"; //FORBIDDEN
			}
		}
	default:
		if( lPictureStructure ) {
			switch( lPictureType ) {
			case 0:  return "I/I";
			case 1:  return "I/P";
			case 2:  return "I/B";
			case 8:  return "P/I";
			case 9:  return "P/P";
			case 10: return "P/B";
			case 16: return "B/I";
			case 17: return "B/P";
			case 18: return "B/B";
			default: return "U/U";
			}
		}
		else
		{
			switch( lPictureType ) {
			case PIC_TYPE_I:  return "I";
			case PIC_TYPE_P:  return "P";
			case PIC_TYPE_B:  return "B";
			default:          return "U"; //Unknown
			}
		}
	}
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	Gralloc mode helper
//
//
#ifdef MOVE_HW_OPERATION

#ifdef USE_WMIXER_FOR_COPY
#include <mach/tcc_wmixer_ioctrl.h>
#include <mach/vioc_global.h>
#define COPY_DEVICE         "/dev/wmixer"
#else
#include <mach/tcc_scaler_ioctrl.h>
#define COPY_DEVICE         "/dev/scaler1"
#endif

#define TMEM_DEVICE			"/dev/tmem"

#include <fcntl.h>
#include <sys/poll.h>

static void GrallocSrcReset(vpudec_private_t *pstVpuPrivate);
static COPY_MODE SwSetBlackFrame(vpudec_private_t *pstVpuPrivate, OMX_U8 *pbyDst);
static COPY_MODE SwCopyRun(vpudec_private_t *pstVpuPrivate, OMX_U8 *pbySrcY, OMX_U8 *pbySrcCb, OMX_U8 *pbySrcCr, OMX_U8 *pbyDst);
static OMX_BOOL HwCopyConfig(vpudec_private_t *pstVpuPrivate, OMX_BOOL bCopyOnly);
static COPY_MODE HwCopyKickOff(vpudec_private_t *pstVpuPrivate, OMX_U8 *pbySrcY, OMX_U8 *pbySrcCb, OMX_U8 *pbySrcCr, OMX_U8 *pbyDst);
static COPY_MODE HwCopyWait(vpudec_private_t *pstVpuPrivate);
static OMX_BOOL IsValidGrallocBuffer(vpudec_private_t *pstVpuPrivate, OMX_PTR pGrallocBuff);
static OMX_BOOL IsGrallocBufferCopyRun(vpudec_private_t *pstVpuPrivate, OMX_BOOL bIsBufferValid);
static TCC_PLATFORM_PRIVATE_PMEM_INFO* GetPlatformPrivate(vpudec_private_t *pstVpuPrivate, OMX_PTR pGrallocBuff, OMX_BOOL bIsBufferValid);
static COPY_MODE GrallocCopyRun(vpudec_private_t *pstVpuPrivate, buffer_handle_t hBuffer, OMX_BOOL bBlackFrame);
static COPY_MODE GrallocCopyWait(vpudec_private_t *pstVpuPrivate, buffer_handle_t hBuffer);

static 
void 
GrallocSrcReset(
	vpudec_private_t   *pstVpuPrivate
	)
{
	pstVpuPrivate->stGrallocInfo.pDispOut[PA][0] = NULL;
	pstVpuPrivate->stGrallocInfo.pDispOut[PA][1] = NULL;
	pstVpuPrivate->stGrallocInfo.pDispOut[PA][2] = NULL;
	pstVpuPrivate->stGrallocInfo.pDispOut[VA][0] = NULL;
	pstVpuPrivate->stGrallocInfo.pDispOut[VA][1] = NULL;
	pstVpuPrivate->stGrallocInfo.pDispOut[VA][2] = NULL;
}

static
COPY_MODE
SwSetBlackFrame(
	vpudec_private_t   *pstVpuPrivate,
	OMX_U8             *pbyDst
	)
{
	OMX_S32 h_copy_dev = pstVpuPrivate->stGrallocInfo.hCopyDev;
	OMX_S32 framesize_y = pstVpuPrivate->stGrallocInfo.lFrameSizeY;
	OMX_S32 framesize_c = pstVpuPrivate->stGrallocInfo.lFrameSizeC;

	OMX_U8 *p_dst_y		= pbyDst;
	OMX_U8 *p_dst_cb	= p_dst_y + framesize_y;
	OMX_U8 *p_dst_cr	= p_dst_cb + framesize_c;

	memset(p_dst_y, 0, framesize_y);
	if(pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode)
	{
		memset(p_dst_cb, 0, framesize_c*2);
	}
	else
	{
		memset(p_dst_cb, 0, framesize_c);
		memset(p_dst_cr, 0, framesize_c);
	}

	return COPY_DONE;
}

static
COPY_MODE
SwCopyRun(
	vpudec_private_t   *pstVpuPrivate,
	OMX_U8             *pbySrcY, 
	OMX_U8             *pbySrcCb, 
	OMX_U8             *pbySrcCr,
	OMX_U8             *pbyDst
	)
{
	OMX_S32 h_copy_dev = pstVpuPrivate->stGrallocInfo.hCopyDev;
	OMX_S32 framesize_y = pstVpuPrivate->stGrallocInfo.lFrameSizeY;
	OMX_S32 framesize_c = pstVpuPrivate->stGrallocInfo.lFrameSizeC;

	OMX_U8 *p_dst_y		= pbyDst;
	OMX_U8 *p_dst_cb	= p_dst_y + framesize_y;
	OMX_U8 *p_dst_cr	= p_dst_cb + framesize_c;

	memcpy(p_dst_y, pbySrcY, framesize_y);
	if(pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode)
	{
		memcpy(p_dst_cb, pbySrcCb, framesize_c*2);
	}
	else
	{
		memcpy(p_dst_cb, pbySrcCb, framesize_c);
		memcpy(p_dst_cr, pbySrcCr, framesize_c);
	}

	return COPY_DONE;
}

static
OMX_BOOL
HwCopyConfig(
	vpudec_private_t   *pstVpuPrivate,
	OMX_BOOL            bCopyOnly
	)
{
#ifdef USE_WMIXER_FOR_COPY
	WMIXER_INFO_TYPE  *p_wmixer_param = (WMIXER_INFO_TYPE*)pstVpuPrivate->stGrallocInfo.pCopyParam;
#else
	SCALER_TYPE       *p_scaler_param = (SCALER_TYPE*)pstVpuPrivate->stGrallocInfo.pCopyParam;
#endif
	omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	OMX_S32 port_width = p_outport->sPortParam.format.video.nFrameWidth;
	OMX_S32 port_height = p_outport->sPortParam.format.video.nFrameHeight;

	OMX_U32 stride_y = ALIGNED_BUFF(port_width, 16);
	OMX_U32 stride_c = ALIGNED_BUFF(stride_y/2, 16);
	OMX_U32 framesize_y = ALIGNED_BUFF(stride_y * port_height, 64);
	OMX_U32 framesize_c = ALIGNED_BUFF(stride_c * port_height/2, 64);

	if( pstVpuPrivate->stGrallocInfo.hCopyDev < 0 ) {
		OMX_S32 h_copy_dev = open(COPY_DEVICE, O_RDWR);
		if(h_copy_dev < 0) {
			ERROR("HwCopyConfig() - device open failed (DEV: %s / ERR: %s)", COPY_DEVICE, strerror(errno));
			return OMX_FALSE;
		}
		pstVpuPrivate->stGrallocInfo.hCopyDev = h_copy_dev;
	}

	pstVpuPrivate->stGrallocInfo.lFrameSizeY = framesize_y;
	pstVpuPrivate->stGrallocInfo.lFrameSizeC = framesize_c;
	pstVpuPrivate->stGrallocInfo.lFrameSize  = port_height * (stride_y + stride_c);// + 100;
	pstVpuPrivate->stGrallocInfo.lStrideY    = stride_y;
	pstVpuPrivate->stGrallocInfo.lStrideC    = stride_y/2;

#ifdef USE_WMIXER_FOR_COPY

	// alloc copy parameter
	if( p_wmixer_param )
		memset(p_wmixer_param, 0, sizeof(WMIXER_INFO_TYPE));
	else {
		p_wmixer_param = (WMIXER_INFO_TYPE*)TCC_calloc(1, sizeof(WMIXER_INFO_TYPE));
		if( p_wmixer_param == NULL ) {
			ERROR("HwCopyConfig() - out of memory");
			return OMX_FALSE;
		}
		pstVpuPrivate->stGrallocInfo.pCopyParam = p_wmixer_param;
	}

	// response type
	p_wmixer_param->rsp_type   = WMIXER_INTERRUPT;

	// src format
	if(pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode)
		p_wmixer_param->src_fmt = VIOC_IMG_FMT_YUV420IL0;
	else {
		#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
		if(pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MJPG)
		{
			//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
			if(pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
				p_wmixer_param->src_fmt = VIOC_IMG_FMT_YUV422SEP;
			else
				p_wmixer_param->src_fmt = VIOC_IMG_FMT_YUV420SEP;
		}
		else
		#endif
			p_wmixer_param->src_fmt = VIOC_IMG_FMT_YUV420SEP;
	}

	// dst format
	if( bCopyOnly )
		p_wmixer_param->dst_fmt = p_wmixer_param->src_fmt;
	else 
	{
#ifdef USE_YUV420SP_DEFAULT
		p_wmixer_param->dst_fmt    = VIOC_IMG_FMT_YUV420IL0;
#else
		if( CHECK_FLAG(pstVpuPrivate, STATE_LOCAL_FILE_PLAY_MODE) )
			p_wmixer_param->dst_fmt    = VIOC_IMG_FMT_YUV420IL0;
		else
			p_wmixer_param->dst_fmt    = VIOC_IMG_FMT_YUV420SEP;
#endif
	}


	// resolution
	p_wmixer_param->img_width  = stride_y;
	p_wmixer_param->img_height = port_height;

#else

	// alloc copy parameter
	if( p_scaler_param )
		memset(p_scaler_param, 0, sizeof(SCALER_TYPE));
	else {
		p_scaler_param = (SCALER_TYPE*)TCC_calloc(1, sizeof(SCALER_TYPE));
		if( p_scaler_param == NULL ) {
			ERROR("HwCopyConfig() - out of memory");
			return OMX_FALSE;
		}
		pstVpuPrivate->stGrallocInfo.pCopyParam = p_scaler_param;
	}

	// response type
	p_scaler_param->responsetype = SCALER_INTERRUPT;

	// src format
	if(pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode)
		p_scaler_param->src_fmt	 = SCALER_YUV420_inter;
	else
	{
		#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
		if(pstVpuPrivate->gsVDecInit.m_iBitstreamFormat == STD_MJPG)
		{
			//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
			if(pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1)
				p_scaler_param->src_fmt = SCALER_YUV422_sp;
			else
				p_scaler_param->src_fmt = SCALER_YUV420_sp;
		}
		else
		#endif
			p_scaler_param->src_fmt = SCALER_YUV420_sp;
	}
	p_scaler_param->src_ImgWidth   = stride_y;
	p_scaler_param->src_ImgHeight  = port_height;
	p_scaler_param->src_winLeft    = 0;
	p_scaler_param->src_winTop     = 0;
	p_scaler_param->src_winRight   = p_scaler_param->src_winLeft + p_scaler_param->src_ImgWidth;
	p_scaler_param->src_winBottom  = p_scaler_param->src_winTop + p_scaler_param->src_ImgHeight;

	// dst format
	if( bCopyOnly )
		p_scaler_param->dest_fmt = p_scaler_param->src_fmt;
	else 
	{
#ifdef USE_YUV420SP_DEFAULT
		p_scaler_param->dest_fmt   = SCALER_YUV420_inter;
#else
		if( CHECK_FLAG(pstVpuPrivate, STATE_LOCAL_FILE_PLAY_MODE) )
			p_scaler_param->dest_fmt   = SCALER_YUV420_inter;
		else
			p_scaler_param->dest_fmt   = SCALER_YUV420_sp;
#endif
	}

	p_scaler_param->dest_ImgWidth  = stride_y;
	p_scaler_param->dest_ImgHeight = port_height;
	p_scaler_param->dest_winLeft   = 0;
	p_scaler_param->dest_winTop    = 0;
	p_scaler_param->dest_winRight  = p_scaler_param->dest_winLeft + p_scaler_param->dest_ImgWidth;
	p_scaler_param->dest_winBottom = p_scaler_param->dest_winTop + p_scaler_param->dest_ImgHeight;

#endif

	return OMX_TRUE;
}


static
COPY_MODE
HwCopyKickOff(
	vpudec_private_t   *pstVpuPrivate,
	OMX_U8             *pbySrcY, 
	OMX_U8             *pbySrcCb, 
	OMX_U8             *pbySrcCr,
	OMX_U8             *pbyDst
	)
{
	OMX_S32 h_copy_dev = pstVpuPrivate->stGrallocInfo.hCopyDev;

#ifdef USE_WMIXER_FOR_COPY

	WMIXER_INFO_TYPE  *p_wmixer_param = (WMIXER_INFO_TYPE*)pstVpuPrivate->stGrallocInfo.pCopyParam;

	// src
	p_wmixer_param->src_y_addr = (OMX_U32)pbySrcY;
	p_wmixer_param->src_u_addr = (OMX_U32)pbySrcCb;
	p_wmixer_param->src_v_addr = (OMX_U32)pbySrcCr;

	// dst
	p_wmixer_param->dst_y_addr = (OMX_U32)pbyDst;
	p_wmixer_param->dst_u_addr = p_wmixer_param->dst_y_addr + pstVpuPrivate->stGrallocInfo.lFrameSizeY;
	p_wmixer_param->dst_v_addr = p_wmixer_param->dst_u_addr + pstVpuPrivate->stGrallocInfo.lFrameSizeC;

	LOG_GRA("%s copy :: 0x%x-0x%x-0x%x -> 0x%x-0x%x-0x%x"
			, COPY_DEVICE
			, p_wmixer_param->src_y_addr
			, p_wmixer_param->src_u_addr
			, p_wmixer_param->src_v_addr
			, p_wmixer_param->dst_y_addr
			, p_wmixer_param->dst_u_addr
			, p_wmixer_param->dst_v_addr
			);

	// copy run
	if( ioctl(h_copy_dev, TCC_WMIXER_IOCTRL, p_wmixer_param) < 0 ) {
		ERROR("%s Out Error!", COPY_DEVICE);
		return COPY_FAILED;
	}

#else

	SCALER_TYPE       *p_scaler_param = (SCALER_TYPE*)pstVpuPrivate->stGrallocInfo.pCopyParam;

	// src
	p_scaler_param->src_Yaddr  = (OMX_U32)pbySrcY;
	p_scaler_param->src_Uaddr  = (OMX_U32)pbySrcCb;
	p_scaler_param->src_Vaddr  = (OMX_U32)pbySrcCr;

	// dst
	p_scaler_param->dest_Yaddr = (OMX_U32)pbyDst;
	p_scaler_param->dest_Uaddr = p_scaler_param->dest_Yaddr + pstVpuPrivate->stGrallocInfo.lFrameSizeY;
	p_scaler_param->dest_Vaddr = p_scaler_param->dest_Vaddr + pstVpuPrivate->stGrallocInfo.lFrameSizeC;

	LOG_GRA("%s copy :: 0x%x-0x%x-0x%x -> 0x%x-0x%x-0x%x)"
			, COPY_DEVICE
			, p_scaler_param->src_Yaddr
			, p_scaler_param->src_Uaddr
			, p_scaler_param->src_Vaddr
			, p_scaler_param->dest_Yaddr
			, p_scaler_param->dest_Uaddr
			, p_scaler_param->dest_Vaddr
			);

	// copy run
	if( ioctl(h_copy_dev, TCC_SCALER_IOCTRL, p_scaler_param) < 0 ) {
		ERROR("%s Out Error!", COPY_DEVICE);
		return COPY_FAILED;
	}

#endif

	return COPY_START;
}

static
COPY_MODE
HwCopyWait(
	vpudec_private_t   *pstVpuPrivate
	)
{
	OMX_S32 ret = 0;
	OMX_S32 h_copy_dev = pstVpuPrivate->stGrallocInfo.hCopyDev;
	struct pollfd poll_event = {h_copy_dev, POLLIN, 0};

	ret = poll(&poll_event, 1, 100);
	if (ret < 0) {
		ERROR("%s poll error", COPY_DEVICE);
		return COPY_FAILED;
	}else if (ret == 0) {
		ERROR("%s poll timeout", COPY_DEVICE);
		return COPY_FAILED;
	}else if (ret > 0) {
		if (poll_event.revents & POLLERR) {
			ERROR("%s poll POLLERR", COPY_DEVICE);
			return COPY_FAILED;
		}
	}

	return COPY_DONE;
}

static 
OMX_BOOL 
IsValidGrallocBuffer(
	vpudec_private_t *pstVpuPrivate, 
	OMX_PTR           pGrallocBuff
	)
{
	if( pGrallocBuff < pstVpuPrivate->pUMPBase[PA] )
		return OMX_FALSE;

	if(pGrallocBuff > (pstVpuPrivate->pUMPBase[PA] + pstVpuPrivate->lUMPMapSize))
		return OMX_FALSE;

	return OMX_TRUE;
}

static 
OMX_BOOL 
IsGrallocBufferCopyRun(
	vpudec_private_t   *pstVpuPrivate,
	OMX_BOOL            bIsBufferValid
	)
{
	OMX_BOOL isHWrendered = OMX_FALSE;
	char value[PROPERTY_VALUE_MAX];
	OMX_U32 skip_count = 12;
	
	if( bIsBufferValid && pstVpuPrivate->pUMPBase[VA] <= 0 ) {
		ERROR("UMP base is not exist");
		return OMX_TRUE;
	}

	property_get("tcc.video.hwr.id", value, "0");
	if( atoi(value) == pstVpuPrivate->ulComponentUID ){
		vdec_set_rendered_index(pstVpuPrivate->pVpuInstance);
		isHWrendered = OMX_TRUE;
	}

	if( bIsBufferValid ) 
	{
		skip_count = 12;
		property_get("tcc.sys.output_mode_detected", value, "0");
	    if( !atoi(value) )
	    {
#ifdef USE_LCD_VIDEO_VSYNC
			if( CHECK_FLAG(pstVpuPrivate, STATE_16BIT_DRAM_MODE))
				goto EXCEPTIONAL_PROC;
			else
#endif
				return OMX_TRUE;
		}
		else
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
	else
	{
		if( pstVpuPrivate->lTryGrallocCopyCount < skip_count ){
			//ERROR("skip frame %d'th", pstVpuPrivate->lTryGrallocCopyCount);
			pstVpuPrivate->lTryGrallocCopyCount++;
			return OMX_FALSE;
		}
		return OMX_TRUE;
	}
}

static
TCC_PLATFORM_PRIVATE_PMEM_INFO*
GetPlatformPrivate(
	vpudec_private_t   *pstVpuPrivate,
	OMX_PTR             pGrallocBuff,
	OMX_BOOL            bIsBufferValid
	)
{
	OMX_S32 frame_len = 0;
	OMX_PTR p_private;

	if( bIsBufferValid && pstVpuPrivate->pUMPBase[VA] <= 0 ) {
		ERROR("UMP base is not exist");
		return NULL;
	}

	if( bIsBufferValid )
	{
		p_private = pstVpuPrivate->pUMPBase[VA];
		p_private += pGrallocBuff - pstVpuPrivate->pUMPBase[PA];
		p_private += pstVpuPrivate->stGrallocInfo.lFrameSize;
	}
	else
	{
		p_private = pGrallocBuff + pstVpuPrivate->stGrallocInfo.lFrameSize;
	}

    return (TCC_PLATFORM_PRIVATE_PMEM_INFO*)p_private;
}

static
COPY_MODE
GrallocCopyRun(
	vpudec_private_t   *pstVpuPrivate,
	buffer_handle_t     hBuffer,
	OMX_BOOL            bBlackFrame
	)
{
	COPY_MODE ret = COPY_NONE;
	OMX_PTR *p_grbuff_addr = NULL;

	if( pstVpuPrivate->stGrallocInfo.pDispOut[PA][0] ) 
	{
		OMX_BOOL is_gralloc_copy_run = OMX_FALSE;
		OMX_BOOL is_sw_copy = OMX_FALSE;
		OMX_BOOL is_buffer_valid = OMX_FALSE;
		omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
		gralloc_module_t const *p_module = pstVpuPrivate->stGrallocInfo.pstGrallocModule;

		if( CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_RESET) ) {
			if( HwCopyConfig(pstVpuPrivate, OMX_FALSE) == OMX_FALSE )
				return COPY_FAILED;
			CLEAR_FLAG(pstVpuPrivate, STATE_GRALLOC_RESET);
		}

		if( bBlackFrame )
		{
			p_module->lock( p_module,
							hBuffer, 
							GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK,
							0, 
							0,
							p_outport->sPortParam.format.video.nFrameWidth, 
							p_outport->sPortParam.format.video.nFrameHeight, 
							&p_grbuff_addr);

			ret = SwSetBlackFrame(pstVpuPrivate, p_grbuff_addr);

			p_module->unlock(p_module, hBuffer);

			pstVpuPrivate->stGrallocInfo.bBufferLocked = OMX_FALSE;

			LOG_GRA("[COPY-RUN ] [PLAT_PRIVATE: 0x%08X] [MODE: %d] [BUFF_HANDLE: 0x%08X] [BUFF_ADDR: 0x%08X] - (BLACK FRAME)"
					, pstVpuPrivate->pOutputPortPrivate
					, ret
					, hBuffer
					, p_grbuff_addr
					);
		}
		else
		{
	
			p_module->lock( p_module,
							hBuffer, 
							GRALLOC_USAGE_HW_RENDER,
							0, 
							0,
							p_outport->sPortParam.format.video.nFrameWidth, 
							p_outport->sPortParam.format.video.nFrameHeight, 
							&p_grbuff_addr);
	
			is_buffer_valid = IsValidGrallocBuffer(pstVpuPrivate, p_grbuff_addr);
			is_gralloc_copy_run = IsGrallocBufferCopyRun(pstVpuPrivate, is_buffer_valid);
	
			if( is_buffer_valid == OMX_FALSE ){
				p_module->unlock(p_module, hBuffer);
				p_module->lock( p_module,
								hBuffer, 
								GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_SW_WRITE_MASK,
								0, 
								0,
								p_outport->sPortParam.format.video.nFrameWidth, 
								p_outport->sPortParam.format.video.nFrameHeight, 
								&p_grbuff_addr);
			}

			pstVpuPrivate->stGrallocInfo.bBufferLocked = OMX_TRUE;
	
			pstVpuPrivate->pOutputPortPrivate = GetPlatformPrivate(pstVpuPrivate, 
																 p_grbuff_addr,
																 is_buffer_valid);
	
			if( is_gralloc_copy_run )
			{
				if( is_buffer_valid == OMX_FALSE ) 
				{
					ret = SwCopyRun(pstVpuPrivate, 
									pstVpuPrivate->stGrallocInfo.pDispOut[VA][0], 
									pstVpuPrivate->stGrallocInfo.pDispOut[VA][1],
									pstVpuPrivate->stGrallocInfo.pDispOut[VA][2], 
									p_grbuff_addr);
				}
				else
				{
					ret = HwCopyKickOff(pstVpuPrivate, 
										pstVpuPrivate->stGrallocInfo.pDispOut[PA][0], 
										pstVpuPrivate->stGrallocInfo.pDispOut[PA][1],
										pstVpuPrivate->stGrallocInfo.pDispOut[PA][2], 
										p_grbuff_addr);
	
					if( ret == COPY_FAILED ) {
						p_module->unlock(p_module, hBuffer);
						pstVpuPrivate->stGrallocInfo.bBufferLocked = OMX_FALSE;
					}
				}
			}
	
			if( is_buffer_valid == OMX_FALSE ) {
				p_module->unlock(p_module, hBuffer);
				pstVpuPrivate->stGrallocInfo.bBufferLocked = OMX_FALSE;
			}
	
			LOG_GRA("[COPY-RUN ] [PLAT_PRIVATE: 0x%08X] [MODE: %d] [BUFF_HANDLE: 0x%08X] [BUFF_ADDR: 0x%08X]"
					, pstVpuPrivate->pOutputPortPrivate
					, ret
					, hBuffer
					, p_grbuff_addr
					);
		}
	}
	else
	{
		LOG_GRA("[COPY-NONE] [PLAT_PRIVATE: 0x%08X] [MODE: -] [BUFF_HANDLE: 0x%08X] [BUFF_ADDR: ----------]"
				, pstVpuPrivate->pOutputPortPrivate
				, hBuffer
				);
	}

	return ret;
}

static
COPY_MODE
GrallocCopyWait(
	vpudec_private_t   *pstVpuPrivate,
	buffer_handle_t    hBuffer
	)
{
	COPY_MODE ret;
	gralloc_module_t const *p_module = pstVpuPrivate->stGrallocInfo.pstGrallocModule;

	ret = HwCopyWait(pstVpuPrivate);
	p_module->unlock(p_module, hBuffer);
	pstVpuPrivate->stGrallocInfo.bBufferLocked = OMX_FALSE;

	LOG_GRA("[COPY-WAIT] [PLAT_PRIVATE: 0x%08X] [MODE: %d]"
			, pstVpuPrivate->pOutputPortPrivate
			, ret
			);

	return ret;
}

static
void
UnlockGrallocBuffer(
	vpudec_private_t   *pstVpuPrivate,
	buffer_handle_t    hBuffer
	)
{
	if( pstVpuPrivate->stGrallocInfo.bBufferLocked == OMX_TRUE )
	{
		gralloc_module_t const *p_module = pstVpuPrivate->stGrallocInfo.pstGrallocModule;
		p_module->unlock(p_module, hBuffer);
		pstVpuPrivate->stGrallocInfo.bBufferLocked = OMX_FALSE;
	}
}

#endif



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	Input buffer information queue (dynamic size queue)
//
//
static OMX_BOOL InitInputQueue(input_queue_t *pstQue, OMX_S32 lInitSize);
static void DeinitInputQueue(input_queue_t *pstQue);
static void ClearInputQueue(input_queue_t *pstQue);
static OMX_S32 PushInputInfo(input_queue_t *pstQue, const input_info_t *pstInput);
static OMX_S32 GetInputInfo(input_queue_t *pstQue, input_info_t *pstInput);
static OMX_S32 ShowInputInfo(input_queue_t *pstQue, input_info_t *pstInput);
static OMX_S32 ShowLastInputInfo(input_queue_t *pstQue, input_info_t *pstInput); // for debugging
static OMX_S32 ClearFirstInputInfo(input_queue_t *pstQue);

static
OMX_BOOL
InitInputQueue(
	input_queue_t  *pstQue,
	OMX_S32         lInitSize
	)
{
	if( pstQue->pstInfoRing )
		TCC_free(pstQue->pstInfoRing);

	memset(pstQue, 0, sizeof(input_queue_t));

	pstQue->pstInfoRing = TCC_malloc(sizeof(input_info_t) * lInitSize);
	if( pstQue->pstInfoRing == NULL )
	{
		DeinitInputQueue(pstQue);
		ERROR("CreatePTSQueue() - out of memory");
		return OMX_FALSE;
	}
	pstQue->lQueSize = lInitSize;

	return OMX_TRUE;
}

static
void
DeinitInputQueue(
	input_queue_t  *pstQue
	)
{
	if( pstQue->pstInfoRing )
		TCC_free( pstQue->pstInfoRing );

	pstQue->pstInfoRing = NULL;
}

static
void
ClearInputQueue(
	input_queue_t  *pstQue
	)
{
	pstQue->lStartPos = 0;
	pstQue->lEndPos   = 0;
	pstQue->lQueCount = 0;
}

static
OMX_S32
PushInputInfo(
	input_queue_t      *pstQue,
	const input_info_t *pstInput
	)
{
	input_info_t  *p_ring = pstQue->pstInfoRing;
	OMX_S32        size   = pstQue->lQueSize;
	OMX_S32        count  = pstQue->lQueCount;
	OMX_S32        end    = pstQue->lEndPos;

#if 0	// This part will be modify in the near future.
	if( count ) {
		OMX_S32 last = end-1;

		if( last < 0 )
			last = size-1;

		if( p_ring[last].llTimestamp == pstInput->llTimestamp )
		{
			p_ring[last].lFilledLen += pstInput->lFilledLen;
			p_ring[last].pEndPtr = pstInput->pEndPtr;

			LOG_IIQUE("[PUSH ][QUECNT: %3d] [PTS: %8d] [REGION: %7d ~ %7d] [LENGTH: %7d byte]"
					  , pstQue->lQueCount
					  , (OMX_S32)(p_ring[last].llTimestamp/1000)
					  , p_ring[last].pStartPtr - pstQue->pBasePtr
					  , p_ring[last].pEndPtr - pstQue->pBasePtr
					  , p_ring[last].lFilledLen
					  );

			return count;
		}
	}
#endif

	if( ++count > size )
	{
		input_info_t *p_tmp_ring;
		OMX_S32       cpy = pstQue->lStartPos;
		OMX_S32       new_size = size * 2;
		OMX_S32       i;

		INFO("Input information queue resizing");

		p_tmp_ring = TCC_malloc(sizeof(input_info_t) * new_size);
		if( p_tmp_ring == NULL ) {
			ERROR("PushInputInfo() - out of memory");
			return -1;
		}

		for(i = 0; i < count-1; i++, cpy++)
		{
			if(cpy >= size)
				cpy -= size;
			p_tmp_ring[i] = p_ring[cpy];
		}

		size = new_size;
		end	= i;

		TCC_free(p_ring);
		p_ring = p_tmp_ring;
		pstQue->pstInfoRing = p_ring;
		pstQue->lQueSize = new_size;
		pstQue->lStartPos = 0;
		pstQue->lEndPos = end;
	}

	if( count == 1 ) {
		p_ring[pstQue->lEndPos++] = *pstInput;
		pstQue->lQueCount = 1;
	}
	else {
		p_ring[end] = *pstInput;

		if( ++pstQue->lEndPos >= size )
			pstQue->lEndPos -= size;
		pstQue->lQueCount = count;
	}

	LOG_IIQUE("[PUSH ][QUECNT: %3d] [PTS: %8d] [REGION: %7d ~ %7d] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr - pstQue->pBasePtr
			  , pstInput->pEndPtr - pstQue->pBasePtr
			  , pstInput->lFilledLen
			  );

	return count;
}

static
OMX_S32
GetInputInfo(
	input_queue_t  *pstQue,
	input_info_t   *pstInput
	)
{
	input_info_t  *p_ring = pstQue->pstInfoRing;
	OMX_S32        start  = pstQue->lStartPos;
	OMX_S32        size   = pstQue->lQueSize;
	OMX_S32        count  = pstQue->lQueCount;

	if( count == 0 ) {
		ERROR("GetInputInfo() - internal error");
		return -1;
	}

	*pstInput = p_ring[start];

	if( ++start >= size )
		start -= size;

	if( --count )
		pstQue->lStartPos = start;
	else {
		pstQue->lStartPos	= 0;
		pstQue->lEndPos		= 0;
	}

	pstQue->lQueCount = count;

	LOG_IIQUE("[GET  ][QUECNT: %3d] [PTS: %8d] [REGION: %7d ~ %7d] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr - pstQue->pBasePtr
			  , pstInput->pEndPtr - pstQue->pBasePtr
			  , pstInput->lFilledLen
			  );

	return count;
}

static 
OMX_S32 
ShowInputInfo(
	input_queue_t  *pstQue, 
	input_info_t   *pstInput
	)
{
	OMX_S32 count = pstQue->lQueCount;

	if( count == 0 ) {
		ERROR("ShowInputInfo() - info queue is empty");
		return -1;
	}

	*pstInput = pstQue->pstInfoRing[pstQue->lStartPos];

	LOG_IIQUE("[SHOW ][QUECNT: %3d] [PTS: %8d] [REGION: %7d ~ %7d] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr - pstQue->pBasePtr
			  , pstInput->pEndPtr - pstQue->pBasePtr
			  , pstInput->lFilledLen
			  );

	return count;
}

static 
OMX_S32 
ShowLastInputInfo(
	input_queue_t  *pstQue, 
	input_info_t   *pstInput
	)
{
	OMX_S32 count = pstQue->lQueCount;
	OMX_S32 end   = pstQue->lEndPos-1;

	if( count == 0 ) {
		ERROR("ShowInputInfo() - info queue is empty");
		return -1;
	}

	if( end < 0 )
		end = pstQue->lQueSize - 1;

	*pstInput = pstQue->pstInfoRing[end];

	LOG_IIQUE("[SHOWL][QUECNT: %3d] [PTS: %8d] [REGION: %7d ~ %7d] [LENGTH: %7d byte]"
			  , pstQue->lQueCount
			  , (OMX_S32)(pstInput->llTimestamp/1000)
			  , pstInput->pStartPtr - pstQue->pBasePtr
			  , pstInput->pEndPtr - pstQue->pBasePtr
			  , pstInput->lFilledLen
			  );

	return count;
}

static 
OMX_S32 
ClearFirstInputInfo(
	input_queue_t  *pstQue
	)
{
	input_info_t  *p_ring = pstQue->pstInfoRing;
	OMX_S32        start  = pstQue->lStartPos;
	OMX_S32        size   = pstQue->lQueSize;
	OMX_S32        count  = pstQue->lQueCount;

	if( count == 0 ) {
		ERROR("ClearFirstInputInfo() - internal error");
		return -1;
	}

	if( ++start >= size )
		start -= size;

	if( --count )
		pstQue->lStartPos = start;
	else {
		pstQue->lStartPos	= 0;
		pstQue->lEndPos		= 0;
	}

	pstQue->lQueCount = count;

	LOG_IIQUE("[CLEAR][QUECNT: %3d]", pstQue->lQueCount);

	return count;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Display index / VSync buffer ID queue (static size queue)
//
//
static void ClearDispIdxQueue(dispidx_queue_t *pstQue);
static OMX_S32 PushDispIdx(dispidx_queue_t *pstQue, OMX_U32 ulDispIdx, OMX_U32 ulBuffId);
static OMX_S32 GetDispIdx(dispidx_queue_t *pstQue, OMX_U32 *pulDispIdx, OMX_U32 *pulBuffId);
static OMX_S32 ShowDispIdx(dispidx_queue_t *pstQue, OMX_U32 *pulDispIdx, OMX_U32 *pulBuffId);
static OMX_S32 ClearFirstDispIdx(dispidx_queue_t *pstQue);
static OMX_S32 GetDispIdxCount(dispidx_queue_t *pstQue);

static
void
ClearDispIdxQueue(
	dispidx_queue_t  *pstQue
	)
{
	memset(pstQue, 0, sizeof(dispidx_queue_t));

	LOG_IDXQUE("[CLEAR] [QUECNT: %3d]", pstQue->lQueCount);
}

static
OMX_S32
PushDispIdx(
	dispidx_queue_t  *pstQue,
	OMX_U32	         ulDispIdx,
	OMX_U32	         ulBuffId
	)
{
	OMX_S32   count	= pstQue->lQueCount;
	OMX_S32   start	= pstQue->lStartPos;
	OMX_S32   end	= pstQue->lEndPos;
	OMX_S32   i;

	if( ++count > REFERENCE_BUFFER_MAX )
	{
		ERROR("PushDispIdx() - queue overflow");
		return -1;
	}

	if( count == 1 ) {
		pstQue->aulDispIdxRing[end] = ulDispIdx;
		pstQue->aulBuffIdRing[end] = ulBuffId;
		pstQue->lQueCount = 1;
		pstQue->lEndPos++;
	}
	else {
		pstQue->aulDispIdxRing[end] = ulDispIdx;
		pstQue->aulBuffIdRing[end] = ulBuffId;

		if( ++end >= REFERENCE_BUFFER_MAX )
			end = 0;
		pstQue->lEndPos = end;
		pstQue->lQueCount = count;
	}

	LOG_IDXQUE("[PUSH ] [QUECNT: %3d] [DISP_IDX: %3d] [VSYNC_ID: %d]"
			   , pstQue->lQueCount
			   , ulDispIdx
			   , ulBuffId
			   );

	return count;
}

static
OMX_S32
GetDispIdx(
	dispidx_queue_t  *pstQue,
	OMX_U32          *pulDispIdx,
	OMX_U32          *pulBuffId
	)
{
	OMX_S32   start = pstQue->lStartPos;
	OMX_S32   count = pstQue->lQueCount;

	if( count == 0 )
		return -1;

	*pulDispIdx = pstQue->aulDispIdxRing[start];
	*pulBuffId = pstQue->aulBuffIdRing[start];

	if( ++start >= REFERENCE_BUFFER_MAX )
		start = 0;

	if( --count )
		pstQue->lStartPos = start;
	else {
		pstQue->lStartPos	= 0;
		pstQue->lEndPos		= 0;
	}

	pstQue->lQueCount = count;

	LOG_IDXQUE("[GET  ] [QUECNT: %3d] [DISP_IDX: %3d] [VSYNC_ID: %d]"
			   , pstQue->lQueCount
			   , *pulDispIdx
			   , *pulBuffId
			   );

	return count;
}

static
OMX_S32
ShowDispIdx(
	dispidx_queue_t  *pstQue,
	OMX_U32          *pulDispIdx,
	OMX_U32          *pulBuffId
	)
{
	OMX_S32   start = pstQue->lStartPos;
	OMX_S32   count = pstQue->lQueCount;

	if( count == 0 )
		return -1;

	*pulDispIdx = pstQue->aulDispIdxRing[start];
	*pulBuffId = pstQue->aulBuffIdRing[start];

	LOG_IDXQUE("[SHOW ] [QUECNT: %3d] [DISP_IDX: %3d] [VSYNC_ID: %d]"
			   , pstQue->lQueCount
			   , *pulDispIdx
			   , *pulBuffId
			   );

	return count;
}

static
OMX_S32
ClearFirstDispIdx(
	dispidx_queue_t  *pstQue
	)
{
	OMX_S32   start = pstQue->lStartPos;
	OMX_S32   count = pstQue->lQueCount;

	if( count == 0 )
		return -1;

	if( ++start >= REFERENCE_BUFFER_MAX )
		start = 0;

	if( --count )
		pstQue->lStartPos = start;
	else {
		pstQue->lStartPos	= 0;
		pstQue->lEndPos		= 0;
	}

	pstQue->lQueCount = count;

	LOG_IDXQUE("[CLEAR] [QUECNT: %3d]", pstQue->lQueCount);

	return count;
}

static 
OMX_S32 
GetDispIdxCount(
	dispidx_queue_t *pstQue
	)
{
	return pstQue->lQueCount;
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Display index / Buffer list management (non-vsync mode)
//
//
#ifdef VPU_BUFFER_MANAGEMENT

static void InitDispBuffList(dispidx_list_t *pstDispBuffList, OMX_S32 lMaxCount, OMX_PTR pVpuInstance, OMX_S32 hFBDevice);
static void ResetDispBuffList(dispidx_list_t *pstDispBuffList);
static OMX_S32 PushDispBuffToList(dispidx_list_t *pstDispBuffList, OMX_U32 ulDispIdx, OMX_BOOL bDependent, OMX_U8 *pOutputBuffer);
static void ClearDispedBuffFromList(dispidx_list_t *pstDispBuffList, OMX_U8 *pOutputBuffer);
static void ClearAllDispBuffFromList(dispidx_list_t *pstDispBuffList);

static 
void 
InitDispBuffList(
	dispidx_list_t *pstDispBuffList, 
	OMX_S32         lMaxCount, 
	OMX_PTR         pVpuInstance, 
	OMX_S32         hFBDevice
	)
{
	memset(pstDispBuffList, 0, sizeof(dispidx_list_t));
	pstDispBuffList->lMaxCount = lMaxCount;
	pstDispBuffList->pVpuInstance = pVpuInstance;
	pstDispBuffList->hFBDevice = hFBDevice;

	LOG_IDXQUE("[INIT ] [USEDCNT: %2d/%2d]"
			   , pstDispBuffList->lUsedCount
			   , pstDispBuffList->lMaxCount
			   );

	ResetDispBuffList(pstDispBuffList);
}

static 
void 
ResetDispBuffList(
	dispidx_list_t *pstDispBuffList
	)
{
	OMX_S32 max_cnt = pstDispBuffList->lMaxCount;
	OMX_S32 idx;
	dispbuff_state_t *p_state;

	for(idx = 0; idx < max_cnt; idx++) 
	{
		p_state = pstDispBuffList->astDispBuffState + idx;
		p_state->pOmxOutBuff   = NULL;
		p_state->bIsCleared    = OMX_TRUE;
		p_state->alKeepedIndex[0]  = -1;
		p_state->alKeepedIndex[1]  = -1;
		p_state->alDisplayIndex[0] = -1;	
		p_state->alDisplayIndex[1] = -1;	
	}

	pstDispBuffList->lUsedCount = 0;

	LOG_IDXQUE("[CLEAR] [USEDCNT: %2d/%2d]"
			   , pstDispBuffList->lUsedCount
			   , pstDispBuffList->lMaxCount
			   );
}

static 
OMX_S32 
PushDispBuffToList(
	dispidx_list_t *pstDispBuffList, 
	OMX_U32         ulDispIdx, 
	OMX_BOOL        bDependent, 
	OMX_U8         *pOutputBuffer
	)
{
	OMX_S32 used_cnt = pstDispBuffList->lUsedCount;
	OMX_S32 max_cnt = pstDispBuffList->lMaxCount;
	OMX_S32 idx;
	dispbuff_state_t *p_state;

	if( used_cnt == max_cnt )
	{
		// find the index of the list which include current output buffer
		for(idx = 0; idx < used_cnt; idx++) 
		{
			p_state = pstDispBuffList->astDispBuffState + idx;
			if( p_state->pOmxOutBuff == pOutputBuffer )
				break;
		}
	
		if( idx == used_cnt ) {
			return -1; // list fulled and not found (internal error)
		}
	}
	else
	{
		OMX_S32 ret;

		// find the index of the list which include current output buffer
		// and.. clear the vpu buffer of the index.
		for(idx = 0; idx < used_cnt; idx++) 
		{
			p_state = pstDispBuffList->astDispBuffState + idx;
			if( p_state->pOmxOutBuff == pOutputBuffer )
			{
				if( !bDependent )
				{
					OMX_S32 clear_idx;

					clear_idx = p_state->alDisplayIndex[0];
					if( clear_idx >= 0 ) {
						LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, p_state->pOmxOutBuff);
						if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
							ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
							return -1;
						}
						p_state->alDisplayIndex[0] = -1;
					}
	
					clear_idx = p_state->alDisplayIndex[1];
					if( clear_idx >= 0 ) {
						LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, p_state->pOmxOutBuff);
						if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
							ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
							return -1;
						}
						p_state->alDisplayIndex[1] = -1;
					}
				}

				break;
			}
		}

		// if the index is not found, add the new output buffer to list.
		if( idx >= used_cnt ) {
			p_state = pstDispBuffList->astDispBuffState + idx;
			pstDispBuffList->lUsedCount++;
		}
	}

	// set not cleared state.
	p_state->pOmxOutBuff   = pOutputBuffer;
	p_state->bIsCleared    = OMX_FALSE;
	if( bDependent )
		p_state->alDisplayIndex[1] = ulDispIdx;
	else 
		p_state->alDisplayIndex[0] = ulDispIdx;

	LOG_IDXQUE("[PUSH ] [USEDCNT: %2d/%2d] [DISP_IDX: %3d/%3d] [OMX_BUFF: 0x%08X] - [idx: %d]"
			   , pstDispBuffList->lUsedCount
			   , pstDispBuffList->lMaxCount
			   , p_state->alDisplayIndex[0]
			   , p_state->alDisplayIndex[1]
			   , p_state->pOmxOutBuff
			   , idx
			   );

	return pstDispBuffList->lUsedCount;
}

static 
void 
ClearDispedBuffFromList(
	dispidx_list_t *pstDispBuffList, 
	OMX_U8 *pOutputBuffer
	)
{
	OMX_S32 displaying_idx;
	OMX_S32 used_cnt = pstDispBuffList->lUsedCount;
	OMX_S32 max_cnt = pstDispBuffList->lMaxCount;
	OMX_S32 idx;
	OMX_S32 ret;
	dispbuff_state_t *p_state;

	displaying_idx = ioctl(pstDispBuffList->hFBDevice, TCC_VIDEO_GET_DISPLAYING_IDX, NULL);

	// clear KEEPED VPU buffer that is not same with currently diplaying buffer.
	for(idx = 0; idx < used_cnt; idx++) 
	{
		p_state = pstDispBuffList->astDispBuffState + idx;

		if( displaying_idx != -1 && p_state->alKeepedIndex[0] >= 0 && p_state->alKeepedIndex[0] != displaying_idx && p_state->alKeepedIndex[1] != displaying_idx ) {
			LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", p_state->alKeepedIndex[0], p_state->pOmxOutBuff);
			if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &p_state->alKeepedIndex[0], NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", p_state->alKeepedIndex[0], ret);
				return;
			}
			p_state->alKeepedIndex[0] = -1;

			if( p_state->alKeepedIndex[1] >= 0 ) {
				LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", p_state->alKeepedIndex[1], p_state->pOmxOutBuff);
				if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &p_state->alKeepedIndex[1], NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
					ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", p_state->alKeepedIndex[1], ret);
					return;
				}
				p_state->alKeepedIndex[1] = -1;
			}

			p_state->bIsCleared = OMX_TRUE;
		}
	}

	// find the index of the list which include current output buffer
	for(idx = 0; idx < used_cnt; idx++) 
	{
		p_state = pstDispBuffList->astDispBuffState + idx;
		if( p_state->pOmxOutBuff == pOutputBuffer )
		{
			OMX_S32 clear_idx;
			p_state->bIsCleared = OMX_TRUE;

			// If the VPU buffer is currently displaying, keep the index of the VPU buffer.
			// The buffer would be cleared next time. (return -1)
			if( displaying_idx != -1 && 
				( p_state->alDisplayIndex[0] == displaying_idx 
				  || p_state->alDisplayIndex[1] == displaying_idx ) )
			{
				p_state->alKeepedIndex[0] = p_state->alDisplayIndex[0];
				p_state->alKeepedIndex[1] = p_state->alDisplayIndex[1];
				break;
			}

			LOG_IDXQUE("[GET  ] [USEDCNT: %2d/%2d] [DISP_IDX: %3d/%3d] [OMX_BUFF: 0x%08X] - [idx: %d / disp: %d]"
					   , pstDispBuffList->lUsedCount
					   , pstDispBuffList->lMaxCount
					   , p_state->alDisplayIndex[0]
					   , p_state->alDisplayIndex[1]
					   , p_state->pOmxOutBuff
					   , idx
					   , displaying_idx
					   );

			clear_idx = p_state->alDisplayIndex[0];
			if( clear_idx >= 0 ) {
				LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, pOutputBuffer);
				if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
					ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
					return ret;
				}
				p_state->alDisplayIndex[0] = -1;
			}

			clear_idx = p_state->alDisplayIndex[1];
			if( clear_idx >= 0 ) {
				LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, pOutputBuffer);
				if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
					ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
					return ret;
				}
				p_state->alDisplayIndex[1] = -1;
			}

			return;
		}
	}

	LOG_IDXQUE("[GET  ] [USEDCNT: %2d/%2d] [DISP_IDX: %3d/%3d] [OMX_BUFF: 0x%08X] - [idx: %d / disp: %d]"
			   , pstDispBuffList->lUsedCount
			   , pstDispBuffList->lMaxCount
			   , -1
			   , -1
			   , pOutputBuffer
			   , idx
			   , displaying_idx
			   );
}

static 
void 
ClearAllDispBuffFromList(
	dispidx_list_t *pstDispBuffList
	)
{
	OMX_S32 used_cnt = pstDispBuffList->lUsedCount;
	OMX_S32 max_cnt = pstDispBuffList->lMaxCount;
	OMX_S32 idx;
	OMX_S32 ret;
	OMX_S32 clear_idx;
	dispbuff_state_t *p_state;

	for(idx = 0; idx < used_cnt; idx++) 
	{
		p_state = pstDispBuffList->astDispBuffState + idx;

		clear_idx = p_state->alKeepedIndex[0];
		if( clear_idx >= 0 ) {
			LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, p_state->pOmxOutBuff);
			if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
				return -1;
			}
			p_state->alKeepedIndex[0] = -1;
		}

		clear_idx = p_state->alKeepedIndex[1];
		if( clear_idx >= 0 ) {
			LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, p_state->pOmxOutBuff);
			if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
				return -1;
			}
			p_state->alKeepedIndex[1] = -1;
		}

		if( p_state->bIsCleared == OMX_FALSE )
		{
			clear_idx = p_state->alDisplayIndex[0];
			if( clear_idx >= 0 ) {
				LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, p_state->pOmxOutBuff);
				if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
					ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
					return -1;
				}
				p_state->alDisplayIndex[0] = -1;
			}

			clear_idx = p_state->alDisplayIndex[1];
			if( clear_idx >= 0 ) {
				LOG_BUFCLR("[DISP_IDX: %2d] [OMX_BUFF: 0x%08X]", clear_idx, p_state->pOmxOutBuff);
				if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &clear_idx, NULL, pstDispBuffList->pVpuInstance)) < 0 ) {
					ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", clear_idx, ret);
					return -1;
				}
				p_state->alDisplayIndex[1] = -1;
			}

			p_state->bIsCleared = OMX_TRUE;
		}
	}

	LOG_IDXQUE("[CLRAL] [USEDCNT: %2d/%2d]"
			   , pstDispBuffList->lUsedCount
			   , pstDispBuffList->lMaxCount
			   );
}

#endif //VPU_BUFFER_MANAGEMENT



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//  Display information manager
//
//
static void InitDispInfoManager(dispinfo_manager_t *pstMgr, OMX_S32 lStreamType, OMX_S32 lFrameRate);
static void ResetDispInfoManager(dispinfo_manager_t *pstMgr);
static disp_info_t* GetDispInfoSlot(dispinfo_manager_t *pstMgr, OMX_S32 lBuffIdx);
static OMX_BOOL UpdateDispInfo(dispinfo_manager_t *pstMgr, OMX_S32 lBuffIdx, OMX_S32 lFrameRate);
static OMX_BOOL ShowOutputDispInfo(dispinfo_manager_t *pstMgr, OMX_S32 lBuffIdx, disp_info_t **ppstInfo, OMX_TICKS *pllOutPTS);
static OMX_BOOL ShowNonQueuedDispInfo(dispinfo_manager_t *pstMgr, OMX_S32 lBuffIdx, disp_info_t **ppstInfo, OMX_TICKS *pllOutPTS);
static OMX_BOOL ClearDispInfo(dispinfo_manager_t *pstMgr, OMX_S32 lBuffIdx);
static OMX_BOOL ClearNonQueuedDispInfo(dispinfo_manager_t *pstMgr, OMX_S32 lBuffIdx);

static
void
InitDispInfoManager(
	dispinfo_manager_t  *pstMgr,
	OMX_S32              lStreamType,
	OMX_S32              lFrameRate
	)
{
	ResetDispInfoManager(pstMgr);
	pstMgr->lStreamType = lStreamType;
	pstMgr->lFrameRate = lFrameRate;
	if( lFrameRate > 0 )
		pstMgr->llFrameDuration = 1000000000 / lFrameRate;
	else
		pstMgr->llFrameDuration = 0;

	LOG_DIMGR("FRAME-RATE INIT - [FRAME-RATE: %d] [FRAME-DUE: %lld us]"
			  , pstMgr->lFrameRate
			  , pstMgr->llFrameDuration
			  );
}

static
void
ResetDispInfoManager(
	dispinfo_manager_t  *pstMgr
	)
{
	memset(pstMgr->astDispInfoList, 0, sizeof(disp_info_t) * REFERENCE_BUFFER_MAX);
	pstMgr->lFrameDurationFactor = 0;
	pstMgr->llLastBaseTimestamp = -1;
	pstMgr->llLastOutTimestamp = 0;
	pstMgr->llLastDecTimestamp = 0;
	pstMgr->llShowedTimestamp = 0;
	pstMgr->lRvReferenceIdx = -1;
}

static 
disp_info_t* 
GetDispInfoSlot(
	dispinfo_manager_t  *pstMgr, 
	OMX_S32              lBuffIdx
	)
{
	disp_info_t  *p_info = pstMgr->astDispInfoList + lBuffIdx;

	if( lBuffIdx >= REFERENCE_BUFFER_MAX ) {
		ERROR("GetDispInfoSlot() - invalid buffer index (buff_idx %d)", lBuffIdx);
		return NULL;
	}

	if( p_info->bIsValid == OMX_TRUE ){
		ERROR("GetDispInfoSlot() - already valid state: (buff_idx %d)", lBuffIdx);

//		It is abnormal event caused by same decoded buffer index as previous decoding result.
//  	But, it is ignored and renew buffer state from current decoding result.
//		return NULL;
	}

	return pstMgr->astDispInfoList + lBuffIdx;
}

static
OMX_BOOL
UpdateDispInfo(
	dispinfo_manager_t  *pstMgr,
	OMX_S32	             lBuffIdx,
	OMX_S32              lFrameRate
	)
{
	disp_info_t  *p_info = pstMgr->astDispInfoList + lBuffIdx;
	OMX_TICKS     timestamp;
	OMX_S32       i;

	if( lBuffIdx >= REFERENCE_BUFFER_MAX ) {
		ERROR("UpdateDispInfo() - invalid buffer index: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

//	It is abnormal event caused by same decoded buffer index as previous decoding result.
//  But, it is ignored and renew buffer state from current decoding result.
// 
//	if( p_info->bIsValid == OMX_TRUE ){
//		ERROR("UpdateDispInfo() - already valid state: (buff_idx %d)", lBuffIdx);
//		return OMX_FALSE;
//	}

	/* update frame-rate */
	if( lFrameRate && pstMgr->lFrameRate != lFrameRate ) {
		pstMgr->lFrameRate = lFrameRate;
		pstMgr->llFrameDuration = 1000000000 / lFrameRate;

		LOG_DIMGR("FRAME-RATE UPDATED - [FRAME-RATE: %d] [FRAME-DUE: %lld us]"
				  , pstMgr->lFrameRate
				  , pstMgr->llFrameDuration
				  );
	}

	/* RealVideo timestamp calculation */
	if( pstMgr->lStreamType == STD_RV )
	{
		OMX_S32    rv_timestamp   = p_info->lRvTimestamp;
		OMX_TICKS  curr_timestamp = p_info->llTimestamp;
		OMX_S32    ref_idx        = pstMgr->lRvReferenceIdx;
				
		if( ref_idx < 0 )
		{
			ref_idx = 0;
			pstMgr->lRvTimestamp[0] = curr_timestamp;
			pstMgr->lRvTimestamp[1] = curr_timestamp;
			pstMgr->lRvTimeReference[0] = rv_timestamp;
			pstMgr->lRvTimeReference[1] = rv_timestamp;
		}
		else
		{
			OMX_S32 tr_delta = rv_timestamp - pstMgr->lRvTimeReference[ref_idx];

			if( tr_delta < 0 )
				tr_delta += 8192;

			if( p_info->lFrameType == 2 ) //B-frame
				curr_timestamp = pstMgr->lRvTimestamp[ref_idx] + ((OMX_TICKS)tr_delta * 1000);
			else
				ref_idx++;

			if( ref_idx == 1 ) {
				pstMgr->lRvTimestamp[0] = curr_timestamp;
				pstMgr->lRvTimeReference[0] = rv_timestamp;

			}
			else if( ref_idx == 2 ) {
				ref_idx = 0;
				pstMgr->lRvTimestamp[1] = curr_timestamp;
				pstMgr->lRvTimeReference[1] = rv_timestamp;
			}		
		}

		pstMgr->lRvReferenceIdx = ref_idx;
		//p_info->lRvTimestamp = curr_timestamp;
		p_info->llTimestamp = curr_timestamp;
	}

	p_info->bIsValid = OMX_TRUE;

	/* update timestamp queue */
	timestamp = p_info->llTimestamp;
	p_info->llPrevTimestamp = timestamp;

	if( pstMgr->llLastDecTimestamp != timestamp ) {
		p_info->bPtsNotExist = OMX_FALSE;
		pstMgr->llLastDecTimestamp = timestamp;
	} else
		p_info->bPtsNotExist = OMX_TRUE;

	LOG_DIMGR("[UPDATE] [BUFF_IDX: %2d] {PTS} [INPUT   : %8d]"
			  , lBuffIdx
			  , (OMX_S32)(timestamp/1000)
			  );

	return OMX_TRUE;
}

static
OMX_BOOL
ShowOutputDispInfo(
	dispinfo_manager_t  *pstMgr,
	OMX_S32	             lBuffIdx,
	disp_info_t        **ppstInfo,
	OMX_TICKS           *pllOutPTS
	)
{
	disp_info_t  *p_info = pstMgr->astDispInfoList + lBuffIdx;
	OMX_TICKS     out_timestamp;

	if( lBuffIdx >= REFERENCE_BUFFER_MAX ) {
		ERROR("ShowOutputDispInfo() - invalid buffer index: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

	if( p_info->bIsValid == OMX_FALSE ){
		ERROR("ShowOutputDispInfo() - invalid buffer state: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

	out_timestamp = p_info->llTimestamp;

	if( p_info->bIsMvcDependent ) {
		out_timestamp = pstMgr->llLastOutTimestamp;
	}
	else
	{
		if( pstMgr->bBackwardPlayback == OMX_FALSE && p_info->bPtsNotExist == OMX_TRUE )
		{
			if( pstMgr->lFrameDurationFactor )
				out_timestamp = pstMgr->llLastOutTimestamp + ((pstMgr->llFrameDuration * pstMgr->lFrameDurationFactor) >> 1);
			else
				out_timestamp = pstMgr->llLastOutTimestamp + pstMgr->llFrameDuration;
	
			LOG_DIMGR("[SHOW  ] [BUFF_IDX: %2d] {PTS} [LAST_OUT: %8d] [ORG: %8d (%d)][MOD: %8d] [FACTOR: %d]"
					  , lBuffIdx
					  , (OMX_S32)(pstMgr->llLastOutTimestamp/1000)
					  , (OMX_S32)(p_info->llTimestamp/1000)
					  , p_info->bPtsNotExist
					  , (OMX_S32)(out_timestamp/1000)
					  , pstMgr->lFrameDurationFactor
					  );
		}
		else {
			LOG_DIMGR("[SHOW  ] [BUFF_IDX: %2d] {PTS} [LAST_OUT: %8d] [ORG: %8d (%d)]"
					  , lBuffIdx
					  , (OMX_S32)(pstMgr->llLastOutTimestamp/1000)
					  , (OMX_S32)(p_info->llTimestamp/1000)
					  , p_info->bPtsNotExist
					  );
		}
	}

	pstMgr->llShowedTimestamp = out_timestamp;

	*ppstInfo = p_info;
	*pllOutPTS = out_timestamp;

	return OMX_TRUE;
}

static
OMX_BOOL
ShowNonQueuedDispInfo(
	dispinfo_manager_t  *pstMgr,
	OMX_S32	             lBuffIdx,
	disp_info_t        **ppstInfo,
	OMX_TICKS           *pllOutPTS
	)
{
	disp_info_t  *p_info = pstMgr->astDispInfoList + lBuffIdx;
	OMX_TICKS     out_timestamp;

	if( lBuffIdx >= REFERENCE_BUFFER_MAX ) {
		ERROR("ShowNonQueuedDispInfo() - invalid buffer index: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

	if( p_info->bIsMvcDependent ) {
		out_timestamp = pstMgr->llLastOutTimestamp;
	}
	else
	{
		if( pstMgr->bBackwardPlayback == OMX_FALSE )
		{
			if( pstMgr->lFrameDurationFactor )
				out_timestamp = pstMgr->llLastOutTimestamp + ((pstMgr->llFrameDuration * pstMgr->lFrameDurationFactor) >> 1);
			else
				out_timestamp = pstMgr->llLastOutTimestamp + pstMgr->llFrameDuration;
	
			LOG_DIMGR("[SHOW  ] [BUFF_IDX: %2d] {PTS} [LAST_OUT: %8d] [ORG: %8d (%d)][SORT: --------][MOD: %8d] [FACTOR: %d]"
					  , lBuffIdx
					  , (OMX_S32)(pstMgr->llLastOutTimestamp/1000)
					  , (OMX_S32)(p_info->llTimestamp/1000)
					  , p_info->bPtsNotExist
					  , (OMX_S32)(out_timestamp/1000)
					  , pstMgr->lFrameDurationFactor
					  );
		}
		else 
		{
			out_timestamp = pstMgr->llLastBaseTimestamp;
	
			LOG_DIMGR("[SHOW  ] [BUFF_IDX: %2d] {PTS} [LAST_OUT: %8d] [ORG: %8d (%d)][SORT: --------][MOD: %8d]"
					  , lBuffIdx
					  , (OMX_S32)(pstMgr->llLastOutTimestamp/1000)
					  , (OMX_S32)(p_info->llTimestamp/1000)
					  , p_info->bPtsNotExist
					  , (OMX_S32)(out_timestamp/1000)
					  );
		}
	}

	pstMgr->llShowedTimestamp = out_timestamp;

	*ppstInfo = p_info;
	*pllOutPTS = out_timestamp;

	return OMX_TRUE;
}

static
OMX_BOOL
ClearDispInfo(
	dispinfo_manager_t  *pstMgr,
	OMX_S32	             lBuffIdx
	)
{
	disp_info_t  *p_info = pstMgr->astDispInfoList + lBuffIdx;

	if( lBuffIdx >= REFERENCE_BUFFER_MAX ) {
		ERROR("ClearDispInfo() - invalid buffer index: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

	if( p_info->bIsValid == OMX_FALSE ){
		ERROR("ClearDispInfo() - already invalid state: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

	p_info->bIsValid = OMX_FALSE;

	/* last timestamp */
	if( pstMgr->llShowedTimestamp )
		pstMgr->llLastOutTimestamp = pstMgr->llShowedTimestamp;
	else
		pstMgr->llLastOutTimestamp = p_info->llTimestamp;
	pstMgr->llLastBaseTimestamp = p_info->llTimestamp;
	pstMgr->llShowedTimestamp = 0;

	/* duration factor */
	pstMgr->lFrameDurationFactor = p_info->lFrameDurationFactor;

	LOG_DIMGR("[CLEAR ] [BUFF_IDX: %2d] {PTS} [LAST_OUT: %8d] [ORG: %8d (%d)]"
			  , lBuffIdx
			  , (OMX_S32)(pstMgr->llLastOutTimestamp/1000)
			  , (OMX_S32)(p_info->llTimestamp/1000)
			  , p_info->bPtsNotExist
			  );

	return OMX_TRUE;
}

static
OMX_BOOL
ClearNonQueuedDispInfo(
	dispinfo_manager_t  *pstMgr,
	OMX_S32	             lBuffIdx
	)
{
	disp_info_t  *p_info = pstMgr->astDispInfoList + lBuffIdx;

	if( lBuffIdx >= REFERENCE_BUFFER_MAX ) {
		ERROR("ClearDispInfo() - invalid buffer index: (buff_idx %d)", lBuffIdx);
		return OMX_FALSE;
	}

	p_info->bIsValid = OMX_FALSE;

	/* last timestamp */
	if( pstMgr->llShowedTimestamp )
		pstMgr->llLastOutTimestamp = pstMgr->llShowedTimestamp;
	else
		pstMgr->llLastOutTimestamp = p_info->llTimestamp;
	pstMgr->llLastBaseTimestamp = p_info->llTimestamp;
	pstMgr->llShowedTimestamp = 0;

	/* duration factor */
	pstMgr->lFrameDurationFactor = p_info->lFrameDurationFactor;

	LOG_DIMGR("[CLEAR ] [BUFF_IDX: %2d] {PTS} [LAST_OUT: %8d] [ORG: %8d (%d)] -- non-ququed"
			  , lBuffIdx
			  , (OMX_S32)(pstMgr->llLastOutTimestamp/1000)
			  , (OMX_S32)(p_info->llTimestamp/1000)
			  , p_info->bPtsNotExist
			  );

	return OMX_TRUE;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	Configuration Helper
//
//
static void DecideLimitTimeDiffForFeeding(vpudec_private_t *pstVpuPrivate, OMX_U32 ulFrameRate);
static OMX_S32 GetAvccToAnnexbSpaPpsLength(const OMX_U8 *pbyAvcc, OMX_S32 lAvccLen);
static OMX_BOOL ConvertAvccToAnnexbSpaPps(const OMX_U8 *pbyAvcc, OMX_S32 lAvccLen, OMX_U8 *pbyAnnexb, OMX_S32 *plAnnexb);
static OMX_S32 ScanSequenceHeader(OMX_BUFFERHEADERTYPE *pInputBuffer, OMX_S32 lStreamType);
static OMX_BOOL StoreSequenceHeader(vpudec_private_t *pstVpuPrivate, OMX_U8 *pSequenceHeader, OMX_S32 nHeaderLength);
static OMX_BOOL StoreConfigData(vpudec_private_t *pstVpuPrivate, OMX_BUFFERHEADERTYPE *pInputBuffer);
static OMX_BOOL FindTccConfigData(vpudec_private_t *pstVpuPrivate, OMX_BUFFERHEADERTYPE *pInputBuffer);
static OMX_S32 GetDecodedFrameType(vpudec_private_t *pstVpuPrivate);
static void IFrameSearchEnable(vpudec_private_t *pstVpuPrivate);
static void SetFrameSkipMode(vpudec_private_t *pstVpuPrivate, OMX_S32 lSkipMode);
static void CheckSkipSignal(vpudec_private_t *pstVpuPrivate);

#if AVC_DECODE_ONLY_MODE_ENABLE
static OMX_S32 ScanAvcIdrSlice(OMX_BUFFERHEADERTYPE	*pInputBuffer);
static void ConfigDecodeOnlyMode(vpudec_private_t *pstVpuPrivate);
#endif


static
void
DecideLimitTimeDiffForFeeding(
	vpudec_private_t      *pstVpuPrivate,
	OMX_U32                ulFrameRate
	)
{
	char value[PROPERTY_VALUE_MAX];
	OMX_S64 numerator;
	property_get("tcc.vpu.rb.feedlimit.factor", value, "5");
	numerator = (OMX_TICKS)atoi(value) * (OMX_TICKS)1000000000;

	// frame-duration(us) * factor
	if( ulFrameRate < FRAME_RATE_FOR_FEED_LIMIT_MIN	)
		pstVpuPrivate->llFeedMinTimeDiff = numerator / FRAME_RATE_FOR_FEED_LIMIT_MIN;
	else 
		pstVpuPrivate->llFeedMinTimeDiff = numerator / ulFrameRate;

	pstVpuPrivate->llFeedMaxTimeDiff = pstVpuPrivate->llFeedMinTimeDiff * 2;

	INFO("Feed limit time difference: %lld us (%.3f fps)", 
			pstVpuPrivate->llFeedMinTimeDiff,
			(double)ulFrameRate/1000);
}


static
OMX_S32
GetAvccToAnnexbSpaPpsLength(
	const OMX_U8  *pbyAvcc,
	OMX_S32        lAvccLen
	)
{
	OMX_S32	nal_len;
	OMX_S32 spspps_len = 0;
	OMX_S32	count;
	OMX_S32	i;

	const OMX_U8  *p_avcc = pbyAvcc+5;

	if( lAvccLen < 7 ) {
		ERROR("GetAvccToAnnexbSpaPpsLength() - avcC is too short (len: %d) \n", lAvccLen);
		return OMX_FALSE;
	}

	// configurationVersion
	if( *pbyAvcc != 1 ) {
		ERROR("GetAvccToAnnexbSpaPpsLength() - Unknown avcC version %d\n", *p_avcc);
		return OMX_FALSE;
	}

	// lengthSizeMinusOne
	//*plNalLenSize = (pbyAvcc[4] & 0x03)+1;

	// numOfSequenceParameterSets
	count = (*p_avcc++) & 0x1F;

	for(i = 0; i < count; i++)
	{
		// sequenceParameterSetLength
		nal_len = (OMX_S32)(((OMX_U32)p_avcc[0] << 8) | p_avcc[1]);
		spspps_len += 4+nal_len;
		p_avcc += nal_len+2;
	}

	// numOfPictureParameterSets
	count = *p_avcc++;

	for(i = 0; i < count; i++)
	{
		// pictureParameterSetLength
		nal_len = (OMX_S32)(((OMX_U32)p_avcc[0] << 8) | p_avcc[1]);
		spspps_len += 4+nal_len;
		p_avcc += nal_len+2;
	}

	return spspps_len;
}

static
OMX_BOOL
ConvertAvccToAnnexbSpaPps(
	const OMX_U8  *pbyAvcc,
	OMX_S32        lAvccLen,
	OMX_U8        *pbyAnnexb,
	OMX_S32       *plAnnexb
	)
{
	OMX_S32	nal_len;
	OMX_S32	count;
	OMX_S32	i;

	const OMX_U8  *p_avcc = pbyAvcc+5;
	OMX_U8        *p_annexb = pbyAnnexb;

	if( lAvccLen < 7 ) {
		ERROR("ConvertAvccToAnnexbSpaPps() - avcC is too short (len: %d) \n", lAvccLen);
		return OMX_FALSE;
	}

	// configurationVersion
	if( *pbyAvcc != 1 ) {
		ERROR("ConvertAvccToAnnexbSpaPps() - Unknown avcC version %d\n", *p_avcc);
		return OMX_FALSE;
	}

	// lengthSizeMinusOne
	//*plNalLenSize = (pbyAvcc[4] & 0x03)+1;

	// numOfSequenceParameterSets
	count = (*p_avcc++) & 0x1F;

	for(i = 0; i < count; i++)
	{
		// sequenceParameterSetLength
		nal_len = (OMX_S32)(((OMX_U32)p_avcc[0] << 8) | p_avcc[1]);

		// insert start_code
		*p_annexb++ = 0;
		*p_annexb++ = 0;
		*p_annexb++ = 0;
		*p_annexb++ = 1;

		//sequenceParameterSetNALUnit
		memcpy(p_annexb, p_avcc+2, nal_len);

		p_annexb += nal_len;
		p_avcc += nal_len+2;
	}

	// numOfPictureParameterSets
	count = *p_avcc++;

	for(i = 0; i < count; i++)
	{
		// pictureParameterSetLength
		nal_len = (OMX_S32)(((OMX_U32)p_avcc[0] << 8) | p_avcc[1]);

		// insert start_code
		*p_annexb++ = 0;
		*p_annexb++ = 0;
		*p_annexb++ = 0;
		*p_annexb++ = 1;

		// pictureParameterSetNALUnit
		memcpy(p_annexb, p_avcc+2, nal_len);

		p_annexb += nal_len;
		p_avcc += nal_len+2;
	}

	*plAnnexb = (OMX_S32)(p_annexb-pbyAnnexb);

	return OMX_TRUE;
}


static
OMX_S32
ScanSequenceHeader(
	OMX_BUFFERHEADERTYPE	*pInputBuffer,
	OMX_S32                  lStreamType
	)
{
	OMX_U8 *p_buff = pInputBuffer->pBuffer + pInputBuffer->nOffset;
	OMX_U32 data_len = pInputBuffer->nFilledLen;
	OMX_U8 *p_buff_start = p_buff;
	OMX_U8 *p_buff_end = p_buff + data_len - 4;
	OMX_U32 syncword = 0xFFFFFFFF;

	syncword = ((OMX_U32)p_buff[0] << 16) | ((OMX_U32)p_buff[1] << 8) | (OMX_U32)p_buff[2];
	if( lStreamType == STD_AVC || lStreamType == STD_MVC ) 
	{
		while( p_buff < p_buff_end ) {
			syncword <<= 8;
			syncword |= p_buff[3];
			if( (syncword >> 8) == 0x000001 && (syncword & 0x1F) == 7 ) // nal_type: SPS
				return (OMX_S32)(p_buff - p_buff_start);
			p_buff++;
		}
	}
	else if( lStreamType == STD_MPEG4 )
	{
		while( p_buff < p_buff_end ) {
			syncword <<= 8;
			syncword |= p_buff[3];
			if( (syncword >> 8) == 0x000001 && syncword <= 0x0000011F ) // video_object_start_code
				return (OMX_S32)(p_buff - p_buff_start);
			p_buff++;
		}
	}
	else
	{
		OMX_U32 seqhead_sig = 0;

		switch( lStreamType )
		{
		case STD_VC1:
			seqhead_sig = 0x0000010F; //VC-1 sequence start code
			break;
		case STD_MPEG2:
			seqhead_sig = 0x000001B3; //MPEG video sequence start code
			break;
		case STD_AVS:
			seqhead_sig = 0x000001B0; //AVS sequence start code
			break;
		default:
			ERROR("ScanSequenceHeader() - not supported stream type (type: %d)", lStreamType);
			return -1;
		}

		while( p_buff < p_buff_end ) {
			syncword <<= 8;
			syncword |= p_buff[3];
			if( syncword == seqhead_sig )
				return (OMX_S32)(p_buff - p_buff_start);
			p_buff++;
		}
	}

	ERROR("ScanSequenceHeader() - not found (type: %d)", lStreamType);
	return -1;
}

static
OMX_BOOL
StoreSequenceHeader(
	vpudec_private_t    *pstVpuPrivate,
	OMX_U8              *pSequenceHeader,
	OMX_S32              nHeaderLength
	)
{
#if defined(TC_SECURE_MEMORY_COPY)
	if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) )
	{
		ringbuff_state_t *p_rb_state = &pstVpuPrivate->stRingBuffState;

		if( pstVpuPrivate->lSeqBuffMapSize <= 0 ) {
			ERROR("StoreSequenceHeader() - invalid map size of the sequence header buffer");
			return OMX_FALSE;
		}

		if( p_rb_state->pRingBuffBase[VA] <= pSequenceHeader && pSequenceHeader <= p_rb_state->pRingBuffEnd[VA] )
			TC_SecureMemoryCopy((unsigned int)(pstVpuPrivate->pSeqBuffBase[PA]), (unsigned int)(p_rb_state->pRingBuffBase[PA] + (pSequenceHeader - p_rb_state->pRingBuffBase[VA])), (unsigned int)(nHeaderLength));
		else
			memcpy(pstVpuPrivate->pSeqBuffBase[VA], pSequenceHeader, nHeaderLength);

		pstVpuPrivate->lSeqHeaderLength = nHeaderLength;
	}
	else 
#endif
	{
		if( pstVpuPrivate->pSequenceHeader ){
			TCC_free(pstVpuPrivate->pSequenceHeader);
			pstVpuPrivate->lSeqHeaderLength = 0;
		}
	
		pstVpuPrivate->pSequenceHeader = (OMX_U8*)TCC_malloc(nHeaderLength);
		if( pstVpuPrivate->pSequenceHeader == NULL ) {
			ERROR("StoreSequenceHeader() - out of memory");
			return OMX_FALSE;
		}
		memcpy(pstVpuPrivate->pSequenceHeader, pSequenceHeader, nHeaderLength);
		pstVpuPrivate->lSeqHeaderLength = nHeaderLength;
	}

	INFO("Sequence header stored");

	return OMX_TRUE;
}

static
OMX_BOOL
StoreConfigData(
	vpudec_private_t      *pstVpuPrivate,
	OMX_BUFFERHEADERTYPE  *pInputBuffer
	)
{
	OMX_U8 *p_buff = pInputBuffer->pBuffer + pInputBuffer->nOffset;
	OMX_U32 data_len = pInputBuffer->nFilledLen;

	switch( pstVpuPrivate->stVpuInit.m_iBitstreamFormat ) {
	case STD_MVC:
	case STD_AVC:
		{
			OMX_BOOL is_avcc = OMX_TRUE;
			OMX_U32 syncword;
			OMX_U8 *p_tmp = p_buff;

			// check AvcC box format
			syncword = *p_tmp++;
			syncword <<= 8; syncword |= *p_tmp++;
			syncword <<= 8; syncword |= *p_tmp++;
			syncword <<= 8; syncword |= *p_tmp++;
			if( (syncword >> 8) == 0x000001 ) {
				is_avcc = OMX_FALSE;
			}
			else {
				syncword <<= 8;
				syncword |= *p_tmp++;
				if( (syncword >> 8) == 0x000001 ) {
					is_avcc = OMX_FALSE;
				}
			}

			if( data_len < 7 ) {
				is_avcc = OMX_FALSE;
			}

			if( is_avcc ) {
				pstVpuPrivate->lSeqHeaderLength = GetAvccToAnnexbSpaPpsLength(p_buff, data_len);
				if( pstVpuPrivate->lSeqHeaderLength )
				{
#if defined(TC_SECURE_MEMORY_COPY)
					if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) )
					{
						if( pstVpuPrivate->lSeqBuffMapSize <= 0 ) {
							ERROR("StoreConfigData() - invalid map size of the sequence header buffer");
							return OMX_FALSE;
						}

						ConvertAvccToAnnexbSpaPps(p_buff, data_len, pstVpuPrivate->pSeqBuffBase[VA], &pstVpuPrivate->lSeqHeaderLength);
					}
					else
#endif
					{
						if( pstVpuPrivate->pSequenceHeader ){
							TCC_free(pstVpuPrivate->pSequenceHeader);
							pstVpuPrivate->lSeqHeaderLength = 0;
						}
	
						pstVpuPrivate->pSequenceHeader = (OMX_U8*)TCC_malloc(pstVpuPrivate->lSeqHeaderLength);
						if( pstVpuPrivate->pSequenceHeader == NULL ) {
							ERROR("StoreConfigData() - out of memory");
							return OMX_FALSE;
						}
						ConvertAvccToAnnexbSpaPps(p_buff, data_len, pstVpuPrivate->pSequenceHeader, &pstVpuPrivate->lSeqHeaderLength);
					}
				}
			}
			else {
				if( StoreSequenceHeader(pstVpuPrivate, p_buff, data_len) == OMX_FALSE ) {
					ERROR("StoreSequenceHeader() - failed");
					return OMX_FALSE;
				}
			}
		}
		break;
	case STD_VC1:
		break;
	case STD_MPEG4:
		break;
	case STD_MPEG2:
	case STD_H263:
	case STD_DIV3:
	case STD_RV:
	case STD_AVS:
	case STD_SH263:
	case STD_MJPG:
	case STD_VP8:
	case STD_THEORA:
	default:
		if( StoreSequenceHeader(pstVpuPrivate, p_buff, data_len) == OMX_FALSE ) {
			ERROR("StoreSequenceHeader() - failed");
			return OMX_FALSE;
		}
	}

	return OMX_TRUE;
}

static
OMX_BOOL
FindTccConfigData(
	vpudec_private_t      *pstVpuPrivate,
	OMX_BUFFERHEADERTYPE  *pInputBuffer
	)
{
	OMX_U32 data_len = pInputBuffer->nFilledLen;
	OMX_U8 *p_buff = pInputBuffer->pBuffer + pInputBuffer->nOffset + data_len - sizeof(TCCVideoConfigData);

	if(data_len >= sizeof(TCCVideoConfigData))
	{
		TCCVideoConfigData *p_config_data = (TCCVideoConfigData*)p_buff;
		OMX_S32 info_len = sizeof(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo);

		if( strncmp(p_config_data->id, TCC_VIDEO_CONFIG_ID, 9) == 0 )
		{
			omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
			OMX_S32 codec_type = pstVpuPrivate->stVpuInit.m_iBitstreamFormat;

			pstVpuPrivate->ulExtractorType = OMX_BUFFERFLAG_EXTRACTORTYPE_TCC;
			pstVpuPrivate->ulContainerType = p_config_data->iContainerType;
			pstVpuPrivate->stVpuUserInfo.bitrate_mbps = p_config_data->iBitRate;
			pstVpuPrivate->stVpuUserInfo.frame_rate = p_config_data->iFrameRate;

			memset(&(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo), 0, info_len);

			//sync with parser!!
			if(    codec_type == STD_RV 
				|| codec_type == STD_DIV3 
				|| codec_type == STD_MPEG2 
				|| codec_type == STD_VC1 
			   )
			{		
				p_buff -= info_len;
				memcpy(&(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo), (char*)p_buff, info_len);
			}

			#if 0	// WMV7/8 is software codec.
			if( codec_type == STD_WMV78 && pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iExtraDataLen ) {
				OMX_S32 ext_len = pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iExtraDataLen;
				if( pstVpuPrivate->pbyExtraDataBuff ) {
					TCC_free(pstVpuPrivate->pbyExtraDataBuff);
					pstVpuPrivate->pbyExtraDataBuff = NULL;
					pstVpuPrivate->lExtraDataLength = 0;
				}
				pstVpuPrivate->pbyExtraDataBuff = TCC_calloc(1, ext_len); 
				if( pstVpuPrivate->pbyExtraDataBuff ) {
					memcpy(pstVpuPrivate->pbyExtraDataBuff, pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_pExtraData, ext_len);
					pstVpuPrivate->lExtraDataLength = ext_len;
				}
				else {
					ERROR("FindTccConfigData() - out of memory");
					pstVpuPrivate->lExtraDataLength = 0;
				}
			}
			#endif

			INFO("TC CONFIGURATION INFORMATION");
			INFO(" - Resolution : %d x %d", p_outport->sPortParam.format.video.nFrameWidth, p_outport->sPortParam.format.video.nFrameHeight);
			INFO(" - Bit-rate   : %d Mbps", pstVpuPrivate->stVpuUserInfo.bitrate_mbps);
			INFO(" - Frame-rate : %.3f fps", (float)pstVpuPrivate->stVpuUserInfo.frame_rate/1000);
			INFO(" - Container  : %d", pstVpuPrivate->ulContainerType);
			INFO(" - FourCC     : %c%c%c%c (0x%08X)", 
				 (char)(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFourCC>>0), 
				 (char)(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFourCC>>8),
				 (char)(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFourCC>>16),
				 (char)(pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFourCC>>24),
				 pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFourCC);
			INFO(" - Extra Data : %d byte", pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iExtraDataLen);

			pInputBuffer->nFilledLen -= sizeof(TCCVideoConfigData);

			return OMX_TRUE;
		}
	}
	else
	{
		if(pstVpuPrivate->ulExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC)
			ERROR("FindTccConfigData() - Error: TCCVideoConfigData is truncated");
	}

	pstVpuPrivate->stVpuUserInfo.bitrate_mbps = 0;
	pstVpuPrivate->stVpuUserInfo.frame_rate = 0;

	return OMX_FALSE;
}

static 
OMX_S32
GetDecodedFrameType(
	vpudec_private_t   *pstVpuPrivate
	)
{
	#define FRAME_TYPE_UNKNOWN   0
	#define FRAME_TYPE_I         1
	#define FRAME_TYPE_P         2
	#define FRAME_TYPE_B         3
	#define FRAME_TYPE_PB        4

	OMX_S32 pic_type   = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iPicType;
	OMX_S32 pic_struct = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iPictureStructure;

	switch ( pstVpuPrivate->stVpuInit.m_iBitstreamFormat )
	{
	case STD_VC1 :
		switch( pic_type >> 3 ) { //Frame or FIELD_INTERLACED(TOP FIELD)
		case PIC_TYPE_I:      return FRAME_TYPE_I; //I
		case PIC_TYPE_P:      return FRAME_TYPE_P; //P
		case 2:               return FRAME_TYPE_B; //BI
		case 3:               return FRAME_TYPE_B; //B
		case 4:               return FRAME_TYPE_B; //SKIP
		}
		if( pic_struct == 3 ) {
			switch( pic_type & 0x7 ) { // FIELD_INTERLACED(BOTTOM FIELD)
			case PIC_TYPE_I:  return FRAME_TYPE_I; //I
			case PIC_TYPE_P:  return FRAME_TYPE_P; //P
			case 2:           return FRAME_TYPE_B; //BI
			case 3:           return FRAME_TYPE_B; //B
			case 4:           return FRAME_TYPE_B; //SKIP
			}
		}
		break;
	case STD_MPEG4 :
		switch( pic_type ) {
		case PIC_TYPE_I:      return FRAME_TYPE_I;  //I
		case PIC_TYPE_P:      return FRAME_TYPE_P;  //P
		case PIC_TYPE_B:      return FRAME_TYPE_B;  //B
		case PIC_TYPE_B_PB:   return FRAME_TYPE_PB; //B of Packed PB-frame
		}
		break;
	case STD_MPEG2 :
	default:
		switch( pic_type ) {
		case PIC_TYPE_I:      return FRAME_TYPE_I; //I
		case PIC_TYPE_P:      return FRAME_TYPE_P; //P
		case PIC_TYPE_B:      return FRAME_TYPE_B; //B
		}
	}

	return FRAME_TYPE_UNKNOWN;
}

static
void
IFrameSearchEnable(
	vpudec_private_t      *pstVpuPrivate
	)
{
#if AVC_DECODE_ONLY_MODE_ENABLE //[2]
	if( CHECK_FLAG(pstVpuPrivate, STATE_DECODE_ONLY_USED) && 
		!CHECK_FLAG(pstVpuPrivate, STATE_DECODE_BUFFER_OUTPUT) &&
		!CHECK_FLAG(pstVpuPrivate, STATE_WAIT_FIRST_SYNCFRAME) ) 
	{
		SET_FLAG(pstVpuPrivate, STATE_DECODE_ONLY);
		SET_FLAG(pstVpuPrivate, STATE_IDR_SLICE_SCAN);
		pstVpuPrivate->lDecFrameSkipCount   = 0;
		pstVpuPrivate->lFrameSkipErrorCount = 0;
	}
#endif

	if(    pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC
		|| pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MVC )
	{
#if AVC_DECODE_ONLY_MODE_ENABLE
		//IDR picture search
		if( pstVpuPrivate->ulContainerType == CONTAINER_TYPE_MPG ||
		    pstVpuPrivate->ulContainerType == CONTAINER_TYPE_TS ||
			pstVpuPrivate->ulExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_MPEG2TS )
		{
			if( CHECK_FLAG(pstVpuPrivate, STATE_WAIT_FIRST_SYNCFRAME) )
				SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONIDR_SEARCH);
			else {
				pstVpuPrivate->lIdrSliceScanCount = 0;
				SetFrameSkipMode(pstVpuPrivate, SKIPMODE_IDR_SEARCH);
			}
		}
		else if( pstVpuPrivate->ulExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC )
			SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONIDR_SEARCH);
#else
		if( pstVpuPrivate->ulExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_TCC )
			SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONDIR_SEARCH);
#endif
	}
	else 
		SetFrameSkipMode(pstVpuPrivate, SKIPMODE_I_SEARCH);
}

static
void
SetFrameSkipMode(
	vpudec_private_t      *pstVpuPrivate,
	OMX_S32                lSkipMode
	)
{
	switch( lSkipMode ) 
	{
	case SKIPMODE_NONE:
	case SKIPMODE_NEXT_B_SKIP:
		pstVpuPrivate->lFrameSkipMode = lSkipMode;
		pstVpuPrivate->stVpuInput.m_iSkipFrameNum      = 0;
		pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = 0;
		pstVpuPrivate->stVpuInput.m_iSkipFrameMode     = 0;	
		LOG_SKIP("Disabled");
		break;

	case SKIPMODE_IDR_SEARCH:
		pstVpuPrivate->lFrameSkipMode = SKIPMODE_I_SEARCH;
		pstVpuPrivate->stVpuInput.m_iSkipFrameNum      = 0;
		pstVpuPrivate->stVpuInput.m_iSkipFrameMode     = 0;
		pstVpuPrivate->lOutputFailCount                = 0;

		if( CHECK_FLAG(pstVpuPrivate, STATE_DECODE_BUFFER_OUTPUT) ) {
			pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_NONIDR_PICTURE_SEARCH_MODE;
			LOG_SKIP("I-frame Search (non-IDR slice search)");
		}
		else {
			pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_IDR_PICTURE_SEARCH_MODE;
			LOG_SKIP("I-frame Search (IDR slice search)");
		}
		break;

	case SKIPMODE_NONIDR_SEARCH:
		pstVpuPrivate->lFrameSkipMode = SKIPMODE_I_SEARCH;
		pstVpuPrivate->stVpuInput.m_iSkipFrameNum      = 0;
		pstVpuPrivate->stVpuInput.m_iSkipFrameMode     = 0;
		pstVpuPrivate->lOutputFailCount                = 0;
		pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_NONIDR_PICTURE_SEARCH_MODE;
		LOG_SKIP("I-frame Search (non-IDR slice search)");
		break;

	case SKIPMODE_I_SEARCH:
		pstVpuPrivate->lFrameSkipMode = SKIPMODE_I_SEARCH;
		pstVpuPrivate->stVpuInput.m_iSkipFrameNum      = 0;
		pstVpuPrivate->stVpuInput.m_iSkipFrameMode     = 0;
		pstVpuPrivate->lOutputFailCount                = 0;

		if(    pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC 
			|| pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MVC )
		{
			if( CHECK_FLAG(pstVpuPrivate, STATE_DECODE_BUFFER_OUTPUT) ) {
				pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_NONIDR_PICTURE_SEARCH_MODE;
				LOG_SKIP("I-frame Search (non-IDR slice search)");
			}
			else {
				//IDR picture search
				if( pstVpuPrivate->ulContainerType == CONTAINER_TYPE_MPG ||
					pstVpuPrivate->ulContainerType == CONTAINER_TYPE_TS ||
					pstVpuPrivate->ulExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_MPEG2TS ) 
				{
					pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_IDR_PICTURE_SEARCH_MODE;
					LOG_SKIP("I-frame Search (IDR slice search)");
				}
				else {
					pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_NONIDR_PICTURE_SEARCH_MODE;
					LOG_SKIP("I-frame Search (non-IDR slice search)");
				}
			}
		}
		else
		{
			pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = AVC_IDR_PICTURE_SEARCH_MODE;
			LOG_SKIP("I-frame Search");
		}
		break;

	case SKIPMODE_B_SKIP:
		pstVpuPrivate->lFrameSkipMode = lSkipMode;
		pstVpuPrivate->stVpuInput.m_iSkipFrameNum      = 1;
		pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = 0;
		pstVpuPrivate->stVpuInput.m_iSkipFrameMode     = VDEC_SKIP_FRAME_ONLY_B;
		LOG_SKIP("B-frame Skip");
		break;

	case SKIPMODE_EXCEPT_I_SKIP:
		pstVpuPrivate->lFrameSkipMode = lSkipMode;
		pstVpuPrivate->stVpuInput.m_iSkipFrameNum      = 1;
		pstVpuPrivate->stVpuInput.m_iFrameSearchEnable = 0;
		pstVpuPrivate->stVpuInput.m_iSkipFrameMode     = VDEC_SKIP_FRAME_EXCEPT_I;
		LOG_SKIP("Except-I-frame Skip");
		break;

	case SKIPMODE_NEXT_STEP:
		switch( pstVpuPrivate->lFrameSkipMode ) 
		{
		case SKIPMODE_I_SEARCH:
			ASSERT( GetDecodedFrameType(pstVpuPrivate) == FRAME_TYPE_I );

			if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat != STD_MVC )
				SetFrameSkipMode(pstVpuPrivate, SKIPMODE_B_SKIP);
			else if( pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded == 0 )
				SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NEXT_B_SKIP);
			break;

		case SKIPMODE_NEXT_B_SKIP:
			SetFrameSkipMode(pstVpuPrivate, SKIPMODE_B_SKIP);
			break;

		case SKIPMODE_B_SKIP:
			ASSERT( GetDecodedFrameType(pstVpuPrivate) != FRAME_TYPE_B );
			if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat != STD_MVC )
				SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONE);
			else if( pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded == 1 )
				SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONE);

#if AVC_DECODE_ONLY_MODE_ENABLE //[3]
			if( CHECK_FLAG(pstVpuPrivate, STATE_DECODE_ONLY) ) {
				if( pstVpuPrivate->stVpuInput.m_iFrameSearchEnable == AVC_NONIDR_PICTURE_SEARCH_MODE )
					pstVpuPrivate->lDecFrameSkipCount = pstVpuPrivate->lDecFrameSkipMax - 3;
				else
					pstVpuPrivate->lDecFrameSkipCount = 0;
			}
#endif
			break;

		case SKIPMODE_EXCEPT_I_SKIP:
			ASSERT( GetDecodedFrameType(pstVpuPrivate) == FRAME_TYPE_I );
			SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONE);
#if AVC_DECODE_ONLY_MODE_ENABLE
			if( CHECK_FLAG(pstVpuPrivate, STATE_DECODE_ONLY) ) {
				if( pstVpuPrivate->stVpuInput.m_iFrameSearchEnable == AVC_NONIDR_PICTURE_SEARCH_MODE )
					pstVpuPrivate->lDecFrameSkipCount = pstVpuPrivate->lDecFrameSkipMax - 3;
				else
					pstVpuPrivate->lDecFrameSkipCount = 0;
			}
#endif
			break;

		default:
			SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NONE);
		}
		break;
	}
}

static
void
CheckSkipSignal(
	vpudec_private_t      *pstVpuPrivate
	)
{
	char value[PROPERTY_VALUE_MAX];
	property_get("tcc.video.skip.flag", value, "0");
	if ( value[0] == '1' ) {
		SetFrameSkipMode(pstVpuPrivate, SKIPMODE_B_SKIP);
		property_set("tcc.video.skip.flag", "0");
	}
}

#if AVC_DECODE_ONLY_MODE_ENABLE
static
OMX_S32
ScanAvcIdrSlice(
	OMX_BUFFERHEADERTYPE	*pInputBuffer
	)
{
	OMX_U8 *p_buff = pInputBuffer->pBuffer + pInputBuffer->nOffset;
	OMX_U32 data_len = pInputBuffer->nFilledLen;
	OMX_U8 *p_buff_start = p_buff;
	OMX_U8 *p_buff_end = p_buff + data_len - 4;
	OMX_U32 syncword = 0xFFFFFFFF;

	syncword = ((OMX_U32)p_buff[0] << 16) | ((OMX_U32)p_buff[1] << 8) | (OMX_U32)p_buff[2];
	while( p_buff < p_buff_end ) {
		syncword <<= 8;
		syncword |= p_buff[3];
		if( (syncword >> 8) == 0x000001 && (syncword & 0x1F) == 5 ) //IDR slice
			return (OMX_S32)(p_buff - p_buff_start);

		p_buff++;
	}
	return -1;
}

static
void
ConfigDecodeOnlyMode(
	vpudec_private_t      *pstVpuPrivate
	)
{
	char value[PROPERTY_VALUE_MAX];

	INFO("DECODE-ONLY MODE CONFIGURATION");

	property_get("tcc.avc.seektype", value, "0");
	INFO(" - tcc.avc.seektype : %s", value);

	if( value[0] != '0' ) {
		// use IDR slice search
		pstVpuPrivate->lOutputFailMax = VPU_OUTPUT_IDR_FAIL_MAX;
	} 
	else
	{
		property_get("tcc.video.skip.disp", value, "1");
		INFO(" - tcc.video.skip.disp : %s", value);

		if( value[0] == '0' )
			CLEAR_FLAG(pstVpuPrivate, STATE_DECODE_ONLY_USED);
		else
			SET_FLAG(pstVpuPrivate, STATE_DECODE_ONLY_USED);
		
		if( CHECK_FLAG(pstVpuPrivate, STATE_DECODE_ONLY_USED) )
		{
			OMX_S32 skip_max;
			property_get("tcc.video.skip.disp.num", value, "0");
			INFO(" - tcc.video.skip.disp.num : %s", value);
			skip_max = (OMX_S32)atoi(value);
			if( skip_max > pstVpuPrivate->lRefBuffCount )
				pstVpuPrivate->lDecFrameSkipMax = skip_max;
		}
	}
}
#endif //AVC_DECODE_ONLY_MODE_ENABLE


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	VPU Ring-buffer operators
//
//
static OMX_BOOL InitRingBuffer(ringbuff_state_t *pstRingbuff, OMX_PTR pVpuInstance);
static OMX_BOOL FillRingBuffer(ringbuff_state_t *pstRingbuff, OMX_U8 *pInputBuff, OMX_S32 lInputLength);
static OMX_S32 UpdateRingBuffer(ringbuff_state_t *pstRingbuff, OMX_BOOL bFromVpu);
static OMX_S32 ResetRingBuffer(ringbuff_state_t *pstRingbuff);

static
OMX_BOOL
InitRingBuffer(
	ringbuff_state_t   *pstRingbuff,
	OMX_PTR             pVpuInstance
	)
{
	vdec_ring_buffer_out_t ring_stat; 
	OMX_S32 ret = 0;

	INFO("VPU ring-buffer initialize");

	pstRingbuff->pVpuInstance = pVpuInstance;
	pstRingbuff->pRingBuffBase[PA] = vpu_getBitstreamBufAddr(PA, pVpuInstance);
	pstRingbuff->pRingBuffBase[VA] = vpu_getBitstreamBufAddr(VA, pVpuInstance);
	pstRingbuff->lRingBuffSize = vpu_getBitstreamBufSize(pVpuInstance);
	pstRingbuff->pRingBuffEnd[PA] = pstRingbuff->pRingBuffBase[PA] + pstRingbuff->lRingBuffSize;
	pstRingbuff->pRingBuffEnd[VA] = pstRingbuff->pRingBuffBase[VA] + pstRingbuff->lRingBuffSize;

	ResetRingBuffer(pstRingbuff);

	if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pVpuInstance)) < 0 )
	{
		ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
		return OMX_FALSE;
	}

	LOG_RING("[INIT     ] - RING-BUFFER Environment - PA: 0x%08X / VA: 0x%08X / size: %d"
			 , pstRingbuff->pRingBuffBase[PA]
			 , pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->lRingBuffSize);

	LOG_RING("[INIT     ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes]"
			 , ring_stat.m_ptrReadAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]
			 , ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]
			 , pstRingbuff->lEmptySpace
			 , pstRingbuff->lWrittenBytes);

	return OMX_TRUE;
}

static
OMX_BOOL
FillRingBuffer(
	ringbuff_state_t        *pstRingbuff,
	OMX_U8                  *pInputBuff,
	OMX_S32                  lInputLength
	)
{
	OMX_BOOL ret = OMX_FALSE;

	if( pstRingbuff->lEmptySpace > (lInputLength + EMPTY_SIZE_MIN) ) 
	{
		OMX_U8  *p_wp        = pstRingbuff->pWritePtr;
		OMX_U8  *p_data      = pInputBuff;
		OMX_S32  data_size   = lInputLength;
		OMX_S32  tmp_size    = (OMX_S32)(pstRingbuff->pRingBuffEnd[VA]-p_wp);

		pstRingbuff->pPrevWritePtr = p_wp;

		if( tmp_size < data_size ) {
			OMX_U8  *p_ring_base = pstRingbuff->pRingBuffBase[VA];

			memcpy(p_wp, p_data, tmp_size);
			data_size -= tmp_size;
			p_data += tmp_size;

			memcpy(p_ring_base, p_data, data_size);
			p_wp = p_ring_base + data_size;
		}
		else {
			memcpy(p_wp, p_data, data_size);
			p_wp += data_size;
		}

		pstRingbuff->pWritePtr = p_wp;
		pstRingbuff->lWrittenBytes += lInputLength;
		pstRingbuff->lEmptySpace -= lInputLength;

		ret = OMX_TRUE;
	}

	LOG_RING("[FILL     ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes] [INPUT: %7d bytes] %s"
			 , pstRingbuff->pReadPtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->pWritePtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->lEmptySpace
			 , pstRingbuff->lWrittenBytes
			 , lInputLength
			 , ret == OMX_FALSE ? "- [NOT ENOUGH]" : ""
			 );

	return ret;
}

#if defined(TC_SECURE_MEMORY_COPY)	
static
OMX_BOOL
FillRingBuffer_secure(
	ringbuff_state_t        *pstRingbuff,
	OMX_U8                  *pPhyInputBuff,	// physical address
	OMX_S32                  lInputLength
	)
{
	OMX_BOOL ret = OMX_FALSE;

	if( pstRingbuff->lEmptySpace > (lInputLength + EMPTY_SIZE_MIN) ) 
	{
		OMX_U8  *p_wp        = pstRingbuff->pRingBuffBase[PA] + (pstRingbuff->pWritePtr - pstRingbuff->pRingBuffBase[VA]);
		OMX_U8  *p_data      = pPhyInputBuff;
		OMX_S32  data_size   = lInputLength;
		OMX_S32  tmp_size    = (OMX_S32)(pstRingbuff->pRingBuffEnd[PA]-p_wp);

		pstRingbuff->pPrevWritePtr = pstRingbuff->pWritePtr;

		if( tmp_size < data_size ) {
			OMX_U8  *p_ring_base = pstRingbuff->pRingBuffBase[PA];

			TC_SecureMemoryCopy((unsigned int)p_wp, (unsigned int)p_data, (unsigned int)tmp_size);
			data_size -= tmp_size;
			p_data += tmp_size;

			TC_SecureMemoryCopy((unsigned int)p_ring_base, (unsigned int)p_data, (unsigned int)data_size);
			p_wp = p_ring_base + data_size;
		}
		else {
			TC_SecureMemoryCopy((unsigned int)p_wp, (unsigned int)p_data, (unsigned int)data_size);
			p_wp += data_size;
		}

		pstRingbuff->pWritePtr = pstRingbuff->pRingBuffBase[VA] + (p_wp - pstRingbuff->pRingBuffBase[PA]);
		pstRingbuff->lWrittenBytes += lInputLength;
		pstRingbuff->lEmptySpace -= lInputLength;

		ret = OMX_TRUE;
	}

	LOG_RING("[FILL     ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes] [INPUT: %7d bytes] %s"
			 , pstRingbuff->pReadPtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->pWritePtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->lEmptySpace
			 , pstRingbuff->lWrittenBytes
			 , lInputLength
			 , ret == OMX_FALSE ? "- [NOT ENOUGH]" : ""
			 );

	return ret;
}
#endif

static
OMX_S32
UpdateRingBuffer(
	ringbuff_state_t        *pstRingbuff,
	OMX_BOOL                 bFromVpu
	)
{
	OMX_S32 ret = 0;

	if( bFromVpu )
	{
		vdec_ring_buffer_out_t ring_stat; 
		OMX_U8 *p_ring_base_pa = pstRingbuff->pRingBuffBase[PA];
		OMX_U8 *p_ring_base_va = pstRingbuff->pRingBuffBase[VA];
		OMX_U8 *p_prev_rp;
		OMX_U8 *p_curr_rp;
		OMX_S32 ret = 0;

		if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pstRingbuff->pVpuInstance)) < 0 ) {
			ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
			return ret;
		}

		ASSERT( ((pstRingbuff->pReadPtr < pstRingbuff->pWritePtr) && (ring_stat.m_ptrReadAddr_PA <= ring_stat.m_ptrWriteAddr_PA)) 
				|| ((pstRingbuff->pReadPtr > pstRingbuff->pWritePtr)) );

		#if USE_AVAILABLE_SIZE_FROM_VPU
		ASSERT( (pstRingbuff->lEmptySpace + pstRingbuff->lWrittenBytes) <= (ring_stat.m_ulAvailableSpaceInRingBuffer) );
		#endif

		/* prev read pointer */
		p_prev_rp = pstRingbuff->pReadPtr;
		pstRingbuff->pPrevReadPtr = p_prev_rp;

		/* read pointer */
		p_curr_rp = p_ring_base_va + ((OMX_U8*)ring_stat.m_ptrReadAddr_PA - p_ring_base_pa);
		pstRingbuff->pReadPtr = p_curr_rp;

		/* used bytes */
		if( p_curr_rp < p_prev_rp )
			pstRingbuff->lUsedByte = (p_curr_rp - p_ring_base_va) + (pstRingbuff->pRingBuffEnd[VA] - p_prev_rp);
		else
			pstRingbuff->lUsedByte = p_curr_rp - p_prev_rp;
		ASSERT( pstRingbuff->lUsedByte <= (pstRingbuff->lRingBuffSize - (pstRingbuff->lEmptySpace + pstRingbuff->lWrittenBytes)) );

		/* empty size */
		#if USE_AVAILABLE_SIZE_FROM_VPU
		pstRingbuff->lEmptySpace = ring_stat.m_ulAvailableSpaceInRingBuffer - pstRingbuff->lWrittenBytes;
		#else
		{
			OMX_S32 temp_size;
			if( ring_stat.m_ptrReadAddr_PA <= ring_stat.m_ptrWriteAddr_PA )
				temp_size = pstRingbuff->lRingBuffSize - (OMX_S32)(ring_stat.m_ptrWriteAddr_PA - ring_stat.m_ptrReadAddr_PA);
			else
				temp_size = (OMX_S32)(ring_stat.m_ptrReadAddr_PA - ring_stat.m_ptrWriteAddr_PA);
			temp_size -= pstRingbuff->lWrittenBytes;
			pstRingbuff->lEmptySpace = temp_size;
		}
		#endif

		LOG_RING("[UPDATE-DN] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes] [USED : %7d bytes]"
				 , ring_stat.m_ptrReadAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]
				 , ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]
				 , pstRingbuff->lEmptySpace
				 , pstRingbuff->lWrittenBytes
				 , pstRingbuff->lUsedByte
				 );
	}
	else 
	{
#if UPDATE_WRITE_PTR_WITH_ALIGNED_LENGTH
		if( pstRingbuff->lWrittenBytes < WRITE_PTR_ALIGN_BYTES ) 
			return 0;
		else
		{
			OMX_S32 unaligned_byte = pstRingbuff->lWrittenBytes % WRITE_PTR_ALIGN_BYTES;

			if( (ret = vdec_vpu(VDEC_UPDATE_WRITE_BUFFER_PTR, NULL, (void*)(pstRingbuff->lWrittenBytes - unaligned_byte), (void*)pstRingbuff->bFlushRing, pstRingbuff->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_UPDATE_WRITE_BUFFER_PTR] [RET_CODE: %d]", ret);
				return ret;
			}
			pstRingbuff->lWrittenBytes = unaligned_byte;
			pstRingbuff->bFlushRing = OMX_FALSE;
		}
#else
		if( (ret = vdec_vpu(VDEC_UPDATE_WRITE_BUFFER_PTR, NULL, (void*)pstRingbuff->lWrittenBytes, (void*)pstRingbuff->bFlushRing, pstRingbuff->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_UPDATE_WRITE_BUFFER_PTR] [RET_CODE: %d]", ret);
			return ret;
		}
		pstRingbuff->lWrittenBytes = 0;
		pstRingbuff->bFlushRing = OMX_FALSE;
#endif

		#if CHECK_RING_STATUS_AFTER_UPDATE
		{
			vdec_ring_buffer_out_t ring_stat;
			if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pstRingbuff->pVpuInstance)) < 0 ) {
				ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
				return ret;
			}

			if( (pstRingbuff->pWritePtr-pstRingbuff->lWrittenBytes) != pstRingbuff->pRingBuffBase[VA] + (ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]) ) {
				ERROR("[LOG_RING] [RP: %d / %d] [WP: %d / %d] "
					 , pstRingbuff->pReadPtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]		//rp offset (VA)
					 , ring_stat.m_ptrReadAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]	//rp offset (PA)
					 , pstRingbuff->pWritePtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]		//wp offset (VA)
					 , ring_stat.m_ptrWriteAddr_PA-(OMX_U32)pstRingbuff->pRingBuffBase[PA]	//wp offset (PA)
					 );
			}

			#if USE_AVAILABLE_SIZE_FROM_VPU
			if( pstRingbuff->lEmptySpace != ring_stat.m_ulAvailableSpaceInRingBuffer ) {
				ERROR("[LOG_RING] EMPTY: %d / %d", pstRingbuff->lEmptySpace, ring_stat.m_ulAvailableSpaceInRingBuffer);
			}
			#else
			{
				long temp_size;
				if( ring_stat.m_ptrReadAddr_PA <= ring_stat.m_ptrWriteAddr_PA )
					temp_size = pstRingbuff->lRingBuffSize - (OMX_S32)(ring_stat.m_ptrWriteAddr_PA - ring_stat.m_ptrReadAddr_PA);
				else
					temp_size = (OMX_S32)(ring_stat.m_ptrReadAddr_PA - ring_stat.m_ptrWriteAddr_PA);
				if( pstRingbuff->lEmptySpace != temp_size ) {
					ERROR("[LOG_RING] EMPTY: %d / %d", pstRingbuff->lEmptySpace, temp_size);
				}
			}
			#endif
		}
		#endif

		LOG_RING("[UPDATE-UP] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes]"
				 , pstRingbuff->pReadPtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
				 , pstRingbuff->pWritePtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
				 , pstRingbuff->lEmptySpace
				 , pstRingbuff->lWrittenBytes
				 );
	}

	return 0;
}


static
OMX_S32
ResetRingBuffer(
	ringbuff_state_t        *pstRingbuff
	)
{
	OMX_S32 ret = 0;

	pstRingbuff->pWritePtr     = pstRingbuff->pRingBuffBase[VA];
	pstRingbuff->pPrevWritePtr = pstRingbuff->pRingBuffBase[VA];
	pstRingbuff->pReadPtr      = pstRingbuff->pRingBuffBase[VA];
	pstRingbuff->pPrevReadPtr  = pstRingbuff->pRingBuffBase[VA];
	pstRingbuff->lWrittenBytes = 0;
	pstRingbuff->lEmptySpace   = pstRingbuff->lRingBuffSize;
	pstRingbuff->bFlushRing    = OMX_TRUE;

#if FLUSH_RING_BEFORE_UPDATE_TIME
	if( (ret = vdec_vpu(VDEC_UPDATE_WRITE_BUFFER_PTR, NULL, (void*)0, (void*)pstRingbuff->bFlushRing, pstRingbuff->pVpuInstance)) < 0 )
		ERROR( "[VPU_ERROR] [OP: VDEC_UPDATE_WRITE_BUFFER_PTR] [RET_CODE: %d]", ret);
	pstRingbuff->bFlushRing = OMX_FALSE;

	if( ret == 0 ) {
		vdec_ring_buffer_out_t ring_stat; 

		if( (ret = vdec_vpu(VDEC_GET_RING_BUFFER_STATUS, NULL, NULL, &ring_stat, pstRingbuff->pVpuInstance)) < 0 )
		{
			ERROR("[VPU_ERROR] [OP: VDEC_GET_RING_BUFFER_STATUS] [RET_CODE: %d]", ret);
			return ret;
		}

		pstRingbuff->pReadPtr += ((OMX_U8*)ring_stat.m_ptrReadAddr_PA - pstRingbuff->pRingBuffBase[PA]);
		pstRingbuff->pWritePtr += ((OMX_U8*)ring_stat.m_ptrWriteAddr_PA - pstRingbuff->pRingBuffBase[PA]);
		pstRingbuff->pPrevWritePtr = pstRingbuff->pWritePtr;
		#if USE_AVAILABLE_SIZE_FROM_VPU
		pstRingbuff->lEmptySpace = ring_stat.m_ulAvailableSpaceInRingBuffer;
		#endif
	}
#else
#endif

	LOG_RING("[RESET    ] - [RP: %7d][WP: %7d] [EMPTY: %7d bytes] [BUFFERED: %7d bytes]"
			 , pstRingbuff->pReadPtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->pWritePtr-(OMX_U32)pstRingbuff->pRingBuffBase[VA]
			 , pstRingbuff->lEmptySpace
			 , pstRingbuff->lWrittenBytes
			 );

	return ret;
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	VPU Decoder operation helper
//
//
static OMX_S32 GetFrameType(OMX_S32 lVideoType, OMX_S32 lPictureType, OMX_S32 lPictureStructure);
static void SendEventToClient(OMX_COMPONENTTYPE *openmaxStandComp, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
static OMX_S32 CheckPortConfigChange(vpudec_private_t *pstVpuPrivate);
static OMX_BOOL CheckOutputModeChange(vpudec_private_t *pstVpuPrivate);
static OMX_BOOL IsResolutionChanged(vpudec_private_t *pstVpuPrivate);
static OMX_U8* CopyOutputFrame(vpudec_private_t *pstVpuPrivate, OMX_U8 *pDestAddress, OMX_U32 *pulCopyedBytes, OMX_S32 lSrcIndex);
static OMX_S32 ScanFillerSpace(vpudec_private_t *pstVpuPrivate, OMX_U8 *pScanStart, OMX_U8 *pScanEnd, OMX_U8 **ppSpaceEnd);
static OMX_TICKS GetCurrTimestamp(vpudec_private_t *pstVpuPrivate, OMX_TICKS *pllTimestamp, OMX_BOOL bDecodeSuccess, OMX_BOOL bInterlaced);
static OMX_S32 GetOutputBufferInfo(vpudec_private_t *pstVpuPrivate, OMX_S32 lBuffIdx, output_info_t *pstOutputInfo);
static OMX_S32 ClearAllDisplayBuffers(vpudec_private_t *pstVpuPrivate, OMX_BOOL bIsModeChanged, OMX_BOOL bSeek);
static OMX_S32 DisplayBufferProcess(vpudec_private_t *pstVpuPrivate, OMX_U32 ulVpuDispIdx, OMX_BOOL bDependent, OMX_U32 ulVSyncBuffId, OMX_BUFFERHEADERTYPE *pOutputBuffer);

static OMX_BOOL BackupRingBuffer(vpudec_private_t *pstVpuPrivate);
static OMX_BOOL RestoreRingBuffer(vpudec_private_t *pstVpuPrivate);

static OMX_S32 FeedVpuDecoder(vpudec_private_t *pstVpuPrivate, OMX_BUFFERHEADERTYPE *pInputBuffer);
static OMX_S32 VpuDecoderInit(vpudec_private_t *pstVpuPrivate);
static OMX_S32 VpuDecoderSeqInit(vpudec_private_t *pstVpuPrivate);
static OMX_S32 VpuDecoderDecode(vpudec_private_t *pstVpuPrivate,OMX_U32 *pulResultFlags);
static OMX_S32 VpuDecSuccessProcess(vpudec_private_t *pstVpuPrivate, OMX_TICKS llDecTimestamp, OMX_S32 lBuffIdx);
static OMX_S32 VpuOutputProcess(vpudec_private_t *pstVpuPrivate, OMX_BUFFERHEADERTYPE *pOutputBuffer, OMX_BOOL bDecBuffOutput, COPY_MODE enCopyMode);
static OMX_BOOL VpuErrorProcess(OMX_COMPONENTTYPE *openmaxStandComp, OMX_BUFFERHEADERTYPE *pInputBuffer, OMX_BUFFERHEADERTYPE *pOutputBuffer, OMX_S32 ret);
static void VpuPrepareResolutionChange(OMX_COMPONENTTYPE *openmaxStandComp);
static OMX_S32 ClearDisplayedBufferByBufferID(vpudec_private_t *pstVpuPrivate, OMX_U32 ulDispIdx, OMX_U32 ulBuffId);

static
OMX_S32
GetFrameType(
	OMX_S32 lVideoType, 
	OMX_S32 lPictureType, 
	OMX_S32 lPictureStructure 
	)
{
	int frameType = 0; //unknown

	switch ( lVideoType )
	{
	case STD_VC1 :
		switch( (lPictureType>>3) ) //Frame or // FIELD_INTERLACED(TOP FIELD)
		{
		case PIC_TYPE_I:	frameType = PIC_TYPE_I; break;//I
		case PIC_TYPE_P:	frameType = PIC_TYPE_P; break;//P
		case 2:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "BI  :" );
		case 3:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "B   :" );
		case 4:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "SKIP:" );
		}
		if( lPictureStructure == 3)
		{
			switch( (lPictureType&0x7) ) // FIELD_INTERLACED(BOTTOM FIELD)
			{
			case PIC_TYPE_I:	frameType = PIC_TYPE_I; break;//I
			case PIC_TYPE_P:	frameType = PIC_TYPE_P; break;//P
			case 2:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "BI  :" );
			case 3:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "B   :" );
			case 4:				frameType = PIC_TYPE_B; break;//B //DSTATUS( "SKIP:" );
			}
		}
		break;
	case STD_MPEG4 :
		switch( lPictureType )
		{
		case PIC_TYPE_I:	frameType = PIC_TYPE_I;	break;//I
		case PIC_TYPE_P:	frameType = PIC_TYPE_P;	break;//P
		case PIC_TYPE_B:	frameType = PIC_TYPE_B;	break;//B
		case PIC_TYPE_B_PB: frameType = PIC_TYPE_B;	break;//B of Packed PB-frame
		}
		break;
	case STD_MPEG2 :
	default:
		switch( lPictureType & 0xF )
		{
		case PIC_TYPE_I:	frameType = PIC_TYPE_I;	break;//I
		case PIC_TYPE_P:	frameType = PIC_TYPE_P;	break;//P
		case PIC_TYPE_B:	frameType = PIC_TYPE_B;	break;//B
		}
	}
	return frameType;
}

static 
void 
SendEventToClient(
	OMX_COMPONENTTYPE    *openmaxStandComp, 
	OMX_EVENTTYPE         eEvent,
	OMX_U32               nData1,
	OMX_U32               nData2
	)
{
	omx_vpudec_component_PrivateType *p_private = GET_VPUDEC_PRIVATE(openmaxStandComp);

	(*(p_private->callbacks->EventHandler))(
			openmaxStandComp, 
			p_private->callbackData,
			eEvent, 
			nData1,
			nData2, 
			NULL);
}


static 
OMX_S32 
CheckPortConfigChange(
	vpudec_private_t   *pstVpuPrivate
	)
{
#define MAX_RESOLUTION_EXCEEDED         -1
#define PORT_RECONFIGURATION_NEEDED     0x1
#define PORT_CROP_CHANGE_NEEDED         0x2
#define PORT_AR_CHANGE_NEEDED           0x4
#define PORT_CONFIGURATION_DONE         0

    omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	dec_initial_info_t *p_init_info = pstVpuPrivate->stVpuOutput.m_pInitialInfo;
	OMX_COLOR_FORMATTYPE colorformat;
	OMX_CONFIG_RECTTYPE st_crop;
	OMX_U32 width;
	OMX_U32 height;
	OMX_U32 port_width = p_outport->sPortParam.format.video.nFrameWidth;
	OMX_U32 port_height = p_outport->sPortParam.format.video.nFrameHeight;

	OMX_S32 ret = PORT_CONFIGURATION_DONE;

#if AVC_CROP_ENABLE
	if(    pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC 
		|| pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MVC )
	{	
		pic_crop_t avc_crop = p_init_info->m_iAvcPicCrop;
		width  = (p_init_info->m_iPicWidth - avc_crop.m_iCropLeft - avc_crop.m_iCropRight);
		height = (p_init_info->m_iPicHeight - avc_crop.m_iCropBottom - avc_crop.m_iCropTop);
		width  += width & 1;
		height += height & 1;

		if( port_width != width || port_height != height )
			ret |= PORT_RECONFIGURATION_NEEDED;

		st_crop.nLeft    = avc_crop.m_iCropLeft;
		st_crop.nTop     = avc_crop.m_iCropTop;
		st_crop.nWidth   = width;
		st_crop.nHeight  = height;
	}
	else
#endif
	{
		width  = p_init_info->m_iPicWidth;
		height = p_init_info->m_iPicHeight;
		width  += width & 1;
		height += height & 1;

		if( port_width != width || port_height != height )
			ret |= PORT_RECONFIGURATION_NEEDED;

		st_crop.nLeft    = 0;
		st_crop.nTop     = 0;
		st_crop.nWidth   = width;
		st_crop.nHeight  = height;
	}

	if( st_crop.nLeft   != pstVpuPrivate->stCropRectParam.nLeft ||
		st_crop.nTop    != pstVpuPrivate->stCropRectParam.nTop ||
		st_crop.nWidth  != pstVpuPrivate->stCropRectParam.nWidth ||
		st_crop.nHeight != pstVpuPrivate->stCropRectParam.nHeight)
	{
		pstVpuPrivate->stCropRectParam.nLeft    = st_crop.nLeft;
		pstVpuPrivate->stCropRectParam.nTop     = st_crop.nTop;
		pstVpuPrivate->stCropRectParam.nWidth   = st_crop.nWidth;
		pstVpuPrivate->stCropRectParam.nHeight  = st_crop.nHeight;
		ret |= PORT_CROP_CHANGE_NEEDED;
	}

	if( vdec_getAspectRatio(pstVpuPrivate->pVpuInstance, 
							&pstVpuPrivate->stScaleFactor.xWidth, 
							&pstVpuPrivate->stScaleFactor.xHeight, 
							width, height, 
							*pstVpuPrivate->pstCdmxInfo, 
							pstVpuPrivate->ulContainerType, 
							pstVpuPrivate->ulExtractorType & OMX_BUFFERFLAG_EXTRACTORTYPE_TCC) )
	{
		ret |= PORT_AR_CHANGE_NEEDED;
	}


#if RESOLUTION_SWITCHING_SUPPORT
	if( CHECK_FLAG(pstVpuPrivate, STATE_RESOLUTION_CHANGING) ) {
		ret |= PORT_RECONFIGURATION_NEEDED;
		ret |= PORT_CROP_CHANGE_NEEDED;
	}
#endif

	if(width > AVAILABLE_MAX_WIDTH || ((width *height) > AVAILABLE_MAX_REGION)) {
		ERROR("%ld x %ld ==> MAX-Resolution(%ld x %ld) over!!", width, height, AVAILABLE_MAX_WIDTH, AVAILABLE_MAX_HEIGHT);
		ret = MAX_RESOLUTION_EXCEEDED;
	}

	if(width < AVAILABLE_MIN_WIDTH || height < AVAILABLE_MIN_HEIGHT) {
		ERROR("%ld x %ld ==> MIN-Resolution(%ld x %ld) less!!", width, height, AVAILABLE_MIN_WIDTH, AVAILABLE_MIN_HEIGHT);
		ret = MAX_RESOLUTION_EXCEEDED;
	}

#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)	
	if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MJPG )
	{	
		if( pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode != 1 )
		{
			//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
			if( p_init_info->m_iMjpg_sourceFormat == 1 )
				colorformat = OMX_COLOR_FormatYUV422Planar;
			else
				colorformat = OMX_COLOR_FormatYUV420Planar;

			if( p_outport->sPortParam.format.video.eColorFormat != colorformat )
			{
				INFO("Change ColorFormat! (%d --> %d)", p_outport->sPortParam.format.video.eColorFormat, colorformat);

				if( p_init_info->m_iMjpg_sourceFormat == 1 )
					p_outport->sPortParam.nBufferSize = port_width * port_height * 2;
				else
					p_outport->sPortParam.nBufferSize = port_width * port_height * 3/2;

				p_outport->sPortParam.format.video.eColorFormat = colorformat;
				ret = PORT_RECONFIGURATION_NEEDED;				
			}
		}
	}
#endif
	
	if( ret & PORT_RECONFIGURATION_NEEDED )
	{
		INFO("Port reconfiguration is needed! (%d x %d --> %d x %d) \n", port_width, port_height, width, height);

		port_width = width;
		port_height = height;
		p_outport->sPortParam.format.video.nFrameWidth = port_width;
		p_outport->sPortParam.format.video.nFrameHeight = port_height;
		p_outport->bIsPortChanged = OMX_TRUE;		
		
		switch( p_outport->sVideoParam.eColorFormat ) {
			case OMX_COLOR_FormatYUV420Planar:
			case OMX_COLOR_FormatYUV420SemiPlanar:
				if( port_width && port_height )
					p_outport->sPortParam.nBufferSize = port_width * port_height * 3 / 2;
			break;
			default:
				if( port_width && port_height )
					p_outport->sPortParam.nBufferSize = port_width * port_height * 3;
			break;
		}

		SET_FLAG(pstVpuPrivate, STATE_GRALLOC_RESET);
	}

	if( CHECK_FLAG(pstVpuPrivate, STATE_HDMI_OUTPUT) ) {
		//max-clock setup
		vpu_update_sizeinfo(pstVpuPrivate->stVpuInit.m_iBitstreamFormat,
							pstVpuPrivate->stVpuUserInfo.bitrate_mbps,
							pstVpuPrivate->stVpuUserInfo.frame_rate,
							AVAILABLE_MAX_WIDTH,
							AVAILABLE_MAX_HEIGHT, 
							pstVpuPrivate->pVpuInstance);
	}
	else if( ret & PORT_RECONFIGURATION_NEEDED ) {
		vpu_update_sizeinfo(pstVpuPrivate->stVpuInit.m_iBitstreamFormat,
							pstVpuPrivate->stVpuUserInfo.bitrate_mbps,
							pstVpuPrivate->stVpuUserInfo.frame_rate,
							p_init_info->m_iPicWidth,
							p_init_info->m_iPicHeight,
							pstVpuPrivate->pVpuInstance);
	}

#if USE_MAX_VPU_CLOCK
	vpu_update_sizeinfo(STD_AVC,
						1000,
						60,
						1920,1088,
						pstVpuPrivate->pVpuInstance);
#endif
	
	return ret; 
}


static 
OMX_BOOL
CheckOutputModeChange(
	vpudec_private_t   *pstVpuPrivate
	)
{
	char value_vsync[PROPERTY_VALUE_MAX];
	char value_hwrid[PROPERTY_VALUE_MAX];
	char value_mdec[PROPERTY_VALUE_MAX];

	OMX_U32 output_mode = CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT

	property_get("tcc.video.vsync.enable", value_vsync, "0");
	property_get("tcc.video.multidecoding.check", value_mdec, "0");

	if( atoi(value_mdec) ) {
		property_get("tcc.video.hwr.id", value_hwrid, "0");
		if( atoi(value_vsync) && atoi(value_hwrid) == pstVpuPrivate->ulComponentUID )
			SET_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);
		else
		{
			property_get("tcc.solution.mbox", value_vsync, "");
			if( atoi(value_vsync) && atoi(value_hwrid) == pstVpuPrivate->ulComponentUID )
				SET_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);
			else
				CLEAR_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);
		}
	}
	else {
		if( atoi(value_vsync) )
			SET_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);
		else
			CLEAR_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);
	}

#else

	property_get("tcc.sys.output_mode_detected", value_vsync, "");
	if( atoi(value_vsync) )
		SET_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);
	else
		CLEAR_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT);

#endif

	if( output_mode != CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT) )
		return OMX_TRUE;

	return OMX_FALSE;
}

static
OMX_BOOL
IsResolutionChanged(
	vpudec_private_t    *pstVpuPrivate
	)
{
	OMX_S32 dec_width = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iWidth;
	OMX_S32 dec_height = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iHeight;

	if( dec_width > 0 && dec_height > 0 )
	{
		omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
		OMX_S32 port_width = p_outport->sPortParam.format.video.nFrameWidth;
		OMX_S32 port_height = p_outport->sPortParam.format.video.nFrameHeight;

#if AVC_CROP_ENABLE
		if(    pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC
			|| pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MVC )
		{
			pic_crop_t avc_crop = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_CropInfo;
			dec_width  -= avc_crop.m_iCropLeft;
			dec_width  -= avc_crop.m_iCropRight;
			dec_height -= avc_crop.m_iCropBottom;
			dec_height -= avc_crop.m_iCropTop;
		}
#endif
			
		if( port_width != dec_width || port_height != dec_height ) {
			INFO("Resolution change detected (%d x %d --> %d x %d)", 
					port_width, port_height,
					dec_width, dec_height);
		
			return OMX_TRUE;
		}
	}

	return OMX_FALSE;
}


#define COPY_FROM_DECODED_BUFFER	1
#define COPY_FROM_DISPLAY_BUFFER	2
#define COPY_FROM_BLACK_FRAME		3

static 
OMX_U8* 
CopyOutputFrame(
	vpudec_private_t *pstVpuPrivate, 
	OMX_U8           *pDestAddress, 
	OMX_U32          *pulCopyedBytes, 
	OMX_S32           lSrcIndex
	)
{
	OMX_S32 frame_size;
	OMX_S32	y_size, cb_size, cr_size;
	OMX_S32	dst_width, dst_height;
	OMX_S32	y_stride, cbcr_stride;
	OMX_S32 h_off = 0, v_off = 0;
	OMX_U8 *p_dst_y, *p_dst_cb, *p_dst_cr;
	OMX_U8 *p_src_y, *p_src_cb, *p_src_cr;
	OMX_S32 i, j;

	OMX_PTR* p_thumb_buff = NULL;
	pmap_t pmap_thumb;

	if(    pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC 
	    || pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MVC )
	{
		dec_initial_info_t *p_init_info = pstVpuPrivate->stVpuOutput.m_pInitialInfo;
		pic_crop_t avc_crop = p_init_info->m_iAvcPicCrop;

		y_stride  = p_init_info->m_iPicWidth;
		dst_width = p_init_info->m_iPicWidth;
		dst_width -= avc_crop.m_iCropLeft;
		dst_width -= avc_crop.m_iCropRight;

		dst_height = p_init_info->m_iPicHeight;
		dst_height -= avc_crop.m_iCropBottom;
		dst_height -= avc_crop.m_iCropTop;

		h_off = avc_crop.m_iCropLeft;
		h_off -= h_off & 1;
		v_off = avc_crop.m_iCropTop;
		v_off -= v_off & 1;
	}
	else
	{
		dec_initial_info_t *p_init_info = pstVpuPrivate->stVpuOutput.m_pInitialInfo;

		y_stride   = p_init_info->m_iPicWidth;
		dst_width  = p_init_info->m_iPicWidth;
		dst_height = p_init_info->m_iPicHeight;
	}

	dst_width  += dst_width & 1;
	dst_height += dst_height & 1;

	// src size setup
	y_stride    = ((y_stride + 15) >> 4) << 4;
	cbcr_stride = y_stride >> 1;

	// dst size setup
	y_size     = dst_width * dst_height;
	cb_size    = y_size >> 2;
	cr_size    = cb_size;
	frame_size = y_size + cb_size + cr_size;

	if( lSrcIndex != COPY_FROM_BLACK_FRAME )
	{
#if defined(TC_SECURE_MEMORY_COPY)
		if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) ) 
		{
			COPY_MODE  gralloc_copy_mode = COPY_NONE;
			OMX_S32 inst_idx = vdec_get_instance_index(pstVpuPrivate->pVpuInstance);
	
			if( inst_idx <= 1 ) {	// instance idx 0 or 1
				pmap_get_info("video_thumb", &pmap_thumb);
				if( pmap_thumb.size > 0 ) {
					if( ( p_thumb_buff = (void*)mmap(0, pmap_thumb.size, PROT_READ|PROT_WRITE, MAP_SHARED, pstVpuPrivate->hTMemDevice, pmap_thumb.base) ) == MAP_FAILED ) {
						ERROR("Secured thumbnail buffer mmap failed.", TMEM_DEVICE);
						p_thumb_buff = NULL;
					}
				}
			}
	
			if( p_thumb_buff && HwCopyConfig(pstVpuPrivate, OMX_TRUE) == OMX_TRUE )
			{
				#define MAX_FRAME_SIZE 	(3*1024*1024) // MAX: Full HD
				OMX_U32 base_offset = inst_idx * MAX_FRAME_SIZE;
	
				if( lSrcIndex == COPY_FROM_DECODED_BUFFER ) {
					p_src_y  = pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][0];
					p_src_cb = pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][1];
					p_src_cr = pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][2];
				}
				else if( lSrcIndex == COPY_FROM_DISPLAY_BUFFER ) {
					p_src_y  = pstVpuPrivate->stVpuOutput.m_pDispOut[PA][0];
					p_src_cb = pstVpuPrivate->stVpuOutput.m_pDispOut[PA][1];
					p_src_cr = pstVpuPrivate->stVpuOutput.m_pDispOut[PA][2];
				}

				gralloc_copy_mode = HwCopyKickOff(pstVpuPrivate,
												  p_src_y, 
												  p_src_cb,
												  p_src_cr, 
												  pmap_thumb.base + base_offset);
	
				if( gralloc_copy_mode == COPY_START )
					gralloc_copy_mode = HwCopyWait(pstVpuPrivate);
	
				if( gralloc_copy_mode == COPY_DONE ) {
					// src pointer re-setup
					p_src_y  = (OMX_U8*)p_thumb_buff + base_offset;
					p_src_cb = p_src_y + pstVpuPrivate->stGrallocInfo.lFrameSizeY;
					p_src_cr = p_src_cb + pstVpuPrivate->stGrallocInfo.lFrameSizeC;
					y_stride = pstVpuPrivate->stGrallocInfo.lStrideY;
					cbcr_stride = pstVpuPrivate->stGrallocInfo.lStrideC;
				}
			}
	
			if( gralloc_copy_mode != COPY_DONE )
				lSrcIndex = COPY_FROM_BLACK_FRAME;
		}
		else
#endif
		{
			if( lSrcIndex == COPY_FROM_DECODED_BUFFER ) {
				p_src_y  = pstVpuPrivate->stVpuOutput.m_pCurrOut[VA][0];
				p_src_cb = pstVpuPrivate->stVpuOutput.m_pCurrOut[VA][1];
				p_src_cr = pstVpuPrivate->stVpuOutput.m_pCurrOut[VA][2];
			}
			else if( lSrcIndex == COPY_FROM_DISPLAY_BUFFER ) {
				p_src_y  = pstVpuPrivate->stVpuOutput.m_pDispOut[VA][0];
				p_src_cb = pstVpuPrivate->stVpuOutput.m_pDispOut[VA][1];
				p_src_cr = pstVpuPrivate->stVpuOutput.m_pDispOut[VA][2];
			}
		}
	}

	// create buffer
	if( pDestAddress == NULL )
	{
		if( pstVpuPrivate->pThumbnailBuff )
		{
			*pulCopyedBytes = frame_size;
			ERROR("CreateThumbFrame() - already exist");
			return pstVpuPrivate->pThumbnailBuff;
		}
	
		// thumbnail buffer allocation
		pDestAddress = TCC_calloc(1, frame_size);
		if( pDestAddress == 0 ) 
		{
			*pulCopyedBytes = 0;
			ERROR("CreateThumbFrame() - out of memory");
			return 0;
		}

		pstVpuPrivate->pThumbnailBuff = pDestAddress;
	}

	// dst pointer setup
	p_dst_y  = pDestAddress;
	p_dst_cb = p_dst_y + y_size;
	p_dst_cr = p_dst_cb + cb_size;

	if( lSrcIndex == COPY_FROM_BLACK_FRAME )
	{
		memset(p_dst_y, 0x10, y_size);
		memset(p_dst_cb, 0x80, cb_size);
		memset(p_dst_cr, 0x80, cr_size);
	}
	else
	{
		if( pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode )
		{
			OMX_U8 *p_src_cbcr = p_src_cb;
			OMX_U8 *p_temp_cbcr;
			OMX_S32 dst_width_cbcr = dst_width >> 1;
			OMX_S32 dst_height_cbcr = dst_height >> 1;

			// cropping
			p_src_y += h_off + y_stride * v_off;
			p_src_cbcr += ((h_off >> 1) << 1) + (y_stride * (v_off >> 1)); 

			// luminance
			for(i = 0; i < dst_height; i++)
			{
				memcpy(p_dst_y, p_src_y, dst_width);
				p_dst_y += dst_width;
				p_src_y += y_stride;
			}

			// chrominance
			for(i = 0; i < dst_height_cbcr; i++)
			{
				p_temp_cbcr = p_src_cbcr;
				p_src_cbcr += y_stride;
				for(j = 0; j < dst_width_cbcr; j++) {
					*p_dst_cb++ = *p_temp_cbcr++;
					*p_dst_cr++ = *p_temp_cbcr++;
				}
			}
		}
		else
		{
			OMX_S32 dst_width_cbcr = dst_width >> 1;

			//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
			if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MJPG && 
				pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1 )
			{
				// cropping
				p_src_y += h_off + y_stride * v_off;
				h_off >>= 1;
				p_src_cb += h_off + (cbcr_stride * v_off); 
				p_src_cr += h_off + (cbcr_stride * v_off); 

				for(i = 0; i < dst_height; i++)
				{
					// luminance
					memcpy(p_dst_y, p_src_y, dst_width);
					p_dst_y += dst_width;
					p_src_y += y_stride;

					// chrominance (4:2:2 to 4:2:0)
					if( i & 1 ) {
						memcpy(p_dst_cr, p_src_cr, dst_width_cbcr);
						p_dst_cr += dst_width_cbcr;
					}
					else {
						memcpy(p_dst_cb, p_src_cb, dst_width_cbcr);
						p_dst_cb += dst_width_cbcr;
					}
					p_src_cb += cbcr_stride;
					p_src_cr += cbcr_stride;
				}
			}
			else
			{
				OMX_S32 dst_height_cbcr = dst_height >> 1;

				// cropping
				p_src_y += h_off + (y_stride * v_off);
				h_off >>= 1;
				v_off >>= 1;
				p_src_cb += h_off + (cbcr_stride * v_off); 
				p_src_cr += h_off + (cbcr_stride * v_off); 

				// luminance
				for(i = 0; i < dst_height; i++)
				{
					// luminance
					memcpy(p_dst_y, p_src_y, dst_width);
					p_dst_y += dst_width;
					p_src_y += y_stride;
				}

				// chrominance
				for(i = 0; i < dst_height_cbcr; i++) 
				{
					memcpy(p_dst_cb, p_src_cb, dst_width_cbcr);
					p_dst_cb += dst_width_cbcr;
					p_src_cb += cbcr_stride;
					memcpy(p_dst_cr, p_src_cr, dst_width_cbcr);
					p_dst_cr += dst_width_cbcr;
					p_src_cr += cbcr_stride;
				}
			}
		}
	}

#if 0
	{
		FILE *fp;
		OMX_S8 path[256];
		ERROR("Thumbnail frame: %d x %d", dst_width, dst_height);
		sprintf(path, "/sdcard/tflash/thumb_%dx%d.yuv", dst_width, dst_height);
		if( fp = fopen(path, "wb") ) {
			fwrite( pThumbBuff, 1, frame_size, fp);
			fclose(fp);
		}
	}
#endif

	if( p_thumb_buff )
        munmap((void*)p_thumb_buff, pmap_thumb.size);

	*pulCopyedBytes = frame_size;

	return pDestAddress;
}


static
OMX_S32 
ScanFillerSpace_internal(
	OMX_U8      *pRingBase,
	OMX_U8      *pRingEnd,	
	OMX_U8      *pScanStart,
	OMX_U8      *pScanEnd,
	OMX_BOOL     bIsAVC,
	OMX_U8     **ppSpaceEnd
	)
{
#define PRESCAN_MAX		32

	OMX_U8  *p_curr_rp = pScanStart;
	OMX_U32 *p_rp32 = (OMX_U32*)((((OMX_U32)pScanStart + 3) >> 2) << 2);
	OMX_U8  *p_filler_start = 0;
	OMX_U8  *p_filler_end = 0;
	OMX_S32 filler_size = 0;
	OMX_U32 syncword = 0xFFFFFFFF;
	OMX_U32 filler = 0;

	if( pScanStart == pScanEnd )
		return 0;

	if( pScanStart < pScanEnd )
	{
		OMX_U8 *p_temp_end = pScanStart + PRESCAN_MAX;

		if( p_temp_end > pScanEnd )
			p_temp_end = pScanEnd;

		// search zero start to PRESCAN_MAX bytes
		while( p_curr_rp < p_temp_end ) {
			if( *p_curr_rp )
				p_filler_start = NULL;
			else if( p_filler_start == NULL )
				p_filler_start = p_curr_rp;

			if( bIsAVC ) {
				syncword <<= 8;
				syncword |= *p_curr_rp;
				if( (syncword & 0x1F) == 0x0C ) {
					filler = 0xFF;
					p_filler_start = p_curr_rp+1;
					break;
				}
			}
			p_curr_rp++;
		}

		if( p_filler_start )
		{
			p_curr_rp = p_filler_start;
			p_rp32 = (OMX_U32*)((((OMX_U32)p_curr_rp + 3) >> 2) << 2);

			while( p_curr_rp < p_rp32 && p_curr_rp < pScanEnd ) {
				if( *p_curr_rp != filler ) {
					p_filler_end = p_curr_rp;
					break;
				}
				p_curr_rp++;
			}

			if( p_filler_end == NULL && p_curr_rp < pScanEnd )
			{
				if( filler == 0xFF )
					filler = 0xFFFFFFFF;

				while( p_rp32 < pScanEnd-3 ) {
					if( *p_rp32 != filler )
						break;
					p_rp32++;
				}
				
				filler &= 0xFF;

				p_curr_rp = (OMX_U8*)p_rp32;
				while( p_curr_rp < pScanEnd ) {
					if( *p_curr_rp != filler ) {
						p_filler_end = p_curr_rp;
						break;
					}
					p_curr_rp++;
				}
			}
	
			if( p_filler_end == NULL )
				p_filler_end = pScanEnd;

			filler_size = (OMX_S32)(p_filler_end - p_filler_start);

			ASSERT( pScanStart <= p_filler_end && p_filler_end <= pScanEnd );
		}
	}
	else 
	{
		OMX_S32 count;

		count  = (OMX_S32)(pRingEnd - pScanStart);
		count += (OMX_S32)(pScanEnd - pRingBase);

		if( count > PRESCAN_MAX )
			count = PRESCAN_MAX;

		while( count-- ) {
			if( *p_curr_rp )
				p_filler_start = NULL;
			else if( p_filler_start == NULL )
				p_filler_start = p_curr_rp;

			if( bIsAVC ) {
				syncword <<= 8;
				syncword |= *p_curr_rp;
				if( (syncword & 0x1F) == 0x0C ) {
					filler = 0xFF;
					p_filler_start = p_curr_rp+1;
					if( p_filler_start >= pRingEnd )
						p_filler_start = pRingBase;
					break;
				}
			}

			if( ++p_curr_rp >= pRingEnd )
				p_curr_rp = pRingBase;
		}

		if( p_filler_start )
		{
			OMX_U8 *p_scan_end = (OMX_U8*)((((OMX_U32)p_filler_start + 3) >> 2) << 2);

			p_rp32 = (OMX_U32*)p_scan_end;
			p_curr_rp = p_filler_start;

			if( pScanEnd < p_scan_end && p_scan_end < pScanStart )
				p_scan_end = pScanEnd;

			while( p_curr_rp < p_scan_end ) {
				if( *p_curr_rp != filler ) {
					p_filler_end = p_curr_rp;
					break;
				}
				p_curr_rp++;
			}

			if( p_filler_end == NULL && p_curr_rp != pScanEnd )
			{
				if( pScanStart < p_curr_rp )
				{
					p_scan_end = pRingEnd;
	
					if( filler == 0xFF )
						filler = 0xFFFFFFFF;

					// rear
					while( p_rp32 < p_scan_end-3 ) {
						if( *p_rp32 != filler )
							break;
						p_rp32++;
					}
	
					// front
					if( p_rp32 >= p_scan_end )
					{
						p_rp32 = pRingBase;
						p_scan_end = (OMX_U8*)(((OMX_U32)pScanEnd >> 2) << 2);
			
						while( p_rp32 < p_scan_end-3 ) {
							if( *p_rp32 != filler )
								break;
							p_rp32++;
						}
	
						p_scan_end = pScanEnd;
					}
	
					filler &= 0xFF;

					p_curr_rp = (OMX_U8*)p_rp32;
					while( p_curr_rp < p_scan_end ) {
						if( *p_curr_rp != filler ) {
							p_filler_end = p_curr_rp;
							break;
						}
						p_curr_rp++;
					}
				}
				else
				{
					// front
					p_rp32 = p_scan_end;
					p_scan_end = (OMX_U8*)(((OMX_U32)pScanEnd >> 2) << 2);

					if( filler == 0xFF )
						filler = 0xFFFFFFFF;

					while( p_rp32 < p_scan_end-3 ) {
						if( *p_rp32 != filler )
							break;
						p_rp32++;
					}

					filler &= 0xFF;

					p_curr_rp = (OMX_U8*)p_rp32;
					while( p_curr_rp < pScanEnd ) {
						if( *p_curr_rp != filler ) {
							p_filler_end = p_curr_rp;
							break;
						}
						p_curr_rp++;
					}
				}
			}
	
			if( p_filler_end == NULL )
				p_filler_end = pScanEnd;

			if( p_filler_start < p_filler_end ) {
				filler_size  = (OMX_S32)(p_filler_end - p_filler_start);
			} 
			else {
				filler_size  = (OMX_S32)(pRingEnd - p_filler_start);
				filler_size += (OMX_S32)(p_filler_end - pRingBase);
			}

			ASSERT( p_filler_end <= pScanEnd || pScanStart <= p_filler_end );
		}
	}

	if( filler_size && ppSpaceEnd )
		*ppSpaceEnd = p_filler_end;

	return filler_size;
}

static
OMX_S32 
ScanFillerSpace(
	vpudec_private_t   *pstVpuPrivate,
	OMX_U8             *pScanStart,
	OMX_U8             *pScanEnd,
	OMX_U8            **ppSpaceEnd
	)
{
	OMX_S32 size = 0;

#if defined(TC_SECURE_MEMORY_COPY)
	if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) ) 
	{
		ringbuff_state_t *p_rb_state = &pstVpuPrivate->stRingBuffState;
		OMX_S32 size;
		OMX_S32 end_pos;

		size = TC_SecureScanFilterSpace((unsigned int)(p_rb_state->pRingBuffBase[PA]),
									  (int)(p_rb_state->lRingBuffSize),
									  (int)(pScanStart-p_rb_state->pRingBuffBase[VA]),
									  (int)(pScanEnd-p_rb_state->pRingBuffBase[VA]),
									  (int)(pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC),
									  (int *)&end_pos);

		if( size > 0 )
			*ppSpaceEnd = p_rb_state->pRingBuffBase[VA] + end_pos;
	}
	else 
#endif
	{
		size = ScanFillerSpace_internal(pstVpuPrivate->stRingBuffState.pRingBuffBase[VA], 
										pstVpuPrivate->stRingBuffState.pRingBuffEnd[VA],
										pScanStart,
										pScanEnd,
										pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_AVC,
										ppSpaceEnd);
	}

#if 0
	{
		#define DEBUG_BINARY(__p_start, __p_end)\
		{\
			OMX_S32 idx0 = 0, idx1 = 0;\
			OMX_S32 count;\
			OMX_U8 start_bytes[8] = {0,0,0,0,0,0,0,0};\ 
			OMX_U8 end_bytes[8] = {0,0,0,0,0,0,0,0};\
			OMX_U8 *p_bytes;\
			\
			if( !CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) ) \
			{\
				if( __p_start < __p_end )\
				{\
					idx0 = 0;\
					count = 8;\
					p_bytes = __p_start;\
					while( idx0 < count && p_bytes < __p_end )\
						start_bytes[idx0++] = *p_bytes++;\
					\
					if( idx0 >= count ) {\
						idx1 = 0;\
						p_bytes = __p_end-8;\
						while( idx1 < count && p_bytes < __p_end )\
							end_bytes[idx1++] = *p_bytes++;\
					}\
				}\
				else\
				{\
					idx0 = 0;\
					count = 8;\
					p_bytes = __p_start;\
					while( idx0 < count && p_bytes < pRingEnd )\
						start_bytes[idx0++] = *p_bytes++;\
					\
					if( idx0 < count ) {\
						p_bytes = pRingBase;\
						while( idx0 < count && p_bytes < __p_end )\
							start_bytes[idx0++] = *p_bytes++;\
					\
						idx1 = 0;\
						p_bytes = __p_end-8;\
						if( p_bytes < pRingBase ) {\
							p_bytes = pRingBase;\
							count = (OMX_S32)(__p_end - pRingBase);\
							while( idx1 < count )\
								end_bytes[idx1++] = *p_bytes++;\
						}\
						else  {\
							while( idx1 < count && p_bytes < __p_end )\
								end_bytes[idx1++] = *p_bytes++;\
						}\
					}\
					else {\
						idx1 = 0;\
						p_bytes = __p_end-8;\
						if( p_bytes < pRingBase )\
							p_bytes = pRingEnd - (pRingBase - p_bytes);\
						\
						while( idx1 < count && p_bytes < pRingEnd )\
							end_bytes[idx1++] = *p_bytes++;\
						\
						if( idx1 < count ) {\
							p_bytes = pRingBase;\
							while( idx1 < count && p_bytes < __p_end )\
								end_bytes[idx1++] = *p_bytes++;\
						}\
					}\
				}\
				\
				CLOG(B, "[MGE][FILLER] %02X %02X %02X %02X %02X %02X %02X %02X (%2d) ~ %02X %02X %02X %02X %02X %02X %02X %02X (%2d)"\
					 , start_bytes[0] ,start_bytes[1] ,start_bytes[2] ,start_bytes[3]\
					 , start_bytes[4], start_bytes[5], start_bytes[6], start_bytes[7]\
					 , idx0\
					 , end_bytes[0], end_bytes[1], end_bytes[2], end_bytes[3]\
					 , end_bytes[4], end_bytes[5], end_bytes[6], end_bytes[7]\
					 , idx1\
					 );\
			}\
			else {\
				CLOG(B, "[MGE][FILLER] XX XX XX XX XX XX XX XX (--) ~ XX XX XX XX XX XX XX XX (--)");\
			}\
		}

		OMX_S32 rgn_size;
		OMX_U8 *pRingBase = pstVpuPrivate->stRingBuffState.pRingBuffBase[VA];
		OMX_U8 *pRingEnd = pstVpuPrivate->stRingBuffState.pRingBuffEnd[VA];
		OMX_U8 *p_filler_start = *ppSpaceEnd - size;
		OMX_U8 *p_filler_end = *ppSpaceEnd;
		OMX_S32 filler_size = size;

		if( p_filler_start < pRingBase )
			p_filler_start = pRingEnd - (pRingBase - p_filler_start);

		if( pScanStart < pScanEnd )
			rgn_size = (OMX_S32)(pScanEnd - pScanStart);
		else {
			rgn_size  = (OMX_S32)(pRingEnd - pScanStart);
			rgn_size += (OMX_S32)(pScanEnd - pRingBase);
		}

		if( filler_size )
		{
			CLOG(A, "[MGE][FILLER] [SCANRGN: %8d ~ %8d (LEN: %8d)] [PADRGN: %8d ~ %8d (LEN: %8d)] %s"
				  , pScanStart - pRingBase
				  , pScanEnd - pRingBase
				  , rgn_size
				  , filler_size ? p_filler_start - pRingBase : 0
				  , filler_size ? p_filler_end - pRingBase : 0
				  , filler_size ? filler_size : 0
				  , CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) ? "- secured" : ""
				  );

			if( filler_size ) {
				DEBUG_BINARY(p_filler_start, p_filler_end);
			} else {
				DEBUG_BINARY(pScanStart, pRingEnd);
			}
		}
	}
#endif

	return size;
}


static 
OMX_TICKS
GetCurrTimestamp(
	vpudec_private_t   *pstVpuPrivate,
	OMX_TICKS          *pllTimestamp,
	OMX_BOOL            bDecodeSuccess,
	OMX_BOOL            bInterlaced
	)
{
	if( UpdateRingBuffer(&pstVpuPrivate->stRingBuffState, OMX_TRUE) < 0 )
		return -1;

#if SINGLE_FRAME_INPUT_ENABLE
	if( pstVpuPrivate->bSingleFrameInput )
	{
		input_info_t   info;

		if( GetInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
			return -1;
		*pllTimestamp = info.llTimestamp;

		if( ShowInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
			info.llTimestamp = -1;
		pstVpuPrivate->llQueuedStartTime = info.llTimestamp;

		LOG_IIMGE("[RESULT] --------- [CURRENT_BASE_PTS: %8d / NEXT_BASE_PTS: %8d] ---------"
				  , (OMX_S32)(*pllTimestamp/1000)
				  , (OMX_S32)(pstVpuPrivate->llQueuedStartTime/1000)
				  );
	}
	else
#endif
	{
		input_info_t   info;
		OMX_U8        *p_curr_rp = pstVpuPrivate->stRingBuffState.pReadPtr;
		OMX_U8        *p_prev_rp = pstVpuPrivate->stRingBuffState.pPrevReadPtr;
		OMX_TICKS      candidate_pts[4];
		OMX_S32        candidate_byte[4];
		OMX_S32        candidate_count = 0;
		OMX_S32        index = 0;
		OMX_TICKS      result_pts = -1;
		OMX_U8        *p_org_rp = p_curr_rp;	// for debug
		
		if( ShowInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
			return -1;

		if( CHECK_FLAG(pstVpuPrivate, STATE_RESOLUTION_CHANGING) ) {
			*pllTimestamp = result_pts;
			return 0;
		}
	
		if( p_curr_rp == pstVpuPrivate->stRingBuffState.pRingBuffEnd[VA] )
			p_curr_rp = 0;
	
		LOG_IIMGE("[SEARCH] [PTS: %8d / REGION: %7d ~ %7d (%5d Byte)] - [PRP: %7d ~ RP: %7d]"
				  , (OMX_S32)(info.llTimestamp/1000)
				  , info.pStartPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  , info.pEndPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  , info.lFilledLen
				  , p_prev_rp - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  , p_curr_rp - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  );

		if( info.pStartPtr < info.pEndPtr ) {
			if( info.pStartPtr <= p_curr_rp && p_curr_rp <= info.pEndPtr ) {
				if( bDecodeSuccess && ScanFillerSpace(pstVpuPrivate, p_curr_rp, info.pEndPtr, &p_curr_rp) > 0 )
					pstVpuPrivate->stRingBuffState.pReadPtr = p_curr_rp;
				result_pts = info.llTimestamp;
			}
		}
		else {
			if( p_curr_rp <= info.pEndPtr || info.pStartPtr <= p_curr_rp ) {
				if( bDecodeSuccess && ScanFillerSpace(pstVpuPrivate, p_curr_rp, info.pEndPtr, &p_curr_rp) > 0 )
					pstVpuPrivate->stRingBuffState.pReadPtr = p_curr_rp;
				result_pts = info.llTimestamp;
			}
		}

		if( result_pts < 0 ) 
		{
			if( info.pStartPtr < info.pEndPtr ) {
				candidate_byte[index] = (OMX_S32)(info.pEndPtr - p_prev_rp);
			}
			else {
				if( p_prev_rp < info.pEndPtr )
					candidate_byte[index]  = (OMX_S32)(info.pEndPtr - p_prev_rp);
				else {
					candidate_byte[index]  = (OMX_S32)(pstVpuPrivate->stRingBuffState.pRingBuffEnd[VA] - p_prev_rp);
					candidate_byte[index] += (OMX_S32)(info.pEndPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]);
				}
			}
			candidate_pts[index] = info.llTimestamp;
			candidate_count++;

			while( 1 ) 
			{
				ClearFirstInputInfo(&pstVpuPrivate->stInputInfoQueue);
				if( ShowInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 ) {
					info.pStartPtr   = 0;
					info.pEndPtr     = 0;
					info.llTimestamp = -1;	// queued input is not exist.
					break;
				}

				LOG_IIMGE("[SEARCH] [PTS: %8d / REGION: %7d ~ %7d (%5d Byte)] - [PRP: %7d ~ RP: %7d]"
						  , (OMX_S32)(info.llTimestamp/1000)
						  , info.pStartPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
						  , info.pEndPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
						  , info.lFilledLen
						  , p_prev_rp - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
						  , p_curr_rp - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
						  );

				if( ++index >= 4 ) 
					index = 0;
				candidate_pts[index] = info.llTimestamp;
				candidate_byte[index] = info.lFilledLen;
				candidate_count++;

				if( info.pStartPtr < info.pEndPtr ) {
					if( info.pStartPtr <= p_curr_rp && p_curr_rp <= info.pEndPtr ) {
						if( bDecodeSuccess && ScanFillerSpace(pstVpuPrivate, p_curr_rp, info.pEndPtr, &p_curr_rp) > 0 )
							pstVpuPrivate->stRingBuffState.pReadPtr = p_curr_rp;
						candidate_byte[index]  = (OMX_S32)(p_curr_rp - info.pStartPtr);
						break;
					}
				}
				else {
					if( p_curr_rp <= info.pEndPtr || info.pStartPtr <= p_curr_rp ) {
						if( bDecodeSuccess && ScanFillerSpace(pstVpuPrivate, p_curr_rp, info.pEndPtr, &p_curr_rp) > 0 )
							pstVpuPrivate->stRingBuffState.pReadPtr = p_curr_rp;

						if( p_curr_rp <= info.pEndPtr ) {
							candidate_byte[index]  = (OMX_S32)(pstVpuPrivate->stRingBuffState.pRingBuffEnd[VA] - info.pStartPtr);
							candidate_byte[index] += (OMX_S32)(p_curr_rp - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]);
						} else {
							candidate_byte[index]  = (OMX_S32)(p_curr_rp - info.pStartPtr);
						}
						break;
					}
				}
			}
		}

		LOG_IIMGE("[SEARCH][CANDIDATE: %d] [%ll6d (%6d Byte)] [%ll6d (%6d Byte)] [%ll6d (%6d Byte)] [%ll6d (%6d Byte)]"
				  , candidate_count
				  , candidate_count > 0 ? candidate_pts[0]/1000 : -1
				  , candidate_count > 0 ? candidate_byte[0] : -1
				  , candidate_count > 1 ? candidate_pts[1]/1000 : -1
				  , candidate_count > 1 ? candidate_byte[1] : -1
				  , candidate_count > 2 ? candidate_pts[2]/1000 : -1
				  , candidate_count > 2 ? candidate_byte[2] : -1
				  , candidate_count > 3 ? candidate_pts[3]/1000 : -1
				  , candidate_count > 3 ? candidate_byte[3] : -1
				  );

		if( bDecodeSuccess )
		{
			if( bInterlaced )
			{
				if( candidate_count == 2 ) {	// candidate count == 2
					result_pts = candidate_pts[1];
				}
				else if( candidate_count == 3 ) {
					if( candidate_byte[0] < candidate_byte[2] )
						result_pts = candidate_pts[2];
					else
						result_pts = candidate_pts[1];
				}
				else if( candidate_count == 4 ) {
					result_pts = candidate_pts[2];
				}
				else if( candidate_count > 4 ) {
					if( ((candidate_byte[index] * 100) / info.lFilledLen) < 1 ) // < 1 percent
						result_pts = candidate_pts[index-1 < 0 ? 3 : index-1];
					else
						result_pts = candidate_pts[index];
				}
			}
			else
			{
				if( candidate_count == 2 ) {	// candidate count == 2
					if( candidate_byte[0] <= candidate_byte[1] )
						result_pts = candidate_pts[1];
					else
						result_pts = candidate_pts[0];
				}
				else if( candidate_count == 3 ) {
					result_pts = candidate_pts[1];	// second (mid) pts
				}
				else if( candidate_count >= 4 ) {
					if( (candidate_byte[index] * 100) / info.lFilledLen < 1 ) // < 1 percent
						result_pts = candidate_pts[index-1 < 0 ? 3 : index-1];
					else
						result_pts = candidate_pts[index];
				}
					
			}

			*pllTimestamp = result_pts;
		}
		else
		{
			*pllTimestamp = info.llTimestamp;
		}

		if( p_curr_rp == info.pEndPtr ) {
			ClearFirstInputInfo(&pstVpuPrivate->stInputInfoQueue);
			ShowInputInfo(&pstVpuPrivate->stInputInfoQueue, &info);
		}

		if( pstVpuPrivate->llQueuedStartTime != info.llTimestamp )
			pstVpuPrivate->llQueuedStartTime = info.llTimestamp;

		LOG_IIMGE("[RESULT] --------- [INPUT_RP: %8d / BASE_PTS: %8d] ---------"
				  , p_org_rp - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  , (OMX_S32)(*pllTimestamp/1000)
				  );
	}

	return 0;
}


static
OMX_S32
GetOutputBufferInfo(
	vpudec_private_t       *pstVpuPrivate,
	OMX_S32                 lBufferIndex,
	output_info_t          *pstOutputInfo
	)
{
	disp_info_t *p_info;
	OMX_TICKS timestamp;
	OMX_S32 ret;

	if( ShowOutputDispInfo(&pstVpuPrivate->stDispInfoMgr, lBufferIndex, &p_info, &timestamp) == OMX_FALSE )
	{
#if RESOLUTION_SWITCHING_SUPPORT
		if( CHECK_FLAG(pstVpuPrivate, STATE_RESOLUTION_CHANGING) ) {
			SET_FLAG(pstVpuPrivate, STATE_READY_TO_RESET);
			return ERROR_INVALID_BUFFER_STATE;
		}
#endif

		if( ShowNonQueuedDispInfo(&pstVpuPrivate->stDispInfoMgr, lBufferIndex, &p_info, &timestamp) == OMX_FALSE )
			return -1;
	}

	if( pstOutputInfo ) {
		pstOutputInfo->lFrameType      = p_info->lFrameType;
		pstOutputInfo->lPicStructure   = p_info->lPicStructure;
		pstOutputInfo->llTimestamp     = timestamp;
		pstOutputInfo->ulBufferFlags   = p_info->ulBufferFlags;
		pstOutputInfo->bIsMvcDependent = p_info->bIsMvcDependent;
	}

	LOG_OUT("[TYPE: %s (%2d/%d)][PTS: %8d (diff: %4d)] [State: %2d/%2d][BuffIdx: %2d(%c)/%2d(%c)] [FieldSeq: %d][TR: %8d] "
			, GetFrameTypeString( pstVpuPrivate->stVpuInit.m_iBitstreamFormat
								  , p_info->lFrameType
								  , p_info->lPicStructure)
			, p_info->lFrameType
			, p_info->lPicStructure
			, (OMX_S32)(timestamp/1000)
			, (OMX_S32)((timestamp - pstVpuPrivate->stDispInfoMgr.llLastOutTimestamp)/1000)
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodingStatus
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iOutputStatus
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodedIdx
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodedIdx < 0 ? '-' : (pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded ? 'D' : 'B')
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDispOutIdx
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDispOutIdx < 0 ? '-' : (pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDisplay ? 'D' : 'B')
			, p_info->lM2vFieldSequence
			, p_info->lRvTimestamp
			);

	if( p_info->bIsValid == OMX_FALSE )
		ClearNonQueuedDispInfo(&pstVpuPrivate->stDispInfoMgr, lBufferIndex);
	else
		ClearDispInfo(&pstVpuPrivate->stDispInfoMgr, lBufferIndex);

	return 0;
}


static 
OMX_S32
ClearDisplayedBufferByBufferID(
	vpudec_private_t    *pstVpuPrivate,
	OMX_U32              ulDispIdx,
	OMX_U32              ulBuffId
	)
{
	dispidx_queue_t *p_dispidx_queue = &pstVpuPrivate->stDispIdxQueue;
	OMX_S32 ret = 0;
	OMX_S32 clear_count = 0;
	OMX_S32 check_count = 0;
	OMX_S32 buffer_id;
	OMX_S32 queue_count = 0;

	if(CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT))
		buffer_id = ioctl(pstVpuPrivate->hFBDevice, TCC_LCDC_VIDEO_GET_DISPLAYED, NULL) ;  // TCC_LCDC_HDMI_GET_DISPLAYED
	else if(!CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE))
		buffer_id = ioctl(pstVpuPrivate->hFBDevice, TCC_VIDEO_GET_DISPLAYED_BUFFER_ID, vdec_get_instance_index(pstVpuPrivate->pVpuInstance));
	else
		buffer_id = -1;

	if( buffer_id < 0) {
		if( CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT) && buffer_id == -2 )
			return 0; //ZzaU:: in case of seek status, return

		LOG_BUFCLR("[DISP_IDX: %2d] [VSYNC_BUFF_ID: %d]", ulDispIdx, ulBuffId);
		if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &ulDispIdx, NULL, pstVpuPrivate->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR]  [DISP_IDX: %d / RET_CODE: %d]", ulDispIdx, ret);
			return ret;
		}
		ClearFirstDispIdx(p_dispidx_queue);
		return 0;
	}

	while(buffer_id < ulBuffId && check_count < VSYNC_DISP_CHECK_COUNT_MAX)
	{
		usleep(10000);
		if(CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT))
			buffer_id = ioctl(pstVpuPrivate->hFBDevice, TCC_LCDC_VIDEO_GET_DISPLAYED, NULL) ;  // TCC_LCDC_HDMI_GET_DISPLAYED
		else
			buffer_id = ioctl(pstVpuPrivate->hFBDevice, TCC_VIDEO_GET_DISPLAYED_BUFFER_ID, vdec_get_instance_index(pstVpuPrivate->pVpuInstance));
		check_count++;
		if(buffer_id < 0)
			break;
	}

	while( queue_count >= 0 && buffer_id >= ulBuffId )
	{
		LOG_BUFCLR("[DISP_IDX: %2d] [VSYNC_BUFF_ID: %d]", ulDispIdx, ulBuffId);
		if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &ulDispIdx, NULL, pstVpuPrivate->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", ulDispIdx, ret);
			return ret;
		}

		ClearFirstDispIdx(p_dispidx_queue);
		queue_count = ShowDispIdx(p_dispidx_queue, &ulDispIdx, &ulBuffId);
		clear_count++;
		break; // ZzaU :: There is no need to clear all buffers eventhough there are too many buffers having smaller id than coressponding one. clear only one.
	}

	if( clear_count == 0 )
	{
		LOG_BUFCLR("[DISP_IDX: %2d] [VSYNC_BUFF_ID: %d]", ulDispIdx, ulBuffId);
		ioctl(pstVpuPrivate->hFBDevice, TCC_LCDC_VIDEO_CLEAR_FRAME, ulBuffId);
		ERROR("Video Buffer Clear Sync Fail : %d %d , loopcount(%d)\n", buffer_id, ulBuffId, check_count);

		if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &ulDispIdx, NULL, pstVpuPrivate->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", ulDispIdx, ret);
			return ret;
		}
		ClearFirstDispIdx(p_dispidx_queue);
	}

	return 0;
}

static
OMX_S32
ClearAllDisplayBuffers(
	vpudec_private_t       *pstVpuPrivate,
	OMX_BOOL                bIsModeChanged,
	OMX_BOOL				bSeek
	)
{
	OMX_S32 disp_idx;
	OMX_S32 vsync_buff_id;
	OMX_S32 ret;

#ifdef VPU_BUFFER_MANAGEMENT

	OMX_BOOL list_clear = OMX_FALSE;
	if(!CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT) && CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE))
		list_clear = OMX_TRUE;

	if( bIsModeChanged )
		list_clear = !list_clear;

	if( list_clear )
	{
		INFO("Non-VSync Buffer Clear");

		ioctl(pstVpuPrivate->hFBDevice, TCC_LCDC_VIDEO_CLEAR_FRAME, pstVpuPrivate->nVSyncBuffId);

		ClearAllDispBuffFromList(&pstVpuPrivate->stDispBuffList);
		ResetDispBuffList(&pstVpuPrivate->stDispBuffList);
	}
	else
#endif
	{
		INFO("VSync Buffer Clear");

		ioctl(pstVpuPrivate->hFBDevice, TCC_LCDC_VIDEO_CLEAR_FRAME, pstVpuPrivate->nVSyncBuffId);
		if(bSeek == OMX_TRUE)
			ioctl(pstVpuPrivate->hFBDevice, TCC_LCDC_VIDEO_SWAP_VPU_FRAME, pstVpuPrivate->nVSyncBuffId);

		while( GetDispIdx(&pstVpuPrivate->stDispIdxQueue, &disp_idx, &vsync_buff_id) >= 0 )
		{
			LOG_BUFCLR("[DISP_IDX: %2d] [VSYNC_BUFF_ID: %d]", disp_idx, vsync_buff_id);
			if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &disp_idx, NULL, pstVpuPrivate->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", disp_idx, ret);
				return ret;
			}
		}
	}

	return 0;
}


static 
OMX_S32 
DisplayBufferProcess(
	vpudec_private_t *pstVpuPrivate, 
	OMX_U32 ulVpuDispIdx, 
	OMX_BOOL bDependent,
	OMX_U32 ulVSyncBuffId,
	OMX_BUFFERHEADERTYPE *pOutputBuffer
	)
{
	OMX_S32 ret;
	OMX_S32 disp_idx;
	OMX_S32 vsync_buff_id;
	OMX_S32 stored_count;

	if(!CHECK_FLAG(pstVpuPrivate, STATE_VSYNC_OUTPUT) && CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE)) //non-vsync
	{
#ifdef VPU_BUFFER_MANAGEMENT

		ClearDispedBuffFromList(&pstVpuPrivate->stDispBuffList, pOutputBuffer->pBuffer);
		PushDispBuffToList(&pstVpuPrivate->stDispBuffList, ulVpuDispIdx, bDependent, pOutputBuffer->pBuffer);

#else

		if( GetDispIdx(&pstVpuPrivate->stDispIdxQueue, &disp_idx, &vsync_buff_id) >= 0 )
		{
			LOG_BUFCLR("[DISP_IDX: %2d] [VSYNC_BUFF_ID: %d]", disp_idx, vsync_buff_id);
			if( (ret = vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &disp_idx, NULL, pstVpuPrivate->pVpuInstance)) < 0 ) {
				ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", disp_idx, ret);
				return ret;
			}
		}
		PushDispIdx(&pstVpuPrivate->stDispIdxQueue, ulVpuDispIdx, ulVSyncBuffId);

#endif
	}
	else
	{
		PushDispIdx(&pstVpuPrivate->stDispIdxQueue, ulVpuDispIdx, ulVSyncBuffId);
		 
		/* Clear displayed buffer */
		if( GetDispIdxCount(&pstVpuPrivate->stDispIdxQueue) == pstVpuPrivate->lRefBuffCount ) //buffer full
		{
			if( ShowDispIdx(&pstVpuPrivate->stDispIdxQueue, &disp_idx, &vsync_buff_id) >= 0 )
			{
				if( (ret = ClearDisplayedBufferByBufferID(pstVpuPrivate, disp_idx, vsync_buff_id)) < 0 ) {
					ERROR( "[VPU_ERROR] [OP: VDEC_BUF_FLAG_CLEAR] [DISP_IDX: %d / RET_CODE: %d]", disp_idx, ret);
					return ret;
				}
			}
		}
	}

	return 0;
}


static 
OMX_BOOL
BackupRingBuffer(
	vpudec_private_t        *pstVpuPrivate
	)
{
	ringbuff_state_t *p_rb_state = &pstVpuPrivate->stRingBuffState;
	OMX_S32 start_pos = (OMX_S32)(p_rb_state->pReadPtr - p_rb_state->pRingBuffBase[VA]);
	OMX_S32 length = 0;

	INFO("Backup Ring-Buffer");

#if defined(TC_SECURE_MEMORY_COPY)
	if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) )
	{
		if( pstVpuPrivate->lSeqBuffMapSize <= 0 ) {
			ERROR("BackupRingBuffer() - invalid map size of the backup buffer");
			return OMX_FALSE;
		}

		if( p_rb_state->pReadPtr < p_rb_state->pWritePtr )
		{
			length = (OMX_S32)(p_rb_state->pWritePtr - p_rb_state->pReadPtr);
	
			TC_SecureMemoryCopy((unsigned int)(pstVpuPrivate->pSeqBuffBase[PA]),
								(unsigned int)(p_rb_state->pRingBuffBase[PA] + (p_rb_state->pReadPtr - p_rb_state->pRingBuffBase[VA])),
								(unsigned int)length);
		}
		else
		{
			OMX_S32 len0 = p_rb_state->pRingBuffEnd[VA] - p_rb_state->pReadPtr;
			OMX_S32 len1 = p_rb_state->pWritePtr - p_rb_state->pRingBuffBase[VA];
	
			length = len0 + len1;
	
			TC_SecureMemoryCopy((unsigned int)(pstVpuPrivate->pSeqBuffBase[PA]),
								(unsigned int)(p_rb_state->pRingBuffBase[PA] + (p_rb_state->pReadPtr - p_rb_state->pRingBuffBase[VA])),
								(unsigned int)len0);
	
			TC_SecureMemoryCopy((unsigned int)(pstVpuPrivate->pSeqBuffBase[PA] + len0),
								(unsigned int)(p_rb_state->pRingBuffBase[PA]),
								(unsigned int)len1);
		}
	}
	else 
#endif
	{
		if( p_rb_state->pBackupBuffer == NULL ) {
			p_rb_state->pBackupBuffer = TCC_calloc(1, p_rb_state->lRingBuffSize);
			if( p_rb_state->pBackupBuffer == NULL ) {
				ERROR("BackupAndUpdateInputState() - out of memory");
				return OMX_FALSE;
			}
		}
	
		if( p_rb_state->pReadPtr < p_rb_state->pWritePtr )
		{
			length = (OMX_S32)(p_rb_state->pWritePtr - p_rb_state->pReadPtr);
	
			memcpy(p_rb_state->pBackupBuffer,
				   p_rb_state->pReadPtr, 
				   length);
		}
		else
		{
			OMX_S32 len0 = p_rb_state->pRingBuffEnd[VA] - p_rb_state->pReadPtr;
			OMX_S32 len1 = p_rb_state->pWritePtr - p_rb_state->pRingBuffBase[VA];
	
			length = len0 + len1;
	
			memcpy(p_rb_state->pBackupBuffer, 
				   p_rb_state->pReadPtr, 
				   len0);
	
			memcpy(p_rb_state->pBackupBuffer + len0, 
				   p_rb_state->pRingBuffBase[VA], 
				   len1);
		}
	}

	p_rb_state->pPrevRingBuffBase = p_rb_state->pRingBuffBase[VA];
	p_rb_state->pPrevRingBuffEnd = p_rb_state->pRingBuffEnd[VA];
	p_rb_state->lBackupStartOffset = start_pos;
	p_rb_state->lBackupLength = length;

	return OMX_TRUE;
}


static 
OMX_BOOL
RestoreRingBuffer(
	vpudec_private_t        *pstVpuPrivate
	)
{
	input_info_t   info;
	OMX_S32 offset;
	OMX_S32 count;
	OMX_S32 length;

	ringbuff_state_t *p_rb_state = &pstVpuPrivate->stRingBuffState;
	OMX_U8 *p_start;
	OMX_U8 *p_curr_base = p_rb_state->pRingBuffBase[VA];

	INFO("Restore Ring-Buffer");

	ResetRingBuffer(p_rb_state);
	offset = 0;

	if( (count = GetInputInfo(&pstVpuPrivate->stInputInfoQueue, &info)) < 0 )
		return OMX_FALSE;

	LOG_IIMGE("[GET   ] [PTS: %8d / REGION: %7d ~ %7d / LEN: %6d]"
			  , (OMX_S32)(info.llTimestamp/1000)
			  , info.pStartPtr - p_rb_state->pPrevRingBuffBase
			  , info.pEndPtr - p_rb_state->pPrevRingBuffBase
			  , info.lFilledLen
			  );

	info.pStartPtr = p_rb_state->pPrevRingBuffBase + p_rb_state->lBackupStartOffset;

	if( info.pStartPtr < info.pEndPtr )
		info.lFilledLen = (OMX_S32)(info.pEndPtr-info.pStartPtr);
	else {
		info.lFilledLen  = (OMX_S32)(p_rb_state->pPrevRingBuffEnd-info.pStartPtr);
		info.lFilledLen += (OMX_S32)(info.pEndPtr-p_rb_state->pPrevRingBuffBase);
	}

	LOG_IIMGE("[GET   ] [PTS: %8d / REGION: %7d ~ %7d / LEN: %6d]"
			  , (OMX_S32)(info.llTimestamp/1000)
			  , info.pStartPtr - p_rb_state->pPrevRingBuffBase
			  , info.pEndPtr - p_rb_state->pPrevRingBuffBase
			  , info.lFilledLen
			  );

	info.pStartPtr = p_curr_base + offset;
	info.pEndPtr   = info.pStartPtr + info.lFilledLen;
	offset += info.lFilledLen;

	LOG_IIMGE("[PUSH  ] [PTS: %8d / REGION: %7d ~ %7d / LEN: %6d]"
			  , (OMX_S32)(info.llTimestamp/1000)
			  , info.pStartPtr - p_curr_base
			  , info.pEndPtr - p_curr_base
			  , info.lFilledLen
			  );

	if( PushInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
		return OMX_FALSE;

	p_rb_state->pPrevWritePtr = p_rb_state->pWritePtr;
	p_rb_state->pWritePtr     += info.lFilledLen;
	p_rb_state->lWrittenBytes += info.lFilledLen;
	p_rb_state->lEmptySpace   -= info.lFilledLen;

	pstVpuPrivate->llQueuedStartTime = info.llTimestamp;

	while( count-- )
	{
		if( GetInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
			break;

		LOG_IIMGE("[GET   ] [PTS: %8d / REGION: %7d ~ %7d / LEN: %6d]"
				  , (OMX_S32)(info.llTimestamp/1000)
				  , info.pStartPtr - p_rb_state->pPrevRingBuffBase
				  , info.pEndPtr - p_rb_state->pPrevRingBuffBase
				  , info.lFilledLen
				  );

		info.pStartPtr = p_curr_base + offset;
		info.pEndPtr   = info.pStartPtr + info.lFilledLen;
		offset += info.lFilledLen;

		LOG_IIMGE("[PUSH  ] [PTS: %8d / REGION: %7d ~ %7d / LEN: %6d]"
				  , (OMX_S32)(info.llTimestamp/1000)
				  , info.pStartPtr - p_curr_base
				  , info.pEndPtr - p_curr_base
				  , info.lFilledLen
				  );

		if( PushInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
			return OMX_FALSE;

		p_rb_state->pPrevWritePtr = p_rb_state->pWritePtr;
		p_rb_state->pWritePtr     += info.lFilledLen;
		p_rb_state->lWrittenBytes += info.lFilledLen;
		p_rb_state->lEmptySpace   -= info.lFilledLen;
	}

#if defined(TC_SECURE_MEMORY_COPY)
	if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) )
	{
		TC_SecureMemoryCopy((unsigned int)(p_rb_state->pRingBuffBase[PA]),
							(unsigned int)(pstVpuPrivate->pSeqBuffBase[PA]),
							(unsigned int)(p_rb_state->lBackupLength));
	}
	else
#endif
	{
		memcpy(p_curr_base, p_rb_state->pBackupBuffer, p_rb_state->lBackupLength);
	}

	if( UpdateRingBuffer(p_rb_state, OMX_FALSE) < 0 )
		return OMX_FALSE;

	p_rb_state->lBackupStartOffset = -1;

	return OMX_TRUE;
}


static 
OMX_S32
FeedVpuDecoder(
	vpudec_private_t        *pstVpuPrivate,
	OMX_BUFFERHEADERTYPE	*pInputBuffer
	)
{
#define NEED_MORE_DATA	1
#define FEED_COMPLETE	0
#define FEED_FAILED		-1

	/* sequence header first */
	if( pInputBuffer == NULL )
	{
		if( pstVpuPrivate->lSeqHeaderLength > 0 ) 
		{
#if defined(TC_SECURE_MEMORY_COPY)
			if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) ) 
			{
				if( FillRingBuffer_secure(&pstVpuPrivate->stRingBuffState, 
										  pstVpuPrivate->pSeqBuffBase[PA],
										  pstVpuPrivate->lSeqHeaderLength) == OMX_FALSE )
				{
					return FEED_FAILED;
				}
			}
			else 
#endif
			{
				if( FillRingBuffer(&pstVpuPrivate->stRingBuffState, 
								   pstVpuPrivate->pSequenceHeader,
								   pstVpuPrivate->lSeqHeaderLength) == OMX_FALSE )
				{
					return FEED_FAILED;
				}
			}
		}

		if( UpdateRingBuffer(&pstVpuPrivate->stRingBuffState, OMX_FALSE) < 0 )
			return FEED_FAILED;

		return NEED_MORE_DATA;
	}

	if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_INITIALIZED) ) {
		if( CHECK_FLAG(pstVpuPrivate, STATE_INSUFFICIENT_BITSTREAM) )
			CLEAR_FLAG(pstVpuPrivate, STATE_INSUFFICIENT_BITSTREAM);
		else {
			if( pstVpuPrivate->stInputInfoQueue.lQueCount > 1 && 
				(pstVpuPrivate->stRingBuffState.lRingBuffSize - pstVpuPrivate->stRingBuffState.lEmptySpace) > WRITE_PTR_ALIGN_BYTES &&
				(pInputBuffer->nTimeStamp - pstVpuPrivate->llQueuedStartTime) > pstVpuPrivate->llFeedMaxTimeDiff
				)
			{
				LOG_FEED("[INPUT_CNT: %7d] [QUEUED TIME: %8d ~ %8d ms] [DIFF:%10lld us] [LIMIT:%10lld us] - [OVER LIMIT]"
						 , pstVpuPrivate->lInputCount
						 , (OMX_S32)(pstVpuPrivate->llQueuedStartTime/1000)
						 , (OMX_S32)(pInputBuffer->nTimeStamp/1000)
						 , pInputBuffer->nTimeStamp - pstVpuPrivate->llQueuedStartTime
						 , pstVpuPrivate->llFeedMinTimeDiff
						 );

				if( UpdateRingBuffer(&pstVpuPrivate->stRingBuffState, OMX_FALSE) < 0 )
					return FEED_FAILED;
		
				return FEED_COMPLETE;
			}
		}
	}

	if( FillRingBuffer(&pstVpuPrivate->stRingBuffState, 
					   pInputBuffer->pBuffer + pInputBuffer->nOffset,
					   pInputBuffer->nFilledLen) == OMX_TRUE )
	{
		input_info_t info;

		info.llTimestamp = pInputBuffer->nTimeStamp;  
		info.lFilledLen  = pInputBuffer->nFilledLen;
		info.pStartPtr   = pstVpuPrivate->stRingBuffState.pPrevWritePtr;
		info.pEndPtr     = pstVpuPrivate->stRingBuffState.pWritePtr;

		LOG_IIMGE("[PUSH  ] [PTS: %8d / REGION: %7d ~ %7d]"
				  , (OMX_S32)(info.llTimestamp/1000)
				  , info.pStartPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  , info.pEndPtr - pstVpuPrivate->stRingBuffState.pRingBuffBase[VA]
				  );

		if( PushInputInfo(&pstVpuPrivate->stInputInfoQueue, &info) < 0 )
			return FEED_FAILED;

		pInputBuffer->nFilledLen = 0;
		pstVpuPrivate->lInputCount++;

		LOG_FEED("[INPUT_CNT: %7d] [QUEUED TIME: %8d ~ %8d ms] [DIFF:%10lld us] [LIMIT:%10lld us]"
				 , pstVpuPrivate->lInputCount
				 , (OMX_S32)(pstVpuPrivate->llQueuedStartTime/1000)
				 , (OMX_S32)(info.llTimestamp/1000)
				 , info.llTimestamp - pstVpuPrivate->llQueuedStartTime
				 , pstVpuPrivate->llFeedMinTimeDiff
				 );

		if( pstVpuPrivate->llQueuedStartTime < 0 ) {
			pstVpuPrivate->llQueuedStartTime = info.llTimestamp;
			return NEED_MORE_DATA;
		}

		// check criteria amount of time
		if( (info.llTimestamp - pstVpuPrivate->llQueuedStartTime) < pstVpuPrivate->llFeedMinTimeDiff )
			return NEED_MORE_DATA;

		#if UPDATE_WRITE_PTR_WITH_ALIGNED_LENGTH
		if( (pstVpuPrivate->stRingBuffState.lRingBuffSize - pstVpuPrivate->stRingBuffState.lEmptySpace) < WRITE_PTR_ALIGN_BYTES*4 ) //FIXME
			return NEED_MORE_DATA;
		#endif
	}
	else
	{
		LOG_FEED("[INPUT_CNT: %7d] [QUEUED TIME: %8d ~ %8d ms] [DIFF:%10lld us] [LIMIT:%10lld us] - [BUFFER FULL]"
				 , pstVpuPrivate->lInputCount
				 , (OMX_S32)(pstVpuPrivate->llQueuedStartTime/1000)
				 , (OMX_S32)(pInputBuffer->nTimeStamp/1000)
				 , pInputBuffer->nTimeStamp - pstVpuPrivate->llQueuedStartTime
				 , pstVpuPrivate->llFeedMinTimeDiff
				 );
	}

	if( UpdateRingBuffer(&pstVpuPrivate->stRingBuffState, OMX_FALSE) < 0 )
		return FEED_FAILED;

	return FEED_COMPLETE;
}

static 
OMX_S32 InitBufferIDInKernel(
	vpudec_private_t    *pstVpuPrivate
	)
{
	if(*pstVpuPrivate->pbIsVpuClosed == OMX_FALSE)
	{
		vbuffer_manager vBuff_Init;

		vBuff_Init.istance_index = vdec_get_instance_index(pstVpuPrivate->pVpuInstance);
		vBuff_Init.index = -1;
		ioctl(pstVpuPrivate->hFBDevice, TCC_VIDEO_SET_DISPLAYED_BUFFER_ID, &vBuff_Init);
	}

	return 0;
}

static
OMX_S32
VpuDecoderInit(
	vpudec_private_t    *pstVpuPrivate
	)
{
	omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	vdec_init_t      *p_vpu_init = &pstVpuPrivate->stVpuInit;
	vdec_user_info_t *p_userinfo = &pstVpuPrivate->stVpuUserInfo;
	OMX_S32 ret;

	INFO("VPU initialize");

	/* VPU initialize */
	p_vpu_init->m_iPicWidth           = p_outport->sPortParam.format.video.nFrameWidth;
	p_vpu_init->m_iPicHeight          = p_outport->sPortParam.format.video.nFrameHeight;
	p_vpu_init->m_bEnableVideoCache   = 0;
	p_vpu_init->m_bEnableUserData     = 0;
	p_vpu_init->m_pExtraData          = pstVpuPrivate->pbyExtraDataBuff; 
	p_vpu_init->m_iExtraDataLen       = pstVpuPrivate->lExtraDataLength;
	p_vpu_init->m_bM4vDeblk           = 0;
	p_vpu_init->m_uiDecOptFlags       = 0;
	p_vpu_init->m_uiMaxResolution     = 0;
	p_vpu_init->m_bFilePlayEnable     = 0;

	/* CbCr interleaved */
	if( CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE) ) {
		if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MJPG )
			p_vpu_init->m_bCbCrInterleaveMode = 0;
		else
			p_vpu_init->m_bCbCrInterleaveMode = 1;
	}
	else {

#ifdef OPENMAX1_2
		if( CHECK_FLAG(pstVpuPrivate, STATE_THUMBNAIL_MODE) )
			p_vpu_init->m_bCbCrInterleaveMode = 0;
		else // video-editor(STB x) or MooPlayer
			p_vpu_init->m_bCbCrInterleaveMode = 1;
#else
		if( CHECK_FLAG(pstVpuPrivate, STATE_REMOTE_PLAYER_MODE) )
			p_vpu_init->m_bCbCrInterleaveMode = 1;
		else
			p_vpu_init->m_bCbCrInterleaveMode = 0;
#endif
	}

	p_userinfo->bMaxfbMode = 0;	//FIXME
	p_userinfo->extFunction = 0;

#if defined(TC_SECURE_MEMORY_COPY)
	if( CHECK_FLAG(pstVpuPrivate, STATE_VPU_BUFFER_PROTECTION) )
		p_userinfo->extFunction |= EXT_FUNC_LOCK_FOR_SOURCE;
#endif

	if( (ret = vdec_vpu(VDEC_INIT, NULL, p_vpu_init, p_userinfo, pstVpuPrivate->pVpuInstance)) < 0 )
	{
		ERROR( "[VPU_ERROR] [OP: VDEC_INIT] [RET_CODE: %d]", ret);
		if(ret != -VPU_ENV_INIT_ERROR)
			*pstVpuPrivate->pbIsVpuClosed = OMX_FALSE; //to close vpu!!
		return ret;
	}

	/* VPU clock change with resolution and etc. */
	vpu_update_sizeinfo(p_vpu_init->m_iBitstreamFormat, 
						p_userinfo->bitrate_mbps, 
						p_userinfo->frame_rate, 
						p_vpu_init->m_iPicWidth, 
						p_vpu_init->m_iPicHeight,
						pstVpuPrivate->pVpuInstance);

	*pstVpuPrivate->pbIsVpuClosed = OMX_FALSE;
	InitBufferIDInKernel(pstVpuPrivate);

	/* VPU ring-buffer initialize */
	if( InitRingBuffer(&pstVpuPrivate->stRingBuffState, pstVpuPrivate->pVpuInstance) == OMX_FALSE )
		return -1;

	// for debugging
	pstVpuPrivate->stInputInfoQueue.pBasePtr = pstVpuPrivate->stRingBuffState.pRingBuffBase[VA];

	/* display information manager initialize */
	if( !CHECK_FLAG(pstVpuPrivate, STATE_IN_VPU_RESET_PROCESS) )
	{
		OMX_S32 frame_rate = 0;

		/* generate component instance ID */
		//if( !CHECK_FLAG(pstVpuPrivate, STATE_RESOLUTION_CHANGING) )
			pstVpuPrivate->ulComponentUID = (OMX_U32)(systemTime(SYSTEM_TIME_MONOTONIC)/1000000);

		/* input buffer information queue init. */
		if( InitInputQueue(&pstVpuPrivate->stInputInfoQueue, INPUT_INFO_QUEUE_INIT_SIZE) == OMX_FALSE )
			return -1;

		/* display buffer index/id queue init. */
		ClearDispIdxQueue(&pstVpuPrivate->stDispIdxQueue);

		/* display information manager init. */
		if( pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFrameRate )
			frame_rate = pstVpuPrivate->pstCdmxInfo->m_sVideoInfo.m_iFrameRate;
		else if( p_userinfo->frame_rate )
			frame_rate = p_userinfo->frame_rate;

		InitDispInfoManager(&pstVpuPrivate->stDispInfoMgr, 
							p_vpu_init->m_iBitstreamFormat, 
							frame_rate);
	}

	/* I-frame search enable */
	IFrameSearchEnable(pstVpuPrivate);

	CLEAR_FLAG(pstVpuPrivate, STATE_WAIT_FIRST_SYNCFRAME);
	SET_FLAG(pstVpuPrivate, STATE_SKIP_OUTPUT_B_FRAME);

	return 0;
}


static
OMX_S32
VpuDecoderSeqInit(
	vpudec_private_t    *pstVpuPrivate
	)
{
#define RETRY_WITH_MORE_DATA	1

	OMX_S32 ret;
	omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];

	/* VPU Sequence header initialize */
	INFO("Sequence header initialize");
	if( CHECK_FLAG(pstVpuPrivate, STATE_THUMBNAIL_MODE) )
		pstVpuPrivate->stVpuInput.m_iIsThumbnail = 1;
	else
		pstVpuPrivate->stVpuInput.m_iIsThumbnail = 0;

	/* get VPU reference frame buffer count */
#if defined(MVC_PROCESS_ENABLE) && defined(TCC_VPU_C7_INCLUDE)

	if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MVC ) {
		char value[PROPERTY_VALUE_MAX];
		pmap_t pmap;
		property_get("tcc.solution.mbox", value, "0");
		if( atoi(value) && pmap_get_info("video", &pmap) && pmap.size > 0x07000000 ) {
			INFO("MVC Output Enabled");
			SET_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE);
		}
	}

	if( CHECK_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE) ) {
		if(p_outport->sPortParam.format.video.nFrameHeight == 720)
			pstVpuPrivate->lRefBuffCount = p_outport->sPortParam.nBufferCountActual+4;
		else
			pstVpuPrivate->lRefBuffCount = p_outport->sPortParam.nBufferCountActual+6;
	}
	else
		pstVpuPrivate->lRefBuffCount = p_outport->sPortParam.nBufferCountActual;

#else

	pstVpuPrivate->lRefBuffCount = p_outport->sPortParam.nBufferCountActual;

#endif

	/* set additional VPU reference frame buffer count */
#ifdef VPU_BUFFER_MANAGEMENT
	InitDispBuffList(&pstVpuPrivate->stDispBuffList, pstVpuPrivate->lRefBuffCount, pstVpuPrivate->pVpuInstance, pstVpuPrivate->hFBDevice);
	vpu_set_additional_refframe_count(pstVpuPrivate->lRefBuffCount+1, pstVpuPrivate->pVpuInstance);
#else
	vpu_set_additional_refframe_count(pstVpuPrivate->lRefBuffCount, pstVpuPrivate->pVpuInstance);
#endif

	if( (ret = vdec_vpu(VDEC_DEC_SEQ_HEADER, 
						NULL, 
						&pstVpuPrivate->stVpuInput, 
						&pstVpuPrivate->stVpuOutput, 
						pstVpuPrivate->pVpuInstance)) < 0 )
	{
		ERROR("[VPU_ERROR] [OP: VDEC_DEC_SEQ_HEADER] [RET_CODE: %d]", ret);

		if ( ++pstVpuPrivate->lSeqInitCount >= SEQ_HEADER_INIT_ERROR_MAX ||
			 ret == -VPU_NOT_ENOUGH_MEM ||
			 ret == -RETCODE_INVALID_STRIDE || 
			 ret == -RETCODE_CODEC_SPECOUT || 
			 ret == -RETCODE_CODEC_EXIT ) 
		{
			ERROR("Sequence header init failed (INIT-CNT: %d / %d)", 
					pstVpuPrivate->lSeqInitCount, 
					SEQ_HEADER_INIT_ERROR_MAX);
			return ret;
		}

		if( UpdateRingBuffer(&pstVpuPrivate->stRingBuffState, OMX_TRUE) < 0 )
			return -1;

		INFO("Retry sequence header initialization with more data");
		return RETRY_WITH_MORE_DATA;
	}

	/* Store sequence header */
	INFO("Sequence header initialization success");
	if( pstVpuPrivate->lSeqHeaderLength <= 0 ) {
		//FIXME - sequence header backup
		OMX_U8 *p_seqhead = pstVpuPrivate->stRingBuffState.pReadPtr;
		OMX_S32 seqhead_length = pstVpuPrivate->stRingBuffState.lRingBuffSize - pstVpuPrivate->stRingBuffState.lEmptySpace;
		seqhead_length = seqhead_length < SEQHEAD_LENGTH_MAX ? seqhead_length : SEQHEAD_LENGTH_MAX;
		if( StoreSequenceHeader(
					pstVpuPrivate, 
					p_seqhead, 
					seqhead_length) == OMX_FALSE )
		{
			ERROR("StoreSequenceHeader() - returns OMX_FALSE");
			return -1;
		}
	}

	if( UpdateRingBuffer(&pstVpuPrivate->stRingBuffState, OMX_TRUE) < 0 )
		return -1;

	/* VPU ring-buffer reset */
	if( CHECK_FLAG(pstVpuPrivate, STATE_IN_VPU_RESET_PROCESS) ) {
		/* reset VPU ring-buffer state */
		if( !CHECK_FLAG(pstVpuPrivate, STATE_RESOLUTION_CHANGING) )
			ResetRingBuffer(&pstVpuPrivate->stRingBuffState);
		CLEAR_FLAG(pstVpuPrivate, STATE_IN_VPU_RESET_PROCESS);
	}

	/* set init flags */
	SET_FLAG(pstVpuPrivate, STATE_VPU_INITIALIZED);

	return 0;
}


static
OMX_S32
VpuDecoderDecode(
	vpudec_private_t    *pstVpuPrivate,
	OMX_U32             *pulResultFlags
	)
{
	OMX_U32 dec_result = 0;
	OMX_S32 ret;

#if RESOLUTION_SWITCHING_SUPPORT
	if( CHECK_FLAG(pstVpuPrivate, STATE_RESOLUTION_CHANGING) )
	{
		if( (ret = vdec_vpu(VDEC_DEC_FLUSH_OUTPUT, NULL, &pstVpuPrivate->stVpuInput, &pstVpuPrivate->stVpuOutput, pstVpuPrivate->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_DEC_FLUSH_OUTPUT] [RET_CODE: %d]", ret);
			return -RETCODE_CODEC_FINISH;
		}
	}
	else
#endif
	{
		if( (ret = vdec_vpu(VDEC_DECODE, NULL, &pstVpuPrivate->stVpuInput, &pstVpuPrivate->stVpuOutput, pstVpuPrivate->pVpuInstance)) < 0 ) {
			ERROR( "[VPU_ERROR] [OP: VDEC_DECODE] [RET_CODE: %d]", ret);
			return ret;
		}
	}

	/* set decoding status */
	switch( pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodingStatus )
	{
	case VPU_DEC_SUCCESS:
		dec_result |= DECODING_SUCCESS_FRAME;
		break;
	case VPU_DEC_SUCCESS_FIELD_PICTURE:
		dec_result |= DECODING_SUCCESS_FIELD;
		break;
	case VPU_DEC_BUF_FULL:
		dec_result |= DECODING_BUFFER_FULL;
		break;
	case 6: //FIXME - resolution change (not defined value)
		dec_result |= RESOLUTION_CHANGED;
		break;
	}

	/* set output status */
	if( pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS )
		dec_result |= DISPLAY_OUTPUT_SUCCESS;

	*pulResultFlags = dec_result;

	return 0;
}


static
OMX_S32
VpuDecSuccessProcess(
	vpudec_private_t       *pstVpuPrivate,
	OMX_TICKS               llDecTimestamp,
	OMX_S32                 lBufferIndex
	)
{
	dec_output_info_t *p_output = &pstVpuPrivate->stVpuOutput.m_DecOutInfo;
	disp_info_t *p_info = GetDispInfoSlot(&pstVpuPrivate->stDispInfoMgr, lBufferIndex);
	OMX_S32 frame_rate = 0;
	OMX_S32 ret;

	if( p_info == NULL ) {
		ERROR("GetDispInfoSlot() - returns NULL");
		return -1;
	}

	p_info->lFrameType           = p_output->m_iPicType;
	p_info->llTimestamp          = llDecTimestamp;
	p_info->lRvTimestamp         = 0;
	p_info->lPicStructure        = p_output->m_iPictureStructure;
	p_info->lM2vFieldSequence    = 0;
	p_info->lFrameDurationFactor = 0;
	p_info->lFrameSize           = 0; //FIXME - not used
	p_info->ulBufferFlags        = 0;
	p_info->bIsMvcDependent      = 0;

#ifndef OPENMAX1_2
	if( p_output->m_iPicType == 0 )
		p_info->ulBufferFlags |= OMX_BUFFERFLAG_DECODED_PIC_TYPE; // I-frame
	else
		p_info->ulBufferFlags &= ~OMX_BUFFERFLAG_DECODED_PIC_TYPE; // P/B-frame
#endif

	switch( pstVpuPrivate->stVpuInit.m_iBitstreamFormat )
	{
	case STD_RV:
		p_info->lRvTimestamp = p_output->m_iRvTimestamp;
		break;

	case STD_MVC:
	case STD_AVC:
		#if defined(TCC_VPU_C7_INCLUDE)
		if( CHECK_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE) )
			p_info->bIsMvcDependent = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded;
		#endif

		if( (p_output->m_iM2vProgressiveFrame == 0 && p_output->m_iPictureStructure == 3) 
			|| p_output->m_iInterlacedFrame 
			|| ((p_output->m_iPictureStructure == 1) && (pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iInterlace == 0)) )
			p_info->ulBufferFlags |= OMX_BUFFERFLAG_INTERLACED_FRAME;

		p_info->lM2vFieldSequence = 0;
		break;

	case STD_MPEG2: 					
		if ( (p_output->m_iM2vProgressiveFrame == 0 && p_output->m_iPictureStructure == 3 ) 
			 || p_output->m_iInterlacedFrame )
			p_info->ulBufferFlags |= OMX_BUFFERFLAG_INTERLACED_FRAME;

		if( p_output->m_iTopFieldFirst == 0 )
			p_info->ulBufferFlags |= OMX_BUFFERFLAG_ODD_FIRST_FRAME;

		p_info->lFrameDurationFactor = 2;

		if( p_output->m_iPictureStructure != 3 )
		{
			p_info->lFrameDurationFactor = 2; //FIXME - MPEG-2 PTS calculation (origin: 1)
		} 
		else if(p_output->m_iM2vProgressiveSequence == 0 )
		{
			if( p_output->m_iM2vProgressiveFrame == 0 )
			{
				p_info->lFrameDurationFactor = 2;
			}
			else
			{
				if(p_output->m_iRepeatFirstField == 0)
				{
					p_info->lFrameDurationFactor =2 ;
				}
				else
				{
					p_info->lFrameDurationFactor = 3 ;
				}
			}
		}
		else
		{ 
			if( p_output->m_iRepeatFirstField == 0 )
			{
				p_info->lFrameDurationFactor = 2;
			}
			else
			{
				if(p_output->m_iTopFieldFirst == 0)
				{
					p_info->lFrameDurationFactor = 4;
				}
				else
				{
					p_info->lFrameDurationFactor = 6;
				}
			}
		}

		p_info->lM2vFieldSequence = p_output->m_iM2vFieldSequence;
		frame_rate = p_output->m_iM2vFrameRate;
		frame_rate = ((frame_rate & 0xffff) * 1000) / ((frame_rate >> 16) + 1);
		
		break;

	default:
		p_info->lM2vFieldSequence = 0;
		break;
	}

	if( pstVpuPrivate->lNewFrameRate && frame_rate == 0 )  {
		frame_rate = pstVpuPrivate->lNewFrameRate;
		pstVpuPrivate->lNewFrameRate = 0;
	}
		
	if( UpdateDispInfo(&pstVpuPrivate->stDispInfoMgr, lBufferIndex, frame_rate) == OMX_FALSE ) 
		return -1;

	LOG_DEC("[TYPE: %s (%2d/%d)][PTS: %8d (diff: %4d)] [State: %2d/%2d][BuffIdx: %2d(%c)/%2d(%c)] [FieldSeq: %d][TR: %8d] [MBerr: %d]"
			, GetFrameTypeString(pstVpuPrivate->stVpuInit.m_iBitstreamFormat
								 , pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iPicType
								 , pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iPictureStructure)
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iPicType
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iPictureStructure
			, (OMX_S32)(llDecTimestamp/1000)
			, (OMX_S32)((pstVpuPrivate->llQueuedStartTime - llDecTimestamp) /1000)
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodingStatus
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iOutputStatus
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodedIdx
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodedIdx < 0 ? '-' : (pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded ? 'D' : 'B')
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDispOutIdx
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDispOutIdx < 0 ? '-' : (pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDisplay ? 'D' : 'B')
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iM2vFieldSequence
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iRvTimestamp
			, pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iNumOfErrMBs
			);		

	/* set next skip mode */
	if( pstVpuPrivate->lFrameSkipMode ) 
		SetFrameSkipMode(pstVpuPrivate, SKIPMODE_NEXT_STEP);

	return 0;
}


static
OMX_S32
VpuOutputProcess(
	vpudec_private_t       *pstVpuPrivate,
	OMX_BUFFERHEADERTYPE   *pOutputBuffer,
	OMX_BOOL                bDecBuffOutput,
	COPY_MODE               enCopyMode
	)
{
	omx_base_video_PortType *p_outport = (omx_base_video_PortType *)pstVpuPrivate->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	TCC_PLATFORM_PRIVATE_PMEM_INFO *p_plat_priv = NULL;
	TCC_PLATFORM_PRIVATE_PMEM_INFO privOutData;
	OMX_U32 *p_output = pOutputBuffer->pBuffer;
	OMX_S32 port_width = p_outport->sPortParam.format.video.nFrameWidth;
	OMX_S32 port_height = p_outport->sPortParam.format.video.nFrameHeight;
	OMX_U32 vsync_buff_id = pstVpuPrivate->nVSyncBuffId;
	OMX_S32 disp_idx;
	OMX_S32 disp_type = 0;
	OMX_S32 ret;

	if( pstVpuPrivate->pOutputPortPrivate != NULL )
		p_plat_priv = (TCC_PLATFORM_PRIVATE_PMEM_INFO*)pstVpuPrivate->pOutputPortPrivate;
	else
	{
		if( !CHECK_FLAG(pstVpuPrivate, STATE_THUMBNAIL_MODE) && !CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE) )
			p_plat_priv = &privOutData;
	}

	pstVpuPrivate->lOutputFailCount = 0;

	if( bDecBuffOutput )
		disp_idx = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDecodedIdx;
	else
		disp_idx = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_iDispOutIdx;

	if( p_plat_priv )
	{
		p_plat_priv->unique_addr = (OMX_PTR)pstVpuPrivate->ulComponentUID;
		p_plat_priv->copied = (enCopyMode == COPY_DONE) ? 1 : 0;
		p_plat_priv->width  = port_width;
		p_plat_priv->height = port_height;

		if( pstVpuPrivate->stVpuInit.m_bCbCrInterleaveMode )
			p_plat_priv->format = OMX_COLOR_FormatYUV420SemiPlanar;
		else
		{
			#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
			if( pstVpuPrivate->stVpuInit.m_iBitstreamFormat == STD_MJPG ) {
				//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )
				if( pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iMjpg_sourceFormat == 1 )
					p_plat_priv->format = OMX_COLOR_FormatYUV422Planar;
				else
					p_plat_priv->format = OMX_COLOR_FormatYUV420Planar;
			}
			else
			#endif
				p_plat_priv->format = OMX_COLOR_FormatYUV420Planar;
		}

		if( bDecBuffOutput )
		{
			/* physical address (platform private) */
			p_plat_priv->offset[0] = (OMX_U32)pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][0]; 
			p_plat_priv->offset[1] = (OMX_U32)pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][1]; 
			p_plat_priv->offset[2] = (OMX_U32)pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][2]; 
			#if defined(TCC_VPU_C7_INCLUDE)
			if( CHECK_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE) )
				disp_type = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded;
			#endif
		}
		else
		{
			/* physical address (platform private) */
			p_plat_priv->offset[0] = (OMX_U32)pstVpuPrivate->stVpuOutput.m_pDispOut[PA][0]; 
			p_plat_priv->offset[1] = (OMX_U32)pstVpuPrivate->stVpuOutput.m_pDispOut[PA][1]; 
			p_plat_priv->offset[2] = (OMX_U32)pstVpuPrivate->stVpuOutput.m_pDispOut[PA][2]; 
			#if defined(TCC_VPU_C7_INCLUDE)
			if( CHECK_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE) )
				disp_type = pstVpuPrivate->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDisplay;
			#endif
		}

		#if defined(MVC_PROCESS_ENABLE) && defined(TCC_VPU_C7_INCLUDE)
		if( CHECK_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE) )
		{
			if( disp_type )	// dependent
			{
				if( pstVpuPrivate->lMVCBaseViewIndex < 0 ) {
					ERROR("Dependent frame is decoded before its base frame");

					GetOutputBufferInfo(pstVpuPrivate, disp_idx, NULL);
					vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &disp_idx, NULL, pstVpuPrivate->pVpuInstance);

					p_plat_priv->offset[0] = 0;
					p_plat_priv->offset[1] = 0;
					p_plat_priv->offset[2] = 0;

					return ERROR_INVALID_OUTPUT_FRAME;
				}

				sprintf(p_plat_priv->name, "video");
				p_plat_priv->name[5] = 0;
				p_plat_priv->optional_info[0] = pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iPicWidth;
				p_plat_priv->optional_info[1] = pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iPicHeight;
				p_plat_priv->optional_info[2] = port_width;
				p_plat_priv->optional_info[3] = port_height;
				p_plat_priv->optional_info[4] = 0; //buffer_id // there is no need to put it.
				p_plat_priv->optional_info[5] = 0; //timeStamp	// there is no need to put it.
				p_plat_priv->optional_info[6] = 0; //curTime
				p_plat_priv->optional_info[7] = 0; //flags		// there is no need to put it.
				p_plat_priv->optional_info[8] = 0; //framerate	// there is no need to put it.
				p_plat_priv->optional_info[9] = disp_type;  //0: base / 1: dependent
				p_plat_priv->optional_info[10] = pstVpuPrivate->pMVCBaseView[0]; //MVC Base addr0
				p_plat_priv->optional_info[11] = pstVpuPrivate->pMVCBaseView[1]; //MVC Base addr1
				p_plat_priv->optional_info[12] = pstVpuPrivate->pMVCBaseView[2]; //MVC Base addr2
				p_plat_priv->optional_info[13] = 0; //Vsync enable field
				p_plat_priv->optional_info[14] = disp_idx; //current vpu index for on-the-fly mode not using video-vsync.
				p_plat_priv->optional_info[15] = vdec_get_instance_index(pstVpuPrivate->pVpuInstance);// current vpu instance-index.

				pstVpuPrivate->lMVCBaseViewIndex = -1;
				pstVpuPrivate->pMVCBaseView[0] = NULL;
				pstVpuPrivate->pMVCBaseView[1] = NULL;
				pstVpuPrivate->pMVCBaseView[2] = NULL;

				pOutputBuffer->nFilledLen = port_width * port_height * 3 / 2;
			}
			else
			{
				pstVpuPrivate->lMVCBaseViewIndex = disp_idx;
				pstVpuPrivate->pMVCBaseView[0] = p_plat_priv->offset[0];
				pstVpuPrivate->pMVCBaseView[1] = p_plat_priv->offset[1];
				pstVpuPrivate->pMVCBaseView[2] = p_plat_priv->offset[2];

				pOutputBuffer->nFilledLen = 0;
			}
		}
		else
		#endif
		{
			sprintf(p_plat_priv->name, "video");
			p_plat_priv->name[5] = 0;
			p_plat_priv->optional_info[0] = pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iPicWidth;
			p_plat_priv->optional_info[1] = pstVpuPrivate->stVpuOutput.m_pInitialInfo->m_iPicHeight;
			p_plat_priv->optional_info[2] = port_width;
			p_plat_priv->optional_info[3] = port_height;
			p_plat_priv->optional_info[4] = 0; //buffer_id // there is no need to put it.
			p_plat_priv->optional_info[5] = 0; //timeStamp	// there is no need to put it.
			p_plat_priv->optional_info[6] = 0; //curTime
			p_plat_priv->optional_info[7] = 0; //flags		// there is no need to put it.
			p_plat_priv->optional_info[8] = 0; //framerate	// there is no need to put it.
			p_plat_priv->optional_info[9] = disp_type; 
			p_plat_priv->optional_info[10] = 0; //MVC Base addr0
			p_plat_priv->optional_info[11] = 0; //MVC Base addr1
			p_plat_priv->optional_info[12] = 0; //MVC Base addr2
			p_plat_priv->optional_info[13] = 0; //Vsync enable field
			p_plat_priv->optional_info[14] = disp_idx; //current vpu index for on-the-fly mode not using video-vsync.
			p_plat_priv->optional_info[15] = vdec_get_instance_index(pstVpuPrivate->pVpuInstance);// current vpu instance-index.

			pOutputBuffer->nFilledLen = port_width * port_height * 3 / 2;
		}
	}

#ifdef ANDROID_USE_GRALLOC_BUFFER
	if( CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE) ) 
	{
		if( p_plat_priv )
		{
			#if defined(MVC_PROCESS_ENABLE) && defined(TCC_VPU_C7_INCLUDE)
			if( CHECK_FLAG(pstVpuPrivate, STATE_MVC_OUTPUT_ENABLE) )
			{
				if( disp_type )	// dependent
				{
					pOutputBuffer->pOutputPortPrivate = p_plat_priv;
				}
			}
			else
			#endif
			{
				pOutputBuffer->pOutputPortPrivate = p_plat_priv;
			}
		}

		if( disp_type == 0 )
		{
			if( bDecBuffOutput )
			{
				/* physical address */
				pstVpuPrivate->stGrallocInfo.pDispOut[PA][0] = pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][0];
				pstVpuPrivate->stGrallocInfo.pDispOut[PA][1] = pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][1];
				pstVpuPrivate->stGrallocInfo.pDispOut[PA][2] = pstVpuPrivate->stVpuOutput.m_pCurrOut[PA][2];
	
				/* virtual address */
				pstVpuPrivate->stGrallocInfo.pDispOut[VA][0] = pstVpuPrivate->stVpuOutput.m_pCurrOut[VA][0];
				pstVpuPrivate->stGrallocInfo.pDispOut[VA][1] = pstVpuPrivate->stVpuOutput.m_pCurrOut[VA][1];
				pstVpuPrivate->stGrallocInfo.pDispOut[VA][2] = pstVpuPrivate->stVpuOutput.m_pCurrOut[VA][2];
			}
			else
			{
				/* physical address */
				pstVpuPrivate->stGrallocInfo.pDispOut[PA][0] = pstVpuPrivate->stVpuOutput.m_pDispOut[PA][0];
				pstVpuPrivate->stGrallocInfo.pDispOut[PA][1] = pstVpuPrivate->stVpuOutput.m_pDispOut[PA][1];
				pstVpuPrivate->stGrallocInfo.pDispOut[PA][2] = pstVpuPrivate->stVpuOutput.m_pDispOut[PA][2];
	
				/* virtual address */
				pstVpuPrivate->stGrallocInfo.pDispOut[VA][0] = pstVpuPrivate->stVpuOutput.m_pDispOut[VA][0];
				pstVpuPrivate->stGrallocInfo.pDispOut[VA][1] = pstVpuPrivate->stVpuOutput.m_pDispOut[VA][1];
				pstVpuPrivate->stGrallocInfo.pDispOut[VA][2] = pstVpuPrivate->stVpuOutput.m_pDispOut[VA][2];
			}
		}
	}
#endif

	/* send vsync display buffer ID */
	#ifdef ANDROID_USE_GRALLOC_BUFFER
	if( p_plat_priv ) {
		p_plat_priv->optional_info[4] = vsync_buff_id;
		pstVpuPrivate->nVSyncBuffId++;
	}
	#else
	p_output[10] = vsync_buff_id;
	pOutputBuffer->nFilledLen += 4;
	pstVpuPrivate->nVSyncBuffId++;
	#endif

	/* skip B frame before I frame */
	if( CHECK_FLAG(pstVpuPrivate, STATE_SKIP_OUTPUT_B_FRAME) )
	{
		OMX_S32 pic_type;
		output_info_t out_info;

		/* get timestamp */
		if( (ret = GetOutputBufferInfo(pstVpuPrivate, disp_idx, &out_info)) < 0 ) {
			ERROR("GetOutputBufferInfo() - return: %d", ret);
			return ret;
		}

		pic_type = GetFrameType(pstVpuPrivate->stVpuInit.m_iBitstreamFormat, 
								out_info.lFrameType,
								out_info.lPicStructure);

		if( (pstVpuPrivate->stVpuInit.m_iBitstreamFormat != STD_MVC || out_info.bIsMvcDependent == 0) && (pic_type == PIC_TYPE_I || pic_type == PIC_TYPE_P) )  {
			if(!bDecBuffOutput)
				CLEAR_FLAG(pstVpuPrivate, STATE_SKIP_OUTPUT_B_FRAME);
			pOutputBuffer->nTimeStamp = out_info.llTimestamp;
			pOutputBuffer->nFlags = out_info.ulBufferFlags;
		} 
		else {
			//Clear VPU Buffer Flag
			vdec_vpu(VDEC_BUF_FLAG_CLEAR, NULL, &disp_idx, NULL, pstVpuPrivate->pVpuInstance);
			pOutputBuffer->nFilledLen = 0;
			LOG_OUT("[SKIP][BUFF_IDX: %2d]", disp_idx);
			return ERROR_SKIP_OUTPUT_FRAME;
		}
	}
	else
	{
		output_info_t out_info;

		/* get timestamp */
		if( (ret = GetOutputBufferInfo(pstVpuPrivate, disp_idx, &out_info)) < 0 ) {
			ERROR("GetOutputBufferInfo() - return: %d", ret);
			return ret;
		}

		pOutputBuffer->nTimeStamp = out_info.llTimestamp;
		pOutputBuffer->nFlags = out_info.ulBufferFlags;
	}

	/* check output mode changing status */
	if( CheckOutputModeChange(pstVpuPrivate) )
		ClearAllDisplayBuffers(pstVpuPrivate, OMX_TRUE, OMX_FALSE);

	if( ret = DisplayBufferProcess(pstVpuPrivate, disp_idx, disp_type, vsync_buff_id, pOutputBuffer) ) {
		ERROR("DisplayBufferProcess() - return: %d", ret);
		return ret;
	}

	if( CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE) )
	{
		//MOD - MVC 관련 테스트 코드
	}
	else
	{
		//////////////////////////////////////////////////////////////
		/// 
		TCC_PLATFORM_PRIVATE_PMEM_INFO *pmemInfoPtr;
		pmemInfoPtr = (TCC_PLATFORM_PRIVATE_PMEM_INFO *) pOutputBuffer->pOutputPortPrivate;
	}

	if( pOutputBuffer->nFilledLen > 0 ) {
		if( !CHECK_FLAG(pstVpuPrivate, STATE_THUMBNAIL_MODE) && !CHECK_FLAG(pstVpuPrivate, STATE_GRALLOC_MODE) ){
			p_plat_priv->optional_info[5] = (unsigned int)(pOutputBuffer->nTimeStamp/1000);
			p_plat_priv->optional_info[7] = (unsigned int)(pOutputBuffer->nFlags & ~0x000000FF);
			p_plat_priv->optional_info[8] = pstVpuPrivate->stVpuUserInfo.frame_rate/1000;
			memcpy( pOutputBuffer->pBuffer, p_plat_priv, sizeof(TCC_PLATFORM_PRIVATE_PMEM_INFO) );
		}
	}

	return 0;
}


static 
OMX_BOOL // OMX_TRUE: return immediately / OMX_FALSE: try next line
VpuErrorProcess(
	OMX_COMPONENTTYPE     *openmaxStandComp,
	OMX_BUFFERHEADERTYPE  *pInputBuffer, 
	OMX_BUFFERHEADERTYPE  *pOutputBuffer, 
	OMX_S32 lErrorCode
	)
{
	omx_vpudec_component_PrivateType *p_private = GET_VPUDEC_PRIVATE(openmaxStandComp);

	if( pOutputBuffer ) 
		pOutputBuffer->nFilledLen = 0;

	switch( lErrorCode ) {
	case -RETCODE_CODEC_EXIT:
	case -RETCODE_MULTI_CODEC_EXIT_TIMEOUT:
		if( !CHECK_FLAG(p_private, STATE_IN_VPU_RESET_PROCESS) && p_private->lSeqHeaderLength > 0 )
		{
			ERROR("VpuErrorProcess() - [VPU_ERROR] Codec Exit (code: %d)", lErrorCode);

			p_private->lSeqInitCount     = 0;
			p_private->lInputCount       = 0;
			p_private->lThumbFailCount   = 0;
			p_private->lOutputFailCount  = 0;
			p_private->llQueuedStartTime = -1;
	
			SET_FLAG(p_private, STATE_IN_VPU_RESET_PROCESS);
			CLEAR_FLAG(p_private, STATE_VPU_INITIALIZED);
	
			/* reset VPU ring-buffer state */
			ResetRingBuffer(&p_private->stRingBuffState);
	
			/* reset input/output information managers */
			ClearInputQueue(&p_private->stInputInfoQueue);
			ClearDispIdxQueue(&p_private->stDispIdxQueue);
			ResetDispInfoManager(&p_private->stDispInfoMgr);
			CLEAR_FLAG(p_private, STATE_CLEAR_DISPLAY_BUFFER);
	
			/* VPU close */
			if( *p_private->pbIsVpuClosed == OMX_FALSE ) {
				vdec_vpu(VDEC_CLOSE, NULL, NULL, &p_private->stVpuOutput, p_private->pVpuInstance);
				*p_private->pbIsVpuClosed = OMX_TRUE;
			}
	
			#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
			GrallocSrcReset(p_private);
			#endif
		}
		break;

	case -RETCODE_INSUFFICIENT_BITSTREAM:
		{
			ringbuff_state_t *p_ring_state = &p_private->stRingBuffState;
			input_info_t info_first;
			input_info_t info_last;
			OMX_TICKS temp_timestamp;
	
			ERROR("VpuErrorProcess() - [VPU_ERROR] Insufficient bitstream (code: %d)", lErrorCode);
	
			if( ShowInputInfo(&p_private->stInputInfoQueue, &info_first) >= 0 &&
				ShowLastInputInfo(&p_private->stInputInfoQueue, &info_last) >= 0 )
			{
				OMX_U8 *p_ring_base = p_ring_state->pRingBuffBase[VA];
				OMX_S32 start_pos = (OMX_S32)(info_first.pStartPtr - p_ring_base);
				OMX_S32 end_pos = (OMX_S32)(info_last.pEndPtr - p_ring_base);
				OMX_S32 data_size;
	
				if( start_pos < end_pos )
					data_size = end_pos - start_pos;
				else
					data_size = end_pos + (p_ring_state->lRingBuffSize - start_pos);
	
				ALOGD("[INPUTQUE_STATE] [PTS: %8d ~ %8d] [PTR: %d ~ %d] [SIZE: %d]",
					 (OMX_S32)(info_first.llTimestamp/1000),
					 (OMX_S32)(info_last.llTimestamp/1000),
					 start_pos,
					 end_pos,
					 data_size);
			}
	
			ALOGD("[RINGBUFF_STATE] [OMX_EMPTY: %d][OMX_WRITTEN: %d] [VPU_FILLED: %d]",
				 p_ring_state->lEmptySpace, 
				 p_ring_state->lWrittenBytes,
				 p_ring_state->lRingBuffSize - p_ring_state->lEmptySpace + p_ring_state->lWrittenBytes);
	
			// update ring-buffer and input queue status;
			GetCurrTimestamp(p_private, &temp_timestamp, OMX_FALSE, OMX_FALSE);

			SET_FLAG(p_private, STATE_INSUFFICIENT_BITSTREAM);
		}
		break;

	case -RETCODE_CODEC_FINISH:
		ERROR("VpuErrorProcess() - [VPU_ERROR] Codec Finish (code: %d)", lErrorCode);
		if( CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) )
			SET_FLAG(p_private, STATE_READY_TO_RESET);
		break;

	case ERROR_INVALID_BUFFER_STATE:
		ERROR("VpuErrorProcess() - Invalid Buffer State");
		if( CHECK_FLAG(p_private, STATE_READY_TO_RESET) )
			SET_FLAG(p_private, STATE_READY_TO_RESET);
		break;

	case ERROR_INVALID_OUTPUT_FRAME:
		ERROR("VpuErrorProcess() - Invalid Output Frame");
		break;

	case ERROR_SKIP_OUTPUT_FRAME:
		break;

	default:
		ERROR("VpuErrorProcess() - error %s: %d"
			 , lErrorCode<0 ? "code" : "line"
			 , lErrorCode
			 );

		if( *p_private->pbIsVpuClosed == OMX_FALSE )
		{
			vdec_vpu(VDEC_CLOSE, NULL, NULL, &p_private->stVpuOutput, p_private->pVpuInstance);
			*p_private->pbIsVpuClosed = OMX_TRUE;

			SendEventToClient(openmaxStandComp, 
							  OMX_EventError, 
							  OMX_ErrorHardware,
							  0
							  );
		}
		break;
	}

	return OMX_TRUE;
}


static 
void
VpuPrepareResolutionChange(
	OMX_COMPONENTTYPE     *openmaxStandComp
	)
{
	omx_vpudec_component_PrivateType *p_private = GET_VPUDEC_PRIVATE(openmaxStandComp);

	INFO("PREPARE RESOLUTION CHANGE");

	p_private->lSeqInitCount     = 0;
	p_private->lInputCount       = 0;
	p_private->lThumbFailCount   = 0;
	p_private->lOutputFailCount  = 0;
	p_private->llQueuedStartTime = -1;

	SET_FLAG(p_private, STATE_IN_VPU_RESET_PROCESS);
	CLEAR_FLAG(p_private, STATE_VPU_INITIALIZED);

	/* reset VPU ring-buffer state */
	ResetRingBuffer(&p_private->stRingBuffState);

	/* reset input/output information managers */
//	ClearInputQueue(&p_private->stInputInfoQueue);
	ClearDispIdxQueue(&p_private->stDispIdxQueue);
	ResetDispInfoManager(&p_private->stDispInfoMgr);
	CLEAR_FLAG(p_private, STATE_CLEAR_DISPLAY_BUFFER);

	/* VPU close */
	if( *p_private->pbIsVpuClosed == OMX_FALSE ) {
		vdec_vpu(VDEC_CLOSE, NULL, NULL, &p_private->stVpuOutput, p_private->pVpuInstance);
		*p_private->pbIsVpuClosed = OMX_TRUE;
	}

	#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
	GrallocSrcReset(p_private);
	#endif

	if( p_private->pSequenceHeader ) {
		TCC_free(p_private->pSequenceHeader);
		p_private->pSequenceHeader = NULL;
	}
	p_private->lSeqHeaderLength = 0;
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	Initialize / De-initialize
//
//
OMX_BOOL
omx_vpudec_component_InitVpuRingbufferCallback(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_BOOL             bModeChange
	)
{
	omx_videodec_component_PrivateType *p_omx_private = GET_VIDEODEC_PRIVATE(openmaxStandComp);
	omx_vpudec_component_PrivateType *p_vpu_private;

	CREATE_LOG_TAG();

	INFO("VPU-RING BUFFER CALLBACK INIT");

	if( p_omx_private->pVpuRingPrivate ) {
		return OMX_TRUE;
	}

	p_vpu_private = TCC_calloc(1, sizeof(omx_vpudec_component_PrivateType)); 
	if( p_vpu_private == NULL ) {
		ERROR("omx_vpudec_component_InitVpuRingbufferCallback() - out of memory");
		return OMX_FALSE;
	}

	p_omx_private->pVpuRingPrivate = p_vpu_private;
	memcpy(p_vpu_private, p_omx_private, sizeof(omx_base_filter_PrivateType));

	/* init flags */
	if( bModeChange == OMX_FALSE )
		SET_FLAG(p_vpu_private, STATE_WAIT_LIB_INIT);
	SET_FLAG(p_vpu_private, STATE_WAIT_FIRST_SYNCFRAME);
	SET_FLAG(p_vpu_private, STATE_SEQUENCE_HEADER_FOUND);
	SET_FLAG(p_vpu_private, STATE_GRALLOC_RESET);

	/* set parameters */
	p_vpu_private->stCropRectParam.nLeft     = p_omx_private->rectParm.nLeft;
	p_vpu_private->stCropRectParam.nTop      = p_omx_private->rectParm.nTop;
	p_vpu_private->stCropRectParam.nWidth    = p_omx_private->rectParm.nWidth;
	p_vpu_private->stCropRectParam.nHeight   = p_omx_private->rectParm.nHeight;

	p_vpu_private->stScaleFactor.xWidth 	= p_omx_private->scale.xWidth;
	p_vpu_private->stScaleFactor.xHeight 	= p_omx_private->scale.xHeight;

	if( p_omx_private->gralloc_info.PortBuffers[OMX_BASE_FILTER_OUTPUTPORT_INDEX].BufferType == GrallocPtr )
		SET_FLAG(p_vpu_private, STATE_GRALLOC_MODE);

	/* set configurations */
	if( p_omx_private->extradata && p_omx_private->extradata_size > 0) {
		p_vpu_private->pbyExtraDataBuff = TCC_calloc(1, p_omx_private->extradata_size); 
		if( p_vpu_private->pbyExtraDataBuff == NULL ) {
			ERROR("omx_vpudec_component_InitVpuRingbufferCallback() - out of memory");
			return OMX_FALSE;
		}
		memcpy(p_vpu_private->pbyExtraDataBuff, p_omx_private->extradata, p_omx_private->extradata_size);
		p_vpu_private->lExtraDataLength = p_omx_private->extradata_size;
	}

	p_vpu_private->stDispInfoMgr.bBackwardPlayback = !(p_omx_private->bPlayDirection);

	if( p_omx_private->bDecIndexOutput )
		SET_FLAG(p_vpu_private, STATE_DECODE_BUFFER_OUTPUT);

	if( p_omx_private->bThumbnailMode )
		SET_FLAG(p_vpu_private, STATE_THUMBNAIL_MODE);

	if( p_omx_private->bDRAM_16bit )
		SET_FLAG(p_vpu_private, STATE_16BIT_DRAM_MODE);

	/* init from videodec component private data */
	p_vpu_private->pstCodecSyncSem      = p_omx_private->avCodecSyncSem;
	p_vpu_private->pbIsVpuClosed        = &p_omx_private->pVideoDecodInstance.isVPUClosed;
	p_vpu_private->llQueuedStartTime	= -1;
	p_vpu_private->llFeedMinTimeDiff    = FEED_LIMIT_TIMEDIFF_INIT;
	p_vpu_private->llFeedMaxTimeDiff    = FEED_LIMIT_TIMEDIFF_INIT*2;
	p_vpu_private->pVpuInstance         = p_omx_private->pVideoDecodInstance.pVdec_Instance;
	p_vpu_private->hFBDevice            = p_omx_private->mProxy_fd;
	p_vpu_private->hTMemDevice          = p_omx_private->mTmem_fd;
	p_vpu_private->lUMPMapSize          = p_omx_private->mUmpReservedPmap.size;
	p_vpu_private->pUMPBase[PA]         = p_omx_private->mUmpReservedPmap.base;
	p_vpu_private->pUMPBase[VA]         = p_omx_private->mTMapInfo;
	p_vpu_private->lSeqBuffMapSize      = p_omx_private->mSeqBackupPmap.size;
	p_vpu_private->pSeqBuffBase[PA]     = p_omx_private->mSeqBackupPmap.base;
	p_vpu_private->pSeqBuffBase[VA]     = p_omx_private->mSeqBackupMapInfo;
	p_vpu_private->stVpuInit.m_iBitstreamFormat = p_omx_private->pVideoDecodInstance.gsVDecInit.m_iBitstreamFormat;
	p_vpu_private->pstCdmxInfo          = &p_omx_private->pVideoDecodInstance.cdmx_info;

#if SINGLE_FRAME_INPUT_ENABLE
	p_vpu_private->bSingleFrameInput    = OMX_TRUE;
#endif

	/* init from properties */
	char value[PROPERTY_VALUE_MAX] = {0};
	property_get("persist.sys.hdmi_output.enable", value, "");
	if( atoi(value) ) {
		INFO("HDMI output mode (MAX CLOCK)");
		SET_FLAG(p_vpu_private, STATE_HDMI_OUTPUT);	// used only for clock setting
	}

	property_get("tcc.solution.mbox", value, "");
	if( atoi(value) ) {
		INFO("STB solution mode (VSync mode)");
		SET_FLAG(p_vpu_private, STATE_VSYNC_OUTPUT);
	} 
	else {
		//property_get("tcc.sys.output_mode_detected", value, "");
		property_get("tcc.video.vsync.enable", value, "");
		if( atoi(value) ) {
			INFO("VSync output mode");
			SET_FLAG(p_vpu_private, STATE_VSYNC_OUTPUT);
		} 
		else {
			INFO("non-VSync mode");
			CLEAR_FLAG(p_vpu_private, STATE_VSYNC_OUTPUT);
		}
	}

	property_get("tcc.rplayer.stream", value, "");
	if( atoi(value) ) {
		INFO("Remote player mode");
		SET_FLAG(p_vpu_private, STATE_REMOTE_PLAYER_MODE);
	}

	property_get("tcc.remoteplayer.control", value, "");
	if( strcmp(value, "start") == 0 ) {
		INFO("Remote player mode");
		SET_FLAG(p_vpu_private, STATE_REMOTE_PLAYER_MODE);
	}

	property_get("tcc.video.lplayback.mode", value, "");
	if( atoi(value) ) {
		INFO("Local file playback mode");
		SET_FLAG(p_vpu_private, STATE_LOCAL_FILE_PLAY_MODE);
	}

#if defined(TC_SECURE_MEMORY_COPY)
	property_get("tcc.hdcp2.src.session.started", value, "0");
	if( atoi(value) ) {
		INFO("HDCP enable for WFD Source");
		SET_FLAG(p_vpu_private, STATE_VPU_BUFFER_PROTECTION);
	}
#endif

	/* output fail counting (after I-frame searching) */
	p_vpu_private->lThumbFailCount      = 0;
	p_vpu_private->lOutputFailCount     = 0;
	p_vpu_private->lOutputFailMax       = VPU_OUTPUT_FAIL_MAX;

	/* decoded frame skip counting after I-frame searching */
	p_vpu_private->lDecFrameSkipCount   = 0;
	p_vpu_private->lDecFrameSkipMax     = DECODE_ONLY_SKIP_MAX;
	p_vpu_private->lFrameSkipErrorCount = 0;

	/* gralloc module */
	memset(&p_vpu_private->stGrallocInfo, 0, sizeof(gralloc_info_t));
	p_vpu_private->stGrallocInfo.pstGrallocModule = p_omx_private->gralloc_info.grallocModule;
	p_vpu_private->stGrallocInfo.hCopyDev = -1;

	/* VPU ring-buffer state */
	p_vpu_private->stRingBuffState.lBackupStartOffset = -1;

	/* reset MVC base-view address */
	p_vpu_private->lMVCBaseViewIndex = -1;
	p_vpu_private->pMVCBaseView[0] = NULL;
	p_vpu_private->pMVCBaseView[1] = NULL;
	p_vpu_private->pMVCBaseView[2] = NULL;

	INFO("VPU-RING BUFFER CALLBACK INIT COMPLETE");

	return OMX_TRUE;
}

void
omx_vpudec_component_DeinitVpuRingbufferCallback(
	OMX_COMPONENTTYPE   *openmaxStandComp
	)
{
	omx_videodec_component_PrivateType *p_omx_private = GET_VIDEODEC_PRIVATE(openmaxStandComp);
	omx_vpudec_component_PrivateType *p_vpu_private = p_omx_private->pVpuRingPrivate;

	if( p_vpu_private == NULL ) {
		return;
	}

	INFO("VPU-RING BUFFER CALLBACK DEINIT");

	if( p_vpu_private->pbyExtraDataBuff )
		TCC_free(p_vpu_private->pbyExtraDataBuff);

	if( p_vpu_private->pThumbnailBuff )
		TCC_free(p_vpu_private->pThumbnailBuff);

	if( p_vpu_private->pSequenceHeader )
		TCC_free(p_vpu_private->pSequenceHeader);

	if( p_vpu_private->stGrallocInfo.pCopyParam )
		TCC_free(p_vpu_private->stGrallocInfo.pCopyParam);

#ifdef MOVE_HW_OPERATION
	if(p_vpu_private->stGrallocInfo.hCopyDev != -1)
		close(p_vpu_private->stGrallocInfo.hCopyDev);
#endif

	if( p_vpu_private->stRingBuffState.pBackupBuffer )
		TCC_free(p_vpu_private->stRingBuffState.pBackupBuffer);

	DeinitInputQueue(&p_vpu_private->stInputInfoQueue);

	TCC_free(p_vpu_private);

	p_omx_private->pVpuRingPrivate = NULL;

	INFO("VPU-RING BUFFER CALLBACK DEINIT COMPLETE");

	REMOVE_LOG_TAG();
}

void
omx_vpudec_component_SetParameter(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_INDEXTYPE        nParamIndex,
	OMX_PTR              pComponentParameterStructure
	)
{
	omx_videodec_component_PrivateType *p_omx_private = GET_VIDEODEC_PRIVATE(openmaxStandComp);
	omx_vpudec_component_PrivateType *p_vpu_private = p_omx_private->pVpuRingPrivate;

	if( p_vpu_private )
	{
		switch( nParamIndex ) 
		{
		case OMX_IndexParamPortDefinition:
			{
				p_vpu_private->stCropRectParam.nLeft     = p_omx_private->rectParm.nLeft;
				p_vpu_private->stCropRectParam.nTop      = p_omx_private->rectParm.nTop;
				p_vpu_private->stCropRectParam.nWidth    = p_omx_private->rectParm.nWidth;
				p_vpu_private->stCropRectParam.nHeight   = p_omx_private->rectParm.nHeight;

				INFO("[SET_PARAM] [CROP: pos(%d, %d) size(%d x %d)]"
					 , p_vpu_private->stCropRectParam.nLeft
					 , p_vpu_private->stCropRectParam.nTop
					 , p_vpu_private->stCropRectParam.nWidth
					 , p_vpu_private->stCropRectParam.nHeight
					 );
			}
			break;

		case OMX_IndexUseNativeBuffers:
			{
				EnableAndroidNativeBuffersParams *pParamNativeBuffer = (EnableAndroidNativeBuffersParams* )pComponentParameterStructure;

				if(pParamNativeBuffer->enable    == OMX_TRUE && 
				   pParamNativeBuffer->nPortIndex == OMX_BASE_FILTER_OUTPUTPORT_INDEX ) {	//output port
					SET_FLAG(p_vpu_private, STATE_GRALLOC_MODE);
					LOG_GRA("Gralloc mode enabled");
				}
			}
			break;

		default:
			break;
		}
	}
}

void
omx_vpudec_component_SetConfig(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_INDEXTYPE        nConfigIndex,
	OMX_PTR              pComponentConfigStructure	
	)
{
	omx_videodec_component_PrivateType *p_omx_private = GET_VIDEODEC_PRIVATE(openmaxStandComp);
	omx_vpudec_component_PrivateType *p_vpu_private = p_omx_private->pVpuRingPrivate;

	if( p_vpu_private )
	{
		switch( nConfigIndex ) 
		{
#ifndef OPENMAX1_2
		case OMX_IndexConfigVideoPlayDirection:
			p_vpu_private->stDispInfoMgr.bBackwardPlayback = !(*(OMX_BOOL*)pComponentConfigStructure);
			break;

		case OMX_IndexConfigVideoOutputKeyFrameOnly:
			if( *(OMX_BOOL*)pComponentConfigStructure )
				SET_FLAG(p_vpu_private, STATE_DECODE_BUFFER_OUTPUT);
			else
				CLEAR_FLAG(p_vpu_private, STATE_DECODE_BUFFER_OUTPUT);

			break;
#endif

		case OMX_IndexVendorThumbnailMode:
			if( *((OMX_BOOL *)pComponentConfigStructure) )
				SET_FLAG(p_vpu_private, STATE_THUMBNAIL_MODE);
			else
				CLEAR_FLAG(p_vpu_private, STATE_THUMBNAIL_MODE);

			break;

		default:
			break;
		}

		INFO("[SET_CONFIG] [BWD_PLAYBACK: %d] [DECBUFF_OUT: %d] [THUMBNAIL: %d]",
			 p_vpu_private->stDispInfoMgr.bBackwardPlayback,
			 CHECK_FLAG(p_vpu_private, STATE_DECODE_BUFFER_OUTPUT)?1:0,
			 CHECK_FLAG(p_vpu_private, STATE_THUMBNAIL_MODE)?1:0);
	}
}

void
omx_vpudec_component_GetConfig(
	OMX_COMPONENTTYPE   *openmaxStandComp,
	OMX_INDEXTYPE        nIndex,
	OMX_PTR              pComponentConfigStructure
	)
{
	omx_videodec_component_PrivateType *p_omx_private = GET_VIDEODEC_PRIVATE(openmaxStandComp);
	omx_vpudec_component_PrivateType *p_vpu_private = p_omx_private->pVpuRingPrivate;

	if( p_vpu_private )
	{
		switch( nIndex ) 
		{
			case OMX_IndexConfigCommonOutputCrop:
			{
				OMX_CONFIG_RECTTYPE *p_rect_param = (OMX_CONFIG_RECTTYPE *)pComponentConfigStructure;

				if (p_rect_param->nPortIndex != 1) {
			    	return OMX_ErrorUndefined;
				}
				p_rect_param->nLeft   = p_vpu_private->stCropRectParam.nLeft;
				p_rect_param->nTop    = p_vpu_private->stCropRectParam.nTop;
				p_rect_param->nWidth  = p_vpu_private->stCropRectParam.nWidth;
				p_rect_param->nHeight = p_vpu_private->stCropRectParam.nHeight;

				p_omx_private->rectParm.nLeft   = p_vpu_private->stCropRectParam.nLeft;
				p_omx_private->rectParm.nTop    = p_vpu_private->stCropRectParam.nTop;
				p_omx_private->rectParm.nWidth  = p_vpu_private->stCropRectParam.nWidth;
				p_omx_private->rectParm.nHeight = p_vpu_private->stCropRectParam.nHeight;
			}
			break;

			case OMX_IndexConfigCommonScale:
			{
				OMX_CONFIG_SCALEFACTORTYPE *p_scale_param = (OMX_CONFIG_SCALEFACTORTYPE *)pComponentConfigStructure;

				if (p_scale_param->nPortIndex != 1) {
				    return OMX_ErrorUndefined;
				}
				p_scale_param->xWidth  = p_vpu_private->stScaleFactor.xWidth;
				p_scale_param->xHeight = p_vpu_private->stScaleFactor.xHeight;

				p_omx_private->scale.xWidth   = p_vpu_private->stScaleFactor.xWidth;
				p_omx_private->scale.xHeight  = p_vpu_private->stScaleFactor.xHeight;
				ALOGE("SAR OMX_IndexConfigScale: 0x%lx/0x%lx", p_scale_param->xWidth, p_scale_param->xHeight);
			}
			break;

			default:
				break;
		}
	}
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//
//	Buffer management callback main
//
//
void 
omx_vpudec_component_BufferMgmtCallback(
	OMX_COMPONENTTYPE		*openmaxStandComp, 
	OMX_BUFFERHEADERTYPE	*pInputBuffer, 
	OMX_BUFFERHEADERTYPE	*pOutputBuffer
	)
{
	omx_vpudec_component_PrivateType *p_private = GET_VPUDEC_PRIVATE(openmaxStandComp);
	OMX_TICKS  curr_timestamp;
	OMX_U32    dec_result = 0;
	OMX_S32    ret = 0;
	OMX_U32    input_flags = pInputBuffer->nFlags;
#if defined(ANDROID_USE_GRALLOC_BUFFER)
	COPY_MODE  gralloc_copy_mode = COPY_NONE;
#endif

	CHECK_VPULOG_ENABLE();

#if LOG_INPUT_BUFF_INFO
	{
		static OMX_TICKS prev_pts;
		static OMX_U8 *p_prev_buff;

		LOG_INPUT("[LENGTH: %6d] [PTS: %8d] [BUFF: 0x%08x (%6d)] - [Flags: %c%c%c%c (0x%08X)] %s %s"
				  , pInputBuffer->nFilledLen
				  , (OMX_S32)(pInputBuffer->nTimeStamp/1000)
				  , pInputBuffer->pBuffer
				  , pInputBuffer->nOffset
				  , input_flags & OMX_BUFFERFLAG_CODECCONFIG ?    'C' : '-'
				  , input_flags & OMX_BUFFERFLAG_SYNCFRAME ?      'S' : '-'
				  , input_flags & OMX_BUFFERFLAG_IFRAME_ONLY ?    'I' : '-'
				  , input_flags & OMX_BUFFERFLAG_BFRAME_SKIP ?    'B' : '-'
				  , input_flags
				  , p_prev_buff != pInputBuffer->pBuffer ?        " NEWB" : " ----"
				  , prev_pts != pInputBuffer->nTimeStamp ?        " NEWP" : " ----"
				  );

		p_prev_buff = pInputBuffer->pBuffer;
		prev_pts = pInputBuffer->nTimeStamp;
	}
#endif

	if( CHECK_FLAG(p_private, STATE_WAIT_LIB_INIT) ) {
		tsem_down(p_private->pstCodecSyncSem);
		CLEAR_FLAG(p_private, STATE_WAIT_LIB_INIT);
	}

	pOutputBuffer->nFilledLen = 0;

	if( pInputBuffer->nFilledLen == 0 )
		return;

#if RESOLUTION_SWITCHING_SUPPORT
	if( CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) && CHECK_FLAG(p_private, STATE_READY_TO_RESET) ) {
		CLEAR_FLAG(p_private, STATE_READY_TO_RESET);
		VpuPrepareResolutionChange(openmaxStandComp);
	}
#endif

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// extract config data (from TCC extractor or the others)
	if( input_flags & OMX_BUFFERFLAG_CODECCONFIG )
	{
		p_private->ulExtractorType = (input_flags & OMX_BUFFERFLAG_EXTRACTOR_TYPE_FILTER);
		FindTccConfigData(p_private, pInputBuffer);	// TCC config data

		if( p_private->ulExtractorType != OMX_BUFFERFLAG_EXTRACTORTYPE_TCC )
			StoreConfigData(p_private, pInputBuffer); // specific data from non-TCC extractor

		DecideLimitTimeDiffForFeeding(p_private, p_private->stVpuUserInfo.frame_rate);

		//unknown unit input / decode only mode
		if( p_private->ulContainerType == CONTAINER_TYPE_MPG ||
			p_private->ulContainerType == CONTAINER_TYPE_TS ||
		    p_private->ulExtractorType == OMX_BUFFERFLAG_EXTRACTORTYPE_MPEG2TS )
		{
#if SINGLE_FRAME_INPUT_ENABLE
			char value[PROPERTY_VALUE_MAX];
			property_get("tcc.vpu.rb.singleframe.input", value, "0");
			if( value[0] != '0' ) {
				p_private->bSingleFrameInput = OMX_TRUE;
				INFO("Forced single frame input condition");
			}
			else
				p_private->bSingleFrameInput = OMX_FALSE;
#else
			p_private->bSingleFrameInput = OMX_FALSE;
#endif

			SET_FLAG(p_private, STATE_FRAME_RATE_UPDATE_ENABLE);
			CLEAR_FLAG(p_private, STATE_SEQUENCE_HEADER_FOUND);

#if AVC_DECODE_ONLY_MODE_ENABLE
			if(    p_private->stVpuInit.m_iBitstreamFormat == STD_AVC
				|| p_private->stVpuInit.m_iBitstreamFormat == STD_MVC )
				ConfigDecodeOnlyMode(p_private);
#endif
		}

		pInputBuffer->nFilledLen = 0;

		#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
		ioctl(p_private->hFBDevice, TCC_LCDC_VIDEO_SET_FRAMERATE, p_private->stVpuUserInfo.frame_rate);
		#endif

		return;
	}

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// VPU initialization
	///  - find sequence header (for MPEG2 Trasnport Stream)
	///  - VPU decoder init (open)
	///  - feed VPU decoder
	///  - VPU decoder sequence init
	///  - check needs port reconfiguration
	if( !CHECK_FLAG(p_private, STATE_VPU_INITIALIZED) ) 
	{
		/* Sequence header searching */
		if( !CHECK_FLAG(p_private, STATE_SEQUENCE_HEADER_FOUND) )
		{
			OMX_S32 seqhead_pos = ScanSequenceHeader(pInputBuffer, p_private->stVpuInit.m_iBitstreamFormat);
			if( seqhead_pos < 0 ) {
				if( ++p_private->lSeqInitCount >= SEQ_HEADER_SEARCH_MAX ) {
					ERROR("Sequence header is not found! (SEARCH-CNT: %d / %d)", 
							p_private->lSeqInitCount,
							SEQ_HEADER_SEARCH_MAX);
					VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
				}
				pInputBuffer->nFilledLen = 0;
				return;
			}

			INFO("Sequence header found (pos: %d / %d)", seqhead_pos, pInputBuffer->nFilledLen);

			pInputBuffer->nOffset += seqhead_pos;
			pInputBuffer->nFilledLen -= seqhead_pos;
			p_private->lSeqInitCount = 0;
			SET_FLAG(p_private, STATE_SEQUENCE_HEADER_FOUND);
		}

		/* VPU Initialize */
		if( *p_private->pbIsVpuClosed ) 
		{
			if( !CHECK_FLAG(p_private, STATE_THUMBNAIL_MODE) )
			{
#ifdef VPU_BUFFER_MANAGEMENT
				int res_changed = LASTFRAME_FOR_CODEC_CHANGE;
#else
				int res_changed = 0;
#endif
				
#if RESOLUTION_SWITCHING_SUPPORT
				if( CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) )
				{
					char value_vsync[PROPERTY_VALUE_MAX], value_hwrid[PROPERTY_VALUE_MAX];

					property_get("tcc.video.vsync.enable", value_vsync, "0");
					property_get("tcc.video.hwr.id", value_hwrid, "0");
					if( atoi(value_vsync) != 0 && atoi(value_hwrid) == p_private->ulComponentUID ) {
						usleep(p_private->stDispInfoMgr.llFrameDuration*2); //frame-duration * alpha
					} else {
						usleep(p_private->stDispInfoMgr.llFrameDuration*5);
					}

					res_changed = LASTFRAME_FOR_RESOLUTION_CHANGE;

					//temporary code
					//ioctl(p_private->hFBDevice, TCC_LCDC_HDMI_LASTFRAME, &res_changed);

					if( vdec_is_rendered_index(p_private->pVpuInstance) == 0 ) {
						if( ioctl(p_private->hFBDevice, TCC_LCDC_HDMI_LASTFRAME, &res_changed) < 0 ) {
							if( atoi(value_vsync) != 0 )  {
								usleep(p_private->stDispInfoMgr.llFrameDuration*3);
							}
						}
					}
				}
				else
#endif
				{
					if( vdec_is_rendered_index(p_private->pVpuInstance) == 0 )
						ioctl(p_private->hFBDevice, TCC_LCDC_HDMI_LASTFRAME, &res_changed);
				}
			}

			if( (ret = VpuDecoderInit(p_private)) < 0 ) {
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
				return;
			}
		}

#if RESOLUTION_SWITCHING_SUPPORT
		if( CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) && p_private->stRingBuffState.lBackupStartOffset >= 0 ) {
			if( RestoreRingBuffer(p_private) == OMX_FALSE ) {
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
				return;
			}
		}
#endif

		/* Fill ring-buffer with sequence header */
		if( p_private->lInputCount == 0 && p_private->lSeqHeaderLength > 0 ) {
			if( FeedVpuDecoder(p_private, NULL) == FEED_FAILED ) {	//NULL: from sequence header buffer
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
				return;
			}
		}

		/* Fill ring-buffer with input data */
		if( !CHECK_FLAG(p_private, STATE_IN_VPU_RESET_PROCESS) ) {
			if( ret = FeedVpuDecoder(p_private, pInputBuffer) ) {
				if( ret == FEED_FAILED )
					VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
				return; //NEED_MORE_DATA
			}
		}

		/* VPU Sequence header init */
		if( (ret = VpuDecoderSeqInit(p_private)) < 0 ) {
			VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
			return;
		}
		if( ret == RETRY_WITH_MORE_DATA )
			return;

		/* Check port configuration info. change */
		if( ret = CheckPortConfigChange(p_private) )
		{
			if( ret == MAX_RESOLUTION_EXCEEDED ) {
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
				return;
			}
			if( ret > 0 ) {
				if( ret & PORT_AR_CHANGE_NEEDED )
					SendEventToClient(openmaxStandComp, 
									  OMX_EventPortSettingsChanged, 
									  OMX_DirOutput,
									  OMX_IndexConfigCommonScale);

				if( ret & PORT_CROP_CHANGE_NEEDED )
					SendEventToClient(openmaxStandComp, 
									  OMX_EventPortSettingsChanged, 
									  OMX_DirOutput,
									  OMX_IndexConfigCommonOutputCrop);

				if( ret & PORT_RECONFIGURATION_NEEDED )
					SendEventToClient(openmaxStandComp, 
									  OMX_EventPortSettingsChanged, 
									  OMX_DirOutput,
									  0);

#if RESOLUTION_SWITCHING_SUPPORT
				if( CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) ) {
					INFO("Resolution Change Complete");
					CLEAR_FLAG(p_private, STATE_RESOLUTION_CHANGING);
					CLEAR_FLAG(p_private, STATE_READY_TO_RESET);
				}
#endif

				return;
			}
		}
	}

	if( *p_private->pbIsVpuClosed == OMX_TRUE ) {
		VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
		return;
	}


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Skimming / Fast-playback support
	if( input_flags & OMX_BUFFERFLAG_BFRAME_SKIP ) {
		SetFrameSkipMode(p_private, SKIPMODE_B_SKIP);
		CLEAR_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT);
	}
	else if( input_flags & OMX_BUFFERFLAG_IFRAME_ONLY )
	{
		SET_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT);
		SET_FLAG(p_private, STATE_SKIP_OUTPUT_B_FRAME);
	}
	else
	{
		//INFO("STATE_DECODE_BUFFER_OUTPUT is reset");
		//CLEAR_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT);
	}

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Sync-frame process
	if( CHECK_FLAG(p_private, STATE_SYNCFRAME_FORCED) || input_flags & OMX_BUFFERFLAG_SYNCFRAME )
	{
		OMX_BOOL is_seeking = CHECK_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT) ? OMX_FALSE : OMX_TRUE;

		INFO("SYNC-FRAME FLAG RECEIVED - %s", is_seeking ? "SEEKING" : "SKIMMING");

		/* reset MVC base-view address */
		p_private->lMVCBaseViewIndex = -1;
		p_private->pMVCBaseView[0] = NULL;
		p_private->pMVCBaseView[1] = NULL;
		p_private->pMVCBaseView[2] = NULL;

		/* reset gralloc copy state */
		#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
		GrallocSrcReset(p_private);
		#endif

		/* decoded buffer output mode */
		CLEAR_FLAG(p_private, STATE_SYNCFRAME_FORCED);

		/* reset VPU ring-buffer state */
		ResetRingBuffer(&p_private->stRingBuffState);

		/* reset input buffer information queue */
		ClearInputQueue(&p_private->stInputInfoQueue);

		/* reset display information manager */
		ResetDispInfoManager(&p_private->stDispInfoMgr);

		/* clear all display buffer */
		if( (ret = ClearAllDisplayBuffers(p_private, OMX_FALSE, is_seeking)) < 0 ) {
			ERROR("ClearAllDisplayBuffers() - returns %d", ret);
			VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
			return;
		}
		ClearDispIdxQueue(&p_private->stDispIdxQueue);

		/* feeder reset */
		p_private->llQueuedStartTime = -1;

		/* I-frame search enable */
		IFrameSearchEnable(p_private);

		CLEAR_FLAG(p_private, STATE_INSUFFICIENT_BITSTREAM);
		CLEAR_FLAG(p_private, STATE_WAIT_FIRST_SYNCFRAME);
		SET_FLAG(p_private, STATE_SKIP_OUTPUT_B_FRAME);
	}
	else 
	{
		if( p_private->lFrameSkipMode == SKIPMODE_NONE )
			CheckSkipSignal(p_private); //check late signal
	}

#if AVC_DECODE_ONLY_MODE_ENABLE //[1]
	if( CHECK_FLAG(p_private, STATE_IDR_SLICE_SCAN) )
	{
		OMX_S32 idr_pos = ScanAvcIdrSlice(pInputBuffer);
		if( idr_pos >= 0 )
		{
			INFO("IDR-FRAME FOUND");
			/* IDR slice is found */
			if( p_private->stVpuInput.m_iFrameSearchEnable == AVC_NONIDR_PICTURE_SEARCH_MODE ) {
				p_private->lOutputFailCount = 0;
				p_private->lIdrSliceScanCount = 0;
				p_private->lDecFrameSkipCount = 0;
				SetFrameSkipMode(p_private, SKIPMODE_IDR_SEARCH);
			}
		}
		else
		{
			/* IDR slice is not found */
			if( p_private->stVpuInput.m_iFrameSearchEnable == AVC_IDR_PICTURE_SEARCH_MODE ) {
				if( p_private->lIdrSliceScanCount++ < AVC_IDR_SLICE_SCAN_MAX ) {
					pInputBuffer->nFilledLen = 0;
					return;
				}
				INFO("IDR-FRAME NOT FOUND");
				p_private->lDecFrameSkipCount = 0;
				SetFrameSkipMode(p_private, SKIPMODE_NONIDR_SEARCH);
			}
		}

		CLEAR_FLAG(p_private, STATE_IDR_SLICE_SCAN);
	}
#endif
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Feed VPU decoder (fill ring-buffer)
	if( !CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) && pInputBuffer->nFilledLen ) {
		if( ret = FeedVpuDecoder(p_private, pInputBuffer) ) {
			if( ret == FEED_FAILED )
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
			return; //FEED_MORE_DATA
		}
		//FEED_COMPLETE
	}

#if defined(ANDROID_USE_GRALLOC_BUFFER)
	#if defined(MOVE_HW_OPERATION)
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Gralloc Copy Mode (kick-off copy)
	p_private->pOutputPortPrivate = NULL;
	if( CHECK_FLAG(p_private, STATE_GRALLOC_MODE) ) 
		gralloc_copy_mode = GrallocCopyRun(p_private, (buffer_handle_t)pOutputBuffer->pBuffer, OMX_FALSE);
	else
		p_private->pOutputPortPrivate = pOutputBuffer->pOutputPortPrivate;
	#else
	gralloc_copy_mode = COPY_FAILED;
	#endif
#endif

	if( p_private->stVpuInit.m_iBitstreamFormat == STD_VP8 ) {
		//MOD - VPX header 처리
	}
	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Decoding
	if( (ret = VpuDecoderDecode(p_private, &dec_result)) < 0 ) {
		ERROR("VpuDecoderDecode() - returns %d", ret);

#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
		if(gralloc_copy_mode == COPY_START)
			GrallocCopyWait(p_private, (buffer_handle_t)pOutputBuffer->pBuffer); //buffer unlock
#endif

		if( VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret) )
			return;
	}

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Gralloc Buffer Mode (wait copy complete)
#if defined(ANDROID_USE_GRALLOC_BUFFER) && defined(MOVE_HW_OPERATION)
	if(gralloc_copy_mode == COPY_START)
		gralloc_copy_mode = GrallocCopyWait(p_private, (buffer_handle_t)pOutputBuffer->pBuffer);
#endif

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Check resolution changing
	//if( dec_result & (DECODING_SUCCESS | RESOLUTION_CHANGED) ) // from commit 5921315de8a5d4348f11b22c418bfa7567461819 (2013.03.08)
	if( !CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) && 
		(    p_private->stVpuInit.m_iBitstreamFormat == STD_AVC 
		  || p_private->stVpuInit.m_iBitstreamFormat == STD_MVC 
		  || p_private->stVpuInit.m_iBitstreamFormat == STD_MPEG2 )
		)
	{
		if( IsResolutionChanged(p_private) ) {
#if RESOLUTION_SWITCHING_SUPPORT
			INFO("Resolution Change Detected");
			SET_FLAG(p_private, STATE_RESOLUTION_CHANGING);
			CLEAR_FLAG(p_private, STATE_READY_TO_RESET);
			if( BackupRingBuffer(p_private) == OMX_FALSE )
			{
				SendEventToClient(openmaxStandComp, 
								  OMX_EventError,
								  OMX_ErrorStreamCorrupt,
								  0);
			}
			// gralloc buffer copy ..
#else
			SendEventToClient(openmaxStandComp, 
							  OMX_EventError,
							  OMX_ErrorStreamCorrupt,
							  0);
			return;
#endif
		}
	}

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Get timestamp of the currently decoded frame
	///  - update ring buffer status
	///  - clear input information from the input info queue
	if( GetCurrTimestamp(  p_private
						 , &curr_timestamp
						 , dec_result & DECODING_SUCCESS
						 , p_private->stVpuOutput.m_DecOutInfo.m_iInterlacedFrame ) < 0 ) {
		VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
		return;
	}


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Decoding success process
	///  - calculate duration factor of the decoded frame 
	///  - update information of the decoded frame
	///  - set next frame skip mode
	///  - (create thumbnail)
	if( dec_result & DECODING_SUCCESS ) 
	{
		/* Update display information */
		if( dec_result & DECODING_SUCCESS_FRAME )
		{
			OMX_S32 decoded_idx = p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx;

			/* Frame-rate recalcuation */
			p_private->lFrameSuccessCount++;
			if( CHECK_FLAG(p_private, STATE_FRAME_RATE_UPDATE_ENABLE) && p_private->llQueuedStartTime != curr_timestamp ) {
				if( p_private->llQueuedStartTime < curr_timestamp ) {
					CLEAR_FLAG(p_private, STATE_FRAME_RATE_UPDATE_ENABLE);
				}
				else if( p_private->lFrameSuccessCount > 1 ) {
					OMX_TICKS frame_due = (p_private->llQueuedStartTime - curr_timestamp) / p_private->lFrameSuccessCount;
					p_private->lNewFrameRate = (OMX_S32)((OMX_TICKS)1000000000 / frame_due);
				}
				p_private->lFrameSuccessCount = 0;
			}

			if( decoded_idx >= 0 ) 
			{
				/* Decoding success process */
				if( (ret = VpuDecSuccessProcess(p_private, curr_timestamp, decoded_idx)) < 0 ){
					ERROR("VpuDecSuccessProcess() - returns %d", ret);
					VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
					return;
				}

				/* Thumbnail mode process */
				if( CHECK_FLAG(p_private, STATE_THUMBNAIL_MODE) ) {
					/* create thumbnail frame */
					if( p_private->stVpuOutput.m_DecOutInfo.m_iNumOfErrMBs > 0 &&
						p_private->lThumbFailCount++ < THUMBNAIL_DECODING_FAIL_MAX )
						return;
					CopyOutputFrame(p_private, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_DECODED_BUFFER);
					return;
				}

				/* set decoded buffer output flag */
				if( CHECK_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT) ) {
					dec_result |= DECODED_OUTPUT_SUCCESS;
					dec_result |= OUTPUT_SUCCESS;
					CLEAR_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT);
				}
			}
		}
		else if( dec_result & DECODING_SUCCESS_FIELD )	//field success is impossible
		{
			LOG_DEC("[TYPE: %s (%2d/%d)][PTS: %8d (diff: %4d)] [State: %2d/%2d][BuffIdx: %2d(%c)/%2d(%c)] [FieldSeq: %d][TR: %8d] [MBerr: %d]"
					, GetFrameTypeString(p_private->stVpuInit.m_iBitstreamFormat
										 , p_private->stVpuOutput.m_DecOutInfo.m_iPicType
										 , p_private->stVpuOutput.m_DecOutInfo.m_iPictureStructure)
					, p_private->stVpuOutput.m_DecOutInfo.m_iPicType
					, p_private->stVpuOutput.m_DecOutInfo.m_iPictureStructure
					, (OMX_S32)(curr_timestamp/1000)
					, (OMX_S32)((p_private->llQueuedStartTime - curr_timestamp) /1000)
					, p_private->stVpuOutput.m_DecOutInfo.m_iDecodingStatus
					, p_private->stVpuOutput.m_DecOutInfo.m_iOutputStatus
					, p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx
					, p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx < 0 ? '-' : (p_private->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded ? 'D' : 'B')
					, p_private->stVpuOutput.m_DecOutInfo.m_iDispOutIdx
					, p_private->stVpuOutput.m_DecOutInfo.m_iDispOutIdx < 0 ? '-' : (p_private->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDisplay ? 'D' : 'B')
					, p_private->stVpuOutput.m_DecOutInfo.m_iM2vFieldSequence
					, p_private->stVpuOutput.m_DecOutInfo.m_iRvTimestamp
					, p_private->stVpuOutput.m_DecOutInfo.m_iNumOfErrMBs
					);
		}
	}
	else
	{
		ERROR("[LOG_DEC] [TYPE: %s (%2d/%d)][PTS: %8d (diff: %4d)] [State: %2d/%2d][BuffIdx: %2d(%c)/%2d(%c)] [FieldSeq: %d][TR: %8d] [MBerr: %d]"
			 , GetFrameTypeString(p_private->stVpuInit.m_iBitstreamFormat
								  , p_private->stVpuOutput.m_DecOutInfo.m_iPicType
								  , p_private->stVpuOutput.m_DecOutInfo.m_iPictureStructure)
			 , p_private->stVpuOutput.m_DecOutInfo.m_iPicType
			 , p_private->stVpuOutput.m_DecOutInfo.m_iPictureStructure
			 , (OMX_S32)(curr_timestamp/1000)
			 , (OMX_S32)((p_private->llQueuedStartTime - curr_timestamp) /1000)
			 , p_private->stVpuOutput.m_DecOutInfo.m_iDecodingStatus
			 , p_private->stVpuOutput.m_DecOutInfo.m_iOutputStatus
			 , p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx
			 , p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx < 0 ? '-' : (p_private->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDecoded ? 'D' : 'B')
			 , p_private->stVpuOutput.m_DecOutInfo.m_iDispOutIdx
			 , p_private->stVpuOutput.m_DecOutInfo.m_iDispOutIdx < 0 ? '-' : (p_private->stVpuOutput.m_DecOutInfo.m_MvcPicInfo.m_iViewIdxDisplay ? 'D' : 'B')
			 , p_private->stVpuOutput.m_DecOutInfo.m_iM2vFieldSequence
			 , p_private->stVpuOutput.m_DecOutInfo.m_iRvTimestamp
			 , p_private->stVpuOutput.m_DecOutInfo.m_iNumOfErrMBs
			 );

		/* Thumbnail mode process */
		if( CHECK_FLAG(p_private, STATE_THUMBNAIL_MODE) && 
			p_private->lThumbFailCount++ >= THUMBNAIL_DECODING_FAIL_MAX ) 
		{
			/* create BLACK thumbnail frame */
			CopyOutputFrame(p_private, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_BLACK_FRAME);
			return;
		}
	}


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Output success process
	///  - output buffer setup
	///  - queue display buffer index
	///  - clear displayed buffer
	if( dec_result & OUTPUT_SUCCESS )
	{
		/* Output success process */
		if( (ret = VpuOutputProcess(p_private, pOutputBuffer, dec_result&DECODED_OUTPUT_SUCCESS, gralloc_copy_mode)) < 0 ){
			if( ret != ERROR_SKIP_OUTPUT_FRAME )
				ERROR("VpuOutputProcess() - returns %d", ret);
			VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
			return;
		}

#if defined(USE_EXTERNAL_BUFFER)
		if( !CHECK_FLAG(p_private, STATE_GRALLOC_MODE) && !CHECK_FLAG(p_private, STATE_REMOTE_PLAYER_MODE) ) {	//added - isRemotePlayerPlay 조건 추가 - thumbnail
			omx_base_video_PortType *p_outport = (omx_base_video_PortType *)p_private->ports[OMX_BASE_FILTER_OUTPUTPORT_INDEX];
	#ifndef OPENMAX1_2
			CopyOutputFrame(p_private, pOutputBuffer->pBuffer, &pOutputBuffer->nFilledLen, COPY_FROM_DISPLAY_BUFFER);
	#endif
			pOutputBuffer->nFilledLen = p_outport->sPortParam.format.video.nFrameWidth * p_outport->sPortParam.format.video.nFrameHeight * 3/2;
		}
#endif
	}
	else
	{
		if( CHECK_FLAG(p_private, STATE_RESOLUTION_CHANGING) ) {
			SET_FLAG(p_private, STATE_READY_TO_RESET);
			pOutputBuffer->nFilledLen = 0;
			return;
		}

		/* frame search/skip mode error process */
		if( p_private->lFrameSkipMode == SKIPMODE_I_SEARCH ) {
			if( p_private->lOutputFailCount++ >= p_private->lOutputFailMax ) {
				ERROR("Output failed %d times", p_private->lOutputFailCount);
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, __LINE__);
				return;
			}
		}

#if AVC_DECODE_ONLY_MODE_ENABLE //[4]
		if( CHECK_FLAG(p_private, STATE_DECODE_ONLY) && p_private->stVpuInput.m_iFrameSearchEnable == AVC_NONIDR_PICTURE_SEARCH_MODE ) {
			if( p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx >= 0 ) {
				if(p_private->lDecFrameSkipCount > 3)
					p_private->lDecFrameSkipCount -= 3;
				p_private->lFrameSkipErrorCount++;
			}
		}
#endif
	}


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Buffer full error control
	///  - 
#if 0
	if( dec_result & DECODING_BUFFER_FULL )
	{
		ERROR("BUFFER_FULL OCCURED!!");

		/* reset display information manager */
		ResetDispInfoManager(&p_private->stDispInfoMgr);

		/* clear all display buffer */
		if( !CHECK_FLAG(p_private, STATE_DECODE_BUFFER_OUTPUT) ) {
			if( (ret = ClearAllDisplayBuffers(p_private, OMX_FALSE, OMX_FALSE)) < 0 ) {
				ERROR("ClearAllDisplayBuffers() - returns %d", ret);
				VpuErrorProcess(openmaxStandComp, pInputBuffer, pOutputBuffer, ret);
				return;
			}
		}
		ClearDispIdxQueue(&p_private->stDispIdxQueue);
	}
#endif


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	/// Decode only mode process
#if AVC_DECODE_ONLY_MODE_ENABLE //[5] 
	if( CHECK_FLAG(p_private, STATE_DECODE_ONLY) )
	{
		if( p_private->lDecFrameSkipCount >= p_private->lDecFrameSkipMax ||
			p_private->lFrameSkipErrorCount >= DECODE_ONLY_ERROR_MAX )
		{
			CLEAR_FLAG(p_private, STATE_DECODE_ONLY);
			p_private->lDecFrameSkipCount   = 0;
			p_private->lFrameSkipErrorCount = 0;
		}
		else
		{
			if( p_private->stVpuOutput.m_DecOutInfo.m_iDecodedIdx >= 0 ||
				p_private->stVpuOutput.m_DecOutInfo.m_iDispOutIdx >= 0 ) 
				p_private->lDecFrameSkipCount++;

			pInputBuffer->nFlags |= OMX_BUFFERFLAG_DECODEONLY;
		}
	}
#endif
}

#endif //RING_BUFFER_MODULE_ENABLE


