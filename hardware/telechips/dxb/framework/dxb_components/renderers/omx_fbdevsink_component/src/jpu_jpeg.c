/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifdef HAVE_ANDROID_OS
#define LOG_TAG	"JPU_CAM"
#include <utils/Log.h>

#include "cdk_core.h"
#include "TCCMemory.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif

#include <sys/mman.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <libpmap/pmap.h>

#include "../../../dxb_components/decoders/dxb_cdk_library/cdk/jpu_jpeg.h"

#include <string.h>

#define JPEG_BASE_REG	0x75300000 /*8925_0x75300000*/
//#define JPEG_BASE_REG 0x75000000
#define JPEG_CLK		0x740000A4	//PCLKCTRL09 JPEG CLOCK

#define JPU_OUTPUT_SIZE_MAX ( ((8192+64)*(8192+64)/4)*6 )

#define _cdk_fclose	fclose
#define _cdk_fopen fopen
#define _cdk_fwrite fwrite
#define _cdk_fread	fread
#define _cdk_fseek fseek
#define _cdk_ftell ftell

#ifdef JPEG_ENCODE_FOR_MJPEG
#define PRINTF DPRINTF
#define JPEG_BASE_REG	0x75300000
#define TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE	112
#define	TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE		113
int gcam_fd;
#endif

/************************************************************************/
/* TEST and Debugging                                                   */
/************************************************************************/
static int DEBUG_ON	= 1;
#define DPRINTF(msg...)	 ALOGE( ": " msg);
#define DSTATUS(msg...)	 if (DEBUG_ON) { ALOGD( ": " msg);}
#define DPRINTF_FRAME(msg...) //ALOGD(": " msg);

static unsigned int total_frm = 0;

//#define VIDEO_DEC_PROFILE
#ifdef VIDEO_DEC_PROFILE
#ifdef _TCC8920_
#define	TEST_GPIO_BASE  			0xF0102000
// TCC9200S => CAM_26pin, GPIO_D20
//#define GPIO_BASE_OFFSET 0xC0
//#define GPIO_NUM		 0x00100000
// TCC8900 => DXB_23pin, GPIO_E11
#define GPIO_BASE_OFFSET 0x100
#define GPIO_NUM		 0x00000800
#endif
static unsigned int 				pTestRegBase = 0;

//#define DEBUG_TIME_LOG
#ifdef DEBUG_TIME_LOG
#include "time.h"
static unsigned int dec_time[30] = {0,};
static unsigned int time_cnt = 0;
static unsigned int total_frame_cnt = 0;
static unsigned int total_dec_time = 0;
#endif
#endif

#if defined(_TCC8800_) || defined(_TCC9300_)
#define VPU_PERFORMANCE_UP //use sram and vcache
#endif

#define USE_CLK_CTRL
#define VPU_REGISTER_DUMP
//#define VPU_FRAME_DUMP
#else

#include "../cdk/cdk_core.h"
#include "../cdk/cdk_sys.h"
#include "vpu/TCC_VPU_CODEC.h"
#ifdef VPU_CLK_CONTROL
#include "vpu_clk_ctrl.h"
#endif

#endif

static unsigned char vdec_env_opened = 0, vdec_codec_opened = 0;
int dev_fd = -1;
#ifdef HAVE_ANDROID_OS
#define TCC_VPU_DEV_NAME	"/dev/vpu"
#else
#define TCC_VPU_DEV_NAME	"/dev/mem"
#endif

#ifdef USE_CLK_CTRL
#define TCC_CLK_DEV_NAME		"/dev/clockctrl"

#include <fcntl.h>         // O_RDWR
#include <sys/poll.h>
int jpu_clk_fd = -1;
struct pollfd tcc_event[1];
#endif


/************************************************************************/
/* STATIC MEMBERS                                                       */
/************************************************************************/
static pmap_t pmap_jpeg_capture;

#define PA_JPEG_CAPTURE_BASE_ADDR	pmap_jpeg_capture.base
#define JPEG_CAPTURE_MEM_SIZE		pmap_jpeg_capture.size
#define JPEG_TATAL_MEM_SIZE 		JPEG_CAPTURE_MEM_SIZE

#define JPU_PHY_ADDR		PA_JPEG_CAPTURE_BASE_ADDR
#define JPU_MEM_SIZE		JPEG_TATAL_MEM_SIZE

#ifdef JPEG_DECODE_FOR_MJPEG
static jpu_codec_handle_t gsJpuDecHandle = 0;
static jpu_dec_init_t gsJpuDecInit;
static jpu_dec_initial_info_t gsJpuDecInitialInfo;
static jpu_dec_input_t gsJpuDecInput;
static jpu_dec_output_t gsJpuDecOutput;
static jpu_dec_buffer_t gsJpuDecBuffer;

static jpu_codec_addr_t gsbitstream_buf_addr[2] = {0,}, gsframe_buf_addr[2] = {0,};
static int gbitstream_buf_size = 0, gdecoded_frame_buf_size = 0, gframe_buf_size_init = 0, gprev_framebuff_size = 0;
static int gbitstream_buf_dev = 0, gdecoded_frame_buf_dev = 0;
#endif

#ifdef JPEG_ENCODE_FOR_MJPEG
static	jpu_enc_init_t gsenc_init;
static	jpu_enc_input_t gsenc_input;
static	jpu_enc_output_t gsenc_output;

