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

#ifndef _LIBHPD_H_
#define _LIBHPD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/hdmi_hpd.h>

/**
 * HPD_CABLE_OUT indicates HDMI cable out.
 */
#define HPD_CABLE_OUT   0

/**
 * HPD_CABLE_IN indicates HDMI cable in.
 */
#define HPD_CABLE_IN    1

/**
 * @typedef    HPDCallback
 * @brief    Defines function pointer interface for HPD callback.
 */
typedef void HPDCallback(int);
int HPDCheck();
int HPDOpen();
int HPDClose();
int HPDStart();
int HPDStop();
int HPDSetCallback(HPDCallback *callback);

#ifdef __cplusplus
}
#endif

#endif /* _LIBHPD_H_ */
