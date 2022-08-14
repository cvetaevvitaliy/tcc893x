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
#define LOG_TAG	"VPU_DEC_K"
#include <utils/Log.h>

#include "cdk_core.h"
#include "vdec.h"
#include "TCCMemory.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif
#include "TCC_VPU_CODEC_EX.h"

#include <sys/mman.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mach/tcc_vpu_ioctl.h>
#include <dlfcn.h>

#include <fcntl.h>         // O_RDWR
#include <sys/poll.h>

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

#if defined(TCC_88XX_INCLUDE) || defined(TCC_93XX_INCLUDE)
#define VPU_PERFORMANCE_UP //use sram and vcache
#endif
#define TCC_VPU_INPUT_BUF_SIZE 		(1024 * 1024)
#define MAX_FRAME_BUFFER_COUNT		31

#define MAX_BITSTREAM_SIZE			(2 * 1024 * 1024)

//#define USE_VPU_INTERRUPT
//#define VPU_REGISTER_DUMP
//#define VPU_FRAME_DUMP

#ifdef USE_VPU_INTERRUPT
#define TCC_INTR_DEV_NAME		"/dev/tcc_intr"
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

#define ALIGN_LEN (4*1024)

static int vdec_hw_reset_flag =0;

typedef int (*DEC_FUNC)( int iOpCode, int* pHandle, void* pParam1, void* pParam2 );

typedef struct _vdec_ {
	int vdec_instance_index;
	unsigned int total_frm;

	unsigned char vdec_env_opened;
	unsigned char vdec_codec_opened;
	unsigned char prev_codec;

	int vpu_mgr_fd; // default -1
	int vpu_dec_fd; // default -1

#ifdef USE_VPU_INTERRUPT
	int vpu_intr_fd; // default -1
#endif

#ifdef HAVE_ANDROID_OS
	int gsBitstreamBufSize;
	int gsIntermediateBufSize;
	int gsUserdataBufSize;
	int gsMaxBitstreamSize;
	unsigned int gsAdditionalFrameCount;

	codec_addr_t gsBitstreamBufAddr[3];
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

	codec_addr_t gsBitWorkBufAddr[3];
	codec_addr_t gsSliceBufAddr;
	codec_addr_t gsSpsPpsAddr;
	codec_addr_t gsFrameBufAddr[3];
#if (defined(TCC_892X_INCLUDE) && !defined(TCC_8925S_INCLUDE)) || (defined(TCC_893X_INCLUDE) && !defined(TCC_8935S_INCLUDE))
	int gsMbSaveSize;
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

	struct pollfd tcc_event[1];

	int bMaxfb_Mode;
	int extFunction;

	void* pExtDLLModule;
	DEC_FUNC gExtDecFunc;
} _vdec_;

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

static void vpu_env_close(int handle, _vdec_ * pVdec);

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

static void *__cdk_sys_malloc_physical_addr(unsigned int *remap_addr, int uiSize, Buffer_Type type, _vdec_ *pVdec)
{
	MEM_ALLOC_INFO_t alloc_mem;
	_vdec_ * pInst = pVdec;
	memset(&alloc_mem, 0x00, sizeof(MEM_ALLOC_INFO_t));
	
	alloc_mem.request_size = uiSize;
	alloc_mem.buffer_type = type;
	ioctl(pInst->vpu_dec_fd, V_DEC_ALLOC_MEMORY, &alloc_mem);
	if(remap_addr != NULL)
		*remap_addr = alloc_mem.kernel_remap_addr;

	return (void*)( alloc_mem.phy_addr );
}


static void *__cdk_sys_malloc_virtual_addr(int* pDev, codec_addr_t pPtr, int uiSize, _vdec_ *pVdec)
{
	void *map_ptr = MAP_FAILED;
	_vdec_ * pInst = pVdec;
	map_ptr = (void *)mmap(NULL, uiSize, PROT_READ | PROT_WRITE, MAP_SHARED, pInst->vpu_dec_fd, pPtr);
	if(MAP_FAILED == map_ptr)
	{
		DPRINTF("mmap failed. fd(%d), addr(0x%x), size(%d)", pVdec->vpu_dec_fd, pPtr, uiSize);
		return NULL;
	}
	
	return map_ptr;
}

static int __cdk_sys_free_virtual_addr(int* pDev, void* pPtr, int uiSize)
{
	int ret = 0;
	
	if((ret = munmap((void*)pPtr, uiSize)) < 0)
	{
		DPRINTF("munmap failed. addr(0x%x), size(%d)", (unsigned int)pPtr, uiSize);
	}

	return ret;
}

