/****************************************************************************
 *   FileName    : vdec_k.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/

#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"VPU_DEC_K"
#include <utils/Log.h>

#include "cdk_core.h"
#include "vdec.h"
#include "TCCMemory.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif
#ifdef INCLUDE_WMV78_DEC
#include "TCC_WMV78_DEC.h"
#include "TCC_WMV78_DEC_Huff_table.h"
#endif
#ifdef INCLUDE_SORENSON263_DEC
#include "th263dec.h"
#endif
#ifdef INCLUDE_JPU_MJPEG_DEC
#include "TCC_JPU_CODEC.h"  // For TCC892XS / TCC893XS, use JPU as a MJPEG H/W decoder
#endif

#include <sys/mman.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mach/tcc_vpu_ioctl.h>
#include <dlfcn.h>

#define LOGD(x...)    ALOGD(x)
#define LOGE(x...)    ALOGE(x)
#define LOGI(x...)    ALOGI(x)
#define LOGW(x...)    ALOGW(x)

/************************************************************************/
/* TEST and Debugging                                               								 */
/************************************************************************/
static int DEBUG_ON	= 0;
#define DPRINTF(msg...)	  ALOGE( ": " msg);
#define DSTATUS(msg...)	  if (DEBUG_ON) { ALOGD( ": " msg);}
#define DBUG_FLIP(msg...) if (DEBUG_ON) { ALOGD( ": " msg);}
#define DPRINTF_FRAME(msg...) //ALOGD(": " msg);
#define DISPLAY_BUFFER(msg...) //ALOGD(": " msg);
#define CLEAR_BUFFER(msg...) //ALOGE(": " msg);

//#define DEBUG_VPU_K_DEC_INPUT

#if defined(TCC_88XX_INCLUDE) || defined(TCC_93XX_INCLUDE)
#define VPU_PERFORMANCE_UP //use sram and vcache
#endif
#define TCC_VPU_INPUT_BUF_SIZE 		(1024 * 1024)
//#define MAX_FRAME_BUFFER_COUNT		31 // move to vdec.h

#define MAX_BITSTREAM_SIZE			(2 * 1024 * 1024)

//#define USE_VPU_INTERRUPT
//#define VPU_REGISTER_DUMP
//#define VPU_IN_FRAME_DUMP
//#define VPU_OUT_FRAME_DUMP // Change the output format into YUV420 seperated to play on PC.
//#define CHANGE_INPUT_STREAM // to change frame-data to test stream from RTP etc.
#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
unsigned char* backup_data = NULL;
FILE *pFs = NULL;
static unsigned int is1st_dec = 1;
static unsigned int stream_index = 0;
#ifdef CHANGE_INPUT_STREAM
#define  STREAM_MAX_IDX 196 //50
static unsigned int received_len[STREAM_MAX_IDX+1] = {0,};
#define STREAM_NAME "data/received.dat"
#define STREAM_LEN_NAME "data/received_len.dat"
#endif
#endif
#else
#include "vdec.h"

#include "../cdk/cdk_core.h"
#include "../cdk/cdk_sys.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif

#ifdef INCLUDE_SORENSON263_DEC
#include "sorensonH263dec/th263dec.h"
#endif
#endif

//#define DEBUG_TIME_LOG
#ifdef DEBUG_TIME_LOG
#include "time.h"
static unsigned int dec_time[30] = {0,};
static unsigned int time_cnt = 0;
static unsigned int total_dec_time = 0;
#endif
static int vdec_open_count = 0;

#define VPU_MGR_NAME	"/dev/vpu_mgr"
char *vpu_devices[4] =
{
	"/dev/vpu_dec",
	"/dev/vpu_dec_ext",
	"/dev/vpu_dec_ext2",
	"/dev/vpu_dec_ext3"
};

#include <fcntl.h>         // O_RDWR
#include <sys/poll.h>

