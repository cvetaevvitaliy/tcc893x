/****************************************************************************
 *   FileName    : lms480kf01_lcd.c
 *   Description : support for Samsung LMS480KF01 LCD
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

#ifdef LMS480KF
static int lms480kf01_panel_init(struct lcd_panel *panel)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d\n", __func__, 0);
	tcclcd_gpio_config(pdata->display_on, 1);
	tcclcd_gpio_config(pdata->bl_on, 1);
	tcclcd_gpio_config(pdata->reset, 1);
	tcclcd_gpio_config(pdata->power_on, 1);

	return 0;
}
static int lms480kf01_set_power(struct lcd_panel *panel, int on)
{
	struct lcd_platform_data *pdata = &(panel->dev);

	printf("%s : %d \n", __func__, on);
	if (on) {
		tcclcd_gpio_set_value(pdata->power_on, 1);
		tcclcd_gpio_set_value(pdata->reset, 1);

		mdelay(10);
		lcdc_initialize(pdata->lcdc_num, panel);
		
		LCDC_IO_Set(pdata->lcdc_num, panel->bus_width);

		mdelay(16);
		
		tcclcd_gpio_set_value(pdata->display_on, 1);

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

static int lms480kf01_set_backlight_level(struct lcd_panel *panel, int level)
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

static struct lcd_panel lms480kf01_panel = {
	.name		= "LMS480KF01",
	.manufacturer	= "Samsung",
	.id		= PANEL_ID_LMS480KF01,
	.xres		= 800,
	.yres		= 480,
	.width		= 103,
	.height		= 62,
	.bpp		= 32,
	.clk_freq	= 264000,
	.clk_div	= 2,
	.bus_width	= 24,
	.lpw		= 2,
	.lpc		= 800,
	.lswc		= 12,
	.lewc		= 7,
	.vdb		= 0,
	.vdf		= 0,
	.fpw1		= 0,
	.flc1		= 480,
	.fswc1		= 6,
	.fewc1		= 36,
	.fpw2		= 0,
	.flc2		= 480,
	.fswc2		= 6,
	.fewc2		= 36,
	.sync_invert	= IV_INVERT | IH_INVERT,
	.init		= lms480kf01_panel_init,
	.set_power	= lms480kf01_set_power,
	.set_backlight_level = lms480kf01_set_backlight_level,
};

struct lcd_panel *tccfb_get_panel(void)
{
	return &lms480kf01_panel;
}
#endif//LMS480KF
