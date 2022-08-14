/*
 * kernel/power/hibernate.c - Hibernation (a.k.a suspend-to-disk) support.
 *
 * Copyright (c) 2003 Patrick Mochel
 * Copyright (c) 2003 Open Source Development Lab
 * Copyright (c) 2004 Pavel Machek <pavel@ucw.cz>
 * Copyright (c) 2009 Rafael J. Wysocki, Novell Inc.
 *
 * This file is released under the GPLv2.
 */

#include <linux/export.h>
#include <linux/suspend.h>
#include <linux/syscalls.h>
#include <linux/reboot.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/async.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pm.h>
#include <linux/console.h>
#include <linux/cpu.h>
#include <linux/freezer.h>
#include <linux/gfp.h>
#include <linux/syscore_ops.h>
#include <linux/ctype.h>
#include <linux/genhd.h>
#include <scsi/scsi_scan.h>

#include "power.h"

#include <mach/io.h>
#include "../fs/mount.h"	// K3.4
#include <mach/reg_physical.h>
#include <linux/namei.h>
#include <linux/dcache.h>
//#include <linux/earlysuspend.h>
#include <linux/kmod.h>
#include <linux/vmalloc.h>


/*
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/clk.h>

#include <mach/globals.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
*/

//extern struct mount;
extern void mntput_no_expire(struct mount *mnt);
extern int do_umount(struct mount *mnt, int flags);

//extern volatile PLCDC_CHANNEL pLCDC_FB_CH;
#ifdef hb_dbg
#undef hb_dbg
#endif

#if 0
#define hb_dbg(fmt,arg...)    printk("[HB] "fmt,##arg)
#else
#define hb_dbg(fmt,arg...)
#endif

extern void hibernate_thaw_processes(void);

#ifdef CONFIG_SNAPSHOT_BOOT
	#ifdef CONFIG_NOCOMPRESS_SNAPSHOT
static int nocompress = 1;
	#else
static int nocompress = 0;
	#endif
#else
static int nocompress = 0;
#endif

static int noresume = 0;
static int resume_wait;
static int resume_delay;
static char resume_file[256] = CONFIG_PM_STD_PARTITION;
dev_t swsusp_resume_device;
sector_t swsusp_resume_block;
int in_suspend __nosavedata = 0;

extern unsigned int do_hibernation;

unsigned int do_hibernate_boot = 0;
EXPORT_SYMBOL(do_hibernate_boot);

unsigned long long snapshot_restore_start;
unsigned long long profile_dpm_resume;

enum {
	HIBERNATION_INVALID,
	HIBERNATION_PLATFORM,
	HIBERNATION_TEST,
	HIBERNATION_TESTPROC,
	HIBERNATION_SHUTDOWN,
	HIBERNATION_REBOOT,
	/* keep last */
	__HIBERNATION_AFTER_LAST
};
#define HIBERNATION_MAX (__HIBERNATION_AFTER_LAST-1)
#define HIBERNATION_FIRST (HIBERNATION_INVALID + 1)

static int hibernation_mode = HIBERNATION_REBOOT;

bool freezer_test_done;

static const struct platform_hibernation_ops *hibernation_ops;

/**
 * hibernation_set_ops - Set the global hibernate operations.
 * @ops: Hibernation operations to use in subsequent hibernation transitions.
 */
void hibernation_set_ops(const struct platform_hibernation_ops *ops)
{
	if (ops && !(ops->begin && ops->end &&  ops->pre_snapshot
				&& ops->prepare && ops->finish && ops->enter && ops->pre_restore
				&& ops->restore_cleanup && ops->leave)) {
		WARN_ON(1);
		return;
	}
	lock_system_sleep();
	hibernation_ops = ops;
	if (ops)
		hibernation_mode = HIBERNATION_PLATFORM;
	else if (hibernation_mode == HIBERNATION_PLATFORM)
		hibernation_mode = HIBERNATION_SHUTDOWN;

	unlock_system_sleep();
}

static bool entering_platform_hibernation;

bool system_entering_hibernation(void)
{
	return entering_platform_hibernation;
}
EXPORT_SYMBOL(system_entering_hibernation);

#ifdef CONFIG_PM_DEBUG
static void hibernation_debug_sleep(void)
{
	printk(KERN_INFO "hibernation debug: Waiting for 5 seconds.\n");
	mdelay(5000);
}

static int hibernation_testmode(int mode)
{
	    if (hibernation_mode == mode) {
			hibernation_debug_sleep();
			return 1;
	    }
	    return 0;
}

static int hibernation_test(int level)
{
	if (pm_test_level == level) {
		hibernation_debug_sleep();
		return 1;
	}
	return 0;
}
#else /* !CONFIG_PM_DEBUG */
static int hibernation_testmode(int mode) { return 0; }
static int hibernation_test(int level) { return 0; }
#endif /* !CONFIG_PM_DEBUG */

/**
 * platform_begin - Call platform to start hibernation.
 * @platform_mode: Whether or not to use the platform driver.
 */
static int platform_begin(int platform_mode)
{
	return (platform_mode && hibernation_ops) ?
		hibernation_ops->begin() : 0;
}

/**
 * platform_end - Call platform to finish transition to the working state.
 * @platform_mode: Whether or not to use the platform driver.
 */
static void platform_end(int platform_mode)
{
	if (platform_mode && hibernation_ops)
		hibernation_ops->end();
}

/**
 * platform_pre_snapshot - Call platform to prepare the machine for hibernation.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Use the platform driver to prepare the system for creating a hibernate image,
 * if so configured, and return an error code if that fails.
 */

static int platform_pre_snapshot(int platform_mode)
{
	return (platform_mode && hibernation_ops) ?
		hibernation_ops->pre_snapshot() : 0;
}

/**
 * platform_leave - Call platform to prepare a transition to the working state.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Use the platform driver prepare to prepare the machine for switching to the
 * normal mode of operation.
 *
 * This routine is called on one CPU with interrupts disabled.
 */
static void platform_leave(int platform_mode)
{
	if (platform_mode && hibernation_ops)
		hibernation_ops->leave();
}

/**
 * platform_finish - Call platform to switch the system to the working state.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Use the platform driver to switch the machine to the normal mode of
 * operation.
 *
 * This routine must be called after platform_prepare().
 */
