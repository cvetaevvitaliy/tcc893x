/*
 * Copyright (c) 2009, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of The Linux Foundation nor
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
#include <stdlib.h>
#include <kernel/thread.h>
#include <arch/ops.h>

#include <dev/flash.h>
#include <lib/ptable.h>
#include <dev/keys.h>
#include <dev/fbcon.h>
#include <baseband.h>
#include <target.h>
#include <mmc.h>
#include <partition_parser.h>
#include <platform.h>
#include <crypto_hash.h>
#include <malloc.h>
#include <boot_stats.h>

#include <platform/gpio.h>
#include <dev/gpio.h>

#include "image_verify.h"
#include "recovery.h"
#include "bootimg.h"
#include "fastboot.h"
#include "sparse_format.h"
#include "mmc.h"
#include "devinfo.h"
#include "board.h"

#include "scm.h"
#include "sfl.h"

#if defined(TSBM_ENABLE)
#include <tcsb/tcsb_api.h>
extern unsigned int get_tcc_mem_freq(void);
#endif

#if defined(RECOVERY_BY_REMOTE_KEY)
#include <tcc_remocon.h>
#endif

extern  bool target_use_signed_kernel(void);
extern void dsb();
extern void isb();
extern void platform_uninit(void);
extern void target_cmdline_serialno(char *cmdline);
extern void target_cmdline_wifimac(char *cmdline);
extern void target_cmdline_btaddr(char *cmdline);
extern void target_cmdline_bootmode(char *cmdline);
extern void target_cmdline_memtype(char *cmdline);
extern void target_cmdline_qb_prebuilt_menu(char *cmdline);
extern unsigned* target_atag_revision(unsigned* ptr);
extern unsigned* target_atag_is_camera_enable(unsigned* ptr);
extern unsigned* target_atag_chip_revision(unsigned* ptr);
extern unsigned *target_atag_panel(unsigned *ptr);
extern unsigned* target_atag_display(unsigned* ptr);

void write_device_info(device_info *dev);

#define EXPAND(NAME) #NAME
#define TARGET(NAME) EXPAND(NAME)

#ifdef MEMBASE
#define EMMC_BOOT_IMG_HEADER_ADDR (0xFF000+(MEMBASE))
#else
#define EMMC_BOOT_IMG_HEADER_ADDR 0xFF000
#endif

#define RECOVERY_MODE   0x77665502
#define FASTBOOT_MODE   0x77665500

unsigned boot_into_factory = 0;
unsigned boot_into_chrome = 0;
unsigned boot_into_qb_prebuilt = 0;

unsigned skip_loading_quickboot = 0;

static const char *emmc_cmdline = " androidboot.emmc=true";
static const char *usb_sn_cmdline = " androidboot.serialno=";
static const char *battchg_pause = " androidboot.mode=charger";
static const char *auth_kernel = " androidboot.authorized_kernel=true";

static const char *baseband_apq     = " androidboot.baseband=apq";
static const char *baseband_msm     = " androidboot.baseband=msm";
static const char *baseband_csfb    = " androidboot.baseband=csfb";
static const char *baseband_svlte2a = " androidboot.baseband=svlte2a";
static const char *baseband_mdm     = " androidboot.baseband=mdm";
static const char *baseband_sglte   = " androidboot.baseband=sglte";
static const char *baseband_dsda    = " androidboot.baseband=dsda";
static const char *baseband_dsda2   = " androidboot.baseband=dsda2";
static const char *baseband_sglte2  = " androidboot.baseband=sglte2";

/* Assuming unauthorized kernel image by default */
static int auth_kernel_img = 0;

static device_info device = {DEVICE_MAGIC, 0, 0};

static struct udc_device surf_udc_device = {
	.vendor_id	= 0x18d1,
	.product_id	= 0xD00D,
	.version_id	= 0x0100,
	.manufacturer	= "Google",
	.product	= "Android",
};

struct atag_ptbl_entry
{
	char name[16];
	unsigned offset;
	unsigned size;
	unsigned flags;
};

/*
 * Partition info, required to be published
 * for fastboot
 */
struct getvar_partition_info {
	const char part_name[MAX_GPT_NAME_SIZE]; /* Partition name */
	char getvar_size[MAX_GET_VAR_NAME_SIZE]; /* fastboot get var name for size */
	char getvar_type[MAX_GET_VAR_NAME_SIZE]; /* fastboot get var name for type */
	char size_response[MAX_RSP_SIZE];        /* fastboot response for size */
	char type_response[MAX_RSP_SIZE];        /* fastboot response for type */
};

/*
 * Right now, we are publishing the info for only
 * three partitions
 */
struct getvar_partition_info part_info[] =
{
	{ "system"  , "partition-size:", "partition-type:", "", "ext4" },
	{ "userdata", "partition-size:", "partition-type:", "", "ext4" },
	{ "cache"   , "partition-size:", "partition-type:", "", "ext4" },
};

char max_download_size[MAX_RSP_SIZE];
char sn_buf[13];

#ifdef CONFIG_SNAPSHOT_BOOT
#include "libsnapshot.h"
#endif

static void update_ker_tags_rdisk_addr(struct boot_img_hdr *hdr, unsigned int force)
{
	/* overwrite the destination of specified for the project */
   if(force) {
	hdr->kernel_addr = KERNEL_ADDR;
	hdr->ramdisk_addr = RAMDISK_ADDR;
	hdr->tags_addr = TAGS_ADDR;
   }
}

