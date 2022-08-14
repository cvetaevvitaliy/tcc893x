
/****************************************************************************
 *   FileName    : TCC_LCD_interfface.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
/*****************************************************************************
*
* Header Files Include
*
******************************************************************************/
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <reg.h>
#include <platform/iomap.h>
#include <dev/fbcon.h>

#include <platform/reg_physical.h>
#include <lcd.h>
#include <platform/irqs.h>
#include <platform/globals.h>
#include <i2c.h>
#include <dev/gpio.h>
#include <platform/gpio.h>
#include <tcc_lcd.h>
#include "vioc/vioc_wmix.h"
#include "vioc/vioc_rdma.h"
#include "vioc/vioc_wdma.h"
#include "vioc/vioc_scaler.h"
#include "vioc/vioc_config.h"

volatile int lcd_nop_count = 0;
#define _ASM_NOP_ { lcd_nop_count++; }

void lcd_delay_us(unsigned int us)
{
	int i;
	while(us--)
	{
		for(i=0 ; i<20 ; i++)
			_ASM_NOP_ 
	}
}

void LCDC_IO_Set (char lcdctrl_num, unsigned bit_per_pixel)
{
	int i;

	if(lcdctrl_num)	{
		#if defined(CONFIG_CHIP_TCC8930)|| defined(CONFIG_CHIP_TCC8933)
			gpio_config(TCC_GPE(26), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW|GPIO_CD1);	// LPXCLK
		#else
			gpio_config(TCC_GPE(26), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);	// LPXCLK
		#endif
		gpio_config(TCC_GPE(0), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		gpio_config(TCC_GPE(1), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		gpio_config(TCC_GPE(27), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS
	}
	else {

		#if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
			gpio_config(TCC_GPB(0), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW|GPIO_CD1);		// LPXCLK
		#else
			gpio_config(TCC_GPB(0), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		// LPXCLK
		#endif
		gpio_config(TCC_GPB(1), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		gpio_config(TCC_GPB(2), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		gpio_config(TCC_GPB(19), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS

	}

	switch (bit_per_pixel) {
		case 24:
			for(i = 18 ; i < 24; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(4 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}

		case 18:
			for(i = 16 ; i < 18; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(4 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 16:
			for(i = 8 ; i < 16; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(3 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 8:
			for(i = 0 ; i < 8; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(3 + i), GPIO_FN1|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			break;
			
		default:
			// do nothing
			break;
	}

}

void LCDC_IO_Disable (char lcdctrl_num, unsigned bit_per_pixel)
{
	int i;

	if(lcdctrl_num)	{
		gpio_config(TCC_GPE(0), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		gpio_config(TCC_GPE(1), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		gpio_config(TCC_GPE(26), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);	// LPXCLK
		gpio_config(TCC_GPE(27), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS
	}
	else {
		gpio_config(TCC_GPB(0), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		// LPXCLK
		gpio_config(TCC_GPB(1), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LHSYNC
		gpio_config(TCC_GPB(2), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);		//LVSYNC
		gpio_config(TCC_GPB(19), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);	//LACBIAS
	}


	switch (bit_per_pixel) {
		case 24:
			for(i = 18 ; i < 24; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(4 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}

		case 18:
			for(i = 16 ; i < 18; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(4 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 16:
			for(i = 8 ; i < 16; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(3 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			
		case 8:
			for(i = 0 ; i < 8; i++)	{
				if(lcdctrl_num)	{
					gpio_config(TCC_GPE(2 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
				else {
					gpio_config(TCC_GPB(3 + i), GPIO_FN0|GPIO_OUTPUT|GPIO_LOW);
				}
			}
			break;
			
		default:
			// do nothing
			break;
	}

}

void lcdc_initialize(char lcdctrl_num, struct lcd_panel *lcd_spec)
{

	VIOC_DISP * pDISPBase;
	VIOC_WMIX* pWIXBase;
	
#if defined(TARGET_TCC893X_EVM)

#if 0

	pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;
	pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;
#else 	
	// just temp !!!
	if(lcdctrl_num) // Set the address for LCDC0 or LCDC1
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP1;
		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;
	}
	else
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;
		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX0;
	}
#endif

#else	

	if(lcdctrl_num) // Set the address for LCDC0 or LCDC1
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP1;
		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;
	}
	else
	{
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;
		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX0;
	}
	
#endif	

	VIOC_WMIX_SetSize(pWIXBase, lcd_spec->xres, lcd_spec->yres);	
	VIOC_WMIX_SetOverlayPriority(pWIXBase, 24);	
	VIOC_WMIX_SetBGColor(pWIXBase, 0, 0, 0, 0);	

	if(lcd_spec->bus_width == 24)
		//pDISPBase->uCTRL.bREG.PXDW = 0xC;
		BITCSET(pDISPBase->uCTRL.nREG, 0xF << 16 , 0xC << 16);
	else if(lcd_spec->bus_width == 18)
		//pDISPBase->uCTRL.bREG.PXDW = 0x5;
		BITCSET(pDISPBase->uCTRL.nREG, 0xF << 16 , 0x5 << 16) ;
	else
		//pDISPBase->uCTRL.bREG.PXDW = 0x3;
		BITCSET(pDISPBase->uCTRL.nREG, 0xF << 16 , 0x3 << 16);


	if(lcd_spec->sync_invert & ID_INVERT)
		//pDISPBase->uCTRL.bREG.ID = 1;
		BITCSET(pDISPBase->uCTRL.nREG, 1 << 15 , 1 << 15);


	if(lcd_spec->sync_invert & IV_INVERT)
		//pDISPBase->uCTRL.bREG.IV = 1;
		BITCSET(pDISPBase->uCTRL.nREG, 1 << 14 , 1 << 14);

	if(lcd_spec->sync_invert & IH_INVERT)
		//pDISPBase->uCTRL.bREG.IH = 1;
		BITCSET(pDISPBase->uCTRL.nREG, 1 << 13 , 1 << 13);

	if(lcd_spec->sync_invert & IP_INVERT)
		//pDISPBase->uCTRL.bREG.IP = 1;
		BITCSET(pDISPBase->uCTRL.nREG, 1 << 12 , 1 << 12);
	

	//pDISPBase->uCTRL.bREG.CKG = 0; // clock gating enable 
	BITCSET(pDISPBase->uCTRL.nREG, 1 << 23 , 0 << 23);

	BITCSET(pDISPBase->uCTRL.nREG, 1 << 22 , 0 << 22);

	
	//pDISPBase->uCTRL.bREG.NI = 1;
	BITCSET(pDISPBase->uCTRL.nREG, 1 << 8 , 1 << 8);

// Set LCD clock
	//pDISPBase->uCLKDIV.bREG.ACDIV= 1;
	//pDISPBase->uCLKDIV.bREG.PXCLKDIV = lcd_spec->clk_div/2;

	BITCSET(pDISPBase->uCLKDIV.nREG,0x00FF0000, 1  << 16 );
	BITCSET(pDISPBase->uCLKDIV.nREG,0x000000FF, lcd_spec->clk_div/2);


// Background color
	//pDISPBase->uBG.nREG = 0x00000000;
	BITCSET(pDISPBase->uBG.nREG,0xFFFFFFFF,0x00000000);

//	Horizontal timing
	//pDISPBase->uLHTIME1.bREG.LPC  = (lcd_spec->xres - 1);
	BITCSET(pDISPBase->uLHTIME1.nREG, 0x00003FFF, (lcd_spec->xres - 1) );

	//pDISPBase->uLHTIME1.bREG.LPW  = lcd_spec->lpw;
	BITCSET(pDISPBase->uLHTIME1.nREG, 0x01FF0000, lcd_spec->lpw<< 16 );
	
	//pDISPBase->uLHTIME2.bREG.LEWC = lcd_spec->lewc;
	BITCSET(pDISPBase->uLHTIME2.nREG, 0x01FF01FF, (lcd_spec->lswc << 16) | lcd_spec->lewc );


//	Vertical timing
	//pDISPBase->uLVTIME1.bREG.FLC = lcd_spec->yres;
	//pDISPBase->uLVTIME1.bREG.FPW = lcd_spec->fpw1;

	BITCSET(pDISPBase->uLVTIME1.nREG, 0x3F << 16 , lcd_spec->fpw1 << 16);	
	BITCSET(pDISPBase->uLVTIME1.nREG, 0x00003FFF, lcd_spec->yres -1 );
		
	
	//pDISPBase->uLVTIME2.bREG.FEWC = lcd_spec->fewc1;
	//pDISPBase->uLVTIME2.bREG.FSWC = lcd_spec->fswc1;

	BITCSET(pDISPBase->uLVTIME2.nREG, 0x01FF01FF, (lcd_spec->fswc1 << 16) | lcd_spec->fewc1 );

	
	//pDISPBase->uLVTIME3.bREG.FLC = lcd_spec->yres;
	//pDISPBase->uLVTIME3.bREG.FPW = lcd_spec->fpw2;
	
	BITCSET(pDISPBase->uLVTIME3.nREG, 0x3F << 16 , lcd_spec->fpw2 << 16);	
	BITCSET(pDISPBase->uLVTIME3.nREG, 0x00003FFF, lcd_spec->yres-1 );
	
	//pDISPBase->uLVTIME4.bREG.FEWC = lcd_spec->fewc2;
	//pDISPBase->uLVTIME4.bREG.FSWC = lcd_spec->fswc2;

	BITCSET(pDISPBase->uLVTIME4.nREG, 0x01FF01FF, (lcd_spec->fswc2 << 16) | lcd_spec->fewc2 );

	
	//pDISPBase->uLSIZE.bREG.HSIZE = lcd_spec->xres;
	//pDISPBase->uLSIZE.bREG.VSIZE = lcd_spec->yres;

	BITCSET(pDISPBase->uLSIZE.nREG, 0x1FFF1FFF, (lcd_spec->yres << 16) | lcd_spec->xres );

	//pDISPBase->uCTRL.bREG.LEN = 1;

	BITCSET(pDISPBase->uCTRL.nREG,1,1);

}


void tcc_lcdc_dithering_setting(char lcdc)
{

	VIOC_DISP * pDISPBase;

	if(lcdc) // Set the address for LCDC0 or LCDC1
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP1;
	else
		pDISPBase = (VIOC_DISP *)HwVIOC_DISP0;

	/* dithering option */
	BITCSET(pDISPBase->uCTRL.nREG, 0xF << 16 , 0x5 << 16) ;
	BITCSET(pDISPBase->uDITHCTRL.nREG, 0xFFFFFFFF, 0xC0000000) ;
	BITCSET(pDISPBase->uDMAT.nREG[0],  0xFFFFFFFF, 0x6e4ca280) ;
	BITCSET(pDISPBase->uDMAT.nREG[1],  0xFFFFFFFF, 0x5d7f91b3) ;

}


#if defined(TARGET_TCC893X_EVM)
#define	CFG_MISC		(HwVIOC_CONFIG+0x0084)	//0x7200A084

int tca_fb_output_path_init(void)
{
	int CFG_MISC_DISP_OUTPUT = (unsigned int)(CFG_MISC);

	#if defined(CONFIG_CHIP_TCC8930)
	
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD0_SEL, 1 << 24);	// DISP 1 - LCD, 
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD1_SEL, 0 << 26);	// DISP 0 - HDMI

	#elif defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD0_SEL, 0 << 24 );	// DISP 0 - LCD, 
	BITCSET(CFG_MISC_DISP_OUTPUT, LCD1_SEL, 1 << 26);	// DISP 1 - HDMI
	
	#endif	
}
#endif


void tcclcd_image_ch_set(char lcdctrl_num, struct tcc_lcdc_image_update *ImageInfo)
{
	VIOC_RDMA *pRDMA;
	VIOC_WMIX *pWIXBase;
	VIOC_SC *pSC;
	char scaler_num;
	char scaler_rdma;

	#if defined(TARGET_TCC893X_EVM)
		#if 0
		pRDMA = (VIOC_RDMA *)((unsigned int)HwVIOC_RDMA04 + ImageInfo->Lcdc_layer * 0x100);
		pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;

		tca_fb_output_path_init();	
		#else	
		//just temp !!!
		if(lcdctrl_num)	{
			pRDMA = (VIOC_RDMA *)((unsigned int)HwVIOC_RDMA04 + ImageInfo->Lcdc_layer * 0x100);
			pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;
			pSC = (VIOC_SC *)HwVIOC_SC0;
			scaler_num = 0;
			scaler_rdma = VIOC_SC_RDMA_04 + ImageInfo->Lcdc_layer;
		} else {
			pRDMA = (VIOC_RDMA *)((unsigned int)HwVIOC_RDMA00 + ImageInfo->Lcdc_layer * 0x100);
			pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX0;
			pSC = (VIOC_SC *)HwVIOC_SC2;
			scaler_num = 2;
			scaler_rdma = VIOC_SC_RDMA_00 + ImageInfo->Lcdc_layer;
		}
		#endif
	#else 
		if(lcdctrl_num)	{
			pRDMA = (VIOC_RDMA *)((unsigned int)HwVIOC_RDMA04 + ImageInfo->Lcdc_layer * 0x100);
			pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX1;
			pSC = (VIOC_SC *)HwVIOC_SC0;
			scaler_num = 0;
			scaler_rdma = VIOC_SC_RDMA_04 + ImageInfo->Lcdc_layer;
		} else {
			pRDMA = (VIOC_RDMA *)((unsigned int)HwVIOC_RDMA00 + ImageInfo->Lcdc_layer * 0x100);
			pWIXBase = (VIOC_WMIX*)HwVIOC_WMIX0;
			pSC = (VIOC_SC *)HwVIOC_SC2;
			scaler_num = 2;
			scaler_rdma = VIOC_SC_RDMA_00 + ImageInfo->Lcdc_layer;
		}
	#endif
	
	printf("%s lcdc:%d lcdc_ch:%d pRDMA:0x%08x\n", __func__, lcdctrl_num, ImageInfo->Lcdc_layer, pRDMA);
	printf("%s pos_x:%d pos_y:%d img_wd:%d img_ht:%d lcd_wd:%d lcd_ht:%d\n", __func__, ImageInfo->offset_x, ImageInfo->offset_x, ImageInfo->Image_width, ImageInfo->Image_height, ImageInfo->Frame_width, ImageInfo->Frame_height);

	#if defined(DISPLAY_SCALER_USE)
		/* set SCALER before plugging-in */
		VIOC_SC_SetBypass(pSC, OFF);
		VIOC_SC_SetSrcSize(pSC, ImageInfo->Image_width, ImageInfo->Image_height);
		VIOC_SC_SetDstSize(pSC, ImageInfo->Frame_width, ImageInfo->Frame_height);
		VIOC_SC_SetOutSize(pSC, ImageInfo->Frame_width, ImageInfo->Frame_height);
		VIOC_SC_SetUpdate(pSC);

		VIOC_CONFIG_PlugIn(scaler_num, scaler_rdma);
	#endif

	if ( ImageInfo->fmt >= TCC_LCDC_IMG_FMT_YUV420SP && ImageInfo->fmt <= TCC_LCDC_IMG_FMT_YUV422ITL1)
	{
		VIOC_RDMA_SetImageY2REnable(pRDMA, TRUE);
		VIOC_RDMA_SetImageY2RMode(pRDMA, 0); /* Y2RMode Default 0 (Studio Color) */
	}
	
	VIOC_RDMA_SetImageOffset(pRDMA, ImageInfo->fmt, ImageInfo->Image_width);
	VIOC_RDMA_SetImageFormat(pRDMA, ImageInfo->fmt);
	VIOC_RDMA_SetImageSize(pRDMA, ImageInfo->Image_width, ImageInfo->Image_height);
	
	// image address
	VIOC_RDMA_SetImageBase(pRDMA, ImageInfo->addr0, ImageInfo->addr1, ImageInfo->addr2);
	
	// image position
	VIOC_WMIX_SetPosition(pWIXBase, ImageInfo->Lcdc_layer, ImageInfo->offset_x, ImageInfo->offset_y);

	// image enable
	if( ImageInfo->enable)
		VIOC_RDMA_SetImageEnable(pRDMA);
	else
		VIOC_RDMA_SetImageDisable(pRDMA);

	VIOC_WMIX_SetUpdate(pWIXBase);

}

unsigned int DEV_LCDC_Wait_signal(char lcdc)
{
	// TO DO
	#define MAX_LCDC_WAIT_TIEM 		0x70000000

	unsigned loop = 1;
	VIOC_DISP * pDISPBase;
	VIOC_RDMA * pRDMABase;
	
	if(lcdc == 0 ){
		pDISPBase = (VIOC_DISP*)((unsigned int)(HwVIOC_DISP0));
		pRDMABase = (VIOC_RDMA*)((unsigned int)(HwVIOC_RDMA00));
	}
	else{
		pDISPBase = (VIOC_DISP*)((unsigned int)(HwVIOC_DISP1));
		pRDMABase = (VIOC_RDMA*)((unsigned int)(HwVIOC_RDMA04));
	}
	
	if ( ISZERO(pDISPBase->uCTRL.nREG, HwDISP_LEN) )
		return FALSE;

	// UI rdma channel check enable.
	if ( ISZERO(pRDMABase->uCTRL.nREG, HwDMA_IEN))
		return FALSE;

	BITSET(pRDMABase->uSTATUS.nREG, Hw4);

	while(TRUE && (loop < MAX_LCDC_WAIT_TIEM))
	{
		if(ISSET(pRDMABase->uSTATUS.nREG, Hw4))
			break;

		//printf("DEV_LCDC_Wait_signal cnt = %d \n",loop);
		loop++;
	}
	
 	return loop;
}


void tcclcd_gpio_config(unsigned n, unsigned flags)
{
	if(n == GPIO_NC)
	{
		// no connect
	}
	else if(n >= GPIO_EXT1)
	{
		// extend gpio 
	}
	else
	{
		gpio_config(n, flags);
	}
}
void tcclcd_gpio_set_value(unsigned n, unsigned on)
{
	if(n == GPIO_NC)
	{
		// no connect
	}
	else if(n >= GPIO_EXT1)
	{
		// extend gpio 
	}
	else
	{
		gpio_set(n, on);
	}

}
