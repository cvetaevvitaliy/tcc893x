/****************************************************************************
 *   FileName    : lcd_N101L6.c
 *   Description :  support for N101L6 Panel
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

#ifdef N101L6
#include <platform/globals.h>
#include <tnftl/IO_TCCXXX.h>
#include <platform/gpio.h>

extern void tca_ckc_setpmupwroff( unsigned int periname , unsigned int isenable);

static int n101l6_panel_init(struct lcd_panel *panel)
{
	//struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, 0);

	return 0;
}
static int n101l6_set_power(struct lcd_panel *panel, int on)
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

		tcclcd_gpio_set_value( (GPIO_PORTF|13) , 1);	// LVDS_Module_PWR	
		
		tcclcd_gpio_set_value(pdata->power_on, 1);		// LVDS_PWR
		tcclcd_gpio_set_value(pdata->display_on, 1);	// LVDS_PWM
		tcclcd_gpio_set_value(pdata->reset, 1);			// LVDS_LEDEN
		mdelay(100);					

		lcdc_initialize(pdata->lcdc_num, panel);

		// LVDS power on
		tca_ckc_setpmupwroff(PMU_LVDSPHY, ENABLE);

		pDDICfg->LVDS_TXO_SEL0 = 0x15141312; // SEL_03, SEL_02, SEL_01, SEL_00,
		pDDICfg->LVDS_TXO_SEL1 = 0x0B0A1716; // SEL_07, SEL_06, SEL_05, SEL_04,
		pDDICfg->LVDS_TXO_SEL2 = 0x0F0E0D0C; // SEL_11, SEL_10, SEL_09, SEL_08,
		pDDICfg->LVDS_TXO_SEL3 = 0x05040302; // SEL_15, SEL_14, SEL_13, SEL_12,
		pDDICfg->LVDS_TXO_SEL4 = 0x1A190706; // SEL_19, SEL_18, SEL_17, SEL_16,
		pDDICfg->LVDS_TXO_SEL5 = 0x1F1E1F18; //                         SEL_20,
		pDDICfg->LVDS_TXO_SEL6 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL7 = 0x1F1E1F1E;
		pDDICfg->LVDS_TXO_SEL8 = 0x1F1E1F1E;

	    // LVDS reset
	    BITSET(pDDICfg->LVDS_CTRL, Hw1);    // reset
	    BITCLR(pDDICfg->LVDS_CTRL, Hw1);    // normal

		// LVDS Select		
		//BITCLR(pDDICfg->LVDS_CTRL, Hw0); //LCDC0		
		BITSET(pDDICfg->LVDS_CTRL, Hw0); //LCDC1		org must
		

		#ifdef TCC88XX

		M = 10; P = 10; S = 0; VSEL = 0; TC = 4;		
		BITCSET(pDDICfg->LVDS_CTRL.nREG, 0x00FFFFF0, (VSEL<<4)|(S<<5)|(M<<8)|(P<<15)|(TC<<21)); //LCDC1

	    BITSET(pDDICfg->LVDS_CTRL, Hw1);    // reset
		#endif//


	    // LVDS enable
	    BITSET(pDDICfg->LVDS_CTRL, Hw2);

	}

	
	else 	{
		tcclcd_gpio_set_value(pdata->reset, 0);			// LVDS_LEDEN				
		tcclcd_gpio_set_value(pdata->display_on, 0);	// LVDS_PWM
		tcclcd_gpio_set_value(pdata->power_on, 0);		// LVDS_PWR

		tcclcd_gpio_set_value( (GPIO_PORTF|13) , 0);	// LVDS_Module_PWR	

	}
	
	return 0;
}

static int n101l6_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, level);
	
	if (level == 0) {
		tcclcd_gpio_set_value( pdata->display_on ,0);	// LVDS_pwm
		tcclcd_gpio_set_value( pdata->bl_on, 0);	// LVDS_BL_off
	} else {

		tcclcd_gpio_set_value( pdata->display_on ,1);	// LVDS_pwm
		#ifdef TCC892X
		if(HW_REV==0x1005 || HW_REV==0x1006 || HW_REV==0x1007|| HW_REV==0x1008 || HW_REV==0x2002 || HW_REV==0x2003 || HW_REV==0x2004 || HW_REV==0x2005 || HW_REV==0x2006 || HW_REV==0x2007 || HW_REV==0x2008 || HW_REV==0x2009)
			tca_tco_pwm_ctrl(0, pdata->bl_on, MAX_BACKLIGTH, level);
		else
			tca_tco_pwm_ctrl(1, pdata->bl_on, MAX_BACKLIGTH, level);
		#elif defined(TCC88XX) || defined(TCC893X)
			tca_tco_pwm_ctrl(0, pdata->bl_on, MAX_BACKLIGTH, level);
		#else
			printf("%s :  Do not support chipset !!!\n",__func__);
		#endif//
	}
	return 0;
}

static struct lcd_panel n101l6_panel = {
	.name		= "N101L6",
	.manufacturer	= "CHI_MEI",
	.id			= PANEL_ID_N101L6,			// PANEL_ID_N101l6 is announced in the tcclcd.h (in /tcc88xx)
	
	.xres		= 1024,
	.yres		= 600,	
	.width		= 222,
	.height		= 125,
	.bpp		= 24,
	.clk_freq	= 439700, 
	.clk_div	= 2,	
	.bus_width	= 24,		
	.lpw		= 0,
	.lpc		= 1024,			
	.lswc		= 0,		
	.lewc		= 157,	
	.vdb		= 0,
	.vdf		= 0,	
	.fpw1		= 0,
	.flc1		= 600,	
	.fswc1		= 0,
	.fewc1		= 16,	
	.fpw2		= 0,
	.flc2		= 600,
	.fswc2		= 0,
	.fewc2		= 16,
	
	.init		= n101l6_panel_init,
	.set_power	= n101l6_set_power,
	.set_backlight_level = n101l6_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &n101l6_panel;

}
#endif//N101L6
