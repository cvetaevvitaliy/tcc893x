/****************************************************************************
 * platform/tcc892x/include/platform/pm.h
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

#ifndef __TCC_PM_H__
#define __TCC_PM_H__

#if defined(DRAM_DDR3)
#define CONFIG_DRAM_DDR3
#elif defined(DRAM_DDR2)
#define CONFIG_DRAM_DDR2
#else
    #error
#endif

#define TCC_PM_SSTLIO_CTRL
#define TCC_PM_CHECK_WAKEUP_SOURCE
#define TCC_PM_SLEEP_WFI_USED

typedef struct _TCC_REG_{
    CKC ckc;
    PMU pmu;
    GPIO gpio;
} TCC_REG, *PTCC_REG;

#endif  /*__TCC_PM_H__*/
