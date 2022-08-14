/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2009 Telechips Inc.
  Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 \endverbatim
 ***********************************************************************
 */
/*!
 ***********************************************************************
 *
 * \file
 *		vdec.c
 * \date
 *		2009/06/01
 * \author
 *		AV algorithm group(AValgorithm@telechips.com) 
 * \brief
 *		video decoder
 *
 ***********************************************************************
 */
#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"VPUDEC_CAM_K"
#include <utils/Log.h>

#include "cdk_core.h"
#include "vpudec.h"
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

#include <sys/mman.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mach/tcc_vpu_ioctl.h>

/************************************************************************/
/* TEST and Debugging                                               								 */
/************************************************************************/
static int DEBUG_ON	= 0;
#define DPRINTF(msg...)	  ALOGE( ": " msg);
#define DSTATUS(msg...)	  if (DEBUG_ON) { ALOGD( ": " msg);}
#define DBUG_FLIP(msg...) if (DEBUG_ON) { ALOGD( ": " msg);}
#define DPRINTF_FRAME(msg...) //ALOGD(": " msg);

#if defined(TCC_88XX_INCLUDE) || defined(TCC_93XX_INCLUDE)
#define VPU_PERFORMANCE_UP //use sram and vcache
#endif
#define TCC_VPU_INPUT_BUF_SIZE 		(1024 * 1024)
//#define MAX_FRAME_BUFFER_COUNT		31 // move to vdec.h

#define MAX_BITSTREAM_SIZE			(2 * 1024 * 1024)

#undef USE_VPU_INTERRUPT
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
static unsigned int total_frm = 0;
#else
#include "vdec.h"

#include "../cdk/cdk_core.h"
#include "../cdk/cdk_sys.h"
//#include "vpu/TCC_VPU_CODEC.h"
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

static unsigned char vdec_env_opened = 0, vdec_codec_opened = 0, prev_codec = 0;

int vpu_mgr_fd = -1;
int vpu_dec_fd = -1;
#define VPU_MGR_NAME	"/dev/vpu_mgr"

char *vpu_devices[4] =
{
	"/dev/vpu_dec",
	"/dev/vpu_dec_ext",
	"/dev/vpu_dec_ext2",
	"/dev/vpu_dec_ext3"
};
static unsigned int nInstance = 0;

#include <fcntl.h>         // O_RDWR
#include <sys/poll.h>
struct pollfd tcc_event[1];

#ifdef USE_VPU_INTERRUPT
#define TCC_INTR_DEV_NAME		"/dev/tcc_intr"
int vpu_intr_fd = -1;
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

static int gsBitWorkBufSize = 0, gsSliceBufSize = 0, gsSpsPpsSize = 0, gsFrameBufSize = 0, gsIntermediateBufSize = 0;
static codec_addr_t gsBitWorkBufAddr[3] = {0,}, gsSliceBufAddr = 0, gsSpsPpsAddr = 0, gsFrameBufAddr[3] = {0,}, gsIntermediateBufAddr[3] = {0,};
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
static VDEC_GET_VERSION_t gsVpuDecVersion;
static VDEC_RINGBUF_GETINFO_t gsVpuDecBufStatus;
static VDEC_RINGBUF_SETBUF_t gsVpuDecBufFill;
static VDEC_RINGBUF_SETBUF_PTRONLY_t gsVpuDecUpdateWP;

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

#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC) || defined(JPEG_DECODE_FOR_MJPEG) || defined(VPU_FOR_CAMERA)
#define MAX_NUM_OF_VIDEO_ELEMENT 6
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
	{ " ", " ", " ", " ", " ", " " }
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
	{ "", "", "", "", "", "", "", "" }
};

static void vpu_env_close(void);

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

static void *cdk_sys_malloc_physical_addr(unsigned int *remap_addr, int uiSize, Buffer_Type type)
{
	MEM_ALLOC_INFO_t alloc_mem;

	memset(&alloc_mem, 0x00, sizeof(MEM_ALLOC_INFO_t));
	
	alloc_mem.request_size = uiSize;
	alloc_mem.buffer_type = type;
	ioctl(vpu_dec_fd, V_DEC_ALLOC_MEMORY, &alloc_mem);
	if(remap_addr != NULL)
		*remap_addr = alloc_mem.kernel_remap_addr;

	return (void*)( alloc_mem.phy_addr );;
}


static void *cdk_sys_malloc_virtual_addr(int* pDev, codec_addr_t pPtr, int uiSize)
{
	void *map_ptr = MAP_FAILED;
	map_ptr = (void *)mmap(NULL, uiSize, PROT_READ | PROT_WRITE, MAP_SHARED, vpu_dec_fd, pPtr);
	if(MAP_FAILED == map_ptr)
	{
		ALOGE("mmap failed. fd(%d), addr(0x%x), size(%d)", vpu_dec_fd, pPtr, uiSize);
		return NULL;
	}
	
	return map_ptr;
}

static int cdk_sys_free_virtual_addr(int* pDev, void* pPtr, int uiSize)
{
	int ret = 0;
	
	if((ret = munmap((void*)pPtr, uiSize)) < 0)
	{
		ALOGE("munmap failed. addr(0x%x), size(%d)", pPtr, uiSize);
	}

	return ret;
}

static unsigned int cdk_sys_remain_memory_size(void)
{
	unsigned int sz_freeed_mem;

	ioctl(vpu_mgr_fd, VPU_GET_FREEMEM_SIZE, &sz_freeed_mem);

	return sz_freeed_mem;
}

void vpu_update_sizeinfo(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height)
{
	CONTENTS_INFO info;

	info.type = VPU_DEC_EXT;

#if defined(INCLUDE_WMV78_DEC) || defined(INCLUDE_SORENSON263_DEC)
	if(
	#ifdef INCLUDE_WMV78_DEC							
		format == STD_WMV78 
	#endif			
	#ifdef INCLUDE_SORENSON263_DEC				
		|| format == STD_SORENSON263
	#endif
	)
		info.isSWCodec = 1;
	else
#endif
		info.isSWCodec = 0;

#ifdef JPEG_DECODE_FOR_MJPEG
	if( format == STD_MJPG )
	{
		ALOGI("vpu_update_sizeinfo :: STD_MJPG for TCC8900/9200/9200s");
		info.width = AVAILABLE_MAX_WIDTH;
		info.height = AVAILABLE_MAX_HEIGHT;
	}
	else
#endif
	if( format == STD_MJPG && gsVpuDecUserInfo.m_bJpegOnly)
	{
		info.width = AVAILABLE_MAX_WIDTH;
		info.height = AVAILABLE_MAX_HEIGHT;
	}
	else
	{
		info.width = image_width;
		info.height = image_height;
	}
	
	info.bitrate = bps;
	info.framerate = fps;
	ioctl(vpu_mgr_fd, VPU_SET_CLK, &info);

	return;
}

#ifdef VPU_FOR_CAMERA
static int vpu_check_for_video(unsigned char open_status)
{
	if(open_status)
	{
		vpu_mgr_fd = open(VPU_MGR_NAME, O_RDWR);
		if(vpu_mgr_fd < 0)
		{
			ALOGE("%s open error[%s]!!", VPU_MGR_NAME, strerror(errno));			
			return -1;
		}

		ioctl(vpu_mgr_fd, VPU_GET_INSTANCE_IDX, &nInstance);
		if( nInstance < 0 )
		{
			close(vpu_mgr_fd);
			return -1;
		}
		
		vpu_dec_fd = open(vpu_devices[nInstance], O_RDWR);
		if(vpu_dec_fd < 0)
		{
			ioctl(vpu_mgr_fd, VPU_CLEAR_INSTANCE_IDX, &nInstance);
			close(vpu_mgr_fd);
			ALOGE("%s open error[%s]", vpu_devices[nInstance], strerror(errno));
			return -1;
		}

	}

	return 0;
}

