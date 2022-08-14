/****************************************************************************
 *   FileName    : Fwdn_drv_v7.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

/****************************************************************************

  Revision History

 ****************************************************************************

 ****************************************************************************/
#if defined(FWDN_V7)

#if defined(_LINUX_)
//#include "common.h"
#endif

#if defined(_LINUX_) || defined(_WINCE_)
#include <fwdn/fwupgrade.h>
#include <fwdn/Disk.h>
#include <fwdn/TC_File.h>
#include <fwdn/fwdn_drv_v7.h>
#include "nand_drv_v8.h"
#endif
#include <sdmmc/sd_memory.h>
#include <debug.h>

#if defined(SECURE_KEYBOX_INCLUDE)
#include <tcsb/keybox.h>
#endif
#if defined(SECURE_UID_INCLUDE) || defined(OTP_UID_INCLUDE)
#include <tcsb/uid.h>
#endif
#if defined(OTP_UID_INCLUDE)
extern uint tzow_api(uint nMHz, uchar *pucData, uint owsize);
#endif

#if !defined(_LINUX_)
#include "memory.h"
#endif

#if defined(_WINCE_)
#define FWDN_TRACE				KITLOutputDebugString
#elif defined(_LINUX_)
#define FWDN_TRACE(fmt...)				dprintf(INFO,##fmt)
#endif

extern void target_reinit(int);

#define	byte_of(X)					( *(volatile unsigned char *)((X)) )

#define TRANSFER_UNIT_SIZE			(64*512)
#define TRANSFER_UNIT_SECTOR_SIZE	(TRANSFER_UNIT_SIZE>>9)
#define Tbuffersize  				(128*512)	// It should be larger then TRANSFER_UNIT_SECTOR_SIZE
#define TbufferSectorSize			(Tbuffersize>>9)

static unsigned int fwdndBuf[Tbuffersize/4];
static unsigned int veriBuf[Tbuffersize/4];

#if defined(TSBM_ENABLE)
extern unsigned int get_tcc_mem_freq(void);
#endif

//==============================================================
//
//		Global Variables
//
//==============================================================
static const unsigned char gFWDN_FirmwareStorage		=
	#if _EMMC_BOOT
		FWDN_DISK_TRIFLASH
	#else
		#ifdef SNOR_BOOT_INCLUDE
		FWDN_DISK_SNOR
		#elif defined(HDD_BOOT_INCLUDE)
		FWDN_DISK_NOR
		#else
		FWDN_DISK_NAND
		#endif
	#endif
;

unsigned int					g_uiFWDN_OverWriteSNFlag;
unsigned int					g_uiFWDN_WriteSNFlag;
FWDN_DEVICE_INFORMATION			FWDN_DeviceInformation;
static NAND_DEVICE_INFO			gNAND_DEVICE_INFO;

static FXN_FWDN_DRV_FirmwareWrite_ReadFromHost		gfxnFWDN_DRV_FirmwareWrite_ReadFromHost;
static FXN_FWDN_DRV_Progress		gfxnFWDN_DRV_Progress = NULL;

unsigned int				gFWDN_DRV_ErrorCode;

#define SESSION_FLAG_MTD_MASK				(0x00000001)
#define SESSION_FLAG_MTD					(0x00000001)
static unsigned int				s_FWDN_DRV_SessionFlag=0xFFFFFFFF;

// NAND's each disk's name
#if defined(NAND_KERNEL_INCLUDE)
static const char FWDN_NAND_AREA_0_NAME[]	= "KERNEL";
#else
static const char FWDN_NAND_AREA_0_NAME[]	= "NAND RO 0";
#endif
#if defined(_WINCE_) && defined(NAND_KERNEL_INCLUDE)
static const char FWDN_NAND_AREA_1_NAME[]	= "LOGO";
#else
static const char FWDN_NAND_AREA_1_NAME[]	= "NAND RO 1";
#endif
static const char FWDN_NAND_AREA_2_NAME[]	= "NAND RO 2";
static const char FWDN_NAND_AREA_3_NAME[]	= "NAND RO 3";
#if defined(SECURE_KEYBOX_INCLUDE)
static const char FWDN_NAND_AREA_4_NAME[]	= "KEY STORE RO";
static const char FWDN_NAND_AREA_5_NAME[]	= "KEY STORE RW";
#else
static const char FWDN_NAND_AREA_4_NAME[]	= "NAND RW 0";
static const char FWDN_NAND_AREA_5_NAME[]	= "NAND RW 1";
#endif
#if defined(SECURE_UID_INCLUDE)
static const char FWDN_NAND_AREA_6_NAME[]	= "UID";
#else
static const char FWDN_NAND_AREA_6_NAME[]	= "NAND RW 2";
#endif
static const char FWDN_NAND_AREA_7_NAME[]	= "NAND RW 3";
static const char FWDN_NAND_AREA_8_NAME[]	= "NAND RW 4";
static const char FWDN_NAND_AREA_9_NAME[]	= "NAND Data";
static const char *FWDN_NAND_AREA_NAME[] = {
	FWDN_NAND_AREA_0_NAME,
	FWDN_NAND_AREA_1_NAME,
	FWDN_NAND_AREA_2_NAME,
	FWDN_NAND_AREA_3_NAME,
	FWDN_NAND_AREA_4_NAME,
	FWDN_NAND_AREA_5_NAME,
	FWDN_NAND_AREA_6_NAME,
	FWDN_NAND_AREA_7_NAME,
	FWDN_NAND_AREA_8_NAME,
	FWDN_NAND_AREA_9_NAME,
};

#if defined(BOOTSD_KERNEL_INCLUDE)
static const char FWDN_SD_HD0_AREA_NAME[]	= "KERNEL";
#else
#if defined(SECURE_KEYBOX_INCLUDE)
static const char FWDN_SD_HD0_AREA_NAME[]	= "KEY STORE RO";
#else
static const char FWDN_SD_HD0_AREA_NAME[]	= "SD Hidden 0";
#endif
#endif
#if defined(BOOTSD_KERNEL_INCLUDE)
static const char FWDN_SD_HD1_AREA_NAME[]	= "LOGO";
#else
#if defined(SECURE_KEYBOX_INCLUDE)
static const char FWDN_SD_HD1_AREA_NAME[]	= "KEY STORE RW";
#else
static const char FWDN_SD_HD1_AREA_NAME[]	= "SD Hidden 1";
#endif
#endif
#if defined(SECURE_UID_INCLUDE)
static const char FWDN_SD_HD2_AREA_NAME[]	= "UID";
#else
static const char FWDN_SD_HD2_AREA_NAME[]	= "SD Hidden 2";
#endif
static const char FWDN_SD_HD3_AREA_NAME[]	= "SD Hidden 3";
static const char FWDN_SD_HD4_AREA_NAME[]	= "SD Hidden 4";
static const char FWDN_SD_HD5_AREA_NAME[]	= "SD Hidden 5";
static const char FWDN_SD_HD6_AREA_NAME[]	= "SD Hidden 6";
static const char FWDN_SD_HD7_AREA_NAME[]	= "SD Hidden 7";
static const char FWDN_SD_HD8_AREA_NAME[]	= "SD Hidden 8";
static const char FWDN_SD_HD9_AREA_NAME[]	= "SD Hidden 9";
static const char FWDN_SD_HD10_AREA_NAME[]	= "SD Hidden 10";
static const char *FWDN_SD_HD_AREA_NAME[] = {
	FWDN_SD_HD0_AREA_NAME,
	FWDN_SD_HD1_AREA_NAME,
	FWDN_SD_HD2_AREA_NAME,
	FWDN_SD_HD3_AREA_NAME,
	FWDN_SD_HD4_AREA_NAME,
	FWDN_SD_HD5_AREA_NAME,
	FWDN_SD_HD6_AREA_NAME,
	FWDN_SD_HD7_AREA_NAME,
	FWDN_SD_HD8_AREA_NAME,
	FWDN_SD_HD9_AREA_NAME,
	FWDN_SD_HD10_AREA_NAME,
};
static const char FWDN_SD_DATA_AREA_NAME[]	= "SD Data";

