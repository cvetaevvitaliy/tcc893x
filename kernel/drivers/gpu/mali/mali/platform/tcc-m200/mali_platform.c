/*
 * Copyright (C) 2013 Telechips Inc.
 * Copyright (C) 2010-2013 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_platform.c
 * Platform specific Mali driver functions for a default platform
 */
#include "mali_kernel_common.h"
#include "mali_osk.h"
#include "mali_platform.h"

#include <linux/clk.h>
#include <linux/cpufreq.h>

static struct clk *mali_clk = NULL;

typedef enum	{
	MALI_CLK_NONE=0,
	MALI_CLK_ENABLED,
	MALI_CLK_DISABLED
} t_mali_clk_type;
static t_mali_clk_type mali_clk_enable = MALI_CLK_NONE;

_mali_osk_errcode_t mali_platform_init(void)
{
	if(mali_clk_enable != MALI_CLK_ENABLED)
	{
//		printk("mali_platform_init() clk_enable\n");
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
		clk_enable(mali_clk);
		mali_clk_enable = MALI_CLK_ENABLED;
	}

    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_deinit(void)
{
	if(mali_clk_enable == MALI_CLK_ENABLED)
	{
//		printk("mali_platform_deinit() clk_disable\n");
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
		clk_disable(mali_clk);
		mali_clk_enable = MALI_CLK_DISABLED;

	}

    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_powerdown(u32 cores)
{
	if(mali_clk_enable == MALI_CLK_ENABLED)
	{
//		printk("mali_platform_powerdown() clk_disable\n");
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
		clk_disable(mali_clk);
		mali_clk_enable = MALI_CLK_DISABLED;

	}
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_powerup(u32 cores)
{
	if(mali_clk_enable != MALI_CLK_ENABLED)
	{
//		printk("mali_platform_powerup() clk_enable\n");
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
		clk_enable(mali_clk);
		mali_clk_enable = MALI_CLK_ENABLED;

	}
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_power_mode_change(mali_power_mode power_mode)
{
	if(power_mode == MALI_POWER_MODE_ON)
	{
		if(mali_clk_enable != MALI_CLK_ENABLED)
		{
//			printk("mali_platform_power_mode_change() clk_enable\n");
			if (mali_clk == NULL)
				mali_clk = clk_get(NULL, "mali_clk");	
			clk_enable(mali_clk);
			mali_clk_enable = MALI_CLK_ENABLED;
		}
	}

	else
	{
		if( mali_clk_enable == MALI_CLK_ENABLED)
		{
//			printk("mali_platform_power_mode_change() clk_disable\n");
			if (mali_clk == NULL)
				mali_clk = clk_get(NULL, "mali_clk");	
			clk_disable(mali_clk);
			mali_clk_enable = MALI_CLK_DISABLED;
		}

	}
    MALI_SUCCESS;
}

void mali_gpu_utilization_handler(u32 utilization)
{
}

void set_mali_parent_power_domain(void* dev)
{
}