static void ptentry_to_tag(unsigned **ptr, struct ptentry *ptn)
{
	struct atag_ptbl_entry atag_ptn;

	memcpy(atag_ptn.name, ptn->name, 16);
	atag_ptn.name[15] = '\0';
	atag_ptn.offset = ptn->start;
	atag_ptn.size = ptn->length;
	atag_ptn.flags = ptn->flags;
	memcpy(*ptr, &atag_ptn, sizeof(struct atag_ptbl_entry));
	*ptr += sizeof(struct atag_ptbl_entry) / sizeof(unsigned);
}

unsigned char *update_cmdline(const char * cmdline)
{
	unsigned char *cmdline_final = NULL;

#if PLATFORM_TCC
	unsigned char *dst = (unsigned char*) malloc((4096 + 4) & (~3));
	cmdline_final = dst;

	if(cmdline[0])
		strcpy(cmdline_final, (char*)cmdline);
	else
		return (char *)cmdline;

	target_cmdline_serialno(cmdline_final);
	target_cmdline_wifimac(cmdline_final);
	target_cmdline_btaddr(cmdline_final);
	target_cmdline_bootmode(cmdline_final);
	target_cmdline_memtype(cmdline_final);
	target_cmdline_qb_prebuilt_menu(cmdline_final);
#else
	int cmdline_len = 0;
	int have_cmdline = 0;
	int pause_at_bootup = 0;

	if (cmdline && cmdline[0]) {
		cmdline_len = strlen(cmdline);
		have_cmdline = 1;
	}
	if (target_is_emmc_boot()) {
		cmdline_len += strlen(emmc_cmdline);
	}

	cmdline_len += strlen(usb_sn_cmdline);
	cmdline_len += strlen(sn_buf);

	if (target_pause_for_battery_charge()) {
		pause_at_bootup = 1;
		cmdline_len += strlen(battchg_pause);
	}

	if(target_use_signed_kernel() && auth_kernel_img) {
		cmdline_len += strlen(auth_kernel);
	}

	if (cmdline_len > 0) {
		const char *src;
		unsigned char *dst = (unsigned char*) malloc((cmdline_len + 4) & (~3));
		ASSERT(dst != NULL);

		/* Save start ptr for debug print */
		cmdline_final = dst;
		if (have_cmdline) {
			src = cmdline;
			while ((*dst++ = *src++));
		}
		if (target_is_emmc_boot()) {
			src = emmc_cmdline;
			if (have_cmdline) --dst;
			have_cmdline = 1;
			while ((*dst++ = *src++));
		}

		src = usb_sn_cmdline;
		if (have_cmdline) --dst;
		have_cmdline = 1;
		while ((*dst++ = *src++));
		src = sn_buf;
		if (have_cmdline) --dst;
		have_cmdline = 1;
		while ((*dst++ = *src++));

		if (pause_at_bootup) {
			src = battchg_pause;
			if (have_cmdline) --dst;
			while ((*dst++ = *src++));
		}

		if(target_use_signed_kernel() && auth_kernel_img) {
			src = auth_kernel;
			if (have_cmdline) --dst;
			while ((*dst++ = *src++));
		}
	}
#endif

	dprintf(INFO, "cmdline: %s\n", cmdline_final);

	return cmdline_final;
}

unsigned *atag_core(unsigned *ptr)
{
	/* CORE */
	*ptr++ = 2;
	*ptr++ = 0x54410001;

	return ptr;

}

unsigned *atag_ramdisk(unsigned *ptr, void *ramdisk,
							   unsigned ramdisk_size)
{
	if (ramdisk_size) {
		*ptr++ = 4;
		*ptr++ = 0x54420005;
		*ptr++ = (unsigned)ramdisk;
		*ptr++ = ramdisk_size;
	}

	return ptr;
}

unsigned *atag_ptable(unsigned **ptr_addr)
{
	int i;
	struct ptable *ptable;

	if ((ptable = flash_get_ptable()) && (ptable->count != 0)) {
		*(*ptr_addr)++ = 2 + (ptable->count * (sizeof(struct atag_ptbl_entry) /
							sizeof(unsigned)));
		*(*ptr_addr)++ = 0x4d534d70;
		for (i = 0; i < ptable->count; ++i)
			ptentry_to_tag(ptr_addr, ptable_get(ptable, i));
	}

	return (*ptr_addr);
}

unsigned *atag_cmdline(unsigned *ptr, const char *cmdline)
{
	int cmdline_length = 0;
	int n;
	char *dest;

	cmdline_length = strlen((const char*)cmdline);
	n = (cmdline_length + 4) & (~3);

	*ptr++ = (n / 4) + 2;
	*ptr++ = 0x54410009;
	dest = (char *) ptr;
	while ((*dest++ = *cmdline++));
	ptr += (n / 4);

	return ptr;
}

unsigned *atag_end(unsigned *ptr)
{
	/* END */
	*ptr++ = 0;
	*ptr++ = 0;

	return ptr;
}