#define		cdk_sys_malloc_physical_addr 	__cdk_sys_malloc_physical_addr
#define		cdk_sys_free_physical_addr 		__cdk_sys_free_physical_addr
#define		cdk_sys_malloc_virtual_addr 	__cdk_sys_malloc_virtual_addr
#define		cdk_sys_free_virtual_addr	 	__cdk_sys_free_virtual_addr


static unsigned int cdk_sys_remain_memory_size(_vdec_ *pVdec)
{
	unsigned int sz_freeed_mem;
	_vdec_ * pInst = pVdec;
	ioctl(pInst->vpu_mgr_fd, VPU_GET_FREEMEM_SIZE, &sz_freeed_mem);

	return sz_freeed_mem;
}

static void print_frame_data(char* name, int size, _vdec_ *pVdec)
{
#ifdef VPU_FRAME_DUMP
	int i;
	unsigned char* ps = (unsigned char*)pVdec->gsBitstreamBufAddr[VA];

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

void vpu_update_sizeinfo(unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, void *pVdec)
{
	CONTENTS_INFO info;
	_vdec_ * pInst = pVdec;

	info.type = pInst->vdec_instance_index - VPU_DEC;

	info.width = image_width;
	info.height = image_height;	
	info.bitrate = bps;
	info.framerate = fps;
	info.isSWCodec = 0;
	
	ioctl(pInst->vpu_mgr_fd, VPU_SET_CLK, &info);
	DPRINTF("%s:%d:%d:%d:%d", __func__, image_width, image_height, bps, fps);
	return;
}

static int vpu_check_for_video(unsigned char open_status, int handle, _vdec_ * pVdec)
{
//return value : if hw reset need or not!!
	_vdec_ * pInst = pVdec;
	int ret_vpu_reset = 0;
	int vpu_wait;
	OPENED_gINFO gInfo;
	OPENED_sINFO sInfo;
	int temp_vpu_dec_fd = -1;

	if(open_status == 1)
	{
		ret_vpu_reset = 0;

		ALOGI("@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Instance[%d] = %s", pInst->vdec_instance_index, vpu_devices[pInst->vdec_instance_index]);
		temp_vpu_dec_fd = open(vpu_devices[pInst->vdec_instance_index], O_RDWR);
		if(temp_vpu_dec_fd < 0)
		{
			ALOGE("%s open error[%s]", vpu_devices[pInst->vdec_instance_index], strerror(errno));
			return -1;
		}

// check open for 3 second in vpu device driver!!
		ioctl(pInst->vpu_mgr_fd, VPU_GET_OPEN_INFO, &gInfo);
		DBUG_FLIP("open info :0x%x: %d, dmb(%d), video(%d)", &pInst->prev_codec, gInfo.count, gInfo.dmb_opened, gInfo.vid_opened);

		if(gInfo.count != 1 && gInfo.vid_opened == 1)
		{			
			if(gInfo.dmb_opened)
			{
				DBUG_FLIP("VPU for DMB has been opened. So wait for 3 sec and check per 100ms!!");
				vpu_wait = 30;
				while(vpu_wait > 0)
				{
					usleep(100 * 1000);//100ms				
					ioctl(pInst->vpu_mgr_fd, VPU_GET_OPEN_INFO, &gInfo);
					vpu_wait--;

					if(gInfo.dmb_opened == 0)
					{
						DBUG_FLIP("VPU for DMB stopped.");
						break;
					}
					
					if(vpu_wait <= 0)
					{
						DPRINTF("VPU for DMB didn't close!!. so assume to close and reset vpu!!");
						
						sInfo.type = OPEN_DMB;
						sInfo.opened_cnt = 0;
						ioctl(pInst->vpu_mgr_fd, VPU_SET_OPEN_INFO, &sInfo);
						
						ret_vpu_reset = 1;
						break;
					}
				}
			}
		}
	}

	sInfo.type = OPEN_DMB;
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

	if(vdec_hw_reset_flag)
	{
		DBUG_FLIP("VDEC has to reset because other application was opened vpu (ex. DMB).!!");
		ioctl(pInst->vpu_mgr_fd, VPU_HW_RESET, (void*)NULL);
		vdec_hw_reset_flag = 0;
	}
	
	return 0;
}

static int vpu_env_open(int handle, unsigned int format, unsigned int bps, unsigned int fps, unsigned int image_width, unsigned int image_height, _vdec_ *pVdec)
{
	int vpu_reset = 0;
	_vdec_ * pInst = pVdec;
	DSTATUS("In  %s \n",__func__);
		
	if((vpu_reset = vpu_check_for_video(1, handle, pInst)) < 0)
		goto err;
	
#ifdef  USE_VPU_INTERRUPT
	pInst->vpu_intr_fd = open(TCC_INTR_DEV_NAME, O_RDWR);
	if (pInst->vpu_intr_fd < 0) {
		DPRINTF("%s open error", TCC_INTR_DEV_NAME);
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

	memset(&pInst->gsBitstreamBufAddr, 0x00, sizeof(pInst->gsBitstreamBufAddr));
	memset(&pInst->gsUserdataBufAddr, 0x00, sizeof(pInst->gsUserdataBufAddr));
	memset(&pInst->gsBitWorkBufAddr, 0x00, sizeof(pInst->gsBitWorkBufAddr));
	memset(&pInst->gsFrameBufAddr, 0x00, sizeof(pInst->gsFrameBufAddr));
	pInst->gsSliceBufAddr = pInst->gsSpsPpsAddr = 0;

	DSTATUS("Out  %s \n",__func__);

	pInst->gsAdditionalFrameCount = VPU_BUFF_COUNT;
	pInst->total_frm = 0;
	pInst->bMaxfb_Mode = 0;
	return 0;

err:	
	DPRINTF("vpu_env_open error");
	vpu_env_close(handle, pInst);
	
	return -1;	
}


static void vpu_env_close(int handle, _vdec_ *pVdec)
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
	if(pInst->vpu_intr_fd >= 0)
	{
		if(close(pInst->vpu_intr_fd) < 0)
		{
			DPRINTF("%s close error", TCC_INTR_DEV_NAME);
		}
		pInst->vpu_intr_fd = -1;
	}
#endif	

	if(pInst->vpu_dec_fd >= 0)
	{
		if(close(pInst->vpu_dec_fd) < 0)
		{
			DPRINTF("%s close error", vpu_devices[pInst->vdec_instance_index]);
		}
		pInst->vpu_dec_fd = -1;
	}

	pInst->vdec_env_opened = 0;
	vpu_check_for_video(0, handle, pInst);

	DSTATUS("Out  %s \n",__func__);

}

#ifdef VPU_REGISTER_DUMP
static unsigned char bFirst_frame = 1;
static void filewrite_memory(char* name, char* addr, unsigned int size, _vdec_ *pVdec)
{
	FILE *fp;

	if(!bFirst_frame)
		return;
	
	fp = fopen(name, "ab+");		
	fwrite( addr, size, 1, fp);
	fclose(fp);
}
#endif

#if 0
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
			DPRINTF("vpu poll error\n");
			break;
		}else if (ret == 0) {
			DPRINTF("vpu poll timeout\n");		
			break;
		}else if (ret > 0) {
			if (tcc_event[0].revents & POLLERR) {
				DPRINTF("vpu poll POLLERR\n");
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
#endif

static int vdec_cmd_process(int cmd, void* args, _vdec_ *pVdec)
{
	int ret;
	int success = 0;
	_vdec_ * pInst = pVdec;
	int retry_cnt = 10;

	if(ioctl(pInst->vpu_dec_fd, cmd, args) < 0)
	{
		DPRINTF("vpu[%d] ioctl err[%s] : cmd = 0x%x", pInst->vdec_instance_index, strerror(errno), cmd);
	}
	
	while (retry_cnt > 0) {
		memset(pInst->tcc_event, 0, sizeof(pInst->tcc_event));
		pInst->tcc_event[0].fd = pInst->vpu_dec_fd;
		pInst->tcc_event[0].events = POLLIN;
		
		ret = poll((struct pollfd *)&pInst->tcc_event, 1, 1000); // 1000 msec
		if (ret < 0) {
			DPRINTF("vpu[%d]-retry(%d) poll error '%s'", pInst->vdec_instance_index, retry_cnt, strerror(errno));
			retry_cnt--;
			continue;
//			goto Retry;//break;
		}else if (ret == 0) {
			DPRINTF("vpu[%d]-retry(%d) poll timeout: %d'th frames, len %d", pInst->vdec_instance_index, retry_cnt, pInst->total_frm, pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize );
			retry_cnt--;
			continue;
		}else if (ret > 0) {
			if (pInst->tcc_event[0].revents & POLLERR) {
				DPRINTF("vpu[%d] poll POLLERR\n", pInst->vdec_instance_index);
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
		DPRINTF("[VDEC-%d]command(0x%x) didn't work properly. maybe hangup(no return(0x%x))!!", pInst->vdec_instance_index, cmd, ret);
			
		if(ret != RETCODE_CODEC_EXIT && ret != RETCODE_MULTI_CODEC_EXIT_TIMEOUT){
			//ioctl(pInst->vpu_mgr_fd, VPU_HW_RESET, (void*)NULL);
		}
			
		return RETCODE_CODEC_EXIT;
	}

	return ret;
}

void vpu_set_additional_frame_count(int count, void * pInst)
{
	_vdec_ *pInst_dec = (_vdec_ *)pInst;
	pInst_dec->gsAdditionalFrameCount = count;
	DSTATUS( "[VDEC-%d] gsAdditionalFrameCount %d", pInst_dec->vdec_instance_index, pInst_dec->gsAdditionalFrameCount );
}


unsigned char *vpu_getBitstreamBufAddr(unsigned int index, void * pVdec)
{
	unsigned char *pBitstreamBufAddr = NULL;
	_vdec_ *pInst = pVdec;
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

unsigned char *vpu_getFrameBufVirtAddr(unsigned char *convert_addr, unsigned int base_index, void * pVdec)
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
	_vdec_ *pInst = pVdec;
	return pInst->gsBitstreamBufSize;
}

unsigned char *vpu_getUserDataBufAddr(unsigned int index, void * pInst)
{
	_vdec_ *pVdec = (_vdec_ *)pInst;

	unsigned char *pUserDataBufAddr = NULL;
	
	if (index == VA)
	{
		pUserDataBufAddr = (unsigned char *)pVdec->gsUserdataBufAddr[VA];
	}
	else //if (index == PA) //or default
	{
		pUserDataBufAddr = (unsigned char *)pVdec->gsUserdataBufAddr[PA];
	}

	return pUserDataBufAddr;
}

int vpu_getUserDataBufSize(void *pInst)
{
	_vdec_ *pVdec = (_vdec_ *)pInst;
	return pVdec->gsUserdataBufSize;
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
	if( (pDecInit->m_iBitstreamFormat != STD_WMV78) &&
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


	DSTATUS("---------------------------------------------------\n");
	
}

int
vpu_dec_ready( dec_init_t* psVDecInit, _vdec_ *pVdec )
{
	//------------------------------------------------------------
	// [x] PS(SPS/PPS) buffer for each VPU decoder
	//------------------------------------------------------------
	_vdec_ * pInst = pVdec;
	if( psVDecInit->m_iBitstreamFormat == STD_AVC )
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
	pInst->gsBitstreamBufSize = LARGE_STREAM_BUF_SIZE;
	pInst->gsBitstreamBufSize = ALIGNED_BUFF( pInst->gsBitstreamBufSize, ALIGN_LEN );
	if(pInst->vdec_instance_index == 0)
	{
		pInst->gsBitstreamBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitstreamBufAddr[K_VA], pInst->gsBitstreamBufSize, BUFFER_STREAM, pInst);
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
	}
	else
	{
		pInst->gsIntermediateBufSize = 0;
		pInst->gsIntermediateBufAddr[PA] = 0;
		pInst->gsIntermediateBufAddr[VA] = 0;
		pInst->gsIntermediateBufAddr[K_VA] = 0;
	}
	/* Set the maximum size of input bitstream. */
//	pInst->gsMaxBitstreamSize = MAX_BITSTREAM_SIZE;
//	pInst->gsMaxBitstreamSize = ALIGNED_BUFF(pInst->gsMaxBitstreamSize, (4 * 1024));
//	if (pInst->gsMaxBitstreamSize > pInst->gsBitstreamBufSize)
	{
		pInst->gsMaxBitstreamSize = pInst->gsBitstreamBufSize;
	}
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
		//memset( (void*)pInst->gsUserdataBufAddr[VA], 0 , pInst->gsUserdataBufSize);
		DSTATUS("[VDEC-%d] pInst->gsUserdataBufAddr[VA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsUserdataBufAddr[VA], pInst->gsUserdataBufSize );
	}
	
	//------------------------------------------------------------
	// [x] code buffer, work buffer and parameter buffer for VPU 
	//------------------------------------------------------------
	pInst->gsBitWorkBufSize = WORK_CODE_PARA_BUF_SIZE;
	pInst->gsBitWorkBufSize = ALIGNED_BUFF(pInst->gsBitWorkBufSize, ALIGN_LEN);
	if(pInst->vdec_instance_index == 0)
	{
		pInst->gsBitWorkBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitWorkBufAddr[K_VA], pInst->gsBitWorkBufSize, BUFFER_WORK, pInst);
	}
	else
	{
		pInst->gsBitWorkBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsBitWorkBufAddr[K_VA], pInst->gsBitWorkBufSize, BUFFER_WORK, pInst);
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
vpu_dec_seq_header( int iSize , dec_initial_info_t * psInitialInfo, _vdec_ *pVdec )
{
	int ret = 0;
	_vdec_ * pInst = pVdec;
//	DSTATUS("[VDEC-%d]vpu_dec_seq_header in :: size(%d), JpegOnly(%d), format(%d)", pInst->vdec_instance_index, iSize ,pInst->gsVpuDecUserInfo.m_bJpegOnly, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat);
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
	if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat == STD_AVC )
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

#if (defined(TCC_892X_INCLUDE) && !defined(TCC_8925S_INCLUDE)) || (defined(TCC_893X_INCLUDE) && !defined(TCC_8935S_INCLUDE))
        pInst->gsMbSaveSize = 0;
        pInst->gsMbSaveAddr = 0;
#endif

	//------------------------------------------------------------
	// [x] frame buffer for each VPU decoder
	//------------------------------------------------------------
	pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount;
	DSTATUS( "[VDEC-%d] FrameBufDelay %d\n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iFrameBufDelay );
	DSTATUS( "[VDEC-%d] MinFrameBufferCount %d\n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount );
#ifdef HAVE_ANDROID_OS
	{
		int max_count;
		
		pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount + pInst->gsAdditionalFrameCount;
		max_count = cdk_sys_remain_memory_size(pInst) / pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;

		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > max_count)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = max_count;
		
		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount > MAX_FRAME_BUFFER_COUNT)
			pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount = MAX_FRAME_BUFFER_COUNT;

		if(pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount < (pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount))
		{
			DPRINTF( "[VDEC-%d] Not enough memory for VPU frame buffer, Available[%d], Min[%d], Need[%d]", pInst->vdec_instance_index, max_count, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount);
			return -(VPU_NOT_ENOUGH_MEM);
		}
	}
#else	
#endif

	pInst->gsFrameBufSize = pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount * pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize;
	DPRINTF( "[VDEC-%d] FrameBufferCount %d [min %d], min_size = %d \n", pInst->vdec_instance_index, pInst->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize);

	pInst->gsFrameBufSize = ALIGNED_BUFF( pInst->gsFrameBufSize, ALIGN_LEN );
	pInst->gsFrameBufAddr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(&pInst->gsFrameBufAddr[K_VA], pInst->gsFrameBufSize, 0, pInst );
	if( pInst->gsFrameBufAddr[PA] == 0 ) 
	{
		DPRINTF( "[VDEC-%d] frame_buf_addr[PA] malloc() failed \n", pInst->vdec_instance_index);
		return -(VPU_NOT_ENOUGH_MEM);
	}

	DSTATUS( "[VDEC-%d] MinFrameBufferSize %d bytes \n", pInst->vdec_instance_index, pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferSize );
	DSTATUS( "[VDEC-%d] frame_buf_addr[PA] = 0x%x, 0x%x \n", pInst->vdec_instance_index, (codec_addr_t)pInst->gsFrameBufAddr[PA], pInst->gsFrameBufSize );
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
vdec_vpu( int iOpCode, int* pHandle, void* pParam1, void* pParam2, void* pParam3 )
{
	int ret = 0;
	_vdec_ *pInst = (_vdec_ *)pParam3;
	int	handle = (int)pHandle;
	if( iOpCode != VDEC_INIT && iOpCode != VDEC_CLOSE && !pInst->vdec_codec_opened)
		return -RETCODE_NOT_INITIALIZED;

	DBUG_FLIP("[VDEC-%d] ==> OpCode : %02d", pInst->vdec_instance_index, iOpCode);

	if( iOpCode == VDEC_INIT )
	{
		vdec_init_t* p_input_param = (vdec_init_t*)pParam1;
		unsigned int bitrate_mbps = *((unsigned int*)pParam2);

#ifdef HAVE_ANDROID_OS		
		if(vpu_env_open(handle, p_input_param->m_iBitstreamFormat, 30, 30, 1920, 1080, pInst ) < 0)
		{
			DPRINTF( "[VDEC-%d] ret : %02d, vpu_env_open error!!\n", pInst->vdec_instance_index, -VPU_ENV_INIT_ERROR );
			return -VPU_ENV_INIT_ERROR;	
		}
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
#endif

#ifdef VPU_PERFORMANCE_UP
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
	#if defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags = SEC_AXI_BUS_DISABLE;
	#endif
#endif

		pInst->gsVpuDecInit_Info.gsVpuDecInit.m_uiDecOptFlags |= p_input_param->m_uiDecOptFlags;

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
			DPRINTF( "[VDEC-%d] ret : %02d, vpu_dec_ready error!!\n", pInst->vdec_instance_index, ret );
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

		DSTATUS("[VDEC-%d]Format : %d, Stream(0x%x, %d)", pInst->vdec_instance_index, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamFormat, pInst->gsVpuDecInit_Info.gsVpuDecInit.m_BitstreamBufAddr[PA], pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iBitstreamBufSize);
	
		ret = vdec_cmd_process(V_DEC_INIT, &pInst->gsVpuDecInit_Info, pInst);
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_INIT failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
		DPRINTF( "[VDEC-%d] VPU_DEC_INIT OK( has seq = %d) \n", pInst->vdec_instance_index, pInst->gsbHasSeqHeader );

		pInst->gsVpuDecVersion.pszVersion = pInst->gsBitstreamBufAddr[K_VA] + (pInst->gsBitstreamBufSize - 100);
		pInst->gsVpuDecVersion.pszBuildData = pInst->gsBitstreamBufAddr[K_VA] + (pInst->gsBitstreamBufSize - 50);

#if !defined(TCC_88XX_INCLUDE) && !defined(TCC_8925S_INCLUDE) && !defined(TCC_8935S_INCLUDE)
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
				DPRINTF("Multi-instance status : %d/%d/%d/%d/%d", vpu_closed[VPU_DEC], vpu_closed[VPU_DEC_EXT], vpu_closed[VPU_ENC], vpu_closed[VPU_DEC_EXT2], vpu_closed[VPU_DEC_EXT3]);
			}
			DPRINTF( "[VDEC-%d] V_GET_VPU_VERSION OK. Version is %s, and it's built at %s \n", pInst->vdec_instance_index,
					(char*)(pInst->gsBitstreamBufAddr[VA] + (pInst->gsBitstreamBufSize - 100)),
					(char*)(pInst->gsBitstreamBufAddr[VA] + (pInst->gsBitstreamBufSize - 50)));
		}
#endif
		pInst->vdec_codec_opened = 1;

#ifdef VPU_REGISTER_DUMP
		pInst->bFirst_frame = 1;
		filewrite_memory("sdcard/after_init.bin", pInst->gsRegisterBase, 0x200, pInst);
#endif
	}
	else if( iOpCode == VDEC_DEC_SEQ_HEADER )
	{		
#ifdef HAVE_ANDROID_OS
		vdec_input_t* p_input_param = (vdec_input_t*)pParam1;
		vdec_output_t* p_output_param = (vdec_output_t*)pParam2;
		int seq_stream_size = (p_input_param->m_iInpLen > pInst->gsMaxBitstreamSize) ? pInst->gsMaxBitstreamSize : p_input_param->m_iInpLen;

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
			}

			print_frame_data("/sdcard/vpu_inSeq.bin", seq_stream_size, pInst);

			unsigned char* ps = (unsigned char*)pInst->gsBitstreamBufAddr[VA];
			DPRINTF( "[VDEC-%d Seq] 0x%x 0x%x 0x%x 0x%x 0x%x", pInst->vdec_instance_index, ps[0], ps[1], ps[2], ps[3], ps[4] );
		}
		else
		{
			seq_stream_size = 1;
		}
#else
		int seq_stream_size = (int)pParam1;
#endif

		DSTATUS( "[VDEC-%d] VDEC_DEC_SEQ_HEADER start  :: len = %d / %d \n", pInst->vdec_instance_index, seq_stream_size, p_input_param->m_iInpLen);
		ret = vpu_dec_seq_header(seq_stream_size, NULL/*pParam2*/, pInst);
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
				DSTATUS("[VDEC-%d]VPU OutFormat is YUV422SEQ0 ", pInst->vdec_instance_index);
			}
		}
		
		DPRINTF( "[VDEC-%d] VDEC_DEC_SEQ_HEADER - Success mem_free = 0x%x \n", pInst->vdec_instance_index, cdk_sys_remain_memory_size(pInst) );
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
		if( pInst->gsVpuDecInit_Info.gsVpuDecInit.m_iFilePlayEnable ) 
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
			}
			print_frame_data("sdcard/vpu_inDec.bin", pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize, pInst); 	
		}
		else
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[PA] = pInst->gsBitstreamBufAddr[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_BitstreamDataAddr[VA] = pInst->gsBitstreamBufAddr[K_VA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iBitstreamDataSize = 1;
		}
#else
#endif

		if(pInst->gsVpuDecInit_Info.gsVpuDecInit.m_bEnableUserData)
		{		
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = pInst->gsUserdataBufAddr[PA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = pInst->gsUserdataBufAddr[K_VA];
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = pInst->gsUserdataBufSize;
		}
		
//		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[PA] = (codec_addr_t)p_input_param->m_UserDataAddr[PA];
//		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_UserDataAddr[VA] = (codec_addr_t)p_input_param->m_UserDataAddr[VA];
//		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iUserDataBufferSize = p_input_param->m_iUserDataBufferSize;
		
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode = p_input_param->m_iSkipFrameMode;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable = p_input_param->m_iFrameSearchEnable;
		pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = 0;
		if( pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameMode > 0 || pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iFrameSearchEnable > 0 )
		{
			pInst->gsVpuDecInOut_Info.gsVpuDecInput.m_iSkipFrameNum = p_input_param->m_iSkipFrameNum;
		}

		// Start decoding a frame.
		ret = vdec_cmd_process(V_DEC_DECODE, &pInst->gsVpuDecInOut_Info, pInst);
		pInst->total_frm++;

#if 0//def VPU_PERFORMANCE_UP
		{
	         unsigned int regAddr = ((unsigned int)gsRegisterBase + 0x10000); //0xB0910000

	         //VCACHE_CTRL
	         *(volatile unsigned int *)(regAddr+0x08) =      (1<<0);           //DRAIN
	   }
#endif
	
//		if( ret == VPU_DEC_FINISH )
//			return ERR_END_OF_FILE;
		if(ret != RETCODE_SUCCESS && ret != 0x14)
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_DECODE failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}
		else if(ret == 0x14)
		{
			DBUG_FLIP("[VDEC-%d] VPU_DEC_DECODE failed Error code is 0x%x \n", pInst->vdec_instance_index, ret);
			return -ret;
		}

		if(pInst->gsVpuDecInOut_Info.gsVpuDecOutput.m_DecOutInfo.m_iPicType == 0){
			DSTATUS( "[VDEC-%d] I-Frame (%d)", pInst->vdec_instance_index, pInst->total_frm);
		}

#ifdef HAVE_ANDROID_OS
		memcpy((void*)p_output_param, (void*)&pInst->gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
#else
#endif
		p_output_param->m_pInitialInfo = &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;

		p_output_param->m_pDispOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][0], K_VA, pInst);
		p_output_param->m_pDispOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst);
		p_output_param->m_pDispOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA, pInst);

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
//			DPRINTF("Displayed addr 0x%x", p_output_param->m_pDispOut[PA][0]);
		}

		print_frame_data("sdcard/vpu_outDec.bin", p_input_param->m_iInpLen, pInst); 	
