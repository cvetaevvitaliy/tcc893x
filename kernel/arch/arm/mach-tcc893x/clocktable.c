/****************************************************************************
 * arch/arm/mach-tcc893x/clocktable.c
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

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/consumer.h>
#include <plat/nand.h>
#include <mach/bsp.h>
#include <mach/clocktable.h>
#include "clocktable_list.h"
#include "tcc_cpufreq.h"

#ifdef CONFIG_CLOCK_TABLE

#define CLOCKTABLE_DEBUG 0
#if (CLOCKTABLE_DEBUG)
//#define CLOCKTABLE_DEBUG_DETAIL
#endif


#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define CORE_VOLTAGE_OFFSET     25000
#else
#define CORE_VOLTAGE_OFFSET     0
#endif
#define CORE_VOLTAGE_UP_DELAY   0        // 1000     // usec
#define MIN_CORE_VOLTAGE        1050000  // (1050000+CORE_VOLTAGE_OFFSET)
//#define SUSPEND_CORE_VOLTAGE    1250000
#define SUSPEND_CORE_VOLTAGE    0 // 0: Not used
#define NUM_VOLTAGES    ARRAY_SIZE(voltage_table)

static DEFINE_MUTEX(clktbl_lock);
#define CLOCK_TABLE_LOCK(X)     mutex_lock(&clktbl_lock)
#define CLOCK_TABLE_UNLOCK(X)   mutex_unlock(&clktbl_lock)

static LIST_HEAD(clktbl_list);
static struct clock_table_t     curr_clktbl;
static struct clock_table_t     func_clktbl;
static struct clock_table_t     orig_clktbl;
static struct clk               *cpu_clk;
static struct clk               *mem_clk;
static struct clk               *ddi_clk;
static struct clk               *gpu_clk;
static struct clk               *io_clk;
static struct clk               *vbus_clk;
static struct clk               *vcore_clk;
static struct clk               *hsio_clk;
static struct clk               *smu_clk;
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
static struct clk               *g2d_clk;
static struct clk               *cm3_clk;
#endif
#ifdef CONFIG_REGULATOR
static struct regulator         *vdd_core = NULL;
#endif
static unsigned int             curr_core_voltage;
static unsigned int             is_suspended = 0;
static unsigned int             clktbl_enabled = 0;
static unsigned int             cpufreq_actived = 0;
static struct func_clktbl_t     *normal_clktbl = NULL;
static struct mutex             *cpu_lock;

static inline unsigned int find_core_voltage(struct clock_table_t *clktbl)
{
    int i;

    for (i = 0 ; i < NUM_VOLTAGES ; i++) {
        if ( 1
            && voltage_table[i].clks.mem_clk   >= clktbl->mem_clk
            && voltage_table[i].clks.ddi_clk   >= clktbl->ddi_clk
            && voltage_table[i].clks.gpu_clk   >= clktbl->gpu_clk
            && voltage_table[i].clks.io_clk    >= clktbl->io_clk
            && voltage_table[i].clks.vbus_clk  >= clktbl->vbus_clk
            && voltage_table[i].clks.vcore_clk >= clktbl->vcore_clk
            && voltage_table[i].clks.smu_clk   >= clktbl->smu_clk
            && voltage_table[i].clks.hsio_clk  >= clktbl->hsio_clk
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
            && voltage_table[i].clks.g2d_clk   >= clktbl->g2d_clk
            && voltage_table[i].clks.cm3_clk   >= clktbl->cm3_clk
#endif
            )
            break;
    }
    if (i >= NUM_VOLTAGES)
        i = NUM_VOLTAGES - 1;

    return (voltage_table[i].voltage+CORE_VOLTAGE_OFFSET);
}

static inline void restore_original_clocktable(void)
{
    unsigned int new_core_voltage;

    if (num_online_cpus() > 1 )
        return;

    new_core_voltage = find_core_voltage(&orig_clktbl);
    if (new_core_voltage < MIN_CORE_VOLTAGE)
        new_core_voltage = MIN_CORE_VOLTAGE;

    if (new_core_voltage > curr_core_voltage) {
#ifdef CONFIG_REGULATOR
        if (vdd_core)
            regulator_set_voltage(vdd_core, new_core_voltage, new_core_voltage);
#if (CORE_VOLTAGE_UP_DELAY)
	udelay(CORE_VOLTAGE_UP_DELAY);
#endif
#endif
    }
    tcc_nand_lock();
    clk_set_rate(mem_clk,   orig_clktbl.mem_clk+100);
    clk_set_rate(io_clk,    orig_clktbl.io_clk+100);
    tcc_nand_unlock();
    clk_set_rate(ddi_clk,   orig_clktbl.ddi_clk+100);
    clk_set_rate(gpu_clk,   orig_clktbl.gpu_clk+100);
    clk_set_rate(vbus_clk,  orig_clktbl.vbus_clk+100);
    clk_set_rate(vcore_clk, orig_clktbl.vcore_clk+100);
    clk_set_rate(hsio_clk,  orig_clktbl.hsio_clk+100);
    clk_set_rate(smu_clk,   orig_clktbl.smu_clk+100);
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    clk_set_rate(g2d_clk,   orig_clktbl.g2d_clk+100);
    clk_set_rate(cm3_clk,   orig_clktbl.cm3_clk+100);
#endif
    if (new_core_voltage < curr_core_voltage) {
#ifdef CONFIG_REGULATOR
        if (vdd_core)
            regulator_set_voltage(vdd_core, new_core_voltage, new_core_voltage);
#endif
    }
    curr_core_voltage = new_core_voltage;
}

inline static void update_clocktable(void)
{
    unsigned int new_core_voltage;
    static struct clock_table_t new_clktbl;

    new_clktbl.cpu_clk = (curr_clktbl.cpu_clk > func_clktbl.cpu_clk) ? curr_clktbl.cpu_clk : func_clktbl.cpu_clk;
    new_clktbl.mem_clk = (curr_clktbl.mem_clk > func_clktbl.mem_clk) ? curr_clktbl.mem_clk : func_clktbl.mem_clk;
    new_clktbl.ddi_clk = (curr_clktbl.ddi_clk > func_clktbl.ddi_clk) ? curr_clktbl.ddi_clk : func_clktbl.ddi_clk;
    new_clktbl.gpu_clk = (curr_clktbl.gpu_clk > func_clktbl.gpu_clk) ? curr_clktbl.gpu_clk : func_clktbl.gpu_clk;
    new_clktbl.io_clk = (curr_clktbl.io_clk > func_clktbl.io_clk) ? curr_clktbl.io_clk : func_clktbl.io_clk;
    new_clktbl.vbus_clk = (curr_clktbl.vbus_clk > func_clktbl.vbus_clk) ? curr_clktbl.vbus_clk : func_clktbl.vbus_clk;
    new_clktbl.vcore_clk = (curr_clktbl.vcore_clk > func_clktbl.vcore_clk) ? curr_clktbl.vcore_clk : func_clktbl.vcore_clk;
    new_clktbl.smu_clk = (curr_clktbl.smu_clk > func_clktbl.smu_clk) ? curr_clktbl.smu_clk : func_clktbl.smu_clk;
    new_clktbl.hsio_clk = (curr_clktbl.hsio_clk > func_clktbl.hsio_clk) ? curr_clktbl.hsio_clk : func_clktbl.hsio_clk;
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    new_clktbl.g2d_clk = (curr_clktbl.g2d_clk > func_clktbl.g2d_clk) ? curr_clktbl.g2d_clk : func_clktbl.g2d_clk;
    new_clktbl.cm3_clk = (curr_clktbl.cm3_clk > func_clktbl.cm3_clk) ? curr_clktbl.cm3_clk : func_clktbl.cm3_clk;
#endif

    new_core_voltage = find_core_voltage(&new_clktbl);
    if (new_core_voltage < MIN_CORE_VOLTAGE)
        new_core_voltage = MIN_CORE_VOLTAGE;

    if (new_core_voltage > curr_core_voltage) {
#ifdef CONFIG_REGULATOR
        if (vdd_core)
            regulator_set_voltage(vdd_core, new_core_voltage, new_core_voltage);
#if (CORE_VOLTAGE_UP_DELAY)
        udelay(CORE_VOLTAGE_UP_DELAY);
#endif
#endif
    }

    tcc_nand_lock();
    clk_set_rate(mem_clk,   new_clktbl.mem_clk*1000);
    clk_set_rate(io_clk,    new_clktbl.io_clk*1000);
    tcc_nand_unlock();
    clk_set_rate(ddi_clk,   new_clktbl.ddi_clk*1000);
    clk_set_rate(gpu_clk,   new_clktbl.gpu_clk*1000);
    clk_set_rate(vbus_clk,  new_clktbl.vbus_clk*1000);
    clk_set_rate(vcore_clk, new_clktbl.vcore_clk*1000);
    clk_set_rate(smu_clk,   new_clktbl.smu_clk*1000);
    clk_set_rate(hsio_clk,  new_clktbl.hsio_clk*1000);
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    clk_set_rate(g2d_clk,   new_clktbl.g2d_clk*1000);
    clk_set_rate(cm3_clk,   new_clktbl.cm3_clk*1000);
#endif
    if (new_core_voltage < curr_core_voltage) {
#ifdef CONFIG_REGULATOR
        if (vdd_core)
            regulator_set_voltage(vdd_core, new_core_voltage, new_core_voltage);
#endif
    }
    curr_core_voltage = new_core_voltage;
}

int clocktable_process(unsigned int enable)
{
    int ret = 0, i;
    unsigned int cpu_rate;
    CLOCK_TABLE_LOCK();

    if (is_suspended)
        goto check_membus_clock_finish;

    clktbl_enabled = enable;    
    if (clktbl_enabled) {
        cpu_rate = tca_ckc_getfbusctrl(0)/10;
        if (normal_clktbl) {
            for (i = (normal_clktbl->num_clktbls - 1) ; i ; i--) {
                if (cpu_rate >= normal_clktbl->clktbls[i].cpu_clk)
                    break;
            }
            if (i >= normal_clktbl->num_clktbls)
                i = normal_clktbl->num_clktbls - 1;
            memcpy(&curr_clktbl, &(normal_clktbl->clktbls[i]), sizeof(struct clock_table_t));
        }
        else
            goto check_membus_clock_finish;
        update_clocktable();
    } else {
        restore_original_clocktable();
    }
check_membus_clock_finish:
    CLOCK_TABLE_UNLOCK();
    return ret;
}
EXPORT_SYMBOL(clocktable_process);

unsigned int clocktable_getcpu(void)
{
    if (!clktbl_enabled || is_suspended)
        return 0;

    return func_clktbl.cpu_clk;
}
EXPORT_SYMBOL(clocktable_getcpu);

int clocktable_update(unsigned int cpu_rate)
{
    int i;

    CLOCK_TABLE_LOCK();

    if (!cpufreq_actived)
        cpufreq_actived = 1;

    if (is_suspended)
        goto finish_clktbl_process;

    if (!clktbl_enabled)
        goto finish_clktbl_process;

    if (normal_clktbl) {
        for (i = (normal_clktbl->num_clktbls - 1) ; i ; i--) {
            if (cpu_rate >= normal_clktbl->clktbls[i].cpu_clk)
                break;
        }
        if (i >= normal_clktbl->num_clktbls)
            i = normal_clktbl->num_clktbls - 1;
        memcpy(&curr_clktbl, &(normal_clktbl->clktbls[i]), sizeof(struct clock_table_t));
    }
    else
        goto finish_clktbl_process;

    update_clocktable();

finish_clktbl_process:
    CLOCK_TABLE_UNLOCK();
    return 0;
}
EXPORT_SYMBOL(clocktable_update);

void restart_clock_prepare(void)
{
	CLOCK_TABLE_LOCK();
        if (vdd_core) {  // restore core voltage to 1.2v
            regulator_set_voltage(vdd_core, 1200000, 1200000);
            curr_core_voltage = 1200000;
        }

	is_suspended = 1;
	restore_original_clocktable();
	CLOCK_TABLE_UNLOCK();
}
EXPORT_SYMBOL(restart_clock_prepare);

		
static int clocktable_pm_notify(struct notifier_block *nb, unsigned long event, void *dummy)
{
    CLOCK_TABLE_LOCK();
    if (event == PM_SUSPEND_PREPARE) {
        is_suspended = 1;
        restore_original_clocktable();
#ifdef CONFIG_REGULATOR
#if (SUSPEND_CORE_VOLTAGE)
        if (vdd_core) {
            regulator_set_voltage(vdd_core, SUSPEND_CORE_VOLTAGE, SUSPEND_CORE_VOLTAGE);
            curr_core_voltage = SUSPEND_CORE_VOLTAGE;
        }
#endif
#endif
    } else if (event == PM_POST_SUSPEND) {
        is_suspended = 0;
    }
    CLOCK_TABLE_UNLOCK();
    return NOTIFY_OK;
}

static struct notifier_block clocktable_pm_notifier = {
    .notifier_call = clocktable_pm_notify,
};

int clocktable_init(struct mutex *tcc_cpu_lock)
{
    int ret;

    is_suspended = 0;
    register_pm_notifier(&clocktable_pm_notifier);

#ifdef CONFIG_AUTO_HOTPLUG
    clktbl_enabled = 0;
#else
    clktbl_enabled = 1;
#endif
    cpu_lock = tcc_cpu_lock;
    memset(&curr_clktbl, 0x0, sizeof(struct clock_table_t));
    memset(&func_clktbl, 0x0, sizeof(struct clock_table_t));

    normal_clktbl = clocktable_get("normal_clktbl");
    if (IS_ERR(normal_clktbl))
        normal_clktbl = NULL;

    cpu_clk = clk_get(NULL, "cpu");
    if (IS_ERR(cpu_clk)) {
        cpu_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    mem_clk = clk_get(NULL, "membus");
    if (IS_ERR(mem_clk)) {
        mem_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.mem_clk = clk_get_rate(mem_clk)*2;
    ddi_clk = clk_get(NULL, "ddi");
    if (IS_ERR(ddi_clk)) {
        ddi_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.ddi_clk = clk_get_rate(ddi_clk);
    gpu_clk = clk_get(NULL, "gpu");
    if (IS_ERR(gpu_clk)) {
        gpu_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.gpu_clk = clk_get_rate(gpu_clk);
    io_clk = clk_get(NULL, "iobus");
    if (IS_ERR(io_clk)) {
        io_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.io_clk = clk_get_rate(io_clk);
    vbus_clk = clk_get(NULL, "vbus");
    if (IS_ERR(vbus_clk)) {
        vbus_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.vbus_clk = clk_get_rate(vbus_clk);
    vcore_clk = clk_get(NULL, "vcore");
    if (IS_ERR(vcore_clk)) {
        vcore_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.vcore_clk = clk_get_rate(vcore_clk);
    smu_clk = clk_get(NULL, "smu");
    if (IS_ERR(smu_clk)) {
        smu_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.smu_clk = clk_get_rate(smu_clk);
    hsio_clk = clk_get(NULL, "hsio");
    if (IS_ERR(hsio_clk)) {
        hsio_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.hsio_clk = clk_get_rate(hsio_clk);
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    g2d_clk = clk_get(NULL, "g2d");
    if (IS_ERR(g2d_clk)) {
        g2d_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.g2d_clk = clk_get_rate(g2d_clk);
    cm3_clk = clk_get(NULL, "cortex-m3");
    if (IS_ERR(cm3_clk)) {
        cm3_clk = NULL; ret = -ENODEV;
        goto failed_get_clks;
    }
    orig_clktbl.cm3_clk = clk_get_rate(cm3_clk);
#endif

#if defined(CONFIG_REGULATOR)
    if (vdd_core == NULL) {
        vdd_core = regulator_get(NULL, "vdd_core");
        if (IS_ERR(vdd_core)) {
            pr_warning("clock_table: failed to obtain vdd_core\n");
            vdd_core = NULL;
        }
        else
            curr_core_voltage = regulator_get_voltage(vdd_core);
    }
#endif

#if (CLOCKTABLE_DEBUG)
    {
        struct func_clktbl_t *fclktbl_tmp;
        unsigned int bit;
        printk ("\n\n");
        bit = 0;
        list_for_each_entry(fclktbl_tmp, &clktbl_list, list) {
            printk("name: %s, tbls_num:%d, bit:%d (0x%08x)\n", fclktbl_tmp->name, fclktbl_tmp->num_clktbls, bit, 1<<bit);
            bit++;
#ifdef CLOCKTABLE_DEBUG_DETAIL
            if (fclktbl_tmp->num_clktbls) {
                int cnt;
                for (cnt=0; cnt < fclktbl_tmp->num_clktbls ; cnt++) {
                    printk("tlb_%d: cpu:%3d,  mem:%3d, ddi:%3d, gpu:%3d,  io:%3d, vbus:%3d, ", cnt,
                        fclktbl_tmp->clktbls[cnt].cpu_clk/1000, fclktbl_tmp->clktbls[cnt].mem_clk/1000, fclktbl_tmp->clktbls[cnt].ddi_clk/1000,
                        fclktbl_tmp->clktbls[cnt].gpu_clk/1000, fclktbl_tmp->clktbls[cnt].io_clk/1000, fclktbl_tmp->clktbls[cnt].vbus_clk/1000);
                    printk("vcore:%3d, hsio:%3d, smu:%3d, g2d:%3d, cm3:%3d\n",
                        fclktbl_tmp->clktbls[cnt].vcore_clk/1000, fclktbl_tmp->clktbls[cnt].hsio_clk/1000, fclktbl_tmp->clktbls[cnt].smu_clk/1000,
                        fclktbl_tmp->clktbls[cnt].g2d_clk/1000, fclktbl_tmp->clktbls[cnt].cm3_clk/1000);
                }
            }
            printk ("\n");
#endif
        }
        printk ("\n");
    }
#endif
    return 0;

failed_get_clks:
    if (cpu_clk)
        clk_put(cpu_clk);
    if (mem_clk)
        clk_put(mem_clk);
    if (ddi_clk)
        clk_put(ddi_clk);
    if (gpu_clk)
        clk_put(gpu_clk);
    if (io_clk)
        clk_put(io_clk);
    if (vbus_clk)
        clk_put(vbus_clk);
    if (vcore_clk)
        clk_put(vcore_clk);
    if (smu_clk)
        clk_put(smu_clk);
    if (hsio_clk)
        clk_put(hsio_clk);
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    if (g2d_clk)
        clk_put(g2d_clk);
    if (cm3_clk)
        clk_put(cm3_clk);
#endif

    return ret;
}
EXPORT_SYMBOL(clocktable_init);

void clocktable_exit(void)
{
    is_suspended = 0;
#if defined(CONFIG_REGULATOR)
    if (vdd_core)
        regulator_put(vdd_core);
#endif
    if (cpu_clk)
        clk_put(cpu_clk);
    if (mem_clk)
        clk_put(mem_clk);
    if (ddi_clk)
        clk_put(ddi_clk);
    if (gpu_clk)
        clk_put(gpu_clk);
    if (io_clk)
        clk_put(io_clk);
    if (vbus_clk)
        clk_put(vbus_clk);
    if (vcore_clk)
        clk_put(vcore_clk);
    if (smu_clk)
        clk_put(smu_clk);
    if (hsio_clk)
        clk_put(hsio_clk);
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
    if (g2d_clk)
        clk_put(g2d_clk);
    if (cm3_clk)
        clk_put(cm3_clk);
#endif
}
EXPORT_SYMBOL(clocktable_exit);

unsigned int clocktable_ctrl(struct func_clktbl_t *fclktbl, unsigned int tbl_index, clktbl_enable_t enable)
{
    int ret = 0, bit;
    unsigned int  clktbl_flags, cpu_freq_is_changed;
    struct func_clktbl_t *fclktbl_tmp;

    CLOCK_TABLE_LOCK();
    if (fclktbl == NULL) {
        ret = -1;
        goto clocktable_ctrl_finish;
    } else if ((fclktbl->num_clktbls == 0)/* || (tbl_index >= fclktbl->num_clktbls)*/) {
        ret = -1;
        goto clocktable_ctrl_finish;
    }

    if (enable && (tbl_index >= fclktbl->num_clktbls)) {
        printk("%s: name:%s, unvalied index:%d\n", __func__, fclktbl->name, tbl_index);
        tbl_index = fclktbl->num_clktbls - 1;
    }

    if (enable == CLKTBL_ENABLE) {
        if (fclktbl->count && (fclktbl->active_idx == tbl_index)) {
            fclktbl->count++;
            if (fclktbl->count == 0) {
                printk("%s: name:%s, enable count is overflowed\n", __func__, fclktbl->name);
                fclktbl->count++;
            }
            goto clocktable_ctrl_finish;
        }
        fclktbl->count++;
        if (fclktbl->count == 0) {
            printk("%s: name:%s, enable count is overflowed\n", __func__, fclktbl->name);
            fclktbl->count++;
        }
        fclktbl->active_idx = tbl_index;
    } else {
        if (fclktbl->count == 0)
            goto clocktable_ctrl_finish;
        fclktbl->count = 0;
        fclktbl->active_idx = 0;
    }

    memset(&func_clktbl, 0x0, sizeof(struct clock_table_t));
    bit = 0;
    clktbl_flags = 0x0;
    /* rebuilding functional clock table values. */
    list_for_each_entry(fclktbl_tmp, &clktbl_list, list) {
        if (fclktbl_tmp->count) {
            clktbl_flags |= (1<<bit);
            if (func_clktbl.cpu_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].cpu_clk)
                func_clktbl.cpu_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].cpu_clk;
            if (func_clktbl.mem_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].mem_clk)
                func_clktbl.mem_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].mem_clk;
            if (func_clktbl.ddi_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].ddi_clk)
                func_clktbl.ddi_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].ddi_clk;
            if (func_clktbl.gpu_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].gpu_clk)
                func_clktbl.gpu_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].gpu_clk;
            if (func_clktbl.io_clk    < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].io_clk)
                func_clktbl.io_clk    = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].io_clk;
            if (func_clktbl.vbus_clk  < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].vbus_clk)
                func_clktbl.vbus_clk  = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].vbus_clk;
            if (func_clktbl.vcore_clk < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].vcore_clk)
                func_clktbl.vcore_clk = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].vcore_clk;
            if (func_clktbl.smu_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].smu_clk)
                func_clktbl.smu_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].smu_clk;
            if (func_clktbl.hsio_clk  < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].hsio_clk)
                func_clktbl.hsio_clk  = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].hsio_clk;
