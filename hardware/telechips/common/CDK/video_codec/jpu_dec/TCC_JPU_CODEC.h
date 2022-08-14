/****************************************************************************
 *   FileName    : TCC_JPU_CODEC.h
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
#ifndef _TCC8010_JPU_CODEC_H_
#define _TCC8010_JPU_CODEC_H_

#define MAX_NUM_INSTANCE		1

#define PA 0 // physical address
#define VA 1 // virtual  address

typedef int codec_handle_t; 		//!< handle
typedef int codec_result_t; 		//!< return value
typedef unsigned int codec_addr_t;	//!< address

#define COMP_Y 0
#define COMP_U 1
#define COMP_V 2

#define	YUV_FORMAT_420					0
#define	YUV_FORMAT_422					1
#define	YUV_FORMAT_224					2
#define	YUV_FORMAT_444					3
#define	YUV_FORMAT_400					4

// Encoder Op Code
#define JPU_ENC_INIT				0x00	//!< init
#define JPU_ENC_REG_FRAME_BUFFER	0x01	//!< register frame buffer
#define JPU_ENC_ENCODE				0x12	//!< encode
#define JPU_ENC_CLOSE				0x20	//!< close

// Decoder Op Code
#define JPU_DEC_INIT				0x00	//!< init
#define JPU_DEC_REG_FRAME_BUFFER	0x03	//!< register frame buffer
#define JPU_DEC_DECODE				0x10	//!< decode
#define JPU_DEC_CLOSE				0x20	//!< close
#define JPU_CODEC_GET_VERSION		0x3000


#define RETCODE_ERR_MIN_RESOLUTION			101
#define RETCODE_ERR_MAX_RESOLUTION			102

#define JPG_RET_SUCCESS					0
#define JPG_RET_FAILURE					1
#define JPG_RET_BIT_EMPTY				2
#define JPG_RET_EOS						3
#define JPG_RET_INVALID_HANDLE			4
#define JPG_RET_INVALID_PARAM			5
#define JPG_RET_INVALID_COMMAND			6
#define JPG_RET_ROTATOR_OUTPUT_NOT_SET	7
#define JPG_RET_ROTATOR_STRIDE_NOT_SET	8
#define JPG_RET_FRAME_NOT_COMPLETE		9
#define JPG_RET_INVALID_FRAME_BUFFER	10
#define JPG_RET_INSUFFICIENT_FRAME_BUFFERS	11
#define JPG_RET_INVALID_STRIDE			12
#define JPG_RET_WRONG_CALL_SEQUENCE		13
#define JPG_RET_CALLED_BEFORE			14
#define JPG_RET_NOT_INITIALIZED			15
#define JPG_RET_CODEC_EXIT				16
#define JPG_RET_SPEC_OUT				17
#define	JPG_RET_INSUFFICIENT            18 
#define JPG_ERROR_NOTBASELINE			19
#define JPG_ERROR_MAXSIZE				20
#define JPG_ERROR_MINSIZE				21

#define ALIGNED_BUFF(buf, mul) ( ( (unsigned int)buf + (mul-1) ) & ~(mul-1) )

//-----------------------------------------------------
// data structure to get information necessary to 
// start decoding from the decoder (this is an output parameter)
//-----------------------------------------------------

typedef struct jpu_dec_initial_info_t     
{
	int m_iPicWidth;				//!< {(PicX+15)/16} * 16  (this width  will be used while allocating decoder frame buffers. picWidth  is a multiple of 16)
	int m_iPicHeight;				//!< {(PicY+15)/16} * 16  (this height will be used while allocating decoder frame buffers. picHeight is a multiple of 16)
	int m_iJpg_sourceFormat;		//!< the minimum number of frame buffers that are required for decoding. application must allocate at least this number of frame buffers.
	int m_iJpg_MinFrameBufferSize[4];	//!< minimum frame buffer size
										//!< 0 ( Original Size )
										//!< 1 ( 1/2 Scaling Down )
										//!< 2 ( 1/4 Scaling Down )
										//!< 3 ( 1/8 Scaling Down )
	int m_Reserved[1];
	int m_error_reason;
} jpu_dec_initial_info_t;


//! data structure for initializing Video unit
typedef struct jpu_dec_init_t 
{
	codec_addr_t m_RegBaseVirtualAddr;	//!< virtual address BIT_BASE
	codec_addr_t m_BitstreamBufAddr[2];	//!< bitstream buf address : multiple of 4
	int m_iBitstreamBufSize;			//!< bitstream buf size	   : multiple of 1024

//	int m_iBitstreamSize;
	int m_iRot_angle; 
	int m_iRot_enalbe; 
	int m_iMirror_enable; 
	int m_iMirrordir;
	
//	int m_iBitstreamFileSize; 
	int m_bCbCrInterleaveMode;

	//! Callback Func
	void* (*m_Memcpy ) ( void*, const void*, unsigned int );	//!< memcpy
	void  (*m_Memset ) ( void*, int, unsigned int );			//!< memset
	int   (*m_Interrupt ) ( void );								//!< hw interrupt (return value is always 0)
/*
	void* (*m_pfnFopen)(const char*, const char* );						//!< fopen
	unsigned int (*m_pfnFread)( void*, unsigned int, unsigned int, void* );		//!< fread
	int (*m_pfnFseek)( void*, long, int );//!< fseek
	int (*m_pfnFclose)( void* );		//!< fclose
    const char* m_filename;
*/	
} jpu_dec_init_t;

