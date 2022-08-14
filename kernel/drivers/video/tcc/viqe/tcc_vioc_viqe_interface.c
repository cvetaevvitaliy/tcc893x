/****************************************************************************
linux/drivers/video/tcc/viqe/tcc_vioc_viqe_interface.c
Description: TCC VIOC h/w block 

One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <mach/bsp.h>
#include <linux/delay.h>

#include <mach/vioc_rdma.h>
#include <mach/vioc_viqe.h>
#include <mach/vioc_scaler.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_config.h>
#include <mach/vioc_api.h>
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#else
#include <mach/vioc_plugin_tcc892x.h>
#endif
#include "tcc_vioc_viqe.h"
#include "tcc_vioc_viqe_interface.h"
#include <mach/tcc_viqe_ioctl.h>

#include <linux/fb.h>
#include <mach/tccfb_ioctrl.h>
#include <mach/tca_fb_output.h>
#include <plat/pmap.h>

#define USE_DEINTERLACE_S_IN30Hz
//#define USE_DEINTERLACE_S_IN60Hz

#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
//#define DYNAMIC_USE_DEINTL_COMPONENT
#endif

static pmap_t pmap_viqe;
#define PA_VIQE_BASE_ADDR	pmap_viqe.base
#define PA_VIQE_BASE_SIZE	pmap_viqe.size
static unsigned int gPMEM_VIQE_BASE;
static unsigned int gPMEM_VIQE_SIZE;

static VIQE *pVIQE_30Hz;
static VIOC_RDMA *pRDMABase_30Hz;
static int gRDMA_reg_30Hz = 0;
static int gVIQE_RDMA_num_30Hz = 0;
static int gFrmCnt_30Hz = 0;
static int gUseWmixer = 0;

static VIQE *pVIQE_60Hz;
static VIOC_RDMA *pRDMABase_60Hz;
static VIOC_SC *pSCALERBase_60Hz;
static VIOC_DISP *pDISPBase_60Hz;
static VIOC_WMIX *pWMIXBase_60Hz;
static VIOC_IREQ_CONFIG *pIREQConfig_60Hz;
static int gLCDCNum_60Hz = 0;
static int gOutputMode = 0;
static int gRDMA_reg_60Hz = 0;
static int gSCALER_reg_60Hz = 0;
static int gVIQE_RDMA_num_60Hz = 0;
static int gSC_RDMA_num_60Hz = 0;
static int gSCALER_num_60Hz = 0;
static int gFrmCnt_60Hz = 0;
static int gusingScale_60Hz = 0;

#ifdef USE_DEINTERLACE_S_IN60Hz
static int gbfield_30Hz =0;
#endif
static int gLcdc_layer_60Hz = -1;
static VIOC_VIQE_FMT_TYPE gViqe_fmt_60Hz;
static int gImg_fmt_60Hz = -1;
static int gpreCrop_left_60Hz = 0;
static int gpreCrop_right_60Hz = 0;
static int gpreCrop_top_60Hz = 0;
static int gpreCrop_bottom_60Hz = 0;

static int gDeintlS_Use_60Hz = 0;
static int gDeintlS_Use_Plugin = 0;

#ifdef USE_DEINTERLACE_S_IN30Hz
static int gusingDI_S = 0;
#else
static VIOC_VIQE_DEINTL_MODE gDI_mode_30Hz = VIOC_VIQE_DEINTL_MODE_2D;
#endif

#ifdef USE_DEINTERLACE_S_IN60Hz
static VIOC_VIQE_DEINTL_MODE gDI_mode_60Hz = VIOC_VIQE_DEINTL_S;
#else
static VIOC_VIQE_DEINTL_MODE gDI_mode_60Hz = VIOC_VIQE_DEINTL_MODE_2D;
#endif

static int gVIQE_Init_State = 0;

#if 0
#define dprintk(msg...)	 { printk( "tcc_vioc_viqe_interface: " msg); }
#else
#define dprintk(msg...)	 
#endif

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
static VIOC_RDMA *pRDMATempBase_60Hz;
extern int onthefly_LastFrame;
extern int enabled_LastFrame;
#endif

extern void tccxxx_GetAddress(unsigned char format, unsigned int base_Yaddr, unsigned int src_imgx, unsigned int  src_imgy,
					unsigned int start_x, unsigned int start_y, unsigned int* Y, unsigned int* U,unsigned int* V);

#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
extern unsigned char hdmi_get_hdmimode(void);
#endif

void TCC_VIQE_DI_PlugInOut_forAlphablending(int plugIn)
{
	volatile PVIOC_RDMA pRDMABase;
	volatile PVIQE pVIQE;
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	int VIQE_RDMA_num;

	#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935)
		VIQE_RDMA_num = VIOC_VIQE_RDMA_17;
		pRDMABase = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA17);
	#elif defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		VIQE_RDMA_num = VIOC_VIQE_RDMA_16;
		pRDMABase = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA16);
	#else
		VIQE_RDMA_num = VIOC_VIQE_RDMA_12;
		pRDMABase = (volatile PVIOC_RDMA)tcc_p2v((unsigned int)HwVIOC_RDMA12);
	#endif
	pVIQE = (volatile PVIQE)tcc_p2v((unsigned int)HwVIOC_VIQE0);
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

	if(gUseWmixer)
	{
		if(plugIn)
		{
			VIOC_RDMA_SetImageIntl(pRDMABase, 1);
			VIOC_CONFIG_PlugIn(VIOC_DEINTLS, VIQE_RDMA_num);
		}
		else
		{
			VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
			VIOC_RDMA_SetImageIntl(pRDMABase, 0);
		}			
	}
	else
	{
		if(plugIn)
		{
			VIOC_CONFIG_PlugIn(VIOC_VIQE, VIQE_RDMA_num);
			VIOC_RDMA_SetImageIntl(pRDMABase, 1);
		}
		else
		{
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(VIOC_VIQE);
			VIOC_RDMA_SetImageIntl(pRDMABase, 0);
			gFrmCnt_30Hz =gFrmCnt_60Hz = 0;		
		}
	}
}

/* VIQE Set */
//////////////////////////////////////////////////////////////////////////////////////////
void TCC_VIQE_DI_Init(int scalerCh, int useWMIXER, unsigned int srcWidth, unsigned int srcHeight,
						int crop_top, int crop_bottom, int crop_left, int crop_right, int OddFirst)
{
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	
#ifndef USE_DEINTERLACE_S_IN30Hz
	unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
	int imgSize;
	int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.
#endif
	unsigned int framebufWidth, framebufHeight;
	VIOC_VIQE_FMT_TYPE img_fmt = VIOC_VIQE_FMT_YUV420;

	pmap_get_info("viqe", &pmap_viqe);

	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);
	if(useWMIXER)
	{
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935)
		gRDMA_reg_30Hz = HwVIOC_RDMA12;
		gVIQE_RDMA_num_30Hz = VIOC_VIQE_RDMA_12;
		#elif defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		gRDMA_reg_30Hz = HwVIOC_RDMA07;
		gVIQE_RDMA_num_30Hz = VIOC_VIQE_RDMA_07;
		#else
		gRDMA_reg_30Hz = HwVIOC_RDMA14;
		gVIQE_RDMA_num_30Hz = VIOC_VIQE_RDMA_14;	
		#endif
		gUseWmixer = 1;
	}
	else
	{
		#if defined(CONFIG_CHIP_TCC8930) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8935)
			gRDMA_reg_30Hz = HwVIOC_RDMA17;
			gVIQE_RDMA_num_30Hz = VIOC_VIQE_RDMA_17;
		#elif defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			gRDMA_reg_30Hz = HwVIOC_RDMA16;
			gVIQE_RDMA_num_30Hz = VIOC_VIQE_RDMA_16;
		#else
			gRDMA_reg_30Hz = HwVIOC_RDMA12;
			gVIQE_RDMA_num_30Hz = VIOC_VIQE_RDMA_12;
		#endif
		gUseWmixer = 0;
	}
		
	gPMEM_VIQE_BASE = PA_VIQE_BASE_ADDR;
	gPMEM_VIQE_SIZE = PA_VIQE_BASE_SIZE;

	pVIQE_30Hz = (VIQE *)tcc_p2v(HwVIOC_VIQE0);
	pRDMABase_30Hz = (VIOC_RDMA *)tcc_p2v(gRDMA_reg_30Hz);
	
	crop_top = (crop_top >>1)<<1;
	framebufWidth = ((srcWidth - crop_left - crop_right) >> 3) << 3;			// 8bit align
	framebufHeight = ((srcHeight - crop_top - crop_bottom) >> 2) << 2;		// 4bit align

	printk("TCC_VIQE_DI_Init, W:%d, H:%d, FMT:%s, OddFirst:%d, RDMA:%d\n", framebufWidth, framebufHeight, (img_fmt?"YUV422":"YUV420"), OddFirst, ((gRDMA_reg_30Hz-HwVIOC_RDMA00)/256));
	
	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		if(pRDMABase_30Hz->uCTRL.nREG & HwDMA_IEN)
			VIOC_RDMA_SetImageDisable(pRDMABase_30Hz);

		TCC_OUTPUT_FB_ClearVideoImg();
	#endif

	VIOC_RDMA_SetImageY2REnable(pRDMABase_30Hz, FALSE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase_30Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
	VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 1);
	VIOC_RDMA_SetImageBfield(pRDMABase_30Hz, OddFirst);
	
	if(useWMIXER)
	{
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
		VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num_30Hz);
	}
	else
	{
		#ifdef USE_DEINTERLACE_S_IN30Hz
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
			VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num_30Hz);
			gusingDI_S = 1;
			printk("DEINTL-S\n");

		#else
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset
			// If you use 3D(temporal) De-interlace mode, you have to set physical address for using DMA register.
			//If 2D(spatial) mode, these registers are ignored
			imgSize = (framebufWidth * framebufHeight * 2);
			deintl_dma_base0	= gPMEM_VIQE_BASE;
			deintl_dma_base1	= deintl_dma_base0 + imgSize;
			deintl_dma_base2	= deintl_dma_base1 + imgSize;
			deintl_dma_base3	= deintl_dma_base2 + imgSize;	
			if (top_size_dont_use == OFF)
			{
				framebufWidth  = 0;
				framebufHeight = 0;
			}
			
			VIOC_VIQE_SetControlRegister(pVIQE_30Hz, framebufWidth, framebufHeight, img_fmt);
			VIOC_VIQE_SetDeintlRegister(pVIQE_30Hz, img_fmt, top_size_dont_use, framebufWidth, framebufHeight, gDI_mode_30Hz, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
			VIOC_VIQE_SetControlEnable(pVIQE_30Hz, OFF, OFF, OFF, OFF, ON);
			#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
			if(gVIQE_RDMA_num_30Hz== VIOC_VIQE_RDMA_03)
			#else	
			if(gVIQE_RDMA_num_30Hz== VIOC_VIQE_RDMA_02)
			#endif
			{
			#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
				if(gOutputMode != TCC_OUTPUT_HDMI || hdmi_get_hdmimode())
					VIOC_VIQE_SetImageY2REnable(pVIQE_30Hz, FALSE);
				else
					VIOC_VIQE_SetImageY2REnable(pVIQE_30Hz, TRUE);
			#elif defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
				if(gOutputMode == TCC_OUTPUT_COMPOSITE)
					VIOC_VIQE_SetImageY2REnable(pVIQE_30Hz, FALSE);
				else
					VIOC_VIQE_SetImageY2REnable(pVIQE_30Hz, TRUE);
			#else
				VIOC_VIQE_SetImageY2REnable(pVIQE_30Hz, TRUE);
			#endif
				VIOC_VIQE_SetImageY2RMode(pVIQE_30Hz, 0x02);
			}
			VIOC_CONFIG_PlugIn(VIOC_VIQE, gVIQE_RDMA_num_30Hz);
			if(OddFirst)
				gbfield_30Hz =1;
			else
				gbfield_30Hz =0;
		#endif
	}

	gFrmCnt_30Hz = 0;
	
	gVIQE_Init_State = 1;
}


