/*
 * linux/arch/arm/mach-tcc892x/io.c
 *
 * Author:  <linux@telechips.com>
 * Created: November, 2011
 * Description: TCC892x mapping code
 *
 * Copyright (C) Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/init.h>
#include <linux/memblock.h>

#include <asm/tlb.h>
#include <asm/mach/map.h>
#include <asm/io.h>

#ifdef CONFIG_ARM_TRUSTZONE /* JJCONFIRMED */
#include <mach/smc.h>
#endif

#include <mach/bsp.h>
#include <mach/tcc_sram.h>
#include <plat/pmap.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif

void __cpu_early_init(void);

#if defined(__TODO__)
extern void IO_UTIL_ReadECID(void);
#endif

/*
 * The machine specific code may provide the extra mapping besides the
 * default mapping provided here.
 */
static struct map_desc tcc8920_io_desc[] __initdata = {
    {
        .virtual    = 0xF0000000,
        .pfn        = __phys_to_pfn(0xF0000000), //Internel SRAM
        .length     = 0x00100000,
        .type       = MT_MEMORY_TCC
    },
    {
        .virtual    = 0xF1000000,
        .pfn        = __phys_to_pfn(0x70000000), //Graphic Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF2000000,
        .pfn        = __phys_to_pfn(0x71000000), //HSIO Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF3000000,
        .pfn        = __phys_to_pfn(0x72000000), //Display Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF4000000,
        .pfn        = __phys_to_pfn(0x73000000), //Memory Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF5000000,
        .pfn        = __phys_to_pfn(0x74000000), //SMU Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF6000000,
        .pfn        = __phys_to_pfn(0x75000000), //Video Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF7000000,
        .pfn        = __phys_to_pfn(0x76000000), //IO Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF8000000,
        .pfn        = __phys_to_pfn(0x77000000), //CPU Bus
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xF9000000,
        .pfn        = __phys_to_pfn(0x78000000), //DAP(Debug)
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
    {
        .virtual    = 0xFA000000,
        .pfn        = __phys_to_pfn(0x60000000), //L2Cache
        .length     = 0x01000000,
        .type       = MT_DEVICE
    },
	{
        .virtual    = 0xFB000000,
        .pfn        = __phys_to_pfn(0x30000000), //Boot Rom
        .length     = 0x1000,
        .type       = MT_DEVICE
    },
};

#ifdef CONFIG_ARM_TRUSTZONE /* JJCONFIRMED */
// TZ OS uses last 1MB
#define TZ_RESERVE_SIZE 0x100000
#define TZ_RESERVE1_BASE 0x9FF00000
#define TZ_RESERVE2_BASE 0xBFF00000
#endif

void __init tcc_reserve_sdram(void)
{
	pmap_t pmap;

#ifdef CONFIG_ARM_TRUSTZONE /* JJCONFIRMED */
	if (memblock_is_region_reserved(TZ_RESERVE1_BASE, TZ_RESERVE_SIZE)) {
		BUG_ON(1);
	} else {
		if (memblock_reserve(TZ_RESERVE1_BASE, TZ_RESERVE_SIZE))
			printk(KERN_INFO "1. FAIL: reserved for TZSW: base=0x%x, size=0x%x\n", TZ_RESERVE1_BASE, TZ_RESERVE_SIZE);
		//else
		//	printk(KERN_INFO "1. SUCC: reserved for TZSW: base=0x%x, size=0x%x\n", TZ_RESERVE1_BASE, TZ_RESERVE_SIZE);
	}

	if (memblock_is_region_reserved(TZ_RESERVE2_BASE, TZ_RESERVE_SIZE)) {
		BUG_ON(1);
	} else {
		if (memblock_reserve(TZ_RESERVE2_BASE, TZ_RESERVE_SIZE))
			printk(KERN_INFO "2. FAIL: reserved for TZSW: base=0x%x, size=0x%x\n", TZ_RESERVE2_BASE, TZ_RESERVE_SIZE);
		//else
		//	printk(KERN_INFO "2. SUCC: reserved for TZSW: base=0x%x, size=0x%x\n", TZ_RESERVE2_BASE, TZ_RESERVE_SIZE);
	}
#endif

	if (pmap_get_info("total", &pmap)) {
		if (memblock_remove(pmap.base, pmap.size) == 0) {
			printk(KERN_DEBUG "Total reserved memory: base=0x%x, size=0x%x\n", pmap.base, pmap.size);
		} else {
			printk(KERN_ERR "Can't reserve memory (base=0x%x, size=0x%x)\n", pmap.base, pmap.size);
		}
	}
}

/*
 *  Maps common IO regions for tcc892x.
 */
void __init tcc_map_common_io(void)
{
	iotable_init(tcc8920_io_desc, ARRAY_SIZE(tcc8920_io_desc));

	/* Normally devicemaps_init() would flush caches and tlb after
	 * mdesc->map_io(), but we must also do it here because of the CPU
	 * revision check below.
	 */
	local_flush_tlb_all();
	flush_cache_all();

	__cpu_early_init();
	tcc_sram_init();

#ifdef CONFIG_CACHE_L2X0
#ifdef CONFIG_ARM_TRUSTZONE /* JJCONFIRMED, NOTCONFIRMEMD */
	//_tz_smc(SMC_CMD_L2X0INVALL, 0, 0, 0);
	//_tz_smc(SC_CMD_L2X0CTRL, 1, 0, 0);

	// Way size = 16KB, Associativity = 16Way
	l2x0_init((void __iomem *)0xFA000000, 0x7C130001, ~0x022C0000);
#else
	// Way size = 16KB, Associativity = 16Way
	l2x0_init((void __iomem *)0xFA000000, 0x70130001, ~0x022C0000);
#endif
#endif

#if defined(__TODO__)
	IO_UTIL_ReadECID();
#endif

//	tcc_reserve_sdram();

	// XXX
	tca_ckc_init();
}
