/****************************************************************************
 * arch/arm/mach-tcc892x/include/mach/clocktable.h
 * Copyright (C) 2013 Telechips Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/

#ifndef __CLOCKTABLE_H__
#define __CLOCKTABLE_H__

struct clock_table_t {  /* Unit.: kHz */
    unsigned int    cpu_clk;
    unsigned int    mem_clk;
    unsigned int    ddi_clk;
    unsigned int    gpu_clk;
    unsigned int    io_clk;
    unsigned int    vbus_clk;
    unsigned int    vcore_clk;
    unsigned int    hsio_clk;
    unsigned int    smu_clk;
};

struct func_clktbl_t {
	struct list_head            list;
    struct device               *dev;
    char                        *name;
    unsigned int                count;
    unsigned int                active_idx;
    unsigned int                num_clktbls;
    const struct clock_table_t  *clktbls;
};

typedef enum {
    CLKTBL_DISABLE,
    CLKTBL_ENABLE,
} clktbl_enable_t;

extern unsigned int clocktable_ctrl(struct func_clktbl_t *fclktbl, unsigned int tbl_index, clktbl_enable_t enable);
extern struct func_clktbl_t *clocktable_get(const char *id);
extern void clocktable_put(struct func_clktbl_t *fclktbl);

extern unsigned int clocktable_getcpu(void);
extern int clocktable_update(unsigned int cpu_rate);
extern int clocktable_init(struct mutex *cpu_lock);
extern void clocktable_exit(void);
extern int clocktable_early_init(void);

#endif /* __CLOCKTABLE_H__ */
