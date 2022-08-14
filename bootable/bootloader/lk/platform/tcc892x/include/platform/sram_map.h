/****************************************************************************
 * arch/arm/mach-tcc892x/include/mach/sram_map.h
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
    (Code Area)            |  dummy               |  (0x700)
                   0x0700   ----------------------
                           |  sleep               |  (0x700)
                   0x0D00   ----------------------
                           |         ----         |
    0xF0008000(0x10008000)  ----------------------
    (Data Area)            |  board parameter     |  (0x010)
                   0x8010   ----------------------
                           |         ----         |
                            ----------------------
                           |  stack               |
    0xF0010000(0x10010000)  ----------------------
*/

#define SRAM_CODE_BASE              0x10000000
#define SRAM_DATA_BASE              0x10008000
#define SRAM_TOTAL_SIZE             0x00010000

#define SRAM_STACK_ADDR             (SRAM_CODE_BASE+SRAM_TOTAL_SIZE-0x4)
/* SRAM Code Area. */
#define SLEEP_FUNC_ADDR             (SRAM_CODE_BASE+0x700)
#define SLEEP_FUNC_SIZE             0x700       // 0x66B

#define SRAM_PM_CODE_END            (SLEEP_FUNC_ADDR+SLEEP_FUNC_SIZE)

/* SRAM Data Area. */
#define SRAM_PM_DATA_END            SRAM_DATA_BASE

/* SRAM Code Area (Common) */

/* SRAM Data Area (Common) */
#define BORAD_PRAMETER_ADDR         SRAM_PM_DATA_END
#define BOARD_PRAMETER_SIZE         0x010

#endif  /* __SRAM_MAP_H__ */