static void platform_finish(int platform_mode)
{
	if (platform_mode && hibernation_ops)
		hibernation_ops->finish();
}

/**
 * platform_pre_restore - Prepare for hibernate image restoration.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Use the platform driver to prepare the system for resume from a hibernation
 * image.
 *
 * If the restore fails after this function has been called,
 * platform_restore_cleanup() must be called.
 */
static int platform_pre_restore(int platform_mode)
{
	return (platform_mode && hibernation_ops) ?
		hibernation_ops->pre_restore() : 0;
}

/**
 * platform_restore_cleanup - Switch to the working state after failing restore.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Use the platform driver to switch the system to the normal mode of operation
 * after a failing restore.
 *
 * If platform_pre_restore() has been called before the failing restore, this
 * function must be called too, regardless of the result of
 * platform_pre_restore().
 */
static void platform_restore_cleanup(int platform_mode)
{
	if (platform_mode && hibernation_ops)
		hibernation_ops->restore_cleanup();
}

/**
 * platform_recover - Recover from a failure to suspend devices.
 * @platform_mode: Whether or not to use the platform driver.
 */
static void platform_recover(int platform_mode)
{
	if (platform_mode && hibernation_ops && hibernation_ops->recover)
		hibernation_ops->recover();
}

/**
 * swsusp_show_speed - Print time elapsed between two events during hibernation.
 * @start: Starting event.
 * @stop: Final event.
 * @nr_pages: Number of memory pages processed between @start and @stop.
 * @msg: Additional diagnostic message to print.
 */
void swsusp_show_speed(struct timeval *start, struct timeval *stop,
		unsigned nr_pages, char *msg)
{
	s64 elapsed_centisecs64;
	int centisecs;
	int k;
	int kps;

	elapsed_centisecs64 = timeval_to_ns(stop) - timeval_to_ns(start);
	do_div(elapsed_centisecs64, NSEC_PER_SEC / 100);
	centisecs = elapsed_centisecs64;
	if (centisecs == 0)
		centisecs = 1;	/* avoid div-by-zero */
	k = nr_pages * (PAGE_SIZE / 1024);
	kps = (k * 100) / centisecs;
	printk(KERN_INFO "PM: %s %d kbytes in %d.%02d seconds (%d.%02d MB/s)\n",
			msg, k,
			centisecs / 100, centisecs % 100,
			kps / 1000, (kps % 1000) / 10);
}

/*
 *  swsusp_show_speed - print the time elapsed between two events.
 *  @start: Starting event.
 *  @stop: Final event.
 *  @nr_pages - number of pages processed between @start and @stop
 *  @msg -      introductory message to print
 */

void swsusp_show_speed1(s64 elapsed_centisecs64,
		            unsigned nr_pages, char *msg)
{
    int centisecs;
    int k;
    int kps;

    do_div(elapsed_centisecs64, NSEC_PER_SEC / 100);
    centisecs = elapsed_centisecs64;
    if (centisecs == 0)
        centisecs = 1;  /* avoid div-by-zero */
	    k = nr_pages * (PAGE_SIZE / 1024);
	    kps = (k * 100) / centisecs;
	    printk(KERN_INFO "PM: %s %d kbytes in %d.%02d seconds (%d.%02d MB/s)\n",
		    msg, k, centisecs / 100, centisecs % 100, kps / 1000, (kps % 1000) / 10);
}

/**
 * create_image - Create a hibernation image.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Execute device drivers' "late" and "noirq" freeze callbacks, create a
 * hibernation image and run the drivers' "noirq" and "early" thaw callbacks.
 *
 * Control reappears in this routine after the subsequent restore.
 */

#ifdef CONFIG_SNAPSHOT_BOOT
/*===========================================================================
 * for snapshot boot
 * ==============================================================================*/
extern void snapshot_mmu_switching(void);
extern void snapshot_state_store(void);
extern void snapshot_state_restore(void);
unsigned int reg[64];
extern volatile unsigned int do_snapshot_boot;
extern void restore_snapshot_cpu_reg(void);
extern int save_cpu_reg_for_snapshot(unsigned int ptable, unsigned int pReg, void *);
#endif

static int create_image(int platform_mode)
{
	int error;
//    unsigned  long long t;

	hb_dbg(KERN_CRIT "============>> %s\n", __func__);
#ifdef CONFIG_SNAPSHOT_BOOT
    static PPMU pPMU;
    pPMU = (PPMU)tcc_p2v(HwPMU_BASE);
#endif

	error = dpm_suspend_end(PMSG_FREEZE);
	if (error) {
		printk(KERN_ERR "PM: Some devices failed to power down, "
				"aborting hibernation\n");
		return error;
	}

	error = platform_pre_snapshot(platform_mode);
	if (error || hibernation_test(TEST_PLATFORM))
		goto Platform_finish;

	/*		K3.4 - Moved to hibernate().		
	error = disable_nonboot_cpus();
	if (error || hibernation_test(TEST_CPUS))
		goto Enable_cpus;
	*/

	if ( hibernation_test(TEST_CPUS)
        || hibernation_testmode(HIBERNATION_TEST))
        goto Platform_finish;

	local_irq_disable();

	error = syscore_suspend();
	if (error) {
		printk(KERN_ERR "PM: Some system devices failed to power down, "
				"aborting hibernation\n");
		goto Enable_irqs;
	}
#ifdef CONFIG_SNAPSHOT_BOOT
	    do_snapshot_boot = 0;
#endif
	if (hibernation_test(TEST_CORE) || pm_wakeup_pending())
		goto Power_up;

	in_suspend = 1;

    hb_dbg(KERN_CRIT " %s :: save cp15 state done.\n", __func__);
#ifdef CONFIG_SNAPSHOT_BOOT
    /*===========================================================================
	 *     Save context for snapshot boot
	 *=========================================================================*/
    snapshot_state_store();
    hb_dbg(KERN_CRIT " %s :: save register state done.\n", __func__);
    save_processor_state();
    save_cpu_reg_for_snapshot(0x0, reg, restore_snapshot_cpu_reg);
#else
	error = swsusp_arch_suspend();
#endif

	if (error)
		printk(KERN_ERR "PM: Error %d creating hibernation image\n", error);

	/* Restore control flow magically appears here */
	snapshot_restore_start = cpu_clock(smp_processor_id());

#ifdef CONFIG_SNAPSHOT_BOOT
	if (do_snapshot_boot) {
		in_suspend = 0;
	}
	barrier();

	if (do_snapshot_boot) {
		/*	QuickBoot Booting.	*/

		restore_processor_state();
		snapshot_state_restore();
//		in_suspend = 0;
	#ifdef CONFIG_ARCH_TCC892X
		pPMU->PMU_WDTCTRL.bREG.CLR = 1;
	#elif CONFIG_ARCH_TCC893X
		pPMU->PMU_WDTCTRL.bREG.CLR = 1;
	#endif
	}
	else {
		/*	Making QuickBoot Image.	*/
		snapshot_state_restore();
		preempt_enable();
	#if defined(CONFIG_ARCH_TCC893X)
		restore_scu_state();
	#endif	/*	#if defined(CONFIG_ARCH_TCC893X)	*/
	}
#endif

    hb_dbg(KERN_CRIT " %s :: restore cp15 state done.  [cpu%d] \n", __func__, smp_processor_id());

	if (!in_suspend) {
		do_hibernate_boot = 1;
		events_check_enabled = false;
		platform_leave(platform_mode);
	}

Power_up:
	syscore_resume();

Enable_irqs:
	local_irq_enable();

//Enable_cpus:
//	enable_nonboot_cpus();

Platform_finish:
	platform_finish(platform_mode);

	dpm_resume_start(in_suspend ?
			(error ? PMSG_RECOVER : PMSG_THAW) : PMSG_RESTORE);

	return error;
}

