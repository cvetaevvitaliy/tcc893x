/****************************************************************************
*   FileName    : globals.h
*   Description : 
****************************************************************************
*
*   TCC Version : 1.0
*   Copyright (c) Telechips, Inc.
*   ALL RIGHTS RESERVED
*
****************************************************************************/

//using only global defines, macros.. etc - If you want using this file contact to RYU

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <platform/tcc_ckc.h>

//Log Level
#define TC_ERROR 	0x00000001
#define TC_LOG		0x00000002
#define TC_TRACE	0x00000004
#define	TC_DEBUG	0x00000008

	//system info
#define IOCTL_PLATFORM_TYPE                 (L"PLATFORM_TYPE")
#define IOCTL_PLATFORM_OEM                  (L"PLATFORM_OEM")

//------------------------------------------------------------------------------
//  Define:  IOCTL_PROCESSOR_VENDOR/NAME/CORE
//
//  Defines the processor information
//

#define IOCTL_PROCESSOR_VENDOR              (L"Telechips")
#define IOCTL_PROCESSOR_NAME                (L"TCC892x")
#define IOCTL_PROCESSOR_CORE                (L"Cortex-A5")

//------------------------------------------------------------------------------
//
//  Define:  IOCTL_PROCESSOR_INSTRUCTION_SET
//
//  Defines the processor instruction set information
//
#define IOCTL_PROCESSOR_INSTRUCTION_SET     (0)
#define IOCTL_PROCESSOR_CLOCK_SPEED	    266*1000

//macro defines
/************************************************************************************************
*										 MACRO												   *
************************************************************************************************/
#ifndef BITSET
#define BITSET(X, MASK) 			( (X) |= (unsigned int)(MASK) )
#endif
#ifndef BITSCLR
#define BITSCLR(X, SMASK, CMASK)	( (X) = ((((unsigned int)(X)) | ((unsigned int)(SMASK))) & ~((unsigned int)(CMASK))) )
#endif
#ifndef BITCSET
#define BITCSET(X, CMASK, SMASK)	( (X) = ((((unsigned int)(X)) & ~((unsigned int)(CMASK))) | ((unsigned int)(SMASK))) )
#endif
#ifndef BITCLR
#define BITCLR(X, MASK) 			( (X) &= ~((unsigned int)(MASK)) )
#endif
#ifndef BITXOR
#define BITXOR(X, MASK) 			( (X) ^= (unsigned int)(MASK) )
#endif
#ifndef ISZERO
#define ISZERO(X, MASK) 			(  ! (((unsigned int)(X)) & ((unsigned int)(MASK))) )
#endif

#ifndef ISSET
#define	ISSET(X, MASK)				( (unsigned long)(X) & ((unsigned long)(MASK)) )
#endif


#ifndef ENABLE
#define ENABLE 1
#endif
#ifndef DISABLE
#define DISABLE 0
#endif

#ifndef ON
#define ON		1
#endif
#ifndef OFF
#define OFF 	0
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	1
#endif

#define HwVMT_SZ(X) 							(((X)-1)*Hw12)
	#define SIZE_4GB								32
	#define SIZE_2GB								31
	#define SIZE_1GB								30
	#define SIZE_512MB								29
	#define SIZE_256MB								28
	#define SIZE_128MB								27
	#define SIZE_64MB								26
	#define SIZE_32MB								25
	#define SIZE_16MB								24
	#define SIZE_8MB								23
	#define SIZE_4MB								22
	#define SIZE_2MB								21
	#define SIZE_1MB								20
	#define HwVMT_REGION_AP_ALL 				(Hw11+Hw10)
	#define HwVMT_DOMAIN(X) 					((X)*Hw5)
	#define HwVMT_REGION_EN 					Hw9 							// Region Enable Register
	#define HwVMT_CACHE_ON						Hw3 							// Cacheable Register
	#define HwVMT_CACHE_OFF 					HwZERO
	#define HwVMT_BUFF_ON							Hw2 							// Bufferable Register
	#define HwVMT_BUFF_OFF							HwZERO

	#define HwVMT_REGION0_EN						Hw9 							// Region Enable Register
	#define HwVMT_REGION0_CA						Hw3 							// Cacheable Register
	#define HwVMT_REGION0_BU						Hw2 							// Bufferable Register

/************************************************************************************************
*										 ENUM												   *
************************************************************************************************/