void TCC_VIQE_DI_Run(int DI_use)
{
	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		if(gVIQE_Init_State == 0)
		{
			dprintk("%s VIQE block isn't initailized\n", __func__);
			return;
		}
	#endif

	if(gUseWmixer)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase_30Hz, FALSE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase_30Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */

		if(DI_use)
		{
			VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 1);
			VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num_30Hz);
		}
		else
		{
			VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
			VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 0);
		}
		
	}
	else
	{
		#ifndef USE_DEINTERLACE_S_IN30Hz
		if(DI_use)
		{
			VIOC_RDMA_SetImageY2REnable(pRDMABase_30Hz, FALSE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase_30Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
			
			if(gFrmCnt_30Hz >= 3)
			{
				VIOC_VIQE_SetDeintlMode(pVIQE_30Hz, VIOC_VIQE_DEINTL_MODE_3D);
				gDI_mode_30Hz = VIOC_VIQE_DEINTL_MODE_3D;			
			}
			else
			{
				VIOC_VIQE_SetDeintlMode(pVIQE_30Hz, VIOC_VIQE_DEINTL_MODE_2D);
				gDI_mode_30Hz = VIOC_VIQE_DEINTL_MODE_2D;			
			}
			VIOC_VIQE_SetControlMode(pVIQE_30Hz, OFF, OFF, OFF, OFF, ON);
			VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 1);
			
			if (gbfield_30Hz) 					// end fied of bottom field
			{
				VIOC_RDMA_SetImageBfield(pRDMABase_30Hz, 0);				// change the bottom to top field
				// if you want to change the base address, you call the RDMA SetImageBase function in this line.
				gbfield_30Hz= 0;
			} 
			else 
			{
				VIOC_RDMA_SetImageBfield(pRDMABase_30Hz, 1);				// change the top to bottom field
				gbfield_30Hz = 1;
			}
		}
		else
		{	
			VIOC_VIQE_SetControlMode(pVIQE_30Hz, OFF, OFF, OFF, OFF, OFF);
			VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 0);
			gFrmCnt_30Hz = 0;
		}	


		#else
			VIOC_RDMA_SetImageY2REnable(pRDMABase_30Hz, FALSE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase_30Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
			if(DI_use)
			{
				VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 1);
				if(!gusingDI_S)
				{
					VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num_30Hz);
					gusingDI_S = 1;
				}
			}
			else
			{
				VIOC_RDMA_SetImageIntl(pRDMABase_30Hz, 0);
				if(gusingDI_S)
				{
					VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
					gusingDI_S = 0;
				}
			}
		#endif
	}

	gFrmCnt_30Hz++;	
}

