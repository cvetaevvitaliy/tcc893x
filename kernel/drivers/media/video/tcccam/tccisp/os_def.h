/****************************************************************************
One line to give the program's name and a brief idea of what it does.
Copyright (C) 2013 Telechips Inc.

This program is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
Suite 330, Boston, MA 02111-1307 USA
****************************************************************************/


#ifndef _OS_DEF_H
#define _OS_DEF_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
typedef unsigned char       UINT8;  //!<  8 bits wide unsigned integer
typedef unsigned short      UINT16; //!< 16 bits wide unsigned integer
typedef unsigned long       UINT32; //!< 32 bits wide unsigned integer
typedef unsigned long long  UINT64; //!< 64 bits wide unsigned integer
typedef signed char         INT8;   //!<  8 bits wide signed integer
typedef signed short        INT16;  //!< 16 bits wide signed integer
typedef signed long         INT32;  //!< 32 bits wide signed integer
typedef char                CHAR;   //!<  8 bits wide char; signedness depends on compiler settings
typedef int                 BOOL;   //!< boolean using platform base type
typedef float               FLOAT;  //!< float
typedef double              DOUBLE; //!< double
*/
   
#ifndef TRUE
#define TRUE   1                //!< Define TRUE value if undefined so far.
#endif
#ifndef FALSE
#define FALSE  0                //!< Define FALSE value if undefined so far.
#endif

#ifndef NULL
#define NULL  (void *) 0        //!< Define NULL pointer value if undefined so far.
#endif


#ifndef UNUSED_PARAM
#define UNUSED_PARAM(x) (void)(x) //!< Mark parameter as being unused to avoid compiler warnings.
#endif

#ifndef UNUSED_VAR
#define UNUSED_VAR(x) (void)(x)	//!< Mark variable as being unused to avoid compiler warnings.
#endif

//to avoid Lint warnings, use it within const context
#ifndef UNUSED_PARAM1
#define UNUSED_PARAM1(x) if((x) != (x)) {ASSERT (FALSE) } //!< Mark parameter as being unused to avoid Lint warnings.
#endif

// documentation keywords for pointer arguments, to tell the direction of the passing                     
#ifndef OUT
    #define OUT                 //!< pointer content is expected to be filled by called function                     
#endif
#ifndef IN
    #define IN                  //!< pointer content contains parameters from the caller                     
#endif
#ifndef INOUT   
    #define INOUT               //!< pointer content is expected to be read and changed                     
#endif

// some helpfull stuff
#define __STRINGIFY(x) #x
#define TOSTRING(x) __STRINGIFY(x)

#ifndef MIN
    #define MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifdef __cplusplus
}
#endif

#endif //_OS_DEF_H
