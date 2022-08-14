/****************************************************************************
 * arch/arm/mach-tcc893x/snapshot_pm.c
 * Copyright (C) 2014 Telechips Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/



/*===========================================================================

                         INCLUDE FILES FOR MODULE

===========================================================================*/

#include <linux/suspend.h>
#include <linux/reboot.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>         // local_flush_tlb_all(), flush_cache_all();
#include <linux/cpu_pm.h>           // cpu_pm_enter(), cpu_pm_exit()
#include <mach/bsp.h>
#include <mach/system.h>
#include <asm/system.h>
#include <mach/pm.h>
#include <mach/tcc_ddr.h>
#include <mach/tcc_sram.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif
#ifdef CONFIG_ARM_GIC
#include <asm/hardware/gic.h>
#endif
#ifdef CONFIG_HAVE_ARM_TWD
#include <asm/smp_twd.h>
#endif
#ifdef CONFIG_SMP
#include <asm/smp.h>
#include <asm/unified.h>
#endif

extern void __cpu_early_init(void);
extern void tcc_nfc_suspend(PNFC pBackupNFC , PNFC pHwNFC);
extern void tcc_nfc_resume(PNFC pHwNFC , PNFC pBackupNFC);

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
extern void tcc_pm_uart_suspend(bkUART *pBackupUART);
extern void tcc_pm_uart_resume(bkUART *pBackupUART);
#endif

/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/
#define addr_clk(b) (0x74000000+b)
#define addr_mem(b) (0x73000000+b)

#if defined(MEMBUS_CLK_AUTO_RESTORE)
#define SDRAM_INIT_PARAM(x)   (*(volatile unsigned long *)(SDRAM_INIT_PARAM_ADDR + (4*(x))))
enum {
	PLL_VALUE = 0,
	DDR_TCK,
};

typedef int (*AssemFuncPtr)(unsigned int dst, unsigned int src);
static unsigned int time2cycle(unsigned int time, unsigned int tCK)
{
	int cnt;
	if (time == 0)
		return 0;
	time = time + tCK - 1;
	cnt = 0;
	do {
		cnt++;
		time -= tCK;
	} while (time >= tCK);
	return cnt;
}
#else
#define time2cycle(time, tCK)		((int)((time + tCK - 1) / tCK))
#endif

#define denali_ctl(x) (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x) (*(volatile unsigned long *)addr_mem(0x600000+(x*4)))
#define ddr_phy(x) (*(volatile unsigned long *)addr_mem(0x420400+(x*4)))

typedef void (*FuncPtr)(void);

#ifdef CONFIG_CACHE_L2X0
#define L2CACHE_BASE   0xFB000000
#endif
#ifdef CONFIG_ARM_GIC
#define GIC_CPU_BASE   0xF8200100
#define GIC_DIST_BASE  0xF8201000
#endif
#ifdef CONFIG_HAVE_ARM_TWD
#define TWD_TIMER_BASE 0xF8200600
#endif
#ifdef CONFIG_SMP
#define SCU_BASE           0xF8200000
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define SEC_CPU_START_ADDR 0xF5400104
#else
#define SEC_CPU_START_ADDR 0xF000CDF8
#endif
extern void tcc893x_secondary_startup(void);
#endif

//#define TCC_PM_TP_DEBUG		// Test Port debugging - 120611, hjbae
#if defined(TCC_PM_TP_DEBUG)
#include <mach/gpio.h>

//#define TP_GPIO_PORT	(TCC_GPE(30))	// 0x1007 : UT_RTS2
#define TP_GPIO_PORT	(TCC_GPG(4))	// 0x1008 : SD2_WP
//#define TP_GPIO_PORT	(TCC_GPG(5))
#endif

#define OLD_SETTING
#define INIT_CONFIRM
#define PHY_MRS_SETTING
#define IMPROVE_DDI_PERFORMANCE
//#define DQS_EXTENSION
//#define ZDEN
//#define AUTO_TRAIN_CK_CHANGE
//#define CONFIG_DRAM_AUTO_TRAINING
//#define MANUAL_TRAIN
//#define MEM_BIST

#define __memcpy memcpy