typedef int (*DEC_FUNC)( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

typedef struct _vdec_ {
	int vdec_instance_index;
	int renderered;
	unsigned int total_frm;
	
	unsigned char vdec_env_opened;
	unsigned char vdec_codec_opened;
	unsigned char prev_codec;

	int vpu_mgr_fd; // default -1
	int vpu_dec_fd; // default -1

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	unsigned char* backup_data;
	FILE *pFs;
	unsigned int is1st_dec;
	unsigned int stream_index;
#ifdef CHANGE_INPUT_STREAM
	unsigned int received_len[STREAM_MAX_IDX+1];
#endif
#endif

#ifdef USE_VPU_INTERRUPT
	int vpu_intr_fd; // default -1
#endif

#ifdef HAVE_ANDROID_OS
	int gsBitstreamBufSize;
	int gsIntermediateBufSize;
	int gsUserdataBufSize;
	int gsMaxBitstreamSize;
	unsigned int gsAdditionalFrameCount;

	codec_addr_t gsBitstreamBufAddr[3];  /////Working JW
	codec_addr_t gsIntermediateBufAddr[3];
	codec_addr_t gsUserdataBufAddr[3];

	dec_user_info_t gsVpuDecUserInfo;
//	int gsAdditional_frame;

	codec_addr_t gsRegBaseVideoBusAddr;
	codec_addr_t gsRegBaseClkCtrlAddr;
#endif

	int gsBitWorkBufSize;
	int gsSliceBufSize;
	int gsSpsPpsSize;
	int gsFrameBufSize;
#if defined(TCC_VPU_C7_INCLUDE)
	int gsMbSaveSize;
#endif

	codec_addr_t gsBitWorkBufAddr[3];
	codec_addr_t gsSliceBufAddr;
	codec_addr_t gsSpsPpsAddr;
	codec_addr_t gsFrameBufAddr[3];
#if defined(TCC_VPU_C7_INCLUDE)
	codec_addr_t gsMbSaveAddr;
#endif

	VDEC_INIT_t gsVpuDecInit_Info;
	VDEC_SEQ_HEADER_t gsVpuDecSeqHeader_Info;
	VDEC_SET_BUFFER_t gsVpuDecBuffer_Info;
	VDEC_DECODE_t gsVpuDecInOut_Info;
	VDEC_RINGBUF_GETINFO_t gsVpuDecBufStatus;
	VDEC_RINGBUF_SETBUF_t gsVpuDecBufFill;
	VDEC_RINGBUF_SETBUF_PTRONLY_t gsVpuDecUpdateWP;
	VDEC_GET_VERSION_t gsVpuDecVersion;

	int gsbHasSeqHeader;
#ifdef INCLUDE_WMV78_DEC	// for WMV78 video decoder
	//static int gsWMV78DecodedFrmaeBufferDev;
	unsigned int gsWMV78CurYFrameAddress;
	unsigned int gsWMV78CurUFrameAddress;
	unsigned int gsWMV78CurVFrameAddress;
	unsigned int gsWMV78Ref0YFrameAddress;
	unsigned int gsWMV78Ref0UFrameAddress;
	unsigned int gsWMV78Ref0VFrameAddress;

	int 				gsWMV78FrameSize;
	int					gsWMV78NCFrameSize;
	WMV78_HANDLE		gsWMV78DecHandle;
	WMV78_INIT			gsWMV78DecInit;
	WMV78_INPUT			gsWMV78DecInput;
	WMV78_OUTPUT		gsWMV78DecOutput;
	dec_initial_info_t	gsWMV78DecInitialInfo;

#ifndef HAVE_ANDROID_OS
	unsigned int gsWMV78DecodedFrameAddress_NC[2];
#endif
#endif

#ifdef INCLUDE_SORENSON263_DEC
	codec_handle_t 		gsS263DecHandle; 
	dec_init_t 			gsS263DecInit;
	dec_initial_info_t 	gsS263DecInitialInfo;
	h263dec_stats_t 	gsS263DecInitialStats;
	unsigned char 		*gsS263DecHeapAddr;
#endif

#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC) || defined(JPEG_DECODE_FOR_MJPEG) || !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
	int 				gsFirstFrame;
	int 				gsIsINITdone;

	dec_init_t gsVpuDecInit;

	codec_addr_t decoded_phyAddr[10];
	codec_addr_t decoded_virtAddr[10];
	unsigned int decoded_buf_maxcnt;
	unsigned int decoded_buf_size;
	unsigned int decoded_buf_curIdx;
#endif

#ifdef JPEG_DECODE_FOR_MJPEG
	codec_handle_t 		gsJPEGDecHandle;
	dec_init_t 			gsJPEGDecInit;
	dec_user_info_t 	gsJPEGDecUserInfo;
	dec_initial_info_t 	gsJPEGDecInitialInfo;
	dec_input_t			gsJPEGDecInput;
	dec_output_t 		gsJPEGDecOutput;
#endif
	struct pollfd tcc_event[1];

	int bMaxfb_Mode;
	int extFunction;

	void* pExtDLLModule;
	DEC_FUNC gExtDecFunc;
} _vdec_;
//static unsigned char vdec_env_opened = 0, vdec_codec_opened = 0, prev_codec = 0;

//int vpu_mgr_fd = -1;
//int vpu_dec_fd = -1;
//#define VPU_MGR_NAME	"/dev/vpu_mgr"
//#define VPU_DEC_NAME	"/dev/vpu_dec"
//#define VPU_DEC_EXT_NAME	"/dev/vpu_dec_ext"

//#include <fcntl.h>         // O_RDWR
//#include <sys/poll.h>
//struct pollfd tcc_event[1];

#ifdef USE_VPU_INTERRUPT
#define TCC_INTR_DEV_NAME		"/dev/tcc_intr"
//int vpu_intr_fd = -1;
#endif

#ifdef INCLUDE_WMV78_DEC	// for WMV78 video decoder
//static int gsWMV78DecodedFrmaeBufferDev;
unsigned int gsWMV78CurYFrameAddress = 0;
unsigned int gsWMV78CurUFrameAddress = 0;
unsigned int gsWMV78CurVFrameAddress = 0;
unsigned int gsWMV78Ref0YFrameAddress = 0;
unsigned int gsWMV78Ref0UFrameAddress = 0;
unsigned int gsWMV78Ref0VFrameAddress = 0;
#ifndef HAVE_ANDROID_OS
unsigned int gsWMV78DecodedFrameAddress_NC[2];
#endif

//! Callback Func
typedef struct vdec_callback_func_t
{
	void* (*m_pfMalloc		 ) ( unsigned int );					//!< malloc
	void* (*m_pfNonCacheMalloc) ( unsigned int );					//!< non-cacheable malloc 
	void  (*m_pfFree			 ) ( void* );							//!< free
	void  (*m_pfNonCacheFree	 ) ( void* );							//!< non-cacheable free
	void* (*m_pfMemcpy		 ) ( void*, const void*, unsigned int );//!< memcpy
	void  (*m_pfMemset		 ) ( void*, int, unsigned int );		//!< memset
	void* (*m_pfRealloc		 ) ( void*, unsigned int );				//!< realloc
	void* (*m_pfMemmove		 ) ( void*, const void*, unsigned int );//!< memmove

	void* (*m_pfPhysicalAlloc)		( unsigned int );
	void  (*m_pfPhysicalFree)		( void*, unsigned int );
	void* (*m_pfVirtualAlloc)		( int*, unsigned int, unsigned int );
	void  (*m_pfVirtualFree)		( int*, unsigned int, unsigned int );
	int m_Reserved1[16-12];

	void*		 (*m_pfFopen	) (const char *, const char *);						//!< fopen
	unsigned int (*m_pfFread	) (void*, unsigned int, unsigned int, void* );		//!< fread
	int			 (*m_pfFseek	) (void*, long, int );								//!< fseek
	long		 (*m_pfFtell	) (void* );											//!< ftell
	unsigned int (*m_pfFwrite) (const void*, unsigned int, unsigned int, void*);	//!< fwrite
	int			 (*m_pfFclose) (void *);											//!< fclose
	int			 (*m_pfUnlink) ( const char *);										//!< _unlink
	unsigned int (*m_pfFeof  ) (void *);											//!< feof
	unsigned int (*m_pfFflush) (void *);											//!< fflush

	int			 (*m_pfFseek64) (void*, int64_t, int );								//!< fseek 64bi io
	int64_t		 (*m_pfFtell64) (void* );											//!< ftell 64bi io
	int m_Reserved2[16-11];
} vdec_callback_func_t;
#endif

/************************************************************************/
/* STATIC MEMBERS                                                       */
/************************************************************************/
#ifdef HAVE_ANDROID_OS
static int gsBitstreamBufSize = 0, gsUserdataBufSize = 0, gsMaxBitstreamSize = 0;
static codec_addr_t gsBitstreamBufAddr[3] = {0,}, gsUserdataBufAddr[3] = {0,};
//static int gsBitStreamBufDev = 0, gsUserdataBufDev = 0;
//static int gsAdditional_frame = 0;

static codec_addr_t gsRegBaseVideoBusAddr = 0, gsRegBaseClkCtrlAddr = 0;
#endif

static int gsBitWorkBufSize = 0, gsSliceBufSize = 0, gsSpsPpsSize = 0, gsFrameBufSize = 0, gsMbSaveSize = 0;
static codec_addr_t gsBitWorkBufAddr[3] = {0,}, gsSliceBufAddr = 0, gsSpsPpsAddr = 0, gsFrameBufAddr[3] = {0,}, gsMbSaveAddr = 0;
//static int gsBitWorkBufDev = 0, gsSliceBufDev = 0, gsSpsPpsDev = 0, gsFrameBufDev = 0;

#if defined(INCLUDE_WMV78_DEC)
static dec_init_t gsVpuDecInit;
#endif
#ifdef HAVE_ANDROID_OS
static dec_user_info_t gsVpuDecUserInfo;
#endif
static VDEC_INIT_t gsVpuDecInit_Info;
static VDEC_SEQ_HEADER_t gsVpuDecSeqHeader_Info;
static VDEC_SET_BUFFER_t gsVpuDecBuffer_Info;
static VDEC_DECODE_t gsVpuDecInOut_Info;

#ifdef INCLUDE_WMV78_DEC	// for WMV78 video decoder
static int gsWMV78FrameSize = 0, gsWMV78NCFrameSize = 0;
static WMV78_HANDLE			gsWMV78DecHandle = 0;
static WMV78_INIT			gsWMV78DecInit;
static WMV78_INPUT			gsWMV78DecInput;
static WMV78_OUTPUT			gsWMV78DecOutput;
static dec_initial_info_t	gsWMV78DecInitialInfo;
static int gsFirstFrame = 0;
static int gsIsINITdone = 0;
#endif

#ifdef INCLUDE_SORENSON263_DEC
static codec_handle_t gsS263DecHandle = 0;
static dec_init_t gsS263DecInit;
static dec_initial_info_t gsS263DecInitialInfo;
static h263dec_stats_t gsS263DecInitialStats;
static unsigned char *gsS263DecHeapAddr;
#endif

#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC) || defined(JPEG_DECODE_FOR_MJPEG)
static codec_addr_t decoded_phyAddr[10] = {0,};
static codec_addr_t decoded_virtAddr[10] = {0,};
static unsigned int decoded_buf_maxcnt = 0;
static unsigned int decoded_buf_size = 0;
static unsigned int decoded_buf_curIdx = 0;
#endif

#ifdef JPEG_DECODE_FOR_MJPEG
#include <mach/tcc_jpeg_ioctl.h>

#define JPEGDEC_DEVICE		"/dev/jpegdec"
#define JPEGDEC_STREAM_MAX  (4*1024*1024)
#define HW_DECODE_OVER_MAX_SIZE 4096
#define MAX_NUM_OF_JPEG_ELEMENT 10

static codec_handle_t gsJPEGDecHandle = 0;
static dec_init_t gsJPEGDecInit;
static dec_user_info_t gsJPEGDecUserInfo;
static dec_initial_info_t gsJPEGDecInitialInfo;
static dec_input_t gsJPEGDecInput;
static dec_output_t gsJPEGDecOutput;
#endif

static int gsbHasSeqHeader = 0;
#define LEVEL_MAX		11
#define PROFILE_MAX		6

static const char *strProfile[VCODEC_MAX][PROFILE_MAX] =
{
	//STD_AVC
	{ "Base Profile", "Main Profile", "Extended Profile", "High Profile", "Reserved Profile", "Reserved Profile" },
	//STD_VC1
	{ "Simple Profile", "Main Profile", "Advance Profile", "Reserved Profile", "Reserved Profile", "Reserved Profile" },
	//STD_MPEG2
	{ "High Profile", "Spatial Scalable Profile", "SNR Scalable Profile", "Main Profile", "Simple Profile", "Reserved Profile" },
	//STD_MPEG4
	{ "Simple Profile", "Advanced Simple Profile", "Advanced Code Efficiency Profile", "Reserved Profile", "Reserved Profile", "Reserved Profile" },
	//STD_H263
	{ " ", " ", " ", " ", " ", " " },
	//STD_DIV3
	{ " ", " ", " ", " ", " ", " " },
	//STD_RV
	{ "Real video Version 8", "Real video Version 9", "Real video Version 10", " ", " ", " " },
	//STD_AVS
	{ "Jizhun Profile", " ", " ", " ", " ", " " },
	//STD_WMV78
	{ " ", " ", " ", " ", " ", " " },
	//STD_SORENSON263
	{ " ", " ", " ", " ", " ", " " },
	//STD_MJPG
	{ " ", " ", " ", " ", " ", " " },
	//STD_VP8
	{ " ", " ", " ", " ", " ", " " },
	//STD_THEORA
	{ " ", " ", " ", " ", " ", " " },
	//???
	{ " ", " ", " ", " ", " ", " " },
	//STD_MVC
	{ "Base Profile", "Main Profile", "Extended Profile", "High Profile", "Reserved Profile", "Reserved Profile" }
};

static const char *strLevel[VCODEC_MAX][LEVEL_MAX] =
{
	//STD_AVC
	{ "Level_1B", "Level_", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level" },
	//STD_VC1
	{ "Level_L0(LOW)", "Level_L1", "Level_L2(MEDIUM)", "Level_L3", "Level_L4(HIGH)", "Reserved Level", "Reserved Level", "Reserved Level" },
	//STD_MPEG2
	{ "High Level", "High 1440 Level", "Main Level", "Low Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level" },
	//STD_MPEG4
	{ "Level_L0", "Level_L1", "Level_L2", "Level_L3", "Level_L4", "Level_L5", "Level_L6", "Reserved Level" },
	//STD_H263
	{ "", "", "", "", "", "", "", "" },
	//STD_DIV3
	{ "", "", "", "", "", "", "", "" },
	//STD_RV
	{ "", "", "", "", "", "", "", "" },
	//STD_AVS
	{"2.0 Level", "4.0 Level", "4.2 Level", "6.0 Level", "6.2 Level", "", "", ""},
	//STD_WMV78
	{ "", "", "", "", "", "", "", "" },
	//STD_SORENSON263
	{ "", "", "", "", "", "", "", "" },
	//STD_MJPG
	{ "", "", "", "", "", "", "", "" },
	//STD_VP8
	{ "", "", "", "", "", "", "", "" },
	//STD_THEORA
	{ "", "", "", "", "", "", "", "" },
	//???
	{ "", "", "", "", "", "", "", "" },
	//STD_MVC
	{ "Level_1B", "Level_", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level", "Reserved Level" }	
};

static void vpu_env_close(_vdec_ * pVdec);

#ifdef DEBUG_VPU_K_DEC_INPUT
static void PrintVPUHexData(unsigned char* pBuffer, unsigned int length, unsigned char* pTag)
{
	LOGI("[%s:%08d] 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ~ 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", pTag, length
			, pBuffer[0] , pBuffer[1] , pBuffer[2] , pBuffer[3]
			, pBuffer[4] , pBuffer[5] , pBuffer[6] 
			, pBuffer[7] , pBuffer[8] , pBuffer[9]
			, pBuffer[length - 10] , pBuffer[length - 9] , pBuffer[length - 8]
			, pBuffer[length - 7] , pBuffer[length - 6] , pBuffer[length - 5]
			, pBuffer[length - 4] , pBuffer[length - 3] , pBuffer[length - 2], pBuffer[length - 1]
		);
}
#endif

#ifdef HAVE_ANDROID_OS
static void *vdec_malloc(unsigned int size)
{
	void * ptr = TCC_malloc(size);
	return ptr;
}

static void vdec_free(void * ptr )
{
	TCC_free(ptr);
}

static void *cdk_sys_malloc_physical_addr(unsigned int *remap_addr, int uiSize, Buffer_Type type, _vdec_ *pVdec)
{
	MEM_ALLOC_INFO_t alloc_mem;
	_vdec_ * pInst = pVdec;
	memset(&alloc_mem, 0x00, sizeof(MEM_ALLOC_INFO_t));
	
	alloc_mem.request_size = uiSize;
	alloc_mem.buffer_type = type;
	ioctl(pInst->vpu_dec_fd, V_DEC_ALLOC_MEMORY, &alloc_mem);
	if(remap_addr != NULL)
		*remap_addr = alloc_mem.kernel_remap_addr;

	return (void*)( alloc_mem.phy_addr );;
}


static void *cdk_sys_malloc_virtual_addr(int* pDev, codec_addr_t pPtr, int uiSize, _vdec_ *pVdec)
{
	void *map_ptr = MAP_FAILED;
	_vdec_ * pInst = pVdec;
	map_ptr = (void *)mmap(NULL, uiSize, PROT_READ | PROT_WRITE, MAP_SHARED, pInst->vpu_dec_fd, pPtr);
	if(MAP_FAILED == map_ptr)
	{
		LOGE("mmap failed. fd(%d), addr(0x%x), size(%d)", pInst->vpu_dec_fd, pPtr, uiSize);
		return NULL;
	}
	
	return map_ptr;
}

static int cdk_sys_free_virtual_addr(int* pDev, void* pPtr, int uiSize)
{
	int ret = 0;
	
	if((ret = munmap((void*)pPtr, uiSize)) < 0)
	{
		LOGE("munmap failed. addr(0x%x), size(%d)", (unsigned int)pPtr, uiSize);
	}

	return ret;
}

static unsigned int cdk_sys_remain_memory_size( _vdec_ * pVdec )
{
	unsigned int sz_freeed_mem;
	_vdec_ * pInst = pVdec;
	ioctl(pInst->vpu_mgr_fd, VPU_GET_FREEMEM_SIZE, &sz_freeed_mem);

	return sz_freeed_mem;
}

void vpu_update_sizeinfo(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, void *pVdec)
{
	CONTENTS_INFO info;
	_vdec_ * pInst = pVdec;
	
	info.type = pInst->vdec_instance_index - VPU_DEC;

#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC)
	if(
	#ifdef INCLUDE_WMV78_DEC							
		format == STD_WMV78 
	#endif			
	#ifdef INCLUDE_SORENSON263_DEC				
		|| format == STD_SORENSON263
	#endif
	)
	{
		info.isSWCodec = 1;
		bps = 50;
	}
	else
#endif
		info.isSWCodec = 0;

	if( format == STD_MJPG )
	{
		info.width = AVAILABLE_MAX_WIDTH;
		info.height = AVAILABLE_MAX_HEIGHT;
	}
	else
	{
		info.width = image_width;
		info.height = image_height;
	}

#ifndef MOVE_HW_OPERATION	
	info.isSWCodec = 1;
	info.width = AVAILABLE_MAX_WIDTH;
	info.height = AVAILABLE_MAX_HEIGHT;
#endif
	info.bitrate = bps;
	info.framerate = fps/1000;
	ioctl(pInst->vpu_mgr_fd, VPU_SET_CLK, &info);

	return;
}

static int vpu_check_for_video(unsigned char open_status, _vdec_ *pVdec)
{
//return value : if hw reset need or not!!
	_vdec_ * pInst = pVdec;
	int ret_vpu_reset = 0;
	int vpu_wait;
	OPENED_gINFO gInfo;
	OPENED_sINFO sInfo;
	int temp_vpu_dec_fd = -1;

	if(open_status)
	{
		ret_vpu_reset = 0;

		LOGI("@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Instance[%d] = %s", pInst->vdec_instance_index, vpu_devices[pInst->vdec_instance_index]);
		temp_vpu_dec_fd = open(vpu_devices[pInst->vdec_instance_index], O_RDWR);
		if(temp_vpu_dec_fd < 0)
		{
			LOGE("%s open error[%s]", vpu_devices[pInst->vdec_instance_index], strerror(errno));
			return -1;
		}
	}

	sInfo.type = OPEN_VIDEO;
	if(open_status)
	{
		sInfo.opened_cnt = 1;
		
		pInst->vpu_dec_fd = temp_vpu_dec_fd;
		
		ioctl(pInst->vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
		ioctl(pInst->vpu_dec_fd, DEVICE_INITIALIZE, NULL);
	}
	else
	{
		sInfo.opened_cnt = 0;		
		ioctl(pInst->vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
	}
	
	return 0;
}

static int vpu_env_open(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, _vdec_ *pVdec)
{
	int vpu_reset = 0;
	_vdec_ * pInst = pVdec;
	DSTATUS("In  %s \n",__func__);

	if((vpu_reset = vpu_check_for_video(1, pInst)) < 0)
		goto err;

#ifdef  USE_VPU_INTERRUPT
	pInst->vpu_intr_fd = open(TCC_INTR_DEV_NAME, O_RDWR);
	if (pInst->vpu_intr_fd < 0) {
		LOGE("%s open error", TCC_INTR_DEV_NAME);
		goto err;
	}	
#endif

	pInst->prev_codec = format;
	vpu_update_sizeinfo(format, bps, fps, image_width, image_height, pInst);

#if defined(VPU_CLK_CONTROL)
	pInst->gsRegBaseVideoBusAddr	= (unsigned int)sPhysicalMemSetting(0xF0702000, 4*1024);
	if(!pInst->gsRegBaseVideoBusAddr)
		goto err;

	pInst->gsRegBaseClkCtrlAddr	= (unsigned int)sPhysicalMemSetting(0xF0700000, 4*1024);
	if(!pInst->gsRegBaseClkCtrlAddr)
		goto err;
#endif

	pInst->vdec_env_opened = 1;

	memset(&pInst->gsVpuDecInit_Info.gsVpuDecInit, 0x00, sizeof(dec_init_t));
	memset(&pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer, 0x00, sizeof(dec_buffer_t));
	memset(&pInst->gsVpuDecInOut_Info.gsVpuDecInput, 0x00, sizeof(dec_input_t));
//	memset(&pInst->gsVpuDecUserInfo, 0x00, sizeof(dec_user_info_t));
//	memset(&gsVpuDecInOut_Info.gsVpuDecOutput, 0x00, sizeof(dec_output_t));

	memset(&pInst->gsBitstreamBufAddr, 0x00, sizeof(pInst->gsBitstreamBufAddr));
	memset(&pInst->gsUserdataBufAddr, 0x00, sizeof(pInst->gsUserdataBufAddr));
	memset(&pInst->gsBitWorkBufAddr, 0x00, sizeof(pInst->gsBitWorkBufAddr));
	memset(&pInst->gsFrameBufAddr, 0x00, sizeof(pInst->gsFrameBufAddr));
	pInst->gsSliceBufAddr = pInst->gsSpsPpsAddr = 0;

#ifdef INCLUDE_WMV78_DEC
	memset(&pInst->decoded_phyAddr, 0x00, sizeof(pInst->decoded_phyAddr));
	memset(&pInst->decoded_virtAddr, 0x00, sizeof(pInst->decoded_virtAddr));
#endif

	DSTATUS("Out  %s \n",__func__);

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	pInst->pFs = NULL;
	pInst->is1st_dec = 1;
	pInst->stream_index = 0;
	pInst->backup_data = NULL;
#endif
#ifdef DEBUG_TIME_LOG	
	time_cnt = 0;
	total_dec_time = 0;
#endif
	pInst->gsAdditionalFrameCount = VPU_BUFF_COUNT;
	pInst->total_frm = 0;
	pInst->bMaxfb_Mode = 0;

	return 0;

err:	
	LOGE("vpu_env_open error");
	vpu_env_close(pInst);
	
	return -1;	
}


static void vpu_env_close(_vdec_ *pVdec)
{
	DSTATUS("In  %s \n",__func__);

	_vdec_ * pInst = pVdec;
#if defined(VPU_CLK_CONTROL)
	if(pInst->gsRegBaseVideoBusAddr)
		sPhysicalMemFree(pInst->gsRegBaseVideoBusAddr, 4*1024);

	if(pInst->gsRegBaseClkCtrlAddr)
		sPhysicalMemFree(pInst->gsRegBaseClkCtrlAddr, 4*1024);

	pInst->gsRegBaseVideoBusAddr = 0;
	pInst->gsRegBaseClkCtrlAddr = 0;	
#endif

#ifdef  USE_VPU_INTERRUPT
	if(pInst->vpu_intr_fd > 0)
	{
		if(close(pInst->vpu_intr_fd) < 0)
		{
			LOGE("%s close error", TCC_INTR_DEV_NAME);
		}
		pInst->vpu_intr_fd = -1;
	}
#endif	

	if(pInst->vpu_dec_fd)
	{
		if(close(pInst->vpu_dec_fd) < 0)
		{
			LOGE("%s close error", vpu_devices[pInst->vdec_instance_index]);
		}
		pInst->vpu_dec_fd = -1;
	}

	pInst->vdec_env_opened = 0;
	vpu_check_for_video(0, pInst);

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	if(pInst->pFs){
		fclose(pInst->pFs);
		pInst->pFs = NULL;
	}

	if(pInst->backup_data){
		vdec_free((void *) pInst->backup_data);
		pInst->backup_data = NULL;
	}
#endif

	DSTATUS("Out  %s \n",__func__);

}

#ifdef VPU_REGISTER_DUMP
static unsigned char bFirst_frame = 1;
static void filewrite_memory(char* name, char* addr, unsigned int size)
{
	FILE *fp;

	if(!bFirst_frame)
		return;
	
	fp = fopen(name, "ab+");		
	fwrite( addr, size, 1, fp);
	fclose(fp);
}
#endif

#ifdef USE_VPU_INTERRUPT
static void write_reg(unsigned int addr, unsigned int val)
{
	*((volatile unsigned int *)(gsRegisterBase + addr)) = (unsigned int)(val);
}

static unsigned int read_reg(unsigned int addr)
{
	return *(volatile unsigned int *)(gsRegisterBase + addr);
}

static int VpuInterrupt(_vdec_ *pVdec)
{
	int success = 0;
	_vdec_ * pInst = pVdec;
	while (1) {
		int ret;
		memset(pInst->tcc_event, 0, sizeof(pInst->tcc_event));
		pInst->tcc_event[0].fd = pInst->vpu_intr_fd;
		pInst->tcc_event[0].events = POLLIN;
		
		ret = poll((struct pollfd *)&pInst->tcc_event, 1, 500); // 500 msec
		if (ret < 0) {
			LOGE("vpu poll error\n");
			break;
		}else if (ret == 0) {
			LOGE("vpu poll timeout\n");
			break;
		}else if (ret > 0) {
			if (pInst->tcc_event[0].revents & POLLERR) {
				LOGE("vpu poll POLLERR\n");
				break;
			} else if (pInst->tcc_event[0].revents & POLLIN) {
				success = 1;
				break;
			}
		}
	}
	/* todo */

	write_reg(0x174, 0);
	write_reg(0x00C, 1);

	if(success)
		return RETCODE_SUCCESS;
	else
		return RETCODE_CODEC_EXIT;
}
#endif

static int vdec_cmd_process(int cmd, void* args, _vdec_ *pVdec)
{
	int ret;
	int success = 0;
	_vdec_ * pInst = pVdec;
	int retry_cnt = 10;

	if(ioctl(pInst->vpu_dec_fd, cmd, args) < 0)
	{
		LOGE("vpu[%d] ioctl err[%s] : cmd = 0x%x", pInst->vdec_instance_index, strerror(errno), cmd);
	}
	
	while (retry_cnt > 0) {
		memset(pInst->tcc_event, 0, sizeof(pInst->tcc_event));
		pInst->tcc_event[0].fd = pInst->vpu_dec_fd;
		pInst->tcc_event[0].events = POLLIN;
		
		ret = poll((struct pollfd *)&pInst->tcc_event, 1, 1000); // 1 sec
		if (ret < 0) {
			LOGE("vpu[%d]-retry(%d) poll error '%s'", pInst->vdec_instance_index, retry_cnt, strerror(errno));
			retry_cnt--;
			continue;
		}else if (ret == 0) {
			LOGE("vpu[%d]-retry(%d) poll timeout: %d'th frames, len %d", pInst->vdec_instance_index, retry_cnt, pInst->total_frm, pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize );
			retry_cnt--;
			continue;
		}else if (ret > 0) {
			if (pInst->tcc_event[0].revents & POLLERR) {
				LOGE("vpu[%d] poll POLLERR\n", pInst->vdec_instance_index);
				break;
			} else if (pInst->tcc_event[0].revents & POLLIN) {
				success = 1;
				break;
			}
		}
	}
	/* todo */

	switch(cmd)
	{
		case V_DEC_INIT:
			{			 
			 	VDEC_INIT_t* init_info = args;
				
				ioctl(pInst->vpu_dec_fd, V_DEC_INIT_RESULT, args);
				ret = init_info->result;
			}
			break;
			
		case V_DEC_SEQ_HEADER: 
			{			 
			 	VDEC_SEQ_HEADER_t* seq_info = args;
				
				ioctl(pInst->vpu_dec_fd, V_DEC_SEQ_HEADER_RESULT, args);
				ret = seq_info->result;
			}
			break;
			
		case V_DEC_DECODE:
			{			 
			 	VDEC_DECODE_t* decoded_info = args;
				
				ioctl(pInst->vpu_dec_fd, V_DEC_DECODE_RESULT, args);
				ret = decoded_info->result;
			}
			break;
			
		case V_DEC_FLUSH_OUTPUT:			
			{			 
			 	VDEC_DECODE_t* decoded_info = args;
				
				ioctl(pInst->vpu_dec_fd, V_DEC_FLUSH_OUTPUT_RESULT, args);
				ret = decoded_info->result;
			}
			break;
			
		case V_GET_RING_BUFFER_STATUS:
			{
				VDEC_RINGBUF_GETINFO_t* p_param = args;
				ioctl(pInst->vpu_dec_fd, V_GET_RING_BUFFER_STATUS_RESULT, args);
				ret = p_param->result;
			}
			break;
		case V_FILL_RING_BUFFER_AUTO:
			{
				VDEC_RINGBUF_SETBUF_t* p_param = args;
				ioctl(pInst->vpu_dec_fd, V_FILL_RING_BUFFER_AUTO_RESULT, args);
				ret = p_param->result;
			}
			break;
		case V_DEC_UPDATE_RINGBUF_WP:
			{
				VDEC_RINGBUF_SETBUF_PTRONLY_t* p_param = args;
				ioctl(pInst->vpu_dec_fd, V_DEC_UPDATE_RINGBUF_WP_RESULT, args);
				ret = p_param->result;
			}
			break;
		case V_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY:
			{
				VDEC_SEQ_HEADER_t* p_param = args;
				ioctl(pInst->vpu_dec_fd, V_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY_RESULT, args);
				ret = p_param->result;
			}
			break;
		case V_GET_VPU_VERSION:
			{
				VDEC_GET_VERSION_t* p_param = args;
				ioctl(pInst->vpu_dec_fd, V_GET_VPU_VERSION_RESULT, args);
				ret = p_param->result;
			}
			break;

		case V_DEC_REG_FRAME_BUFFER:			
		case V_DEC_BUF_FLAG_CLEAR:
		case V_DEC_CLOSE:
		case V_DEC_GET_INFO:			
		case V_DEC_REG_FRAME_BUFFER2:
		default:
			ioctl(pInst->vpu_dec_fd, V_DEC_GENERAL_RESULT, &ret);
			break;			
	}

	if(!success)
	{	
		LOGE("VDEC[%d] command(0x%x) didn't work properly. maybe hangup(no return(0x%x))!!", pInst->vdec_instance_index, cmd, ret);

		if(ret != RETCODE_CODEC_EXIT && ret != RETCODE_MULTI_CODEC_EXIT_TIMEOUT){
//			ioctl(pInst->vpu_mgr_fd, VPU_HW_RESET, (void*)NULL);
		}

		return RETCODE_CODEC_EXIT;
	}

	return ret;
}

void vpu_set_additional_refframe_count(int count, void* pInst)
{
	_vdec_ *pInst_dec = (_vdec_ *)pInst;

	pInst_dec->gsAdditionalFrameCount = count;
	DSTATUS( "[VDEC-%d] gsAdditionalFrameCount %d", pInst_dec->vdec_instance_index, pInst_dec->gsAdditionalFrameCount );
}

static void save_input_stream(char* name, int size, _vdec_ * pVdec)
{
#ifdef VPU_IN_FRAME_DUMP
	int i;
	_vdec_ * pInst = pVdec;
	unsigned char* ps = (unsigned char*)pInst->gsBitstreamBufAddr[VA];

	if(1)
	{
		FILE *fp;
		fp = fopen(name, "ab+");		  
		fwrite( ps, size, 1, fp);
		fclose(fp);

		return;
	}

	for(i=0; (i+10 <size) && (i+10 < 100); i += 10){
		DPRINTF_FRAME( "[VDEC - Stream] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", ps[i], ps[i+1], ps[i+2], ps[i+3], ps[i+4], ps[i+5], ps[i+6], ps[i+7], ps[i+8], ps[i+9] );
	}
#endif	
}		

static void save_decoded_frame(unsigned char* Y, unsigned char* U, unsigned char *V, int width, int height, _vdec_ *pVdec)
{
#ifdef VPU_OUT_FRAME_DUMP
	FILE *pFs;

	if(width == 176 && height == 144)
	{
		pFs = fopen("/sdcard/frame1.yuv", "ab+");
		if (!pFs) {
			LOGE("Cannot open '/sdcard/frame1.yuv'");
			return;
		}

		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
		fclose(pFs);
	}
	else if(width == 320 && height == 240)
	{
		pFs = fopen("/sdcard/frame2.yuv", "ab+");
		if (!pFs) {
			LOGE("Cannot open '/sdcard/frame2.yuv'");
			return;
		}

		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
		fclose(pFs);
	}
	else if(width == 640 && height == 480)
	{
		pFs = fopen("/sdcard/frame3.yuv", "ab+");
		if (!pFs) {
			LOGE("Cannot open '/sdcard/frame3.yuv'");
			return;
		}

		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
		fclose(pFs);
	}
#endif
}	

static void change_input_stream(unsigned char* out, int* len, int type, _vdec_ *pVdec)
{
#ifdef CHANGE_INPUT_STREAM
	_vdec_ * pInst = pVdec;
#if 1 //test
	if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_H263)
		return;
#endif

	if(type == VDEC_DECODE && pInst->is1st_dec)
	{
		pInst->is1st_dec = 0;
		*len = pInst->received_len[pInst->stream_index-1];
		memcpy( out, pInst->backup_data, pInst->received_len[pInst->stream_index-1]);
		LOGD("DEC => read[%d] - [%p] :: %p %p %p %p %p %p %p %p %p %p", pInst->stream_index -1, *len, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9]);
		return;
	}

	if(!pInst->pFs)
	{
		int len_count;
		pInst->pFs = fopen(STREAM_LEN_NAME, "rb");
		if (!pInst->pFs) 
		{
			LOGE("Cannot open '%s'", STREAM_NAME);
			return;
		}

		memset(pInst->received_len, 0x00, sizeof(pInst->received_len));
		fread((unsigned char*)pInst->received_len, (unsigned int)(STREAM_MAX_IDX*4), 1, pInst->pFs);
		fclose(pInst->pFs);
		pInst->pFs = NULL;
	}

	if(!pInst->pFs)
	{
		pInst->pFs = fopen(STREAM_NAME, "rb");
		if (!pInst->pFs) 
		{
			LOGE("Cannot open '%s'", STREAM_NAME);
			return;
		}
	}

	if(pInst->pFs && STREAM_MAX_IDX > pInst->stream_index)
	{
		if(pInst->received_len[pInst->stream_index])
		{
			fread( out, pInst->received_len[pInst->stream_index], 1, pInst->pFs);
			*len = pInst->received_len[pInst->stream_index];
		}

#if 0 //test
		stream_index++;
		if(received_len[stream_index])
		{
			fread( out, received_len[stream_index], 1, pFs);
			*len = received_len[stream_index];
		}
		stream_index++;
		if(received_len[stream_index])
		{
			fread( out, received_len[stream_index], 1, pFs);
			*len = received_len[stream_index];
		}		
#endif

		if(pInst->backup_data == NULL)
			pInst->backup_data = vdec_malloc(*len + 102400);

		if(pInst->is1st_dec && pInst->backup_data != NULL)
			memcpy(pInst->backup_data, out, *len);

		LOGD("read[%d] - [%p] :: %p %p %p %p %p %p %p %p %p %p", pInst->stream_index, *len, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9]);
		pInst->stream_index++;
	}
#endif	
}

unsigned char *vpu_getBitstreamBufAddr(unsigned int index, void * pVdec)
{
	unsigned char *pBitstreamBufAddr = NULL;
	_vdec_ * pInst = pVdec;
	if (index == PA)
	{
		pBitstreamBufAddr = (unsigned char *)pInst->gsBitstreamBufAddr[PA];
	}
	else if (index == VA)
	{
		pBitstreamBufAddr = (unsigned char *)pInst->gsBitstreamBufAddr[VA];
	}
	else /* default : PA */
	{
		pBitstreamBufAddr = (unsigned char *)pInst->gsBitstreamBufAddr[PA];
	}

	return pBitstreamBufAddr;
}

unsigned char *vpu_getFrameBufVirtAddr(unsigned char *convert_addr, unsigned int base_index, void *pVdec)
{
	unsigned char *pBaseAddr; 
	unsigned char *pTargetBaseAddr = NULL;
	unsigned int szAddrGap = 0;
	_vdec_ * pInst = pVdec;
	pTargetBaseAddr = (unsigned char*)pInst->gsFrameBufAddr[VA];
	
	if (base_index == K_VA)
	{
		pBaseAddr = (unsigned char*)pInst->gsFrameBufAddr[K_VA];
	}
	else /* default : PA */
	{
		pBaseAddr = (unsigned char*)pInst->gsFrameBufAddr[PA];
	}

	szAddrGap = convert_addr - pBaseAddr;
	
	return (pTargetBaseAddr+szAddrGap);
}

int vpu_getBitstreamBufSize(void *pVdec)
{
	_vdec_ * pInst = pVdec;
	return pInst->gsBitstreamBufSize;
}
#endif

static void print_dec_initial_info( dec_init_t* pDecInit, dec_initial_info_t* pInitialInfo )
{
	unsigned int fRateInfoRes = pInitialInfo->m_uiFrameRateRes;
	unsigned int fRateInfoDiv = pInitialInfo->m_uiFrameRateDiv;
	int userDataEnable = 0;
	int profile = 0;
	int level =0;

	DSTATUS("---------------VIDEO INITIAL INFO-----------------\n");
	if (pDecInit->m_iBitstreamFormat == STD_MPEG4) {
		DSTATUS("[VDEC]Data Partition Enable Flag [%1d]\n", pInitialInfo->m_iM4vDataPartitionEnable);
		DSTATUS("[VDEC]Reversible VLC Enable Flag [%1d]\n", pInitialInfo->m_iM4vReversibleVlcEnable);
		if (pInitialInfo->m_iM4vShortVideoHeader) {			
			DSTATUS("[VDEC]Short Video Header\n");
			DSTATUS("[VDEC]AnnexJ Enable Flag [%1d]\n", pInitialInfo->m_iM4vH263AnnexJEnable);
		} else
			DSTATUS("[VDEC]Not Short Video\n");		
	}

	switch(pDecInit->m_iBitstreamFormat) {
	case STD_MPEG2:
		profile = (pInitialInfo->m_iProfile==0 || pInitialInfo->m_iProfile>5) ? 5 : (pInitialInfo->m_iProfile-1);
		level = pInitialInfo->m_iLevel==4 ? 0 : pInitialInfo->m_iLevel==6 ? 1 : pInitialInfo->m_iLevel==8 ? 2 : pInitialInfo->m_iLevel==10 ? 3 : 4;
		break;
	case STD_MPEG4:
		if (pInitialInfo->m_iLevel & 0x80) 
		{
			// if VOS Header 

			if (pInitialInfo->m_iLevel == 8 && pInitialInfo->m_iProfile == 0) {
				level = 0; profile = 0; // Simple, Level_L0
			} else {
				switch(pInitialInfo->m_iProfile) {
					case 0xB:	profile = 2; break;
					case 0xF:	if( (pInitialInfo->m_iLevel&8) == 0) 
									profile = 1; 
								else
									profile = 5;
								break;
					case 0x0:	profile = 0; break;
					default :	profile = 5; break;
				}
				level = pInitialInfo->m_iLevel;
			}
			
			DSTATUS("[VDEC]VOS Header:%d, %d\n", profile, level);
		} 
		else 
		{ 
			// Vol Header Only
			level = 7; // reserved level
			switch(pInitialInfo->m_iProfile) {
				case  0x1: profile = 0; break; // simple object
				case  0xC: profile = 2; break; // advanced coding efficiency object
				case 0x11: profile = 1; break; // advanced simple object
				default  : profile = 5; break; // reserved
			}
			DSTATUS("[VDEC]VOL Header:%d, %d\n", profile, level);
		}

		if( level > 7 )
			level = 0;
		break;
	case STD_VC1:
		profile = pInitialInfo->m_iProfile;
		level = pInitialInfo->m_iLevel;
		break;
	case STD_AVC:
	case STD_MVC:
		profile = (pInitialInfo->m_iProfile==66) ? 0 : (pInitialInfo->m_iProfile==77) ? 1 : (pInitialInfo->m_iProfile==88) ? 2 : (pInitialInfo->m_iProfile==100) ? 3 : 4;
		if(profile<3) {
			// BP, MP, EP
			level = (pInitialInfo->m_iLevel==11 && pInitialInfo->m_iAvcConstraintSetFlag[3] == 1) ? 0 /*1b*/ 
				: (pInitialInfo->m_iLevel>=10 && pInitialInfo->m_iLevel <= 51) ? 1 : 2;
		} else {
			// HP
			level = (pInitialInfo->m_iLevel==9) ? 0 : (pInitialInfo->m_iLevel>=10 && pInitialInfo->m_iLevel <= 51) ? 1 : 2;
		}
		
		break;
	case STD_RV:
		profile = pInitialInfo->m_iProfile - 8;
		level = pInitialInfo->m_iLevel;
		break;
	case STD_H263:
		profile = pInitialInfo->m_iProfile;
		level = pInitialInfo->m_iLevel;
		break;
	case STD_DIV3:
		profile = pInitialInfo->m_iProfile;
		level = pInitialInfo->m_iLevel;
		break;
#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC) 
#ifdef INCLUDE_SORENSON263_DEC
	case STD_SORENSON263:
#endif
#ifdef INCLUDE_WMV78_DEC
	case STD_WMV78:
#endif
		profile = 0;
		level = 0;
		break;
#endif
	default: // STD_MJPG
		;
	}

	if( level >= LEVEL_MAX )
	{
		DSTATUS("[VDEC]Invalid \"level\" value: %d", level);
		level = 0;
	}
	if( profile >= PROFILE_MAX )
	{
		DSTATUS("[VDEC]Invalid \"profile\" value: %d", profile);
		profile = 0;
	}
	if( pDecInit->m_iBitstreamFormat >= VCODEC_MAX )
	{
		DSTATUS("[VDEC]Invalid \"m_iBitstreamFormat\" value: %d", pDecInit->m_iBitstreamFormat);
		pDecInit->m_iBitstreamFormat = 0;
	}

	// No Profile and Level information in WMV78
	if( 
#ifdef INCLUDE_WMV78_DEC
		(pDecInit->m_iBitstreamFormat != STD_WMV78) &&
#endif
		(pDecInit->m_iBitstreamFormat != STD_MJPG)
	)
	{
		DSTATUS("[VDEC]%s\n\r", strProfile[pDecInit->m_iBitstreamFormat][profile]);
		if (pDecInit->m_iBitstreamFormat != STD_RV) { // No level information in Rv.
			if ((pDecInit->m_iBitstreamFormat == STD_AVC || pDecInit->m_iBitstreamFormat == STD_MVC) && level != 0 && level != 2){
				DSTATUS("[VDEC]%s%.1f\n\r", strLevel[pDecInit->m_iBitstreamFormat][level], (float)pInitialInfo->m_iLevel/10);
			}
			else{
				DSTATUS("[VDEC]%s\n\r", strLevel[pDecInit->m_iBitstreamFormat][level]);
			}
		}
	}
	
	if(pDecInit->m_iBitstreamFormat == STD_AVC || pDecInit->m_iBitstreamFormat == STD_MVC) {
		DSTATUS("[VDEC]frame_mbs_only_flag : %d\n", pInitialInfo->m_iInterlace);
	} else if (pDecInit->m_iBitstreamFormat != STD_RV) {// No interlace information in Rv.
		if (pInitialInfo->m_iInterlace){
			DSTATUS("[VDEC]%s\n", "Interlaced Sequence");
		}
		else{
			DSTATUS("[VDEC]%s\n", "Progressive Sequence");
		}
	}

	if (pDecInit->m_iBitstreamFormat == STD_VC1) {
		if (pInitialInfo->m_iVc1Psf){
			DSTATUS("[VDEC]%s\n", "VC1 - Progressive Segmented Frame");
		}
		else{
			DSTATUS("[VDEC]%s\n", "VC1 - Not Progressive Segmented Frame");
		}
	}

	DSTATUS("[VDEC]Aspect Ratio [%1d]\n", pInitialInfo->m_iAspectRateInfo);
				
	switch (pDecInit->m_iBitstreamFormat) {
	case STD_AVC:
	case STD_MVC:
        	DSTATUS("[VDEC]H.264 Profile:%d Level:%d FrameMbsOnlyFlag:%d\n",
			pInitialInfo->m_iProfile, pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace);
		
		if(pInitialInfo->m_iAspectRateInfo) {
			int aspect_ratio_idc;
			int sar_width, sar_height;

			if( (pInitialInfo->m_iAspectRateInfo>>16)==0 ) {
				aspect_ratio_idc = (pInitialInfo->m_iAspectRateInfo & 0xFF);
				DSTATUS("[VDEC]aspect_ratio_idc :%d\n", aspect_ratio_idc);
			}
			else {
				sar_width  = (pInitialInfo->m_iAspectRateInfo >> 16);
				sar_height  = (pInitialInfo->m_iAspectRateInfo & 0xFFFF);
				DSTATUS("[VDEC]sar_width  : %d\nsar_height : %d", sar_width, sar_height);				
			}
		} else {
			LOGE("[VDEC]Aspect Ratio is not present");
		}

		break;
	case STD_VC1:
		if(pInitialInfo->m_iProfile == 0){
			DSTATUS("[VDEC]VC1 Profile: Simple\n");
		}
		else if(pInitialInfo->m_iProfile == 1){
			DSTATUS("[VDEC]VC1 Profile: Main\n");
		}
		else if(pInitialInfo->m_iProfile == 2){
			DSTATUS("[VDEC]VC1 Profile: Advanced\n");
		}
		
		DSTATUS("[VDEC]Level: %d Interlace: %d PSF: %d\n", 
			pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace, pInitialInfo->m_iVc1Psf);

		if(pInitialInfo->m_iAspectRateInfo){
			DSTATUS("[VDEC]Aspect Ratio [X, Y]:[%3d, %3d]\n", (pInitialInfo->m_iAspectRateInfo>>8)&0xff,
					(pInitialInfo->m_iAspectRateInfo)&0xff);
		}
		else{
			DSTATUS("[VDEC]Aspect Ratio is not present");
		}


		break;
	case STD_MPEG2:
        	DSTATUS("[VDEC]Mpeg2 Profile:%d Level:%d Progressive Sequence Flag:%d\n",
			pInitialInfo->m_iProfile, pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace);
		// Profile: 3'b101: Simple, 3'b100: Main, 3'b011: SNR Scalable, 
		// 3'b10: Spatially Scalable, 3'b001: High
		// Level: 4'b1010: Low, 4'b1000: Main, 4'b0110: High 1440, 4'b0100: High
		if(pInitialInfo->m_iAspectRateInfo){
			DSTATUS("[VDEC]Aspect Ratio Table index :%d\n", pInitialInfo->m_iAspectRateInfo);
		}
		else{
			DSTATUS("[VDEC]Aspect Ratio is not present");
		}
        	break;

	case STD_MPEG4:
        	DSTATUS("[VDEC]Mpeg4 Profile: %d Level: %d Interlaced: %d\n",
			pInitialInfo->m_iProfile, pInitialInfo->m_iLevel, pInitialInfo->m_iInterlace);
		// Profile: 8'b00000000: SP, 8'b00010001: ASP
		// Level: 4'b0000: L0, 4'b0001: L1, 4'b0010: L2, 4'b0011: L3, ...
		// SP: 1/2/3/4a/5/6, ASP: 0/1/2/3/4/5
		
		if(pInitialInfo->m_iAspectRateInfo){
			DSTATUS("[VDEC]Aspect Ratio Table index :%d\n", pInitialInfo->m_iAspectRateInfo);
		}
		else{
			DSTATUS("[VDEC]Aspect Ratio is not present");
		}
		break;

	case STD_RV:
        	DSTATUS("[VDEC]Real Video Version %d\n",	pInitialInfo->m_iProfile);
        	break;

#ifdef INCLUDE_SORENSON263_DEC
	case STD_SORENSON263:
			DSTATUS("[VDEC]Sorenson's H.263 \n");
        	break;
#endif

	case STD_MJPG:
		{
			const char mjpegFormatList[5][50] = { {"4:2:0"}, {"4:2:2"}, {"4:2:2 vertical"},{"4:4:4"},{"4:0:0"}};
			DSTATUS("[VDEC] MJPEG \n");
			DSTATUS("[VDEC] WIDTH: %d, HEIGHT: %d, SOURCE_FORMAT:%d %s\n", 
			pInitialInfo->m_iPicWidth,
			pInitialInfo->m_iPicHeight,
			pInitialInfo->m_iMjpg_sourceFormat,
			mjpegFormatList[pInitialInfo->m_iMjpg_sourceFormat] );
		}
		break;

   	}

	if (pDecInit->m_iBitstreamFormat == STD_RV) // RV has no user data
		userDataEnable = 0;


	DSTATUS("[VDEC]Dec InitialInfo =>\n m_iPicWidth: %u\n m_iPicHeight: %u\n frameRate: %.2f\n frRes: %u\n frDiv: %u\n",
		pInitialInfo->m_iPicWidth, pInitialInfo->m_iPicHeight, (double)fRateInfoRes/fRateInfoDiv, fRateInfoRes, fRateInfoDiv);

#ifdef INCLUDE_SORENSON263_DEC
	if (pDecInit->m_iBitstreamFormat == STD_SORENSON263) {		
		int disp_width = pInitialInfo->m_iPicWidth;
		int disp_height = pInitialInfo->m_iPicHeight;
		int crop_left = pInitialInfo->m_iAvcPicCrop.m_iCropLeft;
		int crop_right = pInitialInfo->m_iAvcPicCrop.m_iCropRight;
		int crop_top = pInitialInfo->m_iAvcPicCrop.m_iCropTop;
		int crop_bottom = pInitialInfo->m_iAvcPicCrop.m_iCropBottom;

		if( crop_left | crop_right | crop_top | crop_bottom ){
			disp_width = disp_width - ( crop_left + crop_right );
			disp_height = disp_height - ( crop_top + crop_bottom );

			DSTATUS("[VDEC]Dec InitialInfo =>\n Display_PicWidth: %u\n Display_PicHeight: %u\n",
				disp_width, disp_height);
		}
	}
#endif

	DSTATUS("---------------------------------------------------\n");
	
}


#define ALIGN_LEN (4*1024)
int
vpu_sorensonH263dec_ready( dec_init_t* psVDecInit, _vdec_ *pVdec )
{
#ifdef HAVE_ANDROID_OS
		_vdec_ * pInst = pVdec;
		unsigned int remapped_addr; //to use in kernel!!
		//------------------------------------------------------------
		//! [x] bitstream buffer for each VPU decoder
		//------------------------------------------------------------
		pInst->gsBitstreamBufSize = LARGE_STREAM_BUF_SIZE;
		pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsBitstreamBufSize, ALIGN_LEN );
		if(pInst->vdec_instance_index == 0)
		{
			pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_STREAM, pInst);
		}
		else
		{
			pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_ELSE, pInst);	
		//	pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_WORK, pInst);	
		}
		if( pInst->gsBitstreamBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC] bitstream_buf_addr[PA] malloc() failed \n");
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS( "[VDEC] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize );
		pInst->gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize, pInst );
		if( pInst->gsBitstreamBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC] bitstream_buf_addr[VA] malloc() failed \n");
			return -(VPU_NOT_ENOUGH_MEM);
		}
		memset( (void*)pInst->gsBitstreamBufAddr[VA], 0x00 , pInst->gsBitstreamBufSize);
		DSTATUS("[VDEC] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", (codec_addr_t)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize );
	
		psVDecInit->m_BitstreamBufAddr[PA]	= pInst->gsBitstreamBufAddr[PA];
		psVDecInit->m_BitstreamBufAddr[VA]	= pInst->gsBitstreamBufAddr[K_VA];
		psVDecInit->m_iBitstreamBufSize 	= pInst->gsBitstreamBufSize;
