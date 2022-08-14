/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
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
 *
 */

#include <app.h>
#include <debug.h>
#include <arch/arm.h>
#include <dev/udc.h>
#include <string.h>
#include <kernel/thread.h>
#include <arch/ops.h>
#include <platform.h>

#include <dev/flash.h>
#include <lib/ptable.h>
#include <dev/keys.h>
#include <dev/fbcon.h>

#if defined(PLATFORM_TCC892X)
#include <arch/tcc_used_mem_tcc892x.h>
#endif
#include <arch/tcc_used_mem.h>      // add by Telechips
#include <platform/reg_physical.h>
#include <platform/globals.h>
#if defined(PLATFORM_TCC892X)
#include <platform/structures_smu_pmu.h>
#endif

#include <i2c.h>
//#include <dev/pmic.h>
#if defined(AXP192_PMIC)
#include <dev/axp192.h>
#endif
#if defined(AXP202_PMIC)
#include <dev/axp202.h>
#endif
#if defined(RN5T614_PMIC)
#include <dev/rn5t614.h>
#endif
#if defined(ACT8810_PMIC)
#include <dev/act8810.h>
#endif
#if defined(RT5027_PMIC)
#include <dev/rt5027.h>
#endif


#include <fwdn/Disk.h>
#include <sdmmc/sd_memory.h>
//#include <sdmmc/emmc.h>
#include <partition_parser.h>
#include <sfl.h>


/*========================================================
  = definition
  ========================================================*/
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
#define BYTE_TO_SECOTR(x) (x)/512
#define SWSUSP_SIG               "S1SUSPEND"
#define PAGE_SIZE                4096
#define MAP_PAGE_ENTRIES	    (PAGE_SIZE / sizeof(sector_t) - 1)
#define addr(x) (0xF0000000+x)



/*========================================================
  = Variable 
  ========================================================*/
extern unsigned int page_mask;
unsigned int scratch_addr = QB_SCRATCH_ADDR;

typedef uint64_t sector_t;
typedef uint64_t blkcnt_t;
typedef uint32_t u32;

struct swap_map_page {
     sector_t entries[MAP_PAGE_ENTRIES];
     sector_t next_swap;
 };

 struct swap_map_handle {
     struct swap_map_page *cur;
     sector_t cur_swap;
     sector_t first_sector;
     unsigned int k;
 };

typedef struct {
   unsigned long val;
} swp_entry_t;

#define BSIZE (sizeof(unsigned int) * 100)
#define TSIZE (sizeof(unsigned int) * 7)

struct swsusp_header {
	char reserved[PAGE_SIZE - sizeof(u32) - sizeof(sector_t) - sizeof(unsigned int)
				- sizeof(unsigned int)*64 - sizeof(char)*10*2];
	u32  master_checksum;
	sector_t image;		
	unsigned int flags;	/* Flags to pass to the "boot" kernel */	
	unsigned int reg[64];	
	char	orig_sig[10];
	char	sig[10];		
}__attribute__((packed));


struct new_utsname {
   char sysname[65];
   char nodename[65];
   char release[65];
   char version[65];
   char machine[65];
   char domainname[65];
};

/* page backup entry */
typedef struct pbe {
   unsigned long address;      /* address of the copy */
   unsigned long orig_address; /* original address of page */
   swp_entry_t swap_address;
   swp_entry_t dummy;      /* we need scratch space at
                    * end of page (see link, diskpage)
                    */
} suspend_pagedir_t;

struct swsusp_info {
    struct new_utsname  uts;
    uint32_t            version_code;
    unsigned long       num_physpages;
    int                 cpus;
    unsigned long       image_pages;
    unsigned long       pages;
    unsigned long       size;   
} __attribute__((aligned(PAGE_SIZE)));

static struct swsusp_info	context;
static unsigned int         buffers[100];
static unsigned long long   gstorage_read_addr;
static unsigned long long   gstorage_read_addr_min;
static unsigned long long   gstorage_read_addr_max;
static unsigned long long   gstorage_read_addr_size;
static unsigned long long   gstorage_page_max;