void generate_atags(unsigned *ptr, const char *cmdline,
                    void *ramdisk, unsigned ramdisk_size)
{

	ptr = atag_core(ptr);
	ptr = atag_ramdisk(ptr, ramdisk, ramdisk_size);
	ptr = target_atag_mem(ptr);

	/* Skip MTD partition ATAGS for eMMC boot */
#if _NAND_MTD
	if (!target_is_emmc_boot()){
		ptr = atag_ptable(&ptr);
	}
#endif

	ptr = atag_cmdline(ptr, cmdline);

#if PLATFORM_TCC
	ptr = target_atag_revision(ptr);
	ptr = target_atag_is_camera_enable(ptr);
#if defined(TCC_CHIP_REV)
	ptr = target_atag_chip_revision(ptr);
#endif//defined(TCC_CHIP_REV)
	ptr = target_atag_panel(ptr);
	ptr = target_atag_display(ptr);
#endif

	ptr = atag_end(ptr);
}

typedef void entry_func_ptr(unsigned, unsigned, unsigned*);
void boot_linux(void *kernel, unsigned *tags,
		const char *cmdline, unsigned machtype,
		void *ramdisk, unsigned ramdisk_size)
{
	unsigned char *final_cmdline;

	void (*entry)(unsigned, unsigned, unsigned*) = (entry_func_ptr*)(PA((addr_t)kernel));
	uint32_t tags_phys = PA((addr_t)tags);

	ramdisk = PA(ramdisk);

	final_cmdline = update_cmdline((const char*)cmdline);

	/* Generating the Atags */
	generate_atags(tags, final_cmdline, ramdisk, ramdisk_size);

	dprintf(INFO, "booting linux @ %p, ramdisk @ %p (%d), tags/device tree @ %p\n",
		entry, ramdisk, ramdisk_size, tags_phys);

	enter_critical_section();

	/* do any platform specific cleanup before kernel entry */
	platform_uninit();

#if defined(PLATFORM_TCC892X) || defined(PLATFORM_TCC893X)
#if defined(DEFAULT_DISPLAY_LCD)
        #if defined(TARGET_M805_892X_EVM) || defined(TARGET_M805_893X_EVM) || (HW_REV == 0x1006) || (HW_REV == 0x1008) || defined(CONFIG_CHIP_TCC8925S)
                #define LCDC_NUM        0
        #else
                #define LCDC_NUM        1
        #endif
#elif defined(TARGET_TCC8920ST_EVM)
        #define LCDC_NUM        1
#elif defined(TARGET_TCC8930ST_EVM)
        #define LCDC_NUM        0
#else
        #define LCDC_NUM        1
#endif

        DEV_LCDC_Wait_signal(LCDC_NUM);
#endif

	arch_disable_cache(UCACHE);

#if ARM_WITH_MMU
	arch_disable_mmu();
#endif
	bs_set_timestamp(BS_KERNEL_ENTRY);
	entry(0, machtype, (unsigned*)tags_phys);
}

unsigned page_size = 0;
unsigned page_mask = 0;

#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))

BUF_DMA_ALIGN(buf, 4096); //Equal to max-supported pagesize

int boot_linux_from_storage(void)
{

    struct boot_img_hdr *hdr = (void*)buf;

    if(target_use_signed_kernel()) {
	if(boot_linux_secure(hdr)) 
	    return -1;
    }
    else {
	if(boot_linux_none_secure(hdr))
	    return -1;
    }


unified_boot:

    boot_linux((void *)hdr->kernel_addr, (void *)hdr->tags_addr,
	    (const char *)hdr->cmdline, board_machtype(),
	    (void *)hdr->ramdisk_addr, hdr->ramdisk_size);

    return 0;

}

