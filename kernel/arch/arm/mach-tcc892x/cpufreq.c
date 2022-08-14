/*
 * arch/arm/mach-tcc892x/cpufreq.c
 *
 * TCC892x cpufreq driver
 *
 * Copyright (C) 2012 Telechips, Inc.
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
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/suspend.h>
#include <linux/debugfs.h>
#include <linux/cpu.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include <mach/bsp.h>
#include <mach/gpio.h>
#ifdef CONFIG_CLOCK_TABLE
#include <mach/clocktable.h>
#endif

struct tcc_cpufreq_table_t {
	unsigned int cpu_freq;
	unsigned int voltage;
};

static struct tcc_cpufreq_table_t tcc_cpufreq_table[] = {
	/*  CPU    Voltage */
	{ 343750, 1000000 },
	{ 406250, 1050000 },
	{ 468750, 1100000 },
	{ 546875, 1150000 },
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 625)
	{ 625000, 1200000 },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 716)
	{ 716500, 1250000 },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 808)
	{ 808000, 1300000 },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 833)
	{ 833000, 1320000 },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 996)
	{ 996800, 1425000 },	// Do not recommended.
#endif
};

#define NUM_FREQS	ARRAY_SIZE(tcc_cpufreq_table)

static struct cpufreq_frequency_table freq_table_opp[NUM_FREQS + 1];
static struct cpufreq_frequency_table *freq_table;
static struct clk *cpu_clk;

static unsigned long policy_max_speed;
static unsigned long target_cpu_speed;
static DEFINE_MUTEX(tcc_cpu_lock);

static bool is_suspended;
static int suspend_index;
static bool force_policy_max;

static int tcc_update_cpu_speed(unsigned long rate);
static unsigned long tcc_cpu_highest_speed(void);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)

#if (CONFIG_HIGHSPEED_LIMITED_CLOCK >= CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK)
	#error "Time Limited Clock is higher than HighSpeed Clock"
#endif

#define DEBUG_HIGHSPEED 0
#define dbg_highspeed(msg...)	if (DEBUG_HIGHSPEED) { printk( "TCC_HIGHSPEED: " msg); }

#define TCC_CPU_FREQ_HIGH_SPEED			(CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK*1000)
#define TCC_CPU_FREQ_LIMITED_SPEED		(CONFIG_HIGHSPEED_LIMITED_CLOCK*1000)

enum cpu_highspeed_status_t {
	CPU_FREQ_PROCESSING_NORMAL = 0,
	CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED,
	CPU_FREQ_PROCESSING_HIGHSPEED,
	CPU_FREQ_PROCESSING_MAX
};

#define HIGHSPEED_TIMER_TICK			100  // 100 ms.
#define HIGHSPEED_LIMIT_CLOCK_PERIOD	((CONFIG_HIGHSPEED_LIMITED_TIME*1000)/HIGHSPEED_TIMER_TICK)
#define HIGHSPEED_HIGER_CLOCK_PERIOD	((CONFIG_HIGHSPEED_HIGHSPEED_TIME*1000)/HIGHSPEED_TIMER_TICK)

static int highspeed_highspeed_mode = 0;
static int highspeed_highclock_counter = 0;
static int highspeed_lowclock_counter = 0;
static int highspeed_limit_highspeed_counter = 0;

static struct workqueue_struct *cpufreq_wq;
static struct work_struct cpufreq_work;
static enum cpu_highspeed_status_t cpu_highspeed_status = CPU_FREQ_PROCESSING_NORMAL;
static struct timer_list timer_highspeed;

static int tcc_cpufreq_is_limit_highspeed_status(void)
{
	if ( 0
	  || cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED
	) {
		return 1;
	}
	
	return 0;
}

