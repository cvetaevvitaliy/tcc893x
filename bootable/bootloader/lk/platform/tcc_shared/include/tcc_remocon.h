/*
 * IR driver for remote controller : tcc_remocon.h
 *
 * Copyright (C) 2010 Telechips, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __TCC_REMOCON_H__
#define __TCC_REMOCON_H__

#define IR_SIGNAL_MAXIMUN_BIT_COUNT	32
#define IR_FIFO_READ_COUNT          17
#define IR_BUFFER_BIT_SHIFT         16
#define IR_ID_MASK                  0x0000FFFF
#define IR_CODE_MASK                0xFFFF0000

#define LOW_MIN_VALUE               0x0300 << 1
#define LOW_MAX_VALUE               0x0FFF << 1

#define HIGH_MIN_VALUE              0x1500 << 1
#define HIGH_MAX_VALUE              0x1E00 << 1

#define REPEAT_MIN_VALUE            0x4000 << 1
#define REPEAT_MAX_VALUE            0x7000 << 1

#define START_MIN_VALUE             0x2000 << 1
#define START_MAX_VALUE             0x2800 << 1

#if defined(CONFIG_YAOJIN_IR)
#define REMOCON_ID                  0xFE01
#define SCAN_POWER                  0xE31C
#define SCAN_NUM_0                  0xF30C
#define SCAN_NUM_9                  0xF20D
#elif defined(CONFIG_CS_2000_IR_X_CANVAS)
#define REMOCON_ID                  0xFB04
#define SCAN_POWER                  0xF708
#define SCAN_NUM_0                  0xEF10
#define SCAN_NUM_9                  0xE619
#endif

#if defined(PLATFORM_TCC892X)
	#define CONFIG_ARCH_TCC892X
	#define IO_OFFSET       0x00000000  /* Virtual IO = 0xB0000000 */
	#define tcc_p2v(pa) ((unsigned int)(pa) + IO_OFFSET)
#elif defined(PLATFORM_TCC893X)
	#define CONFIG_ARCH_TCC893X
	#define IO_OFFSET       0x00000000  /* Virtual IO = 0xB0000000 */
	#define tcc_p2v(pa) ((unsigned int)(pa) + IO_OFFSET)
#endif

extern int initRemocon(void);
extern int setUseRemoteIrq(int i);
extern int getRemoteKey(void);

#endif	//__TCC_REMOCON_H__

