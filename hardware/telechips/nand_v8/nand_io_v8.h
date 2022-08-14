/****************************************************************************
 *   FileName    : nand_io_v8.h
 *   Description : NAND I/O driver. NFC(NAND Flash Controller) driver is built in.
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
#ifndef _NAND_IO_V8_H
#define _NAND_IO_V8_H

//#define NAND_GPIO_DEBUG

//**********************************************************************
//*		Define IRQ
//**********************************************************************
#define NAND_IRQ_NFC			INT_NFC//41//IRQ_NFC
#define NAND_IRQ_READY			INT_NFC//41//IRQ_NFC

//*****************************************************************************
//*
//*
//*                       [ General DEFINE & TYPEDEF ]
//*
//*
//*****************************************************************************
#ifndef DWORD_BYTE
typedef	union
{
	unsigned long	DWORD;
	unsigned short	WORD[2];
	unsigned char	BYTE[4];
} DWORD_BYTE;
#endif

#define YAFFS_TAG_SIZE							20

#ifndef NAND_IO_ASSERT
//#define NAND_IO_ASSERT(a)			if(!(a)) while(1);
#define NAND_IO_ASSERT(a)			if(!(a))\
									{\
										ND_TRACE("[ASSERT] %s:%d: %s\n",__FUNCTION__,__LINE__,#a);\
									}
#endif
//*****************************************************************************
//*
//*
//*                         [ ERROR CODE ENUMERATION ]
//*
//*
//*****************************************************************************
#ifndef SUCCESS
#define SUCCESS		0
#endif

typedef enum
{
    NAND_IO_SUCCESS								= 0,
	NAND_IO_ERASED_PAGE							= 1,
	NAND_IO_CHECK_BOOTAREA_PAGE					= 2,
    NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY		= 0xFC000000,
    NAND_IO_ERROR_WRONG_PARAMETER				= 0xFC000001,
    NAND_IO_ERROR_ECC_CORRECTION_FAILURE		= 0xFC000002,
    NAND_IO_ERROR_READ_STATUS_BUSY				= 0xFC000003,
    NAND_IO_ERROR_READ_STATUS_FAILURE			= 0xFC000004,
    NAND_IO_ERROR_READ_STATUS_TIMEOUT			= 0xFC000005,
    NAND_IO_ERROR_READ_FAILURE					= 0xFC000006,
    NAND_IO_ERROR_WRITE_FAILURE					= 0xFC000007,
    NAND_IO_ERROR_GET_DEVICE_INFO				= 0xFC000008,
    NAND_IO_ERROR_INVALID_DEPLOYMENT			= 0xFC000009,
    NAND_IO_ERROR_WRITE_PROTECTED				= 0xFC00000A,
    NAND_IO_ERROR_READ_STATUS_FOR_INTERLEAVING	= 0xFC00000B,
    NAND_IO_ERROR_FAIL_TO_INTERLEAVE			= 0xFC00000C
} NAND_IO_ERROR_T;

//*****************************************************************************
//*
//*
//*                          [ INTERNAL DEFINATION ]
//*
//*
//*****************************************************************************
typedef enum
{
    NAND_IO_READ = 0,
    NAND_IO_WRITE,
    NAND_IO_ERASE,
    NAND_IO_UNDEFINED
} NAND_IO_OPERATION_T;

/* Maker Information of supported NANDFLASH */
enum SUPPORT_MAKER_NAND
{
    SAMSUNG = 0,
    TOSHIBA,
    HYNIX,
    MICRON,
    SPANSION,
    FIDELIX,
    MACRONIX,
    EON,
    EMST,
    MAX_SUPPORT_MAKER_NAND
};

#define SAMSUNG_NAND_MAKER_ID					0xEC
#define TOSHIBA_NAND_MAKER_ID					0x98
#define HYNIX_NAND_MAKER_ID						0xAD
#define ST_NAND_MAKER_ID						0x20
#define MICRON_NAND_MAKER_ID					0x2C
#define INTEL_NAND_MAKER_ID						0x89
#define SPANSION_NAND_MAKER_ID					0x01
#define FIDELIX_NAND_MAKER_ID					0xF8
#define MACRONIX_NAND_MAKER_ID					0xC2
#define EON_NAND_MAKER_ID						0x92
#define EMST_NAND_MAKER_ID						0xC8