void TCC_VIQE_DI_DeInit(void)
{
	volatile PVIOC_IREQ_CONFIG pIREQConfig;
	pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);
	gFrmCnt_30Hz = 0;

	printk("TCC_VIQE_DI_DeInit\n");

	if(gUseWmixer)
	{
		VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
	}
	else
	{
		#ifdef USE_DEINTERLACE_S_IN30Hz	
			VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
			gusingDI_S = 0;
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
		#else
			VIOC_CONFIG_PlugOut(VIOC_VIQE);
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
			BITCSET(pIREQConfig->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset
		#endif
	}

	gVIQE_Init_State = 0;
}

/* 
	//img_fmt
	TCC_LCDC_IMG_FMT_YUV420SP = 24,	
	TCC_LCDC_IMG_FMT_YUV422SP = 25, 
	TCC_LCDC_IMG_FMT_YUYV = 26, 
	TCC_LCDC_IMG_FMT_YVYU = 27,
	
	TCC_LCDC_IMG_FMT_YUV420ITL0 = 28, 
	TCC_LCDC_IMG_FMT_YUV420ITL1 = 29, 
	TCC_LCDC_IMG_FMT_YUV422ITL0 = 30, 
	TCC_LCDC_IMG_FMT_YUV422ITL1 = 31, 
*/

