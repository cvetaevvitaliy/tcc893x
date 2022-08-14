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

#ifndef _VENUS_PROJECT_H_
#define _VENUS_PROJECT_H_


#define VA_PROJECT_AIT705DEMOKIT		0
#define VA_PROJECT_SPEACY				1

#define AIT818_TEST_API					0
#define AIT828_TEST_API					0
#define AIT838_TEST_API					0
#define AIT848_TEST_API					1

/*************************************************************************************************/
/* Venus : Headers                                                                               */
/*************************************************************************************************/
#if VA_PROJECT_AIT705DEMOKIT

	//#include <stdio.h>
	#include "ait_utility.h"
	#include <time.h>

#elif VA_PROJECT_SPEACY

	


#endif


/*************************************************************************************************/
/* Venus : System Clock                                                                          */
/*************************************************************************************************/
#if VA_PROJECT_AIT705DEMOKIT

	#define VA_PLL_M				40
	#define VA_PLL_N				4

#elif VA_PROJECT_SPEACY


#endif

/*************************************************************************************************/
/* Venus : Typedefinition                                                                        */
/*************************************************************************************************/
#if VA_PROJECT_AIT705DEMOKIT

	typedef unsigned char			uint8;
	typedef unsigned short			uint16;
	typedef unsigned int			uint32;
	typedef char					int8;
	typedef short					int16;
	typedef int						int32;

#elif VA_PROJECT_SPEACY
typedef unsigned char           uint8;
typedef unsigned short          uint16;
typedef unsigned int            uint32;
typedef char                    int8;
typedef short                   int16;
typedef int                     int32;


#endif

/*************************************************************************************************/
/* Venus : Message                                                                               */
/*************************************************************************************************/
#if VA_PROJECT_AIT705DEMOKIT

	#define VA_MSG(...)				printf(""          ## __VA_ARGS__)
	#define VA_INFO(...)			printf("  INFO: "  ## __VA_ARGS__)
	#define VA_ERR(...)				printf("  ERROR: " ## __VA_ARGS__)
	#define VA_HIGH(...)			printf("-##- "     ## __VA_ARGS__)

#elif VA_PROJECT_SPEACY

	#define MM_CAMERA				(0)
	#define VA_MSG			
	#define VA_INFO			
	#define VA_ERR			
	#define VA_HIGH			


#endif



#endif // _VENUS_PROJECT_H_