#else
static int vpu_check_for_video(unsigned char open_status)
{
//return value : if hw reset need or not!!
	int ret_vpu_reset = 0;
	int vpu_wait;
	OPENED_gINFO gInfo;
	OPENED_sINFO sInfo;
	int temp_vpu_dec_fd = -1, temp_vpu_mgr_fd= -1;

	if(open_status)
	{
		ret_vpu_reset = 0;

// 1. check open in vdec in case so library was loaded status in the same region!!
		if(vdec_env_opened || vdec_codec_opened)
		{
			//to recover abnormal stop error!!
			ALOGE("VDEC has been already opened. so have to close!! ");

	#ifdef INCLUDE_WMV78_DEC
			if(prev_codec == STD_WMV78)
				vdec_WMV78( VDEC_CLOSE, NULL, NULL, NULL );
			else
	#endif
	#ifdef INCLUDE_SORENSON263_DEC
			if(prev_codec == STD_SORENSON263)
				vdec_sorensonH263dec( VDEC_CLOSE, NULL, NULL, NULL );
			else
	#endif
			{
				//process directly kernel device!!
			}
		}

		temp_vpu_mgr_fd = open(VPU_MGR_NAME, O_RDWR);
		if(temp_vpu_mgr_fd < 0)
		{
			ALOGE("%s open error[%s]!!", VPU_MGR_NAME, strerror(errno));
			return -1;
		}

		ioctl(temp_vpu_mgr_fd, VPU_GET_INSTANCE_IDX, &nInstance);
		if( nInstance < 0 )
		{
			close(temp_vpu_mgr_fd);
			return -1;
		}

		temp_vpu_dec_fd = open(vpu_devices[nInstance], O_RDWR);
		if(temp_vpu_dec_fd < 0)
		{
			ALOGE("%s open error[%s]", vpu_devices[nInstance], strerror(errno));
			ioctl(temp_vpu_mgr_fd, VPU_CLEAR_INSTANCE_IDX, &nInstance);
			close(temp_vpu_mgr_fd);
			return -1;
		}

// 2. check open for 3 second in vpu device driver!!
		ioctl(temp_vpu_mgr_fd, VPU_GET_OPEN_INFO, &gInfo);
		DBUG_FLIP("open info :0x%x: %d, dmb(%d), video(%d)", &prev_codec, gInfo.count, gInfo.dmb_opened, gInfo.vid_opened);
				
		if(gInfo.count != 1)
		{			
			if(gInfo.dmb_opened)
			{
				DBUG_FLIP("VPU for DMB has been opened. So wait for 3 sec and check per 100ms!!");
				vpu_wait = 30;
				while(vpu_wait > 0)
				{
					usleep(100 * 1000);//100ms				
					ioctl(temp_vpu_mgr_fd, VPU_GET_OPEN_INFO, &gInfo);
					vpu_wait--;

					if(gInfo.dmb_opened == 0)
					{
						DBUG_FLIP("VPU for DMB stopped.");
						break;
					}
					
					if(vpu_wait <= 0)
					{
						ALOGE("VPU for DMB didn't close!!. so assume to close and reset vpu!!");
						
						sInfo.type = OPEN_DMB;
						sInfo.opened_cnt = 0;
						ioctl(temp_vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
						
						ret_vpu_reset = 1;
						break;
					}
				}
			}
			
			if(0)//gInfo.vid_opened)
			{
		#if 1				
				if(close(temp_vpu_dec_fd) < 0)
				{
					ALOGE("%s close error[%s]", vpu_devices[nInstance], strerror(errno));
				}
			
				if(close(temp_vpu_mgr_fd) < 0)
				{
					ALOGE("%s close error[%s]", VPU_MGR_NAME, strerror(errno));
				}
				ALOGI("VPU for VIDEO has been opened. forcingly return!!");

				return -1;
		#else
				//in case video open!!
				DBUG_FLIP("VPU for VIDEO has been opened. So wait for 3 sec and check per 100ms!!");
				vpu_wait = 30;
				while(vpu_wait > 0)
				{				
					usleep(100 * 1000);//100ms
					ioctl(temp_vpu_mgr_fd, VPU_GET_OPEN_INFO, &gInfo);
					vpu_wait--;

					if(gInfo.vid_opened == 0)
					{
						DBUG_FLIP("VPU for VIDEO stopped.");
						break;
					}
					
					if(vpu_wait <= 0)
					{
						ALOGE("VPU for VIDEO didn't close!!. so assume to close and reset vpu!!");
						
						sInfo.type = OPEN_VIDEO;
						sInfo.opened_cnt = 0;
						ioctl(temp_vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
						
						ret_vpu_reset = 1;
						break;
					}
				}
		#endif
			}
		}
	}

	sInfo.type = OPEN_VIDEO;
	if(open_status)
	{
		sInfo.opened_cnt = 1;
		
		vpu_mgr_fd = temp_vpu_mgr_fd;
		vpu_dec_fd = temp_vpu_dec_fd;
		
		ioctl(vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
		ioctl(vpu_dec_fd, DEVICE_INITIALIZE, NULL);
	}
	else
	{
		sInfo.opened_cnt = 0;		
		ioctl(vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
	}


	if(ret_vpu_reset)
	{
		DBUG_FLIP("VDEC has to reset because other application was opened vpu (ex. DMB).!!");
		ioctl(vpu_mgr_fd, VPU_HW_RESET, (void*)NULL);
	}
	
	return 0;
}
#endif

static int vpu_env_open(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height)
{
	int vpu_reset = 0;
		
	DSTATUS("In  %s \n",__func__);

	if((vpu_reset = vpu_check_for_video(1)) < 0)
		goto err;

#ifdef  USE_VPU_INTERRUPT
	vpu_intr_fd = open(TCC_INTR_DEV_NAME, O_RDWR);
	if (vpu_intr_fd < 0) {
		ALOGE("%s open error", TCC_INTR_DEV_NAME);
		goto err;
	}	
#endif

	prev_codec = format;
	vpu_update_sizeinfo(format, bps, fps, image_width, image_height);

#if defined(VPU_CLK_CONTROL)
	gsRegBaseVideoBusAddr	= (unsigned int)sPhysicalMemSetting(0xF0702000, 4*1024);
	if(!gsRegBaseVideoBusAddr)
		goto err;

	gsRegBaseClkCtrlAddr	= (unsigned int)sPhysicalMemSetting(0xF0700000, 4*1024);
	if(!gsRegBaseClkCtrlAddr)
		goto err;
#endif

	vdec_env_opened = 1;

	memset(&gsVpuDecInit_Info.gsVpuDecInit, 0x00, sizeof(dec_init_t));
	memset(&gsVpuDecBuffer_Info.gsVpuDecBuffer, 0x00, sizeof(dec_buffer_t));
	memset(&gsVpuDecInOut_Info.gsVpuDecInput, 0x00, sizeof(dec_input_t));
//	memset(&gsVpuDecUserInfo, 0x00, sizeof(dec_user_info_t));
//	memset(&gsVpuDecInOut_Info.gsVpuDecOutput, 0x00, sizeof(dec_output_t));

	memset(&gsBitstreamBufAddr, 0x00, sizeof(gsBitstreamBufAddr));
	memset(&gsUserdataBufAddr, 0x00, sizeof(gsUserdataBufAddr));
	memset(&gsBitWorkBufAddr, 0x00, sizeof(gsBitWorkBufAddr));
	memset(&gsFrameBufAddr, 0x00, sizeof(gsFrameBufAddr));
	gsSliceBufAddr = gsSpsPpsAddr = 0;

#ifdef INCLUDE_WMV78_DEC
	memset(&decoded_phyAddr, 0x00, sizeof(decoded_phyAddr));
	memset(&decoded_virtAddr, 0x00, sizeof(decoded_virtAddr));
#endif

	DSTATUS("Out  %s \n",__func__);

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	pFs = NULL;
	is1st_dec = 1;
	stream_index = 0;
	backup_data = NULL;
#endif
#ifdef DEBUG_TIME_LOG	
	time_cnt = 0;
	total_dec_time = 0;
#endif
	total_frm = 0;

	return 0;

err:	
	ALOGE("vpu_env_open error");
	vpu_env_close();
	
	return -1;	
}


static void vpu_env_close(void)
{
	DSTATUS("In  %s \n",__func__);

#if defined(VPU_CLK_CONTROL)
	if(gsRegBaseVideoBusAddr)
		sPhysicalMemFree(gsRegBaseVideoBusAddr, 4*1024);

	if(gsRegBaseClkCtrlAddr)
		sPhysicalMemFree(gsRegBaseClkCtrlAddr, 4*1024);

	gsRegBaseVideoBusAddr = 0;
	gsRegBaseClkCtrlAddr = 0;	
#endif

#ifdef  USE_VPU_INTERRUPT
	if(vpu_intr_fd > 0)
	{
		if(close(vpu_intr_fd) < 0)
		{
			ALOGE("%s close error", TCC_INTR_DEV_NAME);
		}
		vpu_intr_fd = -1;
	}
#endif	

	if(vpu_dec_fd)
	{
		if(close(vpu_dec_fd) < 0)
		{
			ALOGE("%s close error", vpu_devices[nInstance]);
		}
		vpu_dec_fd = -1;
	}

	vdec_env_opened = 0;
	
#ifndef VPU_FOR_CAMERA	
	vpu_check_for_video(0);
#endif

	if(vpu_mgr_fd)
	{
		ioctl(vpu_mgr_fd, VPU_CLEAR_INSTANCE_IDX, &nInstance);
		if(close(vpu_mgr_fd) < 0)
		{
			ALOGE("%s close error", VPU_MGR_NAME);
		}
		vpu_mgr_fd = -1;
	}

#if defined(VPU_OUT_FRAME_DUMP) || defined(CHANGE_INPUT_STREAM)
	if(pFs){
		fclose(pFs);
		pFs = NULL;
	}

	if(backup_data){
		vdec_free((void *) backup_data);
		backup_data = NULL;
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

static int VpuInterrupt()
{
	int success = 0;
	
	while (1) {
		int ret;
		memset(tcc_event, 0, sizeof(tcc_event));
		tcc_event[0].fd = vpu_intr_fd;
		tcc_event[0].events = POLLIN;
		
		ret = poll((struct pollfd *)&tcc_event, 1, 500); // 500 msec
		if (ret < 0) {
			ALOGE("vpu poll error\n");
			break;
		}else if (ret == 0) {
			ALOGE("vpu poll timeout\n");
			break;
		}else if (ret > 0) {
			if (tcc_event[0].revents & POLLERR) {
				ALOGE("vpu poll POLLERR\n");
				break;
			} else if (tcc_event[0].revents & POLLIN) {
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

static int vdec_cmd_process(int cmd, void* args)
{
	int ret;
	int success = 0;

	if(ioctl(vpu_dec_fd, cmd, args) < 0)
	{
		ALOGE("vpu ioctl err[%s] : cmd = 0x%x", strerror(errno), cmd);
	}
	
	while (1) {
		memset(tcc_event, 0, sizeof(tcc_event));
		tcc_event[0].fd = vpu_dec_fd;
		tcc_event[0].events = POLLIN;
		
		ret = poll((struct pollfd *)&tcc_event, 1, 500); // 100 msec
		if (ret < 0) {
			ALOGE("vpu poll error\n");
			break;
		}else if (ret == 0) {
			ALOGE("vpu poll timeout: %d'th frames", total_frm);
			break;
		}else if (ret > 0) {
			if (tcc_event[0].revents & POLLERR) {
				ALOGE("vpu poll POLLERR\n");
				break;
			} else if (tcc_event[0].revents & POLLIN) {
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
				
				ioctl(vpu_dec_fd, V_DEC_INIT_RESULT, args);
				ret = init_info->result;
			}
			break;
			
		case V_DEC_SEQ_HEADER: 
			{			 
			 	VDEC_SEQ_HEADER_t* seq_info = args;
				
				ioctl(vpu_dec_fd, V_DEC_SEQ_HEADER_RESULT, args);
				ret = seq_info->result;
			}
			break;
			
		case V_DEC_DECODE:
			{			 
			 	VDEC_DECODE_t* decoded_info = args;
				
				ioctl(vpu_dec_fd, V_DEC_DECODE_RESULT, args);
				ret = decoded_info->result;
			}
			break;
			
		case V_DEC_REG_FRAME_BUFFER:			
		case V_DEC_BUF_FLAG_CLEAR:
		case V_DEC_CLOSE:
		case V_DEC_GET_INFO:			
		case V_DEC_REG_FRAME_BUFFER2:
		case V_DEC_FLUSH_OUTPUT:
		case V_GET_RING_BUFFER_STATUS:
		case V_FILL_RING_BUFFER_AUTO:
		case V_GET_INITIAL_INFO_FOR_STREAMING_MODE_ONLY:
		default:
			ioctl(vpu_dec_fd, V_DEC_GENERAL_RESULT, &ret);
			break;			
	}

	if(!success || (ret == RETCODE_CODEC_EXIT))
	{	
		if(cmd == V_DEC_DECODE)
		{
			ALOGE("VDEC has to reset.!!");
//			ioctl(vpu_mgr_fd, VPU_HW_RESET, (void*)NULL);
			
			return RETCODE_CODEC_EXIT;
		}
	}

	return ret;
}

int vpu_get_frame_count(int safety_count)
{
#ifdef VPU_FOR_CAMERA
	return 0;
#else
	int fifo_frame = (gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount - gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount);

#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG) || defined(JPEG_DECODE_FOR_MJPEG)
	if( gsVpuDecInit.m_iBitstreamFormat == STD_MJPG)
		return 0;
#endif
	
	if(fifo_frame < safety_count)
	{
		ALOGE("CAUTION!! fifo_frame count is very low. keep 0x%x frame!!", safety_count);
	}

	return fifo_frame;
#endif
}

static void save_input_stream(char* name, int size)
{
#ifdef VPU_IN_FRAME_DUMP
	int i;
	unsigned char* ps = (unsigned char*)gsBitstreamBufAddr[VA];

	if(0)
	{
		FILE *fp;
		fp = fopen(name, "ab+");		  
		fwrite( ps, size, 1, fp);
		fclose(fp);
	}

	for(i=0; (i+10 <size) && (i+10 < 100); i += 10){
		DPRINTF_FRAME( "[VDEC - Stream] 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", ps[i], ps[i+1], ps[i+2], ps[i+3], ps[i+4], ps[i+5], ps[i+6], ps[i+7], ps[i+8], ps[i+9] );
	}
#endif	
}		

static void save_decoded_frame(unsigned char* Y, unsigned char* U, unsigned char *V, int width, int height)
{
#ifdef VPU_OUT_FRAME_DUMP
	if(!pFs){
		pFs = fopen("data/frame.yuv", "ab+");
		if (!pFs) {
			ALOGE("Cannot open 'data/frame.yuv'");
			return;
		}
	}
	
	if(pFs){
		fwrite( Y, width*height, 1, pFs);
		fwrite( U, width*height/4, 1, pFs);
		fwrite( V, width*height/4, 1, pFs);
	}
#endif	
}	

static void change_input_stream(unsigned char* out, int* len, int type)
{
#ifdef CHANGE_INPUT_STREAM
#if 1 //test
	if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_H263)
		return;
#endif

	if(type == VDEC_DECODE && is1st_dec)
	{
		is1st_dec = 0;
		*len = received_len[stream_index-1];
		memcpy( out, backup_data, received_len[stream_index-1]);
		ALOGD("DEC => read[%d] - [%p] :: %p %p %p %p %p %p %p %p %p %p", stream_index -1, *len, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9]);
		return;
	}

	if(!pFs)
	{
		int len_count;
		pFs = fopen(STREAM_LEN_NAME, "rb");
		if (!pFs) 
		{
			ALOGE("Cannot open '%s'", STREAM_NAME);
			return;
		}

		memset(received_len, 0x00, sizeof(received_len));
		fread((unsigned char*)received_len, (unsigned int)(STREAM_MAX_IDX*4), 1, pFs);
		fclose(pFs);
		pFs = NULL;
	}

	if(!pFs)
	{
		pFs = fopen(STREAM_NAME, "rb");
		if (!pFs) 
		{
			ALOGE("Cannot open '%s'", STREAM_NAME);
			return;
		}
	}

	if(pFs && STREAM_MAX_IDX > stream_index)
	{
		if(received_len[stream_index])
		{
			fread( out, received_len[stream_index], 1, pFs);
			*len = received_len[stream_index];
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

		if(backup_data == NULL)
			backup_data = vdec_malloc(*len + 102400);

		if(is1st_dec && backup_data != NULL)
			memcpy(backup_data, out, *len);

		ALOGD("read[%d] - [%p] :: %p %p %p %p %p %p %p %p %p %p", stream_index, *len, out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7], out[8], out[9]);
		stream_index++;
	}
#endif	
}

unsigned char *vpu_getBitstreamBufAddr(unsigned int index)
{
	unsigned char *pBitstreamBufAddr = NULL;

	if (index == PA)
	{
		pBitstreamBufAddr = (unsigned char *)gsBitstreamBufAddr[PA];
	}
	else if (index == VA)
	{
		pBitstreamBufAddr = (unsigned char *)gsBitstreamBufAddr[VA];
	}
	else /* default : PA */
	{
		pBitstreamBufAddr = (unsigned char *)gsBitstreamBufAddr[PA];
	}

	return pBitstreamBufAddr;
}

unsigned char *vpu_getFrameBufVirtAddr(unsigned char *convert_addr, unsigned int base_index)
{
	unsigned char *pBaseAddr; 
	unsigned char *pTargetBaseAddr = NULL;
	unsigned int szAddrGap = 0;

	pTargetBaseAddr = (unsigned char*)gsFrameBufAddr[VA];
	
	if (base_index == K_VA)
	{
		pBaseAddr = (unsigned char*)gsFrameBufAddr[K_VA];
	}
	else /* default : PA */
	{
		pBaseAddr = (unsigned char*)gsFrameBufAddr[PA];
	}

	szAddrGap = convert_addr - pBaseAddr;
	
	return (pTargetBaseAddr+szAddrGap);
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
#ifndef VPU_FOR_CAMERA
		(pDecInit->m_iBitstreamFormat != STD_WMV78) &&
#endif
		(pDecInit->m_iBitstreamFormat != STD_MJPG)
	)
	{
		DSTATUS("[VDEC]%s\n\r", strProfile[pDecInit->m_iBitstreamFormat][profile]);
		if (pDecInit->m_iBitstreamFormat != STD_RV) { // No level information in Rv.
			if (pDecInit->m_iBitstreamFormat == STD_AVC && level != 0 && level != 2){
				DSTATUS("[VDEC]%s%.1f\n\r", strLevel[pDecInit->m_iBitstreamFormat][level], (float)pInitialInfo->m_iLevel/10);
			}
			else{
				DSTATUS("[VDEC]%s\n\r", strLevel[pDecInit->m_iBitstreamFormat][level]);
			}
		}
	}
	
	if(pDecInit->m_iBitstreamFormat == STD_AVC) {
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
			DSTATUS("[VDEC]Aspect Ratio is not present");
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

int
vpu_sorensonH263dec_ready( dec_init_t* psVDecInit )
{
#ifdef HAVE_ANDROID_OS
		unsigned int remapped_addr; //to use in kernel!!
		//------------------------------------------------------------
		//! [x] bitstream buffer for each VPU decoder
		//------------------------------------------------------------
		gsBitstreamBufSize = LARGE_STREAM_BUF_SIZE;
		gsBitstreamBufSize = ALIGNED_BUFF( gsBitstreamBufSize, 4*1024 );
		gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&gsBitstreamBufAddr[K_VA], gsBitstreamBufSize, 0 );
		if( gsBitstreamBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC] bitstream_buf_addr[PA] malloc() failed \n");
			return -1;
		}
		DSTATUS( "[VDEC] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitstreamBufAddr[PA], gsBitstreamBufSize );
		gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsBitstreamBufAddr[PA], gsBitstreamBufSize );
		if( gsBitstreamBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC] bitstream_buf_addr[VA] malloc() failed \n");
			return -1;
		}
		memset( (void*)gsBitstreamBufAddr[VA], 0x00 , gsBitstreamBufSize);
		DSTATUS("[VDEC] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitstreamBufAddr[VA], gsBitstreamBufSize );
	
		psVDecInit->m_BitstreamBufAddr[PA]	= gsBitstreamBufAddr[PA];
		psVDecInit->m_BitstreamBufAddr[VA]	= gsBitstreamBufAddr[K_VA];
		psVDecInit->m_iBitstreamBufSize 	= gsBitstreamBufSize;
#endif

		return 0;
}

int
vpu_dec_ready( dec_init_t* psVDecInit )
{
#ifdef HAVE_ANDROID_OS
	//------------------------------------------------------------
	//! [x] bitstream buffer for each VPU decoder
	//------------------------------------------------------------

	if(gsVpuDecUserInfo.m_bJpegOnly && gsVpuDecUserInfo.jpg_length > LARGE_STREAM_BUF_SIZE)
		gsBitstreamBufSize = ALIGNED_BUFF( gsVpuDecUserInfo.jpg_length, 64*1024 );
	else
		gsBitstreamBufSize = LARGE_STREAM_BUF_SIZE;
	gsBitstreamBufSize = ALIGNED_BUFF( gsBitstreamBufSize, 4*1024 );
	gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&gsBitstreamBufAddr[K_VA], gsBitstreamBufSize, 0 );
	if( gsBitstreamBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] bitstream_buf_addr[PA] malloc() failed \n");
		return -1;
	}
	DSTATUS( "[VDEC] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitstreamBufAddr[PA], gsBitstreamBufSize );
	gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsBitstreamBufAddr[PA], gsBitstreamBufSize );
	if( gsBitstreamBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] bitstream_buf_addr[VA] malloc() failed \n");
		return -1;
	}
	memset( (void*)gsBitstreamBufAddr[VA], 0x00 , gsBitstreamBufSize);
	DSTATUS("[VDEC] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitstreamBufAddr[VA], gsBitstreamBufSize );

	psVDecInit->m_BitstreamBufAddr[PA]	= gsBitstreamBufAddr[PA];
	psVDecInit->m_BitstreamBufAddr[VA]	= gsBitstreamBufAddr[K_VA];	
	psVDecInit->m_iBitstreamBufSize 	= gsBitstreamBufSize;

	if(gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable == 0)
	{
		gsIntermediateBufSize = LARGE_STREAM_BUF_SIZE;
		gsIntermediateBufSize = ALIGNED_BUFF( gsIntermediateBufSize, 4*1024 );
		gsIntermediateBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&gsIntermediateBufAddr[K_VA], gsIntermediateBufSize, BUFFER_ELSE);
		
		if( gsIntermediateBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[VDEC] gsIntermediateBufAddr[PA] malloc() failed \n");
			return -1;
		}
		DSTATUS( "[VDEC] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsIntermediateBufAddr[PA], gsIntermediateBufSize );
		gsIntermediateBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsIntermediateBufAddr[PA], gsIntermediateBufSize);
		if( gsIntermediateBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[VDEC] gsIntermediateBufAddr[VA] malloc() failed \n");
			return -1;
		}
		memset( (void*)gsIntermediateBufAddr[VA], 0x00 , gsIntermediateBufSize);
		DSTATUS("[VDEC] gsIntermediateBufAddr[VA] = 0x%x, 0x%x \n", (codec_addr_t)gsIntermediateBufAddr[VA], gsIntermediateBufSize );
	}
	else
	{
		gsIntermediateBufSize = 0;
		gsIntermediateBufAddr[PA] = 0;
		gsIntermediateBufAddr[VA] = 0;
		gsIntermediateBufAddr[K_VA] = 0;
	}

	/* Set the maximum size of input bitstream. */
//	gsMaxBitstreamSize = MAX_BITSTREAM_SIZE;
//	gsMaxBitstreamSize = ALIGNED_BUFF(gsMaxBitstreamSize, (4 * 1024));
//	if (gsMaxBitstreamSize > gsBitstreamBufSize)
//	{
		gsMaxBitstreamSize = gsBitstreamBufSize;
//	}
#endif

	//------------------------------------------------------------
	//! [x] user data buffer for each VPU decoder
	//------------------------------------------------------------
	if(gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
	{
		gsUserdataBufSize = 50 * 1024;
		gsUserdataBufSize = ALIGNED_BUFF( gsUserdataBufSize, 4*1024 );
		gsUserdataBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&gsUserdataBufAddr[K_VA], gsUserdataBufSize, 0 );
		if( gsUserdataBufAddr[PA] == 0 ) 
		{
			DPRINTF( "[CDK_CORE:Err%d] gsUserdataBufAddr physical alloc failed \n", -1 );
			return -1;
		}
		DSTATUS( "[CDK_CORE] gsUserdataBufAddr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsUserdataBufAddr[PA], gsUserdataBufSize );
		gsUserdataBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsUserdataBufAddr[PA], gsUserdataBufSize );
		if( gsUserdataBufAddr[VA] == 0 ) 
		{
			DPRINTF( "[CDK_CORE:Err%d] gsUserdataBufAddr virtual alloc failed \n", -1 );
			return -1;
		}
		//memset( (void*)gsUserdataBufAddr[VA], 0 , gsUserdataBufSize);
		DSTATUS("[CDK_CORE] gsUserdataBufAddr[VA] = 0x%x, 0x%x \n", (codec_addr_t)gsUserdataBufAddr[VA], gsUserdataBufSize );
	}
	
	//------------------------------------------------------------
	// [x] code buffer, work buffer and parameter buffer for VPU 
	//------------------------------------------------------------
	gsBitWorkBufSize = WORK_CODE_PARA_BUF_SIZE;
	gsBitWorkBufSize = ALIGNED_BUFF(gsBitWorkBufSize, 4*1024);
	gsBitWorkBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&gsBitWorkBufAddr[K_VA], gsBitWorkBufSize, BUFFER_WORK );
	if( gsBitWorkBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] bit_work_buf_addr[PA] malloc() failed \n");
		return -1;
	}
	DSTATUS("[VDEC] bit_work_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitWorkBufAddr[PA], gsBitWorkBufSize );
	gsBitWorkBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsBitWorkBufAddr[PA], gsBitWorkBufSize );
	if( gsBitWorkBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] bit_work_buf_addr[VA] malloc() failed \n");
		return -1;
	}
	DSTATUS("[VDEC] bit_work_buf_addr[VA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitWorkBufAddr[VA], gsBitWorkBufSize );

	//------------------------------------------------------------
	// [x] PS(SPS/PPS) buffer for each VPU decoder
	//------------------------------------------------------------
	if( psVDecInit->m_iBitstreamFormat == STD_AVC )
	{
		gsSpsPpsSize = PS_SAVE_SIZE;
		gsSpsPpsSize = ALIGNED_BUFF( gsSpsPpsSize, 4*1024 );
		gsSpsPpsAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, gsSpsPpsSize, 0 );
		if( gsSpsPpsAddr == 0 ) 
		{
			DPRINTF( "[VDEC] sps_pps_buf_addr malloc() failed \n");
			return -1;
		}
		DSTATUS("[VDEC] sps_pps_buf_addr = 0x%x, 0x%x \n", (codec_addr_t)gsSpsPpsAddr, gsSpsPpsSize );

		psVDecInit->m_pSpsPpsSaveBuffer = (unsigned char*)gsSpsPpsAddr;
		psVDecInit->m_iSpsPpsSaveBufferSize = gsSpsPpsSize;
	}

	psVDecInit->m_BitWorkAddr[PA] = gsBitWorkBufAddr[PA];
	psVDecInit->m_BitWorkAddr[VA] = gsBitWorkBufAddr[K_VA];
	if( psVDecInit->m_bEnableVideoCache == 0 ){
		DSTATUS("[VDEC] Cache OFF\n");
	}
	else{
		DSTATUS("[VDEC] Cache ON\n");
	}

	psVDecInit->m_bCbCrInterleaveMode = psVDecInit->m_bCbCrInterleaveMode;
	if( psVDecInit->m_bCbCrInterleaveMode == 0 ){
		DSTATUS("[VDEC] CbCrInterleaveMode OFF\n");
	}
	else{
		DSTATUS("[VDEC] CbCrInterleaveMode ON\n");
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
vpu_dec_seq_header( int iSize , dec_initial_info_t * psInitialInfo, int iIsThumbnail )
{
	int ret = 0;

	DSTATUS("vpu_dec_seq_header in :: size(%d), JpegOnly(%d), format(%d)", iSize ,gsVpuDecUserInfo.m_bJpegOnly, gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat);
	gsVpuDecSeqHeader_Info.stream_size = iSize;
	ret = vdec_cmd_process(V_DEC_SEQ_HEADER, &gsVpuDecSeqHeader_Info);
	if( ret != RETCODE_SUCCESS )
	{
	#ifndef HAVE_ANDROID_OS
		psInitialInfo->m_iReportErrorReason = gsVpuDecInitialInfo.m_iReportErrorReason;
	#endif
		DPRINTF( "[VDEC] VPU_DEC_SEQ_HEADER failed Error code is 0x%x . ErrorReason is %d\n", ret, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iReportErrorReason );
		return -ret;
	}
	
#ifndef HAVE_ANDROID_OS
	if(psInitialInfo != NULL)
		cdk_memcpy(psInitialInfo, &gsVpuDecInitialInfo, sizeof(dec_initial_info_t));
#endif
	print_dec_initial_info( &gsVpuDecInit_Info.gsVpuDecInit, &gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo );

	//------------------------------------------------------------
	// [x] slice buffer for VPU
	//------------------------------------------------------------
	if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_AVC )
	{
		gsSliceBufSize = SLICE_SAVE_SIZE;
		gsSliceBufSize = ALIGNED_BUFF( gsSliceBufSize, 4*1024 );
		gsSliceBufAddr = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, gsSliceBufSize, 0 );
		if( gsSliceBufAddr == 0 ) 
		{
			DPRINTF( "[VDEC] slice_buf_addr malloc() failed \n");
			return CDK_ERR_MALLOC;
		}
		DSTATUS("[VDEC] slice_buf_addr = 0x%x, 0x%x \n", (codec_addr_t)gsSliceBufAddr, gsSliceBufSize );

		gsVpuDecBuffer_Info.gsVpuDecBuffer.m_AvcSliceSaveBufferAddr  = gsSliceBufAddr;
		gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iAvcSliceSaveBufferSize = gsSliceBufSize;
	}		
	else
	{
		gsSliceBufSize = 0;
		gsSliceBufAddr = 0;
	}

	//------------------------------------------------------------
	// [x] frame buffer for each VPU decoder
	//------------------------------------------------------------
	gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;
	DSTATUS( "[VDEC] FrameBufDelay %d\n", gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iFrameBufDelay );
	DSTATUS( "[VDEC] MinFrameBufferCount %d\n", gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount );
#ifdef HAVE_ANDROID_OS
#ifdef NEED_SPECIFIC_PROCESS_FOR_MJPEG
	if(gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG)
	{
		int max_count;

	#if defined(SUPPORT_MANAGE_MJPEG_BUFFER	)
		if(!gsVpuDecUserInfo.m_bJpegOnly)
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + VPU_BUFF_COUNT;
		else
		{
			gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount = 1;
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;// + 2;
		}
	#else
		gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;// + 2;
	#endif

	#ifdef SUPPORT_MJPEG_SCALING	
		if(gsVpuDecUserInfo.m_bJpegOnly)
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio = gsVpuDecUserInfo.jpg_ScaleRatio;
		else
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio = 0;

		DSTATUS("[VDEC] jpeg ratio = %d", gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio);

		if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio == 0)
			max_count = cdk_sys_remain_memory_size() / gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
		else
			max_count = cdk_sys_remain_memory_size() / gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio];
	#else
		max_count = cdk_sys_remain_memory_size() / gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
	#endif

		if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > max_count)
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = max_count;
		
		if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;
		
		if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount)
		{
			ALOGE( "[VDEC] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d]", max_count, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount);
			return CDK_ERR_MALLOC;
		}
	}		
	else
