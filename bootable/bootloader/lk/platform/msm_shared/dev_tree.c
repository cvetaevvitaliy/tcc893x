/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 *  with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <libfdt.h>
#include <dev_tree.h>
#include <lib/ptable.h>
#include <malloc.h>
#include <qpic_nand.h>
#include <stdlib.h>
#include <string.h>
#include <platform.h>
#include <board.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

extern int target_is_emmc_boot(void);
extern uint32_t target_dev_tree_mem(void *fdt, uint32_t memory_node_offset);

struct msm_id
{
	uint32_t platform_id;
	uint32_t hardware_id;
	uint32_t soc_rev;
};

static bool is_dev_tree_compatible(void *dtb)
{
	int root_offset;
	const void *prop;
	char model[128];
	struct msm_id msm_id;
	int len;

	root_offset = fdt_path_offset(dtb, "/");
	if (root_offset < 0)
		return false;

	prop = fdt_getprop(dtb, root_offset, "model", &len);
	if (prop && len > 0) {
		memcpy(model, prop, min((int)sizeof(model), len));
		model[sizeof(model) - 1] = '\0';
	} else {
		model[0] = '\0';
	}

	prop = fdt_getprop(dtb, root_offset, "qcom,msm-id", &len);
	if (!prop || len <= 0) {
		dprintf(INFO, "qcom,msm-id entry not found\n");
		return false;
	} else if (len < (int)sizeof(struct msm_id)) {
		dprintf(INFO, "qcom,msm-id entry size mismatch (%d != %d)\n",
			len, sizeof(struct msm_id));
		return false;
	}
	msm_id.platform_id = fdt32_to_cpu(((const struct msm_id *)prop)->platform_id);
	msm_id.hardware_id = fdt32_to_cpu(((const struct msm_id *)prop)->hardware_id);
	msm_id.soc_rev = fdt32_to_cpu(((const struct msm_id *)prop)->soc_rev);

	dprintf(INFO, "Found an appended flattened device tree (%s - %d %d 0x%x)\n",
		*model ? model : "unknown",
		msm_id.platform_id, msm_id.hardware_id, msm_id.soc_rev);

	if (msm_id.platform_id != board_platform_id() ||
	    msm_id.hardware_id != board_hardware_id() ||
	    msm_id.soc_rev != board_soc_version()) {
		dprintf(INFO, "Device tree's msm_id doesn't match the board: <%d %d 0x%x> != <%d %d 0x%x>\n",
			msm_id.platform_id,
			msm_id.hardware_id,
			msm_id.soc_rev,
			board_platform_id(),
			board_hardware_id(),
			board_soc_version());
		return false;
	}

	return true;
}

/*
 * Will relocate the DTB to the tags addr if the device tree is found and return
 * its address
 *
 * Arguments:    kernel - Start address of the kernel loaded in RAM
 *               tags - Start address of the tags loaded in RAM
 * Return Value: DTB address : If appended device tree is found
 *               'NULL'         : Otherwise
 */
void *dev_tree_appended(void *kernel, uint32_t kernel_size, void *tags)
{
	void *kernel_end = kernel + kernel_size;
	uint32_t app_dtb_offset = 0;
	void *dtb;

	memcpy((void*) &app_dtb_offset, (void*) (kernel + DTB_OFFSET), sizeof(uint32_t));

	dtb = kernel + app_dtb_offset;
	while (dtb + sizeof(struct fdt_header) < kernel_end) {
		bool compat;
		struct fdt_header dtb_hdr;
		uint32_t dtb_size;

		/* the DTB could be unaligned, so extract the header,
		 * and operate on it separately */
		memcpy(&dtb_hdr, dtb, sizeof(struct fdt_header));
		if (fdt_check_header((const void *)&dtb_hdr) != 0 ||
		    (dtb + fdt_totalsize((const void *)&dtb_hdr) > kernel_end))
			break;
		dtb_size = fdt_totalsize(&dtb_hdr);

		/* now that we know we have a valid DTB, we need to copy
		 * it somewhere aligned, like tags */
		memcpy(tags, dtb, dtb_size);

		compat = is_dev_tree_compatible(tags);
		if (compat) {
			/* clear out the old DTB magic so kernel doesn't find it */
			*((uint32_t *)(kernel + app_dtb_offset)) = 0;
			return tags;
		}

		/* goto the next device tree if any */
		dtb += dtb_size;
	}

	return NULL;
}

/* Function to return the pointer to the start of the correct device tree
 *  based on the platform data.
 */
struct dt_entry * dev_tree_get_entry_ptr(struct dt_table *table)
{
	uint32_t i;
	struct dt_entry *dt_entry_ptr;
	struct dt_entry *latest_dt_entry = NULL;

	dt_entry_ptr = (struct dt_entry *)((char *)table + DEV_TREE_HEADER_SIZE);

