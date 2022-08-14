/*
 * arch/arm/mach-tcc893x/cpufreq.c
 *
 * TCC893x cpufreq driver
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
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <mach/bsp.h>
#include <mach/gpio.h>
#ifdef CONFIG_CLOCK_TABLE
#include <mach/clocktable.h>
#endif
#include "tcc_cpufreq.h"

struct tcc_cpufreq_table_t {
	unsigned int cpu_freq;
	unsigned int voltage;
};

#if defined(CONFIG_TCC_CORE_VOLTAGE_OFFSET) && defined(CONFIG_REGULATOR)
	#define CORE_VOLTAGE_OFFSET	(CONFIG_TCC_CORE_VOLTAGE_OFFSET * 1000)
#else
	#define CORE_VOLTAGE_OFFSET	(0)
#endif

//#define CORE_SUSPEND_VOLTAGE	1050000

static struct tcc_cpufreq_table_t tcc_cpufreq_table[] = {
	/*  CPU    Voltage */
	{ 342000, 1050000+CORE_VOLTAGE_OFFSET },
	{ 424000, 1050000+CORE_VOLTAGE_OFFSET },
	{ 524000, 1050000+CORE_VOLTAGE_OFFSET },
	{ 626000, 1050000+CORE_VOLTAGE_OFFSET },
	{ 728000, 1100000+CORE_VOLTAGE_OFFSET },
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 850)
	{ 850000, 1175000+CORE_VOLTAGE_OFFSET },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 892)
	{ 892000, 1200000+CORE_VOLTAGE_OFFSET },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 996)
	{ 996000, 1275000+CORE_VOLTAGE_OFFSET },
#endif
#if (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK >= 1200)
	{1200000, 1450000+CORE_VOLTAGE_OFFSET },		// Not recommanded.
#endif
};

#define NUM_FREQS	ARRAY_SIZE(tcc_cpufreq_table)

static struct cpufreq_frequency_table freq_table_opp[NUM_FREQS + 1];
static struct cpufreq_frequency_table *freq_table;
static struct clk *cpu_clk;
static struct clk *hsio_clk;
#if defined(CONFIG_REGULATOR) && !defined(CONFIG_CLOCK_TABLE)
//#define SUSPEND_CORE_VOLTAGE	1250000
#define SUSPEND_CORE_VOLTAGE	0 // 0: Not used
#if (SUSPEND_CORE_VOLTAGE)
static struct regulator *vdd_core = NULL;
static unsigned int core_voltage;
#endif
#else
#define SUSPEND_CORE_VOLTAGE	0 // 0: Not used
#endif

#if defined(CONFIG_IOBUS_DFS)
static struct clk *max_io_clk;
static int max_io_enabled = 0;
#endif

static unsigned long policy_max_speed[CONFIG_NR_CPUS];
static unsigned long target_cpu_speed[CONFIG_NR_CPUS];
static DEFINE_MUTEX(tcc_cpu_lock);

static bool is_suspended;
static int suspend_index;
static bool force_policy_max;

#ifdef CONFIG_REGULATOR
static struct regulator *vdd_cpu;
#endif
static unsigned int curr_voltage;


static int tcc_update_cpu_speed(unsigned long rate);
static unsigned long tcc_cpu_highest_speed(void);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)

#if (CONFIG_HIGHSPEED_LIMITED_CLOCK >= CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK)
	#error "Time Limited Clock is higher than HighSpeed Clock"
#endif

#define DEBUG_HIGHSPEED 0
#define dbg_highspeed(msg...)	if (DEBUG_HIGHSPEED) { printk( "TCC_HIGHSPEED: " msg); }

#define TCC_CPU_FREQ_HIGH_SPEED        (CONFIG_TCC_CPUFREQ_HIGHSPEED_CLOCK*1000)
#define TCC_CPU_FREQ_LIMITED_SPEED      (CONFIG_HIGHSPEED_LIMITED_CLOCK*1000)

enum cpu_highspeed_status_t {
	CPU_FREQ_PROCESSING_NORMAL = 0,
	CPU_FREQ_PROCESSING_LIMIT_HIGHSPEED,
	CPU_FREQ_PROCESSING_HIGHSPEED,
	CPU_FREQ_PROCESSING_MAX
};