#endif
	{
		int max_count;
		
		gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + VPU_BUFF_COUNT;
		max_count = cdk_sys_remain_memory_size() / gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;

		if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > max_count)
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = max_count;
		
		if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
			gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;

		if(iIsThumbnail)				
		{
			if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount))
			{
				ALOGE( "[VDEC] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d]", max_count, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount);
				return CDK_ERR_MALLOC;
			}
		}
		else
		{
			if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount+VPU_BUFF_COUNT))
			{
				ALOGE( "[VDEC] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d]", max_count, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + VPU_BUFF_COUNT);
				return CDK_ERR_MALLOC;
			}
		}
	}
#else	
	gsVpuDecBuffer.m_iFrameBufferCount = gsVpuDecInitialInfo.m_iMinFrameBufferCount;
#endif

#ifdef SUPPORT_MJPEG_SCALING	
	if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio != 0 && gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG){
		gsFrameBufSize = gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount * gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio];
		ALOGD( "[VDEC] FrameBufferCount %d [min %d], min_size = %d \n", gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio]);
	}
	else
#endif
	{
		gsFrameBufSize = gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount * gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
		ALOGD( "[VDEC] FrameBufferCount %d [min %d], min_size = %d \n", gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);
	}

	gsFrameBufSize = ALIGNED_BUFF( gsFrameBufSize, 4*1024 );
	gsFrameBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&gsFrameBufAddr[K_VA], gsFrameBufSize, 0 );
	if( gsFrameBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] frame_buf_addr[PA] malloc() failed \n");
		return CDK_ERR_MALLOC;
	}	

#ifdef SUPPORT_MJPEG_SCALING	
	if(gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio != 0 && gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG){
		DSTATUS( "[VDEC] MinFrameBufferSize %d bytes \n", gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_MinFrameBufferSize[gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iMJPGScaleRatio] );
	}
	else
#endif
	{
		DSTATUS( "[VDEC] MinFrameBufferSize %d bytes \n", gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize );
	}
	DSTATUS( "[VDEC] frame_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsFrameBufAddr[PA], gsFrameBufSize );
	gsFrameBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsFrameBufAddr[PA], gsFrameBufSize );
	if( gsFrameBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] frame_buf_addr[VA] malloc() failed \n");
		return CDK_ERR_MALLOC;
	}
	DSTATUS("[VDEC] frame_buf_addr[VA] = 0x%x, frame_buf_addr[K_VA] = 0x%x \n", (codec_addr_t)gsFrameBufAddr[VA], gsFrameBufAddr[K_VA] );

	gsVpuDecBuffer_Info.gsVpuDecBuffer.m_FrameBufferStartAddr[PA] = gsFrameBufAddr[PA];
	gsVpuDecBuffer_Info.gsVpuDecBuffer.m_FrameBufferStartAddr[VA] = gsFrameBufAddr[K_VA];

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

	ret = vdec_cmd_process(V_DEC_REG_FRAME_BUFFER, &gsVpuDecBuffer_Info);

	if( ret != RETCODE_SUCCESS )
	{
		DPRINTF( "[VDEC] DEC_REG_FRAME_BUFFER failed Error code is 0x%x \n", ret );
		return -ret;
	}
	DSTATUS("[VDEC] TCC_VPU_DEC VPU_DEC_REG_FRAME_BUFFER OK!\n");
	return ret;
}

int
vdec_vpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
	int ret = 0;

	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !vdec_codec_opened)
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

		gsVpuDecUserInfo.bitrate_mbps = p_input_user_param->bitrate_mbps;
		gsVpuDecUserInfo.frame_rate   = p_input_user_param->frame_rate;
		gsVpuDecUserInfo.m_bJpegOnly  = p_input_user_param->m_bJpegOnly;	
		gsVpuDecUserInfo.jpg_ScaleRatio  = p_input_user_param->jpg_ScaleRatio;	
		gsVpuDecUserInfo.jpg_length  = p_input_user_param->jpg_length;
		
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, p_input_user_param->bitrate_mbps, p_input_user_param->frame_rate, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight ) < 0)
			return -VPU_ENV_INIT_ERROR;	
