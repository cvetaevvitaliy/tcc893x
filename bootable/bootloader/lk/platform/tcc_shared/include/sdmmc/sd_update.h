void mmc_init_page( 
		page_t *p, 
		unsigned long start_addr,
		unsigned long size,
		unsigned long hidden,
		unsigned char *buf,
		unsigned int option);

int mmc_update_rom_code(
		unsigned long rom_size,
		unsigned long config_size,
		unsigned char *buf,
		unsigned int boot_partition);

int mmc_update_config_code(
		unsigned long rom_size,	//ks_8930
		unsigned long config_size,
		unsigned char *buf,
		unsigned int boot_partition);

int mmc_update_header (
		unsigned long firmware_base,
		unsigned long config_code_size,
		unsigned long rom_code_size,
		unsigned int boot_partition,
		int fwdn_mode);

int mmc_update_valid_test(unsigned int rom_size);
int mmc_get_serial_num(unsigned char* serial_num, unsigned int boot_partition);

#if defined(TSBM_ENABLE) || defined(OTP_UID_INCLUDE)
int init_secure_head(BOOTSD_sHeader_Info_T *h, unsigned char* buf);
int mmc_update_secure_bootloader(unsigned int rom_size, int boot_partition);
#endif









