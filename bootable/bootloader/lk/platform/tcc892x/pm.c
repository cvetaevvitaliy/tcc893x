/****************************************************************************
 * platform/tcc892x/pm.c
 * Copyright (C) 2014 Telechips Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions andlimitations under the License.
 ****************************************************************************/

#include <arch/ops.h>
#include "config.h"
#include "string.h"
#include "debug.h"
#include <platform/globals.h>
#include <platform/reg_physical.h>
#include <platform/pm.h>
#include "tcc_ddr.h"
#include <platform/sram_map.h>

#define tcc_p2v(pa)     (pa)

extern int check_fwdn_mode(void);

extern unsigned int IO_ARM_ChangeStackSRAM(void);
extern void IO_ARM_RestoreStackSRAM(unsigned int);

static TCC_REG RegRepo;

#if defined(TARGET_BOARD_STB)
    #define CONFIG_MACH_TCC8920ST
    #define CONFIG_INPUT_TCC_REMOTE
    #if defined(TARGET_TCC8925_HDB892F)
        #define CONFIG_STB_BOARD_HDB892F
        #define CONFIG_HDB892F_BOARD_YJ8925T
    #elif defined(TARGET_TCC8925_UPC)
        #define CONFIG_STB_BOARD_UPC
    #elif defined(TARGET_TCC8925_STB_DONGLE)
        #define CONFIG_STB_BOARD_DONGLE
    #endif
#endif

/*===========================================================================

                  DEFINITIONS AND DECLARATIONS FOR MODULE

===========================================================================*/

#if defined(CONFIG_CHIP_TCC8925S)
#define CPU_SRC_PLL     0    // cpu
#define CPU_PLL_ADDR    0x000030
#define MEM_SRC_PLL     1    // mem
#define MEM_PLL_ADDR    0x000034
#else
#define CPU_SRC_PLL     9    // cpu
#define CPU_PLL_ADDR    0x000044
#define MEM_SRC_PLL     8    // mem
#define MEM_PLL_ADDR    0x000040
#endif

#define nop_delay(x) for(cnt=0 ; cnt<x ; cnt++){ \
                    __asm__ __volatile__ ("nop\n"); }

#define addr_clk(b) (0x74000000+b)
#define addr_mem(b) (0x73000000+b)
#define time2cycle(time, tCK)        ((int)((time + tCK - 1) / tCK))

#if (0)
#define _memcpy memcpy
#else
static void _memcpy(void* dest, const void* src, size_t len)
{
    unsigned int i;
    unsigned int* dest_i = (unsigned int *)dest;
    unsigned int* src_i = (unsigned int *)src;

    for(i = 0;i<(len>>2);i++)
        dest_i[i] = src_i[i];
}
#endif

#define denali_ctl(x) (*(volatile unsigned long *)addr_mem(0x500000+(x*4)))
#define denali_phy(x) (*(volatile unsigned long *)addr_mem(0x600000+(x)))
#define ddr_phy(x) (*(volatile unsigned long *)addr_mem(0x420400+(x)))

typedef void (*FuncPtr)(void);

#define L2CACHE_BASE   0xFA000000

#if defined(CONFIG_MACH_TCC8920ST)
static inline void tcc_stb_suspend(void)
{
#if defined(TCC_PM_MEMQ_PWR_CTRL)
#if defined(CONFIG_STB_BOARD_HDB892F)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //VDDQ_MEM_ON : low
#else
    BITCLR(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<22); //VDDQ_MEM_ON : low
#endif /* CONFIG_STB_BOARD_HDB892F */
#endif    
#if defined(CONFIG_STB_BOARD_HDB892F)
       BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<8); //SLEEP_MODE_CTL : low
       BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //VDDQ_MEM_ON : low

    #if defined(CONFIG_STB_BOARD_HDB892F)
        #if defined(CONFIG_HDB892F_BOARD_YJ8925T)
               BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
               BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */
        #else
               BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
               BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */
        #endif
    #endif
#elif defined(CONFIG_STB_BOARD_UPC)
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPEDAT.nREG, 1<<19); /* LED_S_PN */
#else
       BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<16); //SLEEP_MODE_CTL : low
    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<19); //CORE1_ON : low
    BITCLR(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<21); //CORE2_ON : low
