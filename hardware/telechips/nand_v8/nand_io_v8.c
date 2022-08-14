/****************************************************************************
 *   FileName    : nand_io_v8.c
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

#if defined(TCC801X)
#include "config/sdk/config.h"
#endif

#include "nand_def.h"
#include "tnftl_v8_api.h"
#include "nand_io_v8.h"
#include "nand_regs.h"
#include "nand_list.h"
#include "nand_board_v8.h"

#if defined(_WINCE_)
	#if defined(NAND_FOR_KERNEL)
		#include "Tcc_ckc.h"
		#include "tcc_gpio.h"
	#else
		#include "Tca_ckc.h"
	#endif
	#include "globals.h"
	#include "bsp.h"
	#include "args.h"
	#if defined(TCC893X)
	#include "TCC893x_Structures.h"
	#include "TCC893x_Physical.h"
	#else
	#include "TCC892x_Structures.h"
	#include "TCC892x_Physical.h"
	#endif
	#include "IO_TCCXXX.h"
	#include "TC_DRV.h"
	#include "stdlib.h"

#elif defined(_LINUX_)
	#if defined(ANDROID_BOOT)
		#include <platform/globals.h>
	#elif defined(ANDROID_KERNEL)
		#include <mach/globals.h>
	#endif
	#if defined(NAND_FOR_KERNEL)
		#include <linux/signal.h>
		
		#include <asm/system.h>
		#include <linux/kernel.h>
		#include <linux/string.h>
		#include <linux/module.h>
		#include <mach/bsp.h>
		#include <asm/io.h>
		//#include "TC_DRV.h"
	#elif defined(ANDROID_BOOT)
		#include <common.h>
		#include <platform/reg_physical.h>
		#include "config.h"
		#include "debug.h"
		#include <platform/globals.h>
		#include "tnftl/TC_DRV.h"
		#if defined(TCC893X)
		#include "tcc_chip_rev.h"
		#endif
	#endif
	#if defined(__USE_NAND_ISR__)
	#include <linux/sched.h>
	#endif
#elif defined(_NU_)
#include "Config/SDK/config.h"
#include "Config/MCU/MCU_HWCfg.h"
#include "SERVICE/TC_File/TC_File.h"
#include "common/globals.h"
#include "nand_drv_v8.h"
#include "drv/fs/disk.h"
#include "boot/main.h"	
#endif

#include "nand_utils.h"

/* for sorting */
#define CTRL_FUNC
#define IRQ_FUNC
#define MISC_FUNC
//#define TEST_FUNC

#if defined(_LINUX_)
#define __INLINE					static inline
#else
#define __INLINE					__inline
#endif

//******************************************************************
//
// Definitions
//
//******************************************************************
#ifndef dim
#define dim(a)					( sizeof(a) / sizeof(a[0]) )
#endif
#ifndef ENABLE
#define ENABLE 1
#endif
#ifndef DISABLE
#define DISABLE 0
#endif

#define	HwCHCTRL_CONT_C				Hw15				// DMA transfer begins from C_SADR/C_DADR Address. It must be used after the former transfer has been executed, so that C_SADR and C_DADR contain a meaningful value.
#define	HwCHCTRL_CONT_ST			(0)					// DMA trnaster begins from ST_SADR/ST_DADR Address
#define	HwCHCTRL_DTM_EN				Hw14				// Differential Transfer Mode Enable
#define	HwCHCTRL_DTM_ON				Hw14				// Differential Transfer Mode Enable
#define	HwCHCTRL_DTM_OFF			(0)					// Differential Transfer Mode Disable
#define	HwCHCTRL_SYNC_ON			Hw13				// Synchronize HardWare Request
#define	HwCHCTRL_SYNC_OFF			(0)					// Do not Synchronize HardWare Request
#define	HwCHCTRL_SYNC_EN			Hw13				// Synchronize Hardware Request
#define	HwCHCTRL_HRD_W				Hw12				// ACK/EOT signals are issued When DMA-Write Operation
#define	HwCHCTRL_LOCK_EN			Hw11				// DMA transfer executed with lock transfer
#define	HwCHCTRL_BST_NOARB			Hw10				// DMA transfer executed with no arbitration(burst operation)
#define	HwCHCTRL_HRD_WR				Hw12				// ACK/EOT signals are issued When DMA-Write Operation
#define	HwCHCTRL_HRD_RD				(0)					// ACK/EOT signals are issued When DMA-Read Operation
#define	HwCHCTRL_LOCK_ON			Hw11				// DMA transfer executed with lock transfer
#define	HwCHCTRL_LOCK_OFF			(0)					//
#define	HwCHCTRL_BST_BURST			Hw10				// DMA transfer executed with no arbitration(burst operation)
#define	HwCHCTRL_BST_ARB			(0)					// DMA transfer executed wth arbitration
#define	HwCHCTRL_TYPE_SINGE			(0)					// SINGLE transfer with edge-triggered detection
#define	HwCHCTRL_TYPE_HW			Hw8					// HW Transfer
#define	HwCHCTRL_TYPE_SW			Hw9					// SW transfer
#define	HwCHCTRL_TYPE_SINGL			(Hw9|Hw8)			// SINGLE transfer with level-triggered detection
#define	HwCHCTRL_TYPE_SL			(Hw9|Hw8)			// SINGLE transfer with level-triggered detection
#define HwCHCTRL_TYPE_SE			HwZERO				// SINGLE transfer with edge-triggered detection

#define	HwCHCTRL_BSIZE_1			(0)					// 1 Burst transfer consists of 1 read or write cycle
#define	HwCHCTRL_BSIZE_2			Hw6					// 1 Burst transfer consists of 2 read or write cycles
#define	HwCHCTRL_BSIZE_4			Hw7					// 1 Burst transfer consists of 4 read or write cycles
#define	HwCHCTRL_BSIZE_8			(Hw6|Hw7)			// 1 Burst transfer consists of 8 read or write cycles

#define	HwCHCTRL_WSIZE_8			(0)					// Each cycle read or write 8bit data
#define	HwCHCTRL_WSIZE_16			Hw4					// Each cycle read or write 16bit data
#define	HwCHCTRL_WSIZE_32			Hw5					// Each cycle read or write 32bit data

#define	HwCHCTRL_FLAG				Hw3					// Clears FLAG to 0
#define	HwCHCTRL_IEN_ON				Hw2					// At the same time the FLAG goes to 1, DMA interrupt request is generated
#define HwCHCTRL_IEN_EN				Hw2					// At the same time the FLAG goes to 1, DMA interrupt request is generated
#define	HwCHCTRL_IEN_OFF			~Hw2				//
#define	HwCHCTRL_REP_EN				Hw1					// The DMA channel remains enabled
#define	HwCHCTRL_REP_DIS			~Hw1				// After all of hop transfer has executed, the DMA channel is disabled
#define	HwCHCTRL_EN_ON				Hw0					// DMA channel is Enabled
#define	HwCHCTRL_EN_OFF				~Hw0				// DMA channel is terminated and disabled/*}}}*/
#define	HwCHCTRL_EN_EN				Hw0					// DMA channel is enabled. If software type transfer is selected, this bit generates DMA request directly, or if hardware type transfer is used, the selected interrupt request flag generate DMA request

#if defined(NAND_FOR_KERNEL) && defined(_LINUX_)
	#define GPIO_BASE_ADDRESS		(HwGPIO_BASE)
	#define EDI_BASE_ADDRESS		(HwEDI_BASE)
	#define NFC_BASE_ADDRESS		(HwNFC_BASE)
	#define ECC_BASE_ADDRESS		(HwECC_BASE)
	#define IOBUSCFG_BASE_ADDRESS	(HwIOBUSCFG_BASE)
	#define PIC_BASE_ADDRESS		(HwPIC_BASE)
	#define NAND_DMA_BASE_ADDRESS	(HwGDMA0_BASE)
#else
	#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC93XX)
		#define GPIO_BASE_ADDRESS		(&HwGPIO_BASE)
		#define EDI_BASE_ADDRESS		(&HwEDI_BASE)
		#define NFC_BASE_ADDRESS		(&HwNFC_BASE)
		#define ECC_BASE_ADDRESS		(&HwECC_BASE)
		#define IOBUSCFG_BASE_ADDRESS	(&HwIOBUSCFG_BASE)
		#define PIC_BASE_ADDRESS		(&HwPIC_BASE)
		#if defined(_LINUX_)
			#define NAND_DMA_BASE_ADDRESS	(&HwGDMA0_BASE)
		#else
			#define NAND_DMA_BASE_ADDRESS	(&HwGDMA2_BASE)
		#endif
	#elif defined(TCC88XX) || defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
		#define GPIO_BASE_ADDRESS		(HwGPIO_BASE)
		#define EDI_BASE_ADDRESS		(HwEDI_BASE)
		#define NFC_BASE_ADDRESS		(HwNFC_BASE)
		#define ECC_BASE_ADDRESS		(HwECC_BASE)
		#define IOBUSCFG_BASE_ADDRESS	(HwIOBUSCFG_BASE)
		#define PIC_BASE_ADDRESS		(HwPIC_BASE)
		#if defined(_LINUX_)
			#define NAND_DMA_BASE_ADDRESS	(HwGDMA0_BASE)
		#else
			#define NAND_DMA_BASE_ADDRESS	(HwGDMA2_BASE)
		#endif
	#elif defined(TCC801X) || defined(TCC370X) || defined(TCC800X)
		#define NFC_BASE_ADDRESS		(HwNFC_BASE)
		#define ECC_BASE_ADDRESS		(HwECCBASE)
		#if defined(_LINUX_)
			#define NAND_DMA_BASE_ADDRESS	(HwGDMA0_BASE)
		#else
			#define NAND_DMA_BASE_ADDRESS	(HwGDMA2_BASE)
		#endif
	#endif
#endif

#if defined(_LINUX_) && defined(TCC88XX)
#define NAND_PGDMANCTRL			PGDMANCTRL
#endif

typedef enum {
	NORMAL_PROGRAM,
	CACHE_PROGRAM,
	CACHE_LAST_PROGRAM,
	MULTI_PLANE_PROGRAM,
	MULTI_PLANE_LAST_PROGRAM,
	PROGRAM_END
} PROGRAM_TYPE_T;


//==================================================================================
// BL0 definition
//==================================================================================
// Do not change! Just add! For backward compatibility.
// Attribute
#define BL0_RAND_IO								0x00000001		// Random Data In/Out
#define BL0_MICRON_READ_RETRY					0x00000002

//==================================================================================
// BL1 definition
//==================================================================================
// Do not change! Just add! For backward compatibility.
// Attribute
#define BL1_MEDIA_TYPE_08BIT						0x00000001		// 8Bit BUS
#define BL1_MEDIA_TYPE_16BIT						0x00000002		// 16Bit BUS
#define BL1_MEDIA_TYPE_32BIT						0x00000003		// 32Bit BUS
#define BL1_MEDIA_TYPE_PARALLEL						0x00000004		// Parallel mode
#define BL1_MEDIA_TYPE_RAND_IO						0x00000010		// Randomizer used
#define BL1_MEDIA_TYPE_OCCUPIED_2					0x00000020		// Big Page Size, previousely
#define BL1_MEDIA_TYPE_OCCUPIED_3					0x00000040		// Single Layer Cell, previousely
#define BL1_MEDIA_TYPE_OCCUPIED_4					0x00000800		// 24Bit MLC ECC, previousely
#define BL1_MEDIA_TYPE_OCCUPIED_5					0x00010000		// Data 8Bit Bus, previousely
#define BL1_MEDIA_TYPE_SPARE_16BIT					0x00100000		// Change SPARE ECC to 16Bit(because of spare size overflow)
#define BL1_MEDIA_TYPE_SPARE_24BIT					0x00200000		// Change SPARE ECC to 24Bit(because of spare size overflow)

// EccType
#define BL1_ECC_TYPE_EMBEDDED						0x0000
#define BL1_ECC_TYPE_1BIT							0x0001			// 10 Byte
#define BL1_ECC_TYPE_4BIT							0x0002			// BCH: 7 Byte( 52 bit)	turn: 2, byte remain: 3, register mask: 0x0F
#define BL1_ECC_TYPE_8BIT							0x0004			// BCH:13 Byte (104 bit)	turn: 4, byte remain: 1, register mask: 0xFF
#define BL1_ECC_TYPE_12BIT							0x0010			// BCH:20 Byte (156 bit)	turn: 5, byte remain: 4, register mask: 0x0F
#define BL1_ECC_TYPE_14BIT							0x0020			// BCH:23 Byte (184 bit)	turn: 6, byte remain: 3, register mask: 0xFF
#define BL1_ECC_TYPE_16BIT							0x0040			// BCH:26 Byte (207 bit)turn: 7, byte remain: 2, register mask: 0x7F
#define BL1_ECC_TYPE_24BIT							0x0080			//
#define BL1_ECC_TYPE_40BIT							0x0100			// 
#define BL1_ECC_TYPE_60BIT							0x0200			// 
//==================================================================================


//******************************************************************
//
// Extern Variables
//
//******************************************************************
#if defined(_WINCE_)
	tSYSTEM_PARAM  		*pSYS_PARAM;
		PGDMANCTRL			pNAND_DMA;
#elif defined(_LINUX_)
		NAND_PGDMANCTRL			pNAND_DMA;
#elif defined(_NU_)
		PGDMANCTRL			pNAND_DMA;
#endif

#if defined(_LINUX_)
	#if defined(NAND_FOR_KERNEL)
		struct dma_buf
		{
			void *v_addr;
			unsigned int dma_addr;
			int buf_size;
		};
		struct dma_buf dma_t;
		EXPORT_SYMBOL(dma_t);
	#else
		#if defined(ANDROID_BOOT)
			unsigned int *dma_addr = NULL;
		#else
			#define DMA_ADDR	(BSS_OFFSET - 0x00700000)
		#endif
	#endif
#endif

//--------------------------------------------------------------------
//		TCC8002 Support ECC I/P GF(13)
//		(1Bit/ 4Bit/ 8Bit/ 12Bit  )
//--------------------------------------------------------------------
#if defined(NFC_VER_50)
static const NAND_IO_ECC_INFO s_MLC_ECC_1Bit  = {         1,                1,           7  ,	ECC_IRQ_SLC_DECODE_END_FLAG};
static const NAND_IO_ECC_INFO s_MLC_ECC_4Bit  = {         4,                4,           7  , 	ECC_IRQ_MLC4_DECODE_END_FLAG};
static const NAND_IO_ECC_INFO s_MLC_ECC_8Bit  = {         8,                8,          13  , 	ECC_IRQ_MLC8_DECODE_END_FLAG};
static const NAND_IO_ECC_INFO s_MLC_ECC_12Bit = {        12,               12,          20  , 	ECC_IRQ_MLC12_DECODE_END_FLAG};

//--------------------------------------------------------------------
//		TCC89/92x Support ECC I/P GF(13)
//		(1Bit/ 4Bit/ 8Bit/ 12Bit /14Bit /16Bit )
//--------------------------------------------------------------------
#elif defined(NFC_VER_100)
                                                /* ErrorNum, ReclaimThreshold, EccCodeSize */
static const NAND_IO_ECC_INFO s_MLC_ECC_1Bit  = {         1,                1,           7  };
static const NAND_IO_ECC_INFO s_MLC_ECC_4Bit  = {         4,                4,           7  };
static const NAND_IO_ECC_INFO s_MLC_ECC_8Bit  = {         8,                8,          13  };
static const NAND_IO_ECC_INFO s_MLC_ECC_12Bit = {        12,               12,          20  };
static const NAND_IO_ECC_INFO s_MLC_ECC_16Bit = {        16,               16,          26  };

//--------------------------------------------------------------------
//		TCC93x Support ECC I/P GF(14)
//		(1Bit/ 4Bit/ 6Bit/ 12Bit /16Bit /24Bit  )
//--------------------------------------------------------------------
#elif defined(NFC_VER_200)
                                                /* ErrorNum, ReclaimThreshold, EccCodeSize */
static const NAND_IO_ECC_INFO s_MLC_ECC_1Bit  = {         1,                1,           7  };
static const NAND_IO_ECC_INFO s_MLC_ECC_4Bit  = {         4,                4,           7  };
static const NAND_IO_ECC_INFO s_MLC_ECC_6Bit  = {         6,                6,          11  };
static const NAND_IO_ECC_INFO s_MLC_ECC_12Bit = {        12,               12,          21  };
static const NAND_IO_ECC_INFO s_MLC_ECC_16Bit = {        16,               16,          28  };
static const NAND_IO_ECC_INFO s_MLC_ECC_24Bit = {        24,               18,          42  };

//--------------------------------------------------------------------
//		TCC8925S Support ECC I/P GF(14)
//		(1Bit/ 4Bit/ 6Bit/ 8Bit/ 12Bit/ 16Bit/ 24Bit/ 40Bit/ 60Bit  )
//--------------------------------------------------------------------
#elif defined(NFC_VER_300)
                                                /* ErrorNum, ReclaimThreshold, EccCodeSize */
static const NAND_IO_ECC_INFO s_MLC_ECC_1Bit  = {         1,                1,           7  };
static const NAND_IO_ECC_INFO s_MLC_ECC_4Bit  = {         4,                4,           7  };
static const NAND_IO_ECC_INFO s_MLC_ECC_6Bit  = {         6,                6,          11  };
static const NAND_IO_ECC_INFO s_MLC_ECC_8Bit  = {         8,                8,          14  };
static const NAND_IO_ECC_INFO s_MLC_ECC_12Bit = {        12,               12,          21  };
static const NAND_IO_ECC_INFO s_MLC_ECC_16Bit = {        16,               16,          28  };
static const NAND_IO_ECC_INFO s_MLC_ECC_24Bit = {        24,               18,          42  };
static const NAND_IO_ECC_INFO s_MLC_ECC_40Bit = {        40,               30,          70  };
static const NAND_IO_ECC_INFO s_MLC_ECC_60Bit = {        60,               45, 	       105  };
#endif


#if defined(NFC_VER_50)
static PNFC_V05_T s_pNFC;
static PECC_V05_T s_pECC;
#elif defined(NFC_VER_100)
static PNFC_V1_T s_pNFC;
static PECC_V1_T s_pECC;
static PIOBUSCFG pIOBUSCFG_T;
#elif defined(NFC_VER_200)
static PNFC_V2_T s_pNFC;
static PECC_V2_T s_pECC;
#elif defined(NFC_VER_300)
static PNFC_V3_T s_pNFC;
static PECC_V3_T s_pECC;
#endif

#if !(defined(TCC801X) || defined(TCC370X) || defined(TCC800X))
static PPIC s_pPIC;
#endif

static NAND_IO_DEVINFO	*s_pDevInfo;

static unsigned int		*s_puiDMA_PhyBuffer = NULL;
static unsigned int		*s_puiDMA_WorkBuffer = NULL;

//******************************************************************
//
// Global Variables
//
//******************************************************************
//=============================================================================
//*                           [ CONST DATA DEFINE ]
//=============================================================================
const NAND_IO_MAKERINFO	NAND_SupportMakerInfo =
{
// MAXIMUM SUPPORT NANDFLASH
	{
		dim( SAMSUNG_NAND_DevInfo ),
		dim( TOSHIBA_NAND_DevInfo ),
		dim( HYNIX_NAND_DevInfo ),
		dim( MICRON_NAND_DevInfo ),
		dim( SPANSION_NAND_DevInfo ),
		dim( FIDELIX_NAND_DevInfo ),
		dim( MACRONIX_NAND_DevInfo ),
		dim( EON_NAND_DevInfo ),
		dim( EMST_NAND_DevInfo ),
		
	},
// NAND MAKER ID
	{
		SAMSUNG_NAND_MAKER_ID,
		TOSHIBA_NAND_MAKER_ID,
		HYNIX_NAND_MAKER_ID,
		MICRON_NAND_MAKER_ID,
		SPANSION_NAND_MAKER_ID,
		FIDELIX_NAND_MAKER_ID,
		MACRONIX_NAND_MAKER_ID,
		EON_NAND_MAKER_ID,
		EMST_NAND_MAKER_ID,
	},

// POINTER OF NANDFLASH INFOMATION
	{
		( NAND_IO_FEATURE * )SAMSUNG_NAND_DevInfo,
		( NAND_IO_FEATURE * )TOSHIBA_NAND_DevInfo,
		( NAND_IO_FEATURE * )HYNIX_NAND_DevInfo,
		( NAND_IO_FEATURE * )MICRON_NAND_DevInfo,
		( NAND_IO_FEATURE * )SPANSION_NAND_DevInfo,
		( NAND_IO_FEATURE * )FIDELIX_NAND_DevInfo,
		( NAND_IO_FEATURE * )MACRONIX_NAND_DevInfo,
		( NAND_IO_FEATURE * )EON_NAND_DevInfo,
		( NAND_IO_FEATURE * )EMST_NAND_DevInfo,
	}
};

#if defined(NFC_VER_50)
#define ECC_DATA_MAX_BYTE			20
#elif defined(NFC_VER_100)
#define ECC_DATA_MAX_BYTE			26
#elif defined(NFC_VER_200)
#define ECC_DATA_MAX_BYTE			42
#elif defined(NFC_VER_300)
#define ECC_DATA_MAX_BYTE			105
#endif
#define ECC_DATA_MAX_DWORD			((ECC_DATA_MAX_BYTE+3)/4)

#if defined(NAND_FOR_KERNEL) && defined(_WINCE_)
DWORD		gNFC_IRQ_Intr;
DWORD		gEXT_IRQ_Intr;
HANDLE 		gNFC_IRQ_Handle = NULL;
HANDLE 		gEXT_IRQ_Handle = NULL;
#endif

#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
static unsigned int s_fIsNAND_ISR_USED		= FALSE;
static unsigned int	s_fIsNAND_RB_ISR_USED	= FALSE;
static unsigned int s_fIsNAND_IRQ_Enable	= FALSE;

#define NAND_IO_IRQ_STATE_NONE			0
#define NAND_IO_IRQ_STATE_WRITE			1
#define NAND_IO_IRQ_STATE_READ			2
#define NAND_IO_IRQ_STATE_WRITE_RB		3
#define NAND_IO_IRQ_STATE_READ_RB		4
#define NAND_IO_IRQ_STATE_RB			5

typedef struct _tag_NAND_ISR_INFO_T {
	//////////////////////////////////
	// GDMA USED
	NAND_IO_DEVINFO	*pNandIoDevInfo;
	unsigned char 	*pbPageBuffer;			// DMA Buffer
	unsigned char	*pbPhyPageBuffer;		// DMA Physical Buffer
	unsigned char 	*pbEccBuffer;
	unsigned short 	usStartPPage;
	unsigned short 	usEndPPage;
	unsigned short 	usCurrentPPage;
	unsigned short 	usPPagesLeft;
	unsigned int 	iEccOnOff;
	NAND_IO_ERROR_T	error;
	//////////////////////////////////
	// GDMA, NDMA USED
	unsigned char 	ubIsRun;
	unsigned int 	uiState;
	wait_queue_head_t wait_q;
	volatile int wait_complete;
} NAND_ISR_INFO_T;
static NAND_ISR_INFO_T gsNAND_IO_NandIsrInfo;
#endif

enum
{
	NAND_IO_TRANSPER_MCU_MODE = 0,
	NAND_IO_TRANSPER_GDMA_MODE,
	NAND_IO_TRANSPER_NDMA_MODE
};

#if defined(ANDROID_KERNEL)
#if defined(TCC893X)
extern unsigned int tcc_chip_rev;
#endif
#endif

static unsigned int			sNAND_IO_uiTransperMode = 0;
static unsigned int			sNAND_IO_uiMaxBusClkMHz = 0;
static unsigned int			sNAND_IO_uiSharedEccBuffer[ 1344/*maximum spare area*/ / 4 ];
static NAND_IO_DEVINFO		sNAND_IO_DeviceInfo;
static NAND_IO_ERROR_T		sNAND_IO_LastError = NAND_IO_SUCCESS;
static unsigned int			sNAND_IO_DataBusType;
static unsigned int			sNAND_IO_uiPortStatus = 0;
static unsigned char		sNAND_IO_fSuppressMessage = FALSE;

static NAND_IO_CYCLE		s_WriteCycleTime;
static NAND_IO_CYCLE		s_ReadCycleTime;
static NAND_IO_CYCLE		s_CommCycleTime;

static unsigned int			sNAND_IO_uiRandomizeBuffer[PAGE_DWORDSIZE_MAX];

//*****************************************************************************
//*
//*
//*                       [ EXTERNAL FUCTIONS DEFINE ]
//*
//*
//*****************************************************************************
void 			NAND_IO_Init( void );
NAND_IO_ERROR_T NAND_IO_GetDeviceInfo( unsigned short nChipNo, NAND_IO_DEVINFO *nDevInfo );
void 			NAND_IO_Reset( unsigned short nChipNo, int nMode );
void 			NAND_IO_ResetForReadID( unsigned short nChipNo, int nMode );
void 			NAND_IO_ReadID( unsigned short nChipNo, NAND_IO_DEVID *nDeviceCode, int nMode );
NAND_IO_ERROR_T NAND_IO_WriteUserSizePage( NAND_IO_DEVINFO *nDevInfo, unsigned long nPageAddr, unsigned short nColumnAddr, unsigned long nWriteSize, unsigned char *nWriteBuffer );
NAND_IO_ERROR_T NAND_IO_ReadUserSizePage( NAND_IO_DEVINFO *nDevInfo, unsigned long nPageAddr, unsigned short nColumnAddr, unsigned long nReadSize, unsigned char *nReadBuffer );
 void 			NAND_IO_PortControl( int nOnOff );
void 			NAND_IO_PreProcess( void );
void 			NAND_IO_PostProcess( void );
void 			NAND_IO_BusControl( NAND_IO_DEVINFO *nDevInfo );
void 			NAND_IO_SetDataWidth( unsigned long width );
void 			NAND_IO_SetBasicCycleTime( void );
void 			NAND_IO_SetWriteCycleTime( void );
void 			NAND_IO_SetReadCycleTime( void );
void 			NAND_IO_SetCommCycleTime( void );
void 			NAND_IO_EnableChipSelect( unsigned short nChipNo );
void 			NAND_IO_DisableChipSelect( void );
void 			NAND_IO_WriteBlockPageAddr( unsigned long nBlockPageAddr, NAND_IO_DEVINFO *nDevInfo );
void 			NAND_IO_SetupECC( unsigned short nEccOnOff, unsigned short nEncDec, unsigned short nEccType, unsigned short nAccessType, unsigned long EccBaseAddr );
NAND_IO_ERROR_T	NAND_IO_EncodeECC( unsigned short nEccType, void *pvEccDataBuffer, unsigned int uiEccDataSize );
NAND_IO_ERROR_T NAND_IO_EncodeBootBinary( NAND_IO_DEVINFO *nDevInfo, unsigned char *nPageBuffer, int nEccOnOff );
NAND_IO_ERROR_T NAND_IO_EccErrorNum( unsigned char nErrorData, unsigned char nCorrectData, unsigned char *rErrorBitNum );
void 			NAND_IO_SetLastError( NAND_IO_ERROR_T Errcode );
unsigned long	NAND_IO_GetBUSTypeOfDataIO( void );
NAND_IO_ERROR_T	NAND_IO_MakeBootBinary( NAND_IO_DEVINFO *nDevInfo, unsigned char *nPageBuffer );
void 			NAND_IO_SuppressMessage( unsigned int fSuppress );

static void				NAND_IO_SetupDMA( void *pvSrcPhysicalAddress, unsigned uSrcInc, unsigned uSrcMask, void *pvDestPhysicalAddress, unsigned uDstInc, unsigned uDstMask, int nMode, int nDSize );
static NAND_IO_ERROR_T	NAND_IO_CorrectionMLC( NAND_IO_DEVINFO *nDevInfo, unsigned char *nPageBuffer, unsigned char *nSpareBuffer, unsigned short nDataSize );

#if defined(NAND_SUPPORT_NDMA)
static void 			NAND_IO_NDMA_WriteDataAndSpareArea( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer, unsigned int uiTagSize );
#endif


// For removed compile waring. For Android
#if defined(_LINUX_)
extern unsigned int tca_ckc_getfbusctrl(unsigned int clkname);
#endif
//////////////////////////

#if defined(ANDROID_BOOT)
extern void *dma_alloc(unsigned int alignment, size_t size);
#endif

static void NAND_IO_SetTransperMode( unsigned int uiMode )
{
	if( uiMode == NAND_IO_TRANSPER_MCU_MODE )
	{
		sNAND_IO_uiTransperMode = NAND_IO_TRANSPER_MCU_MODE;
	}
	else if( uiMode == NAND_IO_TRANSPER_GDMA_MODE )
	{
		sNAND_IO_uiTransperMode = NAND_IO_TRANSPER_GDMA_MODE;
	}
	else if( uiMode == NAND_IO_TRANSPER_NDMA_MODE )
	{
		sNAND_IO_uiTransperMode = NAND_IO_TRANSPER_NDMA_MODE;
	}
	else
	{
		ND_TRACE("Input Wrong Parameter !\nSet MCU Transper Mode \n");
		sNAND_IO_uiTransperMode = NAND_IO_TRANSPER_MCU_MODE;
	}
}

static unsigned int NAND_IO_GetTransperMode( void )
{
	return sNAND_IO_uiTransperMode;
}

#if defined(_LINUX_) && defined(__USE_NAND_ISR__)

void NAND_IO_IRQ_Init( int mode )
{
	ND_TRACE("! IRQ_Init ! \n");
	memset( &gsNAND_IO_NandIsrInfo, 0x00, sizeof(gsNAND_IO_NandIsrInfo) );
	init_waitqueue_head( &(gsNAND_IO_NandIsrInfo.wait_q) );
	s_fIsNAND_ISR_USED = mode;
}

void NAND_IO_IRQ_Enable( unsigned int fEnable )
{
	if ( fEnable == ENABLE )
	{
		if( s_fIsNAND_ISR_USED == ENABLE )
		{
			//ND_TRACE(" IRQ Enable ! \n");
			s_fIsNAND_IRQ_Enable	= TRUE;
			s_fIsNAND_RB_ISR_USED	= TRUE;
		}

	}
	else
	{
		if( s_fIsNAND_ISR_USED == ENABLE )
		{			
			//ND_TRACE(" IRQ Disable ! \n");
			s_fIsNAND_IRQ_Enable 	=  FALSE;
			s_fIsNAND_RB_ISR_USED	=  FALSE;
		}
	}	
}

void NAND_IO_ISR_Control( unsigned int uiOnOff )
{
	unsigned int	irq = NAND_IRQ_NFC;
	unsigned long	flags;
	
	local_irq_save( flags );
	local_irq_disable();
	if( uiOnOff == ENABLE )
	{
		#if !(defined(TCC801X) || defined(TCC370X) || defined(TCC800X))
		if (irq < 32) 
	    {
	        BITSET(s_pPIC->INTMSK0.nREG,   (1 << irq));
	        BITSET(s_pPIC->CLR0.nREG,      (1 << irq));
			BITSET(s_pPIC->IEN0.nREG, 		(1 << irq));
	    } 
	    else 
	    {
	        BITSET(s_pPIC->INTMSK1.nREG,   (1 << (irq - 32)));
	        BITSET(s_pPIC->CLR1.nREG,      (1 << (irq - 32)));
			BITSET(s_pPIC->IEN1.nREG, 		(1 << (irq - 32)));
	    }
		#endif

	}
	else if( uiOnOff == DISABLE )
	{
		#if !(defined(TCC801X) || defined(TCC370X) || defined(TCC800X))
		if (irq < 32)
	    {
			BITCLR(s_pPIC->IEN0.nREG, (1 << irq));
	        BITCLR(s_pPIC->INTMSK0.nREG, (1 << irq));
	    } 
	    else 
	    {
	  		BITCLR(s_pPIC->IEN1.nREG, (1 << (irq - 32)));
	        BITCLR(s_pPIC->INTMSK1.nREG, (1 << (irq - 32)));
	    }
		#endif
	}
	else
		ND_TRACE("[%s] Wrong Input Value \n", __FUNCTION__ );

	local_irq_restore( flags );
	
}
void NAND_IO_ISR(void)
{
	if( gsNAND_IO_NandIsrInfo.ubIsRun == true )
	{
		/**********************************************/
		/* NDMA MODE */
		/**********************************************/		
		if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_NDMA_MODE )
		{
			if( s_pNFC->NFC_IRQ & NFC_IRQ_READ_INTR )
			{
			 	//ND_TRACE("_R_ [NFC_IRQCFG:0x%08X] [NFC_IRQ:0x%08X]\n", s_pNFC->NFC_IRQCFG, s_pNFC->NFC_IRQ );
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READ_INTR_ENABLE );			
				//BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_READ_INTR | NFC_IRQ_READ_FLAG );
				
			}
			else if( s_pNFC->NFC_IRQ & NFC_IRQ_PROGRAM_INTR ) 
			{
				//ND_TRACE("_W_ [NFC_IRQCFG:0x%08X] [NFC_IRQ:0x%08X]\n", s_pNFC->NFC_IRQCFG, s_pNFC->NFC_IRQ );
				BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_INTR | NFC_IRQ_PROGRAM_FLAG );	
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_PROGRAM_INTR_ENABLE );				
			}
			else if( s_pNFC->NFC_IRQ & NFC_IRQ_READY_INTR )
			{
				//ND_TRACE("_R/B_ [NFC_IRQCFG:0x%08X] [NFC_IRQ:0x%08X]\n", s_pNFC->NFC_IRQCFG, s_pNFC->NFC_IRQ );
				BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_READY_INTR );
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READY_INTR_ENABLE );			
			}
			else
			{			
				ND_TRACE(" Unknowed Interrupt Flag ! \n");			
				ND_TRACE(" [NFC_IRQ:0x%08X] [NFC_IRQCFG:0x%08X] \n", s_pNFC->NFC_IRQ, s_pNFC->NFC_IRQCFG);
			}
			gsNAND_IO_NandIsrInfo.uiState		= NAND_IO_IRQ_STATE_NONE;
			gsNAND_IO_NandIsrInfo.ubIsRun		= FALSE;
			gsNAND_IO_NandIsrInfo.wait_complete = 1;
			
			NAND_IO_ISR_Control(DISABLE);				// Disable Global Interrupt for NFC
			wake_up(&(gsNAND_IO_NandIsrInfo.wait_q));
		}
		else if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_GDMA_MODE )
		{			
			/**********************************************/
			/* DMA MODE */
			/**********************************************/

			if( s_pNFC->NFC_IRQ & NFC_IRQ_READ_INTR )
			{
				BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_READ_INTR);
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READ_INTR_ENABLE );	// [IRQ]Set Read Interrput Disable

				if(gsNAND_IO_NandIsrInfo.uiState!=NAND_IO_IRQ_STATE_READ)
				{
					ND_TRACE("wrong state with read irq. (state=%u)\n",gsNAND_IO_NandIsrInfo.uiState);
					return;
				}

				if(gsNAND_IO_NandIsrInfo.usPPagesLeft==0)
				{
					ND_TRACE("no pages left to read\n");
					return;
				}			

				/* Check and Correct ECC code */
				if ( gsNAND_IO_NandIsrInfo.iEccOnOff == ECC_ON )
				{
					gsNAND_IO_NandIsrInfo.error |= NAND_IO_CorrectionMLC( gsNAND_IO_NandIsrInfo.pNandIoDevInfo, gsNAND_IO_NandIsrInfo.pbPageBuffer, gsNAND_IO_NandIsrInfo.pbEccBuffer, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize);
					gsNAND_IO_NandIsrInfo.pbEccBuffer = &gsNAND_IO_NandIsrInfo.pbEccBuffer[gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccCodeSize];
				}

				gsNAND_IO_NandIsrInfo.pbPageBuffer		= &gsNAND_IO_NandIsrInfo.pbPageBuffer[gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize];
				gsNAND_IO_NandIsrInfo.pbPhyPageBuffer	= &gsNAND_IO_NandIsrInfo.pbPhyPageBuffer[gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize];
				gsNAND_IO_NandIsrInfo.usCurrentPPage++;			
				gsNAND_IO_NandIsrInfo.usPPagesLeft--;

				/* Fixed code */
				if( gsNAND_IO_NandIsrInfo.usPPagesLeft > 0) 
				{
					if ( gsNAND_IO_NandIsrInfo.iEccOnOff == ECC_ON )
					{
						/* Setup ECC Block */
						#if defined(_WINCE_) || defined(_LINUX_)
						NAND_IO_SetupECC( ECC_ON, ECC_DECODE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (unsigned long)&NAND_IO_HwLDATA_PA );
						#else
						NAND_IO_SetupECC( ECC_ON, ECC_DECODE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (unsigned long)&pNFC->NFC_LDATA );
						#endif
						s_pECC->ECC_CTRL	|= ( gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize<< ECC_SHIFT_DATASIZE );
						s_pECC->ECC_CLEAR	= 0x00000000;
					}

					NAND_IO_SetupDMA( ( void * )&NAND_IO_HwLDATA_PA, 0, 0,
				             		 ( void * )gsNAND_IO_NandIsrInfo.pbPhyPageBuffer, 4, 0,
				              		NAND_IO_DMA_READ, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize );
				}
				else
				{
					gsNAND_IO_NandIsrInfo.wait_complete = 1;
					NAND_IO_ISR_Control(DISABLE);				// Disable Global Interrupt for NFC
				    wake_up(&(gsNAND_IO_NandIsrInfo.wait_q));
				}
			}		
			else if( s_pNFC->NFC_IRQ & NFC_IRQ_PROGRAM_INTR ) 
			{
				BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_INTR );
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_PROGRAM_INTR_ENABLE );	// [IRQ]Set Write Interrput Disable
				
				if(gsNAND_IO_NandIsrInfo.uiState!=NAND_IO_IRQ_STATE_WRITE)
				{
					ND_TRACE("wrong state with write irq. (state=%u)\n",gsNAND_IO_NandIsrInfo.uiState);
					return;
				}

				
				NAND_IO_EncodeECC( gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, gsNAND_IO_NandIsrInfo.pbEccBuffer, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccCodeSize );
				if(gsNAND_IO_NandIsrInfo.usPPagesLeft)
				{
					gsNAND_IO_NandIsrInfo.pbEccBuffer 		= &gsNAND_IO_NandIsrInfo.pbEccBuffer[gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccCodeSize];
					gsNAND_IO_NandIsrInfo.pbPhyPageBuffer	= &gsNAND_IO_NandIsrInfo.pbPhyPageBuffer[gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize];
					
					gsNAND_IO_NandIsrInfo.usPPagesLeft--;

					if ( gsNAND_IO_NandIsrInfo.iEccOnOff == ECC_ON )
					{
						/* Setup ECC Block */
						#if defined(_WINCE_) || defined(_LINUX_)
						NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (unsigned long)&NAND_IO_HwLDATA_PA );
						#else
						NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, (unsigned long)&pNFC->NFC_LDATA );
						#endif
						s_pECC->ECC_CTRL	|= ( gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize<< ECC_SHIFT_DATASIZE );
						s_pECC->ECC_CLEAR	= 0x00000000;
					}

					NAND_IO_SetupDMA( ( void * )gsNAND_IO_NandIsrInfo.pbPhyPageBuffer, 4, 0,
				             		  ( void * )&NAND_IO_HwLDATA_PA, 0, 0,
				              		  NAND_IO_DMA_WRITE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize );
				}
				else
				{	
						
						gsNAND_IO_NandIsrInfo.wait_complete = 1;
						NAND_IO_ISR_Control(DISABLE);				// Disable Global Interrupt for NFC
					    wake_up(&(gsNAND_IO_NandIsrInfo.wait_q));
				}
			}
			else if( s_pNFC->NFC_IRQ & NFC_IRQ_READY_INTR )
			{
				//ND_TRACE("_R/B_ [NFC_IRQCFG:0x%08X] [NFC_IRQ:0x%08X]\n", s_pNFC->NFC_IRQCFG, s_pNFC->NFC_IRQ );
				BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_READY_INTR );
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READY_INTR_ENABLE );

				gsNAND_IO_NandIsrInfo.uiState		= NAND_IO_IRQ_STATE_NONE;
				gsNAND_IO_NandIsrInfo.ubIsRun		= FALSE;
				gsNAND_IO_NandIsrInfo.wait_complete = 1;
				NAND_IO_ISR_Control(DISABLE);				// Disable Global Interrupt for NFC
				wake_up(&(gsNAND_IO_NandIsrInfo.wait_q));
			}
			else
			{			
				ND_TRACE(" Unknowed Interrupt Flag ! \n");			
				ND_TRACE(" [NFC_IRQ:0x%08X] [NFC_IRQCFG:0x%08X] \n", s_pNFC->NFC_IRQ, s_pNFC->NFC_IRQCFG);
			}	
		}
	}
	else
	{
		ND_TRACE("\n Unexpected  Interrupt Routine \n" );
		ND_TRACE("[NFC_IRQCFG:0x%08X] [NFC_IRQ:0x%08X] \n", s_pNFC->NFC_IRQCFG, s_pNFC->NFC_IRQ );
		BITCLR( s_pNFC->NFC_IRQCFG , s_pNFC->NFC_IRQCFG );
		BITSET( s_pNFC->NFC_IRQ , s_pNFC->NFC_IRQ );		
	}

}

