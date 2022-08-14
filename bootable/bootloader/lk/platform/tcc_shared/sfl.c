/*
 * Copyright (c) 2013 Telechips, Inc.
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


#include<debug.h>
#include<sfl.h>
#include<sdmmc/emmc.h>

int tcc_read(unsigned long long data_addr, void* in, unsigned int data_len)
{
	int ret;
#if _EMMC_BOOT
	ret = emmc_read(BYTE_TO_SECTOR(data_addr), data_len, in);
#else
	ret = flash_read_tnftl_v8(BYTE_TO_SECTOR(data_addr), data_len, in);
#endif

	if(ret < 0)
		dprintf(INFO, "[STORAGE] Read Fail Form Target Storage \n");

	return ret;
}

int tcc_write(char* write_target, unsigned long long data_addr, unsigned int data_len, void* out)
{
	int ret;
#if _EMMC_BOOT
	ret = emmc_write(write_target, BYTE_TO_SECTOR(data_addr), data_len, out);
#else
	ret = flash_write_tnftl_v8(write_target, BYTE_TO_SECTOR(data_addr), data_len, out);
#endif

	if(ret < 0)
		dprintf(INFO, "[STORAGE] Read Fail Form Target Storage \n");

	return ret;
}

unsigned int write_partition(unsigned size, unsigned char *partition)
{
	dprintf(CRITICAL, "Not Support Write partition image !! \n");
}

unsigned long long tcc_get_storage_capacity()
{
	unsigned long long ret;
#if _EMMC_BOOT
	ret = emmc_get_capacity();
#else
	ret = flash_get_capacity();
#endif
	if(ret == 0){
		dprintf(INFO, "[STORAGE] Get Storage Capacity Failed \n");
		return 0;
	}
	return ret;
}