#endif /* CONFIG_STB_BOARD_HDB892F */
}

static inline void tcc_stb_resume(void)
{
#if defined(TCC_PM_MEMQ_PWR_CTRL)
#if defined(CONFIG_STB_BOARD_HDB892F)
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //VDDQ_MEM_ON : high
#else
    BITSET(((PGPIO)HwGPIO_BASE)->GPCDAT.nREG, 1<<22); //VDDQ_MEM_ON : high
#endif /* CONFIG_STB_BOARD_HDB892F */
#endif        
#if defined(CONFIG_STB_BOARD_HDB892F)
      BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<8); //SLEEP_MODE_CTL : high
       BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<9); //VDDQ_MEM_ON : high

    #if defined(CONFIG_STB_BOARD_HDB892F)
        #if defined(CONFIG_HDB892F_BOARD_YJ8925T)
               BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
               BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */
        #else
               BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
               BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<4); /* PHY1_ON */
        #endif
    #endif
#elif defined(CONFIG_STB_BOARD_UPC)
    BITCLR(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<3); /* LED_S_PN */
    BITSET(((PGPIO)HwGPIO_BASE)->GPEDAT.nREG, 1<<19); /* LED_S_PN */
#else
    BITSET(((PGPIO)HwGPIO_BASE)->GPFDAT.nREG, 1<<16); //SLEEP_MODE_CTL : high
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<19); //CORE1_ON : high
    BITSET(((PGPIO)HwGPIO_BASE)->GPBDAT.nREG, 1<<21); //CORE2_ON : high    
#endif /* CONFIG_STB_BOARD_HDB892F */
}
#endif

static void tcc_pm_sleep(void)
{
    volatile unsigned int cnt = 0;


    #define CPU_PLL_VALUE 0x01012C03    // PLL5 : 600MHz for CPU

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
//--------------------------------------------------------------------------
// holding to access to dram

	BITSET(denali_ctl(44),0x1); //inhibit_dram_cmd=1
	while(!(denali_ctl(46)&(0x1<<16)));	// wait for inhibit_dram_cmd
	BITSET(denali_ctl(47), 0x1<<16);


    #if (0)
        BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(1<<1)|(0))<<24);
        BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
    #else

#if (1)
//--------------------------------------------------------------------------
//enter self-refresh

	BITSET(denali_phy(0x34), 0x1<<10); //lp_ext_req=1
	while(!(denali_phy(0x34)&(0x1<<27))); //until lp_ext_ack==1
	BITCSET(denali_phy(0x34), 0x000000FF, (2<<2)|(1<<1)|(0));
	BITSET(denali_phy(0x34), 0x1<<8); //lp_ext_cmd_strb=1
	while((denali_phy(0x34)&(0x007F0000)) !=0x00450000); //until lp_ext_state==0x45 : check self-refresh state
	BITCLR(denali_phy(0x34), 0x1<<8); //lp_ext_cmd_strb=0
	BITCLR(denali_phy(0x34), 0x1<<10); //lp_ext_req=0
	while(denali_phy(0x34)&(0x1<<27)); //until lp_ext_ack==0
#else

        while(denali_ctl(45)&(1<<8)); //CONTROLLER_BUSY
        BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(1<<1)|(0))<<24);
        while(denali_ctl(45)&(1<<8)); //CONTROLLER_BUSY
        while(!(denali_ctl(46)&(0x40)));
        BITSET(denali_ctl(47), 0x40);
