#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


#define FILE_OFFSET_BASE_ADDR			0X60
#define FILE_OFFSET_INIT_CODE			0X70

#define ROM_TNFTL_STRUCTURE_VERSION_OFFSET		0x78
#define ROM_BL1_NAND_OFFSET						0x7C

#define DEBUG 

#if defined(DEBUG)
#define DBG 	printf
#else 
#define DBG
#endif

#define BOOT_MAGIC "ANDROID@"
#define SECURE_TAG "TCSB"
#define ROM_TYPE "LBS_"
#if defined(NAND_USED)
#define STORAGE_TYPE "NAND"
#elif defined(EMMC_USED)
#define STORAGE_TYPE "EMMC"
#endif
#define TCC_TYPE "892X"
#define FWDN_TYPE "V7  "


#if defined(TCC_TRUSTZONE_ENABLE)

#define TZOS_HEADER_SIZE	512

//#define DRAM_SIZE		512
#define TZOS_BASE		0x9FF00000

#endif //END TCC_TRUSTZONE_ENABLE


/*
 * SECURE BOOT HEADER FOR WHEN FIRMWARE DOWNLOAD
 * MAGIC CODE		8 BYTE
 * SECURE TAGS		4 BYTE
 * ROM TYPE		4 BYTE
 * BASE ADDRESS		4 BYTE
 * STORAGE TYPE		4 BYTE
 * FWDN TYPE		4 BYTE
 * TCC TYPE		4 BYTE
 * DRAM INIT START	8 BYTE
 * DRAM INIT SIZE 	8 BYTE
 * BOOTLOADER START	8 BYTE
 * BOOTLOADER SIZE	8 BYTE
 * NAND BL1 START	8 BYTE
 * NAND BL1 SIZE	8 BYTE
#if defined(TCC_TRUSTZONE_ENABLE)
 * TARGET ADDRESS	8 BYTE
 * TZOS START		8 BYTE // for Trust Zone
 * TZOS	SIZE		8 BYTE // for Trust Zone
 * RESERVED		400 BYTE
#else
 * RESERVED			448 BYTE
#endif
 * TOTAL SIZE 		512 BYTE 
 *
 */
struct TCC_SB_HEADER {
	unsigned char magic[8];
	unsigned char sc_tag[4];
	unsigned char rom_type[4];
	unsigned int base_address;
	unsigned char st_type[4];
	unsigned char fwdn_type[4];
	unsigned char tcc_type[4];
	unsigned long long dram_init_start;
	unsigned long long dram_init_size;
	unsigned long long bootloader_start;
	unsigned long long bootloader_size;
	unsigned long long nand_bl1_start;
	unsigned long long nand_bl1_size;
	unsigned long long target_address;
#if defined(TCC_TRUSTZONE_ENABLE)
	unsigned long long revs;
	unsigned long long tzos_start;
	unsigned long long tzos_size;
	unsigned char rev2[400];
#else
	unsigned char rev2[424];
#endif
};

#if 0//defined(TCC_TRUSTZONE_ENABLE)
struct TCC_TZOS_HEADER {
	unsigned int dram_size;
	unsigned int base_addr;
	unsigned char rev[504];
};
#endif

static unsigned long get_file_size(FILE* file)
{
	unsigned long f_len;
	fseek(file , 0L, SEEK_END);
	f_len = ftell(file);
	fseek(file, 0L, SEEK_SET);

	return f_len;
}

int print_ussage()
{
	return -1;
}

static int extract_dram_init(char *infile ,char *outfile)
{
	FILE *fd = fopen(infile , "r");
	FILE *wfd = fopen(outfile, "w");
	unsigned long f_len;
	unsigned char *rBuffer ,*dBuffer; 
	unsigned int sInitCode , eInitCode, baseAddr, sizeInitCode;
	
	f_len = get_file_size(fd);

	rBuffer = malloc(f_len);
	memset(rBuffer , 0x00, f_len);

	if(!fread(rBuffer, sizeof(char), f_len, fd)){
		DBG("ROM FILE READ FAIL\n");
		return -1;
	}

	memcpy((void*)&baseAddr, &rBuffer[FILE_OFFSET_BASE_ADDR],4);
	memcpy((void*)&sInitCode, &rBuffer[FILE_OFFSET_INIT_CODE],4);
	memcpy((void*)&eInitCode, &rBuffer[FILE_OFFSET_INIT_CODE+4],4);

	sizeInitCode = eInitCode - sInitCode;

	DBG("[BASE ADDR : %x] [INIT CODE START : %x] [INIT CODE END : %x] [INIT CODE SIZE : %x]\n"
			,baseAddr, sInitCode, eInitCode, sizeInitCode);

	dBuffer = &rBuffer[sInitCode - baseAddr];

	if(!fwrite(dBuffer, sizeof(char), sizeInitCode, wfd)){
		DBG("DRAM INIT CODE FILE WRITE FAIL\n");
		return -1;
	}

	free(rBuffer);

	return 0;

}

