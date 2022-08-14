/****************************************************************************
 *   FileName    : flv_demuxer.h
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
#ifndef _FLV_DEMUXER_H_
#define _FLV_DEMUXER_H_

#include "../av_common_dmx.h"

/*
 ============================================================
 *
 *	File/Video/Audio/Subtitle Information
 *		- information about video, audio or subtitle into one file.
 *
 ============================================================
*/
//! file information
typedef struct flv_dmx_file_info_t
{
	// common info
	char		    *m_pszFileName;
	unsigned long	 m_ulDuration;
	S64				 m_llFileSize;

	// flv specific
#define	SEEK_NOT_AVAILABLE	0
#define	SEEK_BY_META_INDEX	1
#define	SEEK_BY_TAG_SCAN	2
	unsigned long	 m_ulSeekMode;
} flv_dmx_file_info_t;

//! video stream information
typedef struct flv_dmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// flv specific
	unsigned long	 m_ulLastKeyTimestamp;
} flv_dmx_video_info_t;

//! audio stream information
typedef struct flv_dmx_audio_info_t
{	
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulSamplePerSec;
	unsigned long	 m_ulBitsPerSample;
	unsigned long	 m_ulChannels;
	unsigned long	 m_ulFormatTag;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;
	char			*m_pszLanguage;

	// flv specific
	unsigned long	 m_ulBitRate;
} flv_dmx_audio_info_t;

/*
 ============================================================
 *
 *	Demuxer Initialize
 *
 ============================================================
*/
// FLV Demuxer Specific OP Code
#define FLVDOP_OPEN_MEDIAINFO			 10 //!< demuxer open (initialize)
// flv_dmx_init_t.m_ulOption flags
#define FLVOPT_DEFAULT				0x0000	//!< Sequential Read / Use Internal Buffer
#define FLVOPT_SELECTIVE			0x0001	//!< Video/Audio Selective Read
#define FLVOPT_USERBUFF				0x0002	//!< Use User buffer
#define FLVOPT_PROGRESSIVE_FILE		0x0004	//!< progressive file access
#define FLVOPT_MEMORY_INDEXTABLE	0x0008	//!< read index table (meta data) to heap memory

//! Demuxer Initialize - input
typedef struct flv_dmx_init_t
{
	// common
	av_memory_func_t m_stMemoryFuncs;			//!< [in] callback functions for memory
	av_file_func_t	 m_stFileFuncs;				//!< [in] callback functions for file
	char			*m_pszOpenFileName;			/*!< [in] open file name( when Op is DMXOP_OPEN ) */

	// flv specific
	unsigned long	m_ulOption;
	unsigned long	m_ulFIOBlockSizeNormal;
	unsigned long	m_ulFIOBlockSizeSeek;
} flv_dmx_init_t;

//! Demuxer Initialize - output
typedef struct flv_dmx_info_t
{
	// file info
	flv_dmx_file_info_t	    *m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			 m_ulVideoStreamTotal;		//!< number of video stream
	flv_dmx_video_info_t    *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	flv_dmx_video_info_t    *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	unsigned long			 m_ulAudioStreamTotal;		//!< number of audio stream
	flv_dmx_audio_info_t    *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	flv_dmx_audio_info_t    *m_pstDefaultAudioInfo;		//!< pointer to default audio information

} flv_dmx_info_t;