#if !defined(CONFIG_CHIP_TCC8935S) && !defined(CONFIG_CHIP_TCC8933S) && !defined(CONFIG_CHIP_TCC8937S)
            if (func_clktbl.g2d_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].g2d_clk)
                func_clktbl.g2d_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].g2d_clk;
            if (func_clktbl.cm3_clk   < fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].cm3_clk)
                func_clktbl.cm3_clk   = fclktbl_tmp->clktbls[fclktbl_tmp->active_idx].cm3_clk;
#endif
        }
        bit++;
    }
#if (CLOCKTABLE_DEBUG)
    printk("%s: name:%s: idx:%d, enable:%d, flags:0x%x\n", __func__, fclktbl->name, tbl_index, enable, clktbl_flags);
#ifdef CLOCKTABLE_DEBUG_DETAIL
    printk("%s: cpu:%3d,  mem:%3d, ddi:%3d, gpu:%3d,  io:%3d, vbus:%3d, ", __func__,
        func_clktbl.cpu_clk/1000, func_clktbl.mem_clk/1000, func_clktbl.ddi_clk/1000, func_clktbl.gpu_clk/1000, func_clktbl.io_clk/1000, func_clktbl.vbus_clk/1000);
    printk("vcore:%3d, hsio:%3d, smu:%3d, g2d:%3d, cm3:%3d\n",
        func_clktbl.vcore_clk/1000, func_clktbl.hsio_clk/1000, func_clktbl.smu_clk/1000, func_clktbl.g2d_clk/1000, func_clktbl.cm3_clk/1000);
