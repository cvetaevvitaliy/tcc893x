/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include <debug.h>
#include <string.h>
#include <stdlib.h>

#include <target.h>
#include <sfl.h>
#include <partition_parser.h>
#include <sdmmc/sd_bus.h>

struct partition_entry partition_entries[MAX_PARTITIONS];
extern struct guid_partition android_boot_partitions[];
extern struct guid_partition chrome_boot_partitions[];
extern struct guid_partition dual_boot_partitions[];

static char *ext4_partitions[] = {"system" , "userdata", "cache", "qb_data"};
static char *raw_partitions[] = {"boot","recovery", "kpanic" , "splash" , "misc", "tcc" , "snapshot"};

static unsigned int ext4_count = 0;
static unsigned int raw_count = 0;

unsigned is_gpt_partitions = 0;
unsigned partition_count = 0;

static unsigned int partition_verify_mbr_signature(unsigned size, unsigned char *buffer);

static uint32_t read_boot_MBR();
static uint32_t read_boot_GPT();

unsigned int calculate_crc32(unsigned char *buffer, int len);
int reflect(int data, int len);

static unsigned int partition_parse_gpt_header(unsigned char *buffer,
		unsigned long long *first_usable_lba,
		unsigned int *partition_entry_size,
		unsigned int *header_size,
		unsigned int *max_partition_count);

int debug = 0;

unsigned int read_partition_tlb()
{
	unsigned int ret;

	/* Read MBR of the card */
	ret = read_boot_MBR();
	if (ret) {
		dprintf(CRITICAL, "TCC Boot: MBR read failed!\n");
		return 1;
	}

	/* Read GPT of the card if exist */
	if (is_gpt_partitions) {
		ret = read_boot_GPT();
		if (ret) {
			dprintf(CRITICAL, "TCC Boot: GPT read failed!\n");
			return 1;
		}
		dprintf(INFO ,"BOOT PARTITION TYPE : GPT \n");
	}

	return 0;
}

static uint32_t read_boot_MBR()
{
	unsigned char buffer[512];
	unsigned int dtype;
	unsigned int dfirstsec;
	unsigned int EBR_first_sec;
	unsigned int EBR_current_sec;
	int ret = 0;
	int idx, i;

	/* Read Master Boot Record from eMMC */
	ret = tcc_read(0, buffer, BLOCK_SIZE);

	if(ret)
	{
		dprintf(INFO , "Master Boot Record Read Fail on eMMC\n");
		return ret;
	}

	/* check to see if signature exists */
	if(ret = partition_verify_mbr_signature(BLOCK_SIZE, buffer)) 
		return ret;

	/* Search 4 Primary Partitions */
	idx = TABLE_ENTRY_0;
	for(i=0; i<4; i++)
	{
		
		dtype = buffer[idx+i*TABLE_ENTRY_SIZE + OFFSET_TYPE];
		if(dtype == MBR_PROTECTED_TYPE){
			is_gpt_partitions = 1;
			return ret;
		}
		partition_entries[partition_count].attribute_flag = buffer[idx+i*TABLE_ENTRY_SIZE + OFFSET_STATUS];
		partition_entries[partition_count].dtype = dtype;
		partition_entries[partition_count].first_lba = GET_LWORD_FROM_BYTE(&buffer[idx+i*TABLE_ENTRY_SIZE + OFFSET_FIRST_SEC]);
		partition_entries[partition_count].size = GET_LWORD_FROM_BYTE(&buffer[idx+i*TABLE_ENTRY_SIZE + OFFSET_SIZE]);
		dtype = partition_entries[partition_count].dtype;
		dfirstsec = partition_entries[partition_count].first_lba;
		mbr_set_partition_name(&partition_entries[partition_count], partition_entries[partition_count].dtype); 
		partition_count++;
		if (partition_count == MAX_PARTITIONS)
			return ret;
	}

	if(dtype != WIN_EXTENDED_PARTITION) return ret;

	EBR_first_sec = dfirstsec;
	EBR_current_sec = dfirstsec;

	ret = tcc_read((EBR_first_sec*BLOCK_SIZE), buffer, BLOCK_SIZE);
	if(ret) return ret;

	/* Search for Extended partion Extended Boot Record*/
	for (i = 0;; i++)
	{

		/* check to see if signature exists */
		if((buffer[TABLE_SIGNATURE] != 0x55) || (buffer[TABLE_SIGNATURE+1] !=0xAA))
		{
			dprintf(CRITICAL , "Extended Partition MBR Signature does not match \n");
			break;
		}

		partition_entries[partition_count].attribute_flag =  buffer[TABLE_ENTRY_0 + OFFSET_STATUS];
		partition_entries[partition_count].dtype   = buffer[TABLE_ENTRY_0 + OFFSET_TYPE];
		partition_entries[partition_count].first_lba = GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_0 + OFFSET_FIRST_SEC])+EBR_current_sec;
		partition_entries[partition_count].size = GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_0 + OFFSET_SIZE]);
		mbr_set_partition_name(&partition_entries[partition_count], partition_entries[partition_count].dtype); 
		partition_count++;
		if (partition_count == MAX_PARTITIONS)
			return ret;

		dfirstsec = GET_LWORD_FROM_BYTE(&buffer[TABLE_ENTRY_1 + OFFSET_FIRST_SEC]);
		if(dfirstsec == 0)
		{
			/* Getting to the end of the EBR tables */
			break;
		}
		/* More EBR to follow - read in the next EBR sector */
		ret = tcc_read((EBR_first_sec+dfirstsec)*BLOCK_SIZE, buffer, BLOCK_SIZE);

		if (ret)
		{
			return ret;
		}
		EBR_current_sec = EBR_first_sec + dfirstsec;
	}

	return ret;

}