void NAND_IO_RB_ISR( NAND_IO_DEVINFO *nDevInfo, unsigned int mode )
{
	int ret;
	unsigned int uiCommand = 0;

	#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
	gsNAND_IO_NandIsrInfo.uiState		= NAND_IO_IRQ_STATE_RB;
	gsNAND_IO_NandIsrInfo.ubIsRun		= TRUE;
	gsNAND_IO_NandIsrInfo.wait_complete = 0;
	
	BITSET(s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READY_INTR_ENABLE );
	
	switch( mode )
	{
		case NAND_IO_WRITE_NORMAL_MODE :
			uiCommand = 0x1010;
			break;

		case NAND_IO_WRITE_CACHE_MODE :
			uiCommand = 0x1515;
			break;	

		case NAND_IO_ERASE_MODE :
			uiCommand = 0xD0D0;
			break;			
	}

	//ND_TRACE(" R/B Interrupt Sleep ! \n");
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;

	NAND_IO_ISR_Control( ENABLE );	// Enable Global Interrupt for NFC
	do
	{
		ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );	

		if( ret > 0)
		{
			//ND_TRACE("R/B Interrupt Wake Up ! \n");
			break;
		}
		else
		{
			if( s_pNFC->NFC_STATUS & NFC_STATUS_READY_FLAG )	
			{
				ND_DEBUG(" Ready ISR Time OUT ~!\n");
				BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_READY_INTR );
				BITCLR( s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READY_INTR_ENABLE );
				break;
			}
			else
			{
				//ND_TRACE("[NAND ISR    ]wait_event_interruptible error!(ret=%d)\n",ret);
			}			
		}
	} while(1);

	#endif
}

static  unsigned int NAND_IO_IRQ_IsEnabled( void )
{
	return s_fIsNAND_IRQ_Enable;
}

void* NAND_IO_GetISRStructure( void )
{
	return &gsNAND_IO_NandIsrInfo;
}

#endif

static unsigned int NAND_IO_IsAllValue( void *pvSrc, unsigned char ucValue, unsigned int uiLength )
{
	unsigned char *pucSrcValue = ( unsigned char * )pvSrc;
	while( uiLength-- )
	{
		if( *pucSrcValue++ != ucValue )
			return FALSE;
	}

	return TRUE;
}

void NAND_IO_SuppressMessage( unsigned int fSuppress )
{
	if( fSuppress )
		sNAND_IO_fSuppressMessage = TRUE;
	else
		sNAND_IO_fSuppressMessage = FALSE;
}

