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
#if defined(HAVE_ANDROID_OS)
#define LOG_TAG	"JPU_CAM"
#include <utils/Log.h>

#include "cdk_core.h"
#include "jpu_jpeg.h"
#include "TCCMemory.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif

#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <libpmap/pmap.h>

#if defined(JPEG_ENCODE_FOR_MJPEG)
#define JPEG_BASE_REG 								0x75300000
#define TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE 	112
#define TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE 		113
extern int gcam_fd;
#endif

/************************************************************************/
/* TEST and Debugging                                                   */
/************************************************************************/
static int DEBUG_ON				= 1;
#define DPRINTF(msg...) 		ALOGE( ": " msg);
#define DSTATUS(msg...) 		if (DEBUG_ON) { ALOGD( ": " msg);}

static unsigned int total_frm = 0;

#if defined(_TCC8800_) || defined(_TCC9300_)
#define VPU_PERFORMANCE_UP //use sram and vcache
#endif

#define USE_CLK_CTRL

#else // HAVE_ANDROID_OS

#include "../cdk/cdk_core.h"
#include "../cdk/cdk_sys.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif

#endif // HAVE_ANDROID_OS

static unsigned char jpu_codec_opened = 0;
int dev_fd = -1;

#if defined(USE_CLK_CTRL)
#include <fcntl.h>
#include <sys/poll.h>
#define TCC_CLK_DEV_NAME		"/dev/clockctrl"
int jpu_clk_fd = -1;
#endif


/************************************************************************/
/* STATIC MEMBERS                                                       */
/************************************************************************/
static pmap_t pmap_video;
static pmap_t pmap_jpeg_stream;
static pmap_t pmap_jpeg_raw;

#define VIDEO_PHY_ADDR 				pmap_video.base
#define VIDEO_MEM_SIZE 					pmap_video.size
#define PA_JPEG_STREAM_BASE_ADDR 		pmap_jpeg_stream.base
#define JPEG_STREAM_MEM_SIZE 			pmap_jpeg_stream.size
#define PA_JPEG_RAW_BASE_ADDR 		pmap_jpeg_raw.base
#define JPEG_RAW_MEM_SIZE 				pmap_jpeg_raw.size
#define JPEG_TATAL_MEM_SIZE 			(JPEG_STREAM_MEM_SIZE + JPEG_RAW_MEM_SIZE)
#define JPU_PHY_ADDR 					(VIDEO_PHY_ADDR + JPEG_TATAL_MEM_SIZE)
#define JPU_MEM_SIZE 					(VIDEO_MEM_SIZE - JPEG_TATAL_MEM_SIZE) 

#if defined(JPEG_DECODE_FOR_MJPEG)
static codec_handle_t 		gsJpuDecHandle = 0;
static jpu_dec_init_t 		gsJpuDecInit;
static jpu_dec_initial_info_t 	gsJpuDecInitialInfo;
static jpu_dec_input_t 		gsJpuDecInput;
static jpu_dec_output_t 		gsJpuDecOutput;
static jpu_dec_buffer_t 		gsJpuDecBuffer;
static codec_addr_t 		gsbitstream_buf_addr[2] = {0, }, gsframe_buf_addr[2] = {0, };
static int 					gbitstream_buf_size = 0, gdecoded_frame_buf_size = 0, gframe_buf_size_init = 0;
static int 					gbitstream_buf_dev = 0, gdecoded_frame_buf_dev = 0;
#endif

#if defined(JPEG_ENCODE_FOR_MJPEG)
static jpu_enc_init_t 	gsenc_init;
static jpu_enc_input_t 	gsenc_input;
static jpu_enc_output_t 	gsenc_output;
static unsigned int 		gsUsedPhysicalMemSize = 0, gsLogicalBase = 0, gsRegisterBase = 0; 
static codec_addr_t 	gencoded_out_buf_addr[2] = {0, }, gframe_buf_addr[2] = {0, };
static unsigned int 		gencoded_out_buf_size = 0, gframe_buf_size = 0;
#endif

#if defined(JPEG_DECODE_FOR_MJPEG)
//#include <mach/tcc_jpeg_ioctl.h>
#endif