#define GET_ECC_TYPE_FROM_MEDIATYPE(MediaType)	( ((MediaType)&0x00000F00) >> 8 )
#define ECC_TYPE_EMBEDDED						0x0
#define ECC_TYPE_4BIT							0x1
#define ECC_TYPE_8BIT							0x2
#define ECC_TYPE_12BIT							0x3
#define ECC_TYPE_16BIT							0x4
#define ECC_TYPE_24BIT							0x5
#define ECC_TYPE_40BIT							0x6
#define ECC_TYPE_60BIT							0x7

/* Attribute (Mandatory) */
#define A_BUSWIDTH_OF(x)						((x)&0x00000003)
#define A_08BIT									0x00000000		// 8Bit BUS
#define A_16BIT									0x00000001		// 16Bit BUS
#define A_32BIT									0x00000002		// 32Bit BUS

#define A_ECC_EMBEDDED							(ECC_TYPE_EMBEDDED<<8)		// ECC Embedded (Smart NAND, Clear NAND)
#define A_ECC_4BIT								(ECC_TYPE_4BIT<<8)			// 4Bit MLC ECC
#define A_ECC_8BIT								(ECC_TYPE_8BIT<<8)			// 8Bit MLC ECC
#define A_ECC_12BIT								(ECC_TYPE_12BIT<<8)			// 12Bit MLC ECC
#define A_ECC_16BIT								(ECC_TYPE_16BIT<<8)			// 16Bit MLC ECC
#define A_ECC_24BIT								(ECC_TYPE_24BIT<<8)			// 24Bit MLC ECC
#define A_ECC_40BIT								(ECC_TYPE_40BIT<<8)			// 40Bit MLC ECC
#define A_ECC_60BIT								(ECC_TYPE_60BIT<<8)			// 60Bit MLC ECC

#define A_CODEWORD_OF(x)						((x)&0x00003000)
#define A_512B									0x00000000
#define A_1KB									0x00001000

#define A_PARALLEL								0x00004000		// Parallel Composition
#define A_RAND_IO								0x00008000		// Random Data In/Out
#define A_READ_RETRY_TYPE_OF(x)					((x)&0x00FF0000)
#define A_RR_M									0x00010000		// micron read retry
#define A_RR_H_26NM								0x00020000		// hynix read retry for 26nm or less
#define A_RR_H_20NM								0x00080000		// hynix read retry for 20nm or less
#define A_RR_S									0x00040000		// samsung read retry

#define A_CHANGE_SPARE_ECC_OF(x)				((x)&0x0F000000)
#define A_ECC_SPARE_16BIT						0x01000000		// 16bit MLC ECC for only Spare Area of 24Bit ECC Nand
#define A_ECC_SPARE_24BIT						0x02000000		// 24bit MLC ECC for only Spare Area of 40Bit ECC Nand

/* Functions (Optional) */
#define F_MP									0x00000001		// MULTI PLANE
#define F_CP									0x00000002		// CACHE PROGRAM
#define F_CR									0x00000004		// CACHE READ
#define F_EDO									0x00000008      // Extened data Output

#define F_IL									0x00000100		// INTER LEAVE
#define F_EIL									0x00000200		// EXTERNAL INTER LEAVE


#define NAND_IO_SERIAL_COMBINATION_MODE			0x00
#define NAND_IO_PARALLEL_COMBINATION_MODE		0x01

#define	ECC_OFF                     			0
#define	ECC_ON                      			1

#define DATA_AREA_ECC_OFF						0
#define DATA_AREA_ECC_ON						1

#define TNFTL_READ_SPARE_ON						0
#define TNFTL_READ_SPARE_OFF					1

#define NAND_MCU_ACCESS							0
#define NAND_DMA_ACCESS							1

#define ECC_DECODE								0
#define ECC_ENCODE								1

#define	INTER_LEAVE_OFF                			0
#define	INTER_LEAVE_ON                			1

