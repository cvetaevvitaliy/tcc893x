/****************************************************************************
 * arch/arm/mach-tcc893x/pm.c
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

#include <linux/module.h>
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
#include <mach/sram_map.h>
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
#ifdef CONFIG_ARM_TRUSTZONE
#include <mach/smc.h>
#endif

extern void save_cpu_reg(int sram_addr, unsigned int pReg, void *);
extern void resore_cpu_reg(void);
extern void __cpu_early_init(void);
extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

static TCC_REG RegRepo;

/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

/* tcc pm board configuration index */
enum {
    BOARD_PARAM_PLL25_PORT = 0,
#ifdef CONFIG_INPUT_TCC_REMOTE
    BOARD_PARAM_PMU_CFG_RMISEL,
#endif
    BOARD_PARAM_PMU_WKUP0,
    BOARD_PARAM_PMU_WKPOL0,
    BOARD_PARAM_PMU_WKUP1,
    BOARD_PARAM_PMU_WKPOL1,
#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    BOARD_PARAM_PMU_WUSTS0,
    BOARD_PARAM_PMU_WUSTS1,
#endif
};

enum {
    PMU_WKUP0_TSADC_UPDOWN = 0,
    PMU_WKUP0_TSADC_STOP_WKU,
    PMU_WKUP0_TSADC_WAKEUP,
    PMU_WKUP0_RTC_WAKEUP,
    PMU_WKUP0_RESERVED,
    PMU_WKUP0_GP_D08,
    PMU_WKUP0_GP_D09,
    PMU_WKUP0_GP_D12,
    PMU_WKUP0_GP_D13,
    PMU_WKUP0_GP_D14,
    PMU_WKUP0_GP_B11,       // 10
    PMU_WKUP0_GP_B12,
    PMU_WKUP0_GP_B13,
    PMU_WKUP0_GP_B14,
    PMU_WKUP0_GP_B15,
    PMU_WKUP0_GP_C00,
    PMU_WKUP0_GP_G05,
    PMU_WKUP0_GP_G08,
    PMU_WKUP0_GP_G09,
    PMU_WKUP0_GP_G10,
    PMU_WKUP0_GP_G11,       // 20
    PMU_WKUP0_GP_G12,
    PMU_WKUP0_GP_G13,
    PMU_WKUP0_GP_G14,
    PMU_WKUP0_GP_G16,
    PMU_WKUP0_GP_G17,
    PMU_WKUP0_GP_G18,
    PMU_WKUP0_GP_G19,
    PMU_WKUP0_GP_HDMI00,
    PMU_WKUP0_GP_HDMI01,
    PMU_WKUP0_GP_ADC04,     // 30
    PMU_WKUP0_GP_ADC05,
};

enum {
    PMU_WKUP1_GP_E00 = 0,
    PMU_WKUP1_GP_E01,
    PMU_WKUP1_GP_E02,
    PMU_WKUP1_GP_E03,
    PMU_WKUP1_GP_E04,
    PMU_WKUP1_GP_E05,
    PMU_WKUP1_GP_E06,
    PMU_WKUP1_GP_E07,
    PMU_WKUP1_GP_E08,
    PMU_WKUP1_GP_E09,
    PMU_WKUP1_GP_E10,
    PMU_WKUP1_GP_E11,
    PMU_WKUP1_GP_E12,
    PMU_WKUP1_GP_E13,
    PMU_WKUP1_GP_E14,
    PMU_WKUP1_GP_E15,
    PMU_WKUP1_GP_E16,
    PMU_WKUP1_GP_E17,
    PMU_WKUP1_GP_E18,
    PMU_WKUP1_GP_E19,
    PMU_WKUP1_GP_E20,
    PMU_WKUP1_GP_E21,
    PMU_WKUP1_GP_E22,
    PMU_WKUP1_GP_E23,
    PMU_WKUP1_GP_E24,
    PMU_WKUP1_GP_E25,
    PMU_WKUP1_GP_E26,
    PMU_WKUP1_GP_E27,
    PMU_WKUP1_GP_E28,
    PMU_WKUP1_GP_E29,
    PMU_WKUP1_GP_E30,
    PMU_WKUP1_RESERVED,
};

#define tcc_pm_board_param(x) (*(volatile unsigned long *)(BOARD_PARAMETER_ADDR+(4*x)))

#define nop_delay(x) for(cnt=0 ; cnt<x ; cnt++){ \
                    __asm__ __volatile__ ("nop\n"); }

#define addr_clk(b) (0x74000000+b)
#define addr_mem(b) (0x73000000+b)

#if defined(MEMBUS_CLK_AUTO_RESTORE)
#define SDRAM_INIT_PARAM(x)   (*(volatile unsigned long *)(SDRAM_INIT_PARAM_ADDR + (4*(x))))
enum {
    PLL_VALUE = 0,
    DDR_TCK,
#ifdef CONFIG_SUSPEND_MEMCLK
    PLL2_VALUE,
    PLL3_VALUE,
    PLL4_VALUE,
    PLL5_VALUE,
    CKC1_VALUE,    /* membus */
#endif
};

typedef int (*AssemFuncPtr)(unsigned int dst, unsigned int src);
unsigned int tcc_pm_time2cycle(unsigned int time, unsigned int tCK)
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
EXPORT_SYMBOL(tcc_pm_time2cycle);
#else
#define time2cycle(time, tCK)        ((int)((time + tCK - 1) / tCK))
#endif

#define denali_ctl(x)   (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x)   (*(volatile unsigned long *)addr_mem(0x600000+(x*4)))
#define ddr_phy(x)      (*(volatile unsigned long *)addr_mem(0x420400+(x*4)))

typedef void (*FuncPtr)(void);

#ifdef CONFIG_CACHE_L2X0
#define L2CACHE_BASE        0xFB000000
#endif
#ifdef CONFIG_ARM_GIC
#define GIC_CPU_BASE        0xF8200100
#define GIC_DIST_BASE       0xF8201000
#endif
#ifdef CONFIG_HAVE_ARM_TWD
#define TWD_TIMER_BASE      0xF8200600
#endif
#ifdef CONFIG_SMP
#define SCU_BASE            0xF8200000
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define SEC_CPU_START_ADDR  0xF5400104
#else
#define SEC_CPU_START_ADDR  0xF000CDF8
#endif
extern void tcc893x_secondary_startup(void);
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

/*===========================================================================
FUNCTION
===========================================================================*/
#if defined(CONFIG_MACH_TCC8930ST)
static inline void tcc_stb_suspend(void)
{
#if defined(TCC_PM_MEMQ_PWR_CTRL)
#if defined(CONFIG_STB_BOARD_YJ8935T)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //VDDQ_MEM_ON : low
#else
    BITCLR(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<24); //VDDQ_MEM_ON : low
#endif /* CONFIG_STB_BOARD_YJ8935T */
#endif    

#if defined(CONFIG_STB_BOARD_YJ8935T)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<8); //SLEEP_MODE_CTL : low
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */

    BITCLR(((PGPIO)HwGPIO_BASE)->GPGDAT.nREG, 1<<14); /* CPU# */
#elif defined(CONFIG_STB_BOARD_YJ8933T)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<8); //SLEEP_MODE_CTL : low
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */

    BITCLR(((PGPIO)HwGPIO_BASE)->GPGDAT.nREG, 1<<15); /* CPU# */
    BITCLR(((PGPIO)HwGPIO_BASE)->GPGDAT.nREG, 1<<16); /* CPU# */
#elif defined(CONFIG_STB_BOARD_UPC_TCC893X)
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPEDAT.nREG, 1<<15); /* LED_S_PN */
#else
	BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //SLEEP_MODE_CTL : low
    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<22); //PLL25_ON : low

    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<19); //CORE1_ON : low
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<21); //CORE2_ON : high

    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<24); /* LED_S_PN */
    BITCLR(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<27); /* PHY1_ON */
#endif /* CONFIG_STB_BOARD_YJ8935T */
}

static inline void tcc_stb_resume(void)
{
#if defined(TCC_PM_MEMQ_PWR_CTRL)
#if defined(CONFIG_STB_BOARD_YJ8935T)
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //VDDQ_MEM_ON : high
#else
    BITSET(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<24); //VDDQ_MEM_ON : high
#endif /* CONFIG_STB_BOARD_YJ8935T */
#endif        
#if defined(CONFIG_STB_BOARD_YJ8935T)
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<8); //SLEEP_MODE_CTL : high
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */
    BITSET(((PGPIO)HwGPIO_BASE)->GPGDAT.nREG, 1<<14); /* CPU# */
#elif  defined(CONFIG_STB_BOARD_YJ8933T)    
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<8); //SLEEP_MODE_CTL : high
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */

    BITSET(((PGPIO)HwGPIO_BASE)->GPGDAT.nREG, 1<<15); /* CPU# */
    BITSET(((PGPIO)HwGPIO_BASE)->GPGDAT.nREG, 1<<16); /* CPU# */
#elif defined(CONFIG_STB_BOARD_UPC_TCC893X)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPEDAT.nREG, 1<<15); /* LED_S_PN */
#else
	BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //SLEEP_MODE_CTL : high
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<22); //PLL25_ON : low

    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<19); //CORE1_ON : high
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<21); //CORE2_ON : high    

    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<24); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<27); /* PHY1_ON */

#endif /* CONFIG_STB_BOARD_YJ8935T */
}
#endif

