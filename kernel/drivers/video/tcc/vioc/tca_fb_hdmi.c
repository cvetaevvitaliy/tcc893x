
/****************************************************************************
 * linux/drivers/video/tca_fb_hdmi.c
 *
 * Author:  <linux@telechips.com>
 * Created: March 18, 2012
 * Description: TCC HDMI TV-Out Driver
 *
 * Copyright (C) 20010-2011 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/
/*****************************************************************************
*
* Header Files Include
*
******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <linux/cpufreq.h>
#include <mach/bsp.h>

#if defined(CONFIG_ARCH_TCC892X)
#include <mach/vioc_plugin_tcc892x.h>
#endif
#if defined(CONFIG_ARCH_TCC893X)
#include <mach/vioc_plugin_tcc893x.h>
#endif
#include <mach/tccfb_ioctrl.h>
#include <mach/tca_fb_hdmi.h>
#include <mach/tca_fb_output.h>
#include <mach/vioc_outcfg.h>
#include <mach/vioc_rdma.h>
#include <mach/vioc_wdma.h>
#include <mach/vioc_wmix.h>
#include <mach/vioc_disp.h>
#include <mach/vioc_config.h>
#include <mach/vioc_scaler.h>
#include <mach/tccfb_address.h>

#if defined(CONFIG_TCC_HDMI_DRIVER_V1_3)
#include <mach/hdmi_1_3_hdmi.h>
#elif defined(CONFIG_TCC_HDMI_DRIVER_V1_4)
#include <mach/hdmi_1_4_hdmi.h>
#else
#error
#endif

#define RDMA_UVI_MAX_WIDTH             3072

extern void tccxxx_GetAddress(unsigned char format, unsigned int base_Yaddr, unsigned int src_imgx, unsigned int  src_imgy,
									unsigned int start_x, unsigned int start_y, unsigned int* Y, unsigned int* U,unsigned int* V);

#if 0
#define dprintk(msg...)	 { printk( "tca_hdmi: " msg); }
#else
#define dprintk(msg...)	 
#endif

extern struct display_platform_data tcc_display_data;

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
extern int onthefly_LastFrame;
extern int enabled_LastFrame;
extern VIOC_RDMA * pLastFrame_RDMABase;
#endif
#if 1//def CONFIG_HDMI_DISPLAY_LASTFRAME
extern unsigned int LastFrame;
extern TCC_OUTPUT_TYPE	Output_SelectMode;
extern void tcc_plugout_for_component(int ch_layer);
extern void tcc_plugout_for_composite(int ch_layer);
#endif

#if defined(MVC_PROCESS)
extern int hdmi_get_VBlank(void);
#endif

#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
extern char output_starter_state;
extern unsigned char hdmi_get_hdmimode(void);
#endif

void TCC_HDMI_LCDC_Timing(char hdmi_lcdc, struct lcdc_timimg_parms_t *mode)
{
	unsigned int width, height;
	VIOC_DISP *pDISP;
	VIOC_WMIX * pWMIX;	
	stLCDCTR pCtrlParam;
	stLTIMING HDMI_TIMEp;

	if(hdmi_lcdc)	{
		pDISP = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP1);
		pWMIX =(VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX1); 
	}
	else		{
		pDISP = (VIOC_DISP *)tcc_p2v(HwVIOC_DISP0);
		pWMIX =(VIOC_WMIX *)tcc_p2v(HwVIOC_WMIX0); 
	}

	if(mode->dp)
	{
		width = (mode->lpc + 1)/2;
		height = mode->flc + 1;
	}
	else
	{
		width = (mode->lpc + 1);
		height = mode->flc + 1;
	}

	printk("##### %s hdmi_lcdc=%d, resolution=%dx%d%c\n", __func__, hdmi_lcdc, width, height, mode->ni? 'p':'i');

	HDMI_TIMEp.lpw= mode->lpw;
	HDMI_TIMEp.lpc= mode->lpc + 1;
	HDMI_TIMEp.lswc= mode->lswc+ 1;
	HDMI_TIMEp.lewc= mode->lewc+ 1;
	
	HDMI_TIMEp.vdb = mode->vdb;
	HDMI_TIMEp.vdf = mode->vdf;
	HDMI_TIMEp.fpw = mode->fpw;
	HDMI_TIMEp.flc = mode->flc;
	HDMI_TIMEp.fswc = mode->fswc;
	HDMI_TIMEp.fewc = mode->fewc;
	HDMI_TIMEp.fpw2 = mode->fpw2;
	HDMI_TIMEp.flc2 = mode->flc2;
	HDMI_TIMEp.fswc2 = mode->fswc2;
	HDMI_TIMEp.fewc2 = mode->fewc2;
	
	VIOC_DISP_SetTimingParam(pDISP, &HDMI_TIMEp);

	memset(&pCtrlParam, 0x00, sizeof(pCtrlParam));
	pCtrlParam.id= mode->id;
	pCtrlParam.iv= mode->iv;
	pCtrlParam.ih= mode->ih;
	pCtrlParam.ip= mode->ip;
	pCtrlParam.clen = 0;
	pCtrlParam.r2y = 0;
	//pCtrlParam.pxdw = 12; //RGB888
	pCtrlParam.dp = mode->dp;
	pCtrlParam.ni = mode->ni;
#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
	if( mode->ni == 0)
		pCtrlParam.advi = 0;
#else
	if( mode->ni == 0)
		pCtrlParam.advi = 1;
#endif
	pCtrlParam.tv = mode->tv;
	pCtrlParam.opt = 0;
	pCtrlParam.stn = 0;
	pCtrlParam.evsel = 0;
	pCtrlParam.ovp = 0;
	
	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
		if(output_starter_state || hdmi_get_hdmimode() == DVI)
			pCtrlParam.pxdw = 12; //RGB888
		else
			pCtrlParam.pxdw = 8; //YCbCr
	#else
		pCtrlParam.pxdw = 12; //RGB888
	#endif
	
	VIOC_DISP_SetControlConfigure(pDISP, &pCtrlParam);

	VIOC_DISP_SetSize (pDISP, width, height);
	VIOC_DISP_SetBGColor(pDISP, 0, 0 , 0);

	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
		if(output_starter_state || hdmi_get_hdmimode() == DVI)
			VIOC_WMIX_SetBGColor(pWMIX, 0x00, 0x00, 0x00, 0xff);
		else
			VIOC_WMIX_SetBGColor(pWMIX, 0x00, 0x80, 0x80, 0x00);
	#else
		VIOC_WMIX_SetBGColor(pWMIX, 0x00, 0x00, 0x00, 0xff);
	#endif

	VIOC_WMIX_SetSize(pWMIX, width, height);
	VIOC_WMIX_SetUpdate (pWMIX);
	
	VIOC_OUTCFG_SetOutConfig(VIOC_OUTCFG_HDMI, hdmi_lcdc);
	
}

// 0 : 3 : layer enable/disable 
static unsigned int onthefly_using;
void TCC_HDMI_SET_OnTheFly(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo)
{
	
	VIOC_RDMA * pRDMABase;
	unsigned int iSCType;
	unsigned int nRDMB;

	if(ImageInfo->MVCframeView == 1)
	{
		return;
	}
	
#if defined(CONFIG_ARCH_TCC893X)
	#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		iSCType = VIOC_SC1;
	#else
		iSCType = VIOC_SC3;
	#endif
#else
	iSCType = VIOC_SC1;
#endif /* CONFIG_ARCH_TCC893X */
	
	#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	ImageInfo->Lcdc_layer = 3;
	#else		
	ImageInfo->Lcdc_layer = 2;
	#endif
	
	if(hdmi_lcdc)
	{
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
				nRDMB = VIOC_WMIX_RDMA_04;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
				nRDMB = VIOC_WMIX_RDMA_05;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
				nRDMB = VIOC_WMIX_RDMA_06;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
				nRDMB = VIOC_WMIX_RDMA_07;
				break;
					
		}
	}
	else
	{
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);
				nRDMB = VIOC_WMIX_RDMA_00;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
				nRDMB = VIOC_WMIX_RDMA_01;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);
				nRDMB = VIOC_WMIX_RDMA_02;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);
				nRDMB = VIOC_WMIX_RDMA_03;
				break;						
		}		
	}
		
	if(ImageInfo->on_the_fly)
	{
		if(!onthefly_using)
		{ 
			VIOC_RDMA_SetImageDisable(pRDMABase);
			printk(" %s  scaler[%d] is plug in RDMA %d \n",__func__, iSCType,nRDMB);
			BITSET(onthefly_using, 1 << ImageInfo->Lcdc_layer);
			VIOC_CONFIG_PlugIn (iSCType, nRDMB);	
		}
	}
	else
	{
		if(ISSET(onthefly_using, 1<<ImageInfo->Lcdc_layer))
		{
			printk(" %s  scaler[%d] is plug out	\n",__func__,iSCType);
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(iSCType);
			BITCLR(onthefly_using, 1 << ImageInfo->Lcdc_layer);
		}
	}
}