MMC_DISK_INFO_T		triflashDiskInfo;

#if defined(OTP_UID_INCLUDE)
static const char FWDN_FAT_ONLY_DATA_AREA_NAME[] = "FAT_ONLY";
static const char FWDN_OTP_DATA_AREA_NAME[] = "OTP Data";
#endif

//==============================================================
//
//		External Variables
//
//==============================================================
extern unsigned long				FirmwareSize;

//extern unsigned long				ImageRwBase;

//extern char							FirmwareVersion[];

extern unsigned int					gMAX_ROMSIZE;
extern const unsigned int			CRC32_TABLE[];
//==============================================================
//
//		External Functionss
//
//==============================================================
#define FWDN_DRV_MESSAGE_SIZE_MAX		200
#define FWDN_DRV_MESSAGE_LINE_MAX		5
static unsigned short sFWDN_DRV_usMessageSize[FWDN_DRV_MESSAGE_LINE_MAX];
static unsigned char sFWDN_DRV_ucMessageLinePush;
static unsigned char sFWDN_DRV_ucMessageLinePop;
static unsigned int sFWDN_DRV_pMessage[FWDN_DRV_MESSAGE_LINE_MAX][FWDN_DRV_MESSAGE_SIZE_MAX/4];
void FWDN_DRV_ClearMessage(void)
{
	sFWDN_DRV_ucMessageLinePush = 0;
	sFWDN_DRV_ucMessageLinePop = 0;
}

int FWDN_DRV_AddMessage(const char *pErrMsg)
{
	if(sFWDN_DRV_ucMessageLinePush<FWDN_DRV_MESSAGE_LINE_MAX)
	{
		strncpy((char*)sFWDN_DRV_pMessage[sFWDN_DRV_ucMessageLinePush],pErrMsg,FWDN_DRV_MESSAGE_SIZE_MAX-1);
		sFWDN_DRV_usMessageSize[sFWDN_DRV_ucMessageLinePush] = strlen((char*)sFWDN_DRV_pMessage[sFWDN_DRV_ucMessageLinePush]) + 1;
		sFWDN_DRV_ucMessageLinePush++;
		return TRUE;
	}
	else
		return FALSE;
}

int FWDN_DRV_GetMessage(unsigned int **ppMessage, unsigned int *pLength)
{
	if(sFWDN_DRV_ucMessageLinePop<sFWDN_DRV_ucMessageLinePush)
	{
		*pLength = (unsigned int)sFWDN_DRV_usMessageSize[sFWDN_DRV_ucMessageLinePop];
		*ppMessage = sFWDN_DRV_pMessage[sFWDN_DRV_ucMessageLinePop++];
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void FWDN_DRV_Reset(void)
{
	// goto reset vector

	FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_RESET_NOT_SUPPORT);
}

int FWDN_DRV_SessionStart(void)
{
	FWDN_TRACE("===== FWDN Session Start! =====\n");

	s_FWDN_DRV_SessionFlag = 0x00000000;

	return TRUE;
}

int FWDN_DRV_SessionEnd(unsigned int bSuccess)
{
	if (!target_is_emmc_boot())
		NAND_RO_Area_Flush_WriteCache(0);

	if(bSuccess)
		FWDN_TRACE("===== FWDN Session End! =====\n");
	else
		FWDN_TRACE("===== FWDN Session is Failed!=====\n");

	s_FWDN_DRV_SessionFlag = 0xFFFFFFFF;

	return TRUE;
}

static void FWDN_DRV_NAND_ProgressHandler( unsigned int uiCurrent, unsigned int uiTotal )
{
	if(gfxnFWDN_DRV_Progress != NULL)
	{
		(*gfxnFWDN_DRV_Progress)( ( uiCurrent * 100 ) / uiTotal );
	}
}

int FWDN_DRV_Init(unsigned int bmFlag, const FXN_FWDN_DRV_Progress fxnFwdnDrvProgress, char *message, unsigned int messageSize)
{
	int res = 0;

	g_uiFWDN_OverWriteSNFlag = 0;

	gfxnFWDN_DRV_Progress = fxnFwdnDrvProgress;

	if(bmFlag & FWDN_DEVICE_INIT_BITMAP_RESERVED_MASK) {
		FWDN_DRV_AddMessage("Unknown Flag.");
		return -1;
	}

	if (target_is_emmc_boot()) {
		if(DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_INITIALIZE, NULL ) < 0 ) {
			FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_IOCTRL_DEV_INITIALIZE);
			res = -1;
		}

		if( bmFlag & FWDN_DEVICE_INIT_BITMAP_LOW_FORMAT ) {
#if _EMMC_BOOT
			if( erase_emmc(0, 0, 1) )
				printf("eMMC low format error has occurred!\n");
			else
				printf("eMMC erasing completed!\n");
#endif
		}
	}
	else {
		if(bmFlag & FWDN_DEVICE_INIT_BITMAP_DEBUG_LEVEL_FORMAT_MASK) {
			switch(bmFlag&FWDN_DEVICE_INIT_BITMAP_DEBUG_LEVEL_FORMAT_MASK) {
				case FWDN_DEVICE_INIT_BITMAP_DEBUG_LEVEL_FORMAT_ERASE_ONLY:
					FWDN_DRV_AddMessage("Format - Erase Only");
					res = NAND_Init(0,NAND_INIT_TYPE_LOWFORMAT_DEBUG_ERASE_ONLY,FWDN_DRV_NAND_ProgressHandler);
					break;
				default:
					FWDN_DRV_AddMessage("Unknown Init Flag.");
					return -1;
					break;
			}
		}
		else if(bmFlag & FWDN_DEVICE_INIT_BITMAP_UPDATE) {
			res = NAND_Init(0,NAND_INIT_TYPE_LOWFORMAT,FWDN_DRV_NAND_ProgressHandler);
		}
		else if(bmFlag & FWDN_DEVICE_INIT_BITMAP_DUMP) {
		}
		else {
			NAND_ERROR_T errNAND = NAND_Init(0,NAND_INIT_TYPE_NORMAL,FWDN_DRV_NAND_ProgressHandler);
			if(errNAND == NAND_SUCCESS)
				res = 0;
			else if(errNAND == NAND_ERROR_INIT_AREA_CHANGE) {
				memcpy(message, "NAND partition changes is detected.\nIt will erase whole NAND.", messageSize);
				message[messageSize-1] = 0;
				res = 1;
			}
			else {
				FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_IOCTRL_DEV_INITIALIZE);
				res = -1;
			}
		}
	}

	return res;
}