static uint32_t read_boot_GPT()
{
	unsigned char* data = malloc(512);
	int ret = 0;
	unsigned int header_size;
	unsigned long long first_usable_lba;
	unsigned long long backup_header_lba;
	unsigned long long storage_size_sec;
	unsigned int max_partition_count = 0;
	unsigned int partition_entry_size;
	unsigned int i = 0;	/* Counter for each 512 block */
	unsigned int j = 0;	/* Counter for each 128 entry in the 512 block */
	unsigned int n = 0;	/* Counter for UTF-16 -> 8 conversion */
	unsigned char UTF16_name[MAX_GPT_NAME_SIZE];
	/* LBA of first partition -- 1 Block after Protected MBR + 1 for PT */
	unsigned long long partition_0;

	partition_count = 0;

	/* Get the density of the Storage device */

	/* Print out the GPT first */
	ret = tcc_read(PROTECTIVE_MBR_SIZE, data, BLOCK_SIZE);

	debug = 0;
	while(debug);

	if (ret)
		dprintf(CRITICAL, "GPT: Could not read primary gpt from Storage\n");

	ret = partition_parse_gpt_header(data, &first_usable_lba,
			&partition_entry_size, &header_size,
			&max_partition_count);
	if (ret) {
		dprintf(INFO, "GPT: (WARNING) Primary signature invalid\n");

		/* Check the backup gpt */

		/* Get size of MMC */
		storage_size_sec = tcc_get_storage_capacity(); // get number of total sector count
		ASSERT (storage_size_sec > 0);


		backup_header_lba = storage_size_sec - 1;
		tcc_read((backup_header_lba * BLOCK_SIZE), data, BLOCK_SIZE);

		if (ret) {
			dprintf(CRITICAL,
					"GPT: Could not read backup gpt from Storage\n");
			return ret;
		}

		ret = partition_parse_gpt_header(data, &first_usable_lba,
				&partition_entry_size,
				&header_size,
				&max_partition_count);
		if (ret) {
			dprintf(CRITICAL,
					"GPT: Primary and backup signatures invalid\n");
			return ret;
		}
	}
	partition_0 = GET_LLWORD_FROM_BYTE(&data[PARTITION_ENTRIES_OFFSET]);
	/* Read GPT Entries */
	for (i = 0; i < (ROUNDUP(max_partition_count, 4)) / 4; i++) {
		ASSERT(partition_count < NUM_PARTITIONS);

		ret = tcc_read((partition_0 * BLOCK_SIZE) + (i * BLOCK_SIZE), data ,BLOCK_SIZE);

		if (ret) {
			dprintf(CRITICAL,
					"GPT: read Storage failed reading partition entries*.\n");
			return ret;
		}

		for (j = 0; j < 4; j++) {
			
			memcpy(&(partition_entries[partition_count].type_guid),
					&data[(j * partition_entry_size)],
					PARTITION_TYPE_GUID_SIZE);

			if (partition_entries[partition_count].type_guid[0] == 0x00
					&& partition_entries[partition_count].
					type_guid[1] == 0x00) {
				i = ROUNDUP(max_partition_count, 4);
				break;
			}
			memcpy(&(partition_entries[partition_count].unique_partition_guid),
					&data[(j * partition_entry_size) + UNIQUE_GUID_OFFSET],
					UNIQUE_PARTITION_GUID_SIZE);

			partition_entries[partition_count].first_lba =
				GET_LLWORD_FROM_BYTE(&data
						[(j * partition_entry_size) +
						FIRST_LBA_OFFSET]);
			partition_entries[partition_count].last_lba =
				GET_LLWORD_FROM_BYTE(&data
						[(j * partition_entry_size) +
						LAST_LBA_OFFSET]);
			partition_entries[partition_count].size =
				partition_entries[partition_count].last_lba -
				partition_entries[partition_count].first_lba + 1;
			partition_entries[partition_count].attribute_flag =
				GET_LLWORD_FROM_BYTE(&data
						[(j * partition_entry_size) +
						ATTRIBUTE_FLAG_OFFSET]);

			memset(&UTF16_name, 0x00, MAX_GPT_NAME_SIZE);
			memcpy(UTF16_name, &data[(j * partition_entry_size) +
					PARTITION_NAME_OFFSET],
					MAX_GPT_NAME_SIZE);
			/*
			 * Currently partition names in *.xml are UTF-8 and lowercase
			 * Only supporting english for now so removing 2nd byte of UTF-16
			 */
			for (n = 0; n < MAX_GPT_NAME_SIZE / 2; n++) {
				partition_entries[partition_count].name[n] =
					UTF16_name[n * 2];
			}
			partition_count++;
		}
	}
	return ret;
}