/*==========================================================================
  = function
  ===========================================================================*/

extern void tcc_mmu_switch(void);
extern void restore_coprocessor(void);

static struct swsusp_header    *gpSwsusp_header;
static struct swsusp_info      *gpSwsusp_info;
static struct swap_map_page    *gpSwap_map_page;
static uint32_t                *gpMeta_page_info;

static sector_t         gdata_read_offset_page_num;
static unsigned int		gdata_read_offset;
static unsigned int		gdata_write_offset;

static uint32_t         gswap_map_page_offset;
static unsigned int     gnr_meta_pages;
static unsigned int     gnr_copy_pages;

static unsigned int     gdebug_storage_read_size;

static PPMU pPMU;
#define TCCWDT_RESET_TIME       8 // sec unit
#define TCCWDT_KICK_TIME_DIV    4
static int tccwdt_reset_time = TCCWDT_RESET_TIME;

struct tcc_snapshot {
    unsigned char swsusp_header[2*PAGE_SIZE];
    unsigned char swsusp_info[2*PAGE_SIZE];
    unsigned char swap_map_page[2*PAGE_SIZE];

    unsigned int    buffers[100];
    unsigned int    ptable[4];
};

#define IO_OFFSET	0x00000000
#define io_p2v(pa)	((pa) - IO_OFFSET)
#define tcc_p2v(pa)         io_p2v(pa)

//=============================================================================
// memory map for snapshot image.
//=============================================================================
#define SNAPSHOT_FLASH_DATA_SIZE            0x00080000	// 512 kB(0x00080000), 256kB(0x00040000)
#define SNAPSHOT_UNCOMPRESS_DATA_SIZE       0x00040000	// 256 kB
#define SNAPSHOT_COMPRESS_DATA_SIZE         0x00040000	// 256 kB
#define SNAPSHOT_META_DATA_SIZE             0x00080000	// 512 kB
//#define SNAPSHOT_INFO_SIZE                  0x00040000	// 256 kB


static unsigned char *gpData;
static unsigned char *gpDataUnc;
static unsigned char *gpDataCmp;
static unsigned char *gpDataMeta;
static struct tcc_snapshot  *gpSnapshot;


/* We need to remember how much compressed data we need to read. */
#define LZ4_HEADER	sizeof(uint32_t) * 2

/* Number of pages/bytes we'll compress at one time. */
#define LZ4_UNC_PAGES	32
#define LZ4_UNC_SIZE	(LZ4_UNC_PAGES * PAGE_SIZE)

static void storage_read_offset_page_num_inc() 
{
    gdata_read_offset_page_num++;
    if (gdata_read_offset_page_num > gstorage_page_max) {
        gdata_read_offset_page_num = 1;
    }
}

static void get_next_data_from_storage()
{
    uint32_t        index;
    unsigned int	data_offset;
    unsigned int	req_size;
    unsigned char  *dst, *src;

    dst = gpData;
    src = gpData + gdata_read_offset;
    req_size    = gdata_read_offset + SNAPSHOT_FLASH_DATA_SIZE - gdata_write_offset;
    data_offset = gdata_write_offset - gdata_read_offset;

    // copy remain data from tail to head.
    for(index = 0;index < data_offset/PAGE_SIZE;index++) {
        tcc_memcpy(dst, src);
        dst += PAGE_SIZE;
        src += PAGE_SIZE;
    }

    // check address ragne of snapshot partition.
    if (gstorage_read_addr >= gstorage_read_addr_max) {
        gstorage_read_addr = gstorage_read_addr_min + PAGE_SIZE;
    }

    if (gstorage_read_addr + req_size >= gstorage_read_addr_max) {
        req_size = gstorage_read_addr_max - gstorage_read_addr;
    }

    // read next data from storage.
    if (tcc_read(gstorage_read_addr, dst, req_size) != 0) {
        dprintf(CRITICAL, "Error: can not read data from storage...   gstorage_read_addr:%llu,  req_size:%u\n", gstorage_read_addr, req_size);
    }
    gdebug_storage_read_size += req_size;		// for debug

    // update flash offset
    gstorage_read_addr += req_size;

    gdata_read_offset = 0;
    gdata_write_offset = data_offset + req_size;
}