#define HIGHSPEED_TIMER_TICK             100                                // 100 ms.
#define HIGHSPEED_LIMIT_CLOCK_PERIOD     ((CONFIG_HIGHSPEED_LIMITED_TIME*1000)/HIGHSPEED_TIMER_TICK)
#define HIGHSPEED_HIGER_CLOCK_PERIOD     ((CONFIG_HIGHSPEED_HIGHSPEED_TIME*1000)/HIGHSPEED_TIMER_TICK)

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
		status_changed = 1;
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

static unsigned int tcc_get_voltage(unsigned int freq)
{
	int i;
	for (i=0 ; i<NUM_FREQS ; i++) {
		if (freq <= tcc_cpufreq_table[i].cpu_freq)
			return tcc_cpufreq_table[i].voltage;			
	}
	return tcc_cpufreq_table[NUM_FREQS-1].voltage;
}

#if defined(CONFIG_GPIO_CORE_VOLTAGE_CONTROL)
static unsigned int tcc_cpufreq_set_voltage_by_gpio(int uV)
{
	if(machine_is_tcc8930st()) {
		#if defined(CONFIG_STB_BOARD_EV_TCC893X) || defined(CONFIG_STB_BOARD_YJ8930T)
			if( uV > 1050000 ) {
				//CORE1_ON == 1, CORE2_ON == 1 ==> 1.30V
				gpio_set_value(TCC_GPB(19), 1);
				gpio_set_value(TCC_GPB(21), 1);
			}
			else {
				//CORE1_ON == 0, CORE2_ON == 1 ==> 1.10V
				gpio_set_value(TCC_GPB(19), 0);
				gpio_set_value(TCC_GPB(21), 1);
			}
        #elif defined(CONFIG_STB_BOARD_YJ8933T)			
            if( uV > 1050000 ) {
				//CORE1_ON == 1, CORE2_ON == 1 ==> 1.30V
				gpio_set_value(TCC_GPG(15), 1);
				gpio_set_value(TCC_GPG(16), 1);
			}
			else {
				//CORE1_ON == 0, CORE2_ON == 1 ==> 1.10V
				gpio_set_value(TCC_GPG(15), 0);
				gpio_set_value(TCC_GPG(16), 1);
			}
		#elif defined(CONFIG_STB_BOARD_UPC_TCC893X)
			if( uV > 1050000 ) {
				//CORE0_ON == 1 ==> 1.30V
				gpio_set_value(TCC_GPG(15), 1);
			}
			else {
				//CORE0_ON == 0 ==> 1.10V
				gpio_set_value(TCC_GPG(15), 0);
			}
		#elif defined(CONFIG_STB_BOARD_DONGLE_TCC893X)
			if( uV > 1050000 ) {
				// V0.1 : CORE0_ON == 1 ==> 1.10V
				// V0.2 : CORE1_ON == 1 ==> 1.25V
				gpio_set_value(TCC_GPG(14), 1);
				gpio_set_value(TCC_GPG(15), 1);
			}
			else {
				// V0.1 : CORE0_ON == 0 ==> 1.10V
				// V0.2 : CORE1_ON == 0 ==> 1.05V
				gpio_set_value(TCC_GPG(14), 1);
				gpio_set_value(TCC_GPG(15), 0);
			}
		#elif defined(CONFIG_STB_BOARD_YJ8935T)
			//CORE0_ON == 1 ==> 1.30V
			gpio_set_value(TCC_GPG(14), 1);
		#endif
		udelay(100);
	}

	return 0;
}
#endif


static int tcc_verify_speed(struct cpufreq_policy *policy)
{
	if (!freq_table)
		return -EINVAL;

	return cpufreq_frequency_table_verify(policy, freq_table);
}

unsigned int tcc_getspeed(unsigned int cpu)
{
	if (cpu >= NR_CPUS)
		return 0;

	return clk_get_rate(cpu_clk) / 1000;
}