void tcc_pm_sdram_init(void)
{
#if defined(CONFIG_DRAM_DDR3)
    volatile int i;
    register unsigned int tmp;
    uint nCL, nCWL;
#ifdef CONFIG_DRAM_AUTO_TRAINING
    int count = 16;
#endif

#ifdef MANUAL_TRAIN0
// WL delay    
    unsigned int    b0_wld = 0x00000010;
    unsigned int    b1_wld = 0x00000010;
    unsigned int    b2_wld = 0x00000010;
    unsigned int    b3_wld = 0x00000010;

// DQS gate delay
    unsigned int    b0_dqs_gd = 0x00000050;
    unsigned int    b1_dqs_gd = 0x00000050;
    unsigned int    b2_dqs_gd = 0x00000050;
    unsigned int    b3_dqs_gd = 0x00000050;
     
    unsigned int    b0_dqs_gsl = 0x00000000;
    unsigned int    b1_dqs_gsl = 0x00000000;
    unsigned int    b2_dqs_gsl = 0x00000000;
    unsigned int    b3_dqs_gsl = 0x00000000;

// READ DQS Master delay
    unsigned int    b0_rdqsn_d = 0x00000000;
    unsigned int    b1_rdqsn_d = 0x00000000;
    unsigned int    b2_rdqsn_d = 0x00000000;
    unsigned int    b3_rdqsn_d = 0x00000000;

    unsigned int    b0_rdqs_d = 0x00000020;
    unsigned int    b1_rdqs_d = 0x00000020;
    unsigned int    b2_rdqs_d = 0x00000020;
    unsigned int    b3_rdqs_d = 0x00000020;

// READ delays
//
    unsigned int    b0_rmdl        = 0x60;
    unsigned int    b1_rmdl        = 0x60;
    unsigned int    b2_rmdl        = 0x60;
    unsigned int    b3_rmdl        = 0x60;

    unsigned int    b0_dm_rbd    = 0x00;
    unsigned int    b0_dq0_rbd     = 0x00;
    unsigned int    b0_dq1_rbd     = 0x00;
    unsigned int    b0_dq2_rbd     = 0x00;
    unsigned int    b0_dq3_rbd     = 0x00;
    unsigned int    b0_dq4_rbd     = 0x00;
    unsigned int    b0_dq5_rbd     = 0x00;
    unsigned int    b0_dq6_rbd     = 0x00;
    unsigned int    b0_dq7_rbd     = 0x00;

    unsigned int    b1_dm_rbd    = 0x00;
    unsigned int    b1_dq0_rbd     = 0x00;
    unsigned int    b1_dq1_rbd     = 0x00;
    unsigned int    b1_dq2_rbd     = 0x00;
    unsigned int    b1_dq3_rbd     = 0x00;
    unsigned int    b1_dq4_rbd     = 0x00;
    unsigned int    b1_dq5_rbd     = 0x00;
    unsigned int    b1_dq6_rbd     = 0x00;
    unsigned int    b1_dq7_rbd     = 0x00;

    unsigned int    b2_dm_rbd    = 0x00;
    unsigned int    b2_dq0_rbd     = 0x00;
    unsigned int    b2_dq1_rbd     = 0x00;
    unsigned int    b2_dq2_rbd     = 0x00;
    unsigned int    b2_dq3_rbd     = 0x00;
    unsigned int    b2_dq4_rbd     = 0x00;
    unsigned int    b2_dq5_rbd     = 0x00;
    unsigned int    b2_dq6_rbd     = 0x00;
    unsigned int    b2_dq7_rbd     = 0x00;

    unsigned int    b3_dm_rbd    = 0x00;
    unsigned int    b3_dq0_rbd     = 0x00;
    unsigned int    b3_dq1_rbd     = 0x00;
    unsigned int    b3_dq2_rbd     = 0x00;
    unsigned int    b3_dq3_rbd     = 0x00;
    unsigned int    b3_dq4_rbd     = 0x00;
    unsigned int    b3_dq5_rbd     = 0x00;
    unsigned int    b3_dq6_rbd     = 0x00;
    unsigned int    b3_dq7_rbd     = 0x00;

// WRITE DQ Master Delay
    unsigned int    b0_wdq_d = 0x00000020;
    unsigned int    b1_wdq_d = 0x00000020;
    unsigned int    b2_wdq_d = 0x00000020;
    unsigned int    b3_wdq_d = 0x00000020;
    
// WRITE delays
    unsigned int    b0_dqs_wbd    = 0x00;
    unsigned int    b0_dm_wbd    = 0x00;
    unsigned int    b0_dq0_wbd     = 0x00;
    unsigned int    b0_dq1_wbd     = 0x00;
    unsigned int    b0_dq2_wbd     = 0x00;
    unsigned int    b0_dq3_wbd     = 0x00;
    unsigned int    b0_dq4_wbd     = 0x00;
    unsigned int    b0_dq5_wbd     = 0x00;
    unsigned int    b0_dq6_wbd     = 0x00;
    unsigned int    b0_dq7_wbd     = 0x00;

    unsigned int    b1_dqs_wbd    = 0x00;
    unsigned int    b1_dm_wbd    = 0x00;
    unsigned int    b1_dq0_wbd     = 0x00;
    unsigned int    b1_dq1_wbd     = 0x00;
    unsigned int    b1_dq2_wbd     = 0x00;
    unsigned int    b1_dq3_wbd     = 0x00;
    unsigned int    b1_dq4_wbd     = 0x00;
    unsigned int    b1_dq5_wbd     = 0x00;
    unsigned int    b1_dq6_wbd     = 0x00;
    unsigned int    b1_dq7_wbd     = 0x00;

    unsigned int    b2_dqs_wbd    = 0x00;
    unsigned int    b2_dm_wbd    = 0x00;
    unsigned int    b2_dq0_wbd     = 0x00;
    unsigned int    b2_dq1_wbd     = 0x00;
    unsigned int    b2_dq2_wbd     = 0x00;
    unsigned int    b2_dq3_wbd     = 0x00;
    unsigned int    b2_dq4_wbd     = 0x00;
    unsigned int    b2_dq5_wbd     = 0x00;
    unsigned int    b2_dq6_wbd     = 0x00;
    unsigned int    b2_dq7_wbd     = 0x00;

    unsigned int    b3_dqs_wbd    = 0x00;
    unsigned int    b3_dm_wbd    = 0x00;
    unsigned int    b3_dq0_wbd     = 0x00;
    unsigned int    b3_dq1_wbd     = 0x00;
    unsigned int    b3_dq2_wbd     = 0x00;
    unsigned int    b3_dq3_wbd     = 0x00;
    unsigned int    b3_dq4_wbd     = 0x00;
    unsigned int    b3_dq5_wbd     = 0x00;
    unsigned int    b3_dq6_wbd     = 0x00;
    unsigned int    b3_dq7_wbd     = 0x00;

// Not Used
    unsigned int    b0_dqsn_rbd = 0x00;
    unsigned int    b0_dqs_rbd = 0x00;
    unsigned int    b0_dq_oebd = 0x00;
    unsigned int    b0_dqs_oebd = 0x00;

    unsigned int    b1_dqsn_rbd = 0x00;
    unsigned int    b1_dqs_rbd = 0x00;
    unsigned int    b1_dq_oebd = 0x00;
    unsigned int    b1_dqs_oebd = 0x00;

    unsigned int    b2_dqsn_rbd = 0x00;
    unsigned int    b2_dqs_rbd = 0x00;
    unsigned int    b2_dq_oebd = 0x00;
    unsigned int    b2_dqs_oebd = 0x00;

    unsigned int    b3_dqsn_rbd = 0x00;
    unsigned int    b3_dqs_rbd = 0x00;
    unsigned int    b3_dq_oebd = 0x00;
    unsigned int    b3_dqs_oebd = 0x00;
#endif

    //#define PLL0_VALUE      0x01013806    // PLL0 : 624MHz for CPU
    #define PLL0_VALUE      0x4201A906    // PLL0 : 425MHz for CPU
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    AssemFuncPtr time2cycle = (AssemFuncPtr)(SDRAM_TIME2CYCLE_ADDR);
    #define PLL1_VALUE      SDRAM_INIT_PARAM(PLL_VALUE)    // PLL1 for MEM
    #define tCK SDRAM_INIT_PARAM(DDR_TCK)
#else
    #define PLL1_VALUE      0x01012C06    // PLL1 : 600MHz for MEM
    #define MEMBUS_CLK      300 // = PLL1/4
    //#define PLL1_VALUE      0x4101F406    // PLL1 : 1GHz for MEM
    //#define MEMBUS_CLK      300 // = PLL1/4
    #define DDR3_CLK        (MEMBUS_CLK*2)
    #define tCK (1000000/DDR3_CLK)
#endif

//--------------------------------------------------------------------------
//Clock setting..

#ifdef CONFIG_SUSPEND_MEMCLK
    *(volatile unsigned long *)addr_clk(0x000038) = SDRAM_INIT_PARAM(PLL2_VALUE);
    *(volatile unsigned long *)addr_clk(0x00003C) = SDRAM_INIT_PARAM(PLL3_VALUE);
    *(volatile unsigned long *)addr_clk(0x000040) = SDRAM_INIT_PARAM(PLL4_VALUE);
    *(volatile unsigned long *)addr_clk(0x000044) = SDRAM_INIT_PARAM(PLL5_VALUE);
#endif
 
    *(volatile unsigned long *)addr_clk(0x000000) = 0x002ffff4; //cpu bus : XIN
    *(volatile unsigned long *)addr_clk(0x000004) = 0x00200014; //mem bus : XIN/2 
    *(volatile unsigned long *)addr_clk(0x000010) = 0x00200014; //io bus : XIN/2
    *(volatile unsigned long *)addr_clk(0x000020) = 0x00200014; //smu bus : XIN/2
    *(volatile unsigned long *)addr_clk(0x000030) = PLL0_VALUE; //pll0 for cpu
    *(volatile unsigned long *)addr_clk(0x000030) = 0x80000000 | PLL0_VALUE;
    *(volatile unsigned long *)addr_clk(0x000034) = PLL1_VALUE; //pll1 for mem
    *(volatile unsigned long *)addr_clk(0x000034) = 0x80000000 | PLL1_VALUE;
    i=3200; while(i--);
    //*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
    *(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
    //*(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
#ifdef CONFIG_SUSPEND_MEMCLK
    *(volatile unsigned long *)addr_clk(0x000004) = SDRAM_INIT_PARAM(CKC1_VALUE); //0x00200011;  // mem bus
#else
    *(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
#endif
    *(volatile unsigned long *)addr_clk(0x000010) = 0x00200011;  // io bus
    *(volatile unsigned long *)addr_clk(0x000020) = 0x00200011;  // smu bus
    *(volatile unsigned long *)addr_clk(0x0000E0) = 0x2000001A;  // sdmmc3 peri (24MHz)    // for emmcboot
    i=3200; while(i--);


//--------------------------------------------------------------------------
// Phy setting..

    //----------------------------------------
    // PHY RESETn (de-assert)
    denali_phy(2) = 0x0; // phy cfg resetn
    i=3200; while(i--);
    denali_phy(2) = 0x1; // phy cfg resetn
    //----------------------------------------

    //----------------------------------------
    // PHY Configuration
#if (0) /* This is 666MHz init values. but it is not tested a lot. */
    ddr_phy(0xb) = (266400<<0) | (1333<<18);  // PTR4
    ddr_phy(0x7) = (16<<0) | (1333<<6) | (334<<21);  // PTR0
    ddr_phy(0x8) = (2997<<0) | (33300<<16);  // PTR1
    ddr_phy(0x9) = (0x0F<<0) | (0x0F<<5) | (0x0F<<10) | (0x10<<15);  // PTR2
    ddr_phy(0xa) = (666000<<0) | (227<<20);  // PTR3
#else
    ddr_phy(0xb) = (200<<0) | (638<<18);  // PTR4
    ddr_phy(0x7) = (0x10<<0) | (0x07D0<<6) | (0x190<<21);  // PTR0
    ddr_phy(0x8) = (0x578<<0) | (0x2EE0<<16);  // PTR1
    ddr_phy(0x9) = (0x0F<<0) | (0x0F<<5) | (0x0F<<10) | (0x10<<15);  // PTR2
    ddr_phy(0xa) = (533334<<0) | (384<<20);  // PTR3
#endif
    ddr_phy(0x1) = 0x00050000;

#ifdef ZDEN
    ddr_phy(0x1) = 0x40050000;
#else
    ddr_phy(0x1) = 0x00050000;
#endif


#ifdef ZDEN
// ZQ over-ride
    ddr_phy(0x60) =   ( 0x1 << 29 )        // ZCALBYP
            | ( 0x0 << 30 )        // ZCALEN
            | ( 0x1 << 28 )        // ZDEN
            | ( 0x0D << 15 )    // pull-up ODT 58ohm fix
            | ( 0x0D << 10 )    // pull-down don't care
            | ( 0x09 <<  5 )    // pull-up output Z 40.6 ohm
            | ( 0x09 <<  0 );    // pull-down don't care
#endif
#if defined(CONFIG_DRAM_16BIT_USED)
    // 16bit
    ddr_phy(0x90) = (0x0<<0)|(0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<14);    // DX2GCR disable, power-down all  
    ddr_phy(0xa0) = (0x0<<0)|(0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<14);    // DX3GCR disable, power-down all  
#endif

#if defined(DQS_EXTENSION)
    ddr_phy(0x10) |= (1<<6);  // edit DQS range
    ddr_phy(0x7A) = 0x0;
    ddr_phy(0x8A) = 0x0;
    ddr_phy(0x9A) = 0x0;
    ddr_phy(0xAA) = 0x0;
#else
    #ifndef CONFIG_DRAM_AUTO_TRAINING
        #ifdef CONFIG_TCC_MEM_EXTRA_DQSDC_1
        ddr_phy(0x7c) = 0x00005001;
        ddr_phy(0x8c) = 0x00005001;
        ddr_phy(0x9c) = 0x00005001;
        ddr_phy(0xac) = 0x00005001;
        #endif

        #ifdef CONFIG_TCC_MEM_EXTRA_DQSDC_2
        ddr_phy(0x7c) = 0x00005002;
        ddr_phy(0x8c) = 0x00005002;
        ddr_phy(0x9c) = 0x00005002;
        ddr_phy(0xac) = 0x00005002;
        #endif

        ddr_phy(0x7A) = 0x50505050;    //0x20202020;
        ddr_phy(0x8A) = 0x50505050;    //0x20202020;
        ddr_phy(0x9A) = 0x50505050;    //0x20202020;
        ddr_phy(0xAA) = 0x50505050;    //0x20202020;
    #endif
    
    #ifdef CONFIG_ODT_TCC8931
        ddr_phy(0x61) = (3<<4) | (15<<0);
        ddr_phy(0x65) = (3<<4) | (15<<0);
        ddr_phy(0x69) = (3<<4) | (15<<0);
        ddr_phy(0x6D) = (3<<4) | (15<<0);
    #endif

    #if defined(CONFIG_MACH_M805_893X) && defined(CONFIG_CHIP_TCC8937S)
    ddr_phy(0x61) = (2<<4) | (10<<0);
    ddr_phy(0x65) = (2<<4) | (10<<0);
    ddr_phy(0x69) = (2<<4) | (10<<0);
    ddr_phy(0x6D) = (2<<4) | (10<<0);
    #endif
#endif

    (*(volatile unsigned long *)0x73810010) |= 0x00008000; //Hw15; MBUSCFG.DCLKR - Bus Clock : DRAM Clock = 1:2

//--------------------------------------------------------------------------
// Timing Parameter

    if(tCK >= 2500){ /* 2.5ns, 400MHz */
        nCL = 6;
        nCWL = 5;
    }else if(tCK >= 1875){ // 1.875ns, 533..MHz
        nCL = 8;
        nCWL = 6;
    }else if(tCK >= 1500){ // 1.5 ns, 666..MHz
        if(DDR3_MAX_SPEED < DDR3_1600)
            nCL = 9;
        else
            nCL = 10;
#if defined(CONFIG_MACH_TCC8930ST) && defined(CONFIG_CHIP_TCC8930)
        nCWL = 8;
#else
        nCWL = 7;
#endif
    }else if(tCK >= 1250){ // 1.25 ns, 800MHz
        nCL = 11;
        nCWL = 8;
    }else if(tCK >= 1070){ // 1.07 ns, 933..MHz
        nCL = 13;
        nCWL = 9;
    }else if(tCK >= 935){ // 0.935 ns, 1066..MHz
        nCL = 14;
        nCWL = 10;
    }
    else {
        nCL = 14;
        nCWL = 10;
    }

    denali_ctl(0) = 0x20410600; //DRAM_CLASS[11:8] = 6:DDR3, 4:DDR2

#ifdef OLD_SETTING
    denali_ctl(2) = time2cycle(10000, tCK); //TINIT[23:0]
    denali_ctl(3) = 0; //time2cycle(200000000, tCK); //TRST_PWRON
    denali_ctl(4) = time2cycle(500000000, tCK); //CKE_INACTIVE
    if(DDR3_AL == AL_DISABLED){ //nAL = 0;
        denali_ctl(5) = (0x1<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x8);
    }
    else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
        denali_ctl(5) = (0x1<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x8);
    }    
    else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
        denali_ctl(5) = (0x1<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x8);
    }
#else
    denali_ctl(2) = 0x00000007; //TINIT[23:0]
    denali_ctl(3) = 0; //0x000000a0; //TRST_PWRON
    denali_ctl(4) = 0x00000090; //CKE_INACTIVE
    if(DDR3_AL == AL_DISABLED){ //nAL = 0;
        denali_ctl(5) = (0x4<<24 | (nCWL)<<16 | ((nCL<<1)|0)<<8 | 0x0);
    }
    else if(DDR3_AL == AL_CL_MINUS_ONE){ //nAL = nCL - 1;
        denali_ctl(5) = (0x4<<24 | (nCWL+nCL-1)<<16 | ((nCL<<1)|0)<<8 | 0x0);
    }    
    else if(DDR3_AL == AL_CL_MINUS_TWO){ //nAL = nCL - 2;
        denali_ctl(5) = (0x4<<24 | (nCWL+nCL-2)<<16 | ((nCL<<1)|0)<<8 | 0x0);
    }
#endif

    denali_ctl(6) = (time2cycle(DDR3_tRAS_ps,tCK)<<24 | time2cycle(DDR3_tRC_ps,tCK)<<16 | time2cycle(DDR3_tRRD_ps,tCK)<<8 | DDR3_tCCD_ck);
    denali_ctl(7) = (DDR3_tMRD_ck<<24 | time2cycle(DDR3_tRTP_ps,tCK)<<16 | time2cycle(DDR3_tRP_ps,tCK)<<8 | (time2cycle(DDR3_tWTR_ps,tCK)+1));
    if(time2cycle(DDR3_tMOD_ps,tCK) < DDR3_tMOD_ck)
        denali_ctl(8) = (time2cycle(DDR3_tRAS_MAX_ps,tCK)<<8 | DDR3_tMOD_ck);
    else
        denali_ctl(8) = (time2cycle(DDR3_tRAS_MAX_ps,tCK)<<8 | time2cycle(DDR3_tMOD_ps,tCK));
    denali_ctl(9) = ((time2cycle(DDR3_tCKE_ps,tCK)+1)<<8 | time2cycle(DDR3_tCKE_ps,tCK));
    denali_ctl(10) = (time2cycle(DDR3_tWR_ps,tCK)<<24 | time2cycle(DDR3_tRCD_ps,tCK)<<16 | 1<<8 | 1);
    denali_ctl(11) = (1<<24 | DDR3_tDLLK_ck<<8 | (time2cycle(DDR3_tWR_ps,tCK)+nCL));
    denali_ctl(12) = (1<<16 | time2cycle(DDR3_tFAW_ps,tCK)<<8 | 3);
    denali_ctl(13) = time2cycle(DDR3_tRP_ps,tCK)+1;
    denali_ctl(14) = (time2cycle(DDR3_tRFC_ps,tCK)<<16 | 1<<8 | 0);
    denali_ctl(15) = time2cycle(DDR3_tREFI_ps,tCK);
    denali_ctl(16) = (time2cycle(DDR3_tXPDLL_ps,tCK)<<16 | time2cycle(DDR3_tXP_ps,tCK)); // DDR3 Only
    denali_ctl(17) = 0x0;//(6<<16 | 2); //TXARD[-0] = 0x2, TXARDS[-16] = 0x6 // DDR2 only
    denali_ctl(18) = (time2cycle(DDR3_tXS_ps,tCK)<<16 | DDR3_tXSDLL_ck);
    denali_ctl(19) = (1<<16); //ENABLE_QUICK_SREFRESH = 0x1
    denali_ctl(20) = (time2cycle(DDR3_tCKSRX_ps,tCK)<<16 | time2cycle(DDR3_tCKSRE_ps,tCK)<<8);
    denali_ctl(21) = 0; denali_ctl(24) = 0; denali_ctl(25) = 0; denali_ctl(26) = 0;

//--------------------------------------------------------------------------
// MRS Setting

    // MRS0
    tmp = DDR3_BURST_LEN | (DDR3_READ_BURST_TYPE<<3);

    if(nCL < 5)
        tmp |= ((5-4)<<4);
    else if(nCL > 11)
        tmp |= ((11-4)<<4);
    else
        tmp |= ((nCL-4)<<4);

    if(time2cycle(DDR3_tWR_ps,tCK) <= 5) // Write Recovery for autoprecharge
        tmp |= WR_5<<9;
    else if(time2cycle(DDR3_tWR_ps,tCK) == 6)
        tmp |= WR_6<<9;
    else if(time2cycle(DDR3_tWR_ps,tCK) == 7)
        tmp |= WR_7<<9;
    else if(time2cycle(DDR3_tWR_ps,tCK) == 8)
        tmp |= WR_8<<9;
    else if((time2cycle(DDR3_tWR_ps,tCK) == 9) || (time2cycle(DDR3_tWR_ps,tCK) == 10))
        tmp |= WR_10<<9;
    else if(time2cycle(DDR3_tWR_ps,tCK) >= 11)
        tmp |= WR_12<<9;

    tmp    |= (1<<8); // DLL Reset

    denali_ctl(27) = tmp<<8;
    BITCSET(denali_ctl(30), 0x0000FFFF, tmp);
#ifdef PHY_MRS_SETTING
    ddr_phy(0x15) = tmp;
#endif

    // MRS1
    BITCSET(denali_ctl(28), 0x0000FFFF, (DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1));
    BITCSET(denali_ctl(30), 0xFFFF0000, ((DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1))<<16);
#ifdef PHY_MRS_SETTING
    ddr_phy(0x16) = (DDR3_AL<<3) | (DDR3_ODT<<2) | (DDR3_DIC<<1);
#endif

    // MRS2
    tmp = 0;
    if(nCWL <= 5)
        tmp |= 0;
    else if(nCWL == 6)
        tmp |= 1<<3;
    else if(nCWL == 7)
        tmp |= 2<<3;
    else if(nCWL >= 8)
        tmp |= 3<<3;

    BITCSET(denali_ctl(28), 0xFFFF0000, tmp<<16);
    BITCSET(denali_ctl(31), 0x0000FFFF, tmp);
#ifdef PHY_MRS_SETTING
    ddr_phy(0x17) = tmp;
#endif

    // MRS3
    BITCSET(denali_ctl(29), 0xFFFF0000, 0<<16);
    BITCSET(denali_ctl(32), 0x0000FFFF, 0);
#ifdef PHY_MRS_SETTING
    ddr_phy(0x18) = 0;
#endif

//--------------------------------------------------------------------------

#ifndef OLD_SETTING
    //BIST Start
    BITCLR(denali_ctl(32), 1<<16); //BIST_GO = 0x0
    denali_ctl(33) = (1<<16)|(1<<8); //BIST_ADDR_CHECK[-16] = 0x1, BIST_DATA_CHECK[-8] = 0x1
    //BIST End
#endif
    denali_ctl(34) = 0; denali_ctl(35) = 0; denali_ctl(36) = 0; denali_ctl(37) = 0;

    denali_ctl(38) = (DDR3_tZQOPER_ck<<16 | DDR3_tZQINIT_ck);
    denali_ctl(39) = (0x2<<24 | DDR3_tZQCS_ck); //ZQCS, ZQ_ON_SREF_EXIT
    denali_ctl(40) = 0x1<<16; //ZQCS_ROTATE=0x1, REFRESH_PER_ZQ=0x0
    denali_ctl(41) = 0xFF<<24|DDR3_APBIT<<16; //AGE_COUNT = 0xff
#ifdef IMPROVE_DDI_PERFORMANCE
    denali_ctl(42) = 0;
    denali_ctl(43) = 0;
#else
    denali_ctl(42) = 0x1<<24|0x1<<16|0x1<<8|0xFF; //COMMAND_AGE_CCOUNT = 0xff, ADDR_CMP_EN = 0x1, BANK_SPLIT_EN = 0x1 PLACEMENT_EN = 0x1
    denali_ctl(43) = 0x1<<16|0x1<<8|0x1; //PRIORITY_EN = 0x1, RW_SAME_EN = 0x1,SWAP_EN = 0x1
#endif

#if defined(CONFIG_DRAM_16BIT_USED)
    if(DDR3_LOGICAL_CHIP_NUM == 2)
        denali_ctl(44) = (0x1<<24|0xc<<16|0x3<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
    else
        denali_ctl(44) = (0x1<<24|0xc<<16|0x1<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
#else
    if(DDR3_LOGICAL_CHIP_NUM == 2)
        denali_ctl(44) = (0x0<<24|0xc<<16|0x3<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
    else
        denali_ctl(44) = (0x0<<24|0xc<<16|0x1<<8); //REDUC[24] = DQ 0:32BIT, 1:16BIT
#endif

    denali_ctl(45) = 0x0; //Q_FULLNESS = 0x000
#ifndef OLD_SETTING
    denali_ctl(47) = 0x0; //INT_ACK = 0x000000
#endif
    denali_ctl(48) = 0x0;

//--------------------------------------------------------------------------
// ODT Setting

#ifdef OLD_SETTING
    BITCSET(denali_ctl(64), 0x00030000, 0x0<<16); //ODT_RD_MAP_CS0
    if(DDR3_LOGICAL_CHIP_NUM == 2)
        BITCSET(denali_ctl(65), 0x00000003, 0x0<<0); //ODT_RD_MAP_CS1

    if(DDR3_PINMAP_TYPE == 0 || DDR3_PINMAP_TYPE == 1){
        BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
        if(DDR3_LOGICAL_CHIP_NUM == 2)
            BITCSET(denali_ctl(65), 0x00000300, 0x1<<8); //ODT_WR_MAP_CS1
    } else {
        BITCSET(denali_ctl(64), 0x03000000, 0x3<<24); //ODT_WR_MAP_CS0
        if(DDR3_LOGICAL_CHIP_NUM == 2)
            BITCSET(denali_ctl(65), 0x00000300, 0x3<<8); //ODT_WR_MAP_CS1
    }
#else
    if(DDR3_LOGICAL_CHIP_NUM == 1){
        BITCSET(denali_ctl(64), 0x00030000, 0x2<<16); //ODT_RD_MAP_CS0
        BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
        BITCSET(denali_ctl(65), 0x00000003, 0x1<<0); //ODT_RD_MAP_CS1
        BITCSET(denali_ctl(65), 0x00000300, 0x2<<8); //ODT_WR_MAP_CS1
    }else{
        BITCSET(denali_ctl(64), 0x00030000, 0x0<<16); //ODT_RD_MAP_CS0
        BITCSET(denali_ctl(64), 0x03000000, 0x1<<24); //ODT_WR_MAP_CS0
        BITCSET(denali_ctl(65), 0x00000003, 0x0<<0); //ODT_RD_MAP_CS1
        BITCSET(denali_ctl(65), 0x00000300, 0x1<<8); //ODT_WR_MAP_CS1
    }
#endif
    BITCSET(denali_ctl(65), 0x000F0000, 0x2<<16); //ADD_ODT_CLK_R2W_SAMECS = 0x2
    denali_ctl(66) = 0x1<<24|0x1<<16|0x2<<8|0x2; //ADD_ODT_CLK_DIFFTYPE_DIFFCS = 0x2, ADD_ODT_CLK_SAMETYPE_DIFFCS = 0x2, R2R_DIFFCS_DLY = 0x1, R2W_DIFFCS_DLY = 0x1
    denali_ctl(67) = 0x2<<24|0x0<<16|0x1<<8|0x1; //W2R_DIFFCS_DLY = 0x1, W2W_DIFFCS_DLY = 0x1, R2R_SAMECS_DLY = 0x0, R2W_SAMECS_DLY = 0x2
    BITCSET(denali_ctl(68), 0x00000707, 0x0<<8|0x0); //W2R_SAMECS_DLY = 0x0, W2W_SAMECS_DLY = 0x0
    denali_ctl(69) = 0; denali_ctl(71) = 0;
    denali_ctl(72) = 0x0<<16|0x28<<8|0x19; //WLDQSEN = 0x19, WLMRD = 0x28, WRLVL_EN = 0x0
    denali_ctl(73) = 0;
    denali_ctl(74) = (0x1<<8); //WRLVL_DELAY_0 = 0x0001, WRLVL_REG_EN = 0x00
    denali_ctl(75) = (0x1<<16|0x1); //WRLVL_DELAY_2 = 0x0001, WRLVL_DELAY_1 = 0x0001
    denali_ctl(76) = 0x1; //RDLVL_GATE_REQ = 0x00, RDLVL_REQ = 0x00, WRLVL_DELAY_3 = 0x0001
    denali_ctl(77) = 0; denali_ctl(78) = 0; denali_ctl(80) = 0;
    denali_ctl(81) = 0x1<<16|0x2222; //RDLVL_GATE_DELAY_0 = 0x1, RDLVL_DELAY_0 = 0x2222
    denali_ctl(83) = 0x0<<16; //RDLVL_OFFSET_DELAY_1 = 0x0000
    denali_ctl(84) = 0x2222<<8; //RDLVL_DELAY_1 = 0x2222, RDLVL_OFFSET_DIR_1 = 0x00
    denali_ctl(85) = 0x1; //RDLVL_GATE_DELAY_1 = 0x0001
    denali_ctl(87) = 0; //RDLVL_OFFSET_DIR_2 = 0x00, RDLVL_OFFSET_DELAY_2 = 0x0000
#ifdef OLD_SETTING
    denali_ctl(88) = 0x2121; //RDLVL_GATE_DELAY_2 = 0x0000, RDLVL_DELAY_2 = 0x2121
#else
    denali_ctl(88) = 0x1<<16|0x2222; //RDLVL_GATE_DELAY_2 = 0x0001, RDLVL_DELAY_2 = 0x2222
#endif
    denali_ctl(90) = 0;
#ifdef OLD_SETTING
    denali_ctl(91) = 0x2121<<8; //RDLVL_DELAY_3 = 0x2121, RDLVL_OFFSET_DIR_3 = 0x00
    denali_ctl(92) = 0xFFFF<<16; //AXI0_EN_SIZE_LT_WIDTH_INSTR = 0xffff, RDLVL_GATE_DELAY_3 = 0x0000
#else
    denali_ctl(91) = 0x2222<<8; //RDLVL_DELAY_3 = 0x2222, RDLVL_OFFSET_DIR_3 = 0x00
    denali_ctl(92) = 0xFFFF<<16|0x1; //AXI0_EN_SIZE_LT_WIDTH_INSTR = 0xffff, RDLVL_GATE_DELAY_3 = 0x0001
#endif
    denali_ctl(93) = 0x8<<8|0x8; //AXI0_R_PRIORITY = 0x8. AXI0_W_PRIORITY = 0x8
    denali_ctl(94) = 0;


//--------------------------------------------------------------------------
// DFI Timing

    BITCSET(denali_ctl(95), 0x3F0000FF, 0xd<<24); //TDFI_PHY_RDLAT = 0xd <- MOD_THIS (0x0d, FIXED) 0x0c, DLL_RST_ADJ_DLY = 0x00
    BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
    denali_ctl(97) = 0x200<<16|0x1450; //TDFI_CTRLUPD_MAX = 0x1450, TDFI_PHYUPD_TYPE0 = 0x200
    denali_ctl(98) = 0x200<<16|0x200; //TDFI_PHYUPD_TYPE1 = 0x200, TDFI_PHYUPD_TYPE2 = 0x200
    denali_ctl(99) = 0x1450<<16|0x200; //TDFI_PHYUPD_TYPE3 = 0x200, TDFI_PHYUPD_RESP = 0x1450
    denali_ctl(100) = 0x00006590; //TDFI_CTRLUPD_INTERVAL = 0x00006590
    denali_ctl(101) = (0x00<<24|0x02<<16|(nCWL-3+(nCWL%2))<<8|(nCL-3+(nCL%2)));
    denali_ctl(102) = 0x3<<24|0x8000<<8|0x1; //TDFI_DRAM_CLK_ENABLE = 0x1, DFI_WRLVL_MAX_DELAY = 0x8000, TDFI_WRLVL_EN = 0x3
    denali_ctl(103) = 0x4<<16|0x7<<8|0x3; //TDFI_WRLVL_DLL = 0x3, TDFI_WRLVL_LOAD = 0x7, TDFI_WRLVL_RESPLAT = 0x4
    denali_ctl(104) = 0xA; //TDFI_WRLVL_WW = 0xa
    denali_ctl(105) = 0; denali_ctl(106) = 0;
    denali_ctl(107) = 0x10<<16|0xFFFF; //RDLVL_MAX_DELAY = 0xffff, RDLVL_GATE_MAX_DELAY = 0x10
    denali_ctl(108) = 0x1a<<24|0x7<<16|0x3<<8|0x3; //TDFI_RDLVL_EN = 0x3, TDFI_RDLVL_DLL = 0x3, TDFI_RDLVL_LOAD = 0x7, TDFI_RDLVL_RESPLAT = 0x1a 
    denali_ctl(109) = 0xF; //TDFI_RDLVL_RR = 0xf
    denali_ctl(110) = 0; denali_ctl(111) = 0; denali_ctl(112) = 0; denali_ctl(113) = 0; denali_ctl(114) = 0;
    denali_ctl(115) = 0x2<<8|0x4; //RDLVL_DQ_ZERO_COUNT = 0x4, RDLVL_GATE_DQ_ZERO_COUNT = 0x2
    denali_ctl(117) = 0;
    denali_ctl(118) = 0; //ODT_ALT_EN = 0x0
    denali_ctl(119) = 0; //LP_AUTO_PD_IDLE = 0x0000, LP_AUTO_MEM_GATE_EN = 0x00
    denali_ctl(120) = 0x00000040; //REFRESH_PER_ZQ = 0x00000040
    denali_ctl(121) = 0x1<<24|(11-DDR3_COLBITS)<<16|(16-DDR3_ROWBITS)<<8|(3-DDR3_BANKBITS); //RW_SAME_PAGE_EN = 0x1, COL_DIFF = 0x01, ROW_DIFF = 0x01, BANK_DIFF = 0x00
    denali_ctl(122) = 0x0b<<24|0x03<<16|0x01<<8|0x01; //NUM_Q_ENTRIES_ACT_DISABLE = 0x0b, DISABLE_RW_GROUP_W_BNK_CONFLICT = 0x03, bReg.W2R_SPLIT_EN = 0x01, CS_SAME_EN = 0x01
    denali_ctl(123) = 0x01<<24|0x00<<16|0x00<<8|0x00; //CTRLUPD_REQ_PER_AREF_EN = 0x01, CTRLUPD_REQ = 0x00, IN_ORDER_ACCEPT = 0x00, DISABLE_RD_INTERLEAVE = 0x00
    denali_ctl(124) = 1<<8|0x06; //TDFI_PHY_WRDATA = 1,TODTL_2CMD = 0x06

//--------------------------------------------------------------------------
// Start denali MC init

#ifndef INIT_CONFIRM
    if(DDR3_LOGICAL_CHIP_NUM == 1){
    #ifdef ZDEN
        ddr_phy(0x1) = 0x40050001; // Wait for Synop. PHY init done...
    #else
        ddr_phy(0x1) = 0x00050001; // Wait for Synop. PHY init done...
    #endif
    }
#endif

    BITSET(denali_ctl(0), 1); //START = 1

#ifndef INIT_CONFIRM
    if(DDR3_LOGICAL_CHIP_NUM == 2){
#endif
    #ifdef    ZDEN    
        while((ddr_phy(0x4)&0x7) != 0x7); // Wait for Synop. PHY init done...
    #else
        while((ddr_phy(0x4)&0xf) != 0xf); // Wait for Synop. PHY init done...
    #endif
        ddr_phy(0x1) = 0x00050001; // PIR

        while(!(denali_ctl(46)&0x20)); // PHY Init
#ifndef INIT_CONFIRM
    }
#endif

#ifdef CONFIG_DRAM_AUTO_TRAINING

// 1CS
    ddr_phy(0x1a) &= 0xf1ffff7f;    // no data compare    
// 2CS
//    ddr_phy(0x1a) &= 0xf3ffff7f;    // no data compare    
//
    

    i=3200; while(i--);

phy_wl_t:
    ddr_phy(0x01) |= (0x1 << 9);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x21 ) != 0x21 );

    if( ( ddr_phy(0x04) & 0x00200021 ) == 0x00200021 && count > 0 )
        goto phy_wl_t;

    count = 16;
    i=3200; while(i--);



phy_dqs_t:
    ddr_phy(0x01) |= (0x1 << 10);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x41 ) != 0x41 );

    if( ( ddr_phy(0x04) & 0x00400041 ) == 0x00400041 && count > 0 )
        goto phy_dqs_t;

    count = 16;
    i=3200; while(i--);