static void _loc_strncpy(char *dst, const char *src, unsigned int n)
{
	while(n--) {
		*dst++=*src;
		if(*src++==0)
			break;
	}	
}

static int _loc_strcmp(const char *dst, const char *src)
{
	while( *dst!=0 || *src!=0 )
	{
		if( *dst++ != *src++ )
			return -1;
	}
	return 0;
}

static void _loc_register_area(unsigned int index, const char *name, unsigned int nSector)
{
	memset(FWDN_DeviceInformation.area[index].name,0,16);
	_loc_strncpy(FWDN_DeviceInformation.area[index].name,name,16);
	FWDN_DeviceInformation.area[index].nSector = nSector;
}

//static void _loc_register_default_disk(unsigned int diskID)
//{
//	_loc_register_disk(0,diskID);
//}

pFWDN_DEVICE_INFORMATION FWDN_DRV_GetDeviceInfo(void)
{
#if defined (NKUSE)
	memset(&FWDN_DeviceInformation,0,sizeof(FWDN_DeviceInformation));
#else
	unsigned int i, nDisk=0, iSize=0;

	memset(&FWDN_DeviceInformation,0,sizeof(FWDN_DeviceInformation));

	switch(gFWDN_FirmwareStorage)
	{
		case FWDN_DISK_NAND:
			iSize = FwdnGetNandSerial(FWDN_DeviceInformation.DevSerialNumber,sizeof(FWDN_DeviceInformation.DevSerialNumber));
			if(iSize == 32)
				FWDN_DeviceInformation.DevSerialNumberType = SN_VALID_32;
			else
				FWDN_DeviceInformation.DevSerialNumberType = SN_NOT_EXIST;
			break;
		case FWDN_DISK_TRIFLASH:
			fwdn_mmc_get_serial_num();
			break;
		case FWDN_DISK_HDD:
			FwdnGetNorSerial();
			break;
		case FWDN_DISK_SNOR:
			FwdnGetSerialNorSerial();
			break;
	}

	// Reset All Area Information
	for(i=0;i<FWDN_AREA_LIST_MAX;i++)
		_loc_register_area(i,"",0);

	if (target_is_emmc_boot()) {
		unsigned long 	ulDataTotalSector;
		unsigned long	ulHiddenCount;

		DISK_Ioctl(DISK_DEVICE_TRIFLASH,DEV_GET_HIDDEN_COUNT, (void *)&ulHiddenCount);
		for(i=0;i<ulHiddenCount;i++) {
			ioctl_diskhidden_t stDiskHidden;
			stDiskHidden.ulIndex = (int)i;
			stDiskHidden.ulTotalSector = 0;
			DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_GET_HIDDEN_SIZE, (void *)&stDiskHidden);

			if(stDiskHidden.ulTotalSector) {
				_loc_register_area(
								nDisk++,
								FWDN_SD_HD_AREA_NAME[i],
								stDiskHidden.ulTotalSector
				);
			}
		}

		//SD data area total size
		DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_GET_DISK_SIZE, (void *)&ulDataTotalSector);
	
		_loc_register_area(
						nDisk++,
						FWDN_SD_DATA_AREA_NAME,
#if 0
#########
#if !defined(BOOT_USING_MBR) 
						ulDataTotalSector + 1 // because of sd/mmc sector count == > EXTCSD.sectorcount -1
#else						
						ulDataTotalSector
#endif
#endif
						ulDataTotalSector

		);
	}
	else {
		for(i=0; i<NAND_Area_GetCount(0);i++) {
			unsigned int uiTotalSector = NAND_Area_GetTotalSectorCount(0, i);
			if(uiTotalSector != 0) {
				_loc_register_area(
								nDisk++,
								FWDN_NAND_AREA_NAME[i],
								uiTotalSector
				);
			}
		}
	}

#ifdef OTP_UID_INCLUDE
	_loc_register_area(
			nDisk++,
			FWDN_FAT_ONLY_DATA_AREA_NAME,
			0
			);

	_loc_register_area(
					nDisk++,
					FWDN_OTP_DATA_AREA_NAME,
					0
	);
#endif

#endif
	return &FWDN_DeviceInformation;
}

int	FWDN_DRV_SerialNumberWrite(unsigned char *serial, unsigned int overwrite)
{
	int	res	= -1;

	switch (gFWDN_FirmwareStorage) {
		case FWDN_DISK_NAND:
			res = FwdnSetNandSerial( serial, overwrite);
			break;
		case FWDN_DISK_TRIFLASH:
			res = FwdnSetBootSDSerial( serial, overwrite);
			break;
		case FWDN_DISK_HDD:
			res = FwdnSetNorSerial( serial, overwrite);
			break;
		case FWDN_DISK_SNOR:
			res = FwdnSetSerialFlashSerial( serial, overwrite);
			break;
	}

	if ( res == 0)
		return TRUE;
	else
		return FALSE;
}

#if defined(OTP_UID_INCLUDE)
extern unsigned int TARGET_ADDR; 
extern unsigned int SECURE_LK_SIZE;
#endif
int FWDN_DRV_FirmwareWrite(unsigned int fwSize, FXN_FWDN_DRV_FirmwareWrite_ReadFromHost fxnFWDN_DRV_FirmwareWrite_ReadFromHost)
{
	int ret = 0;
	gfxnFWDN_DRV_FirmwareWrite_ReadFromHost = fxnFWDN_DRV_FirmwareWrite_ReadFromHost;

	switch (gFWDN_FirmwareStorage)
	{
		case FWDN_DISK_NAND:
		{
			unsigned char *pucRomBuffer = (unsigned char*)FWUG_GetTempBuffer(fwSize);
			if(pucRomBuffer == NULL)
				return ERR_FWUG_FAIL_MEMORY_ALLOCATION;
#if defined(OTP_UID_INCLUDE)
			return 	FwdnWriteNandFirmware(TARGET_ADDR, SECURE_LK_SIZE);
#else
			if(FWDN_DRV_FirmwareWrite_Read(0, pucRomBuffer, fwSize, 100))
					return ERR_FWUG_FAIL_COMMUNICATION;
			return 	FwdnWriteNandFirmware(pucRomBuffer,fwSize);
#endif
		}
		case FWDN_DISK_TRIFLASH:
#if defined(TSBM_ENABLE)
			ret = fwdn_mmc_update_secure_bootloader( fwSize, 0 );
			ret = fwdn_mmc_update_secure_bootloader( fwSize, 1 );
#elif defined(OTP_UID_INCLUDE)
			ret = fwdn_mmc_otp_uid_include_update_bootloader( 0, 0 );
			ret = fwdn_mmc_otp_uid_include_update_bootloader( 0, 1 );
#else
			ret = fwdn_mmc_update_bootloader( fwSize, 0 );
			ret = fwdn_mmc_update_bootloader( fwSize, 1 );
#endif
			return ret;
		case FWDN_DISK_HDD:
		case FWDN_DISK_NOR:
			return	FwdnWriteNorFlashFirmware(fwSize);
		case FWDN_DISK_SNOR:
			return	FwdnWriteSerialNorFirmware(fwSize);
	}

	return	0;
}