#define MULTI_PLANE_NORMAL_PAGE					0
#define MULTI_PLANE_START_PAGE					1
#define MULTI_PLANE_MID_PAGE					2
#define MULTI_PLANE_LAST_PAGE					3
#define MULTI_PLANE_STUFF_PAGE					4

#define MULTI_PLANE_GOOD_BLOCK					0
#define MULTI_PLANE_BAD_BLOCK					1

#define NAND_IO_STATUS_FAIL_CS0_SERIAL			0x0001
#define NAND_IO_STATUS_FAIL_CS0_PARALLEL		0x0002
#define NAND_IO_STATUS_FAIL_CS1_PARALLEL		0x0004
#define NAND_IO_STATUS_FAIL_PREVIOUS_PROGRAM	0x0008
#define NAND_IO_STATUS_FAIL_BUSY				0x0010
#define NAND_IO_STATUS_DISTRICT_0				0x0020
#define NAND_IO_STATUS_DISTRICT_1				0x0040

#define NAND_IO_WRITE_NORMAL_MODE				0
#define NAND_IO_WRITE_CACHE_MODE				1
#define NAND_IO_WRITE_COPYBACK_MODE				2
#define NAND_IO_READ_NORAML_MODE				3
#define NAND_IO_ERASE_MODE						4
#define NAND_IO_STATUS_MODE						5

#define NAND_IO_NFC_BUS							0
#define NAND_IO_MEM_BUS							1

#define NAND_IO_DATA_WITDH_8BIT					0
#define NAND_IO_DATA_WITDH_16BIT				1

#define NAND_IO_IRQ_STATE_NONE					0
#define NAND_IO_IRQ_STATE_WRITE					1
#define NAND_IO_IRQ_STATE_READ					2
#define NAND_IO_IRQ_STATE_WRITE_RB				3
#define NAND_IO_IRQ_STATE_READ_RB				4
#define NAND_IO_IRQ_STATE_RB					5

/////////////////////////////////////////////
// example
//#define NAND_IO_USE_MCU_ACCESS
//#define NAND_IO_USE_DMA_ACCESS
//#define NAND_IO_USE_NDMA_ACCESS
/////////////////////////////////////////////
#if !defined(NFC_VER_50)
#define NAND_SUPPORT_NDMA
#endif

#if defined(_WINCE_)
	#if defined(TCC801X) || defined(TCC892X) || defined(TCC370X)
		#define NAND_IO_USE_NDMA_ACCESS
	#elif defined(TCC8925S)
		#define NAND_IO_USE_DMA_ACCESS
	#elif defined(TCC893X)
		#define NAND_IO_USE_DMA_ACCESS
	#elif defined(TCC88XX)
		#define NAND_IO_USE_DMA_ACCESS
	#else
		#error nand access type is not selected.
	#endif
#elif defined(_LINUX_)
	#if defined(TCC801X) || defined(TCC892X) || defined(TCC370X)
		#define NAND_IO_USE_NDMA_ACCESS
	#elif defined(TCC8925S)
		#define NAND_IO_USE_DMA_ACCESS
	#elif defined(TCC893X)
		#define NAND_IO_USE_NDMA_ACCESS
	#elif defined(TCC88XX)
		#define NAND_IO_USE_DMA_ACCESS
	#elif defined(TCC800X)
		#define NAND_IO_USE_MCU_ACCESS
	#else
		#error nand access type is not selected.
	#endif
#else
	#if defined(TCC801X) || defined(TCC370X)
		#define NAND_IO_USE_NDMA_ACCESS
 	#else
		#error nand access type is not selected.
	#endif
#endif

#if defined(NAND_IO_USE_NDMA_ACCESS) || defined(NAND_IO_USE_DMA_ACCESS)
#if defined(_LINUX_) && defined(KERNEL_DRIVER)
	#if defined(TCC892X)
	#define __USE_NAND_ISR__
	#elif defined(TCC8925S) || defined(TCC893X)
	#define __USE_NAND_ISR__
	#elif defined(TCC88XX)
	//#define __USE_NAND_ISR__
	#endif	