//////////////////////////////////////////////////////////////////////////////////////////
void TCC_VIQE_DI_Init60Hz(TCC_OUTPUT_TYPE outputMode, int lcdCtrlNum, int Lcdc_layer, int useSCALER, unsigned int img_fmt, 
						unsigned int srcWidth, unsigned int srcHeight,
						unsigned int destWidth, unsigned int destHeight,
						unsigned int offset_x, unsigned int offset_y, int OddFirst)
{
	unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
	unsigned int framebufWidth, framebufHeight;
	unsigned int lcd_width = 0, lcd_height = 0, scale_x = 0, scale_y = 0;
	int imgSize;
	int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.

#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	gLcdc_layer_60Hz = 3;
#else		
	gLcdc_layer_60Hz = 2;
#endif

	if(img_fmt == 24 || img_fmt == 28 || img_fmt==29)
		gViqe_fmt_60Hz = VIOC_VIQE_FMT_YUV420;
	else
		gViqe_fmt_60Hz = VIOC_VIQE_FMT_YUV422;
	gImg_fmt_60Hz = img_fmt;
		
	pmap_get_info("viqe", &pmap_viqe);
	gPMEM_VIQE_BASE = PA_VIQE_BASE_ADDR;
	gPMEM_VIQE_SIZE = PA_VIQE_BASE_SIZE;

	gOutputMode = outputMode;
	if(gOutputMode == TCC_OUTPUT_LCD)
	{
		gSCALER_reg_60Hz = HwVIOC_SC1;
		gSCALER_num_60Hz = VIOC_SC1;
	}
	else /* TCC_OUTPUT_HDMI, COMPONENT, COMPOSITE*/
	{

		#if defined(CONFIG_ARCH_TCC893X)
			#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
				gSCALER_reg_60Hz = HwVIOC_SC1;
				gSCALER_num_60Hz = VIOC_SC1;
			#else
				gSCALER_reg_60Hz = HwVIOC_SC3;
				gSCALER_num_60Hz = VIOC_SC3;
			#endif
		#else
		gSCALER_reg_60Hz = HwVIOC_SC1;
		gSCALER_num_60Hz = VIOC_SC1;
		#endif
	}
	
	gLCDCNum_60Hz = lcdCtrlNum;
	if(gLCDCNum_60Hz)
	{
		#if defined(CONFIG_CHIP_TCC8925S)|| defined(CONFIG_ARCH_TCC893X)
			gRDMA_reg_60Hz = HwVIOC_RDMA07;
			gVIQE_RDMA_num_60Hz = VIOC_VIQE_RDMA_07;
			gSC_RDMA_num_60Hz = VIOC_SC_RDMA_07;
		#else
			gRDMA_reg_60Hz = HwVIOC_RDMA06;
			gVIQE_RDMA_num_60Hz = VIOC_VIQE_RDMA_06;
			gSC_RDMA_num_60Hz = VIOC_SC_RDMA_06;
		#endif /* CONFIG_CHIP_TCC8925S */
		
		pDISPBase_60Hz = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase_60Hz = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		
	}
	else
	{
		#if defined(CONFIG_CHIP_TCC8925S)|| defined(CONFIG_ARCH_TCC893X)
			gRDMA_reg_60Hz = HwVIOC_RDMA03;
			gVIQE_RDMA_num_60Hz = VIOC_VIQE_RDMA_03;
			gSC_RDMA_num_60Hz = VIOC_SC_RDMA_03;
		#else
			gRDMA_reg_60Hz = HwVIOC_RDMA02;
			gVIQE_RDMA_num_60Hz = VIOC_VIQE_RDMA_02;
			gSC_RDMA_num_60Hz = VIOC_SC_RDMA_02;
		#endif /* CONFIG_CHIP_TCC8925S */

		pDISPBase_60Hz = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase_60Hz = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
	}	

	pVIQE_60Hz = (VIQE *)tcc_p2v(HwVIOC_VIQE0);
	pRDMABase_60Hz = (VIOC_RDMA *)tcc_p2v(gRDMA_reg_60Hz);
	pSCALERBase_60Hz = (VIOC_SC *)tcc_p2v(gSCALER_reg_60Hz );
	pIREQConfig_60Hz = (VIOC_IREQ_CONFIG *)tcc_p2v(HwVIOC_IREQ);

	framebufWidth = ((srcWidth) >> 3) << 3;			// 8bit align
	framebufHeight = ((srcHeight) >> 2) << 2;		// 4bit align
	printk("TCC_VIQE_DI_Init60Hz, W:%d, H:%d, DW:%d, DH:%d, FMT:%d, VFMT:%s, OddFirst:%d, RDMA:%d\n", framebufWidth, framebufHeight, destWidth, destHeight, img_fmt, (gViqe_fmt_60Hz?"YUV422":"YUV420"), OddFirst, ((gRDMA_reg_60Hz-HwVIOC_RDMA00)/256));

	VIOC_DISP_GetSize(pDISPBase_60Hz, &lcd_width, &lcd_height);
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s invalid lcd size\n", __func__);
		return;
	}

	#if defined(USE_DEINTERLACE_S_IN60Hz)
		gDeintlS_Use_60Hz = 1;
	#else
		#if defined(DYNAMIC_USE_DEINTL_COMPONENT)
			if((framebufWidth >= 1280) && (framebufHeight >= 720))
			{
				gDeintlS_Use_60Hz = 1;
				gDI_mode_60Hz = VIOC_VIQE_DEINTL_S;
			}
			else
			{
				gDeintlS_Use_60Hz = 0;
				gDI_mode_60Hz = VIOC_VIQE_DEINTL_MODE_2D;
			}
		#endif
	#endif

	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
		if(pRDMABase_60Hz->uCTRL.nREG & HwDMA_IEN)
			VIOC_RDMA_SetImageDisable(pRDMABase_60Hz);

		TCC_OUTPUT_FB_ClearVideoImg();
	#endif

	//RDMA SETTING
	if(gDeintlS_Use_60Hz)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase_60Hz, TRUE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase_60Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
	}
	else
	{
		#if 0
		if( gDI_mode == VIOC_VIQE_DEINTL_MODE_BYPASS)
		{
			VIOC_RDMA_SetImageY2REnable(pRDMABase_60Hz, TRUE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase_60Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
		}
		else
		#endif		
		{
			VIOC_RDMA_SetImageY2REnable(pRDMABase_60Hz, FALSE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase_60Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
		}
	}

	VIOC_RDMA_SetImageOffset(pRDMABase_60Hz, img_fmt, framebufWidth);
	VIOC_RDMA_SetImageFormat(pRDMABase_60Hz, img_fmt);
	VIOC_RDMA_SetImageScale(pRDMABase_60Hz, scale_x, scale_y);
	VIOC_RDMA_SetImageSize(pRDMABase_60Hz, framebufWidth, framebufHeight);
	VIOC_RDMA_SetImageIntl(pRDMABase_60Hz, 1);
	VIOC_RDMA_SetImageBfield(pRDMABase_60Hz, OddFirst);

	if(gDeintlS_Use_60Hz)
	{
		printk("%s, DeinterlacerS is used!!\n", __func__);
		
		deintl_dma_base0	= 0;
		deintl_dma_base1	= 0;
		deintl_dma_base2	= 0;
		deintl_dma_base3	= 0;
		VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num_60Hz);
		gDeintlS_Use_Plugin = 1;
	}
	else
	{
		printk("%s, VIQE is used!!\n", __func__);
		
		// If you use 3D(temporal) De-interlace mode, you have to set physical address for using DMA register.
		//If 2D(spatial) mode, these registers are ignored
		imgSize = (framebufWidth * framebufHeight * 2);
		deintl_dma_base0	= gPMEM_VIQE_BASE;
		deintl_dma_base1	= deintl_dma_base0 + imgSize;
		deintl_dma_base2	= deintl_dma_base1 + imgSize;
		deintl_dma_base3	= deintl_dma_base2 + imgSize;	

		VIOC_VIQE_SetControlRegister(pVIQE_60Hz, framebufWidth, framebufHeight, gViqe_fmt_60Hz);
		VIOC_VIQE_SetDeintlRegister(pVIQE_60Hz, gViqe_fmt_60Hz, top_size_dont_use, framebufWidth, framebufHeight, gDI_mode_60Hz, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
		//VIOC_VIQE_SetDenoise(pVIQE_60Hz, gViqe_fmt_60Hz, framebufWidth, framebufHeight, 1, 0, deintl_dma_base0, deintl_dma_base1); 	//for bypass path on progressive frame
		VIOC_VIQE_SetControlEnable(pVIQE_60Hz, OFF, OFF, OFF, OFF, ON);
		//if(gDI_mode != VIOC_VIQE_DEINTL_MODE_BYPASS)
		{
		#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
			if(gOutputMode != TCC_OUTPUT_HDMI || hdmi_get_hdmimode())
				VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, FALSE);
			else
				VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, TRUE);
		#elif defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
			if(gOutputMode == TCC_OUTPUT_COMPOSITE)
				VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, FALSE);
			else
				VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, TRUE);
		#else
			VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, TRUE);
		#endif
			VIOC_VIQE_SetImageY2RMode(pVIQE_60Hz, 0x02);
		}
		VIOC_CONFIG_PlugIn(VIOC_VIQE, gVIQE_RDMA_num_60Hz);
	}

	//SCALER SETTING
	if(useSCALER)
	{
		VIOC_CONFIG_PlugOut(gSCALER_num_60Hz);			
		VIOC_CONFIG_PlugIn (gSCALER_num_60Hz, gSC_RDMA_num_60Hz);			
		VIOC_SC_SetSWReset(gSCALER_num_60Hz, 0xFF, 0xFF);
		VIOC_SC_SetBypass (pSCALERBase_60Hz, OFF);
		
		VIOC_SC_SetDstSize (pSCALERBase_60Hz, destWidth, destHeight);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSCALERBase_60Hz, destWidth, destHeight);			// set output size in scaer
		VIOC_SC_SetUpdate (pSCALERBase_60Hz);
		gusingScale_60Hz = 1;
	}
	
	VIOC_WMIX_SetPosition(pWMIXBase_60Hz, gLcdc_layer_60Hz,  offset_x, offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase_60Hz);

	gFrmCnt_60Hz= 0;		

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	if(gLCDCNum_60Hz)
		pRDMATempBase_60Hz = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);		
	else
		pRDMATempBase_60Hz = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
