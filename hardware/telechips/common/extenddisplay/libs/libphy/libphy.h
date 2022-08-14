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

#ifndef _LIBPHY_H_
#define _LIBPHY_H_

#if defined(HDMI_V1_4)
#include <mach/hdmi_1_4_hdmi.h>
#elif defined(HDMI_V1_3)
#include <mach/hdmi_1_3_video.h>
#endif//

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HDMI_V1_4)
int PHYConfig(const enum PHYFreq freq, const enum ColorDepth depth);
#elif defined(HDMI_V1_3)
int PHYConfig(const enum PixelFreq clk, const enum ColorDepth depth);
#endif
int PHYPowerDown(void);

#ifdef __cplusplus
}
#endif

#endif /* _LIBPHY_H_ */
