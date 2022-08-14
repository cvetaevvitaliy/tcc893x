/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc893x/board-tcc8930st-panel.c
Description : 

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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/system.h>

#include <mach/gpio.h>
#include <mach/tcc_fb.h>
#include "devices.h"
#include "board-tcc8930st.h"

#define DEFAULT_BACKLIGHT_BRIGHTNESS	102

static struct lcd_platform_data lcd_pdata = {
	.power_on	= GPIO_LCD_ON,
	.display_on	= GPIO_LCD_DISPLAY,
	.bl_on		= GPIO_LCD_BL,
	.reset		= GPIO_LCD_RESET,
};

#ifdef CONFIG_LCD_HDMI1280X720
static struct platform_device hdmi1280x720_lcd = {
	.name	= "hdmi1280x720_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

#ifdef CONFIG_LCD_HDMI1920X1080
static struct platform_device hdmi1920x1080_lcd = {
	.name	= "hdmi1920x1080_lcd",
	.dev	= {
		.platform_data	= &lcd_pdata,
	},
};
#endif

int __init tcc8930_init_panel(void)
{
	int ret;
	if (!machine_is_tcc8930st())
		return 0;
	
	printk("supported LCD panel type %d  system_rev:0x%x \n", tcc_panel_id, system_rev);

	#ifdef CONFIG_LCD_HDMI1920X1080
 		platform_device_register(&hdmi1920x1080_lcd);
	#else
		platform_device_register(&hdmi1280x720_lcd);
	#endif

	ret = platform_device_register(&tcc_lcd_device);
	if (ret)
		return ret;

	return 0;
}

device_initcall(tcc8930_init_panel);