static void cpufreq_work_func(struct work_struct *work)
{
	dbg_highspeed("######### %s\n", __func__);

	mutex_lock(&tcc_cpu_lock);

	if (cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED) {
		tcc_update_cpu_speed(TCC_CPU_FREQ_LIMITED_SPEED);
	}
	else if(cpu_highspeed_status == CPU_FREQ_PROCESSING_HIGHSPEED) {
	 	if (tcc_cpufreq_is_limit_highspeed_status())
			tcc_update_cpu_speed(TCC_CPU_FREQ_LIMITED_SPEED);
		else
			tcc_update_cpu_speed(TCC_CPU_FREQ_HIGH_SPEED);
	}

	mutex_unlock(&tcc_cpu_lock);
}

static void highspeed_reset_setting_values(void)
{
 	highspeed_highspeed_mode = 0;
	highspeed_highclock_counter = 0;
	highspeed_lowclock_counter = 0;
	highspeed_limit_highspeed_counter = 0;
	cpu_highspeed_status = CPU_FREQ_PROCESSING_NORMAL;
}

static void highspeed_timer_func(unsigned long data)
{
	int status_changed = 0;
	unsigned int noraml_speed_clock;

	noraml_speed_clock = TCC_CPU_FREQ_LIMITED_SPEED;

	// increase counters
	if (cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED)
		highspeed_limit_highspeed_counter++;
 	else if (highspeed_highspeed_mode) {
		if (tcc_cpu_highest_speed() > noraml_speed_clock)
			highspeed_highclock_counter++;
		else
			highspeed_lowclock_counter++;
 	}

	// start highspeed process
	if (cpu_highspeed_status == CPU_FREQ_PROCESSING_NORMAL && tcc_cpu_highest_speed() > noraml_speed_clock) {
		cpu_highspeed_status = CPU_FREQ_PROCESSING_HIGHSPEED;
		highspeed_highspeed_mode = 1;
		highspeed_highclock_counter = 0;
		highspeed_lowclock_counter = 0;
		dbg_highspeed("######### Change to highspeed status\n");
	}

	// during over clock status
	else if (cpu_highspeed_status == CPU_FREQ_PROCESSING_HIGHSPEED) {
		if (highspeed_highclock_counter >= HIGHSPEED_HIGER_CLOCK_PERIOD && highspeed_lowclock_counter < HIGHSPEED_HIGER_CLOCK_PERIOD) {
			cpu_highspeed_status = CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED;
			highspeed_limit_highspeed_counter = highspeed_lowclock_counter;	// optional.  add lowclock_counter or not.
			highspeed_highclock_counter = 0;
			highspeed_lowclock_counter = 0;
			dbg_highspeed("######### Change to limit highspeed status\n");
			status_changed = 1;
		}
		else if (highspeed_lowclock_counter >= HIGHSPEED_HIGER_CLOCK_PERIOD) {
			highspeed_reset_setting_values();
			dbg_highspeed("######### Change to normal status\n");
			status_changed = 1;
 		}
	}

	// during limit over clock status
	else if (cpu_highspeed_status == CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED) {
		if (highspeed_limit_highspeed_counter >= HIGHSPEED_LIMIT_CLOCK_PERIOD) {
			highspeed_reset_setting_values();
			dbg_highspeed("######### Change to normal status\n");
			status_changed = 1;
		}
 	}

	// unkown status
	else if (cpu_highspeed_status != CPU_FREQ_PROCESSING_NORMAL) {
		highspeed_reset_setting_values();
		dbg_highspeed("######### Change to normal status (dummy)\n");
		status_changed = 1;
	}

	if (status_changed)
		queue_work(cpufreq_wq, &cpufreq_work);

	timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
	add_timer(&timer_highspeed);
}
#endif /* CONFIG_HIGHSPEED_TIME_LIMIT */

static int tcc_verify_speed(struct cpufreq_policy *policy)
{
	if (!freq_table)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, freq_table);
}

unsigned int tcc_getspeed(unsigned int cpu)
{
	return clk_get_rate(cpu_clk) / 1000;
}

