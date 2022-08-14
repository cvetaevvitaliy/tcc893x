/****************************************************************************
 *   FileName    : tnftl_v8_api.h
 *   Description : TNFTL Driver V8 API
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
#ifndef _TNFTL_V8_API_H
#define _TNFTL_V8_API_H

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DO NOT MODIFY THIS FILE !!!
//
// THIS FILE IS API FOR TNFTL V8 LIBRARY.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// NAND AREA
////////////////////////////////////////////////////////////////
#define NAND_RO_AREA_COUNT_MAX					4
#define NAND_RW_AREA_COUNT_MAX					6
#define NAND_AREA_COUNT_MAX						(NAND_RO_AREA_COUNT_MAX+NAND_RW_AREA_COUNT_MAX)

#define NAND_RO_0_AREA_INDEX					0
#define NAND_RO_1_AREA_INDEX					1
#define NAND_RO_2_AREA_INDEX					2
#define NAND_RO_3_AREA_INDEX					3
#define NAND_RW_0_AREA_INDEX					4
#define NAND_RW_1_AREA_INDEX					5
#define NAND_RW_2_AREA_INDEX					6
#define NAND_RW_3_AREA_INDEX					7
#define NAND_RW_4_AREA_INDEX					8
#define NAND_DATA_AREA_INDEX					9
////////////////////////////////////////////////////////////////

#define DIE_COUNT_MAX							2

#define CMT_COUNT_MAX							6

#define TNFTL_PAGE_BUFFER_COUNT					4

#define TNFTL_TAG_DWORD_SIZE					3
#define TNFTL_TAG_BYTE_SIZE						(TNFTL_TAG_DWORD_SIZE*4)

#define MTD_TAG_DWORD_SIZE						5
#define MTD_TAG_BYTE_SIZE						(MTD_TAG_DWORD_SIZE*4)

#define TNFTL_BADMARK_DWORD_SIZE				1
#define TNFTL_BADMARK_BYTE_SIZE					(TNFTL_BADMARK_DWORD_SIZE*4)

#define NAND_CS_0								(1<<0)
#define NAND_CS_1								(1<<1)
#define NAND_CS_2								(1<<2)
#define NAND_CS_3								(1<<3)

#define MAX_DEBUG_BLOCK_COUNT						3	

#define	ASSERT_LEVEL_1							1
#define	ASSERT_LEVEL_2							2
#define	ASSERT_LEVEL_3							3
#define	ASSERT_LEVEL_4							4
#define	ASSERT_LEVEL_5							5
#define	ASSERT_LEVEL_6							6
#define	ASSERT_LEVEL_7							7
#define	ASSERT_LEVEL_8							8

typedef enum
{
	TNFTL_BLOCK_NORMAL			= 0,
	TNFTL_V5_BLOCK_NORMAL,
	TNFTL_BLOCK_BAD
} TNFTL_BAD_BLOCK_STATUS_T;

typedef enum
{
    TNFTL_SUCCESS								= 0,
    TNFTL_ERROR									= 0xFF000000,
    TNFTL_ERROR_WRONG_PARAMETER					= 0xFF000001,
    TNFTL_ERROR_NAND_IO_ERROR					= 0xFF000002,
    TNFTL_ERROR_MEMORY_ALLOCATION				= 0xFF000003,
    TNFTL_ERROR_UMT_FAIL_TO_ADD					= 0xFF000004,
    TNFTL_ERROR_BLOCK_ALLOC						= 0xFF000005,
    TNFTL_ERROR_WRITE							= 0xFF000006,
    TNFTL_ERROR_READ							= 0xFF000007,
    TNFTL_ERROR_COMPARE							= 0xFF000008,
    TNFTL_ERROR_GOLDEN_INFO						= 0xFF000009,
    TNFTL_ERROR_GOLDEN_INFO_BOOT_CHANGE			= 0xFF00000A,
    TNFTL_ERROR_GOLDEN_INFO_RO_CHANGE			= 0xFF00000B,
    TNFTL_ERROR_GOLDEN_INFO_RW_CHANGE			= 0xFF00000C,
    TNFTL_ERROR_WRONG_TAG						= 0xFF00000D,
    TNFTL_ERROR_BOOTCODE_CRC_ERROR				= 0xFF00000E,
    TNFTL_ERROR_BAD_BLOCK						= 0xFF00000F,
    TNFTL_ERROR_BLOCK_FULL						= 0xFF000010,
    TNFTL_ERROR_DEBUG_FULL						= 0xFF000020
} TNFTL_ERROR_T;

typedef enum
{
    TNFTL_FORMAT_NORMAL,
	TNFTL_FORMAT_DEBUG_CHECKANDERASE,
    TNFTL_FORMAT_DEBUG_ERASE
} TNFTL_FORMAT_TYPE_T;

typedef enum
{
    DIE_LAST_STATUS_NONE,
    DIE_LAST_STATUS_READ,
    DIE_LAST_STATUS_WRITE,
    DIE_LAST_STATUS_ERASE,
    DIE_LAST_STATUS_WRITE_PLANE0,
    DIE_LAST_STATUS_WRITE_PLANE1
} DIE_LAST_STATUS_T;

typedef struct
{
	unsigned int *puiDataBuffer;
	unsigned int *puiTagBuffer;
} TNFTL_PAGE_BUFFER_T;

typedef struct
{
// general
	unsigned int uiRelevantCS;
	TNFTL_ERROR_T errLastError;
	TNFTL_PAGE_BUFFER_T sPageBuffer[TNFTL_PAGE_BUFFER_COUNT];
	unsigned int uiCurrentPageBufferIndex;
	unsigned int *puiROTempPageBuffer;
	unsigned int uiRWTempPageBufferPageAddress;
	unsigned int *puiRWTempPageBuffer;
	unsigned int uiSectorsPerPage;
	unsigned int uiSectorsPerPageShift;
	unsigned int uiTotalDataBlockCount;
	unsigned int uiCurrentDieIndex;

	void *pvDie[DIE_COUNT_MAX];
	DIE_LAST_STATUS_T enumLastStatus[DIE_COUNT_MAX];
	unsigned int uiBFCntMax;
	unsigned int uiPMDCntMax;
	unsigned int uiECDCntMax;
	unsigned int uiCMTCntMax;
	unsigned int uiMBCntMax;
	unsigned int uiECBCntMax;
	unsigned int uiEntryCountMax;
	unsigned int uiPageStatus_SizeInByte;

	unsigned int uiGoldenInfo[256 / 4];
	unsigned int *uiClusterInfo;
	unsigned int uiROAreaBaseBlockAddress[NAND_RO_AREA_COUNT_MAX];
	unsigned short usROAreaBlockCount[NAND_RO_AREA_COUNT_MAX];

	unsigned int uiBlockCountExceptRWArea;

	unsigned int uiRWAreaBaseBlockAddress;
	unsigned int uiRWAreaLogicalSectorAddress[NAND_RW_AREA_COUNT_MAX];
	unsigned int uiRWAreaReservedBlockCount;

// NAND Device Information
	void *pvNandDevice;
	unsigned int uiCodeWordSize;
	unsigned int uiBytesPerPage;
	unsigned int uiBytesPerPageShift;
	unsigned int uiPagesPerBlock;
	unsigned int uiPagesPerBlockShift;
	unsigned int uiBlocksPerDie;
	unsigned int uiDieCount;
	unsigned int uiDieCountShift;
	const unsigned char *pucMLC_LowerPageTable;
	unsigned int uiSafePagesPerBlock;
	unsigned int uiBadBlockCountMax;
	unsigned char ucPlaneCount;
	unsigned char ucPlaneCountShift;
	unsigned short usBL1_ECCType;
	unsigned short usECCCodeSize;
	unsigned int uiBL1_MediaType;
	unsigned char ucColumnCycle;
	unsigned char ucRowCycle;
	unsigned int uiReadCycleRegValue;
	unsigned int uiCommCycleRegValue;

// flags
	unsigned int fGoldenInfoReseted			:1;
	unsigned int fRWBlockCollected			:1;
	unsigned int fUseHotBlock				:1;
	unsigned int fReadRetry					:1;
	unsigned int fRWTempPageBufferLock		:1;
	unsigned int fUseLowerPageOnlyForBL1	:1;

// FTL Information
	unsigned int uiLast_convert_version;

// Debug in block
	unsigned int uiLastAllocatedDebugBlock;
	unsigned int uiCurrentDebuginfoBlockAddress;
	unsigned int uiPage_append_index;
	unsigned int uiDebuginfoBlockCount;
} TNFTL_OBJECT_T;

typedef enum
{
    BOOT_ROM_WRITE_TYPE_START,
    BOOT_ROM_WRITE_TYPE_MIRROR_IMAGE_START,
    BOOT_ROM_WRITE_TYPE_CONTINUE
} BOOT_ROM_WRITE_TYPE_T;

typedef struct
{
	unsigned int uiSectorCount[NAND_RO_AREA_COUNT_MAX];
} RO_CONFIG_T;

typedef struct
{
	unsigned int uiSectorCount[NAND_RW_AREA_COUNT_MAX];
} RW_CONFIG_T;

typedef struct
{
	unsigned int uiStructureVersion;
	unsigned int uiBootSizeInMB;
	RO_CONFIG_T sRO_Config;
	RW_CONFIG_T sRW_Config;
} TNFTL_AREA_INFO_T;

typedef struct
{
	void *pvDie;
	unsigned int uiDieBufSize;
	void *pvBlockFeature;
	unsigned int uiBlockFeatureBufSize;
	unsigned int *puiPMD;
	unsigned int *puiMTB;
	void *pvCMT;
	unsigned int uiCMTBufSize;
	void *pvEntry;
	unsigned int uiEntryBufSize;
	unsigned int *puiECD;
	unsigned char *pucPageStatus;
} TNFTL_BUFFER_PER_DIE_T;

typedef struct
{
	TNFTL_BUFFER_PER_DIE_T *pBufferTablePerDie;

	unsigned int uiBlockFeatureCnt;
	unsigned int uiPMDCnt;
	unsigned int uiCMTCnt;
	unsigned int uiEntryCount;
	unsigned int uiECDCnt;
	unsigned int uiPageStatusSizeInByte;
} TNFTL_BUFFER_TABLE_T;

typedef enum {
	MOVE_BROKEN_BY_SUDDEN_POWER_OFF,
	MOVE_KNOWN_BAD_BLOCK,
	MOVE_UNKNOWN_BAD_BLOCK
} MOVE_TYPE_T;

typedef void (*TNFTL_ProgressHandler)( unsigned int uiCurrent, unsigned int uiTotal );
typedef unsigned int (*Firmup_ProgressCallback)(unsigned int);

//======================================================================================================
//
// tnftl_v8 shortcut
//
//======================================================================================================
#define PageAddressOfBlock(pTnftlObj,uiBlockAddress)			( (uiBlockAddress) << (pTnftlObj)->uiPagesPerBlockShift )
#define BlockAddressOfPage(pTnftlObj,uiPageAddress)				( (uiPageAddress) >> (pTnftlObj)->uiPagesPerBlockShift )
#define BlockOffset(pTnftlObj,uiPageAddress)					( (uiPageAddress) & ((pTnftlObj)->uiPagesPerBlock-1) )


//======================================================================================================
//
// tnftl_v8_glue.c
//
//======================================================================================================
#if 1
#if !defined(NAND_FOR_KERNEL) && !defined(_NU_)
extern void *memcpy(void *, const void *, unsigned int);
extern void *memset(void *, int, unsigned int);
extern int memcmp(const void *, const void *, unsigned int);
#endif
#define TNFTL_memcpy		memcpy
#define TNFTL_memset		memset
#define TNFTL_memcmp		memcmp
#else
void TNFTL_memcpy( void *pvDst, const void *pvSrc, unsigned int uiLength );
void TNFTL_memset( void *pvDst, unsigned char ucValue, unsigned int uiLength );
int TNFTL_memcmp( const void *pvSrc0, const void *pvSrc1, unsigned int uiLength );
#endif
void TNFTL_printf( char *fmt, ... );
void *TNFTL_malloc( unsigned int uiLength );
void TNFTL_free( void *ptr );
unsigned int TNFTL_CalcCRC( void *pvBase, unsigned int uiLength, unsigned int uiCRCIN );
unsigned int TNFTL_CalcCRCI( void *pvBase, unsigned int uiLength, unsigned int uiCRCIN );
unsigned int TNFTL_IO_GetFeatureOfNAND( TNFTL_OBJECT_T *pTnftlObj );
unsigned int TNFTL_IO_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPage );
unsigned int TNFTL_IO_MultiPlane_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int TNFTL_IO_MultiPlane_WriteLastPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int TNFTL_IO_WriteChipBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );
unsigned int TNFTL_IO_ReadPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPageForNextTurn );
unsigned int TNFTL_IO_ReadChipBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int TNFTL_IO_ReadSpare( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int TNFTL_IO_IsReadReclaimRequired( TNFTL_OBJECT_T *pTnftlObj );
unsigned int TNFTL_IO_EraseBlock( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress );
void TNFTL_IO_MarkBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress );
unsigned int TNFTL_IO_MakeBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer );
unsigned int TNFTL_IO_MakeBL2Page( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer, unsigned int uiPageSize, const unsigned int *puiTag );
unsigned int TNFTL_IO_CheckBL1ReadReclaim( TNFTL_OBJECT_T *pTnftlObj );
unsigned char TNFTL_IO_GetEccErrorBitCount( void );
TNFTL_BAD_BLOCK_STATUS_T TNFTL_IO_IsBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress );

//======================================================================================================
//
// tnftl_v8 library
//
//======================================================================================================
TNFTL_ERROR_T TNFTL_Init_Buffer( TNFTL_OBJECT_T *pTnftlObj, TNFTL_BUFFER_TABLE_T *pTnftlBuffer, unsigned int uiDieCountMax );
TNFTL_ERROR_T TNFTL_Init_Query_NANDFeature( TNFTL_OBJECT_T *pTnftlObj );
TNFTL_ERROR_T TNFTL_Init_Blocks( TNFTL_OBJECT_T *pTnftlObj, unsigned int ucIsScanROOnly );
void TNFTL_Print_CB(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiDieIndex);
void TNFTL_Print_EC(TNFTL_OBJECT_T *pTnftlObj);
TNFTL_ERROR_T TNFTL_Init_Collect_RW_Blocks( TNFTL_OBJECT_T *pTnftlObj );
void TNFTL_Format( TNFTL_OBJECT_T *pTnftlObj, TNFTL_FORMAT_TYPE_T eType, const TNFTL_ProgressHandler fnProgressHandler );
unsigned int TNFTL_Area_GetTotalSectorCount( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiAreaIndex );
unsigned int TNFTL_Get_SectorsPerPage( TNFTL_OBJECT_T *pTnftlObj );
TNFTL_ERROR_T TNFTL_RO_Area_ReadPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiROIndex, unsigned int uiPageAddr, void *pvBuffer, unsigned int uiTotalPageCount );
TNFTL_ERROR_T TNFTL_RO_Area_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiROIndex, unsigned int uiPageAddr, void *pvBuffer, unsigned int uiTotalPageCount );
TNFTL_ERROR_T TNFTL_WriteSector( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiRWIndex, unsigned int uiSectorAddress, void *pvBuffer, unsigned int uiSectorCount );
TNFTL_ERROR_T TNFTL_RO_Area_ReadSector( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiROIndex, unsigned int uiSectorAddress, void *pvBuffer, unsigned int uiSectorCount );
TNFTL_ERROR_T TNFTL_ReadSector( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiRWIndex, unsigned int uiSectorAddress, void *pvBuffer, unsigned int uiSectorCount );
TNFTL_ERROR_T TNFTL_FW_WriteBootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucBuffer, unsigned int uiByteSize, BOOT_ROM_WRITE_TYPE_T eBootRomWriteType );
TNFTL_ERROR_T TNFTL_FW_Read_GoldenBlock( TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucTable, unsigned int uiTableSize );
TNFTL_ERROR_T TNFTL_FW_Verify_GoldenInfo( TNFTL_OBJECT_T *pTnftlObj, TNFTL_AREA_INFO_T *pTnftlAreaInfo );
TNFTL_ERROR_T TNFTL_FW_VerifyBL1ErrorBit( TNFTL_OBJECT_T *pTnftlObj, unsigned int *uiFirstBL1ErrorBit, unsigned int *uiSecondBL1ErrorBit, unsigned char *pucFirstBlockAddress, unsigned char *pucSecondBlockAddress );
TNFTL_ERROR_T TNFTL_FW_RecoveryBL1( TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucSrcBlockAddress, unsigned char *pucDestBlockAddress, unsigned char IsSecondRecovery );
TNFTL_ERROR_T TNFTL_FW_Reset_GoldenInfo(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiMemInitCodeSizeInByte, TNFTL_AREA_INFO_T *pTnftlAreaInfo);
TNFTL_ERROR_T TNFTL_FW_Write_BL1(TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucBL1, unsigned int uiBL1TargetAddress, unsigned int uiBL1Size, unsigned char *pucMemInitCode, unsigned int uiMemInitCodeSize);
void TNFTL_FW_SetBootCodeParameter( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiTargetAddress, unsigned int uiSize, unsigned int uiCompsize );
TNFTL_ERROR_T TNFTL_FW_Write_BootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucBuffer, unsigned int uiByteSize, BOOT_ROM_WRITE_TYPE_T eBootRomWriteType );
TNFTL_ERROR_T TNFTL_FW_Verify_BootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiByteSize, unsigned int uiRomFileCRC );
TNFTL_ERROR_T TNFTL_FW_Write_GoldenBlock( TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucTable, unsigned int uiTableSize, unsigned long uiBootType);
TNFTL_ERROR_T TNFTL_FW_RecoveryBootCode( TNFTL_OBJECT_T *pTnftlObj );
TNFTL_ERROR_T TNFTL_FW_RecoveryMemInitCode( TNFTL_OBJECT_T *pTnftlObj );

unsigned int TNFTL_FW_Read_GoldenInfo(TNFTL_OBJECT_T *pTnftlObj);
unsigned int TNFTL_FW_Recovery_BackupBlock(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBackupBlockAddress);
TNFTL_ERROR_T TNFTL_FW_Backup_GoldenBlock(TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucTable, unsigned int uiTableSize, unsigned long uiBootType, unsigned int *puiBlockAddress);
void TNFTL_FW_Erase_BackupBlock(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress);

TNFTL_ERROR_T TNFTL_MTD_Area_WritePage(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiROIndex, unsigned int uiPageAddr, void *pvBuffer, void *pvTag, unsigned char bUseYaffs );
TNFTL_ERROR_T TNFTL_MTD_Area_ReadPage(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiROIndex, unsigned int uiPageAddr, void *pvBuffer, void *pvTag, unsigned char bUseYaffs );
TNFTL_ERROR_T TNFTL_RO_Area_Erase(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiAreaIndex, unsigned int uiPageAddr, unsigned int isWholeArea );

unsigned int TNFTL_IO_WritePage_For_BootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );
unsigned int TNFTL_IO_ReadPage_For_BootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );
unsigned int TNFTL_IO_ReadPage_For_GMC( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );
unsigned int TNFTL_IO_WritePage_For_GMC( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );
TNFTL_ERROR_T TNFTL_FW_Write_BootCode_For_TCC800X(TNFTL_OBJECT_T *pTnftlObj, unsigned char *pucBuffer, unsigned int uiByteSize, BOOT_ROM_WRITE_TYPE_T eBootRomWriteType);
TNFTL_ERROR_T TNFTL_FW_PreProcess( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiRomFileSize );	
TNFTL_ERROR_T TNFTL_FW_Write_MC(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiMCNum );
TNFTL_ERROR_T TNFTL_FW_Write_GMC(TNFTL_OBJECT_T *pTnftlObj);
void TNFTL_IO_AdjustPageSpareSize( unsigned short *pusPPages, unsigned short *pusPageSize, unsigned short *pusSpareSize, unsigned int *puiECCType );

unsigned int TNFTL_Convert_CacheBlocks(TNFTL_OBJECT_T *pTnftlObj);
unsigned int TNFTL_Padding_CacheBlocks(TNFTL_OBJECT_T *pTnftlObj);

// S/W randomizer
void TNFTL_RAND_SetAddress(unsigned int uiPagePerBlock, unsigned int uiPageAddr, unsigned int uiColumnAddr);
void TNFTL_RAND_Randomize( void *pvDst, const void *pvSrc, unsigned int uiLength );
//USE_NAND_COMPRESS
unsigned int TNFTL_Compress(char* in_buff, char* out_buff, unsigned int inSize);
void TNFTL_PMT_Check(TNFTL_OBJECT_T *pTnftlObj);
// Debug Block
unsigned int TNFTL_AllocateBlock_Ref(TNFTL_OBJECT_T *pTnftlObj,unsigned int uiType, unsigned int uiLastAllocatedBlockAddress);
void TNFTL_MoveDebugBlock_Ref( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiSourcePBA, MOVE_TYPE_T eMoveType );
unsigned int TNFTL_GetDriverVersion_ref(void);
unsigned int TNFTL_Allocate_Debug_Block(TNFTL_OBJECT_T *pTnftlObj);
unsigned int TNFTL_Print_Debug_info(TNFTL_OBJECT_T *pTnftlObj);
void Write_Assert_Info(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiAssertLevel, void *pFunc, unsigned int uiline, unsigned int uiArg_Count, ...);
#endif