int FWDN_DRV_FirmwareWrite_Read(unsigned int addr, unsigned char *buff, unsigned int size, unsigned int percent)
{
	return (*gfxnFWDN_DRV_FirmwareWrite_ReadFromHost)(buff, size, addr, percent);
}

int _loc_AreaRead(char *name, unsigned int lba, unsigned short nSector, void *buff)
{
	unsigned int i;

	if (target_is_emmc_boot()) {
		ioctl_diskrwpage_t rwHiddenPage;
		
		if( _loc_strcmp(FWDN_SD_DATA_AREA_NAME,name) == 0 
#if defined(OTP_UID_INCLUDE)
				|| _loc_strcmp(FWDN_FAT_ONLY_DATA_AREA_NAME,name) == 0
#endif
				)
			return DISK_ReadSector(DISK_DEVICE_TRIFLASH, 0, lba, nSector, buff);

		for( i = 0; i < BOOTSD_HIDDEN_MAX_COUNT; i++ ) {
			if( _loc_strcmp(FWDN_SD_HD_AREA_NAME[i],name) == 0 ) {
				rwHiddenPage.start_page		= lba;
				rwHiddenPage.rw_size		= nSector;
				rwHiddenPage.hidden_index	= i;
				rwHiddenPage.buff			= buff;
				return DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_HIDDEN_READ_PAGE, (void *) &rwHiddenPage);
			}
		}
	}
	else {
		for(i=0 ; i<NAND_Area_GetCount(0); i++) {
#if defined(OTP_UID_INCLUDE)
			if (_loc_strcmp(FWDN_FAT_ONLY_DATA_AREA_NAME,name) == 0)
				strcpy(name, "NAND Data");
#endif
			if(_loc_strcmp(FWDN_NAND_AREA_NAME[i],name) == 0)
				return NAND_Area_ReadSector(0, i, lba, nSector, buff);
		}
	}

	return -1;
}

int _loc_AreaWrite(char *name, unsigned int lba, unsigned short nSector, void *buff)
{
	unsigned int i;

	if (target_is_emmc_boot()) {
		ioctl_diskrwpage_t rwHiddenPage;

		if( _loc_strcmp(FWDN_SD_DATA_AREA_NAME,name) == 0  
#if defined(OTP_UID_INCLUDE)
				|| _loc_strcmp(FWDN_FAT_ONLY_DATA_AREA_NAME,name) == 0
#endif
				)
			return DISK_WriteSector(DISK_DEVICE_TRIFLASH, 0, lba, nSector, buff);
		
		for(i = 0; i < BOOTSD_HIDDEN_MAX_COUNT; i++) {
			if(_loc_strcmp(FWDN_SD_HD_AREA_NAME[i],name) == 0) {
				rwHiddenPage.start_page		= lba;
				rwHiddenPage.rw_size		= nSector;
				rwHiddenPage.hidden_index	= i;
				rwHiddenPage.buff			= buff;
				return DISK_Ioctl(DISK_DEVICE_TRIFLASH, DEV_HIDDEN_WRITE_PAGE, (void *) &rwHiddenPage);
			}
		}
	}
	else {
		for(i=0 ; i<NAND_Area_GetCount(0); i++) {
#if defined(OTP_UID_INCLUDE)
			if (_loc_strcmp(FWDN_FAT_ONLY_DATA_AREA_NAME,name) == 0)
				strcpy(name, "NAND Data");
#endif
			if(_loc_strcmp(FWDN_NAND_AREA_NAME[i],name) == 0)
				return NAND_Area_WriteSector(0, i, lba, nSector, buff);
		}
	}

	return -1;
}

