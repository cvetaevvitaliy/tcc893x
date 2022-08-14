/****************************************************************************
 * arch/arm/mach-tcc893x/tcc893x_smp.c
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

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/smp_scu.h>
#include <asm/unified.h>
#include <asm/hardware/gic.h>	// It is possible for this file to be changed next.
//#include <asm/memory.h>

#ifdef CONFIG_ARM_TRUSTZONE
#include <mach/smc.h>
#endif

#define TCC_SCU_BASE_ADDR 0x77200000
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define SEC_CPU_START_ADDR 0xF5400104
#define SEC_CPU_START_CFG  0xF5400100
#else
#define SEC_CPU_START_ADDR 0xF000CDF8
#define SEC_CPU_START_CFG  0xF000CDFC
#endif

/* 
 * Contents for booting secondary CPU
 */
extern void tcc893x_secondary_startup(void);

static DEFINE_SPINLOCK(boot_lock);
/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen"
 */
volatile int pen_release = -1;

/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not. This is neccessary for the hotplug code to work reliably.
 */
static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range( __pa(&pen_release), __pa(&pen_release + 1));
}


#if 0
int __cpuinit local_timer_setup(struct clock_event_device *evt)
{
	return 0;
}
#endif

void platform_secondary_init(unsigned int cpu)
{
	/*
	 * Depending on chip vendor, this function is used to initialize specific code and/or function 
	 * such as gic interrupt initialization for secondary cpu when secodary cpu is started.
	 */
    gic_secondary_init(0);

	/*
	 * let the primary processor knows we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

static void __iomem *scu_base_addr(void)
{
	return (void __iomem *)tcc_p2v(TCC_SCU_BASE_ADDR);
}


int boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	/*
	 * no.3
	 * kernel_init()[ps1] -> smp_init() -> cpu_up() -> _cpu_up() -> __cpu_up() -> boot_secondary()
	 */
	unsigned long timeout;
	unsigned long magic_num = 0;

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one.
	 */
	magic_num = __raw_readl(SEC_CPU_START_CFG);
//	printk("Before waking up the secondary cpu, SEC_CPU_START_CFG value is %ld\n", 
//			magic_num);

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * This is really belt and braces; we hold uninteded secondary
	 * CPUs in the holding pen until we're ready for them. However,
	 * since we haven't sent them a soft interrupt, they shouldn't 
	 * be there.
	 */
	write_pen_release(cpu);

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flgs register,
	 * and branch to the address found there.
	 */
	smp_wmb();
	dsb_sev();	// Awake the secondary cpu from WFE mode.
	mb();
	gic_raise_softirq(cpumask_of(cpu), 1); // Awake the secondary cpu from WFI mode.
	mb();

	/*
	 * After sev instruction is sent, 
	 * the secondary cpu is still in chip boot code.
	 * To jump to linux kernel, 
	 * SEC_CPU_START_CFG[7:4] register must be set to non-zero.
	 */
#if !(defined(CONFIG_CLOCK_TABLE) && !defined(CONFIG_AUTO_HOTPLUG))
#ifdef CONFIG_ARM_TRUSTZONE
	_tz_smc(SMC_CMD_SMP_SECONDARY_CFG, 0, 0, 0);
#else
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	__raw_writel(0x1, SEC_CPU_START_CFG);
#else
	__raw_writel(0x10, SEC_CPU_START_CFG);
#endif
#endif
#endif
	smp_wmb();

	timeout = jiffies + HZ;
	while (time_before(jiffies, timeout)) {
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/* 
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

void platform_smp_prepare_cpus(unsigned int max_cpus)
{
	/*	no.2
	 *	kernel_init()[ps1] -> smp_prepare_cpus()
	 *	Enable scu, 
	 *	and then write start addr for secondary cpu via SEC_CPU_START_ADDR register. 
	 */
	scu_enable(scu_base_addr());

	/*
	 * Write the address of secondary startup into the
	 * system-wide flags register. The BootMonitor waits
	 * until it receives a soft interrupt, and then the
	 * secondary CPU branches to this address.
	 */
#ifdef CONFIG_ARM_TRUSTZONE
	_tz_smc(SMC_CMD_SMP_SECONDARY_ADDR, (virt_to_phys(tcc893x_secondary_startup)), 0, 0);
#else
	__raw_writel((virt_to_phys(tcc893x_secondary_startup)), SEC_CPU_START_ADDR);
#endif
	
}

void smp_init_cpus(void)
{
	/*
	 * no.1
	 * start_kernel()[ps0] -> setup_arch() -> smp_init_cpus()
	 * Get cpu number from scu, and then set cpu mask & gic_raise_softirq.
	 */

	void __iomem *scu_base = scu_base_addr();
	unsigned int i, ncores;

	ncores = scu_base ? scu_get_core_count(scu_base) : 1;

	if (ncores > nr_cpu_ids) {
		pr_warn("SMP : %u cores greather than maximum(%u), clipping\n",
				ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);

	set_smp_cross_call(gic_raise_softirq);
}

int local_timer_ack(void)
{
	  return 0;
}

