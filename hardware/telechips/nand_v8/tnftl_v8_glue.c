/****************************************************************************
 *   FileName    : tnftl_v8_glue.c
 *   Description : glue functions for tnftl library
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
#include <stdarg.h>
#include "nand_def.h"
#include "tnftl_v8_api.h"
#include "nand_io_v8.h"

#if defined(_LINUX_) || defined(_WINCE_)
#include "fwupgrade.h"
#else
#include "SERVICE/fwdn/fwupgrade.h"
#endif

#if defined(_LINUX_) && defined(NAND_FOR_KERNEL)
#include <linux/kernel.h>
#include <linux/string.h>
#else
#include <stdio.h>
#endif

#define TNFTL_GLUE_ASSERT(a)			if(!(a))\
										{\
											ND_TRACE("[ASSERT] %s:%d: %s\n",__FUNCTION__,__LINE__,#a);\
										}

#if 0
/******************************************************************************

	void			TNFTL_memcpy

	Input	: pvDst - New buffer
			  pvSrc - Buffer to copy from
			  uiLength - Number of bytes to copy
	Output	:
	Return	:

	Description : Copy bytes between buffers

*******************************************************************************/
void TNFTL_memcpy( void *pvDst, const void *pvSrc, unsigned int uiLength )
{
	if( ( unsigned int )pvDst & 0x3 || ( unsigned int )pvSrc & 0x3 || uiLength & 0x3 )
	{
		unsigned char *pucDst = ( unsigned char * )pvDst;
		const unsigned char *pucSrc = ( const unsigned char * )pvSrc;
		while( uiLength-- )
			*pucDst++ = *pucSrc++;
	}
	else
	{
		memcpy( pvDst, pvSrc, uiLength );
	}
}

/******************************************************************************

	void			TNFTL_memset

	Input	: pvDst - Buffer to be set
			  ucValue - value to set
			  uiLength - Number of bytes to set
	Output	:
	Return	:

	Description : Set value to buffers

*******************************************************************************/
void TNFTL_memset( void *pvDst, unsigned char ucValue, unsigned int uiLength )
{
	memset( pvDst, ucValue, uiLength );
}

/******************************************************************************

	void			TNFTL_memcmp

	Input	: pvSrc0 - source 0
			  pvSrc1 - source 1
			  uiLength - Number of bytes to compare
	Output	:
	Return	:

	Description : Compare bytes between buffers

*******************************************************************************/
int TNFTL_memcmp( const void *pvSrc0, const void *pvSrc1, unsigned int uiLength )
{
	return memcmp( pvSrc0, pvSrc1, uiLength );
}
#endif

/******************************************************************************

	void			TNFTL_printf

	Input	:
	Output	:
	Return	:

	Description :

*******************************************************************************/
static unsigned int _PrintHex( char *szFmt, unsigned int uiLength, unsigned int n )
{
	unsigned int uiPrinted = 0;

	if( ( n & ~0xf ) )
	{
		uiPrinted = _PrintHex( szFmt, uiLength, n >> 4 );
		szFmt = &szFmt[uiPrinted];
		uiLength -= uiPrinted;
		n &= 0xf;
	}

	if( uiLength-- )
	{
		if( n < 10 )
		{
			szFmt[0] = ( char )( n + '0' );
		}
		else
		{
			szFmt[0] = ( char )( n - 10 + 'A' );
		}
		uiPrinted++;
	}

	return uiPrinted;
}

static unsigned int _PrintDecimal( char *szFmt, unsigned int uiLength, unsigned int n )
{
	unsigned int uiPrinted = 0;

	if( ( n / 10 ) )
	{
		uiPrinted = _PrintDecimal( szFmt, uiLength, n / 10 );
		szFmt = &szFmt[uiPrinted];
		uiLength -= uiPrinted;
		n %= 10;
	}

	if( uiLength-- )
	{
		szFmt[0] = ( char )( n + '0' );
		uiPrinted++;
	}

	return uiPrinted;
}

static unsigned int _PrintString( char *szFmt, unsigned int uiLength, const char *s )
{
	unsigned int uiPrinted = 0;
	while( *s && uiLength-- )
	{
		if( *s == '\n' )
		{
			*szFmt++ = '\r';
			uiPrinted++;
		}
		*szFmt++ = *s++;
		uiPrinted++;
	}

	return uiPrinted;
}