#endif

		return 0;
}

int
vpu_dec_ready( dec_init_t* psVDecInit, _vdec_ *pVdec)
{
	//------------------------------------------------------------
	// [x] PS(SPS/PPS) buffer for each VPU decoder
	//------------------------------------------------------------
	_vdec_ * pInst = pVdec;
	if( psVDecInit->m_iBitstreamFormat == STD_AVC || psVDecInit->m_iBitstreamFormat == STD_MVC)
	{
		pInst->gsSpsPpsSize = PS_SAVE_SIZE;
		pInst->gsSpsPpsSize = ALIGNED_BUFF( pInst->gsSpsPpsSize, ALIGN_LEN );
		pInst->gsSpsPpsAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsSpsPpsSize, 0, pInst );
		if( pInst->gsSpsPpsAddr == 0 ) 
		{
			DPRINTF( "[VDEC-%d] sps_pps_buf_addr malloc() failed \n", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS("[VDEC-%d] sps_pps_buf_addr = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsSpsPpsAddr, pInst->gsSpsPpsSize );

		psVDecInit->m_pSpsPpsSaveBuffer = (unsigned char*)pInst->gsSpsPpsAddr;
		psVDecInit->m_iSpsPpsSaveBufferSize = pInst->gsSpsPpsSize;
	}

#ifdef HAVE_ANDROID_OS
	//------------------------------------------------------------
	//! [x] bitstream buffer for each VPU decoder
	//------------------------------------------------------------

	if(pInst->gsVpuDecUserInfo.m_bJpegOnly && pInst->gsVpuDecUserInfo.jpg_length > LARGE_STREAM_BUF_SIZE)
		pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsVpuDecUserInfo.jpg_length, 64*1024 );
	else
		pInst->gsBitstreamBufSize = LARGE_STREAM_BUF_SIZE;
	pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsBitstreamBufSize, ALIGN_LEN );
	if(pInst->vdec_instance_index == 0)
	{
		pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_STREAM, pInst );
	}
	else
	{
		pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_ELSE, pInst );
	}
	
	if( pInst->gsBitstreamBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bitstream_buf_addr[PA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS( "[VDEC-%d] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize );
	pInst->gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize, pInst );
	if( pInst->gsBitstreamBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bitstream_buf_addr[VA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	memset( (void*)pInst->gsBitstreamBufAddr[VA], 0x00 , pInst->gsBitstreamBufSize);
	DSTATUS("[VDEC-%d] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize );

	psVDecInit->m_BitstreamBufAddr[PA]	= pInst->gsBitstreamBufAddr[PA];
	psVDecInit->m_BitstreamBufAddr[VA]	= pInst->gsBitstreamBufAddr[K_VA];	
	psVDecInit->m_iBitstreamBufSize 	= pInst->gsBitstreamBufSize;

	if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable == 0)
	{
#if 0 // intermediate buffer is not used.
		pInst->gsIntermediateBufSize = LARGE_STREAM_BUF_SIZE;
		pInst->gsIntermediateBufSize = ALIGNED_BUFF( pInst->gsIntermediateBufSize, ALIGN_LEN );
		pInst->gsIntermediateBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsIntermediateBufAddr[K_VA], pInst->gsIntermediateBufSize, BUFFER_ELSE, pInst );
		
		if( pInst->gsIntermediateBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d] gsIntermediateBufAddr[PA] malloc() failed \n", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS( "[VDEC-%d] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsIntermediateBufAddr[PA], pInst->gsIntermediateBufSize );
		pInst->gsIntermediateBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsIntermediateBufAddr[PA], pInst->gsIntermediateBufSize, pInst );
		if( pInst->gsIntermediateBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d] gsIntermediateBufAddr[VA] malloc() failed \n", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		memset( (void*)pInst->gsIntermediateBufAddr[VA], 0x00 , pInst->gsIntermediateBufSize);
		DSTATUS("[VDEC-%d] gsIntermediateBufAddr[VA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsIntermediateBufAddr[VA], pInst->gsIntermediateBufSize );
#endif
	}
	else
	{
		pInst->gsIntermediateBufSize = 0;
		pInst->gsIntermediateBufAddr[PA] = 0;
		pInst->gsIntermediateBufAddr[VA] = 0;
		pInst->gsIntermediateBufAddr[K_VA] = 0;
	}

	/* Set the maximum size of input bitstream. */
//	gsMaxBitstreamSize = MAX_BITSTREAM_SIZE;
//	gsMaxBitstreamSize = ALIGNED_BUFF(gsMaxBitstreamSize, (4 * 1024));
//	if (gsMaxBitstreamSize > gsBitstreamBufSize)
//	{
		pInst->gsMaxBitstreamSize = pInst->gsBitstreamBufSize;
//	}
#endif

	//------------------------------------------------------------
	//! [x] user data buffer for each VPU decoder
	//------------------------------------------------------------
	if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
	{
		pInst->gsUserdataBufSize = 50 * 1024;
		pInst->gsUserdataBufSize = ALIGNED_BUFF( pInst->gsUserdataBufSize, ALIGN_LEN );
		if(pInst->vdec_instance_index == 0)
		{
			pInst->gsUserdataBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsUserdataBufAddr[K_VA], pInst->gsUserdataBufSize, BUFFER_ELSE, pInst );
		}
		else
		{
			pInst->gsUserdataBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsUserdataBufAddr[K_VA], pInst->gsUserdataBufSize, 0, pInst );
		}
		if( pInst->gsUserdataBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d:Err%d] pInst->gsUserdataBufAddr physical alloc failed \n", pInst->vdec_instance_index, -1 );
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS( "[VDEC-%d] pInst->gsUserdataBufAddr[PA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsUserdataBufAddr[PA], pInst->gsUserdataBufSize );
		pInst->gsUserdataBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsUserdataBufAddr[PA], pInst->gsUserdataBufSize, pInst );
		if( pInst->gsUserdataBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC-%d:Err%d] pInst->gsUserdataBufAddr virtual alloc failed \n", pInst->vdec_instance_index, -1 );
			return -(VPU_NOT_ENOUGH_MEM);
		}
		//memset( (void*)pInst->gsUserdataBufAddr[VA], 0 , gsUserdataBufSize);
		DSTATUS("[VDEC-%d] pInst->gsUserdataBufAddr[VA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsUserdataBufAddr[VA], pInst->gsUserdataBufSize );
	}
	
	//------------------------------------------------------------
	// [x] code buffer, work buffer and parameter buffer for VPU 
	//------------------------------------------------------------
	pInst->gsBitWorkBufSize = WORK_CODE_PARA_BUF_SIZE;
	pInst->gsBitWorkBufSize = ALIGNED_BUFF(pInst->gsBitWorkBufSize, ALIGN_LEN);
	if(pInst->vdec_instance_index == 0)
	{
		pInst->gsBitWorkBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitWorkBufAddr[K_VA], pInst->gsBitWorkBufSize, BUFFER_WORK, pInst );
	}
	else
	{
		pInst->gsBitWorkBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitWorkBufAddr[K_VA], pInst->gsBitWorkBufSize, BUFFER_WORK, pInst );
	}		
	if( pInst->gsBitWorkBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bit_work_buf_addr[PA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS("[VDEC-%d] bit_work_buf_addr[PA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitWorkBufAddr[PA], pInst->gsBitWorkBufSize );
	pInst->gsBitWorkBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsBitWorkBufAddr[PA], pInst->gsBitWorkBufSize, pInst );
	if( pInst->gsBitWorkBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] bit_work_buf_addr[VA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS("[VDEC-%d] bit_work_buf_addr[VA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitWorkBufAddr[VA], pInst->gsBitWorkBufSize );

	psVDecInit->m_BitWorkAddr[PA] = pInst->gsBitWorkBufAddr[PA];
	psVDecInit->m_BitWorkAddr[VA] = pInst->gsBitWorkBufAddr[K_VA];
	if( psVDecInit->m_bEnableVideoCache == 0 ){
		DSTATUS("[VDEC-%d] Cache OFF\n", pInst->vdec_instance_index);
	}
	else{
		DSTATUS("[VDEC-%d] Cache ON\n", pInst->vdec_instance_index);
	}

	psVDecInit->m_bCbCrInterleaveMode = psVDecInit->m_bCbCrInterleaveMode;
	if( psVDecInit->m_bCbCrInterleaveMode == 0 ){
		DSTATUS("[VDEC-%d] CbCrInterleaveMode OFF\n", pInst->vdec_instance_index);
	}
	else{
		DSTATUS("[VDEC-%d] CbCrInterleaveMode ON\n", pInst->vdec_instance_index);
	}

	if( psVDecInit->m_uiDecOptFlags&M4V_DEBLK_ENABLE ){
		DSTATUS( TC_YELLOW"[VDEC] MPEG-4 Deblocking ON\n"STYLE_RESET );
	}
	if( psVDecInit->m_uiDecOptFlags&M4V_GMC_FRAME_SKIP ){
		DSTATUS( TC_YELLOW"[VDEC] MPEG-4 GMC Frame Skip\n"STYLE_RESET );
	}

	return 0;
}

int
vpu_dec_seq_header( int iSize , dec_initial_info_t * psInitialInfo, int iIsThumbnail, _vdec_ *pVdec )
{
	int ret = 0;
	_vdec_ * pInst = pVdec;
	DSTATUS("[VDEC-%d]vpu_dec_seq_header in :: size(%d), JpegOnly(%d), format(%d)", pInst->vdec_instance_index, iSize ,pInst->gsVpuDecUserInfo.m_bJpegOnly, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat);
	LOGE("[VDEC-%d]vpu_dec_seq_header in :: size(%d), JpegOnly(%d), format(%d)", pInst->vdec_instance_index, iSize ,pInst->gsVpuDecUserInfo.m_bJpegOnly, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat);
	pInst->gsVpuDecSeqHeader_Info.stream_size = iSize;
	ret = vdec_cmd_process(V_DEC_SEQ_HEADER, &pInst->gsVpuDecSeqHeader_Info, pInst);
	if( ret != RETCODE_SUCCESS )
	{
	#ifndef HAVE_ANDROID_OS
		psInitialInfo->m_iReportErrorReason = pInst->gsVpuDecInitialInfo.m_iReportErrorReason;
	#endif
		DPRINTF( "[VDEC-%d] VPU_DEC_SEQ_HEADER failed Error code is 0x%x. ErrorReason is %d", pInst->vdec_instance_index, ret, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iReportErrorReason);
        if(ret == RETCODE_CODEC_SPECOUT)
            DPRINTF("NOT SUPPORTED CODEC. VPU SPEC OUT!!");     // This is a very common error. Notice the detailed reason to users.
		return -ret;
	}
	
#ifndef HAVE_ANDROID_OS
	if(psInitialInfo != NULL)
		cdk_memcpy(psInitialInfo, &pInst->gsVpuDecInitialInfo, sizeof(dec_initial_info_t));
#endif

	print_dec_initial_info( &pInst->gsVpuDecInit_Info.gsVpuDecInit, &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo );

#ifdef SET_FRAMEBUFFER_INTO_MAX
	if(pInst->bMaxfb_Mode)
	{
		pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth = 1920;
		pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight = 1080;
		DSTATUS("[VDEC-%d]Set seq framebuffer into 1080p", pInst->vdec_instance_index);
	}
#endif

	//------------------------------------------------------------
	// [x] slice buffer for VPU
	//------------------------------------------------------------
	if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_AVC || pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MVC )
	{
		pInst->gsSliceBufSize = SLICE_SAVE_SIZE;
		pInst->gsSliceBufSize = ALIGNED_BUFF( pInst->gsSliceBufSize, ALIGN_LEN );
		pInst->gsSliceBufAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsSliceBufSize, 0, pInst );
		if( pInst->gsSliceBufAddr == 0 ) 
		{
			DPRINTF( "[VDEC-%d] slice_buf_addr malloc() failed \n", pInst->vdec_instance_index);
			return -(VPU_NOT_ENOUGH_MEM);
		}
		DSTATUS("[VDEC-%d] slice_buf_addr = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsSliceBufAddr, pInst->gsSliceBufSize );

		pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_AvcSliceSaveBufferAddr  = pInst->gsSliceBufAddr;
		pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iAvcSliceSaveBufferSize = pInst->gsSliceBufSize;
	}		
	else
	{
		pInst->gsSliceBufSize = 0;
		pInst->gsSliceBufAddr = 0;
	}

#if defined(TCC_VPU_C7_INCLUDE)
    if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_VP8 )
    {
        pInst->gsMbSaveSize = 17*4*((pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth * pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight)>>8);
        pInst->gsMbSaveSize = ALIGNED_BUFF( pInst->gsMbSaveSize, 4*1024 );
        pInst->gsMbSaveAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsMbSaveSize, 0, pInst );
        if( pInst->gsMbSaveAddr == 0 )
        {
            DPRINTF( "[VDEC-%d] MbSaveAddr malloc() failed \n", pInst->vdec_instance_index);
            return -(VPU_NOT_ENOUGH_MEM);
        }
        DSTATUS("[VDEC-%d] MbSaveAddr = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsMbSaveAddr, pInst->gsMbSaveSize );

        pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_Vp8MbDataSaveBufferAddr = pInst->gsMbSaveAddr;
        pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iVp8MbDataSaveBufferSize = pInst->gsMbSaveSize;
    }
    else
    {
        pInst->gsMbSaveSize = 0;
        pInst->gsMbSaveAddr = 0;
    }
#endif

	//------------------------------------------------------------
	// [x] frame buffer for each VPU decoder
	//------------------------------------------------------------
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;
	DSTATUS( "[VDEC-%d] FrameBufDelay %d\n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iFrameBufDelay );
	DSTATUS( "[VDEC-%d] MinFrameBufferCount %d\n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount );
	ALOGD( "[VDEC-%d] MinFrameBufferCount %d\n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount );
#ifdef HAVE_ANDROID_OS
#ifdef NEED_SPECIFIC_PROCESS_FOR_MJPEG
	if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG)
	{
		int max_count;

	#if defined(SUPPORT_MANAGE_MJPEG_BUFFER)
		if(!pInst->gsVpuDecUserInfo.m_bJpegOnly)
		{
			if(!iIsThumbnail)
				pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + pInst->gsAdditionalFrameCount;
		}
		else
		{
			pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount = 1;
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;// + 2;
		}
	#else
		//del for mjpeg multi-frame buffer
		//pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;// + 2;
		pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + 2;
	#endif

	#ifdef SUPPORT_MJPEG_SCALING	
		if(pInst->gsVpuDecUserInfo.m_bJpegOnly)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio = pInst->gsVpuDecUserInfo.jpg_ScaleRatio;
		else
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio = 0;

		DSTATUS("[VDEC-%d] jpeg ratio = %d", pInst->vdec_instance_index, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio);

		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio == 0)
			max_count = cdk_sys_remain_memory_size(pInst) / pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
		else
			max_count = cdk_sys_remain_memory_size(pInst) / pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio];
	#else
		max_count = cdk_sys_remain_memory_size(pInst) / pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
	#endif

		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > max_count)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = max_count;
		
		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;
		
		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount)
		{
			LOGE( "[VDEC-%d] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d]", pInst->vdec_instance_index, max_count, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount);
			return -(VPU_NOT_ENOUGH_MEM);
		}
	}		
	else
#endif
	{
		int max_count;

		if(!iIsThumbnail)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + pInst->gsAdditionalFrameCount;
		max_count = cdk_sys_remain_memory_size(pInst) / pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;

		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > max_count)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = max_count;
		
		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;

		if(iIsThumbnail)				
		{
			if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount))
			{
				LOGE( "[VDEC-%d] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d], min_size = %d", pInst->vdec_instance_index, max_count, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
				return -(VPU_NOT_ENOUGH_MEM);
			}
		}
		else
		{
			if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount+pInst->gsAdditionalFrameCount))
			{
				LOGE( "[VDEC-%d] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d], min_size = %d", pInst->vdec_instance_index, max_count, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + pInst->gsAdditionalFrameCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
				return -(VPU_NOT_ENOUGH_MEM);
			}
		}
	}
#else	
	pInst->gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecInitialInfo.m_iMinFrameBufferCount;
#endif

#ifdef SUPPORT_MJPEG_SCALING	
	if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio != 0 && pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG){
		pInst->gsFrameBufSize = pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount * pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio];
		LOGD( "[VDEC-%d] FrameBufferCount %d [min %d], min_size = %d \n", pInst->vdec_instance_index, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio]);
	}
	else
#endif
	{
		pInst->gsFrameBufSize = pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount * pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
		LOGD( "[VDEC-%d] FrameBufferCount %d [min %d], min_size = %d \n", pInst->vdec_instance_index, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
	}

	pInst->gsFrameBufSize = ALIGNED_BUFF( pInst->gsFrameBufSize, ALIGN_LEN );
	pInst->gsFrameBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsFrameBufAddr[K_VA], pInst->gsFrameBufSize, 0, pInst );
	if( pInst->gsFrameBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] frame_buf_addr[PA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}	