static int tcc_update_cpu_speed(unsigned long rate)
{
	int ret = 0;
	struct cpufreq_freqs freqs;
	unsigned int new_voltage;
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

	new_voltage = tcc_get_voltage(rate);
	freqs.new = rate;

	if (curr_voltage < new_voltage) {
#if defined(CONFIG_REGULATOR)
		if (vdd_cpu)
			regulator_set_voltage(vdd_cpu, new_voltage, new_voltage);
#elif defined(CONFIG_GPIO_CORE_VOLTAGE_CONTROL)
		tcc_cpufreq_set_voltage_by_gpio(new_voltage);
#endif
	}

	for_each_online_cpu(freqs.cpu)
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

	ret = clk_set_rate(cpu_clk, freqs.new * 1000);
	if (ret) {
		pr_err("%s: Failed to set cpu frequency to %d kHz\n", __func__, freqs.new);
		goto failed_cpu_clk_set_rate;
	}
#ifdef CONFIG_CLOCK_TABLE
	clocktable_update(freqs.new);
#endif

	for_each_online_cpu(freqs.cpu)
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

#if defined(CONFIG_IOBUS_DFS)
	if (max_io_clk) {
		if ((freqs.new > 425000) && (max_io_enabled==0)) {
			clk_enable(max_io_clk);
			max_io_enabled = 1;
		}
		else if ((freqs.new <= 425000) && max_io_enabled) {
			clk_disable(max_io_clk);
			max_io_enabled = 0;
		}
	}
#endif

	if (curr_voltage > new_voltage) {
#if defined(CONFIG_REGULATOR)
		if (vdd_cpu)
			regulator_set_voltage(vdd_cpu, new_voltage, new_voltage);
#elif defined(CONFIG_GPIO_CORE_VOLTAGE_CONTROL)
		tcc_cpufreq_set_voltage_by_gpio(new_voltage);
#endif
	}

	curr_voltage = new_voltage;

	return 0;

failed_cpu_clk_set_rate:
	curr_voltage = new_voltage;
	return ret;
}