	for(i = 0; i < table->num_entries; i++)
	{
		/* DTBs are stored in the ascending order of soc revision.
		 * For eg: Rev0..Rev1..Rev2 & so on.
		 * we pickup the DTB with highest soc rev number which is less
		 * than or equal to actual hardware
		 */
		if((dt_entry_ptr->platform_id == board_platform_id()) &&
		   (dt_entry_ptr->variant_id == board_hardware_id()) &&
		   (dt_entry_ptr->soc_rev == board_soc_version()))
			{
				return dt_entry_ptr;
			}
		/* if the exact match not found, return the closest match
		 * assuming it to be the nearest soc version
		 */
		if((dt_entry_ptr->platform_id == board_platform_id()) &&
		  (dt_entry_ptr->variant_id == board_hardware_id()) &&
		  (dt_entry_ptr->soc_rev <= board_soc_version())) {
			latest_dt_entry = dt_entry_ptr;
		}
		dt_entry_ptr++;
	}

	if (latest_dt_entry) {
		dprintf(SPEW, "Loading DTB with SOC version:%x\n", latest_dt_entry->soc_rev);
		return latest_dt_entry;
	}

	return NULL;
}

/* Function to add the first RAM partition info to the device tree.
 * Note: The function replaces the reg property in the "/memory" node
 * with the addr and size provided.
 */
int dev_tree_add_first_mem_info(uint32_t *fdt, uint32_t offset, uint32_t addr, uint32_t size)
{
	int ret;

	ret = fdt_setprop_u32(fdt, offset, "reg", addr);

	if (ret)
	{
		dprintf(CRITICAL, "Failed to add the memory information addr: %d\n",
				ret);
	}


	ret = fdt_appendprop_u32(fdt, offset, "reg", size);

	if (ret)
	{
		dprintf(CRITICAL, "Failed to add the memory information size: %d\n",
				ret);
	}

	return ret;
}

/* Function to add the subsequent RAM partition info to the device tree. */
int dev_tree_add_mem_info(void *fdt, uint32_t offset, uint32_t addr, uint32_t size)
{
	static int mem_info_cnt = 0;
	int ret;

	if (!mem_info_cnt)
	{
		/* Replace any other reg prop in the memory node. */
		ret = fdt_setprop_u32(fdt, offset, "reg", addr);
		mem_info_cnt = 1;
	}
	else
	{
		/* Append the mem info to the reg prop for subsequent nodes.  */
		ret = fdt_appendprop_u32(fdt, offset, "reg", addr);
	}

	if (ret)
	{
		dprintf(CRITICAL, "Failed to add the memory information addr: %d\n",
				ret);
	}


	ret = fdt_appendprop_u32(fdt, offset, "reg", size);

	if (ret)
	{
		dprintf(CRITICAL, "Failed to add the memory information size: %d\n",
				ret);
	}

	return ret;
}

/* Top level function that updates the device tree. */
int update_device_tree(void *fdt, const char *cmdline,
					   void *ramdisk, uint32_t ramdisk_size)
{
	int ret = 0;
	uint32_t offset;

	/* Check the device tree header */
	ret = fdt_check_header(fdt);
	if (ret)
	{
		dprintf(CRITICAL, "Invalid device tree header \n");
		return ret;
	}

	/* Add padding to make space for new nodes and properties. */
	ret = fdt_open_into(fdt, fdt, fdt_totalsize(fdt) + DTB_PAD_SIZE);
	if (ret!= 0)
	{
		dprintf(CRITICAL, "Failed to move/resize dtb buffer: %d\n", ret);
		return ret;
	}

	/* Get offset of the memory node */
	ret = fdt_path_offset(fdt, "/memory");
	if (ret < 0)
	{
		dprintf(CRITICAL, "Could not find memory node.\n");
		return ret;
	}

	offset = ret;

	ret = target_dev_tree_mem(fdt, offset);
	if(ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update memory node\n");
		return ret;
	}

	/* Get offset of the chosen node */
	ret = fdt_path_offset(fdt, "/chosen");
	if (ret < 0)
	{
		dprintf(CRITICAL, "Could not find chosen node.\n");
		return ret;
	}

	offset = ret;
	/* Adding the cmdline to the chosen node */
	ret = fdt_setprop_string(fdt, offset, (const char*)"bootargs", (const void*)cmdline);
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update chosen node [bootargs]\n");
		return ret;
	}

	/* Adding the initrd-start to the chosen node */
	ret = fdt_setprop_u32(fdt, offset, "linux,initrd-start", (uint32_t)ramdisk);
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update chosen node [linux,initrd-start]\n");
		return ret;
	}

	/* Adding the initrd-end to the chosen node */
	ret = fdt_setprop_u32(fdt, offset, "linux,initrd-end", ((uint32_t)ramdisk + ramdisk_size));
	if (ret)
	{
		dprintf(CRITICAL, "ERROR: Cannot update chosen node [linux,initrd-end]\n");
		return ret;
	}

	fdt_pack(fdt);

	return ret;
}
