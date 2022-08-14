/****************************************************************************
 * arch/arm/mach-tcc892x/include/mach/sram_map.h
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
#ifndef __SRAM_MAP_H__
#define __SRAM_MAP_H__

/*
                                Shutdown Mode 
    0xF0000000(0x10000000)  ---------------------- 
    (Code Area)            |  sram boot           |  (0x300)
                   0x0300   ----------------------
                           |  shutdown            |  (0x400)
                   0x0700   ----------------------
                           |  wakeup              |  (0x100)
                   0x0800   ----------------------
                           |  sdram init          |  (0x800)
                   0x1000   ----------------------
                           |  sdram change        |  (0x800)
                   0x1800   ----------------------
                           |         ----         |
    0xF0008000(0x10008000)  ----------------------
    (Data Area)            |  reserved            |  (0x100)
                   0x8100   ----------------------
                           |  gpio repository     |  (0x300)
                   0x8400   ----------------------
                           |  cpu reg/mmu data    |  (0x100)
                   0x8500   ----------------------
                           |  board parameter     |  (0x030)
                   0x8530   ----------------------
                           |  sdram change values |  (0x100)
                   0x8630   ----------------------
                           |         ----         |
                            ----------------------
                           |  stack               |
    0xF0010000(0x10010000)  ----------------------


                                 Sleep Mode 
    0xF0000000(0x10000000)  ---------------------- 
    (Code Area)            |  sleep               |  (0x700)
                   0x0700   ----------------------
                           |  sdram init          |  (0x800)
                   0x0F00   ----------------------
                           |  sdram change        |  (0x800)
                   0x1700   ----------------------
                           |         ----         |
    0xF0008000(0x10008000)  ----------------------
    (Data Area)            |  board parameter     |  (0x030)
                   0x8030   ----------------------
                           |  sdram change values |  (0x100)
                   0x8130   ----------------------
                           |         ----         |
                            ----------------------
                           |  stack               |
    0xF0010000(0x10010000)  ----------------------


                              Snapshot (only use in quick-boot time)
    0xF0000000(0x10000000)  ---------------------- 
    (Code Area)            |         ----         |
    0xF0008000(0x10008000)  ----------------------
    (Data Area)            |         ----         |
                   0xF000   ----------------------
                           |  cpu reg/mmu data    |  (0x100)
                   0xF100   ----------------------
                           |  Snapshot restore    |  (0x400)
                   0xF500   ----------------------
                           |         ----         |
                            ----------------------
                           |  stack               |
    0xF0010000(0x10010000)  ----------------------
*/

#define SRAM_CODE_BASE              0xF0000000
#define SRAM_DATA_BASE              0xF0008000
#define SRAM_TOTAL_SIZE             0x00010000

#define SRAM_STACK_ADDR             (SRAM_CODE_BASE+SRAM_TOTAL_SIZE-0x4)
#if defined(CONFIG_SHUTDOWN_MODE)
/* SRAM Code Area. */
#define SRAM_BOOT_ADDR              SRAM_CODE_BASE
#define SRAM_BOOT_SIZE              0x300

#define SHUTDOWN_FUNC_ADDR          (SRAM_BOOT_ADDR+SRAM_BOOT_SIZE)
#define SHUTDOWN_FUNC_SIZE          0x400       // 0x387

#define WAKEUP_FUNC_ADDR            (SHUTDOWN_FUNC_ADDR+SHUTDOWN_FUNC_SIZE)
#define WAKEUP_FUNC_SIZE            0x100       // 0x09b

#define SDRAM_INIT_FUNC_ADDR        (WAKEUP_FUNC_ADDR+WAKEUP_FUNC_SIZE)
#define SDRAM_INIT_FUNC_SIZE        0x800       // 0x6C7

#define SRAM_PM_CODE_END            (SDRAM_INIT_FUNC_ADDR+SDRAM_INIT_FUNC_SIZE)

/* SRAM Data Area. */
#define GPIO_REPOSITORY_ADDR        (SRAM_DATA_BASE+0x100)
#define GPIO_REPOSITORY_SIZE        0x300

#define CPU_DATA_REPOSITORY_ADDR    (GPIO_REPOSITORY_ADDR+GPIO_REPOSITORY_SIZE)
#define CPU_DATA_REPOSITORY_SIZE    0x100

#define SRAM_PM_DATA_END            (CPU_DATA_REPOSITORY_ADDR+CPU_DATA_REPOSITORY_SIZE)

#elif defined(CONFIG_SLEEP_MODE)
/* SRAM Code Area. */
#define SLEEP_FUNC_ADDR             SRAM_CODE_BASE
#define SLEEP_FUNC_SIZE             0x700       // 0x63F

#define SDRAM_INIT_FUNC_ADDR        (SLEEP_FUNC_ADDR+SLEEP_FUNC_SIZE)
#define SDRAM_INIT_FUNC_SIZE        0x800       // 0x6C7

#define SRAM_PM_CODE_END            (SDRAM_INIT_FUNC_ADDR+SDRAM_INIT_FUNC_SIZE)

/* SRAM Data Area. */
#define SRAM_PM_DATA_END            SRAM_DATA_BASE
#endif

/* SRAM Code Area (Common) */
#define SDRAM_CHANGE_FUNC_ADDR      SRAM_PM_CODE_END
#define SDRAM_CHANGE_FUNC_SIZE      0x800       // 0x5FB

/* SRAM Data Area (Common) */
#define BOARD_PARAMETER_ADDR        SRAM_PM_DATA_END
#define BOARD_PARAMETER_SIZE        0x30
#define SDRAM_CHANGE_ARG_BASE       (BOARD_PARAMETER_ADDR+BOARD_PARAMETER_SIZE)
#define SDRAM_CHANGE_ARG_SIZE       0x100

#define SRAM_CODE_SIZE              ((SDRAM_CHANGE_FUNC_ADDR+SDRAM_CHANGE_FUNC_SIZE+(0x1000-1))/0x1000)*0x1000
#endif  /* __SRAM_MAP_H__ */
