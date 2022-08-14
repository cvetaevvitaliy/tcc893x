/*
 * arch/arch/mach-tcc893x/timer.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2011 NVIDIA Corporation.
 * Copyright (C) 2012 Telechips Inc.
 *
 * Author:
 *	Colin Cross <ccross@google.com>
 *  androidce team <androidce@telechips.com>
 *
 * Copyright (C) 2010-2012 Telechips Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/syscore_ops.h>

#include <asm/mach/time.h>
#include <asm/localtimer.h>
#include <asm/smp_twd.h>
#include <asm/sched_clock.h>

#include <mach/bsp.h>
#include <mach/io.h>
#include <mach/irqs.h>

#define TCC_TIMER_FREQ (24000000L) /* 24MHz */

#if (TCC_TIMER_FREQ < (USEC_PER_SEC))
#   define PRESCALE_TO_MICROSEC(X) ((X) * ((USEC_PER_SEC) / (TCC_TIMER_FREQ)))
#else
#   define PRESCALE_TO_MICROSEC(X) ((X) / ((TCC_TIMER_FREQ) / (USEC_PER_SEC)))
#endif

// Global
static volatile PTIMER pTIMER;
static struct clk *pTimerClk;


#undef TICKLESS_DEBUG_TCC

//#define TCC_USE_BIT_FIELD


#if defined(TICKLESS_DEBUG_TCC)
static unsigned int  gInt_cnt = 0;
static unsigned int  gTimer_cnt = 0;
static unsigned int  gEvent_cnt = 0;
static unsigned int  gEvent_over_cnt = 0;
extern unsigned long volatile __jiffy_data jiffies;
static unsigned long gdelta_min = 0xFFFFFF;
static unsigned long gdelta_max = 0;
#endif

#define MIN_OSCR_DELTA 2


static int tcc893x_timer_set_next_event(unsigned long cycles, struct clock_event_device *evt)
{
	u32 next, oscr;
    unsigned long flags;

#if defined(TICKLESS_DEBUG_TCC)
	static unsigned long jiffies_old;
#endif

    raw_local_irq_save(flags);

    next = pTIMER->TC32MCNT.nREG + cycles;
    pTIMER->TC32CMP0.nREG = next;				/* Load counter value */

    oscr = pTIMER->TC32MCNT.nREG;
    pTIMER->TC32IRQ.bREG.IRQEN0 = 1;			/* Enable interrupt at the end of count */

    raw_local_irq_restore(flags);

#if defined(TICKLESS_DEBUG_TCC)
	if (cycles > 0xEA00)	/* 10ms == 0xEA60 */
		gEvent_over_cnt++;

	if (gdelta_min > cycles)
		gdelta_min = cycles;
	if (gdelta_max < cycles)
		gdelta_max = cycles;

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
		       cycles,
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

static void tcc893x_timer_set_mode(enum clock_event_mode mode, struct clock_event_device *evt)
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
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_ONESHOT:
        raw_local_irq_save(flags);
		pTIMER->TC32IRQ.bREG.IRQEN0 = 0;        /* Disable interrupt when the counter value matched with CMP0 */
		if (pTIMER->TC32IRQ.bREG.IRQCLR)        /* IRQ clear */
			pTIMER->TC32IRQ.bREG.IRQCLR = 1;
        raw_local_irq_restore(flags);
		break;
	case CLOCK_EVT_MODE_PERIODIC:
	case CLOCK_EVT_MODE_RESUME:
		break;
	}

}

static struct clock_event_device tcc893x_clockevent = {
	.name		= "timer0",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.shift      	= 32,
	.rating		= 250,
	.set_next_event	= tcc893x_timer_set_next_event,
	.set_mode	= tcc893x_timer_set_mode,
};

static cycle_t tcc893x_read_cycles(struct clocksource *cs)
{
	return pTIMER->TC32MCNT.nREG;
}


