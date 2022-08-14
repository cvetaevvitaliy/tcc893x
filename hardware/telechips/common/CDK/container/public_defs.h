/* public_defs.h
 *
 * Description: See below.
 */

/*****************************************************************************
* Copyright (c) 2007-2012 AuthenTec B.V. All Rights Reserved.
*
* This confidential and proprietary software may be used only as authorized
* by a licensing agreement from AuthenTec.
*
* The entire notice above must be reproduced on all authorized copies that
* may only be made to the extent permitted by a licensing agreement from
* AuthenTec.
*
* For more information or support, please go to our online support system at
* https://oemsupport.authentec.com. In case you do not have an account for
* this system, please send an e-mail to EmbeddedHW-Support@authentec.com.
*****************************************************************************/

/*
  This header provides public definitions required by interface modules
  The definitions are considered public, because they are required by the
  public API headers.

  - C99 exact-width integer types, e.g. uint32_t
  - C99 Boolean

  This particular header should is used for building on basic
  development platforms that are Linux and Win32.
*/

#ifndef INCLUDE_GUARD_PUBLIC_DEFS_H
#define INCLUDE_GUARD_PUBLIC_DEFS_H

#if defined(WIN32)

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef uint32_t size_t;
typedef uint32_t uintptr_t;

typedef uint8_t bool;

#define true 1
#define false 0

#define INT8_MIN (-128)
#define INT8_MAX  (127)

#define INT16_MIN (-32768)
#define INT16_MAX (32767)

#define INT32_MIN (-2147483647-1)
#define INT32_MAX (2147483647)

#define INT64_MIN  (-9223372036854775808LL)
#define INT64_MAX  (9223372036854775807LL)
#define UINT64_MAX (18446744073709551615ULL)

#define UINT8_MAX  (255)

#define UINT16_MAX (65535)

#define UINT32_MAX (4294967295)

#define restrict

#elif (defined(__GNUC__) || defined(__CC_ARM))

#include <stdint.h>        // exact width integers
#include <stdbool.h>       // Boolean
#include <stddef.h>        // offsetof, size_t

#else
#error Unsupported platform
#endif

#endif /* Include guard */

/* end of file public_defs.h */
