/****************************************************************************
 * arch/arm/mach-tcc893x/tcc_sram.c
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

#include <linux/module.h>
#include <mach/tcc_sram.h>

#if defined(CONFIG_SHUTDOWN_MODE)
extern void SRAM_Boot(void);
extern void tcc_pm_shutdown(void);
extern void tcc_pm_wakeup(void);
extern void tcc_pm_sdram_init(void);
#elif defined(CONFIG_SLEEP_MODE)
extern void tcc_pm_sleep(void);
#endif
extern unsigned int tcc_pm_time2cycle(unsigned int time, unsigned int tCK);
#if defined(CONFIG_CLOCK_TABLE) || defined(CONFIG_SUSPEND_MEMCLK)
extern void tcc_sdram_change(void);
#endif

void tcc_sram_init(void)
{
#if defined(CONFIG_SHUTDOWN_MODE)
    memcpy((void*)SRAM_BOOT_ADDR,           (void*)SRAM_Boot,           SRAM_BOOT_SIZE);
    memcpy((void*)SHUTDOWN_FUNC_ADDR,       (void*)tcc_pm_shutdown,        SHUTDOWN_FUNC_SIZE);
    memcpy((void*)WAKEUP_FUNC_ADDR,         (void*)tcc_pm_wakeup,          WAKEUP_FUNC_SIZE);
    memcpy((void*)SDRAM_INIT_FUNC_ADDR,     (void*)tcc_pm_sdram_init,      SDRAM_INIT_FUNC_SIZE);
#elif defined(CONFIG_SLEEP_MODE)
    memcpy((void*)SLEEP_FUNC_ADDR,          (void*)tcc_pm_sleep,           SLEEP_FUNC_SIZE);
#endif
    memcpy((void*)SDRAM_TIME2CYCLE_ADDR,    (void*)tcc_pm_time2cycle,          SDRAM_TIME2CYCLE_SIZE);
#if defined(CONFIG_CLOCK_TABLE) || defined(CONFIG_SUSPEND_MEMCLK)
    memcpy((void *)SDRAM_CHANGE_FUNC_ADDR,  (void*)tcc_sdram_change,    SDRAM_CHANGE_FUNC_SIZE);
#endif
}
EXPORT_SYMBOL(tcc_sram_init);

