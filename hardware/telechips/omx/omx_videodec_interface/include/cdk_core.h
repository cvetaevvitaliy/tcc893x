/****************************************************************************
 *   FileName    : cdk_core.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/

#ifndef _CDK_CORE_H_
#define _CDK_CORE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cdk_pre_define.h"
#ifdef HAVE_ANDROID_OS
#else
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
#endif

#ifdef __cplusplus
}
#endif
#endif //_CDK_CORE_H_