#if defined(NAND_USED)
/******************************************************************************
*
*	int				extract_NAND_BL1
*
*	Input	: infile  - lk bootloader.
*  			  outfile - 1st Bootloader for NAND boot.
*	Output	:
*	Return	:
*
*	Description : extract 1st Bootloader for nand in lk bootloader.
*
*******************************************************************************/
static int extract_NAND_BL1(char *infile ,char *outfile)
{
	FILE *fd = fopen(infile , "r");
	FILE *wfd = fopen(outfile, "w");
	unsigned long f_len;
	unsigned char *rBuffer ,*dBuffer; 
	unsigned int *nTempBuffer;
	unsigned int sBL1Code, sBL1Info, baseAddr, sizeInitCode;
	
	f_len = get_file_size(fd);

	rBuffer = malloc(f_len);
	memset(rBuffer , 0x00, f_len);

	if(!fread(rBuffer, sizeof(char), f_len, fd)){
		DBG("ROM FILE READ FAIL\n");
		return -1;
	}

	memcpy((void*)&baseAddr, &rBuffer[FILE_OFFSET_BASE_ADDR],4);
	memcpy((void*)&sBL1Code, &rBuffer[ROM_BL1_NAND_OFFSET],4);

	nTempBuffer	 = (unsigned int*) rBuffer;
	sizeInitCode = nTempBuffer[ ((sBL1Code - baseAddr)>>2) + 2 ];

	DBG("[BASE ADDR : %x] [BL1 CODE START : %x] [BL1 CODE SIZE : %x]\n"
			, baseAddr, sBL1Code, sizeInitCode);

	dBuffer = &rBuffer[sBL1Code - baseAddr];

	if(!fwrite(dBuffer, sizeof(char), sizeInitCode, wfd)){
		DBG("DRAM INIT CODE FILE WRITE FAIL\n");
		return -1;
	}

	free(rBuffer);

	return 0;

}
#endif

#if defined(TCC_TRUSTZONE_ENABLE)
#if 0
static int make_tzos_image(FILE * tzos)
{
	struct TCC_TZOS_HEADER *tzos_header;
	char* tBuf;

	unsigned int filesize = 0;

	//FILE *fd  = fopen("test.img","w");

	tzos_header = malloc(sizeof(struct TCC_TZOS_HEADER));
	memset(tzos_header , 0x00 , sizeof(struct TCC_TZOS_HEADER));

	filesize = get_file_size(tzos); 

	DBG("File Size : %d \n" ,filesize);

	tBuf = malloc(filesize+sizeof(struct TCC_TZOS_HEADER));
	memset(tBuf, 0x00, filesize+sizeof(struct TCC_TZOS_HEADER));

	tzos_header->dram_size = DRAM_SIZE;
	tzos_header->base_addr = TZOS_BASE;

	memcpy(tBuf,tzos_header, sizeof(struct  TCC_TZOS_HEADER));

	fseek(tzos, 0L, SEEK_SET);
	if(fread(tBuf+sizeof(struct TCC_TZOS_HEADER), sizeof(char), filesize, tzos) < 0)
	{
		DBG("TZOS FILE READ FAILED \n");
		return -1;
	}

	fseek(tzos, 0L, SEEK_SET);

	if(fwrite(tBuf, sizeof(char), get_file_size(tzos) + sizeof(struct TCC_TZOS_HEADER), tzos) < 0)
	{
		DBG("Write TZOS HEADER Error \n");
		return -1;
	}

	free(tBuf); free(tzos_header);

	//fclose(fd);

	return 0;
}
#endif

