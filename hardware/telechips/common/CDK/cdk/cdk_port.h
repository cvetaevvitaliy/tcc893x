/****************************************************************************
 *   FileName    : cdk_port.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _CDK_PORT_H_
#define _CDK_PORT_H_

#include "cdk_pre_define.h"
#ifndef ENABLE_TERMINAL_INTERFACE
#ifndef _TM_DEFINED
#ifndef _TIME_H_
struct tm {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
};
#endif
#define _TM_DEFINED
#endif
#endif

#ifndef NULL
#	define NULL	0
#endif

/************************************************************************

	typedef
				
************************************************************************/
#define CDK_TYPES
#if defined(ARM_WINCE) // ARM WinCE
	typedef unsigned __int64  uint64_t;
	typedef __int64 int64_t;
#else
#ifndef _STDINT_H
	typedef unsigned long long uint64_t;
	typedef signed long long int64_t;
#endif
#endif
typedef int cdk_handle_t;
typedef int cdk_result_t;
typedef int cdk_bool_t;

/************************************************************************

	defines

************************************************************************/
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#endif//_CDK_PORT_H_