#endif

#endif

#if defined(TCC88XX)								//TCC88X NFC BASE ADDR: 0xF05B0000
#define	NAND_IO_HwCMD_PA							*(volatile unsigned long*)0xF05B0000		//( gNAND_IO_pHwND->CMD )
#define	NAND_IO_HwLADR_PA							*(volatile unsigned long*)0xF05B0004		//( gNAND_IO_pHwND->LADR )
#define	NAND_IO_HwDATA_PA							*(volatile unsigned long*)0xF05B0010		//( gNAND_IO_pHwND->WDATA.D32 )
#define	NAND_IO_HwLDATA_PA							*(volatile unsigned long*)0xF05B0020		//( gNAND_IO_pHwND->LDATA )
#define	NAND_IO_HwSDATA_PA							*(volatile unsigned long*)0xF05B000C		//(( gNAND_IO_DataBusType == NAND_IO_MEM_BUS ) ? gNAND_IO_pHwND->WDATA.D8 : gNAND_IO_pHwND->SDATA.D32 )
#define	NAND_IO_HwSADR_PA							*(volatile unsigned long*)0xF05B0008		//( gNAND_IO_pHwND->SADR )
#elif defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
#define	NAND_IO_HwCMD_PA							*(volatile unsigned long*)0x76600000		//( gNAND_IO_pHwND->CMD )
#define	NAND_IO_HwLADR_PA							*(volatile unsigned long*)0x76600004		//( gNAND_IO_pHwND->LADR )
#define	NAND_IO_HwDATA_PA							*(volatile unsigned long*)0x76600010		//( gNAND_IO_pHwND->WDATA.D32 )
#define	NAND_IO_HwLDATA_PA							*(volatile unsigned long*)0x76600020		//( gNAND_IO_pHwND->LDATA )
#define	NAND_IO_HwSDATA_PA							*(volatile unsigned long*)0x7660000C		//(( gNAND_IO_DataBusType == NAND_IO_MEM_BUS ) ? gNAND_IO_pHwND->WDATA.D8 : gNAND_IO_pHwND->SDATA.D32 )
#define	NAND_IO_HwSADR_PA							*(volatile unsigned long*)0x76600008		//( gNAND_IO_pHwND->SADR )
#elif defined(TCC801X) || defined(TCC370X) || defined(TCC800X)
#define	NAND_IO_HwCMD_PA							*(volatile unsigned long*)0x8000D000		//( gNAND_IO_pHwND->CMD )
#define	NAND_IO_HwLADR_PA							*(volatile unsigned long*)0x8000D004		//( gNAND_IO_pHwND->LADR )
#define	NAND_IO_HwDATA_PA							*(volatile unsigned long*)0x8000D010		//( gNAND_IO_pHwND->WDATA.D32 )
#define	NAND_IO_HwLDATA_PA							*(volatile unsigned long*)0x8000D020		//( gNAND_IO_pHwND->LDATA )
#if defined(TCC800X)
#define	NAND_IO_HwSDATA_PA							*(volatile unsigned long*)0x8000D040		//(( gNAND_IO_DataBusType == NAND_IO_MEM_BUS ) ? gNAND_IO_pHwND->WDATA.D8 : gNAND_IO_pHwND->SDATA.D32 )
#define	NAND_IO_HwSADR_PA							*(volatile unsigned long*)0x8000D00C		//( gNAND_IO_pHwND->SADR )
#else //defined(TCC801X) || defined(TCC370X)
#define	NAND_IO_HwSDATA_PA							*(volatile unsigned long*)0x8000D00C		//(( gNAND_IO_DataBusType == NAND_IO_MEM_BUS ) ? gNAND_IO_pHwND->WDATA.D8 : gNAND_IO_pHwND->SDATA.D32 )
#define	NAND_IO_HwSADR_PA							*(volatile unsigned long*)0x8000D008		//( gNAND_IO_pHwND->SADR )
#endif
#endif