#endif
#endif

    if (strcmp(fclktbl->name, "hdmi_clktbl") == 0)
        auto_hotplug_set_forced_multicore(fclktbl->count);

    if (is_suspended || !cpufreq_actived)
        goto clocktable_ctrl_finish;

    if (func_clktbl.cpu_clk > curr_clktbl.cpu_clk) {
        clk_set_rate(cpu_clk, func_clktbl.cpu_clk*1000);
        cpu_freq_is_changed = 1;
    }

    if (clktbl_enabled)
        update_clocktable();

clocktable_ctrl_finish:
    CLOCK_TABLE_UNLOCK();

    if (cpu_freq_is_changed) {
        cpufreq_update_policy(smp_processor_id());
        cpu_freq_is_changed = 0;
    }
    return ret;
}
EXPORT_SYMBOL(clocktable_ctrl);

struct func_clktbl_t *clocktable_get(const char *id)
{
    struct func_clktbl_t *fclktbl;
#if (CLOCKTABLE_DEBUG)
    printk("%s: name:%s\n", __func__, id);
#endif
    list_for_each_entry(fclktbl, &clktbl_list, list) {
        if (!strcmp(id, fclktbl->name))
            goto found;
    }
    fclktbl = ERR_PTR(-ENOENT);
found:
    return fclktbl;
}
EXPORT_SYMBOL(clocktable_get);