#if defined(NFC_VER_300)
/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static unsigned int NAND_IO_GetECCMode( unsigned short nEncDec, unsigned short nEccType )
{
	unsigned int uiECCMode = 0;
	if( nEncDec == ECC_DECODE )
	{		
		if( nEccType == ECC_TYPE_4BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC4_DECODE;
		else if( nEccType == ECC_TYPE_8BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC8_DECODE;
		else if( nEccType == ECC_TYPE_12BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC12_DECODE;
		else if( nEccType == ECC_TYPE_16BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC16_DECODE;
		else if( nEccType == ECC_TYPE_24BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC24_DECODE;
		else if( nEccType == ECC_TYPE_40BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC40_DECODE;
		else if( nEccType == ECC_TYPE_60BIT )
			uiECCMode		= 	ECC_CTRL_ECC_EN_MLC_ECC60_DECODE;
		else
			NAND_IO_ASSERT( 0 );
	}
	else if( nEncDec == ECC_ENCODE )
	{
		if( nEccType == ECC_TYPE_4BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC4_ENCODE;
		else if( nEccType == ECC_TYPE_8BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC8_ENCODE;
		else if( nEccType == ECC_TYPE_12BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC12_ENCODE;
		else if( nEccType == ECC_TYPE_16BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC16_ENCODE;
		else if( nEccType == ECC_TYPE_24BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC24_ENCODE;
		else if( nEccType == ECC_TYPE_40BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC40_ENCODE;
		else if( nEccType == ECC_TYPE_60BIT )
			uiECCMode	= 	ECC_CTRL_ECC_EN_MLC_ECC60_ENCODE;
		else
			NAND_IO_ASSERT( 0 );
	}

	return uiECCMode;
}
#endif

static const NAND_IO_ECC_INFO *NAND_IO_GetECCInfo( unsigned short usEccType )
{
	if( usEccType == ECC_TYPE_4BIT )
		return &s_MLC_ECC_4Bit;
#if defined(NFC_VER_50) || defined(NFC_VER_100) || defined(NFC_VER_300)
	else if( usEccType == ECC_TYPE_8BIT )
		return &s_MLC_ECC_8Bit;
#endif
	else if( usEccType == ECC_TYPE_12BIT )
		return &s_MLC_ECC_12Bit;
#if defined(NFC_VER_100) || defined(NFC_VER_200)|| defined(NFC_VER_300)
	else if( usEccType == ECC_TYPE_16BIT )
		return &s_MLC_ECC_16Bit;
#endif
#if defined(NFC_VER_200)|| defined(NFC_VER_300)
	else if( usEccType == ECC_TYPE_24BIT )
		return &s_MLC_ECC_24Bit;
#endif
#if defined(NFC_VER_300)
	else if( usEccType == ECC_TYPE_40BIT )		
		return &s_MLC_ECC_40Bit;
	else if( usEccType == ECC_TYPE_60BIT )
		return &s_MLC_ECC_60Bit;
#endif

	return NULL;
}

static void NAND_IO_CHANGE_SPARE_ECC_TYPE(NAND_IO_DEVINFO *nDevInfo, unsigned int uiEccMode)
{
	const NAND_IO_ECC_INFO *pECCInfo;
	
	switch( uiEccMode )
	{
		case A_ECC_SPARE_16BIT: // change ECC to 16bit
			nDevInfo->EccType = ECC_TYPE_16BIT;
			break;
		case A_ECC_SPARE_24BIT: // change ECC to 24bit
			nDevInfo->EccType = ECC_TYPE_24BIT;
			break;			
		default :
			nDevInfo->EccType = GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType );
			break;
	}
	#if defined(NFC_VER_200)
	if(nDevInfo->EccType == ECC_TYPE_8BIT) //NFC_VER_200 doesn't support 8bit ECC
		nDevInfo->EccType = ECC_TYPE_12BIT;
	#endif
	
	pECCInfo = NAND_IO_GetECCInfo( nDevInfo->EccType );
	if( pECCInfo == NULL )
	{
		ND_TRACE( "Unsupported ECC_TYPE(%x)\n", nDevInfo->EccType );
		return;
	}
	nDevInfo->EccCodeSize = pECCInfo->ucEccCodeSize;
}

static unsigned long NAND_IO_CheckReadyAndBusyFlag( void )
{
	#if 0
	// Check Read Busy Flag
	return ISZERO(s_pNFC->NFC_IRQ, NFC_IRQ_READY_FLAG);	
	#else
	// Check Ready Busy GPIO Status
	#if defined(NFC_VER_50)
	return ISZERO( s_pNFC->NFC_CTRL, NFC_CTRL_RDY_RDY );
	#else
	return ISZERO( s_pNFC->NFC_STATUS, NFC_STATUS_READY_FLAG );
	#endif
	#endif
}

static unsigned int NAND_IO_CheckBootPage( unsigned char *pucTag )
{
	unsigned int res;

	{
		if( pucTag[3] == 0x10 ) // 0x10 is Boot Tag Signature.
			res = TRUE;
		else 
			res = FALSE;
	}

	return res;
}

static void NAND_IO_WaitBusy( unsigned short nChipNo )
{
	NAND_Util_Delay();NAND_Util_Delay();
	
	while ( NAND_IO_CheckReadyAndBusyFlag() );
}

static unsigned long NAND_IO_CheckReadyAndBusyStatus( void )
{
	#if defined(NFC_VER_50)
	return ISZERO( s_pNFC->NFC_CTRL, NFC_CTRL_RDY_RDY );
	#else
	return ISZERO( s_pNFC->NFC_STATUS, NFC_STATUS_READY_FLAG );
	#endif
}

static void NAND_IO_WaitBusyStatus( unsigned short nChipNo )
{
	while ( NAND_IO_CheckReadyAndBusyStatus() );
}

static void NAND_IO_WaitBusyCheckForWriteEnd( NAND_IO_DEVINFO *nDevInfo )
{
	NAND_Util_Delay();
	NAND_IO_PreProcess();

	NAND_BD_Enable_WP_Port();
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();
}

static void NAND_IO_WriteRowColAddr( unsigned int nRowAddr, unsigned int nColumnAddr, NAND_IO_DEVINFO *nDevInfo )
{
	unsigned int		nTempAddr;
	unsigned char		i;

	//==================================================
	// Write Column Address
	//==================================================
	for( i = 0; i < nDevInfo->Feature.ucColCycle; ++i )
	{
		nTempAddr = nDevInfo->CmdMask & ( ( ( nColumnAddr << 8 ) & 0xFF00 ) | ( nColumnAddr & 0x00FF ) );
		s_pNFC->NFC_SADDR = nTempAddr;

		nColumnAddr = nColumnAddr >> 8;
	}

	//==================================================
	// Write Row Address
	//==================================================
	nRowAddr = nRowAddr;

	for( i = 0; i < nDevInfo->Feature.ucRowCycle; ++i )
	{
		nTempAddr = nDevInfo->CmdMask & ( ( ( nRowAddr << 8 ) & 0xFF00 ) | ( nRowAddr & 0x00FF ) );
		s_pNFC->NFC_SADDR = nTempAddr;

		nRowAddr = nRowAddr >> 8;
	}
}

static void NAND_IO_WriteColAddr( unsigned long nColumnAddr, NAND_IO_DEVINFO *nDevInfo )
{
	unsigned char		i;

	//==================================================
	// Write Column Address
	//==================================================
	for( i = 0; i < nDevInfo->Feature.ucColCycle; ++i )
	{
		s_pNFC->NFC_SADDR	= nDevInfo->CmdMask & ( ( ( nColumnAddr << 8 ) & 0xFF00 ) | ( nColumnAddr & 0x00FF ) );
		nColumnAddr = nColumnAddr >> 8;
	}
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_SetCycle( NAND_IO_DEVINFO *nDevInfo, unsigned int uiBusClkMHz )
{
	unsigned int nDevSTP, nDevPW, nDevHLD;
	unsigned int nReGateDelay, nWrGateDelay;
	unsigned int uiSurplusDelay = 5;
	unsigned char ucPulseWidthTime;
	unsigned char ucCaptureDelay;
	//===================================
	// Set Cycle
	//===================================

	/* Basis Setting */
	nReGateDelay	= 0;
	nWrGateDelay	= 0;

	ND_DEBUG("NAND_IO_SetCycle: uiBusClkMHz = %d MHz\n",uiBusClkMHz);

	if( uiBusClkMHz <= 120 )
		uiSurplusDelay = 15;

	// Read Cycle
	nDevSTP					= 0;
	ucPulseWidthTime = nDevInfo->Feature.ucReadCycleTime - nDevInfo->Feature.ucReadHoldTime;
	if( nDevInfo->Feature.ucReadPulseWidthTime > ucPulseWidthTime )
	{
		ND_DEBUG("ucReadPulseWidthTime is choosed.\n");
		ucPulseWidthTime = nDevInfo->Feature.ucReadPulseWidthTime;
	}
	if(nDevInfo->Feature.UsableFunc & F_EDO)
	{
		ucPulseWidthTime += 10; 
		ND_DEBUG("EDO Support!");	
	}
	else
	{
		if( (nDevInfo->Feature.ucReadAccessTime + 5 ) >= ucPulseWidthTime )
		{
			ucPulseWidthTime = nDevInfo->Feature.ucReadAccessTime + 5;
			ND_DEBUG("(ucReadAccessTime + 5) is choosed.\n");
		}
		else
		{
			ND_DEBUG("(ucReadCycleTime - ucReadHoldTime) is choosed.\n");
		}
	}

	nDevPW					= ucPulseWidthTime + uiSurplusDelay + nReGateDelay;
	nDevHLD					= nDevInfo->Feature.ucReadHoldTime + uiSurplusDelay;

	s_ReadCycleTime.STP		= ( unsigned char )( ( ( nDevSTP * uiBusClkMHz ) + 999 ) / 1000 );
	s_ReadCycleTime.PW		= ( unsigned char )( ( ( nDevPW * uiBusClkMHz ) + 999 ) / 1000 );
	s_ReadCycleTime.HLD		= ( unsigned char )( ( ( nDevHLD * uiBusClkMHz ) + 999 ) / 1000 );

	if (nDevInfo->Feature.UsableFunc & F_EDO)
	{
		ND_DEBUG("EDO Support! RDNDLY is same as HLD time \n");		
		ucCaptureDelay = s_ReadCycleTime.HLD;
	}
	else
	{
		ucCaptureDelay = ( unsigned char )( ( ( 5/*Capture Delay(ns)*/ * uiBusClkMHz ) + 999 ) / 1000 );
	}

	if (ucCaptureDelay > 7)
	{
		ND_TRACE("Warning !!! ucCaptureDelay is %d", ucCaptureDelay);
		ucCaptureDelay = 7;
	}

	#if defined(NFC_VER_200) || defined(NFC_VER_300)
	BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_READ_DATA_CAPTURE_DELAY_FIELD, NFC_CTRL_READ_DATA_CAPTURE_DELAY(ucCaptureDelay) );
	ND_DEBUG("NAND_IO_SetCycle: RDNDLY = %d, STP = %d, PW = %d, HLD = %d \n",ucCaptureDelay, s_ReadCycleTime.STP, s_ReadCycleTime.PW, s_ReadCycleTime.HLD);
	#endif

	// Write Cycle
	nDevSTP					= 0;
	ucPulseWidthTime = nDevInfo->Feature.ucWriteCycleTime - nDevInfo->Feature.ucWriteHoldTime;
	if( nDevInfo->Feature.ucWritePulseWidthTime > ucPulseWidthTime )
	{
		ucPulseWidthTime = nDevInfo->Feature.ucWritePulseWidthTime;
		ND_DEBUG("ucWritePulseWidthTime is choosed.\n");
	}
	else
	{
		ND_DEBUG("(ucWriteCycleTime - ucWriteHoldTime) is choosed.\n");
	}
	nDevPW					= ucPulseWidthTime + uiSurplusDelay + nWrGateDelay;
	nDevHLD					= nDevInfo->Feature.ucWriteHoldTime + uiSurplusDelay;

	s_WriteCycleTime.STP	= ( unsigned char )( ( ( nDevSTP * uiBusClkMHz ) + 999 ) / 1000 );
	s_WriteCycleTime.PW		= ( unsigned char )( ( ( nDevPW * uiBusClkMHz ) + 999 ) / 1000 );
	s_WriteCycleTime.HLD	= ( unsigned char )( ( ( nDevHLD * uiBusClkMHz ) + 999 ) / 1000 );

	#if defined(NAND_FOR_KERNEL) // #H27UBG8T2B read fail issue - when ioBusClk is low as 96MHz on TCC8920 ICS
	if(s_WriteCycleTime.HLD < 2)
	{
		s_WriteCycleTime.HLD = 2;
	}
	if(s_WriteCycleTime.PW < 2)
	{
		s_WriteCycleTime.PW = 2;
	}
	#endif
	
	// Command Cycle
	nDevSTP					= 1;
	//nDevPW					= 80 + nWrGateDelay;
	//nDevHLD					= 40;

	s_CommCycleTime.STP		= ( unsigned char )( ( ( nDevSTP * uiBusClkMHz ) + 999 ) / 1000 );
	s_CommCycleTime.PW		= s_WriteCycleTime.PW;
	s_CommCycleTime.HLD		= s_WriteCycleTime.HLD;
	{
		// tCLH/tALH is s_CommCycleTime.HLD - 1
		unsigned char ucCLE_ALE_HoldTimeInTick = ( unsigned char )( ( ( 5/*(ns)*/ * uiBusClkMHz ) + 999 ) / 1000 );
		if( (unsigned char)( ucCLE_ALE_HoldTimeInTick + 1 ) > s_CommCycleTime.HLD )
		{
			ND_DEBUG( "s_CommCycleTime.HLD is updated.  %d -> %d\n", s_CommCycleTime.HLD, ucCLE_ALE_HoldTimeInTick + 1 );
			s_CommCycleTime.HLD = ucCLE_ALE_HoldTimeInTick + 1;
		}
	}

	if( s_WriteCycleTime.STP >= 16 )
		s_WriteCycleTime.STP = 15;
	if( s_WriteCycleTime.PW >= 16 )
		s_WriteCycleTime.PW = 15;
	if( s_WriteCycleTime.HLD >= 16 )
		s_WriteCycleTime.HLD = 15;

	if( s_ReadCycleTime.STP >= 16 )
		s_ReadCycleTime.STP = 15;
	if( s_ReadCycleTime.PW >= 16 )
		s_ReadCycleTime.PW = 15;
	if( s_ReadCycleTime.HLD >= 16 )
		s_ReadCycleTime.HLD = 15;

	if( s_CommCycleTime.STP >= 16 )
		s_CommCycleTime.STP = 15;
	if( s_CommCycleTime.PW >= 16 )
		s_CommCycleTime.PW = 15;
	if( s_CommCycleTime.HLD >= 16 )
		s_CommCycleTime.HLD = 15;

	if( sNAND_IO_DataBusType == NAND_IO_NFC_BUS )
	{
#if defined(NFC_VER_50) || defined(NFC_VER_100)
		s_WriteCycleTime.RegValue	= ( s_WriteCycleTime.STP << 8 ) + ( s_WriteCycleTime.PW << 4 ) + s_WriteCycleTime.HLD;
		s_ReadCycleTime.RegValue	= ( s_ReadCycleTime.STP << 8 ) + ( s_ReadCycleTime.PW << 4 ) + s_ReadCycleTime.HLD;
		s_CommCycleTime.RegValue	= ( s_CommCycleTime.STP << 8 ) + ( s_CommCycleTime.PW << 4 ) + s_CommCycleTime.HLD;
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		s_WriteCycleTime.RegValue	= ( s_WriteCycleTime.STP << 16 ) + ( s_WriteCycleTime.PW << 8 ) + s_WriteCycleTime.HLD;
		s_ReadCycleTime.RegValue	= ( s_ReadCycleTime.STP << 16 ) + ( s_ReadCycleTime.PW << 8 ) + s_ReadCycleTime.HLD;
		s_CommCycleTime.RegValue	= ( s_CommCycleTime.STP << 16 ) + ( s_CommCycleTime.PW << 8 ) + s_CommCycleTime.HLD;
#endif
	}
	else if( sNAND_IO_DataBusType == NAND_IO_MEM_BUS )
	{
#if defined(TCC860x)
		s_WriteCycleTime.RegValue	= ( s_WriteCycleTime.STP << 6 ) + ( s_WriteCycleTime.PW << 3 ) + s_WriteCycleTime.HLD;
		s_ReadCycleTime.RegValue	= ( s_ReadCycleTime.STP << 6 ) + ( s_ReadCycleTime.PW << 3 ) + s_ReadCycleTime.HLD;
		s_CommCycleTime.RegValue	= ( s_CommCycleTime.STP << 6 ) + ( s_CommCycleTime.PW << 3 ) + s_CommCycleTime.HLD;
#else
		s_WriteCycleTime.RegValue	= ( s_WriteCycleTime.STP << 11 ) + ( ( s_WriteCycleTime.PW - 1 ) << 3 ) + s_WriteCycleTime.HLD;
		s_ReadCycleTime.RegValue	= ( s_ReadCycleTime.STP << 11 ) + ( ( s_ReadCycleTime.PW - 1 ) << 3 ) + s_ReadCycleTime.HLD;
		s_CommCycleTime.RegValue	= ( s_CommCycleTime.STP << 11 ) + ( ( s_CommCycleTime.PW - 1 ) << 3 ) + s_CommCycleTime.HLD;
#endif
	}

	ND_DEBUG("CommCycleTime STP=%d PW=%d HLD=%d\n",s_CommCycleTime.STP,s_CommCycleTime.PW,s_CommCycleTime.HLD);
	ND_DEBUG("ReadCycleTime STP=%d PW=%d HLD=%d\n",s_ReadCycleTime.STP,s_ReadCycleTime.PW,s_ReadCycleTime.HLD);
	ND_DEBUG("WriteCycleTime STP=%d PW=%d HLD=%d\n",s_WriteCycleTime.STP,s_WriteCycleTime.PW,s_WriteCycleTime.HLD);

	sNAND_IO_uiMaxBusClkMHz = uiBusClkMHz;

	return NAND_IO_SUCCESS;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_InitDMABuffer( NAND_IO_DEVINFO *nDevInfo )
{
#if defined(_WINCE_)
	#if defined(TCC893X)
	tSYSTEM_PARAM	*pSYS_Work_PARAM	= ( tSYSTEM_PARAM * )( SYSTEMPARAM_ADDR );
	#else
	tSYSTEM_PARAM	*pSYS_Work_PARAM	= ( tSYSTEM_PARAM * )( SYSTEM_PARAM_BASEADDRESS );
	#endif
#endif

#if defined(_WINCE_)
	s_puiDMA_PhyBuffer = ( unsigned int * )pSYS_Work_PARAM->BUFFERAREA_NAND;				// Working Address
	s_puiDMA_WorkBuffer = ( unsigned int * )pSYS_PARAM->BUFFERAREA_NAND;					// Physical Address
#elif defined(_LINUX_)
#if defined(NAND_FOR_KERNEL)
	s_puiDMA_PhyBuffer = ( unsigned int * )dma_t.dma_addr;
	s_puiDMA_WorkBuffer = ( unsigned int * )dma_t.v_addr;
#else
#if defined(ANDROID_BOOT)
	if( dma_addr == NULL )
	{
		dma_addr = ( unsigned int * )dma_alloc( 2048, PAGE_BYTESIZE_MAX );
	}
	s_puiDMA_PhyBuffer 	= ( unsigned int * )dma_addr;
	s_puiDMA_WorkBuffer	= ( unsigned int * )dma_addr;
#else
	s_puiDMA_PhyBuffer 	= ( unsigned int * )DMA_ADDR;
	s_puiDMA_WorkBuffer	= ( unsigned int * )DMA_ADDR;
#endif
#endif
#else
	if( s_puiDMA_PhyBuffer == NULL )
	{
		s_puiDMA_PhyBuffer 	= ( unsigned int * )TC_Allocate_NC_Aligned_Memory( PAGE_BYTESIZE_MAX, 4);
	}
	s_puiDMA_WorkBuffer	= s_puiDMA_PhyBuffer;
#endif

	if( s_puiDMA_PhyBuffer == NULL || s_puiDMA_WorkBuffer == NULL )
	{
		//ND_TRACE( "DMA Buffer Pointer is null!!! s_puiDMA_PhyBuffer=0x%x, s_puiDMA_WorkBuffer=%x\n" , s_puiDMA_PhyBuffer, s_puiDMA_WorkBuffer );
	}

#if defined(NAND_IO_DMA_ADDR_LOG)
	ND_TRACE( "DMA_PhyBufferAddr0:0x%x\n", s_puiDMA_PhyBuffer );
	ND_TRACE( "DMA_WorkBufferAddr0:0x%x\n", s_puiDMA_WorkBuffer );
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_Init( void )
{
	unsigned int		i;
	NAND_IO_DEVINFO		s_pDevInfo;

	memset( &s_pDevInfo, 0x00, sizeof( NAND_IO_DEVINFO ) );

#if defined(NAND_FOR_KERNEL)
	#if defined(_LINUX_)
		#if defined(NFC_VER_50)
			s_pNFC 		= ( PNFC_V05_T )tcc_p2v( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V05_T )tcc_p2v( ECC_BASE_ADDRESS );
		#elif defined(NFC_VER_100)
			s_pNFC 		= ( PNFC_V1_T )tcc_p2v( NFC_BASE_ADDRESS );
			s_pECC 		= ( PNFC_V1_T )tcc_p2v( ECC_BASE_ADDRESS );
			pIOBUSCFG_T = ( PIOBUSCFG )tcc_p2v( IOBUSCFG_BASE_ADDRESS );
		#elif defined(NFC_VER_200)
			s_pNFC 		= ( PNFC_V2_T )tcc_p2v( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V2_T )tcc_p2v( ECC_BASE_ADDRESS );
		#elif defined(NFC_VER_300)
			s_pNFC 		= ( PNFC_V3_T )tcc_p2v( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V3_T )tcc_p2v( ECC_BASE_ADDRESS );
		#endif
		pNAND_DMA	= ( NAND_PGDMANCTRL )tcc_p2v( NAND_DMA_BASE_ADDRESS );
		#if !(defined(TCC801X) || defined(TCC370X) || defined(TCC800X))
		s_pPIC		= ( PPIC )tcc_p2v( PIC_BASE_ADDRESS );
		#endif
	#elif defined(_WINCE_)
		#if defined(NFC_VER_100)
			s_pNFC 		= ( PNFC_V1_T )tcc_allocbaseaddress( ( unsigned int )NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V1_T )tcc_allocbaseaddress( ( unsigned int )ECC_BASE_ADDRESS );
			pIOBUSCFG_T = ( PIOBUSCFG )tcc_allocbaseaddress( ( unsigned int )IOBUSCFG_BASE_ADDRESS );
		#elif defined(NFC_VER_200)
			s_pNFC 		= ( PNFC_V2_T )tcc_allocbaseaddress( ( unsigned int )NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V2_T )tcc_allocbaseaddress( ( unsigned int )ECC_BASE_ADDRESS );
		#elif defined(NFC_VER_300)
			s_pNFC 		= ( PNFC_V3_T )tcc_allocbaseaddress( ( unsigned int )NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V3_T )tcc_allocbaseaddress( ( unsigned int )ECC_BASE_ADDRESS );
		#endif
		#if !(defined(TCC801X) || defined(TCC370X) || defined(TCC800X))
			s_pPIC		= ( PPIC )tcc_allocbaseaddress( ( unsigned int )PIC_BASE_ADDRESS );		
		#endif	
			#if defined(TCC893X)
			pSYS_PARAM	= ( tSYSTEM_PARAM * )tcc_allocbaseaddress( ( unsigned int )SYSTEMPARAM_ADDR );
			#else
			pSYS_PARAM	= ( tSYSTEM_PARAM * )tcc_allocbaseaddress( ( unsigned int )SYSTEM_PARAM_BASEADDRESS );
			#endif
			pNAND_DMA	= ( PGDMANCTRL )tcc_allocbaseaddress( ( unsigned int )NAND_DMA_BASE_ADDRESS );
	#endif
#else
		#if defined(NFC_VER_50)
			s_pNFC 		= ( PNFC_V05_T )( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V05_T )( ECC_BASE_ADDRESS );
		#elif defined(NFC_VER_100)
			s_pNFC 		= ( PNFC_V1_T )( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V1_T )( ECC_BASE_ADDRESS );
			pIOBUSCFG_T = ( PIOBUSCFG )( IOBUSCFG_BASE_ADDRESS );
		#elif defined(NFC_VER_200)
			s_pNFC 		= ( PNFC_V2_T )( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V2_T )( ECC_BASE_ADDRESS );
		#elif defined(NFC_VER_300)
			s_pNFC 		= ( PNFC_V3_T )( NFC_BASE_ADDRESS );
			s_pECC 		= ( PECC_V3_T )( ECC_BASE_ADDRESS );
		#endif

	    #if !defined(TCC801X) && !defined(TCC370X) && !defined(TCC800X) 
		s_pPIC		= ( PPIC )( PIC_BASE_ADDRESS );
		#endif

		#if defined(_WINCE_)
			#if defined(TCC893X)
			pSYS_PARAM	= ( tSYSTEM_PARAM * )( SYSTEMPARAM_ADDR );
		#else
			pSYS_PARAM	= ( tSYSTEM_PARAM * )( SYSTEM_PARAM_BASEADDRESS );
			#endif
			pNAND_DMA	= ( PGDMANCTRL )( NAND_DMA_BASE_ADDRESS );
		#elif defined(_LINUX_)
			pNAND_DMA	= ( NAND_PGDMANCTRL )( NAND_DMA_BASE_ADDRESS );
		#endif
#endif

    #if defined(TCC801X)
	BITSET( HwBCLKCTR, HwBCLKCTR_ND );
	    BITSET( HwBCLKCTR1, HwBCLKCTR1_SCFG );
		#if defined(NFC_XD_HIGH_16BIT)
		// Caution : Data bus width setting must be 8 or 16-bit. (not allowed 32-bit)
		HwXMCCFG    |= ( Hw10 | Hw2 | Hw1 ); 
		#else
		HwXMCCFG    |= Hw1; 
		HwXMCCFG    &= ~Hw2; 		
		#endif
		#endif

	#if defined(TCC370X) || defined(TCC800X)
	#if defined(TCC800X)
	NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_NFC | NAND_IO_CKC_BUS_ECC );
	#else
	BITSET( NAND_BUS_CLOCK_CTRL, HwBCLKCTR_ND );
	#endif
    BITSET( NAND_BUS_CLOCK_CTRL1, HwBCLKCTR1_SCFG );
	#if defined(NFC_XD_HIGH_16BIT)
	// Caution : Data bus width setting must be 8 or 16-bit. (not allowed 32-bit)
	NAND_EXT_MEM_CONFIG_REG    |= ( Hw10 | Hw2 | Hw1 ); 
	#else
	NAND_EXT_MEM_CONFIG_REG    |= Hw1; 
	NAND_EXT_MEM_CONFIG_REG    &= ~Hw2; 		
	#endif
	#endif
		
#ifdef TCC93XX
	tca_ckc_setiobus( RB_GDMA2CONTROLLER, ENABLE );
#endif

	/*************************************/
	/*Don't remove NAND_Util_Delay Function*/
	/*************************************/
	for( i = 0; i < 5000; ++i )
		NAND_Util_Delay();

	NAND_BD_Init( &sNAND_IO_uiPortStatus );

	sNAND_IO_DataBusType	= NAND_IO_NFC_BUS;
	sNAND_IO_fSuppressMessage = FALSE;

	//----------------------------------------
	// NFC RESET
	//----------------------------------------
	s_pNFC->NFC_RST = 0;

	//---------------------------------------------
	// SET NFC CONFIGURATION
	//---------------------------------------------
#if defined(NFC_VER_50)
	s_pNFC->NFC_CTRL  = 0
						| NFC_CTRL_BURST_SIZE_1;

	s_pNFC->NFC_CTRL1 = 0
						| NFC_CTRL1_MDATA_COUNT(3);

#elif defined(NFC_VER_100)
	s_pNFC->NFC_CTRL	= HwNFC_CTRL_DEN_EN
	                      | HwNFC_CTRL_CFG_NOACT
	                      | HwNFC_CTRL_BSIZE_1
	                      | ( 4 << 4 )
	                      | ( 1 << 0 );

	s_pNFC->NFC_CTRL1 |= HwNFC_CTRL1_DACK_EN;
	s_pNFC->NFC_CTRL1 |= HwNFC_CTRL1_ARB_EN;

	BITSET( s_pPIC->CLR1,		HwINT1_NFC );
	BITSET( s_pPIC->SEL1,	  	HwINT1_NFC );		// Set NFC as IRQ interrupt
	BITSET( s_pPIC->MODE1, 	HwINT1_NFC );		// Level type for NFC interrupt, IO_INT_HwNFC );	// Level type for NFC interrupt
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	s_pNFC->NFC_CTRL = 0
	                   | NFC_CTRL_CS_FIELD
	                   | NFC_CTRL_READ_DATA_CAPTURE_DELAY(0)
	                   | NFC_CTRL_RDYCFG(0)
	                   | NFC_CTRL_BURST_MODE_DATA_N_SPARE
	                   | NFC_CTRL_MDATA_COUNT(3)
	                   | NFC_CTRL_RDYSEL_0_ONLY
	                   | NFC_CTRL_READY_FLAG_N_INTR_UNMASK
	                   ;

	//-----------------------------------------
	//[WRITE/READ] STP: 0 PW: 4 HLD: 2
	//-----------------------------------------
	BITCSET( s_pNFC->NFC_WRCYC, 0xF0F0F, 0x402 );
	BITCSET( s_pNFC->NFC_RECYC, 0xF0F0F, 0x402 );

	//-----------------------------------------
	//[DMA Mode Set]: GDMA
	//[SET DMA ACKKNOWLEDGE]
	//-----------------------------------------
	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_DMA_MODE_NDMA );
	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_DMA_ACK_ENABLE );

	//--------------------------------------------
	//[WRITE/READ] STP: 0 PW: 4 HLD: 2
	//--------------------------------------------
	BITCSET( s_pNFC->NFC_WRCYC, 0xF0F0F, 0x402 );
	BITCSET( s_pNFC->NFC_RECYC, 0xF0F0F, 0x402 );

	//BITSET( s_pPIC->CLR1, HwINT1_NFC );
	//BITSET( s_pPIC->SEL1, HwINT1_NFC );			// Set NFC as IRQ interrupt
	//BITSET( s_pPIC->MODE1, HwINT1_NFC );		// Level type for NFC interrupt, IO_INT_HwNFC );	// Level type for NFC interrupt
#endif

	/* Searching NANDFLASH */
	NAND_IO_GetDeviceInfo( 0, &s_pDevInfo );

	//----------------------------------------
	// Setup Variable about ECC
	//  ECC Control Register
	//  Base Address for ECC Calculation
	//	Address mask for ECC area
	//----------------------------------------
#if defined(NFC_VER_100)
	s_pECC->ECC_CTRL	= 0x04000000;
	s_pECC->ECC_BASE	= s_pNFC->NFC_WDATA;
	s_pECC->ECC_MASK	= 0x00000000;
#endif

	NAND_Util_memcpy( &sNAND_IO_DeviceInfo, &s_pDevInfo, sizeof( NAND_IO_DEVINFO ) );

	NAND_IO_InitDMABuffer( &s_pDevInfo );

	//--------------------------------------------
	// For Nand Cycle Reset.
	//--------------------------------------------
	sNAND_IO_uiMaxBusClkMHz = 0;

	//--------------------------------------------
	// Initialize Transper Mode
	//--------------------------------------------
	#if defined(NAND_IO_USE_MCU_ACCESS)
	ND_TRACE("Use MCU Mode \n");
	NAND_IO_SetTransperMode( NAND_IO_TRANSPER_MCU_MODE );
	#elif defined(NAND_IO_USE_DMA_ACCESS)
	ND_TRACE("Use GDMA Mode \n");
	NAND_IO_SetTransperMode( NAND_IO_TRANSPER_GDMA_MODE );
	#elif defined(NAND_IO_USE_NDMA_ACCESS)
	{		
		#if defined(ANDROID_BOOT) || defined(ANDROID_KERNEL)
		#if defined(TCC893X)		
		unsigned int uiChipRevision;

		#if defined(ANDROID_BOOT)
		uiChipRevision = tcc_get_chip_revision();
		#elif defined(ANDROID_KERNEL)
		uiChipRevision = tcc_chip_rev;
		#endif
		
		if( uiChipRevision == 0xA )
		{
			ND_TRACE("Use GDMA Mode \n");
			NAND_IO_SetTransperMode( NAND_IO_TRANSPER_GDMA_MODE );
		}
		else if( uiChipRevision == 0xB )			
		#endif
		#endif
		{
			ND_TRACE("Use NDMA Mode \n");
			NAND_IO_SetTransperMode( NAND_IO_TRANSPER_NDMA_MODE );
		}
	}
	#endif

}

unsigned long NAND_IO_GetBootType()
{
	NAND_IO_DEVINFO *pDevInfo = &sNAND_IO_DeviceInfo;
	unsigned long uiBootType = 0;;

	if( A_READ_RETRY_TYPE_OF( pDevInfo->Feature.MediaType ) == A_RR_M )
	{
		uiBootType |= BL0_MICRON_READ_RETRY;
	}

	return uiBootType;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_Reset( unsigned short nChipNo, int nMode )
{
	/* Pre Process */
	NAND_IO_PreProcess();

	/* Set Setuo Time and Hold Time */
	NAND_IO_SetBasicCycleTime();

	/* Enable Chip Select */
	NAND_IO_EnableChipSelect( nChipNo );

	/* Set Data Bus as 16Bit */
	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );

	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	/* Command RESET [ 0xFF ] */
	if( nMode == NAND_IO_PARALLEL_COMBINATION_MODE )
		s_pNFC->NFC_CMD = 0xFFFF;
	else
		s_pNFC->NFC_CMD = 0x00FF;

	/* Wait until it is ready */
	NAND_Util_Delay();
	NAND_IO_WaitBusyStatus( nChipNo );

	/* Disable Chip Select */
	NAND_IO_DisableChipSelect();

	/* Post Process */
	NAND_IO_PostProcess();
}

/******************************************************************************
*
*	void    NAND_IO_ResetForReadID
*
*	Input	: Chip Select Number
*	Output	: NONE
*	Return	: NONE
*
*	Description : Reset NANDFLASH
*
*******************************************************************************/
void NAND_IO_ResetForReadID( unsigned short nChipNo, int nMode )
{
	unsigned int i;

	/* Pre Process */
	NAND_IO_PreProcess();

	/* Set Setuo Time and Hold Time */
	NAND_IO_SetBasicCycleTime();

	/* Enable Chip Select */
	NAND_IO_EnableChipSelect( nChipNo );

	/* Set Data Bus as 16Bit */
	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );

	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	/* Command RESET [ 0xFF ] */
	if( nMode == NAND_IO_PARALLEL_COMBINATION_MODE )
		s_pNFC->NFC_CMD = 0xFFFF;
	else
		s_pNFC->NFC_CMD = 0x00FF;

	/* Wait until it is ready */
	for( i = 0; i < 0x100; ++i )
	{
		NAND_Util_Delay();
	}
	NAND_IO_WaitBusyStatus( nChipNo );

	/* Disable Chip Select */
	NAND_IO_DisableChipSelect();

	/* Post Process */
	NAND_IO_PostProcess();
}

/******************************************************************************
*
*	void    NAND_IO_ReadID
*
*	Input	: Chip Select Number
*			  Mode : 0 => Serial Composition , 1 => Parallel Composition
*	Output	: NANDFLASH Device Code
*	Return	: NONE
*
*	Description : Get Device Code of NANDFLASH
*
*******************************************************************************/
void NAND_IO_ReadID( unsigned short nChipNo, NAND_IO_DEVID *nDeviceCode, int nMode )
{
	/* Pre Process */
	NAND_IO_PreProcess();

	/* Set Setup Time and Hold Time */
	NAND_IO_SetBasicCycleTime();

	/* Enable Chip Select */
	NAND_IO_EnableChipSelect( nChipNo );

	/* Set Data Bus as 16Bit */
	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );

	/* Parallel Composition */
	if( nMode == NAND_IO_PARALLEL_COMBINATION_MODE )
	{
		s_pNFC->NFC_CMD = 0x9090;	/* Command READ ID [ 0x90 ] */
		s_pNFC->NFC_SADDR = 0x0000;	/* Address [ 0x00 ] */
	}
	/* Serial Composition */
	else
	{
		s_pNFC->NFC_CMD = 0x0090;	/* Command READ ID [ 0x90 ] */
		s_pNFC->NFC_SADDR = 0x0000;	/* Address [ 0x00 ] */
	}

	/* Delay : tAR1[READID] Max 200nS */
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;

	/* Parallel Composition */
	if( nMode == NAND_IO_PARALLEL_COMBINATION_MODE )
	{
		nDeviceCode->Code[0] = ( unsigned short )s_pNFC->NFC_SDATA;
		nDeviceCode->Code[1] = ( unsigned short )s_pNFC->NFC_SDATA;
		nDeviceCode->Code[2] = ( unsigned short )s_pNFC->NFC_SDATA;
		nDeviceCode->Code[3] = ( unsigned short )s_pNFC->NFC_SDATA;
		nDeviceCode->Code[4] = ( unsigned short )s_pNFC->NFC_SDATA;
		nDeviceCode->Code[5] = ( unsigned short )s_pNFC->NFC_SDATA;
	}
	/* Serial Composition */
	else
	{
		nDeviceCode->Code[0] = ( unsigned char )( s_pNFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[1] = ( unsigned char )( s_pNFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[2] = ( unsigned char )( s_pNFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[3] = ( unsigned char )( s_pNFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[4] = ( unsigned char )( s_pNFC->NFC_SDATA & 0xFF );
		nDeviceCode->Code[5] = ( unsigned char )( s_pNFC->NFC_SDATA & 0xFF );
	}

	/* Disable Chip Select */
	NAND_IO_DisableChipSelect();

	/* Post Process */
	NAND_IO_PostProcess();
}

void NAND_IO_SkipDataAreaRandomizing(unsigned int fEnable)
{
	if(fEnable)
	{
		sNAND_IO_DeviceInfo.fSkipDataAreaRandomizing = TRUE;
	}
	else
	{
		sNAND_IO_DeviceInfo.fSkipDataAreaRandomizing = FALSE;
	}
}


/**************************************************************************
*  DESCRIPTION : DevInfo의 Chip no. 및 Die index 재설정.
*                DDP의 경우 Row address에 DDP bit 설정(odd numbered die only).
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_Update_CSNum_And_PageAddr( TNFTL_OBJECT_T *pTnftlObj, NAND_IO_DEVINFO *pDevInfo, unsigned int *puiPageAddress )
{
	NAND_IO_ASSERT(pTnftlObj->uiCurrentDieIndex<DIE_COUNT_MAX);

	pDevInfo->ChipNo = pTnftlObj->uiCurrentDieIndex;
	pDevInfo->ucDieIndex = (unsigned char)pTnftlObj->uiCurrentDieIndex;

	if( pDevInfo->Feature.ucDDPBit )
	{
		if( puiPageAddress != NULL )
		{
			NAND_IO_ASSERT( !( *puiPageAddress & ( 1 << pDevInfo->Feature.ucDDPBit ) ) );
			if( pDevInfo->ChipNo & 1 )
				*puiPageAddress = ( 1 << pDevInfo->Feature.ucDDPBit ) | *puiPageAddress;
		}

		pDevInfo->ChipNo = pDevInfo->ChipNo >> 1;
	}
}


/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_GenerateRowColAddr( unsigned int uiPageAddr, unsigned int uiColumnAddr,
                                        unsigned int *puiRowAddr, unsigned int *puiColumnAddr,
                                        NAND_IO_DEVINFO *nDevInfo )
{
	unsigned int	RowAddr;
	unsigned int	ColumnAddr;

	NAND_IO_ASSERT( uiColumnAddr < (unsigned int)( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) );

	ColumnAddr	= ( nDevInfo->Feature.MediaType & A_16BIT )  ? ( uiColumnAddr >> 1 ) : uiColumnAddr;
	RowAddr		= uiPageAddr;

	*puiRowAddr		= RowAddr;
	*puiColumnAddr	= ColumnAddr;
}


/**************************************************************************
*  DESCRIPTION : 현재 interleaving 중인 die 번호와 page address 를 저장.
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_SetInterleavingStatus( NAND_IO_DEVINFO *pDevInfo, unsigned int uiPageAddress, NAND_IO_OPERATION_T ucOperationType )
{
	//ND_TRACE("[IL:Die#%d] %d, %d\n", pDevInfo->ucDieIndex, uiPageAddress, ucOperationType);
	if( pDevInfo->fInterleaveUsable )
	{
		if( pDevInfo->ucDieIndex == 0 )
			pDevInfo->ucInterleavingStatus |= NAND_IO_STATUS_INTERLEAVING_DIE0;
		else
			pDevInfo->ucInterleavingStatus |= NAND_IO_STATUS_INTERLEAVING_DIE1;

		pDevInfo->InterleavingOperation[pDevInfo->ucDieIndex] = ucOperationType;
		pDevInfo->uiInterleavingPageAddr[pDevInfo->ucDieIndex] = uiPageAddress;
	}
}


/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadStatusForInterleaveClear( NAND_IO_DEVINFO *pDevInfo )
{
	unsigned int		uStatus;
	unsigned int		uCheckBit;
	unsigned long int	timeout;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	//=============================================
	//	Command READ STATUS - Die 0
	//=============================================
	//if( pDevInfo->ucDieIndex == 0 )
	if( ( pDevInfo->ucDieIndex == 0 ) &&
		( ( pDevInfo->ucInterleavingStatus & NAND_IO_STATUS_INTERLEAVING_DIE0 ) == NAND_IO_STATUS_INTERLEAVING_DIE0 ) )
	{
		if ( pDevInfo->Feature.DeviceID.Code[0] == SAMSUNG_NAND_MAKER_ID )
			s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0xF1F1;
		// not tested yet
		//else if ( pDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID )
		//{
			// Smart NAND[24nm]
			//if ( pDevInfo->Feature.DeviceID.Code[4] & 0x80 )
				//s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0xF1F1;
			//else
				//s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0x7171;
		//}
		//
		//else if ( ( pDevInfo->Feature.DeviceID.Code[0] == MICRON_NAND_MAKER_ID ) )
		//{
			//s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0x7878;
			//NAND_IO_WriteBlockPageAddr( pDevInfo->uiInterleavingPageAddr, pDevInfo );
		//}

	 	timeout = 0xFFFFF;
	 	while( timeout )
	 	{
			uStatus	  = pDevInfo->CmdMask & s_pNFC->NFC_SDATA;
			uCheckBit = pDevInfo->CmdMask & 0x4040;
			
			/* Check if it is ready */
			if( ( uStatus & uCheckBit ) == uCheckBit )
			{
				uCheckBit	= pDevInfo->CmdMask & 0x0101;

				/* Check if it is Fail */
				if( uStatus & uCheckBit )
				{
					res = NAND_IO_ERROR_FAIL_TO_INTERLEAVE;
					if( ( pDevInfo->Feature.UsableFunc & F_MP ) )
					{
						pDevInfo->BadBlockInfo.BlockStatus[0] = MULTI_PLANE_BAD_BLOCK;
						pDevInfo->BadBlockInfo.BlockStatus[1] = MULTI_PLANE_BAD_BLOCK;
					}
					else
						pDevInfo->BadBlockInfo.BlockStatus[pDevInfo->ChipNo] = MULTI_PLANE_BAD_BLOCK;
				}
				break;
			}
		}

		if( !timeout )
			res = NAND_IO_ERROR_READ_STATUS_FOR_INTERLEAVING;
		else
			pDevInfo->ucInterleavingStatus &= ~NAND_IO_STATUS_INTERLEAVING_DIE0;
	}

	//=============================================
	//	Command READ STATUS - Die 1
	//=============================================
	if( ( pDevInfo->ucDieIndex == 1 ) &&
		( ( pDevInfo->ucInterleavingStatus & NAND_IO_STATUS_INTERLEAVING_DIE1 ) == NAND_IO_STATUS_INTERLEAVING_DIE1 ) )
	{
		if( pDevInfo->Feature.DeviceID.Code[0] == SAMSUNG_NAND_MAKER_ID )
			s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0xF2F2;
		//else if ( pDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID )
		//{
			// Smart NAND[24nm]
			//if ( pDevInfo->Feature.DeviceID.Code[4] & 0x80 )
				//s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0xF1F1;
			//else
			//s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0x7171;
		//}
		/* -- not test yet
		else if ( ( pDevInfo->Feature.DeviceID.Code[0] == MICRON_NAND_MAKER_ID ) || ( pDevInfo->Feature.DeviceID.Code[0] == INTEL_NAND_MAKER_ID ) )
		{
			s_pNFC->NFC_CMD = pDevInfo->CmdMask & 0x7878;
			NAND_IO_WriteBlockPageAddr( gInterLeaveDie1BlockAddr, pDevInfo );
		}
		-- */

	 	timeout = 0xFFFFF;
	 	while( timeout )
	 	{
			uStatus		= pDevInfo->CmdMask & s_pNFC->NFC_SDATA;
			uCheckBit = pDevInfo->CmdMask & 0x4040;
			
			/* Check if it is ready */
			if( ( uStatus & uCheckBit ) == uCheckBit )
			{
				uCheckBit	= pDevInfo->CmdMask & 0x0101;

				/* Check if it is Fail */
				if( uStatus & uCheckBit )
				{
					res = NAND_IO_ERROR_FAIL_TO_INTERLEAVE;
					if( ( pDevInfo->Feature.UsableFunc & F_MP ) )
					{
						pDevInfo->BadBlockInfo.BlockStatus[2] = MULTI_PLANE_BAD_BLOCK;
						pDevInfo->BadBlockInfo.BlockStatus[3] = MULTI_PLANE_BAD_BLOCK;
					}
					else
						pDevInfo->BadBlockInfo.BlockStatus[pDevInfo->ChipNo] = MULTI_PLANE_BAD_BLOCK;
				}
				break;
			}
		}

		if( !timeout )
			res =  NAND_IO_ERROR_READ_STATUS_FOR_INTERLEAVING;
		else
			pDevInfo->ucInterleavingStatus &= ~NAND_IO_STATUS_INTERLEAVING_DIE1;
	}

 	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_WaitBusyForInterleave( NAND_IO_DEVINFO *pDevInfo )
{
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	while( (res = NAND_IO_ReadStatusForInterleaveClear( pDevInfo )) == NAND_IO_ERROR_READ_STATUS_FOR_INTERLEAVING );
	return res;
}


/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadStatus( NAND_IO_DEVINFO *nDevInfo )
{
	unsigned int uStatus,uiReady;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	unsigned short usReadStatus = 0;

	//================================
	//	Command READ STATUS [ 0x70 ]
	//================================
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x7070;

	// Delay : more than 200nS
	NAND_Util_Delay();

	//================================
	//	Read IO Status
	//================================
	uStatus = nDevInfo->CmdMask & s_pNFC->NFC_SDATA;
	uiReady = nDevInfo->CmdMask & 0x4040;

	/* Check if it is ready */
	if( ( uStatus & uiReady ) == uiReady )
	{
		unsigned int uiFail = nDevInfo->CmdMask & 0x0101;
		if( uStatus & uiFail )
		{
			if( nDevInfo->Feature.MediaType & A_PARALLEL )
			{
				if( uStatus & ( uiFail & 0x0100 ) )
					usReadStatus |= NAND_IO_STATUS_FAIL_CS1_PARALLEL;

				if( uStatus & ( uiFail & 0x0001 ) )
					usReadStatus |= NAND_IO_STATUS_FAIL_CS0_PARALLEL;
			}
			else
			{
				if( uStatus & uiFail )
					usReadStatus |= NAND_IO_STATUS_FAIL_CS0_SERIAL;
			}
		}
	}
	else
	{
		usReadStatus = NAND_IO_STATUS_FAIL_BUSY;
	}

	if( usReadStatus == NAND_IO_STATUS_FAIL_BUSY )
	{
		res = NAND_IO_ERROR_READ_STATUS_BUSY;
	}
	else if( usReadStatus )
	{
		res = NAND_IO_ERROR_READ_STATUS_FAILURE;
	}

	if( res != NAND_IO_SUCCESS )
	{
		ND_DEBUG("ReadStatus = %X\n", uStatus);
	}

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadStatusOfCacheProgram( NAND_IO_DEVINFO *nDevInfo, unsigned int fLastPage )
{
	unsigned int uStatus;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	unsigned short usReadStatus = 0;

	//================================
	//	Command READ STATUS [ 0x70 ]
	//================================
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x7070;

	// Delay : more than 200nS
	NAND_Util_Delay();

	uStatus = nDevInfo->CmdMask & s_pNFC->NFC_SDATA;

	if( !fLastPage )
	{
		unsigned int uiTrueReady = nDevInfo->CmdMask & 0x4040; /* Cache Ready/Busy */
		if( ( uStatus & uiTrueReady ) == uiTrueReady )
		{
			unsigned int uiPrevFail = nDevInfo->CmdMask & 0x0202; /* Pass/Fail of (N-1) */
			if( uStatus & uiPrevFail )
			{
				usReadStatus |= NAND_IO_STATUS_FAIL_PREVIOUS_PROGRAM;
			}
		}
	}
	else
	{
		unsigned int uiAllReady = nDevInfo->CmdMask & 0x6060; /* Cache Ready/Busy + True Ready/Busy */
		if( ( uStatus & uiAllReady ) == uiAllReady )
		{
			unsigned int uiPrevFail = nDevInfo->CmdMask & 0x0202; /* Pass/Fail of (N-1) */
			unsigned int uiFail = nDevInfo->CmdMask & 0x0101; /* Pass/Fail of N */
			if( uStatus & uiPrevFail )
			{
				usReadStatus |= NAND_IO_STATUS_FAIL_PREVIOUS_PROGRAM;
			}
			if( uStatus & uiFail )
			{
				usReadStatus |= NAND_IO_STATUS_FAIL_CS0_SERIAL;
			}
		}
		else
		{
			usReadStatus |= NAND_IO_STATUS_FAIL_BUSY;
		}
	}

	if( usReadStatus )
		res = NAND_IO_ERROR_READ_STATUS_FAILURE;

	if( res != NAND_IO_SUCCESS )
	{
		ND_DEBUG("ReadStatus = %X\n", uStatus);
	}

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadStatusOfMultiPlane( NAND_IO_DEVINFO *nDevInfo )
{
	unsigned int uStatus, uiReady;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	unsigned short usReadStatus = 0;

	//================================
	//	Command READ STATUS [ 0x70 ]
	//================================
	if( nDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID )
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x7171;
	else
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x7070;

	// Delay : more than 200nS
	NAND_Util_Delay();

	//=============================================
	// DATA BUS WIDTH Setting
	//=============================================
	//if( nDevInfo->Feature.MediaType & A_16BIT )
	//	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	//else
	//	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//================================
	//	Read IO Status
	//================================
	uStatus = nDevInfo->CmdMask & s_pNFC->NFC_SDATA;
	uiReady = nDevInfo->CmdMask & 0x4040;

	/* Check if it is ready */
	if( ( uStatus & uiReady ) == uiReady )
	{
		unsigned int uiFail = nDevInfo->CmdMask & 0x0101;

		//================================
		//	Check Bit Status
		//================================
		if( nDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID )
		{
			if( uStatus & uiFail )
			{
				if( uStatus & ( nDevInfo->CmdMask & 0x0202 ) )
					usReadStatus = ( NAND_IO_ERROR_T )NAND_IO_STATUS_DISTRICT_0;
				else if( uStatus & ( nDevInfo->CmdMask & 0x0404 ) )
					usReadStatus = ( NAND_IO_ERROR_T )NAND_IO_STATUS_DISTRICT_1;
			}
		}
		else
		{
			if( uStatus & uiFail )
			{
				if( nDevInfo->Feature.MediaType & A_PARALLEL )
				{
					if( uStatus & ( uiFail & 0x0100 ) )
						usReadStatus |= NAND_IO_STATUS_FAIL_CS1_PARALLEL;

					if( uStatus & ( uiFail & 0x0001 ) )
						usReadStatus |= NAND_IO_STATUS_FAIL_CS0_PARALLEL;
				}
				else
				{
					if( uStatus & uiFail )
						usReadStatus |= NAND_IO_STATUS_FAIL_CS0_SERIAL;
				}
			}
		}
	}
	else
	{
		usReadStatus |= NAND_IO_STATUS_FAIL_BUSY;
	}

	if( usReadStatus )
		res = NAND_IO_ERROR_READ_STATUS_FAILURE;

	if( res != NAND_IO_SUCCESS )
	{
		ND_DEBUG("ReadStatus = %X\n", uStatus);
	}

	return res;
}


/**************************************************************************
*  DESCRIPTION : Read Retry offset Defatult value from nand register.
*                Set Read Retry Offset for each retry set. ( for Hynix 26nm NAND Flash)
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : 
*
**************************************************************************/
#define RETRY_COUNT_MAX                      6
static unsigned char sNAND_IO_ReadRetryReg_1[RETRY_COUNT_MAX+1];
static unsigned char sNAND_IO_ReadRetryReg_2[RETRY_COUNT_MAX+1];
static unsigned char sNAND_IO_ReadRetryReg_3[RETRY_COUNT_MAX+1];
static unsigned char sNAND_IO_ReadRetryReg_4[RETRY_COUNT_MAX+1];

static void NAND_IO_Hynix_GetReadRetryParameter_For_26nm(NAND_IO_DEVINFO *pDevInfo)
{
	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( pDevInfo );

	NAND_IO_EnableChipSelect( pDevInfo->ChipNo );

	NAND_BD_Enable_WP_Port();

	/* data bus configuration is needed? */

	NAND_IO_SetCommCycleTime();
	NAND_IO_SetReadCycleTime();
	s_pNFC->NFC_CMD 			= 0x37373737;
	
	s_pNFC->NFC_SADDR        	= 0xA7A7A7A7;
	sNAND_IO_ReadRetryReg_1[0] 	= s_pNFC->NFC_SDATA;
	s_pNFC->NFC_SADDR        	= 0xADADADAD;
	sNAND_IO_ReadRetryReg_2[0] 	= s_pNFC->NFC_SDATA;
	s_pNFC->NFC_SADDR        	= 0xAEAEAEAE;
	sNAND_IO_ReadRetryReg_3[0] 	= s_pNFC->NFC_SDATA;
	s_pNFC->NFC_SADDR        	= 0xAFAFAFAF;
	sNAND_IO_ReadRetryReg_4[0] = s_pNFC->NFC_SDATA;

	sNAND_IO_ReadRetryReg_1[1] = sNAND_IO_ReadRetryReg_1[0];
	sNAND_IO_ReadRetryReg_1[2] = 0;
	sNAND_IO_ReadRetryReg_1[3] = 0;
	sNAND_IO_ReadRetryReg_1[4] = 0;
	sNAND_IO_ReadRetryReg_1[5] = 0;
	sNAND_IO_ReadRetryReg_1[6] = 0;

	sNAND_IO_ReadRetryReg_2[1] = sNAND_IO_ReadRetryReg_2[0] + 0x06;
	sNAND_IO_ReadRetryReg_2[2] = sNAND_IO_ReadRetryReg_2[0] - 0x03;
	sNAND_IO_ReadRetryReg_2[3] = sNAND_IO_ReadRetryReg_2[0] - 0x06;
	sNAND_IO_ReadRetryReg_2[4] = sNAND_IO_ReadRetryReg_2[0] - 0x09;
	sNAND_IO_ReadRetryReg_2[5] = 0;
	sNAND_IO_ReadRetryReg_2[6] = 0;

	sNAND_IO_ReadRetryReg_3[1] = sNAND_IO_ReadRetryReg_3[0] + 0x0A;
	sNAND_IO_ReadRetryReg_3[2] = sNAND_IO_ReadRetryReg_3[0] - 0x07;
	sNAND_IO_ReadRetryReg_3[3] = sNAND_IO_ReadRetryReg_3[0] - 0x0D;
	sNAND_IO_ReadRetryReg_3[4] = sNAND_IO_ReadRetryReg_3[0] - 0x14;
	sNAND_IO_ReadRetryReg_3[5] = sNAND_IO_ReadRetryReg_3[0] - 0x1A;
	sNAND_IO_ReadRetryReg_3[6] = sNAND_IO_ReadRetryReg_3[0] - 0x20;

	sNAND_IO_ReadRetryReg_4[1] = sNAND_IO_ReadRetryReg_4[0] + 0x06;
	sNAND_IO_ReadRetryReg_4[2] = sNAND_IO_ReadRetryReg_4[0] - 0x08;
	sNAND_IO_ReadRetryReg_4[3] = sNAND_IO_ReadRetryReg_4[0] - 0x0F;
	sNAND_IO_ReadRetryReg_4[4] = sNAND_IO_ReadRetryReg_4[0] - 0x17;
	sNAND_IO_ReadRetryReg_4[5] = sNAND_IO_ReadRetryReg_4[0] - 0x1E;
	sNAND_IO_ReadRetryReg_4[6] = sNAND_IO_ReadRetryReg_4[0] - 0x25;

	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

}

/**************************************************************************
*  DESCRIPTION : Read Retry offset value from OTP Area.
*                ( for Hynix 20nm NAND Flash)
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : 
*
**************************************************************************/
static unsigned char sNAND_IO_ReadRetryCount[2];
static unsigned char sNAND_IO_ReadRetryRegOfOTP[1024];

unsigned int NAND_IO_Hynix_VerifyReadRetryParameter_For_20nm( const unsigned char *pucParameter, const unsigned char*pucRevParameter  )
{
	unsigned int res = FALSE, i;
	unsigned char *pucParam, *pucRevParam;

	pucParam 	= (unsigned char*)pucParameter;
	pucRevParam	= (unsigned char*)pucRevParameter;

	for( i = 0; i < 64; i++ )
	{
		if( !(( pucParam[i] ^ pucRevParam[i] ) == 0xFF) )
			break;
	}

	if( i == 64 )
		res = TRUE;

	//_ND_TRACE(" Check Verify : %d \n", i);

	return res;
	
}

static void NAND_IO_Hynix_GetReadRetryParameter_For_20nm(NAND_IO_DEVINFO *pDevInfo)
{
	unsigned int uiNandDie_Identifier = pDevInfo->Feature.DeviceID.Code[3];
	unsigned int i;
	
	NAND_IO_PreProcess();
	NAND_IO_BusControl( pDevInfo );

	NAND_IO_EnableChipSelect( pDevInfo->ChipNo );

	//NAND_BD_Enable_WP_Port();
	NAND_BD_Disable_WP_Port();

	/* data bus configuration is needed? */

	NAND_IO_SetCommCycleTime();
	NAND_IO_SetReadCycleTime();
	NAND_IO_SetWriteCycleTime();
	
	s_pNFC->NFC_CMD 			= 0xFFFFFFFF;
	NAND_IO_WaitBusy( pDevInfo->ChipNo );

	s_pNFC->NFC_CMD 			= 0x36363636;

	if ( uiNandDie_Identifier == 0xDA ) // For 20nm MLC A-Die Nand
	{
		s_pNFC->NFC_SADDR        	= 0xFFFFFFFF;
		s_pNFC->NFC_SDATA			= 0x40404040;
		s_pNFC->NFC_SADDR        	= 0xCCCCCCCC;
		s_pNFC->NFC_SDATA			= 0x4D4D4D4D;
	} 
	else if( ( uiNandDie_Identifier == 0xEB ) || // For 20nm MLC B-Die Nand
			 ( uiNandDie_Identifier == 0x91 ) )  // For 20nm MLC C-Die Nand
	{
		ND_DEBUG(" Get Read-Retry Paramater from 20nm B or C die Nand \n");
		s_pNFC->NFC_SADDR        	= 0xAEAEAEAE;
		s_pNFC->NFC_SDATA			= 0x00000000;
		s_pNFC->NFC_SADDR        	= 0xB0B0B0B0;
		s_pNFC->NFC_SDATA			= 0x4D4D4D4D;
	}
	else
	{
		ND_TRACE(" Get Read-Retry Paramaeter Error! Wrong NAND Die Identifier\n" );
	}

	s_pNFC->NFC_CMD 			= 0x16161616;
	s_pNFC->NFC_CMD 			= 0x17171717;
	s_pNFC->NFC_CMD 			= 0x04040404;
	s_pNFC->NFC_CMD 			= 0x19191919;
	s_pNFC->NFC_CMD 			= 0x00000000;

	s_pNFC->NFC_SADDR        	= 0x00000000;
	s_pNFC->NFC_SADDR        	= 0x00000000;
	s_pNFC->NFC_SADDR        	= 0x00000000;
	s_pNFC->NFC_SADDR        	= 0x02020202;
	s_pNFC->NFC_SADDR        	= 0x00000000;

	s_pNFC->NFC_CMD 			= 0x30303030;
	NAND_IO_WaitBusy( pDevInfo->ChipNo );

	sNAND_IO_ReadRetryCount[0] 	= s_pNFC->NFC_SDATA;
	sNAND_IO_ReadRetryCount[1] 	= s_pNFC->NFC_SDATA;

	for( i = 0; i < 1024; i++ )
		sNAND_IO_ReadRetryRegOfOTP[i] = s_pNFC->NFC_SDATA;	

	s_pNFC->NFC_CMD 			= 0xFFFFFFFF;
	NAND_IO_WaitBusy( pDevInfo->ChipNo );

	s_pNFC->NFC_CMD 			= 0x38383838;
	NAND_IO_WaitBusy( pDevInfo->ChipNo );

	{
		unsigned int res;
		for( i = 0; i < 8; i++ )
		{
			res = NAND_IO_Hynix_VerifyReadRetryParameter_For_20nm( &sNAND_IO_ReadRetryRegOfOTP[i*128]
																  ,&sNAND_IO_ReadRetryRegOfOTP[(i*128)+64] );

			if( res == TRUE )
			{
				ND_DEBUG(" Set Retry Parameter : %d \n", i );
				if( i != 0 )
					memcpy( (void*)sNAND_IO_ReadRetryRegOfOTP, (void*)&sNAND_IO_ReadRetryRegOfOTP[i*128], 64 );
				break;
			}
			else
			{
				ND_TRACE("Read-Retry Parameter Set %d is Broken ! \n", i);
			}
		}
	}

	#if 0
	ND_TRACE("NAND_IO_Hynix_GetReadRetry\n");
	for( i = 0; i < 8; i++ )
	{
		unsigned int uiReadParameterPos = i * 8;		
		ND_TRACE("Parameter: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X \n", sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+1]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+2]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+3]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+4]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+5]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+6]
																		  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+7]);
	}
	#endif

	//
	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_BD_Enable_WP_Port();
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();
}


/**************************************************************************
*  DESCRIPTION : 1~6 까지 Circular Iteration value 를 위한 Func.
*  				 
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : 
*
**************************************************************************/
#define HYNIX_CIRCULAR_VALUE_MAX_FOR_26NM 	6
#define HYNIX_CIRCULAR_VALUE_MAX_FOR_20NM 	7
static unsigned int NAND_IO_Hynix_GetCircularValue( unsigned int uiCircularMaxValue )
{
	static unsigned int uiCircularValue = 0;

	if( uiCircularMaxValue == (unsigned int)(-1) )
	{
		uiCircularValue = 0;
	}
	else
	{
		uiCircularValue++;
		/* Retry Trial : #1 ~#6 (Circular Iteration) */
		if ( uiCircularValue > uiCircularMaxValue )
		{
	//		uiCircularValue = 1;
			uiCircularValue = 0; //return to normal
		}
	}

	return uiCircularValue;
}


/**************************************************************************
*  DESCRIPTION : Read Retry offset 설정 (for Hynix)
*  				 
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : max 6회. 타사와 달리 Read Retry Offset이 Circular Iteration 구조로 되어 있다.
*           Read Retry Offset 설정 시, WP가 반드시 High인 상태여야 한다.
**************************************************************************/
static unsigned int NAND_IO_Hynix_SetReadRetry_For_26nm(NAND_IO_DEVINFO *pDevInfo, unsigned int uiStep)
{
	unsigned int res = TRUE;
	unsigned int uiMode;

	/* WP# must be keep on High during Set Parameter Sequence */
	if( NAND_BD_Disable_WP_Port() != TRUE )
		res = FALSE;

	if ( res == TRUE)
	{
		/* Read Retry Flag Setting */
		if( 1 <= uiStep && uiStep <= 6 )
		{
			pDevInfo->fReadRetryNow = TRUE;

			uiMode = NAND_IO_Hynix_GetCircularValue( HYNIX_CIRCULAR_VALUE_MAX_FOR_26NM );

			//ND_TRACE("NAND_IO_Hynix_SetReadRetry#%d : C#%d ( 0x%x, 0x%x, 0x%x, 0x%x)\n", uiStep, uiMode, sNAND_IO_ReadRetryReg_1[uiMode], sNAND_IO_ReadRetryReg_2[uiMode], sNAND_IO_ReadRetryReg_3[uiMode], sNAND_IO_ReadRetryReg_4[uiMode]);

			NAND_IO_SetCommCycleTime();
			NAND_IO_SetWriteCycleTime();			
			s_pNFC->NFC_CMD		= 0x36363636;

			s_pNFC->NFC_SADDR	= 0xA7A7A7A7;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_1[uiMode];
			s_pNFC->NFC_SADDR	= 0xADADADAD;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_2[uiMode];
			s_pNFC->NFC_SADDR	= 0xAEAEAEAE;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_3[uiMode];
			s_pNFC->NFC_SADDR	= 0xAFAFAFAF;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_4[uiMode];

			/* NAND_IO_ReadStatus is needed? */

		}
		
		else if(uiStep == 0) // When Reseting board(use reset button), readretry parameters are not initialized.
		{
			NAND_IO_SetCommCycleTime();
			NAND_IO_SetWriteCycleTime();			
			s_pNFC->NFC_CMD		= 0x36363636;

			s_pNFC->NFC_SADDR	= 0xA7A7A7A7;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_1[0];
			s_pNFC->NFC_SADDR	= 0xADADADAD;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_2[0];
			s_pNFC->NFC_SADDR	= 0xAEAEAEAE;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_3[0];
			s_pNFC->NFC_SADDR	= 0xAFAFAFAF;
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryReg_4[0];

			pDevInfo->fReadRetryNow = FALSE;
			res = FALSE;			
		}
		else
		{
			pDevInfo->fReadRetryNow = FALSE;
			res = FALSE;
		}

		s_pNFC->NFC_CMD		= 0x16161616;
		NAND_Util_Delay();

		NAND_BD_Enable_WP_Port();
	}
	
	return res;
}



/**************************************************************************
*  DESCRIPTION : Read Retry offset 설정 (for Hynix)
*  				 
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : max 8회. 
**************************************************************************/
static unsigned char sNAND_IO_ReadRetryAddr_For_Adie[8] = {0xCC, 0xBF, 0xAA, 0xAB, 0xCD, 0xAD, 0xAE, 0xAF};
static unsigned char sNAND_IO_ReadRetryAddr_For_BCdie[8] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7};

static unsigned int NAND_IO_Hynix_SetReadRetry_For_20nm(NAND_IO_DEVINFO *pDevInfo, unsigned int uiStep)
{
	unsigned int res = TRUE;
	unsigned int uiMode, uiReadParameterPos;
	unsigned int uiNandDie_Identifier = pDevInfo->Feature.DeviceID.Code[3];	
	unsigned char *pReadRetryAddr=NULL;

	/* WP# must be keep on High during Set Parameter Sequence */
	if( NAND_BD_Disable_WP_Port() != TRUE )
		res = FALSE;

	if ( uiNandDie_Identifier == 0xDA ) // For 20nm MLC A-Die Nand
	{
		pReadRetryAddr = sNAND_IO_ReadRetryAddr_For_Adie;
	} 
	else if( ( uiNandDie_Identifier == 0xEB ) || // For 20nm MLC B-Die Nand
			 ( uiNandDie_Identifier == 0x91 ) )  // For 20nm MLC B-Die Nand
	{
		pReadRetryAddr = sNAND_IO_ReadRetryAddr_For_BCdie;
	}
	else
	{
		ND_TRACE(" Set Read-Retry Paramaeter Error! Wrong NAND Die Identifier\n" );
	}

	if ( res == TRUE)
	{
		/* Read Retry Flag Setting */
		if( 1 <= uiStep && uiStep <= (HYNIX_CIRCULAR_VALUE_MAX_FOR_20NM + 1) )
		{
			pDevInfo->fReadRetryNow = TRUE;

			uiMode = NAND_IO_Hynix_GetCircularValue( HYNIX_CIRCULAR_VALUE_MAX_FOR_20NM );
			uiReadParameterPos = uiMode * 8;

			#if 0
			ND_TRACE("NAND_IO_Hynix_SetReadRetry#%d : C#%d, %d\n", uiStep, uiMode, uiReadParameterPos );
			ND_TRACE("Addr: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X \n", pReadRetryAddr[0]
																			  , pReadRetryAddr[1]
																			  , pReadRetryAddr[2]
																			  , pReadRetryAddr[3]
																			  , pReadRetryAddr[4]
																			  , pReadRetryAddr[5]
																			  , pReadRetryAddr[6]
																			  , pReadRetryAddr[7]);
			ND_TRACE("Parameter: 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X \n", sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+1]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+2]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+3]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+4]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+5]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+6]
																			  , sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos+7]);
			#endif

			NAND_IO_SetCommCycleTime();
			s_pNFC->NFC_CMD		= 0x36363636;

			s_pNFC->NFC_SADDR	= pReadRetryAddr[0]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos];		NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[1]; 									NAND_Util_Delay();	
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 1];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[2]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 2];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[3]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 3];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[4]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 4];	NAND_Util_Delay();	
			s_pNFC->NFC_SADDR	= pReadRetryAddr[5]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 5];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[6]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 6];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[7]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 7];	NAND_Util_Delay();

			/* NAND_IO_ReadStatus is needed? */
			s_pNFC->NFC_CMD		= 0x16161616;			

			NAND_IO_WaitBusy( pDevInfo->ChipNo );			
		}	
		else
		{
			uiReadParameterPos = 0;

			NAND_IO_SetCommCycleTime();
			s_pNFC->NFC_CMD		= 0x36363636;

			s_pNFC->NFC_SADDR	= pReadRetryAddr[0]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos];		NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[1]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 1];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[2]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 2];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[3]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 3];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[4]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 4];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[5]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 5];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[6]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 6];	NAND_Util_Delay();
			s_pNFC->NFC_SADDR	= pReadRetryAddr[7]; 									NAND_Util_Delay();
			s_pNFC->NFC_SDATA	= sNAND_IO_ReadRetryRegOfOTP[uiReadParameterPos + 7];	NAND_Util_Delay();

			/* NAND_IO_ReadStatus is needed? */
			s_pNFC->NFC_CMD		= 0x16161616;

			NAND_IO_WaitBusy( pDevInfo->ChipNo );

			NAND_IO_Hynix_GetCircularValue( (-1) );

			pDevInfo->fReadRetryNow = FALSE;
			res = FALSE;
			
		}

		NAND_BD_Enable_WP_Port();
	}
	
	return res;	
}


static unsigned int ReadRetrySet_Samsung[15][4] = {
	{	/* SET00 */ 0x00, 0x00, 0x00, 0x00,	},
	{	/* SET01 */ 0x05, 0x0A, 0x00, 0x00,	},
	{	/* SET02 */ 0x28, 0x00, 0xEC, 0xD8,	},
	{	/* SET03 */ 0xED, 0xF5, 0xED, 0xE6,	},
	{	/* SET04 */ 0x0A, 0x0F, 0x05, 0x00,	},
	{	/* SET05 */ 0x0F, 0x0A, 0xFB, 0xEC,	},
	{	/* SET06 */ 0xE8, 0xEF, 0xE8, 0xDC,	},
	{	/* SET07 */ 0xF1, 0xFB, 0xFE, 0xF0,	},
	{	/* SET08 */ 0x0A, 0x00, 0xFB, 0xEC,	},
	{	/* SET09 */ 0xD0, 0xE2, 0xD0, 0xC2,	},
	{	/* SET10 */ 0x14, 0x0F, 0xFB, 0xEC,	},
	{	/* SET11 */ 0xE8, 0xFB, 0xE8, 0xDC,	},
	{	/* SET12 */ 0x1E, 0x14, 0xFB, 0xEC,	},
	{	/* SET13 */ 0xFB, 0xFF, 0xFB, 0xF8,	},
	{	/* SET14 */ 0x07, 0x0C, 0x02, 0x00	}
};


/**************************************************************************
*  DESCRIPTION : Read Retry offset 설정 (for Samsung)
*  				 
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : max 14회. uiStep이 0이면 default로 setting.
*           Read Retry가 성공하면 설정값을 default로 돌려놓아야 한다.
*           Read Retry offset 설정 후 최소 300ns waiting.
**************************************************************************/
static unsigned int NAND_IO_Samsung_SetReadRetry(NAND_IO_DEVINFO *nDevInfo, unsigned int uiStep)
{
	unsigned int res = TRUE;
	unsigned int i;

	// Read Retry Flag Setting
	if( 1 <= uiStep && uiStep <= 14 )
	{
		nDevInfo->fReadRetryNow = TRUE;
	}
	else
	{
		nDevInfo->fReadRetryNow = FALSE;
		res = FALSE;
	}
	
	if( res == TRUE )
	{
		NAND_IO_SetCommCycleTime();
		s_pNFC->NFC_CMD = 0xA1A1A1A1;

		/* P0 */
		s_pNFC->NFC_SADDR	= 0x00000000;
		s_pNFC->NFC_SADDR	= 0xA7A7A7A7;
		s_pNFC->NFC_SDATA	= ReadRetrySet_Samsung[uiStep][0];
		/* P1 */
		s_pNFC->NFC_SADDR	= 0x00000000;
		s_pNFC->NFC_SADDR	= 0xA4A4A4A4;
		s_pNFC->NFC_SDATA	= ReadRetrySet_Samsung[uiStep][1];
		/* P2 */
		s_pNFC->NFC_SADDR	= 0x00000000;
		s_pNFC->NFC_SADDR	= 0xA5A5A5A5;
		s_pNFC->NFC_SDATA	= ReadRetrySet_Samsung[uiStep][2];
		/* P3 */
		s_pNFC->NFC_SADDR	= 0x00000000;
		s_pNFC->NFC_SADDR	= 0xA6A6A6A6;
		s_pNFC->NFC_SDATA	= ReadRetrySet_Samsung[uiStep][3];

		/* delay for 300ns */
		for ( i=0; i<3; i++ )
			NAND_Util_Delay();    /* more than 300ns? */
	}

	return res;
}