/**
 * hibernation_snapshot - Quiesce devices and create a hibernation image.
 * @platform_mode: If set, use platform driver to prepare for the transition.
 *
 * This routine must be called with pm_mutex held.
 */
int hibernation_snapshot(int platform_mode)
{
	pm_message_t msg;
	int error;

#ifdef CONFIG_SNAPSHOT_BOOT
	static PPMU pPMU;
	pPMU = (PPMU)tcc_p2v(HwPMU_BASE);
#endif
	unsigned long long t;

	error = platform_begin(platform_mode);
	if (error)
		goto Close;

	/* Preallocate image memory before shutting down devices. */
	error = hibernate_preallocate_memory();
	if (error)
		goto Close;

	error = freeze_kernel_threads();
	if (error)
		goto Cleanup;

	if (hibernation_test(TEST_FREEZER)) {

		/*
		 * Indicate to the caller that we are returning due to a
		 * successful freezer test.
		 */
		freezer_test_done = true;
		goto Thaw;
	}

	error = dpm_prepare(PMSG_FREEZE);
	if (error) {
		dpm_complete(PMSG_RECOVER);
		goto Thaw;
	}

	suspend_console();
	ftrace_stop();
	pm_restrict_gfp_mask();

	error = dpm_suspend(PMSG_FREEZE);

	if (error || hibernation_test(TEST_DEVICES))
		platform_recover(platform_mode);
	else
		error = create_image(platform_mode);

	/*
	 * In the case that we call create_image() above, the control
	 * returns here (1) after the image has been created or the
	 * image creation has failed and (2) after a successful restore.
	 */

	/* We may need to release the preallocated image pages here. */
	if (error || !in_suspend)
		swsusp_free();

	msg = in_suspend ? (error ? PMSG_RECOVER : PMSG_THAW) : PMSG_RESTORE;

#ifdef CONFIG_SNAPSHOT_BOOT
	if (do_snapshot_boot) {
	#ifdef CONFIG_ARCH_TCC892X
		pPMU->PMU_WDTCTRL.bREG.CLR = 1;
	#elif CONFIG_ARCH_TCC893X
	    pPMU->PMU_WDTCTRL.bREG.CLR = 1;
	#endif
	}
#endif

	t = cpu_clock(smp_processor_id());

	dpm_resume(msg);
/*
#if defined(CONFIG_HIBERNATION)
	    if (!do_hibernate_boot)
#endif
	resume_console();
*/

	profile_dpm_resume = (cpu_clock(smp_processor_id()) - t);
	
	if (error || !in_suspend)
		pm_restore_gfp_mask();

	ftrace_start();
#if defined(CONFIG_HIBERNATION)
	if (!do_hibernate_boot) 
#endif
		resume_console();

	dpm_complete(msg);

	thaw_kernel_threads();	// K3.4 - Insert it here. I'm not sure. 

Close:
	platform_end(platform_mode);
	return error;

Thaw:
	thaw_kernel_threads();
Cleanup:
	swsusp_free();
	goto Close;
}

/**
 * resume_target_kernel - Restore system state from a hibernation image.
 * @platform_mode: Whether or not to use the platform driver.
 *
 * Execute device drivers' "noirq" and "late" freeze callbacks, restore the
 * contents of highmem that have not been restored yet from the image and run
 * the low-level code that will restore the remaining contents of memory and
 * switch to the just restored target kernel.
 */
static int resume_target_kernel(bool platform_mode)
{
	int error;

	error = dpm_suspend_end(PMSG_QUIESCE);
	if (error) {
		printk(KERN_ERR "PM: Some devices failed to power down, "
				"aborting resume\n");
		return error;
	}

	error = platform_pre_restore(platform_mode);
	if (error)
		goto Cleanup;

	error = disable_nonboot_cpus();
	if (error)
		goto Enable_cpus;

	local_irq_disable();

	error = syscore_suspend();
	if (error)
		goto Enable_irqs;

	save_processor_state();

	error = restore_highmem();

	if (!error) {
		error = swsusp_arch_resume();
		/*
		 * The code below is only ever reached in case of a failure.
		 * Otherwise, execution continues at the place where
		 * swsusp_arch_suspend() was called.
		 */
		BUG_ON(!error);
		/*
		 * This call to restore_highmem() reverts the changes made by
		 * the previous one.
		 */
		restore_highmem();
	}
	/*
	 * The only reason why swsusp_arch_resume() can fail is memory being
	 * very tight, so we have to free it as soon as we can to avoid
	 * subsequent failures.
	 */
	swsusp_free();
	restore_processor_state();
	touch_softlockup_watchdog();

	syscore_resume();

Enable_irqs:
	local_irq_enable();

Enable_cpus:
	enable_nonboot_cpus();

Cleanup:
	platform_restore_cleanup(platform_mode);

	dpm_resume_start(PMSG_RECOVER);

	return error;
}