static unsigned int gsUsedPhysicalMemSize=0, gsLogicalBase =0, gsRegisterBase=0; 
static jpu_codec_addr_t gencoded_out_buf_addr[2] = {0,}, gframe_buf_addr[2] = {0,};
static unsigned int gencoded_out_buf_size = 0, gframe_buf_size = 0;
static int gencoded_out_buf_dev =-1, gframe_buf_dev = -1;
#endif

static unsigned int	enc_end_check_flag =0;
static void vpu_env_close(void);

extern int		jpeg_enc_fd;

//#define FILE_ENC_TEST

#ifdef DEBUG_VPU_DEC_INPUT
static void PrintVPUHexData(unsigned char* pBuffer, unsigned int length, unsigned char* pTag)
{
	ALOGI("[%s:%08d] 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ~ 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", pTag, length
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
static unsigned char * sPhysicalMemSetting(unsigned int nPhysicalAddr, unsigned int size)
{	
	void *map_ptr = MAP_FAILED;

	map_ptr = (void *)mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, nPhysicalAddr);
	
	if(MAP_FAILED == map_ptr)
	{
		ALOGE("mmap failed. fd(%d), addr(0x%x), size(%d)", dev_fd, nPhysicalAddr, size);
		return NULL;
	}
	
	return (unsigned char *)map_ptr;
}

static void sPhysicalMemFree(unsigned int LogicalAddr, unsigned int size)
{
	int ret = 0;
	
	if((ret = munmap((void*)LogicalAddr, size)) < 0)
	{
		ALOGE("munmap failed. addr(0x%x), size(%d)", LogicalAddr, size);
	}
}

static void *vdec_malloc(unsigned int size)
{
	void * ptr = TCC_malloc(size);
	return ptr;
}

static void vdec_free(void * ptr )
{
	TCC_free(ptr);
}

static void *cdk_sys_malloc_physical_addr_jpu(int uiSize)
{
	jpu_codec_addr_t phsical_addr = JPU_PHY_ADDR + gsUsedPhysicalMemSize;
	gsUsedPhysicalMemSize += uiSize;
	
	if( gsUsedPhysicalMemSize > JPU_MEM_SIZE ){
		ALOGE("sys_malloc_physical_addr error! %d bytes \n", gsUsedPhysicalMemSize);
	}
	
	return (void*)( phsical_addr );
}

static void cdk_sys_free_physical_addr_jpu(void* pPtr, int uiSize)
{
	gsUsedPhysicalMemSize -= uiSize;
	return;
}

static void *cdk_sys_malloc_virtual_addr_jpu(int* pDev, jpu_codec_addr_t pPtr, int uiSize)
{
	jpu_codec_addr_t used_size = pPtr - JPU_PHY_ADDR;
	
	return (void*)( gsLogicalBase + used_size);
}

static void cdk_sys_free_virtual_addr_jpu(int* pDev, jpu_codec_addr_t pPtr, int uiSize)
{
	
}

static unsigned int cdk_sys_remain_memory_size_jpu(void)
{
	return (JPU_MEM_SIZE - gsUsedPhysicalMemSize);
}

int LoadYuvImageHelper( FILE *yuvFp,
					   jpu_codec_addr_t pyuv, jpu_codec_addr_t vyuv, 
					   jpu_codec_addr_t* pAddrY, 
					   jpu_codec_addr_t* pAddrCb, 
					   jpu_codec_addr_t* pAddrCr, 
					   int picWidth, 
					   int picHeight, 
					   int stride, 
					   int interleave,
					   int yuvformat)
{
	int divX, divY, lum_size, chr_size, input_yuv_size;

	switch (yuvformat)
	{
		case 0:	// YUV_FORMAT_420:
			input_yuv_size = (unsigned int)(picWidth * picHeight * 1.5);
			break;
		case 1:	// YUV_FORMAT_422:
		case 2:	// YUV_FORMAT_224:
			input_yuv_size = (unsigned int)(picWidth * picHeight * 2);
			break;
		case 3:	// YUV_FORMAT_444:
			input_yuv_size = (unsigned int)(picWidth * picHeight * 3);
			break;
		case 4:	// YUV_FORMAT_400:
			input_yuv_size = (unsigned int)(picWidth * picHeight);
			break;
		default:
			break;
	}

#ifdef  FILE_ENC_TEST
	// Load source one picture image to encode to SDRAM frame buffer.
	if( !fread( (void*)vyuv, 1, input_yuv_size, yuvFp) ) 
	{
		if( !feof( yuvFp ) )
			printf(  "Yuv Data fread failed file handle is 0x%x \n", (unsigned int)yuvFp );			
		return 0;
	}
#endif

	divX = yuvformat == 0 || yuvformat == 1 ? 2 : 1;
	divY = yuvformat == 0 || yuvformat == 2 ? 2 : 1;

	switch (yuvformat)
	{
		case 0:	// YUV_FORMAT_420:
			picHeight = (picHeight+1)/2*2;
			picWidth = (picWidth+1)/2*2;
			break;
		case 2:	// YUV_FORMAT_224:
			picHeight = (picHeight+1)/2*2;
			break;
		case 1:	// YUV_FORMAT_422:
			picWidth = (picWidth+1)/2*2;
			break;
		case 3:	// YUV_FORMAT_444:
			break;
		case 4:	// YUV_FORMAT_400:
			picHeight = (picHeight+1)/2*2;
			picWidth = (picWidth+1)/2*2;	
			break;
	}

	lum_size   = picWidth * picHeight;
	chr_size   = lum_size/divX/divY;

	ALOGE("%s %d lum_size= %d , chr_size = %d \n", __func__, __LINE__, lum_size, chr_size);

	*pAddrY  = pyuv;
	*pAddrCb = *pAddrY  + lum_size;
	*pAddrCr = *pAddrCb + chr_size;

	ALOGE("%s %d pAddrY = 0x%x, pAddrCb= 0x%x, pAddrCr= 0x%x\n", __func__, __LINE__, *pAddrY, *pAddrCb, *pAddrCr);

	return 1;
}

static void 
plane_copy( unsigned char* pDest, int DestStride, 
		   unsigned char* pSour, int SourStride, int SourWidth, int SourHeight )
{
	do 
	{
		memcpy( pDest, pSour, SourWidth );
		pDest += DestStride;
		pSour += SourStride;
	} while( --SourHeight );
	return 0;
}

static void 
yuv420_to_yuv420_crop( unsigned char* pInpY, unsigned char* pInpCb, unsigned char* pInpCr,
					   int picWidth, int picHeight, int stride, int CropLeft, int CropRight, int CropTop, int CropBottom,
                       unsigned char* pDest, int* pDestWidth, int* pDestHeight )
{
	unsigned char* p_sour[3];
	unsigned char* p_dest[3];

	int inp_width  = picWidth;
	int inp_height = picHeight;
	int out_width  = picWidth;
	int out_height = picHeight;

	p_sour[0] = pInpY;
	p_sour[1] = pInpCb;
	p_sour[2] = pInpCr;

	if( CropLeft || CropRight || CropTop || CropBottom )
	{
		p_sour[0] = pInpY  + ( CropTop   * stride   + CropLeft   );
		p_sour[1] = pInpCb + ( CropTop/2 * stride/2 + CropLeft/2 );
		p_sour[2] = pInpCr + ( CropTop/2 * stride/2 + CropLeft/2 );

		inp_width  = inp_width  - ( CropLeft + CropRight  );
		inp_height = inp_height - ( CropTop  + CropBottom );
		out_width  = inp_width;
		out_height = inp_height;
	}

	*pDestWidth  = out_width;
	*pDestHeight = out_height;


	p_dest[0] = pDest;
	p_dest[1] = p_dest[0] + out_width	* out_height;
	p_dest[2] = p_dest[1] + out_width/2 * out_height/2;

	plane_copy( p_dest[0], out_width,   p_sour[0], stride  , inp_width,   inp_height   );
	plane_copy( p_dest[1], out_width/2, p_sour[1], stride/2, inp_width/2, inp_height/2 );
	plane_copy( p_dest[2], out_width/2, p_sour[2], stride/2, inp_width/2, inp_height/2 );
}

void 
yuvi_to_yuv420( unsigned char** ppY, unsigned char** ppCb, unsigned char** ppCr, unsigned char* pDst,
			    int picWidth, int picHeight, int stride )
{
	int y, nY, nCb, nCr/*, bufferWidth, bufferHeight*/;
	int addr;
	unsigned char* puc;

	if( !ppY || !ppCb || !ppCr )
		return;

	nY = picHeight;
	nCb = nCr = picHeight/2;

	ALOGE("%s %d \n", __func__, __LINE__);
	puc = pDst;
	addr = (int)*ppY;
	ALOGE("stride = %d %s %d \n", stride,  __func__, __LINE__);

	if( picWidth == stride ) 
	{
		unsigned char t0,t1,t2,t3,t4,t5,t6,t7;
		int i;
		unsigned char * pTemp;
		unsigned char * dstAddrCb;
		unsigned char * dstAddrCr;

		memcpy( (void*)puc, (void*)addr, ( picWidth * picHeight ) );

		addr = (int)*ppCb;
		dstAddrCb = (unsigned char*)(puc + picWidth * picHeight);
		dstAddrCr = (unsigned char*)(dstAddrCb + picWidth/2 * picHeight/2);

		pTemp = malloc(picWidth);
		if (!pTemp) {
			printf( "malloc() failed \n");
		}

		for (y = 0; y < nY / 2; ++y) 
		{
			memcpy( (void*)pTemp, (void*)(addr + stride * y), picWidth );
			for (i = 0; i < picWidth; i += 8) {
				t0 = pTemp[i];
				t1 = pTemp[i + 1];
				t2 = pTemp[i + 2];
				t3 = pTemp[i + 3];
				t4 = pTemp[i + 4];
				t5 = pTemp[i + 5];
				t6 = pTemp[i + 6];
				t7 = pTemp[i + 7];
				*dstAddrCb++ = t0;
				*dstAddrCb++ = t2;
				*dstAddrCb++ = t4;
				*dstAddrCb++ = t6;
				*dstAddrCr++ = t1;
				*dstAddrCr++ = t3;
				*dstAddrCr++ = t5;
				*dstAddrCr++ = t7;
			}
		}
		free(pTemp);
	}
	else
	{
		int i;
		unsigned char * pTemp;
		unsigned char * dstAddrCb;
		unsigned char * dstAddrCr;

		for (y = 0; y < nY; ++y) {
			memcpy( (void*)(puc + y * picWidth), (void*)(addr + stride * y), ((picWidth+7)/8)*8 );
		}

		addr = (int)*ppCb;
		stride = stride;
		dstAddrCb = (unsigned char *)(puc + picWidth * picHeight);
		dstAddrCr = (unsigned char *)(dstAddrCb + picWidth/2 * picHeight/2);

		pTemp = malloc((picWidth + 7) /8 *8);
		if (!pTemp) {
			printf( "malloc() failed \n");
		}
		for (y = 0; y < nY / 2; y++) {
			memcpy( (void*)pTemp, (void*)(addr + stride * y), (picWidth + 7) /8 *8 );
			for (i = 0; (i < picWidth); i+=2) {
				*dstAddrCb++ = pTemp[(i/8)*8 + (i%8)];
				*dstAddrCr++ = pTemp[(i/8)*8 + (i%8) + 1];
			}
		}
		free(pTemp);
	}

	*ppY  = pDst;
	*ppCb = *ppY  + stride * picHeight;
	*ppCr = *ppCb + stride/2 * picHeight/2;

}

int
save_yuv_image( void *yuvFp, unsigned char* pOutYuv, 
			    unsigned char* pInpY,
				unsigned char* pInpCb,
				unsigned char* pInpCr,
				int picWidth,
				int picHeight,
				int stride,
				int CropLeft, int CropRight, int CropTop, int CropBottom,
				unsigned int uiCbCrInterleave )
{
	int dest_width = 0;
	int dest_height = 0;
 	FILE    *fp_dest;  
	unsigned char *p_outyuv = NULL;
 
	if( !pInpY || !pInpCb || !pInpCr )
		return 1;

	ALOGE("%s %d \n", __func__, __LINE__);
	fp_dest  = fopen( "/mnt/sdcard/yuv.dat", "w");
	p_outyuv = (unsigned char*)malloc(JPU_OUTPUT_SIZE_MAX);

//	if( uiCbCrInterleave )
	{
		yuvi_to_yuv420( &pInpY, &pInpCb, &pInpCr, p_outyuv, picWidth, picHeight, stride );
//		if( !CropLeft && !CropRight && !CropTop && !CropBottom )
		{
			dest_width = stride;
			dest_height = picHeight;
			goto SAVE_YUV_END;
		}
	}

	yuv420_to_yuv420_crop( pInpY, pInpCb, pInpCr, picWidth, picHeight, stride, CropLeft, CropRight, CropTop, CropBottom,
		p_outyuv, &dest_width, &dest_height );

SAVE_YUV_END:
//	if( !fwrite(pOutYuv, sizeof(unsigned char), dest_width * dest_height * 3 / 2 , yuvFp) ) 
	if( !fwrite(p_outyuv, sizeof(unsigned char), dest_width * dest_height * 3 / 2 , fp_dest) ) 
	{
		printf( "picWidth = %d, picHeight = %d, stride = %d \n", picWidth, picHeight, stride );
		printf( "rcCrop.left %d|| rcCrop.top %d|| rcCrop.bottom %d|| rcCrop.right %d\n", CropLeft, CropTop,CropBottom, CropRight );
		printf( "Frame Data fwrite failed file handle is 0x%x, width = %d, height = %d \n", (unsigned int)yuvFp, dest_width, dest_height );
		return 0;			
	}
	if(dest_width == 1600)
		memset(pInpY, 0, dest_width * dest_height * 3 / 2);

	fclose( fp_dest);
	
	if( p_outyuv )
		free( p_outyuv );

	return 1;
}




static int
yuv420_to_yuv420( unsigned char* pInpY, unsigned char* pInpCb, unsigned char* pInpCr,
					   int picWidth, int picHeight, int stride,
                       unsigned char* pDest, int* pDestWidth, int* pDestHeight, unsigned int uiFormat )
{
	int frame_buf_size;
	int c_width, c_height;
	int c_stride;
	unsigned char* p_dest[3];

	switch (uiFormat)
	{
		case TCC_FMT_YUV420:
			c_width = (picWidth + 1) / 2;
			c_height = (picHeight+1) / 2;
			c_stride = stride / 2;
			break;
		case TCC_FMT_YUV224:
			c_width = picWidth;
			c_height = (picHeight+1) / 2;
			c_stride = stride;
			break;
		case TCC_FMT_YUV422:
			c_width = (picWidth + 1) / 2;
			c_height = picHeight;
			c_stride = stride / 2;
			break;
		case TCC_FMT_YUV444:
			c_width = picWidth;
			c_height = picHeight;
			c_stride = stride;
			break;
		case TCC_FMT_YUV400:
			c_width = (picWidth + 1) / 2;
			c_height = (picHeight + 1) / 2;
			c_stride = stride / 2;
			break;
	}

	*pDestWidth  = picWidth;
	*pDestHeight = picHeight;

	p_dest[0] = pDest;
	p_dest[1] = p_dest[0] + picWidth * picHeight;
	p_dest[2] = p_dest[1] + c_width * c_height;

	frame_buf_size = picWidth * picHeight + (c_width * c_height) * 2;

	plane_copy( p_dest[0], picWidth,   pInpY, stride  , stride,   picHeight   );
	if (uiFormat !=4) 
	{
		plane_copy( p_dest[1], c_width, pInpCb, c_stride, c_stride, c_height );
		plane_copy( p_dest[2], c_width, pInpCr, c_stride, c_stride, c_height );
	}
	else
		frame_buf_size = picWidth * picHeight;
	

	return frame_buf_size;
}

int 
save_yuv_image_mjpg( void *yuvFp, unsigned char* pOutYuv, 
					unsigned char* pInpY,
					unsigned char* pInpCb,
					unsigned char* pInpCr,
					int picWidth,
					int picHeight,
					int stride,
					unsigned int uiFrameFormat,
					unsigned int uiSubSampleRatio)
{
	int dest_width = 0;
	int dest_height = 0;
	int frameSize=0;
	int ratio;
	ratio = 1;

	if( !pInpY || !pInpCb || !pInpCr )
		return 1;

	frameSize = yuv420_to_yuv420( pInpY, pInpCb, pInpCr, picWidth, picHeight, stride/*stride*/,
		pOutYuv, &dest_width, &dest_height, uiFrameFormat );

	if( !fwrite(pOutYuv, sizeof(unsigned char), frameSize , yuvFp) ) 
	{
		printf( "picWidth = %d, picHeight = %d, stride = %d \n", picWidth, picHeight, stride );
		printf( "Frame Data fwrite failed file handle is 0x%x, width = %d, height = %d \n", (unsigned int)yuvFp, dest_width, dest_height );
		return 0;			
	}
	return 1;
}

static int vpu_env_open()
{
	int vpu_reset = 0;
	int ret_val;
	
	DSTATUS("In  %s \n",__func__);

	pmap_get_info("jpg_enc_dxb", &pmap_jpeg_capture);

	gcam_fd = jpeg_enc_fd;
	
	jpu_clk_fd = open(TCC_CLK_DEV_NAME,O_WRONLY);
	if(!jpu_clk_fd)
	{
		ALOGE("%s open error!!", TCC_CLK_DEV_NAME);
		goto err;
	}

	ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_ENABLE, &ret_val);

	// connect fd to dev_fd
	dev_fd = gcam_fd;
	DSTATUS("dev_fd[0x%x] JPU_MEM_SIZE[0x%x]",dev_fd, JPU_MEM_SIZE);

	gsLogicalBase = (unsigned int)sPhysicalMemSetting(JPU_PHY_ADDR, JPU_MEM_SIZE);
	if(!gsLogicalBase)
		goto err;

	gsRegisterBase	= (unsigned int)sPhysicalMemSetting(JPEG_BASE_REG, 8*1024);
	if(!gsRegisterBase)
		goto err;	

	DSTATUS("[JPU ENC] gsLogicalBase = 0x%x, gsRegisterBase = 0x%x", (jpu_codec_addr_t)gsLogicalBase, (jpu_codec_addr_t)gsRegisterBase);
	gsUsedPhysicalMemSize = 0;

#ifdef VIDEO_DEC_PROFILE
	pTestRegBase	= sPhysicalMemSetting(TEST_GPIO_BASE, 1024);
	if(!pTestRegBase)
		goto err;	
#ifdef DEBUG_TIME_LOG	
	time_cnt = 0;
	total_frame_cnt = 0;
	total_dec_time = 0;
#endif
#endif

	memset(&gsenc_init, 0x00, sizeof(jpu_enc_init_t));
	memset(&gsenc_input, 0x00, sizeof(jpu_enc_input_t));
	memset(&gsenc_output, 0x00, sizeof(jpu_enc_output_t));

	memset(&gencoded_out_buf_addr, 0x00, sizeof(gencoded_out_buf_addr));
	memset(&gframe_buf_addr, 0x00, sizeof(gframe_buf_addr));
	
	DSTATUS("Out  %s \n",__func__);
	total_frm = 0;

	return 0;

err:	
	ALOGE("vpu_env_open error");
	vpu_env_close();
	
	return -1;	
}


