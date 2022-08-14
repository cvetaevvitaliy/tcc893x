/****************************************************************************
 *   FileName    : nand_drv_v8.c
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

//#define FEATURE_TNFTL_DEBUG
#if defined(TCC801X)
#include "config/sdk/config.h"
#endif
#include "nand_def.h"
#if defined(_WINCE_) && !defined(NAND_FOR_KERNEL)
#define NAND_USE_COMPRESS       // USE tcBoot Compression
#endif
//////////////////////////////////////////////////////////////////////////////
////////////////////////////////// _LINUX_ ///////////////////////////////////
#if defined(_LINUX_)
#if defined(NAND_FOR_KERNEL)
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#if !defined(ANDROID_KERNEL)
#include <linux/export.h>
#endif
#else
#include <string.h>
#include <debug.h>
#endif
#endif
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "tnftl_v8_api.h"
#include "nand_drv_v8.h"
#include "nand_io_v8.h"
#include "nand_utils.h"

#if defined(_NU_)
#include "DRV/FS/Disk.h"
#include "SERVICE/fwdn/fwupgrade.h"
#include "Config/SDK/config.h"
#include "Config/MCU/MCU_HWCfg.h"
#include "SERVICE/TC_File/TC_File.h"
#include "common/globals.h"
#include "SERVICE/DeviceManager/DeviceManager.h"
#include "OS/TC_Kernel.h"
#else
#include "Disk.h"
#include "fwupgrade.h"
#endif


#ifdef FEATURE_TNFTL_DEBUG
#include "tnftl_v8.h"
#endif

#if !defined(_LINUX_)
#define EXPORT_SYMBOL(...)
#endif

#ifdef NAND_USE_COMPRESS
#define comp_buff_size (1*1024*1024)
static char comp_buff[comp_buff_size];
#endif


//#define NAND_ASSERT(a)			if(!(a)) while(1);
#define NAND_ASSERT(a)			if(!(a))\
								{\
									ND_TRACE("[ASSERT] %s:%d: %s\n",__FUNCTION__,__LINE__,#a);\
								}

#define DRIVE_COUNT_MAX						1

#define BLOCK_COUNT_MAX						8192

#define PMD_COUNT_MAX						2048

#define PAGES_PER_BLOCK_MAX					256
#define ENTRY_COUNT_MAX						PAGES_PER_BLOCK_MAX

#define ECD_COUNT_MAX						16

//#define PAGE_STATUS_SIZE_IN_BYTE			(256*1024)
#define TOTAL_PAGES_PER_DIE_MAX				BLOCK_COUNT_MAX*PAGES_PER_BLOCK_MAX		// 8192*256
#define PAGE_STATUS_SIZE_IN_BYTE			TOTAL_PAGES_PER_DIE_MAX>>3 /* /8 */

#ifndef TRUE
#define TRUE					1
#endif
#ifndef FALSE
#define FALSE					0
#endif
#ifndef NULL
#define NULL					((void*)0)
#endif

static NAND_USER_INFO_TABLE s_NandUserInfoTable;

extern unsigned int TNFTL_GetDriverVersion( const char **ppcStr );
static TNFTL_OBJECT_T s_NAND_TnftlObj[DRIVE_COUNT_MAX];
#ifdef FEATURE_TNFTL_DEBUG
static DIE_STATE_T s_NAND_Die[DIE_COUNT_MAX];
static BLOCK_FEATURE_T s_NAND_BlockFeature[DIE_COUNT_MAX][BLOCK_COUNT_MAX];
static CMT_T s_NAND_CMT[DIE_COUNT_MAX][CMT_COUNT_MAX];
static ENTRY_T s_NAND_Entry[DIE_COUNT_MAX][CMT_COUNT_MAX][ENTRY_COUNT_MAX];
#else
static unsigned int s_NAND_Die[DIE_COUNT_MAX][24];
static unsigned int s_NAND_BlockFeature[DIE_COUNT_MAX][BLOCK_COUNT_MAX][4];
static unsigned int s_NAND_CMT[DIE_COUNT_MAX][CMT_COUNT_MAX][4];
static unsigned int s_NAND_Entry[DIE_COUNT_MAX][CMT_COUNT_MAX][ENTRY_COUNT_MAX][3];
#endif
static unsigned int s_NAND_uiPageBuffer[TNFTL_PAGE_BUFFER_COUNT][PAGE_DWORDSIZE_MAX + TNFTL_TAG_DWORD_SIZE + ( 640 / 4 )];
static unsigned int s_NAND_uiROTempPageBuffer[PAGE_DWORDSIZE_MAX];
static unsigned int s_NAND_uiRWTempPageBuffer[PAGE_DWORDSIZE_MAX];

static unsigned int s_NAND_uiPMD[DIE_COUNT_MAX][PMD_COUNT_MAX];
static unsigned int s_NAND_uiMTB[DIE_COUNT_MAX][PAGE_DWORDSIZE_MAX];

static unsigned int s_NAND_uiECD[DIE_COUNT_MAX][ECD_COUNT_MAX];

static unsigned char s_NAND_ucPageStatus[DIE_COUNT_MAX][PAGE_STATUS_SIZE_IN_BYTE];

static NAND_ERROR_T s_NAND_DRV_errorDrive[DRIVE_COUNT_MAX];

// For removed compile warning.
unsigned int FWUG_GetMemInitCodeSizeInByte(void);
//

#if defined(_NU_)
#define NAND_DRV_UNLOCKED 		0
#define NAND_DRV_LOCKED			1
static unsigned int s_Lockflag = NAND_DRV_UNLOCKED;
#endif

void NAND_EnterLock(void)
{
	#if defined(_NU_)
	while(1)
	{
		TC_SchedLock();
		if(s_Lockflag == NAND_DRV_UNLOCKED)
		{
			s_Lockflag = NAND_DRV_LOCKED;
			TC_SchedUnlock();
			break;
		}
		TC_SchedUnlock();
		// did it need some delay?
		TC_TimeDly(1);
	}
	#endif
}

void NAND_LeaveLock(void)
{
	#if defined(_NU_)
	TC_SchedLock();
	s_Lockflag = NAND_DRV_UNLOCKED;
	TC_SchedUnlock();
	#endif
}

NAND_ERROR_T NAND_GetLastError( unsigned int uiDriveNumber )
{
	if( uiDriveNumber >= DRIVE_COUNT_MAX )
		return NAND_ERROR_WRONG_PARAMETER;
	return s_NAND_DRV_errorDrive[uiDriveNumber];
}

static void NAND_SetLastError( unsigned int uiDriveNumber, NAND_ERROR_T err )
{
	if( uiDriveNumber < DRIVE_COUNT_MAX )
		s_NAND_DRV_errorDrive[uiDriveNumber] = err;
}

unsigned int NAND_GetDriverCount( void )
{
	return DRIVE_COUNT_MAX;
}

TNFTL_OBJECT_T *NAND_GetObject( unsigned int uiDrvNum )
{
	NAND_ASSERT( uiDrvNum < DRIVE_COUNT_MAX );
	if( uiDrvNum < DRIVE_COUNT_MAX )
		return &s_NAND_TnftlObj[uiDrvNum];
	else
		return NULL;
}

static int s_ImageBaseOffset = 0;
void NAND_SetImageBaseOffset(int offset)
{
	s_ImageBaseOffset = offset;
}