#ifdef VPU_REGISTER_DUMP
		filewrite_memory("sdcard/after_decode.bin", pInst->gsRegisterBase, 0x200, pInst);
		pInst->bFirst_frame = 0;
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
	#if defined(TCC_88XX_INCLUDE) || defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
		p_out_param->m_ulAvailableSpaceInRingBuffer = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_iAvailableSpaceInRingBuffer;
	#else
		p_out_param->m_ulAvailableSpaceInRingBuffer = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ulAvailableSpaceInRingBuffer;
		p_out_param->m_ptrReadAddr_PA = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrReadAddr_PA;
		p_out_param->m_ptrWriteAddr_PA = pInst->gsVpuDecBufStatus.gsVpuDecRingStatus.m_ptrWriteAddr_PA;
//		DPRINTF("[VDEC-%d] [AVAIL: %8d] [RP: 0x%08X / WP: 0x%08X]", pInst->vdec_instance_index
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
		DBUG_FLIP("[VDEC-%d] ==> Result : %02d", pInst->vdec_instance_index, ret);
		return 0;
	}
	else if( iOpCode == VDEC_UPDATE_WRITE_BUFFER_PTR )
	{
		pInst->gsVpuDecUpdateWP.iCopiedSize = (int)pParam1;
		pInst->gsVpuDecUpdateWP.iFlushBuf = (int)pParam2;

		ret = vdec_cmd_process(V_DEC_UPDATE_RINGBUF_WP, &pInst->gsVpuDecUpdateWP,  pInst);  // fille the Ring Buffer 

		DBUG_FLIP( "[VDEC-%d] iCopiedSize:%d, iFlushBuf:%d, ret:%d\n", pInst->vdec_instance_index, pInst->gsVpuDecUpdateWP.iCopiedSize, pInst->gsVpuDecUpdateWP.iFlushBuf, ret );

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
			DPRINTF( "[VDEC-%d] VPU_DEC_FLUSH_OUTPUT failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			return -ret;
		}

		memcpy((void*)p_output_param, (void*)&pInst->gsVpuDecInOut_Info.gsVpuDecOutput, sizeof(dec_output_t ) );
		p_output_param->m_pInitialInfo = &pInst->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo;

		p_output_param->m_pDispOut[VA][0] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][0], K_VA, pInst);
		p_output_param->m_pDispOut[VA][1] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][1], K_VA, pInst);
		p_output_param->m_pDispOut[VA][2] = (unsigned char *)vpu_getFrameBufVirtAddr(p_output_param->m_pDispOut[VA][2], K_VA, pInst);

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
		ret = vdec_cmd_process(V_DEC_CLOSE, &pInst->gsVpuDecInOut_Info, pInst);
		if( ret != RETCODE_SUCCESS )
		{
			DPRINTF( "[VDEC-%d] VPU_DEC_CLOSE failed Error code is 0x%x \n", pInst->vdec_instance_index, ret );
			ret = -ret;
		}
		
		pInst->vdec_codec_opened = 0;
				
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
#if defined(VPU_CLK_CONTROL)
		vpu_clock_deinit();