#endif
	
	gVIQE_Init_State = 1;
}


void TCC_VIQE_DI_Swap60Hz(int mode)
{
	VIOC_PlugInOutCheck VIOC_PlugIn;

	VIOC_CONFIG_Device_PlugState(VIOC_VIQE, &VIOC_PlugIn);
	if(VIOC_PlugIn.enable)
		VIOC_VIQE_SwapDeintlBase(pVIQE_60Hz, mode);
}
void TCC_VIQE_DI_SetFMT60Hz(int enable)
{
	VIOC_PlugInOutCheck VIOC_PlugIn;

	VIOC_CONFIG_Device_PlugState(VIOC_VIQE, &VIOC_PlugIn);
	if(VIOC_PlugIn.enable)
		VIOC_VIQE_SetDeintlFMT(pVIQE_60Hz, enable);
}

void TCC_VIQE_DI_Run60Hz(int useSCALER, unsigned int addr0, unsigned int addr1, unsigned int addr2,
						unsigned int srcWidth, unsigned int srcHeight,	
						int crop_top, int crop_bottom, int crop_left, int crop_right, 
						unsigned int destWidth, unsigned int destHeight, 
						unsigned int offset_x, unsigned int offset_y, int OddFirst, int FrameInfo_Interlace, int reset_frmCnt)
{
	unsigned int lcd_width = 0, lcd_height = 0;
	int cropWidth, cropHeight;
	VIOC_PlugInOutCheck VIOC_PlugIn;

	#if defined(CONFIG_MACH_TCC8920ST) || defined(CONFIG_MACH_TCC8930ST)
	if(gVIQE_Init_State == 0)
	{
		dprintk("%s VIQE block isn't initailized\n", __func__);
		return;
	}
	#endif

	VIOC_CONFIG_Device_PlugState(VIOC_VIQE, &VIOC_PlugIn);
	if(VIOC_PlugIn.enable) {

	}
	else {
		dprintk("%s VIQE block isn't pluged!!!\n", __func__);
		return;		
	}

	VIOC_DISP_GetSize(pDISPBase_60Hz, &lcd_width, &lcd_height);
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s invalid lcd size\n", __func__);
		return;
	}
	dprintk("DI %d :: crop(%d, %d ~ %dx%d) => %d x %d, addr: 0x%x, R-0x%x/0x%x, S-0x%x/0x%x, V-0x%x \n", useSCALER, crop_left, crop_top, cropWidth, cropHeight, destWidth, destHeight, addr0,
					pRDMABase_60Hz->uCTRL.nREG, pRDMABase_60Hz->uSTATUS.nREG, pSCALERBase_60Hz->uCTRL.nREG, pSCALERBase_60Hz->uSTATUS.nREG, pVIQE_60Hz->cVIQE_CTRL.nVIQE_CTRL.nREG);

	if(reset_frmCnt)
		gFrmCnt_60Hz = 0;
	
	crop_top = (crop_top >>1)<<1;
	cropWidth = ((crop_right - crop_left) >> 3) << 3;		//8bit align
	cropHeight = ((crop_bottom - crop_top) >> 2) << 2;		//4bit align
	{
		int addr_Y = (unsigned int)addr0;
		int addr_U = (unsigned int)addr1;
		int addr_V = (unsigned int)addr2;

	#if defined(CONFIG_ARCH_TCC892X) && !defined(CONFIG_CHIP_TCC8925S) && !defined(CONFIG_VIOC_FIFO_UNDER_RUN_COMPENSATION)
	//Display FIFO under-run on TCC8920 and TCC8925, when current_crop size is smaller than prev_crop size(screen mode: PANSCAN<->BOX)
	//So, Soc team suggested VIQE block resetting
		if(gDeintlS_Use_60Hz == 0)
		{
			if((gFrmCnt_60Hz!=0) && ((gpreCrop_left_60Hz != crop_left) || (gpreCrop_right_60Hz !=crop_right) || (gpreCrop_top_60Hz != crop_top) || (gpreCrop_bottom_60Hz !=crop_bottom)))
			{
				unsigned int deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3;
				int imgSize;
				int top_size_dont_use = OFF;		//If this value is OFF, The size information is get from VIOC modules.
				
				VIOC_RDMA_SetImageDisable(pRDMABase_60Hz);	
				VIOC_CONFIG_PlugOut(VIOC_VIQE);

				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset

				imgSize = (srcWidth * srcHeight * 2);
				deintl_dma_base0	= gPMEM_VIQE_BASE;
				deintl_dma_base1	= deintl_dma_base0 + imgSize;
				deintl_dma_base2	= deintl_dma_base1 + imgSize;
				deintl_dma_base3	= deintl_dma_base2 + imgSize;	

				VIOC_VIQE_SetControlRegister(pVIQE_60Hz, srcWidth, srcHeight, gViqe_fmt_60Hz);
				VIOC_VIQE_SetDeintlRegister(pVIQE_60Hz, gViqe_fmt_60Hz, top_size_dont_use, srcWidth, srcHeight, gDI_mode_60Hz, deintl_dma_base0, deintl_dma_base1, deintl_dma_base2, deintl_dma_base3);
				//VIOC_VIQE_SetDenoise(pVIQE_60Hz, gViqe_fmt_60Hz, srcWidth, srcHeight, 1, 0, deintl_dma_base0, deintl_dma_base1); 	//for bypass path on progressive frame

			#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
				if(gOutputMode != TCC_OUTPUT_HDMI || hdmi_get_hdmimode())
					VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, FALSE);
				else
					VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, TRUE);
			#elif defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
				if(gOutputMode == TCC_OUTPUT_COMPOSITE)
					VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, FALSE);
				else
					VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, TRUE);
			#else
				VIOC_VIQE_SetImageY2REnable(pVIQE_60Hz, TRUE);
			#endif
				VIOC_VIQE_SetImageY2RMode(pVIQE_60Hz, 0x02);
				
				VIOC_CONFIG_PlugIn(VIOC_VIQE, gVIQE_RDMA_num_60Hz);
			}
		}
	#endif			

		tccxxx_GetAddress(gImg_fmt_60Hz, (unsigned int)addr0, srcWidth, srcHeight, crop_left, crop_top, &addr_Y, &addr_U, &addr_V);
		
		VIOC_RDMA_SetImageSize(pRDMABase_60Hz, cropWidth, cropHeight);
		VIOC_RDMA_SetImageBase(pRDMABase_60Hz, addr_Y, addr_U, addr_V);
		VIOC_RDMA_SetImageOffset(pRDMABase_60Hz, gImg_fmt_60Hz, srcWidth);

		gpreCrop_left_60Hz = crop_left;
		gpreCrop_right_60Hz = crop_right;
		gpreCrop_top_60Hz = crop_top;
		gpreCrop_bottom_60Hz = crop_bottom;
	}
	VIOC_RDMA_SetImageBfield(pRDMABase_60Hz, OddFirst);
	
	dprintk(" Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d], W:%d, H:%d odd:%d\n", crop_left, crop_right, crop_top, crop_bottom, cropWidth, cropHeight, OddFirst);

	if(gDeintlS_Use_60Hz)
	{
		if(FrameInfo_Interlace)
		{
			VIOC_RDMA_SetImageIntl(pRDMABase_60Hz, 1);
			if(!gDeintlS_Use_Plugin)
			{
				VIOC_CONFIG_PlugIn(VIOC_DEINTLS, gVIQE_RDMA_num_60Hz);
				gDeintlS_Use_Plugin = 1;
			}
		}
		else
		{
			VIOC_RDMA_SetImageIntl(pRDMABase_60Hz, 0);
			if(gDeintlS_Use_Plugin)
			{
				VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
				gDeintlS_Use_Plugin = 0;
			}
		}
	}
	else
	{
		if(FrameInfo_Interlace)
		{
			VIOC_RDMA_SetImageY2REnable(pRDMABase_60Hz, FALSE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase_60Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
			if(gFrmCnt_60Hz >= 3)
			{
				VIOC_VIQE_SetDeintlMode(pVIQE_60Hz, VIOC_VIQE_DEINTL_MODE_3D);
				gDI_mode_60Hz = VIOC_VIQE_DEINTL_MODE_3D;
			}
			else
			{
				VIOC_VIQE_SetDeintlMode(pVIQE_60Hz, VIOC_VIQE_DEINTL_MODE_2D);
				gDI_mode_60Hz = VIOC_VIQE_DEINTL_MODE_2D;
			}
			VIOC_VIQE_SetControlMode(pVIQE_60Hz, OFF, OFF, OFF, OFF, ON);
			VIOC_RDMA_SetImageIntl(pRDMABase_60Hz, 1);
		}
		else
		{	
			VIOC_RDMA_SetImageY2REnable(pRDMABase_60Hz, TRUE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase_60Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
			VIOC_VIQE_SetControlMode(pVIQE_60Hz, OFF, OFF, OFF, OFF, OFF);
			VIOC_RDMA_SetImageIntl(pRDMABase_60Hz, 0);
			gFrmCnt_60Hz = 0;
		}
	}

	if(useSCALER)
	{
		if(!gusingScale_60Hz) {
			gusingScale_60Hz = 1;
			VIOC_CONFIG_PlugIn (gSCALER_num_60Hz, gSC_RDMA_num_60Hz);			
			VIOC_SC_SetBypass (pSCALERBase_60Hz, OFF);
		}
		
		VIOC_SC_SetDstSize (pSCALERBase_60Hz, destWidth, destHeight);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSCALERBase_60Hz, destWidth, destHeight);			// set output size in scaer
		VIOC_SC_SetUpdate (pSCALERBase_60Hz);
	}
	else
	{
		if(gusingScale_60Hz == 1)	{
			VIOC_RDMA_SetImageDisable(pRDMABase_60Hz);
			VIOC_CONFIG_PlugOut(gSCALER_num_60Hz);
			gusingScale_60Hz = 0;
		}
	}

	// position
	VIOC_WMIX_SetPosition(pWMIXBase_60Hz, gLcdc_layer_60Hz,  offset_x, offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase_60Hz);

	VIOC_RDMA_SetImageEnable(pRDMABase_60Hz);
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	if(enabled_LastFrame)
	{
		VIOC_RDMA_SetImageDisable(pRDMATempBase_60Hz);
		if(onthefly_LastFrame)
		{
			onthefly_LastFrame = 0;
			VIOC_CONFIG_PlugOut(VIOC_SC0); //printk("\n\n\n\n");
		}
	}
#endif

	gFrmCnt_60Hz++;	
}