#ifdef SUPPORT_MJPEG_SCALING	
	if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio != 0 && pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG){
		DSTATUS( "[VDEC-%d] MinFrameBufferSize %d bytes \n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio] );
	}
	else
#endif
	{
		DSTATUS( "[VDEC-%d] MinFrameBufferSize %d bytes \n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize );
	}
	DSTATUS( "[VDEC-%d] frame_buf_addr[PA] = 0x%x, 0x%x , index = %d \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsFrameBufAddr[PA], pInst->gsFrameBufSize, pInst->vdec_instance_index );
	pInst->gsFrameBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsFrameBufAddr[PA], pInst->gsFrameBufSize, pInst );
	if( pInst->gsFrameBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] frame_buf_addr[VA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS("[VDEC-%d] frame_buf_addr[VA] = 0x%x, frame_buf_addr[K_VA] = 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsFrameBufAddr[VA], pInst->gsFrameBufAddr[K_VA] );
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_FrameBufferStartAddr[PA] = pInst->gsFrameBufAddr[PA];
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_FrameBufferStartAddr[VA] = pInst->gsFrameBufAddr[K_VA];

#if 0//def VPU_PERFORMANCE_UP
	{
		 unsigned int regAddr = ((unsigned int)gsRegisterBase + 0x10000); //0xB0910000

		 //VCACHE_CTRL
		 *(volatile unsigned int *)(regAddr+0x00)	= (1<<0);			  //CACHEON

		 //VCACHE_REG
		 *(volatile unsigned int *)(regAddr+0x04)	= (3<<0);			//WR0|RD0
		 *(volatile unsigned int *)(regAddr+0x024)	= gsFrameBufAddr[PA];//VIDEO_PHY_ADDR;							//VCACHE_R0MIN
		 *(volatile unsigned int *)(regAddr+0x028)	= VIDEO_PHY_ADDR+ VIDEO_MEM_SIZE;	  //VCACHE_R0MAX
		 *(volatile unsigned int *)(regAddr+0x02C)	= 0; //VCACHE_R1MIN
		 *(volatile unsigned int *)(regAddr+0x030)	= 0; //VCACHE_R1MAX
		 *(volatile unsigned int *)(regAddr+0x034)	= 0; //VCACHE_R2MIN
		 *(volatile unsigned int *)(regAddr+0x038)	= 0; //VCACHE_R2MAX
		 *(volatile unsigned int *)(regAddr+0x03C)	= 0; //VCACHE_R3MIN
		 *(volatile unsigned int *)(regAddr+0x040)	= 0; //VCACHE_R3MAX
   }
#endif

	ret = vdec_cmd_process(V_DEC_REG_FRAME_BUFFER, &pInst->gsVpuDecBuffer_Info, pInst);

	if( ret != RETCODE_SUCCESS )
	{
		DPRINTF( "[VDEC-%d] DEC_REG_FRAME_BUFFER failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
		return -ret;
	}
	DSTATUS("[VDEC-%d] TCC_VPU_DEC VPU_DEC_REG_FRAME_BUFFER OK!\n", pInst->vdec_instance_index);
	return ret;
}

int
vdec_vpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void * pParam3 )
{
	int ret = 0;
	_vdec_ *pInst = (_vdec_ *)pParam3;
	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

#ifdef DEBUG_TIME_LOG
	clock_t start, end;
	start = clock();
#endif

	if( iOpCode == VDEC_INIT )
	{
		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;

#ifdef HAVE_ANDROID_OS
		vdec_user_info_t* p_input_user_param = (vdec_user_info_t*)pParam2;

		pInst->gsVpuDecUserInfo.bitrate_mbps = p_input_user_param->bitrate_mbps;
		pInst->gsVpuDecUserInfo.frame_rate   = p_input_user_param->frame_rate;
		pInst->gsVpuDecUserInfo.m_bJpegOnly  = p_input_user_param->m_bJpegOnly;	
		pInst->gsVpuDecUserInfo.jpg_ScaleRatio  = p_input_user_param->jpg_ScaleRatio;	
		pInst->gsVpuDecUserInfo.jpg_length  = p_input_user_param->jpg_length;
		
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, p_input_user_param->bitrate_mbps, p_input_user_param->frame_rate, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight, pInst ) < 0)
			return -VPU_ENV_INIT_ERROR;	
#endif

#if defined(VPU_CLK_CONTROL)
		vpu_clock_init();
#endif

#ifdef HAVE_ANDROID_OS
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_RegBaseVirtualAddr = (unsigned int)NULL;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat		= p_input_param->m_iBitstreamFormat;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData		= p_input_param->m_bEnableUserData;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableVideoCache	= p_input_param->m_bEnableVideoCache;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_Memcpy				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_Memset				= (void (*) ( void*, int, unsigned int ))memset;
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_Interrupt			= (int	(*) ( void ))NULL;	
#else
		pInst->gsVpuDecInit.m_RegBaseVirtualAddr	= p_input_param->m_RegBaseAddr;
		pInst->gsVpuDecInit.m_iBitstreamFormat		= p_input_param->m_iBitstreamFormat;
		pInst->gsVpuDecInit.m_BitstreamBufAddr[PA] = p_input_param->m_BitstreamBufAddr[PA];
		pInst->gsVpuDecInit.m_BitstreamBufAddr[VA] = p_input_param->m_BitstreamBufAddr[VA];
		pInst->gsVpuDecInit.m_iBitstreamBufSize	= p_input_param->m_iBitstreamBufSize;
		pInst->gsVpuDecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		pInst->gsVpuDecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		pInst->gsVpuDecInit.m_bEnableUserData		= p_input_param->m_bEnableUserData;
		pInst->gsVpuDecInit.m_bEnableVideoCache	= p_input_param->m_bEnableVideoCache;
		pInst->gsVpuDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;

		pInst->gsVpuDecInit.m_uiDecOptFlags = 0;
		if( p_input_param->m_iBitstreamFormat == STD_MPEG4 )
		{
			if( p_input_param->m_bM4vDeblk )
				pInst->gsVpuDecInit.m_uiDecOptFlags |= M4V_DEBLK_ENABLE;
		}
		if( p_input_param->m_uiDecOptFlags )
		{
			if( p_input_param->m_uiDecOptFlags & 0x2 )
			{
				pInst->gsVpuDecInit.m_uiDecOptFlags |= M4V_GMC_FRAME_SKIP;
			}
			if( p_input_param->m_uiDecOptFlags >> 16 )
			{
				pInst->gsVpuDecInit.m_uiDecOptFlags |= ( p_input_param->m_uiDecOptFlags & 0xFFFF0000 );
			}
		}

		if( p_input_param->m_uiMaxResolution == 2 )
		{
			pInst->gsVpuDecInit.m_iMaxResolution = RESOLUTION_625_SD;
		}
		else if( p_input_param->m_uiMaxResolution == 1 )
		{
			pInst->gsVpuDecInit.m_iMaxResolution = RESOLUTION_720P_HD;
		}
		else
		{
			pInst->gsVpuDecInit.m_iMaxResolution = RESOLUTION_1080_HD;
		}

		pInst->gsVpuDecInit.m_Memcpy				= p_input_param->m_pfMemcpy;
		pInst->gsVpuDecInit.m_Memset				= p_input_param->m_pfMemset;
		pInst->gsVpuDecInit.m_Interrupt			= p_input_param->m_pfInterrupt;
#endif

#ifdef VPU_PERFORMANCE_UP
		//gsVpuDecInit.m_bEnableVideoCache	= 1;
	#if defined(TCC_93XX_INCLUDE)
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC93XX;		// use secAXI SRAM0 128K
	#elif defined(TCC_88XX_INCLUDE)
		if((pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight * pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth) > (1280*720))
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC88XX;	// use secAXI SRAM0 80K
		else
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_DISABLE;

		if(pInst->gsVpuDecUserInfo.m_bJpegOnly)
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC88XX;	// use secAXI SRAM0 80K
	#endif
#else
	#if defined(TCC_VPU_C5_INCLUDE)
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_DISABLE;
	#endif
#endif

		pInst->extFunction = p_input_user_param->extFunction;
#ifdef SET_FRAMEBUFFER_INTO_MAX
		pInst->bMaxfb_Mode = p_input_user_param->bMaxfbMode;
		if(pInst->bMaxfb_Mode) 
		{
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags |= (1<<16);
			DSTATUS("[VDEC-%d]Set framebuffer into 1080p", pInst->vdec_instance_index);
		}
#endif

		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable		= p_input_param->m_bFilePlayEnable;
		pInst->gsbHasSeqHeader = 0;//p_input_param->m_bHasSeqHeader; 

		ret = vpu_dec_ready( &pInst->gsVpuDecInit_Info.gsVpuDecInit, pInst );
		if( ret != RETCODE_SUCCESS )
		{
			return ret;
		}

		DSTATUS("[VDEC-%d]workbuff 0x%x/0x%x, Reg: 0x%x, format : %d, Stream(0x%x/0x%x, %d), Res: %d x %d", pInst->vdec_instance_index, 
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitWorkAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitWorkAddr[VA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_RegBaseVirtualAddr,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[VA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight);
		DSTATUS("[VDEC-%d]optFlag 0x%x, avcBuff: 0x%x- %d, Userdata(%d), VCache: %d, Inter: %d, PlayEn: %d, MaxRes: %d", pInst->vdec_instance_index, 
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags, (unsigned int)pInst->gsVpuDecInit_Info.gsVpuDecInit.m_pSpsPpsSaveBuffer, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iSpsPpsSaveBufferSize,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableVideoCache, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode,
					pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iMaxResolution);

	
		if( (pInst->extFunction & EXT_FUNC_NO_BUFFER_DELAY) != 0x0 )
		{
			ALOGI("[VDEC_K] : No BufferDelay Mode....");
			pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags |= (1<<2);
		}
		DSTATUS("[VDEC-%d]Format : %d, Stream(0x%x, %d)", pInst->vdec_instance_index, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize);
		ret = vdec_cmd_process(V_DEC_INIT, &pInst->gsVpuDecInit_Info, pInst);
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_INIT failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}

		if( (pInst->extFunction & (EXT_FUNC_LOCK_FOR_SINK | EXT_FUNC_LOCK_FOR_SOURCE)) == 0x0 )
		{
			pInst->gsVpuDecVersion.pszVersion = (char*)(pInst->gsBitstreamBufAddr[K_VA] + (pInst->gsBitstreamBufSize - 100));
			pInst->gsVpuDecVersion.pszBuildData = (char*)(pInst->gsBitstreamBufAddr[K_VA] + (pInst->gsBitstreamBufSize - 50));

#if defined(TCC_VPU_C7_INCLUDE)
			ret = vdec_cmd_process(V_GET_VPU_VERSION, &pInst->gsVpuDecVersion, pInst);
			if( ret != RETCODE_SUCCESS )
			{
				//If this operation returns fail, it doesn't mean that there's a problem in vpu
				//so do not return error to host.
				DPRINTF( "[VDEC-%d] V_GET_VPU_VERSION failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			}
			else
			{
				int vpu_closed[VPU_MAX];

				if( 0 <= ioctl(pInst->vpu_mgr_fd, VPU_CHECK_CODEC_STATUS, &vpu_closed) ) {
					LOGI("Multi-instance status : %d/%d/%d/%d/%d", vpu_closed[VPU_DEC], vpu_closed[VPU_DEC_EXT], vpu_closed[VPU_ENC], vpu_closed[VPU_DEC_EXT2], vpu_closed[VPU_DEC_EXT3]);
				}
				LOGI( "[VDEC-%d] V_GET_VPU_VERSION OK. Version is %s, and it's built at %s \n", pInst->vdec_instance_index,
					(char*)(pInst->gsBitstreamBufAddr[VA] + (pInst->gsBitstreamBufSize - 100)),
					(char*)(pInst->gsBitstreamBufAddr[VA] + (pInst->gsBitstreamBufSize - 50)));
			}
#endif
		}
		
		pInst->vdec_codec_opened = 1;

#ifdef VPU_REGISTER_DUMP
		bFirst_frame = 1;
		filewrite_memory("data/after_init.bin", gsRegisterBase, 0x200);
#endif
		LOGI( "[VDEC-%d] VPU_DEC_INIT OK( has seq = %d) \n", pInst->vdec_instance_index, pInst->gsbHasSeqHeader );

	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;
		unsigned int iIsThumbnail = p_input_param->m_iIsThumbnail;

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable)
		{
			if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
			     && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			}
			else
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];		
				memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], seq_stream_size);
				change_input_stream((unsigned char *)pInst->gsBitstreamBufAddr[VA], &seq_stream_size, iOpCode, pInst);
			}

			save_input_stream("/sdcard/vpu_inSeq.bin", seq_stream_size, pInst);

			if( (pInst->extFunction & (EXT_FUNC_LOCK_FOR_SINK | EXT_FUNC_LOCK_FOR_SOURCE)) == 0x0 )
			{
				unsigned char* ps = (unsigned char*)pInst->gsBitstreamBufAddr[VA];
				DSTATUS( "[VDEC-%d Seq %d] \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"
									"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"
									"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n", 
									pInst->vdec_instance_index, seq_stream_size,
									ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
									ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
									ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
									ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63],
									ps[64], ps[65], ps[66], ps[67], ps[68], ps[69], ps[70], ps[71], ps[72], ps[73], ps[74], ps[75], ps[76], ps[77], ps[78], ps[79]);
			}
		}
		else
		{
			seq_stream_size = 1;
		}
#else
		int seq_stream_size = (int)pParam1;
#endif

		DSTATUS( "[VDEC-%d] VDEC_DEC_SEQ_HEADER start  :: len = %d / %d \n", pInst->vdec_instance_index, seq_stream_size, p_input_param->m_iInpLen);
		ret = vpu_dec_seq_header(seq_stream_size, NULL/*pParam2*/, iIsThumbnail, pInst);
		if( ret != RETCODE_SUCCESS )
		{
			return ret;
		}
#ifdef HAVE_ANDROID_OS
		pInst->gsbHasSeqHeader = 1;
		p_output_param->m_pInitialInfo = &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
#endif
		//check the maximum/minimum video resolution limitation
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG )
		{
#ifdef HAVE_ANDROID_OS
			vdec_info_t * pVdecInfo = (vdec_info_t *)&pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
#else		
			vdec_info_t * pVdecInfo = (vdec_info_t *)pParam2;
#endif
			int max_width, max_height;
			int min_width, min_height;
		 
			max_width  	= ((AVAILABLE_MAX_WIDTH+15)&0xFFF0);
			max_height 	= ((AVAILABLE_MAX_HEIGHT+15)&0xFFF0);
			min_width 	= AVAILABLE_MIN_WIDTH;
			min_height 	= AVAILABLE_MIN_HEIGHT;
			
			if(    (pVdecInfo->m_iPicWidth > max_width)
				|| ((pVdecInfo->m_iPicWidth * pVdecInfo->m_iPicHeight) > AVAILABLE_MAX_REGION)
				|| (pVdecInfo->m_iPicWidth < min_width)
				|| (pVdecInfo->m_iPicHeight < min_height) )
			{
				ret = 0 - RETCODE_INVALID_STRIDE;
				DPRINTF( "[VDEC-%d] VDEC_DEC_SEQ_HEADER - don't support the resolution %dx%d  \n", pInst->vdec_instance_index, 
									pVdecInfo->m_iPicWidth, pVdecInfo->m_iPicHeight);
				return ret;
			}
		}
		else //MJPEG
		{
#ifdef HAVE_ANDROID_OS
			vdec_info_t * pVdecInfo = (vdec_info_t *)&pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
#else		
			vdec_info_t * pVdecInfo = (vdec_info_t *)pParam2;
#endif
			
			if(  (pVdecInfo->m_iPicWidth > 8192)		\
				|| (pVdecInfo->m_iPicHeight > 8192) )
			{
				ret = 0 - RETCODE_INVALID_STRIDE;
				DSTATUS( "[VDEC-%d] VDEC_DEC_SEQ_HEADER - don't support the resolution %dx%d  \n", pInst->vdec_instance_index, 
									pVdecInfo->m_iPicWidth, pVdecInfo->m_iPicHeight);
				return ret;
			}

			if( pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat != 0 )
			{
				DSTATUS("[VDEC-%d] VPU OutFormat is YUV422SEQ0 ", pInst->vdec_instance_index);
			}

	#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG	) && !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
			if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && !pInst->gsVpuDecUserInfo.m_bJpegOnly)
			{
				unsigned int buf_idx;
				
				pInst->decoded_buf_curIdx = 0;
				pInst->decoded_buf_size = pVdecInfo->m_iPicWidth*pVdecInfo->m_iPicHeight*2; //for YUV422
				pInst->decoded_buf_size = ALIGNED_BUFF(pInst->decoded_buf_size, ALIGN_LEN);
				
				for(buf_idx =0; buf_idx < pInst->gsAdditionalFrameCount + 1; buf_idx++)
				{
					pInst->decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->decoded_buf_size, 0, pInst );
					if( pInst->decoded_phyAddr[buf_idx] == 0 ) 
					{
						DPRINTF( "[VDEC-%d,Err:%d] vdec_vpu pInst->decoded_virtAddr[PA] alloc failed \n", pInst->vdec_instance_index, ret );
						pInst->decoded_buf_maxcnt = buf_idx;
						break;
					}	
					pInst->decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->decoded_phyAddr[buf_idx], pInst->decoded_buf_size, pInst ); 
					if( pInst->decoded_virtAddr[buf_idx] == 0 ) 
					{
						DPRINTF( "[VDEC-%d,Err:%d] vdec_vpu pInst->decoded_virtAddr[VA] alloc failed \n", pInst->vdec_instance_index, ret );
						pInst->decoded_buf_maxcnt = buf_idx;
						break;
					}
	
					pInst->decoded_buf_maxcnt = pInst->gsAdditionalFrameCount + 1;
					DSTATUS("[VDEC-%d] OUT-Buffer %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", pInst->vdec_instance_index, buf_idx, pInst->decoded_phyAddr[buf_idx], pInst->decoded_virtAddr[buf_idx],	pInst->decoded_buf_size);
				}			
			}
	#endif

		}
		
		LOGI( "[VDEC-%d] VDEC_DEC_SEQ_HEADER - Success mem_free = 0x%x \n", pInst->vdec_instance_index, cdk_sys_remain_memory_size(pInst) );
		DSTATUS( "=======================================================\n\n" );		
	}
	else if( iOpCode == VDEC_DECODE )
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		#ifdef PRINT_VPU_INPUT_STREAM
		{
			int kkk;
			unsigned char* p_input = p_input_param->m_pInp[VA];
			int input_size = p_input_param->m_iInpLen;
			printf("FS = %7d :", input_size);
			for( kkk = 0; kkk < PRINT_BYTES; kkk++ )
				printf("%02X ", p_input[kkk] );
			printf("\n");
		}
		#endif


#ifdef HAVE_ANDROID_OS
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;

		if( (pInst->extFunction & EXT_FUNC_LOCK_FOR_SINK) != 0x0 )
		{
			//LOGI("Usable Input Addr : 0x%x", p_input_param->m_pInp[PA]);
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		}
		else if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable )
		{
			if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
				 && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			}
			else
			{
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
				pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
				memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize);
				change_input_stream((unsigned char *)pInst->gsBitstreamBufAddr[VA], (&pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize), iOpCode, pInst);
			}
			save_input_stream("data/vpu_inDec.bin", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, pInst);
		}
		else
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = 1;
		}
#else
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = p_input_param->m_iInpLen;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
#endif

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{		
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = pInst->gsUserdataBufAddr[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = pInst->gsUserdataBufAddr[K_VA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = pInst->gsUserdataBufSize;
		}
		
//		gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = (codec_addr_t)p_input_param->m_UserDataAddr[PA];
//		gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = (codec_addr_t)p_input_param->m_UserDataAddr[VA];
//		gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = p_input_param->m_iUserDataBufferSize;
		
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = p_input_param->m_iSkipFrameMode;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = p_input_param->m_iFrameSearchEnable;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;
		if( pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode > 0 || pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable > 0 )
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = p_input_param->m_iSkipFrameNum;
		}

	#ifdef DEBUG_VPU_K_DEC_INPUT
		PrintVPUHexData((unsigned char*)pInst->gsBitstreamBufAddr[VA], 
		                pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, "VPU");
	#endif

		// Start decoding a frame.
		ret = vdec_cmd_process(V_DEC_DECODE, &pInst->gsVpuDecInOut_Info, pInst);
		pInst->total_frm++;
//		if(gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus != VPU_DEC_OUTPUT_SUCCESS)
//			LOGD("systemtime:: ## decoded frame but no-output");
//		else
//			LOGD("systemtime:: ## decoded frame");

#if 0//def VPU_PERFORMANCE_UP
		{
	         unsigned int regAddr = ((unsigned int)gsRegisterBase + 0x10000); //0xB0910000

	         //VCACHE_CTRL
	         *(volatile unsigned int *)(regAddr+0x08) =      (1<<0);           //DRAIN
	   }
#endif
	
//		if( ret == VPU_DEC_FINISH )
//			return ERR_END_OF_FILE;
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_DECODE failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );

			return -ret;
		}

		if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iPicType == 0){
			DSTATUS( "[VDEC-%d] I-Frame (%d)", pInst->vdec_instance_index, pInst->total_frm);
		}

#ifdef NEED_SPECIFIC_PROCESS_FOR_MJPEG
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && 
#if defined(SUPPORT_MANAGE_MJPEG_BUFFER	)
			pInst->gsVpuDecUserInfo.m_bJpegOnly && 
#endif			
			pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL &&
			pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;
		}
#endif

#ifdef HAVE_ANDROID_OS
		memcpy((void*)p_output_param, (void*)&pInst->gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
	#ifdef NEED_SPECIFIC_PROCESS_FOR_MJPEG		
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && pInst->gsVpuDecUserInfo.m_bJpegOnly && pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4)
		{
			unsigned int frame_size = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth*pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight;

			if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode)
			{
				memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst), 0x80, frame_size/2);
			}
			else
			{
				memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst), 0x80, frame_size/4);
				memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA, pInst), 0x80, frame_size/4);
			}
		}

		//if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && !pInst->gsVpuDecUserInfo.m_bJpegOnly)
		if(0)	//del for mjpeg multi-frame buffer
		{
			unsigned int frame_size = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth*pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight;
			int divide_val = 4; 					
				
			if( pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 1 ) //YUV422
				divide_val = 2;
			
		#if !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
			p_output_param->m_pDispOut[PA][0] = (unsigned char*)pInst->decoded_phyAddr[pInst->decoded_buf_curIdx];
			p_output_param->m_pDispOut[PA][1] = (unsigned char*)p_output_param->m_pDispOut[PA][0] + frame_size;
			p_output_param->m_pDispOut[PA][2] = (unsigned char*)p_output_param->m_pDispOut[PA][1] + frame_size/divide_val;	
			p_output_param->m_pDispOut[VA][0] = (unsigned char*)pInst->decoded_virtAddr[pInst->decoded_buf_curIdx];
			p_output_param->m_pDispOut[VA][1] = (unsigned char*)p_output_param->m_pDispOut[VA][0] + frame_size;
			p_output_param->m_pDispOut[VA][2] = (unsigned char*)p_output_param->m_pDispOut[VA][1] + frame_size/divide_val;	

			if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode)
			{
				memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][0], vpu_getFrameBufVirtAddr(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][0], K_VA, pInst), frame_size);

				if( pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4) //YUV400
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][1], 0x80, frame_size/divide_val);
				else
					memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][1], vpu_getFrameBufVirtAddr(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][1], K_VA, pInst), frame_size/divide_val);
			}
			else	
			{
				//copy occured
				memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][0], vpu_getFrameBufVirtAddr(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][0], K_VA, pInst), frame_size);
				if( pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4) //YUV400
				{
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][1], 0x80, frame_size/divide_val);
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][2], 0x80, frame_size/divide_val);
				}
				else
				{
					//copy two memorys
					memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][1], vpu_getFrameBufVirtAddr(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][1], K_VA, pInst), frame_size/divide_val);
					memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][2], vpu_getFrameBufVirtAddr(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][2], K_VA, pInst), frame_size/divide_val);
				}
			}

			pInst->decoded_buf_curIdx++;
			if(pInst->decoded_buf_curIdx >= pInst->decoded_buf_maxcnt)	//decoded_buf_maxcnt = 8
				pInst->decoded_buf_curIdx = 0;
		#else
			if( pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4)
			{
				if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode)
				{
					memset((unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst), 0x80, frame_size/2);
				}
				else
				{
					memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst), 0x80, frame_size/divide_val);
					memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA, pInst), 0x80, frame_size/divide_val);
				}
			}
		#endif
		}	
	#endif
#else
		cdk_memcpy( p_output_param, &(pInst->gsVpuDecInOut_Info.gsVpuDecOutput), sizeof(dec_output_t ) );
#endif
		p_output_param->m_pInitialInfo = &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;

#if !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
		//if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG )
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG || !pInst->gsVpuDecUserInfo.m_bJpegOnly)
#endif
		{
			p_output_param->m_pDispOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][0], K_VA, pInst);
			p_output_param->m_pDispOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst);
			p_output_param->m_pDispOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA, pInst);
		}

		p_output_param->m_pCurrOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][0], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][1], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][2], K_VA, pInst);

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{	
			unsigned int addr_gap = 0;

			addr_gap = pInst->gsUserdataBufAddr[K_VA] - pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_UserDataAddress[VA];
			p_output_param->m_DecOutInfo.m_UserDataAddress[VA] = pInst->gsUserdataBufAddr[VA] + addr_gap;
		}

		if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS){
//			LOGE("Displayed addr 0x%x", p_output_param->m_pDispOut[PA][0]);
			save_decoded_frame((unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[PA][0], PA, pInst),
								(unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[PA][1], PA, pInst),
								(unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[PA][2], PA, pInst),
								pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight, pInst);
		}

		save_input_stream("data/vpu_outDec.bin", p_input_param->m_iInpLen, pInst);
#ifdef VPU_REGISTER_DUMP
		filewrite_memory("data/after_decode.bin", gsRegisterBase, 0x200);
		bFirst_frame = 0;
#endif
		DISPLAY_BUFFER("[VDEC-%d] Display idx = %d", pInst->vdec_instance_index, pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDispOutIdx);
	}
	else if( iOpCode == VDEC_GET_RING_BUFFER_STATUS )
	{
		vdec_ring_buffer_out_t* p_out_param = (vdec_ring_buffer_out_t*)pParam2;

		ret = vdec_cmd_process(V_GET_RING_BUFFER_STATUS, &pInst->gsVpuDecBufStatus, pInst); // get the available space in the ring buffer
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] GET_RING_BUFFER_STATUS failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
	#if defined(TCC_VPU_C5_INCLUDE)
		p_out_param->m_ulAvailableSpaceInRingBuffer = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_iAvailableSpaceInRingBuffer;
	#else
		p_out_param->m_ulAvailableSpaceInRingBuffer = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ulAvailableSpaceInRingBuffer;
		p_out_param->m_ptrReadAddr_PA = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrReadAddr_PA;
		p_out_param->m_ptrWriteAddr_PA = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrWriteAddr_PA;