static unsigned int onthefly_using_lcd = 0x0;
void TCC_LCD_SET_OnTheFly(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo)
{
	
	VIOC_RDMA * pRDMABase;
	unsigned int iSCType;
	unsigned int nRDMB;
	
	if(ImageInfo->MVCframeView == 1)
	{
		return;
	}
	
	iSCType = VIOC_SC1;
	
	#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	ImageInfo->Lcdc_layer = 3;
	#else		
	ImageInfo->Lcdc_layer = 2;
	#endif
	
	if(hdmi_lcdc)
	{
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
				nRDMB = VIOC_WMIX_RDMA_04;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
				nRDMB = VIOC_WMIX_RDMA_05;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
				nRDMB = VIOC_WMIX_RDMA_06;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
				nRDMB = VIOC_WMIX_RDMA_07;
				break;
					
		}
	}
	else
	{
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);
				nRDMB = VIOC_WMIX_RDMA_00;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
				nRDMB = VIOC_WMIX_RDMA_01;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);
				nRDMB = VIOC_WMIX_RDMA_02;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);
				nRDMB = VIOC_WMIX_RDMA_03;
				break;						
		}		
	}
		
	if(ImageInfo->on_the_fly)
	{
		if(!onthefly_using_lcd)
		{ 
			VIOC_RDMA_SetImageDisable(pRDMABase);
			printk(" %s  scaler[%d] is plug in RDMA %d \n",__func__, iSCType,nRDMB);
			BITSET(onthefly_using_lcd, 1 << ImageInfo->Lcdc_layer);
			VIOC_CONFIG_PlugIn (iSCType, nRDMB);	
		}
	}
	else
	{
		if(ISSET(onthefly_using_lcd, 1<<ImageInfo->Lcdc_layer))
		{
			printk(" %s  scaler[%d] is plug out	\n",__func__,iSCType);
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(iSCType);
			BITCLR(onthefly_using_lcd, 1 << ImageInfo->Lcdc_layer);
		}
	}
}

void TCC_LCD_DISPLAY_UPDATE(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_DISP * pDISPBase;
	VIOC_WMIX * pWMIXBase;
	VIOC_RDMA * pRDMABase;	
	unsigned int nRDMB;
	unsigned int lcd_width = 0, lcd_height = 0; // lcd_h_pos = 0, lcd_w_pos = 0, scale_x = 0, scale_y = 0;
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	VIOC_RDMA * pRDMABase_temp;
#endif	
	VIOC_SC *pSC;
	unsigned int iSCType;
	pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC1);
	iSCType = VIOC_SC1;		

	dprintk("%s enable:%d, layer:%d, addr:0x%x, ts:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", __func__, ImageInfo->enable, ImageInfo->Lcdc_layer, ImageInfo->addr0, ImageInfo->time_stamp,
			ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);
	
	if((ImageInfo->Lcdc_layer >= 4) || (ImageInfo->fmt >TCC_LCDC_IMG_FMT_MAX)){
		printk("LCD :: hdmi_lcdc:%d, enable:%d, layer:%d, addr:0x%x, ts:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", hdmi_lcdc, ImageInfo->enable, ImageInfo->Lcdc_layer, ImageInfo->addr0, ImageInfo->time_stamp,
				ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);
		return;
	}

	#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	ImageInfo->Lcdc_layer = 3;
	#else		
	ImageInfo->Lcdc_layer = 2;
	#endif

	if(hdmi_lcdc)
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		

		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
				nRDMB = VIOC_WMIX_RDMA_04;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
				nRDMB = VIOC_WMIX_RDMA_05;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
				nRDMB = VIOC_WMIX_RDMA_06;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
				nRDMB = VIOC_WMIX_RDMA_07;
				break;
					
		}

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		pLastFrame_RDMABase = pRDMABase_temp  = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
#endif
	}
	else
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);
				nRDMB = VIOC_WMIX_RDMA_00;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
				nRDMB = VIOC_WMIX_RDMA_01;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);
				nRDMB = VIOC_WMIX_RDMA_02;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);
				nRDMB = VIOC_WMIX_RDMA_03;
				break;						
		}

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		pLastFrame_RDMABase = pRDMABase_temp  = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
#endif		
	}

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	
	if((!lcd_width) || (!lcd_height)){
		printk("%s LCD :: Error :: lcd_width %d, lcd_height %d \n", __func__,lcd_width, lcd_height);
		return;
	}
	
	
	if(!ImageInfo->enable)	{
		volatile PVIOC_IREQ_CONFIG pIREQConfig;
		pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

		VIOC_RDMA_SetImageDisable(pRDMABase);	
		
		#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x1<<nRDMB)); // rdma reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x0<<nRDMB)); // rdma reset
		#else
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x1<<nRDMB)); // rdma reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x0<<nRDMB)); // rdma reset
		#endif

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		if(enabled_LastFrame){
			VIOC_RDMA_SetImageDisable(pRDMABase_temp);	
		}
