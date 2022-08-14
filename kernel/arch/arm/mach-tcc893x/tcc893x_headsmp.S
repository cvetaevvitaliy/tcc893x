/****************************************************************************
arch/arm/mach-tcc893x/tcc893x_headsmp.S
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/


#include <linux/linkage.h>
#include <linux/init.h>

#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define SEC_CPU_START_CFG  0x74400100
#else
#define SEC_CPU_START_CFG 0x1000CDFC	// 0x1000CDFC
#endif

ENTRY(v7_invalidate_l1)
	mov	r0, #0
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate I cache
	mcr	p15, 2, r0, c0, c0, 0
	mrc	p15, 1, r0, c0, c0, 0
	ldr	r1, =0x7fff
	and	r2, r1, r0, lsr #13
	ldr	r1, =0x3ff
	and	r3, r1, r0, lsr #3	@ NumWays - 1
	add	r2, r2, #1		@ NumSets
	and	r0, r0, #0x7
	add	r0, r0, #4	@ SetShift
	clz	r1, r3		@ WayShift
	add	r4, r3, #1	@ NumWays
1:	sub	r2, r2, #1	@ NumSets--
	mov	r3, r4		@ Temp = NumWays
2:	subs	r3, r3, #1	@ Temp--
	mov	r5, r3, lsl r1
	mov	r6, r2, lsl r0
	orr	r5, r5, r6	@ Reg = (Temp<<WayShift)|(NumSets<<SetShift)
	mcr	p15, 0, r5, c7, c6, 2
	bgt	2b
	cmp	r2, #0
	bgt	1b
	dsb
	isb
	mov	pc, lr
ENDPROC(v7_invalidate_l1)

ENTRY(tcc893x_secondary_startup)
hold:	
#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
	ldr r0, =SEC_CPU_START_CFG
	ldr r1, [r0]
	cmp r1, #0x1
#else
	ldr r0, =SEC_CPU_START_CFG
	ldr r1, [r0]
	cmp r1, #0x10
#endif
	bne hold

	//invalidate gabage data on L1 cache.
	bl	v7_invalidate_l1

	/*
	 * we've been released from the cpu_release,secondary_stack
	 * should now contain the SVC stack for this core
	 */
	b	secondary_startup

	.align
1: .long .
	.long pen_release