int FWDN_DRV_AREA_Write( char *name
								,unsigned int lba
								,unsigned int nSector
								,unsigned int nRemain
								,FXN_FWDN_DRV_Response_RequestData fxnFwdnDrvResponseRequestData
								,FXN_FWDN_DRV_DirectReadFromHost fxnFwdnDrvDirectReadFromHost )
{
//#define BUNCH_SECTOR_SIZE		(0x80000000>>9)
#define BUNCH_SECTOR_SIZE		(1024/*KB*/*2)

	unsigned char *receiveBuf;// = (unsigned char *)fwdndBuf;
	//unsigned char *verifyBuf  = (unsigned char *)veriBuf;
	unsigned short nReceiveSector;
	unsigned int nReceiveBytes;
	int _result = TRUE;
	unsigned int nBunch;

	if( nSector > BUNCH_SECTOR_SIZE )
	{
		nBunch = BUNCH_SECTOR_SIZE;
	}
	else
	{
		nBunch = nSector;
	}
	nSector -= nBunch;
	fxnFwdnDrvResponseRequestData(nBunch<<9);

#if defined(SECURE_KEYBOX_INCLUDE)
		if(!strcmp(name, "KEY STORE RO", 12)) {
			unsigned char* TempBuffer = (unsigned char*)malloc(4*1024*1024*sizeof(char));
			unsigned int TAddr = (unsigned int)TempBuffer;

			while(nBunch){

			nReceiveBytes = fxnFwdnDrvDirectReadFromHost((void**)&receiveBuf);

			if((nReceiveBytes&0x1FF)!=0 || receiveBuf == NULL)
			{
				return FALSE;
			}

			nReceiveSector = (unsigned short)(nReceiveBytes>>9);
			nBunch -= nReceiveSector;

			if(nBunch == 0 && nSector > 0 && _result == TRUE)
			{
				if( nSector > BUNCH_SECTOR_SIZE )
				{
					nBunch = BUNCH_SECTOR_SIZE;
				}
				else
				{
					nBunch = nSector;
				}
				nSector -= nBunch;
				fxnFwdnDrvResponseRequestData(nBunch<<9);
			}

			/* copy form use to memory for secure key store */
			memcpy((unsigned char*)TAddr, receiveBuf, nReceiveBytes);

			if(nBunch)
				TAddr = TAddr+nReceiveBytes;

			}

			_result = FWDN_DRV_DRM_WriteKeyStore(TempBuffer, name);

			free(TempBuffer);

		}
		else
#endif

#if defined(SECURE_UID_INCLUDE)
		if(!strcmp(name, "UID", 3)) {
			unsigned char* TempBuffer = (unsigned char*)malloc(4*1024*1024*sizeof(char));	// 4MB
			unsigned int TAddr = (unsigned int)TempBuffer;

			while(nBunch){

			nReceiveBytes = fxnFwdnDrvDirectReadFromHost((void**)&receiveBuf);

			if((nReceiveBytes&0x1FF)!=0 || receiveBuf == NULL)
			{
				return FALSE;
			}

			nReceiveSector = (unsigned short)(nReceiveBytes>>9);
			nBunch -= nReceiveSector;

			if(nBunch == 0 && nSector > 0 && _result == TRUE)
			{
				if( nSector > BUNCH_SECTOR_SIZE )
				{
					nBunch = BUNCH_SECTOR_SIZE;
				}
				else
				{
					nBunch = nSector;
				}
				nSector -= nBunch;
				fxnFwdnDrvResponseRequestData(nBunch<<9);
			}

			/* copy form use to memory for secure uid store */
			memcpy((unsigned char*)TAddr, receiveBuf, nReceiveBytes);

			if(nBunch)
				TAddr = TAddr+nReceiveBytes;

			}

			_result = FWDN_DRV_UID_WriteUidStore(TempBuffer, name);
			
			free(TempBuffer);

		}
		else
#endif

#if defined(OTP_UID_INCLUDE)
		if(!strcmp(name, "OTP Data", 8)) {
			unsigned char* TempBuffer = (unsigned char*)malloc(2*1024*sizeof(char));	// 2KB
			unsigned int TAddr = (unsigned int)TempBuffer;

			while(nBunch){

			nReceiveBytes = fxnFwdnDrvDirectReadFromHost((void**)&receiveBuf);

			if((nReceiveBytes&0x1FF)!=0 || receiveBuf == NULL)
			{
				return FALSE;
			}

			nReceiveSector = (unsigned short)(nReceiveBytes>>9);
			nBunch -= nReceiveSector;

			if(nBunch == 0 && nSector > 0 && _result == TRUE)
			{
				if( nSector > BUNCH_SECTOR_SIZE )
				{
					nBunch = BUNCH_SECTOR_SIZE;
				}
				else
				{
					nBunch = nSector;
				}
				nSector -= nBunch;
				fxnFwdnDrvResponseRequestData(nBunch<<9);
			}

			/* copy form use to memory for secure uid store */
			memcpy((unsigned char*)TAddr, receiveBuf, nReceiveBytes);

			if(nBunch)
				TAddr = TAddr+nReceiveBytes;

			}


			_result = FWDN_DRV_OTP_Write(TempBuffer);
			
			free(TempBuffer);

		}
		else
#endif
		{

		while(nBunch)
		{
			nReceiveBytes = fxnFwdnDrvDirectReadFromHost((void**)&receiveBuf);
			if((nReceiveBytes&0x1FF)!=0 || receiveBuf == NULL)
			//if( fxnFwdnDrvReadFromHost(receiveBuf, nReceiveSector<<9) != nReceiveSector<<9 )
			{
				return FALSE;
			}
			nReceiveSector = (unsigned short)(nReceiveBytes>>9);
			nBunch -= nReceiveSector;

			if(nBunch == 0 && nSector > 0 && _result == TRUE)
			{
				if( nSector > BUNCH_SECTOR_SIZE )
				{
					nBunch = BUNCH_SECTOR_SIZE;
				}
				else
				{
					nBunch = nSector;
				}
				nSector -= nBunch;
				fxnFwdnDrvResponseRequestData(nBunch<<9);
			}

			if(_result == TRUE && _loc_AreaWrite(name,lba,nReceiveSector,receiveBuf)!=0)
			{
				FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_WRITE);
				_result = FALSE;
			}
			//else if(_result == TRUE && _loc_DiskRead(name,lba,nReceiveSector,verifyBuf)!=0)
			//{
			//	FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_READ);
			//	_result = FALSE;
			//}
			//else if _result == TRUE && (memcmp(receiveBuf, verifyBuf, nReceiveSector<<9) != 0)
			//{
			//	FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_WRITE_COMPARE);
			//	_result = FALSE;
			//}

			lba += nReceiveSector;
		}
	}
	return _result;
}

int FWDN_DRV_AREA_Read( char *name
								,unsigned int lba
								,unsigned int nSector
								,unsigned int nRemain
								,FXN_FWDN_DRV_Response_NotifyData fxnFwdnDrvResponseNotifyData
								,FXN_FWDN_DRV_SendToHost fxnFwdnDrvSendToHost )
{
//#define BUNCH_SECTOR_SIZE		(0x80000000>>9)
#define BUNCH_SECTOR_SIZE		(1024/*KB*/*2)

	unsigned char *receiveBuf;// = (unsigned char *)fwdndBuf;
	//unsigned char *verifyBuf  = (unsigned char *)veriBuf;
	unsigned short nReceiveSector;
	unsigned int nReceiveBytes;
	int _result = TRUE;
	unsigned int nBunch;


	{
		if(nRemain!=0)
		{
			return FALSE;
		}
	}

	return FALSE;
}

int FWDN_DRV_AREA_CalcCRC( char *name
							,unsigned int lba
							,unsigned int nSector
							,unsigned int nRemain
							,unsigned int *pCrc
							,FXN_FWDN_DRV_SendStatus fxnFwdnDrvSendStatus )
{
#define SEND_STATUS_EVERY_N_SECTOR		2048
	unsigned char *_buf  = (unsigned char *)fwdndBuf;

	unsigned short nReadSector;
	unsigned int nCalcedSector = 0;
	unsigned int nNextSendStatusSector = SEND_STATUS_EVERY_N_SECTOR;

	{
		if(nRemain!=0)
		{
			return FALSE;
		}
	}

	while(nSector)
	{
		if(nCalcedSector>nNextSendStatusSector)
		{
			nNextSendStatusSector += SEND_STATUS_EVERY_N_SECTOR;
			fxnFwdnDrvSendStatus(nCalcedSector,0,0);
		}

		if( nSector > TbufferSectorSize )
			nReadSector = TbufferSectorSize;
		else
			nReadSector = (unsigned short)nSector;

		if(_loc_AreaRead(name,lba,nReadSector,_buf)!=0)
		{
			FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_READ);
			return FALSE;
		}
		*pCrc = FWUG_CalcCrc8I(*pCrc,_buf,(unsigned int)nReadSector<<9);

		lba += nReadSector;
		nCalcedSector += nReadSector;
		nSector -= nReadSector;
	}

	return TRUE;
}

#define	FWDN_DRV_BLOCKREAD_BUFFER_SIZE		(32*1024)		// because of LGE CDC
unsigned char FWDN_DRV_DUMP_InfoRead(void *pBuf)
{
	unsigned int  i = 0;
	unsigned char length = 0;

	return length;
}

int FWDN_DRV_DUMP_BlockRead(unsigned int Param0, unsigned int Param1, unsigned int Param2, 
											FXN_FWDN_DRV_Response_NotifyData fxnFwdnDrvResponseNotifyData,
											FXN_FWDN_DRV_SendToHost fxnFwdnDrvSendToHost)
{
	return FALSE;
}

int FWDN_DRV_DUMP_BlockWrite(unsigned int Param0, unsigned int Param1, unsigned int Param2, 
											FXN_FWDN_DRV_Response_RequestData fxnFwdnDrvResponseRequestData,
											FXN_FWDN_DRV_ReadFromHost fxnFwdnDrvReadFromHost)
{
	return FALSE;
}

static int _loc_GetAreaSize(char *name)
{
	unsigned int i;

	if (target_is_emmc_boot())
		return 0;

	for(i=0 ; i<NAND_Area_GetCount(0); i++) {
		if(_loc_strcmp(FWDN_NAND_AREA_NAME[i],name) == 0)
			return NAND_Area_GetTotalSectorCount(0, i);
	}

	return 0;
}