void TNFTL_printf( char *fmt, ... )
{
	char			szBuf[256];
	char			*pBuf = szBuf;
	unsigned int	uiBufLength = sizeof( szBuf ) - 1;
	char			c;
	unsigned int	uiPrinted;
	va_list			vl;

	va_start( vl, fmt );

	while( *fmt )
	{
		c = *fmt++;
		switch( c )
		{
			case '%':
				c = *fmt++;
				switch( c )
				{
					case 'x':
					case 'X':
						uiPrinted = _PrintHex( pBuf, uiBufLength, va_arg( vl, unsigned int ) );
						pBuf = &pBuf[uiPrinted];
						uiBufLength -= uiPrinted;
						break;
					case 'd':
					{
						long    l;
						l = va_arg( vl, long );
						if( l < 0 )
						{
							if( uiBufLength-- )
								*pBuf++ = '-';
							l = - l;
						}
						uiPrinted = _PrintDecimal( pBuf, uiBufLength, ( unsigned int )l );
						pBuf = &pBuf[uiPrinted];
						uiBufLength -= uiPrinted;
					}
					break;
					case 'u':
						uiPrinted = _PrintDecimal( pBuf, uiBufLength, va_arg( vl, unsigned int ) );
						pBuf = &pBuf[uiPrinted];
						uiBufLength -= uiPrinted;
						break;
					case 's':
						uiPrinted = _PrintString( pBuf, uiBufLength, va_arg( vl, char * ) );
						pBuf = &pBuf[uiPrinted];
						uiBufLength -= uiPrinted;
						break;
					case '%':
						if( uiBufLength-- )
							*pBuf++ = '%';
						break;
					case 'c':
						c = ( char )va_arg( vl, unsigned int );
						if( uiBufLength-- )
							*pBuf++ = c;
						break;

					default:
						if( uiBufLength-- )
							*pBuf++ = ' ';
						break;
				}
				break;
			case '\r':
				if( *fmt == '\n' )
					fmt ++;
				c = '\n';
				// fall through
			case '\n':
				if( uiBufLength-- )
					*pBuf++ = '\r';
				// fall through
			default:
				if( uiBufLength-- )
					*pBuf++ = c;
		}
	}

	va_end( vl );

	*pBuf = '\0';

#if defined(NAND_FOR_KERNEL) && defined(_UNICODE)
	{
		wchar_t wcBuf[256];
		mbstowcs( wcBuf, szBuf, 256 );
		wcBuf[255] = L'\0';
		_ND_TRACE( "%s", wcBuf );
	}
#else
	_ND_TRACE( "%s", szBuf );
#endif
}

void *TNFTL_malloc( unsigned int uiLength )
{
#if defined(NAND_FOR_KERNEL)
	#if defined(_LINUX_)
		return NULL;
	#else
		return malloc( uiLength );
	#endif

#elif defined(_NU_)
	return malloc( uiLength );

#else
	return ( void * )ROMFILE_TEMP_BUFFER;

#endif
}

void TNFTL_free( void *ptr )
{
#if defined(NAND_FOR_KERNEL) && !defined(_LINUX_)
	free( ptr );
#endif
}

unsigned int TNFTL_CalcCRC( void *pvBase, unsigned int uiLength, unsigned int uiCRCIN )
{
	return FWUG_CalcCrc8I( uiCRCIN, ( unsigned char * )pvBase, uiLength );
}

#if defined(TCC800X)
unsigned int TNFTL_CalcCRCI( void *pvBase, unsigned int uiLength, unsigned int uiCRCIN )
{
	return FWUG_CalcCrcI( uiCRCIN, ( unsigned int * )pvBase, uiLength );
}
#endif

unsigned int TNFTL_IO_GetFeatureOfNAND( TNFTL_OBJECT_T *pTnftlObj )
{
	return NAND_IO_GetFeatureOfNAND( pTnftlObj );
}

unsigned int TNFTL_IO_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPage )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_WRITE;
	res = NAND_IO_WritePage( pTnftlObj, (unsigned int)uiPageAddress, (unsigned int *)puiBuffer, (unsigned int *)puiTag, uiTagSize, fNextPage );

	//{
	//	static unsigned int s_uiVerifyPageBuffer[8192/4];
	//	static unsigned int s_uiVerifyTagBuffer[448/4];
	//	NAND_IO_ReadPage(pTnftlObj, uiPageAddress, s_uiVerifyPageBuffer, s_uiVerifyTagBuffer);
	//	if(memcmp(puiBuffer,s_uiVerifyPageBuffer,pTnftlObj->uiBytesPerPage))
	//	{
	//		ND_TRACE("TNFTL_IO_WritePage: memcmp() for data is failed!\n");
	//	}
	//	if(memcmp(puiTag,s_uiVerifyTagBuffer,TNFTL_TAG_BYTE_SIZE))
	//	{
	//		ND_TRACE("TNFTL_IO_WritePage: memcmp() for tag is failed!\n");
	//	}
	//}

	return res;
}

unsigned int TNFTL_IO_MultiPlane_WritePage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	res = NAND_IO_MultiPlane_WritePage( pTnftlObj, (unsigned int)uiPageAddress, (unsigned int *)puiBuffer, (unsigned int *)puiTag, uiTagSize );
	if( res == TRUE )
	{
		pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_WRITE_PLANE0;
	}

	return res;
}

