/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <debug.h>
#include <reg.h>
#include <platform/iomap.h>
#include <qgic.h>
#include <qtimer.h>
#include <mmu.h>
#include <arch/arm/mmu.h>
#include <board.h>
#include <boot_stats.h>
#include <platform/tcc_ckc.h>
#include <lib/heap.h>

#define DMA_START ((unsigned long)&_dma_start)
#define DMA_LEN ((size_t)&_end_of_dma - (size_t)&_dma_start)


static void platform_init_timer(void);

static uint32_t ticks_per_sec = 0;
unsigned int system_rev;

#define MB (1024*1024)

/* 64KB Internal SRAM (Shared by Hardware)*/
#define SRAM_BASE       0x10000000
#define SRAM_SIZE       1
#define SRAM_MEMORY     (MMU_MEMORY_TYPE_STRONGLY_ORDERED | \
                         MMU_MEMORY_AP_READ_WRITE)

/* LK memory - cacheable, write through */
#define LK_BASE         BASE_ADDR
#define LK_SIZE         (((MEMBASE-BASE_ADDR)+MEMSIZE)/MB)
#define LK_MEMORY       (MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE | \
                         MMU_MEMORY_AP_READ_WRITE)

/* Peripherals - non-shared device */
#define IOMAP_BASE      0x70000000
#define IOMAP_SIZE      (0x10000000/MB)
#define IOMAP_MEMORY    (MMU_MEMORY_TYPE_DEVICE_NON_SHARED | \
                         MMU_MEMORY_AP_READ_WRITE)

#ifdef WITH_DMA_ZONE
/* DMA memory - cacheable, write through */
#define DMA_BASE        (((MEMBASE+MEMSIZE+0x100000-1)>>20)<<20)
#define DMA_SIZE        (_DMA_SIZE/MB)
#define DMA_MEMORY      (MMU_MEMORY_TYPE_STRONGLY_ORDERED/*MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH*/ | \
                         MMU_MEMORY_AP_READ_WRITE)
#endif

#ifdef CONFIG_SNAPSHOT_BOOT
/*  QUICKBOOT SCRATCH memory - cacheable, buffered, write back  */
#define QB_SCRATCH_BASE     QB_SCRATCH_ADDR
#define QB_SCRATCH_SIZE     2   /*  2MB     */
#define QB_SCRATCH_MEMORY   (MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE | \
		                            MMU_MEMORY_AP_READ_WRITE)
#endif

#define SCRATCH_BASE    SCRATCH_ADDR
#define SCRATCH_SIZE    (((TCC_MEM_SIZE*1024*1024)-(SCRATCH_ADDR-BASE_ADDR))/MB)
#define SCRATCH_MEMORY  (MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH /* MMU_MEMORY_TYPE_NORMAL_WRITE_BACK_NO_ALLOCATE */ | \
                         MMU_MEMORY_AP_READ_WRITE)

#if defined(TSBM_ENABLE)
#define OTP_BASE	0x40000000
#define OTP_SIZE	(0x00100000/MB)
#define OTP_MEMORY	(MMU_MEMORY_TYPE_STRONGLY_ORDERED/*MMU_MEMORY_TYPE_NORMAL_WRITE_THROUGH*/ | \
                         MMU_MEMORY_AP_READ_WRITE)
#endif




static mmu_section_t mmu_section_table[] = {
/*   Physical addr, Virtual addr , Size (in MB) ,   Flags */
 	{0x00000000   , 0x00000000   , SRAM_SIZE    , SRAM_MEMORY},
 	{SRAM_BASE    , SRAM_BASE    , SRAM_SIZE    , SRAM_MEMORY},
 	{IOMAP_BASE   , IOMAP_BASE   , IOMAP_SIZE   , IOMAP_MEMORY},
	{LK_BASE      , LK_BASE      , LK_SIZE      , LK_MEMORY},
#ifdef WITH_DMA_ZONE
	{DMA_BASE     , DMA_BASE     , DMA_SIZE     , DMA_MEMORY},
#endif
#ifdef CONFIG_SNAPSHOT_BOOT
	{QB_SCRATCH_BASE , QB_SCRATCH_BASE , QB_SCRATCH_SIZE , QB_SCRATCH_MEMORY},
#endif
	{SCRATCH_BASE , SCRATCH_BASE , SCRATCH_SIZE , SCRATCH_MEMORY},
#if defined(TSBM_ENABLE)
	{OTP_BASE, OTP_BASE, OTP_SIZE, OTP_MEMORY},
#endif

};

void platform_early_init(void)
{
	system_rev = HW_REV;
	clock_init_early();
	gpio_init_early();
	platform_init_interrupts();
	platform_init_timer();
	uart_init_early();
	i2c_init_early();
}

void platform_init(void)
{
	dprintf(INFO, "platform_init()\n");

	i2c_init();
	clock_init();

#if defined(TCC_PCA953X_USE)
	pca953x_init();
#endif

	uart_init();

#ifdef TCC_CHIP_REV
	tcc_extract_chip_revision();
#endif//TCC_CHIP_REV
}

void platform_uninit(void)
{
	platform_uninit_timer();
}

int platform_use_identity_mmu_mappings(void)
{
	/* Use only the mappings specified in this file. */
	return 1;
}

/* Setup memory for this platform */
void platform_init_mmu_mappings(void)
{
	uint32_t i;
	uint32_t sections;
	uint32_t table_size = ARRAY_SIZE(mmu_section_table);

 	/* Configure the MMU page entries for memory read from the
	   mmu_section_table */
	for (i = 0; i < table_size; i++) {
		sections = mmu_section_table[i].num_of_sections;

		while (sections--) {
			arm_mmu_map_section(mmu_section_table[i].paddress +
					    sections * MB,
					    mmu_section_table[i].vaddress +
					    sections * MB,
					    mmu_section_table[i].flags);
		}
	}
}

/* Initialize 32bit timer */
static void platform_init_timer(void)
{
	ticks_per_sec = tca_ckc_getperi(PERI_TCZ) * 100;
}

/* Returns timer ticks per sec */
uint32_t platform_tick_rate(void)
{
	return ticks_per_sec;
}