#endif
		
		if(ImageInfo->MVCframeView != 1){
			if(ISSET(onthefly_using_lcd, 1<<ImageInfo->Lcdc_layer))
			{
				VIOC_CONFIG_PlugOut(iSCType);
				BITCLR(onthefly_using_lcd, 1 << ImageInfo->Lcdc_layer);

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+iSCType)), (0x1<<(28+iSCType))); // scaler reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+iSCType)), (0x0<<(28+iSCType))); // scaler reset

			}
		}
		return;
	}	

	dprintk("%s lcdc:%d, pRDMA:0x%08x, pWMIX:0x%08x, pDISP:0x%08x, addr0:0x%08x\n", __func__, hdmi_lcdc, pRDMABase, pWMIXBase, pDISPBase, ImageInfo->addr0);
	if((ImageInfo->MVCframeView != 1) && ImageInfo->on_the_fly)
	{		
		VIOC_SC_SetSrcSize(pSC, ImageInfo->Frame_width, ImageInfo->Frame_height);
		VIOC_SC_SetDstSize (pSC, ImageInfo->Image_width, ImageInfo->Image_height);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSC, ImageInfo->Image_width, ImageInfo->Image_height);			// set output size in scaer

		if(((ImageInfo->crop_right - ImageInfo->crop_left) == ImageInfo->Image_width) && ((ImageInfo->crop_bottom - ImageInfo->crop_top) == ImageInfo->Image_height)){
			VIOC_SC_SetBypass (pSC, ON);
			dprintk(" %s  scaler 1 is plug in SetBypass ON \n",__func__);
		}else {
			VIOC_SC_SetBypass (pSC, OFF);
			dprintk(" %s  scaler 1 is plug in SetBypass OFF \n",__func__);
		}

		if(!onthefly_using_lcd)
		{ 
			VIOC_RDMA_SetImageDisable(pRDMABase);
			dprintk(" %s  scaler 1 is plug in RDMA %d \n",__func__, nRDMB);
			BITSET(onthefly_using_lcd, 1 << ImageInfo->Lcdc_layer);
			VIOC_CONFIG_PlugIn (iSCType, nRDMB);	
		}
	}
	else
	{
		if(ISSET(onthefly_using_lcd, 1<<ImageInfo->Lcdc_layer))
		{
			dprintk(" %s  scaler 1 is plug out \n",__func__);
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(iSCType);
			BITCLR(onthefly_using_lcd, 1 << ImageInfo->Lcdc_layer);
		}
	}

	if(onthefly_using_lcd)
		VIOC_SC_SetUpdate (pSC);
		
	// position
	VIOC_WMIX_SetPosition(pWMIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x, ImageInfo->offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase);	
		
	if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase, 0); /* Y2RMode Default 0 (Studio Color) */

		if( ImageInfo->Frame_width <= RDMA_UVI_MAX_WIDTH )
			VIOC_RDMA_SetImageUVIEnable(pRDMABase, TRUE);
		else
			VIOC_RDMA_SetImageUVIEnable(pRDMABase, FALSE);
	}

	if(ImageInfo->one_field_only_interlace)
		VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width*2);
	else
		VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width);

	VIOC_RDMA_SetImageFormat(pRDMABase, ImageInfo->fmt);

	// scale
	//VIOC_RDMA_SetImageScale(pRDMABase, scale_x, scale_y);

	if( !( ((ImageInfo->crop_left == 0) && (ImageInfo->crop_right == ImageInfo->Frame_width)) &&  ((ImageInfo->crop_top == 0) && (ImageInfo->crop_bottom == ImageInfo->Frame_height)))  )
	{

		int addr_Y = (unsigned int)ImageInfo->addr0;
		int addr_U = (unsigned int)ImageInfo->addr1;
		int addr_V = (unsigned int)ImageInfo->addr2;

		dprintk(" Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d] \n", ImageInfo->crop_left, ImageInfo->crop_right, ImageInfo->crop_top, ImageInfo->crop_bottom);
        
		tccxxx_GetAddress(ImageInfo->fmt, (unsigned int)ImageInfo->addr0, ImageInfo->Frame_width, ImageInfo->Frame_height, 		
								ImageInfo->crop_left, ImageInfo->crop_top, &addr_Y, &addr_U, &addr_V);		

		if(ImageInfo->one_field_only_interlace)
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->crop_right - ImageInfo->crop_left, (ImageInfo->crop_bottom - ImageInfo->crop_top)/2);
		else
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->crop_right - ImageInfo->crop_left, ImageInfo->crop_bottom - ImageInfo->crop_top);

		VIOC_RDMA_SetImageBase(pRDMABase, addr_Y, addr_U, addr_V);
	}
	else
	{	
		dprintk(" don't Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d] \n", ImageInfo->crop_left, ImageInfo->crop_right, ImageInfo->crop_top, ImageInfo->crop_bottom);
		if(ImageInfo->one_field_only_interlace)
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height/2);
		else
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height);

		VIOC_RDMA_SetImageBase(pRDMABase, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);		
	}
		
	VIOC_RDMA_SetImageIntl(pRDMABase, 0);

	VIOC_WMIX_SetUpdate(pWMIXBase);	
	
	VIOC_RDMA_SetImageEnable(pRDMABase);

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	if(enabled_LastFrame)
	{
		VIOC_RDMA_SetImageDisable(pRDMABase_temp);
		if(onthefly_LastFrame)
		{
			onthefly_LastFrame = 0;
			VIOC_CONFIG_PlugOut(VIOC_SC0); //printk("\n\n\n\n");
		}
	}
