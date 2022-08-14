/*
 * Copyright (C) 2013 Telechips Inc.
 * Copyright (C) 2010, 2012-2013 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_platform.c
 * Platform specific Mali driver functions for:
 * - Realview Versatile platforms with ARM11 Mpcore and virtex 5.
 * - Versatile Express platforms with ARM Cortex-A9 and virtex 6.
 */
#include "mali_kernel_common.h"
#include "mali_osk.h"
#include "mali_platform.h"

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/pm.h>
#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif
#include <asm/io.h>
#include <linux/mali/mali_utgard.h>
#include "mali_kernel_common.h"
#include <linux/dma-mapping.h>
#include <linux/moduleparam.h>
#include "mali_pp_scheduler.h"
#include "arch/config.h"

#if defined(CONFIG_CLOCK_TABLE)
#include <mach/clocktable.h>
static struct func_clktbl_t *mali_clktbl = NULL;
#endif
static struct clk *mali_clk = NULL;

typedef enum	{
	MALI_CLK_NONE=0,
	MALI_CLK_ENABLED,
	MALI_CLK_DISABLED
} t_mali_clk_type;
static t_mali_clk_type mali_clk_enable = MALI_CLK_NONE;

#if defined(CONFIG_GPU_BUS_SCALING)
extern mali_bool init_mali_dvfs_staus(int step);
extern void deinit_mali_dvfs_staus(void);
extern mali_bool mali_dvfs_handler(u32 utilization);
#endif

static int mali_os_suspend(struct device *device);
static int mali_os_resume(struct device *device);
static int mali_os_freeze(struct device *device);
static int mali_os_thaw(struct device *device);
#ifdef CONFIG_PM_RUNTIME
static int mali_runtime_suspend(struct device *device);
static int mali_runtime_resume(struct device *device);
static int mali_runtime_idle(struct device *device);
#endif

static void mali_platform_device_release(struct device *device);

static struct resource mali_gpu_resources[] =
{
#if defined(CONFIG_ARCH_TCC893X)
	MALI_GPU_RESOURCES_MALI400_MP2(0x70000000, _IRQ_3DGP_, _IRQ_3DGPMMU_, _IRQ_3DPP0_, _IRQ_3DPP0MMU_, _IRQ_3DPP1_, _IRQ_3DPP1MMU_)
#else
	MALI_GPU_RESOURCES_MALI400_MP1(0x70000000, _IRQ_3DGP_, _IRQ_3DGPMMU_, _IRQ_3DPP0_, _IRQ_3DPP0MMU_)

#endif
};

static struct dev_pm_ops mali_gpu_device_type_pm_ops =
{
	.suspend = mali_os_suspend,
	.resume = mali_os_resume,
	.freeze = mali_os_freeze,
	.thaw = mali_os_thaw,
        .restore = mali_os_thaw,
#ifdef CONFIG_PM_RUNTIME
	.runtime_suspend = mali_runtime_suspend,
	.runtime_resume = mali_runtime_resume,
	.runtime_idle = mali_runtime_idle,
#endif
};

static struct device_type mali_gpu_device_device_type =
{
	.pm = &mali_gpu_device_type_pm_ops,
};


static struct mali_gpu_device_data mali_gpu_data =
{
#if 0
	/* Dedicated memory setup (not sure if this is actually reserved on the platforms any more) */
	.dedicated_mem_start = 0x48A00000, /* Physical start address */
	.dedicated_mem_size = 0x07800000, /* 120MB */
#endif
	.shared_mem_size = CONFIG_MALI_MEMORY_SIZE * 1024UL * 1024UL,
/*
	.fb_start = 0x48200000,
	.fb_size = 0x00800000,
*/

#if USING_GPU_UTILIZATION
	.utilization_interval = 300,
	.utilization_callback = mali_gpu_utilization_callback,
#endif
};
static struct platform_device mali_gpu_device =
{
	.name = MALI_GPU_NAME_UTGARD,
	.id = 0,
	.dev.release = mali_platform_device_release,
	.dev.coherent_dma_mask = DMA_BIT_MASK(32),
	.dev.platform_data = &mali_gpu_data,
	.dev.type = &mali_gpu_device_device_type, /* We should probably use the pm_domain instead of type on newer kernels */
};
int mali_platform_device_register(void)
{
	int err;
	u32 m400_gp_version;

	MALI_DEBUG_PRINT(2, ("mali_platform_device_register() called\n"));

	mali_gpu_device.num_resources = ARRAY_SIZE(mali_gpu_resources);
	mali_gpu_device.resource = mali_gpu_resources;
	/* Register the platform device */
	err = platform_device_register(&mali_gpu_device);
	if (0 == err)
	{
				mali_platform_init();

#ifdef CONFIG_PM_RUNTIME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
				pm_runtime_set_autosuspend_delay(&(mali_gpu_device.dev), 1000);
				pm_runtime_use_autosuspend(&(mali_gpu_device.dev));
#endif
				pm_runtime_enable(&(mali_gpu_device.dev));
#endif

				return 0;
	}

	return err;
}