#define DUMP_ADDR KERNEL_ADDR + 0x100000

int _loc_Physical_AreaRead( void *buff, FXN_FWDN_DRV_Response_NotifyData fxnFwdnDrvResponseNotifyData, FXN_FWDN_DRV_SendToHost fxnFwdnDrvSendToHost )
{
	ioctl_diskphysical_t 	ioctl_physical_info;
	int res = FALSE;
	unsigned int uiBlockAddress;;
	unsigned int uiTotalPhyBlockNum;
	unsigned int uiNANDPhyReadSize, uiBlockSize;
	unsigned int uiUsbBufferIndex, uiRequestSize;

	unsigned char *pucUsbBuffer = (unsigned char*)DUMP_ADDR;
	
	// Get Total NAND Physical Block Number.
	DISK_Ioctl( DISK_DEVICE_NAND, DEV_GET_PHYSICAL_INFO, (void*) &ioctl_physical_info );

	uiTotalPhyBlockNum	= ioctl_physical_info.uiBlockNumberOfDevice;
	uiNANDPhyReadSize	= ioctl_physical_info.uiPageSize + ioctl_physical_info.uiSpareSize;
	uiBlockSize			= uiNANDPhyReadSize << ioctl_physical_info.uiShiftPpB;

	FWDN_TRACE("[Physical Total Block:%d] [Page:%d] [Spare:%d] [BlockSize : %d]\n", uiTotalPhyBlockNum, 
		   		ioctl_physical_info.uiPageSize, ioctl_physical_info.uiSpareSize, uiBlockSize );
	
	// Read All Physical Block.
	for( uiBlockAddress = 0 ; uiBlockAddress < uiTotalPhyBlockNum; uiBlockAddress++ )
	{	
		res = NAND_ReadPhysicalBlock( uiBlockAddress, uiNANDPhyReadSize, (void*)DUMP_ADDR );

		uiRequestSize 	 = uiBlockSize;
		uiUsbBufferIndex = 0;
		
		fxnFwdnDrvResponseNotifyData( uiRequestSize, 0 );

		while( uiRequestSize > 0 )
		{
			unsigned int packetSize = uiRequestSize>FWDN_DRV_BLOCKREAD_BUFFER_SIZE? FWDN_DRV_BLOCKREAD_BUFFER_SIZE : uiRequestSize;
			uiRequestSize -= packetSize;
			fxnFwdnDrvSendToHost( &pucUsbBuffer[uiUsbBufferIndex], packetSize );
			uiUsbBufferIndex += packetSize;
		}
	}

	return res;
}

int FWDN_DRV_Dump(char *pName, unsigned int uiAddress, unsigned int uiSize,
											FXN_FWDN_DRV_Response_NotifyData fxnFwdnDrvResponseNotifyData,
											FXN_FWDN_DRV_SendToHost fxnFwdnDrvSendToHost)
{
	int res = FALSE;

	if (target_is_emmc_boot())
		return res;

	if( _loc_strcmp(pName,"boot") == 0 ) {
		uiSize = NAND_DumpExtArea( (unsigned char *)DUMP_ADDR );
		if( uiSize == 0 )
		{
			FWDN_TRACE(" NAND Dump Fail !!\n");
			return FALSE;
		}
		fxnFwdnDrvResponseNotifyData(uiSize,0);
		fxnFwdnDrvSendToHost(DUMP_ADDR, uiSize);

		res = TRUE;
	}
	else if( _loc_strcmp( pName, "NAND" ) == 0 )
	{
		FWDN_TRACE(" NAND Physical Dump ! \n" );
		FWDN_TRACE(" Size : %d, NAME:%s, Address:%d \n", uiSize, pName, uiAddress );
		res = _loc_Physical_AreaRead( DUMP_ADDR, fxnFwdnDrvResponseNotifyData, fxnFwdnDrvSendToHost );
	}
	else if( _loc_GetAreaSize(pName) != 0 )
	{
		if( uiSize == 0 )
		{
			uiAddress = 0;
			uiSize = _loc_GetAreaSize(pName);
		}
		while( uiSize > 0 )
		{
			unsigned int uiPacketSectorSize = (uiSize>(FWDN_DRV_BLOCKREAD_BUFFER_SIZE/512))? (FWDN_DRV_BLOCKREAD_BUFFER_SIZE/512) : uiSize;
			fxnFwdnDrvResponseNotifyData(uiPacketSectorSize<<9,0);
			_loc_AreaRead(pName, uiAddress, uiPacketSectorSize, DUMP_ADDR);
			fxnFwdnDrvSendToHost( DUMP_ADDR, uiPacketSectorSize<<9 );
			uiAddress += uiPacketSectorSize;
			uiSize -= uiPacketSectorSize;
		}

		res = TRUE;
	}
	return res;
}

int FWDN_DRV_TOTALBIN_Read(FXN_FWDN_DRV_Response_NotifyData fxnFwdnDrvResponseNotifyData,
							FXN_FWDN_DRV_SendToHost fxnFwdnDrvSendToHost)
{
	return FALSE;
}

int FWDN_DRV_TOTALBIN_Write(FXN_FWDN_DRV_RquestData fxnFwdnDrvRRequestData)
{
	return FALSE;
}

unsigned int FWDN_FNT_SetSN(unsigned char* ucTempData, unsigned int uiSNOffset)
{
	unsigned int		uiVerifyCrc;
	unsigned int		uiTempCrc;

	if (memcmp( &ucTempData[20+uiSNOffset], "ZERO", 4 ) == 0)
	{		
		uiVerifyCrc = (ucTempData[uiSNOffset + 16] <<24) ^ ( ucTempData[uiSNOffset + 17] <<16) ^ 
			( ucTempData[uiSNOffset + 18] << 8) ^ ( ucTempData[uiSNOffset + 19]); 
		uiTempCrc = FWUG_CalcCrc8((uiSNOffset + ucTempData), 16 );

		if ( ( uiTempCrc == uiVerifyCrc ) || (uiVerifyCrc == 0x0000 )) 	//16 bytes Serial Exist
		{
			g_uiFWDN_WriteSNFlag = 1;
			return SUCCESS;
		}
	}
	else if (memcmp(&ucTempData[20+uiSNOffset], "FWDN", 4) == 0 || memcmp(&ucTempData[20+uiSNOffset], "GANG", 4) == 0)
	{
		uiVerifyCrc = ( ucTempData[uiSNOffset + 16] <<24) ^ ( ucTempData[uiSNOffset + 17] <<16) ^ 
			( ucTempData[uiSNOffset + 18] << 8) ^ ( ucTempData[uiSNOffset + 19]); 
		uiTempCrc = FWUG_CalcCrc8( (uiSNOffset + ucTempData), 16 );
		if  (uiVerifyCrc == 0x0000 ) {
			g_uiFWDN_WriteSNFlag = 0; 										// cleared SN
			return 1;
		}
		else {
			if (memcmp(&ucTempData[52+uiSNOffset], "FWDN", 4) == 0 || memcmp(&ucTempData[52+uiSNOffset], "GANG", 4) == 0) {
				uiVerifyCrc = ( ucTempData[uiSNOffset + 48] <<24) ^ ( ucTempData[uiSNOffset + 49] <<16) ^ 
					( ucTempData[uiSNOffset + 50] << 8) ^ ( ucTempData[uiSNOffset + 51]); 
				uiTempCrc = FWUG_CalcCrc8((uiSNOffset + ucTempData+32), 16 );				
				if (( uiVerifyCrc == uiTempCrc ) && ( uiVerifyCrc != 0x0000 )) {
					g_uiFWDN_WriteSNFlag = 1;
					return SUCCESS;
				}				
				else
					g_uiFWDN_WriteSNFlag = 0; 										// cleared SN
			}
			else
				g_uiFWDN_WriteSNFlag = 0; 										// cleared SN
		}
	}
	return 1;
}