unsigned int TNFTL_IO_MultiPlane_WriteLastPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] == DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_WRITE_PLANE1;
	res = NAND_IO_MultiPlane_WriteLastPage( pTnftlObj, (unsigned int)uiPageAddress, (unsigned int *)puiBuffer, (unsigned int *)puiTag, uiTagSize );

	return res;
}

unsigned int TNFTL_IO_WriteChipBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag )
{
	unsigned int res;

	NAND_IO_SkipDataAreaRandomizing(TRUE);
	res = TNFTL_IO_WritePage( pTnftlObj, uiPageAddress, puiBuffer, puiTag, TNFTL_TAG_BYTE_SIZE, FALSE );
	NAND_IO_SkipDataAreaRandomizing(FALSE);

	return res;
}

unsigned int TNFTL_IO_ReadPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize, unsigned int fNextPageForNextTurn )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_READ;
	res = NAND_IO_ReadPage( pTnftlObj, uiPageAddress, puiBuffer, puiTag, uiTagSize, fNextPageForNextTurn );
	return res;
}

unsigned int TNFTL_IO_ReadChipBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag, unsigned int uiTagSize )
{
	unsigned int res;

	NAND_IO_SkipDataAreaRandomizing(TRUE);
	res = TNFTL_IO_ReadPage( pTnftlObj, uiPageAddress, puiBuffer, puiTag, uiTagSize, FALSE );
	NAND_IO_SkipDataAreaRandomizing(FALSE);

	return res;
}

unsigned int TNFTL_IO_ReadSpare( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiTag, unsigned int uiTagSize )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_READ;
	res = NAND_IO_ReadSpare( pTnftlObj, uiPageAddress, puiTag, uiTagSize );

	return res;
}

unsigned int TNFTL_IO_IsReadReclaimRequired( TNFTL_OBJECT_T *pTnftlObj )
{
	return NAND_IO_IsReadReclaimRequired(pTnftlObj);
}

unsigned char TNFTL_IO_GetEccErrorBitCount()
{
	return _NAND_IO_GetEccErrorBitCount();
}

unsigned int TNFTL_IO_CheckBL1ReadReclaim( TNFTL_OBJECT_T *pTnftlObj )
{
	TNFTL_ERROR_T res;
	unsigned int uiFirstBL1ErrorBit = 0, uiSecondBL1ErrorBit = 0;
	unsigned char ucFirstBootBlock[3]={0,}, ucSecondBootBlock[3]={0,};

	// Check Error Bit.
	res = TNFTL_FW_VerifyBL1ErrorBit( pTnftlObj, &uiFirstBL1ErrorBit, &uiSecondBL1ErrorBit, ucFirstBootBlock, ucSecondBootBlock );	

	if( res == TNFTL_SUCCESS )
	{
		//ND_TRACE("ErrorBit : %d,%d, BlockNum:%d,%d\n", uiFirstBL1ErrorBit, uiSecondBL1ErrorBit, ucFirstBootBlock[0], ucSecondBootBlock[0] );
	}

	// if there is no any BL1(first, second). it's init status.
	if( ( ucFirstBootBlock[0] == 0 ) && ( ucSecondBootBlock[0] == 0 ) )
		return TNFTL_SUCCESS;

	NAND_IO_SkipDataAreaRandomizing( TRUE );
	// Check whether there is broken BL1 or not.
	if( ucFirstBootBlock[0] == 0 )
	{
		ND_TRACE(" First BL1 is broken. Enter Recovery BL1\n");
		// Src : Second BL1. - Dest: First BL1
		res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucSecondBootBlock, ucFirstBootBlock, FALSE ); 
	}

	if( ucSecondBootBlock[0] == 0 )
	{
		ND_TRACE(" Second BL1 is broken. Enter Recovery BL1 \n");
		// Src : First BL1. - Dest: Second BL1
		res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucFirstBootBlock, ucSecondBootBlock, TRUE ); 
	}

	// Order by Error Bit. Erase Block first that have more error bit.
	if( NAND_IO_IsBL1ReadReclaimRequired( uiFirstBL1ErrorBit ) && NAND_IO_IsBL1ReadReclaimRequired( uiSecondBL1ErrorBit ) )
	{
		if( uiFirstBL1ErrorBit > uiSecondBL1ErrorBit )
		{
			res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucSecondBootBlock, ucFirstBootBlock, FALSE ); 
			if( res == TNFTL_SUCCESS )				
				res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucFirstBootBlock, ucSecondBootBlock, TRUE );
		}
		else
		{
			res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucFirstBootBlock, ucSecondBootBlock, TRUE );
			if( res == TNFTL_SUCCESS )				
				res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucSecondBootBlock, ucFirstBootBlock, FALSE ); 
		}
	}
	else if( NAND_IO_IsBL1ReadReclaimRequired( uiFirstBL1ErrorBit ) )
	{
		// Src : Second BL1. - Dest: First BL1
		res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucSecondBootBlock, ucFirstBootBlock, FALSE ); 
	}
	else if( NAND_IO_IsBL1ReadReclaimRequired( uiSecondBL1ErrorBit ) )
	{
		// Src : First BL1. - Dest: Second BL1
		res = TNFTL_FW_RecoveryBL1( pTnftlObj, ucFirstBootBlock, ucSecondBootBlock, TRUE ); 
	}

	NAND_IO_SkipDataAreaRandomizing( FALSE );

	if( res != TNFTL_SUCCESS )
		ND_TRACE(" NAND_BL1 Recovery Fail [0x%08X]\n", res );
	
	return res;
}