#define TZOS_MAGIC_1	0x21458E6A
#define TZOS_MAGIC_2	0x94C6289B
#define TZOS_MAGIC_3	0xFA89ED03
#define TZOS_MAGIC_4	0x9968728F
#define TZOS_MAGIC_M	0xA372B85C
static int mend_tzos_image(char *romfile, char * tzos)
{
	unsigned int tzos_magic[4] = {TZOS_MAGIC_M, 0, 0, 0};	//magic, base, offset, size
	struct TCC_TZOS_INFO *tzos_info;
	char* tBuf = 0;
	unsigned int * piBuf = 0;
	int i;

	unsigned int romsize = 0;
	unsigned int tzossize = 0;
	unsigned int lksize = 0;

	FILE* romfd = fopen(romfile, "r+");
	FILE* tzfd = fopen(tzos, "r+");

	romsize = get_file_size(romfd);
	tzossize = get_file_size(tzfd);
	if(romsize > tzossize)
		lksize = romsize -tzossize;
	else
	{
		DBG("[MEND TZOS] romsize(%d) < tzossize(%d) \n", romsize, tzossize);
		goto GOTO_RETURN;
	}

	tBuf = malloc(lksize);
	if(tBuf == 0)
	{
		DBG("[MEND TZOS] malloc Error \n");
		goto GOTO_RETURN;
	}
	memset(tBuf , 0x00 , lksize);
	piBuf = (unsigned int *)tBuf;

	fseek(romfd, 0L, SEEK_SET);
	if(fread(tBuf, sizeof(char), lksize, romfd) < 0)
	{
		DBG("[MEND TZOS] lk file read Error \n");
		return -1;
	}

	for(i=0; i<(lksize-16)/sizeof(int); i++)
	{
		if((piBuf[i] == TZOS_MAGIC_1)&&(piBuf[i+1] == TZOS_MAGIC_2)&&(piBuf[i+2] == TZOS_MAGIC_3)&&(piBuf[i+3] == TZOS_MAGIC_4))
			break;
	}
	if(i < (lksize-16)/sizeof(int))
	{
		//tzos_magic[0] = TZOS_MAGIC_M;
		tzos_magic[1] = TZOS_BASE;
		tzos_magic[2] = lksize;
		tzos_magic[3] = tzossize;

		fseek(romfd, i*sizeof(int), SEEK_SET);

		if(fwrite(&tzos_magic[0], sizeof(char), sizeof(tzos_magic), romfd) < 0)
		{
			DBG("[MEND TZOS] Write TZOS Info Error \n");
			return -1;
		}

		DBG("[MEND TZOS] TRUST ZONE [TZOS BASE : 0x%08X] [LK SIZE : %8X] [TZOS SIZE : %8X]\n"
				,TZOS_BASE, lksize, tzossize);
	}
	else
	{
		DBG("[MEND TZOS] MAGIC not found ! \n");
	}

GOTO_RETURN:
	if(tBuf != 0)
		free(tBuf);

	fclose(romfd);
	fclose(tzfd);

	return 0;
}
#endif