//		ALOGE("[VDEC] [AVAIL: %8d] [RP: 0x%08X / WP: 0x%08X]"
//			  , pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ulAvailableSpaceInRingBuffer
//			  , pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrReadAddr_PA
//			  , pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrWriteAddr_PA
//			  );
	#endif
	}
	else if( iOpCode == VDEC_FILL_RING_BUFFER )
	{
		vdec_ring_buffer_set_t* p_set_param = (vdec_ring_buffer_set_t*)pParam1;

		memcpy((void*)pInst->gsIntermediateBufAddr[VA],(void*)p_set_param->m_pbyBuffer, p_set_param->m_ulBufferSize);
		pInst->gsVpuDecBufFill.gsVpuDecRingFeed.m_iOnePacketBufferSize = p_set_param->m_ulBufferSize;
		pInst->gsVpuDecBufFill.gsVpuDecRingFeed.m_OnePacketBufferAddr = pInst->gsIntermediateBufAddr[K_VA];

		ret = vdec_cmd_process(V_FILL_RING_BUFFER_AUTO, &pInst->gsVpuDecBufFill, pInst);  // fille the Ring Buffer 

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] FILL_RING_BUFFER_AUTO failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_GET_INTERMEDIATE_BUF_INFO )
	{
		*(unsigned int*)pParam1 = pInst->gsIntermediateBufAddr[VA];
		*(unsigned int*)pParam2 = pInst->gsIntermediateBufSize;
		return 0;
	}
	else if( iOpCode == VDEC_UPDATE_WRITE_BUFFER_PTR )
	{
		pInst->gsVpuDecUpdateWP.iCopiedSize = (int)pParam1;
		pInst->gsVpuDecUpdateWP.iFlushBuf = (int)pParam2;

		ret = vdec_cmd_process(V_DEC_UPDATE_RINGBUF_WP, &pInst->gsVpuDecUpdateWP,  pInst);  // fille the Ring Buffer 

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VDEC_UPDATE_WRITE_BUFFER_PTR failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_BUF_FLAG_CLEAR )
	{
		int idx_display = *(int*)pParam1;
		CLEAR_BUFFER("[VDEC-%d] ************* cleared idx = %d", pInst->vdec_instance_index, idx_display);
		ret = vdec_cmd_process(V_DEC_BUF_FLAG_CLEAR, &idx_display, pInst);
		
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_BUF_FLAG_CLEAR failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_DEC_FLUSH_OUTPUT)
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = 0;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = 0;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;

		ret = vdec_cmd_process(V_DEC_FLUSH_OUTPUT, &pInst->gsVpuDecInOut_Info, pInst);

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VDEC_DEC_FLUSH_OUTPUT failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}

		memcpy((void*)p_output_param, (void*)&pInst->gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
		p_output_param->m_pInitialInfo = &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;

#if !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
		//if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG) 
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG || !pInst->gsVpuDecUserInfo.m_bJpegOnly)
#endif
		{
			p_output_param->m_pDispOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][0], K_VA, pInst);
			p_output_param->m_pDispOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst);
			p_output_param->m_pDispOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA, pInst);
		}

		p_output_param->m_pCurrOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][0], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][1], K_VA, pInst);
		p_output_param->m_pCurrOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][2], K_VA, pInst);

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{
			unsigned int addr_gap = 0;

			addr_gap = pInst->gsUserdataBufAddr[K_VA] - pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_UserDataAddress[VA];
			p_output_param->m_DecOutInfo.m_UserDataAddress[VA] = pInst->gsUserdataBufAddr[VA] + addr_gap;
		}
	}
	else if( iOpCode == VDEC_SW_RESET)
	{
		ret = vdec_cmd_process(V_DEC_SWRESET, NULL, pInst);

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] V_DEC_SWRESET failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_CLOSE )
	{
		if(!pInst->vdec_codec_opened && !pInst->vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		if(pInst->vdec_codec_opened)
		{		
			ret = vdec_cmd_process(V_DEC_CLOSE, &pInst->gsVpuDecInOut_Info, pInst);
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC-%d] VPU_DEC_CLOSE failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
				ret = -ret;
			}
			
			pInst->vdec_codec_opened = 0;
		}

		if(!pInst->vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;
				
		if( pInst->gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize)  >= 0)
			{
				pInst->gsBitstreamBufAddr[VA] = 0;
			}
		}
		
		if( pInst->gsUserdataBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsUserdataBufAddr[VA], pInst->gsUserdataBufSize )  >= 0)
			{
				pInst->gsUserdataBufAddr[VA] = 0;
			}
			
		}

		if( pInst->gsBitWorkBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsBitWorkBufAddr[VA], pInst->gsBitWorkBufSize )  >= 0)
			{
				pInst->gsBitWorkBufAddr[VA] = 0;
			}
			
		}

		if( pInst->gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsFrameBufAddr[VA], pInst->gsFrameBufSize )  >= 0)
			{
				pInst->gsFrameBufAddr[VA] = 0;
			}
		}
		
#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG	) && !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
		//if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && !pInst->gsVpuDecUserInfo.m_bJpegOnly)
		if(0)	//del for mjpeg multi-frame buffer
		{
			unsigned int buf_idx = 0;
			
			for(buf_idx =0; buf_idx < pInst->decoded_buf_maxcnt; buf_idx++)
			{
				if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->decoded_virtAddr[buf_idx], pInst->decoded_buf_size )  >= 0)
				{
					pInst->decoded_virtAddr[buf_idx] = 0;
				}
			}			
		}
#endif

		
#if defined(VPU_CLK_CONTROL)
		vpu_clock_deinit();
#endif
#ifdef HAVE_ANDROID_OS
		vpu_env_close(pInst);
#endif
	}
	else
	{
		DPRINTF( "[VDEC-%d] Invalid Operation!!\n", pInst->vdec_instance_index );
		return -ret;
	}

#if 0
#ifdef DEBUG_TIME_LOG
	end = clock();

	if( iOpCode == VDEC_INIT ){
		LOGD("[VDEC-%d] VDEC_INIT_TIME %d ms", pInst->vdec_instance_index, (end-start)*1000/CLOCKS_PER_SEC);
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER){
		LOGD("[VDEC-%d] VDEC_SEQ_TIME %d ms", pInst->vdec_instance_index, (end-start)*1000/CLOCKS_PER_SEC);
	}
	else if( iOpCode == VDEC_DECODE )
	{
		dec_time[time_cnt] = (end-start)*1000/CLOCKS_PER_SEC;
		total_dec_time += dec_time[time_cnt];
		if(time_cnt != 0 && time_cnt % 29 == 0)
		{
			LOGD("VDEC_DEC_TIME %.1f ms: %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d",
				total_dec_time/(float)pInst->total_frm, dec_time[0], dec_time[1], dec_time[2], dec_time[3], dec_time[4], dec_time[5], dec_time[6], dec_time[7], dec_time[8], dec_time[9], 
				dec_time[10], dec_time[11], dec_time[12], dec_time[13], dec_time[14], dec_time[15], dec_time[16], dec_time[17], dec_time[18], dec_time[19], 
				dec_time[20], dec_time[21], dec_time[22], dec_time[23], dec_time[24], dec_time[25], dec_time[26], dec_time[27], dec_time[28], dec_time[29]);
			time_cnt = 0;
		}
		else{
			time_cnt++;
		}
	}
#endif
#endif
	return ret;
}

#ifdef INCLUDE_WMV78_DEC
int
WMV78_dec_seq_header(_vdec_ *pVdec)
{
	int i, ret = 0;
	_vdec_ * pInst = pVdec;
	memset( &pInst->gsWMV78DecInitialInfo, 0, sizeof(pInst->gsWMV78DecInitialInfo) );

	pInst->gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropBottom = 0;
	pInst->gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropLeft = 0;
	pInst->gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropRight = 0;
	pInst->gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropTop = 0;
	pInst->gsWMV78DecInitialInfo.m_iMinFrameBufferCount = 1;
	pInst->gsWMV78DecInitialInfo.m_iPicWidth = pInst->gsWMV78DecInit.m_iWidth;
	pInst->gsWMV78DecInitialInfo.m_iPicHeight = pInst->gsWMV78DecInit.m_iHeight;
	pInst->gsWMV78DecInitialInfo.m_iAspectRateInfo = 0;
	pInst->gsWMV78DecInitialInfo.m_iInterlace = 0;
	print_dec_initial_info( &pInst->gsVpuDecInit, &pInst->gsWMV78DecInitialInfo );

	return ret;
}

static void* pExtDLLModule = NULL;
#define EXT_LIB_NAME "libTCC.wmv78dec.so"

int WMV78_LoadLibrary(_vdec_ *pVdec)
{
	pVdec->pExtDLLModule = dlopen(EXT_LIB_NAME, RTLD_LAZY | RTLD_GLOBAL);
    if( pVdec->pExtDLLModule == NULL ) {
        ALOGE("Load library '%s' failed: %s", EXT_LIB_NAME, dlerror());
        return -1;
    } else {
		ALOGI("Library '%s' Loaded", EXT_LIB_NAME);
    }

	pVdec->gExtDecFunc = dlsym(pVdec->pExtDLLModule, "Video_Proc");
    if( pVdec->gExtDecFunc == NULL ) {
		ALOGE("pVdec->gExtDecFunc Error");
        return -1;
    }

	return 0;
}

int WMV78_CloseLibrary(_vdec_ *pVdec)
{
	if( pVdec->pExtDLLModule != NULL)
		dlclose(pVdec->pExtDLLModule);

	return 0;
}

int
vdec_WMV78( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void* pParam3 )
{
	int ret = 0;
	_vdec_ *pInst = (_vdec_ *)pParam3;
	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	if( iOpCode == VDEC_INIT )
	{
		unsigned int ChromaSize;
		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;
		vdec_callback_func_t* pf_callback = (vdec_callback_func_t*)pParam2;

		if( 0 > WMV78_LoadLibrary(pInst))
			return -VPU_ENV_INIT_ERROR;

#ifdef HAVE_ANDROID_OS
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, 0, 0, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight, pInst ) < 0) // to operate Max-clock for s/w codec!!
			return -VPU_ENV_INIT_ERROR;
#endif

		pInst->gsWMV78FrameSize = ( (p_input_param->m_iPicWidth+15)&0xfffffff0 ) * ( (p_input_param->m_iPicHeight+15)&0xfffffff0 );
		ChromaSize = pInst->gsWMV78FrameSize>>2;
		pInst->gsWMV78NCFrameSize = pInst->gsWMV78FrameSize*1.5;
		pInst->gsWMV78NCFrameSize = ALIGNED_BUFF( pInst->gsWMV78NCFrameSize, ALIGN_LEN );

#ifdef HAVE_ANDROID_OS
		pInst->gsWMV78CurYFrameAddress = (unsigned int)((unsigned char*)TCC_malloc(pInst->gsWMV78NCFrameSize));
		pInst->gsWMV78CurUFrameAddress  = (unsigned int)((unsigned char*)pInst->gsWMV78CurYFrameAddress + pInst->gsWMV78FrameSize);
		pInst->gsWMV78CurVFrameAddress  = (unsigned int)((unsigned char*)pInst->gsWMV78CurUFrameAddress + ChromaSize);

		pInst->gsWMV78Ref0YFrameAddress = (unsigned int)((unsigned char*)TCC_malloc(pInst->gsWMV78NCFrameSize));
		pInst->gsWMV78Ref0UFrameAddress = (unsigned int)((unsigned char*)pInst->gsWMV78Ref0YFrameAddress + pInst->gsWMV78FrameSize);
		pInst->gsWMV78Ref0VFrameAddress = (unsigned int)((unsigned char*)pInst->gsWMV78Ref0UFrameAddress + ChromaSize);
#else
		pInst->gsWMV78DecodedFrameAddress_NC[PA]  = (codec_addr_t)cdk_sys_malloc_physical_addr(NULL, pInst->gsWMV78NCFrameSize, 0,pInst );
		pInst->gsWMV78DecodedFrameAddress_NC[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( &NULL, pInst->gsWMV78DecodedFrameAddress_NC[PA], pInst->gsWMV78NCFrameSize, pInst );
		if( pInst->gsWMV78DecodedFrameAddress_NC[VA] == 0 ) 
		{
			DPRINTF( "[VDEC_WMV78:Err%d] pInst->gsWMV78DecodedFrameAddress_NC virtual alloc failed \n", -1 );
			return -1;
		}

		pInst->gsWMV78CurYFrameAddress  = (unsigned int)cdk_malloc(sizeof(unsigned char)*pInst->gsWMV78FrameSize*1.5);
		pInst->gsWMV78CurUFrameAddress  = (unsigned int)((unsigned char*)pInst->gsWMV78CurYFrameAddress + pInst->gsWMV78FrameSize);
		pInst->gsWMV78CurVFrameAddress  = (unsigned int)((unsigned char*)pInst->gsWMV78CurUFrameAddress + ChromaSize);
		pInst->gsWMV78Ref0YFrameAddress = (unsigned int)cdk_malloc(sizeof(unsigned char)*pInst->gsWMV78FrameSize*1.5);
		pInst->gsWMV78Ref0UFrameAddress = (unsigned int)((unsigned char*)pInst->gsWMV78Ref0YFrameAddress + pInst->gsWMV78FrameSize);
		pInst->gsWMV78Ref0VFrameAddress = (unsigned int)((unsigned char*)pInst->gsWMV78Ref0UFrameAddress + ChromaSize);
#endif
		pInst->gsWMV78DecInit.m_pExtraData			= p_input_param->m_pExtraData;
		pInst->gsWMV78DecInit.m_iExtraDataLen		= p_input_param->m_iExtraDataLen;
		pInst->gsWMV78DecInit.m_iWidth				= (p_input_param->m_iPicWidth+15)&0xfffffff0;
		pInst->gsWMV78DecInit.m_iHeight			= p_input_param->m_iPicHeight;
		pInst->gsWMV78DecInit.m_iFourCC			= p_input_param->m_iFourCC;
		pInst->gsWMV78DecInit.m_pHeapAddress		= (unsigned char*)TCC_malloc(sizeof(unsigned char)*200*1024);
		pInst->gsWMV78DecInit.m_pHuff_tbl_address	= (unsigned char*)WMV78_Huff_table;

		pInst->gsWMV78DecInit.m_pCurFrameAddress  = (tYUV420Frame_WMV*)TCC_malloc(sizeof(tYUV420Frame_WMV));
		pInst->gsWMV78DecInit.m_pRef0FrameAddress = (tYUV420Frame_WMV*)TCC_malloc(sizeof(tYUV420Frame_WMV));
		pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucYPlane		= (unsigned char*)pInst->gsWMV78CurYFrameAddress;
		pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucUPlane		= (unsigned char*)pInst->gsWMV78CurUFrameAddress;
		pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucVPlane		= (unsigned char*)pInst->gsWMV78CurVFrameAddress;
		pInst->gsWMV78DecInit.m_pRef0FrameAddress->m_pucYPlane		= (unsigned char*)pInst->gsWMV78Ref0YFrameAddress;
		pInst->gsWMV78DecInit.m_pRef0FrameAddress->m_pucUPlane		= (unsigned char*)pInst->gsWMV78Ref0UFrameAddress;
		pInst->gsWMV78DecInit.m_pRef0FrameAddress->m_pucVPlane		= (unsigned char*)pInst->gsWMV78Ref0VFrameAddress;
		pInst->gsWMV78DecOutput.m_pDecodedData = (tYUV420Frame_WMV*)TCC_malloc(sizeof(tYUV420Frame_WMV));

#ifdef HAVE_ANDROID_OS
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMalloc			= (void*  (*) ( unsigned int ))TCC_malloc;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pFree				= (void   (*) ( void* ))TCC_free;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMemcpy			= (void*  (*) ( void*, const void*, unsigned int ))memcpy;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMemset			= (void  (*) ( void*, int, unsigned int ))memset;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pRealloc			= (void*  (*) ( void*, unsigned int ))TCC_realloc;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMemmove			= (void*  (*) ( void*, const void*, unsigned int ))memmove;

		{
			unsigned int buf_idx;
			
			pInst->decoded_buf_curIdx = 0;
			pInst->decoded_buf_size = pInst->gsWMV78FrameSize* 1.5;
			pInst->decoded_buf_size = ALIGNED_BUFF(pInst->decoded_buf_size, ALIGN_LEN);
			
			for(buf_idx =0; buf_idx < pInst->gsAdditionalFrameCount + 1; buf_idx++)
			{
				pInst->decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->decoded_buf_size, 0, pInst );				
				if( pInst->decoded_phyAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC,Err:%d] vdec_vpu pInst->decoded_virtAddr[PA] alloc failed \n", ret );
					pInst->decoded_buf_maxcnt = buf_idx;
					break;
				}	
				pInst->decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->decoded_phyAddr[buf_idx], pInst->decoded_buf_size, pInst );	
				if( pInst->decoded_virtAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC,Err:%d] vdec_vpu pInst->decoded_virtAddr[VA] alloc failed \n", ret );
					pInst->decoded_buf_maxcnt = buf_idx;
					break;
				}

				pInst->decoded_buf_maxcnt = pInst->gsAdditionalFrameCount + 1;
				DSTATUS("OUT-Buffer %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, pInst->decoded_phyAddr[buf_idx], pInst->decoded_virtAddr[buf_idx],	pInst->decoded_buf_size);
			}			
		}		
#else
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMalloc			= pf_callback->m_pfMalloc;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pFree				= pf_callback->m_pfFree;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMemcpy			= pf_callback->m_pfMemcpy;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMemset			= pf_callback->m_pfMemset;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pRealloc			= pf_callback->m_pfRealloc;
		pInst->gsWMV78DecInit.m_sCallbackFunc.m_pMemmove			= pf_callback->m_pfMemmove;
#endif

		pInst->gsVpuDecInit.m_iBitstreamFormat	= p_input_param->m_iBitstreamFormat;

		pInst->gsFirstFrame = 1;

		DSTATUS( "[VDEC_WMV78] WMV78_DEC_INIT Enter \n" );
		ret = pInst->gExtDecFunc( VDEC_INIT, &pInst->gsWMV78DecHandle, &pInst->gsWMV78DecInit, NULL );
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC_WMV78] WMV78_DEC_INIT failed Error code is 0x%x \n", ret );
			return -ret;
		}
		pInst->gsIsINITdone = 1;
		pInst->vdec_codec_opened = 1;
		DSTATUS( "[VDEC_WMV78] WMV78_DEC_INIT OK \n" );
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{
#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		if( pInst->gsFirstFrame )
		{
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER start \n" );
			p_output_param->m_pInitialInfo = &pInst->gsWMV78DecInitialInfo;
			ret = WMV78_dec_seq_header(pInst);
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC_WMV78] vpu_dec_seq_header failed Error code is 0x%x \n", ret );
				return ret;
			}
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER - Success \n" );
			pInst->gsFirstFrame = 0;
		}
#endif
		return RETCODE_SUCCESS;
	}
	else if( iOpCode == VDEC_DECODE )
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		#ifdef PRINT_VPU_INPUT_STREAM
		{
			int kkk;
			unsigned char* p_input = p_input_param->m_pInp[VA];
			int input_size = p_input_param->m_iInpLen;
			printf("FS = %7d :", input_size);
			for( kkk = 0; kkk < PRINT_BYTES; kkk++ )
				printf("%02X ", p_input[kkk] );
			printf("\n");
		}
		#endif

		if( pInst->gsFirstFrame )
		{
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER start \n" );
			p_output_param->m_pInitialInfo = &pInst->gsWMV78DecInitialInfo;
			ret = WMV78_dec_seq_header(pInst);
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC_WMV78] vpu_dec_seq_header failed Error code is 0x%x \n", ret );
				return ret;
			}
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER - Success \n" );
			pInst->gsFirstFrame = 0;
		}

		pInst->gsWMV78DecInput.m_pPacketBuff = (unsigned char*)p_input_param->m_pInp[VA];
		pInst->gsWMV78DecInput.m_iPacketBuffSize = p_input_param->m_iInpLen;

#ifdef HAVE_ANDROID_OS
	#if 0 // temporarily no-use!!
		pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucYPlane		= (unsigned char*)decoded_virtAddr[decoded_buf_curIdx];
		pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucUPlane		= (unsigned char*)pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucYPlane + pInst->gsWMV78FrameSize;
		pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucVPlane		= (unsigned char*)pInst->gsWMV78DecInit.m_pCurFrameAddress->m_pucUPlane + pInst->gsWMV78FrameSize/4;		
	#endif
#endif

		// Start decoding a frame.
		ret = pInst->gExtDecFunc( VDEC_DECODE, &pInst->gsWMV78DecHandle, &pInst->gsWMV78DecInput, &pInst->gsWMV78DecOutput );
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC_WMV78] VDEC_DECODE failed Error code is 0x%x \n", ret );
			return ret;
		}
		
#ifdef HAVE_ANDROID_OS
	#if 1 // temporarily use!!
		memcpy( (unsigned char *)pInst->decoded_virtAddr[pInst->decoded_buf_curIdx], pInst->gsWMV78DecOutput.m_pDecodedData->m_pucYPlane, pInst->gsWMV78FrameSize*1.5*sizeof(unsigned char) );
	#endif
		p_output_param->m_pCurrOut[0][0] = p_output_param->m_pDispOut[0][0] = (unsigned char*)pInst->decoded_phyAddr[pInst->decoded_buf_curIdx];
		p_output_param->m_pCurrOut[0][1] = p_output_param->m_pDispOut[0][1] = (unsigned char*)p_output_param->m_pDispOut[0][0] + pInst->gsWMV78FrameSize;
		p_output_param->m_pCurrOut[0][2] = p_output_param->m_pDispOut[0][2] = (unsigned char*)p_output_param->m_pDispOut[0][1] + pInst->gsWMV78FrameSize/4;	
		p_output_param->m_pCurrOut[1][0] = p_output_param->m_pDispOut[1][0] = (unsigned char*)pInst->decoded_virtAddr[pInst->decoded_buf_curIdx];
		p_output_param->m_pCurrOut[1][1] = p_output_param->m_pDispOut[1][1] = (unsigned char*)p_output_param->m_pDispOut[1][0] + pInst->gsWMV78FrameSize;
		p_output_param->m_pCurrOut[1][2] = p_output_param->m_pDispOut[1][2] = (unsigned char*)p_output_param->m_pDispOut[1][1] + pInst->gsWMV78FrameSize/4;	

		pInst->decoded_buf_curIdx++;
		if(pInst->decoded_buf_curIdx >= pInst->decoded_buf_maxcnt)
			pInst->decoded_buf_curIdx = 0;
#else
		cdk_memcpy( (unsigned char *)pInst->gsWMV78DecodedFrameAddress_NC[VA], pInst->gsWMV78DecOutput.m_pDecodedData->m_pucYPlane, pInst->gsWMV78FrameSize*1.5*sizeof(unsigned char) );

		p_output_param->m_pCurrOut[0][0] = p_output_param->m_pDispOut[0][0] = (unsigned char *)pInst->gsWMV78DecodedFrameAddress_NC[PA];
		p_output_param->m_pCurrOut[0][1] = p_output_param->m_pDispOut[0][1] = (unsigned char *)(pInst->gsWMV78DecodedFrameAddress_NC[PA]+pInst->gsWMV78FrameSize);
		p_output_param->m_pCurrOut[0][2] = p_output_param->m_pDispOut[0][2] = (unsigned char *)(pInst->gsWMV78DecodedFrameAddress_NC[PA]+pInst->gsWMV78FrameSize+pInst->gsWMV78FrameSize/4);
		p_output_param->m_pCurrOut[1][0] = p_output_param->m_pDispOut[1][0] = (unsigned char *)pInst->gsWMV78DecodedFrameAddress_NC[VA];
		p_output_param->m_pCurrOut[1][1] = p_output_param->m_pDispOut[1][1] = (unsigned char *)(pInst->gsWMV78DecodedFrameAddress_NC[VA]+pInst->gsWMV78FrameSize);
		p_output_param->m_pCurrOut[1][2] = p_output_param->m_pDispOut[1][2] = (unsigned char *)(pInst->gsWMV78DecodedFrameAddress_NC[VA]+pInst->gsWMV78FrameSize+pInst->gsWMV78FrameSize/4);