/**
 * hibernation_restore - Quiesce devices and restore from a hibernation image.
 * @platform_mode: If set, use platform driver to prepare for the transition.
 *
 * This routine must be called with pm_mutex held.  If it is successful, control
 * reappears in the restored target kernel in hibernation_snapshot().
 */
int hibernation_restore(int platform_mode)
{
	int error;

	pm_prepare_console();
	suspend_console();
	ftrace_stop();
	pm_restrict_gfp_mask();

	error = dpm_suspend_start(PMSG_QUIESCE);
	if (!error) {
		error = resume_target_kernel(platform_mode);
		dpm_resume_end(PMSG_RECOVER);
	}
	pm_restore_gfp_mask();
	ftrace_start();
	resume_console();
	pm_restore_console();
	return error;
}

/**
 * hibernation_platform_enter - Power off the system using the platform driver.
 */
int hibernation_platform_enter(void)
{
	int error;

	if (!hibernation_ops)
		return -ENOSYS;

	/*
	 * We have cancelled the power transition by running
	 * hibernation_ops->finish() before saving the image, so we should let
	 * the firmware know that we're going to enter the sleep state after all
	 */
	error = hibernation_ops->begin();
	if (error)
		goto Close;

	entering_platform_hibernation = true;
	suspend_console();
	ftrace_stop();
	error = dpm_suspend_start(PMSG_HIBERNATE);
	if (error) {
		if (hibernation_ops->recover)
			hibernation_ops->recover();
		goto Resume_devices;
	}

	error = dpm_suspend_end(PMSG_HIBERNATE);
	if (error)
		goto Resume_devices;

	error = hibernation_ops->prepare();
	if (error)
		goto Platform_finish;

	error = disable_nonboot_cpus();
	if (error)
		goto Platform_finish;

	local_irq_disable();
	syscore_suspend();
	if (pm_wakeup_pending()) {
		error = -EAGAIN;
		goto Power_up;
	}

	hibernation_ops->enter();
	/* We should never get here */
	while (1);

Power_up:
	syscore_resume();
	local_irq_enable();
	enable_nonboot_cpus();

Platform_finish:
	hibernation_ops->finish();

	dpm_resume_start(PMSG_RESTORE);

Resume_devices:
	entering_platform_hibernation = false;
	dpm_resume_end(PMSG_RESTORE);
	ftrace_start();
	resume_console();

Close:
	hibernation_ops->end();

	return error;
}

/**
 * power_down - Shut the machine down for hibernation.
 *
 * Use the platform driver, if configured, to put the system into the sleep
 * state corresponding to hibernation, or try to power it off or reboot,
 * depending on the value of hibernation_mode.
 */
static void power_down(void)
{
	switch (hibernation_mode) {
		case HIBERNATION_TEST:
		case HIBERNATION_TESTPROC:
			break;
		case HIBERNATION_REBOOT:
			kernel_restart(NULL);
			break;
		case HIBERNATION_PLATFORM:
			hibernation_platform_enter();
		case HIBERNATION_SHUTDOWN:
			kernel_power_off();
			break;
	}
	kernel_halt();
	/*
	 * Valid image is on the disk, if we continue we risk serious data
	 * corruption after resume.
	 */
	printk(KERN_CRIT "PM: Please power down manually\n");
	while(1);
}

/*		From K3.1. Do not use it in K3.4
static int prepare_processes(void)
{
	int error = 0;

	if (freeze_processes()) {
		error = -EBUSY;
//		thaw_processes();
//		free_basic_memory_bitmaps();
	}
	return error;
}
*/

int set_hibernate_boot_property(void) {
	static char * path = "/system/bin/setprop";

#ifdef CONFIG_SNAPSHOT_BOOT
	char *argv[] = { path,  "tcc.QB.boot.with", "snapshot", NULL};
#else
	char *argv[] = { path,  "tcc.QB.boot.with", "hibernate", NULL};
#endif

	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(path, argv, envp, UMH_WAIT_PROC);
}

int set_hibernate_swap_path_property(void) {
	static char * path = "/system/bin/setprop";

	char *argv[] = { path,  "tcc.swap.path", resume_file, NULL};

	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(path, argv, envp, UMH_WAIT_PROC);
}

int reload_persistent_property(void) {
	static char * path = "/system/bin/reload_persistent_property";
	char *argv[] = { path,  NULL};
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(path, argv, envp, UMH_WAIT_PROC);
}


int load_module(char* path) {
	static char * cmdpath = "/system/bin/insmod";
	char *argv[] = { cmdpath,  path, NULL};
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(cmdpath, argv, envp, UMH_WAIT_PROC);
}

int hibernate_mkswap(char* path) {
	static char * cmdpath = "/system/bin/mkswap";
	char *argv[] = { cmdpath, path, NULL};
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(cmdpath, argv, envp, UMH_WAIT_PROC);
}

int hibernate_swapon(char* path) {
	static char * cmdpath = "/system/bin/swapon";
	char *argv[] = { cmdpath, path, NULL};
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(cmdpath, argv, envp, UMH_WAIT_PROC);
}

int hibernate_swapoff(char* path) {
	static char * cmdpath = "/system/bin/swapoff";
	char *argv[] = { cmdpath, path, NULL};
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(cmdpath, argv, envp, UMH_WAIT_PROC);
}

static int slogi(char* message) {
	static char * cmdpath = "/system/bin/log";
	char *argv[] = { cmdpath, message, NULL};
	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(cmdpath, argv, envp, UMH_WAIT_PROC);
}

static int hibernate_e2fsck(char* path){
	static char *cmdpath = "/system/bin/e2fsck";
//	char *argv[] = {cmdpath , "-py", path, NULL};
	char *argv[] = {cmdpath , "-p", path, NULL};

	static char *envp[] = { "HOME=/",
		"TERM=linux",
		"PATH=/sbin:/usr/inssbin:/bin:/usr/bin",
		NULL };

	return call_usermodehelper(cmdpath, argv, envp, UMH_WAIT_PROC);
}

