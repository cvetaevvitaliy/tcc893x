/**
 * Copyright (C) 2013, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef _W_DECODER_H_
#define _W_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

int TCC_VDEC_Init_H264(int Width, int Height);
int TCC_VDEC_Dec(unsigned int *pInputStream, unsigned int *pOutstream)	;																			
void TCC_VDEC_Close(void);

#ifdef __cplusplus
}
#endif
#endif