phy_wla_t:
    ddr_phy(0x01) |= (0x1 << 11);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x81 ) != 0x81 );

    if( ( ddr_phy(0x04) & 0x00800081 ) == 0x00800081 && count > 0 )
        goto phy_wla_t;

    count = 16;
    i=3200; while(i--);

phy_rddskw_t:
    ddr_phy(0x01) |= (0x1 << 12);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x101 ) != 0x101 );

    if( ( ddr_phy(0x04) & 0x01000101 ) == 0x01000101 && count > 0 )
        goto phy_rddskw_t;

    count = 16;
    i=3200; while(i--);

phy_wrdskw_t:
    ddr_phy(0x01) |= (0x1 << 13);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x201 ) != 0x201 );

    if( ( ddr_phy(0x04) & 0x02000201 ) == 0x02000201 && count > 0 )
        goto phy_rddskw_t;

    count = 16;
    i=3200; while(i--);


phy_rdeye_t:
    ddr_phy(0x01) |= (0x1 << 14);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x401 ) != 0x401 );

    if( ( ddr_phy(0x04) & 0x04000401 ) == 0x04000401 && count > 0 )
        goto phy_rdeye_t;

    count = 16;
    i=3200; while(i--);


phy_wreye_t:
    ddr_phy(0x01) |= (0x1 << 15);
    ddr_phy(0x01) |= 0x1;
    count--;

    while( ( ddr_phy(0x04) & 0x801 ) != 0x801 );

    if( ( ddr_phy(0x04) & 0x08000801 ) == 0x08000801 && count > 0 )
        goto phy_wreye_t;

    count = 16;
    i=3200; while(i--);


