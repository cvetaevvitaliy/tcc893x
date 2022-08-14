/****************************************************************************
 * arch/arm/mach-tcc892x/include/mach/pm.h
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
#ifndef __TCC_PM_H__
#define __TCC_PM_H__

//#define TCC_PM_PMU_CTRL
//#define TCC_PM_MEMQ_PWR_CTRL
#define TCC_PM_SSTLIO_CTRL
#define TCC_PM_CHECK_WAKEUP_SOURCE

#if defined(CONFIG_PM_CONSOLE_NOT_SUSPEND)
typedef struct _BACKUP_UART {
    volatile unsigned int    DLL;   // 0x000  R/W  0x00000000   Divisor Latch (LSB) (DLAB=1)
    volatile unsigned int    IER;   // 0x004  R/W  0x00000000   Interrupt Enable Register (DLAB=0)
    volatile unsigned int    DLM;   // 0x004  R/W  0x00000000   Divisor Latch (MSB) (DLAB=1)
    volatile unsigned int    LCR;   // 0x00C  R/W  0x00000003   Line Control Register
    volatile unsigned int    MCR;   // 0x010  R/W  0x00000040   MODEM Control Register
    volatile unsigned int    AFT;   // 0x020  R/W  0x00000000   AFC Trigger Level Register
    volatile unsigned int    UCR;   // 0x024  R/W  0x00000000   UART Control register
    volatile unsigned int    CFG0;  // R/W         0x00000000   Port Configuration Register 0(PCFG0)
    volatile unsigned int    CFG1;  // R/W         0x00000000   Port Configuration Register 1(PCFG1)
} bkUART;
#endif

#if defined(CONFIG_SHUTDOWN_MODE)
typedef struct _TCC_REG_{
    CKC ckc;
    PIC pic;
    VIC vic;
    GPIO gpio;
    TIMER timer;
    #if defined(TCC_PM_PMU_CTRL)
    PMU pmu;
    #endif
    SMUCONFIG smuconfig;
    IOBUSCFG iobuscfg;
    MEMBUSCFG membuscfg;
    NFC nfc;
    unsigned L2_aux;
    #if defined(CONFIG_PM_CONSOLE_NOT_SUSPEND)
    bkUART    bkuart;
    #endif
    #if defined(CONFIG_BROADCOM_WIFI)
    MMC mmc;
    MMCCH mmcch;
    #endif
} TCC_REG, *PTCC_REG;
#elif defined(CONFIG_SLEEP_MODE)
#define TCC_PM_SLEEP_WFI_USED

typedef struct _TCC_REG_{
    CKC ckc;
    PMU pmu;
    GPIO gpio;
    unsigned L2_aux;
    #ifdef CONFIG_SNAPSHOT_BOOT
    PIC pic;
    VIC vic;
    TIMER timer;
    SMUCONFIG smuconfig;
    IOBUSCFG iobuscfg;
    MEMBUSCFG membuscfg;
    NFC nfc;
    #if defined(CONFIG_PM_CONSOLE_NOT_SUSPEND)
    bkUART    bkuart;
    #endif
    #endif
} TCC_REG, *PTCC_REG;
#endif

#endif  /*__TCC_PM_H__*/
