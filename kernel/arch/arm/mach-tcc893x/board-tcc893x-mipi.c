/* linux/arch/arm/mach-tcc893x/board-tcc8900.c
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/input.h>

#include <mach/gpio.h>
#include <mach/bsp.h>
#include "devices.h"
#include "board-tcc8930.h"
#include <mach/tcc_fb.h>


int tcc_mipi_power(struct lcd_panel *db, tcc_db_power_s pwr)
{

		printk("%s  tcc893x  pwr:%d  \n", __func__, pwr);

		switch(pwr)
		{

			case TCC_PWR_INIT:
			{
				gpio_request(GPIO_MIPI_EN, "mipi_block");
				gpio_direction_output(GPIO_MIPI_EN, 1);
				break;
			}

			case TCC_PWR_ON:
			{
				gpio_set_value(GPIO_MIPI_EN, 1);
				break;
			}

			case TCC_PWR_OFF:
			{
				gpio_set_value(GPIO_MIPI_EN, 0);
				break;
			}
			default:
				break;
		}		


	return 0;

}

static struct tcc_db_platform_data tcc_mipi_platform_data = {
	.set_power	= tcc_mipi_power,
};

struct tcc_db_platform_data *get_tcc_db_platform_data(void)
{
	return &tcc_mipi_platform_data;
}
	
EXPORT_SYMBOL(get_tcc_db_platform_data);


