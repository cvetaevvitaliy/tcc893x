/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions andlimitations under the License.

****************************************************************************/

#ifndef	_TCC_UTIL_H__
#define	_TCC_UTIL_H__

/******************************************************************************
* include 
******************************************************************************/
#include "globals.h"

/******************************************************************************
* typedefs & structure
******************************************************************************/
typedef unsigned char	U8;
typedef unsigned short	U16;
typedef unsigned int	U32;

/******************************************************************************
* defines 
******************************************************************************/

#ifndef BITSET
#define	BITSET(X, MASK)				( (X) |= (U32)(MASK) )
#endif

#ifndef BITSET
#define	BITSET(X, MASK)				( (X) |= (U32)(MASK) )
#endif
#ifndef BITSCLR
#define	BITSCLR(X, SMASK, CMASK)	( (X) = ((((U32)(X)) | ((U32)(SMASK))) & ~((U32)(CMASK))) )
#endif
#ifndef BITCSET
#define	BITCSET(X, CMASK, SMASK)	( (X) = ((((U32)(X)) & ~((U32)(CMASK))) | ((U32)(SMASK))) )
#endif
#ifndef BITCLR
#define	BITCLR(X, MASK)				( (X) &= ~((U32)(MASK)) )
#endif
#ifndef BITXOR
#define	BITXOR(X, MASK)				( (X) ^= (U32)(MASK) )
#endif
#ifndef ISZERO
#define	ISZERO(X, MASK)				(  ! (((U32)(X)) & ((U32)(MASK))) )
#endif
#ifndef BYTE_OF
#define	BYTE_OF(X)					( *(volatile unsigned char *)(&(X)) )
#endif
#ifndef HWORD_OF
#define	HWORD_OF(X)					( *(volatile unsigned short *)(&(X)) )
#endif
#ifndef WORD_OF
#define	WORD_OF(X)					( *(volatile unsigned int *)(&(X)) )
#endif

#ifndef MCHw1
#define MCHw1(x)  ( (0x00000001<<(x)))
#endif

#ifndef MCHw0
#define	MCHw0(x)  (~(0x00000001<<(x)))
#endif



/******************************************************************************
* globals
******************************************************************************/

/******************************************************************************
* locals
******************************************************************************/


/******************************************************************************
* declarations
******************************************************************************/



#endif //_TCC_UTIL_H___