static TCC_REG RegRepo;

#ifdef CONFIG_SNAPSHOT_BOOT
static void snapshot_wakeup(void)
{
	volatile unsigned int cnt;
	volatile unsigned *src;
	volatile unsigned *dest;

// -------------------------------------------------------------------------
// GPIO Restore

	src = (unsigned*)&RegRepo.gpio;
	dest = (unsigned*)tcc_p2v(HwGPIO_BASE);

	for(cnt=0 ; cnt<(sizeof(GPIO)/sizeof(unsigned)) ; cnt++)
		dest[cnt] = src[cnt];

// -------------------------------------------------------------------------
// clear wake-up
	//disable all for accessing PMU Reg. !!!
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_WKUP0.nREG = 0x00000000;
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_WKUP1.nREG = 0x00000000;
#ifdef TCC_PM_CHECK_WAKEUP_SOURCE
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_WKCLR0.nREG = 0xFFFFFFFF;
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_WKCLR1.nREG = 0xFFFFFFFF;
#endif

// -------------------------------------------------------------------------
// SSTL & IO Retention Disable

#ifndef TCC_PM_SSTLIO_CTRL
	while(!(((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_XIN.nREG&(1<<8))){
		BITSET(((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
	}
#endif
	while(!(((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_XIN.nREG&(1<<4))){
		BITSET(((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1
	}

// -------------------------------------------------------------------------
#if defined(CONFIG_MACH_TCC8930ST)
//    tcc_stb_resume();
#endif
// BUS Power On

#if defined(TCC_PM_PMU_CTRL)
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE != 1);
#else
	while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE);
#endif

	((PPMU)tcc_p2v(HwPMU_BASE0)->PWRUP_HSBUS.bREG.DATA = 1;
	while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_HSB == 0);
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.HSB = 1;
	((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.HSIOBUS = 1;
	while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.HSIOBUS == 0);

	((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_DBUS.bREG.DATA = 1;
	while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_DB == 0);
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.DB = 1;
	((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.DBUS = 1;
	while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS0 & ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS1) == 0);

	((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_GBUS.bREG.DATA = 1;
	while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_GB == 0);
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.GB = 1;
	((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.GBUS = 1;
	while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.GBUS == 0);

	((PPMU)tcc_p2v(HwPMU_BASE))->PWRUP_VBUS.bREG.DATA = 1;
	while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PU_VB == 0);
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.VB = 1;
	((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.VBUS = 1;
	while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS0 & ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS1) == 0);

	//IP isolation on
	//((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000BD0;
	((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = 0x00000000;
#endif
}

void snapshot_state_store(void)
{
	unsigned way_mask;
#if defined(MEMBUS_CLK_AUTO_RESTORE)
	unsigned int pll1,p,m,s, tck_value;
#endif

#if defined(MEMBUS_CLK_AUTO_RESTORE)
	__memcpy((void*)SDRAM_TIME2CYCLE_ADDR, (void*)time2cycle, SDRAM_TIME2CYCLE_SIZE);
	pll1 = (*(volatile unsigned long *)tcc_p2v(addr_clk(0x000034))) & 0x7FFFFFFF;
	SDRAM_INIT_PARAM(PLL_VALUE) = pll1;
	p = pll1&0x3F;
	m = (pll1>>8)&0x3FF;
	s = (pll1>>24)&0x7;
	tck_value = 1000000/((((XIN_CLK_RATE/10000)*m)/p)>>s);
	SDRAM_INIT_PARAM(DDR_TCK) = tck_value;
#endif

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
	/*--------------------------------------------------------------
	 UART block suspend
	--------------------------------------------------------------*/
	tcc_pm_uart_suspend(&RegRepo.bkuart);
#endif

	/*--------------------------------------------------------------
	 NFC suspend 
	--------------------------------------------------------------*/
	tcc_nfc_suspend(&RegRepo.nfc, (PNFC)tcc_p2v(HwNFC_BASE));

	/*--------------------------------------------------------------
	 BUS Config
	--------------------------------------------------------------*/
	__memcpy(&RegRepo.membuscfg, (PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), sizeof(MEMBUSCFG));
	__memcpy(&RegRepo.iobuscfg, (PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), sizeof(IOBUSCFG));

	/* all peri io bus on */
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

#if defined(CONFIG_ARM_GIC)
	/*--------------------------------------------------------------
	 GIC
	--------------------------------------------------------------*/
    cpu_pm_enter();
    cpu_cluster_pm_enter();
#endif

#ifdef CONFIG_HAVE_ARM_TWD
	/*--------------------------------------------------------------
	 ARM Private Timer (TWD)
	--------------------------------------------------------------*/
	RegRepo.twd_timer_control = *(volatile unsigned int *)(TWD_TIMER_BASE+TWD_TIMER_CONTROL);
	RegRepo.twd_timer_load = *(volatile unsigned int *)(TWD_TIMER_BASE+TWD_TIMER_LOAD);
#endif

	/*--------------------------------------------------------------
	 SMU & PMU
	--------------------------------------------------------------*/
	__memcpy(&RegRepo.smuconfig, (PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), sizeof(SMUCONFIG));
#if defined(TCC_PM_PMU_CTRL)
	__memcpy(&RegRepo.pmu, (PPMU)tcc_p2v(HwPMU_BASE), sizeof(PMU));
#endif
	__memcpy(&RegRepo.timer, (PTIMER *)tcc_p2v(HwTMR_BASE), sizeof(TIMER));
	__memcpy(&RegRepo.vic, (PVIC)tcc_p2v(HwVIC_BASE), sizeof(VIC));
	__memcpy(&RegRepo.pic, (PPIC)tcc_p2v(HwPIC_BASE), sizeof(PIC));
	__memcpy(&RegRepo.ckc, (PCKC)tcc_p2v(HwCKC_BASE), sizeof(CKC));

	/*--------------------------------------------------------------
	 GPIO
	--------------------------------------------------------------*/
	//__memcpy((void*)GPIO_REPOSITORY_ADDR, (PGPIO)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));
	__memcpy(&RegRepo.gpio, (PGPIO)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));
#ifdef CONFIG_CACHE_L2X0
	/*--------------------------------------------------------------
	 L2 cache
	--------------------------------------------------------------*/
	RegRepo.L2_aux = *(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL); //save aux
	if (RegRepo.L2_aux & (1 << 16))
		way_mask = (1 << 16) - 1;
	else
		way_mask = (1 << 8) - 1;
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CLEAN_WAY) = way_mask; //clean all
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CLEAN_WAY)&way_mask); //wait for clean
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0; //sync
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1); //wait for sync
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
#endif

#ifdef CONFIG_SMP
	/*--------------------------------------------------------------
	 SMP Setting.
	--------------------------------------------------------------*/
	//platform_smp_prepare_cpus(2);
	#ifdef CONFIG_ARM_ERRATA_764369 /* Cortex-A9 only */
	RegRepo.scu_0x30 = *(volatile unsigned int *)(SCU_BASE+0x30);
	#endif
	RegRepo.scu_ctrl = *(volatile unsigned int *)(SCU_BASE+0x00);
	BITCLR(*(volatile unsigned int *)(SCU_BASE+0x00), 0x1);
#endif

	/*--------------------------------------------------------------
	 flush tlb & cache
	--------------------------------------------------------------*/
	local_flush_tlb_all();
	flush_cache_all();
}

/*===========================================================================
FUNCTION
===========================================================================*/
void _snapshot_state_restore(void)
{
    int         cnt;
	unsigned    way_mask;

	__asm__ __volatile__ ("nop\n");


#if defined(TCC_PM_TP_DEBUG)	// 1
	gpio_set_value(TP_GPIO_PORT, 0);
#endif

	/*--------------------------------------------------------------
	 cpu re-init for VFP
	--------------------------------------------------------------*/
	__cpu_early_init();

	/* all peri io bus on */
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
	((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

#if defined(TCC_PM_TP_DEBUG)	// 2
	gpio_set_value(TP_GPIO_PORT, 1);
#endif

	/*--------------------------------------------------------------
	 SMU & PMU
	--------------------------------------------------------------*/

	//__memcpy((PCKC)tcc_p2v(HwCKC_BASE), &RegRepo.ckc, sizeof(CKC));
	{
		//PLL
		//((PCKC)tcc_p2v(HwCKC_BASE))->PLL0CFG.nREG = RegRepo.ckc.PLL0CFG.nREG;
		//((PCKC)tcc_p2v(HwCKC_BASE))->PLL1CFG.nREG = RegRepo.ckc.PLL1CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL2CFG.nREG = RegRepo.ckc.PLL2CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL3CFG.nREG = RegRepo.ckc.PLL3CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL4CFG.nREG = RegRepo.ckc.PLL4CFG.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->PLL5CFG.nREG = RegRepo.ckc.PLL5CFG.nREG;
		nop_delay(1000);

		//Divider
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC0.nREG = RegRepo.ckc.CLKDIVC0.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC1.nREG = RegRepo.ckc.CLKDIVC1.nREG;
		nop_delay(100);

		//((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL0.nREG = RegRepo.ckc.CLKCTRL0.nREG; //CPU Clock can't be adjusted freely.
		//((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL1.nREG = RegRepo.ckc.CLKCTRL1.nREG; //Memory Clock can't be adjusted freely.
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL2.nREG = RegRepo.ckc.CLKCTRL2.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL3.nREG = RegRepo.ckc.CLKCTRL3.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL4.nREG = RegRepo.ckc.CLKCTRL4.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL5.nREG = RegRepo.ckc.CLKCTRL5.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL6.nREG = RegRepo.ckc.CLKCTRL6.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL7.nREG = RegRepo.ckc.CLKCTRL7.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL8.nREG = RegRepo.ckc.CLKCTRL8.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL9.nREG = RegRepo.ckc.CLKCTRL9.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL10.nREG = RegRepo.ckc.CLKCTRL10.nREG;
		((PCKC)tcc_p2v(HwCKC_BASE))->SWRESET.nREG = ~(RegRepo.ckc.SWRESET.nREG);

		//Peripheral clock
		__memcpy((void*)&(((PCKC)tcc_p2v(HwCKC_BASE))->PCLKCTRL00.nREG), (void*)&(RegRepo.ckc.PCLKCTRL00.nREG), 0x17c-0x80);
	}
	nop_delay(100);

#if defined(TCC_PM_TP_DEBUG)	// 3
	gpio_set_value(TP_GPIO_PORT, 0);
#endif

#if defined(TCC_PM_PMU_CTRL)
	//__memcpy((PPMU)tcc_p2v(HwPMU_BASE), &RegRepo.pmu, sizeof(PMU));
	{
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
		while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE != 1);
#else
		while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE);
#endif

		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_HSB){ //if High Speed I/O Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.HSIOBUS = 0;
			while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.HSIOBUS == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.HSB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_HSBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_HSB == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_DB){ //if DDI Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.DBUS = 0;
			while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS0 | ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS1) == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.DB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_DBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_DB == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_GB){ //if Graphic Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.GBUS = 0;
			while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.GBUS == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.GB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_GBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_GB == 0);
		}
		if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_VB){ //if Video Bus has been turn off.
			((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.VBUS = 0;
			while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS0 | ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.VBUS1) == 1);
			((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.VB = 0;
			((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_VBUS.bREG.DATA = 1;
			while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_VB == 0);
		}

		//IP isolation restore
		//Bruce, PMU_ISOL is only write-only.. so it can't be set to back-up data..
		//((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = RegRepo.PMU_ISOL.nREG;
		((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = 0x00000000;
	}
#endif

#if defined(TCC_PM_TP_DEBUG)	// 4
	gpio_set_value(TP_GPIO_PORT, 1);
#endif

	__memcpy((PPIC)tcc_p2v(HwPIC_BASE), &RegRepo.pic, sizeof(PIC));
	__memcpy((PVIC)tcc_p2v(HwVIC_BASE), &RegRepo.vic, sizeof(VIC));
	__memcpy((PTIMER *)tcc_p2v(HwTMR_BASE), &RegRepo.timer, sizeof(TIMER));
	__memcpy((PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), &RegRepo.smuconfig, sizeof(SMUCONFIG));
	__memcpy((PGPIO)tcc_p2v(HwGPIO_BASE), &RegRepo.gpio, sizeof(GPIO));

#if defined(CONFIG_ARM_GIC)
    cpu_cluster_pm_exit();
    cpu_pm_exit();
#endif

	/*	Re-Init SRAM Functions */
	tcc_sram_init();

#ifdef CONFIG_HAVE_ARM_TWD
	*(volatile unsigned int *)(TWD_TIMER_BASE+TWD_TIMER_CONTROL) = RegRepo.twd_timer_control;
	*(volatile unsigned int *)(TWD_TIMER_BASE+TWD_TIMER_LOAD) = RegRepo.twd_timer_load;
#endif

#if defined(TCC_PM_TP_DEBUG)	// 5
	gpio_set_value(TP_GPIO_PORT, 0);
#endif

	/*--------------------------------------------------------------
	 BUS Config
	--------------------------------------------------------------*/
	__memcpy((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), &RegRepo.iobuscfg, sizeof(IOBUSCFG));
	__memcpy((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), &RegRepo.membuscfg, sizeof(MEMBUSCFG));

#if defined(TCC_PM_TP_DEBUG)	// 6
	gpio_set_value(TP_GPIO_PORT, 1);
#endif

	/*--------------------------------------------------------------
	 NFC
	--------------------------------------------------------------*/
	tcc_nfc_resume((PNFC)tcc_p2v(HwNFC_BASE), &RegRepo.nfc);	

#if defined(TCC_PM_TP_DEBUG)	// 7
	gpio_set_value(TP_GPIO_PORT, 0);
#endif

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
	/*--------------------------------------------------------------
	 UART block resume
	--------------------------------------------------------------*/
	tcc_pm_uart_resume(&RegRepo.bkuart);
#endif

#if defined(TCC_PM_TP_DEBUG)	// 8
	gpio_set_value(TP_GPIO_PORT, 1);
#endif

#if defined(TCC_PM_TP_DEBUG)	// 9
	gpio_set_value(TP_GPIO_PORT, 0);
#endif

#ifdef CONFIG_CACHE_L2X0
	/*--------------------------------------------------------------
	 L2 cache Clean & Disable
	--------------------------------------------------------------*/
	unsigned int L2_aux_tmp;
	L2_aux_tmp = *(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL); //save aux
	if (L2_aux_tmp & (1 << 16))
		way_mask = (1 << 16) - 1;
	else
		way_mask = (1 << 8) - 1;
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CLEAN_WAY) = way_mask; //clean all
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CLEAN_WAY)&way_mask); //wait for clean
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0; //sync
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1); //wait for sync
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off

	/*--------------------------------------------------------------
	 L2 cache Enable
	--------------------------------------------------------------*/
//	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL) = RegRepo.L2_aux; //restore aux
	if (RegRepo.L2_aux & (1 << 16))
		way_mask = (1 << 16) - 1;
	else
		way_mask = (1 << 8) - 1;
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY) = way_mask; //invalidate all
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY)&way_mask); //wait for invalidate
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0; //sync
	while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1); //wait for sync
	*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 1; //cache on
#endif

#ifdef CONFIG_SMP
	/*--------------------------------------------------------------
	 SMP Setting.
	--------------------------------------------------------------*/
	//platform_smp_prepare_cpus(2);
	#ifdef CONFIG_ARM_ERRATA_764369 /* Cortex-A9 only */
	*(volatile unsigned int *)(SCU_BASE+0x30) = RegRepo.scu_0x30;
	#endif
	*(volatile unsigned int *)(SCU_BASE+0x00) = RegRepo.scu_ctrl;
	flush_cache_all();

	__raw_writel((virt_to_phys(tcc893x_secondary_startup)), SEC_CPU_START_ADDR);
#endif

#if defined(TCC_PM_TP_DEBUG)	// 10
	gpio_set_value(TP_GPIO_PORT, 1);
#endif
}

void snapshot_state_restore(void)
{
	snapshot_wakeup();
	_snapshot_state_restore();
}
#endif

