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

#ifndef __TC3X_API_H__
#define __TC3X_API_H__

#include "IO/TC3X_IO_UserDefine.h"
#include "IO/TC3X_IO_UTIL.h"

#if defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
void      TC3X_API_DVBT_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3, void *param4);
#endif // defined (STDDEF_DVBT) || defined (STDDEF_DVBH)
#if defined (STDDEF_DVBH)
void      TC3X_API_DVBH_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3, void *param4);
#endif // STDDEF_DVBH
#if defined (STDDEF_CMMB)
void      TC3X_API_CMMB_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3, void *param4);
#endif // STDDEF_CMMB
#if defined (STDDEF_DMB)
void      TC3X_API_TDMB_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3, void *param4);
#endif // STDDEF_DMB
#if defined(STDDEF_ISDBT1SEG)
void      TC3X_API_ISDBT1SEG_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3,
                                         void *param4);
#endif // defined(STDDEF_ISDBT1SEG)
#if defined(STDDEF_ISDBT13SEG)
void      TC3X_API_ISDBT13SEG_OccurEvent (int moduleidx, TC3X_DNUM handle, int message, void *param1, void *param2, void *param3,
                                          void *param4);
#endif // defined(STDDEF_ISDBT13SEG)

#endif
