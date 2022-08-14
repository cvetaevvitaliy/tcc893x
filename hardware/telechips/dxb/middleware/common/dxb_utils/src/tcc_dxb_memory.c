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

/******************************************************************************
* include 
******************************************************************************/
#include "tcc_dxb_memory.h"
#define LOG_TAG	"tcc_dxb_memory"
#include <utils/Log.h>


#ifdef	TCC_MEMORY_DEBUG
static unsigned int gMemallocCnt = 0;
static unsigned int gMemRemain = 0;
#endif

void* tcc_dxb_malloc (unsigned int iSize)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
	void *ptr;
	ptr = malloc(iSize);
#ifdef TCC_MEMORY_DEBUG
	ALOGD("mAlloc, 0x%08x, size[%d], mem cnt [%d]\n", ptr, iSize, gMemallocCnt);		
	gMemallocCnt++;	
#endif

	return ptr;
#else  
#endif
}

void* tcc_dxb_calloc (unsigned int isize_t, unsigned int iSize)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
	void *ptr;
	ptr = calloc(isize_t, iSize);
#ifdef TCC_MEMORY_DEBUG
	ALOGD("cAlloc, 0x%08x, size[%d], mem cnt [%d]\n", ptr, iSize, gMemallocCnt);		
	gMemallocCnt++;	
#endif
	return ptr;
#else  
#endif
}

void* tcc_dxb_realloc (void *p,unsigned int iSize)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
	void *ptr;
	ptr = realloc(p, iSize);
#ifdef TCC_MEMORY_DEBUG
	ALOGD("reAlloc, 0x%08x, size[%d], mem cnt [%d]\n", ptr, iSize, gMemallocCnt);		
	gMemallocCnt++;	

#endif
	return ptr;	
#else  
#endif
}

int tcc_dxb_free(void *pvPtr)
{
#ifdef TCC_LINUX_MEMORY_SYTEM
#ifdef TCC_MEMORY_DEBUG
	gMemallocCnt--;
	ALOGD("MemFree, 0x%08x, mem cnt [%d]\n", pvPtr, gMemallocCnt);	
#endif
	free(pvPtr);
#else
#endif

	return 0;
}

void* TCC_malloc(unsigned int iSize)
{
    return tcc_dxb_malloc(iSize);
}

void* TCC_free(void *pvPtr)
{
    return tcc_dxb_free(pvPtr);
}