int boot_linux_secure(struct boot_img_hdr *hdr)
{

#if defined(TSBM_ENABLE)

    struct sboot_img_hdr *shdr = SCRATCH_ADDR;
    unsigned int otp_clk = get_tcc_mem_freq() / 2; // 1:2 mode

    unsigned sheader_offset=0, offset = 0;
    unsigned long long ptn = 0;
    int index = INVALID_PTN;

    unsigned char *image_addr = 0;
    unsigned kernel_actual;
    unsigned ramdisk_actual;
    unsigned second_actual = 0;

    if (!boot_into_recovery) {

	if(boot_into_chrome)
	    index = partition_get_index("KERN-A");
	else
	    index = partition_get_index("boot");

	ptn = partition_get_offset(index);
	if(ptn == 0) {
	    dprintf(CRITICAL, "ERROR: No boot partition found\n");
	    return -1;
	}
    }
    else {
	index = partition_get_index("recovery");
	ptn = partition_get_offset(index);
	if(ptn == 0) {
	    dprintf(CRITICAL, "ERROR: No recovery partition found\n");
	    return -1;
	}
    }

    if (tcc_read(ptn + offset, (unsigned int *)shdr, page_size)) {
	dprintf(CRITICAL, "ERROR: Cannot read boot image header\n");
	return -1;
    }

    if(memcmp(shdr->magic, SBOOT_MAGIC, SBOOT_MAGIC_SIZE)){
	dprintf(CRITICAL ,"ERROR : Invaild Secure boot image header !! \n");
	return -1;
    }

    sheader_offset = sizeof(struct sboot_img_hdr);
    memset(SKERNEL_ADDR , 0x0, shdr->image_size);
    
    if(tcc_read(ptn+sheader_offset, SKERNEL_ADDR , shdr->image_size)){
	dprintf(CRITICAL, "ERROR: Cannot read secure boot image\n");
	return -1;
    }

    if(tcsb_api_check_secureboot(otp_clk, NULL , NULL) == 0){
	dprintf(CRITICAL, "ERROR : Do Not Support Secure Boot \n");
	return -1;
    }else{
	if(tcsb_api_decrypt(SKERNEL_ADDR, SCRATCH_ADDR, shdr->image_size) == NULL){
	    dprintf(CRITICAL, "ERROR : Secure Boot Kernel Image Verify Failed !!\n");
	    return -1;
	}

	memcpy(buf, SCRATCH_ADDR, page_size);
    }

    if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
	dprintf(CRITICAL, "ERROR: Invalid boot image header\n");
	return -1;
    }

    if (hdr->page_size && (hdr->page_size != page_size)) {
	page_size = hdr->page_size;
	page_mask = page_size - 1;
    }

    /*
     * Update the kernel/ramdisk/tags address if the boot image header
     * has default values, these default values come from mkbootimg when
     * the boot image is flashed using fastboot flash:raw
     */
    update_ker_tags_rdisk_addr(hdr, 0);

    /* Get virtual addresses since the hdr saves physical addresses. */
    hdr->kernel_addr = VA((addr_t)(hdr->kernel_addr));
    hdr->ramdisk_addr = VA((addr_t)(hdr->ramdisk_addr));
    hdr->tags_addr = VA((addr_t)(hdr->tags_addr));

    kernel_actual  = ROUND_TO_PAGE(hdr->kernel_size,  page_mask);
    ramdisk_actual = ROUND_TO_PAGE(hdr->ramdisk_size, page_mask);
    second_actual  = ROUND_TO_PAGE(hdr->second_size,  page_mask);

    dprintf(INFO, "Loading boot image (%d): start\n",
	    kernel_actual + ramdisk_actual);
    bs_set_timestamp(BS_KERNEL_LOAD_START);

    offset = page_size;

    /* Load kernel */
    //if (tcc_read(ptn + offset, (void *)hdr->kernel_addr, kernel_actual)) {
    if (!memcpy((void*)hdr->kernel_addr, SCRATCH_ADDR + offset, kernel_actual)) {
	dprintf(CRITICAL, "ERROR: Cannot read kernel image\n");
	return -1;
    }
    offset += kernel_actual;

    if (!boot_into_chrome){
	/* Load ramdisk */
	if(ramdisk_actual != 0)
	{
	    if (!memcpy((void*)hdr->ramdisk_addr, SCRATCH_ADDR + offset, ramdisk_actual)) {
		dprintf(CRITICAL, "ERROR: Cannot read ramdisk image\n");
		return -1;
	    }
	}
    }
    offset += ramdisk_actual;

    dprintf(INFO, "Loading boot image (%d): done\n",
	    kernel_actual + ramdisk_actual);
    bs_set_timestamp(BS_KERNEL_LOAD_DONE);

    if(hdr->second_size != 0) {
	offset += second_actual;
	/* Second image loading not implemented. */
	ASSERT(0);
    }

#endif

    return 0;

}