static void get_next_swap_page_info() 
{
    while(gpSwap_map_page->next_swap != gdata_read_offset_page_num) {
        gdata_read_offset += PAGE_SIZE;
        storage_read_offset_page_num_inc();

        if (gdata_write_offset <= gdata_read_offset) {
            get_next_data_from_storage();
        }
    }

    // find next pfn of swap map page info.
    tcc_memcpy(gpSnapshot->swap_map_page, gpData + gdata_read_offset);

    gdata_read_offset += PAGE_SIZE;
    storage_read_offset_page_num_inc();
    gswap_map_page_offset = 0;

}

static void get_next_swap_data(unsigned char *out, unsigned req_page) 
{
    uint32_t index;
    unsigned char *dst, *src;

    if (req_page >= (gdata_write_offset - gdata_read_offset)/PAGE_SIZE) {
        get_next_data_from_storage();
    }

    dst = out;
    src = gpData + gdata_read_offset;

    for(index = 0;index < req_page;index++) {
        // update swap map page
        if (gswap_map_page_offset >= MAP_PAGE_ENTRIES)
            get_next_swap_page_info();

        // copy data
        if(gpSwap_map_page->entries[gswap_map_page_offset] != gdata_read_offset_page_num) {
            while(gpSwap_map_page->entries[gswap_map_page_offset] != gdata_read_offset_page_num) {
                gdata_read_offset += PAGE_SIZE;
                storage_read_offset_page_num_inc();
                if (gdata_write_offset <= gdata_read_offset) {
                    get_next_data_from_storage();
                }
            }
        }

        // copy data
        //tcc_memcpy(dst, gpData + gdata_read_offset, PAGE_SIZE);
        tcc_memcpy(dst, gpData + gdata_read_offset);

        dst += PAGE_SIZE;
        gdata_read_offset += PAGE_SIZE;
        gswap_map_page_offset++;
        storage_read_offset_page_num_inc();
    }

}



#if defined(PLATFORM_TCC892X)
static void tcc_USB20HPHY_cfg(void)
{
	unsigned int temp;
	PHSIOBUSCFG pEHCIPHYCFG;
	
	pEHCIPHYCFG = (PHSIOBUSCFG)tcc_p2v(HwHSIOBUSCFG_BASE);
	
	/* A2X_USB20H: not use prefetch buffer all R/W
	 *             set A2X_USB20H in the bootloader
	 */
	//pEHCIPHYCFG->HSIO_CFG.nREG = 0xF000CFCF;
	pEHCIPHYCFG->HSIO_CFG.bREG.A2X_USB20H = 0x3;

	temp = 0x00000000;
	temp = temp | Hw29 | Hw28;		// [31:28] UTM_TXFSLSTUNE[3:0]
	temp = temp | Hw25 | Hw24;		// [26:24] UTM_SQRXTUNE[2:0]
	//temp = temp | Hw22; 			// [22:20] UTM_OTGTUNE[2:0] // OTG Reference Value [00]
	temp = temp | Hw21 | Hw20;		// tmp
	//temp = temp | Hw17 | Hw16;	// [18:16] UTM_COMPDISTUNE[2:0]
	temp = temp | Hw18;				// [18:16] UTM_COMPDISTUNE[2:0] // OTG Reference Value[111]
	//temp = temp | Hw13;				// [13] UTM_COMMONONN
	temp = temp | Hw11;				// [12:11] REFCLKSEL
	//temp = temp | Hw10;			// [10:9] REFCLKDIV
	//temp = temp | Hw8;			// [8] : SIDDQ
	temp = temp | Hw6;				// [6]
	temp = temp | Hw5;				// [5]
	temp = temp | Hw4;				// [4]
	temp = temp | Hw3;				// [3]
	temp = temp | Hw2;				// [2]

	pEHCIPHYCFG->USB20H_PCFG0.nREG = temp;

	temp = 0x00000000;
	temp = temp | Hw29;
	temp = temp | Hw28;				// [28] OTG Disable
	//temp = temp | Hw27;			// [27] IDPULLUP
	temp = temp | Hw19 | Hw18;		// [19:18]
	//temp = temp | Hw17;
	temp = temp | Hw16;
	temp = temp | Hw6 | Hw5;
	temp = temp | Hw0;				// [0] TP HS transmitter Pre-Emphasis Enable

	pEHCIPHYCFG->USB20H_PCFG1.nREG = temp;

	temp = 0x00000000;
	//temp = temp | Hw9;
	//temp = temp | Hw6;
	temp = temp | Hw15 | Hw5;

	pEHCIPHYCFG->USB20H_PCFG2.nREG = temp;


	thread_sleep(10);
	pEHCIPHYCFG->USB20H_PCFG1.nREG |= Hw31;
	thread_sleep(20);
}

