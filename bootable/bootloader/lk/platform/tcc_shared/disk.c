/****************************************************************************
 *   FileName    :disk.c
 *   Description :
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/

#include <fwdn/FSAPP.h>
//#include "KFSutils.h"
#include <nand_drv_v8.h>
#include <fwdn/Disk.h>
#include <sdmmc/sd_memory.h>

/*****************************************************************************
 *	disk device name string array
 *	the order should be matched with DISK_DEVICE type.
 *****************************************************************************/
#ifndef NULL
#define NULL		0
#endif

#if _EMMC_BOOT
#define TRIFLASH_DRIVER	{	{"TRIFLASH", DISK_DEVICE_UNLOCK, DISK_DEVICE_TRIFLASH, DISK_STATE_NOTMOUNTED, DISK_STATE_FREE, DISK_MSC_DRV_SUPPORT, DISK_MOUNT_TYPE_EXTERNAL, 0}, \
							BOOTSD_Read,						\
							BOOTSD_Write,						\
							BOOTSD_ReadMultiStart,			\
							BOOTSD_ReadMulti,					\
							BOOTSD_ReadMultiStop,				\
							BOOTSD_WriteMultiStart,			\
							BOOTSD_WriteMulti,				\
							BOOTSD_WriteMultiStop,	 		\
							NULL,				\
							NULL,				\
							NULL,				\
							BOOTSD_Ioctl	}
#else
#define NAND_DRIVER		{	{"NAND",DISK_DEVICE_UNLOCK, DISK_DEVICE_NAND, DISK_STATE_NOTMOUNTED, DISK_STATE_FREE, DISK_MSC_DRV_SUPPORT, DISK_MOUNT_TYPE_INTERNAL, 0}, \
							NAND_ReadSector,				\
							NAND_WriteSector,				\
							NULL,							\
							NAND_ReadSector,				\
							NULL,							\
							NULL,							\
							NAND_WriteSector,				\
							NULL,							\
							NAND_ReadSector,				\
							NAND_WriteSector,				\
							NULL,							\
							NAND_Ioctl	}
#endif

/*****************************************************************************
 * 	DiskList Array Definition
 *
 * 	Structure array for device driver.
 * 	you can add another device before NULL_DRIVER.
 *
 * 	CAUTION!!
 * 	Do Not Delete NULL_DRIVER entry.
 ****************************************************************************/
tDeviceDriver DiskList[] = {
#if _EMMC_BOOT
	TRIFLASH_DRIVER,
#else
	NAND_DRIVER,
#endif
};

const char UnknownDeviceName[]="UNKNOWN";

/*******************************************************************************
 *	FUNCTION 	: 	DISK_FindDisk
 *
 *	Description :	Returns Index in the DiskList array given DiskType
 *
 * 	Parameter 	:	drv_type - Disk Type(NAND, MMC, HDD ... )
 *
 *  Returns		:	Returns index if success. Returns negative value if fail.
 *****************************************************************************/
int	DISK_FindDisk( int drv_type )
{
	int index = 0;
	//while (DiskList[index].DiskType != TOTAL_DEVICE_NUM) {
	while(index < (sizeof(DiskList)/sizeof(tDeviceDriver)) )
	{
		if( DiskList[index].Property.DiskType == drv_type)
		{
			return index;
		}
		index++;
	}
	return -1;
}

/*******************************************************************************
 *	FUNCTION 	: 	DISK_ReadSector
 *
 *	Description :	Read sector(s) from Lun at the drv_type given Address
 *
 * 	Parameter 	:	drv_type 	- Drive Type(NAND, MMC, HDD ...)
 * 					lun			- Physical Drive Number. If there is two HDD disks in
 * 								  the set, the first is 0 and second is 1
 * 					lba_addr	- Sector Address
 * 					nSector		- Sector Count
 * 					buff		- buffer pointer
 *
 *  Returns		:	Returns zero if success or returns minus value if fail.
 *
 *
 *  TODO		:	The lun parameter is experimental now. Fix "0"
 *****************************************************************************/
int		DISK_ReadSector( int drv_type, int lun, unsigned long lba_addr, unsigned short nSector, void *buff )
{
	int		res;
	int		index = 0;

	index = DISK_FindDisk( drv_type );

	if ( index < 0 )
		return index;

	if ( DiskList[index].ReadSector != NULL )
	{
		while ( DiskList[index].Property.LockFlag == DISK_DEVICE_LOCK );

		DiskList[index].Property.LockFlag = DISK_DEVICE_LOCK;
		res = DiskList[index].ReadSector(lun, lba_addr, nSector, buff);
		DiskList[index].Property.LockFlag = DISK_DEVICE_UNLOCK;

		return res;
	}

	return -1;
}

/*******************************************************************************
 *	FUNCTION 	: 	DISK_WriteSector
 *
 *	Description :	Write sector(s) from Lun at the drv_type given Address
 *
 * 	Parameter 	:	drv_type 	- Drive Type(NAND, MMC, HDD ...)
 * 					lun			- Physical Drive Number. If there is two HDD disks in
 * 								  the set, the first is 0 and second is 1
 * 					lba_addr	- Sector Address
 * 					nSector		- Sector Count
 * 					buff		- buffer pointer
 *
 *  Returns		:	Returns zero if success or returns minus value if fail.
 *
 *  TODO		:	The lun parameter is experimental now. Fix "0"
 *****************************************************************************/
int		DISK_WriteSector( int drv_type, int lun, unsigned long lba_addr, unsigned short nSector, void *buff )
{
	int		res;
	int		index = 0;

	index = DISK_FindDisk( drv_type );

	if ( index < 0 )
		return index;

	if ( DiskList[index].WriteSector != NULL )
	{
		while ( DiskList[index].Property.LockFlag == DISK_DEVICE_LOCK );

		DiskList[index].Property.LockFlag = DISK_DEVICE_LOCK;
		res = DiskList[index].WriteSector(lun, lba_addr, nSector, buff);
		DiskList[index].Property.LockFlag = DISK_DEVICE_UNLOCK;
		return res;
	}
	return -1;
}

/*******************************************************************************
 *	FUNCTION 	: 	DISK_Ioctl
 *
 *	Description :	Execute disk-specific functions.
 *					(Refer to IOCTL_FUNCTIONS enumerations at disk.h)
 *
 * 	Parameter 	:	drv_type 	- disk type (NAND, MMC, HDD ...)
 * 					function	- function number(DEV_INITIALIZE, DEV_GET_DISKINFO...)
 * 					param		- the parameter pointer which depends on function
 *
 *  Returns		:	Returns "0" if success. Returns "-1" if fail.
 *****************************************************************************/
int		DISK_Ioctl( unsigned int drv_type, int function, void *param )
{
	int		res;
	int		index = 0;

	index = DISK_FindDisk( drv_type );

	if ( index < 0 )
		return index;

	if ( DiskList[index].Ioctl != NULL )
	{
		if (( function != DEV_GET_POWER)&&(function != DEV_GET_INSERTED))
		{
			while ( DiskList[index].Property.LockFlag == DISK_DEVICE_LOCK );

			DiskList[index].Property.LockFlag = DISK_DEVICE_LOCK;
		}
		res = DiskList[index].Ioctl( function, param );
		if (( function != DEV_GET_POWER)&&(function != DEV_GET_INSERTED)){
			DiskList[index].Property.LockFlag = DISK_DEVICE_UNLOCK;
		}

		return res;
	}

	return -1;
}

