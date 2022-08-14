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
   																				
#ifndef _CDK_PRE_DEFINE_RELEASE_H_
#define _CDK_PRE_DEFINE_RELEASE_H_

#include "cdk_pre_define_origin.h"

#undef INCLUDE_EXT_F
#undef INCLUDE_RAG2_DEC
#undef INCLUDE_AC3_DEC
#undef INCLUDE_DTS_DEC
#undef INCLUDE_RV9_DEC

// if you wanna use a specific library, 
// remove comment at head of the correspoding line

// EXT media fileformat parser
//#define INCLUDE_EXT_F

// real audio codec
//#define INCLUDE_RAG2_DEC

// ac3 audio codec
//#define INCLUDE_AC3_DEC

// dts audio codec
//#define INCLUDE_DTS_DEC

// real video 9 video codec
//#define INCLUDE_RV9_DEC

#endif // _CDK_PRE_DEFINE_RELEASE_H_