void clocktable_put(struct func_clktbl_t *fclktbl)
{
#if (CLOCKTABLE_DEBUG)
    if ((fclktbl->name) && fclktbl)
        printk("%s: name:%s\n", __func__, fclktbl->name);
    else
        printk("%s: \n", __func__);
#endif
    /* do nothing */
}
EXPORT_SYMBOL(clocktable_put);

static struct func_clktbl_t clktbl_normal = {
    .name           = "normal_clktbl",
    .clktbls        = normal_clktbls,
    .num_clktbls    = ARRAY_SIZE(normal_clktbls),
};

static struct func_clktbl_t clktbl_camera = {
    .name           = "camera_clktbl",
    .clktbls        = camera_clktbls,
    .num_clktbls    = ARRAY_SIZE(camera_clktbls),
};

static struct func_clktbl_t clktbl_isp_camera = {
    .name           = "isp_camera_clktbl",
    .clktbls        = isp_camera_clktbls,
    .num_clktbls    = ARRAY_SIZE(isp_camera_clktbls),
};

static struct func_clktbl_t clktbl_vpu = {
    .name           = "vpu_clktbl",
    .clktbls        = vpu_clktbls,
    .num_clktbls    = ARRAY_SIZE(vpu_clktbls),
};

static struct func_clktbl_t clktbl_vpu_480p = {
    .name           = "vpu_480p_clktbl",
    .clktbls        = vpu_480p_clktbls,
    .num_clktbls    = ARRAY_SIZE(vpu_480p_clktbls),
};