static unsigned int NAND_Control_AreaInfo( unsigned int uiDrvNum, unsigned int uiControlType )
{
	TNFTL_OBJECT_T *pTnftlObj = &s_NAND_TnftlObj[uiDrvNum];

	NAND_ASSERT( uiDrvNum < DRIVE_COUNT_MAX );

	if( uiDrvNum == 0 )
	{
		TNFTL_AREA_INFO_T *pTnftlAreaInfo = NULL;
#if !defined(NAND_FOR_KERNEL)
		extern unsigned int TNFTL_STRUCTURE_VERSION;
		extern unsigned int BOOT_AREA_SIZE_IN_MB;
		extern unsigned int RO_0_AREA_SECTOR_COUNT;
		extern unsigned int RO_1_AREA_SECTOR_COUNT;
		extern unsigned int RO_2_AREA_SECTOR_COUNT;
		extern unsigned int RO_3_AREA_SECTOR_COUNT;
		extern unsigned int RW_0_AREA_SECTOR_COUNT;
		extern unsigned int RW_1_AREA_SECTOR_COUNT;
		extern unsigned int RW_2_AREA_SECTOR_COUNT;
		extern unsigned int RW_3_AREA_SECTOR_COUNT;
		extern unsigned int RW_4_AREA_SECTOR_COUNT;
		extern unsigned int DATA_AREA_SECTOR_COUNT;

		TNFTL_AREA_INFO_T sTnftlAreaInfo;

		sTnftlAreaInfo.uiStructureVersion	= TNFTL_STRUCTURE_VERSION;
		sTnftlAreaInfo.uiBootSizeInMB		= *(unsigned int *)((unsigned int)&BOOT_AREA_SIZE_IN_MB-s_ImageBaseOffset);

		sTnftlAreaInfo.sRO_Config.uiSectorCount[0] = *(unsigned int *)((unsigned int)&RO_0_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRO_Config.uiSectorCount[1] = *(unsigned int *)((unsigned int)&RO_1_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRO_Config.uiSectorCount[2] = *(unsigned int *)((unsigned int)&RO_2_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRO_Config.uiSectorCount[3] = *(unsigned int *)((unsigned int)&RO_3_AREA_SECTOR_COUNT-s_ImageBaseOffset);

		sTnftlAreaInfo.sRW_Config.uiSectorCount[0] = *(unsigned int *)((unsigned int)&RW_0_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRW_Config.uiSectorCount[1] = *(unsigned int *)((unsigned int)&RW_1_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRW_Config.uiSectorCount[2] = *(unsigned int *)((unsigned int)&RW_2_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRW_Config.uiSectorCount[3] = *(unsigned int *)((unsigned int)&RW_3_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRW_Config.uiSectorCount[4] = *(unsigned int *)((unsigned int)&RW_4_AREA_SECTOR_COUNT-s_ImageBaseOffset);
		sTnftlAreaInfo.sRW_Config.uiSectorCount[5] = *(unsigned int *)((unsigned int)&DATA_AREA_SECTOR_COUNT-s_ImageBaseOffset);

		pTnftlAreaInfo = &sTnftlAreaInfo;
#endif
		if( uiControlType == NAND_CTRL_AREA_INFO_VERIFY )
		{
			pTnftlObj->errLastError = TNFTL_FW_Verify_GoldenInfo( pTnftlObj, pTnftlAreaInfo );
			if( pTnftlObj->errLastError != TNFTL_SUCCESS )
				return FALSE;
		}
#if !defined(NAND_FOR_KERNEL)
		else if( uiControlType == NAND_CTRL_AREA_INFO_RESET )
		{
			TNFTL_FW_Reset_GoldenInfo( pTnftlObj, FWUG_GetMemInitCodeSizeInByte(), pTnftlAreaInfo );
		}
#endif
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

unsigned int NAND_Area_GetCount( unsigned int uiDrvNum )
{
	return NAND_RO_AREA_COUNT_MAX + NAND_RW_AREA_COUNT_MAX;
}

unsigned int NAND_Area_GetTotalSectorCount( unsigned int uiDrvNum, unsigned int uiAreaIndex )
{
	return TNFTL_Area_GetTotalSectorCount( NAND_GetObject( uiDrvNum ), uiAreaIndex );
}

NAND_ERROR_T NAND_Init( unsigned int uiDrvNum, NAND_INIT_TYPE_T eType, const NAND_ProgressHandler fnProgressHandler )
{
	unsigned int i;
	TNFTL_OBJECT_T *pTnftlObj;

	NAND_EnterLock();
	//ND_TRACE( "+NAND_Init\n" );
	{
		const char *pucVer;
		ND_TRACE( "V8.%d.%d\n", TNFTL_GetDriverVersion( &pucVer ), NAND_DRIVER_VERSION );
	}

	if( uiDrvNum >= DRIVE_COUNT_MAX )
	{
		NAND_LeaveLock();
		return NAND_ERROR_WRONG_PARAMETER;
	}

	pTnftlObj = &s_NAND_TnftlObj[uiDrvNum];

	memset( pTnftlObj, 0x00, sizeof( TNFTL_OBJECT_T ) );

	pTnftlObj->uiRelevantCS = NAND_CS_0 | NAND_CS_1 | NAND_CS_2 | NAND_CS_3;

	{
		TNFTL_BUFFER_PER_DIE_T sTnftlBufPerDie[DIE_COUNT_MAX] =
		{
			{
				&s_NAND_Die[0],
				sizeof( s_NAND_Die[0] ),
				s_NAND_BlockFeature[0],
				sizeof( s_NAND_BlockFeature[0] ),
				s_NAND_uiPMD[0],
				s_NAND_uiMTB[0],
				s_NAND_CMT[0],
				sizeof( s_NAND_CMT[0] ),
				s_NAND_Entry[0][0],
				sizeof( s_NAND_Entry[0] ),
				s_NAND_uiECD[0],
				s_NAND_ucPageStatus[0]
			},
			{
				&s_NAND_Die[1],
				sizeof( s_NAND_Die[1] ),
				s_NAND_BlockFeature[1],
				sizeof( s_NAND_BlockFeature[1] ),
				s_NAND_uiPMD[1],
				s_NAND_uiMTB[1],
				s_NAND_CMT[1],
				sizeof( s_NAND_CMT[1] ),
				s_NAND_Entry[1][0],
				sizeof( s_NAND_Entry[1] ),
				s_NAND_uiECD[1],
				s_NAND_ucPageStatus[1]
			}
		};
		TNFTL_BUFFER_TABLE_T sTnftlBufTable =
		{
			NULL,
			BLOCK_COUNT_MAX,
			PMD_COUNT_MAX,
			CMT_COUNT_MAX,
			ENTRY_COUNT_MAX,
			ECD_COUNT_MAX,
			PAGE_STATUS_SIZE_IN_BYTE
		};
		sTnftlBufTable.pBufferTablePerDie = sTnftlBufPerDie;	// for avoiding the compile warning messages
		if( TNFTL_Init_Buffer( pTnftlObj, &sTnftlBufTable, DIE_COUNT_MAX ) != TNFTL_SUCCESS )
		{
			ND_TRACE( "-NAND_Init: TNFTL_Init_Buffer() is failed.\n" );
			NAND_LeaveLock();
			return NAND_ERROR_MEMORY_ALLOCATION;
		}
	}

	for( i = 0 ; i < TNFTL_PAGE_BUFFER_COUNT ; i++ )
	{
		pTnftlObj->sPageBuffer[i].puiDataBuffer = &s_NAND_uiPageBuffer[i][0];
		pTnftlObj->sPageBuffer[i].puiTagBuffer = &s_NAND_uiPageBuffer[i][PAGE_DWORDSIZE_MAX];
	}

	pTnftlObj->puiROTempPageBuffer = s_NAND_uiROTempPageBuffer;
	pTnftlObj->uiRWTempPageBufferPageAddress = (unsigned int)-1;
	pTnftlObj->puiRWTempPageBuffer = s_NAND_uiRWTempPageBuffer;
	pTnftlObj->fRWTempPageBufferLock = FALSE;

	if( TNFTL_Init_Query_NANDFeature( &s_NAND_TnftlObj[uiDrvNum] ) != TNFTL_SUCCESS )
	{
		ND_TRACE( "-NAND_Init: TNFTL_Init_Query_NANDFeature() is failed.\n" );
		NAND_LeaveLock();
		return NAND_ERROR_INIT_FAILURE;
	}

////////////////////////////////////////////////////////////////////////
// verify each parameter
	NAND_ASSERT( pTnftlObj->uiBytesPerPage <= PAGE_BYTESIZE_MAX );
	NAND_ASSERT( pTnftlObj->uiBlocksPerDie <= BLOCK_COUNT_MAX );
	NAND_ASSERT( pTnftlObj->uiPagesPerBlock <= PAGES_PER_BLOCK_MAX );
	NAND_ASSERT( pTnftlObj->uiDieCount <= DIE_COUNT_MAX );
////////////////////////////////////////////////////////////////////////

#if defined(FEATURE_TNFTL_DEBUG)
	NAND_IO_SuppressMessage( FALSE );
#else
	NAND_IO_SuppressMessage( TRUE );
#endif

	#if !defined(NAND_FOR_KERNEL)
	if( eType == NAND_INIT_TYPE_SKIP_BLOCK_INIT )
	{
		if( NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_VERIFY ) != TRUE )
		{
			ND_TRACE( "-NAND_Init: Area Info. is not verified.\n" );
			NAND_LeaveLock();
			return NAND_ERROR_INIT_AREA_CHANGE;
		}
		TNFTL_Init_Blocks( pTnftlObj, NAND_INIT_RO_ONLY);
	}
	else
	#endif
	{
		TNFTL_Init_Blocks( pTnftlObj, NAND_INIT_WHOLE_AREA);
	}

#if !defined(NAND_FOR_KERNEL)
	if( eType == NAND_INIT_TYPE_LOWFORMAT )
	{
		ND_TRACE( "+Low Format\n" );
		TNFTL_Format( pTnftlObj, TNFTL_FORMAT_NORMAL, (TNFTL_ProgressHandler)fnProgressHandler );
		ND_TRACE( "-Low Format\n" );
		NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_RESET );
		#if defined(USED_WITH_NOR_BOOT) || defined(BOOTSD_INCLUDE)
		// GoldenInfo should be write here to avoid failure of initialization
		if( TNFTL_FW_Write_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType() ) != TNFTL_SUCCESS )
		{
			NAND_LeaveLock();
			return ERR_FWUG_FAIL_GMCDATAWRITE;
		}
		#endif
	}
	else if( eType == NAND_INIT_TYPE_LOWFORMAT_DEBUG_CHECKANDERASE )
	{
		ND_TRACE( "+Debug Format (CheckAndErase)\n" );
		TNFTL_Format( pTnftlObj, TNFTL_FORMAT_DEBUG_CHECKANDERASE, (TNFTL_ProgressHandler)fnProgressHandler );
		ND_TRACE( "-Debug Format (CheckAndErase)\n" );
		NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_RESET );
		#if defined(USED_WITH_NOR_BOOT)|| defined(BOOTSD_INCLUDE)
		if( TNFTL_FW_Write_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType() ) != TNFTL_SUCCESS )
		{
			NAND_LeaveLock();
			return ERR_FWUG_FAIL_GMCDATAWRITE;
		}
		#endif
	}
	else if( eType == NAND_INIT_TYPE_LOWFORMAT_DEBUG_ERASE_ONLY )
	{
		ND_TRACE( "+Debug Format (Erase Only)\n" );
		TNFTL_Format( pTnftlObj, TNFTL_FORMAT_DEBUG_ERASE, (TNFTL_ProgressHandler)fnProgressHandler );
		ND_TRACE( "-Debug Format (Erase Only)\n" );
		NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_RESET );
		#if defined(USED_WITH_NOR_BOOT)|| defined(BOOTSD_INCLUDE)
		if( TNFTL_FW_Write_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType() ) != TNFTL_SUCCESS )
		{
			NAND_LeaveLock();
			return ERR_FWUG_FAIL_GMCDATAWRITE;
		}
		#endif
	}
#endif

	if( NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_VERIFY ) != TRUE )
	{
		#if defined(BOOTSD_INCLUDE) && !defined(NAND_FOR_KERNEL)
		ND_TRACE( "+Low Format\n" );
		TNFTL_Format( pTnftlObj, TNFTL_FORMAT_NORMAL, (TNFTL_ProgressHandler)fnProgressHandler );
		ND_TRACE( "-Low Format\n" );
		NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_RESET );
		// GoldenInfo should be write here to avoid failure of initialization
		if( TNFTL_FW_Write_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType() ) != TNFTL_SUCCESS )
		{
			NAND_LeaveLock();
			ND_TRACE("NAND Golden Info Write Fail ! \n");
			return ERR_FWUG_FAIL_GMCDATAWRITE;
		}		

		if( NAND_Control_AreaInfo( uiDrvNum, NAND_CTRL_AREA_INFO_VERIFY ) != TRUE ) 		
		#endif
		{
			ND_TRACE( "-NAND_Init: Area Info. is not verified.\n" );
			NAND_LeaveLock();
			return NAND_ERROR_INIT_AREA_CHANGE;
		}		
	}

	if( ( eType != NAND_INIT_TYPE_FAST ) && ( eType != NAND_INIT_TYPE_SKIP_BLOCK_INIT ) )
	{
		TNFTL_Init_Collect_RW_Blocks( &s_NAND_TnftlObj[uiDrvNum] );
		TNFTL_PMT_Check( &s_NAND_TnftlObj[uiDrvNum] );
	}

	{
		unsigned int uiSectorCount;

		ND_TRACE( "Die=%d, B/D=%d, P/B=%d, Pg=%d CW=%d Pln/D=%d\n"
		          , pTnftlObj->uiDieCount
		          , pTnftlObj->uiBlocksPerDie
		          , pTnftlObj->uiPagesPerBlock
		          , pTnftlObj->uiBytesPerPage
		          , pTnftlObj->uiCodeWordSize
		          , ( unsigned int )pTnftlObj->ucPlaneCount );

		ND_TRACE( "[RO]" );
		for( i = 0 ; i < NAND_RO_AREA_COUNT_MAX ; i++ )
		{
			uiSectorCount = NAND_Area_GetTotalSectorCount( uiDrvNum, NAND_RO_0_AREA_INDEX + i );
			if( uiSectorCount != 0 )
			{
				_ND_TRACE( " %d:%d", i, uiSectorCount );
			}
		}
		_ND_TRACE( " sectors\n" );

		if( ( eType != NAND_INIT_TYPE_FAST ) && ( eType != NAND_INIT_TYPE_SKIP_BLOCK_INIT ) )
		{
			ND_TRACE( "[RW]" );
			for( i = 0 ; i < NAND_RW_AREA_COUNT_MAX - 1 ; i++ )
			{
				uiSectorCount = NAND_Area_GetTotalSectorCount( uiDrvNum, NAND_RW_0_AREA_INDEX + i );
				if( uiSectorCount != 0 )
				{
					_ND_TRACE( " %d:%d", NAND_RW_0_AREA_INDEX + i, uiSectorCount );
				}
			}

			uiSectorCount = NAND_Area_GetTotalSectorCount( uiDrvNum, NAND_DATA_AREA_INDEX );
			if( uiSectorCount != 0 )
			{
				_ND_TRACE( " Data:%d", uiSectorCount );
			}
			_ND_TRACE( " sectors\n" );
		}
	}

	//ND_TRACE( "-NAND_Init\n" );

	#if defined(Debug_Print)
	{
		TNFTL_Print_Debug_info(&s_NAND_TnftlObj[uiDrvNum]);
	}
	#endif

	NAND_LeaveLock();
	return NAND_SUCCESS;
}

