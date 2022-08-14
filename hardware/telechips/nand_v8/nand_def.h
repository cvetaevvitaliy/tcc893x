/****************************************************************************
 *   FileName    : nand_def.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-
distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall 
constitute any express or implied warranty of any kind, including without limitation, any warranty 
of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright 
or other third party intellectual property right. No warranty is made, express or implied, 
regarding the information's accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of 
or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************/
#ifndef _NAND_COMMON_H
#define _NAND_COMMON_H

#define NAND_DRIVER_VERSION					163

// In Android Platform 
#if defined(CONFIG_CHIP_TCC8925S)
#ifndef TCC8925S
#define TCC8925S
#endif
#endif

#if defined(CONFIG_CHIP_TCC8930)
#ifndef TCC8930
#define TCC8930
#endif
#endif

#if defined(CONFIG_CHIP_TCC8935) || defined(CONFIG_CHIP_TCC8935S) || defined(CONFIG_CHIP_TCC8937S)
#ifndef TCC8935
#define TCC8935
#endif
#endif

#if defined(CONFIG_CHIP_TCC8933) || defined(CONFIG_CHIP_TCC8933S)
#ifndef TCC8933
#define TCC8933
#endif
#endif
//

#if defined(TCC8925S)
#undef TCC892X
#endif

#if defined(TCC8925S) || defined(TCC893X)
#define NFC_VER_300
#elif defined(TCC801X) || defined(TCC88XX) || defined(TCC892X) || defined(TCC93XX) || defined(TCC370X)
#define NFC_VER_200
#elif defined(TCC800X)
#define NFC_VER_50
#endif

#if defined(NKUSE) || defined(__KERNEL__)
#define NAND_FOR_KERNEL
#endif

#if !defined(_LINUX_) && !defined(_WINCE_)
#define _NU_
#define __FUNCTION__ __func__
#endif

#if defined(_LINUX_)
#if defined(NAND_FOR_KERNEL)
#define ND_TRACE(fmt, ...)			printk("[NAND\t\t] " fmt, ##__VA_ARGS__)
#define _ND_TRACE(fmt, ...)			printk(fmt, ##__VA_ARGS__)
#define ND_ERROR(fmt, ...)			printk("[NAND ERROR %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ND_DEBUG_(fmt, ...)			printk("[NAND %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define n_printk(f, a...)
#define _n_printk(a...)
#else // NAND_FOR_KERNEL
#if defined(ANDROID_BOOT)
#define ND_TRACE(fmt, ...)			dprintf(INFO, "[NAND\t\t] " fmt, ##__VA_ARGS__)
#define _ND_TRACE(fmt, ...)			dprintf(INFO, fmt, ##__VA_ARGS__)
#define ND_ERROR(fmt, ...)			dprintf(INFO, "[NAND ERROR %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ND_DEBUG_(fmt, ...)			dprintf(INFO, "[NAND %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ND_TRACE(fmt, ...)			printf("[NAND\t\t] " fmt, ##__VA_ARGS__)
#define _ND_TRACE(fmt, ...)			printf(fmt, ##__VA_ARGS__)
#define ND_ERROR(fmt, ...)			printf("[NAND ERROR %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ND_DEBUG_(fmt, ...)			printf("[NAND %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif // NAND_FOR_KERNEL

#elif defined(_WINCE_)
#if defined(NAND_FOR_KERNEL)
#include <windows.h>
#define ND_TRACE(fmt, ...)			RETAILMSG( 1, (L"[NAND\t\t] " L##fmt,  ##__VA_ARGS__) )
#define _ND_TRACE(fmt, ...)			RETAILMSG( 1, (L##fmt,  ##__VA_ARGS__) )
//#define ND_TRACE(...)				RETAILMSG( 1, (L##__VA_ARGS__) )
#define ND_ERROR(fmt, ...)			RETAILMSG( 1, (L"[NAND ERROR %s:%d] " L##fmt, _T(__FUNCTION__), __LINE__, ##__VA_ARGS__) )
#define ND_DEBUG_(fmt, ...)			RETAILMSG( 1, (L"[NAND %s:%d] " L##fmt, _T(__FUNCTION__), __LINE__, ##__VA_ARGS__) )
#else // NAND_FOR_KERNEL
extern void B_RETAILMSG( const char *sz, ... );

#define ND_TRACE(fmt, ...)			B_RETAILMSG("[NAND\t\t] " fmt, ##__VA_ARGS__)
#define _ND_TRACE(fmt, ...)			B_RETAILMSG(fmt, ##__VA_ARGS__)
#define ND_ERROR(fmt, ...)			B_RETAILMSG("[NAND ERROR %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ND_DEBUG_(fmt, ...)			B_RETAILMSG("[NAND %s:%d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif // NAND_FOR_KERNEL

#else // NU
#if defined(TCC801X)
extern void TC_ComAPI_LOGPrintf(unsigned int level, char *format, ...);
#define ND_TRACE(...)				TC_ComAPI_LOGPrintf(999, __VA_ARGS__)
#define _ND_TRACE(...)				TC_ComAPI_LOGPrintf(999, __VA_ARGS__)
#define ND_ERROR(...)				TC_ComAPI_LOGPrintf(999, __VA_ARGS__)	
#else
#define ND_TRACE					TccUIDebug_Print
#define _ND_TRACE					TccUIDebug_Print
#define ND_ERROR					TccUIDebug_Print	
#endif
#endif 

#if !defined(NAND_FOR_KERNEL)
#define tcc_p2v(x) (x)
#endif

//#define NFC_XD_HIGH_16BIT
#ifdef NOR_BOOT_INCLUDE
#define USED_WITH_NOR_BOOT
#endif
//#define ND_DEBUG					ND_DEBUG_
#define ND_DEBUG(...)

#ifndef FALSE
#define FALSE           		0
#endif
#ifndef TRUE
#define TRUE            		1
#endif
#ifndef NULL
#define NULL            		0
#endif

#define PAGE_DWORDSIZE_MAX					(16384/4)
#define PAGE_BYTESIZE_MAX					(PAGE_DWORDSIZE_MAX*4)

#endif //_NAND_COMMON_H