static struct func_clktbl_t clktbl_vpu_720p = {
    .name           = "vpu_720p_clktbl",
    .clktbls        = vpu_720p_clktbls,
    .num_clktbls    = ARRAY_SIZE(vpu_720p_clktbls),
};

static struct func_clktbl_t clktbl_vpu_1080p = {
    .name           = "vpu_1080p_clktbl",
    .clktbls        = vpu_1080p_clktbls,
    .num_clktbls    = ARRAY_SIZE(vpu_1080p_clktbls),
};

static struct func_clktbl_t clktbl_hwc_ddi = {
    .name           = "hwc_ddi_clktbl",
    .clktbls        = hwc_ddi_clktbls,
    .num_clktbls    = ARRAY_SIZE(hwc_ddi_clktbls),
};

static struct func_clktbl_t clktbl_jpeg = {
    .name           = "jpeg_clktbl",
    .clktbls        = jpeg_clktbls,
    .num_clktbls    = ARRAY_SIZE(jpeg_clktbls),
};

static struct func_clktbl_t clktbl_hdmi = {
    .name           = "hdmi_clktbl",
    .clktbls        = hdmi_clktbls,
    .num_clktbls    = ARRAY_SIZE(hdmi_clktbls),
};

static struct func_clktbl_t clktbl_mali = {
    .name           = "mali_clktbl",
    .clktbls        = mali_clktbls,
    .num_clktbls    = ARRAY_SIZE(mali_clktbls),
};