int load_tcc_nand_module(int flag)
{
	int retval = 0;

	/* Only considered for TCC88xx */

	return retval;
}

#include <linux/rtc.h>
extern int rtc_hctosys_ret;

static int __init rtc_hctosys(void)
{
	int err = -ENODEV;
	struct rtc_time tm;
	struct timespec tv = {
		.tv_nsec = NSEC_PER_SEC >> 1,
	};
	struct rtc_device *rtc = rtc_class_open(CONFIG_RTC_HCTOSYS_DEVICE);

	if (rtc == NULL) {
		pr_err("%s: unable to open rtc device (%s)\n",
				__FILE__, CONFIG_RTC_HCTOSYS_DEVICE);
		goto err_open;
	}

	err = rtc_read_time(rtc, &tm);
	if (err) {
		dev_err(rtc->dev.parent,
				"hctosys: unable to read the hardware clock\n");
		goto err_read;

	}

	err = rtc_valid_tm(&tm);
	if (err) {
		dev_err(rtc->dev.parent,
				"hctosys: invalid date/time\n");
		goto err_invalid;
	}

	rtc_tm_to_time(&tm, &tv.tv_sec);

	do_settimeofday(&tv);

	dev_info(rtc->dev.parent,
			"setting system clock to "
			"%d-%02d-%02d %02d:%02d:%02d UTC (%u)\n",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			(unsigned int) tv.tv_sec);

err_invalid:
err_read:
	rtc_class_close(rtc);

err_open:
	rtc_hctosys_ret = err;

	return err;
}

/**
 *  * hibernate - Carry out system hibernation, including saving the image.
 *   */
extern int fb_external_output_update(void);

#ifdef CONFIG_QUICK_BOOT_LOGO
#define QUICKBOOT_LOGO         "QuickBoot_logo.rle"
extern int fb_hibernation_logo(char *rle_filename);
#ifdef CONFIG_USING_LAST_FRAMEBUFFER
extern int fb_quickboot_lastframe_display(void);
#endif
#endif

extern void tcc_usb_notification(void);


/**
 * hibernate - Carry out system hibernation, including saving the image.
 */
