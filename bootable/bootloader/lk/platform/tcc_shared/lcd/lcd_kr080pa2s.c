/****************************************************************************
 *   FileName    : lcd_kr080pa2s.c
 *   Description : support for LG PHILIPS LB080WV6 LCD
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

#ifdef KR080PA2S
#include <dev/gpio.h>

static int kr080pa2s_panel_init(struct lcd_panel *panel)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d  \n", __func__, 0);

	tcclcd_gpio_config(pdata->display_on, GPIO_OUTPUT);
	tcclcd_gpio_config(pdata->bl_on, GPIO_OUTPUT);
	tcclcd_gpio_config(pdata->reset, GPIO_OUTPUT);
	tcclcd_gpio_config(pdata->power_on, GPIO_OUTPUT);

	tcclcd_gpio_set_value(pdata->display_on, 0);
	tcclcd_gpio_set_value(pdata->bl_on, 0);
	tcclcd_gpio_set_value(pdata->reset, 0);
	tcclcd_gpio_set_value(pdata->power_on, 0);

	return 0;
}


static int kr080pa2s_set_power(struct lcd_panel *panel, int on)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d ~ \n", __func__, on);

	if (on) {
		tcclcd_gpio_set_value(pdata->reset, 1);
#if defined(_M801_88_)
		lcd_delay_us(1000*10);
#else
		lcd_delay_us(1000);
#endif
		tcclcd_gpio_set_value(pdata->power_on, 1);
		lcd_delay_us(1000);
		tcclcd_gpio_set_value(pdata->reset, 0);
		lcd_delay_us(1000);
		tcclcd_gpio_set_value(pdata->reset, 1);
		
		tcclcd_gpio_set_value(pdata->display_on, 1);
		mdelay(10);

		lcdc_initialize(pdata->lcdc_num, panel);

		LCDC_IO_Set(pdata->lcdc_num, panel->bus_width);

		LCDC_IO_SetCurrent(pdata->lcdc_num, panel->bus_width);

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


static int kr080pa2s_set_backlight_level(struct lcd_panel *panel, int level)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, level);

	if (level == 0) {
		tcclcd_gpio_set_value(pdata->bl_on, 0);
	} else {
//		tcclcd_gpio_set_value(pdata->bl_on, 1);
		#ifdef TCC892X
		if(HW_REV==0x1005 || HW_REV==0x1006 || HW_REV==0x1007 || HW_REV==0x1008 || HW_REV==0x2002 || HW_REV==0x2003 || HW_REV==0x2004 || HW_REV==0x2005 || HW_REV==0x2006 || HW_REV==0x2007 || HW_REV==0x2008 || HW_REV==0x2009)
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



static struct lcd_panel kr080pa2s_panel = {
	.name		= "kr080pa2s",
	.manufacturer	= "xingyuan",
	.id			= PANEL_ID_KR080PA2S,
	.xres		= 1024,
	.yres		= 768,
	.width		= 103,
	.height		= 62,
	.bpp		= 24,
	.clk_freq	= 650000,
	.clk_div	= 1,
	.bus_width	= 24,
	.lpw		= 5,
	.lpc		= 1024,
	.lswc		= 150, //160,
	.lewc		= 150, //160,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 2,
	.flc1		= 768,
	.fswc1		= 20,
	.fewc1		= 15,
	.fpw2		= 2,
	.flc2		= 768,
	.fswc2		= 20,
	.fewc2		= 15,
	//.sync_invert	= IV_INVERT| IH_INVERT,
	.init		= kr080pa2s_panel_init,
	.set_power	= kr080pa2s_set_power,
	.set_backlight_level = kr080pa2s_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &kr080pa2s_panel;
}

#endif