static struct func_clktbl_t clktbl_fb = {
    .name           = "fb_clktbl",
    .clktbls        = fb_clktbls,
    .num_clktbls    = ARRAY_SIZE(fb_clktbls),
};

static struct func_clktbl_t clktbl_overlay = {
    .name           = "overlay_clktbl",
    .clktbls        = overlay_clktbls,
    .num_clktbls    = ARRAY_SIZE(overlay_clktbls),
};

static struct func_clktbl_t clktbl_bt = {
    .name           = "bt_clktbl",
    .clktbls        = bt_clktbls,
    .num_clktbls    = ARRAY_SIZE(bt_clktbls),
};

static struct func_clktbl_t clktbl_usb = {
    .name           = "usb_clktbl",
    .clktbls        = usb_clktbls,
    .num_clktbls    = ARRAY_SIZE(usb_clktbls),
};

static struct func_clktbl_t clktbl_remocon = {
    .name           = "remocon_clktbl",
    .clktbls        = remocon_clktbls,
    .num_clktbls    = ARRAY_SIZE(remocon_clktbls),
};

static struct func_clktbl_t clktbl_ehci = {
    .name           = "ehci_clktbl",
    .clktbls        = hsio_clktbls,
    .num_clktbls    = ARRAY_SIZE(hsio_clktbls),
};

