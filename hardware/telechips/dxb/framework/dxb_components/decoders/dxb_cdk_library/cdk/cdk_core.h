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
   																				
#ifndef _CDK_CORE_H_
#define _CDK_CORE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cdk_pre_define.h"
#include "cdk_sys.h"
//! Container demuxer format
#include "../container/cdmx.h"
//! Container muxer format
#include "../container/cmux.h"
//! Video codec Decoder
#include "../video_codec/vdec.h"
//! Video codec Encoder
#include "../video_codec/venc.h"
//! Audio codec Decoder
#include "../audio_codec/adec.h"
//! Audio codec Encoder
#include "../audio_codec/aenc.h"


#ifdef __cplusplus
}
#endif

#endif //_CDK_CORE_H_
