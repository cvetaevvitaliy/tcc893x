/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

#ifndef	_TCC_DXB_MEMORY_H__
#define	_TCC_DXB_MEMORY_H__

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
void* tcc_dxb_malloc (unsigned int iSize);
void* tcc_dxb_calloc (unsigned int isize_t, unsigned int iSize);
void* tcc_dxb_realloc (void *p,unsigned int iSize);
int tcc_dxb_free(void *pvPtr);

#if 0
#define TCC_malloc		tcc_dxb_malloc
#define TCC_calloc		tcc_dxb_calloc
#define TCC_Realloc		tcc_dxb_realloc
#define TCC_Free		tcc_dxb_free
#endif

#ifdef __cplusplus
}
#endif

#endif
