#ifndef _AENC_VARS_H_
#define _AENC_VARS_H_

#include "../audio_codec/aenc.h"

typedef struct _AENC_VARS
{
	cdk_audio_func_t* gspfAEnc;
	aenc_input_t gsAEncInput;
	aenc_output_t gsAEncOutput;
	aenc_init_t gsAEncInit;
	unsigned char *gpucStreamBuffer;
	unsigned char *gpucPcmBuffer;
	unsigned int guiRecTime;
	unsigned int guiTimePos;
	TCAS_CDSyncInfo	gCDSync;
} AENC_VARS;

#endif // _AENC_VARS_H_