#endif

#if defined(VPU_CLK_CONTROL)
		vpu_clock_init();
#endif

#ifdef HAVE_ANDROID_OS
		gsVpuDecInit_Info.gsVpuDecInit.m_RegBaseVirtualAddr = (unsigned int)NULL;
		gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat		= p_input_param->m_iBitstreamFormat;
		gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData		= p_input_param->m_bEnableUserData;
		gsVpuDecInit_Info.gsVpuDecInit.m_bEnableVideoCache	= p_input_param->m_bEnableVideoCache;
		gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;
		gsVpuDecInit_Info.gsVpuDecInit.m_Memcpy				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		gsVpuDecInit_Info.gsVpuDecInit.m_Memset				= (void (*) ( void*, int, unsigned int ))memset;
		gsVpuDecInit_Info.gsVpuDecInit.m_Interrupt			= (int	(*) ( void ))NULL;	
#else
		gsVpuDecInit.m_RegBaseVirtualAddr	= p_input_param->m_RegBaseAddr;
		gsVpuDecInit.m_iBitstreamFormat		= p_input_param->m_iBitstreamFormat;
		gsVpuDecInit.m_BitstreamBufAddr[PA] = p_input_param->m_BitstreamBufAddr[PA];
		gsVpuDecInit.m_BitstreamBufAddr[VA] = p_input_param->m_BitstreamBufAddr[VA];
		gsVpuDecInit.m_iBitstreamBufSize	= p_input_param->m_iBitstreamBufSize;
		gsVpuDecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		gsVpuDecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		gsVpuDecInit.m_bEnableUserData		= p_input_param->m_bEnableUserData;
		gsVpuDecInit.m_bEnableVideoCache	= p_input_param->m_bEnableVideoCache;
		gsVpuDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;

		gsVpuDecInit.m_uiDecOptFlags = 0;
		if( p_input_param->m_iBitstreamFormat == STD_MPEG4 )
		{
			if( p_input_param->m_bM4vDeblk )
				gsVpuDecInit.m_uiDecOptFlags |= M4V_DEBLK_ENABLE;
		}
		if( p_input_param->m_uiDecOptFlags )
		{
			if( p_input_param->m_uiDecOptFlags & 0x2 )
			{
				gsVpuDecInit.m_uiDecOptFlags |= M4V_GMC_FRAME_SKIP;
			}
			if( p_input_param->m_uiDecOptFlags >> 16 )
			{
				gsVpuDecInit.m_uiDecOptFlags |= ( p_input_param->m_uiDecOptFlags & 0xFFFF0000 );
			}
		}

		if( p_input_param->m_uiMaxResolution == 2 )
		{
			gsVpuDecInit.m_iMaxResolution = RESOLUTION_625_SD;
		}
		else if( p_input_param->m_uiMaxResolution == 1 )
		{
			gsVpuDecInit.m_iMaxResolution = RESOLUTION_720P_HD;
		}
		else
		{
			gsVpuDecInit.m_iMaxResolution = RESOLUTION_1080_HD;
		}

		gsVpuDecInit.m_Memcpy				= p_input_param->m_pfMemcpy;
		gsVpuDecInit.m_Memset				= p_input_param->m_pfMemset;
		gsVpuDecInit.m_Interrupt			= p_input_param->m_pfInterrupt;
#endif

#ifdef VPU_PERFORMANCE_UP
		//gsVpuDecInit.m_bEnableVideoCache	= 1;
	#if defined(TCC_93XX_INCLUDE)
		gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC93XX;		// use secAXI SRAM0 128K
	#elif defined(TCC_88XX_INCLUDE)
		if((gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight * gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth) > (1280*720))
			gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC88XX;	// use secAXI SRAM0 80K
		else
			gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_DISABLE;

		if(gsVpuDecUserInfo.m_bJpegOnly)
			gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_ENABLE_TCC88XX;	// use secAXI SRAM0 80K
	#endif
#endif

		gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable		= p_input_param->m_bFilePlayEnable;
		// 현재는 새로운 VPU Firmware가 완벽히 test되지 않음.
		gsbHasSeqHeader = 0;//p_input_param->m_bHasSeqHeader; 

		vpu_dec_ready( &gsVpuDecInit_Info.gsVpuDecInit );

		DSTATUS("workbuff 0x%x/0x%x, Reg: 0x%x, format : %d, Stream(0x%x/0x%x, %d), Res: %d x %d", 
					gsVpuDecInit_Info.gsVpuDecInit.m_BitWorkAddr[PA], gsVpuDecInit_Info.gsVpuDecInit.m_BitWorkAddr[VA], gsVpuDecInit_Info.gsVpuDecInit.m_RegBaseVirtualAddr,
					gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[VA], gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize,
					gsVpuDecInit_Info.gsVpuDecInit.m_iPicWidth, gsVpuDecInit_Info.gsVpuDecInit.m_iPicHeight);
		DSTATUS("optFlag 0x%x, avcBuff: 0x%x- %d, Userdata(%d), VCache: %d, Inter: %d, PlayEn: %d, MaxRes: %d", 
					gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags, gsVpuDecInit_Info.gsVpuDecInit.m_pSpsPpsSaveBuffer, gsVpuDecInit_Info.gsVpuDecInit.m_iSpsPpsSaveBufferSize,
					gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData, gsVpuDecInit_Info.gsVpuDecInit.m_bEnableVideoCache, gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode,
					gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable, gsVpuDecInit_Info.gsVpuDecInit.m_iMaxResolution);

	
		DSTATUS("Format : %d, Stream(0x%x, %d)", gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize);
		ret = vdec_cmd_process(V_DEC_INIT, &gsVpuDecInit_Info);
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] VPU_DEC_INIT failed Error code is 0x%x \n", ret );
			return -ret;
		}
		ALOGI( "[VDEC] VPU_DEC_INIT OK( has seq = %d) \n", gsbHasSeqHeader );

		gsVpuDecVersion.pszVersion = gsBitstreamBufAddr[K_VA] + (gsBitstreamBufSize - 100);
		gsVpuDecVersion.pszBuildData = gsBitstreamBufAddr[K_VA] + (gsBitstreamBufSize - 50);

#if !defined(_TCC8800_) && !defined(TCC_8925S_INCLUDE) && !defined(TCC_8935S_INCLUDE)
		ret = vdec_cmd_process(V_GET_VPU_VERSION, &gsVpuDecVersion);
		if( ret != RETCODE_SUCCESS )
		{
			//If this operation returns fail, it doesn't mean that there's a problem in vpu
			//so do not return error to host.
			DPRINTF( "[VDEC] V_GET_VPU_VERSION failed Error code is 0x%x \n", ret );
		}
		else
		{
			ALOGI( "[VDEC] V_GET_VPU_VERSION OK. Version is %s, and it's built at %s \n",
				gsBitstreamBufAddr[VA] + (gsBitstreamBufSize - 100),
				gsBitstreamBufAddr[VA] + (gsBitstreamBufSize - 50));
		}
#endif

		vdec_codec_opened = 1;

#ifdef VPU_REGISTER_DUMP
		bFirst_frame = 1;
		filewrite_memory("data/after_init.bin", gsRegisterBase, 0x200);
#endif
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = (p_input_param->m_iInpLen > gsMaxBitstreamSize) ? gsMaxBitstreamSize : p_input_param->m_iInpLen;
		unsigned int iIsThumbnail = p_input_param->m_iIsThumbnail;

		if(gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable)
		{
			if (    ((codec_addr_t)p_input_param->m_pInp[PA] == gsBitstreamBufAddr[PA])
			     && ((codec_addr_t)p_input_param->m_pInp[VA] == gsBitstreamBufAddr[VA]) )
			{
				gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = gsBitstreamBufAddr[PA];
				gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = gsBitstreamBufAddr[K_VA];
			}
			else
			{
				gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = gsBitstreamBufAddr[PA];
				gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = gsBitstreamBufAddr[K_VA];		
				memcpy( (void*)gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], seq_stream_size);
				change_input_stream((unsigned char *)gsBitstreamBufAddr[VA], &seq_stream_size, iOpCode);
			}

			save_input_stream("/sdcard/vpu_inSeq.bin", seq_stream_size);
			
			unsigned char* ps = (unsigned char*)gsBitstreamBufAddr[VA];
			ALOGI( "[VDEC - Seq] 0x%x 0x%x 0x%x 0x%x 0x%x", ps[0], ps[1], ps[2], ps[3], ps[4] );
		}
		else
		{
			seq_stream_size = 1;
		}
#else
		int seq_stream_size = (int)pParam1;
#endif

		DSTATUS( "[VDEC] VDEC_DEC_SEQ_HEADER start  :: len = %d / %d \n", seq_stream_size, p_input_param->m_iInpLen);
		ret = vpu_dec_seq_header(seq_stream_size, NULL/*pParam2*/, iIsThumbnail);
		if( ret != RETCODE_SUCCESS )
		{
			return ret;
		}
#ifdef HAVE_ANDROID_OS
		gsbHasSeqHeader = 1;
		p_output_param->m_pInitialInfo = &gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
#endif
		//check the maximum/minimum video resolution limitation
		if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG )
		{
#ifdef HAVE_ANDROID_OS
			vdec_info_t * pVdecInfo = (vdec_info_t *)&gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
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
				DPRINTF( "[VDEC] VDEC_DEC_SEQ_HEADER - don't support the resolution %dx%d  \n", 
									pVdecInfo->m_iPicWidth, pVdecInfo->m_iPicHeight);
				return ret;
			}
		}
		else //MJPEG
		{
#ifdef HAVE_ANDROID_OS
			vdec_info_t * pVdecInfo = (vdec_info_t *)&gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;
#else		
			vdec_info_t * pVdecInfo = (vdec_info_t *)pParam2;
#endif
			
			if(  (pVdecInfo->m_iPicWidth > 8192)		\
				|| (pVdecInfo->m_iPicHeight > 8192) )
			{
				ret = 0 - RETCODE_INVALID_STRIDE;
				DSTATUS( "[VDEC] VDEC_DEC_SEQ_HEADER - don't support the resolution %dx%d  \n", 
									pVdecInfo->m_iPicWidth, pVdecInfo->m_iPicHeight);
				return ret;
			}

			if( gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat != 0 )
			{
				DSTATUS("VPU OutFormat is YUV422SEQ0 ");
			}

	#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG	) && !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
			if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && !gsVpuDecUserInfo.m_bJpegOnly)
			{
				unsigned int buf_idx;
				
				decoded_buf_curIdx = 0;
				decoded_buf_size = pVdecInfo->m_iPicWidth*pVdecInfo->m_iPicHeight*2; //for YUV422
				decoded_buf_size = ALIGNED_BUFF(decoded_buf_size, 4*1024);
				
				for(buf_idx =0; buf_idx < MAX_NUM_OF_VIDEO_ELEMENT; buf_idx++)
				{
					decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, decoded_buf_size, 0 );				
					if( decoded_phyAddr[buf_idx] == 0 ) 
					{
						DPRINTF( "[VDEC,Err:%d] vdec_vpu decoded_virtAddr[PA] alloc failed \n", ret );
						decoded_buf_maxcnt = buf_idx;
						break;
					}	
					decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, decoded_phyAddr[buf_idx], decoded_buf_size ); 
					if( decoded_virtAddr[buf_idx] == 0 ) 
					{
						DPRINTF( "[VDEC,Err:%d] vdec_vpu decoded_virtAddr[VA] alloc failed \n", ret );
						decoded_buf_maxcnt = buf_idx;
						break;
					}
	
					decoded_buf_maxcnt = MAX_NUM_OF_VIDEO_ELEMENT;
					DSTATUS("OUT-Buffer %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, decoded_phyAddr[buf_idx], decoded_virtAddr[buf_idx],	decoded_buf_size);
				}			
			}
	#endif

		}
		
		DSTATUS( "[VDEC] VDEC_DEC_SEQ_HEADER - Success \n" );
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
		gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = (p_input_param->m_iInpLen > gsMaxBitstreamSize) ? gsMaxBitstreamSize : p_input_param->m_iInpLen;

		if (    ((codec_addr_t)p_input_param->m_pInp[PA] == gsBitstreamBufAddr[PA])
		     && ((codec_addr_t)p_input_param->m_pInp[VA] == gsBitstreamBufAddr[VA]) )
		{
			gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = gsBitstreamBufAddr[PA];
			gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = gsBitstreamBufAddr[K_VA];
		}
		else
		{
			gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = gsBitstreamBufAddr[PA];
			gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = gsBitstreamBufAddr[K_VA];
			memcpy( (void*)gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize);
			change_input_stream((unsigned char *)gsBitstreamBufAddr[VA], (&gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize), iOpCode);
		}
		save_input_stream("data/vpu_inDec.bin", gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize);
#else
		gsVpuDecInput.m_iBitstreamDataSize = p_input_param->m_iInpLen;
		gsVpuDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
		gsVpuDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
#endif

		if(gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{		
			gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = gsUserdataBufAddr[PA];
			gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = gsUserdataBufAddr[K_VA];
			gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = gsUserdataBufSize;
		}
		
//		gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = (codec_addr_t)p_input_param->m_UserDataAddr[PA];
//		gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = (codec_addr_t)p_input_param->m_UserDataAddr[VA];
//		gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = p_input_param->m_iUserDataBufferSize;
		
		gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = p_input_param->m_iSkipFrameMode;
		gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = p_input_param->m_iFrameSearchEnable;
		gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;
		if( gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode > 0 || gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable > 0 )
		{
			gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = p_input_param->m_iSkipFrameNum;
		}

		// Start decoding a frame.
		ret = vdec_cmd_process(V_DEC_DECODE, &gsVpuDecInOut_Info);

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
			DPRINTF( "[VDEC] VPU_DEC_DECODE failed Error code is 0x%x \n", ret );

			return -ret;
		}

		if(gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iPicType == 0){
			DSTATUS( "[VDEC] I-Frame (%d)", total_frm);
		}
		total_frm++;

#ifdef NEED_SPECIFIC_PROCESS_FOR_MJPEG
		if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && 
#if defined(SUPPORT_MANAGE_MJPEG_BUFFER	)
			gsVpuDecUserInfo.m_bJpegOnly && 
