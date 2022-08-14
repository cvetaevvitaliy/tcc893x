/*
 * Copyright (C) 2010 Telechips, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <debug.h>
#include <arch/arm.h>
#include <reg.h>
#include <kernel/thread.h>
#include <platform/interrupts.h>
#include <platform/irqs.h>
#include <platform/iomap.h>

#define GIC_CPU_BASE		HwARMPERI_GIC_BASE
#define GIC_DIST_BASE           HwARMPERI_INT_DIST_BASE

#define GIC_CPU_REG(off)	(GIC_CPU_BASE + (off))
#define GIC_DIST_REG(off)	(GIC_DIST_BASE + (off))

#define GIC_CPU_CTRL		GIC_CPU_REG(0x00)
#define GIC_CPU_PRIMASK		GIC_CPU_REG(0x04)
#define GIC_CPU_BINPOINT	GIC_CPU_REG(0x08)
#define GIC_CPU_INTACK		GIC_CPU_REG(0x0c)
#define GIC_CPU_EOI		GIC_CPU_REG(0x10)
#define GIC_CPU_RUNNINGPRI	GIC_CPU_REG(0x14)
#define GIC_CPU_HIGHPRI		GIC_CPU_REG(0x18)

#define GIC_DIST_CTRL		GIC_DIST_REG(0x000)
#define GIC_DIST_CTR		GIC_DIST_REG(0x004)
#define GIC_DIST_ENABLE_SET	GIC_DIST_REG(0x100)
#define GIC_DIST_ENABLE_CLEAR	GIC_DIST_REG(0x180)
#define GIC_DIST_PENDING_SET	GIC_DIST_REG(0x200)
#define GIC_DIST_PENDING_CLEAR	GIC_DIST_REG(0x280)
#define GIC_DIST_ACTIVE_BIT	GIC_DIST_REG(0x300)
#define GIC_DIST_PRI		GIC_DIST_REG(0x400)
#define GIC_DIST_TARGET		GIC_DIST_REG(0x800)
#define GIC_DIST_CONFIG		GIC_DIST_REG(0xc00)
#define GIC_DIST_SOFTINT	GIC_DIST_REG(0xf00)

struct ihandler {
	int_handler func;
	void *arg;
};

static struct ihandler handler[NR_IRQS];
void platform_init_interrupts(void)
{
    int i;
    unsigned int gic_irqs;

       /* XXX: relocate vector table */
       __asm__ ("ldr   r1, =_vectors\n" \
                "mov   r0, #0x00000000\n" \
                "add   r2, r0, #(15*4)\n" \
                "1:\n" \
                "ldr   r3, [r1], #4\n" \
                "str   r3, [r0], #4\n" \
                "cmp   r0, r2\n" \
                "bne   1b\n");
	platform_gic_dist_init();
	platform_gic_cpu_init();
}

void platform_gic_dist_init(void)
{
	int i;
	unsigned gic_irqs = 0;

	/* Disable GIC */
	writel(0x0, GIC_DIST_CTRL);

	/*
	 * Find out how many interrupts are supported.
	 * The GIC only supports up to 1020 interrupt sources.
	 */
	gic_irqs = readl(GIC_DIST_CTR) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;
	if (gic_irqs > 1020)
		gic_irqs = 1020;

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (i = 32; i < gic_irqs; i += 16)
		writel(0/*0xffffffff*/, GIC_DIST_CONFIG + i * 4 / 16);

	/*
	 * Set all global interrupts to this CPU only.
	 */
	for (i = 32; i < gic_irqs; i += 4)
		writel(0x01010101, GIC_DIST_TARGET + i * 4 / 4);

 	/*
	 * Set priority on all global interrupts.
	 */
	for (i = 32; i < gic_irqs; i += 4)
		writel(0xa0a0a0a0, GIC_DIST_PRI + i * 4 / 4);

	/*
	 * Disable all interrupts.  Leave the PPI and SGIs alone
	 * as these enables are banked registers.
	 */
	for (i = 32; i < gic_irqs; i += 32)
		writel(0xffffffff, GIC_DIST_ENABLE_CLEAR + i * 4 / 32);

	//writel(0x0000ffff, GIC_DIST_ENABLE_SET);

	/* Enable GIC */
	writel(0x1, GIC_DIST_CTRL);
}

void platform_gic_cpu_init(void)
{
	int i;

	/*
	 * Deal with the banked PPI and SGI interrupts - disable all
	 * PPI interrupts, ensure all SGI interrupts are enabled.
	 */
	writel(0xffff0000, GIC_DIST_ENABLE_CLEAR);
	writel(0x0000ffff, GIC_DIST_ENABLE_SET);

	/*
	 * Set priority on PPI and SGI interrupts
	 */
	for (i = 0; i < 32; i += 4)
		writel(0xa0a0a0a0, GIC_DIST_PRI + i * 4 / 4);

	writel(0xf0, GIC_CPU_PRIMASK);

	writel(0x01, GIC_CPU_CTRL);
}

enum handler_return platform_irq(struct arm_iframe *frame)
{
	unsigned num;
	enum handler_return ret;

	num = readl(GIC_CPU_INTACK) & 0x3ff;
	if (num > NR_IRQS)
		return 0;

	ret = handler[num].func(handler[num].arg);

	/* clear Timer32 */
	writel(((num + 1) % 32), GIC_DIST_PENDING_CLEAR + (num / 32) * 4);

	writel(num, GIC_CPU_EOI);

	return ret;
}

void platform_fiq(struct arm_iframe *frame)
{
	PANIC_UNIMPLEMENTED;
}

status_t mask_interrupt(unsigned int vector)
{
	unsigned mask = 1 << (vector & 31);

	writel(mask, GIC_DIST_ENABLE_CLEAR + (vector / 32) * 4);
	return 0;
}

status_t unmask_interrupt(unsigned int vector)
{
	unsigned mask = 1 << (vector & 31);

	writel(mask, GIC_DIST_ENABLE_SET + (vector / 32) * 4);
	return 0;
}

void register_int_handler(unsigned int vector, int_handler func, void *arg)
{
	if (vector >= NR_IRQS)
		return;

	enter_critical_section();
	handler[vector].func = func;
	handler[vector].arg = arg;
	exit_critical_section();
}

