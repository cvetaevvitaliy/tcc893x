/****************************************************************************
 * arch/arm/mach-tcc893x/membus_scaling.c
 * Copyright (C) 2013 Telechips Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <plat/nand.h>
#include <linux/delay.h>
#include <mach/tcc_ckc.h>

struct memclk_table_t {
	unsigned int cpu_freq;
	unsigned int mem_freq;
};
static struct memclk_table_t memclk_table[] = {
	/*  CPU    MEM  */
//	{ 342000, 300000 },
	{ 424000, 300000 },
	{ 524000, 372000 },
	{ 626000, 443250 },
	{ 728000, 514500 },
	{ 850000, 600000 },
};
#define NUM_FREQS	ARRAY_SIZE(memclk_table)

static struct workqueue_struct *membus_wq;
static struct delayed_work membus_work;
//static struct work_struct membus_work;

static struct clk *mem_clk;
static struct mutex *cpu_lock;
static DEFINE_MUTEX(tcc_memclk_lock);
static unsigned int memclk_change_enabled = 0;

static void membus_work_func(struct work_struct *work)
{
	unsigned int i;
#ifndef CONFIG_AUTO_HOTPLUG
	int ret;
#endif

	mutex_lock(&tcc_memclk_lock);

#ifdef CONFIG_AUTO_HOTPLUG
	if (num_online_cpus() > 1 || (memclk_change_enabled == 0))
		goto finish_memclk_change;
#else
	if (num_online_cpus() > 1) {
		ret = cpu_down(1);
		if (ret) {
			printk("%s:, cpu_down failed ret:0x%x\n", __func__, ret);
			goto finish_memclk_change;
		}
	}
#endif

	if (NUM_FREQS > 1) {
		for (i=(NUM_FREQS-1) ; i ; i--) {
			if (memclk_table[i].cpu_freq <= (tca_ckc_getfbusctrl(0)/10))
				break;
		}
	}
	else if (NUM_FREQS == 1)
		i = 0;
	else
		goto finish_memclk_change;

	tcc_nand_lock();
	clk_set_rate(mem_clk, (memclk_table[i].mem_freq * 1000));
	tcc_nand_unlock();

#ifndef CONFIG_AUTO_HOTPLUG
	if (num_online_cpus() <= 1) {
		ret = cpu_up(1);
		if (ret)
			printk("%s:, cpu_up failed ret:0x%x\n", __func__, ret);
	}
	msleep(300);
#endif

finish_memclk_change:
	mutex_unlock(&tcc_memclk_lock);
}

static int membus_cpufreq_transition_notifier(struct notifier_block *nb, unsigned long event, void *data)
{
	if (event == CPUFREQ_POSTCHANGE)
		queue_delayed_work_on(0, membus_wq, &membus_work, 1/*msecs_to_jiffies(100)*/);

	return NOTIFY_OK;
}

static struct notifier_block membus_cpufreq_transition_nb = {
	.notifier_call = membus_cpufreq_transition_notifier,
};

int membus_clock_process(unsigned int enable)
{
	int ret = 0;
	unsigned int curr_memclk;
	mutex_lock(&tcc_memclk_lock);

	memclk_change_enabled = enable;	
	if (memclk_change_enabled)
		goto check_membus_clock_finish;

	if (num_online_cpus() > 1 ) {
		ret = -1;
		goto check_membus_clock_finish;
	}
	curr_memclk = clk_get_rate(mem_clk);
	if (curr_memclk < 600*1000*1000)
		clk_set_rate(mem_clk, 600*1000*1000);

check_membus_clock_finish:
	mutex_unlock(&tcc_memclk_lock);
	return ret;
}

int membus_scaling_init(struct mutex *tcc_cpu_lock)
{
	int ret;

	memclk_change_enabled = 0;

	mem_clk = clk_get(NULL, "membus");
	if (IS_ERR(mem_clk))
		mem_clk = NULL;

	membus_wq = alloc_workqueue("membus_wq", WQ_UNBOUND | WQ_RESCUER | WQ_FREEZABLE, 1);
	if (!membus_wq)
		return -ENOMEM;
	INIT_DELAYED_WORK(&membus_work, membus_work_func);
//	INIT_WORK(&membus_work, membus_work_func);

	ret = cpufreq_register_notifier(&membus_cpufreq_transition_nb, CPUFREQ_TRANSITION_NOTIFIER);
	if (ret)
		return ret;

	cpu_lock = tcc_cpu_lock;

	return 0;
}

void membus_scaling_exit(void)
{
	if (mem_clk)
		clk_put(mem_clk);

	destroy_workqueue(membus_wq);
}