void mali_platform_device_unregister(void)
{
	MALI_DEBUG_PRINT(2, ("mali_platform_device_unregister() called\n"));

	mali_platform_deinit();

	platform_device_unregister(&mali_gpu_device);
}

static void mali_platform_device_release(struct device *device)
{
	MALI_DEBUG_PRINT(4, ("mali_platform_device_release() called\n"));
}

_mali_osk_errcode_t mali_platform_init(void)
{
#if defined(CONFIG_GPU_BUS_SCALING)
	if(!init_mali_dvfs_staus(0))
		MALI_DEBUG_PRINT(1, ("mali_platform_init failed\n"));        
#endif

	if(mali_clk_enable != MALI_CLK_ENABLED)
	{
		MALI_DEBUG_PRINT(2, ("mali_platform_init() clk_enable\n"));
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
		clk_enable(mali_clk);
#if defined(CONFIG_CLOCK_TABLE)
		if (mali_clktbl == NULL) {
			mali_clktbl = clocktable_get("mali_clktbl");
			if (IS_ERR(mali_clktbl))
				mali_clktbl = NULL;
		}
		if (mali_clktbl)
			clocktable_ctrl(mali_clktbl, 0, CLKTBL_ENABLE);
#endif
		mali_clk_enable = MALI_CLK_ENABLED;
	}

    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_deinit(void)
{
	if(mali_clk_enable == MALI_CLK_ENABLED)
	{
		MALI_DEBUG_PRINT(2, ("mali_platform_deinit() clk_disable\n"));
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
#if defined(CONFIG_CLOCK_TABLE)
		if (mali_clktbl == NULL) {
			mali_clktbl = clocktable_get("mali_clktbl");
			if (IS_ERR(mali_clktbl))
				mali_clktbl = NULL;
		}
		if (mali_clktbl)
			clocktable_ctrl(mali_clktbl, 0, CLKTBL_DISABLE);
#endif
		clk_disable(mali_clk);
		mali_clk_enable = MALI_CLK_DISABLED;

	}

#if defined(CONFIG_GPU_BUS_SCALING)
	deinit_mali_dvfs_staus();
#endif
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_powerdown(u32 cores)
{
	if(mali_clk_enable == MALI_CLK_ENABLED)
	{
		MALI_DEBUG_PRINT(2, ("mali_platform_powerdown() clk_disable\n"));
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
#if defined(CONFIG_CLOCK_TABLE)
		if (mali_clktbl == NULL) {
			mali_clktbl = clocktable_get("mali_clktbl");
			if (IS_ERR(mali_clktbl))
				mali_clktbl = NULL;
		}
		if (mali_clktbl)
			clocktable_ctrl(mali_clktbl, 0, CLKTBL_DISABLE);
#endif
		clk_disable(mali_clk);
		mali_clk_enable = MALI_CLK_DISABLED;

	}
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_powerup(u32 cores)
{
	if(mali_clk_enable != MALI_CLK_ENABLED)
	{
		MALI_DEBUG_PRINT(2, ("mali_platform_powerup() clk_enable\n"));
		if (mali_clk == NULL)
			mali_clk = clk_get(NULL, "mali_clk");	
		clk_enable(mali_clk);
#if defined(CONFIG_CLOCK_TABLE)
		if (mali_clktbl == NULL) {
			mali_clktbl = clocktable_get("mali_clktbl");
			if (IS_ERR(mali_clktbl))
				mali_clktbl = NULL;
		}
		if (mali_clktbl)
			clocktable_ctrl(mali_clktbl, 0, CLKTBL_ENABLE);
#endif
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
			MALI_DEBUG_PRINT(2, ("mali_platform_power_mode_change() clk_enable\n"));
			if (mali_clk == NULL)
				mali_clk = clk_get(NULL, "mali_clk");	
			clk_enable(mali_clk);
#if defined(CONFIG_CLOCK_TABLE)
		if (mali_clktbl == NULL) {
			mali_clktbl = clocktable_get("mali_clktbl");
			if (IS_ERR(mali_clktbl))
				mali_clktbl = NULL;
		}
		if (mali_clktbl)
			clocktable_ctrl(mali_clktbl, 0, CLKTBL_ENABLE);
#endif
			mali_clk_enable = MALI_CLK_ENABLED;
		}
	}

	else
	{
		if( mali_clk_enable == MALI_CLK_ENABLED)
		{
			MALI_DEBUG_PRINT(2, ("mali_platform_power_mode_change() clk_disable\n"));
			if (mali_clk == NULL)
				mali_clk = clk_get(NULL, "mali_clk");	
#if defined(CONFIG_CLOCK_TABLE)
		if (mali_clktbl == NULL) {
			mali_clktbl = clocktable_get("mali_clktbl");
			if (IS_ERR(mali_clktbl))
				mali_clktbl = NULL;
		}
		if (mali_clktbl)
			clocktable_ctrl(mali_clktbl, 0, CLKTBL_DISABLE);
#endif
			clk_disable(mali_clk);
			mali_clk_enable = MALI_CLK_DISABLED;
		}

	}
    MALI_SUCCESS;
}

void mali_gpu_utilization_callback(struct mali_gpu_utilization_data *data)
{
	//printk("%s: utilization:%d, %d, %d\n", __func__,  data->utilization_gpu, data->utilization_pp, data->utilization_gp);
	//todo
#if defined(CONFIG_GPU_BUS_SCALING)
	if(mali_clk_enable == MALI_CLK_ENABLED) {
		if(!mali_dvfs_handler(data->utilization_gpu))
			MALI_DEBUG_PRINT(1,( "error on mali dvfs status in utilization\n"));
	}
#endif
}

void set_mali_parent_power_domain(void* dev)
{
}
static int mali_os_suspend(struct device *device)
{
	int ret = 0;

	MALI_DEBUG_PRINT(2, ("mali_os_suspend() called\n"));

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->suspend)
	{
		/* Need to notify Mali driver about this event */
		ret = device->driver->pm->suspend(device);
	}

	mali_platform_power_mode_change(MALI_POWER_MODE_DEEP_SLEEP);

	return ret;
}

static int mali_os_resume(struct device *device)
{
	int ret = 0;

	MALI_DEBUG_PRINT(2, ("mali_os_resume() called\n"));

	mali_platform_power_mode_change(MALI_POWER_MODE_ON);

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->resume)
	{
		/* Need to notify Mali driver about this event */
		ret = device->driver->pm->resume(device);
	}

	return ret;
}

