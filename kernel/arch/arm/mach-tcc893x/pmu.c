/*
 * linux/arch/arm/mach-tcc893x/pmu.c
 *
 * Copyright (c) 2013 Telechips, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/platform_device.h>
#include <asm/pmu.h>
#include <mach/irqs.h>

static struct resource tcc893x_pmu_resources[] = {
	[0] = {
		.start	= INT_ARMPMU0,
		.end	= INT_ARMPMU0,
		.flags	= IORESOURCE_IRQ,
	},
	[1] = {
		.start	= INT_ARMPMU1,
		.end	= INT_ARMPMU1,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device tcc893x_pmu_device = {
	.name		= "arm-pmu",
	.id		= ARM_PMU_DEVICE_CPU,
	.num_resources	= ARRAY_SIZE(tcc893x_pmu_resources),
	.resource	= tcc893x_pmu_resources,
};

static int __init tcc893x_pmu_init(void)
{
	platform_device_register(&tcc893x_pmu_device);
	return 0;
}
arch_initcall(tcc893x_pmu_init);
