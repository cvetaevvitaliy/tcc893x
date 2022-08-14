/**
 * Copyright (C) 2012, Telechips Limited
 * by ZzaU(B070371)
 */

#ifndef __TCC_COMMON_IF__H__
#define __TCC_COMMON_IF__H__

typedef enum CODEC_FORMAT {
	CODEC_FORMAT_H263 = 0,
	CODEC_FORMAT_MPEG4,
	CODEC_FORMAT_H264,	
	CODEC_FORMAT_RV,     	// Decoder only
	CODEC_FORMAT_MPEG2,     // Decoder only 
	CODEC_FORMAT_DIV3,      // Decoder only
	CODEC_FORMAT_VC1,       // Decoder only
	CODEC_FORMAT_MJPG,      // Decoder only
	CODEC_FORMAT_ILLEGAL
} tCODEC_FORMAT;

typedef enum pic_type {
	TYPE_I_FRAME	= 0,
	TYPE_P_FRAME,	
	TYPE_B_FRAME,	
	TYPE_ILLEGAL	
} tPIC_TYPE;

typedef enum frame_buf_format {

/****************/
/*YYYY...   	*/
/*YYYY...   	*/
/****************/
/*UUUU..*/
/********/
/*VVVV..*/
/********/
	FRAME_BUF_FORMAT_YUV420P	= 0,	/* YUV420 planar : Y field + U field + V field */

/****************/
/*YYYY...   	*/
/*YYYY...   	*/
/****************/
/*VVVV..*/
/********/
/*UUUU..*/
/****************/
	FRAME_BUF_FORMAT_YVU420P,			/* YVU420 planar : Y field + V field + U field */

/****************/
/*YYYY...   	*/
/*YYYY...   	*/
/****************/
/*UVUV..*/
/*UVUV..*/
/********/
	FRAME_BUF_FORMAT_YUV420I,			/* YUV420 interleaved : Y field + UV field. */

	FRAME_BUF_FORMAT_YUV422P,			/* only for Output format in MJPEG case. but, can't be assigned this format forcingly on external request. */

	FRAME_BUF_FORMAT_ILLEGAL	
} tFRAME_BUF_FORMAT;

#endif

