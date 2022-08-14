/*
 * arch/arm/pl310_cache.c
 *
 * (C)Copyright All rights reserved by Telechips Inc.
 *
 * This source code contains confidential information of Telechips.
 * Any unauthorized use without a written permission of Telechips including not limited
 * to re-distribution in source or binary form is strictly prohibited.
 * This source code is provided ¡°AS IS¡± and nothing contained in this source code
 * shall constitute any express or implied warranty of any kind, including without limitation,
 * any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent,
 * copyright or other third party intellectual property right. No warranty is made, express or implied,
 * regarding the information¡¯s accuracy, completeness, or performance. 
 * In no event shall Telechips be liable for any claim, damages or other liability arising from,
 * out of or in connection with this source code or the use in the source code. 
 * This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
 * between Telechips and Company.
 *
 */

#include <reg.h>
#include <arch/defines.h>

#ifdef CONFIG_ARM_TRUSTZONE /* JJCONFIRMED */
#include <platform/smc.h>
#endif

#if defined(TCC893X)
#define	L2CACHE_BASE			0x6C000000
#define L2CACHE_WAY				16
#define L2CACHE_WAY_SIZE		(32*1024)
#define L2CACHE_AUXCTRL_VAL		0x70150001
#elif defined(TCC892X)
#define	L2CACHE_BASE			0x60000000
#define L2CACHE_WAY				16
#define L2CACHE_WAY_SIZE		(16*1024)
#define L2CACHE_AUXCTRL_VAL		0x70130001
#else
  #err
#endif
#define L2CACHE_AUXCTRL_MASK	(~0x7C3F0001)
#define L2CACHE_WAY_MASK		((unsigned int)((1<<L2CACHE_WAY)-1))

// pl310 registers
#define REG0_CACHE_ID			(L2CACHE_BASE + 0x000)
#define REG0_CACHE_TYPE			(L2CACHE_BASE + 0x004)
#define REG1_CONTROL			(L2CACHE_BASE + 0x100)
#define REG1_AUX_CONTROL		(L2CACHE_BASE + 0x104)
#define REG1_TAG_RAM_CONTROL	(L2CACHE_BASE + 0x108)
#define REG1_DATA_RAM_CONTROL	(L2CACHE_BASE + 0x10C)
#define REG2_EV_COUNTER_CTRL	(L2CACHE_BASE + 0x200)
#define REG2_EV_COUNTER1_CFG	(L2CACHE_BASE + 0x204)
#define REG2_EV_COUNTER0_CFG	(L2CACHE_BASE + 0x208)
#define REG2_EV_COUNTER1		(L2CACHE_BASE + 0x20C)
#define REG2_EV_COUNTER0		(L2CACHE_BASE + 0x210)
#define REG2_INT_MASK			(L2CACHE_BASE + 0x214)
#define REG2_INT_MASK_STATUS	(L2CACHE_BASE + 0x218)
#define REG2_INT_RAW_STATUS		(L2CACHE_BASE + 0x21C)
#define REG2_INT_CLEAR			(L2CACHE_BASE + 0x220)
#define REG7_CACHE_SYNC			(L2CACHE_BASE + 0x730)
#define REG7_INV_PA				(L2CACHE_BASE + 0x770)
#define REG7_INV_WAY			(L2CACHE_BASE + 0x77C)
#define REG7_CLEAN_PA			(L2CACHE_BASE + 0x7B0)
#define REG7_CLEAN_INDEX		(L2CACHE_BASE + 0x7B8)
#define REG7_CLEAN_WAY			(L2CACHE_BASE + 0x7BC)
#define REG7_CLEAN_INV_PA		(L2CACHE_BASE + 0x7F0)
#define REG7_CLEAN_INV_INDEX	(L2CACHE_BASE + 0x7F8)
#define REG7_CLEAN_INV_WAY		(L2CACHE_BASE + 0x7FC)

#ifdef CONFIG_ARM_TRUSTZONE
extern int lk_in_normalworld;
#endif

void pl310_cache_flush_all(void)
{
	writel(L2CACHE_WAY_MASK, REG7_CLEAN_INV_WAY);
	while (readl(REG7_CLEAN_INV_WAY)&L2CACHE_WAY_MASK);
	writel(0, REG7_CACHE_SYNC);
}

void pl310_cache_flush_range(unsigned long start, unsigned long len)
{
	start &= ~(CACHE_LINE-1);
	while (len) {
		writel(start, REG7_CLEAN_INV_PA);
		start += CACHE_LINE;
		len -= CACHE_LINE;
	}
	writel(0, REG7_CACHE_SYNC);
}

void pl310_cache_clean_all(void)
{
	writel(L2CACHE_WAY_MASK, REG7_CLEAN_WAY);
	while (readl(REG7_CLEAN_WAY)&L2CACHE_WAY_MASK);
	writel(0, REG7_CACHE_SYNC);
	return;
}

void pl310_cache_clean_range(unsigned long start, unsigned long len)
{
	start &= ~(CACHE_LINE-1);
	while (len) {
		writel(start, REG7_CLEAN_PA);
		start += CACHE_LINE;
		len -= CACHE_LINE;
	}
	writel(0, REG7_CACHE_SYNC);
}

void pl310_cache_enable(void)
{
	unsigned aux_control;

	// check pl310 cache enable status
	if (readl(REG1_CONTROL) & 1)
		return;

	aux_control = readl(REG1_AUX_CONTROL);
	aux_control &= L2CACHE_AUXCTRL_MASK;
	aux_control |= L2CACHE_AUXCTRL_VAL;
#ifdef CONFIG_ARM_TRUSTZONE
	aux_control |= 0x0C000000;
	if (lk_in_normalworld)
		_tz_smc(SMC_CMD_L2X0AUXCTRL, aux_control, 0, 0);
	else
#endif
	writel(aux_control, REG1_AUX_CONTROL);

	writel(L2CACHE_WAY_MASK, REG7_INV_WAY);
	while (readl(REG7_INV_WAY)&L2CACHE_WAY_MASK);
	writel(0, REG7_CACHE_SYNC);
#ifdef CONFIG_ARM_TRUSTZONE
	if (lk_in_normalworld) {
#if defined(TCC892X)
		_tz_smc(SMC_CMD_L2X0INVALL, 0, 0, 0);
#endif
		_tz_smc(SMC_CMD_L2X0CTRL, 1, 0, 0);
	} else
#endif
	writel(1, REG1_CONTROL);
}

void pl310_cache_disable(void)
{
	pl310_cache_flush_all();
#ifdef CONFIG_ARM_TRUSTZONE
	if (lk_in_normalworld)
		_tz_smc(SMC_CMD_L2X0CTRL, 0, 0, 0);
	else
#endif
	writel(0, REG1_CONTROL);
	__asm__ __volatile__( \
	"dsb" \
	: \
	: \
	: "memory");
}