#endif
}

void TCC_HDMI_DISPLAY_UPDATE(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_DISP * pDISPBase;
	VIOC_WMIX * pWMIXBase;
	VIOC_RDMA * pRDMABase;
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	VIOC_RDMA * pRDMABase_temp;
#endif		
	unsigned int nRDMB;
	unsigned int lcd_width = 0, lcd_height = 0, interlace_output = 0; // lcd_h_pos = 0, lcd_w_pos = 0, scale_x = 0, scale_y = 0, 
	
	VIOC_SC *pSC;
	unsigned int iSCType;

#if defined(CONFIG_ARCH_TCC893X)
	#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC1);
		iSCType = VIOC_SC1;
	#else
		pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC3);
		iSCType = VIOC_SC3;
	#endif
#else
	pSC = (VIOC_SC *)tcc_p2v(HwVIOC_SC1);
	iSCType = VIOC_SC1;
#endif
		

	dprintk("%s enable:%d, layer:%d, addr:0x%x, ts:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", __func__, ImageInfo->enable, ImageInfo->Lcdc_layer, ImageInfo->addr0, ImageInfo->time_stamp,
			ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);
	
	if((ImageInfo->Lcdc_layer >= 4) || (ImageInfo->fmt >TCC_LCDC_IMG_FMT_MAX)){
		printk("hdmi_lcdc:%d, enable:%d, layer:%d, addr:0x%x, ts:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", hdmi_lcdc, ImageInfo->enable, ImageInfo->Lcdc_layer, ImageInfo->addr0, ImageInfo->time_stamp,
				ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);
		return;
	}

	#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
	ImageInfo->Lcdc_layer = 3;
	#else		
	ImageInfo->Lcdc_layer = 2;
	#endif

	if(hdmi_lcdc)
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		

		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA04);
				nRDMB = VIOC_WMIX_RDMA_04;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
				nRDMB = VIOC_WMIX_RDMA_05;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
				nRDMB = VIOC_WMIX_RDMA_06;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
				nRDMB = VIOC_WMIX_RDMA_07;
				break;
					
		}
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		pLastFrame_RDMABase = pRDMABase_temp  = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);
#endif		
	}
	else
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		
		switch(ImageInfo->Lcdc_layer)
		{			
			case 0:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA00);
				nRDMB = VIOC_WMIX_RDMA_00;
				break;
			case 1:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
				nRDMB = VIOC_WMIX_RDMA_01;
				break;
			case 2:
				pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);
				nRDMB = VIOC_WMIX_RDMA_02;
				break;
			case 3:
				pRDMABase = (VIOC_RDMA *)tcc_p2v(HwVIOC_RDMA03);
				nRDMB = VIOC_WMIX_RDMA_03;
				break;						
		}		
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		pLastFrame_RDMABase = pRDMABase_temp  = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
#endif		
	}
	
	if(!ImageInfo->enable)	{
		volatile PVIOC_IREQ_CONFIG pIREQConfig;
		pIREQConfig = (volatile PVIOC_IREQ_CONFIG)tcc_p2v((unsigned int)HwVIOC_IREQ);

		#if defined(CONFIG_TCC_OUTPUT_ATTACH)
			//TCC_OUTPUT_FB_RDMA_OnOff(1, TCC_OUTPUT_HDMI);
		#endif

		VIOC_RDMA_SetImageDisable(pRDMABase);	
		
		#if defined(CONFIG_CHIP_TCC8925S) || defined(CONFIG_ARCH_TCC893X)
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x1<<nRDMB)); // rdma reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x0<<nRDMB)); // rdma reset
		#else
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x1<<nRDMB)); // rdma reset
		BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<nRDMB), (0x0<<nRDMB)); // rdma reset
		#endif
		
#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
		if(enabled_LastFrame){
			VIOC_RDMA_SetImageDisable(pRDMABase_temp);	
		}
#endif	

		if(ImageInfo->MVCframeView != 1){
			if(ISSET(onthefly_using, 1<<ImageInfo->Lcdc_layer))
			{
				VIOC_CONFIG_PlugOut(iSCType);
				BITCLR(onthefly_using, 1 << ImageInfo->Lcdc_layer);

				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+iSCType)), (0x1<<(28+iSCType))); // scaler reset
				BITCSET(pIREQConfig->uSOFTRESET.nREG[0], (0x1<<(28+iSCType)), (0x0<<(28+iSCType))); // scaler reset
			}
		}

		return;
	}	

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	
	if((!lcd_width) || (!lcd_height)){
		printk(" %s Error :: lcd_width %d, lcd_height %d, hdmi_lcdc=%d, enable=%d\n", __func__,lcd_width, lcd_height, hdmi_lcdc, ImageInfo->enable);
		return;
	}

	dprintk("%s lcdc:%d, pRDMA:0x%08x, pWMIX:0x%08x, pDISP:0x%08x, addr0:0x%08x\n", __func__, hdmi_lcdc, pRDMABase, pWMIXBase, pDISPBase, ImageInfo->addr0);

#if defined(CONFIG_TCC_M2M_USE_INTERLACE_OUTPUT)
	if( !(pDISPBase->uCTRL.nREG & HwDISP_NI )) {//interlace mode
		interlace_output = 1;
	}
