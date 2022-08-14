/****************************************************************************
 *   FileName    : nand_utils.h
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
#ifndef __NAND_UTIL_H
#define __NAND_UTIL_H

//========================================================================
//
// Defines
//
//========================================================================
#ifndef BITSET
#define	BITSET(X, MASK)				( (X) |= (MASK) )
#endif
#ifndef BITSCLR
#define	BITSCLR(X, SMASK, CMASK)	( (X) = (((X) | (SMASK)) & ~(CMASK)) )
#endif
#ifndef BITCSET
#define	BITCSET(X, CMASK, SMASK)	( (X) = (((X) & ~(CMASK)) | (SMASK)) )
#endif
#ifndef BITCLR
#define	BITCLR(X, MASK)				( (X) &= ~(MASK) )
#endif
#ifndef BITXOR
#define	BITXOR(X, MASK)				( (X) ^= (MASK) )
#endif
#ifndef ISZERO
#define	ISZERO(X, MASK)				(  ! ((X) & (MASK)) )
#endif
#ifndef ISALLONE
#define	ISALLONE(X, MASK)			( ((X) & (MASK)) == (MASK) )
#endif
#ifndef BYTE_OF
#define	BYTE_OF(X)					( *(volatile unsigned char *)(&(X)) )
#endif
#ifndef HWORD_OF
#define	HWORD_OF(X)					( *(volatile unsigned short *)(&(X)) )
#endif
#ifndef WORD_OF
#define	WORD_OF(X)					( *(volatile unsigned long *)(&(X)) )
#endif

#ifndef dim
#define dim(a)					( sizeof(a) / sizeof(a[0]) )
#endif

#ifndef GetDword
#define GetDword(x)					(((x)[0]) + ((x)[1]<<8) + ((x)[2]<<16) + ((x)[3]<<24))
#endif // GetDword

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef GetOffsetOf
#define GetOffsetOf(x)	( (unsigned int)(&x) & 0x0FFFFFFF )
#endif

#ifndef ASM_NOP
#if defined(_LINUX_)
	#define ASM_NOP {					\
			__asm__ __volatile__ ("nop");	\
			__asm__ __volatile__ ("nop");	\
			__asm__ __volatile__ ("nop");	\
			__asm__ __volatile__ ("nop");	\
			__asm__ __volatile__ ("nop");	\
			__asm__ __volatile__ ("nop");	\
		}
#elif defined(_WINCE_)
	static volatile int __asm_nop_count = 0;
	#define ASM_NOP { __asm_nop_count++; }
#else
	#define ASM_NOP { __asm{ NOP }; __asm{ NOP };  __asm{ NOP }; __asm{ NOP }; __asm{ NOP }; __asm{ NOP };}
#endif
#endif


//========================================================================
//
// Extern Functions
//
//========================================================================
#if 1
#if !defined(_NU_)
extern void						*memcpy(void *, const void *, unsigned int);
#endif
#define NAND_Util_memcpy		memcpy
#else
//extern void 					NAND_Util_memcpy( void *pvDst, const void *pvSrc, unsigned int uiCount );
#endif
extern void 					NAND_Util_Delay( void );
extern unsigned int				NAND_Util_GetShiftValueForFastMultiPly( unsigned int uiNumber, unsigned int uiMaxFactor );
#ifdef SPEED_CHECK
extern void 					NAND_Util_GPIO_Toggle( unsigned int nBitNum );
#endif


#endif	/* __NAND_UTIL_H */
