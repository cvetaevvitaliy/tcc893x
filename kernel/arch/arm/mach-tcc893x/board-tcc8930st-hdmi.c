/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc893x/board-tcc8930st-hdmi.c
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
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>

#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/system.h>

#include <mach/gpio.h>
#include <mach/bsp.h>
#include <mach/tcc_board_hdmi.h>

#include <mach/tcc_board_power.h>
#include <linux/platform_device.h>

#include <linux/err.h>
#include "board-tcc8930st.h"

tcc_hdmi_power_s hdmipwr;

int tcc_hdmi_power(struct device *dev, tcc_hdmi_power_s pwr)
{
	hdmipwr = pwr;
	
 	if (machine_is_tcc8930st()) 
	{
		printk("%s  tcc893x  pwr:%d system_rev:0x%x \n", __func__, pwr, system_rev);

		switch(pwr)
		{

			case TCC_HDMI_PWR_INIT:
				break;

			case TCC_HDMI_PWR_ON:
				break;

			case TCC_HDMI_PWR_OFF:
				break;

			case TCC_HDMI_PWR_V5_ON:
				break;

			case TCC_HDMI_PWR_V5_OFF:
				break;

			default:
				break;
		}		
	}
	else
	{
		printk("ERROR : can not find HDMI POWER SETTING ");
	}

	return 0;

}


static struct tcc_hdmi_platform_data hdmi_pdata = {
	.set_power	= tcc_hdmi_power,
};


struct platform_device tcc_hdmi_device = {
	.name	= "tcc_hdmi",
	.dev	= {
		.platform_data	= &hdmi_pdata,
	},
};

struct platform_device tcc_audio_device = {
	.name	= "tcc_hdmi_audio",
};


struct platform_device tcc_cec_device = {
	.name	= "tcc_hdmi_cec",
};

static struct tcc_hpd_platform_data hpd_pdata = {
	.hpd_port = TCC_GPHDMI(1),
};

struct platform_device tcc_hpd_device = {
	.name	= "tcc_hdmi_hpd",
	.dev = {
		.platform_data = &hpd_pdata,
	}, 
};

static int __init tcc8930_init_hdmi(void)
{

	printk("%s (built %s %s)\n", __func__, __DATE__, __TIME__);

	platform_device_register(&tcc_hdmi_device);

	platform_device_register(&tcc_audio_device);

	platform_device_register(&tcc_cec_device);

	platform_device_register(&tcc_hpd_device);

	return 0;
}

device_initcall(tcc8930_init_hdmi);
