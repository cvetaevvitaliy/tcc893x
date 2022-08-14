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
#include "board-tcc8930.h"

/*******************************************************************
		TCC METRIX KEY
*******************************************************************/

//------------------------------------------------------------------
// system_rev = 0x1000
static unsigned int tcc8930_col_gpios[] = { TCC_GPB(24), TCC_GPB(25), TCC_GPB(26) };
static unsigned int tcc8930_row_gpios[] = { TCC_GPB(27), TCC_GPB(29), TCC_GPE(30) };

#define KEYMAP_INDEX_TCC8930(col, row) ((col)*ARRAY_SIZE(tcc8930_row_gpios) + (row))

static const unsigned short tcc8930_keymap[] = {
	[KEYMAP_INDEX_TCC8930(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX_TCC8930(0, 1)] = KEY_RESERVED,
	[KEYMAP_INDEX_TCC8930(0, 2)] = KEY_POWER,	// KEY_END

	[KEYMAP_INDEX_TCC8930(1, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX_TCC8930(1, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX_TCC8930(1, 2)] = KEY_RESERVED,
	
	[KEYMAP_INDEX_TCC8930(2, 0)] = KEY_HOMEPAGE,	// KEY_HOME
	[KEYMAP_INDEX_TCC8930(2, 1)] = KEY_BACK,
	[KEYMAP_INDEX_TCC8930(2, 2)] = KEY_RESERVED,
};

static struct gpio_event_matrix_info tcc8930_keymap_info = {
	.info.func = gpio_event_matrix_func,
	.keymap = tcc8930_keymap,
	.output_gpios = tcc8930_col_gpios,
	.input_gpios = tcc8930_row_gpios,
	.noutputs = ARRAY_SIZE(tcc8930_col_gpios),
	.ninputs = ARRAY_SIZE(tcc8930_row_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_delay.tv.nsec = 50 * NSEC_PER_MSEC,
#if 1 // XXX: disable interrupt for now
	.flags = GPIOKPF_DRIVE_INACTIVE /*| GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_PRINT_MAPPED_KEYS*/,
#else
	.flags = GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS,
#endif
};

static struct gpio_event_info *tcc8930_matrix_key_info[] = {
	&tcc8930_keymap_info.info,
};

static struct gpio_event_platform_data tcc8930_matrix_key_data = {
	.name = "tcc-matrixkey",
	.info = tcc8930_matrix_key_info,
	.info_count = ARRAY_SIZE(tcc8930_matrix_key_info),
};

static struct platform_device tcc8930_matrix_key_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &tcc8930_matrix_key_data,
	},
};

//------------------------------------------------------------------
// system_rev = 0x2000, 0x3000
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
static unsigned int tcc8935_col_gpios[] = { TCC_GPE(19), TCC_GPE(20), TCC_GPE(21) };
static unsigned int tcc8935_row_gpios[] = { TCC_GPE(22), TCC_GPE(23), TCC_GPE(24) };
#else
static unsigned int tcc8935_col_gpios[] = { TCC_GPE(15), TCC_GPE(16), TCC_GPE(21) };
static unsigned int tcc8935_row_gpios[] = { TCC_GPE(22), TCC_GPE(23), TCC_GPE(24) };
#endif

#define KEYMAP_INDEX_TCC8935(col, row) ((col)*ARRAY_SIZE(tcc8935_row_gpios) + (row))

static const unsigned short tcc8935_keymap[] = {
	[KEYMAP_INDEX_TCC8935(0, 0)] = KEY_MENU,
	[KEYMAP_INDEX_TCC8935(0, 1)] = KEY_RESERVED,
	[KEYMAP_INDEX_TCC8935(0, 2)] = KEY_POWER,	// KEY_END

	[KEYMAP_INDEX_TCC8935(1, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX_TCC8935(1, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX_TCC8935(1, 2)] = KEY_RESERVED,
	
	[KEYMAP_INDEX_TCC8935(2, 0)] = KEY_HOMEPAGE,	// KEY_HOME
	[KEYMAP_INDEX_TCC8935(2, 1)] = KEY_BACK,
	[KEYMAP_INDEX_TCC8935(2, 2)] = KEY_RESERVED,
};

static struct gpio_event_matrix_info tcc8935_keymap_info = {
	.info.func = gpio_event_matrix_func,
	.keymap = tcc8935_keymap,
	.output_gpios = tcc8935_col_gpios,
	.input_gpios = tcc8935_row_gpios,
	.noutputs = ARRAY_SIZE(tcc8935_col_gpios),
	.ninputs = ARRAY_SIZE(tcc8935_row_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.debounce_delay.tv.nsec = 50 * NSEC_PER_MSEC,
#if 1 // XXX: disable interrupt for now
	.flags = GPIOKPF_DRIVE_INACTIVE /*| GPIOKPF_PRINT_UNMAPPED_KEYS | GPIOKPF_PRINT_MAPPED_KEYS*/,
#else
	.flags = GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_PRINT_UNMAPPED_KEYS,
#endif
};

static struct gpio_event_info *tcc8935_matrix_key_info[] = {
	&tcc8935_keymap_info.info,
};

static struct gpio_event_platform_data tcc8935_matrix_key_data = {
	.name = "tcc-matrixkey",
	.info = tcc8935_matrix_key_info,
	.info_count = ARRAY_SIZE(tcc8935_matrix_key_info),
};

static struct platform_device tcc8935_matrix_key_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &tcc8935_matrix_key_data,
	},
};
//------------------------------------------------------------------

static int __init tcc8930_keypad_init(void)
{
	if (!machine_is_tcc893x())
		return 0;

	if(system_rev == 0x1000){
		platform_device_register(&tcc8930_matrix_key_device);
	}else if(system_rev == 0x2000 || system_rev == 0x3000){
		platform_device_register(&tcc8935_matrix_key_device);
	}

	return 0;
}

device_initcall(tcc8930_keypad_init);