static int tcc_update_cpu_speed(unsigned long rate)
{
	int ret = 0;
	struct cpufreq_freqs freqs;
#ifdef CONFIG_CLOCK_TABLE
	unsigned int func_cpu_rate;
#endif

	freqs.old = tcc_getspeed(0);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	if (rate > TCC_CPU_FREQ_LIMITED_SPEED) {
		if (tcc_cpufreq_is_limit_highspeed_status())
			rate = TCC_CPU_FREQ_LIMITED_SPEED;
	}

#if (DEBUG_HIGHSPEED)
	if (freqs.old > TCC_CPU_FREQ_LIMITED_SPEED && rate <= TCC_CPU_FREQ_LIMITED_SPEED ) {
		dbg_highspeed("change to %dMHz\n", TCC_CPU_FREQ_LIMITED_SPEED/1000);
	}
	else if (freqs.old <= TCC_CPU_FREQ_LIMITED_SPEED && rate > TCC_CPU_FREQ_LIMITED_SPEED ) {
		dbg_highspeed("change to %dMHz\n", TCC_CPU_FREQ_HIGH_SPEED/1000);
	}
#endif
#endif

#ifdef CONFIG_CLOCK_TABLE
	func_cpu_rate = clocktable_getcpu();
	rate = (rate < func_cpu_rate) ? func_cpu_rate : rate;
#endif

	freqs.new = rate;

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

#ifdef CONFIG_CLOCK_TABLE
	clocktable_update(freqs.new);
#else
	ret = clk_set_rate(cpu_clk, freqs.new * 1000);
	if (ret) {
		pr_err("%s: Failed to set cpu frequency to %d kHz\n", __func__, freqs.new);
		goto failed_cpu_clk_set_rate;
	}
#endif

	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	return 0;

failed_cpu_clk_set_rate:
	return ret;
}

static unsigned long tcc_cpu_highest_speed(void)
{
	unsigned long policy_max = ULONG_MAX;
	unsigned long rate = 0;

	if (force_policy_max)
		policy_max = min(policy_max, policy_max_speed);
	rate = max(rate, target_cpu_speed);
	rate = min(rate, policy_max);
	return rate;
}

static int tcc_cpu_set_speed_cap(unsigned int *speed_cap)
{
	int ret = 0;
	unsigned int new_speed = tcc_cpu_highest_speed();

	if (is_suspended)
		return -EBUSY;

	if (speed_cap)
		*speed_cap = new_speed;

	ret = tcc_update_cpu_speed(new_speed);
	return ret;
}

static int tcc_target(struct cpufreq_policy *policy,
				  unsigned int target_freq,
				  unsigned int relation)
{
	int idx;
	unsigned int freq;
	unsigned int new_speed;
	int ret = 0;

	mutex_lock(&tcc_cpu_lock);

	ret = cpufreq_frequency_table_target(policy, freq_table, target_freq,
		relation, &idx);
	if (ret)
		goto _out;

	freq = freq_table[idx].frequency;

	target_cpu_speed = freq;
	ret = tcc_cpu_set_speed_cap(&new_speed);
_out:
	mutex_unlock(&tcc_cpu_lock);

	return ret;
}

static int tcc_pm_notify(struct notifier_block *nb, unsigned long event,
	void *dummy)
{
	mutex_lock(&tcc_cpu_lock);
	if (event == PM_SUSPEND_PREPARE) {
#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
		highspeed_reset_setting_values();
#endif
		is_suspended = true;
		pr_info("TCC cpufreq suspend: setting frequency to %d kHz\n",
			freq_table[suspend_index].frequency);
		tcc_update_cpu_speed(freq_table[suspend_index].frequency);
#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
		del_timer(&timer_highspeed);
#endif
	} else if (event == PM_POST_SUSPEND) {
		unsigned int freq;
		is_suspended = false;
		tcc_cpu_set_speed_cap(&freq);
		pr_info("TCC cpufreq resume: restoring frequency to %d kHz\n",
			freq);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
		timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
		add_timer(&timer_highspeed);
#endif
	}
	mutex_unlock(&tcc_cpu_lock);

	return NOTIFY_OK;
}