static void tcc_ehci_phy_ctrl(int on)
{
	tca_ckc_setippwdn(PMU_ISOL_USBHP, !on);
}
#endif
static volatile unsigned int cnt = 0;
#define nop_delay(x) for(cnt=0 ; cnt<x ; cnt++){ \
					__asm__ __volatile__ ("nop\n"); }

static inline u32 checksum(u32 *addr, int len)
{
	u32 csum = 0;

	len /= sizeof(*addr);
	while (len-- > 0)
		csum ^= *addr++;
	csum = ((csum>>1) & 0x55555555)  ^  (csum & 0x55555555);

	return csum;
}

static int storage_load_snapshot_image()
{
    int                     error;
    unsigned                index, percent = 9999;
    unsigned                loop, max_copy_pages;
    static uint32_t         cmp_data_size;
    static uint32_t         unc_data_size;
    uint32_t                cmp_page_num;
    uint32_t                copy_page_num;
    static u32				saved_checksum_data;
    static u32				cal_checksum_data;
	u32		 				cal_master_checksum = 0;
    unsigned char          *dst, *src;
    static uint32_t         meta_page_offset;
    time_t                  time_start, time_prev, time_end;
    time_t                  time_lz4, time_memcpy, time_flash;



#if defined(PLATFORM_TCC892X)
	pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));

	/* remap to internal ROM */
	pPMU->PMU_CONFIG.bREG.REMAP = 0;
	
	pPMU->PMU_ISOL.nREG = 0x00000000;
	nop_delay(100000);
	pPMU->PWRUP_MBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_VBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_GBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_DBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_HSBUS.bREG.DATA = 0;
	nop_delay(100000);
	
	pPMU->PMU_SYSRST.bREG.VB = 1;
	pPMU->PMU_SYSRST.bREG.GB = 1;
	pPMU->PMU_SYSRST.bREG.DB = 1;
	pPMU->PMU_SYSRST.bREG.HSB = 1;
	nop_delay(100000);

	pPMU->PMU_WDTCTRL.bREG.EN = 0;
	pPMU->PMU_WDTCTRL.bREG.CNT = tccwdt_reset_time*200;	
	pPMU->PMU_WDTCTRL.bREG.EN = 1;
	
	dprintf(CRITICAL, "watchdog_time=%dsec, kick_time=%dsec\n", tccwdt_reset_time, tccwdt_reset_time/TCCWDT_KICK_TIME_DIV);		
#elif defined(PLATFORM_TCC893X)
   pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));

   pPMU->PMU_WDTCTRL.bREG.EN = 0;
   pPMU->PMU_WDTCTRL.bREG.CNT = tccwdt_reset_time*200;	
   pPMU->PMU_WDTCTRL.bREG.EN = 1;

   dprintf(CRITICAL, "Watchdog time=%dsec, kick_time=%dsec\n", tccwdt_reset_time, tccwdt_reset_time/TCCWDT_KICK_TIME_DIV);		
#endif

#if defined(PLATFORM_TCC892X) 
   {
      PUSB_EHCI pEHCI = (PUSB_EHCI)tcc_p2v(HwUSB20HOST_EHCI_BASE);

      tcc_ehci_phy_ctrl(1);
      tcc_USB20HPHY_cfg();
      pEHCI->USBCMD.nREG = 0x80002;
   }