static unsigned int s_RO_Area_uiPageBuffer[PAGE_DWORDSIZE_MAX];
static unsigned char s_RO_Area_ucCurrentAreaIndex = ( unsigned char ) - 1;
static unsigned int s_RO_Area_uiCurrentSectorAddress = ( unsigned int ) - 1;
int NAND_RO_Area_Flush_WriteCache( unsigned int uiDrvNum )
{
	if( s_RO_Area_ucCurrentAreaIndex != ( unsigned char ) - 1 && s_RO_Area_uiCurrentSectorAddress != ( unsigned int ) - 1 )
	{
		unsigned int uiPageAddress = s_RO_Area_uiCurrentSectorAddress / TNFTL_Get_SectorsPerPage( &s_NAND_TnftlObj[uiDrvNum] );
		TNFTL_RO_Area_WritePage( &s_NAND_TnftlObj[uiDrvNum], s_RO_Area_ucCurrentAreaIndex, uiPageAddress, s_RO_Area_uiPageBuffer, 1 );
	}

	s_RO_Area_ucCurrentAreaIndex = ( unsigned char ) - 1;
	s_RO_Area_uiCurrentSectorAddress = ( unsigned int ) - 1;

	return 0;
}

static int NAND_RO_Area_WriteSector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	int res = 0;
	unsigned int uiROIndex = uiAreaIndex;
	unsigned int uiSectorsPerPage = TNFTL_Get_SectorsPerPage( &s_NAND_TnftlObj[uiDrvNum] );
	unsigned int uiPageAddress = uiSectorAddress / uiSectorsPerPage;
	unsigned int uiSectorOffset = uiSectorAddress % uiSectorsPerPage;

	{
		unsigned int uiCurrentPageAddress = ( s_RO_Area_uiCurrentSectorAddress == ( unsigned int ) - 1 ) ? ( unsigned int ) - 1 : s_RO_Area_uiCurrentSectorAddress / uiSectorsPerPage;
		if( s_RO_Area_ucCurrentAreaIndex != ( unsigned char )uiAreaIndex || uiCurrentPageAddress != uiPageAddress )
		{
			res = NAND_RO_Area_Flush_WriteCache( uiDrvNum );
			memset( s_RO_Area_uiPageBuffer, 0xFF, sizeof( s_RO_Area_uiPageBuffer ) );
		}
	}

	if( res == 0 && uiSectorOffset )
	{
		unsigned int uiWriteSectorMax = uiSectorsPerPage - uiSectorOffset;
		unsigned int uiWriteSector = ( uiSectorCount > uiWriteSectorMax ) ? uiWriteSectorMax : uiSectorCount;

		NAND_Util_memcpy( &s_RO_Area_uiPageBuffer[uiSectorOffset * ( 512 / 4 )], pvBuffer, uiWriteSector * 512 );
		s_RO_Area_ucCurrentAreaIndex = ( unsigned char )uiAreaIndex;
		s_RO_Area_uiCurrentSectorAddress = ( uiPageAddress * uiSectorsPerPage ) + uiSectorOffset + uiWriteSector;

		uiSectorCount -= uiWriteSector;
		pvBuffer = ( void * )( ( unsigned int )pvBuffer + ( uiWriteSector * 512 ) );

		if( uiWriteSector == uiWriteSectorMax )
		{
			if( TNFTL_RO_Area_WritePage( &s_NAND_TnftlObj[uiDrvNum], uiAreaIndex, uiPageAddress, s_RO_Area_uiPageBuffer, 1 ) != TNFTL_SUCCESS )
				res = -1;

			uiPageAddress += 1;

			s_RO_Area_ucCurrentAreaIndex = ( unsigned char ) - 1;
			s_RO_Area_uiCurrentSectorAddress = ( unsigned int ) - 1;
		}
	}

	if( res == 0 )
	{
		unsigned int uiPageCount = uiSectorCount / uiSectorsPerPage;
		if( uiPageCount )
		{
			if( TNFTL_RO_Area_WritePage( &s_NAND_TnftlObj[uiDrvNum], uiROIndex, uiPageAddress, pvBuffer, uiPageCount ) != TNFTL_SUCCESS )
			{
				res = -1;
			}

			pvBuffer = ( void * )( ( unsigned int )pvBuffer + ( uiPageCount * uiSectorsPerPage * 512 ) );
			uiSectorCount -= uiPageCount * uiSectorsPerPage;
			uiPageAddress += uiPageCount;

			s_RO_Area_ucCurrentAreaIndex = ( unsigned char ) - 1;
			s_RO_Area_uiCurrentSectorAddress = ( unsigned int ) - 1;
		}
	}

	if( res == 0 && uiSectorCount )
	{
		NAND_ASSERT( uiSectorCount < uiSectorsPerPage );

		NAND_Util_memcpy( s_RO_Area_uiPageBuffer, pvBuffer, uiSectorCount * 512 );
		s_RO_Area_ucCurrentAreaIndex = ( unsigned char )uiAreaIndex;
		s_RO_Area_uiCurrentSectorAddress = ( uiPageAddress * uiSectorsPerPage ) + uiSectorCount;
	}

	return res;
}