#endif			
			gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus == VPU_DEC_BUF_FULL &&
			gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iOutputStatus == VPU_DEC_OUTPUT_SUCCESS)
		{
			gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;
		}
#endif

#ifdef HAVE_ANDROID_OS
		memcpy((void*)p_output_param, (void*)&gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
	#ifdef NEED_SPECIFIC_PROCESS_FOR_MJPEG		
		if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && gsVpuDecUserInfo.m_bJpegOnly && gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4)
		{
			unsigned int frame_size = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth*gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight;
			
			if(gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode)
			{
				memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA), 0x80, frame_size/2);
			}
			else
			{
				memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA), 0x80, frame_size/4);
				memset( (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA), 0x80, frame_size/4);
			}
		}
		
		if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && !gsVpuDecUserInfo.m_bJpegOnly)
		{
			unsigned int frame_size = gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth*gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight;
			int divide_val = 4; 					
				
			if( gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 1 ) //YUV422
				divide_val = 2;
			
		#if !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
			p_output_param->m_pDispOut[PA][0] = (unsigned char*)decoded_phyAddr[decoded_buf_curIdx];
			p_output_param->m_pDispOut[PA][1] = (unsigned char*)p_output_param->m_pDispOut[PA][0] + frame_size;
			p_output_param->m_pDispOut[PA][2] = (unsigned char*)p_output_param->m_pDispOut[PA][1] + frame_size/divide_val;	
			p_output_param->m_pDispOut[VA][0] = (unsigned char*)decoded_virtAddr[decoded_buf_curIdx];
			p_output_param->m_pDispOut[VA][1] = (unsigned char*)p_output_param->m_pDispOut[VA][0] + frame_size;
			p_output_param->m_pDispOut[VA][2] = (unsigned char*)p_output_param->m_pDispOut[VA][1] + frame_size/divide_val;	

			if(gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode)
			{
				memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][0], vpu_getFrameBufVirtAddr(gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][0], K_VA), frame_size);

				if( gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4) //YUV400
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][1], 0x80, frame_size/divide_val);
				else
					memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][1], vpu_getFrameBufVirtAddr(gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][1], K_VA), frame_size/divide_val);
			}
			else
			{
				memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][0], vpu_getFrameBufVirtAddr(gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][0], K_VA), frame_size);
				if( gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4) //YUV400
				{
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][1], 0x80, frame_size/divide_val);
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][2], 0x80, frame_size/divide_val);
				}
				else
				{					
					memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][1], vpu_getFrameBufVirtAddr(gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][1], K_VA), frame_size/divide_val);
					memcpy( (unsigned char *)p_output_param->m_pDispOut[VA][2], vpu_getFrameBufVirtAddr(gsVpuDecInOut_Info.gsVpuDecOutput.m_pDispOut[VA][2], K_VA), frame_size/divide_val);
				}
			}

			decoded_buf_curIdx++;
			if(decoded_buf_curIdx >= decoded_buf_maxcnt)
				decoded_buf_curIdx = 0;
		#else
			if( gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMjpg_sourceFormat == 4)
			{
				if(gsVpuDecInit_Info.gsVpuDecInit.m_bCbCrInterleaveMode)
				{
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][1], 0x80, frame_size/2);
				}
				else
				{
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][1], 0x80, frame_size/divide_val);
					memset( (unsigned char *)p_output_param->m_pDispOut[VA][2], 0x80, frame_size/divide_val);
				}
			}
		#endif
		}
	#endif
#else
		cdk_memcpy( p_output_param, &gsVpuDecOutput, sizeof(dec_output_t ) );
#endif
		p_output_param->m_pInitialInfo = &gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;

#if !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
		if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat != STD_MJPG)
#endif
		{
			p_output_param->m_pDispOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][0], K_VA);
			p_output_param->m_pDispOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA);
			p_output_param->m_pDispOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA);
		}

		p_output_param->m_pCurrOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][0], K_VA);
		p_output_param->m_pCurrOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][1], K_VA);
		p_output_param->m_pCurrOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pCurrOut[VA][2], K_VA);

		if(gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{	
			unsigned int addr_gap = 0;

			addr_gap = gsUserdataBufAddr[K_VA] - gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_UserDataAddress[VA];
			p_output_param->m_DecOutInfo.m_UserDataAddress[VA] = gsUserdataBufAddr[VA] + addr_gap;
		}
		
		save_decoded_frame((unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][0], K_VA),
							(unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA),
							(unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA),
							gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicWidth, gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iPicHeight);

		save_input_stream("data/vpu_outDec.bin", p_input_param->m_iInpLen);
#ifdef VPU_REGISTER_DUMP
		filewrite_memory("data/after_decode.bin", gsRegisterBase, 0x200);
		bFirst_frame = 0;
#endif
	}
	else if( iOpCode == VDEC_GET_RING_BUFFER_STATUS )
	{
		vdec_ring_buffer_out_t* p_out_param = (vdec_ring_buffer_out_t*)pParam2;

		ret = vdec_cmd_process(V_GET_RING_BUFFER_STATUS, &gsVpuDecBufStatus); // get the available space in the ring buffer
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] GET_RING_BUFFER_STATUS failed Error code is 0x%x \n", ret );
			return -ret;
		}
	#if defined(TCC_88XX_INCLUDE) || defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
		p_out_param->m_ulAvailableSpaceInRingBuffer = gsVpuDecBufStatus.gsVpuDecRingStatus.m_iAvailableSpaceInRingBuffer;
	#else
		p_out_param->m_ulAvailableSpaceInRingBuffer = gsVpuDecBufStatus.gsVpuDecRingStatus.m_ulAvailableSpaceInRingBuffer;
		p_out_param->m_ptrReadAddr_PA = gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrReadAddr_PA;
		p_out_param->m_ptrWriteAddr_PA = gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrWriteAddr_PA;
	#endif
	}
	else if( iOpCode == VDEC_FILL_RING_BUFFER )
	{
		vdec_ring_buffer_set_t* p_set_param = (vdec_ring_buffer_set_t*)pParam1;

		memcpy(gsIntermediateBufAddr[VA], p_set_param->m_pbyBuffer, p_set_param->m_ulBufferSize);
		gsVpuDecBufFill.gsVpuDecRingFeed.m_iOnePacketBufferSize = p_set_param->m_ulBufferSize;
		gsVpuDecBufFill.gsVpuDecRingFeed.m_OnePacketBufferAddr = gsIntermediateBufAddr[K_VA];

		ret = vdec_cmd_process(V_FILL_RING_BUFFER_AUTO, &gsVpuDecBufFill);  // fille the Ring Buffer 

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] FILL_RING_BUFFER_AUTO failed Error code is 0x%x \n", ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_GET_INTERMEDIATE_BUF_INFO )
	{
		*(unsigned int*)pParam1 = gsIntermediateBufAddr[VA];
		*(unsigned int*)pParam2 = gsIntermediateBufSize;
		return 0;
	}
	else if( iOpCode == VDEC_UPDATE_WRITE_BUFFER_PTR )
	{
		gsVpuDecUpdateWP.iCopiedSize = (int)pParam1;
		gsVpuDecUpdateWP.iFlushBuf = (int)pParam2;

		ret = vdec_cmd_process(V_DEC_UPDATE_RINGBUF_WP, &gsVpuDecUpdateWP);  // fille the Ring Buffer 

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] FILL_RING_BUFFER_AUTO failed Error code is 0x%x \n", ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_BUF_FLAG_CLEAR )
	{
		int idx_display = *(int*)pParam1;
		ret = vdec_cmd_process(V_DEC_BUF_FLAG_CLEAR, &idx_display);
		
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] VPU_DEC_BUF_FLAG_CLEAR failed Error code is 0x%x \n", ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_DEC_FLUSH_OUTPUT)
	{
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		//vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
		gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = 0;
		gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = VDEC_SKIP_FRAME_DISABLE;
		gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = 0;
		gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;

		ret = vdec_cmd_process(V_DEC_FLUSH_OUTPUT, &gsVpuDecInOut_Info);

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] VPU_DEC_BUF_FLAG_CLEAR failed Error code is 0x%x \n", ret );
			return -ret;
		}
	}
	else if( iOpCode == VDEC_CLOSE )
	{
		if(!vdec_codec_opened && !vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		if(vdec_codec_opened)
		{		
			ret = vdec_cmd_process(V_DEC_CLOSE, &gsVpuDecInOut_Info);
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC] VPU_DEC_CLOSE failed Error code is 0x%x \n", ret );
				ret = -ret;
			}
			
			vdec_codec_opened = 0;
		}

		if(!vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;
				
		if( gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsBitstreamBufAddr[VA], gsBitstreamBufSize)  >= 0)
			{
				gsBitstreamBufAddr[VA] = 0;
			}
		}
		
		if( gsUserdataBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsUserdataBufAddr[VA], gsUserdataBufSize )  >= 0)
			{
				gsUserdataBufAddr[VA] = 0;
			}
			
		}

		if( gsBitWorkBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsBitWorkBufAddr[VA], gsBitWorkBufSize )  >= 0)
			{
				gsBitWorkBufAddr[VA] = 0;
			}
			
		}

		if( gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsFrameBufAddr[VA], gsFrameBufSize )  >= 0)
			{
				gsFrameBufAddr[VA] = 0;
			}
		}
		
#if defined(NEED_SPECIFIC_PROCESS_FOR_MJPEG	) && !defined(SUPPORT_MANAGE_MJPEG_BUFFER)
		if( gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_MJPG && !gsVpuDecUserInfo.m_bJpegOnly)
		{
			int buf_idx = 0;
			
			for(buf_idx =0; buf_idx < decoded_buf_maxcnt; buf_idx++)
			{
				if(cdk_sys_free_virtual_addr( NULL, (void*)decoded_virtAddr[buf_idx], decoded_buf_size )  >= 0)
				{
					decoded_virtAddr[buf_idx] = 0;
				}
			}			
		}
#endif

		
#if defined(VPU_CLK_CONTROL)
		vpu_clock_deinit();
#endif
#ifdef HAVE_ANDROID_OS
		vpu_env_close();
#endif
	}
	else
	{
		DPRINTF( "Invalid Operation!!\n" );
		return -ret;
	}

#ifdef DEBUG_TIME_LOG
	end = clock();

	if( iOpCode == VDEC_INIT ){
		ALOGD("VDEC_C_INIT_TIME %d ms", (end-start)*1000/CLOCKS_PER_SEC);
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER){
		ALOGD("VDEC_C_SEQ_TIME %d ms", (end-start)*1000/CLOCKS_PER_SEC);
	}
	else if( iOpCode == VDEC_DECODE )
	{
		dec_time[time_cnt] = (end-start)*1000/CLOCKS_PER_SEC;
		total_dec_time += dec_time[time_cnt];
		if(time_cnt != 0 && time_cnt % 29 == 0)
		{
			ALOGD("VDEC_C_DEC_TIME %.1f ms: %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d",
				total_dec_time/(float)total_frm, dec_time[0], dec_time[1], dec_time[2], dec_time[3], dec_time[4], dec_time[5], dec_time[6], dec_time[7], dec_time[8], dec_time[9], 
				dec_time[10], dec_time[11], dec_time[12], dec_time[13], dec_time[14], dec_time[15], dec_time[16], dec_time[17], dec_time[18], dec_time[19], 
				dec_time[20], dec_time[21], dec_time[22], dec_time[23], dec_time[24], dec_time[25], dec_time[26], dec_time[27], dec_time[28], dec_time[29]);
			time_cnt = 0;
		}
		else{
			time_cnt++;
		}
	}
#endif

	return ret;
}

#ifdef INCLUDE_WMV78_DEC
int
WMV78_dec_seq_header()
{
	int i, ret = 0;

	memset( &gsWMV78DecInitialInfo, 0, sizeof(gsWMV78DecInitialInfo) );

	gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropBottom = 0;
	gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropLeft = 0;
	gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropRight = 0;
	gsWMV78DecInitialInfo.m_iAvcPicCrop.m_iCropTop = 0;
	gsWMV78DecInitialInfo.m_iMinFrameBufferCount = 1;
	gsWMV78DecInitialInfo.m_iPicWidth = gsWMV78DecInit.m_iWidth;
	gsWMV78DecInitialInfo.m_iPicHeight = gsWMV78DecInit.m_iHeight;
	gsWMV78DecInitialInfo.m_iAspectRateInfo = 0;
	gsWMV78DecInitialInfo.m_iInterlace = 0;
	print_dec_initial_info( &gsVpuDecInit, &gsWMV78DecInitialInfo );

	return ret;
}

int
vdec_WMV78( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
	int ret = 0;

	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	if( iOpCode == VDEC_INIT )
	{
		unsigned int ChromaSize;
		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;
		vdec_callback_func_t* pf_callback = (vdec_callback_func_t*)pParam2;

#ifdef HAVE_ANDROID_OS
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, 0, 0, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight ) < 0) // to operate Max-clock for s/w codec!!
			return -1;
#endif

		gsWMV78FrameSize = ( (p_input_param->m_iPicWidth+15)&0xfffffff0 ) * ( (p_input_param->m_iPicHeight+15)&0xfffffff0 );
		ChromaSize = gsWMV78FrameSize>>2;
		gsWMV78NCFrameSize = gsWMV78FrameSize*1.5;
		gsWMV78NCFrameSize = ALIGNED_BUFF( gsWMV78NCFrameSize, 4*1024 );

#ifdef HAVE_ANDROID_OS
		gsWMV78CurYFrameAddress = (unsigned int)((unsigned char*)TCC_malloc(gsWMV78NCFrameSize));
		gsWMV78CurUFrameAddress  = (unsigned int)((unsigned char*)gsWMV78CurYFrameAddress + gsWMV78FrameSize);
		gsWMV78CurVFrameAddress  = (unsigned int)((unsigned char*)gsWMV78CurUFrameAddress + ChromaSize);

		gsWMV78Ref0YFrameAddress = (unsigned int)((unsigned char*)TCC_malloc(gsWMV78NCFrameSize));
		gsWMV78Ref0UFrameAddress = (unsigned int)((unsigned char*)gsWMV78Ref0YFrameAddress + gsWMV78FrameSize);
		gsWMV78Ref0VFrameAddress = (unsigned int)((unsigned char*)gsWMV78Ref0UFrameAddress + ChromaSize);