#endif

	if((ImageInfo->MVCframeView != 1) && ImageInfo->on_the_fly)
	{		
		VIOC_SC_SetSrcSize(pSC, ImageInfo->Frame_width, ImageInfo->Frame_height);
		VIOC_SC_SetDstSize (pSC, ImageInfo->Image_width, ImageInfo->Image_height);			// set destination size in scaler
		VIOC_SC_SetOutSize (pSC, ImageInfo->Image_width, ImageInfo->Image_height);			// set output size in scaer

		if(((ImageInfo->crop_right - ImageInfo->crop_left) == ImageInfo->Image_width) && ((ImageInfo->crop_bottom - ImageInfo->crop_top) == ImageInfo->Image_height)){
			VIOC_SC_SetBypass (pSC, ON);
			dprintk(" %s  scaler 1 is plug in SetBypass ON \n",__func__);
		}else {
			VIOC_SC_SetBypass (pSC, OFF);
			dprintk(" %s  scaler 1 is plug in SetBypass OFF \n",__func__);
		}

		if(!onthefly_using)
		{ 
			VIOC_RDMA_SetImageDisable(pRDMABase);
			dprintk(" %s  scaler 1 is plug in RDMA %d \n",__func__, nRDMB);
			BITSET(onthefly_using, 1 << ImageInfo->Lcdc_layer);
			VIOC_CONFIG_PlugIn (iSCType, nRDMB);	
		}
	}
	else
	{
		if(ISSET(onthefly_using, 1<<ImageInfo->Lcdc_layer))
		{
			dprintk(" %s  scaler 1 is plug out \n",__func__);
			VIOC_RDMA_SetImageDisable(pRDMABase);
			VIOC_CONFIG_PlugOut(iSCType);
			BITCLR(onthefly_using, 1 << ImageInfo->Lcdc_layer);
		}
	}

	if(onthefly_using)
		VIOC_SC_SetUpdate (pSC);
		
	// position
	VIOC_WMIX_SetPosition(pWMIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x, ImageInfo->offset_y);
	VIOC_WMIX_SetUpdate(pWMIXBase);	
		
	#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
		if(hdmi_get_hdmimode() == HDMI)
			VIOC_RDMA_SetImageY2REnable(pRDMABase, FALSE);
		else
		{
			if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
			{
				VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
				VIOC_RDMA_SetImageY2RMode(pRDMABase, 0); /* Y2RMode Default 0 (Studio Color) */

				if( ImageInfo->Frame_width <= RDMA_UVI_MAX_WIDTH )
					VIOC_RDMA_SetImageUVIEnable(pRDMABase, TRUE);
				else
					VIOC_RDMA_SetImageUVIEnable(pRDMABase, FALSE);
			}
		}
	#else
		if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
		{
			VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
			VIOC_RDMA_SetImageY2RMode(pRDMABase, 0); /* Y2RMode Default 0 (Studio Color) */

			if( ImageInfo->Frame_width <= RDMA_UVI_MAX_WIDTH )
				VIOC_RDMA_SetImageUVIEnable(pRDMABase, TRUE);
			else
				VIOC_RDMA_SetImageUVIEnable(pRDMABase, FALSE);
		}
	#endif

	if(ImageInfo->one_field_only_interlace)
		VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width*2);
	else
		VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width);

	VIOC_RDMA_SetImageFormat(pRDMABase, ImageInfo->fmt);

	// scale
	//VIOC_RDMA_SetImageScale(pRDMABase, scale_x, scale_y);

	if( !( ((ImageInfo->crop_left == 0) && (ImageInfo->crop_right == ImageInfo->Frame_width)) &&  ((ImageInfo->crop_top == 0) && (ImageInfo->crop_bottom == ImageInfo->Frame_height)))  )
	{

		int addr_Y = (unsigned int)ImageInfo->addr0;
		int addr_U = (unsigned int)ImageInfo->addr1;
		int addr_V = (unsigned int)ImageInfo->addr2;

		dprintk(" Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d] \n", ImageInfo->crop_left, ImageInfo->crop_right, ImageInfo->crop_top, ImageInfo->crop_bottom);
        
		tccxxx_GetAddress(ImageInfo->fmt, (unsigned int)ImageInfo->addr0, ImageInfo->Frame_width, ImageInfo->Frame_height, 		
								ImageInfo->crop_left, ImageInfo->crop_top, &addr_Y, &addr_U, &addr_V);		

		if(ImageInfo->one_field_only_interlace)
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->crop_right - ImageInfo->crop_left, (ImageInfo->crop_bottom - ImageInfo->crop_top)/2);
		else
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->crop_right - ImageInfo->crop_left, ImageInfo->crop_bottom - ImageInfo->crop_top);

		VIOC_RDMA_SetImageBase(pRDMABase, addr_Y, addr_U, addr_V);
	}
	else
	{	
		dprintk(" don't Image Crop left=[%d], right=[%d], top=[%d], bottom=[%d] \n", ImageInfo->crop_left, ImageInfo->crop_right, ImageInfo->crop_top, ImageInfo->crop_bottom);
		if(ImageInfo->one_field_only_interlace)
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height/2);
		else
			VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height);

		VIOC_RDMA_SetImageBase(pRDMABase, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);		
	}
		
	VIOC_RDMA_SetImageIntl(pRDMABase, interlace_output);

	VIOC_WMIX_SetUpdate(pWMIXBase);	
	
	VIOC_RDMA_SetImageEnable(pRDMABase);

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
	if(enabled_LastFrame)
	{
		VIOC_RDMA_SetImageDisable(pRDMABase_temp);
		if(onthefly_LastFrame)
		{
			onthefly_LastFrame = 0;
			VIOC_CONFIG_PlugOut(VIOC_SC0); //printk("\n\n\n\n");
		}
	}
#endif

}

