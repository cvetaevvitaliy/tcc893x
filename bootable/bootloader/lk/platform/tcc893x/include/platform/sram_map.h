/****************************************************************************
 * arch/arm/mach-tcc893x/include/mach/sram_map.h
 * Copyright (C) 2014 Telechips Inc.

 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions andlimitations under the License.
 ****************************************************************************/

#ifndef __SRAM_MAP_H__
#define __SRAM_MAP_H__

/*
                                 Sleep Mode 
    0xF0000000(0x10000000)  ---------------------- 
    (Code Area)            |  dummy               |  (0x0700)
                   0x0700   ----------------------
    (Code Area)            |  sleep               |  (0x1000)
                   0x1000   ----------------------
                           |         ----         |
    0xF0008000(0x10008000)  ----------------------
    (Data Area)            |  ddr clk_rate value  |  (0x010)
                   0x8010   ----------------------
                           |  board parameter     |  (0x010)
                   0x8020   ----------------------
                           |         ----         |
                            ----------------------
                           |  stack               |
    0xF0010000(0x10010000)  ----------------------
*/

#define SRAM_CODE_BASE              0x10000000
#define SRAM_DATA_BASE              0x10008000
#define SRAM_TOTAL_SIZE             0x00010000

#define SRAM_STACK_ADDR             (SRAM_CODE_BASE+SRAM_TOTAL_SIZE-0x4)
#define TCC_PM_SLEEP_WFI_USED

/* SRAM Code Area. */
#define SRAM_BOOT_ADDR              SRAM_CODE_BASE
#define SRAM_BOOT_SIZE              0x700
#define SLEEP_FUNC_ADDR             (SRAM_BOOT_ADDR+SRAM_BOOT_SIZE)
#define SLEEP_FUNC_SIZE             0x1000
#define WAKEUP_FUNC_ADDR            (SLEEP_FUNC_ADDR+SLEEP_FUNC_SIZE)
#define WAKEUP_FUNC_SIZE            0x200
#define SDRAM_INIT_FUNC_ADDR        (WAKEUP_FUNC_ADDR+WAKEUP_FUNC_SIZE)
#define SDRAM_INIT_FUNC_SIZE        0x1000
#define SRAM_PM_CODE_END            (SDRAM_INIT_FUNC_ADDR+SDRAM_INIT_FUNC_SIZE)

/* SRAM Data Area. */
#define CPU_DATA_REPOSITORY_ADDR    SRAM_DATA_BASE
#define CPU_DATA_REPOSITORY_SIZE    0x100
#define SRAM_PM_DATA_END            (CPU_DATA_REPOSITORY_ADDR+CPU_DATA_REPOSITORY_SIZE)

/* SRAM Code Area (Common) */
#define SRAM_CODE_AREA_END          SRAM_PM_CODE_END

#if (SRAM_CODE_AREA_END > SRAM_DATA_BASE)
#error Overflow the code area
#endif

/* SRAM Data Area (Common) */
#define SDRAM_INIT_PARAM_ADDR       SRAM_PM_DATA_END
#define SDRAM_INIT_PARAM_SIZE       0x010

#define BORAD_PRAMETER_ADDR         (SDRAM_INIT_PARAM_ADDR+SDRAM_INIT_PARAM_SIZE)
#define BOARD_PRAMETER_SIZE         0x010

#define SRAM_DATA_AREA_END          (BORAD_PRAMETER_ADDR+BOARD_PRAMETER_SIZE)

#if (SRAM_DATA_AREA_END > (SRAM_CODE_BASE+SRAM_TOTAL_SIZE))
#error Overflow the data area
#endif

#endif  /* __SRAM_MAP_H__ */