// ZQ over-ride
//    ddr_phy(0x60) =   ( 0x0 << 29 )        // ZCALBYP
//            | ( 0x1 << 30 )        // ZCALEN
//            | ( 0x1 << 28 )        // ZDEN
//            | ( 0x0D << 15 )    // pull-up ODT 58ohm fix
//            | ( 0x0D << 10 )    // pull-down don't care
//            | ( 0x09 <<  5 )    // pull-up output Z 40.6 ohm
//            | ( 0x08 <<  0 );    // pull-down don't care

    i=3200; while(i--);

//    PGCR1 -
//    [12:11] : LPFDEPTH
//    [14:13] : FDEPTH
//    ddr_phy(0x03) = ddr_phy(0x03) | ( 0x3 << 11 ) | ( 0x3 << 13 );


#endif

#ifdef MANUAL_TRAIN0
    
// VT Drift Disable
    ddr_phy(0x02) &= 0xffffffc0;
    ddr_phy(0x03) |= (0x1 << 26);
     
// DX?LCDLR0 : WL
//      [31:24] : RANK3 Write Leveling Delay
//      [23:16] : RANK2 Write Leveling Delay
//      [15:08] : RANK1 Write Leveling Delay
//      [07:00] : RANK0 Write Leveling Delay
ddr_phy(0x78) = b0_wld;
ddr_phy(0x88) = b1_wld;
ddr_phy(0x98) = b2_wld;
ddr_phy(0xA8) = b3_wld;


// DX?GTR       : DQS
//      [19:18] : RANK3 Write Leveling System Latency : 01 -> Write Latency = WL (00)
//      [17:16] : RANK2 Write Leveling System Latency : 01 -> Write Latency = WL (00)
//      [15:14] : RANK1 Write Leveling System Latency : 01 -> Write Latency = WL (01)
//      [13:12] : RANK0 Write Leveling System Latency : 01 -> Write Latency = WL (01)

//      [11:09] : RAND3 DQS Gatng System Latency
//      [08:06] : RAND2 DQS Gatng System Latency
//      [05:03] : RAND1 DQS Gatng System Latency
//      [02:00] : RAND0 DQS Gatng System Latency (11)
ddr_phy(0x7c) = 0x00005000 | b0_dqs_gsl;
ddr_phy(0x8c) = 0x00005000 | b1_dqs_gsl;
ddr_phy(0x9c) = 0x00005000 | b2_dqs_gsl;
ddr_phy(0xAc) = 0x00005000 | b3_dqs_gsl;

// DX?LCDLR2 : DQS
//      [31:24] : RANK3 Read DQS Gating Delay
//      [23:16] : RANK2 Read DQS Gating Delay
//      [15:08] : RANK1 Read DQS Gating Delay
//      [07:00] : RANK0 Read DQS Gating Delay
ddr_phy(0x7a) = b0_dqs_gd;
ddr_phy(0x8a) = b1_dqs_gd;
ddr_phy(0x9a) = b2_dqs_gd;
ddr_phy(0xAa) = b3_dqs_gd;


// DX?LCDLR1 : RD - WR
//      [31:24] : Reserved
//      [23:16] : RDQSND (Read DQSN Delay)
//      [15:08] : RDQSD (Read DQS Delay)
//      [07:00] : WDQD (Write Data Delay)
ddr_phy(0x79) = (b0_rdqsn_d << 16) | (b0_rdqs_d) << 8 | b0_wdq_d;
ddr_phy(0x89) = (b1_rdqsn_d << 16) | (b1_rdqs_d) << 8 | b1_wdq_d;
ddr_phy(0x99) = (b2_rdqsn_d << 16) | (b2_rdqs_d) << 8 | b2_wdq_d;
ddr_phy(0xA9) = (b3_rdqsn_d << 16) | (b3_rdqs_d) << 8 | b3_wdq_d;


// RMDL

ddr_phy(0x7B) = (b0_rmdl<<16);
ddr_phy(0x8B) = (b1_rmdl<<16);
ddr_phy(0x9B) = (b2_rmdl<<16);
ddr_phy(0xAB) = (b3_rmdl<<16);


// DX?BDLR3 : RD
//      [31:30] : Reserved
//      [29:24] : DQ4RBD
//      [23:18] : DQ3RBDk
//      [17:12] : DQ2RBD
//      [11:06] : DQ1RBD
//      [05:00] : DQ0RBD



ddr_phy(0x76) = (b0_dq4_rbd << 24) | (b0_dq3_rbd << 18) | (b0_dq2_rbd << 12) | (b0_dq1_rbd < 6) | b0_dq0_rbd;
ddr_phy(0x86) = (b1_dq4_rbd << 24) | (b1_dq3_rbd << 18) | (b1_dq2_rbd << 12) | (b1_dq1_rbd < 6) | b1_dq0_rbd;
ddr_phy(0x96) = (b2_dq4_rbd << 24) | (b2_dq3_rbd << 18) | (b2_dq2_rbd << 12) | (b2_dq1_rbd < 6) | b2_dq0_rbd;
ddr_phy(0xA6) = (b3_dq4_rbd << 24) | (b3_dq3_rbd << 18) | (b3_dq2_rbd << 12) | (b3_dq1_rbd < 6) | b3_dq0_rbd;
            
// DX?BDLR4 : RD
//      [31:24] : Reserved
//      [23:18] : DMRBD
//      [17:12] : DQ7RBD
//      [11:06] : DQ6RBD
//      [05:00] : DQ5RBD
ddr_phy(0x77) = (b0_dm_rbd << 18) | (b0_dq7_rbd << 12) | (b0_dq6_rbd < 6) | b0_dq5_rbd;
ddr_phy(0x87) = (b1_dm_rbd << 18) | (b1_dq7_rbd << 12) | (b1_dq6_rbd < 6) | b1_dq5_rbd;
ddr_phy(0x97) = (b2_dm_rbd << 18) | (b2_dq7_rbd << 12) | (b2_dq6_rbd < 6) | b2_dq5_rbd;
ddr_phy(0xA7) = (b3_dm_rbd << 18) | (b3_dq7_rbd << 12) | (b3_dq6_rbd < 6) | b3_dq5_rbd;

// DX?BDLR0 : WR
//      [31:30] : Reserved
//      [29:24] : DQ4WBD
//      [23:18] : DQ3WBD
//      [17:12] : DQ2WBD
//      [11:06] : DQ1WBD
//      [05:00] : DQ0WBD



ddr_phy(0x73) = (b0_dq4_wbd << 24) | (b0_dq3_wbd << 18) | (b0_dq2_wbd << 12) | (b0_dq1_wbd < 6) | b0_dq0_wbd;
ddr_phy(0x83) = (b1_dq4_wbd << 24) | (b1_dq3_wbd << 18) | (b1_dq2_wbd << 12) | (b1_dq1_wbd < 6) | b1_dq0_wbd;
ddr_phy(0x93) = (b2_dq4_wbd << 24) | (b2_dq3_wbd << 18) | (b2_dq2_wbd << 12) | (b2_dq1_wbd < 6) | b2_dq0_wbd;
ddr_phy(0xA3) = (b3_dq4_wbd << 24) | (b3_dq3_wbd << 18) | (b3_dq2_wbd << 12) | (b3_dq1_wbd < 6) | b3_dq0_wbd;

// DX?BDLR1 : WR
//      [31:30] : Reserved
//      [29:24] : DQSWBD
//      [23:18] : DMWBD
//      [17:12] : DQ7WBD
//      [11:06] : DQ6WBD
//      [05:00] : DQ5WBD
ddr_phy(0x74) = (b0_dqs_wbd << 24) | (b0_dm_wbd << 18) | (b0_dq7_wbd << 12) | (b0_dq6_wbd < 6) | b0_dq5_wbd;
ddr_phy(0x84) = (b1_dqs_wbd << 24) | (b1_dm_wbd << 18) | (b1_dq7_wbd << 12) | (b1_dq6_wbd < 6) | b1_dq5_wbd;
ddr_phy(0x94) = (b2_dqs_wbd << 24) | (b2_dm_wbd << 18) | (b2_dq7_wbd << 12) | (b2_dq6_wbd < 6) | b2_dq5_wbd;
ddr_phy(0xA4) = (b3_dqs_wbd << 24) | (b3_dm_wbd << 18) | (b3_dq7_wbd << 12) | (b3_dq6_wbd < 6) | b3_dq5_wbd;

// DX?BDLR2 : 0
//      [31:24] : Reserved
//      [23:18] : DQSN RD BD
//      [17:12] : DQS  RD BD
//      [11:06] : DQ OE BD
//      [05:00] : DQS OE BD


// Not Used
ddr_phy(0x75) = (b0_dqsn_rbd << 18) | (b0_dqs_rbd << 12) | (b0_dq_oebd < 6) | b0_dqs_oebd;
ddr_phy(0x85) = (b1_dqsn_rbd << 18) | (b1_dqs_rbd << 12) | (b1_dq_oebd < 6) | b1_dqs_oebd;
ddr_phy(0x95) = (b2_dqsn_rbd << 18) | (b2_dqs_rbd << 12) | (b2_dq_oebd < 6) | b2_dqs_oebd;
ddr_phy(0xA5) = (b3_dqsn_rbd << 18) | (b3_dqs_rbd << 12) | (b3_dq_oebd < 6) | b3_dqs_oebd;


    i=3200; while(i--);

#endif    // ifndef AUTO_TRAIN

#ifdef TCC_PM_SSTLIO_CTRL
//--------------------------------------------------------------------------
//enter self-refresh

//--------------------------------------------------------------------------
// holding to access to dram

    denali_ctl(47) = 0xFFFFFFFF;
    BITSET(denali_ctl(44),0x1); //inhibit_dram_cmd=1
    while(!(denali_ctl(46)&(0x1<<15)));    // wait for inhibit_dram_cmd
    BITSET(denali_ctl(47), 0x1<<15);

