/****************************************************************************
 *   FileName    : fwdn_NAND.c
 *   Description : NAND FLASH F/W downloader function
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips, Inc.
 *   ALL RIGHTS RESERVED
 *
 ****************************************************************************/
#if defined(_LINUX_) || defined(_WINCE_)
#if defined(FWDN_V6)
#include <fwdn/fwdn_drv_v3.h>
#else
#include <fwdn/fwdn_drv_v7.h>
#endif
#include <fwdn/fwupgrade.h>
#include "nand_drv_v8.h"

#if defined (NKUSE)
	#include "windows.h"
	#include "stdlib.h"
#else
	#include <fwdn/TC_File.h>
#endif
#endif

#if defined(_WINCE_)
#include <memory.h>
#endif

//=============================================================================
//*
//*
//*                           [ CONST DATA DEFINE ]
//*
//*
//=============================================================================
#undef	LPRINTF
#define	LPRINTF	(1) ? (void)0 : IO_DBG_SerialPrintf_
#ifndef min
	#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

//=============================================================================
//*
//*
//*                           [ Internal Functions ]
//*
//*
//=============================================================================


//=============================================================================
//*
//*
//*                     [ EXTERN VARIABLE & FUNCTIONS DEFINE ]
//*
//*
//=============================================================================

extern unsigned short int		gFWUG_NAND_GoodBlockList[];
extern const unsigned int		CRC32_TABLE[256];

extern unsigned int				g_uiFWDN_OverWriteSNFlag;
extern unsigned int				g_uiFWDN_WriteSNFlag;
extern unsigned int				gMAX_ROMSIZE;	//twkwon: Initialize????

unsigned int	gFileHandle; // Getting from WinCE Application
unsigned int 	gFileSize;

/**************************************************************************
*  FUNCTION NAME :
*
*      int		FwdnWriteNandFirmware(unsigned char *pucRomBuffer, unsigned uFWSize);
*
*  DESCRIPTION : Check Size, Prepare Buffer, Write ROM code into NAND Flash.
*
*  INPUT:
*			uFWSize	= Code Size (byte unit)
*
*  OUTPUT:	int - Return Type
*  			= 0 : successful
*			= other : failure
*
**************************************************************************/
#include <debug.h>

#define FWDN_BUF_SIZE		(64*1024)			// It should be larger then 2048 + 512
unsigned char	gFWDN_Buf[FWDN_BUF_SIZE]__attribute__((aligned(8)));


int		FwdnWriteNandFirmware( unsigned char *pucRomBuffer, unsigned uFWSize )
{
	int res = 0;

	res = NAND_WriteFirmware(0,pucRomBuffer,uFWSize);
	return res;
}

/**************************************************************************
*  FUNCTION NAME :
*
*      int	FwdnGetNandSerial(unsigned char *pucSN, unsigned int uiSize);
*
*  DESCRIPTION : Read and Check the Serial Number stored at NAND Flash.
*                        Verification result is reflected to global structure of FWDN_DeviceInformation.
*
*  INPUT:
*			None
*
*  OUTPUT:	int - Return Type
*  			= always 0
*
**************************************************************************/
int	FwdnGetNandSerial(unsigned char *pucSN, unsigned int uiSize)
{
	return NAND_GetSerialNumber(pucSN, uiSize);
}


/**************************************************************************
*  FUNCTION NAME :
*
*      int FwdnSetNandSerial(unsigned char *ucData, unsigned int overwrite);
*
*  DESCRIPTION : Set Serial Number into global structure of FWDN_DeviceInformation.
*
*  INPUT:
*			ucData	= Serial Number
*			overwrite	=	0 : check existing serial number and not overwrite if that is valid.
*						1 : overwrite serial number regardless of existing one.
*
*  OUTPUT:	int - Return Type
*  			= always 0
*
**************************************************************************/
int FwdnSetNandSerial(unsigned char *ucData, unsigned int overwrite)
{
	memcpy( FWDN_DeviceInformation.DevSerialNumber, ucData , 32 );

	if( overwrite )
	{
		NAND_SetSerialNumber(ucData, 32);
	}
	return 0;
}

/* end of file */