static void vpu_env_close(void)
{
	int vpu_reset, ret_val;
	
	DSTATUS("In  %s \n",__func__);

	if(gsLogicalBase)
		sPhysicalMemFree(gsLogicalBase, JPU_MEM_SIZE);

	if(gsRegisterBase)
		sPhysicalMemFree(gsRegisterBase, 8*1024);

	gsLogicalBase = 0;
	gsRegisterBase = 0;


	ret_val = ioctl(jpu_clk_fd, TCC_CLOCKCTRL_JPEG_MAX_CLOCK_DISABLE, &ret_val);

	if(jpu_clk_fd)
	{		
		if(close(jpu_clk_fd) < 0)
		{
			ALOGE("Err[%s] :: vpu(%s) close failed!", strerror(errno), TCC_CLK_DEV_NAME);
		}
		dev_fd = -1;
	}

	vdec_env_opened = 0;

#ifdef DEBUG_TIME_LOG		
	LOGD("========== Average decode time : %d ==========",total_dec_time/total_frame_cnt);
#endif

	DSTATUS("Out  %s \n",__func__);

}

#ifdef VPU_REGISTER_DUMP
static void filewrite_memory(char* name, char* addr, unsigned int size)
{
	FILE *fp;

	fp = fopen(name, "ab+");		
	fwrite( addr, size, 1, fp);
	fclose(fp);
}
#endif