int hibernate(void)
{
	int error;
	unsigned long long start;
	unsigned long long start_remount;
	unsigned long long stop_remount;
	unsigned long long start_e2fsck;
	unsigned long long stop_e2fsck;
	unsigned long long stop;
	unsigned long long remount_time;
	unsigned long long total_time;
	unsigned long long resume_time;
	unsigned long long etc_time;
	unsigned long long e2fsck_time;
	unsigned long long remaind;
	struct mount *mount;
	struct path path;

#ifdef CONFIG_SNAPSHOT_BOOT
	static PPMU pPMU;
	pPMU = (PPMU)tcc_p2v(HwPMU_BASE);
#endif
	char message[512];

	lock_system_sleep(); // K3.4
	/* The snapshot device should not be opened while we're running */
	if (!atomic_add_unless(&snapshot_device_available, -1, 0)) {
		error = -EBUSY;
		goto Unlock;
	}

	pm_prepare_console();

	error = pm_notifier_call_chain(PM_HIBERNATION_PREPARE);
	if (error)
		goto Exit;

	error = disable_nonboot_cpus();
	if (error)
		goto Enable_cpus;

	set_hibernate_swap_path_property();
	printk(KERN_INFO "PM: Syncing filesystems ... \n");
	sys_sync();
	sys_sync();
	sys_sync();

#if 1
	printk(KERN_INFO "PM: un-mount /data ...\n");
	error = kern_path("/data", 0, &path);
	if(error) {
		printk(" PM: Can't find the path of /data... [%d] \n\n", error);
	}
	else {
		mount = real_mount(path.mnt);
		error = do_umount(mount, 0);
		if(error) {
			printk(" PM: Can't do umount /data... [%d] \n\n", error);
			goto Enable_cpus;
		}

		/* we must not call path_put() as that would clear mnt_expiry_mark */
		dput(path.dentry);
		mntput_no_expire(mount);	// K3.4 - vfsmount to mount.
	}

	printk(KERN_INFO "PM: un-mount /cache ...\n");
	error = kern_path("/cache", 0, &path);
	if(error) {
		printk(" PM: Can't find the path of /cache... [%d] \n\n", error);
	}
	else {
		mount = real_mount(path.mnt);
		error = do_umount(mount, 0);
		if(error) {
			printk(" PM: Can't do umount /cache... [%d] \n\n", error);
			goto Enable_cpus;
		}

		/* we must not call path_put() as that would clear mnt_expiry_mark */
		dput(path.dentry);
		mntput_no_expire(mount);	// K3.4 - vfsmount to mount.
	}
#endif

	printk("done.\n");
	/*		usermodehelper_disable() moved to freeze_processes()	*/

	/* Allocate memory management structures */
	error = create_basic_memory_bitmaps();
	if (error)
		goto Enable_cpus;

	/*		prepare_processes() is substituted by freeze_processes()	*/

	error = freeze_processes();
	if (error)
		goto Free_bitmaps;

	if (hibernation_testmode(HIBERNATION_TESTPROC))
	    goto Thaw;

	#ifdef CONFIG_QUICK_BOOT_LOGO
	    #ifdef CONFIG_USING_LAST_FRAMEBUFFER
		    fb_quickboot_lastframe_display();
	    #endif
	    fb_hibernation_logo(QUICKBOOT_LOGO);
	#endif

	error = hibernation_snapshot(hibernation_mode == HIBERNATION_PLATFORM);
	if (error || freezer_test_done) {
		printk("failed hibernation_snapshot! error = %d\n", error);
		goto Thaw;
	}

	if (in_suspend) {
		unsigned int flags = 0;

		usermodehelper_enable();	// Originally it was in thaw_processes();

		error = hibernate_mkswap(resume_file);
        printk(KERN_CRIT "hibernate_mkswap(%s) %d\n", resume_file, error);

		error = hibernate_swapon(resume_file);
		printk(KERN_CRIT "hibernate_swapon(%s) %d\n", resume_file, error);

		if (hibernation_mode == HIBERNATION_PLATFORM)
			flags |= SF_PLATFORM_MODE;
		if (nocompress)
			flags |= SF_NOCOMPRESS_MODE;
		else
			flags |= SF_CRC32_MODE;	// USE Telechips's CheckSum Algorithm.

		pr_debug("PM: writing image.\n");
		error = swsusp_write(flags);
		swsusp_free();
		if (!error)
			power_down();
		in_suspend = 0;
		pm_restore_gfp_mask();
	} else {
#ifdef CONFIG_SNAPSHOT_BOOT
		if (do_snapshot_boot) {
			printk("*** Disable Watchdog ***\n");
	#ifdef CONFIG_ARCH_TCC892X
			pPMU->PMU_WDTCTRL.bREG.EN = 0;
	#elif CONFIG_ARCH_TCC893X
			pPMU->PMU_WDTCTRL.bREG.EN = 0;
	#endif
	    }
#endif
		start = cpu_clock(smp_processor_id());
		pr_debug("PM: Image restored successfully.\n");
		
		usermodehelper_enable();

		//thaw_kernel_threads();	// K3.4 - I'm not sure...
		hibernate_thaw_processes();

		start_remount = cpu_clock(smp_processor_id());

		do_hibernate_boot = 0;

#if 1
		start_e2fsck = cpu_clock(smp_processor_id());
		hibernate_e2fsck("/dev/block/platform/bdm/by-name/cache");
		stop_e2fsck = cpu_clock(smp_processor_id());
		e2fsck_time = (stop_e2fsck - start_e2fsck);
		remaind = do_div(e2fsck_time, 1000000);

		printk("Cache Partition CheckDisk Time : %lldms \n", e2fsck_time);
//		error = do_mount("/dev/block/platform/bdm/by-num/p5", "/cache", "ext4", MS_NOATIME | MS_NOSUID | MS_NODEV, data);

		start_e2fsck = cpu_clock(smp_processor_id());
		hibernate_e2fsck("/dev/block/platform/bdm/by-name/userdata");
		stop_e2fsck = cpu_clock(smp_processor_id());
		e2fsck_time = (stop_e2fsck - start_e2fsck);
		remaind = do_div(e2fsck_time, 1000000);

		printk("UserData Partition CheckDisk Time : %lldms \n", e2fsck_time);
//		error = do_mount("/dev/block/platform/bdm/by-num/p3", "/data", "ext4", MS_NOATIME | MS_NOSUID | MS_NODEV, data);

#endif
		stop_remount = cpu_clock(smp_processor_id());

		rtc_hctosys();

		do_hibernation = 0;
//		set_hibernate_swap_path_property();
	}

Thaw:
	usermodehelper_disable(); // It will enabled in thaw_processes();
	thaw_processes();

	/* Don't bother checking whether freezer_test_done is true */
	freezer_test_done = false;

Free_bitmaps:
	free_basic_memory_bitmaps();
Enable_cpus:
	enable_nonboot_cpus();
Exit:
	pm_notifier_call_chain(PM_POST_HIBERNATION);
	pm_restore_console();
	atomic_inc(&snapshot_device_available);
Unlock:
	unlock_system_sleep();

	if( !in_suspend ) {
		error = set_hibernate_boot_property();
	}

	stop = cpu_clock(smp_processor_id());

	total_time = (stop - snapshot_restore_start);
	remaind = do_div(total_time, 1000000);

	resume_time = (start - snapshot_restore_start);
	remaind = do_div(resume_time, 1000000);

	remount_time = (stop_remount - start_remount);
	remaind = do_div(remount_time, 1000000);

	etc_time = total_time - resume_time;
	etc_time = etc_time - remount_time;
	do_div(profile_dpm_resume, 1000000);

	sprintf(message, "quickboot profile kernel - total [%lldms], resume [%lldms] chkdsk [%lldms] etc [%lldms] dmp resume[%lldms]\n", total_time, resume_time, remount_time, etc_time, profile_dpm_resume);

	slogi(message);
	printk(message);

	return error;
}


/**
 * software_resume - Resume from a saved hibernation image.
 *
 * This routine is called as a late initcall, when all devices have been
 * discovered and initialized already.
 *
 * The image reading code is called to see if there is a hibernation image
 * available for reading.  If that is the case, devices are quiesced and the
 * contents of memory is restored from the saved image.
 */
void *read_file(const char *fn, unsigned *_sz)
{
    char *data;
    int sz;
    int fd;

    data = 0;
    fd = sys_open(fn, O_RDONLY, 0);
    if(fd < 0) return 0;

    sz = sys_lseek(fd, 0, SEEK_END);
    if(sz < 0) goto oops;

    if(sys_lseek(fd, 0, SEEK_SET) != 0) goto oops;

    data = (char*) vmalloc(sz + 2);
    //data = (char*) vmalloc(sz + 2);
    if(data == 0) goto oops;

    if(sys_read(fd, data, sz) != sz) goto oops;
    sys_close(fd);
    data[sz] = '\n';
    data[sz+1] = 0;
    if(_sz) *_sz = sz;
    return data;

oops:
    sys_close(fd);
    if(data != 0) vfree(data);
    return 0;
}

static int insmod(const char *filename)
{
    void *module;
    void __user *umodule;
    void __user *uoptions;
    char options[2];
    unsigned size;
    int ret;
	unsigned long ret_copy_to_user;

    options[0] = '\0';
    module = read_file(filename, &size);
    if (!module)
        return -1;

    umodule =  kmalloc(size + 2, GFP_USER);
    
	ret_copy_to_user = copy_to_user(umodule, module, size + 2);
    vfree(module);

    uoptions =  kmalloc(2, GFP_USER);
    ret_copy_to_user = copy_to_user(uoptions, options, 2);
	
    ret =sys_init_module(umodule, size, uoptions);

    kfree(umodule);
    kfree(uoptions);

    return ret;
}