#define NAND_IO_STATUS_ENABLE					0x0001
#define NAND_IO_STATUS_INTERLEAVING_MASK		0x00F0
#define NAND_IO_STATUS_INTERLEAVING_CHIP1		0x0010
#define NAND_IO_STATUS_INTERLEAVING_CHIP2		0x0020
#define NAND_IO_STATUS_INTERLEAVING_CHIP3		0x0040
#define NAND_IO_STATUS_INTERLEAVING_CHIP4		0x0080

#define NAND_IO_STATUS_INTERLEAVING_DIE0		0x10
#define NAND_IO_STATUS_INTERLEAVING_DIE1		0x20

#define NAND_IO_DMA_WRITE						0x0001
#define NAND_IO_DMA_READ						0x0002

#if defined(TCC88XX) || defined(TCC892X) || defined(TCC8925S) || defined(TCC801X) || defined(TCC893X) || defined(TCC370X) || defined(TCC800X)
#define NAND_SB_BOOT_PAGE_SIZE					260
#define NAND_SB_BOOT_PAGE_ECC_SIZE				304
#endif

//*****************************************************************************
//*
//*
//*                       [ INTERNAL STRUCT DEFINE ]
//*
//*
//*****************************************************************************
#define GMC_NAND_NOMAL_BOOT_TYPE				0x54435342		// TCSB
#define GMC_NAND_SECURE_BOOT_TYPE				0x54435353		// TCSS

typedef struct	__tag_NAND_GOLDEN_INFO
{
	unsigned long	Signature;
	unsigned short	PageSize;
	unsigned short	SpareSize;
	unsigned short	Ppage;
	unsigned short	PpB;
	unsigned long	PpBforAddress;
	unsigned short	EccType;
	unsigned short	EccCodeSize;
	unsigned long	TargetAddress;	// Working Address
	unsigned long	BootLoaderSize;
	unsigned long	LastBlockPpB;
	unsigned short	RomCopyNum;
	unsigned short	RomBlockNum;
	unsigned long	RomCRC;
	unsigned long	BlockUppuerLimit;
	unsigned long	BlockLowerLimit;
#if defined(TCC93XX) || defined(TCC88XX) || defined(TCC892X) || defined(TCC8925S) || defined(TCC801X) || defined(TCC370X) || defined(TCC800X)
	unsigned char	CAHLD;
	unsigned char	CAPW;
	unsigned char	CASTP;
	unsigned char	CmdAddrCycSetup;
	unsigned char	RDHLD;
	unsigned char	RDPW;
	unsigned char	RDSTP;
	unsigned char	ReadCycSetup;
#endif
//---------------------------------- 44Byte
	unsigned long	GoldenInfoCRC;
	unsigned long	SecureData[4];
} 	NAND_GOLDEN_INFO;

typedef struct	__tag_NAND_BOOTLOADER_INFO
{
	unsigned short	BL_ColCycle;
	unsigned short	BL_RowCycle;
	unsigned long	BL_ReadCycle;
	unsigned long	BL_MediaType;
	unsigned long	BL_SpareDataSize;
	unsigned long	BL_TargetAddress;	// Working Address
	unsigned long	BL_MemInitCodeSize;
	unsigned long	BL_MemInitCodePpB;
	unsigned long	BL_RomFileSize;
	unsigned short	BL_RomCopyNum;
	unsigned short	BL_RomBlockNum;
	unsigned long	BL_BlockUppuerLimit;
	unsigned long	BL_BlockLowerLimit;
	unsigned long	BL_LastBlockPpB;
	unsigned long	BL_BytesPerSector;
	unsigned long	BL_RomFileCRC;
} 	NAND_BOOTLOADER_INFO;

typedef struct __tag_NAND_IO_ECCSizeInfo
{
	unsigned char			ucErrorNum;
	unsigned char			ucReadReclaimThreshold;
	unsigned char			ucEccCodeSize;
#if defined(NFC_VER_50) ||defined(NFC_VER_100)
	unsigned int			DecodeFlag;
#endif
} NAND_IO_ECC_INFO;

typedef struct __tag_NAND_IO_WriteStatus
{
	unsigned char			ChipNo;
	unsigned char			BlockStatus;
	unsigned int			ErrorPHYPageAddr;
} NAND_IO_WRITESTATUS;

