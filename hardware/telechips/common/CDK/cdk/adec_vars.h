#ifndef _ADEC_VARS_H_
#define _ADEC_VARS_H_

#include "../audio_codec/adec.h"

typedef struct _ADEC_VARS
{
	cdk_audio_func_t* gspfADec;
	adec_input_t gsADecInput;
	adec_output_t gsADecOutput;
	adec_init_t gsADecInit;
	unsigned char *gpucAudioBuffer;
	unsigned int guiAudioMinDataSize;
	unsigned int guiAudioInBufferSize;
	unsigned int guiAudioContinueThroughErrors;
	int giAudioErrorCount;
	void *gspLatmHandle;
} ADEC_VARS;

#endif // _ADEC_VARS_H_