#if defined(_LINUX_)
#if defined(NAND_FOR_KERNEL)
int NAND_HDWriteSector(int nDrvNo, unsigned int LBA, unsigned int nSecsize, void* nReaBuffer)
{
	return -1;
}
EXPORT_SYMBOL(NAND_HDWriteSector);
int NAND_HDReadSector(int nDrvNom, unsigned int LBA, unsigned int nSecsize, void* nReadBuffer)
{
	return -1;
}
EXPORT_SYMBOL(NAND_HDReadSector);
#endif

int NAND_MTD_Area_WritePage( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiPageAddress, void *pvBuffer, void *pvTag, unsigned char bUseYaffs )
{
	int res = -1;

	if( uiAreaIndex < NAND_RO_AREA_COUNT_MAX )
	{
		/*
			Warning : If you want to write data to RO area, you should carefully do it.
			          TNFTL writes data to RO area with simple process.
			            1. erase NAND block if it's needed.
			              * note) TNFTL do not preserve previous data where the block is erased and new data is partially written.
			            2. writes data you requested.
		*/
		res = TNFTL_MTD_Area_WritePage( &s_NAND_TnftlObj[uiDrvNum], uiAreaIndex, uiPageAddress, pvBuffer, pvTag, bUseYaffs );
	}	
	else
	{
		s_NAND_DRV_errorDrive[uiDrvNum] = NAND_ERROR_WRONG_PARAMETER;
	}

	return res;
}

int NAND_MTD_Area_ReadPage( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiPageAddress, void *pvBuffer, void *pvTag, unsigned char bUseYaffs )
{
	int res = -1;

	if( uiAreaIndex < NAND_RO_AREA_COUNT_MAX )
	{
		if( TNFTL_MTD_Area_ReadPage( &s_NAND_TnftlObj[uiDrvNum], uiAreaIndex, uiPageAddress, pvBuffer, pvTag, bUseYaffs ) == TNFTL_SUCCESS )
			res = 0;
	}
	else
	{
		NAND_SetLastError( uiDrvNum, NAND_ERROR_WRONG_PARAMETER );
	}

	return res;
}

int  NAND_MTD_Area_Erase( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiPageAddr, unsigned int isWholeArea )
{
	return TNFTL_RO_Area_Erase( &s_NAND_TnftlObj[uiDrvNum], uiAreaIndex, uiPageAddr, isWholeArea );
}

int NAND_Flush_Cacheblock(unsigned int uiDrvNum)
{
	return TNFTL_Convert_CacheBlocks(&s_NAND_TnftlObj[uiDrvNum]);
}

int NAND_Padding_Cacheblock(unsigned int uiDrvNum)
{
	return TNFTL_Padding_CacheBlocks(&s_NAND_TnftlObj[uiDrvNum]);
}

/******************************************************************************
*
*	int NAND_MTD_Update( unsigned int PageOffset, unsigned char *Databuffer )
*
*	Input	:
*	Output	:
*	Return	:
*
*	Description :
*
*******************************************************************************/
int NAND_MTD_Update( unsigned int PageOffset, unsigned char *DataBuffer, unsigned int  partition_type )
{
	NAND_IO_DEVINFO *sDevInfo = (NAND_IO_DEVINFO*)s_NAND_TnftlObj[0].pvNandDevice;
	unsigned int uiAreaIndex = 0;
	unsigned int bUseYaffs = 0;
	unsigned char pTagBuffer[20];
	
	// Get Partition Specific. Get Start Offset.
	switch( partition_type ){
		case MTD_PART_BOOT : 
			bUseYaffs = FALSE;
			//uiWriteSize	= sDevInfo->Feature.PageSize + 0;
			uiAreaIndex = NAND_MTD_BOOT_INDEX;
			break;
		case MTD_PART_RECOVERY:
			bUseYaffs = FALSE;
			//uiWriteSize	= sDevInfo->Feature.PageSize + 0;
			uiAreaIndex = NAND_MTD_RECOVERY_INDEX;
			break;
		case MTD_PART_USERDATA:
			{
				//unsigned int uiTagSize;
				//if( sDevInfo->Feature.PageSize == 16384 )
				//	uiTagSize = 512;
				//else if( sDevInfo->Feature.PageSize == 8192 )
				//	uiTagSize = 256;
				//else if( sDevInfo->Feature.PageSize == 4096 )
				//	uiTagSize = 128;
				//else if( sDevInfo->Feature.PageSize == 2048 )
				//	uiTagSize = 64;

				bUseYaffs = TRUE;
				//uiWriteSize	= sDevInfo->Feature.PageSize + uiTagSize;
				uiAreaIndex = NAND_MTD_USERDATA_INDEX;

			}
			break;
		default : 
			ND_TRACE(" Write Unknowed Partition : %d  \n", partition_type );			
			break;
	}

		
	// if PageOffset is zero, Erase Partition.
	if( PageOffset == 0 )
	{
		ND_TRACE(" Erase MTD Area : %d\n", uiAreaIndex );
		TNFTL_RO_Area_Erase( &s_NAND_TnftlObj[0], uiAreaIndex, 0, 1/* Erase Whole Index Area*/ );
	}

	ND_TRACE("[MTD-Update] [Part:%d] [PageOffset:%d] ", uiAreaIndex, PageOffset );

	if( partition_type == MTD_PART_USERDATA )
	{
		memcpy( pTagBuffer, &DataBuffer[ sDevInfo->Feature.PageSize ], 20 );
	}
	else
	{
		memset( pTagBuffer, 0xFF, 20 );
	}
	
	if( NAND_MTD_Area_WritePage(0, uiAreaIndex, PageOffset, (void*)DataBuffer, (void*)&pTagBuffer[0], bUseYaffs) )
	{
		ND_TRACE(" Write MTD Data Fail ! \n");
		return -1;
	}
	
	return SUCCESS;
}
#endif

int NAND_Area_WriteSector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	int res = -1;
	NAND_EnterLock();

	if( uiAreaIndex < NAND_RO_AREA_COUNT_MAX )
	{
		/*
			Warning : If you want to write data to RO area, you should carefully do it.
			          TNFTL writes data to RO area with simple process.
			            1. erase NAND block if it's needed.
			              * note) TNFTL do not preserve previous data where the block is erased and new data is partially written.
			            2. writes data you requested.
		*/
		res = NAND_RO_Area_WriteSector( uiDrvNum, uiAreaIndex, uiSectorAddress, uiSectorCount, pvBuffer );
	}
	else if( uiAreaIndex < NAND_RO_AREA_COUNT_MAX + NAND_RW_AREA_COUNT_MAX )
	{
		NAND_RO_Area_Flush_WriteCache( uiDrvNum );
		if( TNFTL_WriteSector( &s_NAND_TnftlObj[uiDrvNum], uiAreaIndex - NAND_RO_AREA_COUNT_MAX, uiSectorAddress, pvBuffer, uiSectorCount ) == TNFTL_SUCCESS )
			res = 0;
	}
	else
	{
		s_NAND_DRV_errorDrive[uiDrvNum] = NAND_ERROR_WRONG_PARAMETER;
	}

	NAND_LeaveLock();
	return res;
}

int NAND_Area_ReadSector( unsigned int uiDrvNum, unsigned int uiAreaIndex, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	int res = -1;
	NAND_EnterLock();

	if( uiAreaIndex < NAND_RO_AREA_COUNT_MAX )
	{
		unsigned int uiROIndex = uiAreaIndex;
		NAND_RO_Area_Flush_WriteCache( uiDrvNum );
		if( TNFTL_RO_Area_ReadSector( &s_NAND_TnftlObj[uiDrvNum], uiROIndex, uiSectorAddress, pvBuffer, uiSectorCount ) == TNFTL_SUCCESS )
			res = 0;
	}
	else if( uiAreaIndex < NAND_RO_AREA_COUNT_MAX + NAND_RW_AREA_COUNT_MAX )
	{
		if( TNFTL_ReadSector( &s_NAND_TnftlObj[uiDrvNum], uiAreaIndex - NAND_RO_AREA_COUNT_MAX, uiSectorAddress, pvBuffer, uiSectorCount ) == TNFTL_SUCCESS )
			res = 0;
	}
	else
	{
		NAND_SetLastError( uiDrvNum, NAND_ERROR_WRONG_PARAMETER );
	}

	NAND_LeaveLock();
	return res;
}

int NAND_WriteSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	NAND_EnterLock();
	if( TNFTL_WriteSector( &s_NAND_TnftlObj[uiDrvNum], NAND_DATA_AREA_INDEX - NAND_RO_AREA_COUNT_MAX, uiSectorAddress, pvBuffer, uiSectorCount ) == TNFTL_SUCCESS )
	{
		NAND_LeaveLock();
		return 0;
	}

	NAND_LeaveLock();
	return -1;
}

int NAND_ReadSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	NAND_EnterLock();
	if( TNFTL_ReadSector( &s_NAND_TnftlObj[uiDrvNum], NAND_DATA_AREA_INDEX - NAND_RO_AREA_COUNT_MAX, uiSectorAddress, pvBuffer, uiSectorCount ) == TNFTL_SUCCESS )
	{
		NAND_LeaveLock();
		return 0;
	}

	NAND_LeaveLock();
	return -1;
}