int boot_linux_none_secure(struct boot_img_hdr *hdr)
{
    unsigned offset = 0;
    unsigned long long ptn = 0;
    int index = INVALID_PTN;

    unsigned char *image_addr = 0;
	unsigned kernel_actual;
	unsigned ramdisk_actual;
	unsigned second_actual = 0;

	if (!boot_into_recovery) {

		if(boot_into_chrome)
			index = partition_get_index("KERN-A");
		else
			index = partition_get_index("boot");

		ptn = partition_get_offset(index);
		if(ptn == 0) {
			dprintf(CRITICAL, "ERROR: No boot partition found\n");
                    return -1;
		}
	}
	else {
		index = partition_get_index("recovery");
		ptn = partition_get_offset(index);
		if(ptn == 0) {
			dprintf(CRITICAL, "ERROR: No recovery partition found\n");
                    return -1;
		}
	}

	if (tcc_read(ptn + offset, (unsigned int *) buf, page_size)) {
		dprintf(CRITICAL, "ERROR: Cannot read boot image header\n");
                return -1;
	}

	if (memcmp(hdr->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
		dprintf(CRITICAL, "ERROR: Invalid boot image header\n");
                return -1;
	}

	if (hdr->page_size && (hdr->page_size != page_size)) {
		page_size = hdr->page_size;
		page_mask = page_size - 1;
	}

	/*
	 * Update the kernel/ramdisk/tags address if the boot image header
	 * has default values, these default values come from mkbootimg when
	 * the boot image is flashed using fastboot flash:raw
	 */
	update_ker_tags_rdisk_addr(hdr, 0);

	/* Get virtual addresses since the hdr saves physical addresses. */
	hdr->kernel_addr = VA((addr_t)(hdr->kernel_addr));
	hdr->ramdisk_addr = VA((addr_t)(hdr->ramdisk_addr));
	hdr->tags_addr = VA((addr_t)(hdr->tags_addr));

	kernel_actual  = ROUND_TO_PAGE(hdr->kernel_size,  page_mask);
	ramdisk_actual = ROUND_TO_PAGE(hdr->ramdisk_size, page_mask);
	second_actual  = ROUND_TO_PAGE(hdr->second_size,  page_mask);

	dprintf(INFO, "Loading boot image (%d): start\n",
		kernel_actual + ramdisk_actual);
	bs_set_timestamp(BS_KERNEL_LOAD_START);

	offset = page_size;

	/* Load kernel */
	if (tcc_read(ptn + offset, (void *)hdr->kernel_addr, kernel_actual)) {
	    dprintf(CRITICAL, "ERROR: Cannot read kernel image\n");
	    return -1;
	}
	offset += kernel_actual;

	if (!boot_into_chrome){
	    /* Load ramdisk */
	    if(ramdisk_actual != 0)
	    {
		if (tcc_read(ptn + offset, (void *)hdr->ramdisk_addr, ramdisk_actual)) {
		    dprintf(CRITICAL, "ERROR: Cannot read ramdisk image\n");
		    return -1;
		}
	    }
	}
	offset += ramdisk_actual;

	dprintf(INFO, "Loading boot image (%d): done\n",
		kernel_actual + ramdisk_actual);
	bs_set_timestamp(BS_KERNEL_LOAD_DONE);

	if(hdr->second_size != 0) {
	    offset += second_actual;
	    /* Second image loading not implemented. */
	    ASSERT(0);
	}

	return 0;

}

BUF_DMA_ALIGN(info_buf, 4096);
void write_device_info(device_info *dev)
{
	struct device_info *info = (void*) info_buf;
	unsigned long long ptn = 0;
	unsigned long long size;
	int index = INVALID_PTN;

	index = partition_get_index("aboot");
	ptn = partition_get_offset(index);
	if(ptn == 0)
		return;

	size = partition_get_size(index);

	memcpy(info, dev, sizeof(device_info));

	if(tcc_write(NULL, (ptn + size - 512), 512, (void *)info_buf))
	{
		dprintf(CRITICAL, "ERROR: Cannot write device info\n");
		return;
	}
}

void read_device_info(device_info *dev)
{
	struct device_info *info = (void*) info_buf;
	unsigned long long ptn = 0;
	unsigned long long size;
	int index = INVALID_PTN;

	index = partition_get_index("aboot");
	ptn = partition_get_offset(index);
	if(ptn == 0)
		return;

	size = partition_get_size(index);

	if(tcc_read((ptn + size - 512), (void *)info_buf, 512))
	{
		dprintf(CRITICAL, "ERROR: Cannot read device info\n");
		return;
	}

	if (memcmp(info->magic, DEVICE_MAGIC, DEVICE_MAGIC_SIZE))
	{
		memcpy(info->magic, DEVICE_MAGIC, DEVICE_MAGIC_SIZE);
		info->is_unlocked = 0;
		info->is_tampered = 0;

		write_device_info(info);
	}
	memcpy(dev, info, sizeof(device_info));
}

void reset_device_info()
{
	dprintf(ALWAYS, "reset_device_info called.");
	device.is_tampered = 0;
	write_device_info(&device);
}

void set_device_root()
{
	dprintf(ALWAYS, "set_device_root called.");
	device.is_tampered = 1;
	write_device_info(&device);
}


void cmd_boot(const char *arg, void *data, unsigned sz)
{
	unsigned kernel_actual;
	unsigned ramdisk_actual;
	struct boot_img_hdr hdr;
	char *ptr = ((char*) data);
	int ret = 0;
	uint8_t dtb_copied = 0;

	if (sz < sizeof(hdr)) {
		fastboot_fail("invalid bootimage header");
		return;
	}

	memcpy(&hdr, data, sizeof(struct boot_img_hdr));

	/* ensure commandline is terminated */
	hdr.cmdline[BOOT_ARGS_SIZE-1] = 0;

	if(target_is_emmc_boot() && hdr.page_size) {
		page_size = hdr.page_size;
		page_mask = page_size - 1;
	}

	kernel_actual = ROUND_TO_PAGE(hdr.kernel_size, page_mask);
	ramdisk_actual = ROUND_TO_PAGE(hdr.ramdisk_size, page_mask);

	/*
	 * Update the kernel/ramdisk/tags address if the boot image header
	 * has default values, these default values come from mkbootimg when
	 * the boot image is flashed using fastboot flash:raw
	 */
	update_ker_tags_rdisk_addr(&hdr, 1);

	/* Get virtual addresses since the hdr saves physical addresses. */
	hdr.kernel_addr = VA(hdr.kernel_addr);
	hdr.ramdisk_addr = VA(hdr.ramdisk_addr);
	hdr.tags_addr = VA(hdr.tags_addr);

	/* sz should have atleast raw boot image */
	if (page_size + kernel_actual + ramdisk_actual > sz) {
		fastboot_fail("incomplete bootimage");
		return;
	}

	/* Load ramdisk & kernel */
	memmove((void*) hdr.ramdisk_addr, ptr + page_size + kernel_actual, hdr.ramdisk_size);
	memmove((void*) hdr.kernel_addr, ptr + page_size, hdr.kernel_size);

	fastboot_okay("");
	udc_stop();

	boot_linux((void*) hdr.kernel_addr, (void*) hdr.tags_addr,
		   (const char*) hdr.cmdline, board_machtype(),
		   (void*) hdr.ramdisk_addr, hdr.ramdisk_size);
}

void cmd_erase(const char *arg, void *data, unsigned sz)
{
	unsigned long long ptn = 0;
	int index = INVALID_PTN;

	index = partition_get_index(arg);
	ptn = partition_get_offset(index);

	if(ptn == 0) {
		fastboot_fail("Partition table doesn't exist\n");
		return;
	}

	if(target_is_emmc_boot()){

		if (tcc_write("fastboot_erase", ptn, partition_get_size(index)>>9, NULL)) {
			fastboot_fail("failed to erase partition");
			return;
		}
	}
	else{
		if(flash_erase(arg)){
			fastboot_fail("failed to erase partition");
			return ;
		}
	}


	fastboot_okay("");
}


void cmd_flash_img(const char *arg, void *data, unsigned sz)
{
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	int index = INVALID_PTN;

	if (!strcmp(arg, "partition"))
	{
		dprintf(INFO, "Attempt to write partition image.\n");
		if (write_partition(sz, (unsigned char *) data)) {
			fastboot_fail("failed to write partition");
			return;
		}
	}
	else
	{
		index = partition_get_index(arg);
		ptn = partition_get_offset(index);
		if(ptn == 0) {
			fastboot_fail("partition table doesn't exist");
			return;
		}

		if (!strcmp(arg, "boot") || !strcmp(arg, "recovery")) {
			if (memcmp((void *)data, BOOT_MAGIC, BOOT_MAGIC_SIZE)) {
				fastboot_fail("image is not a boot image");
				return;
			}
		}

		size = partition_get_size(index);
		if (ROUND_TO_PAGE(sz,511) > size) {
			fastboot_fail("size too large");
			return;
		}
		else if (tcc_write(NULL, ptn , sz, (unsigned int *)data)) {
			fastboot_fail("flash write failure");
			return;
		}
	}
	fastboot_okay("");
	return;
}

void cmd_flash_sparse_img(const char *arg, void *data, unsigned sz)
{
	unsigned int chunk;
	unsigned int chunk_data_sz;
	sparse_header_t *sparse_header;
	chunk_header_t *chunk_header;
	uint32_t total_blocks = 0;
	unsigned long long ptn = 0;
	unsigned long long size = 0;
	int index = INVALID_PTN;

	index = partition_get_index(arg);
	ptn = partition_get_offset(index);
	if(ptn == 0) {
		fastboot_fail("partition table doesn't exist");
		return;
	}

	size = partition_get_size(index);
	if (ROUND_TO_PAGE(sz,511) > size) {
		fastboot_fail("size too large");
		return;
	}

	/* Read and skip over sparse image header */
	sparse_header = (sparse_header_t *) data;
	data += sparse_header->file_hdr_sz;
	if(sparse_header->file_hdr_sz > sizeof(sparse_header_t))
	{
		/* Skip the remaining bytes in a header that is longer than
		 * we expected.
		 */
		data += (sparse_header->file_hdr_sz - sizeof(sparse_header_t));
	}

	dprintf (SPEW, "=== Sparse Image Header ===\n");
	dprintf (SPEW, "magic: 0x%x\n", sparse_header->magic);
	dprintf (SPEW, "major_version: 0x%x\n", sparse_header->major_version);
	dprintf (SPEW, "minor_version: 0x%x\n", sparse_header->minor_version);
	dprintf (SPEW, "file_hdr_sz: %d\n", sparse_header->file_hdr_sz);
	dprintf (SPEW, "chunk_hdr_sz: %d\n", sparse_header->chunk_hdr_sz);
	dprintf (SPEW, "blk_sz: %d\n", sparse_header->blk_sz);
	dprintf (SPEW, "total_blks: %d\n", sparse_header->total_blks);
	dprintf (SPEW, "total_chunks: %d\n", sparse_header->total_chunks);

	/* Start processing chunks */
	for (chunk=0; chunk<sparse_header->total_chunks; chunk++)
	{
		/* Read and skip over chunk header */
		chunk_header = (chunk_header_t *) data;
		data += sizeof(chunk_header_t);

		dprintf (SPEW, "=== Chunk Header ===\n");
		dprintf (SPEW, "chunk_type: 0x%x\n", chunk_header->chunk_type);
		dprintf (SPEW, "chunk_data_sz: 0x%x\n", chunk_header->chunk_sz);
		dprintf (SPEW, "total_size: 0x%x\n", chunk_header->total_sz);

		if(sparse_header->chunk_hdr_sz > sizeof(chunk_header_t))
		{
			/* Skip the remaining bytes in a header that is longer than
			 * we expected.
			 */
			data += (sparse_header->chunk_hdr_sz - sizeof(chunk_header_t));
		}

		chunk_data_sz = sparse_header->blk_sz * chunk_header->chunk_sz;
		switch (chunk_header->chunk_type)
		{
			case CHUNK_TYPE_RAW:
			if(chunk_header->total_sz != (sparse_header->chunk_hdr_sz +
											chunk_data_sz))
			{
				fastboot_fail("Bogus chunk size for chunk type Raw");
				return;
			}

			if(tcc_write(NULL,
			      ptn + ((uint64_t)total_blocks*sparse_header->blk_sz),
						chunk_data_sz,
						(unsigned int*)data))
			{
				fastboot_fail("flash write failure");
				return;
			}
			total_blocks += chunk_header->chunk_sz;
			data += chunk_data_sz;
			break;

			case CHUNK_TYPE_DONT_CARE:
			total_blocks += chunk_header->chunk_sz;
			break;

			case CHUNK_TYPE_CRC:
			if(chunk_header->total_sz != sparse_header->chunk_hdr_sz)
			{
				fastboot_fail("Bogus chunk size for chunk type Dont Care");
				return;
			}
			total_blocks += chunk_header->chunk_sz;
			data += chunk_data_sz;
			break;

			default:
			fastboot_fail("Unknown chunk type");
			return;
		}
	}

	dprintf(INFO, "Wrote %d blocks, expected to write %d blocks\n",
					total_blocks, sparse_header->total_blks);

	if(total_blocks != sparse_header->total_blks)
	{
		fastboot_fail("sparse image write failure");
	}

	fastboot_okay("");
	return;
}

void cmd_flash(const char *arg, void *data, unsigned sz)
{
	sparse_header_t *sparse_header;
	sparse_header = (sparse_header_t*)data;

	if(sparse_header->magic != SPARSE_HEADER_MAGIC) 
		cmd_flash_img(arg, data, sz);
	else 
		cmd_flash_sparse_img(arg, data, sz);

	fastboot_okay("");
}

void cmd_continue(const char *arg, void *data, unsigned sz)
{
	fastboot_okay("");
	udc_stop();
	boot_linux_from_storage();
}

void cmd_reboot(const char *arg, void *data, unsigned sz)
{
	dprintf(INFO, "rebooting the device\n");
	fastboot_okay("");
	reboot_device(0);
}

void cmd_reboot_bootloader(const char *arg, void *data, unsigned sz)
{
	dprintf(INFO, "rebooting the device\n");
	fastboot_okay("");
	reboot_device(0);
}

void cmd_oem_unlock(const char *arg, void *data, unsigned sz)
{
	if(!device.is_unlocked)
	{
		device.is_unlocked = 1;
		write_device_info(&device);
	}
	fastboot_okay("");
}

void cmd_oem_devinfo(const char *arg, void *data, unsigned sz)
{
	char response[64];
	snprintf(response, 64, "\tDevice tampered: %s", (device.is_tampered ? "true" : "false"));
	fastboot_info(response);
	snprintf(response, 64, "\tDevice unlocked: %s", (device.is_unlocked ? "true" : "false"));
	fastboot_info(response);
	fastboot_okay("");
}

void cmd_oem_format(const char *arg, void *data, unsigned sz)
{

	if(!guid_format(arg)){
		fastboot_okay("");
	}else fastboot_fail("");

	reboot_device(0);
}

void cmd_preflash(const char *arg, void *data, unsigned sz)
{
	fastboot_okay("");
}

void splash_screen()
{
	uint64_t ptn = 0;
	uint64_t index = 0;
	struct fbcon_config *fb_display = NULL;

	index = partition_get_index("splash");

	if (index == 0) {
		dprintf(CRITICAL, "ERROR: Partition table not found\n");
		return;
	}

	ptn = partition_get_offset(index);
	if (ptn == 0) {
		dprintf(CRITICAL, "ERROR: No splash partition found\n");
	} else {
		fb_display = fbcon_display();
		if (fb_display) {
			if (tcc_read(ptn, fb_display->base,
						(fb_display->width * fb_display->height * fb_display->bpp/8))) {
				fbcon_clear();
				dprintf(CRITICAL, "ERROR: Cannot read splash image\n");
			}
		}
	}
}

/* Get the size from partiton name */
static void get_partition_size(const char *arg, char *response)
{
	uint64_t ptn = 0;
	uint64_t size;
	int index = INVALID_PTN;

	index = partition_get_index(arg);

	if (index == INVALID_PTN){
		dprintf(CRITICAL, "Invalid partition index\n");
		return;
	}

	ptn = partition_get_offset(index);

	if(!ptn){
		dprintf(CRITICAL, "Invalid partition name %s\n", arg);
		return;
	}

	size = partition_get_size(index);

	snprintf(response, MAX_RSP_SIZE, "\t 0x%llx", size);
	return;
}

/*
 * Publish the partition type & size info
 * fastboot getvar will publish the required information.
 * fastboot getvar partition_size:<partition_name>: partition size in hex
 * fastboot getvar partition_type:<partition_name>: partition type (ext/fat)
 */
static void publish_getvar_partition_info(struct getvar_partition_info *info, uint8_t num_parts)
{
	uint8_t i;

	for (i = 0; i < num_parts; i++) {
		get_partition_size(info[i].part_name, info[i].size_response);

		if (strlcat(info[i].getvar_size, info[i].part_name, MAX_GET_VAR_NAME_SIZE) >= MAX_GET_VAR_NAME_SIZE)
		{
			dprintf(CRITICAL, "partition size name truncated\n");
			return;
		}
		if (strlcat(info[i].getvar_type, info[i].part_name, MAX_GET_VAR_NAME_SIZE) >= MAX_GET_VAR_NAME_SIZE)
		{
			dprintf(CRITICAL, "partition type name truncated\n");
			return;
		}

		/* publish partition size & type info */
		fastboot_publish((const char *) info[i].getvar_size, (const char *) info[i].size_response);
		fastboot_publish((const char *) info[i].getvar_type, (const char *) info[i].type_response);
	}
}

void aboot_factory_reset()
{
       struct recovery_message recovery_msg;

	strcpy(recovery_msg.command,  "boot-recovery");
	strcpy(recovery_msg.recovery, "recovery\n--wipe_data\n");

	set_recovery_message(&recovery_msg);
}

void aboot_init(const struct app_descriptor *app)
{

	unsigned reboot_mode = 0;
	unsigned usb_init = 0;
	unsigned sz = 0;
	unsigned char c = 0;

	/* Setup page size information for nand/emmc reads */
	if (target_is_emmc_boot()){
		page_size = 2048;
		page_mask = page_size - 1;
	}else{
		page_size = flash_page_size();
		page_mask = page_size - 1;
	}

	if(target_use_signed_kernel()){
		read_device_info(&device);
	}

	target_serialno((unsigned char *) sn_buf);
	dprintf(SPEW,"serial number: %s\n",sn_buf);
	surf_udc_device.serialno = sn_buf;

	if(getc(&c) >= 0)
	{
		dprintf(CRITICAL,"getc: %c\n", c);

		if (c == 'r')
			boot_into_recovery = 1;
		if (c == 'f')
			goto fastboot;
		if (c == 'd')
			aboot_factory_reset();
		if (c == 's')
			skip_loading_quickboot = 1;
		if (c == 'q')
			boot_into_qb_prebuilt = boot_into_recovery = 1;
	}
	else
	{
		boot_into_recovery = 0;
		boot_into_factory = 0;
	}

	if(keys_get_state(KEY_F1) != 0){
		boot_into_chrome = 1;
	}

#ifdef RECOVERY_BY_REMOTE_KEY
	if(getRemoteKey() == SCAN_NUM_9)
	{
		printf("==========================================\n");
		printf("getRemoteKey: 0x%x\n", getRemoteKey());
		boot_into_recovery = 1;
	}
#endif
#ifdef TCC_FACTORY_RESET_SUPPORT //use key1
    #if defined(TARGET_TCC8925_STB_DONGLE)
        gpio_config(GPIO_PORTE|20, GPIO_FN0 | GPIO_INPUT);

	if(gpio_get(GPIO_PORTE|20) == false)
	{
		while(1)
		{
			if(gpio_get(GPIO_PORTE|20) == true)
				break;
		}

                for(int i=0;i<=5000;i++)
		{
			if(gpio_get(GPIO_PORTE|20) == false)
			{
				aboot_factory_reset();
				break;
			}
		}
	}
    #elif defined(TARGET_TCC8935_DONGLE)
	#if defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8933S) || defined(CONFIG_CHIP_TCC8937S)
        gpio_config(GPIO_PORTE|20, GPIO_FN0 | GPIO_INPUT);

	if(gpio_get(GPIO_PORTE|20) == false)
	{
		while(1)
		{
			if(gpio_get(GPIO_PORTE|20) == true)
				break;
		}

                for(int i=0;i<=5000;i++)
		{
			if(gpio_get(GPIO_PORTE|20) == false)
			{
				aboot_factory_reset();
				break;
			}
		}
	}
	#else
        gpio_config(GPIO_PORTE|16, GPIO_FN0 | GPIO_INPUT);

	if(gpio_get(GPIO_PORTE|16) == false)
	{
		while(1)
		{
			if(gpio_get(GPIO_PORTE|16) == true)
				break;
		}

		for(int i=0;i<=5000;i++)
		{
			if(gpio_get(GPIO_PORTE|16) == false)
			{
				aboot_factory_reset();
				break;
			}
		}
	}
	#endif
    #endif
#endif

	/* Check if we should do something other than booting up */
	if ((keys_get_state(KEY_HOME) != 0) || (keys_get_state(KEY_VOLUMEUP) != 0)){
		boot_into_recovery = 1;
	}

	if(!boot_into_recovery)
	{
		if ((keys_get_state(KEY_BACK) != 0) || (keys_get_state(KEY_VOLUMEDOWN) != 0)){

			goto fastboot;
		}
	}

	reboot_mode = check_reboot_mode();
	if (reboot_mode == RECOVERY_MODE) {
		boot_into_recovery = 1;
	} else if(reboot_mode == FASTBOOT_MODE) {
		goto fastboot;
	}

	if(recovery_init())
	  dprintf(ALWAYS,"error in recovery_init\n");

#ifdef CONFIG_SNAPSHOT_BOOT
	if( boot_into_recovery == 0 && skip_loading_quickboot == 0 ) {
#if _EMMC_BOOT
		dprintf(CRITICAL, " - Load QuickBoot Image from EMMC.\n");
#else
		dprintf(CRITICAL, " - Load QuickBoot Image from NAND.\n");
#endif
		snapshot_boot_from_storage();
	}
#endif
	
	boot_linux_from_storage();

	dprintf(CRITICAL, "ERROR: Could not do normal boot. Reverting "
		"to fastboot mode.\n");

fastboot:

	sz = target_get_max_flash_size();

	target_fastboot_init();

	if(!usb_init)
		udc_init(&surf_udc_device);

	fastboot_register("boot", cmd_boot);

	fastboot_register("flash:", cmd_flash);
	fastboot_register("erase:", cmd_erase);
	fastboot_register("continue", cmd_continue);
	fastboot_register("reboot", cmd_reboot);
	fastboot_register("reboot-bootloader", cmd_reboot_bootloader);
	fastboot_register("oem unlock", cmd_oem_unlock);
	fastboot_register("oem device-info", cmd_oem_devinfo);
	fastboot_register("oem format", cmd_oem_format);
	fastboot_register("preflash", cmd_preflash);
	fastboot_publish("product", BOARD);
	fastboot_publish("kernel", "lk");
	fastboot_publish("serialno", sn_buf);
	publish_getvar_partition_info(part_info, ARRAY_SIZE(part_info));

	/* Max download size supported */
	snprintf(max_download_size, MAX_RSP_SIZE, "\t0x%x", sz);
	fastboot_publish("max-download-size", (const char *) max_download_size);
	fastboot_init(target_get_scratch_address(), sz);
	udc_start();
}

APP_START(aboot)
	.init = aboot_init,
APP_END