#endif
        BITSET(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x3
        BITCLR(denali_ctl(0),0x1); //START[0] = 0
        BITSET(denali_phy(0x0C), 0x1<<22); //ctrl_cmosrcv[22] = 0x1
        BITCSET(denali_phy(0x0C), 0x001F0000, 0x1F<<16); //ctrl_pd[20:16] = 0x1f
        BITCSET(denali_phy(0x20), 0x000000FF, 0xF<<4|0xF); //ctrl_pulld_dq[7:4] = 0xf, ctrl_pulld_dqs[3:0] = 0xf
    #endif
#elif defined(CONFIG_DRAM_DDR2)
// -------------------------------------------------------------------------
// SDRAM Self-refresh
    //Miscellaneous Configuration : COMMON
    *(volatile unsigned long *)addr_mem(0x410020) &= ~Hw17; //creq2=0 : enter low power
    while((*(volatile unsigned long *)addr_mem(0x410020))&Hw24); //cack2==0 : wait for acknowledgement to request..

    //MEMCTRL
    *(volatile unsigned long *)addr_mem(0x430004) |= 0x00000001; //clk_stop_en=1 //Bruce_temp_8920
#elif defined(CONFIG_DRAM_LPDDR2)
    #error "LPDDR2 is not implemented"
#else
    #error "not selected"
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

    /* PLL <- OFF */
    ((PCKC)HwCKC_BASE)->PLL0CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL1CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL2CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL3CFG.nREG &= ~0x80000000;
#if !defined(CONFIG_CHIP_TCC8925S)
    ((PCKC)HwCKC_BASE)->PLL4CFG.nREG &= ~0x80000000;
    ((PCKC)HwCKC_BASE)->PLL5CFG.nREG &= ~0x80000000;
#endif

#if defined(CONFIG_MACH_TCC8920ST)
    tcc_stb_suspend();
#endif

    /* SRAM Retention */
    BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<16); //PD_RETN : SRAM retention mode=0

    /* SSTL & IO Retention */
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
    while(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4))
        BITCLR(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode =0

// -------------------------------------------------------------------------
// set wake-up
//Bruce, wake-up source should be surely set here !!!!

    /* set wake-up trigger mode : edge */
    ((PPMU)HwPMU_BASE)->PMU_WKMOD0.nREG = 0xFFFFFFFF;
    ((PPMU)HwPMU_BASE)->PMU_WKMOD1.nREG = 0xFFFFFFFF;
    /* set wake-up polarity : default : active high */
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.nREG = 0x00000000;
    ((PPMU)HwPMU_BASE)->PMU_WKPOL1.nREG = 0x00000000;

    /* Power Key */
#if defined(CONFIG_MACH_M805_892X)
    // TODO:
#elif defined(CONFIG_MACH_TCC8920ST)
    #if defined(CONFIG_STB_BOARD_UPC)
    //set wake-up polarity
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_G16 = 1; //power key - Active Low
    //set wake-up source
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_G16 = 1; //power key
    #else
    //set wake-up polarity
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_D14 = 1; //power key - Active Low
    //set wake-up source
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_D14 = 1; //power key
    #endif

  #if defined(TCC_PM_SLEEP_WFI_USED)
    ((PPIC)HwPIC_BASE)->IEN0.nREG = 0;
    ((PPIC)HwPIC_BASE)->IEN1.nREG = 0;

    #if defined(CONFIG_STB_BOARD_HDB892S) || defined(CONFIG_STB_BOARD_HDB892F)

    #else
    ((PPIC)HwPIC_BASE)->IEN0.bREG.EINT0 = 1;
        #if defined(CONFIG_STB_BOARD_UPC)
        ((PGPIO)HwGPIO_BASE)->EINTSEL0.bREG.EINT00SEL = 24;
        #else
        ((PGPIO)HwGPIO_BASE)->EINTSEL0.bREG.EINT00SEL = 105;
        #endif
    ((PPIC)HwPIC_BASE)->POL0.bREG.EINT0 = 1;
    ((PPIC)HwPIC_BASE)->MODE0.bREG.EINT0 = 1;
    ((PPIC)HwPIC_BASE)->INTMSK0.bREG.EINT0 = 1;
    #endif
    ((PPIC)HwPIC_BASE)->IEN1.bREG.REMOCON = 1;
    #if defined(SUPPORT_WFI_WAKEUP_BY_RTC)
    ((PPIC)HwPIC_BASE)->IEN1.bREG.RTC = 1;
    ((PPIC)HwPIC_BASE)->INTMSK1.bREG.RTC = 1;
    #endif
  #endif
#else
  #if defined(TCC_PM_SLEEP_WFI_USED)
    ((PPIC)HwPIC_BASE)->IEN0.nREG = 0;
    ((PPIC)HwPIC_BASE)->IEN1.nREG = 0;

    ((PPIC)HwPIC_BASE)->IEN0.bREG.EINT0 = 1;
    if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1005 || *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1007 || \
       *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1008)
        ((PGPIO)HwGPIO_BASE)->EINTSEL0.bREG.EINT00SEL = 89;
    else if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1006)
        ((PGPIO)HwGPIO_BASE)->EINTSEL0.bREG.EINT00SEL = 83;
    else
        ((PGPIO)HwGPIO_BASE)->EINTSEL0.bREG.EINT00SEL = 24;
    ((PPIC)HwPIC_BASE)->POL0.bREG.EINT0 = 1;
    ((PPIC)HwPIC_BASE)->MODE0.bREG.EINT0 = 1;
    ((PPIC)HwPIC_BASE)->INTMSK0.bREG.EINT0 = 1;
  #else
    if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1005 || *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1007 || \
       *(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1008)
    {
        //set wake-up polarity
        ((PPMU)HwPMU_BASE)->PMU_WKPOL1.bREG.GPIO_E30 = 1; //power key - Active Low
        //set wake-up source
        ((PPMU)HwPMU_BASE)->PMU_WKUP1.bREG.GPIO_E30 = 1; //power key
    }
    else if(*(volatile unsigned long *)BORAD_PRAMETER_ADDR == 0x1006)
    {
        //set wake-up polarity
        ((PPMU)HwPMU_BASE)->PMU_WKPOL1.bREG.GPIO_E24 = 1; //power key - Active Low
        //set wake-up source
        ((PPMU)HwPMU_BASE)->PMU_WKUP1.bREG.GPIO_E24 = 1; //power key
    }
    else
    {
        //set wake-up polarity
        ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.GPIO_G16 = 1; //power key - Active Low
        //set wake-up source
        ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.GPIO_G16 = 1; //power key
    }
  #endif
