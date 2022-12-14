/*
 * linux/arch/arm/mach-tcc892x/time.c
 *
 * Author:  <linux@telechips.com>
 *
 * Description: TCC Timers
 *
 * Copyright (C) 2011 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *  Returns elapsed usecs since last system timer interrupt
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/cnt32_to_63.h>
#include <linux/spinlock.h>
#include <linux/irq.h>	/* for setup_irq() */
#include <linux/mm.h>	/* for PAGE_ALIGN */
#include <linux/clk.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/mutex.h>

#include <asm/io.h>
#include <mach/io.h>
#include <asm/leds.h>
#include <asm/mach/time.h>
#include <mach/bsp.h>
#include <asm/sched_clock.h>

#define TCC_TIMER_FREQ (12000000L) /* 12M */
#if (TCC_TIMER_FREQ < (USEC_PER_SEC))
#   define PRESCALE_TO_MICROSEC(X) ((X) * ((USEC_PER_SEC) / (TCC_TIMER_FREQ)))
#else
#   define PRESCALE_TO_MICROSEC(X) ((X) / ((TCC_TIMER_FREQ) / (USEC_PER_SEC)))
#endif

#define MIN_OSCR_DELTA 2

// Global
static volatile PTIMER pTIMER;
static volatile PPIC pPIC;

#undef TICKLESS_DEBUG_TCC
//#define TICKLESS_DEBUG_TCC


#if defined(TICKLESS_DEBUG_TCC)
static unsigned int  gInt_cnt = 0;
static unsigned int  gTimer_cnt = 0;
static unsigned int  gEvent_cnt = 0;
static unsigned int  gEvent_over_cnt = 0;
extern unsigned long volatile __jiffy_data jiffies;
static unsigned long gdelta_min = 0xFFFFFF;
static unsigned long gdelta_max = 0;
#endif

static u32 notrace tcc892x_read_oscr(void)
{
	return (u32)(pTIMER->TC32MCNT.nREG);
}

/*************************************************************************************************
 * Tickless Part
 *************************************************************************************************/
static irqreturn_t tcc892x_ost0_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *c = dev_id;

	pTIMER->TC32IRQ.bREG.IRQEN0 = 0;			/* Disable interrupt when the counter value matched with CMP0 */
	if (irq >= 32) {
		pPIC->CLR1.nREG |= (1 << (irq-32));		/* Interrupt clear */
	}
	else {
		pPIC->CLR0.nREG |= (1 << irq);		/* Interrupt clear */
	}
	if (pTIMER->TC32IRQ.bREG.IRQCLR)			/* IRQ clear */
		pTIMER->TC32IRQ.bREG.IRQCLR = 1;

	c->event_handler(c);

#if defined(TICKLESS_DEBUG_TCC)
	gInt_cnt++;
#endif

	return IRQ_HANDLED;
}

static int tcc892x_osmr0_set_next_event(unsigned long delta, struct clock_event_device *c)
{
	unsigned long flags, next, oscr;

#if defined(TICKLESS_DEBUG_TCC)
	static unsigned long jiffies_old;
#endif

	raw_local_irq_save(flags);

	pTIMER->TC32IRQ.bREG.IRQEN0 = 1;			/* Enable interrupt at the end of count */
	next = pTIMER->TC32MCNT.nREG + delta;
	pTIMER->TC32CMP0.nREG = next;				/* Load counter value */
	oscr = pTIMER->TC32MCNT.nREG;

	raw_local_irq_restore(flags);

#if defined(TICKLESS_DEBUG_TCC)
	if (delta > 0xEA00)	/* 10ms == 0xEA60 */
		gEvent_over_cnt++;

	if (gdelta_min > delta)
		gdelta_min = delta;
	if (gdelta_max < delta)
		gdelta_max = delta;

	gEvent_cnt++;
	if (gInt_cnt >= 5000) {
		printk("\nMin Delta: %x \t Max Delta: %x\n", gdelta_min, gdelta_max);
		printk("%s(%d) .. jiffies[%d, %d], int[%4d] event[%4d] delta[%08x] next[%08x]oscr[%08x]\n",
		       __func__,
		       gEvent_over_cnt,
		       jiffies,
		       jiffies - jiffies_old,
		       gInt_cnt,
		       gEvent_cnt,
		       delta,
		       next,
		       oscr);
		jiffies_old = jiffies;
		gEvent_cnt = 0;
		gInt_cnt = 0;
		gEvent_over_cnt = 0;
		gdelta_min = 0xffffff;
		gdelta_max = 0;
	}
#endif

	return (signed)(next - oscr) <= MIN_OSCR_DELTA ? -ETIME : 0;
}