static struct func_clktbl_t clktbl_xhci = {
    .name           = "xhci_clktbl",
    .clktbls        = hsio_clktbls,
    .num_clktbls    = ARRAY_SIZE(hsio_clktbls),
};

static struct func_clktbl_t clktbl_usb30 = {
    .name           = "usb30_clktbl",
    .clktbls        = hsio_clktbls,
    .num_clktbls    = ARRAY_SIZE(hsio_clktbls),
};

static struct func_clktbl_t clktbl_gmac = {
    .name           = "gmac_clktbl",
    .clktbls        = hsio_clktbls,
    .num_clktbls    = ARRAY_SIZE(hsio_clktbls),
};

static struct func_clktbl_t clktbl_voip = {
    .name           = "voip_clktbl",
    .clktbls        = voip_clktbls,
    .num_clktbls    = ARRAY_SIZE(voip_clktbls),
};

static struct func_clktbl_t clktbl_voip_max = {
    .name           = "voip_max_clktbl",
    .clktbls        = voip_clktbls,
    .num_clktbls    = ARRAY_SIZE(voip_clktbls),
};

static struct func_clktbl_t clktbl_wfd = {
    .name           = "wfd_clktbl",
    .clktbls        = wfd_clktbls,
    .num_clktbls    = ARRAY_SIZE(wfd_clktbls),
};