#else
		gsWMV78DecodedFrameAddress_NC[PA]  = (codec_addr_t)cdk_sys_malloc_physical_addr( gsWMV78NCFrameSize );
		gsWMV78DecodedFrameAddress_NC[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( &NULL, gsWMV78DecodedFrameAddress_NC[PA], gsWMV78NCFrameSize );
		if( gsWMV78DecodedFrameAddress_NC[VA] == 0 ) 
		{
			DPRINTF( "[VDEC_WMV78:Err%d] gsWMV78DecodedFrameAddress_NC virtual alloc failed \n", -1 );
			return -1;
		}

		gsWMV78CurYFrameAddress  = (unsigned int)cdk_malloc(sizeof(unsigned char)*gsWMV78FrameSize*1.5);
		gsWMV78CurUFrameAddress  = (unsigned int)((unsigned char*)gsWMV78CurYFrameAddress + gsWMV78FrameSize);
		gsWMV78CurVFrameAddress  = (unsigned int)((unsigned char*)gsWMV78CurUFrameAddress + ChromaSize);
		gsWMV78Ref0YFrameAddress = (unsigned int)cdk_malloc(sizeof(unsigned char)*gsWMV78FrameSize*1.5);
		gsWMV78Ref0UFrameAddress = (unsigned int)((unsigned char*)gsWMV78Ref0YFrameAddress + gsWMV78FrameSize);
		gsWMV78Ref0VFrameAddress = (unsigned int)((unsigned char*)gsWMV78Ref0UFrameAddress + ChromaSize);
#endif
		gsWMV78DecInit.m_pExtraData			= p_input_param->m_pExtraData;
		gsWMV78DecInit.m_iExtraDataLen		= p_input_param->m_iExtraDataLen;
		gsWMV78DecInit.m_iWidth				= (p_input_param->m_iPicWidth+15)&0xfffffff0;
		gsWMV78DecInit.m_iHeight			= p_input_param->m_iPicHeight;
		gsWMV78DecInit.m_iFourCC			= p_input_param->m_iFourCC;
		gsWMV78DecInit.m_pHeapAddress		= (unsigned char*)TCC_malloc(sizeof(unsigned char)*200*1024);
		gsWMV78DecInit.m_pHuff_tbl_address	= (unsigned char*)WMV78_Huff_table;

		gsWMV78DecInit.m_pCurFrameAddress  = (tYUV420Frame_WMV*)TCC_malloc(sizeof(tYUV420Frame_WMV));
		gsWMV78DecInit.m_pRef0FrameAddress = (tYUV420Frame_WMV*)TCC_malloc(sizeof(tYUV420Frame_WMV));
		gsWMV78DecInit.m_pCurFrameAddress->m_pucYPlane		= (unsigned char*)gsWMV78CurYFrameAddress;
		gsWMV78DecInit.m_pCurFrameAddress->m_pucUPlane		= (unsigned char*)gsWMV78CurUFrameAddress;
		gsWMV78DecInit.m_pCurFrameAddress->m_pucVPlane		= (unsigned char*)gsWMV78CurVFrameAddress;
		gsWMV78DecInit.m_pRef0FrameAddress->m_pucYPlane		= (unsigned char*)gsWMV78Ref0YFrameAddress;
		gsWMV78DecInit.m_pRef0FrameAddress->m_pucUPlane		= (unsigned char*)gsWMV78Ref0UFrameAddress;
		gsWMV78DecInit.m_pRef0FrameAddress->m_pucVPlane		= (unsigned char*)gsWMV78Ref0VFrameAddress;
		gsWMV78DecOutput.m_pDecodedData = (tYUV420Frame_WMV*)TCC_malloc(sizeof(tYUV420Frame_WMV));

#ifdef HAVE_ANDROID_OS
		gsWMV78DecInit.m_sCallbackFunc.m_pMalloc			= (void*  (*) ( unsigned int ))TCC_malloc;
		gsWMV78DecInit.m_sCallbackFunc.m_pFree				= (void   (*) ( void* ))TCC_free;
		gsWMV78DecInit.m_sCallbackFunc.m_pMemcpy			= (void*  (*) ( void*, const void*, unsigned int ))memcpy;
		gsWMV78DecInit.m_sCallbackFunc.m_pMemset			= (void  (*) ( void*, int, unsigned int ))memset;
		gsWMV78DecInit.m_sCallbackFunc.m_pRealloc			= (void*  (*) ( void*, unsigned int ))TCC_realloc;
		gsWMV78DecInit.m_sCallbackFunc.m_pMemmove			= (void*  (*) ( void*, const void*, unsigned int ))memmove;

		{
			unsigned int buf_idx;
			
			decoded_buf_curIdx = 0;
			decoded_buf_size = gsWMV78FrameSize* 1.5;
			decoded_buf_size = ALIGNED_BUFF(decoded_buf_size, 4*1024);
			
			for(buf_idx =0; buf_idx < MAX_NUM_OF_VIDEO_ELEMENT; buf_idx++)
			{
				decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, decoded_buf_size, 0 );				
				if( decoded_phyAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC,Err:%d] vdec_vpu decoded_virtAddr[PA] alloc failed \n", ret );
					decoded_buf_maxcnt = buf_idx;
					break;
				}	
				decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, decoded_phyAddr[buf_idx], decoded_buf_size );	
				if( decoded_virtAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC,Err:%d] vdec_vpu decoded_virtAddr[VA] alloc failed \n", ret );
					decoded_buf_maxcnt = buf_idx;
					break;
				}

				decoded_buf_maxcnt = MAX_NUM_OF_VIDEO_ELEMENT;
				DSTATUS("OUT-Buffer %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, decoded_phyAddr[buf_idx], decoded_virtAddr[buf_idx],	decoded_buf_size);
			}			
		}		
#else
		gsWMV78DecInit.m_sCallbackFunc.m_pMalloc			= pf_callback->m_pfMalloc;
		gsWMV78DecInit.m_sCallbackFunc.m_pFree				= pf_callback->m_pfFree;
		gsWMV78DecInit.m_sCallbackFunc.m_pMemcpy			= pf_callback->m_pfMemcpy;
		gsWMV78DecInit.m_sCallbackFunc.m_pMemset			= pf_callback->m_pfMemset;
		gsWMV78DecInit.m_sCallbackFunc.m_pRealloc			= pf_callback->m_pfRealloc;
		gsWMV78DecInit.m_sCallbackFunc.m_pMemmove			= pf_callback->m_pfMemmove;
#endif

		gsVpuDecInit.m_iBitstreamFormat	= p_input_param->m_iBitstreamFormat;

		gsFirstFrame = 1;

		DSTATUS( "[VDEC_WMV78] WMV78_DEC_INIT Enter \n" );
		ret = TCC_WMV78_DEC( VDEC_INIT, &gsWMV78DecHandle, &gsWMV78DecInit, NULL );
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC_WMV78] WMV78_DEC_INIT failed Error code is 0x%x \n", ret );
			return -ret;
		}
		gsIsINITdone = 1;
		vdec_codec_opened = 1;
		DSTATUS( "[VDEC_WMV78] WMV78_DEC_INIT OK \n" );
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{
#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;

		if( gsFirstFrame )
		{
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER start \n" );
			p_output_param->m_pInitialInfo = &gsWMV78DecInitialInfo;
			ret = WMV78_dec_seq_header();
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC_WMV78] vpu_dec_seq_header failed Error code is 0x%x \n", ret );
				return ret;
			}
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER - Success \n" );
			gsFirstFrame = 0;
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

		if( gsFirstFrame )
		{
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER start \n" );
			p_output_param->m_pInitialInfo = &gsWMV78DecInitialInfo;
			ret = WMV78_dec_seq_header();
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC_WMV78] vpu_dec_seq_header failed Error code is 0x%x \n", ret );
				return ret;
			}
			DSTATUS( "[VDEC_WMV78] VDEC_DEC_SEQ_HEADER - Success \n" );
			gsFirstFrame = 0;
		}

		gsWMV78DecInput.m_pPacketBuff = (unsigned char*)p_input_param->m_pInp[VA];
		gsWMV78DecInput.m_iPacketBuffSize = p_input_param->m_iInpLen;

#ifdef HAVE_ANDROID_OS
	#if 0 // temporarily no-use!!
		gsWMV78DecInit.m_pCurFrameAddress->m_pucYPlane		= (unsigned char*)decoded_virtAddr[decoded_buf_curIdx];
		gsWMV78DecInit.m_pCurFrameAddress->m_pucUPlane		= (unsigned char*)gsWMV78DecInit.m_pCurFrameAddress->m_pucYPlane + gsWMV78FrameSize;
		gsWMV78DecInit.m_pCurFrameAddress->m_pucVPlane		= (unsigned char*)gsWMV78DecInit.m_pCurFrameAddress->m_pucUPlane + gsWMV78FrameSize/4;		
	#endif
#endif

		// Start decoding a frame.
		ret = TCC_WMV78_DEC( VDEC_DECODE, &gsWMV78DecHandle, &gsWMV78DecInput, &gsWMV78DecOutput );
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC_WMV78] VDEC_DECODE failed Error code is 0x%x \n", ret );
			return ret;
		}
		
#ifdef HAVE_ANDROID_OS
	#if 1 // temporarily use!!
		memcpy( (unsigned char *)decoded_virtAddr[decoded_buf_curIdx], gsWMV78DecOutput.m_pDecodedData->m_pucYPlane, gsWMV78FrameSize*1.5*sizeof(unsigned char) );
	#endif
		p_output_param->m_pDispOut[0][0] = (unsigned char*)decoded_phyAddr[decoded_buf_curIdx];
		p_output_param->m_pDispOut[0][1] = (unsigned char*)p_output_param->m_pDispOut[0][0] + gsWMV78FrameSize;
		p_output_param->m_pDispOut[0][2] = (unsigned char*)p_output_param->m_pDispOut[0][1] + gsWMV78FrameSize/4;	
		p_output_param->m_pDispOut[1][0] = (unsigned char*)decoded_virtAddr[decoded_buf_curIdx];
		p_output_param->m_pDispOut[1][1] = (unsigned char*)p_output_param->m_pDispOut[1][0] + gsWMV78FrameSize;
		p_output_param->m_pDispOut[1][2] = (unsigned char*)p_output_param->m_pDispOut[1][1] + gsWMV78FrameSize/4;	

		decoded_buf_curIdx++;
		if(decoded_buf_curIdx >= decoded_buf_maxcnt)
			decoded_buf_curIdx = 0;
#else
		cdk_memcpy( (unsigned char *)gsWMV78DecodedFrameAddress_NC[VA], gsWMV78DecOutput.m_pDecodedData->m_pucYPlane, gsWMV78FrameSize*1.5*sizeof(unsigned char) );

		p_output_param->m_pDispOut[0][0] = (unsigned char *)gsWMV78DecodedFrameAddress_NC[PA];
		p_output_param->m_pDispOut[0][1] = (unsigned char *)(gsWMV78DecodedFrameAddress_NC[PA]+gsWMV78FrameSize);
		p_output_param->m_pDispOut[0][2] = (unsigned char *)(gsWMV78DecodedFrameAddress_NC[PA]+gsWMV78FrameSize+gsWMV78FrameSize/4);
		p_output_param->m_pDispOut[1][0] = (unsigned char *)gsWMV78DecodedFrameAddress_NC[VA];
		p_output_param->m_pDispOut[1][1] = (unsigned char *)(gsWMV78DecodedFrameAddress_NC[VA]+gsWMV78FrameSize);
		p_output_param->m_pDispOut[1][2] = (unsigned char *)(gsWMV78DecodedFrameAddress_NC[VA]+gsWMV78FrameSize+gsWMV78FrameSize/4);
#endif
		p_output_param->m_DecOutInfo.m_iPicType          = gsWMV78DecOutput.m_iPictureType;
		p_output_param->m_DecOutInfo.m_iDecodedIdx       = 0;
		p_output_param->m_DecOutInfo.m_iDispOutIdx       = 0;
		p_output_param->m_DecOutInfo.m_iDecodingStatus   = gsWMV78DecOutput.m_iDecodingStatus;
		p_output_param->m_DecOutInfo.m_iOutputStatus     = 1;
		p_output_param->m_DecOutInfo.m_iInterlacedFrame  = 0;
		p_output_param->m_DecOutInfo.m_iPictureStructure = 0;

		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC_WMV78] WMV78_DEC_DECODE failed Error code is 0x%x \n", ret );
			return -ret;
		}

		p_output_param->m_pInitialInfo = &gsWMV78DecInitialInfo;
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

		if(!vdec_codec_opened)
			return -RETCODE_NOT_INITIALIZED;
			
		if ( gsIsINITdone )
		{
			ret = TCC_WMV78_DEC( VDEC_CLOSE, &gsWMV78DecHandle, NULL, NULL );
			if( ret != RETCODE_SUCCESS )
			{
				DPRINTF( "[VDEC_WMV78] WMV78_DEC_CLOSE failed Error code is 0x%x \n", ret );
				ret = -ret;
			}
		}

		vdec_codec_opened = 0;
		gsIsINITdone = 0;

		if ( gsWMV78DecInit.m_pHeapAddress )
			TCC_free(gsWMV78DecInit.m_pHeapAddress);
		if ( gsWMV78DecOutput.m_pDecodedData )
			TCC_free(gsWMV78DecOutput.m_pDecodedData);
		if ( gsWMV78DecInit.m_pCurFrameAddress )
			TCC_free(gsWMV78DecInit.m_pCurFrameAddress);
		if ( gsWMV78DecInit.m_pRef0FrameAddress )
			TCC_free(gsWMV78DecInit.m_pRef0FrameAddress);

		gsWMV78DecInit.m_pHeapAddress = 0;
		gsWMV78DecOutput.m_pDecodedData = 0;
		gsWMV78DecInit.m_pCurFrameAddress = 0;
		gsWMV78DecInit.m_pRef0FrameAddress = 0;

		if( gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsFrameBufAddr[VA], gsFrameBufSize )  >= 0)
			{
				gsFrameBufAddr[VA] = 0;
			}
		}

#ifndef HAVE_ANDROID_OS
		if( gsWMV78DecodedFrameAddress_NC[PA] )
			cdk_sys_free_physical_addr( (void*)gsWMV78DecodedFrameAddress_NC[PA], gsWMV78NCFrameSize );
		if( gsWMV78DecodedFrameAddress_NC[VA] )
			cdk_sys_free_virtual_addr( &NULL, gsWMV78DecodedFrameAddress_NC[VA], gsWMV78NCFrameSize );
