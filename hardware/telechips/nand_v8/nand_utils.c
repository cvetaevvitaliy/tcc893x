/****************************************************************************
 *   FileName    : nand_util.c
 *   Description : 
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
#if defined(_LINUX_) && defined(NAND_FOR_KERNEL)
#include <linux/kernel.h>
#include <linux/string.h>
#else
#include <stdio.h>
#endif
#include "nand_utils.h"

//void NAND_Util_memcpy( void *pvDst, const void *pvSrc, unsigned int uiCount )
//{
//	unsigned char *pucDst = ( unsigned char * )pvDst;
//	unsigned char *pucSrc = ( unsigned char * )pvSrc;
//
//	if( ( ( unsigned int )pvDst ) & 0x3 || ( ( unsigned int )pvSrc ) & 0x3 || uiCount & 0x3 )
//	{
//		while( uiCount-- )
//			*pucDst++ = *pucSrc++;
//	}
//	else
//	{
//		memcpy( pvDst, pvSrc, uiCount );
//	}
//}

void NAND_Util_Delay( void )
{
	/* TO DO : us 등의 단위로 구현 */
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
	ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP; ASM_NOP;
}

void NAND_Util_PrintOneSector( unsigned char *pucBuffer )
{
	int i;
	for( i = 0; i < 512; i++ )
	{
		_ND_TRACE( "%x ", pucBuffer[i] );
		if( ( i & 0xf ) == 0xf )
			_ND_TRACE( "\n" );
	}
}

unsigned int NAND_Util_GetShiftValueForFastMultiPly( unsigned int uiNumber, unsigned int uiMaxFactor )
{
	unsigned int i;

	for( i = 0 ; i < uiMaxFactor ; i++)
	{
		if(uiNumber == (unsigned int)(1<<i))
			break;
	}

	while(i==uiMaxFactor);

	return i;
}

#ifdef SPEED_CHECK
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
void NAND_Util_GPIO_Toggle( unsigned int nBitNum )
{
	if( pGPIO->GPFDAT & nBitNum )
		BITCLR( pGPIO->GPFDAT, nBitNum );
	else
		BITSET( pGPIO->GPFDAT, nBitNum );
}
#endif


#ifdef TCC_GUI_INCLUDE
#else
void TccUIDebug_Print( char *pStr, ... ) {
}
#endif
#ifdef COMMON_COM_LOG_INCLUDE
#else
void TC_ComAPI_LOGPrintf(unsigned int level, char *format, ...)
{
}
#endif

