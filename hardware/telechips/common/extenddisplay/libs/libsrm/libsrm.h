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

#ifndef __LIBSRM_H__
#define __LIBSRM_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "../libsmp/hdmi_sha1.h"
#include "../libsmp/smp.h"

//! HDCP SRM function message structure
typedef struct _hdcp_srm_data_st_
{
    unsigned char* pbRcvr;      //!< Received version of r (r')
    unsigned char* pbRcvs;      //!< Received version of s (s')
} SRM_DATA;

//! HDCP SRM function key structure
typedef struct _hdmi_srm_key_st_
{
    unsigned char* pbP;         //!< Prime Modulus (p)
    unsigned char* pbQ;         //!< Prime Divisor (q)
    unsigned char* pbGen;       //!< Generator     (g)
    unsigned char* pbPublic;    //!< Public key    (y)
} SRM_KEY;

//-------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------

// HDCP SRM functions
// check_SRM_ver() is deleted.
unsigned int verify_SRM(unsigned char* message);

#ifdef  __cplusplus
}
#endif
#endif