#endif

    /* RTC Alarm Wake Up */
    //set wake-up polarity
    ((PPMU)HwPMU_BASE)->PMU_WKPOL0.bREG.RTC_WAKEUP = 0; //RTC_PMWKUP - Active High
    //set wake-up source
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.bREG.RTC_WAKEUP = 1; //RTC_PMWKUP - PMU WakeUp Enable

#if defined(CONFIG_INPUT_TCC_REMOTE)
    ((PPIC)HwPIC_BASE)->IEN1.bREG.REMOCON = 1;
    ((PPIC)HwPIC_BASE)->INTMSK1.bREG.REMOCON = 1;
#endif

// -------------------------------------------------------------------------
// Enter Sleep !!
////////////////////////////////////////////////////////////////////////////
#ifdef TCC_PM_SLEEP_WFI_USED
    __asm__ __volatile__ ("dsb");
    __asm__ __volatile__ ("wfi");
#else
    BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<0);
#endif
////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------
// set wake-up
    //disable all for accessing PMU Reg. !!!
    ((PPMU)HwPMU_BASE)->PMU_WKUP0.nREG = 0x00000000;
    ((PPMU)HwPMU_BASE)->PMU_WKUP1.nREG = 0x00000000;

// -------------------------------------------------------------------------
// SSTL & IO Retention Disable
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<8)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    while(!(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG&(1<<4)))
        BITSET(((PPMU)HwPMU_BASE)->PWRDN_XIN.nREG, 1<<4); //IO_RTO : I/O retention mode disable=1

#if defined(CONFIG_MACH_TCC8920ST)
    tcc_stb_resume();
#endif

// Exit SDRAM Self-refresh ------------------------------------------------------------
#if defined(CONFIG_DRAM_DDR3)

#if defined(CONFIG_CHIP_TCC8925S)
    ((PCKC)HwCKC_BASE)->PLL0CFG.nREG = CPU_PLL_VALUE; //for CPU clock
    ((PCKC)HwCKC_BASE)->PLL0CFG.nREG |= 0x80000000;
    ((PCKC)HwCKC_BASE)->PLL1CFG.nREG |= 0x80000000; //for Mem clock
#else
    ((PCKC)HwCKC_BASE)->PLL5CFG.nREG = CPU_PLL_VALUE; //for CPU clock
    ((PCKC)HwCKC_BASE)->PLL5CFG.nREG |= 0x80000000;
    ((PCKC)HwCKC_BASE)->PLL4CFG.nREG |= 0x80000000; //for Mem clock