#if defined(HAVE_ANDROID_OS)
static unsigned char * sPhysicalMemSetting(unsigned int nPhysicalAddr, unsigned int size)
{	
	void *map_ptr = (void *)mmap(0, size, (PROT_READ|PROT_WRITE), MAP_SHARED, dev_fd, nPhysicalAddr);
	
	if(map_ptr == MAP_FAILED) {
		ALOGE("mmap failed. fd(%d), addr(0x%x), size(%d)", dev_fd, nPhysicalAddr, size);
		return NULL;
	}

	return (unsigned char *)map_ptr;
}

static void sPhysicalMemFree(unsigned int LogicalAddr, unsigned int size)
{
	int ret = munmap((void*)LogicalAddr, size);

	if(ret < 0) {
		ALOGE("munmap failed. addr(0x%x), size(%d)", LogicalAddr, size);
	}
}

static void *cdk_sys_malloc_physical_addr(int uiSize)
{
	codec_addr_t phsical_addr = JPU_PHY_ADDR + gsUsedPhysicalMemSize;

	gsUsedPhysicalMemSize += uiSize;
	if(gsUsedPhysicalMemSize > JPU_MEM_SIZE) {
		ALOGE("sys_malloc_physical_addr error! %d bytes \n", gsUsedPhysicalMemSize);
	}
	
	return (void*)( phsical_addr );
}

static void cdk_sys_free_physical_addr(void* pPtr, int uiSize)
{
	gsUsedPhysicalMemSize -= uiSize;
	return;
}

static void *cdk_sys_malloc_virtual_addr(int* pDev, codec_addr_t pPtr, int uiSize)
{
	codec_addr_t used_size = pPtr - JPU_PHY_ADDR;
	return (void*)( gsLogicalBase + used_size);
}

static void cdk_sys_free_virtual_addr(int* pDev, codec_addr_t pPtr, int uiSize)
{
	// todo : 
}

static unsigned int cdk_sys_remain_memory_size(void)
{
	return (JPU_MEM_SIZE - gsUsedPhysicalMemSize);
}

int jpu_dec_open(void)
{
	int vpu_reset = 0;
	int ret_val;
	
	DSTATUS("In  %s \n",__func__);

	pmap_get_info("video", 		&pmap_video);
	pmap_get_info("jpeg_stream", 	&pmap_jpeg_stream);
	pmap_get_info("jpeg_raw", 	&pmap_jpeg_raw);

	DSTATUS("jpeg_stream size[0x%x] base[0x%x] jpeg_raw size[0x%x] base[0x%x]", pmap_jpeg_stream.size, 	\
		pmap_jpeg_stream.base, 	pmap_jpeg_raw.size, pmap_jpeg_raw.base);
	
	jpu_clk_fd = open(TCC_CLK_DEV_NAME, O_WRONLY);
	if(!jpu_clk_fd) {
		ALOGE("%s open error!!", TCC_CLK_DEV_NAME);
		goto err;
	}

	ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE, &ret_val);

	// connect fd to dev_fd
	dev_fd = gcam_fd;
	DSTATUS("dev_fd[0x%x] JPU_MEM_SIZE[0x%x]", dev_fd, JPU_MEM_SIZE);

	gsLogicalBase = (unsigned int)sPhysicalMemSetting(JPU_PHY_ADDR, JPU_MEM_SIZE);
	if(!gsLogicalBase) {
		goto err;
	}

	gsRegisterBase	= (unsigned int)sPhysicalMemSetting(JPEG_BASE_REG, (8*1024));
	if(!gsRegisterBase) {
		goto err;
	}
	DSTATUS("[VDEC] gsLogicalBase = 0x%x, gsRegisterBase = 0x%x", (codec_addr_t)gsLogicalBase, (codec_addr_t)gsRegisterBase);

	gsUsedPhysicalMemSize = 0;
	jpu_codec_opened = 1;

	memset(&gsJpuDecInit, 0x00, sizeof(jpu_dec_init_t));
	memset(&gsJpuDecInitialInfo, 0x00, sizeof(jpu_dec_initial_info_t));
	memset(&gsJpuDecInput, 0x00, sizeof(jpu_dec_input_t));
	memset(&gsJpuDecOutput, 0x00, sizeof(jpu_dec_output_t));
	memset(&gsJpuDecBuffer, 0x00, sizeof(jpu_dec_buffer_t));
	memset(&gsbitstream_buf_addr, 0x00, sizeof(gsbitstream_buf_addr));
	memset(&gsframe_buf_addr, 0x00, sizeof(gsframe_buf_addr));
	
	DSTATUS("Out  %s \n",__func__);
	total_frm = 0;

	return 0;