int NAND_ReadPhysicalBlock( unsigned int uiBlockAddress, unsigned int uiReadSize, void *pvBuffer )
{
	NAND_IO_DEVINFO *sDevInfo = (NAND_IO_DEVINFO*)s_NAND_TnftlObj[0].pvNandDevice;
	NAND_IO_ERROR_T	res = SUCCESS;
	unsigned char *pucPageBuffer = (unsigned char*) pvBuffer;
	unsigned int	uiPageOffset, uiPageAddress, uiBufferOffset = 0;	

	uiPageAddress = uiBlockAddress << sDevInfo->ShiftPpB; 

	for( uiPageOffset = 0; uiPageOffset < sDevInfo->Feature.PpB; uiPageOffset++ )
	{
		res = NAND_IO_ReadUserSizePage( sDevInfo, uiPageAddress + uiPageOffset, 0, uiReadSize, &pucPageBuffer[uiBufferOffset] );
		
		if( res != NAND_IO_SUCCESS )
		{
			ND_TRACE("Physical Dump Error ! 0x%08X\n", res );
			break;
		}
		
		uiBufferOffset += uiReadSize;
	}

	return res;
}


int	NAND_GetSerialNumber( unsigned char *pucSN, unsigned int uiSize )
{
	TNFTL_OBJECT_T *pTnftlObj = &s_NAND_TnftlObj[0];
	unsigned int uiCRC;
	NAND_EnterLock();
	TNFTL_FW_Read_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ) );
	NAND_LeaveLock();
	uiCRC = FWUG_CalcCrc8( s_NandUserInfoTable.ucSerialNumber, sizeof( s_NandUserInfoTable.ucSerialNumber ) );
	if( uiCRC != 0 && uiCRC == s_NandUserInfoTable.uiSerialNumberCRC && uiSize >= sizeof( s_NandUserInfoTable.ucSerialNumber ) )
	{
		memcpy( pucSN, s_NandUserInfoTable.ucSerialNumber, min( uiSize, sizeof( s_NandUserInfoTable.ucSerialNumber ) ) );
		return min( uiSize, sizeof( s_NandUserInfoTable.ucSerialNumber ) );
	}
	else
	{
		return 0;
	}
}

int NAND_SetSerialNumber( unsigned char *ucData, unsigned int uiSize )
{
	memcpy( s_NandUserInfoTable.ucSerialNumber, ucData , min( uiSize, sizeof( s_NandUserInfoTable.ucSerialNumber ) ) );
	s_NandUserInfoTable.uiSerialNumberCRC = FWUG_CalcCrc8( s_NandUserInfoTable.ucSerialNumber, sizeof( s_NandUserInfoTable.ucSerialNumber ) );
	return min( uiSize, sizeof( s_NandUserInfoTable.ucSerialNumber ) );
}

int NAND_WriteSerialNumber( unsigned char *ucData, unsigned int uiSize )
{
	TNFTL_OBJECT_T *pTnftlObj = &s_NAND_TnftlObj[0];
	unsigned int uiBlockAddress;
	unsigned int res;

	NAND_EnterLock();
	
	// read golden info
	res = TNFTL_FW_Read_GoldenInfo( pTnftlObj );
	if( res != TNFTL_SUCCESS )
	{
		NAND_LeaveLock();
		return res;
	}
	
	// read golden block
	res = TNFTL_FW_Read_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof(s_NandUserInfoTable) );
	if( res != TNFTL_SUCCESS )
	{
		NAND_LeaveLock();	
		return res;
	}
	
	// backup golden block to next block
	res = TNFTL_FW_Backup_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType(), &uiBlockAddress );
	if( res != TNFTL_SUCCESS )
	{
		NAND_LeaveLock();
		return res;
	}

	// set serial number
	NAND_SetSerialNumber( ucData, uiSize);

	// write golden block
	res = TNFTL_FW_Write_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType() );
	if( res != TNFTL_SUCCESS )
	{
		TNFTL_FW_Recovery_BackupBlock( pTnftlObj, uiBlockAddress );
		TNFTL_FW_Erase_BackupBlock( pTnftlObj, uiBlockAddress );
		NAND_LeaveLock();
		return res;
	}
	
	// verify golden block
	res = NAND_Control_AreaInfo( 0, NAND_CTRL_AREA_INFO_VERIFY );
	if( res != TRUE )
	{
		TNFTL_FW_Recovery_BackupBlock( pTnftlObj, uiBlockAddress );
		TNFTL_FW_Erase_BackupBlock( pTnftlObj, uiBlockAddress );
		NAND_LeaveLock();
		return res;
	}
	
	// erase backup block
	TNFTL_FW_Erase_BackupBlock( pTnftlObj, uiBlockAddress );

	NAND_LeaveLock();
	return res;
}

#if defined(TCC800X)
int NAND_WriteFirmware_For_TCC800X( unsigned int uiDrvNum, unsigned char *pucRomBuffer, unsigned int uiRomFileSize ) 
{
	int res = 0;
	/////////////////////////////////////////////////////////////////////
	// Block 0 : Golden Master Cluster. ( Page 0 ~ 1 )
	// Block 1 : Master Cluster 1.
	// Block 2 : Master Cluster 2.
	// Block 3 ~ x : Bootloader 1.
	// Block x ~ y : Bootloader 2.  
	// Block y ~   : Kernel & Ramdisk.
	/////////////////////////////////////////////////////////////////////
	TNFTL_OBJECT_T *pTnftlObj = &s_NAND_TnftlObj[uiDrvNum];
	
	// Collect Good Block & Parameter Setting For MC and GMC.
	TNFTL_FW_PreProcess( pTnftlObj, uiRomFileSize );
	
	// Write Bootloader.
	if( TNFTL_FW_Write_BootCode_For_TCC800X( pTnftlObj, pucRomBuffer, uiRomFileSize, BOOT_ROM_WRITE_TYPE_START ) != TNFTL_SUCCESS )
	{
		res = ERR_FWUG_FAIL_CODEDATAWRITE;
	}
	else if( TNFTL_FW_Write_BootCode_For_TCC800X( pTnftlObj, pucRomBuffer, uiRomFileSize, BOOT_ROM_WRITE_TYPE_MIRROR_IMAGE_START ) != TNFTL_SUCCESS )
	{
		res = ERR_FWUG_FAIL_CODEDATAWRITE;
	}
	//else if( TNFTL_FW_Verify_BootCode( pTnftlObj, uiRomFileSize, uiRomFileCRC ) != TNFTL_SUCCESS )
	//{
	//	res = ERR_FWUG_FAIL_VERIFY;
	//}
	
	// Write MC
	TNFTL_FW_Write_MC( pTnftlObj, 0 );
	TNFTL_FW_Write_MC( pTnftlObj, 1 );

	// Write GMC
	TNFTL_FW_Write_GMC( pTnftlObj );

	NAND_Control_AreaInfo( 0, NAND_CTRL_AREA_INFO_RESET );
	TNFTL_FW_Write_GoldenBlock( pTnftlObj, NULL, 0, NAND_IO_GetBootType() );

	return res;
}
#else
int NAND_WriteFirmware( unsigned int uiDrvNum, unsigned char *pucRomBuffer, unsigned int uiFWSize )
{
	int res = 0;
	TNFTL_OBJECT_T *pTnftlObj = &s_NAND_TnftlObj[uiDrvNum];
	unsigned int uiRomFileSize;
	#if defined(TSBM_ENABLE) || defined(OTP_UID_INCLUDE)
	struct TCC_SB_HEADER sTCC_SB_Header;		
	#endif

	#ifdef NAND_USE_COMPRESS		
	char* pout_buff;	
	unsigned int compressed_size=0;
	#if defined(NAND_FOR_KERNEL)
	pout_buff = (char *)malloc(uiFWSize);
	#else 
	pout_buff = comp_buff;
	#endif	
	#endif

	NAND_EnterLock();
	NAND_ASSERT( uiDrvNum < DRIVE_COUNT_MAX );

	{
		unsigned long uiBaseAddress;
		unsigned long uiNandLoaderOffset;
		unsigned int  uiNandLoaderSize;
		unsigned long ulMemInitCodeOffset;
		unsigned int  uiMemInitCodeSize;
		unsigned long ulTargetAddress;

		#if defined(TSBM_ENABLE) || defined(OTP_UID_INCLUDE)
		TNFTL_memcpy( &sTCC_SB_Header, pucRomBuffer, 512 );
		uiBaseAddress		= (unsigned int)sTCC_SB_Header.base_address;
		uiNandLoaderOffset	= (unsigned int)sTCC_SB_Header.nand_bl1_start;
		uiNandLoaderSize	= (unsigned int)sTCC_SB_Header.nand_bl1_size;
		ulMemInitCodeOffset = (unsigned long)sTCC_SB_Header.dram_init_start;
		uiMemInitCodeSize	= (unsigned long)sTCC_SB_Header.dram_init_size;		
		ulTargetAddress		= (unsigned long)sTCC_SB_Header.target_address;
		#else		
		uiBaseAddress		= GetDword( &pucRomBuffer[ROM_FIRMWARE_BASE_OFFSET] );
		uiNandLoaderOffset	= GetDword( &pucRomBuffer[ROM_BL1_NAND_OFFSET] ) - uiBaseAddress;
		uiNandLoaderSize	= GetDword( &pucRomBuffer[uiNandLoaderOffset + 8] );
		ulMemInitCodeOffset = GetDword( &pucRomBuffer[ROM_CONFIG_CODE_START_OFFSET] ) - uiBaseAddress;
		uiMemInitCodeSize	= GetDword( &pucRomBuffer[ROM_CONFIG_CODE_END_OFFSET] ) - GetDword( &pucRomBuffer[ROM_CONFIG_CODE_START_OFFSET] );		
		ulTargetAddress		= GetDword( &pucRomBuffer[uiNandLoaderOffset + 4] );						
		#endif

		if( TNFTL_FW_Write_BL1( pTnftlObj, 
								&pucRomBuffer[uiNandLoaderOffset], ulTargetAddress, uiNandLoaderSize, 
								&pucRomBuffer[ulMemInitCodeOffset], uiMemInitCodeSize ) != TNFTL_SUCCESS )
		{
			res = ERR_FWUG_FAIL_CODEDATAWRITE;
		}

		#if defined(TSBM_ENABLE) || defined(OTP_UID_INCLUDE)
		pucRomBuffer += sTCC_SB_Header.bootloader_start;
		uiRomFileSize = sTCC_SB_Header.bootloader_size;		
		#else
		uiRomFileSize = uiFWSize;		
		#endif
		
		#ifdef NAND_USE_COMPRESS
		ND_TRACE("Compressed Bootloader\n");
		compressed_size = TNFTL_Compress(pucRomBuffer, pout_buff, uiRomFileSize);

		if(compressed_size >= comp_buff_size)
		{
			ND_TRACE("Compressed file is bigger than compress buff !!! [file size = %d]\n", compressed_size);
			NAND_LeaveLock();
			return -1;
		}
		else
		{
			ND_TRACE("compressed filesize = %d\n", compressed_size);
			TNFTL_FW_SetBootCodeParameter( pTnftlObj, uiBaseAddress, uiRomFileSize, compressed_size);
	}
		#else
		ND_TRACE("Non-compressed Bootloader!!\n");
		TNFTL_FW_SetBootCodeParameter( pTnftlObj, uiBaseAddress, uiRomFileSize, 0 );
		#endif
	}

	#ifdef NAND_USE_COMPRESS
		pucRomBuffer = pout_buff;        /* Rom buff and file size are replaced */
		uiRomFileSize = compressed_size;	
	#endif

	if( res == 0 )
	{
		unsigned int uiRomFileCRC;		
		
		uiRomFileCRC = FWUG_CalcCrc8( (unsigned char*)pucRomBuffer, uiRomFileSize );

		if( TNFTL_FW_Write_BootCode( pTnftlObj, pucRomBuffer, uiRomFileSize, BOOT_ROM_WRITE_TYPE_START ) != TNFTL_SUCCESS )
		{
			res = ERR_FWUG_FAIL_CODEDATAWRITE;
		}
		else if( TNFTL_FW_Write_BootCode( pTnftlObj, pucRomBuffer, uiRomFileSize, BOOT_ROM_WRITE_TYPE_MIRROR_IMAGE_START ) != TNFTL_SUCCESS )
		{
			res = ERR_FWUG_FAIL_CODEDATAWRITE;
		}
		else if( TNFTL_FW_Write_GoldenBlock( pTnftlObj, ( unsigned char * )&s_NandUserInfoTable, sizeof( s_NandUserInfoTable ), NAND_IO_GetBootType() ) != TNFTL_SUCCESS )
		{
			res = ERR_FWUG_FAIL_GMCDATAWRITE;
		}
		else if( TNFTL_FW_Verify_BootCode( pTnftlObj, uiRomFileSize, uiRomFileCRC ) != TNFTL_SUCCESS )
		{
			res = ERR_FWUG_FAIL_VERIFY;
		}
	}

	NAND_LeaveLock();
	return res;
}
#endif