#endif
    cnt=3200; while(cnt--);
    ((PCKC)HwCKC_BASE)->CLKCTRL0.nREG = 0x002FFFF0|CPU_SRC_PLL;
    ((PCKC)HwCKC_BASE)->CLKCTRL1.nREG = 0x00200010|MEM_SRC_PLL;

    BITCLR(denali_phy(0x0C), 0x1<<22); //ctrl_cmosrcv[22] = 0x0
    BITCLR(denali_phy(0x0C), 0x001F0000); //ctrl_pd[20:16] = 0x0
    BITCLR(denali_phy(0x20), 0x000000FF); //ctrl_pulld_dq[7:4] = 0x0, ctrl_pulld_dqs[3:0] = 0x0
    while(((PPMU)(HwPMU_BASE))->PWRDN_XIN.nREG&(1<<8))
        BITCLR(((PPMU)(HwPMU_BASE))->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode =0
    *(volatile unsigned long *)addr_mem(0x810004) &= ~(1<<2); //PHY=0
    *(volatile unsigned long *)addr_mem(0x810004) |= (1<<2); //PHY=1
    while(!(((PPMU)(HwPMU_BASE))->PWRDN_XIN.nREG&(1<<8)))
        BITSET(((PPMU)(HwPMU_BASE))->PWRDN_XIN.nREG, 1<<8); //SSTL_RTO : SSTL I/O retention mode disable=1
    BITCLR(denali_ctl(96), 0x3<<8); //DRAM_CLK_DISABLE[9:8] = [CS1, CS0] = 0x0
    BITSET(denali_ctl(0),0x1); //START[0] = 1
//    while(!(denali_ctl(46)&(0x20000)));
//    BITSET(denali_ctl(47), 0x20000);
    BITCSET(denali_ctl(20), 0xFF000000, ((2<<2)|(0<<1)|(1))<<24);
    while(!(denali_ctl(46)&(0x40)));
    BITSET(denali_ctl(47), 0x40);

//--------------------------------------------------------------------------
// MRS Write

    // MRS2
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(31)&0x0000FFFF)<<16);
    denali_ctl(26) = (2<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x8000)));
    BITSET(denali_ctl(47), 0x8000);

    // MRS3
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(29)&0xFFFF0000)>>16);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(32)&0x0000FFFF)<<16);
    denali_ctl(26) = (3<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x8000)));
    BITSET(denali_ctl(47), 0x8000);

    // MRS1
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(28)&0x0000FFFF)>>0);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0xFFFF0000)<<0);
    denali_ctl(26) = (1<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x8000)));
    BITSET(denali_ctl(47), 0x8000);

    // MRS0
    BITCSET(denali_ctl(29), 0x0000FFFF, (denali_ctl(27)&0x00FFFF00)>>8);
    BITCSET(denali_ctl(31), 0xFFFF0000, (denali_ctl(30)&0x0000FFFF)<<16);
    denali_ctl(26) = (0<<0)|(1<<23)|(1<<24)|(1<<25); // MR Select[7-0], MR enable[23], All CS[24], Trigger[25]
    while(!(denali_ctl(46)&(0x8000)));
    BITSET(denali_ctl(47), 0x8000);

//--------------------------------------------------------------------------

    while(!(denali_phy(0x24)&(1<<10))); //ctrl_locked, DLL Locked ...

    BITCLR(denali_ctl(40) ,0x1<<16); //ZQCS_ROTATE=0x0
    BITCSET(denali_ctl(39) ,0x3<<16, 0x2<<16); //ZQ_REQ=2 : 0x1=short calibration, 0x2=long calibration

//--------------------------------------------------------------------------
// release holding to access to dram

    cnt=100; while(cnt--); // for reset DLL on DDR3
    BITCLR(denali_ctl(44),0x1); //inhibit_dram_cmd=0

#elif defined(CONFIG_DRAM_DDR2)
    {
        FuncPtr pFunc = (FuncPtr)(SDRAM_INIT_FUNC_ADDR);
        pFunc();
    }
#elif defined(CONFIG_DRAM_LPDDR2)
    #error "LPDDR2 is not implemented"
#else
    #error "not selected"
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
}

#if defined(TCC_PM_SLEEP_WFI_USED)
static unsigned int uiRegBackup0;
static unsigned int uiRegBackup1;
#endif

