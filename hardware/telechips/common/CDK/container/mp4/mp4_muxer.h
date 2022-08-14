/****************************************************************************
 *   FileName    : mp4_muxer.h
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
#ifndef _MP4_MUXER_H_
#define _MP4_MUXER_H_

#include "../common.h"

typedef common_callback_func_t mp4_mux_callback_func_t; //!< callback functions of libmp4
typedef common_handle_t mp4_mux_handle_t;	//!< handle of libmp4
typedef common_result_t mp4_mux_result_t;	//!< return value of libmp4

//! init parameter of TCC_MP4_MUX() : 128+32+96 bytes
typedef struct mp4_mux_init_t 
{
	mp4_mux_callback_func_t m_sCallbackFunc;	//!< [in] callback functions of libmp4
	void* m_pOutFileName;						//!< [in] out file name
	int m_iFormatMode;							//!< not used: you have to set zero
	int m_iStreamType;							//!< ( PACKET_VIDEO | PACKET_AUDIO )
	int (*m_pfTCAppendHandleByCluster) (unsigned int uiHandle1, unsigned int uiHandle12, unsigned int uiMode); 
												//!< for speed up(TCC append function)
												//!< System group function: TC_STAT	TC_AppendhandleByCluster(TC_U32 uIndex1, TC_U32 uIndex2, TC_U32 uOption);
	int m_Reserved[8-4];

	//! 96 Bytes
	union 
	{
		int m_ReservedExtra[24];
		struct 
		{
			unsigned char* m_pWriteCache;	//!< [in] allocated write cache(buffer) pointer( for mp4 file, video and audio tmp file )
			int m_iWriteCacheSize;			//!< [in] cache(buffer) size for file writing
			int m_iTmpFileVideoCacheSize;	//!< [in] cache(buffer) size for video temp file writing
			int m_iTmpFileAudioCacheSize;	//!< [in] cache(buffer) size for audio temp file writing

			//! for temp file writing(mp4 info)
			void* m_pTmpFilePath;			//!< [in] pointer of temp file path name( "C:/tmp_folder" )
											//!< NULL-->current folder
			int m_ReservedExtra[24-5];
		} m_sCache;
	} m_uExtra;
	
} mp4_mux_init_t;

//! output information of libmp4
typedef common_mux_info_t mp4_mux_info_t;

//! input parameter of TCC_MP4_MUX() : 48 bytes
typedef struct mp4_mux_input_t 
{
	unsigned char* m_pData;		//!< [in] the pointer to the packet data
	int m_iDataSize;			//!< [in] the size of the packet data
	int m_Dts;					//!< [in] decoding timestamp(msec)
	int m_Pts;					//!< [in] presentation timestamp(msec)
	int m_iFlags;				//!< [in] the flag of the key frame
	int m_iType;				//!< [in] the type of the packet data
	i64_t m_lEstiFileSize;		//!< [out] the size of the estimated whole mp4 file ( when a file is closed after current muxing )
	int m_Reserved[12-8];
} mp4_mux_input_t;


//! close parameter of TCC_MP4_MUX() : 16 bytes
typedef struct mp4_mux_close_t 
{
	// Free memory region
	//! Muxer will use this memory for file writing instead of m_pWriteCache
	unsigned char* m_pWriteExtensionCache;	//!< [in] extension cache(buffer) for file writing
	int m_iWriteExtensionCacheSize;			//!< [in] length of extension cache(buffer)
											//!< this memory must be greater than m_iWriteCacheSize
											//!< if m_pWriteExtensionCache is zero, m_pWriteCache will be used.
											//!< if m_pfTCAppendHandleByCluster exists, m_pWriteExtensionCache and m_iWriteExtensionCacheSize is ignored.
	int m_Reserved[4-2];
} mp4_mux_close_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP4_MUX		: main api function of mp4 muxer
 * \param
 *		[in]Op			: muxer operation 
 * \param
 *		[in,out]pHandle	: handle of mp4 muxer 
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP4_MUX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
mp4_mux_result_t 
TCC_MP4_MUX( int Op, mp4_mux_handle_t* pMp4Handle, void* pParam1, void* pParam2 );

#endif//_MP4_MUXER_H_
