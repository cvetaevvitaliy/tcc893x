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