static void gpio_rearrange(void)
{
    // TODO:
}

static void sleep_mode(void)
{
    volatile unsigned int cnt = 0;
    unsigned stack;

    FuncPtr  pFunc = (FuncPtr )SLEEP_FUNC_ADDR;

    _memcpy((void*)SLEEP_FUNC_ADDR, (void*)tcc_pm_sleep, SLEEP_FUNC_SIZE);

    /*--------------------------------------------------------------
     CKC & gpio
    --------------------------------------------------------------*/
    _memcpy(&RegRepo.ckc,  (PCKC)tcc_p2v(HwCKC_BASE), sizeof(CKC));

    _memcpy(&RegRepo.gpio, (PCKC)tcc_p2v(HwGPIO_BASE), sizeof(GPIO));
    gpio_rearrange();
    
#if defined(TCC_PM_SLEEP_WFI_USED)
    uiRegBackup0 = ((PPIC)tcc_p2v(HwPIC_BASE))->IEN0.nREG;
    uiRegBackup1 = ((PPIC)tcc_p2v(HwPIC_BASE))->IEN1.nREG;
#endif

    /*--------------------------------------------------------------
     flush tlb & cache
    --------------------------------------------------------------*/
//    local_flush_tlb_all();
//    flush_cache_all();

    stack = IO_ARM_ChangeStackSRAM();

    /////////////////////////////////////////////////////////////////
    pFunc();
    /////////////////////////////////////////////////////////////////
    IO_ARM_RestoreStackSRAM(stack);

#if defined(TCC_PM_SLEEP_WFI_USED)
    ((PPIC)tcc_p2v(HwPIC_BASE))->IEN0.nREG = uiRegBackup0;
    ((PPIC)tcc_p2v(HwPIC_BASE))->IEN1.nREG = uiRegBackup1;
#endif

    /*--------------------------------------------------------------
     CKC & GPIO
    --------------------------------------------------------------*/

    _memcpy((PGPIO)tcc_p2v(HwGPIO_BASE), &RegRepo.gpio, sizeof(GPIO));

    //_memcpy((PCKC)tcc_p2v(HwCKC_BASE), &RegRepo.ckc, sizeof(CKC));
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
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL2.nREG = RegRepo.ckc.CLKCTRL2.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL3.nREG = RegRepo.ckc.CLKCTRL3.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL4.nREG = RegRepo.ckc.CLKCTRL4.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL5.nREG = RegRepo.ckc.CLKCTRL5.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL6.nREG = RegRepo.ckc.CLKCTRL6.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL7.nREG = RegRepo.ckc.CLKCTRL7.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->CLKCTRL8.nREG = RegRepo.ckc.CLKCTRL8.nREG;
        ((PCKC)tcc_p2v(HwCKC_BASE))->SWRESET.nREG = ~(RegRepo.ckc.SWRESET.nREG);

#if (0)
        //Peripheral clock
        _memcpy((void*)&(((PCKC)tcc_p2v(HwCKC_BASE))->PCLKCTRL00.nREG), (void*)&(RegRepo.ckc.PCLKCTRL00.nREG), 0x150-0x80);
#endif
    }
    nop_delay(100);
}

int tcc_pm_enter(void)
{
    unsigned long flags, tmp;

    if (check_fwdn_mode())
        return 0;

    /* initial register back-up variables. */
    memset(&RegRepo, 0x0, sizeof(TCC_REG));

    //local_irq_save(flags);
    //local_irq_disable();
    /* disable interrupt */
    __asm__ __volatile__( \
    "mrs    %0, cpsr\n" \
    "cpsid    i\n" \
    "mrs    %1, cpsr\n" \
    "orr    %1, %1, #128\n" \
    "msr    cpsr_c, %1" \
    : "=r" (flags), "=r" (tmp) \
    : \
    : "memory", "cc");

    /* set board information */
    *(volatile unsigned long *)BORAD_PRAMETER_ADDR = HW_REV;

    /* enter sleep mode */
    sleep_mode();

    //local_irq_restore(flags);
    /* enable interrupt */
    __asm__ __volatile__( \
    "msr    cpsr_c, %0\n " \
    "cpsid    i" \
    : \
    : "r" (flags) \
    : "memory", "cc");

    return 0;
}

