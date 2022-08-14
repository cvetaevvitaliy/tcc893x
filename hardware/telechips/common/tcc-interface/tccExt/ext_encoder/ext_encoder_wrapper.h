/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef _W_ENCODER_H_
#define _W_ENCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

int TCC_VENC_Init_H264(unsigned int *pInitParam);
int TCC_VENC_Enc(unsigned int *pInputStream, unsigned int *pOutstream);																			
int TCC_VENC_Close(void);

#ifdef __cplusplus
}
#endif
#endif
