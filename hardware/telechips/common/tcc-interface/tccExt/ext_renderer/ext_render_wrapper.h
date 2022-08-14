/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef _W_RENDERER_H_
#define _W_RENDERER_H_

#include "common_if.h"

#ifdef __cplusplus
extern "C" {
#endif

void * wCreateRender( void *pNativeWindow, void *surf/*Surface pointer*/, unsigned int inWidth, unsigned int inHeight, tFRAME_BUF_FORMAT inFormat, int rotationDegrees, int cntOutBuff, bool aSyncMode);
void wDestoryRender( void *pInst );
int wRender( void *pInst, unsigned int Yaddr, unsigned int Uaddr, unsigned int Vaddr, char bAddrPhy, int64_t timestamp_ms, /*TCC_PLATFORM_PRIVATE_PMEM_INFO */ void *plat_priv );

#ifdef __cplusplus
}
#endif
#endif