//--------------------------------------------------------------------------
//enter self-refresh

    BITSET(denali_phy(13), 0x1<<10); //lp_ext_req=1
    while(!(denali_phy(13)&(0x1<<26))); //until lp_ext_ack==1
    BITCSET(denali_phy(13), 0x000000FF, (2<<2)|(1<<1)|(0));
    BITSET(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=1
    while((denali_phy(13)&(0x003F0000)) != (0x25<<16)); //until lp_ext_state==0x25 : check self-refresh state
    BITCLR(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=0
    BITCLR(denali_phy(13), 0x1<<10); //lp_ext_req=0
    while(denali_phy(13)&(0x1<<26)); //until lp_ext_ack==0
    BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
    BITCLR(denali_ctl(0),0x1); //START[0] = 0

    ddr_phy(0x0E) = 0xffffffff; // AC IO Config

// -------------------------------------------------------------------------
// SSTL Retention Disable

    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))){
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    }

//--------------------------------------------------------------------------
// Exit self-refresh

    ddr_phy(0x0E) = 0x30c01812; // AC IO Config

    BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
    BITCLR(denali_ctl(39), 0x3<<24); //ZQ_ON_SREF_EXIT = 0
     BITSET(denali_ctl(0),0x1); //START[0] = 1

    //for(i=0;i<11;i++){
    //    BITSET(denali_ctl(123), 0x1<<16); //CTRLUPD_REQ = 1
    //    while(!(denali_ctl(46)&(0x20000)));
    //    BITSET(denali_ctl(47), 0x20000);
    //}

    BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(0<<1)|(1))<<24);
    while(!(denali_ctl(46)&(0x40)));
    BITSET(denali_ctl(47), 0x40);

//--------------------------------------------------------------------------
// MRS Write

    // MRS2
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(31)&0x0000FFFF)<<16);
    denali_ctl(26) = (2<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS3
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(29)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(32)&0x0000FFFF)<<16);
    denali_ctl(26) = (3<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS1
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0x0000FFFF)>>0);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0xFFFF0000)<<0);
    denali_ctl(26) = (1<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS0
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(27)&0x00FFFF00)>>8);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0x0000FFFF)<<16);
    denali_ctl(26) = (0<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

//--------------------------------------------------------------------------

    BITCLR(denali_ctl(40) ,0x1<<16); //ZQCS_ROTATE=0x0
    BITCSET(denali_ctl(39) ,0x3<<16, 0x2<<16); //ZQ_REQ=2 : 0x1=short calibration, 0x2=long calibration

//--------------------------------------------------------------------------
// release holding to access to dram

    i = 10;    while(i--) tmp = BITSET(denali_ctl(13), 1<<24); // AREFRESH = 1
    BITCLR(denali_ctl(44),0x1); //inhibit_dram_cmd=0

//--------------------------------------------------------------------------

#endif
#else
    #error "not selected ddr type.."
#endif
}
EXPORT_SYMBOL(tcc_pm_sdram_init);

#if defined(CONFIG_SHUTDOWN_MODE) || defined(CONFIG_HIBERNATION)
static void edi_init(void)
{
    PEDI pEDI = (PEDI)tcc_p2v(HwEDI_BASE);

    BITCSET(pEDI->EDI_RDYCFG.nREG,  0x000FFFFF, 0x00032104 );
    BITCSET(pEDI->EDI_CSNCFG0.nREG, 0x0000FFFF, 0x00008765 );
    BITCSET(pEDI->EDI_OENCFG.nREG,  0x0000000F, 0x00000001 );
    BITCSET(pEDI->EDI_WENCFG.nREG,  0x0000000F, 0x00000001 );
}

void tcc_nfc_suspend(PNFC pBackupNFC , PNFC pHwNFC)
{
    pBackupNFC->NFC_CACYC = pHwNFC->NFC_CACYC;
    pBackupNFC->NFC_WRCYC = pHwNFC->NFC_WRCYC;
    pBackupNFC->NFC_RECYC = pHwNFC->NFC_RECYC;
    pBackupNFC->NFC_CTRL = pHwNFC->NFC_CTRL;
    pBackupNFC->NFC_IRQCFG = pHwNFC->NFC_IRQCFG;
    pBackupNFC->NFC_RFWBASE = pHwNFC->NFC_RFWBASE;
}
EXPORT_SYMBOL(tcc_nfc_suspend);

void tcc_nfc_resume(PNFC pHwNFC , PNFC pBackupNFC)
{
    edi_init();
    pHwNFC->NFC_CACYC = pBackupNFC->NFC_CACYC;
    pHwNFC->NFC_WRCYC = pBackupNFC->NFC_WRCYC;
    pHwNFC->NFC_RECYC = pBackupNFC->NFC_RECYC;
    pHwNFC->NFC_CTRL = pBackupNFC->NFC_CTRL;
    pHwNFC->NFC_IRQCFG = pBackupNFC->NFC_IRQCFG;
    pHwNFC->NFC_IRQ = 0xFFFFFFFF;
    pHwNFC->NFC_RFWBASE = pBackupNFC->NFC_RFWBASE;
}
EXPORT_SYMBOL(tcc_nfc_resume);
#endif

#if defined(CONFIG_PM_CONSOLE_NOT_SUSPEND)
void tcc_pm_uart_suspend(bkUART *pBackupUART)
{
    UART *pHwUART = (UART *)tcc_p2v(HwUART0_BASE);

    pBackupUART->CFG0 = *(volatile unsigned long *)tcc_p2v(HwUART_PORTCFG_BASE);
    pBackupUART->CFG1 = *(volatile unsigned long *)tcc_p2v(HwUART_PORTCFG_BASE + 0x4);

    pBackupUART->IER    = pHwUART->REG2.nREG;    //0x04 : IER
    pHwUART->REG2.nREG    &= ~Hw2;    //disable interrupt : ELSI
    pBackupUART->LCR    = pHwUART->LCR.nREG;    //0x0C : LCR
    pHwUART->LCR.nREG    |= Hw7;        // DLAB = 1
    pBackupUART->DLL    = pHwUART->REG1.nREG;    //0x00 : DLL
    pBackupUART->DLM    = pHwUART->REG2.nREG;    //0x04 : DLM
    pBackupUART->MCR    = pHwUART->MCR.nREG;    //0x10 : MCR
    pBackupUART->AFT    = pHwUART->AFT.nREG;    //0x20 : AFT
    pBackupUART->UCR    = pHwUART->UCR.nREG;    //0x24 : UCR
}
EXPORT_SYMBOL(tcc_pm_uart_suspend);

void tcc_pm_uart_resume(bkUART *pBackupUART)
{
    UART *pHwUART = (UART *)tcc_p2v(HwUART0_BASE);

    *(volatile unsigned long *)tcc_p2v(HwUART_PORTCFG_BASE) = pBackupUART->CFG0;
    *(volatile unsigned long *)tcc_p2v(HwUART_PORTCFG_BASE + 0x4) = pBackupUART->CFG1;

    pHwUART->REG2.nREG    &= ~Hw2;    //disable interrupt : ELSI
    pHwUART->LCR.nREG    |= Hw7;        // DLAB = 1
    pHwUART->REG3.nREG    = Hw2 + Hw1 + Hw0;    //0x08 : FCR
    pHwUART->REG1.nREG    = pBackupUART->DLL;    //0x00 : DLL
    pHwUART->REG2.nREG    = pBackupUART->DLM;    //0x04 : DLM
    pHwUART->MCR.nREG    = pBackupUART->MCR;    //0x10 : MCR
    pHwUART->AFT.nREG    = pBackupUART->AFT;    //0x20 : AFT
    pHwUART->UCR.nREG    = pBackupUART->UCR;    //0x24 : UCR
    pHwUART->LCR.nREG    = pBackupUART->LCR;    //0x0C : LCR
    pHwUART->REG2.nREG    = pBackupUART->IER;    //0x04 : IER

    printk("\n======== wakeup source ========\n");
    printk("wakeup0_reg:0x%08x\n", (unsigned int)tcc_pm_board_param(BOARD_PARAM_PMU_WUSTS0));
    printk("wakeup1_reg:0x%08x\n\n\n", (unsigned int)tcc_pm_board_param(BOARD_PARAM_PMU_WUSTS1));
}
EXPORT_SYMBOL(tcc_pm_uart_resume);
#endif

#if defined(CONFIG_SHUTDOWN_MODE)
void tcc_pm_shutdown(void)
{
    volatile unsigned int cnt = 0;
#ifdef TCC_PM_CHECK_WAKEUP_SOURCE
    volatile unsigned int loop = 0;
#endif

// -------------------------------------------------------------------------
// REMAP 0xF0000000 area to SRAM
    *(volatile unsigned *)0xF4810010 |= 0x80000000;

// -------------------------------------------------------------------------
// mmu & cache off

    __asm__ volatile (
    "mrc p15, 0, r0, c1, c0, 0 \n"
    "bic r0, r0, #(1<<12) \n" //ICache
    "bic r0, r0, #(1<<2) \n" //DCache
    "bic r0, r0, #(1<<0) \n" //MMU
    "mcr p15, 0, r0, c1, c0, 0 \n"
    "nop \n"
    );

// -------------------------------------------------------------------------
// SDRAM Self-refresh
#if defined(CONFIG_DRAM_DDR3)
//--------------------------------------------------------------------------
// holding to access to dram

    denali_ctl(47) = 0xFFFFFFFF;
    BITSET(denali_ctl(44),0x1); //inhibit_dram_cmd=1
    while(!(denali_ctl(46)&(0x1<<15)));    // wait for inhibit_dram_cmd
    BITSET(denali_ctl(47), 0x1<<15);

//--------------------------------------------------------------------------
//enter self-refresh

    BITSET(denali_phy(13), 0x1<<10); //lp_ext_req=1
    while(!(denali_phy(13)&(0x1<<26))); //until lp_ext_ack==1
    BITCSET(denali_phy(13), 0x000000FF, (2<<2)|(1<<1)|(0));
    BITSET(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=1
    while((denali_phy(13)&(0x003F0000)) != (0x25<<16)); //until lp_ext_state==0x25 : check self-refresh state
    BITCLR(denali_phy(13), 0x1<<8); //lp_ext_cmd_strb=0
    BITCLR(denali_phy(13), 0x1<<10); //lp_ext_req=0
    while(denali_phy(13)&(0x1<<26)); //until lp_ext_ack==0
    BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
    BITCLR(denali_ctl(0),0x1); //START[0] = 0

    ddr_phy(0x0E) = 0xffffffff; // AC IO Config
#elif defined(CONFIG_DRAM_DDR2)
    #error "DDR2 is not implemented"
#endif

    BITSET(ddr_phy(0x0F), Hw1|Hw3|Hw4); //DXIOM=1(LVCMOS mode), DXPDD=1(output driver powered down), DXPDR=1(input buffer powered down)

    nop_delay(1000);

// -------------------------------------------------------------------------
// Clock <- XIN, PLL <- OFF

    ((PCKC)HwCKC_BASE)->CLKCTRL0.nREG = 0x002ffff4; //CPU
    ((PCKC)HwCKC_BASE)->CLKCTRL1.nREG = 0x00200014; //Memory Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL2.nREG = 0x00000014; //DDI Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL3.nREG = 0x00000014; //Graphic Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL4.nREG = 0x00200014; //I/O Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL5.nREG = 0x00000014; //Video Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL6.nREG = 0x00000014; //Video core
    ((PCKC)HwCKC_BASE)->CLKCTRL7.nREG = 0x00000014; //HSIO BUS
    ((PCKC)HwCKC_BASE)->CLKCTRL8.nREG = 0x00200014; //SMU Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL9.nREG = 0x00000014; //G2D BUS
    ((PCKC)HwCKC_BASE)->CLKCTRL10.nREG = 0x00000014; //CM3 Bus

    ((PCKC)HwCKC_BASE)->PLL0CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL1CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL2CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL3CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL4CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL5CFG.nREG &= ~0x80000000;

// -------------------------------------------------------------------------
// PLL25 Power OFF
    if (tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) != 0xFFFFFFFF) {
        volatile unsigned *reg = (volatile unsigned *)((HwGPIO_BASE+(0x40*(tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) >> GPIO_REG_SHIFT))));
        *reg &= ~(1<<(tcc_pm_board_param(BOARD_PARAM_PLL25_PORT)&GPIO_BITMASK));
    }

#if defined(CONFIG_MACH_TCC8930ST)
    tcc_stb_suspend();
#endif

// -------------------------------------------------------------------------
// SRAM Retention

    BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<16); //PD_RETN : SRAM retention mode=0

// -------------------------------------------------------------------------
// Remap

    BITCSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, 0x30000000, 0x10000000); //remap : 0x00000000 <- sram(0x10000000)
    BITSET(((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSCFG.nREG, 1<<31); //RMIR : remap for top area : 0xF0000000 <- sram(0x10000000)


// -------------------------------------------------------------------------
// BUS Power Down

#if defined(TCC_PM_PMU_CTRL)
    //IP isolation off
    ((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000BDF;

    //High Speed I/O Bus power down
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.HSIOBUS = 0;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.HSIOBUS == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.HSB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_HSBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_HSB == 0);

    //DDI Bus power down
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.DBUS = 0;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS0 | ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS1) == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.DB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_DBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_DB == 0);

    //Graphic Bus power down
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.GBUS = 0;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.GBUS == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.GB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_GBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_GB == 0);

    //Video Bus power down
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.VBUS = 0;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS0 | ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS1) == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.VB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_VBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_VB == 0);
#endif

// -------------------------------------------------------------------------
// SSTL & IO Retention

    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8)){
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
    }
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4)){
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode =0
    }

// -------------------------------------------------------------------------
// set wake-up

    //set wake-up trigger mode : edge
    ((PPMU)HwPMU_BASE)->PMU_WKMOD0.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKMOD1.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x0;
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x0;

#if defined(CONFIG_INPUT_TCC_REMOTE)
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw26);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw25);
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw24);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw23);

    BITCSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, 0x00700000, tcc_pm_board_param(BOARD_PARAM_PMU_CFG_RMISEL));
#endif
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.nREG = tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0);
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG  = tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0);
    ((PPMU)HwPMU_BASE)->PMU_WKPOL1.nREG = tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1);
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG  = tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1);

    /* RTC Alarm Wake Up */
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.RTC_WAKEUP = 0; //RTC_PMWKUP - Active High
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.RTC_WAKEUP = 1; //RTC_PMWKUP - PMU WakeUp Enable


// -------------------------------------------------------------------------
// Enter Shutdown !!

    while(1){
        ((PPMU)HwPMU_BASE)->PWRDN_TOP.nREG = 1;
        nop_delay(100);

#ifdef TCC_PM_CHECK_WAKEUP_SOURCE
        if(loop++ > 100){
            if(((PPMU)HwPMU_BASE)->PMU_WKSTS0.nREG || ((PPMU)HwPMU_BASE)->PMU_WKSTS1.nREG){
                ((PPMU)HwPMU_BASE)->PMU_WDTCTRL.nREG = (Hw31 + 0x1);
                while(1);
            }
        }
#endif
    };
}
EXPORT_SYMBOL(tcc_pm_shutdown);