static unsigned long tcc_cpu_highest_speed(void)
{
	unsigned long policy_max = ULONG_MAX;
	unsigned long rate = 0;
	int i;

	for_each_online_cpu(i) {
		if (force_policy_max)
			policy_max = min(policy_max, policy_max_speed[i]);
		rate = max(rate, target_cpu_speed[i]);
	}
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
#if defined(CONFIG_AUTO_HOTPLUG)
	if (ret == 0)
		auto_hotplug_governor(new_speed, false);
#endif
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

	target_cpu_speed[policy->cpu] = freq;
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
#if defined(CONFIG_AUTO_HOTPLUG)
		auto_hotplug_governor(freq_table[suspend_index].frequency, true);
#endif
#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
		del_timer(&timer_highspeed);
#endif
		/* fix the rebooting issue when system running resume process. */
		if (hsio_clk)
			clk_enable(hsio_clk);
#if (SUSPEND_CORE_VOLTAGE)
		if (vdd_core)
			regulator_set_voltage(vdd_core, SUSPEND_CORE_VOLTAGE, SUSPEND_CORE_VOLTAGE);
#endif
	} else if (event == PM_POST_SUSPEND) {
		unsigned int freq = 0;
		is_suspended = false;
#if (SUSPEND_CORE_VOLTAGE)
		if (vdd_core)
			regulator_set_voltage(vdd_core, core_voltage, core_voltage);
#endif
		/* fix the rebooting issue when system running resume process. */
		if (hsio_clk)
			clk_disable(hsio_clk);
		tcc_cpu_set_speed_cap(&freq);
		pr_info("TCC cpufreq resume: restoring frequency to %d kHz\n", freq);

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
	if (policy->cpu >= CONFIG_NR_CPUS)
		return -EINVAL;

	mutex_lock(&tcc_cpu_lock);

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	if (policy->cpu == 0) {
		INIT_WORK(&cpufreq_work, cpufreq_work_func);
	}
#endif

#if defined(CONFIG_REGULATOR)
	if (policy->cpu == 0) {
		int i;
		if (vdd_cpu == NULL) {
			vdd_cpu = regulator_get(NULL, "vdd_cpu");
			if (IS_ERR(vdd_cpu)) {
				pr_warning("cpufreq: failed to obtain vdd_cpu\n");
				vdd_cpu = NULL;
			}
			else
				curr_voltage = regulator_get_voltage(vdd_cpu);
		}
		if (!vdd_cpu) {
			for (i=0 ; i< NUM_FREQS ; i++) {
				if (tcc_cpufreq_table[i].voltage > 1200000)
					break;
			}
			freq_table_opp[i].frequency = CPUFREQ_TABLE_END;
		}
	}
#endif

	cpu_clk = clk_get(NULL, "cpu");
	if (IS_ERR(cpu_clk))
		return PTR_ERR(cpu_clk);

	clk_enable(cpu_clk);

	if (policy->cpu == 0) {
		/* fix the rebooting issue when system running resume process. */
		hsio_clk = clk_get(NULL, "hsio");
		if (IS_ERR(hsio_clk))
			hsio_clk = NULL;
#if (SUSPEND_CORE_VOLTAGE)
		if (vdd_core == NULL) {
			vdd_core = regulator_get(NULL, "vdd_core");
			if (IS_ERR(vdd_core))
				vdd_core = NULL;
			else
				core_voltage = regulator_get_voltage(vdd_core);
		}
#endif
	}

#if defined(CONFIG_IOBUS_DFS)
	max_io_clk = clk_get(NULL, "max_io_clk");
	if (IS_ERR(max_io_clk))
		max_io_clk = NULL;

	if (max_io_clk && (max_io_enabled==0)) {
		clk_enable(max_io_clk);
		max_io_enabled = 1;
	}
#endif

	cpufreq_frequency_table_cpuinfo(policy, freq_table);
	cpufreq_frequency_table_get_attr(freq_table, policy->cpu);
	policy->cur = tcc_getspeed(policy->cpu);
	target_cpu_speed[policy->cpu] = policy->cur;

	/* FIXME: what's the actual transition time? */
	policy->cpuinfo.transition_latency = 300 * 1000;

	policy->shared_type = CPUFREQ_SHARED_TYPE_ALL;
	cpumask_copy(policy->related_cpus, cpu_possible_mask);

	if (policy->cpu == 0) {
		register_pm_notifier(&tcc_cpu_pm_notifier);
	}

#if defined(CONFIG_HIGHSPEED_TIME_LIMIT)
	if (policy->cpu == 0) {
	    init_timer(&timer_highspeed);
	    timer_highspeed.data = 0;
	    timer_highspeed.function = highspeed_timer_func;
	    timer_highspeed.expires = jiffies + msecs_to_jiffies(HIGHSPEED_TIMER_TICK);	// 100 milisec.
	    add_timer(&timer_highspeed);
	}
#endif
	mutex_unlock(&tcc_cpu_lock);

	return 0;
}

static int tcc_cpu_exit(struct cpufreq_policy *policy)
{
#if defined(CONFIG_IOBUS_DFS)
	if (max_io_clk)
		clk_put(max_io_clk);
#endif

	clk_put(cpu_clk);
	return 0;
}

static int tcc_cpufreq_policy_notifier(struct notifier_block *nb, unsigned long event, void *data)
{
	int i, ret;
	struct cpufreq_policy *policy = data;

	if (event == CPUFREQ_NOTIFY) {
		ret = cpufreq_frequency_table_target(policy, freq_table, policy->max, CPUFREQ_RELATION_H, &i);
		policy_max_speed[policy->cpu] =	ret ? policy->max : freq_table[i].frequency;
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

#if (1)
	suspend_index = 0;
#else
	for (i = 0; i < NUM_FREQS; i++) {
		if (tcc_cpufreq_table[i].voltage >= CORE_SUSPEND_VOLTAGE) {
			suspend_index = i;
			break;
		}
	}
#endif

#if defined(CONFIG_AUTO_HOTPLUG)
	ret = auto_hotplug_init(&tcc_cpu_lock);
	if (ret)
		return ret;
#endif

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
#if defined(CONFIG_AUTO_HOTPLUG)
	auto_hotplug_exit();
#endif
	cpufreq_unregister_driver(&tcc_driver);
	cpufreq_unregister_notifier(&tcc_cpufreq_policy_nb, CPUFREQ_POLICY_NOTIFIER);
}

MODULE_DESCRIPTION("cpufreq driver for TCC893X SOCs");
MODULE_LICENSE("GPL");
module_init(tcc_cpufreq_init);
module_exit(tcc_cpufreq_exit);
