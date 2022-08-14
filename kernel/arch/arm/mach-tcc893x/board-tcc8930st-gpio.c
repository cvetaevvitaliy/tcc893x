/****************************************************************************
FileName    : kernel/arch/arm//mach-tcc893x/board-tcc8930st-gpio.c
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
#include <mach/gpio.h>
#include <linux/i2c.h>
#include "board-tcc8930st.h"
#include <asm/mach-types.h>

static struct board_gpio_irq_config tcc8930st_gpio_irqs[] = {
	{ -1, -1 },
};

void __init tcc8930st_init_gpio(void)
{
	if (!machine_is_tcc8930st())
		return;

	board_gpio_irqs = tcc8930st_gpio_irqs;
	printk(KERN_INFO "TCC8930ST GPIO initialized\n");
	return;
}