#ifdef CONFIG_HDMI_DISPLAY_LASTFRAME
int TCC_HDMI_LAST_FRAME_UPDATE(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_DISP * pDISPBase;
	VIOC_WMIX * pWMIXBase;
	VIOC_RDMA * pRDMABase;
	unsigned int lcd_width = 0, lcd_height = 0;

	if((ImageInfo->Lcdc_layer >= 4) || (ImageInfo->fmt >TCC_LCDC_IMG_FMT_MAX)){
		printk("LCD :: hdmi_lcdc:%d, enable:%d, layer:%d, addr:0x%x, ts:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", hdmi_lcdc, ImageInfo->enable, ImageInfo->Lcdc_layer, ImageInfo->addr0, ImageInfo->time_stamp,
				ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);
		return -1;
	}

	if(hdmi_lcdc)
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		
		pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA05);		
	}
	else
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		pRDMABase = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA01);
	}		
	
	ImageInfo->Lcdc_layer = 1;

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);		
	if((!lcd_width) || (!lcd_height))
		return -1;

	// position
	VIOC_WMIX_SetPosition(pWMIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x, ImageInfo->offset_y);

	if(machine_is_tcc8920st() || machine_is_tcc8930st()) {
		VIOC_RDMA_SetImageUVIEnable(pRDMABase, TRUE);
	}
		
	if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
	{
		#if defined(CONFIG_TCC_OUTPUT_COLOR_SPACE_YUV)
			if(Output_SelectMode != TCC_OUTPUT_HDMI && Output_SelectMode != TCC_OUTPUT_COMPOSITE && Output_SelectMode != TCC_OUTPUT_COMPONENT)
		#elif defined(CONFIG_TCC_COMPOSITE_COLOR_SPACE_YUV)
			if(Output_SelectMode != TCC_OUTPUT_COMPOSITE)
		#endif
			{
				VIOC_RDMA_SetImageY2REnable(pRDMABase, TRUE);
				VIOC_RDMA_SetImageY2RMode(pRDMABase, 0); /* Y2RMode Default 0 (Studio Color) */
			}
	}
	
	VIOC_RDMA_SetImageOffset(pRDMABase, ImageInfo->fmt, ImageInfo->Frame_width);
	VIOC_RDMA_SetImageFormat(pRDMABase, ImageInfo->fmt);

	// scale
	//VIOC_RDMA_SetImageScale(pRDMABase, scale_x, scale_y);

	VIOC_RDMA_SetImageSize(pRDMABase, ImageInfo->Frame_width, ImageInfo->Frame_height);
	VIOC_RDMA_SetImageBase(pRDMABase, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);		
		
	VIOC_RDMA_SetImageIntl(pRDMABase, 0);
	VIOC_WMIX_SetUpdate(pWMIXBase);	
	
	VIOC_RDMA_SetImageEnable(pRDMABase);

	return 0;
}

void TCC_HDMI_PREV_FRAME_OFF(char hdmi_lcdc, int interlace_onthefly_mode)
{
	VIOC_RDMA * pRDMABase_temp;
	unsigned int old_lcdc_layer;
	unsigned int iSCType;

	#if defined(CONFIG_ARCH_TCC893X)
		#if defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S) || defined(CONFIG_MACH_TCC8930ST)
		iSCType = VIOC_SC1;
		#else
		iSCType = VIOC_SC3;
		#endif
		old_lcdc_layer = 3;
	#else
		iSCType = VIOC_SC1;
		old_lcdc_layer = 2;
	#endif /* CONFIG_ARCH_TCC893X */

	if(hdmi_lcdc)
	{
		switch(old_lcdc_layer)
		{
			case 2 :
				pRDMABase_temp = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);		
				break;
			case 3 :
				pRDMABase_temp = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
				break;
		}
				
	}
	else
	{	
		switch(old_lcdc_layer)
		{
			case 2 :
				pRDMABase_temp = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);		
				break;
			case 3 :
				pRDMABase_temp = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA03);
				break;
		}
	}

	if(!interlace_onthefly_mode)
	{
		VIOC_RDMA_SetImageDisable(pRDMABase_temp);			

#ifdef CONFIG_TCC_LCD_VIDEO_DISPLAY_BY_VSYNC_INT
		if(Output_SelectMode == TCC_OUTPUT_LCD){
			iSCType = VIOC_SC1;
			if(ISSET(onthefly_using_lcd, 1<<old_lcdc_layer))
			{
				VIOC_CONFIG_PlugOut(iSCType);
				BITCLR(onthefly_using_lcd, 1 << old_lcdc_layer);
			}
		}
		else 
#endif
		if(Output_SelectMode == TCC_OUTPUT_HDMI){
			if(ISSET(onthefly_using, 1<<old_lcdc_layer))
			{
				VIOC_CONFIG_PlugOut(iSCType);
				BITCLR(onthefly_using, 1 << old_lcdc_layer);
			}
		}
		#if defined(CONFIG_FB_TCC_COMPOSITE)
		else if(Output_SelectMode== TCC_OUTPUT_COMPOSITE){
			tcc_plugout_for_composite(old_lcdc_layer);
		}
		#endif
		#if defined(CONFIG_FB_TCC_COMPONENT)
		else if(Output_SelectMode == TCC_OUTPUT_COMPONENT){
			tcc_plugout_for_component(old_lcdc_layer);
		}
		#endif
	}
}
#endif

void TCC_HDMI_DISPLAY_UPDATE_3D(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_DISP * pDISPBase;
	VIOC_WMIX * pWMIXBase;
	VIOC_RDMA * pRDMABase0;
	VIOC_RDMA * pRDMABase1;

	unsigned int lcd_width = 0, lcd_height = 0, iVBlank = 0;

	dprintk("%s enable:%d, layer:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", __func__, ImageInfo->enable, ImageInfo->Lcdc_layer,
			ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);


	if(hdmi_lcdc)
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP1);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX1);		
		pRDMABase0 = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA06);
		pRDMABase1 = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA07);
	}
	else
	{
		pDISPBase = (VIOC_DISP*)tcc_p2v(HwVIOC_DISP0);
		pWMIXBase = (VIOC_WMIX*)tcc_p2v(HwVIOC_WMIX0);
		pRDMABase0 = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA02);
		pRDMABase1 = (VIOC_RDMA*)tcc_p2v(HwVIOC_RDMA03);
	}

	VIOC_DISP_GetSize(pDISPBase, &lcd_width, &lcd_height);
	
	if((!lcd_width) || (!lcd_height))
	{
		printk("%s - lcd width and hight is not normal!!!!\n", __func__);
		return;
	}

#if defined(MVC_PROCESS)
	iVBlank = hdmi_get_VBlank();
