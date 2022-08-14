/**

  Copyright (C) 2009-2010 Telechips Inc.

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA
  02110-1301  USA

*/

#ifndef	_TCC_MEMORY_H__
#define	_TCC_MEMORY_H__

/******************************************************************************
* include 
******************************************************************************/
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
* typedefs & structure
******************************************************************************/


/******************************************************************************
* defines 
******************************************************************************/

#define	TCC_LINUX_MEMORY_SYTEM

//#define	TCC_MEMORY_DEBUG	// for Telechips Memory  Debugging 


/******************************************************************************
* globals
******************************************************************************/

/******************************************************************************
* locals
******************************************************************************/


/******************************************************************************
* declarations
******************************************************************************/
void* TCC_malloc (unsigned int iSize);
void* TCC_calloc (unsigned int isize_t, unsigned int iSize);
void* TCC_realloc (void *p,unsigned int iSize);
int TCC_free(void *pvPtr);

 #ifdef	TCC_MEMORY_DEBUG
 int TCC_memcmp(char *s0, char *s1, int size);
int TCC_memcpy(char *d, char *s, int size);
int TCC_memset(char *p, char val, int size);
void* TCC_malloc_1(unsigned int iSize);
 void* TCC_calloc_1(unsigned int size_t, unsigned int iSize);
 void* TCC_realloc_1(void *p,unsigned int iSize);
 int TCC_free_1(void *pvPtr);
#endif

#ifdef __cplusplus
}
#endif

#endif //_TCC_UTIL_H___
