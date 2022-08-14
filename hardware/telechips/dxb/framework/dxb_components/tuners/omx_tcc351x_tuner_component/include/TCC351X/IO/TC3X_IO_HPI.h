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

#ifndef __TC3X_IO_HPI_h__
#define __TC3X_IO_HPI_h__

#include "TC3X_IO_UserDefine.h"

#if defined (USE_IF_HPI)
void      TC3X_IO_HPI_Init (int moduleidx);
void      TC3X_IO_HPI_ReadStream (int moduleidx, unsigned char *data, int iSize);
#endif // USE_IF_HPI

#endif