#include <linux/module.h>
int rmmod(const char *module_name)
{
    char module[MODULE_NAME_LEN];
	void __user *umodule;
	void __user *uoptions;
	char options[2];
	unsigned int size;
	int ret;
	unsigned long ret_copy_to_user;

	options[0] = '\0';

	memset(module, 0, MODULE_NAME_LEN);
	strcpy(module, module_name);
	size = (unsigned int)strlen(module);

	umodule =  kmalloc(size + 2, GFP_USER);
	ret_copy_to_user = copy_to_user(umodule, module, size + 2);

	/* pass it to the kernel */
	ret = sys_delete_module(umodule, O_RDONLY);//O_TRUNC);

	kfree(umodule);

	return ret;
}

/*
 * If this is successful, control reappears in the restored target kernel in
 * hibernation_snaphot() which returns to hibernate().  Otherwise, the routine
 * attempts to recover gracefully and make the kernel return to the normal mode
 * of operation.
 */
static int software_resume(void)
{
	int error;
	unsigned int flags;

	pr_info("PM: %s \n", __func__);
	/*
	 * If the user said "noresume".. bail out early.
	 */
	if (noresume)
		return 0;

	if( !strncmp(resume_file, "/dev/block/mtd", 14)  || !strncmp(resume_file, "/dev/block/ndd", 14)) 
	{
		if(error = insmod("/lib/modules/tcc_nand_core.ko"))
			pr_info("load_nand_core_module result = %d\n", error);

		if(error = insmod("/lib/modules/tcc_nand.ko"))
			pr_info("load_nand_module result = %d\n", error);
		
//		error = insmod("/lib/modules/tcc_mtd.ko");
//		pr_info("load_mtd_module result = %d\n", error);
	}

	/*
	 * name_to_dev_t() below takes a sysfs buffer mutex when sysfs
	 * is configured into the kernel. Since the regular hibernate
	 * trigger path is via sysfs which takes a buffer mutex before
	 * calling hibernate functions (which take pm_mutex) this can
	 * cause lockdep to complain about a possible ABBA deadlock
	 * which cannot happen since we're in the boot code here and
	 * sysfs can't be invoked yet. Therefore, we use a subclass
	 * here to avoid lockdep complaining.
	 */
	mutex_lock_nested(&pm_mutex, SINGLE_DEPTH_NESTING);

	if (swsusp_resume_device)
		goto Check_image;

	if (!strlen(resume_file)) {
		error = -ENOENT;
		goto Unlock;
	}

	pr_debug("PM: Checking hibernation image partition %s\n", resume_file);

	if (resume_delay) {
		printk(KERN_INFO "Waiting %dsec before reading resume device...\n",	resume_delay);
		ssleep(resume_delay);
	}

	/* Check if the device is there */
	swsusp_resume_device = name_to_dev_t(resume_file);

	/*
	 * name_to_dev_t is ineffective to verify parition if resume_file is in
	 * integer format. (e.g. major:minor)
	 */
	if (isdigit(resume_file[0]) && resume_wait) {
		int partno;
		while (!get_gendisk(swsusp_resume_device, &partno))
			msleep(10);
	}

	if (!swsusp_resume_device) {
		/*
		 * Some device discovery might still be in progress; we need
		 * to wait for this to finish.
		 */
		wait_for_device_probe();

		if (resume_wait) {
			while ((swsusp_resume_device = name_to_dev_t(resume_file)) == 0)
				msleep(10);
			async_synchronize_full();
		}

		/*
		 * We can't depend on SCSI devices being available after loading
		 * one of their modules until scsi_complete_async_scans() is
		 * called and the resume device usually is a SCSI one.
		 */
		scsi_complete_async_scans();

		swsusp_resume_device = name_to_dev_t(resume_file);
		if (!swsusp_resume_device) {
			error = -ENODEV;
			goto Unlock;
		}
	}

Check_image:
	pr_info("PM: Hibernation image partition %d:%d present\n",
			MAJOR(swsusp_resume_device), MINOR(swsusp_resume_device));

#ifdef CONFIG_SNAPSHOT_BOOT
	goto Unlock;
#endif

	pr_info("PM: Looking for hibernation image.\n");
	error = swsusp_check();
	if (error)
		goto Unlock;

	/* The snapshot device should not be opened while we're running */
	if (!atomic_add_unless(&snapshot_device_available, -1, 0)) {
		error = -EBUSY;
		swsusp_close(FMODE_READ);
		goto Unlock;
	}

	pm_prepare_console();
	error = pm_notifier_call_chain(PM_RESTORE_PREPARE);
	if (error)
		goto close_finish;

	/*	usermodehelper_diable() is moved in freeze_processes().	*/

	error = create_basic_memory_bitmaps();
	if (error)
		goto close_finish;

	pr_debug("PM: Preparing processes for restore.\n");
	error = freeze_processes();
	if (error) {
		swsusp_close(FMODE_READ);
		goto Done;
	}

	pr_debug("PM: Loading hibernation image.\n");

	error = swsusp_read(&flags);

	swsusp_close(FMODE_READ);

	/*	Remove nand modules 	*/
	if( !strncmp(resume_file, "/dev/block/mtd", 14)  || !strncmp(resume_file, "/dev/block/ndd", 14)) 
	{
//		if(error = rmmod("tcc_nand"))
//			pr_info("Remove nand_module result = %d\n", error);

//		if(error = rmmod("tcc_nand_core"))
//			pr_info("Remove nand_core_module result = %d\n", error);
		
//		if(error = insmod("/lib/modules/tcc_nand.ko"))
//			pr_info("load_nand_module result = %d\n", error);
	}

	if (!error)
		hibernation_restore(flags & SF_PLATFORM_MODE);

	printk(KERN_ERR "PM: Failed to load hibernation image, recovering.\n");
	swsusp_free();
	thaw_processes();
Done:
	free_basic_memory_bitmaps();
	usermodehelper_enable();	// K3.4
Finish:
	pm_notifier_call_chain(PM_POST_RESTORE);
	pm_restore_console();
	atomic_inc(&snapshot_device_available);
	/* For success case, the suspend path will release the lock */
Unlock:
	mutex_unlock(&pm_mutex);
	pr_debug("PM: Hibernation image not present or could not be loaded.\n");
	return error;
close_finish:
	swsusp_close(FMODE_READ);
	goto Finish;
}

late_initcall(software_resume);


