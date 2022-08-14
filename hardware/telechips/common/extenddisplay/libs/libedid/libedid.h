/****************************************************************************
Copyright (C) 2013 Telechips Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

#ifndef _LIBEDID_H_
#define _LIBEDID_H_

#if defined(HDMI_V1_3)
#include <mach/hdmi_1_3_hdmi.h>
#include <mach/hdmi_1_3_video.h>
#include <mach/hdmi_1_3_audio.h>
#else
#include "mach/hdmi_1_4_hdmi.h"
#include <mach/hdmi_1_4_video.h>
#include <mach/hdmi_1_4_audio.h>
#endif//
#ifdef __cplusplus
extern "C" {
#endif

int EDIDOpen();
int EDIDRead();
void EDIDReset();
int EDIDHDMIModeSupport(struct HDMIVideoParameter const *video);
int EDIDHDMIModeSupportForCTS(const struct HDMIVideoParameter * const video);
int EDIDVideoModeSupport(struct HDMIVideoParameter const *video);
int EDIDAudioModeSupport(struct HDMIAudioParameter const *audio);
int EDIDGetCECPhysicalAddress(int* const outAddr);
int EDIDClose();
void EDIDInfo(void);
#if defined(TELECHIPS)
int EDIDReadBlock(const unsigned int blockNum, unsigned char* const outBuffer);
int EDIDParse(int *hmax, int *vmax, int *interlaced);
int EDIDSimpleDetectResolution(enum VideoFormat *res_out, int *hactive_out, int *vactive_out);
int CheckResolution(const enum VideoFormat videoFormat, const enum PixelAspectRatio pixelRatio);
int CheckResolutionOfDVI(const enum VideoFormat videoFormat, const enum PixelAspectRatio pixelRatio, unsigned int width, unsigned int height, unsigned frame_hz);
int CheckColorSpace(const enum ColorSpace space);
const enum ColorSpace CheckColorSpaceWithEDID(void);
int getEDID(unsigned char* outBuffer);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _LIBEDID_H_ */
