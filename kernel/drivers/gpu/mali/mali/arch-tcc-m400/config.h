/*
 * Copyright (C) 2013 Telechips Inc.
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __ARCH_CONFIG_H__
#define __ARCH_CONFIG_H__

/*
 * IRQ Number
 * 3DPPMMU	16
 * 3DGPMMU	17
 * 3DL2		19
 * 3DPP		24
 * 3DGP		25
 * 3DPMU		26
 */
#if defined(CONFIG_ARCH_TCC893X)
#define GIC_SPI_OFFSET      32
#define _IRQ_3DPP0MMU_	(16+GIC_SPI_OFFSET)
#define _IRQ_3DGPMMU_	(17+GIC_SPI_OFFSET)
#define _IRQ_3DL2_	(19+GIC_SPI_OFFSET)
#define _IRQ_3DPP0_	(24+GIC_SPI_OFFSET)
#define _IRQ_3DGP_	(25+GIC_SPI_OFFSET)
#define _IRQ_3DPMU_	(26+GIC_SPI_OFFSET)

#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
#define _IRQ_3DPP1_	(50+GIC_SPI_OFFSET)
#define _IRQ_3DPP1MMU_	(51+GIC_SPI_OFFSET)
#else
#define _IRQ_3DPP1_	(35+GIC_SPI_OFFSET)
#define _IRQ_3DPP1MMU_	(40+GIC_SPI_OFFSET)
#endif
#else
#define _IRQ_3DPP0MMU_	16
#define _IRQ_3DGPMMU_	17
#define _IRQ_3DL2_	19
#define _IRQ_3DPP0_	24
#define _IRQ_3DGP_	25
#define _IRQ_3DPMU_	26
#endif

/* Note: IRQ auto detection (setting irq to -1) only works if the IRQ is not shared with any other hardware resource */
static _mali_osk_resource_t arch_configuration [] =
{

    	{
                .description = "Mali-400 PMU",
                .base = 0x70002000,
                //.irq = _IRQ_3DPMU_,		
        },
	{
		.description = "Mali-400 GP",
		.base = 0x70000000,
		.irq = _IRQ_3DGP_,
	},
	{
		.base = 0x70008000,
		.irq = _IRQ_3DPP0_,
		.description = "Mali-400 PP 0",
	},
#if defined(CONFIG_ARCH_TCC893X)
	{
		.base = 0x7000A000,
		.irq = _IRQ_3DPP1_,
		.description = "Mali-400 PP 1",
        },
#endif		
	{
		.base = 0x70003000,
		.irq = _IRQ_3DGPMMU_,
		.description = "Mali-400 MMU for GP",
	},
	{
		.base = 0x70004000,
		.irq = _IRQ_3DPP0MMU_,
		.description = "Mali-400 MMU for PP 0",
	},
#if defined(CONFIG_ARCH_TCC893X)
	{
		.base = 0x70005000,
		.irq = _IRQ_3DPP1MMU_,
		.description = "Mali-400 MMU for PP 1",
	},
#endif
#if 0//USING_OS_MEMORY
	{		
		.description = "OS Memory",
		.size = CONFIG_MALI_MEMORY_SIZE * 1024UL * 1024UL,
		.alloc_order = 0, /* highest preference for this memory */
		.flags = _MALI_CPU_WRITEABLE | _MALI_CPU_READABLE | _MALI_PP_READABLE | _MALI_PP_WRITEABLE |_MALI_GP_READABLE | _MALI_GP_WRITEABLE
	},
#endif /*USING_OS_MEMORY*/
#if 0
	{
		.description = "Framebuffer",
		.base = 0xe0000000,
		.size = 0x01000000,
		.flags = _MALI_CPU_WRITEABLE | _MALI_CPU_READABLE | _MALI_PP_WRITEABLE | _MALI_PP_READABLE
	},
#endif	
	{
		.base = 0x70001000,
		.description = "Mali-400 L2 cache"
	},
	

};
#endif /* __ARCH_CONFIG_H__ */