/*===========================================================================
FUNCTION
===========================================================================*/
void tcc_pm_wakeup(void)
{
    volatile unsigned int cnt;
    volatile unsigned *src;
    volatile unsigned *dest;

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    tcc_pm_board_param(BOARD_PARAM_PMU_WUSTS0) = ((PPMU)HwPMU_BASE)->PMU_WKSTS0.nREG;
    tcc_pm_board_param(BOARD_PARAM_PMU_WUSTS1) = ((PPMU)HwPMU_BASE)->PMU_WKSTS1.nREG;
#endif

// -------------------------------------------------------------------------
// GPIO Restore

    src = (unsigned*)GPIO_REPOSITORY_ADDR;
    dest = (unsigned*)HwGPIO_BASE;

    for(cnt=0 ; cnt<(sizeof(GPIO)/sizeof(unsigned)) ; cnt++)
        dest[cnt] = src[cnt];

// -------------------------------------------------------------------------
// clear wake-up
    //disable all for accessing PMU Reg. !!!
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x00000000;
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x00000000;
#ifdef TCC_PM_CHECK_WAKEUP_SOURCE
    ((PPMU)HwPMU_BASE)->PMU_WKCLR0.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKCLR1.nREG = 0xFFFFFFFF;
#endif

// -------------------------------------------------------------------------
// SSTL & IO Retention Disable

#ifndef TCC_PM_SSTLIO_CTRL
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))){
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    }
#endif
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4))){
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1
    }

// -------------------------------------------------------------------------
#if defined(CONFIG_MACH_TCC8930ST)
    tcc_stb_resume();
#endif
// BUS Power On

#if defined(TCC_PM_PMU_CTRL)
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    while(((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.MAIN_STATE != 1);
#else
    while(((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.MAIN_STATE);
#endif

    ((PPMU)HwPMU_BASE)->PWRUP_HSBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_HSB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.HSB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.HSIOBUS = 1;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.HSIOBUS == 0);

    ((PPMU)HwPMU_BASE)->PWRUP_DBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_DB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.DB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.DBUS = 1;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS0 & ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS1) == 0);

    ((PPMU)HwPMU_BASE)->PWRUP_GBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_GB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.GB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.GBUS = 1;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.GBUS == 0);

    ((PPMU)HwPMU_BASE)->PWRUP_VBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_VB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.VB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.VBUS = 1;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS0 & ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS1) == 0);

    //IP isolation on
    //((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000BD0;
    ((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000000;
#endif
}
EXPORT_SYMBOL(tcc_pm_wakeup);

#if defined(CONFIG_BROADCOM_WIFI)
/*===========================================================================
FUNCTION
===========================================================================*/
#define PM_MAX_REG_NUM 20
void pm_print_sdmmc_register(unsigned int start_addr)
{
  int i;
  for (i=0; i<PM_MAX_REG_NUM; i++) {
    printk("0x%08x ", *(volatile unsigned int*)(tcc_p2v(start_addr + i*4)));
    if ((i+1)%4==0)
      printk("\n");
  }
}

unsigned int mmc_bak[9];
static void tcc_mmc_restore(unsigned int start_addr)
{
  printk("[%s] start_addr : 0x%08x\n", __func__, start_addr);
#if 1
  pm_print_sdmmc_register(start_addr);
#endif
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x00)) = mmc_bak[0];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x04)) = mmc_bak[1];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x08)) = mmc_bak[2];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x0c)) = mmc_bak[3];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x28)) = mmc_bak[4];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x2c)) = mmc_bak[5];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x34)) = mmc_bak[6];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x38)) = mmc_bak[7];
  *(volatile unsigned int*)(tcc_p2v(start_addr + 0x3e)) = mmc_bak[8];
#if 1
  pm_print_sdmmc_register(start_addr);
#endif
}

static void tcc_mmc_backup(unsigned int start_addr)
{
  printk("[%s] start_addr : 0x%08x\n", __func__, start_addr);
#if 1
  pm_print_sdmmc_register(start_addr);
#endif
  mmc_bak[0] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x00));
  mmc_bak[1] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x04));
  mmc_bak[2] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x08));
  mmc_bak[3] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x0c));
  mmc_bak[4] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x28));
  mmc_bak[5] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x2c));
  mmc_bak[6] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x34));
  mmc_bak[7] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x38));
  mmc_bak[8] = *(volatile unsigned int*)(tcc_p2v(start_addr + 0x3e));
#if 1
  pm_print_sdmmc_register(start_addr);
#endif
}



static void tcc_mmc_ch_restore(PMMCCH pHwMMC , PMMCCH pBackupMMC)
{
    pHwMMC->SDCTRL = pBackupMMC->SDCTRL;     
    pHwMMC->SD0CMDDAT = pBackupMMC->SD0CMDDAT;    
    pHwMMC->SD1CMDDAT = pBackupMMC->SD1CMDDAT;    
    pHwMMC->SD2CMDDAT = pBackupMMC->SD2CMDDAT;    
    pHwMMC->SD3CMDDAT = pBackupMMC->SD3CMDDAT;    
    pHwMMC->SD0PRESET1 = pBackupMMC->SD0PRESET1;
    pHwMMC->SD0PRESET2 = pBackupMMC->SD0PRESET2;
    pHwMMC->SD0PRESET3 = pBackupMMC->SD0PRESET3;
    pHwMMC->SD0PRESET4 = pBackupMMC->SD0PRESET4;
    pHwMMC->SD0PRESET5 = pBackupMMC->SD0PRESET5;
    pHwMMC->SD0PRESET6 = pBackupMMC->SD0PRESET6;
    pHwMMC->SD1PRESET1 = pBackupMMC->SD0PRESET6;
    pHwMMC->SD1PRESET2 = pBackupMMC->SD1PRESET2;
    pHwMMC->SD1PRESET3 = pBackupMMC->SD1PRESET3;
    pHwMMC->SD1PRESET4 = pBackupMMC->SD1PRESET4;
    pHwMMC->SD1PRESET5 = pBackupMMC->SD1PRESET5;
    pHwMMC->SD1PRESET6 = pBackupMMC->SD1PRESET6;
    pHwMMC->SD2PRESET1 = pBackupMMC->SD2PRESET1;
    pHwMMC->SD2PRESET2 = pBackupMMC->SD2PRESET2;
    pHwMMC->SD2PRESET3 = pBackupMMC->SD2PRESET3;
    pHwMMC->SD2PRESET4 = pBackupMMC->SD2PRESET4;
    pHwMMC->SD2PRESET5 = pBackupMMC->SD2PRESET5;
    pHwMMC->SD2PRESET6 = pBackupMMC->SD2PRESET6;
    pHwMMC->SD3PRESET1 = pBackupMMC->SD3PRESET1;
    pHwMMC->SD3PRESET2 = pBackupMMC->SD3PRESET2;
    pHwMMC->SD3PRESET3 = pBackupMMC->SD3PRESET3;
    pHwMMC->SD3PRESET4 = pBackupMMC->SD3PRESET4;
    pHwMMC->SD3PRESET5 = pBackupMMC->SD3PRESET5;
    pHwMMC->SD3PRESET6 = pBackupMMC->SD3PRESET6;    
}

static void tcc_mmc_ch_backup(PMMCCH pBackupMMC , PMMCCH pHwMMC)
{
    pBackupMMC->SDCTRL = pHwMMC->SDCTRL;     
    pBackupMMC->SD0CMDDAT = pHwMMC->SD0CMDDAT;    
    pBackupMMC->SD1CMDDAT = pHwMMC->SD1CMDDAT;    
    pBackupMMC->SD2CMDDAT = pHwMMC->SD2CMDDAT;    
    pBackupMMC->SD3CMDDAT = pHwMMC->SD3CMDDAT;    
    pBackupMMC->SD0PRESET1 = pHwMMC->SD0PRESET1;
    pBackupMMC->SD0PRESET2 = pHwMMC->SD0PRESET2;
    pBackupMMC->SD0PRESET3 = pHwMMC->SD0PRESET3;
    pBackupMMC->SD0PRESET4 = pHwMMC->SD0PRESET4;
    pBackupMMC->SD0PRESET5 = pHwMMC->SD0PRESET5;
    pBackupMMC->SD0PRESET6 = pHwMMC->SD0PRESET6;
    pBackupMMC->SD1PRESET1 = pHwMMC->SD0PRESET6;
    pBackupMMC->SD1PRESET2 = pHwMMC->SD1PRESET2;
    pBackupMMC->SD1PRESET3 = pHwMMC->SD1PRESET3;
    pBackupMMC->SD1PRESET4 = pHwMMC->SD1PRESET4;
    pBackupMMC->SD1PRESET5 = pHwMMC->SD1PRESET5;
    pBackupMMC->SD1PRESET6 = pHwMMC->SD1PRESET6;
    pBackupMMC->SD2PRESET1 = pHwMMC->SD2PRESET1;
    pBackupMMC->SD2PRESET2 = pHwMMC->SD2PRESET2;
    pBackupMMC->SD2PRESET3 = pHwMMC->SD2PRESET3;
    pBackupMMC->SD2PRESET4 = pHwMMC->SD2PRESET4;
    pBackupMMC->SD2PRESET5 = pHwMMC->SD2PRESET5;
    pBackupMMC->SD2PRESET6 = pHwMMC->SD2PRESET6;
    pBackupMMC->SD3PRESET1 = pHwMMC->SD3PRESET1;
    pBackupMMC->SD3PRESET2 = pHwMMC->SD3PRESET2;
    pBackupMMC->SD3PRESET3 = pHwMMC->SD3PRESET3;
    pBackupMMC->SD3PRESET4 = pHwMMC->SD3PRESET4;
    pBackupMMC->SD3PRESET5 = pHwMMC->SD3PRESET5;
    pBackupMMC->SD3PRESET6 = pHwMMC->SD3PRESET6;    
}
#endif

static void shutdown_mode(void)
{
    volatile unsigned int cnt = 0;
    unsigned way_mask;
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    unsigned int pll1,p,m,s, tck_value;
#endif

#if defined(MEMBUS_CLK_AUTO_RESTORE)
    pll1 = (*(volatile unsigned long *)tcc_p2v(addr_clk(0x000034))) & 0x7FFFFFFF;
    SDRAM_INIT_PARAM(PLL_VALUE) = pll1;
    p = pll1&0x3F;
    m = (pll1>>8)&0x3FF;
    s = (pll1>>24)&0x7;
    tck_value = 1000000/((((XIN_CLK_RATE/10000)*m)/p)>>s);
    SDRAM_INIT_PARAM(DDR_TCK) = tck_value;
#ifdef CONFIG_SUSPEND_MEMCLK
    SDRAM_INIT_PARAM(PLL2_VALUE) = *(volatile unsigned long*)tcc_p2v(addr_clk(0x000038));
    SDRAM_INIT_PARAM(PLL3_VALUE) = *(volatile unsigned long*)tcc_p2v(addr_clk(0x00003C));
    SDRAM_INIT_PARAM(PLL4_VALUE) = *(volatile unsigned long*)tcc_p2v(addr_clk(0x000040));
    SDRAM_INIT_PARAM(PLL5_VALUE) = *(volatile unsigned long*)tcc_p2v(addr_clk(0x000044));
    SDRAM_INIT_PARAM(CKC1_VALUE) = *(volatile unsigned long*)tcc_p2v(addr_clk(0x000004));
#endif
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

#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_SHUTDOWN, 0, 0, 0);
#endif

    /*--------------------------------------------------------------
     BUS Config
    --------------------------------------------------------------*/
    __memcpy(&RegRepo.membuscfg, (PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), sizeof(MEMBUSCFG));
    __memcpy(&RegRepo.iobuscfg, (PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), sizeof(IOBUSCFG));

    /* all peri io bus on */
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

#if defined(CONFIG_ARM_GIC)
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
    __memcpy((void*)GPIO_REPOSITORY_ADDR, (PGPIO)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));

#if defined(CONFIG_BROADCOM_WIFI)
    /*--------------------------------------------------------------
     MMC
    --------------------------------------------------------------*/
#if defined(CONFIG_SUPPORT_BCM4335)
    unsigned int mmc_start_addr = HwSDMMC0_BASE;
#else
    unsigned int mmc_start_addr = HwSDMMC2_BASE;
#endif
    tcc_mmc_backup(mmc_start_addr);
    tcc_mmc_ch_backup(&RegRepo.mmcch, (PMMCCH)tcc_p2v(HwSDMMC_CHCTRL_BASE));
#endif

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
#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_L2X0CTRL, 0, 0, 0);
#else
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
#endif
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



    /////////////////////////////////////////////////////////////////
    save_cpu_reg(SHUTDOWN_FUNC_ADDR, CPU_DATA_REPOSITORY_ADDR, resore_cpu_reg);
    /////////////////////////////////////////////////////////////////



    __asm__ __volatile__ ("nop\n");

    /*--------------------------------------------------------------
     cpu re-init for VFP
    --------------------------------------------------------------*/
    __cpu_early_init();

    /* all peri io bus on */
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN0.nREG = 0xFFFFFFFF;
    ((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE))->HCLKEN1.nREG = 0xFFFFFFFF;

    /*--------------------------------------------------------------
     SMU & PMU
    --------------------------------------------------------------*/

    //__memcpy((PCKC)tcc_p2v(HwCKC_BASE), &RegRepo.ckc, sizeof(CKC));
    {
        //PLL
        //((PCKC)tcc_p2v(HwCKC_BASE))->PLL0CFG.nREG = RegRepo.ckc.PLL0CFG.nREG;
        //((PCKC)tcc_p2v(HwCKC_BASE))->PLL1CFG.nREG = RegRepo.ckc.PLL1CFG.nREG;
#ifndef CONFIG_SUSPEND_MEMCLK
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL2CFG.nREG = RegRepo.ckc.PLL2CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL3CFG.nREG = RegRepo.ckc.PLL3CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL4CFG.nREG = RegRepo.ckc.PLL4CFG.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->PLL5CFG.nREG = RegRepo.ckc.PLL5CFG.nREG;
#endif
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

        tcc_ckc_restore_misc_regs();
    }
    nop_delay(100);

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

    __memcpy((PPIC)tcc_p2v(HwPIC_BASE), &RegRepo.pic, sizeof(PIC));
    __memcpy((PVIC)tcc_p2v(HwVIC_BASE), &RegRepo.vic, sizeof(VIC));
    __memcpy((PTIMER *)tcc_p2v(HwTMR_BASE), &RegRepo.timer, sizeof(TIMER));
    __memcpy((PSMUCONFIG)tcc_p2v(HwSMUCONFIG_BASE), &RegRepo.smuconfig, sizeof(SMUCONFIG));