#endif
		if( gsWMV78CurYFrameAddress )
			TCC_free( (void*)gsWMV78CurYFrameAddress );
		if( gsWMV78Ref0YFrameAddress )
			TCC_free( (void*)gsWMV78Ref0YFrameAddress );	

		gsWMV78CurYFrameAddress = 0;
		gsWMV78Ref0YFrameAddress = 0;

		for(buf_idx =0; buf_idx < decoded_buf_maxcnt; buf_idx++)
		{
			if( decoded_virtAddr[buf_idx] )
			{
				cdk_sys_free_virtual_addr( NULL, (void*)decoded_virtAddr[buf_idx], decoded_buf_size );
				decoded_virtAddr[buf_idx]= 0;
				
				DSTATUS("OUT-Buffer Release %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, decoded_phyAddr[buf_idx], decoded_virtAddr[buf_idx],	decoded_buf_size);
			}
		}			


#ifdef HAVE_ANDROID_OS
		vpu_env_close();
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
sorensonH263_dec_seq_header( int iSize , dec_initial_info_t * psInitialInfo )
{
	int ret = 0;
	int width, height;
	h263dec_stats_t s263_dec_stat;
	h263dec_input_t s263_dec_input;

#ifdef HAVE_ANDROID_OS
	memset(&s263_dec_stat, 0, sizeof(h263dec_stats_t));
	memset(&s263_dec_input, 0, sizeof(h263dec_input_t));
#else
	cdk_memset(&s263_dec_stat, 0, sizeof(h263dec_stats_t));
	cdk_memset(&s263_dec_input, 0, sizeof(h263dec_input_t));
#endif

	s263_dec_input.m_pBitstreamStartAddr = (void *)gsS263DecInit.m_BitstreamBufAddr[VA];
	if( iSize == 0 )
	{
		iSize = gsS263DecInit.m_iBitstreamBufSize;
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

	ret = TCC_H263_DEC( H263_DEC_SEQ_HEADER, (void *)gsS263DecHandle, &s263_dec_input, &s263_dec_stat );
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
  memcpy(&gsS263DecInitialStats, &s263_dec_stat, sizeof(h263dec_stats_t));
	memset(&gsS263DecInitialInfo, 0, sizeof(dec_initial_info_t));

#else
	cdk_memcpy(&gsS263DecInitialStats, &s263_dec_stat, sizeof(h263dec_stats_t));
	cdk_memset(&gsS263DecInitialInfo, 0, sizeof(dec_initial_info_t));

#endif
	

	gsS263DecInitialInfo.m_iMinFrameBufferCount = 1;
	gsS263DecInitialInfo.m_iMinFrameBufferSize = s263_dec_stat.m_iNeedFrameBufferSize;		// minimum frame buffer size

	width = s263_dec_stat.m_uInfo.m_sVol.m_iWidth;
	width = ( (width + 1 ) & (~0x1) );
	gsS263DecInitialInfo.m_iPicWidth = width; // {(PicX+1)/2} * 2  (this width  will be used while allocating decoder frame buffers. picWidth  is a multiple of 2)

	height = s263_dec_stat.m_uInfo.m_sVol.m_iHeight;
	height = ( (height+ 1 ) & (~0x1) );
	gsS263DecInitialInfo.m_iPicHeight = height; // {(PicY+1)/2} * 2  (this height will be used while allocating decoder frame buffers. picHeight is a multiple of 2)

	//  represents rectangular window information in a frame 
	gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropLeft = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Left;		
	gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropRight = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Right;
	gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropTop = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Top;
	gsS263DecInitialInfo.m_iAvcPicCrop.m_iCropBottom = s263_dec_stat.m_uInfo.m_sVol.m_iCropOffset_Bottom;

	if( gsS263DecInit.m_iPicWidth != gsS263DecInitialInfo.m_iPicWidth ||
		gsS263DecInit.m_iPicHeight != gsS263DecInitialInfo.m_iPicHeight  )
	{
		gsS263DecInit.m_iPicWidth = gsS263DecInitialInfo.m_iPicWidth;
		gsS263DecInit.m_iPicHeight = gsS263DecInitialInfo.m_iPicHeight;
		DPRINTF( "[VDEC] H263_DEC_SEQ_HEADER : resolution changing %dx%d => %dx%d\n",
			gsS263DecInit.m_iPicWidth, gsS263DecInit.m_iPicHeight,
			gsS263DecInitialInfo.m_iPicWidth, gsS263DecInitialInfo.m_iPicHeight);
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
		memcpy(psInitialInfo, &gsS263DecInitialInfo, sizeof(dec_initial_info_t));

#else
	if(psInitialInfo != NULL)
	#ifdef HAVE_ANDROID_OS
		memcpy(psInitialInfo, &gsS263DecInitialInfo, sizeof(dec_initial_info_t));
	#else
		cdk_memcpy(psInitialInfo, &gsS263DecInitialInfo, sizeof(dec_initial_info_t));
	#endif
#endif
	
	print_dec_initial_info( &gsS263DecInit, &gsS263DecInitialInfo );

	/* Display output buffer */

	gsFrameBufSize = (int)(width * height * 1.5);
	gsFrameBufSize = ALIGNED_BUFF( gsFrameBufSize, 4*1024 );
	gsFrameBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr( NULL, gsFrameBufSize, 0 );
	if( gsFrameBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] gsFrameBufAddr[PA] malloc() failed \n");
		return CDK_ERR_MALLOC;
	}	
	gsFrameBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsFrameBufAddr[PA], gsFrameBufSize );
	if( gsFrameBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] gsFrameBufAddr[VA] malloc() failed \n");
		return CDK_ERR_MALLOC;
	}
#ifdef HAVE_ANDROID_OS
	memset( (void*)gsFrameBufAddr[VA], 0, gsFrameBufSize);
#else
	cdk_memset( gsFrameBufAddr[VA], 0, gsFrameBufSize);
#endif
	DSTATUS( "[VDEC] gsFrameBufAddr [PA] = 0x%x, [VA] = 0x%x, size = 0x%x \n",		\
												(codec_addr_t)gsFrameBufAddr[PA],	\
												(codec_addr_t)gsFrameBufAddr[VA],	\
												gsFrameBufSize );

	/* decoding instance buffer */

	gsSliceBufSize = s263_dec_stat.m_iNeedFrameBufferSize;
	
	#ifdef HAVE_ANDROID_OS
	gsSliceBufAddr = (codec_addr_t)vdec_malloc( gsSliceBufSize );
	#else
	gsSliceBufAddr = (codec_addr_t)cdk_malloc( gsSliceBufSize );
	#endif
	
	if( gsSliceBufAddr == 0 )
	{
		DPRINTF( "[VDEC] gsSliceBufAddr malloc() failed \n");
		return CDK_ERR_MALLOC;
	}
	DSTATUS( "[VDEC] gsSliceBufAddr = 0x%x, 0x%x \n", (codec_addr_t)gsSliceBufAddr, gsSliceBufSize );

	ret = TCC_H263_DEC( H263_DEC_REG_FRAME_BUFFER, (void *)gsS263DecHandle, (void *)gsSliceBufAddr, (void *)NULL );
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
vdec_sorensonH263dec( int iOpCode, cdk_handle_t* pHandle, void* pParam1, void* pParam2 )
{
	int ret = 0;

	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	if( iOpCode == VDEC_INIT )
	{
		int heap_size;
		h263dec_init_t s263_dec_init;

		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;

#ifdef HAVE_ANDROID_OS
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, 0, 0, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight ) < 0)
			return -1;
#endif

#ifdef HAVE_ANDROID_OS
		gsS263DecInit.m_iBitstreamFormat	= p_input_param->m_iBitstreamFormat;
		gsS263DecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		gsS263DecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		gsS263DecInit.m_Memcpy				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		gsS263DecInit.m_Memset				= (void (*) ( void*, int, unsigned int ))memset;
		vpu_sorensonH263dec_ready(&gsS263DecInit);
#else
		gsS263DecInit.m_iBitstreamFormat	= p_input_param->m_iBitstreamFormat;
		gsS263DecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		gsS263DecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		gsS263DecInit.m_Memcpy				= p_input_param->m_pfMemcpy;
		gsS263DecInit.m_Memset				= p_input_param->m_pfMemset;
		gsS263DecInit.m_BitstreamBufAddr[PA] = p_input_param->m_BitstreamBufAddr[PA];
		gsS263DecInit.m_BitstreamBufAddr[VA] = p_input_param->m_BitstreamBufAddr[VA];
		gsS263DecInit.m_iBitstreamBufSize = p_input_param->m_iBitstreamBufSize;
#endif
		gsbHasSeqHeader = 0;//p_input_param->m_bHasSeqHeader; 

		heap_size = H263_DEC_HEAP_SIZE;
		
		#ifdef HAVE_ANDROID_OS
		gsS263DecHeapAddr = (unsigned char *) vdec_malloc( sizeof(unsigned char)*heap_size );
		#else
		gsS263DecHeapAddr = (unsigned char *) cdk_malloc( sizeof(unsigned char)*heap_size );
		#endif
		
		if( gsS263DecHeapAddr == 0 ) 
		{
			DPRINTF( "[VDEC] gsS263DecHeapAddr malloc() failed \n");
			return -1;
		}

#ifdef HAVE_ANDROID_OS
		memset( &s263_dec_init, 0, sizeof(h263dec_init_t) );
#else
		cdk_memset( &s263_dec_init, 0, sizeof(h263dec_init_t) );
#endif

		s263_dec_init.m_iCodecType = Th263_SORENSON_H263;
		s263_dec_init.m_pHeapAddr = (char*)gsS263DecHeapAddr;
		s263_dec_init.m_iHeapSize = heap_size;
		s263_dec_init.m_Memcpy = gsS263DecInit.m_Memcpy;
		s263_dec_init.m_Memset = gsS263DecInit.m_Memset;

		ret =  TCC_H263_DEC( H263_DEC_INIT, NULL, (void *)&s263_dec_init, (void *)NULL );
		gsS263DecHandle = (codec_handle_t)s263_dec_init.m_pHandle;
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] H263_DEC_INIT failed Error code is 0x%x \n", ret );
			return -ret;
		}
		
		vdec_codec_opened = 1;		
		DSTATUS( "[VDEC] H263_DEC_INIT OK( has seq = %d) \n", gsbHasSeqHeader );
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
	
	#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = p_input_param->m_iInpLen;

		gsS263DecInit.m_BitstreamBufAddr[PA] = gsBitstreamBufAddr[PA];
		gsS263DecInit.m_BitstreamBufAddr[VA] = gsBitstreamBufAddr[VA];
		memcpy( (void*)gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], p_input_param->m_iInpLen);

		save_input_stream("data/vpu_inSeq.bin", p_input_param->m_iInpLen);
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
			gsbHasSeqHeader = 1;
			p_output_param->m_pInitialInfo = &gsS263DecInitialInfo;
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
		save_input_stream("data/vpu_inDec.bin", p_input_param->m_iInpLen);

		pBitstreamAddr = (char *)p_input_param->m_pInp[VA];
	#else
		pBitstreamAddr = (char *)p_input_param->m_pInp[VA];
	#endif
		useful_bytes = p_input_param->m_iInpLen;

		/* Output frame structure */
		s263_dec_input.m_DispOout.m_pPlane[0]  = (void *)gsFrameBufAddr[VA];
		
		stride = gsS263DecInitialInfo.m_iPicWidth;
	#if defined(SORENSON263_DEC_OUTPUT_MEM_ALIGN16) || defined(SORENSON263_DEC_OUTPUT_MEM_ALIGN16_STRIDE)
		if( stride&0x0F )
		{
			stride = (stride+15)&(~15);	//multiple of 16 for M2M scaler
		}
	#endif

		s263_dec_input.m_DispOout.m_iStride[0] = stride;
		s263_dec_input.m_DispOout.m_iCsp = Th263_CSP_I420;

		dispOutSizeY = stride * gsS263DecInitialInfo.m_iPicHeight;
		pDispOut[PA][0] = (unsigned char* )gsFrameBufAddr[PA];
		pDispOut[VA][0] = (unsigned char* )gsFrameBufAddr[VA];
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
			ret = TCC_H263_DEC( H263_DEC_DECODE, (void *)gsS263DecHandle, (void *)&s263_dec_input, (void *)&s263_dec_stat );
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

		p_output_param->m_pInitialInfo = &gsS263DecInitialInfo;
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
		if(!vdec_codec_opened)
			return -RETCODE_NOT_INITIALIZED;
		ret = TCC_H263_DEC( H263_DEC_CLOSE, (void *)gsS263DecHandle, (void *)NULL, (void *)NULL );
		gsS263DecHandle = 0;
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC] H263_DEC_CLOSE failed Error code is 0x%x \n", ret );
			ret = -ret;
		}
		vdec_codec_opened = 0;

		if( gsS263DecHeapAddr )
		{
		#ifdef HAVE_ANDROID_OS
			vdec_free( (void*)gsS263DecHeapAddr );
		#else
			cdk_free( gsS263DecHeapAddr );
		#endif
			gsS263DecHeapAddr = 0;
		}

		if( gsSliceBufAddr )
		{
		#ifdef HAVE_ANDROID_OS
			vdec_free( (void*)gsSliceBufAddr );
		#else
			cdk_free( (void*)gsSliceBufAddr );
		#endif
			gsSliceBufAddr = 0;
		}

		if( gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsBitstreamBufAddr[VA], gsBitstreamBufSize )  >= 0)
			{
				gsBitstreamBufAddr[VA] = 0;
			}
		}
		
		if( gsFrameBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsFrameBufAddr[VA], gsFrameBufSize )  >= 0)
			{
				gsFrameBufAddr[VA] = 0;
			}
		}
				
		#ifdef HAVE_ANDROID_OS
			vpu_env_close();
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

#ifdef JPEG_DECODE_FOR_MJPEG
int
jpeg_dec_ready( dec_init_t* psVDecInit )
{
	//------------------------------------------------------------
	//! [x] bitstream buffer for each VPU decoder
	//------------------------------------------------------------
	gsMaxBitstreamSize = gsBitstreamBufSize = JPEGDEC_STREAM_MAX;
	gsBitstreamBufSize = ALIGNED_BUFF( gsBitstreamBufSize, 4*1024 );
	gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(NULL, gsBitstreamBufSize, 0 );
	if( gsBitstreamBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC] bitstream_buf_addr[PA] malloc() failed \n");
		return -1;
	}
	DSTATUS( "[VDEC] bitstream_buf_addr[PA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitstreamBufAddr[PA], gsBitstreamBufSize );
	gsBitstreamBufAddr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, gsBitstreamBufAddr[PA], gsBitstreamBufSize );
	if( gsBitstreamBufAddr[VA] == 0 ) 
	{
		DPRINTF( "[VDEC] bitstream_buf_addr[VA] malloc() failed \n");
		return -1;
	}
	memset( (void*)gsBitstreamBufAddr[VA], 0x00 , gsBitstreamBufSize);
	DSTATUS("[VDEC] bitstream_buf_addr[VA] = 0x%x, 0x%x \n", (codec_addr_t)gsBitstreamBufAddr[VA], gsBitstreamBufSize );

	psVDecInit->m_BitstreamBufAddr[PA]	= gsBitstreamBufAddr[PA];
	psVDecInit->m_BitstreamBufAddr[VA]	= gsBitstreamBufAddr[VA];
	psVDecInit->m_iBitstreamBufSize 	= gsBitstreamBufSize;

	return 0;
}