static void print_frame_data(char* name, int size)
{
#ifdef VPU_FRAME_DUMP
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

#endif

static void vpu_hw_reset()
{
	codec_addr_t ckc_swrst, vidbus_swrst;
	unsigned int ckc_swrst_offset, vidbus_swrst_offset;

#ifdef _TCC9300_
	ckc_swrst_offset = 0x5C;
 	vidbus_swrst_offset = 0x04;
#else
	ckc_swrst_offset = 0x44;
	vidbus_swrst_offset = 0x04;
#endif

	ckc_swrst   = (unsigned int)sPhysicalMemSetting(VIDEOCODECBUS_CLK_BASE_ADDR, 100);   
	if(!ckc_swrst)
	   return;
	
	vidbus_swrst   = (unsigned int)sPhysicalMemSetting(VIDEOBUSCONF_BASE_ADDR, 100);
	if(!vidbus_swrst)
	   return;

	ALOGI(" **************** addr:: ckc 0x%x, vid 0x%x ", ckc_swrst, vidbus_swrst);
	// reset
#if !defined(_TCC9300_) && !defined(_TCC8800_) // to prevent before chip-revision!!
	*((volatile unsigned int *)(ckc_swrst + ckc_swrst_offset))   |= (unsigned int)0x00000040;
#endif
	*((volatile unsigned int *)(vidbus_swrst + vidbus_swrst_offset)) |= (unsigned int)0x00000004;
	
	//wait until reset for 1 ms
	usleep( 1 * 1000 );

	// not-reset
#if !defined(_TCC9300_) && !defined(_TCC8800_) // to prevent before chip-revision!!
	*((volatile unsigned int *)(ckc_swrst + ckc_swrst_offset))   &= (unsigned int)(~(0x00000040));
#endif
	*((volatile unsigned int *)(vidbus_swrst + vidbus_swrst_offset)) &= (unsigned int)(~(0x00000004));

	//wait until VPU reboot for 10 ms
	usleep( 10 * 1000 );

	sPhysicalMemFree(ckc_swrst, 100);
	sPhysicalMemFree(vidbus_swrst, 100);

	ALOGI(" **************** VPU H/W Reset ****************");
    //restart VPU process..
}

#ifdef JPEG_ENCODE_FOR_MJPEG
int jpu_enc ( jpu_conf_enc_t* pConfEnc )
{
	//codec_addr_t reg_base_addr = 0;
	//unsigned int reg_base_size = 0;
	//int reg_base_dev = -1;

	void* fp_input = NULL;	// input YUV file pointer
	void* fp_out = NULL;	// output Bitstream file pointer.
	int frame_idx = 0;
	int input_size = 0;
	unsigned int y_offset = 0, uv_offset = 0;

	PRINTF("%s %d \n", __func__, __LINE__);

	jpu_codec_result_t ret = 0;
	jpu_codec_handle_t enc_handle = 0;
	int stride;

#ifdef FILE_ENC_TEST
	char m_InpFileName[256];
	
	sprintf(m_InpFileName, "%s", "/mnt/sdcard/org.raw" );
	fp_input = fopen((char *)m_InpFileName, "rb");

	gConfEnc.m_iTotalFrames = pConfEnc->m_iTotalFrames;
	gConfEnc.m_iWidth = pConfEnc->m_iWidth;
	gConfEnc.m_iHeight = pConfEnc->m_iHeight;
	gConfEnc.m_iFramesPerSecond = pConfEnc->m_iFramesPerSecond;
	gConfEnc.m_iRestartInterval = 0;
	gConfEnc.m_iRotMode = 0;
	gConfEnc.m_iYuvFormat = pConfEnc->m_iYuvFormat;
	gConfEnc.m_iEncQuality = pConfEnc->m_iEncQuality;
	gConfEnc.m_bChromaInterleaved = 0;
	gConfEnc.m_iYUVWidth = pConfEnc->m_iWidth; //½ÇÁ¦ size
	gConfEnc.m_iYUVHeight = pConfEnc->m_iHeight;

	ALOGE( "- Total frames: %d\r\n", gConfEnc.m_iTotalFrames );
	ALOGE( "- Width: %d\r\n", gConfEnc.m_iWidth );
	ALOGE( "- Height: %d\r\n", gConfEnc.m_iHeight );
	ALOGE( "- FramesPerSecond: %d\r\n", gConfEnc.m_iFramesPerSecond );
	ALOGE( "- RestartInterval: %d\r\n", gConfEnc.m_iRestartInterval );
	ALOGE( "- Rotation Mode: %d\r\n", gConfEnc.m_iRotMode );
	ALOGE( "- Yuvstreamformat: %d\r\n", gConfEnc.m_iYuvFormat );
	ALOGE( "- Enc Quality: %d\r\n", gConfEnc.m_iEncQuality );
	ALOGE( "- chroma interleave: %d\r\n", gConfEnc.m_bChromaInterleaved );
#endif

	ret = vpu_env_open();
	if(ret == -1)
	{
		PRINTF( "[enc_test] vpu_env_open fail  \r\n" );
		goto ERR_ENC_INIT;
	}

	DSTATUS("-------------------ENCODER INIT-------------------\r\n");

	//------------------------------------------------------------
	//! [x] YUV input
	//------------------------------------------------------------
	if( pConfEnc->m_iWidth & 0x0F )
	{
		PRINTF( "[enc_test] the stride of YUV input is not multiple of 16\r\n" );
		goto ERR_ENC_INIT;
	}

	//! Set Encoder Init
	{
		gsenc_init.m_RegBaseVirtualAddr = gsRegisterBase;

		//!< encoding options
		gsenc_init.m_iMjpg_sourceFormat 	= pConfEnc->m_iYuvFormat;
		gsenc_init.m_iPicWidth 			= stride = pConfEnc->m_iWidth;
		gsenc_init.m_iPicHeight 			= pConfEnc->m_iHeight;
		gsenc_init.m_iRestartInterval 		= pConfEnc->m_iRestartInterval;
		gsenc_init.m_iRotMode 			= pConfEnc->m_iRotMode;
		gsenc_init.m_iEncQuality 			= pConfEnc->m_iEncQuality;
		gsenc_init.m_bCbCrInterleaveMode 	= pConfEnc->m_bChromaInterleaved;
		
		gsenc_init.m_Memcpy 				= (void* (*) ( void*, const void*, unsigned int ))memcpy;
		gsenc_init.m_Memset 				= (void  (*) ( void*, int, unsigned int ))memset;
		gsenc_init.m_Interrupt 			=  (int	(*) ( void ))NULL;
		
		//------------------------------------------------------------
		//! [x] bitstream output buffer for each JPU encoder
		//------------------------------------------------------------
		gencoded_out_buf_size = ( 10000 * 4 * 1024 ) / pConfEnc->m_iFramesPerSecond;
		gencoded_out_buf_size = ALIGNED_BUFF( gencoded_out_buf_size, 4*1024 );

		gencoded_out_buf_addr[PA] = (codec_addr_t)cdk_sys_malloc_physical_addr_jpu( gencoded_out_buf_size );
		if( gencoded_out_buf_addr[PA] == 0 ) 
		{
			PRINTF( "[enc_test] gencoded_out_buf_addr[PA] malloc() failed \r\n");
			goto ERR_ENC_INIT;
		}
		DSTATUS("[enc_test] gencoded_out_buf_addr[PA] = 0x%x, 0x%x(%dK) \r\n", (codec_addr_t)gencoded_out_buf_addr[PA], gencoded_out_buf_size, gencoded_out_buf_size/1024 );
		gencoded_out_buf_addr[VA] = (codec_addr_t)cdk_sys_malloc_virtual_addr_jpu( &gencoded_out_buf_dev, gencoded_out_buf_addr[PA], gencoded_out_buf_size );
		if( gencoded_out_buf_addr[VA] == 0 ) 
		{
			PRINTF( "[enc_test] gencoded_out_buf_addr[VA] malloc() failed \r\n");
			goto ERR_ENC_INIT;
		}
		DSTATUS("[enc_test] gencoded_out_buf_addr[VA] = 0x%x, 0x%x \r\n", (codec_addr_t)gencoded_out_buf_addr[VA], gencoded_out_buf_size );
		memset( (void*)gencoded_out_buf_addr[VA], 0 , gencoded_out_buf_size);

		switch (pConfEnc->m_iYuvFormat)
		{
			case YUV_FORMAT_420:
				gframe_buf_size = (unsigned int)(pConfEnc->m_iYUVWidth * pConfEnc->m_iYUVHeight * 1.5);
				break;
			case YUV_FORMAT_422:
			case YUV_FORMAT_224:
				gframe_buf_size = (unsigned int)(pConfEnc->m_iYUVWidth * pConfEnc->m_iYUVHeight * 2);
				break;
			case YUV_FORMAT_444:
				gframe_buf_size = (unsigned int)(pConfEnc->m_iYUVWidth * pConfEnc->m_iYUVHeight * 3);
				break;
			case YUV_FORMAT_400:
				gframe_buf_size = (unsigned int)(pConfEnc->m_iYUVWidth * pConfEnc->m_iYUVHeight);
				break;
			default:
				break;
		}
		gframe_buf_size = ALIGNED_BUFF(gframe_buf_size, 8);
		DSTATUS("[enc_test] m_iYuvFormat = %d , gframe_buf_size = %d \r\n", pConfEnc->m_iYuvFormat, gframe_buf_size);
#ifdef FILE_ENC_TEST
		gframe_buf_addr[PA] = (jpu_codec_addr_t)cdk_sys_malloc_physical_addr_jpu( gframe_buf_size );
		if( gframe_buf_addr[PA] == 0 ) 
		{
			PRINTF( "[enc_test] gencoded_out_buf_addr[PA] malloc() failed \r\n");
			goto ERR_ENC_INIT;
		}
		DSTATUS("[enc_test] gencoded_out_buf_addr[PA] = 0x%x, 0x%x(%dK) \r\n", (jpu_codec_addr_t)gframe_buf_addr[PA], gframe_buf_size, gframe_buf_size/1024 );
		gframe_buf_addr[VA] = (jpu_codec_addr_t)cdk_sys_malloc_virtual_addr_jpu( &gencoded_out_buf_dev, gframe_buf_addr[PA], gframe_buf_size );
		if( gframe_buf_addr[VA] == 0 ) 
		{
			PRINTF( "[enc_test] gencoded_out_buf_addr[VA] malloc() failed \r\n");
			goto ERR_ENC_INIT;
		}
		DSTATUS("[enc_test] gencoded_out_buf_addr[VA] = 0x%x, 0x%x \r\n", (jpu_codec_addr_t)gframe_buf_addr[VA], gframe_buf_size );
		memset( (void*)gframe_buf_addr[VA], 0 , gframe_buf_size);	
#else
		gframe_buf_addr[PA] = (jpu_codec_addr_t)pConfEnc->m_InpPhyYUVBuffAddr;
		if( gframe_buf_addr[PA] == 0 ) 
		{
			PRINTF( "[enc_test] gframe_buf_addr[PA] malloc() failed \r\n");
			goto ERR_ENC_INIT;
		}
		gframe_buf_addr[VA] = (jpu_codec_addr_t)pConfEnc->m_InpVirtYUVBuffAddr;
		if( gframe_buf_addr[VA]  == 0 ) 
		{
			PRINTF( "[enc_test] gframe_buf_addr[VA] malloc() failed \r\n");
			goto ERR_ENC_INIT;
		}
		DSTATUS("[enc_test] gframe_buf_addr[VA] = 0x%x, 0x%x \r\n", (jpu_codec_addr_t)gframe_buf_addr[VA], gframe_buf_size );
#endif

		gsenc_init.m_BitstreamBufferAddr[PA] = gencoded_out_buf_addr[PA];
		gsenc_init.m_BitstreamBufferAddr[VA] = gencoded_out_buf_addr[VA];
		gsenc_init.m_iBitstreamBufferSize = gencoded_out_buf_size;

		ret = TCC_JPU_ENC( JPU_ENC_INIT, &enc_handle, &gsenc_init, NULL);  // ENCODER INIT

		DSTATUS("[enc_test] enc_init_ret = %d  \r\n", ret);

		if( ret != JPG_RET_SUCCESS )
		{
			PRINTF( "[enc_test] VPU_ENC_INIT failed.. Error code is 0x%x \r\n", ret );
			goto ERR_ENC_OPEN;
		}
	}

	//! Run Encoder
	DSTATUS("-------------------ENCODER RUN-------------------\r\n");
	{
		for( frame_idx = 0; frame_idx < pConfEnc->m_iTotalFrames; frame_idx++ )
		{
			DSTATUS("-------------------ENCODER RUN  %5d -------------------\r\n", frame_idx);

			gsenc_input.m_BitstreamBufferAddr[PA] = gencoded_out_buf_addr[PA];
			gsenc_input.m_BitstreamBufferAddr[VA] = gencoded_out_buf_addr[VA];
			gsenc_input.m_iBitstreamBufferSize = gencoded_out_buf_size;

			if( !LoadYuvImageHelper(fp_input, gframe_buf_addr[PA], gframe_buf_addr[VA], 
										 &gsenc_input.m_PicYAddr, 
										 &gsenc_input.m_PicCbAddr,
										 &gsenc_input.m_PicCrAddr, 
										 pConfEnc->m_iYUVWidth,
										 pConfEnc->m_iYUVHeight,
										 stride, gsenc_init.m_bCbCrInterleaveMode, pConfEnc->m_iYuvFormat )
										 )
										break;
			
			ret = TCC_JPU_ENC( JPU_ENC_ENCODE, &enc_handle, &gsenc_input, &gsenc_output ); 

			if( ret != JPG_RET_SUCCESS )
			{
				PRINTF( "[enc_test] JPU_EncStartOneFrame failed Error code is 0x%x \r\n", ret );
				continue;
			}

			DSTATUS( "[enc_test] VA addr[0x%x] BitstreamSize : %d\r\n",gsenc_output.m_BitstreamOut[VA], gsenc_output.m_iBitstreamOutSize );
			DSTATUS( "[enc_test] BitstreamSize = %d,  m_iHeaderOutSize = %d \r\n",gsenc_output.m_iBitstreamOutSize, gsenc_output.m_iHeaderOutSize );

			if ( pConfEnc->m_bStoreOutFile == 1 )
				_cdk_fwrite(gsenc_output.m_BitstreamOut[VA], sizeof(unsigned char), gsenc_output.m_iBitstreamOutSize, fp_out);
			else
			{				
				pConfEnc->m_outSizeOfHeaderBuff 	= gsenc_output.m_iHeaderOutSize;
				pConfEnc->m_outSizeOfEncodedBuff = 0;				
				pConfEnc->m_outSizeOfTotalBuff 	= gsenc_output.m_iBitstreamOutSize;
				memcpy(pConfEnc->m_outVirtHeaderBuffAddr, gsenc_output.m_BitstreamOut[VA], pConfEnc->m_outSizeOfTotalBuff);
			}
		}
	}

ERR_ENC_OPEN:
	DSTATUS("-------------------ENCODER CLOSE-------------------\r\n");
	// Now that we are done with encoding, close the open instance.
	ret = TCC_JPU_ENC( JPU_ENC_CLOSE, &enc_handle, 0, &gsenc_output );
	DSTATUS( "[enc_test] \nEnc End. Tot Frame %d\r\n", frame_idx );

ERR_ENC_INIT:

	if( fp_input )
		_cdk_fclose( fp_input );

	if( gencoded_out_buf_addr[PA] )
		cdk_sys_free_physical_addr_jpu( (void*)gencoded_out_buf_addr[PA], gencoded_out_buf_size );
	if( gencoded_out_buf_addr[VA] )
		cdk_sys_free_virtual_addr_jpu( &gencoded_out_buf_dev, (unsigned long)gencoded_out_buf_addr[VA], (unsigned long)gencoded_out_buf_size );

#ifdef  FILE_ENC_TEST
	if( gframe_buf_addr[PA] )
		cdk_sys_free_physical_addr_jpu( (void*)gframe_buf_addr[PA], gframe_buf_size );
	if( gframe_buf_addr[VA] )
		cdk_sys_free_virtual_addr_jpu( &gframe_buf_dev, (unsigned long)gframe_buf_addr[VA], (unsigned long)gframe_buf_size );
#endif

	vpu_env_close();

	return ret;
}


#endif


