/* arch/arm/mach-tcc893x/board-tcc8930-keypad.c
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
*/

#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_event.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/bsp.h>
#include <mach/io.h>

#include <asm/system.h>
#include "board-m805_893x.h"

/*******************************************************************
		TCC GPIO KEY
*******************************************************************/
static const struct gpio_event_direct_entry m805_893x_gpio_keymap[] = {
#if !defined(CONFIG_REGULATOR_AXP192_PEK) && !defined(CONFIG_REGULATOR_AXP202_PEK) && !defined(CONFIG_REGULATOR_TCC270_PEK)
	{ TCC_GPE(19),	KEY_POWER },
#endif
	{ TCC_GPE(21),	KEY_VOLUMEUP },   // volume +
	{ TCC_GPE(22),	KEY_VOLUMEDOWN }, // volume -
};

static struct gpio_event_input_info m805_893x_gpio_key_input_info = {
	.info.func = gpio_event_input_func,
	.keymap = m805_893x_gpio_keymap,
	.keymap_size = ARRAY_SIZE(m805_893x_gpio_keymap),
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_time.tv.nsec = 20 * NSEC_PER_MSEC,
	//.flags = 0 /*GPIOEDF_PRINT_KEYS*/,
	.type = EV_KEY,
};

static struct gpio_event_info *m805_893x_gpio_key_info[] = {
	&m805_893x_gpio_key_input_info.info,
};

static struct gpio_event_platform_data m805_893x_gpio_key_data = {
	.name = "tcc-gpiokey",
	.info = m805_893x_gpio_key_info,
	.info_count = ARRAY_SIZE(m805_893x_gpio_key_info),
};

static struct platform_device m805_893x_gpio_key_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &m805_893x_gpio_key_data,
	},
};

static int __init m805_893x_keypad_init(void)
{
	if (!machine_is_m805_893x())
		return 0;

	m805_893x_gpio_key_input_info.keymap_size = ARRAY_SIZE(m805_893x_gpio_keymap);
	platform_device_register(&m805_893x_gpio_key_device);

	return 0;
}

device_initcall(m805_893x_keypad_init);