static struct clocksource tcc893x_clocksource = {
	.name		= "timer0",
	.rating		= 300,
	.read		= tcc893x_read_cycles,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 20,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static int clock_valid = 0;

#if 0
unsigned long long notrace sched_clock(void)
{
	return (unsigned long long)(jiffies - INITIAL_JIFFIES) * (NSEC_PER_SEC / HZ);
}
#endif

static u32 notrace tcc893x_update_sched_clock(void)
{
    if (likely(clock_valid))
        return pTIMER->TC32MCNT.nREG;

    return 0;
}

static irqreturn_t tcc893x_timer_interrupt(int irq, void *dev_id)
{
    struct clock_event_device *evt = (struct clock_event_device *)dev_id;

    pTIMER->TC32IRQ.bREG.IRQEN0 = 0;        /* Disable interruptwhen the counter value matched with CMP0 */
    if (pTIMER->TC32IRQ.bREG.IRQCLR)        /* IRQ clear */
        pTIMER->TC32IRQ.bREG.IRQCLR = 1;

	evt->event_handler(evt);

#if defined(TICKLESS_DEBUG_TCC)
	gInt_cnt++;
#endif

    return IRQ_HANDLED;
}

static struct irqaction tcc893x_timer_irq = {
	.name		= "timer1",
    	.irq        	= INT_TC1,
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_TRIGGER_HIGH,
	.handler	= tcc893x_timer_interrupt,
	.dev_id		= &tcc893x_clockevent,
};


#ifdef CONFIG_HAVE_ARM_TWD
static DEFINE_TWD_LOCAL_TIMER(twd_local_timer,
			      HwARMPERI_TIMER_PRIVATE_BASE,
			      INT_LOCALTIMER);

static void __init tcc893x_twd_init(void)
{
	int err = twd_local_timer_register(&twd_local_timer);
	if (err)
		pr_err("twd_local_timer_register failed %d\n", err);
}
#else
#define tcc893x_twd_init()	do {} while(0)
#endif

static void __init tcc893x_init_timer(void)
{
	int ret;

	pTIMER	= (volatile PTIMER) tcc_p2v(HwTMR_BASE);

    /*
     * Enable timer clock
     */
	pTimerClk = clk_get(NULL, "timerz");
	BUG_ON(IS_ERR(pTimerClk));
	clk_enable(pTimerClk);
    	clk_set_rate(pTimerClk, TCC_TIMER_FREQ);


#ifdef CONFIG_HAVE_ARM_TWD
	//twd_base = IO_ADDRESS(HwARMPERI_TIMER_PRIVATE_BASE);
#endif

    /* Initialize the timer */
    pTIMER->TC32EN.nREG = 1;            /* Timer Disable, Prescale is one */
    pTIMER->TC32EN.bREG.EN = 1;         /* Timer Enable */
    if (pTIMER->TC32IRQ.bREG.IRQCLR)    /* Timer IRQ Clear */
        pTIMER->TC32IRQ.bREG.IRQCLR = 1;

    /*
     * Initialize the clocksource device 
     */
    setup_sched_clock_needs_suspend(tcc893x_update_sched_clock, 32, CLOCK_TICK_RATE);
    clocksource_register_hz(&tcc893x_clocksource, CLOCK_TICK_RATE);
    pr_info("Initialize the clocksource device.... rate[%ld] ", clk_get_rate(pTimerClk));

    /*
     * Initialize the clockevent device 
     */
	ret = setup_irq(tcc893x_timer_irq.irq, &tcc893x_timer_irq);
	if (ret) {
		printk(KERN_ERR "Failed to register timer IRQ: %d\n", ret);
		BUG();
	}

	tcc893x_clockevent.mult =
		div_sc(CLOCK_TICK_RATE, NSEC_PER_SEC, tcc893x_clockevent.shift);
	tcc893x_clockevent.max_delta_ns =
		clockevent_delta2ns(0x7fffffff, &tcc893x_clockevent);
	tcc893x_clockevent.min_delta_ns =
		clockevent_delta2ns(0x4, &tcc893x_clockevent);
	tcc893x_clockevent.cpumask = cpumask_of(0);
	tcc893x_clockevent.irq = tcc893x_timer_irq.irq;
	clockevents_register_device(&tcc893x_clockevent);

    clock_valid = 1;

    tcc893x_twd_init();
}

struct sys_timer tcc893x_timer = {
	.init = tcc893x_init_timer,
};

