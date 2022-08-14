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
/******************************************************************************
 file name : TCCXXX_PNG_DEC.h 
 version   : V2.00
******************************************************************************/

#ifndef __TCCXXX_PNG_DEC_H__
#define __TCCXXX_PNG_DEC_H__

#include "TCCXXX_IMAGE_CUSTOM_OUTPUT_SET_ISDBT.h"

#define PD_DEC_INIT						0
#define PD_DEC_DECODE					1

#define PD_RETURN_INIT_FAIL				-1
#define PD_RETURN_INIT_DONE				0
#define PD_RETURN_DECODE_FAIL			-1
#define PD_RETURN_DECODE_DONE			0
#define PD_RETURN_DECODE_PROCESSING		1

#define PD_ERROR_CHK_NONE				0
#define PD_ERROR_CHK_ALL				1
#define PD_ERROR_CHK_CRC				2
#define PD_ERROR_CHK_ADLER				3

#define PD_RESOURCE_LEVEL_NONE			0		//Needs Less Resource
#define PD_RESOURCE_LEVEL_LOW			1
#define PD_RESOURCE_LEVEL_MID			2
#define PD_RESOURCE_LEVEL_HIGH			3
#define PD_RESOURCE_LEVEL_ALL			4		//Needs More Resource

//Use of Alpha Blending
#define PD_ALPHA_DISABLE				0
#define PD_ALPHA_AVAILABLE				1

#define PD_INSTANCE_MEM_SIZE			(49412)	// the size of instance buffer


typedef struct {
	int			(*read_func)	(void *ptr, int size, int nmemb, void *datasource);	
										/*[IN] PD_CALLBACKS struct 
											   which indicates desired custom file manipulation routines
										*/
}PD_CALLBACKS;


typedef struct {
	int				lcd_width;			//[IN] Width of lcd_frame_buffer
	int				lcd_height;			//[IN] Height of lcd_frame_buffer
	unsigned int	image_width;		//[OUT] width of original image
	unsigned int	image_height;		//[OUT] height of original image
	unsigned int	alpha_available;	//[OUT] If this field is set, use of alpha data is available
	unsigned int	heap_size;			//[OUT] the size of required heap memory
	void			*pInstanceBuf;		//[IN] A pointer to instance buffer (refer to PD_INSTANCE_MEM_SIZE)
	void			*datasource;		//[IN] PD_CALLBACKS struct which indicates desired custom file manipulation routines

	unsigned int	iTotFileSize;		//[IN] size in bytes of the input file	(V1.65)
	unsigned int	pixel_depth;		//[OUT] bits per pixel
	unsigned int	iOption;			//Reserved
	unsigned int	iReserved;
}PD_INIT;


typedef struct {
	unsigned char 	*Heap_Memory;		//[IN] Heap Memory for Decoding
	int				ERROR_DET_MODE;		//[IN] Use of CRC and Adler
	int				RESOURCE_OCCUPATION;//[IN] Degree of Resource Occupation
	int				USE_ALPHA_DATA;		//[IN] Set this field if alpha data is required.
	int				MODIFY_IMAGE_POS;	//[IN] If set, modification of output position is available.
	unsigned int	IMAGE_POS_X;		//[IN] Distance from the left of LCD
	unsigned int	IMAGE_POS_Y;		//[IN] Distance from the top of LCD
	void			(*write_func)	(IM_PIX_INFO out_info);	//[IN] A function pointer to output format function
}PD_CUSTOM_DECODE;


/* function definition */

extern int 
TCCXXX_PNG_Decode(
	int iOp,			/* 	Operation
							0 : PD_DEC_INIT
							1 : PD_DEC_DECODE
						*/
	void * pParam1,		/* 
							PD_INIT type when PD_DEC_INIT operation
							PD_CUSTOM_DECODE type when PD_DEC_DECODE operation
						*/
	void * pParam2,		/*
							PD_CALLBACKS type when PD_DEC_INIT operation
							NULL when PD_DEC_DECODE operation
						*/
	int iReserved		/* 
							0(Reserved)
						*/
);


#endif //__TCCXXX_PNG_DEC_H__