#if defined(CONFIG_ARM_GIC)
    cpu_cluster_pm_exit();
    cpu_pm_exit();
#endif

#ifdef CONFIG_HAVE_ARM_TWD
    /*--------------------------------------------------------------
     ARM Private Timer (TWD)
    --------------------------------------------------------------*/
     *(volatile unsigned int *)(TWD_TIMER_BASE+TWD_TIMER_CONTROL) = RegRepo.twd_timer_control;
     *(volatile unsigned int *)(TWD_TIMER_BASE+TWD_TIMER_LOAD) = RegRepo.twd_timer_load;
#endif

    /*--------------------------------------------------------------
     BUS Config
    --------------------------------------------------------------*/
    __memcpy((PIOBUSCFG)tcc_p2v(HwIOBUSCFG_BASE), &RegRepo.iobuscfg, sizeof(IOBUSCFG));
    __memcpy((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE), &RegRepo.membuscfg, sizeof(MEMBUSCFG));

    /*--------------------------------------------------------------
     NFC
    --------------------------------------------------------------*/
    tcc_nfc_resume((PNFC)tcc_p2v(HwNFC_BASE), &RegRepo.nfc);    

#if defined(CONFIG_BROADCOM_WIFI)
    /*--------------------------------------------------------------
     MMC
    --------------------------------------------------------------*/
    tcc_mmc_restore(mmc_start_addr);
    tcc_mmc_ch_restore((PMMCCH)tcc_p2v(HwSDMMC_CHCTRL_BASE), &RegRepo.mmcch);
#endif

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    /*--------------------------------------------------------------
     UART block resume
    --------------------------------------------------------------*/
    tcc_pm_uart_resume(&RegRepo.bkuart);
#endif

#ifdef CONFIG_CACHE_L2X0
    /*--------------------------------------------------------------
     L2 cache enable
    --------------------------------------------------------------*/
#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_L2X0CTRL, 0, 0, 0); // turn-off again
    _tz_smc(SMC_CMD_L2X0AUXCTRL, RegRepo.L2_aux, 0, 0);
#else
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL) = RegRepo.L2_aux; //restore aux
#endif
    if (RegRepo.L2_aux & (1 << 16))
        way_mask = (1 << 16) - 1;
    else
        way_mask = (1 << 8) - 1;
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY) = way_mask; //invalidate all
    while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY)&way_mask); //wait for invalidate
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0; //sync
    while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1); //wait for sync
#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_L2X0INVALL, 0, 0, 0);
    _tz_smc(SMC_CMD_L2X0CTRL, 1, 0, 0);
#else
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 1; //cache on
#endif
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

#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_SMP_SECONDARY_ADDR, (virt_to_phys(tcc893x_secondary_startup)), 0, 0);
#else
    __raw_writel((virt_to_phys(tcc893x_secondary_startup)), SEC_CPU_START_ADDR);
#endif
#endif
}
/*=========================================================================*/

#elif defined(CONFIG_SLEEP_MODE)
void tcc_pm_sleep(void)
{
    volatile unsigned int cnt = 0;
    volatile unsigned tmp;

    /* REMAP 0xF0000000 area to SRAM */
    *(volatile unsigned *)0xF4810010 |= 0x80000000;

    /* mmu & cache off */
    __asm__ volatile (
    "mrc p15, 0, r0, c1, c0, 0 \n"
    "bic r0, r0, #(1<<12) \n"       //ICache
    "bic r0, r0, #(1<<2) \n"        //DCache
    "bic r0, r0, #(1<<0) \n"        //MMU
    "mcr p15, 0, r0, c1, c0, 0 \n"
    "nop \n"
    );

#if defined(CONFIG_DRAM_DDR3)
    /* holding to access to dram */
    denali_ctl(47) = 0xFFFFFFFF;
    BITSET(denali_ctl(44),0x1);             // inhibit_dram_cmd=1
    while(!(denali_ctl(46)&(0x1<<15)));     // wait for inhibit_dram_cmd
    BITSET(denali_ctl(47), 0x1<<15);

    /* enter self-refresh */
    BITSET(denali_phy(13), 0x1<<10);                        // lp_ext_req=1
    while(!(denali_phy(13)&(0x1<<26)));                     // until lp_ext_ack==1
    BITCSET(denali_phy(13), 0x000000FF, (2<<2)|(1<<1)|(0));
    BITSET(denali_phy(13), 0x1<<8);                         // lp_ext_cmd_strb=1
    while((denali_phy(13)&(0x003F0000)) != (0x25<<16));     // until lp_ext_state==0x25 : check self-refresh state
    BITCLR(denali_phy(13), 0x1<<8);                         // lp_ext_cmd_strb=0
    BITCLR(denali_phy(13), 0x1<<10);                        // lp_ext_req=0
    while(denali_phy(13)&(0x1<<26));                        // until lp_ext_ack==0
    BITSET(denali_ctl(96), 0x3<<8);                         // DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
    BITCLR(denali_ctl(0),0x1);                              // START[0] = 0

    ddr_phy(0x0E) = 0xffffffff;     // AC IO Config
#elif defined(CONFIG_DRAM_DDR2)
    #error "DDR2 is not implemented"
#endif

    nop_delay(1000);

    /* Clock <- XIN, */
    ((PCKC)HwCKC_BASE)->CLKCTRL0.nREG = 0x002ffff4;     // CPU
    ((PCKC)HwCKC_BASE)->CLKCTRL1.nREG = 0x00200014;     // Memory Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL2.nREG = 0x00000014;     // DDI Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL3.nREG = 0x00000014;     // Graphic Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL4.nREG = 0x00200014;     // I/O Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL5.nREG = 0x00000014;     // Video Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL6.nREG = 0x00000014;     // Video core
    ((PCKC)HwCKC_BASE)->CLKCTRL7.nREG = 0x00000014;     // HSIO BUS
    ((PCKC)HwCKC_BASE)->CLKCTRL8.nREG = 0x00200014;     // SMU Bus
    ((PCKC)HwCKC_BASE)->CLKCTRL9.nREG = 0x00000014;     // G2D BUS
    ((PCKC)HwCKC_BASE)->CLKCTRL10.nREG = 0x00000014;    // CM3 Bus

    /* PLL <- OFF */
    ((PCKC)HwCKC_BASE)->PLL0CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL1CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL2CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL3CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL4CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL5CFG.nREG &= ~0x80000000;

#if defined(CONFIG_MACH_TCC8930ST)
    tcc_stb_suspend();
#endif

    /* SRAM Retention */
    BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<16); //PD_RETN : SRAM retention mode=0

#if defined(TCC_PM_PMU_CTRL)
    /* IP isolation off */
    ((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000BDF;

    /* High Speed I/O Bus power down */
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.HSIOBUS = 0;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.HSIOBUS == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.HSB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_HSBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_HSB == 0);

    /* DDI Bus power down */
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.DBUS = 0;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS0 | ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS1) == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.DB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_DBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_DB == 0);

    /* Graphic Bus power down */
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.GBUS = 0;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.GBUS == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.GB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_GBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_GB == 0);

    /* Video Bus power down */
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.VBUS = 0;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS0 | ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS1) == 1);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.VB = 0;
    ((PPMU)HwPMU_BASE)->PWRDN_VBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PD_VB == 0);
#endif

    /* SSTL & IO Retention */
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode =0

// -------------------------------------------------------------------------
// set wake-up


    /* set wake-up trigger mode : edge */
    ((PPMU)HwPMU_BASE)->PMU_WKMOD0.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKMOD1.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x0;
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x0;

#if defined(CONFIG_INPUT_TCC_REMOTE)
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw26);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw25);
    BITSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw24);
    BITCLR(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, Hw23);

    BITCSET(((PPMU)HwPMU_BASE)->PMU_CONFIG.nREG, 0x00700000, tcc_pm_board_param(BOARD_PARAM_PMU_CFG_RMISEL));
#endif
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.nREG = tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0);
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG  = tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0);
    ((PPMU)HwPMU_BASE)->PMU_WKPOL1.nREG = tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1);
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG  = tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1);

    /* RTC Alarm Wake Up */
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.RTC_WAKEUP = 0; //RTC_PMWKUP - Active High
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.RTC_WAKEUP = 1; //RTC_PMWKUP - PMU WakeUp Enable


// -------------------------------------------------------------------------
// Enter Sleep !!
////////////////////////////////////////////////////////////////////////////
    BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<0);
    nop_delay(10000*3);
////////////////////////////////////////////////////////////////////////////


#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    tcc_pm_board_param(BOARD_PARAM_PMU_WUSTS0) = ((PPMU)HwPMU_BASE)->PMU_WKSTS0.nREG;
    tcc_pm_board_param(BOARD_PARAM_PMU_WUSTS1) = ((PPMU)HwPMU_BASE)->PMU_WKSTS1.nREG;
#endif

// -------------------------------------------------------------------------
// set wake-up
    /* disable all for accessing PMU Reg. !!! */
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x00000000;
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x00000000;

    /* SSTL & IO Retention Disable */
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1

#if defined(CONFIG_MACH_TCC8930ST)
    tcc_stb_resume();
#endif

// -------------------------------------------------------------------------
// BUS Power On
#if defined(TCC_PM_PMU_CTRL)
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    while(((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.MAIN_STATE != 1);
#else
    while(((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.MAIN_STATE);
#endif

    ((PPMU)HwPMU_BASE)->PWRUP_HSBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_HSB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.HSB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.HSIOBUS = 1;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.HSIOBUS == 0);

    ((PPMU)HwPMU_BASE)->PWRUP_DBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_DB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.DB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.DBUS = 1;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS0 & ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.DBUS1) == 0);

    ((PPMU)HwPMU_BASE)->PWRUP_GBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_GB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.GB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.GBUS = 1;
    while (((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.GBUS == 0);

    ((PPMU)HwPMU_BASE)->PWRUP_VBUS.bREG.DATA = 1;
    while (((PPMU)HwPMU_BASE)->PMU_PWRSTS.bREG.PU_VB == 0);
    ((PPMU)HwPMU_BASE)->PMU_SYSRST.bREG.VB = 1;
    ((PMEMBUSCFG)HwMBUSCFG_BASE)->HCLKMASK.bREG.VBUS = 1;
    while ((((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS0 & ((PMEMBUSCFG)HwMBUSCFG_BASE)->MBUSSTS.bREG.VBUS1) == 0);

    /* IP isolation on */
    ((PPMU)HwPMU_BASE)->PMU_ISOL.nREG = 0x00000000;
#endif

// Exit SDRAM Self-refresh ------------------------------------------------------------
#if defined(CONFIG_DRAM_DDR3)

    //Bruce_temp.. insert a routine save/restore routine..
    //#define PLL0_VALUE      0x01013806    // PLL0 : 624MHz for CPU
    #define PLL0_VALUE      0x4201A906    // PLL0 : 425MHz for CPU
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    #define PLL1_VALUE      SDRAM_INIT_PARAM(PLL_VALUE)    // PLL1 for MEM
#else
    #define PLL1_VALUE      0x01012C06    // PLL1 : 600MHz for MEM
#endif
    //#define PLL1_VALUE      0x4101F406    // PLL1 : 1GHz for MEM

//--------------------------------------------------------------------------
//Clock setting..

    *(volatile unsigned long *)addr_clk(0x000000) = 0x002ffff4; //cpu bus : XIN
    *(volatile unsigned long *)addr_clk(0x000004) = 0x00200014; //mem bus : XIN/2 
    *(volatile unsigned long *)addr_clk(0x000010) = 0x00200014; //io bus : XIN/2
    *(volatile unsigned long *)addr_clk(0x000020) = 0x00200014; //smu bus : XIN/2
    *(volatile unsigned long *)addr_clk(0x000030) = PLL0_VALUE; //pll0 for cpu
    *(volatile unsigned long *)addr_clk(0x000030) = 0x80000000 | PLL0_VALUE;
    *(volatile unsigned long *)addr_clk(0x000034) = PLL1_VALUE; //pll1 for mem
    *(volatile unsigned long *)addr_clk(0x000034) = 0x80000000 | PLL1_VALUE;
    cnt=3200; while(cnt--);
    //*(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
    *(volatile unsigned long *)addr_clk(0x000000) = 0x002FFFF0;  // cpu
    //*(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
    *(volatile unsigned long *)addr_clk(0x000004) = 0x00200011;  // mem bus
    *(volatile unsigned long *)addr_clk(0x000010) = 0x00200011;  // io bus
    *(volatile unsigned long *)addr_clk(0x000020) = 0x00200011;  // smu bus
    cnt=3200; while(cnt--);

//--------------------------------------------------------------------------
// Exit self-refresh

    ddr_phy(0x0E) = 0x30c01812; // AC IO Config

    BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
    BITCLR(denali_ctl(39), 0x3<<24); //ZQ_ON_SREF_EXIT = 0
    BITSET(denali_ctl(0),0x1); //START[0] = 1

    //for(i=0;i<11;i++){
    //    BITSET(denali_ctl(123), 0x1<<16); //CTRLUPD_REQ = 1
    //    while(!(denali_ctl(46)&(0x20000)));
    //    BITSET(denali_ctl(47), 0x20000);
    //}

    BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(0<<1)|(1))<<24);
    while(!(denali_ctl(46)&(0x40)));
    BITSET(denali_ctl(47), 0x40);

//--------------------------------------------------------------------------
// MRS Write

    // MRS2
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(31)&0x0000FFFF)<<16);
    denali_ctl(26) = (2<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS3
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(29)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(32)&0x0000FFFF)<<16);
    denali_ctl(26) = (3<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS1
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0x0000FFFF)>>0);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0xFFFF0000)<<0);
    denali_ctl(26) = (1<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

    // MRS0
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(27)&0x00FFFF00)>>8);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0x0000FFFF)<<16);
    denali_ctl(26) = (0<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x4000)));
    BITSET(denali_ctl(47), 0x4000);

//--------------------------------------------------------------------------

    BITCLR(denali_ctl(40) ,0x1<<16); //ZQCS_ROTATE=0x0
    BITCSET(denali_ctl(39) ,0x3<<16, 0x2<<16); //ZQ_REQ=2 : 0x1=short calibration, 0x2=long calibration

//--------------------------------------------------------------------------
// release holding to access to dram

    cnt = 10;    while(cnt--) tmp = BITSET(denali_ctl(13), 1<<24); // AREFRESH = 1
    BITCLR(denali_ctl(44),0x1); //inhibit_dram_cmd=0

//--------------------------------------------------------------------------
#elif defined(CONFIG_DRAM_DDR2)
    #error "DDR2 is not implemented"
#endif

// -------------------------------------------------------------------------
// mmu & cache on

    __asm__ volatile (
    "mrc p15, 0, r0, c1, c0, 0 \n"
    "orr r0, r0, #(1<<12) \n" //ICache
    "orr r0, r0, #(1<<2) \n" //DCache
    "orr r0, r0, #(1<<0) \n" //MMU
    "mcr p15, 0, r0, c1, c0, 0 \n"
    "nop \n"
    );