/******************************************************************************
*
*	int				NAND_Ioctl
*
*	Input	:
*	Output	:
*	Return	:
*
*	Description :
*
*******************************************************************************/
int NAND_Ioctl( int function, void *param )
{
	int res = 0;

	#if defined(_NU_)
	DISK_IOCTL_PARAM *Param = (DISK_IOCTL_PARAM *)param;
	#endif

	switch( function )
	{
		case	DEV_INITIALIZE:
#if !defined(NAND_FOR_KERNEL)
			if( NAND_Init( 0, NAND_INIT_TYPE_FAST, NULL ) != NAND_SUCCESS )
#else
			if( NAND_Init( 0, NAND_INIT_TYPE_NORMAL, NULL ) != NAND_SUCCESS )
#endif
				res = EINITFAIL;
			break;

		case DEV_DEVICE_FAST_INIT:
			if( NAND_Init( 0, NAND_INIT_TYPE_SKIP_BLOCK_INIT, NULL ) != NAND_SUCCESS )
				res = EINITFAIL;
			break;

		case	DEV_GET_DISKINFO:
		{
			ioctl_diskinfo_t	*info = ( ioctl_diskinfo_t * ) param;

#if defined(NU_FILE_INCLUDE)
			gFormatType	= TC_LOWLEVEL_NO;
#endif

			info->sector_size	= 512;
			NAND_EnterLock();
			info->Total_sectors	= NAND_Area_GetTotalSectorCount( 0, NAND_DATA_AREA_INDEX );
			NAND_LeaveLock();

#if defined(_NU_)
			info->head			= 1;
			info->sector		= info->Total_sectors;
			info->cylinder		= 1;
#else
			info->head			= 0;
			info->sector		= 0;
			info->cylinder		= 0;
#endif
			break;
		}

		case DEV_GET_DISK_SIZE:
		{
			unsigned int *puiTotalSector = ( unsigned int * )param;
			NAND_EnterLock();
			*puiTotalSector = NAND_Area_GetTotalSectorCount( 0, NAND_DATA_AREA_INDEX );
			NAND_LeaveLock();
			break;
		}

		case	DEV_FORMAT_DISK:
		{
			//unsigned short	mode = *((unsigned short *)param);
			//
			//#if defined(NU_FILE_INCLUDE)
			//gFormatType	= TC_LOWLEVEL_NO;
			//#endif
			//
			//if ( gNAND_DrvInfo[0].NFTLDrvInfo->NANDType == NAND_TYPE_PURE_NAND )
			//{
			//    TNFTL_AREAFormat( gNAND_DrvInfo[0].NFTLDrvInfo,
			//				      &gNAND_DrvInfo[0].NFTLDrvInfo->PriPartition,
			//				      mode );
			//}
			//#ifdef NAND_LBA_INCLUDE
			//else
			//{
			////	NAND_IO_LBA_AREAClear( gLBA_DevInfo, NAND_LBA_DATA_AREA );
			//}
			//#endif

			res = 0;
			break;
		}

		case	DEV_ERASE_INIT:
		{
			ioctl_diskeraseinit_t	*erase = ( ioctl_diskeraseinit_t * ) param;
			erase = erase;
			break;
		}

		case	DEV_ERASE_BLOCK:
		{
			ioctl_diskerase_t	*erase = ( ioctl_diskerase_t * ) param;
			erase = erase;
			break;
		}
		case DEV_GET_PHYSICAL_INFO:
		{
			ioctl_diskphysical_t 	*ioctl_physical_info = (ioctl_diskphysical_t*)param;
			NAND_IO_DEVINFO			*sDevInfo = (NAND_IO_DEVINFO*)s_NAND_TnftlObj[0].pvNandDevice;

			ioctl_physical_info->uiDieCount				= s_NAND_TnftlObj[0].uiDieCount;
			ioctl_physical_info->uiBlockNumberOfDevice	= s_NAND_TnftlObj[0].uiBlocksPerDie;
			ioctl_physical_info->uiPageNumberOfBlock	= s_NAND_TnftlObj[0].uiPagesPerBlock;
			ioctl_physical_info->uiShiftPpB				= s_NAND_TnftlObj[0].uiPagesPerBlockShift;
			ioctl_physical_info->uiPageSize				= sDevInfo->Feature.PageSize;
			ioctl_physical_info->uiSpareSize			= sDevInfo->Feature.SpareSize;
			break;
		}

		case	DEV_WRITEBACK_ON_IDLE:
			break;

		case	DEV_ERASE_CLOSE:
			break;

		case	DEV_HIDDEN_READ_PAGE_4:
			break;
		case	DEV_SET_POWER:
			break;
		case	DEV_GET_MAX_SECTOR_PER_BLOCK:
		{
			unsigned short *value = ( unsigned short * )param;
			*value = ( unsigned short )s_NAND_TnftlObj[0].uiSectorsPerPage;
			break;
		}

		case	DEV_TELL_DATASTARTSECTOR:
			break;

		case	DEV_CHECK_CRC_NANDBOOT_IMAGE_ROM:
			break;

		case	DEV_RECOVERY_TCBOOT:
			break;
		case	DEV_GET_WRITE_PROTECT:
			break;
		case	DEV_GET_INSERTED:
			#if defined(_NU_)
			if( param != NULL )
			{
				Param->ioctl_param.GET_INSERTED_param.DevNo = DM_FIXED_DEV_BASE_NO;
				Param->ioctl_param.GET_INSERTED_param.DevType = (int)DISK_DEVICE_NAND;
			}
			#endif
			res = 1;
			break;
		case DEV_GET_HIDDEN_COUNT:
		{
			unsigned int *puiCount = ( unsigned int * )param;
			*puiCount = 0;//(unsigned int)gNAND_DrvInfo[0].NFTLDrvInfo->ExtendedPartitionNo;
			break;
		}
#ifdef NU_FILE_INCLUDE
		case	DEV_GET_HIDDEN_SIZE:
			*( int * )param	= -1;
			break;
#endif
		case	DEV_GET_INITED:
			res = 1;
			break;
		case DEV_GET_PLAYABLE_STATUS:
			res = 1;
			break;
		default:
			res = ENOTSUPPORT;
			break;
	}

	return res;
}