#endif

    time_lz4 = time_memcpy = time_flash = 0;

    time_start = current_time();

    // copy swap map page info.
    tcc_memcpy(gpSnapshot->swap_map_page, gpData);

    storage_read_offset_page_num_inc();
    gdata_read_offset = PAGE_SIZE;

    // copy swsusp_info
    get_next_swap_data(gpSnapshot->swsusp_info, 1);

    gnr_meta_pages = gpSwsusp_info->pages - gpSwsusp_info->image_pages - 1;
    gnr_copy_pages = gpSwsusp_info->image_pages;
    copy_page_num = gnr_copy_pages;

	// load meta data 
    max_copy_pages = 0;


/*===================================================================
 *																	*
 *		 			Load MetaData from Storage						*
 *																	*
 *=================================================================*/
    while (max_copy_pages < gnr_meta_pages) {
        // get meta data
        get_next_swap_data(gpDataCmp, 1);

        // uncompress data
        cmp_data_size = *(uint32_t *)gpDataCmp;	// 4byte
        saved_checksum_data = *(u32 *)(gpDataCmp + 4) ; // 4byte

		cmp_page_num  = (LZ4_HEADER + cmp_data_size + PAGE_SIZE - 1) / PAGE_SIZE;
	
		get_next_swap_data(gpDataCmp + PAGE_SIZE, (cmp_page_num - 1));

		unc_data_size = LZ4_uncompress_unknownOutputSize(gpDataCmp + LZ4_HEADER,
			   (gpDataMeta + max_copy_pages * PAGE_SIZE), cmp_data_size, LZ4_UNC_SIZE);
    	if (unc_data_size <= 0 || unc_data_size > LZ4_UNC_SIZE) {
    		dprintf(CRITICAL, " 01. LZ4 Decompression failed. UNC_LEN:[%u] CMP_LEN:[0x%u]\n", unc_data_size, cmp_data_size);
            return (-1);
    	}

#ifdef CONFIG_SNAPSHOT_CHECKSUM
    	cal_checksum_data = checksum((gpDataMeta + max_copy_pages * PAGE_SIZE), unc_data_size);

       if( cal_checksum_data != saved_checksum_data ) {
        	dprintf(CRITICAL, " 01. LZ4 Decompression Checksum check failed. CAL:[%x] SAVED:[%x]\n", cal_checksum_data, saved_checksum_data);
        	hexdump((gpDataMeta + max_copy_pages * PAGE_SIZE), 512 );
        	return -1;
        }

	   /*	Master Checksum	*/
		cal_master_checksum = cal_master_checksum ^ cal_checksum_data;
#endif

		// copy swap page data.
        meta_page_offset = 0;
        max_copy_pages += (unc_data_size / PAGE_SIZE);
    }

    copy_page_num = copy_page_num - (max_copy_pages - gnr_meta_pages);
    
    for (index = gnr_meta_pages;index < max_copy_pages;index++) {
        tcc_memcpy(gpMeta_page_info[meta_page_offset++] * PAGE_SIZE, gpDataMeta + index * PAGE_SIZE);
    }


// Load snapshot image to memory
    dprintf(CRITICAL, "load_image_from_flash.... meta[%6u] copy[%6u]\n", gnr_meta_pages, gnr_copy_pages);