typedef struct __tag_NAND_IO_BadBlockAddr
{
	unsigned char			BlockStatus[4];
	unsigned int			BadBlkPHYAddr[4];
} NAND_IO_BADBLOCK;

typedef struct __tag_NAND_IO_Cycle
{
	unsigned char			STP;
	unsigned char			PW;
	unsigned char			HLD;
	unsigned int			RegValue;
} NAND_IO_CYCLE;

typedef struct __tag_NAND_IO_DeviceCode
{
	unsigned short			Code[6];			// Factory ID code
} NAND_IO_DEVID;

typedef struct __tag_NAND_IO_Feature
{
	const char				*DeviceName[3];
	NAND_IO_DEVID			DeviceID;			// Maker & Device ID Code
	unsigned short  		BpD;				// Blocks per Die
	unsigned char			ucDDPBit;			// Dual Die Package select bit
	unsigned short			BBpD;				// Bad Block Number per Die
	unsigned short  		PpB;				// Page Number Per Block
	unsigned short  		PageSize;			// Page Size
	unsigned short  		SpareSize;			// Spare Size
	unsigned char			ucColCycle;			// Column Address Cycle
	unsigned char			ucRowCycle;			// Row Address Cycle
	unsigned char			ucWriteCycleTime;
	unsigned char			ucWritePulseWidthTime;
	unsigned char			ucWriteHoldTime;
	unsigned char			ucReadCycleTime;
	unsigned char			ucReadAccessTime;
	unsigned char			ucReadPulseWidthTime;
	unsigned char			ucReadHoldTime;
	unsigned long			MediaType;			// Characters of NANDFLASH
	unsigned long			UsableFunc;			// Supported Functions
	const unsigned char		*pucLSBPageTable;	// LSB page table (paired-page)
} NAND_IO_FEATURE;

typedef struct __tag_NAND_IO_DevInfo
{
	NAND_IO_FEATURE			Feature;			// Feature of NANDFLASH
	unsigned short			IoStatus;			// IO Status
	unsigned short			ChipNo;				// ChipSelect Number
	unsigned short			CmdMask;			// Command Mask Bit
	unsigned short			EccType;			// Type of ECC [ SLC , MLC4 ]
	unsigned short			CodewordSize;
	unsigned short			EccCodeSize;
	unsigned short			EccCodeSizeTotalInDword;
	unsigned short			usCodewordsPerPage;
	unsigned short			ExtInterleaveUsable;
	unsigned short			ShiftPpB;
	unsigned short			ShiftBytesPerSector;
	unsigned short			usShiftCodewordsPerPage;
	unsigned char			fCacheProgramed[DIE_COUNT_MAX];
	NAND_IO_WRITESTATUS		WriteStatus;
	NAND_IO_BADBLOCK		BadBlockInfo;
	unsigned int			(*fnSetReadRetry)(struct __tag_NAND_IO_DevInfo *, unsigned int);

	unsigned char			ucDieIndex;
	unsigned char			fSkipDataAreaRandomizing;
	unsigned char			fReadCommandSent;
	unsigned char			fReadRetryNow;
	unsigned int			uiInterleavingPageAddr[DIE_COUNT_MAX];
	NAND_IO_OPERATION_T		InterleavingOperation[DIE_COUNT_MAX];
	unsigned char 			ucInterleavingStatus;
	unsigned char 			fInterleaveUsable;
} NAND_IO_DEVINFO;


#define MAX_NAND_WORK_SECTOR_NUM	512
typedef	struct __tag_WriteStuffInfo
{
	unsigned short			StuffInfoNum;
	unsigned short			StartPPageNo[MAX_NAND_WORK_SECTOR_NUM];
	unsigned short			SectorSize[MAX_NAND_WORK_SECTOR_NUM];
	unsigned char			*nStuffPageBuffer;
} ndd_stuff_info;