#if defined(NAND_USED)
/******************************************************************************
*
*	int				make_header
*
*	Input	: secure_dram  - lk bootloader.
*  			  outfile - 1st Bootloader for NAND boot.
*	Output	:
*	Return	:
*
*	Description : extract 1st Bootloader for nand in lk bootloader.
*
*******************************************************************************/
#if 0//defined(TCC_TRUSTZONE_ENABLE)
static int make_header(char *romfile, char* normal_bl1, char *secure_dram ,char *secure_bootloader, char *secure_bl1, char *tzos, char *outfile)
#else
static int make_header(char *romfile, char* normal_bl1, char *secure_dram ,char *secure_bootloader, char *secure_bl1, char *outfile)
#endif
#else
#if 0//defined(TCC_TRUSTZONE_ENABLE)
static int make_header(char *romfile, char *infile1 ,char *infile2, char *tzos, char *outfile)
#else
static int make_header(char *romfile, char *infile1 ,char *infile2, char *outfile)
#endif
#endif
{
	struct TCC_SB_HEADER *header;
	unsigned int base_address, target_address;
	FILE *romfd = fopen(romfile , "r");

	#if defined(NAND_USED)
	FILE *dfd = fopen(secure_dram , "r");
	FILE *bfd = fopen(secure_bootloader, "r");
	FILE *nfd = fopen(secure_bl1 , "r");
	FILE *fp_normal_bl1 = fopen(normal_bl1, "r");
	#else
	FILE *dfd = fopen(infile1 , "r");
	FILE *bfd = fopen(infile2, "r");
	#endif	
	FILE *hfd = fopen(outfile, "w");

	#if 0//defined(TCC_TRUSTZONE_ENABLE)
	FILE* tzfd = fopen(tzos, "r+");
	#endif

	header = malloc(sizeof(struct TCC_SB_HEADER));

	memset(header , 0x0 , sizeof(struct TCC_SB_HEADER));

	fseek(romfd, FILE_OFFSET_BASE_ADDR, SEEK_SET);
	if(!fread(&base_address, sizeof(unsigned int), 1, romfd)){
		DBG("ROM FILE READ FAIL\n");
		return -1;
	}

	memcpy(header->magic, BOOT_MAGIC, sizeof(header->magic));
	memcpy(header->sc_tag, SECURE_TAG, sizeof(header->sc_tag));
	memcpy(header->rom_type, ROM_TYPE, sizeof(header->rom_type));
	header->base_address = base_address;
	memcpy(header->st_type, STORAGE_TYPE, sizeof(header->st_type));
	memcpy(header->fwdn_type, FWDN_TYPE, sizeof(header->fwdn_type));
	memcpy(header->tcc_type, TCC_TYPE, sizeof(header->tcc_type));
	header->dram_init_start = sizeof(struct TCC_SB_HEADER);
	header->dram_init_size = get_file_size(dfd);
	header->bootloader_start = (header->dram_init_size + sizeof(struct TCC_SB_HEADER));
	header->bootloader_size = get_file_size(bfd);	
	#if defined(NAND_USED)	
	header->nand_bl1_start = header->bootloader_start + header->bootloader_size;
	header->nand_bl1_size = get_file_size(nfd);

	fseek( fp_normal_bl1, 4, SEEK_SET );
	fread( &target_address, sizeof(unsigned int), 1, fp_normal_bl1 );
	header->target_address = target_address;
	#if 0//defined(TCC_TRUSTZONE_ENABLE)
	if(make_tzos_image(tzfd) < 0)
		return -1;
	header->tzos_start = header->nand_bl1_start + header->nand_bl1_size;
	header->tzos_size = get_file_size(tzfd);
	#endif
	#else
	header->nand_bl1_start	= 0;
	header->nand_bl1_size	= 0;	
	header->target_address	= 0;
	#if 0//defined(TCC_TRUSTZONE_ENABLE)
	if(make_tzos_image(tzfd) < 0)
		return -1;
	header->tzos_start = header->bootloader_start + header->bootloader_size;
	header->tzos_size = get_file_size(tzfd);
	#endif
	#endif

	#if 0//defined(TCC_TRUSTZONE_ENABLE)
	DBG("TRUST ZONE [INIT CODE START : %llx] [INIT CODE SIZE : %llx] [BOOTLOADER START : %llx] [BOOTLOADER SIZE : %llx] [BL1 START:%llx] [BL1 SIZE:%llx] [TZOS START : %llx] [TZOS SIZE : %llx] [BaseAddress:0x%08X] [TargetAddress: 0x%08X]\n"
			,header->dram_init_start, header->dram_init_size, header->bootloader_start, header->bootloader_size, header->nand_bl1_start, header->nand_bl1_size, header->tzos_start, header-> tzos_size, header->base_address, header->target_address);
	#else
	DBG("[INIT CODE START : %llx] [INIT CODE SIZE : %llx] [BOOTLOADER START : %llx] [BOOTLOADER SIZE : %llx] [BL1 START:%llx] [BL1 SIZE:%llx] [BaseAddress:0x%08X] [TargetAddress: 0x%08X]\n"
			,header->dram_init_start, header->dram_init_size, header->bootloader_start, header->bootloader_size, header->nand_bl1_start, header->nand_bl1_size, header->base_address, header->target_address);
	#endif


	if(!fwrite(header, sizeof(char), 512, hfd)){
		DBG("Header FILE WRITE FAIL\n");
		return -1;
	}

	free(header);

	#if defined(NAND_USED)
	fclose(dfd);fclose(bfd);fclose(nfd);fclose(fp_normal_bl1);
	#else
	fclose(dfd); fclose(bfd);
	#endif	
	fclose(hfd);

	#if 0//defined(TCC_TRUSTZONE_ENABLE)
	fclose(tzfd);
	#endif

	return 0;
}

main(int argc, char *argv[])
{
	#if defined(NAND_USED)
	int nArgNum = 5;
	#else
	int nArgNum = 4;
	#endif

	if(argc < 3){
		return print_ussage();
	}
	
	if(argc == nArgNum){
		if(!strcmp("extract", argv[1])){
			extract_dram_init(argv[2], argv[3]);
			#if defined(NAND_USED)
			extract_NAND_BL1(argv[2], argv[4]);			
			#endif
		}		
	}

	if(argc > nArgNum){
		if(!strcmp("mkheader", argv[1])){
			#if defined(NAND_USED)
			#if 0//defined(TCC_TRUSTZONE_ENABLE)
			make_header(argv[2] , argv[3] , argv[4], argv[5], argv[6], argv[7], argv[8]);
			#else
			make_header(argv[2] , argv[3] , argv[4], argv[5], argv[6], argv[7]);
			#endif
			#else
			#if 0//defined(TCC_TRUSTZONE_ENABLE)
			make_header(argv[2] , argv[3] , argv[4], argv[5], argv[6]);
			#else
			make_header(argv[2] , argv[3] , argv[4], argv[5]);
			#endif
			#endif
		}
	}

	#if defined(TCC_TRUSTZONE_ENABLE)
	if(argc == 4){
		if(!strcmp("tzmend", argv[1])){
			mend_tzos_image(argv[2], argv[3]);
		}		
	}
	#endif

	return 0;

}