/*===================================================================
 *																	*
 *		 			Load Snapshot Data from Storage					*
 *																	*
 *=================================================================*/
    for (index = 0;index < copy_page_num;) {
        // get meta data
        get_next_swap_data(gpDataCmp, 1);

        // uncompress data
    	unc_data_size = LZ4_UNC_SIZE;
        cmp_data_size = *(uint32_t *)gpDataCmp;
        saved_checksum_data = *(u32 *)(gpDataCmp + 4) ;


        cmp_page_num  = (LZ4_HEADER + cmp_data_size + PAGE_SIZE - 1) / PAGE_SIZE;
     
		get_next_swap_data(gpDataCmp + PAGE_SIZE, (cmp_page_num - 1));

        time_prev = current_time();

		unc_data_size = LZ4_uncompress_unknownOutputSize(gpDataCmp + LZ4_HEADER,
				gpDataUnc, cmp_data_size, LZ4_UNC_SIZE);
	    if (unc_data_size <= 0 || unc_data_size > LZ4_UNC_SIZE) {
    		dprintf(CRITICAL, " 02. LZ4 Decompression failed. UNC_LEN:[%u] CMP_LEN:[%u]\n", unc_data_size, cmp_data_size);
            return (-1);
    	}

#ifdef CONFIG_SNAPSHOT_CHECKSUM
        cal_checksum_data = checksum(gpDataUnc, unc_data_size );

       if( cal_checksum_data != saved_checksum_data ) {
        	dprintf(CRITICAL, " 02. LZ4 Decompression Checksum check failed. CAL:[%x] SAVED:[%x]\n", cal_checksum_data, saved_checksum_data);
        	hexdump(gpDataUnc, 512 );
        	return -1;
        }

	   /*	Master Checksum	*/
		cal_master_checksum = cal_master_checksum ^ cal_checksum_data;
#endif

        time_lz4 += (current_time() - time_prev);
        time_prev = current_time();

        max_copy_pages = unc_data_size / PAGE_SIZE;

        src = gpDataUnc;
        for (loop = 0;loop < max_copy_pages;loop++) {
            dst = gpMeta_page_info[meta_page_offset++] * PAGE_SIZE;
            //memcpy(dst, src, PAGE_SIZE);
            tcc_memcpy(dst, src);
            src += PAGE_SIZE;
        }

        time_memcpy += (current_time() - time_prev);

        index += max_copy_pages;

		if ((index * 100 / copy_page_num) != percent) 
			dprintf(CRITICAL, " load %3d % \r", percent = (index * 100 / copy_page_num));
#if defined(PLATFORM_TCC892X)
        pPMU->PMU_WDTCTRL.bREG.CLR = 1; // Clear watchdog
#elif defined(PLATFORM_TCC893X)
        pPMU->PMU_WDTCTRL.bREG.CLR = 1; // Clear watchdog
#endif           
    }

    time_end = current_time();
    time_flash = time_end - time_start - time_memcpy - time_lz4;

#ifdef CONFIG_SNAPSHOT_CHECKSUM
	/*		Check Master Checksum	*/
	if (cal_master_checksum != gpSwsusp_header->master_checksum) {
		dprintf(CRITICAL, "Error : Master_Checksum is invalid. Header : [0x%08x] Cal : [0x%08x]\n", gpSwsusp_header->master_checksum, cal_master_checksum);
		return (-1);
	}
#endif

	dprintf(CRITICAL, "\n");
	dprintf(CRITICAL, " load Done. \n");
    dprintf(CRITICAL, "index [%d] copy_page_num[%u][%uMByte] unc_data_size %lu \n", index, copy_page_num, copy_page_num*4/1024, unc_data_size);
    dprintf(CRITICAL, "load_image_from_storage.... meta[%6u] copy[%6u] index[%u]\n", gnr_meta_pages, gnr_copy_pages, index);
    dprintf(CRITICAL, "time(ms)... totoal[%4lu]  memcpy[%4lu] lz4[%4lu] read snapshot image[%4lu][%uMBps] device_init[%4lu]\n", current_time() - time_start, time_memcpy, time_lz4, time_flash, gdebug_storage_read_size/1024/time_flash, time_start);

#if defined(PLATFORM_TCC892X)
    pPMU->PMU_WDTCTRL.bREG.CLR = 1; // Clear watchdog
#elif defined(PLATFORM_TCC893X)
    pPMU->PMU_WDTCTRL.bREG.CLR = 1; // Clear watchdog
#endif           

#if defined(PLATFORM_TCC892X)
    pPMU->PMU_WDTCTRL.bREG.EN = 0; // Disable watchdog
#elif defined(PLATFORM_TCC893X)
    pPMU->PMU_WDTCTRL.bREG.EN = 0; // Disable watchdog