void mbr_set_partition_name(struct partition_entry* ptn_ent, unsigned int type)
{
	switch(type) {
		memset(ptn_ent->name , 0, MAX_GPT_NAME_SIZE);
		case VFAT_PARTITION:
		case VFAT16_PARTITION:
			memcpy(ptn_ent->name , "data" , 4);
			break;
		case NATIVE_LINUX_PARTITION:
			if(ext4_count == sizeof(ext4_partitions) / sizeof(char*))
				return;
			strlcpy((char*)ptn_ent->name ,
				(const char*)ext4_partitions[ext4_count], sizeof(ptn_ent->name));
			ext4_count++;
			break;
		case RAW_EMPTY_PARTITION:
			if(raw_count == sizeof(raw_partitions)/sizeof(char*))
				return;
			strlcpy((char*)ptn_ent->name,
				(const char*)raw_partitions[raw_count], sizeof(ptn_ent->name));
			raw_count++;
			break;
		default : 
			break;
	}
}

void partition_dump()
{
	unsigned idx;

	for(idx = 0; idx < partition_count; idx++)
	{
		if(partition_entries[idx].dtype == LINUX_EXTENDED_PARTITION)
			continue;

		dprintf(INFO, "[PARTITION : %10s] [START : %10llu] [SIZE : %10llu] [TYPE : %2x]\n",
				partition_entries[idx].name, partition_entries[idx].first_lba,
				partition_entries[idx].size ,partition_entries[idx].dtype);
	}
}

int get_partition_info(char *partition_name, unsigned long *addr, unsigned long *size)
{
	unsigned int idx;

	for( idx=0; idx<partition_count; idx++ )
	{
		if( !strcmp(partition_entries[idx].name, partition_name ) )
		{
			*addr = partition_entries[idx].first_lba;
			*size = partition_entries[idx].size;
			dprintf(INFO, "[ERASED PARTITION : %10s] [START : %10llu] [SIZE : %10llu] [TYPE : %2x]\n" , 
					partition_entries[idx].name, partition_entries[idx].first_lba,
					partition_entries[idx].size ,partition_entries[idx].dtype);
			return 0;
		}
	}
	return -1;
}

int partition_get_index(const char *name)
{
	unsigned n;

	for(n=0; n< partition_count; n++){
		if(!strcmp((const char*)partition_entries[n].name, (const char*)name))
			return n;
	}
	return 0;
}

unsigned long long partition_get_offset(int index)
{
	return partition_entries[index].first_lba * 512;
}

unsigned long long partition_get_size(int index)
{
	if (index < partition_count)
		return partition_entries[index].size * 512;

	return 0;
}

static unsigned int partition_verify_mbr_signature(unsigned size, unsigned char *buffer)
{
	/* Avoid checking past end of buffer */
	if ((TABLE_SIGNATURE + 1) > size) {
		return 1;
	}
	/* Check to see if signature exists */
	if ((buffer[TABLE_SIGNATURE] != MMC_MBR_SIGNATURE_BYTE_0) ||
			(buffer[TABLE_SIGNATURE + 1] != MMC_MBR_SIGNATURE_BYTE_1)) {
		dprintf(CRITICAL, "MBR signature does not match.\n");
		return 1;
	}
	return 0;
}