typedef	struct __tag_NanddWorkInfo
{
	unsigned int 			SectorNum;		// Total Sector Num
	unsigned int			StartSector;
	unsigned int			BufferAddr[MAX_NAND_WORK_SECTOR_NUM];

	unsigned int			PhyPageNum;
	unsigned int			PhyPageAddr[MAX_NAND_WORK_SECTOR_NUM];
	unsigned short			StartPPageNo[1024];
	unsigned short			PPageSize[1024];	// TNFTL_MAX_SUPPORT_NAND_IO_PAGE_SIZE_PER_1BLOCK
	unsigned int			FTLPPage;
	unsigned char			PageWriteFunc[1024];
	ndd_stuff_info			StuffInfo;
	unsigned char			*nSpareBuffer;
	NAND_IO_DEVINFO			*nDevInfo[MAX_NAND_WORK_SECTOR_NUM];
} ndd_work_info;

typedef struct __tag_NAND_IO_MakerInfo
{
	unsigned short			MaxSupportNAND[MAX_SUPPORT_MAKER_NAND];
	unsigned short			MakerID[MAX_SUPPORT_MAKER_NAND];
	NAND_IO_FEATURE			*DevInfo[MAX_SUPPORT_MAKER_NAND];
} NAND_IO_MAKERINFO;

//*****************************************************************************
//*
//*
//*                       [ EXTERNAL VARIABLE DEFINE ]
//*
//*
//*****************************************************************************

//*****************************************************************************
//*
//*
//*                       [ EXTERNAL FUCTIONS DEFINE ]
//*
//*
//*****************************************************************************
void NAND_IO_PowerUp( void );
void NAND_IO_PowerDown( void );
void NAND_IO_SkipDataAreaRandomizing(unsigned int fEnable);
void NAND_IO_Reset( unsigned short nChipNo, int nMode );
void NAND_IO_SuppressMessage( unsigned int fSuppress );
unsigned int NAND_IO_GetFeatureOfNAND( TNFTL_OBJECT_T *pTnftlObj );
unsigned int NAND_IO_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPage );
unsigned int NAND_IO_ReadPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPageForNextTurn );
unsigned int NAND_IO_EraseBlock( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress );
unsigned int NAND_IO_MultiPlane_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int NAND_IO_MultiPlane_WriteLastPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int NAND_IO_ReadSpare( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiTag, unsigned int uiTagSize );
unsigned int NAND_IO_IsReadReclaimRequired( TNFTL_OBJECT_T *pTnftlObj );
unsigned int NAND_IO_IsBL1ReadReclaimRequired( unsigned int uiBL1EccErrorBit );
void NAND_IO_MarkBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress );
unsigned int NAND_IO_MakeBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer );
unsigned int NAND_IO_MakeBL2Page( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer, unsigned int uiPageSize, const unsigned int *puiTag );
unsigned long NAND_IO_GetBootStatus( void );
NAND_IO_ERROR_T NAND_IO_GetUID( NAND_IO_DEVINFO *nDevInfo, unsigned short *nCmd, unsigned char *rReadData );
unsigned char _NAND_IO_GetEccErrorBitCount(void);
unsigned long NAND_IO_GetBootType(void);
NAND_IO_ERROR_T NAND_IO_ReadUserSizePage( NAND_IO_DEVINFO *nDevInfo, unsigned long nPageAddr, unsigned short nColumnAddr, unsigned long nReadSize, unsigned char *nReadBuffer );
TNFTL_BAD_BLOCK_STATUS_T NAND_IO_IsBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress );

void NAND_IO_IRQ_Init( int mode );
void NAND_IO_IRQ_Enable( unsigned int fEnable );

void NAND_IO_AdjustPageSpareSize( unsigned short *pusPPages, unsigned short *pusPageSize, unsigned short *pusSpareSize, unsigned int *pECCType );
NAND_IO_ERROR_T NAND_IO_ReadPage_For_TCC800X( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );
NAND_IO_ERROR_T NAND_IO_WritePage_For_TCC800X( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer );
NAND_IO_ERROR_T NAND_IO_WriteGoldenPage_For_TCC800X(TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer );
NAND_IO_ERROR_T NAND_IO_ReadGoldenPage_For_TCC800X(TNFTL_OBJECT_T *pTnftlObj,unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag );

#endif

