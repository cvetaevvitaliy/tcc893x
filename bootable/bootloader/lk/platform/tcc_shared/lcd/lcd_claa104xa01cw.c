/****************************************************************************
 *   FileName    : lcd_claa104xa01cw.c
 *   Description : support for CLAA104XA01CW LVDS Panel
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

#ifdef CLAA104XA01CW
#include <platform/globals.h>
#include <tnftl/IO_TCCXXX.h>

extern void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable);

static int claa104xa01cw_panel_init(struct lcd_panel *panel)
{
	//struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, 0);

	return 0;
}
static int claa104xa01cw_set_power(struct lcd_panel *panel, int on)
{


	unsigned int P, M, S, VSEL, TC;
	#ifdef TCC88XX
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
	}
	else 	{
		tcclcd_gpio_set_value(pdata->power_on, 0);
	}
	return 0;
}

static int claa104xa01cw_set_backlight_level(struct lcd_panel *panel, int level)
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

static struct lcd_panel claa104xa01cw_panel = {
	.name		= "CLAA104XA01CW",
	.manufacturer	= "CPT",
	.id		= PANEL_ID_CLAA104XA01CW,
	.xres		= 1024,
	.yres		= 768,
	.width		= 211,
	.height		= 158,
	.bpp		= 24,
	.clk_freq	= 650000,
	.clk_div	= 2,
	.bus_width	= 24,
	.lpw		= 0,
	.lpc		= 1024,
	.lswc		= 0,
	.lewc		= 317,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 0,
	.flc1		= 768,
	.fswc1		= 0,
	.fewc1		= 35,
	.fpw2		= 0,
	.flc2		= 768,
	.fswc2		= 0,
	.fewc2		= 35,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= claa104xa01cw_panel_init,
	.set_power	= claa104xa01cw_set_power,
	.set_backlight_level = claa104xa01cw_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &claa104xa01cw_panel;
}
#endif//CLAA104XA01CW