/*
 * Parse the gpt header and get the required header fields
 * Return 0 on valid signature
 */
static unsigned int partition_parse_gpt_header(unsigned char *buffer,
		unsigned long long *first_usable_lba,
		unsigned int *partition_entry_size,
		unsigned int *header_size,
		unsigned int *max_partition_count)
{

	/* Check GPT Signature */
	if (((uint32_t *) buffer)[0] != GPT_SIGNATURE_2 ||
			((uint32_t *) buffer)[1] != GPT_SIGNATURE_1)
		return 1;


	*header_size = GET_LWORD_FROM_BYTE(&buffer[HEADER_SIZE_OFFSET]);
	*first_usable_lba =
		GET_LLWORD_FROM_BYTE(&buffer[FIRST_USABLE_LBA_OFFSET]);
	*max_partition_count =
		GET_LWORD_FROM_BYTE(&buffer[PARTITION_COUNT_OFFSET]);
	*partition_entry_size =
		GET_LWORD_FROM_BYTE(&buffer[PENTRY_SIZE_OFFSET]);

	return 0;
}
/*
 * A8h reflected is 15h, i.e. 10101000 <--> 00010101
*/
int reflect(int data, int len)
{
	int ref = 0;
	for (int i = 0; i < len; i++) {
		if (data & 0x1) {
			ref |= (1 << ((len - 1) - i));
		}
		data = (data >> 1);
	}
	return ref;
}

/*
* Function to calculate the CRC32
*/
unsigned int calculate_crc32(unsigned char *buffer, int len)
{
	int byte_length = 8;	/*length of unit (i.e. byte) */
	int msb = 0;
	int polynomial = 0x104C11DB7;	/* IEEE 32bit polynomial */
	unsigned int regs = 0xFFFFFFFF;	/* init to all ones */
	int regs_mask = 0xFFFFFFFF;	/* ensure only 32 bit answer */
	int regs_msb = 0;
	unsigned int reflected_regs;

	for (int i = 0; i < len; i++) {
		int data_byte = buffer[i];
		data_byte = reflect(data_byte, 8);
		for (int j = 0; j < byte_length; j++) {
			msb = data_byte >> (byte_length - 1);	/* get MSB */
			msb &= 1;	/* ensure just 1 bit */
			regs_msb = (regs >> 31) & 1;	/* MSB of regs */
			regs = regs << 1;	/* shift regs for CRC-CCITT */
			if (regs_msb ^ msb) {	/* MSB is a 1 */
				regs = regs ^ polynomial;	/* XOR with generator poly */
			}
			regs = regs & regs_mask;	/* Mask off excess upper bits */
			data_byte <<= 1;	/* get to next bit */
		}
	}
	regs = regs & regs_mask;
	reflected_regs = reflect(regs, 32) ^ 0xFFFFFFFF;

	return reflected_regs;
}

static void prepare_mbr(unsigned char* mbr, unsigned int lba)
{
	mbr[0x1be] = 0x00; // bootalbe == false
	mbr[0x1bf] = 0x00; // CHS
	mbr[0x1c0] = 0x00; // CHS
	mbr[0x1c1] = 0x00; // CHS

	mbr[0x1c2] = MBR_PROTECTED_TYPE; // SET GPT partitoin 
	mbr[0x1c3] = 0x00; // CHS
	mbr[0x1c4] = 0x00; // CHS 
	mbr[0x1c5] = 0x00; // CHS

	mbr[0x1c6] = 0x01; // Relative Start Sector
	mbr[0x1c7] = 0x00;
	mbr[0x1c8] = 0x00;
	mbr[0x1c9] = 0x00;

	memcpy(mbr + 0x1ca, &lba, sizeof(unsigned int));

	mbr[0x1fe] = 0x55; // MBR Signature
	mbr[0x1ff] = 0xAA; // MBR Signature
}