err:	
	ALOGE("jpu_dec_open error");
	jpu_dec_close();
	
	return -1;	
}

void jpu_dec_close(void)
{
	int vpu_reset, ret_val;
	
	DSTATUS("In  %s \n",__func__);

	if(gsLogicalBase) {
		sPhysicalMemFree(gsLogicalBase, JPU_MEM_SIZE);
	}

	if(gsRegisterBase) {
		sPhysicalMemFree(gsRegisterBase, 8*1024);
	}

	gsLogicalBase = 0;
	gsRegisterBase = 0;

	ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE, &ret_val);
	if(jpu_clk_fd) {
		if(close(jpu_clk_fd) < 0) {
			ALOGE("Err[%s] :: vpu(%s) close failed!", strerror(errno), TCC_CLK_DEV_NAME);
		}
		dev_fd = -1;
	}

	jpu_codec_opened = 0;
	DSTATUS("Out  %s \n",__func__);
}

int jpu_enc_open()
{
	int vpu_reset = 0;
	int ret_val;
	
	DSTATUS("In  %s \n",__func__);

	pmap_get_info("video",		 &pmap_video);
	pmap_get_info("jpeg_stream", &pmap_jpeg_stream);
	pmap_get_info("jpeg_raw",	 &pmap_jpeg_raw);

	jpu_clk_fd = open(TCC_CLK_DEV_NAME,O_WRONLY);
	if(!jpu_clk_fd) {
		ALOGE("%s open error!!", TCC_CLK_DEV_NAME);
		goto err;
	}

	ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE, &ret_val);

	// connect fd to dev_fd
	dev_fd = gcam_fd;
	DSTATUS("dev_fd[0x%x] JPU_MEM_SIZE[0x%x]",dev_fd, JPU_MEM_SIZE);

	gsLogicalBase = (unsigned int)sPhysicalMemSetting(JPU_PHY_ADDR, JPU_MEM_SIZE);
	if(!gsLogicalBase) {
		goto err;
	}

	gsRegisterBase	= (unsigned int)sPhysicalMemSetting(JPEG_BASE_REG, 8*1024);
	if(!gsRegisterBase) {
		goto err;
	}
	DSTATUS("[JPU ENC] gsLogicalBase = 0x%x, gsRegisterBase = 0x%x", (codec_addr_t)gsLogicalBase, (codec_addr_t)gsRegisterBase);

	gsUsedPhysicalMemSize = 0;
	jpu_codec_opened = 1;

	memset(&gsenc_init, 0x00, sizeof(jpu_enc_init_t));
	memset(&gsenc_input, 0x00, sizeof(jpu_enc_input_t));
	memset(&gsenc_output, 0x00, sizeof(jpu_enc_output_t));

	memset(&gencoded_out_buf_addr, 0x00, sizeof(gencoded_out_buf_addr));
	memset(&gframe_buf_addr, 0x00, sizeof(gframe_buf_addr));
	
	DSTATUS("Out  %s \n",__func__);
	total_frm = 0;

	return 0;

err:	
	ALOGE("jpu_enc_open error");
	jpu_enc_close();
	
	return -1;	
}


void jpu_enc_close(void)
{
	int vpu_reset, ret_val;
	
	DSTATUS("In  %s \n",__func__);

	if(gsLogicalBase) {
		sPhysicalMemFree(gsLogicalBase, JPU_MEM_SIZE);
	}

	if(gsRegisterBase) {
		sPhysicalMemFree(gsRegisterBase, 8*1024);
	}

	gsLogicalBase = 0;
	gsRegisterBase = 0;

	ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE, &ret_val);
	if(jpu_clk_fd) {
		if(close(jpu_clk_fd) < 0) {
			ALOGE("Err[%s] :: vpu(%s) close failed!", strerror(errno), TCC_CLK_DEV_NAME);
		}
		dev_fd = -1;
	}

	jpu_codec_opened = 0;
	DSTATUS("Out  %s \n",__func__);
}
#endif