/*
 ============================================================
 *
 *	Demuxer Running..
 *		- flv has no a specific data
 *
 ============================================================
*/
#if 0
typedef av_dmx_getstream_t	flv_dmx_getstream_t;
typedef av_dmx_outstream_t	flv_dmx_outstream_t;
#else
typedef struct flv_dmx_getstream_t
{
	unsigned char  *m_pbyStreamBuff;		//!< [in] the pointer to getstream buffer
	unsigned long	m_ulStreamBuffSize;		//!< [in] the size of getstream buffer
	unsigned long	m_ulStreamType;			//!< [in] the type of getstream
	void		   *m_pSpecificData;		//!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} flv_dmx_getstream_t;

typedef struct flv_dmx_outstream_t
{
	unsigned char  *m_pbyStreamData;		//!< [out] the pointer to outstream data
	unsigned long	m_ulStreamDataSize;		//!< [out] the size to outstream data
	unsigned long	m_ulStreamType;			//!< [out] the type of outstream
	unsigned long	m_ulTimeStamp;			//!< [out] the timestamp of outstream
	unsigned long	m_ulEndTimeStamp;		//!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
	void		   *m_pSpecificData;		//!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} flv_dmx_outstream_t;

#endif


/*
 ============================================================
 *
 *	Get Errors
 *
 ============================================================
*/
#define FLVDERR_NONE								0

// system error
#define FLVDERR_BASE_SYSTEM_ERROR					0
#define FLVDERR_SYSTEM_ERROR						(FLVDERR_BASE_SYSTEM_ERROR - 1 )
#define FLVDERR_FILE_SEEK_FAILED					(FLVDERR_BASE_SYSTEM_ERROR - 2 )
#define FLVDERR_PRIVATE_BUFF_ALLOCATION_FAILED		(FLVDERR_BASE_SYSTEM_ERROR - 3 )
#define FLVDERR_DEMUXER_OBJECT_ALLOCATION_FAILED	(FLVDERR_BASE_SYSTEM_ERROR - 4 )
#define FLVDERR_FILE_OPEN_FAILED0					(FLVDERR_BASE_SYSTEM_ERROR - 5 )
#define FLVDERR_FILE_OPEN_FAILED1					(FLVDERR_BASE_SYSTEM_ERROR - 6 )
#define FLVDERR_FRAME_BUFFER_ALLOCATION_FAILED		(FLVDERR_BASE_SYSTEM_ERROR - 7 )

// broken file error
#define FLVDERR_BASE_BROKEN_FILE					(-1000)
#define FLVDERR_BROKEN_FILE							(FLVDERR_BASE_BROKEN_FILE - 0 )
#define FLVDERR_INVALID_BODYOFFSET					(FLVDERR_BASE_BROKEN_FILE - 1 )
#define FLVDERR_ELEMENT_STREAM_NOT_FOUND			(FLVDERR_BASE_BROKEN_FILE - 2 )
#define FLVDERR_METADATA_NOT_FOUND					(FLVDERR_BASE_BROKEN_FILE - 3 )
#define FLVDERR_MAIN_HEADER_NOT_FOUND				(FLVDERR_BASE_BROKEN_FILE - 4 )
#define FLVDERR_ITS_BROKEN_FILE						(FLVDERR_BASE_BROKEN_FILE - 5 )
#define FLVDERR_INVALID_PREVIOUS_TAG_SIZE			(FLVDERR_BASE_BROKEN_FILE - 6 )

// seek failed error
#define FLVDERR_BASE_SEEK_FAILED					(-2000)
#define FLVDERR_SEEK_FAILED							(FLVDERR_BASE_SEEK_FAILED - 0)
#define FLVDERR_VIDEO_SEEK_FAILED					(FLVDERR_BASE_SEEK_FAILED - 1)
#define FLVDERR_AUDIO_SEEK_FAILED					(FLVDERR_BASE_SEEK_FAILED - 2)
#define FLVDERR_SEEK_FAILED_EOF						(FLVDERR_BASE_SEEK_FAILED - 3)
#define FLVDERR_VIDEO_SEEK_FAILED_EOF				(FLVDERR_BASE_SEEK_FAILED - 4)
#define FLVDERR_AUDIO_SEEK_FAILED_EOF				(FLVDERR_BASE_SEEK_FAILED - 5)
#define FLVDERR_INTERNAL_SEEK_ERROR					(FLVDERR_BASE_SEEK_FAILED - 6)
#define FLVDERR_TABLE_ERROR_ENCOUNTED				(FLVDERR_BASE_SEEK_FAILED - 7)

// not supported format
#define FLVDERR_BASE_NOT_SUPPORTED_FORMAT			(-3000)
#define FLVDERR_NOT_SUPPORTED_FORMAT				(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 0)
#define FLVDERR_AVCC_PARSING_FAILED					(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 1)
#define FLVDERR_UNKNOWN_VIDEO_HEADER				(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 2)
#define FLVDERR_UNKNOWN_AUDIO_HEADER				(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 3)
#define FLVDERR_SEQUENCE_HEADER_NOT_FOUND			(FLVDERR_BASE_NOT_SUPPORTED_FORMAT - 4)

// invalid parameter error
#define FLVDERR_BASE_INVALID_FUNC_PARAM				(-4000)
#define FLVDERR_INVALID_FUNC_PARAM					(FLVDERR_BASE_INVALID_FUNC_PARAM - 0 )
#define FLVDERR_INVALID_DEMUXER_HANDLE				(FLVDERR_BASE_INVALID_FUNC_PARAM - 1 )
#define FLVDERR_VIDEO_STREAM_NOT_EXIST				(FLVDERR_BASE_INVALID_FUNC_PARAM - 2 )
#define FLVDERR_AUDIO_STREAM_NOT_EXIST				(FLVDERR_BASE_INVALID_FUNC_PARAM - 3 )
#define FLVDERR_INVALID_SELECTION_INFO				(FLVDERR_BASE_INVALID_FUNC_PARAM - 4 )
#define FLVDERR_INVALID_SEEK_TIME					(FLVDERR_BASE_INVALID_FUNC_PARAM - 5 )
#define FLVDERR_OPERATION_NOT_SUPPORTED				(FLVDERR_BASE_INVALID_FUNC_PARAM - 6 )
#define FLVDERR_OPENED_FOR_MEDIAINFO				(FLVDERR_BASE_INVALID_FUNC_PARAM - 7 )

// demuxer internal error
#define FLVDERR_BASE_DEMUXER_INTERNAL				(-10000)
#define FLVDERR_DEMUXER_INTERNAL					(FLVDERR_BASE_DEMUXER_INTERNAL - 0)
#define FLVDERR_LAST_TAG_NOT_FOUND					(FLVDERR_BASE_DEMUXER_INTERNAL - 1)
#define FLVDERR_END_OF_STREAM						(FLVDERR_BASE_DEMUXER_INTERNAL - 2)
#define FLVDERR_SEEK_UNAVAILABLE					(FLVDERR_BASE_DEMUXER_INTERNAL - 3)


#define IS_SYSERROR(code)						(    0 >  code && code > -1000)
#define IS_FILEERROR(code)						(-1000 >= code && code > -2000)
#define IS_SEEKERROR(code)						(-2000 >= code && code > -3000)
#define IS_FORMATERROR(code)					(-3000 >= code && code > -4000)
#define IS_PARAMERROR(code)						(-4000 >= code && code > -5000)
#define IS_INTERNALERROR(code)					(-10000 >= code)


/*!
 ***********************************************************************
 * \brief
 *		TCC_FLV_DMX		: main api function of flv demuxer library
 * \param
 *		[in]Op			: demuxer operation 
 * \param
 *		[in,out]pHandle	: handle of flv demuxer library
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in,out]pParam2	: output or information parameter
 * \return
 *		If successful, TCC_FLV_DMX returns 0 or positive value. Otherwise, it returns a negative value.
 ***********************************************************************
 */
av_dmx_result_t 
TCC_FLV_DMX( int Op, av_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );



#endif/*_FLV_DEMUXER_H_*/