/***************************************Interrup****************************************************/
enum {
	IRQ_TC0=0,      // 0   Timer 0 interrupt enable 
	IRQ_TC32,       // 1   Timer 1 interrupt enable 
	IRQ_SMUI2C,     // 2   SMU_I2C interrupt enable 
	IRQ_EI0,        // 3   External interrupt 0 enable 
	IRQ_EI1,        // 4   External interrupt 1 enable 
	IRQ_EI2,        // 5   External interrupt 2 enable 
	IRQ_EI3,        // 6   External interrupt 3 enable 
	IRQ_EI4,        // 7   External interrupt 4 enable 
	IRQ_EI5,        // 8   External interrupt 5 enable 
	IRQ_EI6,        // 9   External interrupt 6 enable 
	IRQ_EI7,        // 10  External interrupt 7 enable 
	IRQ_EI8,        // 11  External interrupt 8 enable 
	IRQ_EI9,        // 12  External interrupt 9 enable 
	IRQ_EI10,       // 13  External interrupt 10 enable 
	IRQ_EI11,       // 14  External interrupt 11 enable 
	IRQ_JPGE,       // 15  JPEG Encoder interrupt enable 
	IRQ_3DPPMMU,    // 16  3D Pixel Processor MMU interrupt enable 
	IRQ_3DGPMMU,    // 17  3D Geometry Processor MMU interrupt enable 
	IRQ_DNL,        // 18  Denali Controller interrupt enable
	IRQ_L2,         // 19  L2 cache interrupt enable
	IRQ_LCD,  	    // 20  LCD controller interrupt enable 
	IRQ_JPG,        // 21  JPEG interrupt enable (Video codec)
	IRQ_VCDC=23,    // 23  Video CODEC interrupt enable 
	IRQ_3DPP,       // 24  3D Pixel Processor interrupt enable 
	IRQ_3DGP,       // 25  3D Geometry Processor interrupt enable 
	IRQ_3DPMU,      // 26  3D PMU interrupt enable 
	IRQ_OM,         // 27  Overlay Mixer interrupt enable 
	IRQ_TSADC,      // 28  TSADC interrupt enable 
	IRQ_GDMA,       // 29  General DMA controller interrupt enable  (IOBUS)
	IRQ_DMAX,       // 30  DMAX interrupt enable
	IRQ_EHI,        // 31  External Host interrupt enable 

	IRQ_SD3,        // 32 0  SDMMC 3 interrupt enable 
	IRQ_SD2,        // 33 1  SDMMC 2 interrupt enable 
	IRQ_HDMI,       // 34 2  HDMI interrupt enable 
	IRQ_GPSB=36,    // 36 4  GPSB Interrupt Enable 
	IRQ_I2C=38,     // 38 6  I2C interrupt enable
	IRQ_MPEFEC,     // 39 7  MPEFEC interrupt enable 
	IRQ_NFC=41,     // 41 9  Nand flash controller interrupt enable 
	IRQ_RMT,        // 42 10 Remote Control interrupt enable 
	IRQ_RTC,        // 43 11 RTC interrupt enable 
	IRQ_SD0,        // 44 12 SD/MMC 0 interrupt enable 
	IRQ_SD1,        // 45 13 SD/MMC 1 interrupt enable 
	IRQ_HSGDMA,     // 46 14 General DMA controller interrupt enable  (HSIOBUS)
	IRQ_UART,       // 47 15 UART interrupt enable 
	IRQ_UOTG,       // 48 16 USB 2.0 OTG interrupt enable 
	IRQ_UB20H,  	// 49 17 USB 2.0 HOST interrupt enable 
	IRQ_GMAC=51,    // 51 19 GMAC interrupt enable 
	IRQ_CIPHER,     // 52 20 Cipher interrupt enable 
	IRQ_TSIF,       // 53 21 TS interface interrupt enable 
	IRQ_UOTGOFF=56, // 56 24 USB OTG VBOFF interrupt enable 
	IRQ_UOTGON,     // 57 25 USB OTG VBON interrupt enable 
	IRQ_ADMA1,      // 58 26 AUDIO 1 DMA interrupt enable 
	IRQ_AUDIO1,     // 59 27 AUDIO 1 interrupt enable 
	IRQ_ADMA0,      // 60 28 AUDIO 0 DMA interrupt enable 
	IRQ_AUDIO0,     // 61 29 AUDIO 0 interrupt enable 
	IRQ_DUMMY7,     // 62 30 
	IRQ_ARMPMU,     // 63 31 ARM PMU interrupt enable 
};

#ifdef __cplusplus
}
#endif

#endif // __GLOBALS_H__