static int mali_os_freeze(struct device *device)
{
	int ret = 0;

	MALI_DEBUG_PRINT(2, ("mali_os_freeze() called\n"));

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->freeze)
	{
		/* Need to notify Mali driver about this event */
		ret = device->driver->pm->freeze(device);
	}
	mali_platform_power_mode_change(MALI_POWER_MODE_DEEP_SLEEP);

	return ret;
}

static int mali_os_thaw(struct device *device)
{
	int ret = 0;

	MALI_DEBUG_PRINT(2, ("mali_os_thaw() called\n"));
	mali_platform_power_mode_change(MALI_POWER_MODE_ON);

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->thaw)
	{
		/* Need to notify Mali driver about this event */
		ret = device->driver->pm->thaw(device);
	}

	return ret;
}

#ifdef CONFIG_PM_RUNTIME
static int mali_runtime_suspend(struct device *device)
{
	int ret = 0;

	MALI_DEBUG_PRINT(2, ("mali_runtime_suspend() called\n"));

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->runtime_suspend)
	{
		/* Need to notify Mali driver about this event */
		ret = device->driver->pm->runtime_suspend(device);
	}

	mali_platform_power_mode_change(MALI_POWER_MODE_LIGHT_SLEEP);

	return ret;
}

static int mali_runtime_resume(struct device *device)
{
	int ret = 0;

	MALI_DEBUG_PRINT(2, ("mali_runtime_resume() called\n"));

	mali_platform_power_mode_change(MALI_POWER_MODE_ON);

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->runtime_resume)
	{
		/* Need to notify Mali driver about this event */
		ret = device->driver->pm->runtime_resume(device);
	}

	return ret;
}

static int mali_runtime_idle(struct device *device)
{
	MALI_DEBUG_PRINT(2, ("mali_runtime_idle() called\n"));

	if (NULL != device->driver &&
	    NULL != device->driver->pm &&
	    NULL != device->driver->pm->runtime_idle)
	{
		/* Need to notify Mali driver about this event */
		int ret = device->driver->pm->runtime_idle(device);
		if (0 != ret)
		{
			return ret;
		}
	}

	pm_runtime_suspend(device);

	return 0;
}
#endif
