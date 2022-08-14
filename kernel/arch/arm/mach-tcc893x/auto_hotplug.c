/*
 * arch/arm/mach-tcc893x/auto_hotplug.c
 *
 * CPU auto-hotplug for TCC CPUs
 *
 * Copyright (C) 2013 Telechips Inc.
 * Copyright (c) 2011-2012, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/cpu.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include "tcc_cpufreq.h"

#ifdef CONFIG_CLOCK_TABLE
#include <mach/clocktable.h>
#endif

#define INITIAL_STATE		AUTO_HOTPLUG_IDLE //AUTO_HOTPLUG_DISABLED
#define UP_DELAY_MS			500
#define DOWN_DELAY_MS		2000

#define HOTPLUG_DEFALUT_FREQ	(850*1000)

enum {
	AUTO_HOTPLUG_DISABLED = 0,
	AUTO_HOTPLUG_IDLE,
	AUTO_HOTPLUG_DOWN,
	AUTO_HOTPLUG_UP,
};

enum {
	TCC_CPU_SPEED_BALANCED,
	TCC_CPU_SPEED_BIASED,
	TCC_CPU_SPEED_SKEWED,
};

static struct mutex *tcc_cpu_lock;

static struct workqueue_struct *hotplug_wq;
static struct delayed_work hotplug_work;

static unsigned long up_delay;
static unsigned long down_delay;

#if (0)
static struct {
	cputime64_t time_up_total;
	u64 last_update;
	unsigned int up_down_count;
} hotplug_stats[CONFIG_NR_CPUS];
#endif

static int hotplug_state;
static int forced_multicore_status;
static void hotplug_init_stats(void)
{
#if (0)
	int i;
	u64 cur_jiffies = get_jiffies_64();

	for (i = 0; i <= CONFIG_NR_CPUS; i++) {
		hotplug_stats[i].time_up_total = 0;
		hotplug_stats[i].last_update = cur_jiffies;

		hotplug_stats[i].up_down_count = 0;
		if ((i < nr_cpu_ids) && cpu_online(i))
			hotplug_stats[i].up_down_count = 1;
	}
#endif
}

static void hotplug_stats_update(unsigned int cpu, bool up)
{
#if (0)
	u64 cur_jiffies = get_jiffies_64();
	bool was_up = hotplug_stats[cpu].up_down_count & 0x1;

	if (was_up)
		hotplug_stats[cpu].time_up_total = cputime64_add(
			hotplug_stats[cpu].time_up_total, cputime64_sub(
				cur_jiffies, hotplug_stats[cpu].last_update));

	if (was_up != up) {
		hotplug_stats[cpu].up_down_count++;
		if ((hotplug_stats[cpu].up_down_count & 0x1) != up) {
			/* FIXME: sysfs user space CPU control breaks stats */
			pr_err("hotplug stats out of sync with %s CPU%d",
			       (cpu < CONFIG_NR_CPUS) ? "G" : "LP",
			       (cpu < CONFIG_NR_CPUS) ?  cpu : 0);
			hotplug_stats[cpu].up_down_count ^=  0x1;
		}
	}
	hotplug_stats[cpu].last_update = cur_jiffies;
#endif
}

static noinline int tcc_cpu_speed_balance(void)
{
	unsigned long curr_speed = tcc_getspeed(0);

	if (curr_speed < HOTPLUG_DEFALUT_FREQ)
		return TCC_CPU_SPEED_SKEWED;
	else if (curr_speed > HOTPLUG_DEFALUT_FREQ)
		return TCC_CPU_SPEED_BALANCED;

	return TCC_CPU_SPEED_BIASED;
}
void disable_auto_hotplug(void)
{
	hotplug_state=AUTO_HOTPLUG_DISABLED;
	cancel_delayed_work(&hotplug_work);
}
EXPORT_SYMBOL(disable_auto_hotplug);