void TCC_VIQE_DI_DeInit60Hz(void)
{
	printk("TCC_VIQE_DI_DeInit60Hz\n");
	VIOC_RDMA_SetImageDisable(pRDMABase_60Hz);	
	VIOC_RDMA_SetImageY2REnable(pRDMABase_60Hz, TRUE);
	VIOC_RDMA_SetImageY2RMode(pRDMABase_60Hz, 0x02); /* Y2RMode Default 0 (Studio Color) */
	VIOC_CONFIG_PlugOut(gSCALER_num_60Hz);
	gusingScale_60Hz = 0;
	gFrmCnt_60Hz = 0;	
	
	if(gDeintlS_Use_60Hz)
	{
		VIOC_CONFIG_PlugOut(VIOC_DEINTLS);
		BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[1], (0x1<<17), (0x01<<17)); // DEINTLS reset
		BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[1], (0x1<<17), (0x00<<17)); // DEINTLS reset
		gDeintlS_Use_Plugin = 0;
	}
	else
	{
		VIOC_CONFIG_PlugOut(VIOC_VIQE);
		BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[1], (0x1<<16), (0x01<<16)); // VIQE reset
		BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[1], (0x1<<16), (0x00<<16)); // VIQE reset

		if(gOutputMode == TCC_OUTPUT_LCD)
		{
			BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x1<<(28+VIOC_SC1))); // scaler reset
			BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x0<<(28+VIOC_SC1))); // scaler reset
		}
		else	//HDMI, COMPONENT, COMPOSITE
		{
			#if defined(CONFIG_ARCH_TCC893X)
				#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
					BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x1<<(28+VIOC_SC1))); // scaler reset
					BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x0<<(28+VIOC_SC1))); // scaler reset
				#else
					BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC3)), (0x1<<(28+VIOC_SC3))); // scaler reset
					BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC3)), (0x0<<(28+VIOC_SC3))); // scaler reset
				#endif
			#else
			BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x1<<(28+VIOC_SC1))); // scaler reset
			BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<(28+VIOC_SC1)), (0x0<<(28+VIOC_SC1))); // scaler reset
			#endif

		}

		if(gLCDCNum_60Hz)
		{
			#if defined(CONFIG_CHIP_TCC8925S)|| defined(CONFIG_ARCH_TCC893X)
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_07), (0x1<<VIOC_WMIX_RDMA_07)); // rdma reset
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_07), (0x0<<VIOC_WMIX_RDMA_07)); // rdma reset
			#else
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_06), (0x1<<VIOC_WMIX_RDMA_06)); // rdma reset
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_06), (0x0<<VIOC_WMIX_RDMA_06)); // rdma reset
			#endif /* CONFIG_CHIP_TCC8925S */
		}
		else
		{
			#if defined(CONFIG_CHIP_TCC8925S)|| defined(CONFIG_ARCH_TCC893X)
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_03), (0x1<<VIOC_WMIX_RDMA_03)); // rdma reset
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_03), (0x0<<VIOC_WMIX_RDMA_03)); // rdma reset
			#else
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_02), (0x1<<VIOC_WMIX_RDMA_02)); // rdma reset
				BITCSET(pIREQConfig_60Hz->uSOFTRESET.nREG[0], (0x1<<VIOC_WMIX_RDMA_02), (0x0<<VIOC_WMIX_RDMA_02)); // rdma reset
			#endif /* CONFIG_CHIP_TCC8925S */
		
		}
	}

	gVIQE_Init_State = 0;
}