#endif           
    return 0;
}

static int snapshot_boot()
{
    int ret = 0;
    time_t start_time;

    dprintf(CRITICAL, "snapshot_boot...\n");

	/*              Enable Cache for QuickBoot.             */
	arch_enable_cache(UCACHE);


    start_time = current_time();

// Initialize variable
    // flash read buffer
    gdata_read_offset  = 0;
    gdata_write_offset = 0;
    gdata_read_offset_page_num = gpSwsusp_header->image;

    // swap map page
    gswap_map_page_offset = 0;

    // swsusp info
    gnr_copy_pages = 0;
    gnr_meta_pages = 0;

    gpSwsusp_info    = (struct swsusp_info   *)gpSnapshot->swsusp_info;
    gpSwap_map_page  = (struct swap_map_page *)gpSnapshot->swap_map_page;
    gpMeta_page_info = gpDataMeta;

    // read data from flash
    get_next_data_from_storage();


    // Load Image
    ret = storage_load_snapshot_image();

    dprintf(CRITICAL, "load_image_from_storage done... [%lu]   0x%p, 0x%p, 0x%p\n", current_time() - start_time, gpData, gpDataUnc, gpDataCmp);
    dprintf(CRITICAL, "read data size from storage [%6uMByte] \n", gdebug_storage_read_size/1024/1024);		// for debug

    return ret;
}


static int storage_check_sig(void)
{
	return !memcmp(SWSUSP_SIG, gpSwsusp_header->sig, 10);
}

extern int jump_to_kernel(unsigned int start, unsigned int arch);
static int storage_snapshot_jump_to_resume(unsigned int *start)
{
#if defined(PLATFORM_TCC892X)
	pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));

	/* remap to internal ROM */
	pPMU->PMU_CONFIG.bREG.REMAP = 0;
	
	pPMU->PMU_ISOL.nREG = 0x00000000;
	nop_delay(100000);
	pPMU->PWRUP_MBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_VBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_GBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_DBUS.bREG.DATA = 0;
	nop_delay(100000);
	pPMU->PWRUP_HSBUS.bREG.DATA = 0;
	nop_delay(100000);
	
	pPMU->PMU_SYSRST.bREG.VB = 1;
	pPMU->PMU_SYSRST.bREG.GB = 1;
	pPMU->PMU_SYSRST.bREG.DB = 1;
	pPMU->PMU_SYSRST.bREG.HSB = 1;
	nop_delay(100000);

	pPMU->PMU_WDTCTRL.bREG.EN = 0;
	pPMU->PMU_WDTCTRL.bREG.CNT = tccwdt_reset_time*200;	
	pPMU->PMU_WDTCTRL.bREG.EN = 1;
	
	dprintf(CRITICAL, "Watchdog time=%dsec, kick_time=%dsec\n", tccwdt_reset_time, tccwdt_reset_time/TCCWDT_KICK_TIME_DIV);		
#elif defined(PLATFORM_TCC893X)
   pPMU = (volatile PPMU)(tcc_p2v(HwPMU_BASE));

//   __asm__ __volatile__("": : :"memory");	// It prevent Stopping this SYSTEM.
//   thread_sleep(10);

   /* remap to internal ROM */
   pPMU->PMU_CONFIG.bREG.REMAP = 0;

   /* PMU is not initialized with WATCHDOG */
   pPMU->PMU_ISOL.nREG = 0x00000000;
   nop_delay(100000);
   pPMU->PWRUP_MBUS.bREG.DATA = 0;
   nop_delay(100000);
   pPMU->PWRUP_VBUS.bREG.DATA = 0;
   nop_delay(100000);
   pPMU->PWRUP_GBUS.bREG.DATA = 0;
   nop_delay(100000);
   pPMU->PWRUP_DBUS.bREG.DATA = 0;
   nop_delay(100000);
   pPMU->PWRUP_HSBUS.bREG.DATA = 0;
   nop_delay(100000);

   pPMU->PMU_SYSRST.bREG.VB = 1;
   pPMU->PMU_SYSRST.bREG.GB = 1;
   pPMU->PMU_SYSRST.bREG.DB = 1;
   pPMU->PMU_SYSRST.bREG.HSB = 1;
   nop_delay(100000);

	pPMU->PMU_WDTCTRL.bREG.EN = 0;
	pPMU->PMU_WDTCTRL.bREG.CNT = tccwdt_reset_time*200;	
	pPMU->PMU_WDTCTRL.bREG.EN = 1;

	dprintf(CRITICAL, "Watchdog time=%dsec, kick_time=%dsec\n", tccwdt_reset_time, tccwdt_reset_time/TCCWDT_KICK_TIME_DIV);		
