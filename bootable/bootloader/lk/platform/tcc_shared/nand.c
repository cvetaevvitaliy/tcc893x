/*
 * Copyright (c) 2010 Telechips, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <debug.h>
#include <lib/ptable.h>
#include <Disk.h>

#include <partition_parser.h>

/* Test Code */
#define NAND_BOOT_MAGIC "ANDROID!"
#define NAND_BOOT_MAGIC_SIZE 8
#define NAND_BOOT_NAME_SIZE 16
#define NAND_BOOT_ARGS_SIZE 512
#define NAND_BOOT_IMG_HEADER_ADDR 0xFF000

typedef struct nand_boot_img_hdr nand_boot_img_hdr;

struct nand_boot_img_hdr
{
    unsigned char magic[NAND_BOOT_MAGIC_SIZE];

    unsigned kernel_size;  /* size in bytes */
    unsigned kernel_addr;  /* physical load addr */

    unsigned ramdisk_size; /* size in bytes */
    unsigned ramdisk_addr; /* physical load addr */

    unsigned second_size;  /* size in bytes */
    unsigned second_addr;  /* physical load addr */

    unsigned tags_addr;    /* physical addr for kernel tags */
    unsigned page_size;    /* flash page size we assume */
    unsigned unused[2];    /* future expansion: should be 0 */

    unsigned char name[NAND_BOOT_NAME_SIZE]; /* asciiz product name */
    
    unsigned char cmdline[NAND_BOOT_ARGS_SIZE];

    unsigned id[8]; /* timestamp / checksum / sha1 / etc */
};
/* Test Code */

#define BYTE_TO_SECTOR(X)			((X + 511) >> 9)
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))


void flash_boot_main()
{
	unsigned int nPartitionCnt =0;

	dprintf(INFO , "%s:init start from NAND\n" , __func__ );
	if(NAND_Ioctl(0, 0) != 0)
		dprintf(INFO , "%s:init failure!!!\n" , __func__ );

	read_partition_tlb();
	partition_dump();
}

unsigned int flash_read_tnftl_v8(unsigned long long data_addr , unsigned data_len , void* in)
{
	int val = 0;
	unsigned int pageCount = 0;

	pageCount = (data_len + 511) / 512;

	if(pageCount)
		val = NAND_ReadSector(0, (unsigned long)data_addr, pageCount, in);

	return val;
}

unsigned int flash_write_tnftl_v8(char *part_name, unsigned long long data_addr , unsigned data_len , void* in)
{
	int val = 0;
	if( !strcmp( part_name, "bootloader"))
	{
		unsigned int romsize;
		unsigned int *data;

		data = in;				
		romsize = data[7];
		val = NAND_WriteFirmware(0, (unsigned char*)in, romsize);
	} 
	else
	{	
		unsigned int pageCount = 0;

		pageCount = (data_len + 511) / 512;

		if(pageCount)
			val = NAND_WriteSector(0, (unsigned long)data_addr, pageCount, in);	
	}
	
	return val;	
}

