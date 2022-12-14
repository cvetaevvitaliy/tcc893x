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

#include "mali_kernel_common.h"
#include "mali_osk.h"
#include "mali_platform.h"
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <asm/io.h>
//#include "mali_device_pause_resume.h"
#include <linux/workqueue.h>

#if defined(CONFIG_CLOCK_TABLE)
#include <mach/clocktable.h>
static struct func_clktbl_t *mali_clktbl = NULL;
#else
struct clk *gpu_clk = NULL;
#endif

#define GPU_THRESHOLD(x)	((unsigned int)((x*255)/100))

struct tcc_gpu_dvfs_table_t {
	unsigned int gpu_freq;
	unsigned int up_threshold;
	unsigned int down_threshold;
};

#if defined(CONFIG_ARCH_TCC893X)
static struct tcc_gpu_dvfs_table_t gpu_dvfs_table[] = {
	{ 192000, 60, 40 },	// Core 1.00V
	{ 288000, 60, 40 },	// Core 1.10V
	{ 384000, 65, 40 },	// Core 1.20V
};
#elif defined(CONFIG_ARCH_TCC892X)
static struct tcc_gpu_dvfs_table_t gpu_dvfs_table[] = {
#if (1)	// normal
	{ 184000, 50, 30 },	// Core 1.00V
	{ 224140, 60, 40 },	// Core 1.05V
	{ 264290, 70, 50 },	// Core 1.10V
	{ 317140, 80, 60 },	// Core 1.15V
	{ 370000, 85, 70 },	// Core 1.20V
	{ 414400, 90, 80 },	// Core 1.25V
	{ 458800, 95, 85 },	// Core 1.30V
#else
#if (1)	// for reducing current
	#if (0)
	{ 184000, 85, 75 },	// Core 1.00V
	{ 224140, 85, 75 },	// Core 1.05V
	{ 264290, 85, 75 },	// Core 1.10V
	{ 317140, 85, 75 },	// Core 1.15V
	{ 370000, 90, 80 },	// Core 1.20V
	{ 414400, 95, 85 },	// Core 1.25V
	{ 458800, 95, 90 },	// Core 1.30V
	#else
	{ 184000, 95, 85 },	// Core 1.00V
	{ 224140, 95, 85 },	// Core 1.05V
	{ 264290, 95, 85 },	// Core 1.10V
	{ 317140, 95, 85 },	// Core 1.15V
	{ 370000, 95, 85 },	// Core 1.20V
	{ 414400, 95, 85 },	// Core 1.25V
	{ 458800, 95, 90 },	// Core 1.30V
	#endif
#else	// for performance
	{ 184000, 60, 40 },	// Core 1.00V
	{ 224140, 60, 40 },	// Core 1.05V
	{ 264290, 60, 40 },	// Core 1.10V
	{ 317140, 60, 40 },	// Core 1.15V
	{ 370000, 60, 40 },	// Core 1.20V
	{ 414400, 60, 40 },	// Core 1.25V
	{ 458800, 60, 40 },	// Core 1.30V
#endif
#endif
};
#else
	#error
#endif
#define MALI_DVFS_STEPS ARRAY_SIZE(gpu_dvfs_table)

static struct workqueue_struct *mali_dvfs_wq = 0;
static unsigned int maliDvfsCurrentStep;
static u32 mali_dvfs_utilization = 255;

static void mali_dvfs_work_handler(struct work_struct *w);
static DECLARE_WORK(mali_dvfs_work, mali_dvfs_work_handler);

__inline static unsigned int decideNextStatus(unsigned int utilization, unsigned int currStep)
{
	if (currStep >= MALI_DVFS_STEPS)
		return currStep;

    if(utilization > GPU_THRESHOLD(gpu_dvfs_table[currStep].up_threshold)) {
	    currStep++;
		if (currStep >= (MALI_DVFS_STEPS-1))
			currStep = MALI_DVFS_STEPS-1;
	}
    else if(utilization < GPU_THRESHOLD(gpu_dvfs_table[currStep].down_threshold)) {
		if (currStep > 0)
			currStep--;
	}

	return currStep;
}

static void mali_dvfs_work_handler(struct work_struct *w)
{
	unsigned int nextStep = 0;
	unsigned int currStep = 0;
	mali_bool boostup = MALI_FALSE;

	/*decide next step*/
	currStep = maliDvfsCurrentStep;
	nextStep = decideNextStatus(mali_dvfs_utilization, currStep);

	/*if next status is same with current status, don't change anything*/
	if(currStep != nextStep) {
		/*check if boost up or not*/
		if(nextStep > currStep)
			boostup = 1;

		/*change mali dvfs status*/
		if(boostup) {
#if defined(CONFIG_CLOCK_TABLE)
			if (mali_clktbl == NULL) {
				mali_clktbl = clocktable_get("mali_clktbl");
				if (IS_ERR(mali_clktbl))
					mali_clktbl = NULL;
			}
			if (mali_clktbl)
				clocktable_ctrl(mali_clktbl, nextStep, CLKTBL_ENABLE);
#else
			if (gpu_clk) {
				if (nextStep >= MALI_DVFS_STEPS)
					nextStep = MALI_DVFS_STEPS-1;
				clk_set_rate(gpu_clk, gpu_dvfs_table[nextStep].gpu_freq*1000);
			}
#endif
		}
		else {
#if defined(CONFIG_CLOCK_TABLE)
			if (mali_clktbl == NULL) {
				mali_clktbl = clocktable_get("mali_clktbl");
				if (IS_ERR(mali_clktbl))
					mali_clktbl = NULL;
			}
			if (mali_clktbl)
				clocktable_ctrl(mali_clktbl, nextStep, CLKTBL_ENABLE);
#else
			if (gpu_clk) {
				if (nextStep >= MALI_DVFS_STEPS)
					nextStep = MALI_DVFS_STEPS-1;
				clk_set_rate(gpu_clk, gpu_dvfs_table[nextStep].gpu_freq*1000);
			}
#endif
		}

	    maliDvfsCurrentStep = nextStep;
	}
}

mali_bool init_mali_dvfs_staus(int step)
{
	if (!mali_dvfs_wq)
		mali_dvfs_wq = create_singlethread_workqueue("mali_dvfs");
#if !defined(CONFIG_CLOCK_TABLE)
	if (gpu_clk == NULL) {
		gpu_clk = clk_get(NULL, "gpu");
		if (IS_ERR(gpu_clk))
			gpu_clk = NULL;
		else {
			if (step >= MALI_DVFS_STEPS)
				step = MALI_DVFS_STEPS-1;
			clk_set_rate(gpu_clk, gpu_dvfs_table[step].gpu_freq*1000);
		}
	}
#endif
	maliDvfsCurrentStep = step;
	return MALI_TRUE;
}

void deinit_mali_dvfs_staus(void)
{
	if (mali_dvfs_wq)
		destroy_workqueue(mali_dvfs_wq);
	mali_dvfs_wq = NULL;
}

mali_bool mali_dvfs_handler(u32 utilization)
{
	mali_dvfs_utilization = utilization;
	if (mali_dvfs_wq)
		queue_work_on(0, mali_dvfs_wq,&mali_dvfs_work);
	return MALI_TRUE;
}