#endif//

	if(!ImageInfo->enable)	{
		VIOC_RDMA_SetImageDisable(pRDMABase0);
		VIOC_RDMA_SetImageDisable(pRDMABase1);
		printk("%s - Image Info is not enable, so RDAMA is disable.\n", __func__);
		return;
	}

	dprintk("%s lcdc:%d,pRDMA0:0x%08x pRDMA1:0x%08x, pWMIX:0x%08x, pDISP:0x%08x, addr0:0x%08x\n", __func__, hdmi_lcdc, pRDMABase0, pRDMABase1, pWMIXBase, pDISPBase, ImageInfo->addr0);
	dprintk("%s enable:%d, layer:%d, fmt:%d, Fw:%d, Fh:%d, Iw:%d, Ih:%d, fmt:%d onthefly:%d\n", __func__, ImageInfo->enable, ImageInfo->Lcdc_layer,
			ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->fmt, ImageInfo->on_the_fly);

	if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase0, TRUE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase0, 0); /* Y2RMode Default 0 (Studio Color) */

		if( ImageInfo->Frame_width <= RDMA_UVI_MAX_WIDTH )
			VIOC_RDMA_SetImageUVIEnable(pRDMABase0, TRUE);
		else
			VIOC_RDMA_SetImageUVIEnable(pRDMABase0, FALSE);
	}

	VIOC_RDMA_SetImageOffset(pRDMABase0, ImageInfo->fmt, ImageInfo->Frame_width);
	VIOC_RDMA_SetImageFormat(pRDMABase0, ImageInfo->fmt);

	VIOC_WMIX_SetPosition(pWMIXBase, 2, 0, 0);
	VIOC_WMIX_SetPosition(pWMIXBase, 3, 0, 0);

	VIOC_RDMA_SetImageSize(pRDMABase0, ImageInfo->Image_width, ImageInfo->Image_height);
	VIOC_RDMA_SetImageBase(pRDMABase0, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);		

	VIOC_RDMA_SetImageIntl(pRDMABase0, FALSE);

	if(ImageInfo->fmt >= TCC_LCDC_IMG_FMT_UYVY && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMABase1, TRUE);
		VIOC_RDMA_SetImageY2RMode(pRDMABase1, 0); /* Y2RMode Default 0 (Studio Color) */

		if( ImageInfo->Frame_width <= RDMA_UVI_MAX_WIDTH )
			VIOC_RDMA_SetImageUVIEnable(pRDMABase1, TRUE);
		else
			VIOC_RDMA_SetImageUVIEnable(pRDMABase1, FALSE);

	}

	VIOC_RDMA_SetImageOffset(pRDMABase1, ImageInfo->fmt, ImageInfo->Frame_width);
	VIOC_RDMA_SetImageFormat(pRDMABase1, ImageInfo->fmt);

	// position
	VIOC_WMIX_SetPosition(pWMIXBase, ImageInfo->Lcdc_layer, 0, ImageInfo->Frame_height + iVBlank);

	VIOC_RDMA_SetImageSize(pRDMABase1, ImageInfo->Image_width, ImageInfo->Image_height);
	VIOC_RDMA_SetImageBase(pRDMABase1, ImageInfo->MVC_Base_addr0, ImageInfo->MVC_Base_addr1, ImageInfo->MVC_Base_addr2);

	VIOC_RDMA_SetImageIntl(pRDMABase1, FALSE);

	VIOC_WMIX_SetBGColor(pWMIXBase, 0x00, 0x00, 0x00, 0x00);

	if( ImageInfo->enable ) {
		VIOC_RDMA_SetImageEnable(pRDMABase0);
		VIOC_RDMA_SetImageEnable(pRDMABase1);
	}

	if( ImageInfo->enable )
		VIOC_WMIX_SetUpdate(pWMIXBase);
}

