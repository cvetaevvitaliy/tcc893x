/****************************************************************************
 *   FileName    : lcd_ht121wx2.c
 *   Description : support for HT121WX2 LVDS Panel
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
#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <tcc_lcd.h>


#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <reg.h>
#include <platform/iomap.h>
#include <dev/fbcon.h>
#include <platform/gpio.h>
#include <platform/reg_physical.h>

#ifdef HT121WX2
#include <platform/globals.h>
#include <tnftl/IO_TCCXXX.h>

#define     LVDS_VCO_45MHz        450000
#define     LVDS_VCO_60MHz        600000

extern void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable);

static int ht121wx2_panel_init(struct lcd_panel *panel)
{
	//struct lcd_platform_data *pdata = &(panel->dev);

	printf("!  %s : %d\n", __func__, 0);

	return 0;
}
static int ht121wx2_set_power(struct lcd_panel *panel, int on)
{

	unsigned int P, M, S, VSEL, TC;
	#ifdef TCC88XX
	PDDICONFIG		pDDICfg 	= (DDICONFIG *)HwDDI_CONFIG_BASE;
	#elif defined(TARGET_TCC892X_EVM) || defined(TARGET_TCC893X_EVM)
	PDDICONFIG		pDDICfg 	= (DDICONFIG *)HwDDI_CONFIG_BASE;
	#else
	PDDICONFIG		pDDICfg 	= (DDICONFIG *)&HwDDI_CONFIG_BASE;
	#endif//
	
	struct lcd_platform_data *pdata = &(panel->dev);
	printf("%s : %d \n", __func__, on);

	if (on) {
		tcclcd_gpio_set_value(pdata->power_on, 1);

		mdelay(10);
		lcdc_initialize(pdata->lcdc_num, panel);

		
	#if defined(TARGET_TCC892X_EVM) || defined(TARGET_TCC893X_EVM)

		// LVDS reset	
		pDDICfg->LVDS_CTRL.bREG.RST =1;
		pDDICfg->LVDS_CTRL.bREG.RST =0;		

		BITCSET(pDDICfg->LVDS_TXO_SEL0.nREG, 0xFFFFFFFF, 0x15141312);
		BITCSET(pDDICfg->LVDS_TXO_SEL1.nREG, 0xFFFFFFFF, 0x0B0A1716);
		BITCSET(pDDICfg->LVDS_TXO_SEL2.nREG, 0xFFFFFFFF, 0x0F0E0D0C);
		BITCSET(pDDICfg->LVDS_TXO_SEL3.nREG, 0xFFFFFFFF, 0x05040302);
		BITCSET(pDDICfg->LVDS_TXO_SEL4.nREG, 0xFFFFFFFF, 0x1A190706);
		BITCSET(pDDICfg->LVDS_TXO_SEL5.nREG, 0xFFFFFFFF, 0x1F1E1F18);
		BITCSET(pDDICfg->LVDS_TXO_SEL6.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL7.nREG, 0xFFFFFFFF, 0x1F1E1F1E);
		BITCSET(pDDICfg->LVDS_TXO_SEL8.nREG, 0xFFFFFFFF, 0x001E1F1E);

	
            #if defined(TARGET_TCC893X_EVM)
            if( panel->clk_freq >= LVDS_VCO_45MHz && panel->clk_freq < LVDS_VCO_60MHz)
            {
                   M = 7; P = 7; S = 0; VSEL = 0; TC = 4;
            }
            else
            {
              	M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
            }
            #else
            		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
            #endif
            
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1

		// LVDS Select LCDC 1
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x3 << 30 , 0x1 << 30);

	    	pDDICfg->LVDS_CTRL.bREG.RST = 1;	//  reset
	    		
	    	// LVDS enable
	  	pDDICfg->LVDS_CTRL.bREG.EN = 1;   // enable
	  	
	#else	
		// LVDS power on
		tca_ckc_setpmupwroff(PMU_LVDSPHY, ENABLE);

	    // LVDS reset
	    BITSET(pDDICfg->LVDS_CTRL, Hw1);    // reset
	    BITCLR(pDDICfg->LVDS_CTRL, Hw1);    // normal

		pDDICfg->LVDS_TXO_SEL0 = 0x15141312; // SEL_03, SEL_02, SEL_01, SEL_00,
		pDDICfg->LVDS_TXO_SEL1 = 0x0B0A1716; // SEL_07, SEL_06, SEL_05, SEL_04,
		pDDICfg->LVDS_TXO_SEL2 = 0x0F0E0D0C; // SEL_11, SEL_10, SEL_09, SEL_08,
		pDDICfg->LVDS_TXO_SEL3 = 0x05040302; // SEL_15, SEL_14, SEL_13, SEL_12,
		pDDICfg->LVDS_TXO_SEL4 = 0x1A190706; // SEL_19, SEL_18, SEL_17, SEL_16,
		pDDICfg->LVDS_TXO_SEL5 = 0x1F1E1F18; //                         SEL_20,
		pDDICfg->LVDS_TXO_SEL6 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL7 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL8 = 0x1F1E1F1E;
		
		// LVDS Select
	//	BITCLR(pDDICfg->LVDS_CTRL, Hw0); //LCDC0
		BITSET(pDDICfg->LVDS_CTRL, Hw0); //LCDC1

		#ifdef TCC88XX

		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1

		BITSET(pDDICfg->LVDS_CTRL, Hw1);    // reset
		#endif//
		
	    	// LVDS enable
	  	 BITSET(pDDICfg->LVDS_CTRL, Hw2);    // enable
	#endif
	
	}
	else 	{
		tcclcd_gpio_set_value(pdata->power_on, 0);
	}
	return 0;
}

static int ht121wx2_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, level);
	
	if (level == 0) {
		tcclcd_gpio_set_value(pdata->bl_on, 0);
	} else {
		tcclcd_gpio_set_value(pdata->bl_on, 1);
	}

	return 0;
}

static struct lcd_panel ht121wx2_panel = {
	.name		= "HT121WX2",
	.manufacturer	= "Hyundai-BOEhydis",
	.id		= PANEL_ID_HT121WX2,
	.xres		= 1280,
	.yres		= 800,
	.width		= 261,
	.height		= 163,
	.bpp		= 24,
	.clk_freq	= 711000,
	.clk_div	= 2,
	.bus_width	= 24,
	.lpw		= 31,
	.lpc		= 1280,
	.lswc		= 23,
	.lewc		= 160,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 22,
	.flc1		= 800,
	.fswc1		= 0,
	.fewc1		= 34,
	.fpw2		= 22,
	.flc2		= 800,
	.fswc2		= 0,
	.fewc2		= 34,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= ht121wx2_panel_init,
	.set_power	= ht121wx2_set_power,
	.set_backlight_level = ht121wx2_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &ht121wx2_panel;
}
#endif//HT121WX2