static struct func_clktbl_t clktbl_tsif = {
    .name           = "tsif_clktbl",
    .clktbls        = tsif_clktbls,
    .num_clktbls    = ARRAY_SIZE(tsif_clktbls),
};

static struct func_clktbl_t clktbl_multi_vpu = {
    .name           = "multi_vpu_clktbl",
    .clktbls        = multi_vpu_clktbls,
    .num_clktbls    = ARRAY_SIZE(multi_vpu_clktbls),
};

static struct func_clktbl_t clktbl_smartcard = {
    .name           = "smartcard_clktbl",
    .clktbls        = smartcard_clktbls,
    .num_clktbls    = ARRAY_SIZE(smartcard_clktbls),
};

static struct func_clktbl_t clktbl_cipher = {
    .name           = "cipher_clktbl",
    .clktbls        = cipher_clktbls,
    .num_clktbls    = ARRAY_SIZE(cipher_clktbls),
};

static struct func_clktbl_t *onchip_clktbls[] = {
    &clktbl_normal,
    &clktbl_camera,
    &clktbl_isp_camera,
    &clktbl_vpu,
    &clktbl_vpu_480p,
    &clktbl_vpu_720p,
    &clktbl_vpu_1080p,
    &clktbl_hwc_ddi,
    &clktbl_jpeg,
    &clktbl_hdmi,
    &clktbl_mali,
    &clktbl_fb,
    &clktbl_overlay,
    &clktbl_bt,
    &clktbl_usb,
    &clktbl_remocon,
    &clktbl_ehci,
//    &clktbl_xhci,
//    &clktbl_usb30,
    &clktbl_gmac,
    &clktbl_voip,
    &clktbl_voip_max,
    &clktbl_wfd,
    &clktbl_tsif,
    &clktbl_multi_vpu,
	&clktbl_smartcard,
	&clktbl_cipher,
};

int clocktable_early_init(void)
{
    struct func_clktbl_t **fclktbl;

    for (fclktbl = onchip_clktbls; fclktbl < onchip_clktbls + ARRAY_SIZE(onchip_clktbls); fclktbl++) {
        (*fclktbl)->count = 0;
        (*fclktbl)->active_idx = 0;
        list_add_tail(&(*fclktbl)->list, &clktbl_list);
    }

    pr_info("clocktable initialized\n");
    return 0;
}
EXPORT_SYMBOL(clocktable_early_init);
#else
unsigned int clocktable_ctrl(struct func_clktbl_t *fclktbl, unsigned int tbl_index, clktbl_enable_t enable)
{
    return 0;
}
EXPORT_SYMBOL(clocktable_ctrl);

struct func_clktbl_t *clocktable_get(const char *id)
{
    return ERR_PTR(-ENODEV);
}
EXPORT_SYMBOL(clocktable_get);

void clocktable_put(struct func_clktbl_t *fclktbl)
{
}
EXPORT_SYMBOL(clocktable_put);
#endif

