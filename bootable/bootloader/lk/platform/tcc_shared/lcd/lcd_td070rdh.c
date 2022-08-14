/****************************************************************************
 *   FileName    : tm070rdh11_lcd.c
 *   Description :  support for TPO TD070RDH11 LCD
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

#ifdef TD070RDH11
#include <dev/gpio.h>


static int tm070rdh11_panel_init(struct lcd_panel *panel)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d  \n", __func__, 0);

	tcclcd_gpio_config(pdata->display_on, 1);
	tcclcd_gpio_config(pdata->bl_on, 1);
	tcclcd_gpio_config(pdata->reset, 1);
	tcclcd_gpio_config(pdata->power_on, 1);

	tcclcd_gpio_set_value(pdata->display_on, 0);
	tcclcd_gpio_set_value(pdata->bl_on, 0);
	tcclcd_gpio_set_value(pdata->reset, 0);
	tcclcd_gpio_set_value(pdata->power_on, 0);
	return 0;
}

static int tm070rdh11_set_power(struct lcd_panel *panel, int on)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d ~ \n", __func__, on);

	if (on) {
		tcclcd_gpio_set_value(pdata->reset, 1);
		lcd_delay_us(1000);

		tcclcd_gpio_set_value(pdata->power_on, 1);
		lcd_delay_us(1000);
		tcclcd_gpio_set_value(pdata->reset, 0);
		lcd_delay_us(1000);
		tcclcd_gpio_set_value(pdata->reset, 1);

		mdelay(10);

		lcdc_initialize(pdata->lcdc_num, panel);

		LCDC_IO_Set(pdata->lcdc_num, panel->bus_width);

		// lcd port current 
		BITCSET(HwGPIOC->GPCD0,0xFFFFFFFF, 0xAAAAAAAA);		
		BITCSET(HwGPIOC->GPCD1,0x00FFFFFF, 0x00FFAAAA);

		BITCSET(HwGPIOC->GPFN3, HwPORTCFG_GPFN0_MASK , HwPORTCFG_GPFN0(0));
		BITCSET(HwGPIOC->GPEN, Hw24 ,Hw24);
		BITCSET(HwGPIOC->GPDAT, Hw24 ,0);

		mdelay(16);

	}
	else 
	{
		tcclcd_gpio_set_value(pdata->display_on, 0);
		mdelay(10);
		tcclcd_gpio_set_value(pdata->reset, 0);
		tcclcd_gpio_set_value(pdata->power_on, 0);
		LCDC_IO_Disable(pdata->lcdc_num, panel->bus_width);
	}

	return 0;
}


static int tm070rdh11_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, level);

	if (level == 0) {
		tcclcd_gpio_set_value(pdata->bl_on, 0);
	} else {
//		tcclcd_gpio_set_value(pdata->bl_on, 1);
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


static struct lcd_panel tm070rdh11_panel = {
	.name		= "TD070RDH11",
	.manufacturer	= "OPTOELECTRONICS",
	.id		= PANEL_ID_TD070RDH11,
	.xres		= 800,
	.yres		= 480,
	.width		= 154,
	.height		= 85,
	.bpp		= 24,
	.clk_freq	= 300000,
	.clk_div	= 2,
	.bus_width	= 24,

	.lpw		= 47,
	.lpc		= 800,
	.lswc		= 39,
	.lewc		= 39,

	.vdb		= 0,
	.vdf		= 0,
	
	.fpw1		= 2,
	.flc1		= 480,
	.fswc1		= 28,
	.fewc1		= 12,
	
	.fpw2		= 2,
	.flc2		= 480,
	.fswc2		= 28,
	.fewc2		= 12,
//юс╫ц	.sync_invert	= IV_INVERT | IH_INVERT | IP_INVERT,
	.sync_invert	= IV_INVERT | IH_INVERT,

	.init		= tm070rdh11_panel_init,
	.set_power	= tm070rdh11_set_power,
	.set_backlight_level = tm070rdh11_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &tm070rdh11_panel;
}
#endif//TD070RDH11