static void
tcc892x_osmr0_set_mode(enum clock_event_mode mode, struct clock_event_device *c)
{
	unsigned long flags;

#if defined(TICKLESS_DEBUG_TCC)
	printk("%s: mode %s... %d\n", __func__,
	       mode == CLOCK_EVT_MODE_ONESHOT  ? "ONESHOT"   :
	       mode == CLOCK_EVT_MODE_UNUSED   ? "UNUSED"    :
	       mode == CLOCK_EVT_MODE_SHUTDOWN ? "SHUTDOWN"  :
	       mode == CLOCK_EVT_MODE_RESUME   ? "RESUME"    :
	       mode == CLOCK_EVT_MODE_PERIODIC ? "PERIODIC"  : "non",
	       gTimer_cnt++);
#endif

	switch (mode) {
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		raw_local_irq_save(flags);
		pTIMER->TC32IRQ.bREG.IRQEN0 = 0;		/* Disable interrupt when the counter value matched with CMP0 */
		pPIC->CLR0.bREG.TC1 = 1;					/* PIC Interrupt clear */
		if (pTIMER->TC32IRQ.bREG.IRQCLR)		/* IRQ clear */
			pTIMER->TC32IRQ.bREG.IRQCLR = 1;
		raw_local_irq_restore(flags);
		break;

	case CLOCK_EVT_MODE_RESUME:
	case CLOCK_EVT_MODE_PERIODIC:
		break;
	}
}

static struct clock_event_device ckevt_tcc892x_osmr0 = {
	.name		= "osmr0",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.shift		= 32,
	.rating		= 250,
	.set_next_event	= tcc892x_osmr0_set_next_event,
	.set_mode	= tcc892x_osmr0_set_mode,
};

static struct irqaction tcc892x_timer_irq = {
	.name		= "ost0",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= tcc892x_ost0_interrupt,
	.dev_id		= &ckevt_tcc892x_osmr0,
};

static void __init tcc892x_timer_init(void)
{
	unsigned long	rate;
	int ret;

	rate = TCC_TIMER_FREQ;

	pTIMER	= (volatile PTIMER) tcc_p2v(HwTMR_BASE);
	pPIC	= (volatile PPIC) tcc_p2v(HwPIC_BASE);

	pTIMER->TC32EN.nREG = 1;               /* Timer disable, Prescale is one */
	pTIMER->TC32EN.bREG.EN = 1;            /* Timer Enable */
	if (pTIMER->TC32IRQ.bREG.IRQCLR)       /* IRQ clear */
		pTIMER->TC32IRQ.bREG.IRQCLR = 1;

	pPIC->SEL0.bREG.TC1 = 1;
	pPIC->IEN0.bREG.TC1 = 1;
	pPIC->INTMSK0.bREG.TC1 = 1;
	pPIC->MODEA0.bREG.TC1 = 1;

	setup_sched_clock_needs_suspend(tcc892x_read_oscr, 32, CLOCK_TICK_RATE);
	clockevents_calc_mult_shift(&ckevt_tcc892x_osmr0, CLOCK_TICK_RATE, 4000);

	ckevt_tcc892x_osmr0.max_delta_ns =
		clockevent_delta2ns(0x7fffffff, &ckevt_tcc892x_osmr0);
	ckevt_tcc892x_osmr0.min_delta_ns =
		clockevent_delta2ns(4, &ckevt_tcc892x_osmr0) + 1;
	ckevt_tcc892x_osmr0.cpumask = cpumask_of(0);

    printk("tcc892x_timer_init: CLOCK_TICK_RATE[0x%x]\n", CLOCK_TICK_RATE);

	ret = setup_irq(INT_TC32, &tcc892x_timer_irq);
	if (ret) {
		printk(KERN_ERR "Failed to register timer IRQ: %d\n", ret);
		BUG();
	}

	if (clocksource_mmio_init((void __iomem *)&pTIMER->TC32MCNT.nREG, "oscr", CLOCK_TICK_RATE, 300, 32, clocksource_mmio_readl_up)) {
		printk(KERN_ERR "Failed to register clocksource\n");
		BUG();
	}

	clockevents_register_device(&ckevt_tcc892x_osmr0);
}

struct sys_timer tcc_timer = {
	.init		= tcc892x_timer_init,
};

