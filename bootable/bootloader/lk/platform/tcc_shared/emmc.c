#include <sdmmc/emmc.h>
#include <sdmmc/sd_bus.h>
#include <debug.h>
#include <string.h>

#include <partition_parser.h>

void emmc_boot_main()
{
	ioctl_diskinfo_t disk_info;

	dprintf(INFO , "%s:init start from eMMC\n" , __func__ );
	SD_BUS_Initialize();
	DISK_Ioctl(DISK_DEVICE_TRIFLASH , DEV_INITIALIZE , NULL);
	DISK_Ioctl(DISK_DEVICE_TRIFLASH , DEV_GET_DISKINFO , (void*)&disk_info);

	read_partition_tlb();
	partition_dump();
}


#if defined(FWDN_HIDDEN_KERNEL_MODE)
unsigned int emmc_boot_write(unsigned int data_len, unsigned int* data)
{
	ioctl_diskrwpage_t rwHiddenPage;

	dprintf(INFO, "boot write\n");

	rwHiddenPage.start_page =0;
	rwHiddenPage.rw_size = data_len / 512;
	rwHiddenPage.hidden_index = 0;
	rwHiddenPage.buff = (unsigned char*)data;

	return DISK_Ioctl(DISK_DEVICE_TRIFLASH , DEV_HIDDEN_WRITE_PAGE, (void*)&rwHiddenPage); 
}
#endif

unsigned int emmc_write(char* write_target, unsigned long long data_addr, unsigned int data_len, void* in)
{
	int val = 0;
  if (!strcmp(write_target, "bootloader")) {
#if defined(TSBM_ENABLE)
    	val = mmc_update_secure_bootloader( data_len, 0 );
    	val = mmc_update_secure_bootloader( data_len, 1 );
#else
    	val = mmc_update_bootloader( data_len, 0 );
    	val = mmc_update_bootloader( data_len, 1 );
#endif
  } else if (!strcmp(write_target, "fastboot_erase")) {
        unsigned long erase_start_addr = (unsigned long)data_addr;
        printf("[fastboot_erase] erase_start_addr : %ld, data_len : %d\n", erase_start_addr, data_len);
        return erase_emmc(erase_start_addr, data_len, 0);
  } else {
	unsigned int pageCount = 0;
	unsigned long data_h = (unsigned long)(data_addr>>32);
	unsigned long data_l = (unsigned long)(data_addr&0xFFFFFFFF);

	pageCount = (data_len +511) / 512;
	if(pageCount)
		val = BOOTSD_Write(SD_BUS_GetBootSlotIndex(), data_addr, pageCount, in);
  }
	return val;
}

unsigned int emmc_read(unsigned long long data_addr , unsigned data_len , void* in)
{
	int val = 0;
	unsigned int pageCount = 0;
	unsigned long data_h = (unsigned long)(data_addr>>32);
	unsigned long data_l = (unsigned long)(data_addr&0xFFFFFFFF);

	pageCount = (data_len + 511) / 512;

	if(pageCount)
		val = BOOTSD_Read(SD_BUS_GetBootSlotIndex(), data_addr, pageCount, in);

	return val;
}

int get_emmc_serial(char* Serial)
{
	ioctl_diskinfo_t disk_info;
	ioctl_diskrwpage_t bootCodeRead;
	unsigned char readData[512];
	int res;
	int i;

	if(DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_GET_DISKINFO , (void*)&disk_info) <0)
		return -1;

	if(disk_info.sector_size != 512)
		return -1;

	bootCodeRead.start_page = BOOTSD_GetHeaderAddr(0);
	bootCodeRead.rw_size = 1;
	bootCodeRead.buff  = readData;
	bootCodeRead.boot_partition  = 0;
	res = DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_BOOTCODE_READ_PAGE, (void*)&bootCodeRead);
	if(res !=0) return -1;

	for(i=0; i<16; i++)
		Serial[i] = ((BOOTSD_Header_Info_T*)readData)->ucSerialNum[i];
	for(i=16 ; i<32; i++)
		Serial[i]= ((BOOTSD_Header_Info_T*)readData)->ucSerialNum[i+16];

	return 0;
}
unsigned long long emmc_get_capacity()
{
	unsigned long long capacity = 0;
	DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_GET_DISK_SIZE, (void*)&capacity);
	return capacity;
}