/**************************************************************************
*  DESCRIPTION : Read Retry offset 설정 (for Micron)
*  				 
*  INPUT : 
*  
*  OUTPUT : 
*  
*  REMARK : max 7회. Read Retry offset 설정 후 R/B 체크 필요.
*
**************************************************************************/
static unsigned int NAND_IO_Micron_SetReadRetry(NAND_IO_DEVINFO *nDevInfo, unsigned int uiStep)
{
	unsigned int res = TRUE;
	unsigned int uiMode;

	// Read Retry Flag Setting
	if( 1 <= uiStep && uiStep <= 7 )
	{
		nDevInfo->fReadRetryNow = TRUE;
	}
	else
	{
		nDevInfo->fReadRetryNow = FALSE;
	}

	switch(uiStep)
	{
		case 0:
			uiMode = 0x00000000;
			break;
		case 1:
			uiMode = 0x01010101;
			break;
		case 2:
			uiMode = 0x02020202;
			break;
		case 3:
			uiMode = 0x03030303;
			break;
		case 4:
			uiMode = 0x04040404;
			break;
		case 5:
			uiMode = 0x05050505;
			break;
		case 6:
			uiMode = 0x06060606;
			break;
		case 7:
			uiMode = 0x07070707;
			break;
		default:
			res = FALSE;
			break;
	}

	if( res == TRUE )
	{
		NAND_IO_SetCommCycleTime();

		s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

		s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0xEFEFEFEF;
		s_pNFC->NFC_SADDR = nDevInfo->CmdMask & 0x89898989;

		NAND_Util_Delay();
		NAND_IO_SetWriteCycleTime();

		// P1
		s_pNFC->NFC_SDATA = nDevInfo->CmdMask & uiMode;
		// P2
		s_pNFC->NFC_SDATA = nDevInfo->CmdMask & 0x00000000;
		// P3
		s_pNFC->NFC_SDATA = nDevInfo->CmdMask & 0x00000000;
		// P4
		s_pNFC->NFC_SDATA = nDevInfo->CmdMask & 0x00000000;

		NAND_IO_WaitBusy( nDevInfo->ChipNo );
	}

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_GetDeviceInfo( unsigned short nChipNo, NAND_IO_DEVINFO *nDevInfo )
{
	unsigned short		j, k, l;
	unsigned char			bFindMedia;
	unsigned char			bFindMakerNo;
	unsigned char			bMatchCount;
	NAND_IO_DEVID			sDeviceCode1, sDeviceCode2;
	NAND_IO_FEATURE			*sTempFeatureInfo;
	NAND_IO_FEATURE			*sFindFeatureInfo;

	bFindMedia 				= FALSE;
	nDevInfo->IoStatus		= 0;
	nDevInfo->ChipNo		= 0xFF;

	// Init Variable
	sTempFeatureInfo	= ( NAND_IO_FEATURE * )NAND_SupportMakerInfo.DevInfo[0];
	sFindFeatureInfo	= ( NAND_IO_FEATURE * )NAND_SupportMakerInfo.DevInfo[0];

	if( !( sNAND_IO_uiPortStatus & NAND_PORT_STATUS_NAND_ON_CS( nChipNo ) ) )
		return NAND_IO_ERROR_GET_DEVICE_INFO;

	//=====================================================================
	// Search Matched NANDFLASH (x8 Bit Serial NAND)
	//=====================================================================
	for( j = 0; j < 3; ++j )	/* Check Read ID during 3 turn */
	{
		#if defined(NFC_VER_50)
		NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_NFC );
		#endif
		// 16Bit NAND Mask Disable
		BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );

		/* Read Device CODE */
		NAND_IO_ResetForReadID( nChipNo, NAND_IO_SERIAL_COMBINATION_MODE );			// x8 Bit NAND Search
		NAND_IO_ReadID( nChipNo, &sDeviceCode1, NAND_IO_SERIAL_COMBINATION_MODE );	// x8 Bit NAND Search

		/* Check Maker ID */
		bFindMakerNo = 0xFF;
		for( k = 0; k < MAX_SUPPORT_MAKER_NAND; ++k )
		{
			if( sDeviceCode1.Code[0] == NAND_SupportMakerInfo.MakerID[k] )
			{
				bFindMakerNo		= ( unsigned char )k;
				sTempFeatureInfo	= ( NAND_IO_FEATURE * )NAND_SupportMakerInfo.DevInfo[k];
				break;
			}
		}

		if( bFindMakerNo >= MAX_SUPPORT_MAKER_NAND )
			continue;

		/* Check Device ID */
		for( k = 0; k < NAND_SupportMakerInfo.MaxSupportNAND[bFindMakerNo]; ++k )
		{
			bMatchCount = 0;

			for( l = 0; l < 5; ++l )
			{
				if( sTempFeatureInfo->DeviceID.Code[l + 1] == _ANY )
					++bMatchCount;
				else if( sDeviceCode1.Code[l + 1] == sTempFeatureInfo->DeviceID.Code[l + 1] )
					++bMatchCount;
			}

			/* Found NAND Device */
			if( bMatchCount >= 5 )
			{
				bFindMedia = TRUE;
				sFindFeatureInfo = sTempFeatureInfo;
				break;
			}
			else
				++sTempFeatureInfo;
		}

		/* Found NAND Device */
		if( bFindMedia == TRUE )
		{
			break;
		}
	}

	if( ( bFindMedia != TRUE ) && ( sNAND_IO_uiPortStatus & NAND_PORT_STATUS_DATA_WIDTH_16BIT ) )
	{
		//=====================================================================
		// Search Matched NANDFLASH (x16 Bit Serial NAND)
		//=====================================================================
		for( j = 0; j < 3; ++j )	/* Check Read ID during 3 turn */
		{
			#if defined(NFC_VER_50)
			NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_NFC );
			#endif
			// 16Bit NAND Mask Enable
			BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );

			/* Read Device CODE */
			NAND_IO_ResetForReadID( nChipNo, NAND_IO_PARALLEL_COMBINATION_MODE );			// x16 Bit NAND Command
			NAND_IO_ReadID( nChipNo, &sDeviceCode1, NAND_IO_PARALLEL_COMBINATION_MODE );	// x16 Bit NAND Search

			/* Check Maker ID */
			bFindMakerNo = 0xFF;
			for( k = 0; k < MAX_SUPPORT_MAKER_NAND; ++k )
			{
				if( sDeviceCode1.Code[0] == NAND_SupportMakerInfo.MakerID[k] )
				{
					bFindMakerNo		= ( unsigned char )k;
					sTempFeatureInfo	= ( NAND_IO_FEATURE * )NAND_SupportMakerInfo.DevInfo[k];
					break;
				}
			}

			#if defined(NFC_VER_50)
			NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_NFC );
			#endif
			if( bFindMakerNo >= MAX_SUPPORT_MAKER_NAND )
			{
				BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );
				continue;
			}
			/* Check Device ID */
			for( k = 0; k < NAND_SupportMakerInfo.MaxSupportNAND[bFindMakerNo]; ++k )
			{
				bMatchCount = 0;

				for( l = 0; l < 5; ++l )
				{
					if( sTempFeatureInfo->DeviceID.Code[l + 1] == _ANY )
						++bMatchCount;
					else if( sDeviceCode1.Code[l + 1] == sTempFeatureInfo->DeviceID.Code[l + 1] )
						++bMatchCount;
				}

				/* Found NAND Device */
				if( bMatchCount >= 5 )
				{
					bFindMedia = TRUE;
					sFindFeatureInfo = sTempFeatureInfo;
					break;
				}
				else
					++sTempFeatureInfo;
			}

			/* Found NAND Device */
			if( bFindMedia == TRUE )
			{
				break;
			}
			else
			{
				BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );
			}
		}
	}

	//=====================================================================
	// If Media is founded
	//=====================================================================
	if( bFindMedia == TRUE )
	{
		/* Get NAND Feature Info */
		NAND_Util_memcpy( ( void * )&nDevInfo->Feature,
		                  ( void * )sFindFeatureInfo,
		                  sizeof( NAND_IO_FEATURE ) );

		/* Get ECC Type Info */
//#if defined(NFC_VER_50) || defined(NFC_VER_200) || defined(NFC_VER_300)
		nDevInfo->EccType = GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType );
		#if defined(NFC_VER_200)
		if(nDevInfo->EccType == ECC_TYPE_8BIT)
			nDevInfo->EccType = ECC_TYPE_12BIT;
		#endif
		if(nDevInfo->EccType == ECC_TYPE_EMBEDDED)
		{
			nDevInfo->EccCodeSize = 0;
			nDevInfo->CodewordSize = nDevInfo->Feature.PageSize;
		}
		else
		{
			const NAND_IO_ECC_INFO *pECCInfo = NAND_IO_GetECCInfo( nDevInfo->EccType );
			if( pECCInfo == NULL )
			{
				ND_TRACE( "Unsupported ECC_TYPE(%d) is queried.\n", nDevInfo->EccType );
				return NAND_IO_ERROR_GET_DEVICE_INFO;
			}
			nDevInfo->EccCodeSize = pECCInfo->ucEccCodeSize;

			if( A_CODEWORD_OF( nDevInfo->Feature.MediaType ) == A_512B )
				nDevInfo->CodewordSize = 512;
			else
				nDevInfo->CodewordSize = 1024;
		}
//#endif

		if( A_BUSWIDTH_OF( nDevInfo->Feature.MediaType ) == A_08BIT )
		{
			if( sNAND_IO_uiPortStatus & NAND_PORT_STATUS_DATA_WIDTH_16BIT )
			{
				/* Check if compositin of NAND is parallel or serial */
				NAND_IO_ResetForReadID( nChipNo, NAND_IO_PARALLEL_COMBINATION_MODE );
				NAND_IO_ReadID( nChipNo, &sDeviceCode2, NAND_IO_PARALLEL_COMBINATION_MODE );

				if( ( ( sDeviceCode2.Code[0] & 0xFF ) == ( ( sDeviceCode2.Code[0] >> 8 ) & 0xFF ) ) &&
				    ( ( sDeviceCode2.Code[1] & 0xFF ) == ( ( sDeviceCode2.Code[1] >> 8 ) & 0xFF ) ) &&
				    ( ( sDeviceCode2.Code[2] & 0xFF ) == ( ( sDeviceCode2.Code[2] >> 8 ) & 0xFF ) ) &&
				    ( ( sDeviceCode2.Code[3] & 0xFF ) == ( ( sDeviceCode2.Code[3] >> 8 ) & 0xFF ) ) &&
				    ( ( sDeviceCode2.Code[4] & 0xFF ) == ( ( sDeviceCode2.Code[4] >> 8 ) & 0xFF ) ) )
				{
					nDevInfo->Feature.MediaType |= A_PARALLEL;
					nDevInfo->Feature.MediaType |= A_16BIT;
					nDevInfo->Feature.PageSize 	*= 2;
					nDevInfo->Feature.SpareSize *= 2;
					nDevInfo->CmdMask = 0xFFFF;
				}
				else
				{
					BITCLR( sNAND_IO_uiPortStatus, NAND_PORT_STATUS_DATA_WIDTH_16BIT );
					nDevInfo->CmdMask = 0x00FF;
				}
			}
			else
			{
				nDevInfo->CmdMask = 0x00FF;
			}
		}
		else
		{
			#if defined(NFC_VER_50)
			NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_NFC );
			#endif
			// 16Bit NAND Mask Enable
			BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );
			nDevInfo->Feature.MediaType |= A_16BIT;
			nDevInfo->CmdMask = 0x00FF;
		}

		nDevInfo->usCodewordsPerPage = ( nDevInfo->Feature.PageSize / nDevInfo->CodewordSize );

		/* Get Shift Factors of BpD, PpB, PageSize, SpareSize, PPages */
		nDevInfo->ShiftPpB = NAND_Util_GetShiftValueForFastMultiPly( nDevInfo->Feature.PpB, 16 );
		nDevInfo->usShiftCodewordsPerPage = NAND_Util_GetShiftValueForFastMultiPly( nDevInfo->usCodewordsPerPage, 16 );
		nDevInfo->ShiftBytesPerSector = NAND_Util_GetShiftValueForFastMultiPly( nDevInfo->CodewordSize, 16 );

		nDevInfo->EccCodeSizeTotalInDword = ( ( nDevInfo->EccCodeSize << nDevInfo->usShiftCodewordsPerPage ) + 3 ) >> 2;

		for( j = 0; j < 4; ++j )
		{
			nDevInfo->BadBlockInfo.BlockStatus[j] = MULTI_PLANE_GOOD_BLOCK;
			nDevInfo->BadBlockInfo.BadBlkPHYAddr[j] = 0xFFFFFFFF;
		}

		NAND_IO_CHANGE_SPARE_ECC_TYPE(nDevInfo, A_CHANGE_SPARE_ECC_OF(nDevInfo->Feature.MediaType)); // Change Spare Bit 
		{
			unsigned int uiSpareSpaceMin = 4/*BadMarker*/ + (nDevInfo->EccCodeSizeTotalInDword<<2) + TNFTL_TAG_BYTE_SIZE + nDevInfo->EccCodeSize;
			if( nDevInfo->Feature.SpareSize < uiSpareSpaceMin )
			{
				ND_TRACE("spare space(=%dB) is not enough!!! %dB is required, at least.\n",nDevInfo->Feature.SpareSize,uiSpareSpaceMin);
				return NAND_IO_ERROR_GET_DEVICE_INFO;
			}
		}
		NAND_IO_CHANGE_SPARE_ECC_TYPE(nDevInfo, 0); //Restore to original ECC type

	}
	//=====================================================================
	// Not Found
	//=====================================================================
	else
	{
		BITCLR( sNAND_IO_uiPortStatus, NAND_PORT_STATUS_NAND_ON_CS( nChipNo ) );

		if( nChipNo == 0 )
		{
			ND_TRACE( "No NAND device found!!! ID:" );
			for( j = 0; j < 6; ++j )
				_ND_TRACE( "0x%x ", ( sDeviceCode1.Code[j] & 0xFF ) );

			_ND_TRACE( "\n" );
		}

		return NAND_IO_ERROR_GET_DEVICE_INFO;
	}

	nDevInfo->IoStatus	= NAND_IO_STATUS_ENABLE;
	nDevInfo->ChipNo	= nChipNo;

	//=====================================================================
	// s_pDevInfo( Global Variable ) Initialize
	//=====================================================================
	if( nDevInfo->ChipNo == 0 )
		s_pDevInfo = ( NAND_IO_DEVINFO * )nDevInfo;

	//=====================================================================
	// Register(&Init) Read Retry Function
	//=====================================================================
	if( A_READ_RETRY_TYPE_OF( nDevInfo->Feature.MediaType ) == A_RR_M )
	{
		nDevInfo->fnSetReadRetry = NAND_IO_Micron_SetReadRetry;
	}
	else if( A_READ_RETRY_TYPE_OF( nDevInfo->Feature.MediaType ) == A_RR_H_26NM )
	{
		nDevInfo->fnSetReadRetry = NAND_IO_Hynix_SetReadRetry_For_26nm;
		NAND_IO_Hynix_GetReadRetryParameter_For_26nm( nDevInfo );
	}
	else if( A_READ_RETRY_TYPE_OF( nDevInfo->Feature.MediaType ) == A_RR_H_20NM )
	{
		nDevInfo->fnSetReadRetry = NAND_IO_Hynix_SetReadRetry_For_20nm;
		NAND_IO_Hynix_GetReadRetryParameter_For_20nm( nDevInfo );
	}
	else if( A_READ_RETRY_TYPE_OF( nDevInfo->Feature.MediaType ) == A_RR_S )
	{
		nDevInfo->fnSetReadRetry = NAND_IO_Samsung_SetReadRetry;
	}

	return NAND_IO_SUCCESS;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_GetFeatureOfNAND( TNFTL_OBJECT_T *pTnftlObj )
{
	unsigned int uiCS_Count = 1;
	memset( &sNAND_IO_DeviceInfo, 0x00, sizeof( sNAND_IO_DeviceInfo ) );
	NAND_IO_Init();
	if( NAND_IO_GetDeviceInfo( 0, &sNAND_IO_DeviceInfo ) != NAND_IO_SUCCESS )
		return FALSE;

	while( uiCS_Count < NAND_BD_Get_CSCountMax() )
	{
		NAND_IO_DEVINFO deviceInfo;
		if( NAND_IO_GetDeviceInfo( uiCS_Count, &deviceInfo ) != NAND_IO_SUCCESS )
			break;
		uiCS_Count++;
	}
	NAND_IO_ASSERT( uiCS_Count == 1 || uiCS_Count == 2 || uiCS_Count == 4 );

#if defined(NAND_FOR_KERNEL) && defined(_UNICODE)
	{
		wchar_t wcBuf[20];
		mbstowcs( wcBuf, sNAND_IO_DeviceInfo.Feature.DeviceName[uiCS_Count>>1], 20 );
		wcBuf[19] = L'\0';
		ND_TRACE( "%s\n", wcBuf );
	}
#else
	ND_TRACE( "%s\n", sNAND_IO_DeviceInfo.Feature.DeviceName[uiCS_Count>>1] );
#endif

	pTnftlObj->pvNandDevice = ( void * )&sNAND_IO_DeviceInfo;
	pTnftlObj->uiCodeWordSize = sNAND_IO_DeviceInfo.CodewordSize;
	pTnftlObj->uiBytesPerPage = sNAND_IO_DeviceInfo.Feature.PageSize;
	pTnftlObj->uiPagesPerBlock = sNAND_IO_DeviceInfo.Feature.PpB;
	pTnftlObj->uiBlocksPerDie = sNAND_IO_DeviceInfo.Feature.BpD;
	if( sNAND_IO_DeviceInfo.Feature.pucLSBPageTable != NULL )
	{
		pTnftlObj->pucMLC_LowerPageTable = sNAND_IO_DeviceInfo.Feature.pucLSBPageTable;
		pTnftlObj->uiSafePagesPerBlock = pTnftlObj->uiPagesPerBlock / 2;
	}
	else
	{
		pTnftlObj->pucMLC_LowerPageTable = NULL;
		pTnftlObj->uiSafePagesPerBlock = pTnftlObj->uiPagesPerBlock;
	}
	pTnftlObj->uiDieCount = uiCS_Count << (sNAND_IO_DeviceInfo.Feature.ucDDPBit ? 1 : 0);
	//pTnftlObj->uiBlocksPerDie = 512;
	pTnftlObj->uiBadBlockCountMax = sNAND_IO_DeviceInfo.Feature.BBpD;
	if( sNAND_IO_DeviceInfo.Feature.UsableFunc & F_MP )
	{
		pTnftlObj->ucPlaneCount = 2;
	}
	else
	{
		pTnftlObj->ucPlaneCount = 1;
	}

	/* set interleaving feature */
	{
		if( sNAND_IO_DeviceInfo.Feature.UsableFunc & F_IL )
		{
			sNAND_IO_DeviceInfo.fInterleaveUsable = TRUE;
			ND_TRACE( "IL(Interleaving) Enabled.\n");
		}
		else
			sNAND_IO_DeviceInfo.fInterleaveUsable = FALSE;

		sNAND_IO_DeviceInfo.ucInterleavingStatus = 0;
	}
			
	switch( sNAND_IO_DeviceInfo.EccType )
	{
		case ECC_TYPE_EMBEDDED:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_EMBEDDED;
			break;
		case ECC_TYPE_4BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_4BIT;
			break;
		case ECC_TYPE_8BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_8BIT;
			break;
		case ECC_TYPE_12BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_12BIT;
			break;
		case ECC_TYPE_16BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_16BIT;
			break;
		case ECC_TYPE_24BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_24BIT;
			break;
		case ECC_TYPE_40BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_40BIT;
			break;
		case ECC_TYPE_60BIT:
			pTnftlObj->usBL1_ECCType = BL1_ECC_TYPE_60BIT;
			break;
		default:
			NAND_IO_ASSERT( 0 );
			break;
	}
	pTnftlObj->usECCCodeSize = sNAND_IO_DeviceInfo.EccCodeSize;
	pTnftlObj->uiBL1_MediaType = 0;
	switch( A_BUSWIDTH_OF( sNAND_IO_DeviceInfo.Feature.MediaType ) )
	{
		case A_08BIT:
			pTnftlObj->uiBL1_MediaType |= BL1_MEDIA_TYPE_08BIT;
			break;
		case A_16BIT:
			pTnftlObj->uiBL1_MediaType |= BL1_MEDIA_TYPE_16BIT;
			break;
		case A_32BIT:
			pTnftlObj->uiBL1_MediaType |= BL1_MEDIA_TYPE_32BIT;
			break;
		default:
			NAND_IO_ASSERT( 0 );
			break;
	}
	if( sNAND_IO_DeviceInfo.Feature.MediaType & A_RAND_IO )
	{
		pTnftlObj->uiBL1_MediaType |= BL1_MEDIA_TYPE_RAND_IO;
	}
	if( A_READ_RETRY_TYPE_OF( sNAND_IO_DeviceInfo.Feature.MediaType ) != 0 )
	{
		pTnftlObj->fReadRetry = TRUE;

#if defined(TCC8925S)|| defined(TCC893X)
		pTnftlObj->fUseLowerPageOnlyForBL1 = TRUE;
#elif defined(TCC801X) || defined(TCC88XX) || defined(TCC892X) || defined(TCC370X) || defined(TCC800X)
		// TCC doesn't support 'Use Lower Page Only for BL1'
#else
		#error
#endif
	}
	if(A_CHANGE_SPARE_ECC_OF(sNAND_IO_DeviceInfo.Feature.MediaType) == A_ECC_SPARE_16BIT)
	{
		pTnftlObj->uiBL1_MediaType |= BL1_MEDIA_TYPE_SPARE_16BIT;
	}
	else if(A_CHANGE_SPARE_ECC_OF(sNAND_IO_DeviceInfo.Feature.MediaType) == A_ECC_SPARE_24BIT)
	{
		pTnftlObj->uiBL1_MediaType |= BL1_MEDIA_TYPE_SPARE_24BIT;
	}
		

	pTnftlObj->ucColumnCycle = sNAND_IO_DeviceInfo.Feature.ucColCycle;
	pTnftlObj->ucRowCycle = sNAND_IO_DeviceInfo.Feature.ucRowCycle;

	// for update the value of s_ReadCycleTime and s_CommCycleTime
	NAND_IO_BusControl( &sNAND_IO_DeviceInfo );

	pTnftlObj->uiReadCycleRegValue = s_ReadCycleTime.RegValue;
	pTnftlObj->uiCommCycleRegValue = s_CommCycleTime.RegValue;

	return TRUE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_SetupDMA( void *pvSrcPhysicalAddress, unsigned uSrcInc, unsigned uSrcMask,
                       void *pvDestPhysicalAddress, unsigned uDstInc, unsigned uDstMask, int nMode, int nDSize )
{
	unsigned		uCHCTRL;
	unsigned int	uTmp;

	if( nMode == NAND_IO_DMA_WRITE )
	{		
		#if defined(NAND_FOR_KERNEL) && defined(_LINUX_)
		wmb();
		#endif
		// uiSrcPhysicalAddress: Buffer Address
		// uiDestPhysicalAddress: NFC_LDATA
#if defined(NFC_VER_50)
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_8 | NFC_CTRL_PAGE_SIZE_512B );	
		BITSET( s_pNFC->NFC_CTRL1, NFC_CTRL1_DMA_ACK_ENABLE );
#elif defined(NFC_VER_100)		//HwNFC_CTRL_DEN_EN: NANDFLASH DMA Request Enable
		BITCSET( s_pNFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_DEN_EN  | HwNFC_CTRL_PSIZE_512 );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)//HwNFC_CTRL_DEN_EN: DMA Acknowledge Enable 
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_8 | NFC_CTRL_PAGE_SIZE_512B );
#endif

		//============================================================
		// DMA Control Register Set
		//============================================================
		uCHCTRL =
		          //HwCHCTRL_SYNC_ON		|
		          //HwCHCTRL_HRD_W			|
		          HwCHCTRL_BST_BURST		|
		          HwCHCTRL_TYPE_SINGL		|
		          HwCHCTRL_HRD_WR			|
		          HwCHCTRL_BSIZE_8		|
		          HwCHCTRL_WSIZE_32		|
		          HwCHCTRL_FLAG			|
		          HwCHCTRL_EN_ON			|
		          0;

	}
	else	// NAND_IO_DMA_READ
	{
		#if defined(NAND_FOR_KERNEL) && defined(_LINUX_)
		rmb();
		#endif
		// uiSrcPhysicalAddress: NFC_LDATA
		// uiDestPhysicalAddress: Buffer Address
#if defined(NFC_VER_50)
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_8 | NFC_CTRL_PAGE_SIZE_512B );	
		BITSET( s_pNFC->NFC_CTRL1, NFC_CTRL1_DMA_ACK_ENABLE );
#elif defined(NFC_VER_100)
		BITCSET( s_pNFC->NFC_CTRL, HwNFC_CTRL_BSIZE_8, HwNFC_CTRL_BSIZE_8 | HwNFC_CTRL_DEN_EN  | HwNFC_CTRL_PSIZE_512 );
#elif defined(NFC_VER_200) || defined(NFC_VER_300) 
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_8 | NFC_CTRL_PAGE_SIZE_512B );
#endif

		//============================================================
		// DMA Control Register Set
		//============================================================
		uCHCTRL =
		    //			HwCHCTRL_SYNC_ON		|
		    //			HwCHCTRL_HRD_W			|
		    HwCHCTRL_BST_BURST		|
		    HwCHCTRL_TYPE_SINGL		|
		    HwCHCTRL_HRD_RD			|
		    HwCHCTRL_BSIZE_8		|
		    HwCHCTRL_WSIZE_32		|
		    HwCHCTRL_FLAG			|
		    HwCHCTRL_EN_ON			|
		    0;
	}

	//============================================================
	// Set Source Address & Source Parameter (mask + increment)
	//============================================================
	pNAND_DMA->ST_SADR 	= (unsigned int)pvSrcPhysicalAddress;
	pNAND_DMA->SPARAM[0] = ( uSrcInc | ( uSrcMask << 4 ) );

	//============================================================
	// Set Dest Address & Dest Parameter (mask + increment)
	//============================================================
	pNAND_DMA->ST_DADR 	= (unsigned int)pvDestPhysicalAddress;
	pNAND_DMA->DPARAM[0] = ( uDstInc | ( uDstMask << 4 ) );

	//============================================================
	// Calculate byte size per 1 Hop transfer
	//============================================================
	uTmp	= ( uCHCTRL & ( Hw5 + Hw4 ) ) >> 4;			// calc log2(word size)
	uTmp	= uTmp + ( ( uCHCTRL & ( Hw7 + Hw6 ) ) >> 6 );	// calc log2(word * burst size)

	//============================================================
	// Set External DMA Request Register
	//============================================================
	pNAND_DMA->EXTREQ = Hw18;

	//============================================================
	// Set Hcount
	//============================================================
	if( uTmp )
		pNAND_DMA->HCOUNT	= ( nDSize + ( 1 << uTmp ) - 1 ) >> uTmp;
	else
		pNAND_DMA->HCOUNT	= nDSize;

	//============================================================
	// Set & Enable DMA
	//============================================================
	pNAND_DMA->CHCTRL		= uCHCTRL;
	s_pNFC->NFC_DSIZE		= nDSize;

	//============================================================
	// DMA Transfer Start
	//============================================================
	if( nMode == NAND_IO_DMA_WRITE )
	{
#if defined(NFC_VER_50)
		s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG;
		s_pNFC->NFC_PSTART	= 0;
		while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );
		if( s_pNFC->NFC_CTRL1 & NFC_CTRL1_DMA_ACK_ENABLE )
			BITCLR( s_pNFC->NFC_CTRL1, NFC_CTRL1_DMA_ACK_ENABLE );
#elif defined(NFC_VER_100)
		if( s_pNFC->NFC_CTRL1 & HwNFC_CTRL1_DACK_EN )
			BITCLR( s_pNFC->NFC_CTRL1, HwNFC_CTRL1_DACK_EN );
		s_pNFC->NFC_IRQ = HwNFC_IRQ_PFG;
		s_pNFC->NFC_PSTART	= 0;
		while( ISZERO( s_pNFC->NFC_IRQ, HwNFC_IRQ_PFG ) );
		if( s_pNFC->NFC_CTRL1 & HwNFC_CTRL1_ARB_EN )
			BITSET( s_pNFC->NFC_CTRL1, HwNFC_CTRL1_DACK_EN );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_PROGRAM );

		#if defined(__USE_NAND_ISR__)
		if( NAND_IO_IRQ_IsEnabled() )
		{
			s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG;
			s_pNFC->NFC_IRQCFG 	|= NFC_IRQCFG_PROGRAM_INTR_ENABLE; // [IRQ]Set Write Interrput Enable			
			s_pNFC->NFC_PRSTART	= 0;
		}
		else
		#endif
		{
			s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG;
			s_pNFC->NFC_PRSTART	= 0;
			while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );
		}
#endif
	}
	else
	{
#if defined(NFC_VER_50)
		s_pNFC->NFC_IRQ = NFC_IRQ_READ_FLAG;
		s_pNFC->NFC_RSTART	= 0;
		while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_READ_FLAG ) );
		if( s_pNFC->NFC_CTRL1 & NFC_CTRL1_DMA_ACK_ENABLE )
			BITCLR( s_pNFC->NFC_CTRL1, NFC_CTRL1_DMA_ACK_ENABLE );
#elif defined(NFC_VER_100)
		s_pNFC->NFC_IRQ = HwNFC_IRQ_RDFG;
		s_pNFC->NFC_RSTART	= 0;
		while( ISZERO( s_pNFC->NFC_IRQ, HwNFC_IRQ_RDFG ) );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_PROGRAM );

		#if defined(__USE_NAND_ISR__)
		if( NAND_IO_IRQ_IsEnabled() )
		{
			s_pNFC->NFC_IRQ 	= NFC_IRQ_READ_FLAG;
			s_pNFC->NFC_IRQCFG |= NFC_IRQCFG_READ_INTR_ENABLE;
			s_pNFC->NFC_PRSTART	= 0;		
		}
		else
		#endif
		{
			s_pNFC->NFC_IRQ = NFC_IRQ_READ_FLAG;
			s_pNFC->NFC_PRSTART	= 0;
			while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_READ_FLAG ) );
		}
#endif
	}
}

static void NAND_IO_WriteSpareArea( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiSpareBuffer, unsigned int uiTagSize )
{
	unsigned int		i;
	unsigned int		uiTag[TNFTL_TAG_DWORD_SIZE];
	unsigned int		uiAllTagSize = uiTagSize;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_NDMA_MODE )
	{
		if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
		{
			//----------------------------------------------
			//	Send Bad Block Mark (4Byte)
			//----------------------------------------------
			s_pNFC->NFC_WDATA = 0xFFFFFFFF;		// Keep Factory Bad Marker
		}
	}
	else
	{
		//----------------------------------------------
		//	Send Bad Block Mark (4Byte)
		//----------------------------------------------
		s_pNFC->NFC_WDATA = 0xFFFFFFFF;		// Keep Factory Bad Marker

		//==============================================================
		// Write Data ECC
		//==============================================================
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			unsigned int *pulSharedEccBuf = (unsigned int*)&sNAND_IO_uiSharedEccBuffer[0];

			//----------------------------------------------
			//	Write ECC Code for Data
			//----------------------------------------------
			i = nDevInfo->EccCodeSizeTotalInDword;
			do
			{
				s_pNFC->NFC_WDATA = *pulSharedEccBuf++;
			} while( --i );
		}
	}

	//==============================================================
	// Prepare Tag
	//==============================================================
	if(nDevInfo->Feature.MediaType & A_RAND_IO)
	{
		TNFTL_RAND_SetAddress( (unsigned int)nDevInfo->Feature.PpB, uiPageAddress, (unsigned int)nDevInfo->Feature.PageSize );
		TNFTL_RAND_Randomize( (void *)uiTag, (const void *)puiSpareBuffer, uiAllTagSize );
	}
	else
	{
		NAND_Util_memcpy(uiTag, puiSpareBuffer, uiAllTagSize);
	}

	//==============================================================
	// Write Tag
	//==============================================================
	NAND_IO_CHANGE_SPARE_ECC_TYPE(nDevInfo, A_CHANGE_SPARE_ECC_OF(nDevInfo->Feature.MediaType)); 

#if defined(NFC_VER_50)
	if( !( s_pNFC->NFC_CTRL1 & NFC_CTRL1_DMA_ACK_ENABLE ) )
#elif defined(NFC_VER_100)
	if( !( s_pNFC->NFC_CTRL1 & HwNFC_CTRL1_ARB_EN ) )
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	if( !( s_pNFC->NFC_CTRL & NFC_CTRL_DMA_ACK_ENABLE ) )
#endif
	{
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			/* Setup ECC Block */
#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
#else
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
#endif
			s_pECC->ECC_CTRL	|= ( uiAllTagSize << ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	= 0x00000000;
		}

		/* Write 12Bytes Tag */
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_1 );	// 1R/W Burst Size
		s_pNFC->NFC_DSIZE		= uiAllTagSize;								// NFC DSIZE SETTING
#if defined(NFC_VER_200) || defined(NFC_VER_300)
		s_pNFC->NFC_CTRL 	 |= NFC_CTRL_BURST_PROGRAM;
#endif

#if defined(NFC_VER_50)
		s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG;
		s_pNFC->NFC_PSTART  = 0;
#elif defined(NFC_VER_100)
		s_pNFC->NFC_IRQ = HwNFC_IRQ_PFG;
		s_pNFC->NFC_PSTART  = 0;
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG | NFC_IRQ_SPARE_BURST_WR_FLAG;
		s_pNFC->NFC_PRSTART = 0;
#endif

		for( i = 0 ; i < uiAllTagSize>>2 ; i++ )
		{
#if defined(NFC_VER_50)
			while( !( s_pNFC->NFC_CTRL & NFC_CTRL_FIFO_READY_TO_BURST_ACCESS ) );
#elif defined(NFC_VER_100)
			while( !( s_pNFC->NFC_CTRL & HwNFC_CTRL_FS_RDY ) );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
			while( !( s_pNFC->NFC_STATUS & NFC_STATUS_FIFO_READY_TO_BURST_ACCESS ) );
#endif

			s_pNFC->NFC_LDATA = uiTag[i];
		}

#if defined(NFC_VER_50)
		while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );
#elif defined(NFC_VER_100)
		while( ISZERO( s_pNFC->NFC_IRQ, HwNFC_IRQ_PFG ) );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );
#endif
	}
	else
	{
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			/* Setup ECC Block */
#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif
			s_pECC->ECC_CTRL	|= ( uiAllTagSize << ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	= 0x00000000;
		}

		for( i = 0 ; i < uiAllTagSize>>2 ; i++ )
		{
			s_pNFC->NFC_WDATA = uiTag[i];
		}
	}

	//==============================================================
	// Write Tag ECC
	//==============================================================
	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
	{
		unsigned long		ulTagEccBuffer[ECC_DATA_MAX_DWORD];
		unsigned int		uiEccDataSizeInDword;

		// Clear ECC Buffer
		memset( &ulTagEccBuffer[0], 0xFF, sizeof( ulTagEccBuffer ) );

		/*	Load ECC code from ECC block */
		NAND_IO_EncodeECC( nDevInfo->EccType, ulTagEccBuffer, nDevInfo->EccCodeSize );

		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

		/* Write ECC data */
		uiEccDataSizeInDword = ( nDevInfo->EccCodeSize + 3 ) >> 2;
		for( i = 0 ; i < uiEccDataSizeInDword ; i++ )
		{
			s_pNFC->NFC_WDATA = ulTagEccBuffer[i];
		}
	}

	NAND_IO_CHANGE_SPARE_ECC_TYPE(nDevInfo, 0); //Restore to original ECC type

}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_WriteDataAndSpareArea( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer, unsigned int uiTagSize )
{
	unsigned int		j;
	unsigned char		*pucSharedEccBuf = 0;
	unsigned int 		*puiDmaBuffer, *puiDmaPhysicalBuffer;

	#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
	sigset_t		sigmask;	
	unsigned int 	uiCheckSignal=FALSE;	
	
	sigemptyset(&sigmask);
	sigfillset(&sigmask);
	#endif

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	puiDmaBuffer = s_puiDMA_WorkBuffer;
	puiDmaPhysicalBuffer = s_puiDMA_PhyBuffer;

	/*
	if( (nDevInfo->Feature.MediaType & A_RAND_IO) && !nDevInfo->fSkipDataAreaRandomizing )
	{
		TNFTL_RAND_SetAddress( (unsigned int)nDevInfo->Feature.PpB, uiPageAddress, 0 );
		//TNFTL_RAND_Randomize( (void *)puiDmaBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
		TNFTL_RAND_Randomize( (void *)sNAND_IO_uiRandomizeBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
		NAND_Util_memcpy((void *)puiDmaBuffer, (const void *)sNAND_IO_uiRandomizeBuffer, (unsigned int)nDevInfo->Feature.PageSize );
	}
	else
	{
		NAND_Util_memcpy( (void *)puiDmaBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
	}
	*/

	// Set SpareBuffer Pointer =>> ECCBuffer
	memset( &sNAND_IO_uiSharedEccBuffer[0], 0xFF, sizeof( sNAND_IO_uiSharedEccBuffer ) );
	pucSharedEccBuf = ( unsigned char * )&sNAND_IO_uiSharedEccBuffer[0];

	#if defined(__USE_NAND_ISR__)
	if(NAND_IO_IRQ_IsEnabled())
	{
		int					ret;

		gsNAND_IO_NandIsrInfo.pNandIoDevInfo 	= nDevInfo;
		gsNAND_IO_NandIsrInfo.pbPageBuffer 		= (unsigned char*)puiDmaBuffer;
		gsNAND_IO_NandIsrInfo.pbPhyPageBuffer	= (unsigned char*)puiDmaPhysicalBuffer;
		gsNAND_IO_NandIsrInfo.pbEccBuffer 		= pucSharedEccBuf;
		gsNAND_IO_NandIsrInfo.usStartPPage		= 0;
		gsNAND_IO_NandIsrInfo.usEndPPage		= nDevInfo->usCodewordsPerPage;
		gsNAND_IO_NandIsrInfo.usCurrentPPage	= 0;
		gsNAND_IO_NandIsrInfo.usPPagesLeft 		= nDevInfo->usCodewordsPerPage;
		gsNAND_IO_NandIsrInfo.iEccOnOff 		= ECC_ON;
		gsNAND_IO_NandIsrInfo.wait_complete 	= 0;
		gsNAND_IO_NandIsrInfo.uiState 			= NAND_IO_IRQ_STATE_WRITE;
		gsNAND_IO_NandIsrInfo.error 			= SUCCESS;
		gsNAND_IO_NandIsrInfo.ubIsRun 			= TRUE;

		gsNAND_IO_NandIsrInfo.usPPagesLeft--;
		gsNAND_IO_NandIsrInfo.usCurrentPPage++;

		if ( gsNAND_IO_NandIsrInfo.iEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
			#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (unsigned long)&NAND_IO_HwLDATA_PA );
			#else
			NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, (unsigned long)&pNFC->NFC_LDATA );
			#endif
			s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize<< ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	= 0x00000000;
		}

		NAND_IO_SetupDMA( ( void * )puiDmaPhysicalBuffer, 4, 0,
			              ( void * )&NAND_IO_HwLDATA_PA, 0, 0,
			              NAND_IO_DMA_WRITE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->CodewordSize );

		NAND_IO_ISR_Control( ENABLE );	// Enable Global Interrupt for NFC
		do
		{
			#if 1 //defined(TCC893X)
			//ret = wait_event_interruptible_locked_irq(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete);
			ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );				
			#else
			ret = wait_event_interruptible(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete);
			//ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );				
			#endif

			if( ret > 0 )
			{
				break;
			}
			else
			{
				//if( iIsr_Ret == 0 )  // this option used for  wait_event_interruptible_locked_irq
				if( gsNAND_IO_NandIsrInfo.wait_complete )  // this option used for wait_event_interruptible_timeout
				{
					if( uiCheckSignal == TRUE )
						sigprocmask( SIG_UNBLOCK, &sigmask, NULL );

					break;				
				}
				else if( abs(ret) == ERESTARTSYS )
				{
					uiCheckSignal = TRUE;				
					sigprocmask( SIG_BLOCK, &sigmask, NULL );
					ND_TRACE(" Write Interrupt Wakeup - go sleep again\n");
				}
				else
				{
					ND_TRACE("nand:wait_event_interruptible error!(ret=%d)\n",ret);
				}
			}
		} while(1) ;

		gsNAND_IO_NandIsrInfo.uiState = NAND_IO_IRQ_STATE_NONE;
		gsNAND_IO_NandIsrInfo.ubIsRun = FALSE;
	}
	else
	#endif
	{

		//----------------------------------------------
		//	Write Data as Codeword repeatly
		//----------------------------------------------
		for( j = 0; j < nDevInfo->usCodewordsPerPage ; ++j )
		{
			//####################################################
			//#	Write Codeword
			//####################################################
			//----------------------------------------------
			//	MCU ACCESS
			//----------------------------------------------
			if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_MCU_MODE ) 
			{
				#if defined(NFC_VER_50)
				if( !( s_pNFC->NFC_CTRL1 & NFC_CTRL1_DMA_ACK_ENABLE ) )
				#elif defined(NFC_VER_200) || defined(NFC_VER_300)
				if( !( s_pNFC->NFC_CTRL & NFC_CTRL_DMA_ACK_ENABLE ) )
				#endif
				{
					unsigned int i;
					if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
					{
						/* Setup ECC Block */
						#if defined(_WINCE_) || defined(_LINUX_)
						NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
						#else
						NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
						#endif
						s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
						s_pECC->ECC_CLEAR	= 0x00000000;
					}

					/* Write 512 Data Area */
					BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_1 );	// 1R/W Burst Size
					s_pNFC->NFC_DSIZE		= nDevInfo->CodewordSize;					// NFC DSIZE SETTING


					#if defined(NFC_VER_50)
					s_pNFC->NFC_IRQ	= NFC_IRQ_PROGRAM_FLAG;
					s_pNFC->NFC_PSTART	= 0;
					#elif defined(NFC_VER_200) || defined(NFC_VER_300)
					s_pNFC->NFC_CTRL	|= NFC_CTRL_BURST_PROGRAM;
					s_pNFC->NFC_IRQ		= NFC_IRQ_PROGRAM_FLAG;
					s_pNFC->NFC_PRSTART = 0;
					#endif

					i = ( nDevInfo->CodewordSize >> 2 );
					do
					{
						#if defined(NFC_VER_50)
						while( !( s_pNFC->NFC_CTRL & NFC_CTRL_FIFO_READY_TO_BURST_ACCESS ) );
						#elif defined(NFC_VER_200) || defined(NFC_VER_300)
						while( !( s_pNFC->NFC_STATUS & NFC_STATUS_FIFO_READY_TO_BURST_ACCESS ) );
						#endif

						s_pNFC->NFC_LDATA = *puiDmaBuffer++;
					} while( --i );

					while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );

				}
				else
				{
					unsigned int i;
					if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
					{
						/* Setup ECC Block */
						#if defined(_WINCE_) || defined(_LINUX_)
						NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
						#else
						NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
						#endif
						
						s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
						s_pECC->ECC_CLEAR	= 0x00000000;

						// Is it Need?
						#if defined(NFC_VER_200) || defined(NFC_VER_300)
						s_pNFC->NFC_CTRL		|= NFC_CTRL_ECC_ENABLE;		// NFC ECC Encode/Decode Enable
						#endif
					}

					i = ( nDevInfo->CodewordSize >> 2 );
					do
					{
						s_pNFC->NFC_WDATA = *puiDmaBuffer++;
					} while( --i );
				}
			}
			else if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_GDMA_MODE )
			{
				//----------------------------------------------
				//	DMA ACCESS
				//----------------------------------------------
				if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
					#else
					NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_DMA_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
					#endif
					s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
					s_pECC->ECC_CLEAR	= 0x00000000;
				}

				#if defined(_LINUX_) || defined(_WINCE_)
				NAND_IO_SetupDMA( ( void * )puiDmaPhysicalBuffer, 4, 0,
				                  ( void * )&NAND_IO_HwLDATA_PA, 0, 0,
				                  NAND_IO_DMA_WRITE, nDevInfo->CodewordSize );
				#else
				NAND_IO_SetupDMA( ( void * )puiDmaPhysicalBuffer, 4, 0,
				                  ( void * )&s_pNFC->NFC_LDATA, 0, 0,
				                  NAND_IO_DMA_WRITE, nDevInfo->CodewordSize );
				#endif

				puiDmaPhysicalBuffer = &puiDmaPhysicalBuffer[nDevInfo->CodewordSize >> 2];
			}
			//####################################################
			//####################################################

			/*	Load ECC code from ECC block */
			if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
			{
				NAND_IO_EncodeECC( nDevInfo->EccType, pucSharedEccBuf, nDevInfo->EccCodeSize );
			}
			pucSharedEccBuf = &pucSharedEccBuf[nDevInfo->EccCodeSize];
		}
	}

	//=========================================================================
	// Write Spare Data
	//=========================================================================
	/* Change Cycle */
	NAND_IO_SetWriteCycleTime();

	NAND_IO_WriteSpareArea( nDevInfo, uiPageAddress, puiSpareBuffer, uiTagSize );
}

#if defined(CHECK_PROGRAM_SEQ)
static PROGRAM_TYPE_T s_ePgmType[4] = {NORMAL_PROGRAM,NORMAL_PROGRAM,NORMAL_PROGRAM,NORMAL_PROGRAM};
static unsigned int s_uiPageAddress[4] = {(unsigned int)-1,(unsigned int)-1,(unsigned int)-1,(unsigned int)-1};
static void NAND_IO_CheckProgramSequence(NAND_IO_DEVINFO *nDevInfo,PROGRAM_TYPE_T eProgramType,unsigned int uiPageAddress)
{
	PROGRAM_TYPE_T ePrevPgmType = s_ePgmType[nDevInfo->ucDieIndex];
	unsigned int uiPrevPageAddr = s_uiPageAddress[nDevInfo->ucDieIndex];
//if(eProgramType != PROGRAM_END)
//_ND_TRACE("[%d:%d:%d]",nDevInfo->ucDieIndex,uiPageAddress,eProgramType);
	if( ePrevPgmType == CACHE_PROGRAM )
	{
		if(uiPrevPageAddr + 1 != uiPageAddress)
		{
			ND_TRACE("prevCP addr: %d %d",uiPrevPageAddr,uiPageAddress);
			_ND_TRACE(" die=%d\n",nDevInfo->ucDieIndex);
		}

		if(eProgramType != CACHE_PROGRAM && eProgramType != CACHE_LAST_PROGRAM)
		{
			ND_TRACE("prevCP pgm: %d",eProgramType);
			_ND_TRACE(" die=%d\n",nDevInfo->ucDieIndex);
		}
	}
	else if( ePrevPgmType == MULTI_PLANE_PROGRAM )
	{
		if(uiPrevPageAddr + nDevInfo->Feature.PpB != uiPageAddress)
		{
			ND_TRACE("prevMP addr: %d %d",uiPrevPageAddr,uiPageAddress);
			_ND_TRACE(" die=%d\n",nDevInfo->ucDieIndex);
		}

		if(eProgramType != MULTI_PLANE_LAST_PROGRAM)
		{
			ND_TRACE("prevMP pgm: %d",eProgramType);
			_ND_TRACE(" die=%d\n",nDevInfo->ucDieIndex);
		}
	}

	if( eProgramType == CACHE_PROGRAM )
	{
		if( (unsigned short)( uiPageAddress & (nDevInfo->Feature.PpB - 1) ) == (unsigned short)(nDevInfo->Feature.PpB - 1) )
		{
			ND_TRACE("CP cur_addr: %d",uiPageAddress);
			_ND_TRACE(" die=%d\n",nDevInfo->ucDieIndex);
		}
	}
	else if(eProgramType==PROGRAM_END)
	{
		if(ePrevPgmType == CACHE_PROGRAM
			|| ePrevPgmType == MULTI_PLANE_PROGRAM)
		{
			ND_TRACE("END prev_pgm: %d",ePrevPgmType);
			_ND_TRACE(" die=%d\n",nDevInfo->ucDieIndex);
		}
	}

	s_ePgmType[nDevInfo->ucDieIndex] = eProgramType;
	s_uiPageAddress[nDevInfo->ucDieIndex] = uiPageAddress;
}

void NAND_IO_CheckEndOfProgram(unsigned int uiDieCount)
{
	unsigned int i;
	for(i=0;i<uiDieCount;i++)
	{
		PROGRAM_TYPE_T ePrevPgmType = s_ePgmType[i];
		if( ePrevPgmType == CACHE_PROGRAM
			|| ePrevPgmType == MULTI_PLANE_PROGRAM )
			ND_TRACE("EOP(%d): %d\n",i,ePrevPgmType);
	}
}
#endif

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T _NAND_IO_WritePage( NAND_IO_DEVINFO *nDevInfo, PROGRAM_TYPE_T eProgramType,
		unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer, unsigned int uiTagSize )
{
	unsigned int		RowAddr, ColumnAddr;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	#if defined(NAND_FOR_KERNEL) && defined(_LINUX_)
	wmb();
	#endif

	//NAND_IO_CheckProgramSequence(nDevInfo,eProgramType,uiPageAddress);
	if( NAND_BD_Disable_WP_Port() != TRUE )
		return NAND_IO_ERROR_WRITE_PROTECTED;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	//ND_TRACE("[W:%d]: %d\t(B:%d,\t%d)\n", nDevInfo->ucDieIndex, uiPageAddress&(~(0x1<<20)), (uiPageAddress&(~(0x1<<20)))>>nDevInfo->ShiftPpB, uiPageAddress&(nDevInfo->Feature.PpB-1));
	/* memcpy first */
	if( (nDevInfo->Feature.MediaType & A_RAND_IO) && !nDevInfo->fSkipDataAreaRandomizing )
	{
		TNFTL_RAND_SetAddress( (unsigned int)nDevInfo->Feature.PpB, uiPageAddress, 0 );
		//TNFTL_RAND_Randomize( (void *)s_puiDMA_WorkBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
		TNFTL_RAND_Randomize( (void *)sNAND_IO_uiRandomizeBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
		NAND_Util_memcpy( (void *)s_puiDMA_WorkBuffer, (const void *)sNAND_IO_uiRandomizeBuffer, (unsigned int)nDevInfo->Feature.PageSize );
	}
	else
	{
		NAND_Util_memcpy( (void *)s_puiDMA_WorkBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
	}
	
	if( nDevInfo->fInterleaveUsable )	/* if it can use interleaving */
		res = NAND_IO_WaitBusyForInterleave( nDevInfo );

	if( res == NAND_IO_SUCCESS )
	{
		/* Generate Row and Column Address */
		NAND_IO_GenerateRowColAddr( uiPageAddress, 0, &RowAddr, &ColumnAddr, nDevInfo );

		/* Command Page Program #1 */
		{
			unsigned int uiCommand;
			switch(eProgramType)
			{
				case MULTI_PLANE_LAST_PROGRAM:
					if( nDevInfo->Feature.DeviceID.Code[0] == SAMSUNG_NAND_MAKER_ID
						|| nDevInfo->Feature.DeviceID.Code[0] == TOSHIBA_NAND_MAKER_ID
						|| nDevInfo->Feature.DeviceID.Code[0] == HYNIX_NAND_MAKER_ID
						|| nDevInfo->Feature.DeviceID.Code[0] == SPANSION_NAND_MAKER_ID
						|| nDevInfo->Feature.DeviceID.Code[0] == EMST_NAND_MAKER_ID )
					{
						uiCommand = 0x8181;
					}
					else
					{
						uiCommand = 0x8080;
					}
					break;

				default:
					uiCommand = 0x8080;
					break;
			}
			s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;
		}

		/* Write Row and Column Address */
		NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, nDevInfo );

		/* Change Cycle */
		NAND_IO_SetWriteCycleTime();

		/* Write Data to NAND FLASH */
		#if defined(NAND_SUPPORT_NDMA)
		if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_NDMA_MODE )
		{
			
			NAND_IO_NDMA_WriteDataAndSpareArea( nDevInfo,
		                                  uiPageAddress,
		                                  puiPageBuffer,
		                                  puiSpareBuffer,
		                                  uiTagSize );
		}
		else
		#endif
		{
			NAND_IO_WriteDataAndSpareArea( nDevInfo,
		                             uiPageAddress,
		                             puiPageBuffer,
		                             puiSpareBuffer,
		                             uiTagSize );
		}

		/* Change Cycle */
		NAND_IO_SetCommCycleTime();

		s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

		#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
		if( ( s_fIsNAND_RB_ISR_USED == ENABLE ) && ( eProgramType != MULTI_PLANE_PROGRAM ))
		{
			switch(eProgramType)
			{
				case CACHE_PROGRAM:
					NAND_IO_RB_ISR( nDevInfo, NAND_IO_WRITE_CACHE_MODE );
					break;
				default:
					NAND_IO_RB_ISR( nDevInfo, NAND_IO_WRITE_NORMAL_MODE );
					break;
			}		
		}
		else
		#endif
		{
		/* Command Page Program #2 */
			unsigned int uiCommand;
			switch(eProgramType)
			{
				case CACHE_PROGRAM:
					uiCommand = 0x1515;
					break;

				case MULTI_PLANE_PROGRAM:
					uiCommand = 0x1111;
					break;

				default:
					uiCommand = 0x1010;
					break;
			}
			s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;
		}

		if( !sNAND_IO_DeviceInfo.fInterleaveUsable )	/* if it can not use interleaving */
		{
			NAND_IO_WaitBusy( nDevInfo->ChipNo );

			switch(eProgramType)
			{
				case CACHE_PROGRAM:
					res = NAND_IO_ReadStatusOfCacheProgram(nDevInfo,0);
					break;
				case CACHE_LAST_PROGRAM:
					res = NAND_IO_ReadStatusOfCacheProgram(nDevInfo,1);
					break;

				case MULTI_PLANE_PROGRAM:
					break;
				case MULTI_PLANE_LAST_PROGRAM:
					res = NAND_IO_ReadStatusOfMultiPlane(nDevInfo);
					break;

				default:
					res = NAND_IO_ReadStatus(nDevInfo);
					break;
			}
		}
		else	/* if it can use interleaving */
		{
			switch(eProgramType)
			{
				case NORMAL_PROGRAM:
				case MULTI_PLANE_LAST_PROGRAM:
					NAND_IO_SetInterleavingStatus( nDevInfo, uiPageAddress, NAND_IO_WRITE );
					break;
				
				default:
					break;
			}

		}

		NAND_IO_DisableChipSelect();
		NAND_IO_PostProcess();
	}

	if( res == NAND_IO_SUCCESS )
	{
		switch(eProgramType)
		{
			case CACHE_PROGRAM:
			case MULTI_PLANE_PROGRAM:
				break;
			default:
				NAND_BD_Enable_WP_Port();
				break;
		}
	}
	else
	{
		ND_ERROR("WritePage Failure!!! (PPA=%d,res=%x)\n",uiPageAddress,res);
		NAND_BD_Enable_WP_Port();
	}

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPage )
{
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;

	if( !( sNAND_IO_DeviceInfo.IoStatus & NAND_IO_STATUS_ENABLE ) )
		return FALSE;

	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, &sNAND_IO_DeviceInfo, (unsigned int*)&uiPageAddress);

	if( sNAND_IO_DeviceInfo.Feature.UsableFunc & F_CP && !sNAND_IO_DeviceInfo.fInterleaveUsable )/*It's support cache program*/
	{
		if( fNextPage /*It has data for next program*/
			&& (uiPageAddress&(pTnftlObj->uiPagesPerBlock-1)) != (pTnftlObj->uiPagesPerBlock-1) /*It isn't last page of a block*/ )
		{
			res = _NAND_IO_WritePage(&sNAND_IO_DeviceInfo,CACHE_PROGRAM,uiPageAddress,puiBuffer,puiTag,uiTagSize);
			if( res == NAND_IO_SUCCESS )
				sNAND_IO_DeviceInfo.fCacheProgramed[sNAND_IO_DeviceInfo.ucDieIndex] = TRUE;
		}
		else if( sNAND_IO_DeviceInfo.fCacheProgramed[sNAND_IO_DeviceInfo.ucDieIndex] )
		{
			res = _NAND_IO_WritePage(&sNAND_IO_DeviceInfo,CACHE_LAST_PROGRAM,uiPageAddress,puiBuffer,puiTag,uiTagSize);
			sNAND_IO_DeviceInfo.fCacheProgramed[sNAND_IO_DeviceInfo.ucDieIndex] = FALSE;
		}
		else
		{
			res = _NAND_IO_WritePage(&sNAND_IO_DeviceInfo,NORMAL_PROGRAM,uiPageAddress,puiBuffer,puiTag,uiTagSize);
		}
	}
	else
	{
		res = _NAND_IO_WritePage(&sNAND_IO_DeviceInfo,NORMAL_PROGRAM,uiPageAddress,puiBuffer,puiTag,uiTagSize);
	}

	if( res != NAND_IO_SUCCESS )
	{
		res = NAND_IO_ERROR_WRITE_FAILURE;
	}

	return (res == NAND_IO_SUCCESS)? TRUE : FALSE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_MultiPlane_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize )
{
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	if( !( sNAND_IO_DeviceInfo.IoStatus & NAND_IO_STATUS_ENABLE ) )
		return FALSE;

	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, &sNAND_IO_DeviceInfo, (unsigned int*)&uiPageAddress);

	res = _NAND_IO_WritePage(&sNAND_IO_DeviceInfo,MULTI_PLANE_PROGRAM,uiPageAddress,puiBuffer,puiTag, uiTagSize);

	if( res != NAND_IO_SUCCESS )
	{
		res = NAND_IO_ERROR_WRITE_FAILURE;
	}

	return (res == NAND_IO_SUCCESS)? TRUE : FALSE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_MultiPlane_WriteLastPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize )
{
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;

	if( !( sNAND_IO_DeviceInfo.IoStatus & NAND_IO_STATUS_ENABLE ) )
		return FALSE;

	NAND_IO_Update_CSNum_And_PageAddr( pTnftlObj, &sNAND_IO_DeviceInfo, (unsigned int*)&uiPageAddress );

	res = _NAND_IO_WritePage( &sNAND_IO_DeviceInfo, MULTI_PLANE_LAST_PROGRAM, uiPageAddress, puiBuffer, puiTag, uiTagSize );
	if( res != NAND_IO_SUCCESS )
	{
		res = NAND_IO_ERROR_WRITE_FAILURE;
	}

	return (res == NAND_IO_SUCCESS)? TRUE : FALSE;
}

static void NAND_IO_Send_ReadCommand( NAND_IO_DEVINFO *pDevInfo , unsigned int uiPageAddress, unsigned int uiColumnAddr )
{
	unsigned int RowAddr = 0, ColumnAddr = 0;

	/* Generate Row and Column Address */
	NAND_IO_GenerateRowColAddr( uiPageAddress, uiColumnAddr, &RowAddr, &ColumnAddr, &sNAND_IO_DeviceInfo );

	/* Command READ [ 0x00 ] */
	s_pNFC->NFC_CMD = 0x0000;

	/* Write Row and Column Address	*/
	NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, &sNAND_IO_DeviceInfo );

	/* Command READ2 [ 0x30 ] for Advance NandFlash */
	s_pNFC->NFC_CMD = sNAND_IO_DeviceInfo.CmdMask & 0x3030;
}

static unsigned char sNAND_IO_ucErrorBitCount;
__INLINE void NAND_IO_ResetEccErrorBitCount(void)
{
	sNAND_IO_ucErrorBitCount = 0;
}

__INLINE void NAND_IO_SetEccErrorBitCount(unsigned char ucErrorBitCount)
{
	if( ucErrorBitCount > sNAND_IO_ucErrorBitCount )
	{
		sNAND_IO_ucErrorBitCount = ucErrorBitCount;
	}
}

__INLINE unsigned char NAND_IO_GetEccErrorBitCount(void)
{
	return sNAND_IO_ucErrorBitCount;
}

unsigned char _NAND_IO_GetEccErrorBitCount(void)
{
	return sNAND_IO_ucErrorBitCount;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_CorrectionMLC( NAND_IO_DEVINFO *nDevInfo, unsigned char *nPageBuffer, unsigned char *nSpareBuffer, unsigned short nDataSize )
{
	unsigned int			i;
	unsigned int			uErrAddr;
	unsigned int			uErrorStatus;
	unsigned long			ulEccCode[27];
	const NAND_IO_ECC_INFO	*pECC_Info;
	NAND_IO_ERROR_T			res;

	pECC_Info = NAND_IO_GetECCInfo( nDevInfo->EccType );
	if( pECC_Info == NULL )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	NAND_Util_memcpy( ulEccCode, nSpareBuffer, (unsigned int)pECC_Info->ucEccCodeSize );

	s_pECC->ECC_CODE0 = ulEccCode[0];
	s_pECC->ECC_CODE1 = ulEccCode[1];
	s_pECC->ECC_CODE2 = ulEccCode[2];
	s_pECC->ECC_CODE3 = ulEccCode[3];
	s_pECC->ECC_CODE4 = ulEccCode[4];
#if defined(NFC_VER_100) || defined(NFC_VER_200) || defined(NFC_VER_300)
	s_pECC->ECC_CODE5 = ulEccCode[5];
	s_pECC->ECC_CODE6 = ulEccCode[6];
#endif
#if defined(NFC_VER_200) || defined(NFC_VER_300)
	s_pECC->ECC_CODE7 = ulEccCode[7];
	s_pECC->ECC_CODE8 = ulEccCode[8];
	s_pECC->ECC_CODE9 = ulEccCode[9];
	s_pECC->ECC_CODE10 = ulEccCode[10];
#endif
#if defined(NFC_VER_300)
	s_pECC->ECC_CODE11 = ulEccCode[11];
	s_pECC->ECC_CODE12 = ulEccCode[12];
	s_pECC->ECC_CODE13 = ulEccCode[13];
	s_pECC->ECC_CODE14 = ulEccCode[14];
	s_pECC->ECC_CODE15 = ulEccCode[15];
	s_pECC->ECC_CODE16 = ulEccCode[16];
	s_pECC->ECC_CODE17 = ulEccCode[17];
	s_pECC->ECC_CODE18 = ulEccCode[18];
	s_pECC->ECC_CODE19 = ulEccCode[19];
	s_pECC->ECC_CODE20 = ulEccCode[20];
	s_pECC->ECC_CODE21 = ulEccCode[21];
	s_pECC->ECC_CODE22 = ulEccCode[22];
	s_pECC->ECC_CODE23 = ulEccCode[23];
	s_pECC->ECC_CODE24 = ulEccCode[24];
	s_pECC->ECC_CODE25 = ulEccCode[25];
	s_pECC->ECC_CODE26 = ulEccCode[26];
#endif

	/* Sync Delay */
	//ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;

	/* Wait MLC ECC Correction */
#if defined(NFC_VER_50) || defined(NFC_VER_100)
	while( !( s_pECC->ECC_IREQ & pECC_Info->DecodeFlag ) );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	while( !( s_pNFC->NFC_IRQ & NFC_IRQ_MLC_ECC_DECODE_FLAG ) );
	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_ECC_ENABLE );
#endif

	if( nDevInfo->fReadRetryNow == FALSE/* refer to ticket#1937 for understanding this condition */
		&& NAND_IO_IsAllValue( ulEccCode, 0xFF, (unsigned int)pECC_Info->ucEccCodeSize ) )
	{
		// Check whether page data is all 0xff or not.
		if( !NAND_IO_IsAllValue( nPageBuffer, 0xFF, (unsigned int)nDataSize ) )
		{
			// Check whether page is BL1 or Golden info or not.
			// Check Tag Type.
			return NAND_IO_CHECK_BOOTAREA_PAGE;
		}		
		return NAND_IO_ERASED_PAGE;
	}
	else
	{
		res = ( NAND_IO_ERROR_T )NAND_IO_ERROR_ECC_CORRECTION_FAILURE;

		/* Correction */
#if defined(NFC_VER_50) 
		uErrorStatus = s_pECC->ECC_ERRNUM & 0x0F;	
#elif defined(NFC_VER_100) || defined(NFC_VER_200)		
		uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x1F;
#elif defined(NFC_VER_300)
		uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x3F;
#endif

		if( uErrorStatus > (unsigned int)pECC_Info->ucErrorNum )
		{
			if( sNAND_IO_fSuppressMessage == FALSE )
			{
				//#if defined(NAND_IO_ECC_ERROR_LOG)
#if defined(NAND_FOR_KERNEL)
				ND_TRACE( "ErrorNum[%d],DataSize[%d] - Correction Fail\n", uErrorStatus, nDataSize );
				ND_TRACE( "Data:\n" );
				for( i = 0; i < nDataSize; ++i )
				{
					_ND_TRACE( "%02X", nPageBuffer[i] );
				}
				_ND_TRACE( "\n" );

				ND_TRACE( "ECC: " );
				for( i = 0; i < (unsigned int)pECC_Info->ucEccCodeSize; ++i )
				{
					_ND_TRACE( "%02X", nSpareBuffer[i] );
				}
				_ND_TRACE( "\n" );
#endif
				//#endif
			}
		}
		else if( uErrorStatus == 0 )
		{
			res = NAND_IO_SUCCESS;
		}
		else
		{
			NAND_IO_SetEccErrorBitCount((unsigned char)uErrorStatus);

			for( i = 0 ; i < uErrorStatus ; ++i )
			{
				uErrAddr = s_pECC->ECC_EADDR[i];

				if( ( uErrAddr >> 3 ) < nDataSize )
					nPageBuffer[uErrAddr >> 3] ^= ( 1 << ( uErrAddr & 0x7 ) );
			}
			res = NAND_IO_SUCCESS;
		}
	}

	/* Disable MLC ECC */
	//IO_CKC_DisableBUS( IO_CKC_BUS_ECC );

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadDataArea( NAND_IO_DEVINFO *nDevInfo, unsigned int *puiDmaBuffer, unsigned int *puiDmaPhysicalBuffer )
{
	unsigned int		j;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
	sigset_t		sigmask;	
	unsigned int 	uiCheckSignal=FALSE;	
	
	sigemptyset(&sigmask);
	sigfillset(&sigmask);
	#endif

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res = NAND_IO_SUCCESS;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

#if defined(__USE_NAND_ISR__) && defined(_LINUX_)
	if( NAND_IO_IRQ_IsEnabled() )
	{
		int ret;

		gsNAND_IO_NandIsrInfo.pNandIoDevInfo	= nDevInfo;
		gsNAND_IO_NandIsrInfo.pbPageBuffer		= (unsigned char*)puiDmaBuffer;
		gsNAND_IO_NandIsrInfo.pbPhyPageBuffer	= (unsigned char*)puiDmaPhysicalBuffer;
		gsNAND_IO_NandIsrInfo.pbEccBuffer		= (unsigned char*)&sNAND_IO_uiSharedEccBuffer[0];
		gsNAND_IO_NandIsrInfo.usCurrentPPage	= 0;
		gsNAND_IO_NandIsrInfo.usPPagesLeft		= nDevInfo->usCodewordsPerPage;
		gsNAND_IO_NandIsrInfo.iEccOnOff			= ECC_ON;
		gsNAND_IO_NandIsrInfo.wait_complete 	= 0;
		gsNAND_IO_NandIsrInfo.uiState			= NAND_IO_IRQ_STATE_READ;
		gsNAND_IO_NandIsrInfo.error				= NAND_IO_SUCCESS;
		gsNAND_IO_NandIsrInfo.ubIsRun			= TRUE;

		if( gsNAND_IO_NandIsrInfo.iEccOnOff == ECC_ON )
		{
			/* Setup ECC Block */
#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ECC_ON, ECC_DECODE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
#else
			NAND_IO_SetupECC( ECC_ON, ECC_DECODE, gsNAND_IO_NandIsrInfo.pNandIoDevInfo->EccType, NAND_DMA_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
#endif
			s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	= 0x00000000;
		}

		NAND_IO_SetupDMA( ( void * )&NAND_IO_HwLDATA_PA, 0, 0,
			              ( void * )gsNAND_IO_NandIsrInfo.pbPhyPageBuffer, 4, 0,
			              NAND_IO_DMA_READ, nDevInfo->CodewordSize );

		NAND_IO_ISR_Control( ENABLE );	// Enable Global Interrupt for NFC
		do
		{
			#if 1 //defined(TCC893X)			
			//ret = wait_event_interruptible_locked_irq(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete);
			ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );				
			#else
			//ret = wait_event_interruptible(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete);
			ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );				
			#endif

			if( ret > 0 )
			{
				break;
			}
			else
			{
				//if( iIsr_Ret == 0 )  // this option used for  wait_event_interruptible_locked_irq
				if( gsNAND_IO_NandIsrInfo.wait_complete )  // this option used for wait_event_interruptible_timeout
				{
					if( uiCheckSignal == TRUE )
						sigprocmask( SIG_UNBLOCK, &sigmask, NULL );
					break;				
				}
				else if( abs(ret) == ERESTARTSYS )
				{
					uiCheckSignal = TRUE;				
					sigprocmask( SIG_BLOCK, &sigmask, NULL );
					ND_TRACE(" Write Interrupt Wakeup - go sleep again\n");
				}
				else
				{
					ND_TRACE("nand:wait_event_interruptible error!(ret=%d)\n",ret);
				}
			}
		} while(1) ;		

		gsNAND_IO_NandIsrInfo.uiState = NAND_IO_IRQ_STATE_NONE;
		gsNAND_IO_NandIsrInfo.ubIsRun = FALSE;

		if( gsNAND_IO_NandIsrInfo.error != NAND_IO_SUCCESS && gsNAND_IO_NandIsrInfo.error != NAND_IO_ERASED_PAGE )
			res = gsNAND_IO_NandIsrInfo.error;
	}
	else
#endif //defined(__USE_NAND_ISR__) && defined(_LINUX_)
	{
		// Set SpareBuffer Pointer =>> ECCBuffer
		unsigned char *pucSharedEccBuf = ( unsigned char * )&sNAND_IO_uiSharedEccBuffer[0];

		//----------------------------------------------
		//	Read Data as Codeword repeatly
		//----------------------------------------------
		for( j = 0; j < nDevInfo->usCodewordsPerPage ; ++j )
		{
			//####################################################
			//#	Read 512 Page Data
			//####################################################
			//----------------------------------------------
			//	MCU ACCESS
			//----------------------------------------------
			if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_MCU_MODE )
			{
				unsigned int *puiDataBuf = puiDmaBuffer;
				unsigned int i;

				if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
					#else
					NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
					#endif
					
					s_pECC->ECC_CTRL		|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
					s_pECC->ECC_CLEAR	 = 0x00000000;
				}

				/* Read 512 Data Area */
				i = ( nDevInfo->CodewordSize >> 2 );
				do
				{
					*puiDataBuf++ = s_pNFC->NFC_WDATA;
				} while( --i );
			}
			else if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_GDMA_MODE )
			{
				//----------------------------------------------
				//	DMA ACCESS
				//----------------------------------------------
				if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
					#else
					NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_DMA_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
					#endif
					
					s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
					s_pECC->ECC_CLEAR	 = 0x00000000;
				}

				/* Start DMA on NFC BUS */
				#if defined(_WINCE_) || defined(_LINUX_)
				NAND_IO_SetupDMA( ( void * )&NAND_IO_HwLDATA_PA, 0, 0,
				                  ( void * )puiDmaPhysicalBuffer, 4, 0,
				                  NAND_IO_DMA_READ, nDevInfo->CodewordSize );
				#else
				NAND_IO_SetupDMA( ( void * )&s_pNFC->NFC_LDATA, 0, 0,
				                  ( void * )puiDmaPhysicalBuffer, 4, 0,
				                  NAND_IO_DMA_READ, nDevInfo->CodewordSize );
				#endif

				puiDmaPhysicalBuffer = &puiDmaPhysicalBuffer[( nDevInfo->CodewordSize ) >> 2];
			}
		
			//####################################################
			//####################################################

			/* Check and Correct ECC code */
			if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
			{
				NAND_IO_ERROR_T resTemp = NAND_IO_CorrectionMLC( nDevInfo, ( unsigned char * )puiDmaBuffer, pucSharedEccBuf, nDevInfo->CodewordSize );
				if( resTemp != NAND_IO_SUCCESS && resTemp != NAND_IO_ERASED_PAGE )
					res |= resTemp;

				pucSharedEccBuf = &pucSharedEccBuf[nDevInfo->EccCodeSize];
			}

			puiDmaBuffer = &puiDmaBuffer[( nDevInfo->CodewordSize ) >> 2];
		}
	}

	//=========================================================================
	// Return
	//=========================================================================
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	return res;
}

static NAND_IO_ERROR_T _NAND_IO_ReadTag( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiTagBuffer, unsigned int ulSpareSize)
{
	unsigned int		i;
	unsigned long		ulSpareTotalSize = ulSpareSize;	
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;
	unsigned long		ulTagEccBuffer[ECC_DATA_MAX_DWORD];

	NAND_IO_CHANGE_SPARE_ECC_TYPE(nDevInfo, A_CHANGE_SPARE_ECC_OF(nDevInfo->Feature.MediaType)); 

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
	{
		/* Setup ECC Block for Tag */
#if defined(_WINCE_) || defined(_LINUX_)
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif
		s_pECC->ECC_CTRL	|= ( ulSpareTotalSize << ECC_SHIFT_DATASIZE );
		s_pECC->ECC_CLEAR	= 0x00000000;
	}

	//----------------------------------------------
	//	Read Tag
	//----------------------------------------------
	{
		unsigned int *puiTagBuf = puiTagBuffer;
		i = ( ulSpareTotalSize >> 2 );
		do
		{
			*puiTagBuf++ = s_pNFC->NFC_WDATA;
		} while( --i );
	}

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
	{
		//----------------------------------------------
		// Read Tag ECC
		//----------------------------------------------
		unsigned long ulTagEccSizeInDword = ( nDevInfo->EccCodeSize + 3 ) >> 2;
		for( i = 0 ; i < ulTagEccSizeInDword && i < ECC_DATA_MAX_DWORD ; i++ )
		{
			ulTagEccBuffer[i] = s_pNFC->NFC_WDATA;
		}

		//----------------------------------------------
		// Check and Correct ECC code
		//----------------------------------------------
		res = NAND_IO_CorrectionMLC( nDevInfo, ( unsigned char * )puiTagBuffer, ( unsigned char * )ulTagEccBuffer, ( unsigned short )ulSpareTotalSize );
	}

	if( res == NAND_IO_SUCCESS && nDevInfo->Feature.MediaType & A_RAND_IO )
	{
		TNFTL_RAND_SetAddress( (unsigned int)nDevInfo->Feature.PpB, uiPageAddress, (unsigned int)nDevInfo->Feature.PageSize );
		TNFTL_RAND_Randomize( (void *)puiTagBuffer, (const void *)puiTagBuffer, ulSpareTotalSize );
	}

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
	{
		if(res == NAND_IO_ERASED_PAGE)
		{
			res = NAND_IO_SUCCESS;
		}

		// Bad block's spare data are all zero.
		// All zero data's ECC codes are all zero, too.
		if( NAND_IO_IsAllValue( ( void * )ulTagEccBuffer, 0x00, nDevInfo->EccCodeSize ) == TRUE )
		{
			if( sNAND_IO_fSuppressMessage == FALSE )
				ND_TRACE( "ECC Error: Tag data is all zero\n" );
			res = NAND_IO_ERROR_ECC_CORRECTION_FAILURE;
		}
	}

	NAND_IO_CHANGE_SPARE_ECC_TYPE(nDevInfo, 0); //Restore to original ECC type

	return res;
}

static NAND_IO_ERROR_T NAND_IO_ReadTag( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiTagBuffer, unsigned int uiTagSize )
{
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
	{
		volatile unsigned int uiFactoryBadMarker;

		//----------------------------------------------
		// Read Factory Bad Marker
		//----------------------------------------------
		uiFactoryBadMarker = s_pNFC->NFC_WDATA;

		if( (uiFactoryBadMarker&0xFF) != 0xFF )
		{
			res = NAND_IO_ERROR_ECC_CORRECTION_FAILURE;
		}
	}

	if( res == NAND_IO_SUCCESS )
		res = _NAND_IO_ReadTag(nDevInfo, uiPageAddress, puiTagBuffer, uiTagSize);

	return res;
}

static NAND_IO_ERROR_T NAND_IO_ReadSpareArea( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiTagBuffer, unsigned int uiTagSize )
{
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Read Data Area ECC
	//=========================================================================
	{
		unsigned int i;
		unsigned int *puiSharedDataEccBuffer = sNAND_IO_uiSharedEccBuffer;
		volatile unsigned int uiFactoryBadMarker;

		//----------------------------------------------
		// Skip Factory Bad Marker
		//----------------------------------------------
		uiFactoryBadMarker = s_pNFC->NFC_WDATA;		

		if( (uiFactoryBadMarker&0xFF) != 0xFF )
		{			
		}

		//----------------------------------------------
		//	Read ECC for Data
		//----------------------------------------------
		i = nDevInfo->EccCodeSizeTotalInDword;
		do
		{
			*puiSharedDataEccBuffer++ = s_pNFC->NFC_WDATA;
		} while( --i );
	}

	//=========================================================================
	// Read Tag
	//=========================================================================
	res = _NAND_IO_ReadTag(nDevInfo, uiPageAddress, puiTagBuffer, uiTagSize);

	return res;
}