#endif
		p_output_param->m_DecOutInfo.m_iPicType          = pInst->gsWMV78DecOutput.m_iPictureType;
		p_output_param->m_DecOutInfo.m_iDecodedIdx       = 0;
		p_output_param->m_DecOutInfo.m_iDispOutIdx       = 0;
		p_output_param->m_DecOutInfo.m_iDecodingStatus   = 1;
		p_output_param->m_DecOutInfo.m_iOutputStatus     = 1;
		p_output_param->m_DecOutInfo.m_iInterlacedFrame  = 0;
		p_output_param->m_DecOutInfo.m_iPictureStructure = 0;

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC_WMV78] WMV78_DEC_DECODE failed Error code is 0x%x \n", ret );
			return -ret;
		}

		p_output_param->m_pInitialInfo = &pInst->gsWMV78DecInitialInfo;
	}
	else if( iOpCode == VDEC_BUF_FLAG_CLEAR )
	{
		return RETCODE_SUCCESS;
	}
	else if( iOpCode == VDEC_DEC_FLUSH_OUTPUT)
	{
		return RETCODE_SUCCESS;
	}
	else if( iOpCode == VDEC_CLOSE )
	{
		unsigned int buf_idx;

		if(!pInst->vdec_codec_opened)
			return -RETCODE_NOT_INITIALIZED;
			
		if ( pInst->gsIsINITdone )
		{
			ret = pInst->gExtDecFunc( VDEC_CLOSE, &pInst->gsWMV78DecHandle, NULL, NULL );
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC_WMV78] WMV78_DEC_CLOSE failed Error code is 0x%x \n", ret );
				ret = -ret;
			}
		}

		pInst->vdec_codec_opened = 0;
		pInst->gsIsINITdone = 0;

		if ( pInst->gsWMV78DecInit.m_pHeapAddress )
			TCC_free(pInst->gsWMV78DecInit.m_pHeapAddress);
		if ( pInst->gsWMV78DecOutput.m_pDecodedData )
			TCC_free(pInst->gsWMV78DecOutput.m_pDecodedData);
		if ( pInst->gsWMV78DecInit.m_pCurFrameAddress )
			TCC_free(pInst->gsWMV78DecInit.m_pCurFrameAddress);
		if ( pInst->gsWMV78DecInit.m_pRef0FrameAddress )
			TCC_free(pInst->gsWMV78DecInit.m_pRef0FrameAddress);

		pInst->gsWMV78DecInit.m_pHeapAddress = 0;
		pInst->gsWMV78DecOutput.m_pDecodedData = 0;
		pInst->gsWMV78DecInit.m_pCurFrameAddress = 0;
		pInst->gsWMV78DecInit.m_pRef0FrameAddress = 0;

		if( pInst->gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsFrameBufAddr[VA], pInst->gsFrameBufSize )  >= 0)
			{
				pInst->gsFrameBufAddr[VA] = 0;
			}
		}

#ifndef HAVE_ANDROID_OS
		if( pInst->gsWMV78DecodedFrameAddress_NC[PA] )
			cdk_sys_free_physical_addr( (void*)pInst->gsWMV78DecodedFrameAddress_NC[PA], pInst->gsWMV78NCFrameSize );
		if( pInst->gsWMV78DecodedFrameAddress_NC[VA] )
			cdk_sys_free_virtual_addr( &NULL, pInst->gsWMV78DecodedFrameAddress_NC[VA], pInst->gsWMV78NCFrameSize );
#endif
		if( pInst->gsWMV78CurYFrameAddress )
			TCC_free( (void*)pInst->gsWMV78CurYFrameAddress );
		if( pInst->gsWMV78Ref0YFrameAddress )
			TCC_free( (void*)pInst->gsWMV78Ref0YFrameAddress );	

		pInst->gsWMV78CurYFrameAddress = 0;
		pInst->gsWMV78Ref0YFrameAddress = 0;

		for(buf_idx =0; buf_idx < pInst->decoded_buf_maxcnt; buf_idx++)
		{
			if( pInst->decoded_virtAddr[buf_idx] )
			{
				cdk_sys_free_virtual_addr( NULL, (void*)pInst->decoded_virtAddr[buf_idx], pInst->decoded_buf_size );
				pInst->decoded_virtAddr[buf_idx]= 0;
				
				DSTATUS("OUT-Buffer Release %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, pInst->decoded_phyAddr[buf_idx], pInst->decoded_virtAddr[buf_idx],	pInst->decoded_buf_size);
			}
		}			

		WMV78_CloseLibrary(pInst);
#ifdef HAVE_ANDROID_OS
		vpu_env_close(pInst);
#endif
	}
	else
	{
		DPRINTF( "Invaild Operation!!\n" );
		return -ret;
	}

	return ret;
}
#endif

#ifdef INCLUDE_SORENSON263_DEC

cdk_result_t
sorensonH263_dec_seq_header( int iSize , dec_initial_info_t * psInitialInfo, _vdec_ *pVdec )
{
	int ret = 0;
	int width, height;
	h263dec_stats_t s263_dec_stat;
	h263dec_input_t s263_dec_input;
	_vdec_ * pInst = pVdec;
#ifdef HAVE_ANDROID_OS
	memset(&s263_dec_stat, 0, sizeof(h263dec_stats_t));
	memset(&s263_dec_input, 0, sizeof(h263dec_input_t));
#else
	cdk_memset(&s263_dec_stat, 0, sizeof(h263dec_stats_t));
	cdk_memset(&s263_dec_input, 0, sizeof(h263dec_input_t));
#endif

	s263_dec_input.m_pBitstreamStartAddr = (void *)pInst->gsS263DecInit.m_BitstreamBufAddr[VA];
	if( iSize == 0 )
	{
		iSize = pInst->gsS263DecInit.m_iBitstreamBufSize;
		if( iSize == 0 )
		{
			DPRINTF( "[VDEC] H263_DEC_SEQ_HEADER failed : BitstreamBufSize \n" );
			return CDK_ERR_INVALID_PARAM;
		}
	}
	s263_dec_input.m_iBitstreamSize = iSize;
#ifdef SORENSON263_DEC_OUTPUT_MEM_ALIGN16
	s263_dec_input.m_iDecOption = Th263_SETIMAGEALIGN16;
#else
	s263_dec_input.m_iDecOption = 0;
#endif

	ret = TCC_H263_DEC( H263_DEC_SEQ_HEADER, (void *)pInst->gsS263DecHandle, &s263_dec_input, &s263_dec_stat );
	if( ret == Th263_ERR_NOT_SUPPORT )
	{
		DPRINTF( "[VDEC] H263_DEC_SEQ_HEADER failed Error code is 0x%x (not support)\n", ret );
		return ret;
	}
	if( ret < RETCODE_SUCCESS )
	{
		DPRINTF( "[VDEC] H263_DEC_SEQ_HEADER failed Error code is 0x%x \n", ret );
		return ret;
	}

#ifdef HAVE_ANDROID_OS
  memcpy(&pInst->gsS263DecInitialStats, &s263_dec_stat, sizeof(h263dec_stats_t));
	memset(&pInst->pInst->gsS263DecInitialInfo, 0, sizeof(dec_initial_info_t));

#else
	cdk_memcpy(&pInst->gsS263DecInitialStats, &s263_dec_stat, sizeof(h263dec_stats_t));
	cdk_memset(&pInst->gsS263DecInitialInfo, 0, sizeof(dec_initial_info_t));

#endif
	

	pInst->gsS263DecInitialInfo.m_iMinFrameBufferCount = 1;
	pInst->gsS263DecInitialInfo.m_iMinFrameBufferSize = s263_dec_stat.m_iNeedFrameBufferSize;		// minimum frame buffer size

	width = s263_dec_stat.m_uInfo.m_sVol.m_iWidth;
	width = ( (width + 1 ) & (~0x1) );
	pInst->gsS263DecInitialInfo.m_iPicWidth = width; // {(PicX+1)/2} * 2  (this width  will be used while allocating decoder frame buffers. picWidth  is a multiple of 2)

	height = s263_dec_stat.m_uInfo.m_sVol.m_iHeight;
	height = ( (height+ 1 ) & (~0x1) );
	pInst->gsS263DecInitialInfo.m_iPicHeight = height; // {(PicY+1)/2} * 2  (this height will be used while allocating decoder frame buffers. picHeight is a multiple of 2)

	//  represents rectangular window information in a frame 
	pInst->gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropLeft = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Left;		
	pInst->gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropRight = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Right;
	pInst->gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropTop = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Top;
	pInst->gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropBottom = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Bottom;

	if( pInst->gsS263DecInit.m_iPicWidth != pInst->gsS263DecInitialInfo.m_iPicWidth ||
		pInst->gsS263DecInit.m_iPicHeight != pInst->gsS263DecInitialInfo.m_iPicHeight  )
	{
		pInst->gsS263DecInit.m_iPicWidth = pInst->gsS263DecInitialInfo.m_iPicWidth;
		pInst->gsS263DecInit.m_iPicHeight = pInst->gsS263DecInitialInfo.m_iPicHeight;
		DPRINTF( "[VDEC] H263_DEC_SEQ_HEADER : resolution changing %dx%d => %dx%d\n",
			pInst->gsS263DecInit.m_iPicWidth, pInst->gsS263DecInit.m_iPicHeight,
			pInst->gsS263DecInitialInfo.m_iPicWidth, pInst->gsS263DecInitialInfo.m_iPicHeight);
	}			

#if defined(SORENSON263_DEC_OUTPUT_MEM_ALIGN16) || defined(SORENSON263_DEC_OUTPUT_MEM_ALIGN16_STRIDE)
	if( 1 )
	{
		int	width_tmp = s263_dec_stat.m_uInfo.m_sVol.m_iWidth;
		if( width_tmp & 0x0F ) 
		{
			width = ( (width_tmp + 15 ) & (~15) ); //multiple of 16 for M2M scaler
		}		
	}
#endif
			
#ifdef HAVE_ANDROID_OS
	if(psInitialInfo != NULL)
		memcpy(psInitialInfo, &pInst->gsS263DecInitialInfo, sizeof(dec_initial_info_t));

#else
	if(psInitialInfo != NULL)
	#ifdef HAVE_ANDROID_OS
		memcpy(psInitialInfo, &pInst->gsS263DecInitialInfo, sizeof(dec_initial_info_t));
	#else
		cdk_memcpy(psInitialInfo, &pInst->gsS263DecInitialInfo, sizeof(dec_initial_info_t));
	#endif
#endif
	
	print_dec_initial_info( &pInst->gsS263DecInit, &pInst->gsS263DecInitialInfo );

	/* Display output buffer */

	pInst->gsFrameBufSize = (int)(width * height * 1.5);
	pInst->gsFrameBufSize = ALIGNED_BUFF( pInst->gsFrameBufSize, ALIGN_LEN );
	pInst->gsFrameBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsFrameBufSize, 0, pInst );
	if( pInst->gsFrameBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] pInst->gsFrameBufAddr[PA] malloc() failed \n");
		return -(VPU_NOT_ENOUGH_MEM);
	}	
	pInst->gsFrameBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsFrameBufAddr[PA], pInst->gsFrameBufSize, pInst );
	if( pInst->gsFrameBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] pInst->gsFrameBufAddr[VA] malloc() failed \n");
		return -(VPU_NOT_ENOUGH_MEM);
	}
#ifdef HAVE_ANDROID_OS
	memset( (void*)pInst->gsFrameBufAddr[VA], 0, pInst->gsFrameBufSize);
#else
	cdk_memset( pInst->gsFrameBufAddr[VA], 0, pInst->gsFrameBufSize);
#endif
	DSTATUS( "[VDEC] pInst->gsFrameBufAddr [PA] = 0x%x, [VA] = 0x%x, size = 0x%x \n",		\
												(codec_addr_t)pInst->gsFrameBufAddr[PA],	\
												(codec_addr_t)pInst->gsFrameBufAddr[VA],	\
												pInst->gsFrameBufSize );

	/* decoding instance buffer */

	pInst->gsSliceBufSize = s263_dec_stat.m_iNeedFrameBufferSize;
	
	#ifdef HAVE_ANDROID_OS
	pInst->gsSliceBufAddr = (codec_addr_t)vdec_malloc( pInst->gsSliceBufSize );
	#else
	pInst->gsSliceBufAddr = (codec_addr_t)cdk_malloc( pInst->gsSliceBufSize );
	#endif
	
	if( pInst->gsSliceBufAddr == 0 )
	{
		DPRINTF( "[VDEC] pInst->gsSliceBufAddr malloc() failed \n");
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS( "[VDEC] gsSliceBufAddr = 0x%x, 0x%x \n", (codec_addr_t)pInst->gsSliceBufAddr, pInst->gsSliceBufSize );

	ret = TCC_H263_DEC( H263_DEC_REG_FRAME_BUFFER, (void *)pInst->gsS263DecHandle, (void *)pInst->gsSliceBufAddr, (void *)NULL );
	if( ret < RETCODE_SUCCESS )
	{
		DPRINTF( "[VDEC] H263_DEC_REG_FRAME_BUFFER failed Error code is 0x%x \n", ret );
		return ret;
	}
	DSTATUS("[VDEC] TCC_H263_DEC H263_DEC_REG_FRAME_BUFFER OK!\n");
	DSTATUS( "===================================================================\n" );
	return ret;
}

#define SORENSON263_MIN_USEFUL_BYTES 1

cdk_result_t
vdec_sorensonH263dec( int iOpCode, cdk_handle_t* pHandle, void* pParam1, void* pParam2, void* pParam3)
{
	int ret = 0;
	_vdec_ *pInst  = (_vdec_ *) pParam3;
	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	if( iOpCode == VDEC_INIT )
	{
		int heap_size;
		h263dec_init_t s263_dec_init;

		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;

#ifdef HAVE_ANDROID_OS
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, 0, 0, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight, pInst ) < 0)
			return -1;
#endif

#ifdef HAVE_ANDROID_OS
		pInst->gsS263DecInit.m_iBitstreamFormat	= p_input_param->m_iBitstreamFormat;
		pInst->gsS263DecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		pInst->gsS263DecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		pInst->gsS263DecInit.m_Memcpy				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		pInst->gsS263DecInit.m_Memset				= (void (*) ( void*, int, unsigned int ))memset;
		vpu_sorensonH263dec_ready(&pInst->gsS263DecInit, pInst);
#else
		pInst->gsS263DecInit.m_iBitstreamFormat	= p_input_param->m_iBitstreamFormat;
		pInst->gsS263DecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		pInst->gsS263DecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		pInst->gsS263DecInit.m_Memcpy				= p_input_param->m_pfMemcpy;
		pInst->gsS263DecInit.m_Memset				= p_input_param->m_pfMemset;
		pInst->gsS263DecInit.m_BitstreamBufAddr[PA] = p_input_param->m_BitstreamBufAddr[PA];
		pInst->gsS263DecInit.m_BitstreamBufAddr[VA] = p_input_param->m_BitstreamBufAddr[VA];
		pInst->gsS263DecInit.m_iBitstreamBufSize = p_input_param->m_iBitstreamBufSize;
#endif
		pInst->gsbHasSeqHeader = 0;//p_input_param->m_bHasSeqHeader; 

		heap_size = H263_DEC_HEAP_SIZE;
		
		#ifdef HAVE_ANDROID_OS
		pInst->gsS263DecHeapAddr = (unsigned char *) vdec_malloc( sizeof(unsigned char)*heap_size );
		#else
		pInst->gsS263DecHeapAddr = (unsigned char *) cdk_malloc( sizeof(unsigned char)*heap_size );
		#endif
		
		if( pInst->gsS263DecHeapAddr == 0 ) 
		{
			DPRINTF( "[VDEC] pInst->gsS263DecHeapAddr malloc() failed \n");
			return -1;
		}

#ifdef HAVE_ANDROID_OS
		memset( &s263_dec_init, 0, sizeof(h263dec_init_t) );
#else
		cdk_memset( &s263_dec_init, 0, sizeof(h263dec_init_t) );
#endif

		s263_dec_init.m_iCodecType = Th263_SORENSON_H263;
		s263_dec_init.m_pHeapAddr = (char*)pInst->gsS263DecHeapAddr;
		s263_dec_init.m_iHeapSize = heap_size;
		s263_dec_init.m_Memcpy pInst->= pInst->gsS263DecInit.m_Memcpy;
		s263_dec_init.m_Memset = pInst->gsS263DecInit.m_Memset;

		ret =  TCC_H263_DEC( H263_DEC_INIT, NULL, (void *)&s263_dec_init, (void *)NULL );
		pInst->gsS263DecHandle = (codec_handle_t)s263_dec_init.m_pHandle;
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] H263_DEC_INIT failed Error code is 0x%x \n", ret );
			return -ret;
		}
		
		pInst->vdec_codec_opened = 1;		
		DSTATUS( "[VDEC] H263_DEC_INIT OK( has seq = %d) \n", pInst->gsbHasSeqHeader );
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
	
	#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = p_input_param->m_iInpLen;

		pInst->gsS263DecInit.m_BitstreamBufAddr[PA] = pInst->gsBitstreamBufAddr[PA];
		pInst->gsS263DecInit.m_BitstreamBufAddr[VA] = pInst->gsBitstreamBufAddr[VA];
		memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], p_input_param->m_iInpLen);

		save_input_stream("data/vpu_inSeq.bin", p_input_param->m_iInpLen, pInst);
	#else
		int seq_stream_size = (int)pParam1;
	#endif

		DSTATUS( "[VDEC] H263_DEC_SEQ_HEADER start seq_stream_size %d \n" ,seq_stream_size);

	#ifdef HAVE_ANDROID_OS
		ret = sorensonH263_dec_seq_header(seq_stream_size, NULL);
	#else
		ret = sorensonH263_dec_seq_header(seq_stream_size, pParam2);
	#endif

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] H263_DEC_SEQ_HEADER failed Error code is 0x%x \n", ret );
			return ret;
		}
		
		#ifdef HAVE_ANDROID_OS
			pInst->gsbHasSeqHeader = 1;
			p_output_param->m_pInitialInfo = &pInst->gsS263DecInitialInfo;
		#endif
		DSTATUS( "[VDEC] H263_DEC_SEQ_HEADER - Success \n" );
	}
	else if( iOpCode == VDEC_DECODE )
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		h263dec_input_t s263_dec_input;
		h263dec_stats_t s263_dec_stat;

		char *pBitstreamAddr;
		int useful_bytes, used_bytes;

		unsigned char* pDispOut[2][3];
		int dispOutSizeY;
		int stride;
		
		#ifdef HAVE_ANDROID_OS
		memset(&s263_dec_input, 0, sizeof(h263dec_input_t));
		#else
		cdk_memset(&s263_dec_input, 0, sizeof(h263dec_input_t));
		#endif


	#ifdef HAVE_ANDROID_OS
		//memcpy( (void*)gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], p_input_param->m_iInpLen);
		//pBitstreamAddr = (char *)gsBitstreamBufAddr[VA];
		save_input_stream("data/vpu_inDec.bin", p_input_param->m_iInpLen, pInst);

		pBitstreamAddr = (char *)p_input_param->m_pInp[VA];
	#else
		pBitstreamAddr = (char *)p_input_param->m_pInp[VA];
	#endif
		useful_bytes = p_input_param->m_iInpLen;

		/* Output frame structure */
		s263_dec_input.m_DispOout.m_pPlane[0]  = (void *)pInst->gsFrameBufAddr[VA];
		
		stride = pInst->gsS263DecInitialInfo.m_iPicWidth;
	#if defined(SORENSON263_DEC_OUTPUT_MEM_ALIGN16) || defined(SORENSON263_DEC_OUTPUT_MEM_ALIGN16_STRIDE)
		if( stride&0x0F )
		{
			stride = (stride+15)&(~15);	//multiple of 16 for M2M scaler
		}
	#endif

		s263_dec_input.m_DispOout.m_iStride[0] = stride;
		s263_dec_input.m_DispOout.m_iCsp = Th263_CSP_I420;

		dispOutSizeY = stride * pInst->gsS263DecInitialInfo.m_iPicHeight;
		pDispOut[PA][0] = (unsigned char* )pInst->gsFrameBufAddr[PA];
		pDispOut[VA][0] = (unsigned char* )pInst->gsFrameBufAddr[VA];
		pDispOut[PA][1] = (unsigned char* )pDispOut[PA][0] + dispOutSizeY;
		pDispOut[VA][1] = (unsigned char* )pDispOut[VA][0] + dispOutSizeY;
		pDispOut[PA][2] = (unsigned char* )pDispOut[PA][1] + dispOutSizeY/4;
		pDispOut[VA][2] = (unsigned char* )pDispOut[VA][1] + dispOutSizeY/4;

		do 
		{
			/* Input stream */
			s263_dec_input.m_pBitstreamStartAddr = pBitstreamAddr;
			s263_dec_input.m_iBitstreamSize      = useful_bytes;
		#ifdef SORENSON263_DEC_OUTPUT_MEM_ALIGN16
			s263_dec_input.m_iDecOption			 = Th263_SETIMAGEALIGN16;
		#else
			s263_dec_input.m_iDecOption = 0;
		#endif

			memset(&s263_dec_stat, 0, sizeof(h263dec_stats_t));
			s263_dec_stat.m_iInterMBinP_CheckFlag = 1;

			/* Decode frame */
			ret = TCC_H263_DEC( H263_DEC_DECODE, (void *)pInst->gsS263DecHandle, (void *)&s263_dec_input, (void *)&s263_dec_stat );
			if( ret == RETCODE_SUCCESS )
				break;

			used_bytes = s263_dec_stat.m_iUsedBytes;
			if(used_bytes <= 0) 
			{
				/* Update buffer pointers */
				pBitstreamAddr += 1;
				useful_bytes -= 1;
			}
			else 
			{
				/* Update buffer pointers */
				pBitstreamAddr += used_bytes;
				useful_bytes -= used_bytes;
			}
		} while (s263_dec_stat.m_iDataType <= 0 && useful_bytes > SORENSON263_MIN_USEFUL_BYTES);

		if( ret < RETCODE_SUCCESS )
		{
		#if 0
			DPRINTF( "[VDEC] TCC_H263_DEC failed Error code is 0x%x \n", ret );
			return ret;
		#else
			p_output_param->m_DecOutInfo.m_iDecodedIdx       = -1;
			p_output_param->m_DecOutInfo.m_iDispOutIdx       = -1;
			p_output_param->m_DecOutInfo.m_iDecodingStatus   = 0;
			p_output_param->m_DecOutInfo.m_iOutputStatus     = 0;
			p_output_param->m_DecOutInfo.m_iPicType = 3;
			return RETCODE_SUCCESS;
		#endif
		}

	#ifdef HAVE_ANDROID_OS
		memset( &p_output_param->m_DecOutInfo, 0, sizeof(dec_output_info_t));
	#else
		cdk_memset( &p_output_param->m_DecOutInfo, 0, sizeof(dec_output_info_t));
	#endif

		p_output_param->m_DecOutInfo.m_iDecodedIdx       = 0;
		p_output_param->m_DecOutInfo.m_iDispOutIdx       = 0;
		p_output_param->m_DecOutInfo.m_iDecodingStatus   = 1;
		p_output_param->m_DecOutInfo.m_iOutputStatus     = 1;
		{
			int iPicType = -1;
			if( s263_dec_stat.m_iDataType == Th263_TYPE_IVOP )		/* intra frame */
				iPicType = PIC_TYPE_I;
			else if( s263_dec_stat.m_iDataType == Th263_TYPE_PVOP ) /* predicted frame */
				iPicType = PIC_TYPE_P;
			p_output_param->m_DecOutInfo.m_iPicType      = iPicType;
		}

		p_output_param->m_pDispOut[0][0] =
		p_output_param->m_pCurrOut[0][0] = (unsigned char *) pDispOut[PA][0];
		p_output_param->m_pDispOut[0][1] =
		p_output_param->m_pCurrOut[0][1] = (unsigned char *) pDispOut[PA][1];
		p_output_param->m_pDispOut[0][2] =
		p_output_param->m_pCurrOut[0][2] = (unsigned char *) pDispOut[PA][2];

		p_output_param->m_pDispOut[1][0] =
		p_output_param->m_pCurrOut[1][0] = (unsigned char *) pDispOut[VA][0];
		p_output_param->m_pDispOut[1][1] =
		p_output_param->m_pCurrOut[1][1] = (unsigned char *) pDispOut[VA][1];
		p_output_param->m_pDispOut[1][2] =
		p_output_param->m_pCurrOut[1][2] = (unsigned char *) pDispOut[VA][2];

		/* ignore when display output buffer is only one frame.
		p_output_param->m_pPrevOut[0][0] = (unsigned char *) [PA];
		p_output_param->m_pPrevOut[0][1] = (unsigned char *) [PA];
		p_output_param->m_pPrevOut[0][2] = (unsigned char *) [PA];
		p_output_param->m_pPrevOut[1][0] = (unsigned char *) [VA];
		p_output_param->m_pPrevOut[1][1] = (unsigned char *) [VA];
		p_output_param->m_pPrevOut[1][2] = (unsigned char *) [VA];
		*/

		p_output_param->m_pInitialInfo = &pInst->gsS263DecInitialInfo;
	}
	else if( iOpCode == VDEC_BUF_FLAG_CLEAR )
	{
		return RETCODE_SUCCESS;
	}
	else if( iOpCode == VDEC_DEC_FLUSH_OUTPUT)
	{
		return RETCODE_SUCCESS;
	}
	else if( iOpCode == VDEC_CLOSE )
	{	
		if(!pInst->vdec_codec_opened)
			return -RETCODE_NOT_INITIALIZED;
		ret = TCC_H263_DEC( H263_DEC_CLOSE, (void *)pInst->gsS263DecHandle, (void *)NULL, (void *)NULL );
		pInst->gsS263DecHandle = 0;
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] H263_DEC_CLOSE failed Error code is 0x%x \n", ret );
			ret = -ret;
		}
		pInst->vdec_codec_opened = 0;

		if( pInst->gsS263DecHeapAddr )
		{
		#ifdef HAVE_ANDROID_OS
			vdec_free( (void*)pInst->gsS263DecHeapAddr );
		#else
			cdk_free( pInst->gsS263DecHeapAddr );
		#endif
			pInst->gsS263DecHeapAddr = 0;
		}

		if( pInst->gsSliceBufAddr )
		{
		#ifdef HAVE_ANDROID_OS
			vdec_free( (void*)pInst->gsSliceBufAddr );
		#else
			cdk_free( (void*)pInst->gsSliceBufAddr );
		#endif
			pInst->gsSliceBufAddr = 0;
		}

