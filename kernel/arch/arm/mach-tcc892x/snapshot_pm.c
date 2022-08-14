/****************************************************************************
 * arch/arm/mach-tcc892x/snapshot_pm.c
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
#include <asm/cacheflush.h>        // local_flush_tlb_all(), flush_cache_all();
#include <asm/system.h>
#include <mach/bsp.h>
#include <mach/system.h>
#include <mach/pm.h>
#include <mach/tcc_ddr.h>
#include <mach/tcc_sram.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif
#ifdef CONFIG_ARM_TRUSTZONE /* JJCONFIRMED */
#include <mach/smc.h>
#ifdef CONFIG_CACHE_L2X0
#endif
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

#if defined(CONFIG_CHIP_TCC8925S)
#define CPU_SRC_PLL 0    // cpu
#define CPU_PLL_ADDR 0x000030
#define MEM_SRC_PLL 1    // mem
#define MEM_PLL_ADDR 0x000034
#else
#define CPU_SRC_PLL 9    // cpu
#define CPU_PLL_ADDR 0x000044
#define MEM_SRC_PLL 8    // mem
#define MEM_PLL_ADDR 0x000040
#endif

#define nop_delay(x) for(cnt=0 ; cnt<x ; cnt++){ \
                    __asm__ __volatile__ ("nop\n"); }

#define addr_clk(b) (0x74000000+b)
#define addr_mem(b) (0x73000000+b)
#define time2cycle(time, tCK)        ((int)((time + tCK - 1) / tCK))

#define denali_ctl(x) (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x) (*(volatile unsigned long *)addr_mem(0x600000+(x)))
#define ddr_phy(x) (*(volatile unsigned long *)addr_mem(0x420400+(x)))

typedef void (*FuncPtr)(void);

#define L2CACHE_BASE   0xFA000000

#define LOG_NDEBUG 0

#if defined(TCC_PM_REGULATOR_CTRL)
#define addr_iocfg(b) (0x76066000+b)
#define PMIC_PARAM(x)   (*(volatile unsigned long *)(COREPWR_PARAM_ADDR + (4*(x))))
#define SLEEP           0
#define WAKEUP          1

enum {
    DEV_ADDR = 0,   /* i2c device address */
    CORE_CTRL_REG,  /* coreA power control register */
    READ_SIZE,      /* read data size */
    WRITE_SIZE,     /* write data size */
    WRITE_DATA1_S,  /* write data 1 (sleep) */
    WRITE_DATA2_S,  /* write data 2 (sleep) */
    WRITE_DATA1_W,  /* write data 1 (wakeup) */
    WRITE_DATA2_W,  /* write data 2 (wakeup) */
    DEV_CH,
    MACHINE_ID,
    SYSTEM_REV,
};
typedef void (*CoreFuncPtr)(unsigned int goto_state);
#endif

//#define TCC_PM_TP_DEBUG        // Test Port debugging - 120611, hjbae
#if defined(TCC_PM_TP_DEBUG)
#include <mach/gpio.h>

//#define TP_GPIO_PORT    (TCC_GPE(30))    // 0x1007 : UT_RTS2
#define TP_GPIO_PORT    (TCC_GPG(4))    // 0x1008 : SD2_WP
//#define TP_GPIO_PORT    (TCC_GPG(5))
#endif

static TCC_REG RegRepo;

#ifdef CONFIG_SNAPSHOT_BOOT
/*===========================================================================
FUNCTION
===========================================================================*/
static volatile unsigned int cnt = 0;
static unsigned way_mask;
static NFC *nfc;
void snapshot_state_store(void)
{
#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    /*--------------------------------------------------------------
     UART block suspend
    --------------------------------------------------------------*/
    tcc_pm_uart_suspend(&RegRepo.bkuart);
#endif
    /*--------------------------------------------------------------
     BUS Config
    --------------------------------------------------------------*/
    memcpy(&RegRepo.membuscfg, (PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), sizeof(MEMBUSCFG));
    memcpy(&RegRepo.iobuscfg, (PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), sizeof(IOBUSCFG));

    /* all peri io bus on */
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

    /*--------------------------------------------------------------
     SMU & PMU
    --------------------------------------------------------------*/
    memcpy(&RegRepo.smuconfig, (PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), sizeof(SMUCONFIG));
#if defined(TCC_PM_PMU_CTRL)
    memcpy(&RegRepo.pmu, (PPMU)tcc_p2v(HwPMU_BASE), sizeof(PMU));
#endif
    memcpy(&RegRepo.timer, (PTIMER *)tcc_p2v(HwTMR_BASE), sizeof(TIMER));
    memcpy(&RegRepo.vic, (PVIC)tcc_p2v(HwVIC_BASE), sizeof(VIC));
    memcpy(&RegRepo.pic, (PPIC)tcc_p2v(HwPIC_BASE), sizeof(PIC));
    memcpy(&RegRepo.ckc, (PCKC)tcc_p2v(HwCKC_BASE), sizeof(CKC));    
    memcpy(&RegRepo.gpio, (PGPIO)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));
    /*--------------------------------------------------------------
     NFC suspend 
    --------------------------------------------------------------*/
    //nfc = (NFC*)tcc_p2v(HwNFC_BASE);
    //tcc_nfc_suspend(&RegRepo.nfc, nfc);
    
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
    /*--------------------------------------------------------------
     flush tlb & cache
    --------------------------------------------------------------*/
    local_flush_tlb_all();
    flush_cache_all();
}