static const char * const hibernation_modes[] = {
	[HIBERNATION_PLATFORM]	= "platform",
	[HIBERNATION_SHUTDOWN]	= "shutdown",
	[HIBERNATION_REBOOT]	= "reboot",
	[HIBERNATION_TEST]	= "test",
	[HIBERNATION_TESTPROC]	= "testproc",
};

/*
 * /sys/power/disk - Control hibernation mode.
 *
 * Hibernation can be handled in several ways.  There are a few different ways
 * to put the system into the sleep state: using the platform driver (e.g. ACPI
 * or other hibernation_ops), powering it off or rebooting it (for testing
 * mostly).
 *
 * The sysfs file /sys/power/disk provides an interface for selecting the
 * hibernation mode to use.  Reading from this file causes the available modes
 * to be printed.  There are 3 modes that can be supported:
 *
 *	'platform'
 *	'shutdown'
 *	'reboot'
 *
 * If a platform hibernation driver is in use, 'platform' will be supported
 * and will be used by default.  Otherwise, 'shutdown' will be used by default.
 * The selected option (i.e. the one corresponding to the current value of
 * hibernation_mode) is enclosed by a square bracket.
 *
 * To select a given hibernation mode it is necessary to write the mode's
 * string representation (as returned by reading from /sys/power/disk) back
 * into /sys/power/disk.
 */

static ssize_t disk_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	int i;
	char *start = buf;

	for (i = HIBERNATION_FIRST; i <= HIBERNATION_MAX; i++) {
		if (!hibernation_modes[i])
			continue;
		switch (i) {
			case HIBERNATION_SHUTDOWN:
			case HIBERNATION_REBOOT:
			case HIBERNATION_TEST:
			case HIBERNATION_TESTPROC:
				break;
			case HIBERNATION_PLATFORM:
				if (hibernation_ops)
					break;
				/* not a valid mode, continue with loop */
				continue;
		}
		if (i == hibernation_mode)
			buf += sprintf(buf, "[%s] ", hibernation_modes[i]);
		else
			buf += sprintf(buf, "%s ", hibernation_modes[i]);
	}
	buf += sprintf(buf, "\n");
	return buf-start;
}

static ssize_t disk_store(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t n)
{
	int error = 0;
	int i;
	int len;
	char *p;
	int mode = HIBERNATION_INVALID;

	p = memchr(buf, '\n', n);
	len = p ? p - buf : n;

	lock_system_sleep();
	for (i = HIBERNATION_FIRST; i <= HIBERNATION_MAX; i++) {
		if (len == strlen(hibernation_modes[i])
				&& !strncmp(buf, hibernation_modes[i], len)) {
			mode = i;
			break;
		}
	}
	if (mode != HIBERNATION_INVALID) {
		switch (mode) {
			case HIBERNATION_SHUTDOWN:
			case HIBERNATION_REBOOT:
				hibernation_mode = mode;
				break;
			case HIBERNATION_PLATFORM:
				if (hibernation_ops)
					hibernation_mode = mode;
				else
					error = -EINVAL;
		}
	} else
		error = -EINVAL;

	if (!error)
		pr_debug("PM: Hibernation mode set to '%s'\n",
				hibernation_modes[mode]);
	unlock_system_sleep();
	return error ? error : n;
}

power_attr(disk);

static ssize_t resume_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	return sprintf(buf,"%d:%d\n", MAJOR(swsusp_resume_device),
			MINOR(swsusp_resume_device));
}

static ssize_t resume_store(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t n)
{
	unsigned int maj, min;
	dev_t res;
	int ret = -EINVAL;

	if (sscanf(buf, "%u:%u", &maj, &min) != 2)
		goto out;

	res = MKDEV(maj,min);
	if (maj != MAJOR(res) || min != MINOR(res))
		goto out;

	lock_system_sleep();
	swsusp_resume_device = res;
	unlock_system_sleep();
	printk(KERN_INFO "PM: Starting manual resume from disk\n");
	noresume = 0;
	software_resume();
	ret = n;
out:
	return ret;
}

power_attr(resume);

static ssize_t image_size_show(struct kobject *kobj, struct kobj_attribute *attr,
		char *buf)
{
	return sprintf(buf, "%lu\n", image_size);
}

static ssize_t image_size_store(struct kobject *kobj, struct kobj_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long size;

	if (sscanf(buf, "%lu", &size) == 1) {
		image_size = size;
		return n;
	}

	return -EINVAL;
}

power_attr(image_size);

static ssize_t reserved_size_show(struct kobject *kobj,
		struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", reserved_size);
}

static ssize_t reserved_size_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t n)
{
	unsigned long size;

	if (sscanf(buf, "%lu", &size) == 1) {
		reserved_size = size;
		return n;
	}

	return -EINVAL;
}

power_attr(reserved_size);

static struct attribute * g[] = {
	&disk_attr.attr,
	&resume_attr.attr,
	&image_size_attr.attr,
	&reserved_size_attr.attr,
	NULL,
};


static struct attribute_group attr_group = {
	.attrs = g,
};


static int __init pm_disk_init(void)
{
	return sysfs_create_group(power_kobj, &attr_group);
}

core_initcall(pm_disk_init);


static int __init resume_setup(char *str)
{
	if (noresume)
		return 1;

	strncpy( resume_file, str, 255 );
	return 1;
}

static int __init resume_offset_setup(char *str)
{
	unsigned long long offset;

	if (noresume)
		return 1;

	if (sscanf(str, "%llu", &offset) == 1)
		swsusp_resume_block = offset;

	return 1;
}

static int __init hibernate_setup(char *str)
{
	if (!strncmp(str, "noresume", 8))
		noresume = 1;
	else if (!strncmp(str, "nocompress", 10))
		nocompress = 1;
	return 1;
}

static int __init noresume_setup(char *str)
{
	noresume = 1;
	return 1;
}

static int __init resumewait_setup(char *str)
{
	resume_wait = 1;
	return 1;
}

static int __init resumedelay_setup(char *str)
{
	resume_delay = simple_strtoul(str, NULL, 0);
	return 1;
}

__setup("noresume", noresume_setup);
__setup("resume_offset=", resume_offset_setup);
__setup("resume=", resume_setup);
__setup("hibernate=", hibernate_setup);
__setup("resumewait", resumewait_setup);
__setup("resumedelay=", resumedelay_setup);