typedef struct jpu_dec_input_t 
{
	unsigned int m_BitstreamDataAddr[2];//!< bitstream data address
	int m_iBitstreamDataSize;//!< bitstream data size
	int m_iLooptogle;
	codec_addr_t m_FrameBufferStartAddr[2];
} jpu_dec_input_t;

typedef struct jpu_dec_buffer_t
{
	codec_addr_t m_FrameBufferStartAddr[2];	//!< physical[0] and virtual[1] address of a frame buffer of the decoder.
	int m_iJPGScaleRatio;		//!< JPEG Scaling Ratio
								//!< 0 ( Original Size )
								//!< 1 ( 1/2 Scaling Down )
								//!< 2 ( 1/4 Scaling Down )
								//!< 3 ( 1/8 Scaling Down )
	int m_Reserved[1];
} jpu_dec_buffer_t;

//-----------------------------------------------------
// data structure to get resulting information from 
// JPU after decoding a frame
//-----------------------------------------------------

typedef struct jpu_dec_output_info_t
{
	int m_iHeight;					//!< Height of input bitstream. In some cases, this value can be different from the height of previous frames.
	int m_iWidth;					//!< Width of input bitstream. In some cases, this value can be different from the height of previous frames.
	int m_iDecodingStatus;
	int m_iConsumedBytes;			//!< consumed bytes
	int m_iNumOfErrMBs;				//!< number of error macroblocks
	unsigned int m_Reserved[3];		//! Reserved. 

} jpu_dec_output_info_t;

typedef struct jpu_dec_output_t 
{
	jpu_dec_output_info_t m_DecOutInfo;
	unsigned char* m_pCurrOut[2][3]; //! physical[0] and virtual[1] current  address of Y, Cb, Cr component
	//holic 030614 decode test
	

} jpu_dec_output_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_JPU_DEC		: main api function of libjpudec
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: libvpudec's handle
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or info parameter
 * \return
 *		If successful, TCC_JPU_DEC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
codec_result_t
TCC_JPU_DEC( int Op, codec_handle_t* pHandle, void* pParam1, void* pParam2 );


//------------------------------------------------------------------------------
// encode struct and definition
//------------------------------------------------------------------------------

typedef struct jpu_enc_init_t 
{
	codec_addr_t m_RegBaseVirtualAddr;

	//! Encoding Info
	int m_iMjpg_sourceFormat;				//!< input YUV format for mjpeg encoding
	int m_iPicWidth;					//!< multiple of 16
	int m_iPicHeight;					//!< multiple of 16
	int m_iRestartInterval;
	int m_iRotMode;
	int m_iEncQuality;					//!< jpeg encoding quality

	int m_bCbCrInterleaveMode;	//!< 0 (chroma separate mode   : CbCr data is written in separate frame memories)
										//!< 1 (chroma interleave mode : CbCr data is interleaved in chroma memory)

	codec_addr_t m_BitstreamBufferAddr[2]; //!< physical address : multiple of 4, virtual address : multiple of 4

	int m_iBitstreamBufferSize;			//!< multiple of 1024
	unsigned int m_uiEncOptFlags;

	//! Callback Func
	void* (*m_Memcpy ) ( void*, const void*, unsigned int );	//!< memcpy
	void  (*m_Memset ) ( void*, int, unsigned int );			//!< memset
	int   (*m_Interrupt ) ( void );								//!< hw interrupt (return value is always 0)
} jpu_enc_init_t;

typedef struct jpu_enc_input_t 
{
	//04 25 holic
	codec_addr_t m_PicYAddr;
	codec_addr_t m_PicCbAddr;
	codec_addr_t m_PicCrAddr;

	codec_addr_t m_BitstreamBufferAddr[2]; //!< physical address
	int m_iBitstreamBufferSize;
} jpu_enc_input_t;

typedef struct jpu_enc_output_t 
{
	codec_addr_t m_BitstreamOut[2];
	int m_iBitstreamOutSize;
	int m_iHeaderOutSize;
} jpu_enc_output_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_JPU_ENC		: main api function of libjpudec
 * \param
 *		[in]Op			: encoder operation 
 * \param
 *		[in,out]pHandle	: libvpudec's handle
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or info parameter
 * \return
 *		If successful, TCC_JPU_ENC returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
codec_result_t
TCC_JPU_ENC( int Op, codec_handle_t* pHandle, void* pParam1, void* pParam2 );

#endif//_TCC8010_JPU_CODEC_H_