static struct notifier_block tcc_cpu_pm_notifier = {
	.notifier_call = tcc_pm_notify,
};

static int tcc_cpu_init(struct cpufreq_policy *policy)
{
	mutex_lock(&tcc_cpu_lock);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	INIT_WORK(&cpufreq_work, cpufreq_work_func);
#endif

	cpu_clk = clk_get(NULL, "cpu");
	if (IS_ERR(cpu_clk))
		return PTR_ERR(cpu_clk);

	clk_enable(cpu_clk);

	cpufreq_frequency_table_cpuinfo(policy, freq_table);
	cpufreq_frequency_table_get_attr(freq_table, policy->cpu);
	policy->cur = tcc_getspeed(policy->cpu);
	target_cpu_speed = policy->cur;

	/* FIXME: what's the actual transition time? */
	policy->cpuinfo.transition_latency = 100 * 1000;

	policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
	cpumask_copy(policy->related_cpus, cpu_possible_mask);

	register_pm_notifier(&tcc_cpu_pm_notifier);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	init_timer(&timer_highspeed);
	timer_highspeed.data = 0;
	timer_highspeed.function = highspeed_timer_func;
	timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
	add_timer(&timer_highspeed);
#endif
	mutex_unlock(&tcc_cpu_lock);

	return 0;
}

static int tcc_cpu_exit(struct cpufreq_policy *policy)
{
	clk_put(cpu_clk);
	return 0;
}

static int tcc_cpufreq_policy_notifier(struct notifier_block *nb, unsigned long event, void *data)
{
	int i, ret;
	struct cpufreq_policy *policy = data;

	if (event == CPUFREQ_NOTIFY) {
		ret = cpufreq_frequency_table_target(policy, freq_table, policy->max, CPUFREQ_RELATION_H, &i);
		policy_max_speed = ret ? policy->max : freq_table[i].frequency;
	}
	return NOTIFY_OK;
}

static struct notifier_block tcc_cpufreq_policy_nb = {
	.notifier_call = tcc_cpufreq_policy_notifier,
};

static struct freq_attr *tcc_cpufreq_attr[] = {
	&cpufreq_freq_attr_scaling_available_freqs,
	NULL,
};

static struct cpufreq_driver tcc_driver = {
	.verify	= tcc_verify_speed,
	.target	= tcc_target,
	.get	= tcc_getspeed,
	.init	= tcc_cpu_init,
	.exit	= tcc_cpu_exit,
	.name	= "tcc",
	.attr	= tcc_cpufreq_attr,
};

static int __init tcc_cpufreq_init(void)
{
	int ret, i;

	for (i = 0; i < NUM_FREQS; i++) {
		freq_table_opp[i].index = i;
		freq_table_opp[i].frequency = tcc_cpufreq_table[i].cpu_freq;
	}
	freq_table_opp[i].frequency = CPUFREQ_TABLE_END;
	freq_table = freq_table_opp;

#if defined(CONFIG_CLOCK_TABLE)
	clocktable_init(&tcc_cpu_lock);
#endif

	ret = cpufreq_register_notifier(&tcc_cpufreq_policy_nb, CPUFREQ_POLICY_NOTIFIER);
	if (ret)
		return ret;

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	cpufreq_wq = create_singlethread_workqueue("cpufreq_wq");
	if (!cpufreq_wq)
		return -ENOMEM;
#endif

	return cpufreq_register_driver(&tcc_driver);
}

static void __exit tcc_cpufreq_exit(void)
{
#if defined(CONFIG_CLOCK_TABLE)
	clocktable_exit();
#endif
#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	destroy_workqueue(cpufreq_wq);
#endif
	cpufreq_unregister_driver(&tcc_driver);
	cpufreq_unregister_notifier(&tcc_cpufreq_policy_nb, CPUFREQ_POLICY_NOTIFIER);
}

MODULE_DESCRIPTION("cpufreq driver for TCC892X SOCs");
MODULE_LICENSE("GPL");
module_init(tcc_cpufreq_init);
module_exit(tcc_cpufreq_exit);