#endif

   enter_critical_section();
   platform_uninit_timer();
   arch_disable_cache(UCACHE);
#ifdef CONFIG_CACHE_PL310
//   pl310_cache_clean_all();
//   pl310_cache_flush_all();
#endif
   arch_disable_mmu();

	dprintf(CRITICAL, "Jump to libsnapshot.\n");		

#if defined(PLATFORM_TCC892X)
   return jump_to_kernel(buffers, 0x02);
#elif defined(PLATFORM_TCC893X)
   return jump_to_kernel(buffers, 0x04);
#endif
}


/*==============================================================
  = Snapshot Boot
  =============================================================*/
int snapshot_boot_from_storage()
{
	char *ptn_name = "snapshot";
	unsigned int ptn_index;
    gdebug_storage_read_size = 0;		// for debug
  
	gpData     = (unsigned char *)(scratch_addr);
    gpDataUnc  = (unsigned char *)(gpData + SNAPSHOT_FLASH_DATA_SIZE);
    gpDataCmp  = (unsigned char *)(gpDataUnc + SNAPSHOT_UNCOMPRESS_DATA_SIZE);
    gpDataMeta = (unsigned char *)(gpDataCmp + SNAPSHOT_COMPRESS_DATA_SIZE);
    gpSnapshot = (struct tcc_snapshot *)(gpDataMeta + SNAPSHOT_META_DATA_SIZE);


	/*		Get Snapshot Partition Info.	*/
	ptn_index = partition_get_index(ptn_name);
    gstorage_read_addr_min = partition_get_offset(ptn_index);
    gstorage_read_addr_size = partition_get_size(ptn_index);
    if (gstorage_read_addr_min == 0 || gstorage_read_addr_size == 0) {
        dprintf(CRITICAL, "ERROR: swap partition table not found\n");
        return -1;
    }

    gstorage_page_max = gstorage_read_addr_size / PAGE_SIZE - 1;

    gstorage_read_addr = gstorage_read_addr_min;
    gstorage_read_addr_max = gstorage_read_addr_min + gstorage_read_addr_size;

    /* read snapshot header */
    gpSwsusp_header  = (struct swsusp_header *)gpSnapshot->swsusp_header;
    if (tcc_read(gstorage_read_addr, gpSwsusp_header, PAGE_SIZE) != 0) {
        dprintf(INFO, "ERROR: snapshot header read fail...\n");
        return -1;
    }

    if (!storage_check_sig()) {
		char error_sig[11];
		memset(error_sig, 0, 11);
		memcpy(error_sig, gpSwsusp_header->sig, 10);
        dprintf(CRITICAL, "ERROR: snapshot header sig compare fail. [%s]\n", error_sig);
        return -1;
    } else {
		/*      if it is TCC892X_STB, make clock higher than Normal Booting.    */
#if defined(BOARD_TCC892X_STB_DEMO)
		clock_init_qb();
#endif

		gstorage_read_addr += (gpSwsusp_header->image * PAGE_SIZE);

		/* get coprocessor data */
		memcpy(buffers, gpSwsusp_header->reg, sizeof(buffers));

		/* kernel page restore */
		if (snapshot_boot() == 0) {
			/* recover kernel context */
			return storage_snapshot_jump_to_resume(buffers);
		}
		else {
			dprintf(CRITICAL, "ERROR: snapshot boot fail.\n");
		}
	}

	return 0;
}