int
vdec_jpeg( int iOpCode, int* pHandle, void* pParam1, void* pParam2 )
{
	int ret = 0;
	TCCXXX_JPEG_DEC_DATA dec_data;

	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	if( iOpCode == VDEC_INIT )
	{
		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;		
		vdec_user_info_t* p_input_user_param = (vdec_user_info_t*)pParam2;

		gsJPEGDecUserInfo.bitrate_mbps = p_input_user_param->bitrate_mbps;
		gsJPEGDecUserInfo.frame_rate   = p_input_user_param->frame_rate;
		gsJPEGDecUserInfo.m_bJpegOnly  = p_input_user_param->m_bJpegOnly;
		
		if(vpu_env_open(p_input_param->m_iBitstreamFormat, p_input_user_param->bitrate_mbps, p_input_user_param->frame_rate, p_input_param->m_iPicWidth, p_input_param->m_iPicHeight ) < 0)
			return -VPU_ENV_INIT_ERROR;

		gsJPEGDecHandle = open(JPEGDEC_DEVICE, O_RDWR | O_NDELAY);
		if(!gsJPEGDecHandle)
		{
			ALOGE("Err[%s] :: jpeg(%s) open failed!", strerror(errno), JPEGDEC_DEVICE);
			return -RETCODE_FAILURE;
		}
		
		gsJPEGDecInit.m_RegBaseVirtualAddr	= 0;
		gsJPEGDecInit.m_iBitstreamFormat		= p_input_param->m_iBitstreamFormat;
		gsJPEGDecInit.m_iPicWidth			= p_input_param->m_iPicWidth;
		gsJPEGDecInit.m_iPicHeight			= p_input_param->m_iPicHeight;
		gsJPEGDecInit.m_bEnableUserData		= p_input_param->m_bEnableUserData;
		gsJPEGDecInit.m_bEnableVideoCache	= p_input_param->m_bEnableVideoCache;
		gsJPEGDecInit.m_bCbCrInterleaveMode  = p_input_param->m_bCbCrInterleaveMode;
		gsJPEGDecInit.m_Memcpy				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		gsJPEGDecInit.m_Memset				= (void  (*) ( void*, int, unsigned int ))memset;
		gsJPEGDecInit.m_Interrupt			= (int  (*) ( void ))NULL;	
		gsJPEGDecInit.m_iFilePlayEnable		= p_input_param->m_bFilePlayEnable;
		gsbHasSeqHeader = 0;//p_input_param->m_bHasSeqHeader; 

		jpeg_dec_ready( &gsJPEGDecInit );
		DSTATUS("Format : %d, Stream(0x%x, %d)", gsJPEGDecInit.m_iBitstreamFormat, gsJPEGDecInit.m_BitstreamBufAddr[PA], gsJPEGDecInit.m_iBitstreamBufSize);

		ret = RETCODE_SUCCESS;
		vdec_codec_opened = 1;
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = (p_input_param->m_iInpLen > gsMaxBitstreamSize) ? gsMaxBitstreamSize : p_input_param->m_iInpLen;

		if (    ((codec_addr_t)p_input_param->m_pInp[PA] == gsBitstreamBufAddr[PA])
		     && ((codec_addr_t)p_input_param->m_pInp[VA] == gsBitstreamBufAddr[VA]) )
		{
			gsJPEGDecInit.m_BitstreamBufAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
			gsJPEGDecInit.m_BitstreamBufAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		}
		else
		{
			gsJPEGDecInit.m_BitstreamBufAddr[PA] = gsBitstreamBufAddr[PA];
			gsJPEGDecInit.m_BitstreamBufAddr[VA] = gsBitstreamBufAddr[VA];
			memcpy( (void*)gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], seq_stream_size);
		}

		DSTATUS( "[VDEC_JPG] JPEG_DEC_SEQ_HEADER start  :: len = %d / %d , handle = 0x%x \n", seq_stream_size, p_input_param->m_iInpLen, gsJPEGDecHandle );

		memset(&dec_data, 0x00, sizeof(TCCXXX_JPEG_DEC_DATA));

		dec_data.source_addr = gsJPEGDecInit.m_BitstreamBufAddr[PA];
		dec_data.file_length = seq_stream_size;
		dec_data.target_addr = gsJPEGDecInit.m_BitstreamBufAddr[PA];
		dec_data.target_size = gsBitstreamBufSize;
		
		ret = ioctl(gsJPEGDecHandle, TCC_JPEGD_IOCTL_HEADER_INFO, &dec_data);
		if( ret < 0 )
		{		
			ALOGE("Err[%s] :: jpeg(%d) TCC_JPEGD_IOCTL_HEADER_INFO failed!", strerror(errno), ret);

			if(ret == -1)
				return -RETCODE_INVALID_STRIDE;
			else
				return -ret;
		}
		gsJPEGDecInitialInfo.m_iPicWidth = dec_data.pad_width;
		gsJPEGDecInitialInfo.m_iPicHeight = dec_data.pad_height;
		DSTATUS( "[VDEC_JPG] Resolution %d x %d, Format : %d \n", gsJPEGDecInitialInfo.m_iPicWidth, gsJPEGDecInitialInfo.m_iPicHeight, dec_data.image_format);

		//!< MJPEG source chroma format(0 - 4:2:0, 1 - 4:2:2, 2 - 4:2:2 vertical, 3 - 4:4:4, 4 - 4:0:0 )(only for TCC93XX)
		if(dec_data.image_format == IMAGE_CHROMA_420)
			gsJPEGDecInitialInfo.m_iMjpg_sourceFormat = 0;
		else
			gsJPEGDecInitialInfo.m_iMjpg_sourceFormat = 1;		

		if(dec_data.image_format != IMAGE_CHROMA_420)
		{
			ALOGI("MJPEG Format is not YUV420 (%d)", dec_data.image_format); //1:420, 2:422
		}
		
		gsbHasSeqHeader = 1;
		p_output_param->m_pInitialInfo = &gsJPEGDecInitialInfo;

		//check the maximum/minimum video resolution limitation
		{
			vdec_info_t * pVdecInfo = (vdec_info_t *)&gsJPEGDecInitialInfo;
			unsigned int buf_idx;
			
			decoded_buf_curIdx = 0;
			decoded_buf_size = pVdecInfo->m_iPicWidth*pVdecInfo->m_iPicHeight*2;
			decoded_buf_size = ALIGNED_BUFF(decoded_buf_size, 4*1024);
			
			for(buf_idx =0; buf_idx < MAX_NUM_OF_JPEG_ELEMENT; buf_idx++)
			{
				decoded_phyAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_physical_addr(NULL, decoded_buf_size, 0 );				
				if( decoded_phyAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC_JPG,Err:%d] vdec_vpu decoded_virtAddr[PA] alloc failed \n", ret );
					decoded_buf_maxcnt = buf_idx;
					break;
				}	
				decoded_virtAddr[buf_idx] = (codec_addr_t)cdk_sys_malloc_virtual_addr( NULL, decoded_phyAddr[buf_idx], decoded_buf_size ); 
				if( decoded_virtAddr[buf_idx] == 0 ) 
				{
					DPRINTF( "[VDEC_JPG,Err:%d] vdec_vpu decoded_virtAddr[VA] alloc failed \n", ret );
					decoded_buf_maxcnt = buf_idx;
					break;
				}

				decoded_buf_maxcnt = MAX_NUM_OF_JPEG_ELEMENT;
				DSTATUS("OUT-Buffer %d ::	PA = 0x%x, VA = 0x%x, size = 0x%x!!\n", buf_idx, decoded_phyAddr[buf_idx], decoded_virtAddr[buf_idx],	decoded_buf_size);
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

		gsJPEGDecInput.m_iBitstreamDataSize = (p_input_param->m_iInpLen > gsMaxBitstreamSize) ? gsMaxBitstreamSize : p_input_param->m_iInpLen;
		if (    ((codec_addr_t)p_input_param->m_pInp[PA] == gsBitstreamBufAddr[PA])
		     && ((codec_addr_t)p_input_param->m_pInp[VA] == gsBitstreamBufAddr[VA]) )
		{
			gsJPEGDecInput.m_BitstreamDataAddr[PA] = (codec_addr_t)p_input_param->m_pInp[PA];
			gsJPEGDecInput.m_BitstreamDataAddr[VA] = (codec_addr_t)p_input_param->m_pInp[VA];
		}
		else
		{
			gsJPEGDecInput.m_BitstreamDataAddr[PA] = gsBitstreamBufAddr[PA];
			gsJPEGDecInput.m_BitstreamDataAddr[VA] = gsBitstreamBufAddr[VA];
			DSTATUS("memcpy : 0x%x - %d", gsJPEGDecInput.m_BitstreamDataAddr[PA], gsJPEGDecInput.m_iBitstreamDataSize);
			memcpy( (void*)gsBitstreamBufAddr[VA], (void*)p_input_param->m_pInp[VA], gsJPEGDecInput.m_iBitstreamDataSize);
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

		dec_data.source_addr = gsJPEGDecInput.m_BitstreamDataAddr[PA];
		dec_data.file_length = gsJPEGDecInput.m_iBitstreamDataSize;
		dec_data.target_addr = decoded_phyAddr[decoded_buf_curIdx];
		dec_data.target_size = decoded_buf_size;

		DSTATUS("out[%d] : 0x%x - %d", decoded_buf_curIdx, decoded_phyAddr[decoded_buf_curIdx], decoded_buf_size);		
		ret = ioctl(gsJPEGDecHandle, TCC_JPEGD_IOCTL_DEC, &dec_data);
		if( ret < 0 )
		{
			ALOGE("Err[%s] :: jpeg(%d) TCC_JPEGD_IOCTL_DEC failed!", strerror(errno), ret);
			p_output_param->m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_FAIL;	
			p_output_param->m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;
			return 0;
		}
		gsJPEGDecOutput.m_DecOutInfo.m_iOutputStatus = VPU_DEC_OUTPUT_SUCCESS;
		gsJPEGDecOutput.m_DecOutInfo.m_iDecodingStatus = VPU_DEC_SUCCESS;

#ifdef  VIDEO_DEC_PROFILE
#ifdef DEBUG_TIME_LOG		
		end = clock();
		dec_time[time_cnt] = (end-start)*1000/CLOCKS_PER_SEC;
		total_dec_time += dec_time[time_cnt];
		total_frame_cnt++;
		if(time_cnt != 0 && time_cnt % 29 == 0)
		{
			ALOGD("VDEC_JPG %d ms: %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d, %3d",
				total_dec_time/total_frame_cnt, dec_time[0], dec_time[1], dec_time[2], dec_time[3], dec_time[4], dec_time[5], dec_time[6], dec_time[7], dec_time[8], dec_time[9], 
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
			unsigned int frame_size = gsJPEGDecInitialInfo.m_iPicWidth*gsJPEGDecInitialInfo.m_iPicHeight;
			unsigned char div_val = 4;

			memcpy((void*)p_output_param, (void*)&gsJPEGDecOutput, sizeof(dec_output_t ) );

			if(gsJPEGDecInitialInfo.m_iMjpg_sourceFormat == 0)
				div_val = 4;
			else
				div_val = 2;
			
			p_output_param->m_pDispOut[PA][0] = (unsigned char*)decoded_phyAddr[decoded_buf_curIdx];
			p_output_param->m_pDispOut[PA][1] = (unsigned char*)p_output_param->m_pDispOut[PA][0] + frame_size;
			p_output_param->m_pDispOut[PA][2] = (unsigned char*)p_output_param->m_pDispOut[PA][1] + frame_size/div_val;
			p_output_param->m_pDispOut[VA][0] = (unsigned char*)decoded_virtAddr[decoded_buf_curIdx];
			p_output_param->m_pDispOut[VA][1] = (unsigned char*)p_output_param->m_pDispOut[VA][0] + frame_size;
			p_output_param->m_pDispOut[VA][2] = (unsigned char*)p_output_param->m_pDispOut[VA][1] + frame_size/div_val;

			if( gsJPEGDecInitialInfo.m_iMjpg_sourceFormat == 1) //YUV422
			{
				//To Do::
			}

			decoded_buf_curIdx++;
			if(decoded_buf_curIdx >= decoded_buf_maxcnt)
				decoded_buf_curIdx = 0;
		}
		p_output_param->m_pInitialInfo = &gsJPEGDecInitialInfo;
	}
	else if( iOpCode == VDEC_CLOSE )
	{
		if(!vdec_codec_opened && !vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		if(vdec_codec_opened)
		{
			if(close(gsJPEGDecHandle) < 0)
			{
				ALOGE("Err[%s] :: jpeg(%s) close failed!", strerror(errno), JPEGDEC_DEVICE);
			}
			gsJPEGDecHandle = -1;
			vdec_codec_opened = 0;
		}
		
		if(!vdec_env_opened)
			return -RETCODE_NOT_INITIALIZED;

		gsBitstreamBufAddr[PA] = 0;
		if( gsBitstreamBufAddr[VA] ){
			if(cdk_sys_free_virtual_addr( NULL, (void*)gsBitstreamBufAddr[VA], gsBitstreamBufSize )  >= 0)
			{
				gsBitstreamBufAddr[VA] = 0;
			}
		}

		
		if( gsJPEGDecInit.m_iBitstreamFormat == STD_MJPG && !gsJPEGDecUserInfo.m_bJpegOnly)
		{
			unsigned int buf_idx = 0;
			
			for(buf_idx =0; buf_idx < decoded_buf_maxcnt; buf_idx++)
			{
				decoded_phyAddr[buf_idx] = 0;
				cdk_sys_free_virtual_addr( NULL, (void*)decoded_virtAddr[buf_idx], decoded_buf_size );
				decoded_virtAddr[buf_idx] = 0;
			}			
		}
		vpu_env_close();
	}
	else
	{
		return 0;
	}

	return ret;
}
#endif

