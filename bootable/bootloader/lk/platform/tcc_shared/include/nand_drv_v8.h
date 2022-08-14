/****************************************************************************
 *   FileName    : nand_drv_v8.h
 *   Description : NAND driver for application.
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
 *
 ****************************************************************************/
#ifndef _NAND_DRV_H
#define _NAND_DRV_H

//*****************************************************************************
//*
//*
//*                       [ General DEFINE & TYPEDEF ]
//*
//*
//*****************************************************************************
#ifndef DISABLE
#define DISABLE					0
#endif
#ifndef ENABLE
#define	ENABLE					1
#endif
#ifndef FALSE
#define FALSE           		0
#endif
#ifndef TRUE
#define TRUE            		1
#endif

#define NAND_KERNEL_AREA_INDEX			0 /*NAND_RO_0_AREA_INDEX*/
#define NAND_LOGO_AREA_INDEX			1 /*NAND_RO_1_AREA_INDEX*/

// mirror check define
#define MIRROR_CHECK_SIGNATURE_FIELD	0xFF000000
#define MIRROR_CHECK_SIGNATURE			0x0C000000
#define BOOTLOADER_MIRROR_CHECK			(0x1 << 2)
#define MEMINIT_MIRROR_CHECK			(0x1 << 3)
#define TZOS_MIRROR_CHECK				(0x1 << 4)

typedef enum
{
    NAND_INIT_TYPE_NORMAL,
    NAND_INIT_TYPE_FAST,
    NAND_INIT_TYPE_LOWFORMAT,
    NAND_INIT_TYPE_LOWFORMAT_DEBUG_CHECKANDERASE,
    NAND_INIT_TYPE_LOWFORMAT_DEBUG_ERASE_ONLY,
    NAND_INIT_TYPE_SKIP_BLOCK_INIT
} NAND_INIT_TYPE_T;

#define NAND_CTRL_AREA_INFO_VERIFY		0
#define NAND_CTRL_AREA_INFO_RESET		1

typedef enum
{
    NAND_SUCCESS						= 0x00000000,
    NAND_ERROR							= 0xF1000000,
    NAND_ERROR_WRONG_PARAMETER			= 0xF1000001,
    NAND_ERROR_INIT_FAILURE				= 0xF1000002,
    NAND_ERROR_INIT_AREA_CHANGE			= 0xF1000003,
    NAND_ERROR_MEMORY_ALLOCATION		= 0xF1000004
} NAND_ERROR_T;

typedef struct _NAND_USER_INFO_TABLE
{
	unsigned char ucSerialNumber[32];
	unsigned int uiSerialNumberCRC;
} NAND_USER_INFO_TABLE;

#define TNFTL_V8_USE_MTD
#define TNFTL_V8_END_OF_MTD_PARTITION	4

/// MTD Partition Number for TNFTL RO Area///
/// This Number match to RO Area Index. /// 
#define NAND_MTD_BOOT_INDEX 0			// TNFTL_RO Area Index 0
#define NAND_MTD_RECOVERY_INDEX 1		// TNFTL_RO Area Index 1
#define NAND_MTD_QUICKBOOT_INDEX 2		// TNFTL_RO Area Index 2
#define NAND_MTD_USERDATA_INDEX 3		// TNFTL_RO Area Index 3  - MAX RO Area Number of TNFTL_V8

/// MTD Partition Number for MTD Image ///
// This Number is used FWDN_Write and MTD Update. //
// This Number is written by MTD Image tool into MTD Image //
#define MTD_PART_BOOT				0x0001
#define MTD_PART_RECOVERY			0x0003
#define MTD_PART_USERDATA			0x0004


typedef void (*NAND_ProgressHandler)( unsigned int uiCurrent, unsigned int uiTotal );

//*****************************************************************************
//*
//*
//*                       [ EXTERNAL FUCTIONS DEFINE ]
//*
//*
//*****************************************************************************
extern unsigned int NAND_Area_GetCount( unsigned int uiDrvNum );
extern unsigned int NAND_Area_GetTotalSectorCount( unsigned int uiDrvNum, unsigned int uiAreaIndex );
extern NAND_ERROR_T NAND_Init( unsigned int uiDrvNum, NAND_INIT_TYPE_T eType, const NAND_ProgressHandler fnProgressHandler );
extern unsigned int NAND_LowFormat( unsigned int uiDrvNum );
extern int NAND_ReadSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_WriteSector(  unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_RW_WriteSector( unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_RW_ReadSector( unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_Area_WriteSector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_Area_ReadSector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_HDWriteSector(int nDrvno, unsigned int LBA, unsigned int nSecsize, void* nReadBuffer);
extern int NAND_HDReadSector(int nDrvno, unsigned int LBA, unsigned int nSecsize, void* nReadBuffer);
extern int NAND_RO_Area_Flush_WriteCache( unsigned int uiDrvNum );
extern int	NAND_GetSerialNumber( unsigned char *pucSN, unsigned int uiSize );
extern int NAND_SetSerialNumber( unsigned char *ucData, unsigned int uiSize );
extern int NAND_WriteFirmware( unsigned int uiDrvNum, unsigned char *pucRomBuffer, unsigned int uiFWSize );
extern int NAND_Ioctl( int function, void *param );
extern unsigned int NAND_GetBootStatus(void);
extern void NAND_RecoveryBootArea( unsigned int uiBootStatus );
extern void NAND_Suspend(void);
extern void NAND_Resume(void);

extern int NAND_RW_ClearPages( unsigned int nHDStPageAddr, unsigned int nHDEdPageAddr );
extern int NAND_ReadMediaUID( unsigned char *pucUID );

extern int NAND_MTD_Area_WritePage( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiPageAddress, void *pvBuffer, void *pvTag, unsigned char bUseYaffs );
extern int NAND_MTD_Area_ReadPage( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiPageAddress, void *pvBuffer, void *pvTag, unsigned char bUseYaffs );
extern int NAND_MTD_Update( unsigned int PageOffset, unsigned char *DataBuffer, unsigned int  partition_type );
extern int NAND_MTD_Area_Erase( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiPageAddr, unsigned int isWholeArea );
	
/* NU */
#ifdef USE_FSONNANDHIDDEN
extern int NAND_HDWriteSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_HDReadSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer );
extern int NAND_HDIoctl(int function, void *param);
#endif
#ifdef USE_NANDHIDDENLOCK
extern void NAND_HDWriteEnable(void);
extern void NAND_HDWriteDisable(void);
#endif

#if defined(_LINUX_)
extern unsigned int NAND_DumpExtArea( unsigned char *buf );
#endif

#endif