unsigned int NAND_GetBootStatus( void )
{
	unsigned int uiBootStatus = NAND_IO_GetBootStatus();
	if( ( uiBootStatus & MIRROR_CHECK_SIGNATURE_FIELD ) != MIRROR_CHECK_SIGNATURE )
		uiBootStatus = 0;

	return uiBootStatus;
}

static NAND_ERROR_T NAND_RecoveryBootCode( TNFTL_OBJECT_T *pTnftlObj )
{
	return TNFTL_FW_RecoveryBootCode( pTnftlObj );
}

static NAND_ERROR_T NAND_RecoveryMemInitCode( TNFTL_OBJECT_T *pTnftlObj )
{
	return TNFTL_FW_RecoveryMemInitCode( pTnftlObj );
}


void NAND_RecoveryBootArea( unsigned int uiBootStatus )
{
	unsigned int uiRes;
	TNFTL_OBJECT_T *pTnftlObj = NAND_GetObject( 0 );

	// bootloader recovery process

	if( uiBootStatus & MEMINIT_MIRROR_CHECK )
	{
		ND_TRACE( "'MemoryInitCode' Restoration ... " );
		uiRes = NAND_RecoveryMemInitCode( pTnftlObj );
		if( uiRes == SUCCESS )
		{
			_ND_TRACE( "OK!\n" );
		}
		else
		{
			_ND_TRACE( "Failed.(error code: 0x%x)\n", uiRes );
		}
	}

	if( uiBootStatus & BOOTLOADER_MIRROR_CHECK )
	{
		ND_TRACE( "'BootLoader' Restoration ... " );
		uiRes = NAND_RecoveryBootCode( pTnftlObj );
		if( uiRes == SUCCESS )
		{
			_ND_TRACE( "OK!\n" );
		}
		else
		{
			_ND_TRACE( "Failed.(error code: 0x%x)\n", uiRes );
		}
	}
}

void NAND_Suspend( void )
{
	NAND_IO_PowerDown();
}

void NAND_Resume( void )
{
	unsigned int i;

	NAND_IO_PowerUp();
	
	for( i = 0; i < s_NAND_TnftlObj[0].uiDieCount; i++ )
	{	
		if( s_NAND_TnftlObj[0].uiBL1_MediaType & 0x00000002 ) // 16Bit Mode Check.
			NAND_IO_Reset( i, NAND_IO_PARALLEL_COMBINATION_MODE );
		else
			NAND_IO_Reset( i, NAND_IO_SERIAL_COMBINATION_MODE );
	}
	
		
}

unsigned char NAND_GetBCDValue( unsigned char tmp )
{
	if ( tmp > 15 )
		tmp = tmp%16;

	if ( tmp < 10 )
		return '0'+tmp;
	else
		return 'A'+(tmp-10);
}

int NAND_ReadMediaUID( unsigned char *pucUID )
{
	unsigned int			i,j,k;
	unsigned char			tmp1,tmp2;
	unsigned char			cTemp[256];
	unsigned short int		CmdForUID[2];
	TNFTL_OBJECT_T 			*pTnftlObj = &s_NAND_TnftlObj[0];
	NAND_IO_DEVINFO			*sDevInfo = (NAND_IO_DEVINFO*)pTnftlObj->pvNandDevice;

	unsigned char			*MediaUniqueID = (unsigned char*)pucUID;

	NAND_EnterLock();	
	//=====================================================
	// Initialize
	//=====================================================

	for ( i = 0; i < 16; i++ )
	{
		MediaUniqueID[i]		= 0xFF;
	}
	
	//=====================================================
	// Read UID
	//=====================================================	

	if ( sDevInfo->Feature.DeviceID.Code[0] == SAMSUNG_NAND_MAKER_ID )
	{
		CmdForUID[0] = 0x3030;
		CmdForUID[1] = 0x6565;
	}
	else if ( sDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID )
	{
		CmdForUID[0] = 0x5A5A;
		CmdForUID[1] = 0xB5B5;
	}
	else if ( sDevInfo->Feature.DeviceID.Code[0] == HYNIX_NAND_MAKER_ID )
	{
		CmdForUID[0] = 0x0202;
		CmdForUID[1] = 0x1919;
	}
	else if ( sDevInfo->Feature.DeviceID.Code[0] == MICRON_NAND_MAKER_ID )
	{
		CmdForUID[0] = 0xEDED;	// Unique ID 
		CmdForUID[1] = 0x0000;	// Dummy Data 
	}
		
	if ( NAND_IO_GetUID( sDevInfo, CmdForUID, cTemp ) == SUCCESS )
	{
		/* Check UID */
		for ( j = 0; j < 256; j += 32 )
		{
			for ( k = 0; k < 16; k++ )
			{
				tmp1 = cTemp[j+k];
				tmp2 = cTemp[j+k+16];
				tmp1 = ~(tmp1);

				//Case: Hynix UniqueID not Exist & Hynix UniqueID is broken 
				if ( (sDevInfo->Feature.DeviceID.Code[0] == HYNIX_NAND_MAKER_ID ) && ( cTemp[j] != 0xAD ) && ( cTemp[j+1] !=0x09 ) )
					break;
				//Case: Hynix UniqueID Exist & extra NAND( SAMSUNG, TOSHIBA )
				if ( tmp1 != tmp2 )
					break;
			}
			
			if ( k >= 16 )
				break;
		}
		if ( j >= 256 )
		{
			NAND_LeaveLock();
			return -1;
		}

		if ( ( sDevInfo->Feature.DeviceID.Code[0] == SAMSUNG_NAND_MAKER_ID ) || ( sDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID ) )
		{
			/* Get UID */
			MediaUniqueID[0]	= cTemp[j+11];
			MediaUniqueID[1]	= cTemp[j+10];	/*hour1*/
			MediaUniqueID[2]	= cTemp[j+9];	/*min2*/
			MediaUniqueID[3]	= cTemp[j+8];	/*min1*/
			MediaUniqueID[4]	= cTemp[j+7];	/*sec1*/
			MediaUniqueID[5]	= cTemp[j+6];	/*sec2*/
			MediaUniqueID[6]	= NAND_GetBCDValue( (cTemp[j+2]>>4) & 0x0F );
			MediaUniqueID[7]	= NAND_GetBCDValue( cTemp[j+2] & 0x0F );
			MediaUniqueID[8]	= NAND_GetBCDValue( (cTemp[j+3]>>4) & 0x0F );
			MediaUniqueID[9]	= NAND_GetBCDValue( cTemp[j+3] & 0x0F );
			MediaUniqueID[10]	= NAND_GetBCDValue( (cTemp[j+4]>>4) & 0x0F );
			MediaUniqueID[11]	= NAND_GetBCDValue( cTemp[j+4] & 0x0F );
			MediaUniqueID[12]	= NAND_GetBCDValue( (cTemp[j+5]>>4) & 0x0F );
			MediaUniqueID[13]	= NAND_GetBCDValue( cTemp[j+5] & 0x0F );
			MediaUniqueID[14]	= NAND_GetBCDValue( (cTemp[j+12]>>4) & 0x0F );
			MediaUniqueID[15]	= NAND_GetBCDValue( cTemp[j+12] & 0x0F );				
			
		}
		else if (sDevInfo->Feature.DeviceID.Code[0] == HYNIX_NAND_MAKER_ID || sDevInfo->Feature.DeviceID.Code[0] == MICRON_NAND_MAKER_ID )
		{
			MediaUniqueID[0]	= cTemp[j+0];
			MediaUniqueID[1]	= cTemp[j+1];	
			MediaUniqueID[2]	= cTemp[j+2];	
			MediaUniqueID[3]	= cTemp[j+3];	
			MediaUniqueID[4]	= cTemp[j+4];	
			MediaUniqueID[5]	= cTemp[j+5];	
			MediaUniqueID[6]	= cTemp[j+6];
			MediaUniqueID[7]	= cTemp[j+7];
			MediaUniqueID[8]	= cTemp[j+8];
			MediaUniqueID[9]	= cTemp[j+9];
			MediaUniqueID[10]	= cTemp[j+10];
			MediaUniqueID[11]	= cTemp[j+11];
			MediaUniqueID[12]	= cTemp[j+12];
			MediaUniqueID[13]	= cTemp[j+13];
			MediaUniqueID[14]	= cTemp[j+14];
			MediaUniqueID[15]	= cTemp[j+15];	
		}
		
		/* Write Mark as Maker ID */
		if ( sDevInfo->Feature.DeviceID.Code[0] == SAMSUNG_NAND_MAKER_ID )
			MediaUniqueID[0] = 'A';
		else if ( sDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID )
			MediaUniqueID[0] = 'B';

		NAND_LeaveLock();
		return SUCCESS;			
	}		


	//=====================================================
	// Return
	//=====================================================	

	NAND_LeaveLock();
	return -1;
}

#if defined(_NU_)
void NAND_SetProgressCB(void* fnCallback)
{
	TNFTL_FW_SetProgressCB((Firmup_ProgressCallback)fnCallback);
}

