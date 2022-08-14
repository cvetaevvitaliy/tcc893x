/**/

#include <arch/arm.h>
#include <debug.h>
#include <platform/timer.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <arch/ops.h>

typedef unsigned int uint32;

typedef enum {
	SMEM_SPINLOCK_ARRAY = 7,

	SMEM_AARM_PARTITION_TABLE = 9,

	SMEM_APPS_BOOT_MODE = 106,

	SMEM_BOARD_INFO_LOCATION = 137,

	SMEM_USABLE_RAM_PARTITION_TABLE = 402,

	SMEM_POWER_ON_STATUS_INFO = 403,

	SMEM_RLOCK_AREA = 404,

	SMEM_BOOT_INFO_FOR_APPS = 418,

	SMEM_FIRST_VALID_TYPE = SMEM_SPINLOCK_ARRAY,
	SMEM_LAST_VALID_TYPE = SMEM_BOOT_INFO_FOR_APPS,

	SMEM_MAX_SIZE = SMEM_BOOT_INFO_FOR_APPS + 1,
} smem_mem_type_t;

int flash_ecc_bch_enabled(void)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

unsigned int write_partition(unsigned size, unsigned char *partition)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

int scm_svc_version(uint32 * major, uint32 * minor)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

int encrypt_scm(uint32_t ** img_ptr, uint32_t * img_len_ptr)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

int scm_protect_keystore(uint32_t * img_ptr, uint32_t  img_len)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

int decrypt_scm(uint32_t ** img_ptr, uint32_t * img_len_ptr)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

int decrypt_scm_v2(uint32_t ** img_ptr, uint32_t * img_len_ptr)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

unsigned smem_read_alloc_entry(smem_mem_type_t type, void *buf, int len)
{
	dprintf(CRITICAL, "%s function is dummy function\n", __func__);
}

#if _EMMC_BOOT
#else
unsigned int mmc_write(unsigned long long data_addr, unsigned int data_len, unsigned int *in)
{
	return 0;
}

unsigned int mmc_read(unsigned long long data_addr, unsigned int * out, unsigned int data_len)
{
	return 0;
}

void emmc_boot_main()
{
}
#endif