/**************************************************************************
*  FUNCTION NAME : 
*  
*      void FWDN_FNT_VerifySN(unsigned char* ucTempData, unsigned int uiSNOffset);
*  
*  DESCRIPTION : Verify the validity of Serial Number format
*  
*  INPUT:
*			ucTempData	= base pointer of serial number format
*			uiSNOffset	= offset of serial number format
*  
*  OUTPUT:	void - Return Type
*			update global structure of FWDN_DeviceInformation according to verification result.
*  
**************************************************************************/
void FWDN_FNT_VerifySN(unsigned char* ucTempData, unsigned int uiSNOffset)
{
	unsigned int		uiVerifyCrc;
	unsigned int		uiTempCrc;

	/*---------------------------------------------------------------------
		Check Type for Serial Number of 16 digits
	----------------------------------------------------------------------*/
	if (memcmp(&ucTempData[20+uiSNOffset], "ZERO", 4) == 0)
	{		
		uiVerifyCrc = (ucTempData[uiSNOffset + 16] <<24) ^ ( ucTempData[uiSNOffset + 17] <<16) ^
			( ucTempData[uiSNOffset + 18] << 8) ^ ( ucTempData[uiSNOffset + 19]); 
		uiTempCrc = FWUG_CalcCrc8((uiSNOffset + ucTempData), 16 );

		if ( ( uiTempCrc == uiVerifyCrc ) || (uiVerifyCrc == 0x0000 ))
		{
			if (uiVerifyCrc == 0x0000 ) 
			{
				memset(FWDN_DeviceInformation.DevSerialNumber, 0x30, 16);
				memset(FWDN_DeviceInformation.DevSerialNumber+16, 0x30, 16);
				FWDN_DeviceInformation.DevSerialNumberType = SN_INVALID_16;
			}
			else
			{
				memcpy(FWDN_DeviceInformation.DevSerialNumber, (uiSNOffset + ucTempData), 16);
				memset(FWDN_DeviceInformation.DevSerialNumber+16, 0x30, 16);
				FWDN_DeviceInformation.DevSerialNumberType = SN_VALID_16;
			}
		}
	}
	/*---------------------------------------------------------------------
		Check Type for Serial Number of 32 digits
	----------------------------------------------------------------------*/
	else if (memcmp(&ucTempData[20+uiSNOffset], "FWDN", 4) == 0 || memcmp(&ucTempData[20+uiSNOffset], "GANG", 4) == 0)
	{
		uiVerifyCrc = ( ucTempData[uiSNOffset + 16] <<24) ^ ( ucTempData[uiSNOffset + 17] <<16) ^ 
			( ucTempData[uiSNOffset + 18] << 8) ^ ( ucTempData[uiSNOffset + 19]); 
		uiTempCrc = FWUG_CalcCrc8(uiSNOffset + ucTempData, 16 );
		if  (uiVerifyCrc == 0x0000 )
		{
			memset(FWDN_DeviceInformation.DevSerialNumber, 0x30, 32);
			FWDN_DeviceInformation.DevSerialNumberType = SN_INVALID_32;			
		}
		else
		{
			if (memcmp(&ucTempData[52+uiSNOffset], "FWDN", 4) == 0 || memcmp(&ucTempData[52+uiSNOffset], "GANG", 4) == 0)
			{
				uiVerifyCrc = ( ucTempData[uiSNOffset + 48] <<24) ^ ( ucTempData[uiSNOffset + 49] <<16) ^ 
					( ucTempData[uiSNOffset + 50] << 8) ^ ( ucTempData[uiSNOffset + 51]); 
				uiTempCrc = FWUG_CalcCrc8(( uiSNOffset + ucTempData + 32), 16 );
				if (( uiVerifyCrc == uiTempCrc ) && ( uiVerifyCrc != 0x0000 ))
				{
					memcpy(FWDN_DeviceInformation.DevSerialNumber, (uiSNOffset + ucTempData), 16);
					memcpy(FWDN_DeviceInformation.DevSerialNumber+16, (uiSNOffset + ucTempData + 32 ), 16);
					FWDN_DeviceInformation.DevSerialNumberType = SN_VALID_32;
				}				
				else
				{
					memset(FWDN_DeviceInformation.DevSerialNumber, 0x30, 32);
					FWDN_DeviceInformation.DevSerialNumberType = SN_INVALID_32;			
				}
			}
			else
			{
				memset(FWDN_DeviceInformation.DevSerialNumber, 0x30, 32);
				FWDN_DeviceInformation.DevSerialNumberType = SN_INVALID_32;			
			}
		}		
	}
	else
	{
		memset(FWDN_DeviceInformation.DevSerialNumber, 0x30, 32);
		FWDN_DeviceInformation.DevSerialNumberType = SN_NOT_EXIST;	
	}
}

void FWDN_FNT_InsertSN(unsigned char *pSerialNumber)
{
	unsigned	uCRC;

	memcpy (pSerialNumber, FWDN_DeviceInformation.DevSerialNumber, 16 );
	uCRC	= FWUG_CalcCrc8(pSerialNumber, 16 );
	
	pSerialNumber[16] = (uCRC & 0xFF000000) >> 24;
	pSerialNumber[17] = (uCRC & 0x00FF0000) >> 16;
	pSerialNumber[18] = (uCRC & 0x0000FF00) >> 8;
	pSerialNumber[19] = (uCRC & 0x000000FF) ;

	memcpy(pSerialNumber+20, "FWDN", 4);
	memset(pSerialNumber+24, 0x00, 8);
	memcpy(pSerialNumber+32, FWDN_DeviceInformation.DevSerialNumber+16, 16);
	uCRC	= FWUG_CalcCrc8(pSerialNumber+32, 16 );

	pSerialNumber[48] = (uCRC & 0xFF000000) >> 24;
	pSerialNumber[49] = (uCRC & 0x00FF0000) >> 16;
	pSerialNumber[50] = (uCRC & 0x0000FF00) >> 8;
	pSerialNumber[51] = (uCRC & 0x000000FF) ;

	memcpy(pSerialNumber+52, "FWDN", 4);
	memset(pSerialNumber+56, 0x00, 8);
}

unsigned char FWDN_DRV_FirmwareStorageID(void)
{
	return (unsigned int)gFWDN_FirmwareStorage;
}