DEV_MNT_DRIVER DeviceDriverInfo_NAND;
#ifdef USE_NANDHIDDENLOCK
int NAND_HDWriteSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{

	if (NAND_RW_WriteSector(uiSectorAddress,uiSectorCount,pvBuffer) == TNFTL_SUCCESS )
		return 0;

	return -1;
}

int NAND_HDReadSector( unsigned int uiDrvNum, unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	if (NAND_RW_ReadSector(uiSectorAddress,uiSectorCount,pvBuffer) == TNFTL_SUCCESS )
		return 0;

	return -1;
}

/******************************************************************************
*
*	int				NAND_HDIoctl
*
*	Input	:
*	Output	:
*	Return	:
*
*	Description :
*
*******************************************************************************/
int NAND_HDIoctl( int function, void *param )
{
	int res = 0;

	DISK_IOCTL_PARAM *Param = (DISK_IOCTL_PARAM *)param;

	switch( function )
	{
		case	DEV_INITIALIZE:
#if !defined(NAND_FOR_KERNEL)
			if( NAND_Init( 0, NAND_INIT_TYPE_FAST, NULL ) != NAND_SUCCESS )
#else
			if( NAND_Init( 0, NAND_INIT_TYPE_NORMAL, NULL ) != NAND_SUCCESS )
#endif
				res = EINITFAIL;
			break;

		case	DEV_GET_DISKINFO:
		{
			ioctl_diskinfo_t	*info = ( ioctl_diskinfo_t * ) param;

#if defined(NU_FILE_INCLUDE)
			gFormatType = TC_LOWLEVEL_NO;
#endif

			info->sector_size	= 512;
			NAND_EnterLock();
			info->Total_sectors = NAND_Area_GetTotalSectorCount( 0, NAND_RW_0_AREA_INDEX );
			NAND_LeaveLock();

			info->head			= 1;
			info->sector		= info->Total_sectors;
			info->cylinder		= 1;
			break;
		}

		case DEV_GET_DISK_SIZE:
		{
			unsigned int *puiTotalSector = ( unsigned int * )param;
			NAND_EnterLock();
			*puiTotalSector = NAND_Area_GetTotalSectorCount( 0, NAND_RW_0_AREA_INDEX );
			NAND_LeaveLock();
			break;
		}

		case	DEV_FORMAT_DISK:
		{
			//unsigned short	mode = *((unsigned short *)param);
			//
			//#if defined(NU_FILE_INCLUDE)
			//gFormatType	= TC_LOWLEVEL_NO;
			//#endif
			//
			//if ( gNAND_DrvInfo[0].NFTLDrvInfo->NANDType == NAND_TYPE_PURE_NAND )
			//{
			//	  TNFTL_AREAFormat( gNAND_DrvInfo[0].NFTLDrvInfo,
			//					  &gNAND_DrvInfo[0].NFTLDrvInfo->PriPartition,
			//					  mode );
			//}
			//#ifdef NAND_LBA_INCLUDE
			//else
			//{
			////	NAND_IO_LBA_AREAClear( gLBA_DevInfo, NAND_LBA_DATA_AREA );
			//}
			//#endif

			res = 0;
			break;
		}

		case	DEV_ERASE_INIT:
		{
			ioctl_diskeraseinit_t	*erase = ( ioctl_diskeraseinit_t * ) param;
			erase = erase;
			break;
		}

		case	DEV_ERASE_BLOCK:
		{
			ioctl_diskerase_t	*erase = ( ioctl_diskerase_t * ) param;
			erase = erase;
			break;
		}

		case	DEV_WRITEBACK_ON_IDLE:
			break;

		case	DEV_ERASE_CLOSE:
			break;

		case	DEV_HIDDEN_READ_PAGE_4:
			break;
		case	DEV_SET_POWER:
			break;
		case	DEV_GET_MAX_SECTOR_PER_BLOCK:
		{
			unsigned short *value = ( unsigned short * )param;
			*value = ( unsigned short )s_NAND_TnftlObj[0].uiSectorsPerPage;
			break;
		}

		case	DEV_TELL_DATASTARTSECTOR:
			break;

		case	DEV_CHECK_CRC_NANDBOOT_IMAGE_ROM:
			break;

		case	DEV_RECOVERY_TCBOOT:
			break;
		case	DEV_GET_WRITE_PROTECT:
			break;
		case	DEV_GET_INSERTED:
			if( param != NULL )
			{
				Param->ioctl_param.GET_INSERTED_param.DevNo = DM_FIXED_DEV_BASE_NO;
				Param->ioctl_param.GET_INSERTED_param.DevType = (int)DISK_DEVICE_NAND_HD;
			}
			res = 1;
			break;
		case DEV_GET_HIDDEN_COUNT:
		{
			unsigned int *puiCount = ( unsigned int * )param;
			*puiCount = 0;//(unsigned int)gNAND_DrvInfo[0].NFTLDrvInfo->ExtendedPartitionNo;
			break;
		}
#ifdef NU_FILE_INCLUDE
		case	DEV_GET_HIDDEN_SIZE:
			*( int * )param = -1;
			break;
#endif
		case	DEV_GET_INITED:
			res = 1;
			break;
		case DEV_GET_PLAYABLE_STATUS:
			res = 1;
			break;
		default:
			res = ENOTSUPPORT;
			break;
	}

	return res;
}

static unsigned char EnabledWritingOnNANDHidden=0;
#endif

int NAND_RW_WriteSector( unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	int retval;
#ifdef USE_NANDHIDDENLOCK
	if (EnabledWritingOnNANDHidden==0) {
		retval=-1;
	} else
#endif
	{
		retval=NAND_Area_WriteSector( 0, 4, uiSectorAddress, uiSectorCount, pvBuffer );
	}
	return retval;
}

int NAND_RW_ReadSector( unsigned int uiSectorAddress, unsigned int uiSectorCount, void *pvBuffer )
{
	return NAND_Area_ReadSector( 0, 4, uiSectorAddress, uiSectorCount, pvBuffer );
}

int NAND_RW_ClearPages(  unsigned int nHDStPageAddr, unsigned int nHDEdPageAddr )
{
	nHDStPageAddr = 0;
	nHDEdPageAddr = 0;
	return 0;
}	
#ifdef USE_NANDHIDDENLOCK
void NAND_HDWriteEnable(void)
{
	EnabledWritingOnNANDHidden = 1;
}

void NAND_HDWriteDisable(void)
{
	EnabledWritingOnNANDHidden = 0;
}
#endif
void NAND_initMntDriver(void)
{
	/* ==========  Block 0 init  ========== */

	DEV_INIT_LIST_HEAD(&(DeviceDriverInfo_NAND.mnt_driver_list));

	DeviceDriverInfo_NAND.PhysicalPortNum = 0;
	DeviceDriverInfo_NAND.FirstCheckStatus = DEV_NOTCHECK;

	/*Porting Layer*/
	DeviceDriverInfo_NAND.DiskDevType = DISK_DEVICE_NAND;
	
#ifdef USE_MEDIA_MOUNT
	DeviceManager_SetMntTasktoMntDriver(&DeviceDriverInfo_NAND, DEV_MNT_TYPE_NORMAL_DEV);

	DeviceManager_AddMntDriver(&DeviceDriverInfo_NAND);
#endif
	DeviceDriverInfo_NAND.enable_status = DEV_ENABLE;
	
}

DEV_MNT_DRIVER *NAND_getMntDriver(void)
{
	DEV_MNT_DRIVER *Mnt_driver = NULL;

	Mnt_driver = &DeviceDriverInfo_NAND;

	return Mnt_driver;
}
#endif

#if defined(_LINUX_)
unsigned int NAND_DumpExtArea( unsigned char *buf )
{
#if defined(ANDROID_BOOT)
	unsigned int flash_nand_dump( unsigned char * buf );
	return flash_nand_dump( buf );
#else
	return 0;
#endif
}
#endif

#if !defined(_NU_) && defined(NAND_FOR_KERNEL)
EXPORT_SYMBOL(NAND_Init);
EXPORT_SYMBOL(NAND_Ioctl);
EXPORT_SYMBOL(NAND_Suspend);
EXPORT_SYMBOL(NAND_Resume);
EXPORT_SYMBOL(NAND_WriteSector);
EXPORT_SYMBOL(NAND_ReadSector);
EXPORT_SYMBOL(NAND_GetSerialNumber);
EXPORT_SYMBOL(NAND_WriteFirmware); //for tccbox

EXPORT_SYMBOL(NAND_Area_GetTotalSectorCount);
EXPORT_SYMBOL(NAND_MTD_Area_ReadPage);
EXPORT_SYMBOL(NAND_MTD_Area_WritePage);
EXPORT_SYMBOL(NAND_MTD_Update);
EXPORT_SYMBOL(NAND_MTD_Area_Erase);
EXPORT_SYMBOL(NAND_Area_WriteSector);
EXPORT_SYMBOL(NAND_Area_ReadSector);
EXPORT_SYMBOL(NAND_Flush_Cacheblock);
EXPORT_SYMBOL(NAND_Padding_Cacheblock);
#endif

#if defined(ANDROID_KERNEL)
MODULE_AUTHOR("Telechips Inc. linux@telechips.com");
MODULE_DESCRIPTION("Telechips NAND core");
MODULE_LICENSE("Proprietary. (C) 2008 Telechips Inc.");
#endif