/*===========================================================================
FUNCTION
===========================================================================*/
void snapshot_state_restore(void)
{
    volatile unsigned int cnt;
    volatile unsigned *src;
    volatile unsigned *dest;
    
    __asm__ __volatile__ ("nop\n");

    /* all peri io bus on */
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

    /*--------------------------------------------------------------
     SMU & PMU
    --------------------------------------------------------------*/

    nop_delay(1000);

#if 1    // TO DO?
    //memcpy((PCKC)tcc_p2v(HwCKC_BASE), &RegRepo.ckc, sizeof(CKC));
    {
        //PLL
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL0CFG.nREG = RegRepo.ckc.PLL0CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL1CFG.nREG = RegRepo.ckc.PLL1CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL2CFG.nREG = RegRepo.ckc.PLL2CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL3CFG.nREG = RegRepo.ckc.PLL3CFG.nREG;
        //((PCKC)tcc_p2v(HwCKC_BASE))->PLL4CFG.nREG = RegRepo.ckc.PLL4CFG.nREG; //PLL4 is used as a source of memory bus clock
        //((PCKC)tcc_p2v(HwCKC_BASE))->PLL5CFG.nREG = RegRepo.ckc.PLL5CFG.nREG; //PLL5 is used as a source of CPU bus clock
        nop_delay(1000);

        //Divider
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC0.nREG = RegRepo.ckc.CLKDIVC0.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKDIVC1.nREG = RegRepo.ckc.CLKDIVC1.nREG;
        nop_delay(100);

        //((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL0.nREG = RegRepo.ckc.CLKCTRL0.nREG; //CPU Clock can't be adjusted freely.
        //((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL1.nREG = RegRepo.ckc.CLKCTRL1.nREG; //Memory Clock can't be adjusted freely.
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL2.nREG = RegRepo.ckc.CLKCTRL2.nREG;//Display bus
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL3.nREG = RegRepo.ckc.CLKCTRL3.nREG;//Graphic bus
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL4.nREG = RegRepo.ckc.CLKCTRL4.nREG;//I/O bus
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL5.nREG = RegRepo.ckc.CLKCTRL5.nREG;//Bus Clock for Video Codec
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL6.nREG = RegRepo.ckc.CLKCTRL6.nREG;//Core Clock for Video Codec
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL7.nREG = RegRepo.ckc.CLKCTRL7.nREG;//HSIO Bus
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL8.nREG = RegRepo.ckc.CLKCTRL8.nREG;//SMU Hardware

        ((PCKC)tcc_p2v(HwCKC_BASE))->SWRESET.nREG = ~(RegRepo.ckc.SWRESET.nREG);

        //Peripheral clock
        memcpy((void*)&(((PCKC)tcc_p2v(HwCKC_BASE))->PCLKCTRL00.nREG), (void*)&(RegRepo.ckc.PCLKCTRL00.nREG), 0x150-0x80);
    }
#endif
    nop_delay(100);


#if defined(TCC_PM_PMU_CTRL)
    //memcpy((PPMU)tcc_p2v(HwPMU_BASE), &RegRepo.pmu, sizeof(PMU));
    {

        ((PPMU)tcc_p2v(HwPMU_BASE))->PMU_TSADC.nREG = RegRepo.pmu.PMU_TSADC.nREG;
        ((PPMU)tcc_p2v(HwPMU_BASE))->PMU_CONFIG.nREG = RegRepo.pmu.PMU_CONFIG.nREG;
    
        while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE);

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

    memcpy((PPIC)tcc_p2v(HwPIC_BASE), &RegRepo.pic, sizeof(PIC));
    memcpy((PVIC)tcc_p2v(HwVIC_BASE), &RegRepo.vic, sizeof(VIC));
    memcpy((PTIMER *)tcc_p2v(HwTMR_BASE), &RegRepo.timer, sizeof(TIMER));
    memcpy((PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), &RegRepo.smuconfig, sizeof(SMUCONFIG));
    memcpy((PGPIO)tcc_p2v(HwGPIO_BASE), &RegRepo.gpio, sizeof(GPIO));    
    /*--------------------------------------------------------------
     NFC
    --------------------------------------------------------------*/
    //tcc_nfc_resume(nfc, &RegRepo.nfc);    

    /*--------------------------------------------------------------
     BUS Config
    --------------------------------------------------------------*/
    memcpy((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), &RegRepo.iobuscfg, sizeof(IOBUSCFG));
    memcpy((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), &RegRepo.membuscfg, sizeof(MEMBUSCFG));

    //memcpy((PPMU)tcc_p2v(HwPMU_BASE), &RegRepo.pmu, sizeof(PMU));
    //((PPMU)tcc_p2v(HwPMU_BASE))->PMU_ISOL.nREG = 0x00000000;

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    /*--------------------------------------------------------------
     UART block resume
    --------------------------------------------------------------*/
    tcc_pm_uart_resume(&RegRepo.bkuart);
#endif

    /*--------------------------------------------------------------
     cpu re-init for VFP
    --------------------------------------------------------------*/
    __cpu_early_init();
    tcc_sram_init();
    nop_delay(100);
    
#ifdef CONFIG_CACHE_L2X0
    /*--------------------------------------------------------------
     L2 cache enable
    --------------------------------------------------------------*/
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
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
}
#endif