unsigned int TNFTL_IO_EraseBlock( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_ERASE;
	res = NAND_IO_EraseBlock( pTnftlObj, uiBlockAddress );

	return res;
}

void TNFTL_IO_MarkBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress )
{
	NAND_IO_MarkBad( pTnftlObj, uiBlockAddress );
}

TNFTL_BAD_BLOCK_STATUS_T TNFTL_IO_IsBad( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiBlockAddress )
{
	return NAND_IO_IsBad( pTnftlObj, uiBlockAddress );
}

unsigned int TNFTL_IO_MakeBootPage( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer )
{
	return NAND_IO_MakeBootPage( pTnftlObj, puiBuffer );
}

unsigned int TNFTL_IO_MakeBL2Page( TNFTL_OBJECT_T *pTnftlObj, unsigned int *puiBuffer, unsigned int uiPageSize, const unsigned int *puiTag )
{
	return NAND_IO_MakeBL2Page( pTnftlObj, puiBuffer, uiPageSize, puiTag );
}

#if defined(TCC800X)
unsigned int TNFTL_IO_WritePage_For_BootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_WRITE;
	res = NAND_IO_WritePage_For_TCC800X( pTnftlObj, (unsigned int)uiPageAddress, (unsigned int *)puiBuffer, (unsigned int *)puiTag );

	//{
	//	static unsigned int s_uiVerifyPageBuffer[8192/4];
	//	static unsigned int s_uiVerifyTagBuffer[448/4];
	//	NAND_IO_ReadPage(pTnftlObj, uiPageAddress, s_uiVerifyPageBuffer, s_uiVerifyTagBuffer);
	//	if(memcmp(puiBuffer,s_uiVerifyPageBuffer,pTnftlObj->uiBytesPerPage))
	//	{
	//		ND_TRACE("TNFTL_IO_WritePage: memcmp() for data is failed!\n");
	//	}
	//	if(memcmp(puiTag,s_uiVerifyTagBuffer,TNFTL_TAG_BYTE_SIZE))
	//	{
	//		ND_TRACE("TNFTL_IO_WritePage: memcmp() for tag is failed!\n");
	//	}
	//}

	return ( res == NAND_IO_SUCCESS )? TRUE : FALSE;
}

unsigned int TNFTL_IO_ReadPage_For_BootCode( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag )
{
	unsigned int res;

	TNFTL_GLUE_ASSERT( pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] != DIE_LAST_STATUS_WRITE_PLANE0 );
	pTnftlObj->enumLastStatus[pTnftlObj->uiCurrentDieIndex] = DIE_LAST_STATUS_READ;
	res = NAND_IO_ReadPage_For_TCC800X( pTnftlObj, uiPageAddress, puiBuffer, puiTag );
	return ( res == NAND_IO_SUCCESS )? TRUE : FALSE;
}

unsigned int TNFTL_IO_WritePage_For_GMC( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag )
{
	unsigned int res;
	
	res = NAND_IO_WriteGoldenPage_For_TCC800X( pTnftlObj, uiPageAddress, puiBuffer, puiTag);

	return ( res == NAND_IO_SUCCESS )? TRUE : FALSE;
}

unsigned int TNFTL_IO_ReadPage_For_GMC( TNFTL_OBJECT_T *pTnftlObj, unsigned int uiPageAddress, unsigned int *puiBuffer, unsigned int *puiTag )
{
	unsigned int res;

	res = NAND_IO_ReadGoldenPage_For_TCC800X( pTnftlObj, uiPageAddress, puiBuffer, puiTag);
	
	return ( res == NAND_IO_SUCCESS )? TRUE : FALSE;
}

void TNFTL_IO_AdjustPageSpareSize( unsigned short *pusPPages, unsigned short *pusPageSize, unsigned short *pusSpareSize, unsigned int *puiECCType )
{
	NAND_IO_AdjustPageSpareSize( pusPPages, pusPageSize, pusSpareSize, puiECCType );
}
#endif