#endif
#ifdef HAVE_ANDROID_OS
		vpu_env_close(handle, pInst);
#endif
	}
#if 1
	else if( iOpCode == VDEC_HW_RESET)
	{
		vdec_hw_reset_flag = 1;
	}
#endif
	else
	{
		DPRINTF( "[VDEC-%d] Invalid Operation!!\n", pInst->vdec_instance_index );
		return -ret;
	}

	return ret;
}

void * vdec_alloc_instance(void)
{
	int nInstance;
	_vdec_ *pInst = (_vdec_*)TCC_malloc(sizeof(_vdec_));
	if(pInst)
	{
		memset(pInst, 0x00, sizeof(_vdec_));
	
		pInst->vpu_dec_fd = -1;
#ifdef USE_VPU_INTERRUPT
		pInst->vpu_intr_fd = -1;
#endif
		pInst->vpu_mgr_fd = open(VPU_MGR_NAME, O_RDWR);
		if(pInst->vpu_mgr_fd < 0)
		{
			ALOGE("%s open error[%s]!!", VPU_MGR_NAME, strerror(errno));
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
		ALOGI("vdec_alloc_instance %d, total %d", nInstance, vdec_open_count);
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
				ALOGE("%s close error", VPU_MGR_NAME);
			}
			pVdec->vpu_mgr_fd = -1;
		}

		TCC_free(pVdec);
		pInst = pVdec = NULL;

		vdec_open_count--;
		if(vdec_open_count < 0)
			vdec_open_count = 0;

		ALOGI("vdec_release_instance %d, total %d", used, vdec_open_count);
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
		//pVdec->renderered = 1;
	}	
}

int vdec_is_rendered_index(void * pInst)
{
	if(pInst)
	{
		_vdec_ * pVdec = (_vdec_ *) pInst;
		//return pVdec->renderered;
	}
	
	return 0;
}

int vpu_get_frame_count(int safety_count, void * pInst)
{
	_vdec_ *pVdec = (_vdec_ *)pInst;
	int fifo_frame = (pVdec->gsVpuDecBuffer_Info.gsVpuDecBuffer.m_iFrameBufferCount - pVdec->gsVpuDecSeqHeader_Info.gsVpuDecInitialInfo.m_iMinFrameBufferCount) -1;

	
	if(fifo_frame < safety_count)
	{
		DPRINTF("CAUTION!! fifo_frame count is very low. keep 0x%x frame!!", safety_count);
	}

	return fifo_frame;
}
