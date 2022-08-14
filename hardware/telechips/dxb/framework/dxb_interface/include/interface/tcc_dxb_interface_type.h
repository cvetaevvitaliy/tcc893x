/****************************************************************************

Copyright (C) 2013 Telechips Inc.


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions 
andlimitations under the License.

****************************************************************************/

/****************************************************************************

Revision History

****************************************************************************

****************************************************************************/
#ifndef _TCC_DXB_INTERFACE_TYPE_H_
#define _TCC_DXB_INTERFACE_TYPE_H_

/* 8 bit integer types */
typedef	signed char				INT8;
typedef	unsigned char			UINT8;

/* 16 bit integer types */
typedef	signed short			INT16;
typedef	unsigned short			UINT16;

/* 32 bit integer types */
typedef	int						INT32;
typedef	unsigned int			UINT32;


typedef float					REAL32;

/* 64 bit integer types */
typedef signed long long		INT64;
typedef unsigned long long		UINT64;

/* boolean type */
typedef	int			BOOLEAN;

#ifndef TRUE
#define TRUE 1
#endif


#ifndef FALSE
#define	FALSE 0
#endif

#ifndef NULL
#define  NULL 0
#endif

typedef enum DxB_ERR_CODE
{
	DxB_ERR_OK= 0,
	DxB_ERR_ERROR,
	DxB_ERR_INVALID_PARAM,
	DxB_ERR_NO_RESOURCE,
	DxB_ERR_NO_ALLOC,
	DxB_ERR_NOT_SUPPORTED,
	DxB_ERR_INIT,
	DxB_ERR_SEND,
	DxB_ERR_RECV,
	DxB_ERR_TIMEOUT,
	DxB_ERR_CRC
}DxB_ERR_CODE;

typedef enum DxB_EVENT_ERR_CODE
{
    DxB_EVENT_ERR_OK= 0,
    DxB_EVENT_ERR_ERR,
    DxB_EVENT_ERR_NOFREESPACE,
    DxB_EVENT_ERR_FILEOPENERROR,
    DxB_EVENT_ERR_INVALIDMEMORY,
    DxB_EVENT_ERR_INVALIDOPERATION,
    DxB_EVENT_FAIL_NOFREESPACE,
    DxB_EVENT_FAIL_INVALIDSTORAGE,
    DxB_EVENT_ERR_MAX_SIZE,
    DxB_EVENT_ERR_MAX,
}DxB_EVENT_ERR_CODE;

typedef	enum DxB_STANDARDS
{
   DxB_STANDARD_IPTV = 0,
   DxB_STANDARD_ISDBT,
   DxB_STANDARD_TDMB,
   DxB_STANDARD_DVBT,
   DxB_STARDARD_DVBS,
   DxB_STANDARD_ATSC,
   DxB_STANDARD_MAX
}DxB_STANDARDS;

#endif