#if defined(JPEG_DECODE_FOR_MJPEG)
int tcc_jpu_dec_jpeg(int iOpCode, void* pHandle, void* pParam1, void* pParam2)
{
	int 				stride;
	codec_result_t 	ret_err = JPG_RET_SUCCESS, ret = 0;
	conf_dec_t 		*pConfDec = (conf_dec_t *)pParam1;

	jpu_dec_open();

	//------------------------------------------------------------
	//! [x] bitstream buffer for each JPU decoder
	//------------------------------------------------------------
	// gsBitStreamBufSize = 0xA00000; 		//LARGE_STREAM_BUF_SIZE;
	gbitstream_buf_size = pConfDec->m_InpSizeOfJpeg; 	//holic 12_06_07 MEMORY_INPUT;
	gbitstream_buf_size = ALIGNED_BUFF(gbitstream_buf_size, (4*1024));

	gsbitstream_buf_addr[PA] = pConfDec->m_InpPhyJpegBuffAddr;
	if(gsbitstream_buf_addr[PA] == 0) {
		DPRINTF("gsBitStreamBufAddr[PA] malloc() failed. \n");
		goto ERR_JPU_DEC_INIT;
	}
	DSTATUS("gsbitstream_buf_addr[PA] = 0x%x , gbitstream_buf_size = 0x%x. \n", (codec_addr_t)gsbitstream_buf_addr[PA], gbitstream_buf_size);

	gsbitstream_buf_addr[VA] = pConfDec->m_InpVirtJpegBuffAddr;
	if(gsbitstream_buf_addr[VA] == 0) {
		DPRINTF("gsBitStreamBufAddr[VA] malloc() failed. \n");
		goto ERR_JPU_DEC_INIT;
	}
	DSTATUS("gsbitstream_buf_addr[VA] = 0x%x , gbitstream_buf_size = 0x%x. \n", (codec_addr_t)gsbitstream_buf_addr[VA], gbitstream_buf_size);

	// Set Decoder Init
	gsJpuDecInit.m_iBitstreamSize 				= gbitstream_buf_size; // pConfDec->m_InpSizeOfJpeg;
	gsJpuDecInit.m_iBitstreamBufSize 			= gbitstream_buf_size;
	gsJpuDecInit.m_RegBaseVirtualAddr 			= gsRegisterBase;
	gsJpuDecInit.m_BitstreamBufAddr[PA] 		= gsbitstream_buf_addr[PA];
	gsJpuDecInit.m_BitstreamBufAddr[VA] 		= gsbitstream_buf_addr[VA];
	gsJpuDecInit.m_iRot_angle 				= pConfDec->m_iRotMode;
	gsJpuDecInit.m_iRot_enalbe 				= pConfDec->m_iRotEnable;
	gsJpuDecInit.m_iMirror_enable 				= pConfDec->m_iMirrorEnable;
	gsJpuDecInit.m_iMirrordir 					= pConfDec->m_iMirrorDir;
	gsJpuDecInit.m_bCbCrInterleaveMode 		= pConfDec->m_bChromaInterleaved;
	gsJpuDecInit.m_Memcpy 					= (void* (*) ( void*, const void*, unsigned int ))memcpy;
	gsJpuDecInit.m_Memset 					= (void  (*) ( void*, int, unsigned int ))memset;
	//jpu_dec_init.m_Interrupt 					= (int  (*) ( void ))sys_interrupt;

	ret = TCC_JPU_DEC(JPU_DEC_INIT, &gsJpuDecHandle, &gsJpuDecInit, &gsJpuDecInitialInfo);
	if(ret != JPG_RET_SUCCESS) {
		ret_err = ret;
		DPRINTF("JPU_DEC_INIT failed Error code is 0x%x. \n", ret);
		goto ERR_JPU_DEC_INIT;
	}
	DSTATUS("JPU_DEC_INIT OK. \n");
	DSTATUS("gsJpuDecHandle :  0x%x. \n", (int)gsJpuDecHandle);
	DSTATUS("gsJpuDecInitialInfo.m_iPicWidth = %d. \n", gsJpuDecInitialInfo.m_iPicWidth);
	DSTATUS("gsJpuDecInitialInfo.m_iPicHeight = %d. \n", gsJpuDecInitialInfo.m_iPicHeight);
	DSTATUS("gsJpuDecInitialInfo.m_iJpg_sourceFormat = %d. \n", gsJpuDecInitialInfo.m_iJpg_sourceFormat);

	//--------------------------------------------------------
	// [*] JPU_CODEC_GET_VERSION operation
	//--------------------------------------------------------
	{
		char psz_version[50];
		char psz_build_date[50];

		DSTATUS("/************************************************************************/ \n");
		DSTATUS("JPU_CODEC_GET_VERSION Start. \n");
		ret = TCC_JPU_DEC(JPU_CODEC_GET_VERSION, &gsJpuDecHandle, psz_version, psz_build_date);
		if(ret < 0) {
			DPRINTF("JPU_CODEC_GET_VERSION failed Error code is 0x%08X. \n", ret);
			goto ERR_JPU_DEC_INIT;
		}
		DSTATUS("JPU_CODEC_GET_VERSION OK. VER :  %s , Build Date :  %s. \n", psz_version, psz_build_date);
		DSTATUS("/************************************************************************/ \n");
	}

	//------------------------------------------------------------
	//! [x] frame buffer for each JPU decoder
	//------------------------------------------------------------

	stride = (gsJpuDecInitialInfo.m_iPicWidth + 15) & ~15;

	// scale factor set
	gsJpuDecBuffer.m_iJPGScaleRatio = 0; 	// JPEG Scaling Ratio

	// scale에 따라서 버퍼 size의 크기가 틀려진다.
	gdecoded_frame_buf_size 	= gsJpuDecInitialInfo.m_iJpg_MinFrameBufferSize[gsJpuDecBuffer.m_iJPGScaleRatio];
	gframe_buf_size_init 		= gdecoded_frame_buf_size;		// 20110923 woo
	gdecoded_frame_buf_size 	= ALIGNED_BUFF(gdecoded_frame_buf_size, (4*1024));

	gsframe_buf_addr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr(gdecoded_frame_buf_size);
	if(gsframe_buf_addr[PA] == 0) {
		DPRINTF("gsframe_buf_addr[PA] malloc() failed. \n");
		goto ERR_JPU_DEC_INIT;
	}
	DSTATUS("gsframe_buf_addr[PA] = 0x%x , gdecoded_frame_buf_size = 0x%x. \n", (codec_addr_t)gsframe_buf_addr[PA], gdecoded_frame_buf_size);

	gsframe_buf_addr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr( &gdecoded_frame_buf_dev, gsframe_buf_addr[PA], gdecoded_frame_buf_size);
	if(gsframe_buf_addr[VA] == 0) {
		DPRINTF("gsframe_buf_addr[VA] malloc() failed. \n");
		goto ERR_JPU_DEC_INIT;
	}
	DSTATUS("gsframe_buf_addr[VA] = 0x%x , gdecoded_frame_buf_size = 0x%x. \n", (codec_addr_t)gsframe_buf_addr[VA], gdecoded_frame_buf_size);

	gsJpuDecBuffer.m_FrameBufferStartAddr[PA] = gsframe_buf_addr[PA];
	gsJpuDecBuffer.m_FrameBufferStartAddr[VA] = gsframe_buf_addr[VA];

	ret = TCC_JPU_DEC( JPU_DEC_REG_FRAME_BUFFER, &gsJpuDecHandle, &gsJpuDecBuffer, 0);
	if(ret != JPG_RET_SUCCESS) {
		ret_err = ret;
		DPRINTF("JPU_DEC_REG_FRAME_BUFFER failed Error code is 0x%x. \n", ret );
		goto ERR_JPU_DEC_INIT;
	}
	DSTATUS("JPU_DEC_REG_FRAME_BUFFER OK. \n");

	DSTATUS("-------------------DECODER RUN------------------- \n");

	gsJpuDecInput.m_BitstreamDataAddr[PA] 		= gsbitstream_buf_addr[PA];
	gsJpuDecInput.m_BitstreamDataAddr[VA] 		= gsbitstream_buf_addr[VA];
	gsJpuDecInput.m_iBitstreamDataSize 			= gbitstream_buf_size;
	gsJpuDecInput.m_iLooptogle 					= 0;
	DSTATUS("gsJpuDecInput.m_BitstreamDataAddr[PA] = 0x%x , gsJpuDecInput.m_BitstreamDataAddr[VA]  = 0x%x.", gsJpuDecInput.m_BitstreamDataAddr[PA], gsJpuDecInput.m_BitstreamDataAddr[VA]);

	// Start decoding a frame.
	ret = TCC_JPU_DEC( JPU_DEC_DECODE, &gsJpuDecHandle, &gsJpuDecInput, &gsJpuDecOutput);
	if(ret != JPG_RET_SUCCESS) {
		ret_err = ret;
		DPRINTF( "JPU_DEC_DECODE failed Error code is 0x%x \n", ret );
		goto ERR_JPU_DEC_OPEN;
	}

	pConfDec->m_OutPhyYAddr 	= gsJpuDecOutput.m_pCurrOut[PA][COMP_Y];
	pConfDec->m_OutPhyUAddr 	= gsJpuDecOutput.m_pCurrOut[PA][COMP_U];
	pConfDec->m_OutPhyVAddr 	= gsJpuDecOutput.m_pCurrOut[PA][COMP_V];
	pConfDec->m_OutWidth 		= gsJpuDecOutput.m_DecOutInfo.m_iWidth;
	pConfDec->m_OutHeight 		= gsJpuDecOutput.m_DecOutInfo.m_iHeight;
	pConfDec->m_OutImgFmt 		= gsJpuDecInitialInfo.m_iJpg_sourceFormat;
	
	DSTATUS("Decoded YUV Address :   Y_addr = 0x%x , U_addr = 0x%x , V_addr = 0x%x. \n", pConfDec->m_OutPhyYAddr, pConfDec->m_OutPhyUAddr, pConfDec->m_OutPhyVAddr);
	DSTATUS("Decoded Width = %d , Height = %d. \n", gsJpuDecOutput.m_DecOutInfo.m_iWidth, gsJpuDecOutput.m_DecOutInfo.m_iHeight);

ERR_JPU_DEC_OPEN : 
	DSTATUS("-------------------DECODER CLOSE-------------------\n");

	// Now that we are done with decoding, close the open instance.
	ret = TCC_JPU_DEC(JPU_DEC_CLOSE, &gsJpuDecHandle, 0x00, &gsJpuDecOutput);
	if(ret != JPG_RET_SUCCESS) {
		DPRINTF("JPU_DEC_CLOSE failed Error code is 0x%x. \n", ret );
	}
	DSTATUS("JPU Decode End. \n");

ERR_JPU_DEC_INIT:
	if(gsbitstream_buf_addr[PA])
		cdk_sys_free_physical_addr((void*)gsbitstream_buf_addr[PA], gbitstream_buf_size);
	
	if(gsbitstream_buf_addr[VA])
		cdk_sys_free_virtual_addr(&gbitstream_buf_dev, gsbitstream_buf_addr[VA], gbitstream_buf_size);

	if(gsframe_buf_addr[PA])
		cdk_sys_free_physical_addr((void*)gsframe_buf_addr[PA], gdecoded_frame_buf_size);
	
	if(gsframe_buf_addr[VA])
		cdk_sys_free_virtual_addr(&gdecoded_frame_buf_dev, gsframe_buf_addr[VA], gdecoded_frame_buf_size);

	jpu_dec_close();

	if(ret_err != JPG_RET_SUCCESS) {
		return -ret_err;
	}
	return ret;
}
#endif