static NAND_IO_ERROR_T NAND_IO_ReadStatusOfReadECC(NAND_IO_DEVINFO *nDevInfo)
{
	unsigned int uiStatus,uiFail;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	unsigned short usReadStatus = 0;

	NAND_IO_SetCommCycleTime();
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x7070;

	// delay for tWHR
	NAND_Util_Delay();

	NAND_IO_SetReadCycleTime();
	//================================
	//	Read IO Status
	//================================
	uiStatus = nDevInfo->CmdMask & s_pNFC->NFC_SDATA;
	uiFail = nDevInfo->CmdMask & 0x0101;

	if( uiStatus & uiFail )
	{
		if( nDevInfo->Feature.MediaType & A_PARALLEL )
		{
			if( uiStatus & ( uiFail & 0x0100 ) )
				usReadStatus |= NAND_IO_STATUS_FAIL_CS1_PARALLEL;

			if( uiStatus & ( uiFail & 0x0001 ) )
				usReadStatus |= NAND_IO_STATUS_FAIL_CS0_PARALLEL;
		}
		else
		{
			usReadStatus |= NAND_IO_STATUS_FAIL_CS0_SERIAL;
		}
	}

	if( usReadStatus )
		res = NAND_IO_ERROR_READ_STATUS_FAILURE;

	if( res != NAND_IO_SUCCESS && sNAND_IO_fSuppressMessage == FALSE )
	{
		ND_ERROR("ReadStatus = %X\n", uiStatus);
	}

	NAND_IO_SetCommCycleTime();
	s_pNFC->NFC_CMD = 0x0000;

	return res;
}

#if defined(NAND_SUPPORT_NDMA)
#if 0
/**************************************************************************
*  DESCRIPTION : Cache Read 동작을 수행.
*
*  INPUT : fFirstTime - 첫 회인지.
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static unsigned int NAND_IO_DoCacheRead( NAND_IO_DEVINFO *pDevInfo, unsigned int uiPageAddress, unsigned int fNextPageForNextTurn, unsigned int fFirstTime )
{
	unsigned int uiPageOffsetMask = pDevInfo->Feature.PpB - 1;
	unsigned int uiPageOffset = uiPageOffsetMask & uiPageAddress;
	if( fNextPageForNextTurn && uiPageOffset < uiPageOffsetMask )
	{
		/* Command CACHE READ [ 0x31 ] */
		s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;
		s_pNFC->NFC_CMD = sNAND_IO_DeviceInfo.CmdMask & 0x3131;
		/* Wait until it is ready */
		NAND_IO_WaitBusy( sNAND_IO_DeviceInfo.ChipNo );
		return TRUE;
	}
	else if( !fFirstTime )
	{
		s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;
		/* Command CACHE READ [ 0x3F ] */
		s_pNFC->NFC_CMD = sNAND_IO_DeviceInfo.CmdMask & 0x3F3F;
		/* Wait until it is ready */
		NAND_IO_WaitBusy( sNAND_IO_DeviceInfo.ChipNo );
	}

	return FALSE;
}
#endif
/**************************************************************************
*  FUNCTION NAME :
*
*      static __inline NAND_IO_ERROR NAND_IO_CorrectionMLC( U16 nEccType, U8* nPageBuffer, U8* nSpareBuffer, U16 nDataSize );
*
*  DESCRIPTION : You can add file description here.
*
*  INPUT:
*			nDataSize	=
*			nEccType	=
*			nPageBuffer	=
*			nSpareBuffer	=
*
*  OUTPUT:	NAND_IO_ERROR - Return Type
*  			=
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_CorrectionNDMA( NAND_IO_DEVINFO *nDevInfo, unsigned char *pucPageBuffer, unsigned short usDataSize )
{
	unsigned int			i;
	unsigned int			uErrAddr;
	unsigned int			uErrorStatus;
	const NAND_IO_ECC_INFO	*pECC_Info;
	NAND_IO_ERROR_T			res;

	pECC_Info = NAND_IO_GetECCInfo( nDevInfo->EccType );
	if( pECC_Info == NULL )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	res = NAND_IO_SUCCESS;

	/* Correction */
	#if defined(NFC_VER_100) || defined(NFC_VER_200)		
	uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x1F;
	#elif defined(NFC_VER_300)
		uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x3F;
	#endif

	if( uErrorStatus > pECC_Info->ucErrorNum )
	{
		if( sNAND_IO_fSuppressMessage == FALSE )
		{
			ND_TRACE( "\n[NDMA] ErrorNum[%d],DataSize[%d] - Correction Fail \n", uErrorStatus, usDataSize );
		}
		res = NAND_IO_ERROR_ECC_CORRECTION_FAILURE;

	}
	else if( uErrorStatus == 0 )
	{
		res = NAND_IO_SUCCESS;
	}
	else
	{
		//ND_TRACE(" Correction:%d\n", uErrorStatus);
		NAND_IO_SetEccErrorBitCount((unsigned char)uErrorStatus);
		for( i = 0 ; i < uErrorStatus ; ++i )
		{
			uErrAddr = s_pECC->ECC_EADDR[i];

			if( ( uErrAddr >> 3 ) < usDataSize )
				pucPageBuffer[uErrAddr >> 3] ^= ( 1 << ( uErrAddr & 0x7 ) );
		}
		res = NAND_IO_SUCCESS;

	}
	return res;
}


/**************************************************************************
*  FUNCTION NAME :
*
*      NAND_IO_NDMA_WriteDataAndSpareArea
*
*  DESCRIPTION : You can add file description here.
*
*  INPUT:
*			nDevInfo	=
*			nEccOnOff	=
*			nPageBuffer	=
*			nSpareBuffer	=
*			nStartPPage	=
*			nWritePPSize	=
*
*  OUTPUT:	NAND_IO_ERROR - Return Type
*  			=
*
*  REMARK:
**************************************************************************/
static void NAND_IO_NDMA_WriteDataAndSpareArea( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer, unsigned int uiTagSize )
{
	unsigned char 		*ulcNDMA_MEMBuf;

	unsigned long 		ulBadMarkSize = 0;
	unsigned int 		j;

	#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
	sigset_t		sigmask;	
	int 			iIsr_Ret;	
	unsigned int 	uiCheckSignal=FALSE;	
	
	sigemptyset(&sigmask);
	sigfillset(&sigmask);
	#endif

	// NAND Bad Mark Size (Byte Size)
	ulBadMarkSize	= TNFTL_BADMARK_BYTE_SIZE;

	/*
	if( (nDevInfo->Feature.MediaType & A_RAND_IO) && !nDevInfo->fSkipDataAreaRandomizing )
	{
		TNFTL_RAND_SetAddress( (unsigned int)nDevInfo->Feature.PpB, uiPageAddress, 0 );
		//TNFTL_RAND_Randomize( (void *)s_puiDMA_WorkBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
		TNFTL_RAND_Randomize( (void *)sNAND_IO_uiRandomizeBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
		NAND_Util_memcpy( (void *)s_puiDMA_WorkBuffer, (const void *)sNAND_IO_uiRandomizeBuffer, (unsigned int)nDevInfo->Feature.PageSize );
	}
	else
	{
		NAND_Util_memcpy( (void *)s_puiDMA_WorkBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );
	}
	*/

	//-----------------------------------------
	// BSIZE SETTING
	//-----------------------------------------
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//-----------------------------------------
	// ECC_OFF
	//-----------------------------------------
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//-----------------------------------------
	//[DMA Mode Set]: NDMA
	//-----------------------------------------
	BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_DMA_MODE_NDMA );
	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_MODE_DATA_N_SPARE );
	else
		BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_MODE_DATA_N_SPARE );

	//============================================================
	// DMA Control Register Set
	//============================================================
	s_pNFC->NDMA_CTRL =	NDMA_CTRL_ARBITRATION_OFF		|
						#if defined(TCC88XX)
	                    NDMA_CTRL_LOCK					| 	// If Lock Bit doesn't set at tcc88xx platform. there is possibility to data broken.
	                    #else
						//NDMA_CTRL_LOCK					|
	                    #endif
	                    NDMA_CTRL_BURST_SIZE_8			|
	                    NDMA_CTRL_WORD_SIZE_32BIT		|
	                    //NDMA_CTRL_DMA_DONE_FLAG			|
	                    NDMA_CTRL_SEQUENCIAL_MODE_ON	|
	                    NDMA_CTRL_ENABLE				|
	                    0;

	BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD | NFC_CTRL_PAGE_SIZE_FIELD, NFC_CTRL_BURST_SIZE_8 | NFC_CTRL_PAGE_SIZE_512B );

	//============================================================
	// Set Source Address & Source Parameter (mask + increment)
	//============================================================
	/*[NDMA ADDR: LDATA]*/
	/*[NDMA ADDR: DATA BUFFER]*/
	s_pNFC->NDMA_ADDR		= ( unsigned int )s_puiDMA_PhyBuffer;
	s_pNFC->NDMA_PARAM		= ( 4 | ( 0 << 4 ) );

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		s_pNFC->NFC_SPARE		= ( nDevInfo->EccCodeSizeTotalInDword << 2 ) + ulBadMarkSize;
	else
		s_pNFC->NFC_SPARE		= 0;
	s_pNFC->NDMA_SPCTRL 	= nDevInfo->usCodewordsPerPage - 1;

	//============================================================
	// Set Subpage Size and ECC Offset
	//============================================================
	for( j = 0 ; j < nDevInfo->usCodewordsPerPage && j < 32 ; ++j )
	{
		s_pNFC->SP_CFG[j]   = ( ( ( ( nDevInfo->EccCodeSize * j ) + ulBadMarkSize ) << NDMA_ECC_ADDR_SHIFT ) | nDevInfo->CodewordSize );
		#if defined(NFC_VER_300)
		s_pNFC->SP_CFG[j]	|= ( NAND_IO_GetECCMode( ECC_ENCODE, nDevInfo->EccType ) << NDMA_ECC_MODE_SHIFT );
		#endif
	}

	//============================================================
	// Set ECC Mode to 1 for direct access to memory
	//============================================================
	s_pECC->ECC_CTRL	|= ECC_CTRL_MODE_REFER_TO_NFC_MEMORY;
	ulcNDMA_MEMBuf  = ( unsigned char * )s_pECC->NFC_MEMORY;
	for( j = 0; j < ulBadMarkSize; j++ )
		ulcNDMA_MEMBuf[j] = 0xFF;
	s_pECC->ECC_CTRL	&= ~ECC_CTRL_MODE_REFER_TO_NFC_MEMORY;

	{
		unsigned short usEccType;
		if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
		{
			usEccType = ECC_TYPE_4BIT;
		}
		else
		{
			usEccType = nDevInfo->EccType;
		}
		NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, usEccType, NAND_DMA_ACCESS, ( unsigned long )( &NAND_IO_HwLDATA_PA ) );
	}
	//NDMA transfer ignores ECC_DSIZE in ECC_CTRL
	s_pECC->ECC_CLEAR	= 0;

	//============================================================
	// NFC PROGRAM/READ SELECT
	//============================================================
	BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_PROGRAM );

	#if defined(__USE_NAND_ISR__)
	if( NAND_IO_IRQ_IsEnabled() )
	{
		gsNAND_IO_NandIsrInfo.ubIsRun		= TRUE;
		gsNAND_IO_NandIsrInfo.uiState		= NAND_IO_IRQ_STATE_WRITE;
		gsNAND_IO_NandIsrInfo.wait_complete	= 0;
		BITSET(s_pNFC->NFC_IRQCFG, NFC_IRQCFG_PROGRAM_INTR_ENABLE );		
	}
	#endif

	//============================================================
	// NFC PROGRAM/READ START
	//============================================================
	s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG | NFC_IRQ_SPARE_BURST_WR_FLAG | NFC_IRQ_MLC_ECC_ENCODE_FLAG | NFC_IRQ_PROGRAM_INTR;
	s_pNFC->NFC_PRSTART = 0;

	#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
	if( NAND_IO_IRQ_IsEnabled() )
	{
		NAND_IO_ISR_Control( ENABLE );
		do
		{
			#if 1 //defined(TCC893X)
			iIsr_Ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );	
			//iIsr_Ret = wait_event_interruptible_locked_irq( gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete );
			#else 
			iIsr_Ret = wait_event_interruptible( gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete );
			#endif

			if( iIsr_Ret > 0 )
			{
				break;
			}
			else
			{
				//if( iIsr_Ret == 0 )  // this option used for  wait_event_interruptible_locked_irq
				if( gsNAND_IO_NandIsrInfo.wait_complete )  // this option used for wait_event_interruptible_timeout
				{
					if( uiCheckSignal == TRUE )
						sigprocmask( SIG_UNBLOCK, &sigmask, NULL );
					
					break;
				}
				else if( abs(iIsr_Ret) == ERESTARTSYS )
				{
					uiCheckSignal = TRUE;				
					sigprocmask( SIG_BLOCK, &sigmask, NULL );
					ND_TRACE(" Write Interrupt Wakeup \n");
				}
			}
		} while(1);
	}
	else
	#endif
	{
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
			while( !ISALLONE( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG | NFC_IRQ_MLC_ECC_ENCODE_FLAG ) );
		else
			while( !ISALLONE( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );
	}
	
	s_pNFC->NDMA_CTRL |= NDMA_CTRL_DMA_DONE_FLAG;

	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_DMA_MODE_NDMA );
	BITCLR( s_pNFC->NDMA_CTRL, NDMA_CTRL_LOCK | NDMA_CTRL_ENABLE );

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	#if defined(__USE_NAND_ISR__)	
	BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG | NFC_IRQ_PROGRAM_INTR | NFC_IRQ_SPARE_BURST_WR_FLAG );
	#endif

	//============================================================
	// Write Spare Data
	//============================================================
	NAND_IO_WriteSpareArea( nDevInfo, uiPageAddress, puiSpareBuffer, uiTagSize );
}

/**************************************************************************
*  FUNCTION NAME :
*
*      NAND_IO_ERROR 	NAND_IO_NDMA_ReadDataArea( NAND_IO_DEVINFO *nDevInfo, U8 *nPageBuffer, int nEccOnOff )
*
*  DESCRIPTION : You can add file description here.
*
*  INPUT:
*			nDevInfo	=
*			nEccOnOff	=
*			nPageBuffer	=
*			nSpareBuffer	=
*			nStartPPage	=
*			nWritePPSize	=
*
*  OUTPUT:	NAND_IO_ERROR - Return Type
*  			=
*
*  REMARK:
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_NDMA_ReadDataArea( NAND_IO_DEVINFO *nDevInfo, unsigned int *puiDmaBuffer, unsigned int *puiDmaPhysicalBuffer )
{
	unsigned char	*pucNDMA_MEMBuf;
	unsigned long 	j;
	unsigned long 	ulNDMA_BufferPos;
	unsigned long    ulBadMarkSize;

	NAND_IO_ERROR_T 	res = NAND_IO_SUCCESS;

	#if defined(_LINUX_) && defined(__USE_NAND_ISR__)
	sigset_t		sigmask;	
	int 			iIsr_Ret;	
	unsigned int 	uiCheckSignal=FALSE;	
	
	sigemptyset(&sigmask);
	sigfillset(&sigmask);
	#endif

	// Spare ECC Data Size
	ulBadMarkSize	= TNFTL_BADMARK_BYTE_SIZE;

	//-----------------------------------------
	// BSIZE SETTING
	//-----------------------------------------
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//-----------------------------------------
	// ECC_OFF
	//-----------------------------------------
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//-----------------------------------------
	// [DMA Mode Set]: NDMA
	//-----------------------------------------
	BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_DMA_MODE_NDMA );
	BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_MODE_DATA_N_SPARE );

	//============================================================
	// DMA Control Register Set
	//============================================================
	s_pNFC->NDMA_CTRL =	NDMA_CTRL_ARBITRATION_OFF		|
						#if defined(TCC88XX)
	                    NDMA_CTRL_LOCK					|	// If Lock Bit doesn't set at tcc88xx platform. there is possibility to data broken.
	                    #else
						//NDMA_CTRL_LOCK					|
	                    #endif
	                    NDMA_CTRL_BURST_SIZE_8			|
	                    NDMA_CTRL_WORD_SIZE_32BIT		|
	                    //NDMA_CTRL_DMA_DONE_FLAG			|
	                    NDMA_CTRL_SEQUENCIAL_MODE_ON	|
	                    NDMA_CTRL_ENABLE				|
	                    0;

	BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD | NFC_CTRL_PAGE_SIZE_FIELD, NFC_CTRL_BURST_SIZE_8 | NFC_CTRL_PAGE_SIZE_512B );

	//============================================================
	// Set Source Address & Source Parameter (mask + increment)
	//============================================================
	/*[NDMA ADDR: LDATA]*/
	//s_pNFC->NDMA_ADDR  = (U32)&NAND_IO_HwLDATA_PA;
	/*[NDMA ADDR: DATA BUFFER]*/
	s_pNFC->NDMA_ADDR  = ( unsigned int )puiDmaPhysicalBuffer;
	s_pNFC->NDMA_PARAM = ( 4 | ( 0 << 4 ) );

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		s_pNFC->NFC_SPARE   = ulBadMarkSize + ( nDevInfo->EccCodeSizeTotalInDword << 2 );
	else
		s_pNFC->NFC_SPARE   = ulBadMarkSize + TNFTL_TAG_BYTE_SIZE;
	s_pNFC->NDMA_SPCTRL = nDevInfo->usCodewordsPerPage - 1;

	for( j = 0; j < nDevInfo->usCodewordsPerPage && j < 32 ; ++j )
	{
		s_pNFC->SP_CFG[j]   = ( ( ( ( nDevInfo->EccCodeSize * j ) + ulBadMarkSize ) << NDMA_ECC_ADDR_SHIFT ) | nDevInfo->CodewordSize );
		#if defined(NFC_VER_300)
		s_pNFC->SP_CFG[j]	|= ( NAND_IO_GetECCMode( ECC_DECODE, nDevInfo->EccType ) << NDMA_ECC_MODE_SHIFT );
		#endif
	}

	{
		unsigned short usEccType;
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			usEccType = nDevInfo->EccType;
		}
		else
		{
			usEccType = ECC_TYPE_4BIT;
		}
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, usEccType, NAND_DMA_ACCESS, ( unsigned long )( &NAND_IO_HwLDATA_PA ) );
	}
	//NDMA transfer ignores ECC_DSIZE in ECC_CTRL
	s_pECC->ECC_CLEAR	= 0;

	//============================================================
	// NFC PROGRAM/READ SELECT
	//============================================================
	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_PROGRAM );


	#if defined(__USE_NAND_ISR__)
	if( NAND_IO_IRQ_IsEnabled() )
	{
		gsNAND_IO_NandIsrInfo.ubIsRun		= TRUE;
		gsNAND_IO_NandIsrInfo.uiState		= NAND_IO_IRQ_STATE_READ;
		gsNAND_IO_NandIsrInfo.wait_complete	=	0;
		BITSET(s_pNFC->NFC_IRQCFG, NFC_IRQCFG_READ_INTR_ENABLE );		
	}
	#endif
	
	//============================================================
	// NFC PROGRAM/READ START
	//============================================================
	s_pNFC->NFC_IRQ = NFC_IRQ_READ_FLAG | NFC_IRQ_SPARE_BURST_RD_FLAG | NFC_IRQ_ECC_ERROR_FLAG | NFC_IRQ_READ_INTR;
	s_pNFC->NFC_PRSTART = 0;

	#if defined(__USE_NAND_ISR__)
	if( NAND_IO_IRQ_IsEnabled() )
	{
		NAND_IO_ISR_Control( ENABLE );	// Enable Global Interrupt for NFC
		do
		{
			#if 1 //defined(TCC893X)
			iIsr_Ret = wait_event_interruptible_timeout(gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete, 1*HZ );	
			//iIsr_Ret = wait_event_interruptible_locked_irq( gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete );
			#else
			iIsr_Ret = wait_event_interruptible( gsNAND_IO_NandIsrInfo.wait_q, gsNAND_IO_NandIsrInfo.wait_complete );
			#endif

			if( iIsr_Ret > 0 )
			{
				break;
			}
			else
			{
				//if( iIsr_Ret == 0 )  // this option used for  wait_event_interruptible_locked_irq
				if( gsNAND_IO_NandIsrInfo.wait_complete )  // this option used for wait_event_interruptible_timeout
				{
					if( uiCheckSignal == TRUE )
						sigprocmask( SIG_UNBLOCK, &sigmask, NULL );
					break;
				}
				else if( abs(iIsr_Ret) == ERESTARTSYS )
				{
					uiCheckSignal = TRUE;				
					sigprocmask( SIG_BLOCK, &sigmask, NULL );
					ND_TRACE(" Read Interrupt Wakeup \n");
				}
			}
		} while(1);
	}
	#endif	

	if( s_pNFC->NFC_CTRL & NFC_CTRL_BURST_MODE_DATA_N_SPARE )
	{
		do
		{
			while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_READ_FLAG ) );

			if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
			{
				s_pECC->ECC_RSTART = 0x00;
				break;
			}
#if defined(NFC_VER_200)
			else if( s_pECC->ECC_CSTATE & ECC_CSTATE_ERROR_ON_CURRENT_SUBPAGE )
#elif defined(NFC_VER_300)
			else if( s_pECC->ECC_CSTATE1 & ECC_CSTATE_ERROR_ON_CURRENT_SUBPAGE )
#endif
			{
				unsigned int uiTempResult;
				// if Ecc error Occur
				s_pECC->ECC_CTRL	|= ECC_CTRL_MODE_REFER_TO_NFC_MEMORY;
				pucNDMA_MEMBuf		= ( unsigned char * )s_pECC->NFC_MEMORY;
				ulNDMA_BufferPos	= ( nDevInfo->CodewordSize * ( s_pECC->ECC_ERRSTATUS >> 16 ) ) >> 2;
				uiTempResult 		= NAND_IO_IsAllValue( &pucNDMA_MEMBuf[( ( s_pECC->ECC_ERRSTATUS >> 16 ) * nDevInfo->EccCodeSize ) + ulBadMarkSize], 0xFF, nDevInfo->EccCodeSize );
				s_pECC->ECC_CTRL	&= ~ECC_CTRL_MODE_REFER_TO_NFC_MEMORY;
				if( nDevInfo->fReadRetryNow == FALSE && uiTempResult == TRUE )
				{
					// if spare data value is all 0xFF, that is erased page.
					if( !NAND_IO_IsAllValue( &puiDmaBuffer[ulNDMA_BufferPos], 0xFF, nDevInfo->CodewordSize ) )
					{
						res |= NAND_IO_CHECK_BOOTAREA_PAGE;
					}
					
					j = s_pECC->ECC_ERRSTATUS & 0x1F;						// just read for register clear.
				}
				else
				{
					if( NAND_IO_CorrectionNDMA( nDevInfo, ( unsigned char * )( &puiDmaBuffer[ulNDMA_BufferPos] ), nDevInfo->CodewordSize ) != NAND_IO_SUCCESS )
					{
						if( sNAND_IO_fSuppressMessage == FALSE )
						{
							ND_TRACE( " NDMA Read Fail !!!\n" );
						}
						res |= NAND_IO_ERROR_ECC_CORRECTION_FAILURE;
						NAND_IO_SetLastError( res );
					}
				}

				if( ( unsigned short )( s_pECC->ECC_ERRSTATUS >> 16 ) != ( unsigned short )( nDevInfo->usCodewordsPerPage - 1 ) )
				{
					s_pNFC->NFC_IRQ = NFC_IRQ_READ_FLAG | NFC_IRQ_SPARE_BURST_RD_FLAG | NFC_IRQ_ECC_ERROR_FLAG;
					s_pECC->ECC_RSTART = 0x00;
				}
				else
				{
					// when last subpage ecc error occur
					s_pECC->ECC_RSTART = 0x00;
					break;
				}
			}
			else
			{
				// When if ecc error doesn't occur.
				break;
			}
		} while( 1 );
	}
	else
	{
		while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_READ_FLAG ) );

		// Data Burst Read And Sequance ECC Correction
		j = s_pECC->ECC_STLDCTL;
		for( j = 0; j < nDevInfo->usCodewordsPerPage; j++ )
		{
			s_pECC->ECC_STLDCTL = 1 << j;

			ulNDMA_BufferPos = ( nDevInfo->CodewordSize * j ) >> 2 ;

			{
				NAND_IO_ERROR_T resTemp = NAND_IO_CorrectionMLC( nDevInfo, ( unsigned char * )( &puiDmaBuffer[ulNDMA_BufferPos] ), ( unsigned char * )( &sNAND_IO_uiSharedEccBuffer[j * nDevInfo->EccCodeSize] ), nDevInfo->CodewordSize );
				if( resTemp != NAND_IO_SUCCESS && resTemp != NAND_IO_ERASED_PAGE )
					res |= resTemp;
			}

			s_pNFC->NFC_CTRL	|= NFC_CTRL_ECC_ENABLE;
			s_pECC->ECC_RSTART = 0x01;
		}
	}

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	#if defined(__USE_NAND_ISR__)
	BITSET( s_pNFC->NFC_IRQ, NFC_IRQ_READ_FLAG | NFC_IRQ_READ_INTR | NFC_IRQ_SPARE_BURST_RD_FLAG );
	#endif

	//-----------------------------------------
	//[DMA Mode Clear]: NDMA
	//-----------------------------------------
	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_DMA_MODE_NDMA );
	BITCLR( s_pNFC->NDMA_CTRL, NDMA_CTRL_LOCK | NDMA_CTRL_ENABLE );

	return res;
}

static NAND_IO_ERROR_T _NAND_IO_NDMA_ReadPage(NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPageForNextTurn)
{
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;

	//=============================================
	// Read Data
	//=============================================
	#if 0
	if( nDevInfo->Feature.UsableFunc & F_CR )
	{
		if( nDevInfo->fReadCommandSent == FALSE )
		{
			s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

			NAND_IO_Send_ReadCommand( nDevInfo, uiPageAddress, 0 );

			/* Wait until it is ready */
			NAND_IO_WaitBusy( nDevInfo->ChipNo );

			if( NAND_IO_DoCacheRead(nDevInfo, uiPageAddress, fNextPageForNextTurn, TRUE) == TRUE )
			{
				nDevInfo->fReadCommandSent = TRUE;
			}
		}
		else
		{
			if( NAND_IO_DoCacheRead(nDevInfo, uiPageAddress, fNextPageForNextTurn, FALSE) == FALSE )
			{
				nDevInfo->fReadCommandSent = FALSE;
			}
		}
	}
	else
	#endif
	{
		if( nDevInfo->fReadCommandSent == FALSE )
		{
			s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

			NAND_IO_Send_ReadCommand( nDevInfo, uiPageAddress, 0 );

			/* Wait until it is ready */
			NAND_IO_WaitBusy( nDevInfo->ChipNo );
		}
		else
		{
			nDevInfo->fReadCommandSent = FALSE;
			NAND_IO_WaitBusyStatus( nDevInfo->ChipNo );
		}
	}

	if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
		res = NAND_IO_ReadStatusOfReadECC(nDevInfo);

	/* Change Cycle */
	NAND_IO_SetReadCycleTime();

	{
		NAND_IO_ERROR_T res2 = NAND_IO_NDMA_ReadDataArea( nDevInfo,
														s_puiDMA_WorkBuffer,
														s_puiDMA_PhyBuffer );

		if( res == NAND_IO_SUCCESS )
			res = res2;
	}
	if( ( res != NAND_IO_SUCCESS ) && ( res != NAND_IO_CHECK_BOOTAREA_PAGE ) )
	{
		if( sNAND_IO_fSuppressMessage == FALSE )
			ND_TRACE( "_NAND_IO_NDMA_ReadPage(page_addr=%d) is failed. (res=%X)\n", uiPageAddress, res );
	}

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
	{
		NAND_IO_ERROR_T res2 = _NAND_IO_ReadTag( nDevInfo, uiPageAddress, puiTag, uiTagSize );

		if( res == NAND_IO_CHECK_BOOTAREA_PAGE )
		{
			if( NAND_IO_CheckBootPage( (unsigned char*) puiTag ) == TRUE )
				res = NAND_IO_SUCCESS;
			else
				res = NAND_IO_ERROR_ECC_CORRECTION_FAILURE;
		}

		
		if( res2 != NAND_IO_SUCCESS )
		{
			if( sNAND_IO_fSuppressMessage == FALSE )
				ND_TRACE( "_NAND_IO_NDMA_ReadPage(page_addr=%d) is failed.(spare)\n", uiPageAddress );
			if( res == NAND_IO_SUCCESS )
				res = res2;
		}
	}
	else
	{
		volatile const unsigned int *puiSpareArea;
		BITSET(s_pECC->ECC_CTRL, ECC_CTRL_MODE_REFER_TO_NFC_MEMORY);
		puiSpareArea = s_pECC->NFC_MEMORY;
		NAND_Util_memcpy(puiTag,(const void*)&puiSpareArea[1],TNFTL_TAG_BYTE_SIZE);
		BITCLR(s_pECC->ECC_CTRL, ECC_CTRL_MODE_REFER_TO_NFC_MEMORY);
	}

	if( res == NAND_IO_SUCCESS && ( nDevInfo->Feature.UsableFunc & F_CR ) == 0 && fNextPageForNextTurn != 0 )
	{
		unsigned int uiNextPageAddress = uiPageAddress + 1;
		NAND_IO_Send_ReadCommand( nDevInfo, uiNextPageAddress, 0 );
		nDevInfo->fReadCommandSent = TRUE;
	}

	return res;
}
#endif

static NAND_IO_ERROR_T _NAND_IO_ReadPage(NAND_IO_DEVINFO *nDevInfo,unsigned int uiPageAddress,unsigned int *puiTag, unsigned int uiTagSize)
{
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS, res2;

	//=============================================
	// Read Data
	//=============================================
	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	//ND_TRACE("[R:%d]: %d\t(B:%d,\t%d)\n", nDevInfo->ucDieIndex, uiPageAddress&(~(0x1<<20)), (uiPageAddress&(~(0x1<<20)))>>nDevInfo->ShiftPpB, uiPageAddress&(nDevInfo->Feature.PpB-1));

	if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
	{
		NAND_IO_Send_ReadCommand( nDevInfo, uiPageAddress, 0 );

		/* Wait until it is ready */
		NAND_IO_WaitBusy( nDevInfo->ChipNo );

		res = NAND_IO_ReadStatusOfReadECC(nDevInfo);
	}
	else
	{
		if( res == NAND_IO_SUCCESS )
		{
			NAND_IO_Send_ReadCommand( nDevInfo, uiPageAddress, (unsigned int)nDevInfo->Feature.PageSize );

			/* Wait until it is ready */
			NAND_IO_WaitBusy( nDevInfo->ChipNo );

			/* Change Cycle */
			NAND_IO_SetReadCycleTime();

			res = NAND_IO_ReadSpareArea( nDevInfo, uiPageAddress, puiTag, uiTagSize );
			if( res == NAND_IO_SUCCESS )
			{
				/* Command Random Data Output [ 0x05 ] for Advance NandFlash */
				s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x0505;

				NAND_IO_WriteColAddr( 0, nDevInfo );

				/* Command Random Data Output [ 0xE0 ] for Advance NandFlash */
				s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0xE0E0;
			}
			else
			{
				if( sNAND_IO_fSuppressMessage == FALSE )
				{
					ND_TRACE( "_NAND_IO_ReadPage(page_addr=%d) is failed.(spare) (res=%X)\n", uiPageAddress, res );
				}
			}
		}
	}

	/* Read Data */
	/* Change Cycle */
	NAND_IO_SetReadCycleTime();

	res2 = NAND_IO_ReadDataArea( nDevInfo,
	                            s_puiDMA_WorkBuffer,
	                            s_puiDMA_PhyBuffer );

	if( res2 == NAND_IO_CHECK_BOOTAREA_PAGE)
	{
		if( NAND_IO_CheckBootPage( (unsigned char*)puiTag ) == TRUE )
			res2 = NAND_IO_SUCCESS;
		else
			res2 = NAND_IO_ERROR_ECC_CORRECTION_FAILURE;
	}
	
	if( res2 != NAND_IO_SUCCESS )
	{
		if( res == NAND_IO_SUCCESS )
		{
			res = res2;
		}
		if( sNAND_IO_fSuppressMessage == FALSE )
		{
			ND_TRACE( "_NAND_IO_ReadPage(page_addr=%d) is failed. (res=%X)\n", uiPageAddress, res );
		}
	}

	if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
	{
		/* Change Cycle */
		NAND_IO_SetReadCycleTime();

		NAND_IO_ReadTag( nDevInfo, uiPageAddress, puiTag, uiTagSize );
	}

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_ReadPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPageForNextTurn )
{
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;
	NAND_IO_DEVINFO		*nDevInfo = &sNAND_IO_DeviceInfo;
	unsigned int		fReadRetry;
	unsigned int		uiRetryStep = 0;

	#if defined(NAND_FOR_KERNEL) && defined(_LINUX_)
	rmb();
	#endif	

	NAND_IO_ResetEccErrorBitCount();

	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, nDevInfo, &uiPageAddress);
	//NAND_IO_CheckProgramSequence(&sNAND_IO_DeviceInfo,PROGRAM_END,uiPageAddress);

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
	{
		NAND_IO_SetLastError( NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY );
		return FALSE;
	}

	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_BD_Enable_WP_Port();

	if( sNAND_IO_DeviceInfo.fInterleaveUsable )
		res = NAND_IO_WaitBusyForInterleave( &sNAND_IO_DeviceInfo );

	do
	{
		fReadRetry = FALSE;

		#if defined(NAND_SUPPORT_NDMA)
		if(NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_NDMA_MODE )
		{
			res = _NAND_IO_NDMA_ReadPage(nDevInfo,uiPageAddress,puiTag,uiTagSize,fNextPageForNextTurn);
		}
		else
		#endif
		{
			res = _NAND_IO_ReadPage(nDevInfo,uiPageAddress,puiTag,uiTagSize);				
		}

		if( res != NAND_IO_SUCCESS )
		{
			if( nDevInfo->fnSetReadRetry )
			{
				if( nDevInfo->fnSetReadRetry(nDevInfo, ++uiRetryStep) == TRUE )
				{
					fReadRetry = TRUE;
				}
			}
		}
	} while( fReadRetry );

	if( uiRetryStep && nDevInfo->fnSetReadRetry )
	{
		ND_DEBUG("ReadRetry(step=%d) is operated. (PBA=%d,PPA=%d)\n",uiRetryStep,uiPageAddress>>pTnftlObj->uiPagesPerBlockShift,uiPageAddress);
		nDevInfo->fnSetReadRetry(nDevInfo, 0);
	}

	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	if( nDevInfo->Feature.MediaType & A_RAND_IO && !nDevInfo->fSkipDataAreaRandomizing )
	{
		if( NAND_IO_IsAllValue( s_puiDMA_WorkBuffer, 0xFF, nDevInfo->Feature.PageSize ) )
		{
			NAND_Util_memcpy( (void *)puiBuffer, (const void *)s_puiDMA_WorkBuffer, nDevInfo->Feature.PageSize );
		}
		else
		{
			TNFTL_RAND_SetAddress( (unsigned int)nDevInfo->Feature.PpB, uiPageAddress, 0 );
			//TNFTL_RAND_Randomize( (void *)puiBuffer, (const void *)s_puiDMA_WorkBuffer, (unsigned int)nDevInfo->Feature.PageSize );
			NAND_Util_memcpy((void *)sNAND_IO_uiRandomizeBuffer, (const void *)s_puiDMA_WorkBuffer, (unsigned int)nDevInfo->Feature.PageSize);
			TNFTL_RAND_Randomize( (void *)puiBuffer, (const void *)sNAND_IO_uiRandomizeBuffer, (unsigned int)nDevInfo->Feature.PageSize );	
		}
	}
	else
	{
		NAND_Util_memcpy( (void *)puiBuffer, (const void *)s_puiDMA_WorkBuffer, nDevInfo->Feature.PageSize );
	}

	if( res != NAND_IO_SUCCESS )
	{
		NAND_IO_SetLastError( res );
		NAND_IO_ResetEccErrorBitCount();
		return FALSE;
	}

	return TRUE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_ReadSpare( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiTag, unsigned int uiTagSize )
{
	unsigned int		ColumnAddr;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS, res2;
	NAND_IO_DEVINFO		*nDevInfo = &sNAND_IO_DeviceInfo;
	unsigned int		fReadRetry;
	unsigned int		uiRetryStep = 0;

	NAND_IO_ResetEccErrorBitCount();

	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, nDevInfo, &uiPageAddress);
	//NAND_IO_CheckProgramSequence(&sNAND_IO_DeviceInfo,PROGRAM_END,uiPageAddress);

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
	{
		NAND_IO_SetLastError( NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY );
		return FALSE;
	}

	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_BD_Enable_WP_Port();

	if( sNAND_IO_DeviceInfo.fInterleaveUsable )
		res = NAND_IO_WaitBusyForInterleave( &sNAND_IO_DeviceInfo );

	// Shift column_address for reading tag only
	ColumnAddr = nDevInfo->Feature.PageSize;
	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		ColumnAddr = ColumnAddr + 4/*Factory Bad Marker*/ + ( nDevInfo->EccCodeSizeTotalInDword << 2 );

	do
	{
		res = NAND_IO_SUCCESS;
		fReadRetry = FALSE;

		//=============================================
		// Read Data
		//=============================================
		/* Clear READY_FLAG */
		s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

		NAND_IO_Send_ReadCommand(nDevInfo, uiPageAddress, ColumnAddr);

		/* Wait until it is ready */
		NAND_IO_WaitBusy( nDevInfo->ChipNo );

		if( nDevInfo->EccType == ECC_TYPE_EMBEDDED )
		{
			res = NAND_IO_ReadStatusOfReadECC(nDevInfo);
		}

		/* Change Cycle */
		NAND_IO_SetReadCycleTime();

		/* Read Spare data from NANDFLASH */
		res2 = NAND_IO_ReadTag( nDevInfo, uiPageAddress, puiTag, uiTagSize );
		if( res == NAND_IO_SUCCESS )
		{
			res = res2;
		}

		if( res != NAND_IO_SUCCESS )
		{
			if( nDevInfo->fnSetReadRetry )
			{
				if( nDevInfo->fnSetReadRetry(nDevInfo, ++uiRetryStep) == TRUE )
				{
					fReadRetry = TRUE;
				}
			}
		}
	} while( fReadRetry );

	if( uiRetryStep && nDevInfo->fnSetReadRetry )
	{
		ND_DEBUG("ReadRetry is operated. (PBA=%d,PPA=%d)\n",uiPageAddress>>pTnftlObj->uiPagesPerBlockShift,uiPageAddress);

		// Reset Read-Retry mode
		nDevInfo->fnSetReadRetry(nDevInfo, 0/*step0=reset*/);
	}

	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	if( res != NAND_IO_SUCCESS )
	{
		if( sNAND_IO_fSuppressMessage == FALSE )
			ND_TRACE( "NAND_IO_ReadSpare(page_addr=%d) is failed.\n", uiPageAddress );
		NAND_IO_SetLastError( res );
		NAND_IO_ResetEccErrorBitCount();
		return FALSE;
	}

	return TRUE;
}