unsigned int flash_nand_dump( unsigned char *buf)
{
	struct nand_boot_img_hdr *hdr = (void*) buf;
	struct nand_boot_img_hdr *uhdr;
	unsigned char *databuf = buf;
	unsigned n;
	unsigned long long ptn, idx;
	unsigned offset = 0;
	unsigned int page_mask = 0;
	unsigned int flash_pagesize = 8192;

	read_partition_tlb();
	partition_dump();
		
	uhdr = (struct nand_boot_img_hdr *)NAND_BOOT_IMG_HEADER_ADDR;
	if(!memcmp(uhdr->magic , NAND_BOOT_MAGIC, NAND_BOOT_MAGIC_SIZE)){
		dprintf(INFO , "unified boot method!!\n");
		hdr = uhdr;
		return 0;
	}

	idx = partition_get_index("boot");
	ptn = partition_get_offset(idx);

	if(ptn == 0){
		dprintf(CRITICAL , "ERROR : No BOOT Partition Found!!\n");
		return -1;
	}
	
	dprintf(CRITICAL , "[ Start Read Boot Image Read]\n");
	if(flash_read_tnftl_v8((ptn + BYTE_TO_SECTOR(offset)), flash_pagesize, (unsigned int*)buf))
	{
		dprintf(CRITICAL , "ERROR : Cannot read boto imgage header\n");
		return -1;
	}

	if(memcmp(hdr->magic, NAND_BOOT_MAGIC, NAND_BOOT_MAGIC_SIZE)){
		dprintf(CRITICAL ,"ERROR: Invalid boot image header\n");
		return -1;
	}

	if (hdr->page_size && (hdr->page_size != flash_pagesize)) {
		flash_pagesize = hdr->page_size;
		page_mask = flash_pagesize - 1;
	}

	offset += flash_pagesize;
	n = ROUND_TO_PAGE(hdr->kernel_size, page_mask);
	databuf += offset;

	dprintf(CRITICAL , "[ Start Read Kernel Image Read ]\n");
	
	if (flash_read_tnftl_v8(ptn + BYTE_TO_SECTOR(offset),n, (void *)databuf))
	{
		dprintf(CRITICAL, "ERROR: Cannot read kernel image\n");
                return -1;
	}
	databuf += n;
	n = ROUND_TO_PAGE(hdr->kernel_size, hdr->page_size-1); //base size is different from nand.		
	offset += n;
	n = ROUND_TO_PAGE(hdr->ramdisk_size, page_mask);

	dprintf(CRITICAL , "[ Start Read Ramdisk Image Read ]\n");
	if (flash_read_tnftl_v8(ptn + BYTE_TO_SECTOR(offset), n, (void *)databuf))
	{
		dprintf(CRITICAL, "ERROR: Cannot read ramdisk image\n");
                return -1;
	}
	offset += n;
	dprintf(CRITICAL, "[ End Read Boot Image ] [size:%d]\n", offset );
	return offset;
}

unsigned flash_page_size(void)
{
#if (1)
	return 512;
#else
	return flash_pagesize;
#endif
}

int flash_read_ext(struct ptentry *ptn, unsigned extra_per_page, unsigned offset, void *data, unsigned bytes)
{
	return (int)flash_read_tnftl_v8((unsigned long long)(ptn->start + BYTE_TO_SECTOR(offset)), bytes, (unsigned int*)data);
}

int flash_write(struct ptentry *ptn, unsigned extra_per_page, const void *data, unsigned bytes)
{
	if (extra_per_page == 0)
		return (int)flash_write_tnftl_v8(ptn->name, ptn->start, bytes, data);
	else
		dprintf(CRITICAL, "%s function is dummy function: ext_page:%d\n", __func__, extra_per_page);

	return 0;
}

int flash_erase(char *ptn_name)
{
        int index = INVALID_PTN;
	unsigned long long ptn = 0;
	unsigned remained;
	unsigned char* erase_buffer;

	index = partition_get_index(ptn_name);
	ptn = partition_get_offset(index);

	erase_buffer = malloc(sizeof(char)*16*1024);
	memset(erase_buffer , 0xFF , 16*1024);

#if 0
	/* erase all partition data using overwrite so it's very slow. */
	remained = partition_get_size(index);
#else	
	remained = 16*1024;
#endif


	while(remained) {
		if (remained > 16*1024) {
			if(flash_write_tnftl_v8(ptn_name, BYTE_TO_SECTOR(ptn), 16*1024, erase_buffer)){
				free(erase_buffer);
				return -1;
			}
			remained -= (16*1024);
		}
		else {
			if(flash_write_tnftl_v8(ptn_name, BYTE_TO_SECTOR(ptn), remained, erase_buffer)){
				free(erase_buffer);
				return -1;
			}
			remained = 0;
		}	
	}

	free(erase_buffer);
	return 0;
}
unsigned long long flash_get_capacity()
{
	unsigned long long capacity = 0;
	DISK_Ioctl(DISK_DEVICE_NAND, DEV_GET_DISK_SIZE, (void*)&capacity);
	return capacity;
	
}