#if defined(CONFIG_TCC_EXCLUSIVE_UI_LAYER)
void TCC_HDMI_DISPLAY_PROCESS(char hdmi_lcdc, struct tcc_lcdc_image_update *ImageInfo, char force_update)
{
	unsigned int regl;
	unsigned int lcd_width = 0, lcd_height = 0, lcd_h_pos = 0, lcd_w_pos = 0; // scale_x = 0, scale_y = 0;
	unsigned int img_width = 0, img_height = 0, output_width = 0, output_height = 0;
	unsigned int buffer_width = 0, y2r_option = 0, mixer_interrupt_use = 0;
	exclusive_ui_ar_params aspect_ratio;
	PLCDC pLCDC;
	PLCDC_CHANNEL pLCDC_channel;

	dprintk(" %s onoff:%d :fmt:%d Fw:%d Fh:%d Iw:%d Ih:%d \n", __func__,ImageInfo->enable,
		ImageInfo->fmt,ImageInfo->Frame_width, ImageInfo->Frame_height, ImageInfo->Image_width, ImageInfo->Image_height);

	if((ImageInfo->Lcdc_layer >= 3) || (ImageInfo->fmt >TCC_LCDC_IMG_FMT_MAX))
		return;

	if (hdmi_lcdc)	// lcdc1
		pLCDC = (volatile PLCDC)tcc_p2v(HwLCDC1_BASE);
	else
		pLCDC = (volatile PLCDC)tcc_p2v(HwLCDC0_BASE);

	regl = pLCDC->LDS; // get LCD size 

	lcd_width = (regl & 0xFFFF);
	lcd_height = ((regl>>16) & 0xFFFF);
	
	if((!lcd_width) || (!lcd_height))
		return;

	output_width = lcd_width;
	output_height = lcd_height;

	switch(ImageInfo->Lcdc_layer)
	{
		case 0:
			pLCDC_channel = (volatile PLCDC_CHANNEL)&pLCDC->LI0C;
			break;
		case 1:
			pLCDC_channel = (volatile PLCDC_CHANNEL)&pLCDC->LI1C;
			break;
		case 2:
			pLCDC_channel = (volatile PLCDC_CHANNEL)&pLCDC->LI2C;
			break;
	}
	
	img_width = ImageInfo->Image_width;
	img_height= ImageInfo->Image_height;

	/* Aspect Ratio Conversion for DVD Contents */
	//if((img_width <= HDMI_EXCLUSIVE_UI_DVD_MAX_WD) && (img_height <= HDMI_EXCLUSIVE_UI_DVD_MAX_HT))
	{
		memset((void *)&aspect_ratio, 0x00, sizeof(exclusive_ui_ar_params));
		TCC_OUTPUT_EXCLUSIVE_UI_CfgAR(hdmi_lcdc, img_width, img_height, &aspect_ratio);
	}

	if(!ImageInfo->enable)	{
		#if defined(CONFIG_ARCH_TCC92XX)
		pLCDC_channel->LIC = 0;
		#else
		pLCDC_channel->LIC= HwLCT_RU;
		#endif//
		return;
	}

	/* Get the parameters for exclusive ui */
	TCC_OUTPUT_EXCLUSIVE_UI_GetParam(&hdmi_exclusive_ui_param);

	if(hdmi_exclusive_ui_param.enable)
	{
		/* Clear the on_the_flay for video */
		hdmi_exclusive_ui_onthefly = FALSE;

		/* Clear update flag and backup an information of the last video */
		TCC_OUTPUT_EXCLUSIVE_UI_SetImage(ImageInfo);

		/*-------------------------------------*/
		/*           INTERLACE VIDEO           */
		/* Update video data with exclusive ui */
		/*-------------------------------------*/
		if(hdmi_exclusive_ui_param.interlace)
		{
			/* Interlace BD Contents with 1920x1080 graphic plane */
			if(!TCC_OUTPUT_EXCLUSIVE_UI_ViqeMtoM(hdmi_lcdc, img_width, img_height))
			{
				if((img_width == lcd_width) && (img_height == lcd_height))
				{
					/* 1920x1080 HDMI Output */
				}
				else
				{
					/* 1280x720 HDMI Output */
				}
			}
			else
			{
				/* Disable the lcdc channel_1 */
				TCC_OUTPUT_EXCLUSIVE_UI_Direct(hdmi_lcdc, FALSE);

				/* Check the output format */
				if(ISZERO(pLCDC->LCTRL, HwLCTRL_NI))
					lcd_height = lcd_height >> 1;
			
				if(hdmi_exclusive_ui_param.clear == TRUE)
				{
					/* M2M Scaler without On_the_fly */
					if(TCC_OUTPUT_EXCLUSIVE_UI_Scaler_OnTheFly(ImageInfo, hdmi_lcdc) < 0)
						hdmi_exclusive_ui_onthefly = FALSE;
					else
						hdmi_exclusive_ui_onthefly = TRUE;

					/* Update Screen */
					TCC_HDMI_DISPLAY_UPDATE(hdmi_lcdc, ImageInfo);
				}
				else
				{
					/* VIQE and M2M Scaler with On_the_fly */
					TCC_OUTPUT_EXCLUSIVE_UI_Scaler(ImageInfo, HDMI_EXCLUSIVE_UI_SCALER1, hdmi_exclusive_ui_param.plane_width, hdmi_exclusive_ui_param.plane_height, hdmi_exclusive_ui_idx, 0);

					if(TCC_OUTPUT_EXCLUSIVE_UI_GetUpdate())
					{
						dprintk("E\n");
					}
					else
					{
						/* Overlay Mixer */
						TCC_OUTPUT_EXCLUSIVE_UI_Mixer(ImageInfo, hdmi_exclusive_ui_idx, 0);
						/* Update overlay mixer image */
						TCC_OUTPUT_EXCLUSIVE_UI_Restore_Mixer(ImageInfo, hdmi_lcdc, output_width, output_height, hdmi_exclusive_ui_idx);
						/* Set the interrupt flag */
						mixer_interrupt_use = 1;
	 				}
				}
			}
		}
		else
		/*-------------------------------------*/
		/*          PROGRESSIVE VIDEO          */
		/* Update video data with exclusive ui */
		/*-------------------------------------*/
		{
			/* 1920x1080 Progressive BD Contents */
			if( (img_width == hdmi_exclusive_ui_param.plane_width) && (img_width == lcd_width) && (img_width == TCC_FB_OUT_MAX_WIDTH) &&
				(img_height == hdmi_exclusive_ui_param.plane_height) && (img_height == lcd_height) && (img_height == TCC_FB_OUT_MAX_HEIGHT) )
			{
				/* 1920x1080 HDMI Output */
			}
			else
			{
				/* Disable the lcdc channel_1 */
				TCC_OUTPUT_EXCLUSIVE_UI_Direct(hdmi_lcdc, FALSE);

				if(hdmi_exclusive_ui_param.clear == FALSE)
				{
					/* M2M Scaler without On_the_fly */
					TCC_OUTPUT_EXCLUSIVE_UI_Scaler(ImageInfo, HDMI_EXCLUSIVE_UI_SCALER0, hdmi_exclusive_ui_param.plane_width, hdmi_exclusive_ui_param.plane_height, hdmi_exclusive_ui_idx, 0);
					/* Overlay Mixer */
					TCC_OUTPUT_EXCLUSIVE_UI_Mixer(ImageInfo, hdmi_exclusive_ui_idx, 0);
				}

			#if defined(CONFIG_TCC_VIDEO_DISPLAY_BY_VSYNC_INT)
				/* M2M Scaler without On_the_fly */
				TCC_OUTPUT_EXCLUSIVE_UI_Scaler(ImageInfo, HDMI_EXCLUSIVE_UI_SCALER0, output_width, output_height, hdmi_exclusive_ui_idx, 1);
				//hdmi_exclusive_ui_onthefly = FALSE;
			#else
				/* M2M Scaler with On_the_fly */
				if(TCC_OUTPUT_EXCLUSIVE_UI_Scaler_OnTheFly(ImageInfo, hdmi_lcdc) < 0)
					hdmi_exclusive_ui_onthefly = FALSE;
				else
					hdmi_exclusive_ui_onthefly = TRUE;
			#endif
			}

			#if 0 //shkim - on_the_fly
			/* Set the on_the_flay for android UI */
			if(hdmi_exclusive_ui_onthefly == FALSE)
				TCC_OUTPUT_EXCLUSIVE_UI_SetOnthefly(hdmi_lcdc, 1, TRUE);
			else
				TCC_OUTPUT_EXCLUSIVE_UI_SetOnthefly(hdmi_lcdc, 1, FALSE);
			#endif
		}

		hdmi_exclusive_ui_idx = !hdmi_exclusive_ui_idx;
	}

	if(force_update && !mixer_interrupt_use)
	{
		TCC_HDMI_DISPLAY_UPDATE(hdmi_lcdc, ImageInfo);
	}
}

void TCC_HDMI_DISPLAY_IMAGE(exclusive_ui_update UpdateImage)
{
	/* M2M Scaler with On_the_fly */
	if(TCC_OUTPUT_EXCLUSIVE_UI_Scaler_OnTheFly(&UpdateImage.image, UpdateImage.lcdc) < 0)
		hdmi_exclusive_ui_onthefly = FALSE;
	else
		hdmi_exclusive_ui_onthefly = TRUE;

	dprintk("%s, hdmi_exclusive_ui_onthefly=%d\n", __func__, hdmi_exclusive_ui_onthefly);
			
	/* Update Screen */
	TCC_HDMI_DISPLAY_UPDATE(UpdateImage.lcdc, &UpdateImage.image);
}
#endif

