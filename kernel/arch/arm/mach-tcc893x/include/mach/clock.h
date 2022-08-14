/****************************************************************************
 * arch/arm/mach-tcc893x/include/mach/clock.h
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

#ifndef __TCC893X_CLOCK_H
#define __TCC893X_CLOCK_H

/* Flags */
#define CLK_FIXED_RATE		(1 << 0)
#define CLK_ALWAYS_ENABLED	(1 << 1)
#define CLK_AUTO_OFF		(1 << 2)
#define CLK_RATE_PROPAGATES	(1 << 3)

struct clk {
	struct list_head list;
	unsigned int id;
	const char *name;
	struct clk *parent;
	struct device *dev;
	unsigned long rate;			// requested rate
	unsigned long real_rate;	// setted rate
	unsigned int flags;
	int usecount;
	unsigned long (*get_rate)(struct clk *);
	int (*set_rate)(struct clk *, unsigned long);
	int (*enable)(struct clk *);
	void (*disable)(struct clk *);
	unsigned int pwdn_idx;
	int (*pwdn)(unsigned int, unsigned int);
	unsigned int swreset_idx;
	int (*swreset)(unsigned int, unsigned int);
	unsigned int clock_idx;
	int (*clock)(unsigned int, unsigned int);
};

#endif