static void auto_hotplug_work_func(struct work_struct *work)
{
	bool up = false;
	unsigned long now = jiffies;
	static unsigned long last_change_time;

	mutex_lock(tcc_cpu_lock);

	//printk("%s: hotplug_state:%d\n", __func__, hotplug_state);
	switch (hotplug_state) {
	case AUTO_HOTPLUG_DISABLED:
	case AUTO_HOTPLUG_IDLE:
		break;
	case AUTO_HOTPLUG_DOWN:
		up = false;
		queue_delayed_work_on(0,
			hotplug_wq, &hotplug_work, down_delay);
		break;
	case AUTO_HOTPLUG_UP:
		switch (tcc_cpu_speed_balance()) {
		/* cpu speed is up and balanced - one more on-line */
		case TCC_CPU_SPEED_BALANCED:
			up = true;
			break;
		/* cpu speed is up, but skewed - remove one core */
		case TCC_CPU_SPEED_SKEWED:
			up = false;
			break;
		/* cpu speed is up, but under-utilized - do nothing */
		case TCC_CPU_SPEED_BIASED:
		default:
			break;
		}
		queue_delayed_work_on(0,
			hotplug_wq, &hotplug_work, up_delay);
		break;
	default:
		pr_err("%s: invalid hotplug state %d\n",
		       __func__, hotplug_state);
	}

	if (!up && ((now - last_change_time) < down_delay))
			goto end_auto_hotplug_work_func;

	last_change_time = now;

	if (forced_multicore_status)
		up = true;

	hotplug_stats_update(1, up);
	mutex_unlock(tcc_cpu_lock);

	if (up){
#if defined(CONFIG_CLOCK_TABLE)
		clocktable_process(0);
#endif
		cpu_up(1);
	}else{
		cpu_down(1);
#if defined(CONFIG_CLOCK_TABLE)
		clocktable_process(1);
#endif
	}
	return;

end_auto_hotplug_work_func:
	mutex_unlock(tcc_cpu_lock);
}

void auto_hotplug_governor(unsigned int cpu_freq, bool suspend)
{
	if (hotplug_state == AUTO_HOTPLUG_DISABLED)
		return;

	if (suspend) {
		hotplug_state = AUTO_HOTPLUG_IDLE;
		return;
	}

	switch (hotplug_state) {
	case AUTO_HOTPLUG_IDLE:
		if (cpu_freq > HOTPLUG_DEFALUT_FREQ) {
			hotplug_state = AUTO_HOTPLUG_UP;
			queue_delayed_work_on(0,
				hotplug_wq, &hotplug_work, up_delay);
		} else if (cpu_freq <= HOTPLUG_DEFALUT_FREQ) {
			hotplug_state = AUTO_HOTPLUG_DOWN;
			queue_delayed_work_on(0,
				hotplug_wq, &hotplug_work, down_delay);
		}
		break;
	case AUTO_HOTPLUG_DOWN:
		if (cpu_freq > HOTPLUG_DEFALUT_FREQ) {
			hotplug_state = AUTO_HOTPLUG_UP;
			queue_delayed_work_on(0,
				hotplug_wq, &hotplug_work, up_delay);
		}
		break;
	case AUTO_HOTPLUG_UP:
		if (cpu_freq <= HOTPLUG_DEFALUT_FREQ) {
			hotplug_state = AUTO_HOTPLUG_DOWN;
			queue_delayed_work_on(0,
				hotplug_wq, &hotplug_work, down_delay);
		}
		break;
	default:
		pr_err("%s: invalid tegra hotplug state %d\n",
		       __func__, hotplug_state);
		BUG();
	}
}

void auto_hotplug_set_forced_multicore(unsigned int enable)
{
	if (enable)
		forced_multicore_status = true;
	else
		forced_multicore_status = false;
}

int auto_hotplug_init(struct mutex *cpu_lock)
{
	/*
	 * Not bound to the issuer CPU (=> high-priority), has rescue worker
	 * task, single-threaded, freezable.
	 */
	//printk("%s: nr_cpu_ids:%d\n", __func__, nr_cpu_ids);

	hotplug_wq = alloc_workqueue(
		"auto-hotplug", WQ_UNBOUND | WQ_RESCUER | WQ_FREEZABLE, 1);
	if (!hotplug_wq)
		return -ENOMEM;
	INIT_DELAYED_WORK(&hotplug_work, auto_hotplug_work_func);

	up_delay = msecs_to_jiffies(UP_DELAY_MS);
	down_delay = msecs_to_jiffies(DOWN_DELAY_MS);

	tcc_cpu_lock = cpu_lock;
	hotplug_state = INITIAL_STATE;
	forced_multicore_status = false;
	hotplug_init_stats();
	pr_info("auto-hotplug initialized: %s\n",
		(hotplug_state == AUTO_HOTPLUG_DISABLED) ? "disabled" : "enabled");
	return 0;
}

void auto_hotplug_exit(void)
{
	destroy_workqueue(hotplug_wq);
}
