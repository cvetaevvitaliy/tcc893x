/*
 * linux/arch/arm/mach-tcc893x/board-m805_893x-gpio.c
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c/pca953x.h>
#include "board-m805_893x.h"
#include <asm/mach-types.h>

struct board_gpio_irq_config m805_893x_gpio_irqs[] = {
	{ -1, -1 },
};

void __init m805_893x_init_gpio(void)
{
	if (!machine_is_m805_893x())
		return;

	board_gpio_irqs = m805_893x_gpio_irqs;
	printk(KERN_INFO "M805_893X GPIO initialized\n");
	return;
}