unsigned int NAND_IO_IsReadReclaimRequired( TNFTL_OBJECT_T *pTnftlObj )
{
	NAND_IO_DEVINFO *nDevInfo = &sNAND_IO_DeviceInfo;
	unsigned int fReadReclaimRequired = FALSE;

	if(nDevInfo->EccType == ECC_TYPE_EMBEDDED)
	{
	}
	else
	{
		const NAND_IO_ECC_INFO *pECC_Info = NAND_IO_GetECCInfo( nDevInfo->EccType );
		if( ( NAND_IO_GetEccErrorBitCount() > pECC_Info->ucReadReclaimThreshold ) && ( nDevInfo->fnSetReadRetry ))
		{
			fReadReclaimRequired = TRUE;
		}
	}

	return fReadReclaimRequired;
}

unsigned int NAND_IO_IsBL1ReadReclaimRequired( unsigned int uiBL1EccErrorBit ) 
{
	NAND_IO_DEVINFO *nDevInfo = &sNAND_IO_DeviceInfo;
	unsigned int fReadReclaimRequired = FALSE;

	if(nDevInfo->EccType == ECC_TYPE_EMBEDDED)
	{
	}
	else
	{
		const NAND_IO_ECC_INFO *pECC_Info = NAND_IO_GetECCInfo( nDevInfo->EccType );
		//if( nDevInfo->fReadRetryNow && uiBL1EccErrorBit > pECC_Info->ucReadReclaimThreshold )
		if( uiBL1EccErrorBit > pECC_Info->ucReadReclaimThreshold )
		{
			fReadReclaimRequired = TRUE;
		}
	}

	return fReadReclaimRequired;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_WriteUserSizeData( NAND_IO_DEVINFO *nDevInfo, unsigned short nColumnAddr, unsigned long nWriteSize, unsigned char *nWriteBuffer )
{
	unsigned char		*pPageB;
	DWORD_BYTE			uDWordByte;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// Check Parameter
	//=========================================================================
	if( ( unsigned long )( nColumnAddr + nWriteSize ) > ( unsigned long )( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Write UserSize Data
	//=========================================================================
	pPageB = ( unsigned char * )nWriteBuffer;

	while( nWriteSize )
	{
		/* Write as DWORD */
		if( nWriteSize >= 4 )
		{
			uDWordByte.BYTE[0] = *pPageB; ++pPageB;
			uDWordByte.BYTE[1] = *pPageB; ++pPageB;
			uDWordByte.BYTE[2] = *pPageB; ++pPageB;
			uDWordByte.BYTE[3] = *pPageB; ++pPageB;
			s_pNFC->NFC_WDATA  = uDWordByte.DWORD;
			//WORD_OF( s_pNFC->NFC_WDATA ) = uDWordByte.DWORD;
			nWriteSize -= 4;
		}
		/* Write as WORD */
		else if( nWriteSize >= 2 )
		{
			uDWordByte.BYTE[0] = *pPageB; ++pPageB;
			uDWordByte.BYTE[1] = *pPageB; ++pPageB;			
			s_pNFC->NFC_MDATA = uDWordByte.WORD[0];
			//HWORD_OF( s_pNFC->NFC_WDATA ) = uDWordByte.WORD[0];
			nWriteSize -= 2;
		}
		/* Write as BYTE */
		else
		{
			uDWordByte.BYTE[0] = *pPageB; ++pPageB;
			s_pNFC->NFC_SDATA = uDWordByte.BYTE[0];
			//BYTE_OF( s_pNFC->NFC_WDATA ) = uDWordByte.BYTE[0];
			nWriteSize -= 1;
		}
	}

	//=========================================================================
	// Return
	//=========================================================================
	return NAND_IO_SUCCESS;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_WriteUserSizePage( NAND_IO_DEVINFO *nDevInfo, unsigned long nPageAddr,
        unsigned short nColumnAddr, unsigned long nWriteSize, unsigned char *nWriteBuffer )
{
	unsigned int		RowAddr, ColumnAddr;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
		return NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY;

	if( ( unsigned long )( nColumnAddr + nWriteSize ) > ( unsigned short )( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	if( NAND_BD_Disable_WP_Port() != TRUE )
		return NAND_IO_ERROR_WRITE_PROTECTED;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	if( sNAND_IO_DeviceInfo.fInterleaveUsable )
		res = NAND_IO_WaitBusyForInterleave( &sNAND_IO_DeviceInfo );

	//=============================================
	// Write UserSize Data
	//=============================================

	/* Generate Row and Column Address */
	NAND_IO_GenerateRowColAddr( nPageAddr, nColumnAddr, &RowAddr, &ColumnAddr, nDevInfo );

	/* Command Page Program #1 [ 0x80 ] */
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x8080;

	/* Write Row and Column Address */
	NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, nDevInfo );

	/* Change Cycle */
	NAND_IO_SetWriteCycleTime();

	/* Write Data to NAND FLASH */
	res = NAND_IO_WriteUserSizeData( nDevInfo,
	                                 nColumnAddr,
	                                 nWriteSize,
	                                 nWriteBuffer );
	/* Change Cycle */
	NAND_IO_SetCommCycleTime();

	if( res == NAND_IO_SUCCESS )
	{
		s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

		/* Command Page Program #2 [ 0x10 ] */
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x1010;
		/* Wait until it is ready */
		NAND_IO_WaitBusy( nDevInfo->ChipNo );

		/* Check Status */
		res = NAND_IO_ReadStatus( nDevInfo );
		if( res != NAND_IO_SUCCESS )
		{
			res = NAND_IO_ERROR_WRITE_FAILURE;
		}
	}

	NAND_BD_Enable_WP_Port();
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	if( res != NAND_IO_SUCCESS )
		return res;

	return NAND_IO_SUCCESS;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadUserSizeData( NAND_IO_DEVINFO *nDevInfo, unsigned short nColumnAddr, unsigned long nReadSize, unsigned char *nReadBuffer )
{
	unsigned char		*pPageB = 0;
	DWORD_BYTE			uDWordByte;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// Check Parameter
	//=========================================================================
	if( ( unsigned long )( nColumnAddr + nReadSize ) > ( unsigned short )( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Read UserSize Data
	//=========================================================================

	/* Adapt type of address */
	pPageB = ( unsigned char * )nReadBuffer;

	while( nReadSize )
	{
		/* Read as DWORD */
		if( nReadSize >= 4 )
		{
			//uDWordByte.DWORD = WORD_OF( s_pNFC->NFC_WDATA );
			uDWordByte.DWORD = s_pNFC->NFC_WDATA;
			*pPageB = uDWordByte.BYTE[0]; ++pPageB;
			*pPageB = uDWordByte.BYTE[1]; ++pPageB;
			*pPageB = uDWordByte.BYTE[2]; ++pPageB;
			*pPageB = uDWordByte.BYTE[3]; ++pPageB;
			nReadSize -= 4;
		}
		/* Read as WORD */
		else if( nReadSize >= 2 )
		{
			//uDWordByte.WORD[0] = HWORD_OF( s_pNFC->NFC_WDATA );
			uDWordByte.WORD[0] = s_pNFC->NFC_MDATA;
			*pPageB = uDWordByte.BYTE[0]; ++pPageB;
			*pPageB = uDWordByte.BYTE[1]; ++pPageB;
			nReadSize -= 2;
		}
		/* Read as BYTE */
		else
		{
			//uDWordByte.BYTE[0] = BYTE_OF( s_pNFC->NFC_WDATA );
			uDWordByte.BYTE[0] = s_pNFC->NFC_SDATA;
			*pPageB = uDWordByte.BYTE[0]; ++pPageB;
			nReadSize -= 1;
		}
	}

	//=========================================================================
	// Return
	//=========================================================================
	return NAND_IO_SUCCESS;

}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_ReadUserSizePage( NAND_IO_DEVINFO *nDevInfo, unsigned long nPageAddr,
                                          unsigned short nColumnAddr, unsigned long nReadSize, unsigned char *nReadBuffer )
{
	unsigned int		RowAddr, ColumnAddr;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
		return NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY;

	if( ( unsigned long )( nColumnAddr + nReadSize ) > ( unsigned short )( nDevInfo->Feature.PageSize + nDevInfo->Feature.SpareSize ) )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_BD_Enable_WP_Port();

	if( sNAND_IO_DeviceInfo.fInterleaveUsable )
		res = NAND_IO_WaitBusyForInterleave( &sNAND_IO_DeviceInfo );
	
	//=============================================
	// Read UserSize Data
	//=============================================

	/* Generate Row and Column Address */
	NAND_IO_GenerateRowColAddr( nPageAddr, nColumnAddr, &RowAddr, &ColumnAddr, nDevInfo );

	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	/* Command READ [ 0x00 ] */
	s_pNFC->NFC_CMD = 0x0000;

	/* Write Row and Column Address */
	NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, nDevInfo );

	/* Command READ2 [ 0x30 ] for Advance NandFlash */
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x3030;

	/* Wait until it is ready */
	NAND_IO_WaitBusy( nDevInfo->ChipNo );

	/* Change Cycle */
	NAND_IO_SetReadCycleTime();

	/* Read User Size data from NANDFLASH */
	res = NAND_IO_ReadUserSizeData( nDevInfo,
	                                nColumnAddr,
	                                nReadSize,
	                                nReadBuffer );
	/* Change Cycle */
	NAND_IO_SetCommCycleTime();

	if( res != NAND_IO_SUCCESS )
		goto ErrorReadUserSizePage;

ErrorReadUserSizePage:
	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	if( res != NAND_IO_SUCCESS )
		return res;

	return NAND_IO_SUCCESS;
}

#if defined(NFC_VER_50)
/******************************************************************************
*
*	void		NAND_IO_AdjustPageSpareSize
*
*	Input	:
*	Output	:
*	Return	:
*
*	Description :
*
*******************************************************************************/
void NAND_IO_AdjustPageSpareSize( unsigned short *pusPPages, unsigned short *pusPageSize, unsigned short *pusSpareSize, unsigned int *pECCType )
{
	NAND_IO_DEVINFO *pstDevInfo = &sNAND_IO_DeviceInfo;
	
	if ( ( pstDevInfo->Feature.PageSize == 8192 ) && ( ( pstDevInfo->Feature.MediaType & A_PARALLEL ) == 0) )
	{
		//PageSize: 8K Serial Mode
		if(pusPPages)
			*pusPPages 		= 8;
		if(pusPageSize)
			*pusPageSize 		= 4096;
		if(pusSpareSize)
			*pusSpareSize	 	= 192;
	}
	else if ( ( pstDevInfo->Feature.PageSize > 8192 ) && ( ( pstDevInfo->Feature.MediaType & A_PARALLEL ) != 0 ) )
	{
		//PageSize: 8K Parallel Mode
		if(pusPPages)
			*pusPPages 		= ( 8 << 1 );
		if(pusPageSize)
			*pusPageSize 		= ( 4096 << 1 );
		if(pusSpareSize)
			*pusSpareSize 		= ( 192 << 1 );
	}
	else
	{
		if(pusPPages)
			*pusPPages 		= pstDevInfo->Feature.PageSize / 512;
		if(pusPageSize)
			*pusPageSize	 	= pstDevInfo->Feature.PageSize;
		if(pusSpareSize)
			*pusSpareSize	 	= pstDevInfo->Feature.SpareSize;
	}

	*pECCType = pstDevInfo->EccType;
}


static void NAND_IO_EncodeECC_For_TCC800X( unsigned char* nSpareBuffer )
{
	unsigned int		nMLC_ECC;
	
	nMLC_ECC = s_pECC->ECC_CODE0;
	*(nSpareBuffer +  8) = nMLC_ECC;
	*(nSpareBuffer +  9) = nMLC_ECC >> 8;
	*(nSpareBuffer + 10) = nMLC_ECC >> 16;
	*(nSpareBuffer + 11) = nMLC_ECC >> 24;

	nMLC_ECC = s_pECC->ECC_CODE1;
	*(nSpareBuffer + 12) = nMLC_ECC;
	*(nSpareBuffer + 13) = nMLC_ECC >> 8;
	*(nSpareBuffer + 14) = (nMLC_ECC >> 16) & 0x0F;
}


static void NAND_IO_WriteSpareArea_For_TCC800X( NAND_IO_DEVINFO *nDevInfo, unsigned int *puiSpareBuffer )
{
	unsigned int		i;
	unsigned int		uiTag[TNFTL_TAG_DWORD_SIZE];
	unsigned int		uiAllTagSize = 12;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//==============================================================
	// Prepare Tag
	//==============================================================
	NAND_Util_memcpy(uiTag, puiSpareBuffer, uiAllTagSize);


	if( !( s_pNFC->NFC_CTRL1 & NFC_CTRL1_DMA_ACK_ENABLE ) )
	{
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			/* Setup ECC Block */
#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, ECC_TYPE_4BIT, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
#else
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, ECC_TYPE_4BIT, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
#endif
			s_pECC->ECC_CTRL	|= ( uiAllTagSize << ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	= 0x00000000;
		}

		/* Write 12Bytes Tag */
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_1 );	// 1R/W Burst Size
		s_pNFC->NFC_DSIZE		= uiAllTagSize;								// NFC DSIZE SETTING

		s_pNFC->NFC_IRQ = NFC_IRQ_PROGRAM_FLAG;
		s_pNFC->NFC_PSTART  = 0;

		for( i = 0 ; i < uiAllTagSize>>2 ; i++ )
		{
			while( !( s_pNFC->NFC_CTRL & NFC_CTRL_FIFO_READY_TO_BURST_ACCESS ) );

			s_pNFC->NFC_LDATA = uiTag[i];
		}

		while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );
	}
	else
	{
		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			/* Setup ECC Block */
#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, ECC_TYPE_4BIT, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
			NAND_IO_SetupECC( ( unsigned short )ECC_ON, ECC_ENCODE, ECC_TYPE_4BIT, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif
			s_pECC->ECC_CTRL	|= ( uiAllTagSize << ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	= 0x00000000;
		}

		for( i = 0 ; i < uiAllTagSize>>2 ; i++ )
		{
			s_pNFC->NFC_WDATA = uiTag[i];
		}
	}

	//==============================================================
	// Write Tag ECC
	//==============================================================
	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
	{
		unsigned long		ulTagEccBuffer[ECC_DATA_MAX_DWORD];
		unsigned int		uiEccDataSizeInDword;

		// Clear ECC Buffer
		memset( &ulTagEccBuffer[0], 0xFF, sizeof( ulTagEccBuffer ) );

		/*	Load ECC code from ECC block */
		NAND_IO_EncodeECC( ECC_TYPE_4BIT, ulTagEccBuffer, 8 /*ECC Data size for 4Bit MLC-ECC*/ );

		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

		/* Write ECC data */
		uiEccDataSizeInDword = 8; /*ECC Data size for 4Bit MLC-ECC*/
		for( i = 0 ; i < uiEccDataSizeInDword ; i++ )
		{
			s_pNFC->NFC_WDATA = ulTagEccBuffer[i];
		}
	}

}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static void NAND_IO_WriteDataAndSpareArea_For_TCC800X( NAND_IO_DEVINFO *nDevInfo, unsigned int uiPageAddress, unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer )
{
	unsigned int		j;
	unsigned char		*pucSharedEccBuf = 0;
	unsigned int 		*puiDmaBuffer;// , *puiDmaPhysicalBuffer;
	unsigned int		uiEccDataSize;
	unsigned int		uiTag[28], *puiTagBuffer;   // Max ECC Data 20 + Tag Info 8

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	puiDmaBuffer = s_puiDMA_WorkBuffer;
	//puiDmaPhysicalBuffer = s_puiDMA_PhyBuffer;	

	NAND_Util_memcpy( (void *)puiDmaBuffer, (const void *)puiPageBuffer, (unsigned int)nDevInfo->Feature.PageSize );

	// Set SpareBuffer Pointer =>> ECCBuffer
	memset( &sNAND_IO_uiSharedEccBuffer[0], 0xFF, sizeof( sNAND_IO_uiSharedEccBuffer ) );
	pucSharedEccBuf = ( unsigned char * )&sNAND_IO_uiSharedEccBuffer[0];
	

	//----------------------------------------------
	//	Write Data as Codeword repeatly
	//----------------------------------------------
	for( j = 0; j < nDevInfo->usCodewordsPerPage ; ++j )
	{
		//####################################################
		//#	Write Codeword
		//####################################################
		//----------------------------------------------
		//	MCU ACCESS
		//----------------------------------------------
		//if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_MCU_MODE ) 
		{
			if( !( s_pNFC->NFC_CTRL1 & NFC_CTRL1_DMA_ACK_ENABLE ) )
			{
				unsigned int i;
				if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwLDATA_PA );
					#else
					NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_LDATA );
					#endif
					s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
					s_pECC->ECC_CLEAR	= 0x00000000;
				}

				/* Write 512 Data Area */
				BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BURST_SIZE_FIELD, NFC_CTRL_BURST_SIZE_1 );	// 1R/W Burst Size
				s_pNFC->NFC_DSIZE		= nDevInfo->CodewordSize;					// NFC DSIZE SETTING

				s_pNFC->NFC_IRQ	= NFC_IRQ_PROGRAM_FLAG;
				s_pNFC->NFC_PSTART	= 0;

				i = ( nDevInfo->CodewordSize >> 2 );
				do
				{
					while( !( s_pNFC->NFC_CTRL & NFC_CTRL_FIFO_READY_TO_BURST_ACCESS ) );
					
					s_pNFC->NFC_LDATA = *puiDmaBuffer++;
					
				} while( --i );

				while( ISZERO( s_pNFC->NFC_IRQ, NFC_IRQ_PROGRAM_FLAG ) );

			}
			else
			{
				unsigned int i;
				if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
				{
					/* Setup ECC Block */
					#if defined(_WINCE_) || defined(_LINUX_)
					NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
					#else
					NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
					#endif
					
					s_pECC->ECC_CTRL	|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
					s_pECC->ECC_CLEAR	= 0x00000000;

				}

				i = ( nDevInfo->CodewordSize >> 2 );
				do
				{
					s_pNFC->NFC_WDATA = *puiDmaBuffer++;
				} while( --i );
			}
		}
		//else if( NAND_IO_GetTransperMode() == NAND_IO_TRANSPER_GDMA_MODE )
		//{
		//}		
		
		//####################################################
		//####################################################

		if ( ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_8BIT) ||
			 ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_12BIT) )
		{
			/*	Load ECC code from ECC block */
			if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
			{
				NAND_IO_EncodeECC( nDevInfo->EccType, pucSharedEccBuf, nDevInfo->EccCodeSize );
				uiEccDataSize = nDevInfo->EccCodeSize;
				NAND_Util_memcpy( uiTag, pucSharedEccBuf, uiEccDataSize );
			}
		}
		else
		{
			NAND_IO_EncodeECC_For_TCC800X( (unsigned char*) puiSpareBuffer );
			uiEccDataSize = 16;
			NAND_Util_memcpy( uiTag, puiSpareBuffer, uiEccDataSize );
		}		

		uiEccDataSize = uiEccDataSize >> 2;
		puiTagBuffer = uiTag;
		
		do
		{
			s_pNFC->NFC_WDATA = *puiTagBuffer++;
		} while( --uiEccDataSize );
		
	}

	//=========================================================================
	// Write Spare Data
	//=========================================================================
	/* Change Cycle */
	NAND_IO_SetWriteCycleTime();
	if ( ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_8BIT) ||
		 ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_12BIT) )
	{
		NAND_IO_WriteSpareArea_For_TCC800X( nDevInfo, puiSpareBuffer );
	}

}


NAND_IO_ERROR_T NAND_IO_WritePage_For_TCC800X( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress,
											   unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer )
{
	unsigned int		RowAddr, ColumnAddr;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;
	NAND_IO_DEVINFO		*nDevInfo = &sNAND_IO_DeviceInfo;

	if( NAND_BD_Disable_WP_Port() != TRUE )
		return NAND_IO_ERROR_WRITE_PROTECTED;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_IO_GenerateRowColAddr( uiPageAddress, 0, &RowAddr, &ColumnAddr, nDevInfo );

	/* Command Page Program #1 */
	{
		unsigned int uiCommand;
		uiCommand = 0x8080;
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;
	}

	/* Write Row and Column Address */
	NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, nDevInfo );

	/* Change Cycle */
	NAND_IO_SetWriteCycleTime();

	/* Write Data to NAND FLASH */	
	{
		NAND_IO_WriteDataAndSpareArea_For_TCC800X( nDevInfo,
					                             uiPageAddress,
					                             puiPageBuffer,
					                             puiSpareBuffer);
	}

	/* Change Cycle */
	NAND_IO_SetCommCycleTime();

	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	{
	/* Command Page Program #2 */
		unsigned int uiCommand;
		uiCommand = 0x1010;
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;

		NAND_IO_WaitBusy( nDevInfo->ChipNo );
	}

	res = NAND_IO_ReadStatus(nDevInfo);

	//=============================================
	// FORCE TO SET WP LOW
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	if( res == NAND_IO_SUCCESS )
	{
		NAND_BD_Enable_WP_Port();
	}
	else
	{
		ND_ERROR("WritePage Failure!!! (PPA=%d,res=%x)\n",uiPageAddress,res);
		NAND_BD_Enable_WP_Port();
	}

	return res;
}


NAND_IO_ERROR_T NAND_IO_WriteGoldenPage_For_TCC800X( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress,
												   unsigned int *puiPageBuffer, unsigned int *puiSpareBuffer )
{
	unsigned int		RowAddr, ColumnAddr;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;
	NAND_IO_DEVINFO		*nDevInfo = &sNAND_IO_DeviceInfo;

	if( NAND_BD_Disable_WP_Port() != TRUE )
		return NAND_IO_ERROR_WRITE_PROTECTED;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_IO_GenerateRowColAddr( uiPageAddress, 0, &RowAddr, &ColumnAddr, nDevInfo );

	/* Command Page Program #1 */
	{
		unsigned int uiCommand;
		uiCommand = 0x8080;
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;
	}

	/* Write Row and Column Address */
	NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, nDevInfo );

	/* Change Cycle */
	NAND_IO_SetWriteCycleTime();

	/* Write Data (Serial Number)*/	
	{
		NAND_IO_WriteUserSizeData( nDevInfo, 0, nDevInfo->Feature.PageSize, (unsigned char*)puiPageBuffer );
	}
	/* Write Spare Data (Boot Parameter For Bootcode)*/	
	{
		NAND_IO_WriteUserSizeData( nDevInfo, nDevInfo->Feature.PageSize, nDevInfo->Feature.SpareSize, (unsigned char*)puiSpareBuffer );
	}

	/* Change Cycle */
	NAND_IO_SetCommCycleTime();

	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	{
	/* Command Page Program #2 */
		unsigned int uiCommand;
		uiCommand = 0x1010;
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & uiCommand;

		NAND_IO_WaitBusy( nDevInfo->ChipNo );
	}

	res = NAND_IO_ReadStatus(nDevInfo);

	//=============================================
	// FORCE TO SET WP LOW
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	if( res == NAND_IO_SUCCESS )
	{
		NAND_BD_Enable_WP_Port();
	}
	else
	{
		ND_ERROR("WritePage Failure!!! (PPA=%d,res=%x)\n",uiPageAddress,res);
		NAND_BD_Enable_WP_Port();
	}

	return res;
}


/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_ReadDataArea_For_TCC800X( NAND_IO_DEVINFO *nDevInfo, unsigned int *puiDmaBuffer, unsigned int *puiTagBuffer )
{
	unsigned int		 uiTag[16/4];
	unsigned int		j;
	unsigned int 		*puiSharedEccBuf = ( unsigned int* )&sNAND_IO_uiSharedEccBuffer[0];
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	res = NAND_IO_SUCCESS;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_16BIT );
	else
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//----------------------------------------------
	//	Read Data as Codeword repeatly
	//----------------------------------------------
	for( j = 0; j < nDevInfo->usCodewordsPerPage ; ++j )
	{
		//####################################################
		//#	Read 512 Page Data
		//####################################################
		//----------------------------------------------
		//	MCU ACCESS
		//----------------------------------------------
		unsigned int *puiDataBuf = puiDmaBuffer;
		unsigned int i;

		if( nDevInfo->EccType != ECC_TYPE_EMBEDDED )
		{
			/* Setup ECC Block */
			#if defined(_WINCE_) || defined(_LINUX_)
			NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
			#else
			NAND_IO_SetupECC( ECC_ON, ECC_DECODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
			#endif
			
			s_pECC->ECC_CTRL		|= ( nDevInfo->CodewordSize << ECC_SHIFT_DATASIZE );
			s_pECC->ECC_CLEAR	 = 0x00000000;
		}

		/* Read 512 Data Area */
		i = ( nDevInfo->CodewordSize >> 2 );
		do
		{
			*puiDataBuf++ = s_pNFC->NFC_WDATA;
		} while( --i );
			
		//####################################################
		//####################################################

		if ( ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_8BIT) ||
			 ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_12BIT) )
		{
			unsigned int *puiEccBuf = puiSharedEccBuf;
			unsigned int uiReadTagSize;

			uiReadTagSize = nDevInfo->EccCodeSize >> 2;
			do
			{
				*puiEccBuf++ = s_pNFC->NFC_WDATA;
			} while( --uiReadTagSize );
			
		}
		else
		{
			uiTag[0] = s_pNFC->NFC_WDATA;
			uiTag[1] = s_pNFC->NFC_WDATA;
			puiSharedEccBuf[0] = s_pNFC->NFC_WDATA;
			puiSharedEccBuf[1] = s_pNFC->NFC_WDATA;			

			memcpy( puiTagBuffer, uiTag, 8 );
		}

		/* Check and Correct ECC code */
		{
			NAND_IO_ERROR_T resTemp = NAND_IO_CorrectionMLC( nDevInfo, ( unsigned char*)puiDmaBuffer, (unsigned char*)puiSharedEccBuf, nDevInfo->CodewordSize );
			if( resTemp != NAND_IO_SUCCESS && resTemp != NAND_IO_ERASED_PAGE )
				res |= resTemp;
		}

		puiDmaBuffer = &puiDmaBuffer[( nDevInfo->CodewordSize ) >> 2];
	}	
	
	//=========================================================================
	// Return
	//=========================================================================
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	if ( ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_8BIT) ||
		 ( GET_ECC_TYPE_FROM_MEDIATYPE( nDevInfo->Feature.MediaType ) == ECC_TYPE_12BIT) )
	{
		/* Setup ECC Block */
		#if defined(_WINCE_) || defined(_LINUX_)
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, ECC_TYPE_4BIT, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
		#else
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, ECC_TYPE_4BIT, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
		#endif		

		s_pECC->ECC_CTRL		|= ( 12 << ECC_SHIFT_DATASIZE );
		s_pECC->ECC_CLEAR	 = 0x00000000;

		uiTag[0] = s_pNFC->NFC_WDATA;
		uiTag[1] = s_pNFC->NFC_WDATA;
		uiTag[2] = s_pNFC->NFC_WDATA;
		puiSharedEccBuf[0] = s_pNFC->NFC_WDATA;
		puiSharedEccBuf[1] = s_pNFC->NFC_WDATA;			

		res |= NAND_IO_CorrectionMLC( nDevInfo, (unsigned char*)uiTag, (unsigned char*)puiSharedEccBuf, 12 );		
	}
	

	return res;
}

NAND_IO_ERROR_T NAND_IO_ReadPage_For_TCC800X( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress,
													unsigned int *puiBuffer, unsigned int *puiTag )
{
	NAND_IO_DEVINFO	*nDevInfo = &sNAND_IO_DeviceInfo;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
	{
		NAND_IO_SetLastError( NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY );
		return FALSE;
	}
	
	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_BD_Enable_WP_Port();
	
	//=============================================
	// Read Data
	//=============================================
	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	NAND_IO_SetCommCycleTime();

	NAND_IO_Send_ReadCommand( nDevInfo, uiPageAddress, 0 );

	/* Wait until it is ready */
	NAND_IO_WaitBusy( nDevInfo->ChipNo );

	/* Change Cycle */
	NAND_IO_SetReadCycleTime();

	res = NAND_IO_ReadDataArea_For_TCC800X( nDevInfo,
	                            s_puiDMA_WorkBuffer,
	                            puiTag );

	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	NAND_Util_memcpy( (void *)puiBuffer, (const void *)s_puiDMA_WorkBuffer, nDevInfo->Feature.PageSize );

	if( res != NAND_IO_SUCCESS )
	{
		NAND_IO_SetLastError( res );
		NAND_IO_ResetEccErrorBitCount();
	}

	return res;
}


NAND_IO_ERROR_T NAND_IO_ReadGoldenPage_For_TCC800X( TNFTL_OBJECT_T *pTnftlObj,unsigned int uiPageAddress,
													unsigned int *puiBuffer, unsigned int *puiTag )
{
	NAND_IO_DEVINFO	*nDevInfo = &sNAND_IO_DeviceInfo;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
	{
		NAND_IO_SetLastError( NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY );
		return FALSE;
	}
	
	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );

	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );

	NAND_BD_Enable_WP_Port();
	
	//=============================================
	// Read Data
	//=============================================
	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	NAND_IO_SetCommCycleTime();

	NAND_IO_Send_ReadCommand( nDevInfo, uiPageAddress, 0 );

	/* Wait until it is ready */
	NAND_IO_WaitBusy( nDevInfo->ChipNo );

	/* Change Cycle */
	NAND_IO_SetReadCycleTime();

	// Read Data of Golden Page. ( Serial Number )
	NAND_IO_ReadUserSizeData( nDevInfo, 0, nDevInfo->Feature.PageSize, (unsigned char*)s_puiDMA_WorkBuffer );

	// Read Data of Golden Page. ( Boot Parameter For BootCode. )
	NAND_IO_ReadUserSizeData( nDevInfo, nDevInfo->Feature.PageSize, nDevInfo->Feature.SpareSize, (unsigned char*)puiTag );	
	
	//=============================================
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	NAND_Util_memcpy( (void *)puiBuffer, (const void *)s_puiDMA_WorkBuffer, nDevInfo->Feature.PageSize );

	if( res != NAND_IO_SUCCESS )
	{
		NAND_IO_SetLastError( res );
		NAND_IO_ResetEccErrorBitCount();
	}

	return res;
}
#endif


/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_EraseBlock( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress )
{
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;
	unsigned int		uiBlockPageAddr;
	

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( sNAND_IO_DeviceInfo.IoStatus & NAND_IO_STATUS_ENABLE ) )
	{
		NAND_IO_SetLastError( NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY );
		return FALSE;
	}

	if( NAND_BD_Disable_WP_Port() != TRUE )
		return NAND_IO_ERROR_WRITE_PROTECTED;

	uiBlockPageAddr = uiBlockAddress << pTnftlObj->uiPagesPerBlockShift;
	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, &sNAND_IO_DeviceInfo, &uiBlockPageAddr);

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( &sNAND_IO_DeviceInfo );
	NAND_IO_SetCommCycleTime();

	NAND_IO_EnableChipSelect( sNAND_IO_DeviceInfo.ChipNo );
	if( sNAND_IO_DeviceInfo.fInterleaveUsable )		/* if it can not use interleaving */
		res = NAND_IO_WaitBusyForInterleave( &sNAND_IO_DeviceInfo );
		
	if( res == NAND_IO_SUCCESS )
	{
		//=============================================
		// Erase Block
		//=============================================
		/* Command Block Erase #1 [ 0x60 ] */
		s_pNFC->NFC_CMD = sNAND_IO_DeviceInfo.CmdMask & 0x6060;

		/* Write Block Address */
		NAND_IO_WriteBlockPageAddr( uiBlockPageAddr, &sNAND_IO_DeviceInfo );

		#if defined(NAND_FOR_KERNEL) && defined(__USE_NAND_ISR__)
		if( s_fIsNAND_RB_ISR_USED == TRUE )
			NAND_IO_RB_ISR( &sNAND_IO_DeviceInfo, NAND_IO_ERASE_MODE );
		else
		#endif
		{
			s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

			/* Command Erase Block #2 [ 0xD0 ] */
			s_pNFC->NFC_CMD = sNAND_IO_DeviceInfo.CmdMask & 0xD0D0;
		}

		if( !sNAND_IO_DeviceInfo.fInterleaveUsable )	/* if it can not use interleaving */
		{
			/* Wait until it is ready */
			NAND_IO_WaitBusy( sNAND_IO_DeviceInfo.ChipNo );
			/* Check Status */
			res = NAND_IO_ReadStatus( &sNAND_IO_DeviceInfo );
			if( res != NAND_IO_SUCCESS )
			{
				sNAND_IO_DeviceInfo.BadBlockInfo.BlockStatus[sNAND_IO_DeviceInfo.ChipNo] 	= MULTI_PLANE_BAD_BLOCK;
				sNAND_IO_DeviceInfo.BadBlockInfo.BadBlkPHYAddr[sNAND_IO_DeviceInfo.ChipNo]	= uiBlockPageAddr;
				NAND_BD_Enable_WP_Port();
			}
		}
		else	/* if it can use interleaving */
		{
			NAND_IO_SetInterleavingStatus( &sNAND_IO_DeviceInfo, uiBlockPageAddr, NAND_IO_ERASE );
		}

		NAND_IO_DisableChipSelect();
		NAND_IO_PostProcess();
	}

	// CAUTION : Do not change to else statement here.
	if( res != NAND_IO_SUCCESS )
	{
		NAND_IO_SetLastError( res );
		return FALSE;
	}

	return TRUE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_MarkBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress )
{
	unsigned long ulTagBuffer[TNFTL_BADMARK_DWORD_SIZE];
	unsigned long ulVerifyTagBuffer[TNFTL_BADMARK_DWORD_SIZE];
	unsigned char *pucVerifyTagBuffer = ( unsigned char * )ulVerifyTagBuffer;
	unsigned int uiMarkRetryCount = 100;
	unsigned int uiBlockPageAddr = PageAddressOfBlock( pTnftlObj, uiBlockAddress );

	memset( ulTagBuffer, 0x00, sizeof( ulTagBuffer ) );

	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, &sNAND_IO_DeviceInfo, &uiBlockPageAddr);

	while( uiMarkRetryCount-- )
	{
		NAND_IO_EraseBlock(pTnftlObj,uiBlockAddress);
		// try to mark at 0 page of block
		NAND_IO_WriteUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulTagBuffer ), ( unsigned char * )ulTagBuffer );
		NAND_IO_ReadUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulVerifyTagBuffer ), ( unsigned char * )ulVerifyTagBuffer );
		if( pucVerifyTagBuffer[0] != 0xFF )
			break;

		// try to mark at 1 page of block
		NAND_IO_WriteUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr + 1, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulTagBuffer ), ( unsigned char * )ulTagBuffer );
		NAND_IO_ReadUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr + 1, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulVerifyTagBuffer ), ( unsigned char * )ulVerifyTagBuffer );
		if( pucVerifyTagBuffer[0] != 0xFF )
			break;

		// try to mark at last page of block
		NAND_IO_WriteUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr + pTnftlObj->uiPagesPerBlock - 1, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulTagBuffer ), ( unsigned char * )ulTagBuffer );
		NAND_IO_ReadUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr + pTnftlObj->uiPagesPerBlock - 1, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulVerifyTagBuffer ), ( unsigned char * )ulVerifyTagBuffer );
		if( pucVerifyTagBuffer[0] != 0xFF )
			break;
	}

	NAND_IO_ASSERT( uiMarkRetryCount );
}

TNFTL_BAD_BLOCK_STATUS_T NAND_IO_IsBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress )
{
	unsigned long ulVerifyTagBuffer[TNFTL_BADMARK_DWORD_SIZE + 1];
	unsigned char *pucVerifyTagBuffer = ( unsigned char * )ulVerifyTagBuffer;
	unsigned int uiBlockPageAddr = PageAddressOfBlock( pTnftlObj, uiBlockAddress );

	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, &sNAND_IO_DeviceInfo, &uiBlockPageAddr);

#if defined(TCC800X)
	// Use Randomizer Buffer for temp read.
	if( NAND_IO_ReadPage_For_TCC800X( pTnftlObj, uiBlockPageAddr, sNAND_IO_uiRandomizeBuffer, (unsigned int*)ulVerifyTagBuffer ) == NAND_IO_SUCCESS )
		return TNFTL_V5_BLOCK_NORMAL;