static void prepare_guid_header(struct guid_partition_tbl *ptbl, unsigned int lba)
{
	struct uefi_header *hdr = &ptbl->guid_header;

	memset(ptbl, 0, sizeof(*ptbl));

	prepare_mbr(ptbl->mbr, lba - 1);

	memcpy(hdr->magic, GUID_MAGIC, 8);
	hdr->version		= GUID_VERSION;
	hdr->header_size	= sizeof(struct uefi_header);
	hdr->header_lba		= 1;
	hdr->backup_lba		= lba - 1;
	hdr->first_lba		= 34;
	hdr->last_lba		= lba - 1;
	memcpy(hdr->guid, volume_guid, 16);
	hdr->efi_entries_lba	= 2;
	hdr->efi_entries_count	= MAX_PARTITIONS;
	hdr->efi_entries_size	= sizeof(struct gpt_partition_entry);
}

static void fill_crc32(struct guid_partition_tbl *ptbl)
{
	struct uefi_header *hdr = &ptbl->guid_header;
	unsigned int crc;
	
	crc = 0;
	crc = calculate_crc32((unsigned char*)ptbl->guid_entry, hdr->efi_entries_count*ENTRY_SIZE);
	hdr->efi_entries_crc32 = crc;

	crc = 0;
	crc = calculate_crc32((unsigned char*)&ptbl->guid_header, sizeof(ptbl->guid_header));
	hdr->crc32 = crc;
}

static int guid_add_partition(struct guid_partition_tbl *ptbl, unsigned long long first_lba,
	unsigned long long last_lba, const char* name)
{
	struct uefi_header *hdr = &ptbl->guid_header;
	struct gpt_partition_entry *entry = ptbl->guid_entry;
	unsigned int idx, iname;

	debug = 0;
	while(debug);

	if(first_lba < GUID_RESERVED){
		dprintf(INFO, "partition %s Overwrapped GUID RESERVED Area \n", name);
		return -1;
	}

	if(last_lba > hdr->last_lba){
		dprintf(INFO, "Partition %s Over Size to Whole Storage Size \n", name);
		return -1;
	}

	for(idx = 0; idx < ENTRY_SIZE; idx++, entry++){

		if(entry->last_lba) continue;

		memcpy(entry->type_guid , guid_partition_type[1], 16);
		memcpy(entry->unique_partition_guid , volume_guid, 16);
		entry->first_lba = first_lba;
		entry->last_lba = last_lba;
		for(idx = 0; (idx < MAX_GPT_NAME_SIZE / 2) && *name ; idx++)
			entry->name[idx*2] = *name++;
		return 0;
	}

	dprintf(INFO, "Unrecognized partition %s \n" , name);
	return -1;
}


static int guid_partition_format(struct guid_partition* partitions)
{
	struct guid_partition_tbl *ptbl = malloc(sizeof(struct guid_partition_tbl));
	unsigned int npart, idx;
	int ret;
	unsigned long long storage_size = 0;

	storage_size = tcc_get_storage_capacity();

	if(storage_size < 0){
		dprintf(INFO , "Target Storage has no space !!\n");
		goto error;
	}

	prepare_guid_header(ptbl, storage_size);

	for(idx = 0, npart = 0; partitions[idx].name; idx++){

		unsigned long long size = partitions[idx].size_kb * 2;

		if(!strcmp(partitions[idx].name , "guid")){
			npart += size;
			continue;
		}

		if(size == 0) size = storage_size - npart;

		if(guid_add_partition(ptbl, npart, npart + size -1, partitions[idx].name)){
			goto error;
		}
		npart += size;
	}
	fill_crc32(ptbl);

	ret = tcc_write(NULL, 0, sizeof(struct guid_partition_tbl), ptbl);
	if(ret < 0){
		dprintf(INFO, "fail write partition table to target storage \n");
		goto error;
	}

	dprintf(INFO , "complete write partition table to target storage \n");
	free(ptbl);
	return 0;

error:
	free(ptbl);
	return -1;

}

int guid_format(const char *cmd)
{
	struct guid_partition *partitions;

	dprintf(INFO , "%s \n" ,cmd);
	
	if(!strcmp(cmd , " android")){
		partitions = android_boot_partitions;
	}
	else if(!strcmp(cmd , " chrome")){
		if(target_is_chrome_boot()){
			partitions = chrome_boot_partitions;
		}else{
			dprintf(CRITICAL, "NOT Support Chrome OS Boot for taret Device \n");
			goto err;
		}
	}
	else if(!strcmp(cmd , " dual")) {
		if(target_is_dual_boot()){
			partitions = dual_boot_partitions;
		}else{
			dprintf(CRITICAL, "NOT Support Chrome os and Android dual boot for target Device\n");
			goto err;
		}
	}
	else goto err;

	return guid_partition_format(partitions);
err:
	dprintf(INFO , "Partition format failed");
	return -1;

}