#if defined(SECURE_KEYBOX_INCLUDE)
static int FWDN_DRV_DRM_Write(char* areaname, unsigned char* kBuff, unsigned int start, unsigned int size, unsigned int offset)
{

	unsigned int DRMSector = 0;
	int __result = TRUE; 
	char *wDRMBuf;
	char *rDRMBuf;
	
	DRMSector = (size +511)/512;
	wDRMBuf = (char*)malloc(DRMSector * 512);
	rDRMBuf = (char*)malloc(DRMSector * 512);
	memset(rDRMBuf, 0x00, DRMSector * 512);
	memset(wDRMBuf, 0xff, DRMSector * 512);
	memcpy(wDRMBuf, kBuff+offset, size);

	if(__result == TRUE && _loc_AreaWrite(areaname , start, DRMSector, wDRMBuf) !=0){
		FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_WRITE);
		__result = FALSE;
	}

	if(__result == TRUE && _loc_AreaRead(areaname, start, DRMSector, rDRMBuf) !=0){
		FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_READ);
		__result = FALSE;
	}

	if(!memcmp(wDRMBuf, rDRMBuf, size)){
		dprintf(INFO, "[NAND		] key store verify OK !!\n");
	}else{
		dprintf(INFO, "[NAND		] KEY STORE Verify FAIL !!\n");
		__result = FALSE;
	}

	free(rDRMBuf);
	free(wDRMBuf);

	return __result;
}

int FWDN_DRV_DRM_WriteKeyStore(unsigned char* kBuff, char *areaname)
{
	int drmflag = 0, idx;
	int __result = TRUE;
	struct secure_key_box keybox;
	memcpy(&keybox , kBuff, sizeof(struct secure_key_box));
	
	if(memcmp(keybox.key_tag , "KEYSTORE", 8)){
		dprintf(INFO, "[NAND		] KEYSTORE TAG is not mattached !!\n");
		return -1;
	}

	/* check key store drm flags */

	for(idx=0; idx<MAX_DRM; idx++){

		if((keybox.drm_flags[idx] == 1 ) && (keybox.drm_size[idx] != 0)){
			dprintf(INFO, "[NAND		] START DRM KEY Write !!\n");

			__result = FWDN_DRV_DRM_Write(areaname, kBuff, idx<<9,
					keybox.drm_size[idx], keybox.drm_offset[idx]);
			if(__result == FALSE) dprintf(INFO, "[NAND		] DRM Key Write Fail !!\n");
		}

		if(idx == 1 && keybox.dtcpipsrm_size !=0 && keybox.drm_flags[idx] == 1){
			__result = FWDN_DRV_DRM_Write(areaname, kBuff, 8<<9, keybox.dtcpipsrm_size, keybox.dtcpipsrm_offset);
			if(__result == FALSE) dprintf(INFO, "[NAND		] DTCPIP SRM Write Fail !!\n");
		}

		if(idx == 3 && keybox.hdcpsrm_size !=0 && keybox.drm_flags[idx] == 1 ){
			__result = FWDN_DRV_DRM_Write(areaname, kBuff, 10<<9, keybox.hdcpsrm_size, keybox.hdcpsrm_offset);
			if(__result == FALSE) dprintf(INFO, "[NAND		] HDCP2.X SRM Write Fail !!\n");
		}



	}

	if(__result == FALSE) dprintf(INFO, "[NAND		] KEY STORE Key Write Fail !!\n");

	return __result;
	
}
#endif

#if defined(SECURE_UID_INCLUDE)
static int FWDN_DRV_UID_Write(char* areaname, unsigned char* uBuff, unsigned int start, unsigned int size, unsigned int offset)
{

	unsigned int UIDSector = 0;
	int __result = TRUE; 
	char *wUIDBuf;
	char *rUIDBuf;
	
	UIDSector = (size +511)/512;
	wUIDBuf = (char*)malloc(UIDSector * 512);
	rUIDBuf = (char*)malloc(UIDSector * 512);
	memset(rUIDBuf, 0x00, UIDSector * 512);
	memset(wUIDBuf, 0xff, UIDSector * 512);
	memcpy(wUIDBuf, uBuff+offset, size);

	if(__result == TRUE && _loc_AreaWrite(areaname , start, UIDSector, wUIDBuf) !=0){
		FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_WRITE);
		__result = FALSE;
	}

	if(__result == TRUE && _loc_AreaRead(areaname, start, UIDSector, rUIDBuf) !=0){
		FWDN_DRV_SetErrorCode(ERR_FWDN_DRV_AREA_READ);
		__result = FALSE;
	}

	if(!memcmp(wUIDBuf, rUIDBuf, size)){
		dprintf(INFO, "[NAND		] UID STORE verify OK !!\n");
	}else{
		dprintf(INFO, "[NAND		] UID STORE Verify FAIL !!\n");
		__result = FALSE;
	}

	free(rUIDBuf);
	free(wUIDBuf);

	return __result;
}

int FWDN_DRV_UID_WriteUidStore(unsigned char* uBuff, char *areaname)
{
	int __result = TRUE;
	struct secure_uid_box uidbox;
	memcpy(&uidbox , uBuff, sizeof(struct secure_uid_box));
	
	if(memcmp(uidbox.uid_tag , "UIDSTORE", 8)){
		dprintf(INFO, "[NAND		] UIDSTORE TAG is not mattached !!\n");
		return -1;
	}

	if((uidbox.uid_flags == 1 ) && (uidbox.uid_size != 0)){
		dprintf(INFO, "[NAND		] START UID Write !!\n");

		__result = FWDN_DRV_UID_Write(areaname, uBuff, /*idx<<9*/0,
				uidbox.uid_size, uidbox.uid_offset);
		if(__result == FALSE) dprintf(INFO, "[NAND		] UID Write Fail !!\n");
	}


	if(__result == FALSE) dprintf(INFO, "[NAND		] UID STORE Uid Write Fail !!\n");

	return __result;
	
}
#endif

#if defined(OTP_UID_INCLUDE)
int FWDN_DRV_OTP_Write(unsigned char* oBuff)
{
	int __result = TRUE;
	unsigned int OTPSector = 0;
	char *wOTPBuf;
	struct secure_uid_box otpbox;
	unsigned int otp_clk;

	memcpy(&otpbox , oBuff, sizeof(struct secure_uid_box));

	if(memcmp(otpbox.uid_tag , "OTPSTORE", 8)){
		dprintf(INFO, "[OTP_UID		] OTPSTORE TAG is not mattached !!\n");
		return -1;
	}

	if((otpbox.uid_flags == 1 ) && (otpbox.uid_size != 0)){
		dprintf(INFO, "[OTP_UID		] START OTP_UID Write !!\n");

		otp_clk = get_tcc_mem_freq() / 2; // 1:2 mode 

		OTPSector = (otpbox.uid_size +511)/512;
		wOTPBuf = (char*)malloc(OTPSector * 512);
		memset(wOTPBuf, 0xff, OTPSector * 512);
		memcpy(wOTPBuf, oBuff+otpbox.uid_offset, otpbox.uid_size);

		if(tzow_api(otp_clk, wOTPBuf, otpbox.uid_size) != 0){
			dprintf(INFO, "[OTP_UID		] tzow_api write fail !!\n");
			__result = -1;
		}

		free(wOTPBuf);
	}
	return __result;
}
#endif

#endif //FWDN_V7
/************* end of file *************************************************************/