#endif

	// check 0 page
	NAND_IO_ReadUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulVerifyTagBuffer ), ( unsigned char * )ulVerifyTagBuffer );
	if( pucVerifyTagBuffer[0] != 0xFF )
		return TNFTL_BLOCK_BAD;

	// check 1 page
	NAND_IO_ReadUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr + 1, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulVerifyTagBuffer ), ( unsigned char * )ulVerifyTagBuffer );
	if( pucVerifyTagBuffer[0] != 0xFF )
		return TNFTL_BLOCK_BAD;

	// check last page
	NAND_IO_ReadUserSizePage( &sNAND_IO_DeviceInfo, uiBlockPageAddr + pTnftlObj->uiPagesPerBlock - 1, ( unsigned short )pTnftlObj->uiBytesPerPage, sizeof( ulVerifyTagBuffer ), ( unsigned char * )ulVerifyTagBuffer );
	if( pucVerifyTagBuffer[0] != 0xFF )
		return TNFTL_BLOCK_BAD;

	return TNFTL_BLOCK_NORMAL;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_MakeBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer )
{
	NAND_IO_Update_CSNum_And_PageAddr(pTnftlObj, &sNAND_IO_DeviceInfo, NULL);
	if( NAND_IO_MakeBootBinary( &sNAND_IO_DeviceInfo, ( unsigned char * )puiBuffer ) != 0 )
		return FALSE;
	return TRUE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_EncodeCodeword( NAND_IO_DEVINFO *nDevInfo, unsigned int *puiDataBuffer, unsigned int uiDataSize, unsigned char *pucEccBuffer )
{
	unsigned int		i;
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	if( uiDataSize & 3 )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
		return NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP HIGH
	//=============================================
	//NAND_IO_PreProcess();
	//NAND_IO_BusControl(nDevInfo);
	//NAND_IO_SetCommCycleTime();

	//NAND_IO_EnableChipSelect( nDevInfo->ChipNo );
	//NAND_BD_Disable_WP_Port();

	//=========================================================================
	// Initial Setting
	//=========================================================================
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//####################################################
	//#	Dummy Write Page Data
	//####################################################
	/* Setup ECC Block */
#if defined(_WINCE_) || defined(_LINUX_)
	NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
	NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, nDevInfo->EccType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif

	s_pECC->ECC_CTRL	|= ( uiDataSize << ECC_SHIFT_DATASIZE );
	s_pECC->ECC_CLEAR	= 0x00000000;

	for( i = 0 ; i < ( uiDataSize >> 2 ) ; i++ )
	{
		s_pNFC->NFC_WDATA	 =  puiDataBuffer[i];
	}

	/*	Load ECC code from ECC block */
	res = NAND_IO_EncodeECC( nDevInfo->EccType, pucEccBuffer , nDevInfo->EccCodeSize );

	/* Disable ECC Block */
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	//=============================================
	// FORCE TO SET WP LOW
	// Disable Chip Select
	// PostProcess
	//=============================================
	//NAND_BD_Enable_WP_Port();
	//NAND_IO_DisableChipSelect();
	//NAND_IO_PostProcess();

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned int NAND_IO_MakeBL2Page( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer, unsigned int uiPageSize, const unsigned int *puiTag )
{
	NAND_IO_DEVINFO *nDevInfo = &sNAND_IO_DeviceInfo;
	NAND_IO_ERROR_T res = NAND_IO_SUCCESS;
	unsigned int i, uiCodewordCount;
	unsigned char *pucSharedEccBuffer = ( unsigned char * )sNAND_IO_uiSharedEccBuffer;

	memset( sNAND_IO_uiSharedEccBuffer, 0xFF, sizeof( sNAND_IO_uiSharedEccBuffer ) );

	NAND_IO_Update_CSNum_And_PageAddr( pTnftlObj, nDevInfo, NULL );

	uiCodewordCount = uiPageSize / nDevInfo->CodewordSize;
	for( i = 0 ; i < uiCodewordCount ; i++ )
	{
		//=============================================
		// ECC Encoding
		//=============================================
		NAND_IO_EncodeCodeword( nDevInfo, puiBuffer, nDevInfo->CodewordSize, pucSharedEccBuffer );
		puiBuffer = &puiBuffer[nDevInfo->CodewordSize >> 2];
		pucSharedEccBuffer = &pucSharedEccBuffer[nDevInfo->EccCodeSize];
	}
	// factory bad marker
	*puiBuffer++ = 0xFFFFFFFF;
	// ECC CODE for Data
	{
		unsigned int uiEccCodeSizeTotalInDword = ( ( uiCodewordCount * nDevInfo->EccCodeSize ) + 3 ) >> 2;
		NAND_Util_memcpy( puiBuffer, sNAND_IO_uiSharedEccBuffer, uiEccCodeSizeTotalInDword << 2 );
		puiBuffer = &puiBuffer[uiEccCodeSizeTotalInDword];
	}
	NAND_Util_memcpy( puiBuffer, puiTag, TNFTL_TAG_BYTE_SIZE );
	NAND_IO_EncodeCodeword( nDevInfo, puiBuffer, TNFTL_TAG_BYTE_SIZE, ( unsigned char * )&puiBuffer[TNFTL_TAG_DWORD_SIZE] );

	if( res != NAND_IO_SUCCESS )
		return FALSE;

	return TRUE;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_PowerUp( void )
{
	ND_TRACE( "+PowerUp\n" );
	NAND_IO_Init();
	NAND_IO_SuppressMessage( TRUE );
	ND_TRACE( "-PowerUp\n" );
	return;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_PowerDown( void )
{
	ND_TRACE( "+PowerDown\n" );
	NAND_IO_WaitBusyCheckForWriteEnd( s_pDevInfo );
	ND_TRACE( "-PowerDown\n" );
	return;
}

#if defined(CTRL_FUNC)
//*************************************************************************
//
// Control Functions
//
//*************************************************************************
/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_CKC_Enable( void )
{
	#if defined(NFC_VER_50)
	NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_NFC );
	#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_CKC_Disable( void )
{
	#if defined(NFC_VER_50)
	NAND_IO_CKC_DisableBUS( NAND_IO_CKC_BUS_NFC );
	#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_PortControl( int nOnOff )
{
	/*
		if ( !( s_usInterLeaveIoStatus & NAND_IO_STATUS_INTERLEAVING_MASK ) )
			if ( nOnOff == ENABLE )
			else
	*/
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_PreProcess( void )
{
	NAND_IO_CKC_Enable();
	NAND_IO_PortControl( ENABLE );
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_PostProcess( void )
{
	NAND_IO_PortControl( DISABLE );
	NAND_IO_CKC_Disable();
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_BusControl( NAND_IO_DEVINFO *nDevInfo )
{
	unsigned int	nMaxBusClk;
	unsigned int	nMaxBusClkMHZ;

	//==============================================
	// SET NAND I/O CYCLE
	//==============================================
#if 1

#if defined(TCC89XX) || defined(TCC92XX) || defined(TCC93XX) || defined(TCC88XX)
	#if defined(_WINCE_)
		#if defined(NAND_FOR_KERNEL)
			nMaxBusClk = tcc_ckc_getfbusctrl( CLKCTRL4 );
		#else
			nMaxBusClk = tca_ckc_getfbusctrl( CLKCTRL4 );
		#endif
	#elif defined(_LINUX_)
		nMaxBusClk = tca_ckc_getfbusctrl( CLKCTRL4 );
	#else
		nMaxBusClk = 1660000;
	#endif
#elif defined(TCC892X) || defined(TCC8925S) || defined(TCC893X)
	#if defined(_WINCE_)
		#if defined(NAND_FOR_KERNEL)
			nMaxBusClk = tcc_ckc_getfbusctrl( CLKIOBUS );
		#else
			nMaxBusClk = tca_ckc_getfbusctrl( CLKIOBUS );
		#endif
	#elif defined(_LINUX_)
		nMaxBusClk = tca_ckc_getfbusctrl( FBUS_IO );
	#else
		nMaxBusClk = 1660000;
	#endif
#elif defined(TCC801X) || defined(TCC370X) || defined(TCC800X)
	#ifdef __NUCLEUS_KERNEL__
	nMaxBusClk	= Mcu_GetCurrentBUSClock4Cycle();
	#else
	nMaxBusClk = 1660000;	// frequency of 100Hz unit
	#endif
#endif

#else
		nMaxBusClk = 1660000;
#endif

	if( nMaxBusClk  == 0 )
		nMaxBusClk = 1660000;

	nMaxBusClkMHZ = ( ( nMaxBusClk + 9999 ) / 10000 );

	if( nMaxBusClkMHZ != sNAND_IO_uiMaxBusClkMHz )
	{
		NAND_IO_SetCycle( nDevInfo, nMaxBusClkMHZ );
	}
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetDataWidth( unsigned long width )
{
	if( width == NAND_IO_DATA_WITDH_8BIT )
	{
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BUS_WIDTH_FIELD, NFC_CTRL_BUS_WIDTH_8BIT );
	}
	else if( width == NAND_IO_DATA_WITDH_16BIT )
	{
		BITCSET( s_pNFC->NFC_CTRL, NFC_CTRL_BUS_WIDTH_FIELD, NFC_CTRL_BUS_WIDTH_16BIT );
	}
	else
	{
		NAND_IO_ASSERT( 0 );
	}

	if( sNAND_IO_uiPortStatus & NAND_PORT_STATUS_DATA_WIDTH_16BIT )
		BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );
	else
		BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_HIGH_BYTES_MASK );
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetBasicCycleTime( void )
{
	/* SETUP 1 PW 5 HOLD 1 */
#if defined(NFC_VER_50) || defined(NFC_VER_100)
	BITCSET( s_pNFC->NFC_CTRL, 0xFFF, 0xEEE );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	BITCSET( s_pNFC->NFC_CACYC, 0xF0F0F, 0xE0E0E );
	BITCSET( s_pNFC->NFC_RECYC, 0xF0F0F, 0xE0E0E );
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetWriteCycleTime( void )
{
#if defined(NFC_VER_50) || defined(NFC_VER_100)
	BITCSET( s_pNFC->NFC_CTRL, 0xFFF, s_WriteCycleTime.RegValue );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	BITCSET( s_pNFC->NFC_WRCYC, 0xF0F0F, s_WriteCycleTime.RegValue );
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetReadCycleTime( void )
{
#if defined(NFC_VER_50) || defined(NFC_VER_100)
	BITCSET( s_pNFC->NFC_CTRL, 0xFFF, s_ReadCycleTime.RegValue );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	BITCSET( s_pNFC->NFC_RECYC, 0xF0F0F, s_ReadCycleTime.RegValue );
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetCommCycleTime( void )
{
#if defined(NFC_VER_50) || defined(NFC_VER_100)
	BITCSET( s_pNFC->NFC_CTRL, 0xFFF, s_CommCycleTime.RegValue );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	BITCSET( s_pNFC->NFC_CACYC, 0xF0F0F, s_CommCycleTime.RegValue );
#endif
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_EnableChipSelect( unsigned short nChipNo )
{
	if( nChipNo == 0 )
	{
		BITSCLR( s_pNFC->NFC_CTRL, NFC_CTRL_CS_FIELD, NFC_CTRL_CS0 );
	}
	else if( nChipNo == 1 )
	{
		BITSCLR( s_pNFC->NFC_CTRL, NFC_CTRL_CS_FIELD, NFC_CTRL_CS1 );
	}
	else if( nChipNo == 2 )
	{
		BITSCLR( s_pNFC->NFC_CTRL, NFC_CTRL_CS_FIELD, NFC_CTRL_CS2 );
	}
	else if( nChipNo == 3 )
	{
		BITSCLR( s_pNFC->NFC_CTRL, NFC_CTRL_CS_FIELD, NFC_CTRL_CS3 );
	}
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_DisableChipSelect( void )
{
	BITSET( s_pNFC->NFC_CTRL, NFC_CTRL_CS_FIELD );
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_WriteBlockPageAddr( unsigned long nBlockPageAddr, NAND_IO_DEVINFO *nDevInfo )
{
	unsigned char		i;

	//==================================================
	// Write Block Address
	//==================================================
	for( i = 0; i < nDevInfo->Feature.ucRowCycle; ++i )
	{
		s_pNFC->NFC_SADDR	= nDevInfo->CmdMask & ( ( ( nBlockPageAddr << 8 ) & 0xFF00 ) | ( nBlockPageAddr & 0x00FF ) );
		nBlockPageAddr = nBlockPageAddr >> 8;
	}
}
#endif

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetupECC( unsigned short nEccOnOff, unsigned short nEncDec, unsigned short nEccType, unsigned short nAccessType, unsigned long EccBaseAddr )
{
	if( nEccOnOff == ECC_OFF )
	{
#if defined(NFC_VER_50) || defined(NFC_VER_100)		
		NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_ECC );
		BITSET(NAND_BUS_SOFTWARE_RESET, NAND_BUS_CLOCK_CTRL_ECC);
		BITCLR(NAND_BUS_SOFTWARE_RESET, NAND_BUS_CLOCK_CTRL_ECC);
		s_pECC->ECC_MASK	= 0x00000000;
		s_pECC->ECC_CTRL = 0x00000000;		
		NAND_IO_CKC_DisableBUS( NAND_IO_CKC_BUS_ECC );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		s_pNFC->NFC_CTRL		&= ~NFC_CTRL_ECC_ENABLE;
		s_pECC->ECC_CTRL 		&= ~ECC_CTRL_ECC_EN_FIELD;
		BITCLR( s_pECC->ECC_CTRL, ECC_CTRL_MLC_ECC_CRRECTION_PASS_FIELD );
#endif
	}
	else if( nEccOnOff == ECC_ON )
	{
	
#if defined(NFC_VER_50)	
		NAND_IO_CKC_EnableBUS( NAND_IO_CKC_BUS_ECC );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
		s_pNFC->NFC_CTRL		|= NFC_CTRL_ECC_ENABLE;					// NFC ECC Encode/Decode Enable
		BITCSET( s_pECC->ECC_CTRL, ECC_CTRL_ECC_DATASIZE_FIELD|ECC_CTRL_MLC_ECC_CRRECTION_PASS_FIELD|ECC_CTRL_MODE_FIELD|ECC_CTRL_ECC_EN_FIELD, 0x00000000 );
#endif

		if( nEncDec == ECC_DECODE )
		{
			//==========================================================
			//
			// ECC Decode Setup
			//
			//==========================================================
#if defined(NFC_VER_50) || defined(NFC_VER_100)
			if( nAccessType == NAND_MCU_ACCESS )
			{
				NAND_IO_ECC_SEL = HwECC_SEL_AHB_BUS;
				//pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				//s_pECC->ECC_BASE	= ( 0x000FFFFF & EccBaseAddr );
				s_pECC->ECC_BASE	= EccBaseAddr;
				s_pECC->ECC_MASK	= 0x00000000;
			}
			else if( nAccessType == NAND_DMA_ACCESS )
			{
				NAND_IO_ECC_SEL = HwECC_SEL_AHB_BUS;
				//pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				s_pECC->ECC_BASE	= EccBaseAddr;
				s_pECC->ECC_MASK	= 0x00000000;
			}
#endif

			/* ECC Enable Configuration */
			//if ( nEccType == ECC_TYPE_1BIT )
			//{
			//	#if defined(NFC_VER_100)
			//	s_pECC->ECC_CTRL	|= 	HwECC_CTRL_EN_SLCDE;
			//	#elif defined(NFC_VER_200)
			//	s_pECC->ECC_CTRL |= HwECC_CTRL_EN_SLCDE;
			//	#endif
			//}
			if( nEccType == ECC_TYPE_4BIT )
			{
				#if defined(NFC_VER_50)
				s_pECC->ECC_CTRL	|= ECC_CTRL_ECC4_DECODE_INTR_ENABLE;
				#endif
				s_pECC->ECC_CTRL	|= ECC_CTRL_ECC_EN_MLC_ECC4_DECODE;
			}
#if defined(NFC_VER_50) || defined(NFC_VER_100) || defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_8BIT )
			{
				#if defined(NFC_VER_50)
				s_pECC->ECC_CTRL	|= ECC_CTRL_ECC8_DECODE_INTR_ENABLE;
				#endif
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC8_DECODE;
			}
#endif
			else if( nEccType == ECC_TYPE_12BIT )
			{
#if defined(NFC_VER_50) ||  defined(NFC_VER_100)
				s_pECC->ECC_CTRL	|= ECC_CTRL_ECC12_DECODE_INTR_ENABLE;
				s_pECC->ECC_CTRL	|= ECC_CTRL_ECC_EN_MLC_ECC12_DECODE;
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
				s_pECC->ECC_CTRL		|= 	ECC_CTRL_ECC_EN_MLC_ECC12_DECODE;
#endif
			}
#if defined(NFC_VER_100) || defined(NFC_VER_200) || defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_16BIT )
			{
				s_pECC->ECC_CTRL		|= 	ECC_CTRL_ECC_EN_MLC_ECC16_DECODE;

			}
#endif
#if defined(NFC_VER_200) || defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_24BIT )
			{
				s_pECC->ECC_CTRL		|= 	ECC_CTRL_ECC_EN_MLC_ECC24_DECODE;
			}
#endif
#if defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_40BIT )
			{
				s_pECC->ECC_CTRL		|= 	ECC_CTRL_ECC_EN_MLC_ECC40_DECODE;
			}
			else if( nEccType == ECC_TYPE_60BIT )
			{
				s_pECC->ECC_CTRL		|= 	ECC_CTRL_ECC_EN_MLC_ECC60_DECODE;
			}
#endif
			else
			{
				NAND_IO_ASSERT( 0 );
			}
		}
		else if( nEncDec == ECC_ENCODE )
		{

			//==========================================================
			//
			// ECC Encode Setup
			//
			//==========================================================
#if defined(NFC_VER_50) || defined(NFC_VER_100)
			if( nAccessType == NAND_MCU_ACCESS )
			{
				//pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				//s_pECC->ECC_BASE	= ( 0x000FFFFF & EccBaseAddr );
				NAND_IO_ECC_SEL = HwECC_SEL_AHB_BUS;
				s_pECC->ECC_BASE	= EccBaseAddr;
			}
			else if( nAccessType == NAND_DMA_ACCESS )
			{
				//pIOBUSCFG_T->STORAGE = HwIOBUSCFG_STORAGE_NFC;
				NAND_IO_ECC_SEL = HwECC_SEL_AHB_BUS;
				s_pECC->ECC_BASE	= EccBaseAddr;
			}
#endif

			/* Address mask for ECC area */
			//#if defined(TCC89XX) || defined(TCC92XX)
			s_pECC->ECC_MASK	= 0x00000000;
			//#endif

			//if ( nEccType == ECC_TYPE_1BIT )
			//	s_pECC->ECC_CTRL	|= 	HwECC_CTRL_EN_SLCEN;
			if( nEccType == ECC_TYPE_4BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC4_ENCODE;
#if defined(NFC_VER_50) || defined(NFC_VER_100) || defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_8BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC8_ENCODE;
#endif
			else if( nEccType == ECC_TYPE_12BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC12_ENCODE;
#if defined(NFC_VER_200) || defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_16BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC16_ENCODE;
			else if( nEccType == ECC_TYPE_24BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC24_ENCODE;
#endif
#if defined(NFC_VER_300)
			else if( nEccType == ECC_TYPE_40BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC40_ENCODE;
			else if( nEccType == ECC_TYPE_60BIT )
				s_pECC->ECC_CTRL	|= 	ECC_CTRL_ECC_EN_MLC_ECC60_ENCODE;
#endif
			else
				NAND_IO_ASSERT( 0 );
		}

		
		{
			#if defined(NFC_VER_50)
			const NAND_IO_ECC_INFO	*pECC_Info;

			pECC_Info = NAND_IO_GetECCInfo( nEccType );
			s_pECC->ECC_IREQ = pECC_Info->DecodeFlag;
			#else		
			s_pNFC->NFC_IRQ = NFC_IRQ_MLC_ECC_DECODE_FLAG;		
			#endif
		}
	}	
	s_pECC->ECC_CLEAR = 0x00000000;
	
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_EncodeECC( unsigned short nEccType, void *pvEccDataBuffer, unsigned int uiEccDataSize )
{
	unsigned long		ulECC_DATA[27];

	ulECC_DATA[0] = s_pECC->ECC_CODE0;
	ulECC_DATA[1] = s_pECC->ECC_CODE1;
	ulECC_DATA[2] = s_pECC->ECC_CODE2;
	ulECC_DATA[3] = s_pECC->ECC_CODE3;
	ulECC_DATA[4] = s_pECC->ECC_CODE4;
#if defined(NFC_VER_100) || defined(NFC_VER_200) || defined(NFC_VER_300)	
	ulECC_DATA[5] = s_pECC->ECC_CODE5;
	ulECC_DATA[6] = s_pECC->ECC_CODE6;
#endif
#if defined(NFC_VER_200) || defined(NFC_VER_300)
	ulECC_DATA[7] = s_pECC->ECC_CODE7;
	ulECC_DATA[8] = s_pECC->ECC_CODE8;
	ulECC_DATA[9] = s_pECC->ECC_CODE9;
	ulECC_DATA[10] = s_pECC->ECC_CODE10;
#endif
#if defined(NFC_VER_300)
	ulECC_DATA[11] = s_pECC->ECC_CODE11;
	ulECC_DATA[12] = s_pECC->ECC_CODE12;
	ulECC_DATA[13] = s_pECC->ECC_CODE13;
	ulECC_DATA[14] = s_pECC->ECC_CODE14;
	ulECC_DATA[15] = s_pECC->ECC_CODE15;
	ulECC_DATA[16] = s_pECC->ECC_CODE16;
	ulECC_DATA[17] = s_pECC->ECC_CODE17;
	ulECC_DATA[18] = s_pECC->ECC_CODE18;
	ulECC_DATA[19] = s_pECC->ECC_CODE19;
	ulECC_DATA[20] = s_pECC->ECC_CODE20;
	ulECC_DATA[21] = s_pECC->ECC_CODE21;
	ulECC_DATA[22] = s_pECC->ECC_CODE22;
	ulECC_DATA[23] = s_pECC->ECC_CODE23;
	ulECC_DATA[24] = s_pECC->ECC_CODE24;
	ulECC_DATA[25] = s_pECC->ECC_CODE25;
	ulECC_DATA[26] = s_pECC->ECC_CODE26;
#endif

	NAND_Util_memcpy( pvEccDataBuffer, ulECC_DATA, uiEccDataSize );

	return NAND_IO_SUCCESS;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_EncodeBootBinary( NAND_IO_DEVINFO *nDevInfo, unsigned char *nPageBuffer, int nEccOnOff )
{
	unsigned short		usECCType;
	unsigned int		i, uiEncodeECCSize;
#ifdef _LINUX_
	unsigned char		nECCBuffer[108]__attribute__( ( aligned( 8 ) ) );
#else
	unsigned char		nECCBuffer[108];
#endif
	unsigned char		*pPageB;
	unsigned char		*pDataBuffer;
	unsigned char		*pEccB;
	DWORD_BYTE			uDWordByte;
	NAND_IO_ERROR_T		res;

	//=========================================================================
	// Initial Setting
	//=========================================================================
	//if ( nEccOnOff == ECC_OFF )
	NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	res = NAND_IO_SUCCESS;

	//=========================================================================
	// DATA BUS WIDTH Setting
	//=========================================================================
	NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

	//=========================================================================
	// Get Buffer Pointer
	//=========================================================================
	/* Adapt type of address */
	pPageB		= ( unsigned char * )nPageBuffer;

	/* Get Buffer pointer */
	pDataBuffer = ( unsigned char * )pPageB;

	/* Setup ECC Block */
	if( nEccOnOff == ECC_ON )
	{
#if defined(NFC_VER_50)
		uiEncodeECCSize = 20;
		usECCType		= ECC_TYPE_12BIT;
#elif defined(NFC_VER_100)
		uiEncodeECCSize = 28;
		usECCType		= ECC_TYPE_16BIT;
#elif defined(NFC_VER_200)
		uiEncodeECCSize = 44;
		usECCType		= ECC_TYPE_24BIT;
#elif defined(NFC_VER_300)
		uiEncodeECCSize = 108;
		usECCType		= ECC_TYPE_60BIT;
#endif

#if defined(_WINCE_) || defined(_LINUX_)
		NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, usECCType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
		NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, usECCType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif

		s_pECC->ECC_CTRL	|= ( NAND_SB_BOOT_PAGE_SIZE << ECC_SHIFT_DATASIZE );
		s_pECC->ECC_CLEAR	= 0x00000000;
	}

	i = ( NAND_SB_BOOT_PAGE_SIZE >> 2 );
	do
	{
		uDWordByte.BYTE[0] = *pDataBuffer; ++pDataBuffer;
		uDWordByte.BYTE[1] = *pDataBuffer; ++pDataBuffer;
		uDWordByte.BYTE[2] = *pDataBuffer; ++pDataBuffer;
		uDWordByte.BYTE[3] = *pDataBuffer; ++pDataBuffer;
		s_pNFC->NFC_WDATA	 =  uDWordByte.DWORD;
	} while( --i );

	/* Adapt type of address */
	pEccB = ( unsigned char * )nECCBuffer;

	/*	Load ECC code from ECC block */
	if( nEccOnOff == ECC_ON )
	{
		res = NAND_IO_EncodeECC( usECCType, pEccB , uiEncodeECCSize );

		for( i = 0; i < uiEncodeECCSize; ++i )
		{
			*pDataBuffer = nECCBuffer[i];
			pDataBuffer += 1;
		}
	}

	/* Disable ECC Block */
	if( nEccOnOff == ECC_ON )
		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_EccErrorNum( unsigned char nErrorData, unsigned char nCorrectData, unsigned char *rErrorBitNum )
{
	unsigned int		i;
	unsigned int		nErrorNum;
	unsigned int		nTestBit;

	nErrorNum = 0;

	for( i = 0; i < 8; ++i )
	{
		//nTestBit = NAND_IO_ShiftFactorForMultiplay[i];
		nTestBit = ( 0x1 << i );

		if( ( nErrorData & nTestBit ) != ( nCorrectData & nTestBit ) )
			++nErrorNum;
	}

	*rErrorBitNum = ( unsigned char )nErrorNum;

	return NAND_IO_SUCCESS;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_GetLastError( TNFTL_OBJECT_T *pTnftlObj )
{
	return sNAND_IO_LastError;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
void NAND_IO_SetLastError( NAND_IO_ERROR_T Errcode )
{
	sNAND_IO_LastError = Errcode;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
unsigned long NAND_IO_GetBUSTypeOfDataIO( void )
{
	return sNAND_IO_DataBusType;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_MakeBootBinary( NAND_IO_DEVINFO *nDevInfo, unsigned char *nPageBuffer )
{
	NAND_IO_ERROR_T		res;

	//=============================================
	// PreProcess
	// Set Setup Time and Hold Time
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();

	//=============================================
	// ECC Encording
	//=============================================
	res = NAND_IO_EncodeBootBinary( nDevInfo, nPageBuffer, ECC_ON );

	//=============================================
	// PostProcess
	//=============================================
	NAND_IO_PostProcess();

	if( res != NAND_IO_SUCCESS )
		return res;

	return NAND_IO_SUCCESS;
}

unsigned long NAND_IO_GetBootStatus( void )
{
	unsigned int uiBootStatus;
#if defined(NFC_VER_50)
	PECC_V05_T pEcc = ( PECC_V05_T )( ECC_BASE_ADDRESS );
#elif defined(NFC_VER_200)
	PECC_V2_T pEcc = ( PECC_V2_T )( ECC_BASE_ADDRESS );
#elif defined(NFC_VER_300)
	PECC_V3_T pEcc = ( PECC_V3_T )( ECC_BASE_ADDRESS );
#endif
	uiBootStatus = pEcc->ECC_BASE;
	uiBootStatus &= ( 0xFFFFFFFC );		// LSB[0:1] of ECC_Base register are not valid (for alignment)

	return uiBootStatus;
}

#if defined(MISC_FUNC)
//*************************************************************************
//
// Miscellaneous Functions
//
//*************************************************************************
/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
NAND_IO_ERROR_T NAND_IO_GetUID( NAND_IO_DEVINFO *nDevInfo, unsigned short *nCmd, unsigned char *rReadData )
{
	unsigned int		i;
	unsigned int		RowAddr, ColumnAddr;
	unsigned char		cTempBuffer[512];
	NAND_IO_ERROR_T		res = NAND_IO_SUCCESS;

	//=============================================
	// Check Device and Parameter
	//=============================================
	if( !( nDevInfo->IoStatus & NAND_IO_STATUS_ENABLE ) )
		return NAND_IO_ERROR_DEVICE_IO_IS_NOT_READY;

	//=============================================
	// PreProcess
	// Set Setup and Hold Time
	// Enable Chip Select
	// FORCE TO SET WP LOW
	//=============================================
	NAND_IO_PreProcess();
	NAND_IO_BusControl( nDevInfo );
	NAND_IO_SetCommCycleTime();
	NAND_IO_EnableChipSelect( nDevInfo->ChipNo );
	NAND_BD_Enable_WP_Port();

	if( sNAND_IO_DeviceInfo.fInterleaveUsable )
		res = NAND_IO_WaitBusyForInterleave( &sNAND_IO_DeviceInfo );

	s_pNFC->NFC_IRQ = NFC_IRQ_READY_FLAG;

	/* Command #1 Command #2 ( Micron Unque ID not exitst Command#2 ) */
	s_pNFC->NFC_CMD = nDevInfo->CmdMask & *nCmd;
	if( nDevInfo->Feature.DeviceID.Code[0] != MICRON_NAND_MAKER_ID )
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & *( nCmd + 1 );

	if( nDevInfo->Feature.DeviceID.Code[0] == MICRON_NAND_MAKER_ID )
	{
		s_pNFC->NFC_SADDR	= 0x0000;	/* Address [ 0x00 ] */
	}
	else
	{
		/* Generate Row and Column Address */
		NAND_IO_GenerateRowColAddr( 0, 0, &RowAddr, &ColumnAddr, nDevInfo );

		/* Command READ [ 0x00 ] */
		s_pNFC->NFC_CMD = 0x0000;

		/* Write Row and Column Address	*/
		NAND_IO_WriteRowColAddr( RowAddr, ColumnAddr, nDevInfo );

		/* Command READ2 [ 0x30 ] for Advance NandFlash */
		s_pNFC->NFC_CMD = nDevInfo->CmdMask & 0x3030;
	}

	/* Wait until it is ready */
	NAND_IO_WaitBusy( nDevInfo->ChipNo );

	/* Read Data */
	res = NAND_IO_ReadUserSizeData( nDevInfo,
	                                0,
	                                512,
	                                cTempBuffer );
	if( res != NAND_IO_SUCCESS )
		goto ErrorGetUID;

	/* Copy Read Data */
	if( nDevInfo->Feature.MediaType & A_PARALLEL )
	{
		for( i = 0; i < 256; ++ i )
			rReadData[i] = cTempBuffer[i * 2 + 0];
	}
	else
	{
		for( i = 0; i < 256; ++ i )
			rReadData[i] = cTempBuffer[i];
	}

ErrorGetUID:
	//=============================================
	// FORCE TO SET WP LOW
	// Disable Chip Select
	// PostProcess
	//=============================================
	NAND_IO_DisableChipSelect();
	NAND_IO_PostProcess();

	//=============================================
	// Reset Chip
	//=============================================
	if( nDevInfo->Feature.MediaType & A_16BIT )
		NAND_IO_Reset( nDevInfo->ChipNo, NAND_IO_PARALLEL_COMBINATION_MODE );
	else
		NAND_IO_Reset( nDevInfo->ChipNo, NAND_IO_SERIAL_COMBINATION_MODE );

	if( res != NAND_IO_SUCCESS )
		return res;

	return NAND_IO_SUCCESS;

}
#endif

#if defined(TEST_FUNC)
//*************************************************************************
//
// Test Functions
//
//*************************************************************************


/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
static NAND_IO_ERROR_T NAND_IO_CorrectionMLCForTest( unsigned short usEccType, unsigned char *nPageBuffer, unsigned char *pucEccData, unsigned short nDataSize )
{
	unsigned int			i;
	unsigned int			uErrAddr;
	unsigned int			uErrorStatus;
	unsigned int			nCumulativeBitNum;
	unsigned char			rErrorBitNum;
	const NAND_IO_ECC_INFO	*pECC_Info;
	NAND_IO_ERROR_T			res;
	unsigned long			ulEccCode[27];

	pECC_Info = NAND_IO_GetECCInfo( usEccType );
	if( pECC_Info == NULL )
		return NAND_IO_ERROR_WRONG_PARAMETER;

	NAND_Util_memcpy( ulEccCode, pucEccData, (unsigned int)pECC_Info->ucEccCodeSize );

	s_pECC->ECC_CODE0 = ulEccCode[0];
	s_pECC->ECC_CODE1 = ulEccCode[1];
	s_pECC->ECC_CODE2 = ulEccCode[2];
	s_pECC->ECC_CODE3 = ulEccCode[3];
	s_pECC->ECC_CODE4 = ulEccCode[4];
#if defined(NFC_VER_100) || defined(NFC_VER_200) || defined(NFC_VER_300)
	s_pECC->ECC_CODE5 = ulEccCode[5];
	s_pECC->ECC_CODE6 = ulEccCode[6];
#endif
#if defined(NFC_VER_200) || defined(NFC_VER_300)
	s_pECC->ECC_CODE7 = ulEccCode[7];
	s_pECC->ECC_CODE8 = ulEccCode[8];
	s_pECC->ECC_CODE9 = ulEccCode[9];
	s_pECC->ECC_CODE10 = ulEccCode[10];
#endif
#if defined(NFC_VER_300)
	s_pECC->ECC_CODE11 = ulEccCode[11];
	s_pECC->ECC_CODE12 = ulEccCode[12];
	s_pECC->ECC_CODE13 = ulEccCode[13];
	s_pECC->ECC_CODE14 = ulEccCode[14];
	s_pECC->ECC_CODE15 = ulEccCode[15];
	s_pECC->ECC_CODE16 = ulEccCode[16];
	s_pECC->ECC_CODE17 = ulEccCode[17];
	s_pECC->ECC_CODE18 = ulEccCode[18];
	s_pECC->ECC_CODE19 = ulEccCode[19];
	s_pECC->ECC_CODE20 = ulEccCode[20];
	s_pECC->ECC_CODE21 = ulEccCode[21];
	s_pECC->ECC_CODE22 = ulEccCode[22];
	s_pECC->ECC_CODE23 = ulEccCode[23];
	s_pECC->ECC_CODE24 = ulEccCode[24];
	s_pECC->ECC_CODE25 = ulEccCode[25];
	s_pECC->ECC_CODE26 = ulEccCode[26];
#endif

	/* Sync Delay */
	//ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;

	/* Wait MLC ECC Correction */
#if defined(NFC_VER_50) || defined(NFC_VER_100)
	while( !( s_pECC->ECC_IREQ & pECC_Info->DecodeFlag ) );
#elif defined(NFC_VER_200) || defined(NFC_VER_300)
	while( !( s_pNFC->NFC_IRQ & NFC_IRQ_MLC_ECC_DECODE_FLAG ) );
	BITCLR( s_pNFC->NFC_CTRL, NFC_CTRL_ECC_ENABLE );
#endif

	res = ( NAND_IO_ERROR_T )NAND_IO_ERROR_ECC_CORRECTION_FAILURE;

	/* Correction */
#if defined(NFC_VER_50) 
	uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x0F;
#elif defined(NFC_VER_100) || defined(NFC_VER_200)		
	uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x1F;
#elif defined(NFC_VER_300)
	uErrorStatus = s_pECC->ECC_ERRSTATUS & 0x3F;
#endif

	if( uErrorStatus > (unsigned int)pECC_Info->ucErrorNum )
	{
		if( ( nDataSize == 512 ) || ( nDataSize == 1024 ) )
		{
			nCumulativeBitNum = 0;
			for( i = 0 ; i < (unsigned int)pECC_Info->ucEccCodeSize ; ++i )
			{
				NAND_IO_EccErrorNum( pucEccData[i], 0xFF, &rErrorBitNum );
				nCumulativeBitNum += rErrorBitNum;
			}

			if( nCumulativeBitNum > 4 )
			{
#if defined(NAND_IO_ECC_ERROR_LOG)
				ND_TRACE( "ErrorNum[%02d],DataSize[%03d] - Correction Fail\n", uErrorStatus, nDataSize );
				ND_TRACE( "Data:\n" );
				for( i = 0; i < nDataSize; ++i )
				{
					_ND_TRACE( "%02X", nPageBuffer[i] );
				}

				ND_TRACE( "\nECC:\n" );
				for( i = 0 ; i < (unsigned int)pECC_Info->ucEccCodeSize ; ++i )
				{
					_ND_TRACE( "%02X", pucEccData[i] );
				}
				_ND_TRACE( "\n" );
#endif
			}
			else
			{
#if 0
				ND_TRACE( "Clear Error Bit Num:%d", nCumulativeBitNum );

				ND_TRACE( "\nECC:\n" );
				for( i = 0 ; i < (unsigned int)pECC_Info->ucEccCodeSize ; ++i )
				{
					_ND_TRACE( "%x", nSpareBuffer[i] );
				}
				_ND_TRACE( "\n" );
#endif
				res = NAND_IO_SUCCESS;
			}
		}

		goto ErrorCorrectionMLC;
	}
	else if( uErrorStatus == 0 )
	{
		res = NAND_IO_SUCCESS;
	}
	else
	{
		for( i = 0 ; i < uErrorStatus ; ++i )
		{
			uErrAddr = s_pECC->ECC_EADDR[i];

			if( ( uErrAddr >> 3 ) < nDataSize )
				nPageBuffer[uErrAddr >> 3] ^= ( 1 << ( uErrAddr & 0x7 ) );
		}
		res = NAND_IO_SUCCESS;
	}

ErrorCorrectionMLC:
	/* Disable MLC ECC */
	//IO_CKC_DisableBUS( IO_CKC_BUS_ECC );

	return res;
}

/**************************************************************************
*  DESCRIPTION :
*
*  INPUT :
*
*  OUTPUT :
*
*  REMARK :
*
**************************************************************************/
int NAND_IO_ECC_Test( unsigned char *pucOriginalCodeWord, unsigned char *pucDamagedCodeWord, unsigned int uiCodeWordSize, unsigned int uiEccBit )
{
	unsigned int i;
	unsigned long ulEccData[44 / 4];
	unsigned int uiECCType = 0;
	DWORD_BYTE uDWordByte;
	int res = TRUE;

	switch( uiEccBit )
	{
		case 24:
			uiECCType = ECC_TYPE_24BIT;
			break;
		default:
			return FALSE;
	}

//
// ECC encoding
//
	{
		//=========================================================================
		// Initial Setting
		//=========================================================================
		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

		//=========================================================================
		// DATA BUS WIDTH Setting
		//=========================================================================
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

#if defined(_WINCE_) || defined(_LINUX_)
		NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, ( unsigned short )uiECCType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
		NAND_IO_SetupECC( ECC_ON, ECC_ENCODE, ( unsigned short )uiECCType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif

		s_pECC->ECC_CTRL	|= ( uiCodeWordSize << ECC_SHIFT_DATASIZE );
		s_pECC->ECC_CLEAR	= 0x00000000;

		for( i = 0 ; i < uiCodeWordSize ; i += 4 )
		{
			uDWordByte.BYTE[0] = pucOriginalCodeWord[i];
			uDWordByte.BYTE[1] = pucOriginalCodeWord[i + 1];
			uDWordByte.BYTE[2] = pucOriginalCodeWord[i + 2];
			uDWordByte.BYTE[3] = pucOriginalCodeWord[i + 3];
			s_pNFC->NFC_WDATA	 =  uDWordByte.DWORD;
		}

		NAND_IO_EncodeECC( ( unsigned short )uiECCType, ulEccData, sizeof( ulEccData ) );

		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );
	}

//
// ECC decoding
//
	{
		//=========================================================================
		// Initial Setting
		//=========================================================================
		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );

		//=========================================================================
		// DATA BUS WIDTH Setting
		//=========================================================================
		NAND_IO_SetDataWidth( NAND_IO_DATA_WITDH_8BIT );

#if defined(_WINCE_) || defined(_LINUX_)
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, ( unsigned short )uiECCType, NAND_MCU_ACCESS, ( unsigned long )&NAND_IO_HwDATA_PA );
#else
		NAND_IO_SetupECC( ECC_ON, ECC_DECODE, ( unsigned short )uiECCType, NAND_MCU_ACCESS, ( unsigned long )&s_pNFC->NFC_WDATA );
#endif

		s_pECC->ECC_CTRL	|= ( uiCodeWordSize << ECC_SHIFT_DATASIZE );
		s_pECC->ECC_CLEAR	= 0x00000000;

		for( i = 0 ; i < uiCodeWordSize ; i += 4 )
		{
			uDWordByte.BYTE[0] = pucDamagedCodeWord[i];
			uDWordByte.BYTE[1] = pucDamagedCodeWord[i + 1];
			uDWordByte.BYTE[2] = pucDamagedCodeWord[i + 2];
			uDWordByte.BYTE[3] = pucDamagedCodeWord[i + 3];
			s_pNFC->NFC_WDATA = uDWordByte.DWORD;
		}

		if( NAND_IO_CorrectionMLCForTest( ( unsigned short )uiECCType, pucDamagedCodeWord, ( unsigned char * )ulEccData, ( unsigned short )uiCodeWordSize ) != NAND_IO_SUCCESS )
			res = FALSE;

		NAND_IO_SetupECC( ECC_OFF, 0, 0, 0, 0 );
	}

	return res;
}
#endif


#if defined(__USE_NAND_ISR__)
#if defined(_LINUX_) && defined(NAND_FOR_KERNEL)
EXPORT_SYMBOL(NAND_IO_SetupDMA);
EXPORT_SYMBOL(NAND_IO_IRQ_Enable);
EXPORT_SYMBOL(NAND_IO_IRQ_Init);
EXPORT_SYMBOL(NAND_IO_ISR);
EXPORT_SYMBOL(NAND_IO_GetISRStructure);
#endif
#endif


/* End of file */
