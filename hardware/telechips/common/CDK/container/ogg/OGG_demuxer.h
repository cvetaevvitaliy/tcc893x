/****************************************************************************
 *   FileName    : OGG_demuxer.h
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
#ifndef _OGG_DEMUXER_H_
#define _OGG_DEMUXER_H_

#include "../av_common_dmx.h"
#define OGG_EXTRA_SIZE	(1024*360)
#define OGG_PACKET_SIZE	(1024*1024*1)

typedef av_memory_func_t		ogg_dmx_memory_func_t;		//!< callback functions of TCC_OGG_DMX()
typedef av_file_func_t			ogg_dmx_file_func_t;		//!< callback functions of TCC_OGG_DMX()
typedef av_handle_t				ogg_dmx_handle_t;			//!< handle of TCC_OGG_DMX()
typedef av_result_t				ogg_dmx_result_t;			//!< return value of TCC_OGG_DMX()

typedef struct ogg_dmx_file_info_t
{
	/* common */
	char* m_pszOpenFileName;	//!< open file name
	char* m_pszCopyright;		//!< copyright
	char* m_pszCreationTime;	//!< creation time
	int m_iRunningtime;			//!< runing time * 1000
	U64 m_lFileSize;			//!< total file size
		
	unsigned int m_uiTotalBitrate;
	unsigned int m_uiSeekalble;
	int m_Reserved[32-8];
} ogg_dmx_file_info_t;

typedef struct ogg_dmx_video_info_t
{
	/* common */
	int m_iWidth;				//!< width
	int m_iHeight;				//!< height
	int m_iFrameRate;			//!< framerate * 1000;
	int m_iFourCC;				//!< fourcc

	/* extra info (common) */
	char* m_pszCodecName;		//!< codec name
	char* m_pszCodecVendorName;	//!< codec vendor
	unsigned char* m_pExtraData;//!< extra data
	int m_iExtraDataLen;		//!< extra data length
	
	unsigned int m_uiBitrate;
	int m_Reserved[32-9];			
} ogg_dmx_video_info_t;

typedef struct ogg_dmx_audio_info_t
{
	/* common */
	int m_iTotalNumber;			//!< total audio number
	int m_iSamplePerSec;		//!< samples/sec
	int m_iBitsPerSample;		//!< bits/sample
	int m_iChannels;			//!< channels
	int m_iFormatId;			//!< format id

	/* extra info (common) */
	char* m_pszCodecName;		//!< codec name
	char* m_pszCodecVendorName;	//!< codec vendor
	unsigned char* m_pExtraData;//!< extra data
	int m_iExtraDataLen;		//!< extra data length

	unsigned int m_uiBitrate;		
	int m_Reserved[32-10];			
} ogg_dmx_audio_info_t;

//! Structure for initialization parameter of TCC_OGG_DMX()
typedef struct ogg_dmx_init_t 
{
	ogg_dmx_memory_func_t m_sCallbackFuncMem;	//!< [in] callback functions of ogg dmx
	ogg_dmx_file_func_t	  m_sCallbackFuncFile;	//!< [in] callback functions of ogg dmx
	void* m_pOpenFileName;						//!< [in] open file name( when Op is INIT ) or handle of opened file( when Op is INIT_FROM_FILE_HANDLE )
	unsigned int	m_uiOpenOption;
	unsigned int	m_uiFileCacheSize;
	int m_Reserved[8-4];

	//! extra info: 96 Bytes
	union 
	{
		int m_ReservedExtra[24];		
	} m_uExtra;

} ogg_dmx_init_t;

typedef struct ogg_dmx_info_t
{
	//! file information : 128 Bytes
	ogg_dmx_file_info_t m_sFileInfo;

	//! video information : 128 Bytes
	ogg_dmx_video_info_t m_sVideoInfo;

	//! audio information : 128 Bytes
 	ogg_dmx_audio_info_t m_sAudioInfo;

} ogg_dmx_info_t;

//! Select stream - input
typedef struct ogg_dmx_sel_stream_t
{
	long					 m_lSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	long					 m_lVideoID;		//!< [in] Video ID
	long					 m_lAudioID;		//!< [in] Audio ID
} ogg_dmx_sel_stream_t;

//! Select stream - output
typedef struct ogg_dmx_sel_info_t
{
	ogg_dmx_video_info_t	*m_pstVideoInfo;	//!< pointer to video information
	ogg_dmx_audio_info_t	*m_pstAudioInfo;	//!< pointer to audio information
} ogg_dmx_sel_info_t;

//typedef common_dmx_input_t ogg_dmx_input_t;		//! input parameter of TCC_OGG_DMX()
//typedef common_dmx_output_t ogg_dmx_output_t;		//! output parameter of TCC_OGG_DMX()
//typedef common_dmx_seek_t ogg_dmx_seek_t;			//! seek parameter of TCC_OGG_DMX()

typedef av_dmx_getstream_t		ogg_dmx_input_t;	//! input parameter of TCC_OGG_DMX()
typedef av_dmx_outstream_t		ogg_dmx_output_t;	//! output parameter of TCC_OGG_DMX()
typedef av_dmx_seek_t			ogg_dmx_seek_t;		//! seek parameter of TCC_OGG_DMX()

#define OGG_SEEKABLE_NO				0x0000		// Sequential Read (default)
#define OGG_SEEKABLE_YES			0x0001		// Video/Audio Selective Read

#define OGG_OPENOPT_SEQUENTIAL		0x0001		// Sequential Read (default)
#define OGG_OPENOPT_SELECTIVE		0x0002		// Video/Audio Selective Read

#define OGG_SEEKMODE_RELATIVE_TIME		0x0
#define OGG_SEEKMODE_ABSOLUTE_TIME		0x1
#define OGG_SEEKMODE_DIR_FORWARD		0x0
#define OGG_SEEKMODE_DIR_BACKWARD		0x2

#define OGG_PACKETTYPE_VIDEO		0x1
#define OGG_PACKETTYPE_AUDIO		0x2


#define OGG_DMX_SUCCESS				0
#define OGG_DMX_NOMORE_PAGE			-1
#define OGG_DMX_STREAM_READ_ERROR	-2
#define OGG_DMX_ERROR				-4
#define OGG_DMX_FILE_OPEN_ERROR		-5
#define OGG_DMX_NULL_INSTANCE		-6
#define OGG_DMX_NOT_FOUND_SYNCH		-7
#define OGG_DMX_SMALL_PACKET_SIZE	-8
#define OGG_DMX_INVALID_PACKET_TYPE -9
#define OGG_DMX_NOT_SUPPORT			-11
#define OGG_DMX_SEEK_OUTOFRANGE		-12

/*!
 ***********************************************************************
 * \brief
 *		TCC_OGG_DMX		: main api function of OGG demuxer
 * \param
 *		[in]Op			: demuxer operation 
 * \param
 *		[in,out]pHandle	: handle of OGG demuxer
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_OGG_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
ogg_dmx_result_t 
TCC_OGG_DMX( unsigned long Op, ogg_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );

#endif//_OGG_DEMUXER_H_