#if defined(JPEG_ENCODE_FOR_MJPEG)
int tcc_jpu_enc_jpeg ( conf_enc_t* pConfEnc )
{
	int frame_idx, stride;
	codec_result_t ret = 0;
	codec_handle_t enc_handle = 0;
	unsigned int y_offset = 0, uv_offset = 0;

	jpu_enc_open();

	DSTATUS("-------------------ENCODER INIT-------------------\r\n");

	if(pConfEnc->m_iWidth & 0x0F) {
		DPRINTF("The stride of YUV input is not multiple of 16.");
		goto ERR_ENC_INIT;
	}

	//! Set Encoder Init
	gsenc_init.m_RegBaseVirtualAddr 	= gsRegisterBase;
	//!< encoding options
	gsenc_init.m_iMjpg_sourceFormat 	= pConfEnc->m_iYuvFormat;
	gsenc_init.m_iPicWidth 			= stride = pConfEnc->m_iWidth;
	gsenc_init.m_iPicHeight 			= pConfEnc->m_iHeight;
	gsenc_init.m_iRestartInterval 		= pConfEnc->m_iRestartInterval;
	gsenc_init.m_iRotMode 			= pConfEnc->m_iRotMode;
	gsenc_init.m_iEncQuality 			= pConfEnc->m_iEncQuality;
	gsenc_init.m_bCbCrInterleaveMode 	= pConfEnc->m_bChromaInterleaved;
	gsenc_init.m_Memset 				= (void  (*) ( void*, int, unsigned int ))memset;
	gsenc_init.m_Memcpy 				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
	gsenc_init.m_Interrupt 			= (int  (*) ( void ))NULL;
	
	//------------------------------------------------------------
	//! [x] bitstream output buffer for each JPU encoder
	//------------------------------------------------------------
	gencoded_out_buf_size = ( 10000 * 3 * 1024 ) / pConfEnc->m_iFramesPerSecond;
	gencoded_out_buf_size = ALIGNED_BUFF(gencoded_out_buf_size, (4*1024));

	gencoded_out_buf_addr[PA] = (codec_addr_t)pConfEnc->m_outPhyHeaderBuffAddr;
	if(gencoded_out_buf_addr[PA] == 0) {
		DPRINTF("gencoded_out_buf_addr[PA] malloc() failed.");
		goto ERR_ENC_INIT;
	}
	DSTATUS("gencoded_out_buf_addr[PA] = 0x%x , gencoded_out_buf_size = 0x%x.", (codec_addr_t)gencoded_out_buf_addr[PA], gencoded_out_buf_size);

	gencoded_out_buf_addr[VA] = (codec_addr_t)pConfEnc->m_outVirtHeaderBuffAddr;
	if(gencoded_out_buf_addr[VA] == 0) {
		DPRINTF("gencoded_out_buf_addr[VA] malloc() failed.");
		goto ERR_ENC_INIT;
	}
	DSTATUS("gencoded_out_buf_addr[VA] = 0x%x , gencoded_out_buf_size = 0x%x.", (codec_addr_t)gencoded_out_buf_addr[VA], gencoded_out_buf_size);
	memset((void *)gencoded_out_buf_addr[VA], 0, gencoded_out_buf_size);

	switch (pConfEnc->m_iYuvFormat) {
		case YUV_FORMAT_420:
			gframe_buf_size = (unsigned int)(pConfEnc->m_iWidth * pConfEnc->m_iHeight * 1.5);
			break;
		case YUV_FORMAT_422:
		case YUV_FORMAT_224:
			gframe_buf_size = (unsigned int)(pConfEnc->m_iWidth * pConfEnc->m_iHeight * 2);
			break;
		case YUV_FORMAT_444:
			gframe_buf_size = (unsigned int)(pConfEnc->m_iWidth * pConfEnc->m_iHeight * 3);
			break;
		case YUV_FORMAT_400:
			gframe_buf_size = (unsigned int)(pConfEnc->m_iWidth * pConfEnc->m_iHeight);
			break;
		default:
			break;
	}
	gframe_buf_size = ALIGNED_BUFF(gframe_buf_size, 8);

	gframe_buf_addr[PA] = (codec_addr_t)pConfEnc->m_InpPhyYUVBuffAddr;
	if(gframe_buf_addr[PA] == 0) {
		DPRINTF("gframe_buf_addr[PA] malloc() failed.");
		goto ERR_ENC_INIT;
	}	
	DSTATUS("gframe_buf_addr[PA] = 0x%x , gframe_buf_size = 0x%x.", (codec_addr_t)gframe_buf_addr[PA], gframe_buf_size);
	
	gframe_buf_addr[VA] = (codec_addr_t)pConfEnc->m_InpVirtYUVBuffAddr;
	if(gframe_buf_addr[VA] == 0) {
		DPRINTF("gframe_buf_addr[VA] malloc() failed.");
		goto ERR_ENC_INIT;
	}
	DSTATUS("gframe_buf_addr[VA] = 0x%x , gframe_buf_size = 0x%x.", (codec_addr_t)gframe_buf_addr[VA], gframe_buf_size);

	gsenc_init.m_BitstreamBufferAddr[PA] = gencoded_out_buf_addr[PA];
	gsenc_init.m_BitstreamBufferAddr[VA] = gencoded_out_buf_addr[VA];
	gsenc_init.m_iBitstreamBufferSize = gencoded_out_buf_size;

	ret = TCC_JPU_ENC(JPU_ENC_INIT, &enc_handle, &gsenc_init, NULL);
	if(ret != JPG_RET_SUCCESS) {
		DPRINTF("JPU_ENC_INIT failed. Error code is 0x%x.", ret );
		goto ERR_ENC_OPEN;
	}

	//! Run Encoder
	DSTATUS("-------------------ENCODER RUN-------------------\r\n");

	y_offset = pConfEnc->m_iWidth * pConfEnc->m_iHeight;
	if((pConfEnc->m_iYuvFormat == YUV_FORMAT_422) || (pConfEnc->m_iYuvFormat == YUV_FORMAT_224)) {
	    uv_offset = (pConfEnc->m_iWidth * pConfEnc->m_iHeight) / 2;
	} else {
	    uv_offset = (pConfEnc->m_iWidth * pConfEnc->m_iHeight) / 4;
	}
	gsenc_input.m_PicYAddr 	= gframe_buf_addr[PA];
	gsenc_input.m_PicCbAddr 	= gsenc_input.m_PicYAddr + y_offset;
	gsenc_input.m_PicCrAddr 	= gsenc_input.m_PicCbAddr + uv_offset;
	gsenc_input.m_BitstreamBufferAddr[PA] = gencoded_out_buf_addr[PA];
	gsenc_input.m_BitstreamBufferAddr[VA] = gencoded_out_buf_addr[VA];
	gsenc_input.m_iBitstreamBufferSize = gencoded_out_buf_size;

	for(frame_idx = 0; frame_idx < pConfEnc->m_iTotalFrames; frame_idx++) {
		ret = TCC_JPU_ENC(JPU_ENC_ENCODE, &enc_handle, &gsenc_input, &gsenc_output);
		if(ret != JPG_RET_SUCCESS) {
			DPRINTF("JPU_ENC_ENCODE failed. Error code is 0x%x.", ret);
			continue;
		}
	}
	pConfEnc->m_outSizeOfHeaderBuff 	= gsenc_output.m_iHeaderOutSize;
	pConfEnc->m_outSizeOfEncodedBuff = gsenc_output.m_iBitstreamOutSize;
	pConfEnc->m_outSizeOfTotalBuff 	= gsenc_output.m_iHeaderOutSize + gsenc_output.m_iBitstreamOutSize;
	DSTATUS("gsenc_output.m_BitstreamOut[PA] = 0x%x , gsenc_output.m_BitstreamOut[VA] = 0x%x , BitstreamSize = 0x%d.", \
		gsenc_output.m_BitstreamOut[PA], gsenc_output.m_BitstreamOut[VA], pConfEnc->m_outSizeOfTotalBuff);

ERR_ENC_OPEN:
	DSTATUS("-------------------ENCODER CLOSE-------------------\r\n");

	// Now that we are done with encoding, close the open instance.
	ret = TCC_JPU_ENC( JPU_ENC_CLOSE, &enc_handle, 0, &gsenc_output );
	if(ret != JPG_RET_SUCCESS) {
		DPRINTF("JPU_ENC_ENCODE failed. Error code is 0x%x.", ret);
	}
	DSTATUS("JPU ENC END. Total Frame is %d.", frame_idx);

ERR_ENC_INIT:
	jpu_enc_close();

	return ret;
}
#endif