#if defined(TCC_VPU_C7_INCLUDE)
        if ( pInst->gsMbSaveAddr )
        {
        #ifdef HAVE_ANDROID_OS
            vdec_free( (void*)pInst->gsMbSaveAddr );
        #else
            cdk_free( (void*)pInst->gsMbSaveAddr );
        #endif
            pInst->gsMbSaveAddr = 0;
            pInst->gsMbSaveSize = 0;
        }
#endif
		if( pInst->gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize )  >= 0)
			{
				pInst->gsBitstreamBufAddr[VA] = 0;
			}
		}
		
		if( pInst->gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsFrameBufAddr[VA], pInst->gsFrameBufSize )  >= 0)
			{
				pInst->gsFrameBufAddr[VA] = 0;
			}
		}
				
		#ifdef HAVE_ANDROID_OS
			vpu_env_close(pInst);
		#endif

	}
	else
	{
		DPRINTF( "Invaild Operation!!\n" );
		return -ret;
	}

	return ret;
}
#endif


// JPU Hardware I/P related PARAMETERS (for MJPEG)
#define JPEG_BASE_REG   0x75300000 /*8925_0x75300000*/
//#define JPEG_BASE_REG 0x75000000

#ifndef JPEGDEC_DEVICE
    #define JPEGDEC_DEVICE      "/dev/jpegdec"
#endif
#ifndef TCC_CLK_DEV_NAME
    #define TCC_CLK_DEV_NAME        "/dev/clockctrl"
    #define TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE    112
    #define TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE     113
#endif

int jpu_clk_fd = -1;
static unsigned int  gsRegisterBase = 0, gsRegisterBaseSize = 0;
static unsigned char jpu_codec_opened = 0;

// jpu interrupt sample
int jpu_interrupt_sample(void)
{
    return 1;
}

#ifdef INCLUDE_JPU_MJPEG_DEC
int
vdec_mjpeg_jpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void* pParam3 )     // For TCC892XS / TCC893XS, use JPU as a MJPEG H/W decoder
{

#define ENABLE_MULTIPLE_JPU_FRAME_BUFFER

    codec_result_t ret = 0;
    static codec_handle_t dec_handle = 0;       // multiple-instance play is not supported by JPU
    static codec_addr_t sFrameBufferAddr[2], sBitstreamBufferAddr[2];
    static int sFrameBufferSize = 0, sBitstreamBufferSize;

    _vdec_ *pInst = (_vdec_ *)pParam3;

    if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
        return -RETCODE_NOT_INITIALIZED;

    if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !jpu_codec_opened)
        return -RETCODE_NOT_INITIALIZED;

    if( iOpCode == VDEC_INIT )
    {
        //! Set Decoder Init
        DPRINTF("VDEC_INIT. vdec_mjpeg_jpu");
        vdec_init_t* p_input_param = (vdec_init_t*)pParam1;
        vdec_user_info_t* p_input_user_param = (vdec_user_info_t*)pParam2;

        #ifdef HAVE_ANDROID_OS
        if(vpu_env_open(p_input_param->m_iBitstreamFormat, p_input_user_param->bitrate_mbps, p_input_user_param->frame_rate, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight, pInst ) < 0)
        {
            ALOGE("vpu_env_open error");
            return -VPU_ENV_INIT_ERROR;
        }
        #endif

        jpu_clk_fd = open(TCC_CLK_DEV_NAME, O_WRONLY);
        if(!jpu_clk_fd) {
            ALOGE("%s JPU open error!!", TCC_CLK_DEV_NAME);
        }

        int ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE, &ret_val);
        if(ret_val != 0)
            ALOGE("JPU Clock Enable Error. ret_val = %d", ret_val);

        pInst->gsBitstreamBufSize = 2*1024*1024;//0xA00000;//LARGE_STREAM_BUF_SIZE;
        pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsBitstreamBufSize, ALIGN_LEN );
        pInst->gsMaxBitstreamSize = pInst->gsBitstreamBufSize;

        if(p_input_param->m_pfInterrupt != NULL)
            pInst->gsVpuDecInit.m_Interrupt = p_input_param->m_pfInterrupt;

        pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->gsBitstreamBufSize, 0, pInst );
        if( pInst->gsBitstreamBufAddr[PA] == 0 )
        {
            ALOGE( "[VDEC-%d] bitstream_buf_addr[PA] malloc() failed \n", pInst->vdec_instance_index);
            return -(VPU_NOT_ENOUGH_MEM);
        }
        //ALOGD("[VDEC-%d] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize );

        pInst->gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize, pInst );
        if( pInst->gsBitstreamBufAddr[VA] == 0 )
        {
            ALOGE( "[VDEC-%d] bitstream_buf_addr[VA] malloc() failed \n", pInst->vdec_instance_index);
            return -(VPU_NOT_ENOUGH_MEM);
        }

        memset( (void*)pInst->gsBitstreamBufAddr[VA], 0x00 , pInst->gsBitstreamBufSize);
        //ALOGD("[VDEC-%d] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize );

        sBitstreamBufferAddr[PA] = pInst->gsBitstreamBufAddr[PA];
        sBitstreamBufferAddr[VA] = pInst->gsBitstreamBufAddr[VA];
        sBitstreamBufferSize = pInst->gsBitstreamBufSize;

        pInst->gsVpuDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;
        pInst->gsVpuDecInit.m_iBitstreamFormat  = p_input_param->m_iBitstreamFormat;
        pInst->gsFirstFrame = 1;
        pInst->gsIsINITdone = 1;
        pInst->vdec_codec_opened = 1;
        jpu_codec_opened = 1;
        //ALOGE("VDEC_INIT SUCCESS. vdec_mjpeg_jpu");
        return JPG_RET_SUCCESS;
    }
    else if( iOpCode == VDEC_DEC_SEQ_HEADER )
    {
        //DPRINTF("VDEC_DEC_SEQ_HEADER. vdec_mjpeg_jpu");

        vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
        vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
        int seq_stream_size = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;
        //DPRINTF("seq_stream_size = %d. p_input_param->m_iInpLen = %d. pInst->gsMaxBitstreamSize = %d", seq_stream_size, p_input_param->m_iInpLen, pInst->gsMaxBitstreamSize);
        unsigned int iIsThumbnail = p_input_param->m_iIsThumbnail;

        jpu_dec_init_t dec_init = {0,};
        jpu_dec_initial_info_t dec_initial_info = {0,};
        jpu_dec_buffer_t dec_buffer = { 0 };
        codec_addr_t reg_base_addr = 0;

        //------------------------------------------------------------
        //! [x] reg base for JPU
        //------------------------------------------------------------
        unsigned int reg_base_size = 8 * 1024 ;
        reg_base_addr = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, JPEG_BASE_REG, reg_base_size, pInst );
        memset(reg_base_addr,0x00,reg_base_size);

        if( reg_base_addr == 0 )
        {
            ALOGE( "reg_base_addr malloc() failed \r\n");
            return -1;
        }
        //DPRINTF("reg_base_addr[VA] = 0x%x, 0x%x \r\n", (codec_addr_t)reg_base_addr, reg_base_size );
        gsRegisterBase = dec_init.m_RegBaseVirtualAddr = reg_base_addr;
        gsRegisterBaseSize = reg_base_size;

        //------------------------------------------------------------
        //! [x] bitstream buffer for each JPU decoder
        if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
             && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
        {
            //DPRINTF("BITSTREAM BUFFER ADDRESS IS SAME AS BEFORE");
            dec_init.m_BitstreamBufAddr[PA] = (codec_addr_t)pInst->gsBitstreamBufAddr[PA];
            dec_init.m_BitstreamBufAddr[VA] = (codec_addr_t)pInst->gsBitstreamBufAddr[VA];
        }
        else
        {
            //DPRINTF("BITSTREAM BUFFER ADDRESS IS CHANGED");
            dec_init.m_BitstreamBufAddr[PA] = (codec_addr_t)pInst->gsBitstreamBufAddr[PA];
            dec_init.m_BitstreamBufAddr[VA] = (codec_addr_t)pInst->gsBitstreamBufAddr[VA];
            memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], seq_stream_size);
            change_input_stream((unsigned char *)pInst->gsBitstreamBufAddr[VA], &seq_stream_size, iOpCode, pInst);
        }

        dec_init.m_iBitstreamBufSize = pInst->gsBitstreamBufSize;
        //DPRINTF("dec_init.m_BitstreamBufAddr[PA] = 0x%x", dec_init.m_BitstreamBufAddr[PA]);
        //DPRINTF("dec_init.m_BitstreamBufAddr[VA] = 0x%x", dec_init.m_BitstreamBufAddr[VA]);
        //DPRINTF("dec_init.m_iBitstreamBufSize = %d", dec_init.m_iBitstreamBufSize);

        dec_init.m_bCbCrInterleaveMode = pInst->gsVpuDecInit.m_bCbCrInterleaveMode;
        dec_init.m_iRot_angle = 0;
        dec_init.m_iRot_enalbe = 0;
        dec_init.m_iMirror_enable = 0;
        dec_init.m_iMirrordir = 0;
        dec_init.m_Memcpy    = (void* (*) ( void*, const void*, unsigned int ))memcpy;
        dec_init.m_Memset    = (void  (*) ( void*, int, unsigned int ))memset;

        if (pInst->gsVpuDecInit.m_Interrupt != NULL)
        {
            dec_init.m_Interrupt = pInst->gsVpuDecInit.m_Interrupt;
        }
        else
        {
            dec_init.m_Interrupt = (int  (*) ( void ))jpu_interrupt_sample;
        }

        ret = TCC_JPU_DEC( JPU_DEC_INIT, &dec_handle, &dec_init, &dec_initial_info );

        if( ret != JPG_RET_SUCCESS )
        {
            DPRINTF( "[dec_test] JPU_DEC_INIT failed detail errod  code is 0x%x \r\n", dec_initial_info.m_error_reason);
            return ret;
        }

        //------------------------------------------------------------
        //! [x] frame buffer for each JPU decoder
        //------------------------------------------------------------
        codec_addr_t frame_buf_addr[2] = {0,};
        unsigned int frame_buf_size = 0;

        // scale factor set
        dec_buffer.m_iJPGScaleRatio = 0;        //!< JPEG Scaling Ratio
                                                //!< 0 ( Original Size )
                                                //!< 1 ( 1/2 Scaling Down )
                                                //!< 2 ( 1/4 Scaling Down )
                                                //!< 3 ( 1/8 Scaling Down )

        //different buf size as scale
        frame_buf_size = dec_initial_info.m_iJpg_MinFrameBufferSize[dec_buffer.m_iJPGScaleRatio];
        frame_buf_size = ALIGNED_BUFF( frame_buf_size, 4*1024 );
        //DPRINTF("frame_buf_size = %d", frame_buf_size);

        frame_buf_addr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, frame_buf_size, 0, pInst );
        if( frame_buf_addr[PA] == 0 )
        {
            ALOGE("frame_buf_addr[PA] malloc() failed \r\n");
        }

        frame_buf_addr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, frame_buf_addr[PA], frame_buf_size, pInst );
        if( frame_buf_addr[VA] == 0 )
        {
            ALOGE( "frame_buf_addr[VA] malloc() failed \r\n");
        }
        //DPRINTF("frame_buf_addr[VA] = 0x%x", frame_buf_addr[VA]);

        sFrameBufferAddr[PA] = frame_buf_addr[PA];
        sFrameBufferAddr[VA] = frame_buf_addr[VA];
        sFrameBufferSize = frame_buf_size;

#ifdef ENABLE_MULTIPLE_JPU_FRAME_BUFFER
        {
            // Allocate multiple Output Frame buffers
            unsigned int buf_idx;

            pInst->decoded_buf_curIdx = 0;
            pInst->decoded_buf_size = frame_buf_size * 1.5;
            pInst->decoded_buf_size = ALIGNED_BUFF(pInst->decoded_buf_size, ALIGN_LEN);

            //DPRINTF("pInst->gsAdditionalFrameCount = %d", pInst->gsAdditionalFrameCount);
            for(buf_idx =0; buf_idx < pInst->gsAdditionalFrameCount + 1; buf_idx++)
            {
                pInst->decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, pInst->decoded_buf_size, 0, pInst );
                if( pInst->decoded_phyAddr[buf_idx] == 0 )
                {
                    DPRINTF( "[VDEC,Err:%d] vdec_vpu pInst->decoded_virtAddr[PA] alloc failed \n", ret );
                    pInst->decoded_buf_maxcnt = buf_idx;
                    break;
                }
                pInst->decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->decoded_phyAddr[buf_idx], pInst->decoded_buf_size, pInst );
                if( pInst->decoded_virtAddr[buf_idx] == 0 )
                {
                    DPRINTF( "[VDEC,Err:%d] vdec_vpu pInst->decoded_virtAddr[VA] alloc failed \n", ret );
                    pInst->decoded_buf_maxcnt = buf_idx;
                    break;
                }
                pInst->decoded_buf_maxcnt = pInst->gsAdditionalFrameCount + 1;
                //DPRINTF("OUT-Buffer %d ::   PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, pInst->decoded_phyAddr[buf_idx], pInst->decoded_virtAddr[buf_idx], pInst->decoded_buf_size);
            }
        }
#endif

        dec_buffer.m_FrameBufferStartAddr[PA] = frame_buf_addr[PA];
        dec_buffer.m_FrameBufferStartAddr[VA] = frame_buf_addr[VA];

        ret = TCC_JPU_DEC( JPU_DEC_REG_FRAME_BUFFER, &dec_handle, &dec_buffer, 0 );
        if( ret != JPG_RET_SUCCESS )
        {
            ALOGE("VDEC_DEC_SEQ_HEADER (REG BUFFER FAIL). ret = %d", ret);
            return ret;
        }

        pInst->gsbHasSeqHeader = 1;
        p_output_param->m_pInitialInfo = &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
        p_output_param->m_pInitialInfo->m_iPicWidth = dec_initial_info.m_iPicWidth;
        p_output_param->m_pInitialInfo->m_iPicHeight = dec_initial_info.m_iPicHeight;

        return RETCODE_SUCCESS;
    }
    else if( iOpCode == VDEC_DECODE )
    {
        vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
        vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

        jpu_dec_input_t dec_input = {0,};
        jpu_dec_output_t dec_output = {0,};

        dec_input.m_iBitstreamDataSize = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;
        //DPRINTF("dec_input.m_iBitstreamDataSize = %d", dec_input.m_iBitstreamDataSize);

        if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
             && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
        {
            //DPRINTF("BITSTREAM BUFFER IS SAME AS BEFORE.");
            dec_input.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
            dec_input.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[VA];
        }
        else
        {
            //DPRINTF("BITSTREAM BUFFER IS CHANGED.");
            dec_input.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
            dec_input.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[VA];
            memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], dec_input.m_iBitstreamDataSize);
            change_input_stream((unsigned char *)pInst->gsBitstreamBufAddr[VA], &(dec_input.m_iBitstreamDataSize), iOpCode, pInst);
        }

        unsigned char* ps = (unsigned char*)dec_input.m_BitstreamDataAddr[VA];  // debug purpose only

        /*
        if (dec_input.m_iBitstreamDataSize > 64)
        {
            unsigned char* ps = (unsigned char*)dec_input.m_BitstreamDataAddr[VA];
            DPRINTF( "[VDEC-%d Decode %d] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"
                                "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
                                pInst->vdec_instance_index, dec_input.m_iBitstreamDataSize,
                                ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
                                ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
                                ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
                                ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63]);
        }
        */

        dec_input.m_iLooptogle = 0;
        dec_input.m_FrameBufferStartAddr[PA] = (unsigned char *)sFrameBufferAddr[PA];
        dec_input.m_FrameBufferStartAddr[VA] = (unsigned char *)sFrameBufferAddr[VA];
        dec_input.m_iBitstreamDataSize = sBitstreamBufferSize;

        do
        {
            ret = TCC_JPU_DEC( JPU_DEC_DECODE, &dec_handle, &dec_input, &dec_output );
            if(ret != JPG_RET_SUCCESS)
                ALOGE("DECODE ERROR. ret = %d", ret);

            dec_input.m_iLooptogle =1;

            if(ret ==JPG_RET_CODEC_EXIT)
            {
                ALOGE("ret ==JPG_RET_CODEC_EXIT");
                return ret;
            }
            if(ret ==JPG_RET_WRONG_CALL_SEQUENCE)
            {
                ALOGE("ret ==JPG_RET_WRONG_CALL_SEQUENCE");
                return ret;
            }
            if(ret == JPG_RET_BIT_EMPTY)
            {
                ALOGE("ret ==JPG_RET_BIT_EMPTY");
                return ret;
            }
            //DPRINTF("ret = %d", ret);
        } while (ret != JPG_RET_SUCCESS);

        p_output_param->m_DecOutInfo.m_iWidth = dec_output.m_DecOutInfo.m_iWidth;
        p_output_param->m_DecOutInfo.m_iHeight = dec_output.m_DecOutInfo.m_iHeight;

        p_output_param->m_DecOutInfo.m_iDecodingStatus = dec_output.m_DecOutInfo.m_iDecodingStatus;
        if( dec_output.m_DecOutInfo.m_iDecodingStatus == 1)
        {
            p_output_param->m_pDispOut[PA][0] = p_output_param->m_pCurrOut[PA][0] = (unsigned char *)dec_output.m_pCurrOut[PA][COMP_Y];
            p_output_param->m_pDispOut[PA][1] = p_output_param->m_pCurrOut[PA][1] = (unsigned char *)dec_output.m_pCurrOut[PA][COMP_U];
            p_output_param->m_pDispOut[PA][2] = p_output_param->m_pCurrOut[PA][2] = (unsigned char *)dec_output.m_pCurrOut[PA][COMP_V];
            p_output_param->m_pDispOut[VA][0] = p_output_param->m_pCurrOut[VA][0] = (unsigned char *)dec_output.m_pCurrOut[VA][COMP_Y];
            p_output_param->m_pDispOut[VA][1] = p_output_param->m_pCurrOut[VA][1] = (unsigned char *)dec_output.m_pCurrOut[VA][COMP_U];
            p_output_param->m_pDispOut[VA][2] = p_output_param->m_pCurrOut[VA][2] = (unsigned char *)dec_output.m_pCurrOut[VA][COMP_V];

#ifdef ENABLE_MULTIPLE_JPU_FRAME_BUFFER
            int frame_size = dec_output.m_DecOutInfo.m_iWidth * dec_output.m_DecOutInfo.m_iHeight;
            memcpy( (unsigned char *)pInst->decoded_virtAddr[pInst->decoded_buf_curIdx],(unsigned char *)dec_output.m_pCurrOut[VA][COMP_Y], (unsigned int)(frame_size*1.5*sizeof(unsigned char)) );
            pInst->decoded_buf_curIdx++;
            if(pInst->decoded_buf_curIdx >= pInst->decoded_buf_maxcnt)
                pInst->decoded_buf_curIdx = 0;
#endif

            p_output_param->m_DecOutInfo.m_iOutputStatus = 1;
        }
#if 0
        else
        {
            // print debug info
            ALOGE("False! dec_outputm_iDecodingStatus = %d", dec_output.m_DecOutInfo.m_iDecodingStatus);
            ALOGE("False! dec_output.m_iNumOfErrMBs = %d", dec_output.m_DecOutInfo.m_iNumOfErrMBs);
            ALOGE("False! dec_output.m_iConsumedBytes = %d", dec_output.m_DecOutInfo.m_iConsumedBytes);
            ALOGE("False! dec_input.m_iBitstreamDataSize = %d", dec_input.m_iBitstreamDataSize);

            DPRINTF( "[VDEC-%d Decode %d] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n"
                    "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n" "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",
                    pInst->vdec_instance_index, dec_input.m_iBitstreamDataSize,
                    ps[0], ps[1], ps[2], ps[3], ps[4], ps[5], ps[6], ps[7], ps[8], ps[9], ps[10], ps[11], ps[12], ps[13], ps[14], ps[15],
                    ps[16], ps[17], ps[18], ps[19], ps[20], ps[21], ps[22], ps[23], ps[24], ps[25], ps[26], ps[27], ps[28], ps[29], ps[30], ps[31],
                    ps[32], ps[33], ps[34], ps[35], ps[36], ps[37], ps[38], ps[39], ps[40], ps[41], ps[42], ps[43], ps[44], ps[45], ps[46], ps[47],
                    ps[48], ps[49], ps[50], ps[51], ps[52], ps[53], ps[54], ps[55], ps[56], ps[57], ps[58], ps[59], ps[60], ps[61], ps[62], ps[63]);
        }
#endif

        return ret;
    }
    else if( iOpCode == VDEC_BUF_FLAG_CLEAR )
    {
        //DPRINTF("VDEC_BUF_FLAG_CLEAR. vdec_mjpeg_jpu");
        return RETCODE_SUCCESS;
    }
    else if( iOpCode == VDEC_DEC_FLUSH_OUTPUT)
    {
        return RETCODE_SUCCESS;
    }
    else if( iOpCode == VDEC_CLOSE )
    {
        DPRINTF("VDEC_CLOSE. vdec_mjpeg_jpu");
        jpu_dec_output_t dec_output = {0,};
        int i;

        // Now that we are done with decoding, close the open instance.

        if( sBitstreamBufferAddr[VA] ){
            if(cdk_sys_free_virtual_addr( NULL, (void*)sBitstreamBufferAddr[VA], sBitstreamBufferSize)  >= 0)
            {
                sBitstreamBufferAddr[VA] = 0;
            }
        }

        if( sFrameBufferAddr[VA]){
            if(cdk_sys_free_virtual_addr( NULL, (void*)sFrameBufferAddr[VA], sFrameBufferSize )  >= 0)
            {
                sFrameBufferAddr[VA]= 0;
            }
        }

        if( gsRegisterBase ){
            if(cdk_sys_free_virtual_addr( NULL, (void*)gsRegisterBase, gsRegisterBaseSize )  >= 0)
            {
                gsRegisterBase = 0;
            }
        }

#ifdef ENABLE_MULTIPLE_JPU_FRAME_BUFFER
        unsigned int buf_idx;
        for(buf_idx =0; buf_idx < pInst->decoded_buf_maxcnt; buf_idx++)
        {
            if( pInst->decoded_virtAddr[buf_idx] )
            {
                cdk_sys_free_virtual_addr( NULL, (void*)pInst->decoded_virtAddr[buf_idx], pInst->decoded_buf_size );
                pInst->decoded_virtAddr[buf_idx]= 0;
                //DPRINTF("OUT-Buffer Release %d ::   PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, pInst->decoded_phyAddr[buf_idx], pInst->decoded_virtAddr[buf_idx], pInst->decoded_buf_size);
            }
        }
#endif

        jpu_codec_opened = 0;
        ret = TCC_JPU_DEC( JPU_DEC_CLOSE, &dec_handle, 0, &dec_output );
        if(ret != RETCODE_SUCCESS)
        {
            ALOGE("JPU_DEC_CLOSE FAIL");
            return ret;
        }

        ret = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE, &ret);
        if(jpu_clk_fd) {
            if(close(jpu_clk_fd) < 0) {
                ALOGE("Err[%s] :: vpu(%s) close failed!", strerror(errno), TCC_CLK_DEV_NAME);
            }
        }

        #ifdef HAVE_ANDROID_OS
                vpu_env_close(pInst);
        #endif

        return RETCODE_SUCCESS;
    }

    ALOGE("INVALID OP_CODE");
    return 0;
}
#endif

#ifdef JPEG_DECODE_FOR_MJPEG
int
jpeg_dec_ready( dec_init_t* psVDecInit, _vdec_ *pVdec )
{
	//------------------------------------------------------------
	//! [x] bitstream buffer for each VPU decoder
	//------------------------------------------------------------
	_vdec_ * pInst = pVdec;
	pInst->gsMaxBitstreamSize = pInst->gsBitstreamBufSize = JPEGDEC_STREAM_MAX;
	pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsBitstreamBufSize, ALIGN_LEN );
	pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(NULL, pInst->gsBitstreamBufSize, 0, pInst );
	if( pInst->gsBitstreamBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] bitstream_buf_addr[PA] malloc() failed \n");
		return -(VPU_NOT_ENOUGH_MEM);
	}
	DSTATUS( "[VDEC] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize );
	pInst->gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->gsBitstreamBufAddr[PA], pInst->gsBitstreamBufSize, pInst );
	if( pInst->gsBitstreamBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] bitstream_buf_addr[VA] malloc() failed \n");
		return -(VPU_NOT_ENOUGH_MEM);
	}
	memset( (void*)pInst->gsBitstreamBufAddr[VA], 0x00 , pInst->gsBitstreamBufSize);
	DSTATUS("[VDEC] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", (codec_addr_t)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize );

	psVDecInit->m_BitstreamBufAddr[PA]	= pInst->gsBitstreamBufAddr[PA];
	psVDecInit->m_BitstreamBufAddr[VA]	= pInst->gsBitstreamBufAddr[VA];
	psVDecInit->m_iBitstreamBufSize 	= pInst->gsBitstreamBufSize;

	return 0;
}