// -------------------------------------------------------------------------
// REMAP 0xF0000000 area to DRAM
    *(volatile unsigned *)0xF4810010 &= ~0x80000000;
}
EXPORT_SYMBOL(tcc_pm_sleep);

static void sleep_mode(void)
{
    volatile unsigned int cnt = 0;
    unsigned stack;
    unsigned way_mask;
    FuncPtr  pFunc = (FuncPtr )SLEEP_FUNC_ADDR;
#if defined(MEMBUS_CLK_AUTO_RESTORE)
    unsigned int pll1,p,m,s, tck_value;
#endif

#if defined(MEMBUS_CLK_AUTO_RESTORE)
    pll1 = (*(volatile unsigned long *)tcc_p2v(addr_clk(0x000034))) & 0x7FFFFFFF;
    SDRAM_INIT_PARAM(PLL_VALUE) = pll1;
    p = pll1&0x3F;
    m = (pll1>>8)&0x3FF;
    s = (pll1>>24)&0x7;
    tck_value = 1000000/((((XIN_CLK_RATE/10000)*m)/p)>>s);
    SDRAM_INIT_PARAM(DDR_TCK) = tck_value;
#endif

    /*--------------------------------------------------------------
     CKC & PMU
    --------------------------------------------------------------*/
#if defined(TCC_PM_PMU_CTRL)
    __memcpy(&RegRepo.pmu, (PPMU)tcc_p2v(HwPMU_BASE), sizeof(PMU));
#endif
    __memcpy(&RegRepo.ckc, (PCKC)tcc_p2v(HwCKC_BASE), sizeof(CKC));

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
#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_L2X0CTRL, 0, 0, 0); // turn-off again
#else
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0; //cache off
#endif
#endif

    /*--------------------------------------------------------------
     flush tlb & cache
    --------------------------------------------------------------*/
    local_flush_tlb_all();
    flush_cache_all();

    stack = IO_ARM_ChangeStackSRAM();
    /////////////////////////////////////////////////////////////////
    pFunc();
    /////////////////////////////////////////////////////////////////
    IO_ARM_RestoreStackSRAM(stack);

    /*--------------------------------------------------------------
     CKC & PMU
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

#if defined(TCC_PM_PMU_CTRL)
    //__memcpy((PPMU)tcc_p2v(HwPMU_BASE), &RegRepo.pmu, sizeof(PMU));
    {
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
        while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE != 1);
#else
        while(((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.MAIN_STATE);
#endif

        if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_HSB) {    // if High Speed I/O Bus has been turn off.
            ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.HSIOBUS = 0;
            while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.HSIOBUS == 1);
            ((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.HSB = 0;
            ((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_HSBUS.bREG.DATA = 1;
            while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_HSB == 0);
        }
        if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_DB) {     //if DDI Bus has been turn off.
            ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.DBUS = 0;
            while ((((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS0 | ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.DBUS1) == 1);
            ((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.DB = 0;
            ((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_DBUS.bREG.DATA = 1;
            while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_DB == 0);
        }
        if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_GB) {     //if Graphic Bus has been turn off.
            ((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->HCLKMASK.bREG.GBUS = 0;
            while (((PMEMBUSCFG)tcc_p2v(HwMBUSCFG_BASE))->MBUSSTS.bREG.GBUS == 1);
            ((PPMU)tcc_p2v(HwPMU_BASE))->PMU_SYSRST.bREG.GB = 0;
            ((PPMU)tcc_p2v(HwPMU_BASE))->PWRDN_GBUS.bREG.DATA = 1;
            while (((PPMU)tcc_p2v(HwPMU_BASE))->PMU_PWRSTS.bREG.PD_GB == 0);
        }
        if(RegRepo.pmu.PMU_PWRSTS.bREG.PD_VB) {     //if Video Bus has been turn off.
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

#ifdef CONFIG_CACHE_L2X0
    /* L2 cache enable */
#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_L2X0CTRL, 0, 0, 0); // turn-off again
    _tz_smc(SMC_CMD_L2X0AUXCTRL, RegRepo.L2_aux, 0, 0);
#else
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 0;                     // cache off
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_AUX_CTRL) = RegRepo.L2_aux;    // restore aux
#endif
    if (RegRepo.L2_aux & (1 << 16))
        way_mask = (1 << 16) - 1;
    else
        way_mask = (1 << 8) - 1;
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY) = way_mask;           // invalidate all
    while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_INV_WAY)&way_mask);      // wait for invalidate
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC) = 0;               // sync
    while(*(volatile unsigned int *)(L2CACHE_BASE+L2X0_CACHE_SYNC)&1);          // wait for sync
#ifdef CONFIG_ARM_TRUSTZONE
    _tz_smc(SMC_CMD_L2X0INVALL, 0, 0, 0);
    _tz_smc(SMC_CMD_L2X0CTRL, 1, 0, 0);
#else
    *(volatile unsigned int *)(L2CACHE_BASE+L2X0_CTRL) = 1;                     // cache on
#endif
#endif
}
#endif

#ifdef CONFIG_MMC_TCC_SDHC
extern int tcc893x_sd_card_detect(void);
#endif
static inline void tcc_pm_board_config(void)
{
    memset((void *)BOARD_PARAMETER_ADDR, 0x0, BOARD_PARAMETER_SIZE);

    /* --------------------------------------------------------------------- */
    /* PLL25 Power */
    tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) = 0xFFFFFFFF;
    if (machine_is_tcc893x()) {
        if(system_rev == 0x1000)
            tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) = TCC_GPB(4);
        else if(system_rev == 0x2000 || system_rev == 0x3000)
            ;//tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) = TCC_GPB(29);
    }
    else if (machine_is_tcc8930st()) {
    }
    else if (machine_is_m805_893x()) {
        if (system_rev == 0x5000)
            tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) = TCC_GPC(19);
        else if (system_rev == 0x5001 || system_rev == 0x5002)
            tcc_pm_board_param(BOARD_PARAM_PLL25_PORT) = TCC_GPE(23);
    }

    /* --------------------------------------------------------------------- */
    /* Power Key */
    if (machine_is_tcc893x()) {
        if(system_rev == 0x1000) {
            tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1) |= (1<<PMU_WKUP1_GP_E30);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1) |= (1<<PMU_WKUP1_GP_E30);
        }
        else if(system_rev == 0x2000 || system_rev == 0x3000) {
            tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1) |= (1<<PMU_WKUP1_GP_E24);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1) |= (1<<PMU_WKUP1_GP_E24);
        }
    }
    else if (machine_is_tcc8930st()) {
        #if defined(CONFIG_STB_BOARD_UPC_TCC893X)
        tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0) |= (1<<PMU_WKUP0_GP_G16);
        tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0) |= (1<<PMU_WKUP0_GP_G16);
        #elif defined(CONFIG_CHIP_TCC8930)
        tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0) |= (1<<PMU_WKUP0_GP_D14);
        tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0) |= (1<<PMU_WKUP0_GP_D14);
        #endif
    }
    else if (machine_is_m805_893x()) {
        tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1) |= (1<<PMU_WKUP1_GP_E27);
        tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1) |= (1<<PMU_WKUP1_GP_E27);
    }

    /* --------------------------------------------------------------------- */
    /* Remocon Key */
#ifdef CONFIG_INPUT_TCC_REMOTE
    if (machine_is_tcc893x()) {
    }
    else if (machine_is_tcc8930st()) {
        if (system_rev == 0x7231) {
            tcc_pm_board_param(BOARD_PARAM_PMU_CFG_RMISEL) = (2<<20);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0) |= (1<<PMU_WKUP0_GP_D14);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0) |= (1<<PMU_WKUP0_GP_D14);
        }
        else {
            tcc_pm_board_param(BOARD_PARAM_PMU_CFG_RMISEL) = (6<<20);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0) |= (1<<PMU_WKUP0_GP_G17);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0) |= (1<<PMU_WKUP0_GP_G17);
        }
    }
    else if (machine_is_m805_893x()) {
    }
#endif

    /* --------------------------------------------------------------------- */
    /* SD Card Detect */
#ifdef CONFIG_MMC_TCC_SDHC
    if (tcc893x_sd_card_detect() == 1) {
        if (machine_is_tcc893x()) {
            if(system_rev == 0x1000) {
                tcc_pm_board_param(BOARD_PARAM_PMU_WKUP0) |= (1<<PMU_WKUP0_GP_B12);
                tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL0) &= ~(1<<PMU_WKUP0_GP_B12);
            }
            else if(system_rev == 0x2000 || system_rev == 0x3000) {
                tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1) |= (1<<PMU_WKUP1_GP_E28);
                tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1) &= ~(1<<PMU_WKUP1_GP_E28);
            }
        }
        else if (machine_is_tcc8930st()) {
        }
        else if (machine_is_m805_893x()) {
            tcc_pm_board_param(BOARD_PARAM_PMU_WKUP1) |= (1<<PMU_WKUP1_GP_E28);
            tcc_pm_board_param(BOARD_PARAM_PMU_WKPOL1) &= ~(1<<PMU_WKUP1_GP_E28);
        }
    }
#endif

#ifdef CONFIG_PM_CONSOLE_NOT_SUSPEND
    {
        int i;
        printk("\n\nBoard Parameter values\n");
        for (i=0 ; i< BOARD_PARAMETER_SIZE ; i+=0x10)
            printk("%03X: %08X %08X %08X %08X\n", i, *(volatile unsigned *)(BOARD_PARAMETER_ADDR+i), *(volatile unsigned *)(BOARD_PARAMETER_ADDR+i+0x4), *(volatile unsigned *)(BOARD_PARAMETER_ADDR+i+0x8), *(volatile unsigned *)(BOARD_PARAMETER_ADDR+i+0xC));
        printk("\n");
    }
#endif
}

static int tcc_pm_enter(suspend_state_t state)
{
    unsigned long flags;

#ifdef CONFIG_SUSPEND_MEMCLK
    unsigned int mem_clk;
    mem_clk = (tca_ckc_getfbusctrl(FBUS_MEM)*2);
    if (mem_clk > 5330000)
        tca_ckc_setfbusctrl(FBUS_MEM, NOCHANGE, 5330000);
#endif

    /* initial register back-up variables. */
    memset(&RegRepo, 0x0, sizeof(TCC_REG));

    local_irq_save(flags);
    local_irq_disable();

    tcc_pm_board_config();

    /* enter shutdown/sleep mode */
#if defined(CONFIG_SHUTDOWN_MODE)
    shutdown_mode();
#elif defined(CONFIG_SLEEP_MODE)
    sleep_mode();
#endif

    local_irq_restore(flags);

#ifdef CONFIG_SUSPEND_MEMCLK
    if (mem_clk > 5330000)
        tca_ckc_setfbusctrl(FBUS_MEM, NOCHANGE, mem_clk + 1);
#endif
    return 0;
}

static void tcc_pm_power_off(void)
{
#ifdef CONFIG_RTC_DISABLE_ALARM_FOR_PWROFF_STATE        //Disable the RTC Alarm during the power off state
    {
        extern volatile int tca_alarm_disable(unsigned int rtcbaseaddress);
        tca_alarm_disable(tcc_p2v(HwRTC_BASE));
    }
#endif

#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
    if(system_rev == 0x5001 || system_rev == 0x5002)
    {
        gpio_set_value(TCC_GPE(15), 0); // LCD_IODEN
        gpio_set_value(TCC_GPC(14), 0); // IOD1_PWREN
    }
#else
    if(system_rev == 0x5001 || system_rev == 0x5002)
    {
        gpio_set_value(TCC_GPE(19), 0); // LCD_IODEN
        gpio_set_value(TCC_GPC(19), 0); // IOD1_PWREN
    }
#endif

#if defined(CONFIG_REGULATOR_AXP192)
    {
        extern void axp192_power_off(void);
        axp192_power_off();
    }
#elif defined(CONFIG_REGULATOR_AXP202)
    {
        extern void axp202_power_off(void);
        axp202_power_off();
    }
#elif defined(CONFIG_REGULATOR_TCC270)
    {
        extern void tcc270_power_off(void);
        tcc270_power_off();
    }
#elif defined(CONFIG_REGULATOR_RN5T614)
    {
        extern void rn5t614_power_off(void);
        rn5t614_power_off();
        if(system_rev == 0x1008)
            gpio_set_value(TCC_GPC(9), 0);
        else if (system_rev == 0x1006)
            gpio_set_value(TCC_GPG(2), 0);
    }
#endif

#if defined(CONFIG_STB_BOARD_UPC_TCC893X)
    tcc_gpio_config(TCC_GPE(15), GPIO_LOW|GPIO_OUTPUT|GPIO_PULLDOWN);
    gpio_set_value(TCC_GPE(15), 0);
#endif
    while(1);
}

static struct platform_suspend_ops tcc_pm_ops = {
    .valid   = suspend_valid_only_mem,
    .enter   = tcc_pm_enter,
};

static uint32_t restart_reason = 0x776655AA;
static void tcc_pm_restart(char str, const char *cmd)
{
    /* store restart_reason to USTS register */
    if (restart_reason != 0x776655AA)
        ((PPMU)tcc_p2v(HwPMU_BASE))->PMU_USSTATUS.nREG = restart_reason;

    //printk(KERN_DEBUG "reboot: reason=0x%x\n", restart_reason);
    arch_reset(str, NULL);
}

static int tcc_reboot_call(struct notifier_block *this, unsigned long code, void *cmd)
{
    /* XXX: convert reboot mode value because USTS register
     * hold only 8-bit value
     */
    if (code == SYS_RESTART) {
        if (cmd) {
            if (!strcmp(cmd, "bootloader"))
                restart_reason = 1;    /* fastboot mode */
            else if (!strcmp(cmd, "recovery"))
                restart_reason = 2;    /* recovery mode */
            else
                restart_reason = 0;
        } else
            restart_reason = 0;
    }
    return NOTIFY_DONE;
}

static struct notifier_block tcc_reboot_notifier = {
    .notifier_call = tcc_reboot_call,
};

static int __init tcc_pm_init(void)
{
    pm_power_off = tcc_pm_power_off;
    arm_pm_restart = tcc_pm_restart;

    register_reboot_notifier(&tcc_reboot_notifier);
    suspend_set_ops(&tcc_pm_ops);

#if defined(CONFIG_ARM_TRUSTZONE)	/* Secure SRAM Boot - hjbae */
    _tz_smc(SMC_CMD_FINALIZE, 0, 0, 0);
    _tz_smc(SMC_CMD_BLOCK_SW_JTAG, 0, 0, 0);
#endif

    return 0;
}

__initcall(tcc_pm_init);