int
vdec_jpeg( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void* pParam3)
{
	int ret = 0;
	TCCXXX_JPEG_DEC_DATA dec_data;
	_vdec_ *pInst = (_vdec_ *) pParam3;
	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	if( iOpCode == VDEC_INIT )
	{
		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;		
		vdec_user_info_t* p_input_user_param = (vdec_user_info_t*)pParam2;

		pInst->gsJPEGDecUserInfo.bitrate_mbps = p_input_user_param->bitrate_mbps;
		pInst->gsJPEGDecUserInfo.frame_rate   = p_input_user_param->frame_rate;
		pInst->gsJPEGDecUserInfo.m_bJpegOnly  = p_input_user_param->m_bJpegOnly;
		
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, p_input_user_param->bitrate_mbps, p_input_user_param->frame_rate, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight, pInst ) < 0)
			return -VPU_ENV_INIT_ERROR;

		pInst->gsJPEGDecHandle = open(JPEGDEC_DEVICE, O_RDWR | O_NDELAY);
		if(!pInst->gsJPEGDecHandle)
		{
			LOGE("Err[%s] :: jpeg(%s) open failed!", strerror(errno), JPEGDEC_DEVICE);
			return -RETCODE_FAILURE;
		}
		
		pInst->gsJPEGDecInit.m_RegBaseVirtualAddr	= 0;
		pInst->gsJPEGDecInit.m_iBitstreamFormat		= p_input_param->m_iBitstreamFormat;
		pInst->gsJPEGDecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		pInst->gsJPEGDecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		pInst->gsJPEGDecInit.m_bEnableUserData		= p_input_param->m_bEnableUserData;
		pInst->gsJPEGDecInit.m_bEnableVideoCache	= p_input_param->m_bEnableVideoCache;
		pInst->gsJPEGDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;
		pInst->gsJPEGDecInit.m_Memcpy				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		pInst->gsJPEGDecInit.m_Memset				= (void  (*) ( void*, int, unsigned int ))memset;
		pInst->gsJPEGDecInit.m_Interrupt			= (int  (*) ( void ))NULL;	
		pInst->gsJPEGDecInit.m_iFilePlayEnable		= p_input_param->m_bFilePlayEnable;
		pInst->gsbHasSeqHeader = 0;//p_input_param->m_bHasSeqHeader; 

		jpeg_dec_ready( &pInst->gsJPEGDecInit, pInst );
		DSTATUS("Format : %d, Stream(0x%x, %d)", pInst->gsJPEGDecInit.m_iBitstreamFormat, pInst->gsJPEGDecInit.m_BitstreamBufAddr[PA], pInst->gsJPEGDecInit.m_iBitstreamBufSize);

		ret = RETCODE_SUCCESS;
		pInst->vdec_codec_opened = 1;
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;

		if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
		     && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
		{
			pInst->gsJPEGDecInit.m_BitstreamBufAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
			pInst->gsJPEGDecInit.m_BitstreamBufAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		}
		else
		{
			pInst->gsJPEGDecInit.m_BitstreamBufAddr[PA] = pInst->gsBitstreamBufAddr[PA];
			pInst->gsJPEGDecInit.m_BitstreamBufAddr[VA] = pInst->gsBitstreamBufAddr[VA];
			memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], seq_stream_size);
		}

		DSTATUS( "[VDEC_JPG] JPEG_DEC_SEQ_HEADER start  :: len = %d / %d , handle = 0x%x \n", seq_stream_size, p_input_param->m_iInpLen, pInst->gsJPEGDecHandle );

		memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));

		dec_data.source_addr = pInst->gsJPEGDecInit.m_BitstreamBufAddr[PA];
		dec_data.file_length = seq_stream_size;
		dec_data.target_addr = pInst->gsJPEGDecInit.m_BitstreamBufAddr[PA];
		dec_data.target_size = pInst->gsBitstreamBufSize;
		
		ret = ioctl(pInst->gsJPEGDecHandle, TCC_JPEGD_IOCTL_HEADER_INFO, &dec_data);
		if( ret < 0 )
		{		
			LOGE("Err[%s] :: jpeg(%d) TCC_JPEGD_IOCTL_HEADER_INFO failed!", strerror(errno), ret);

			if(ret == -1)
				return -RETCODE_INVALID_STRIDE;
			else
				return -ret;
		}
		pInst->gsJPEGDecInitialInfo.m_iPicWidth = dec_data.pad_width;
		pInst->gsJPEGDecInitialInfo.m_iPicHeight = dec_data.pad_height;
		DSTATUS( "[VDEC_JPG] Resolution %d x %d, Format : %d \n", pInst->gsJPEGDecInitialInfo.m_iPicWidth, pInst->gsJPEGDecInitialInfo.m_iPicHeight, dec_data.image_format);

		//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )(only for TCC93XX)
		if(dec_data.image_format == IMAGE_CHROMA_420)
			pInst->gsJPEGDecInitialInfo.m_iMjpg_sourceFormat = 0;
		else
			pInst->gsJPEGDecInitialInfo.m_iMjpg_sourceFormat = 1;		

		if(dec_data.image_format != IMAGE_CHROMA_420)
		{
			LOGI("MJPEG Format is not YUV420 (%d)", dec_data.image_format); //1:420, 2:422
		}
		
		pInst->gsbHasSeqHeader = 1;
		p_output_param->m_pInitialInfo = &pInst->gsJPEGDecInitialInfo;

		//check the maximum/minimum video resolution limitation
		{
			vdec_info_t * pVdecInfo = (vdec_info_t *)&pInst->gsJPEGDecInitialInfo;
			unsigned int buf_idx;
			
			pInst->decoded_buf_curIdx = 0;
			pInst->decoded_buf_size = pVdecInfo->m_iPicWidth*pVdecInfo->m_iPicHeight*2;
			pInst->decoded_buf_size = ALIGNED_BUFF(pInst->decoded_buf_size, ALIGN_LEN);
			
			for(buf_idx =0; buf_idx < MAX_NUM_OF_JPEG_ELEMENT; buf_idx++)
			{
				pInst->decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr(NULL, pInst->decoded_buf_size, 0, pInst );				
				if( pInst->decoded_phyAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC_JPG,Err:%d] vdec_vpu pInst->decoded_virtAddr[PA] alloc failed \n", ret );
					pInst->decoded_buf_maxcnt = buf_idx;
					break;
				}	
				pInst->decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, pInst->decoded_phyAddr[buf_idx], pInst->decoded_buf_size, pInst ); 
				if( pInst->decoded_virtAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC_JPG,Err:%d] vdec_vpu pInst->decoded_virtAddr[VA] alloc failed \n", ret );
					pInst->decoded_buf_maxcnt = buf_idx;
					break;
				}

				pInst->decoded_buf_maxcnt = MAX_NUM_OF_JPEG_ELEMENT;
				DSTATUS("OUT-Buffer %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, pInst->decoded_phyAddr[buf_idx], pInst->decoded_virtAddr[buf_idx],	pInst->decoded_buf_size);
			}			
		}
		
		DSTATUS( "[VDEC_JPG] VDEC_DEC_SEQ_HEADER - Success \n" );
		DSTATUS( "=======================================================\n\n" );		
	}
	else if( iOpCode == VDEC_DECODE )
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
#ifdef DEBUG_TIME_LOG
		clock_t start, end;
#endif

		pInst->gsJPEGDecInput.m_iBitstreamDataSize = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;
		if (    ((codec_addr_t)p_input_param->m_pInp[PA] == pInst->gsBitstreamBufAddr[PA])
		     && ((codec_addr_t)p_input_param->m_pInp[VA] == pInst->gsBitstreamBufAddr[VA]) )
		{
			pInst->gsJPEGDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
			pInst->gsJPEGDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		}
		else
		{
			pInst->gsJPEGDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
			pInst->gsJPEGDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[VA];
			DSTATUS("memcpy : 0x%x - %d", pInst->gsJPEGDecInput.m_BitstreamDataAddr[PA], pInst->gsJPEGDecInput.m_iBitstreamDataSize);
			memcpy( (void*)pInst->gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], pInst->gsJPEGDecInput.m_iBitstreamDataSize);
		}
				
#ifdef  VIDEO_DEC_PROFILE
		*((volatile unsigned int *)(pTestRegBase + GPIO_BASE_OFFSET + 0x4)) |= (unsigned int)(GPIO_NUM);
		*((volatile unsigned int *)(pTestRegBase + GPIO_BASE_OFFSET)) |= (unsigned int)(GPIO_NUM);
#ifdef DEBUG_TIME_LOG		
		start = clock();
#endif
#endif

		// Start decoding a frame.
		memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));

		dec_data.source_addr = pInst->gsJPEGDecInput.m_BitstreamDataAddr[PA];
		dec_data.file_length = pInst->gsJPEGDecInput.m_iBitstreamDataSize;
		dec_data.target_addr = pInst->decoded_phyAddr[pInst->decoded_buf_curIdx];
		dec_data.target_size = pInst->decoded_buf_size;

		DSTATUS("out[%d] : 0x%x - %d", pInst->decoded_buf_curIdx, pInst->decoded_phyAddr[pInst->decoded_buf_curIdx], pInst->decoded_buf_size);		
		ret = ioctl(pInst->gsJPEGDecHandle, TCC_JPEGD_IOCTL_DEC, &dec_data);
		if( ret < 0 )
		{
			LOGE("Err[%s] :: jpeg(%d) TCC_JPEGD_IOCTL_DEC failed!", strerror(errno), ret);
			p_output_param->m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_FAIL;	
			p_output_param->m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;
			return 0;
		}
		pInst->gsJPEGDecOutput.m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_SUCCESS;
		pInst->gsJPEGDecOutput.m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;

#ifdef  VIDEO_DEC_PROFILE
#ifdef DEBUG_TIME_LOG		
		end = clock();
		dec_time[time_cnt] = (end-start)*1000/CLOCKS_PER_SEC;
		total_dec_time += dec_time[time_cnt];
		total_frame_cnt++;
		if(time_cnt != 0 && time_cnt % 29 == 0)
		{
			LOGD("VDEC_JPG %2.1f ms: %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d",
				total_dec_time/(float)total_frame_cnt, dec_time[0], dec_time[1], dec_time[2], dec_time[3], dec_time[4], dec_time[5], dec_time[6], dec_time[7], dec_time[8], dec_time[9], 
				dec_time[10], dec_time[11], dec_time[12], dec_time[13], dec_time[14], dec_time[15], dec_time[16], dec_time[17], dec_time[18], dec_time[19], 
				dec_time[20], dec_time[21], dec_time[22], dec_time[23], dec_time[24], dec_time[25], dec_time[26], dec_time[27], dec_time[28], dec_time[29]);
			time_cnt = 0;
		}
		else{
			time_cnt++;
		}
#endif
		*((volatile unsigned int *)(pTestRegBase + GPIO_BASE_OFFSET + 0x4)) |= (unsigned int)(GPIO_NUM);
		*((volatile unsigned int *)(pTestRegBase + GPIO_BASE_OFFSET)) &= (unsigned int)(~GPIO_NUM);
#endif

		{
			unsigned int frame_size = pInst->gsJPEGDecInitialInfo.m_iPicWidth*pInst->gsJPEGDecInitialInfo.m_iPicHeight;
			unsigned char div_val = 4;

			memcpy((void*)p_output_param, (void*)&pInst->gsJPEGDecOutput, sizeof(dec_output_t ) );

			if(pInst->gsJPEGDecInitialInfo.m_iMjpg_sourceFormat == 0)
				div_val = 4;
			else
				div_val = 2;
			
			p_output_param->m_pDispOut[PA][0] = (unsigned char*)pInst->decoded_phyAddr[pInst->decoded_buf_curIdx];
			p_output_param->m_pDispOut[PA][1] = (unsigned char*)p_output_param->m_pDispOut[PA][0] + frame_size;
			p_output_param->m_pDispOut[PA][2] = (unsigned char*)p_output_param->m_pDispOut[PA][1] + frame_size/div_val;
			p_output_param->m_pDispOut[VA][0] = (unsigned char*)pInst->decoded_virtAddr[pInst->decoded_buf_curIdx];
			p_output_param->m_pDispOut[VA][1] = (unsigned char*)p_output_param->m_pDispOut[VA][0] + frame_size;
			p_output_param->m_pDispOut[VA][2] = (unsigned char*)p_output_param->m_pDispOut[VA][1] + frame_size/div_val;

			if( pInst->gsJPEGDecInitialInfo.m_iMjpg_sourceFormat == 1) //YUV422
			{
				//To Do::
			}

			pInst->decoded_buf_curIdx++;
			if(pInst->decoded_buf_curIdx >= pInst->decoded_buf_maxcnt)
				pInst->decoded_buf_curIdx = 0;
		}
		p_output_param->m_pInitialInfo = &pInst->gsJPEGDecInitialInfo;
	}
	else if( iOpCode == VDEC_CLOSE )
	{
		if(!pInst->vdec_codec_opened && !pInst->vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		if(pInst->vdec_codec_opened)
		{
			if(close(pInst->gsJPEGDecHandle) < 0)
			{
				LOGE("Err[%s] :: jpeg(%s) close failed!", strerror(errno), JPEGDEC_DEVICE);
			}
			pInst->gsJPEGDecHandle = -1;
			pInst->vdec_codec_opened = 0;
		}
		
		if(!pInst->vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		pInst->gsBitstreamBufAddr[PA] = 0;
		if( pInst->gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)pInst->gsBitstreamBufAddr[VA], pInst->gsBitstreamBufSize )  >= 0)
			{
				pInst->gsBitstreamBufAddr[VA] = 0;
			}
		}

		
		if( pInst->gsJPEGDecInit.m_iBitstreamFormat == STD_MJPG && !pInst->gsJPEGDecUserInfo.m_bJpegOnly)
		{
			unsigned int buf_idx = 0;
			
			for(buf_idx =0; buf_idx < pInst->decoded_buf_maxcnt; buf_idx++)
			{
				pInst->decoded_phyAddr[buf_idx] = 0;
				cdk_sys_free_virtual_addr( NULL, (void*)pInst->decoded_virtAddr[buf_idx], pInst->decoded_buf_size );
				pInst->decoded_virtAddr[buf_idx] = 0;
			}			
		}
		vpu_env_close(pInst);
	}
	else
	{
		return 0;
	}

	return ret;
}
#endif

int vpu_preCtrl(int curr_fd)
{
	int fd;

	if(curr_fd > 0)
	{
		if(close(curr_fd) < 0)
		{
			LOGE("Err[%s] :: vpu(%s) close failed!", strerror(errno), VPU_MGR_NAME);
		}	
		LOGI( " VPU PreOpen Closed!! " );

		return 0;
	}
	else
	{
		fd = open(VPU_MGR_NAME, O_RDWR);
		if(fd < 0)
		{
			LOGE("Err[%s] :: vpu(%s) open failed!", strerror(errno), VPU_MGR_NAME);
			return -1;
		}
		
		LOGI( " VPU PreOpened!! " );

		return fd;
	}
	
	return 0;
}

const float MPEG2_Aspect_Ratio[]={
	0.0,        // forbidden
	1.0,        // 1 : 1 Square
	4.0/3.0,	// 3 / 4 = 1.33
	16.0/9.0,   // 9 / 16 = 1.77
	221.0/1.0   // 1/2,21 ??
};

const float MPEG4_Aspect_Ratio[]={
	0,          // forbidden
	1,          // 1 : 1 Square
	12.0/11.0,  // 12:11
	10.0/11.0,  // 10:11
	16.0/11.0,  // 16:11
	40.0/33.0   // 40:33
};

const float AVC_Aspect_Ratio[]={
	0.0,        // unspecified
	1.0,        // 1 : 1 Square
	12.0/11.0,  // 12:11
	10.0/11.0,  // 10:11
	16.0/11.0,  // 16:11
	40.0/33.0,  // 40:33
	24.0/11.0,  // 24:11
	20.0/11.0,  // 20:11
	32.0/11.0,  // 32:11
	80.0/33.0,  // 80:33
	18.0/11.0,  // 18:11
	15.0/11.0,  // 15:11
	64.0/33.0,  // 64:33
   160.0/99.0,  // 160:99
     4.0/3.0,
     3.0/2.0,
     2.0/1.0,
};

int vdec_getAspectRatio(void * pInst, signed long *wScale, signed long *hScale, unsigned int width, unsigned int height,
							cdmx_info_t dmxInfo, unsigned int dmxType, unsigned int dmxTcc)
{
	float DAR, SAR, PAR, ImageRatio;
	int PAR_Width, PAR_Height;
	int AspectRatio;
	float display_w, display_h;
	_vdec_ * pVdec = (_vdec_ *) pInst;

	ImageRatio = (float)width/(float)height;
	SAR = PAR = DAR = 1.0;
	*wScale = *hScale = 0x010000;

#ifndef SUPPORT_ASPECT_RATIO
	return 0;
#endif

	if(pInst)
	{
		if( dmxTcc )
		{
			switch(dmxType)
			{
				case CONTAINER_TYPE_MP4:
			//Fixed
					if( dmxInfo.m_sVideoInfo.m_iAspectRatioX > 0 && dmxInfo.m_sVideoInfo.m_iAspectRatioY > 0 ){
						PAR = (float)dmxInfo.m_sVideoInfo.m_iAspectRatioX/(float)dmxInfo.m_sVideoInfo.m_iAspectRatioY;
						LOGE("[MP4] PAR :%d x %d, PAR: %f", dmxInfo.m_sVideoInfo.m_iAspectRatioX, dmxInfo.m_sVideoInfo.m_iAspectRatioY, PAR);
						goto Done;
					}
					break;

				case CONTAINER_TYPE_ASF:
			//Please confirm it :: It works well when I had tried to test.
					if( dmxInfo.m_sVideoInfo.m_iHasPayloadAspectRatio && dmxInfo.m_sVideoInfo.m_iAspectRatioX > 0 && dmxInfo.m_sVideoInfo.m_iAspectRatioY > 0 ){
						PAR = (float)dmxInfo.m_sVideoInfo.m_iAspectRatioX/(float)dmxInfo.m_sVideoInfo.m_iAspectRatioY;
						LOGE("[ASF] PAR :%d x %d, PAR: %f", dmxInfo.m_sVideoInfo.m_iAspectRatioX, dmxInfo.m_sVideoInfo.m_iAspectRatioY, PAR);
						goto Done;
					}
					break;

				case CONTAINER_TYPE_MPG:
			//??
					if( dmxInfo.m_sVideoInfo.m_iAspectRatio > 0){
						if(dmxInfo.m_sVideoInfo.m_iAspectRatio > 1 && dmxInfo.m_sVideoInfo.m_iAspectRatio < sizeof(MPEG2_Aspect_Ratio)/sizeof(float)){
							SAR = MPEG2_Aspect_Ratio[dmxInfo.m_sVideoInfo.m_iAspectRatio];
							if(SAR > 1.0)
								PAR = SAR/ImageRatio;
							LOGE("[MPG] AspectRatio: %d, PAR: %f", dmxInfo.m_sVideoInfo.m_iAspectRatio, PAR);
							goto Done;
						}
					}
					break;

				default:
					break;
			}
		}

		AspectRatio = pVdec->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iAspectRateInfo;
		if(AspectRatio)
		{
			switch (pVdec->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat) {
				case STD_AVC:
				case STD_MVC:
			//Please confirm it :: According to explations on some website, SAR in H.264 is same with PAR.
					if(AspectRatio) {
						int aspect_ratio_idc;

						if( ((AspectRatio >> 16) & 0xFF) == 0 ) {
							aspect_ratio_idc = (AspectRatio & 0xFF);

							if(aspect_ratio_idc > 1 &&  aspect_ratio_idc < sizeof(AVC_Aspect_Ratio)/sizeof(float)){
								PAR = AVC_Aspect_Ratio[aspect_ratio_idc];
								//if(SAR > 1.0)
								//	DAR = SAR/ImageRatio;
								LOGE("[AVC-idc] idx :%d, SAR: %f, DAR: %f", aspect_ratio_idc, SAR, DAR);
							}
							else{
								// 17 <= SAR <= 254 => Reserved
							}
						}
						else {
							// extended PAR
							PAR_Width  = (AspectRatio >> 16) & 0xFFFF;
							PAR_Height  = (AspectRatio & 0xFFFF);
							if(PAR_Height && PAR_Width > 0 && PAR_Height > 0){
								PAR = (float)PAR_Width/(float)PAR_Height;
								LOGE("[AVC-size] PAR :%d x %d, PAR: %f", PAR_Width, PAR_Height, PAR);
							}
						}
					}
					break;

				case STD_VC1:
					if(AspectRatio){
						PAR_Width = (AspectRatio >> 8) & 0xff;
						PAR_Height = (AspectRatio & 0xff);
						if(PAR_Height && PAR_Width > 0 && PAR_Height > 0){
							PAR = (float)PAR_Width/(float)PAR_Height;
							LOGE("[VC1] PAR :%d x %d, PAR: %f", PAR_Width, PAR_Height, PAR);
						}
					}
					break;

				case STD_MPEG2:
			//Fixed
					if(AspectRatio > 1 && AspectRatio < sizeof(MPEG2_Aspect_Ratio)/sizeof(float)){
						SAR = MPEG2_Aspect_Ratio[AspectRatio];
						if(SAR > 1.0)
							DAR = SAR/ImageRatio;
						LOGE("[MPEG2] idx %d, SAR: %f, DAR: %f", AspectRatio, SAR, DAR);
					}
					break;

				case STD_MPEG4:
					if(AspectRatio > 1 && AspectRatio < sizeof(MPEG4_Aspect_Ratio)/sizeof(float)){
						PAR = MPEG4_Aspect_Ratio[AspectRatio];
						LOGE("[MPEG4] idx %d, PAR: %f", AspectRatio, PAR);
					}
					break;

				case STD_RV:
#ifdef INCLUDE_SORENSON263_DEC
				case STD_SORENSON263:
#endif
				case STD_MJPG:
				default:
					break;
			}
		}
	}

Done:
	display_h = (float)height;

	if(PAR != 1.0) {
		display_w = (float)width * PAR;
	}
	else {
		if(DAR == 0.0 || DAR == 1.0) {
			LOGE("[VDEC] Keep Aspect Ratio :: AspectRatio:0x%8x / SAR: %f PAR: %f DAR: %f", AspectRatio, SAR, PAR, DAR);
			return 0;
		}
		display_w = (float)width * DAR;
	}

	*wScale = ((signed long)display_w << 16)/width;
	LOGI("[1. Aspect Ratio] [%d Fmt] [SAR: %f, SAR: %f, DAR:%f] :: [%d x %d => %f x %f]-[0x%x , 0x%x]", pVdec->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat,
																				PAR, SAR, DAR, width, height, display_w, display_h, (unsigned int)*wScale, (unsigned int)*hScale);

	if(*wScale != 0x010000 || *hScale != 0x010000)
		return 1;
	else
		return 0;
}

void * vdec_alloc_instance(void)
{
	_vdec_ *pInst = NULL;
	int fd, nInstance;

	pInst = (_vdec_*)TCC_malloc(sizeof(_vdec_));
	if( pInst )
	{
		memset(pInst, 0x00, sizeof(_vdec_));

		pInst->vpu_mgr_fd = -1;
		pInst->vpu_dec_fd = -1;
#ifdef USE_VPU_INTERRUPT
		pInst->vpu_intr_fd = -1;
#endif

		pInst->vpu_mgr_fd = open(VPU_MGR_NAME, O_RDWR);
		if(pInst->vpu_mgr_fd < 0)
		{
			LOGE("%s open error[%s]!!", VPU_MGR_NAME, strerror(errno));
			close(pInst->vpu_mgr_fd);
			TCC_free(pInst);
			return NULL;
		}

		ioctl(pInst->vpu_mgr_fd, VPU_GET_INSTANCE_IDX, &nInstance);
		if( nInstance < 0 )
		{
			close(pInst->vpu_mgr_fd);
			TCC_free(pInst);
			return NULL;
		}

		pInst->vdec_instance_index = nInstance;
		vdec_open_count++;
		LOGI("vdec_alloc_instance %d, total %d", nInstance, vdec_open_count);
	}

	return pInst;
}

void vdec_release_instance(void * pInst)
{
	if(pInst)
	{
		_vdec_ * pVdec = (_vdec_ *)pInst;
		int used = pVdec->vdec_instance_index;

		ioctl(pVdec->vpu_mgr_fd, VPU_CLEAR_INSTANCE_IDX, &used);

		if(pVdec->vpu_mgr_fd)
		{
			if(close(pVdec->vpu_mgr_fd) < 0)
			{
				LOGE("%s close error", VPU_MGR_NAME);
			}
			pVdec->vpu_mgr_fd = -1;
		}

		TCC_free(pInst);
		pInst = NULL;

		vdec_open_count--;
		if(vdec_open_count < 0)
			vdec_open_count = 0;
		
		LOGI("vdec_release_instance %d, total %d", used, vdec_open_count);
	}
}

int vdec_get_instance_index(void * pInst)
{
	_vdec_ * pVdec = (_vdec_ *) pInst;
	
	return pVdec->vdec_instance_index;
}

void vdec_set_rendered_index(void * pInst)
{
	if(pInst)
	{
		_vdec_ * pVdec = (_vdec_ *) pInst;

		pVdec->renderered = 1;
	}	
}

int vdec_is_rendered_index(void * pInst)
{
	if(pInst)
	{
		_vdec_ * pVdec = (_vdec_ *) pInst;

		return pVdec->renderered;
	}
	
	return 0;
}

