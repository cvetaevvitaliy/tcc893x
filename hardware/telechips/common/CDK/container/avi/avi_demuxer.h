/****************************************************************************
 *   FileName    : avi_demuxer.h
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
#ifndef _AVI_DEMUXER_H_
#define _AVI_DEMUXER_H_

#include "../av_common_dmx.h"
//#include "../inc/AVI_stddefs.h" 

/*!
 ============================================================
 *
 *	File/Video/Audio/Subtitle Information
 *		- information about video, audio or subtitle into one file.
 *
 ============================================================
*/
//! file information
typedef struct avi_dmx_file_info_t
{
	// common info
	char		   *m_pszFileName;
	long			m_lDuration;           //!< Total Running Time
	S64				m_llFileSize;

	// avi specific info.
	long			m_lSuggestedBuffSize;
	long			m_bHasIndex;           //!< Existence of the Seek Table ( 0: Not Exist , 1: Exist )
	S64				m_llFirstVideoFilePos; //!< File position of the first video data (if this value is 0, ignore it)
	S64				m_llFirstAudioFilePos; //!< File position of the first audio data (if this value is 0, ignore it)

	// last key frame information 
	S64				m_ullLastKeyFrameOffset; //!< indicates the location of the last key frame(video). 
	unsigned long   m_ulLastKeyFramePTS;     //!< indicates the PTS of the last key frame(video). 

	// members for making index
	S64				m_llMoviStartPos;	     // 'movi' start position
} avi_dmx_file_info_t;

//! video stream information
typedef struct avi_dmx_video_info_t
{
	// common info
	long			 m_lStreamID;
	long			 m_lWidth;
	long			 m_lHeight;
	long			 m_lFrameRate;
	unsigned long	 m_ulFourCC;
	void			*m_pExtraData;
	long			 m_lExtraLength;

	// avi specific info.
	long			 m_lBitsPerSample;
	long			 m_lVideoBitrate;   // for Media Scan OP.
} avi_dmx_video_info_t;

//! audio stream information
typedef struct avi_dmx_audio_info_t
{	
	// common info
	long			 m_lStreamID;
	long			 m_lSamplePerSec;
	long			 m_lBitsPerSample;
	long			 m_lChannels;
	long			 m_lFormatTag;
	void		    *m_pExtraData;
	long			 m_lExtraLength;
	char			*m_pszLanguage;

	// avi specific info.
	long			m_lBlockAlign;
	long			m_lAvgBytesPerSec;
	long			m_lAudioBitrate;  // for Media Scan OP.
} avi_dmx_audio_info_t;

typedef av_dmx_handle_t avi_dmx_handle_t;
typedef av_dmx_result_t avi_dmx_result_t;



/*!
============================================================
*
*	Divx DRM CHUNK
*
============================================================
*/
//! DRM CHUNK
typedef struct strd_chunk_t
{
	unsigned int  m_ulFcc;						// Specifies a FOURCC code. The value must be 'strd'.
	unsigned int  m_ulCb;						// Specifies the size of the structure, not including the initial 8 bytes.
	unsigned int  m_ulVersion;			    	// Specifies the version of DRM that the RIFF was encrypted with. 
	unsigned int  m_ulSize;				    	// Bytes size of the DRM payload. The DRM payload consists of the DrmHeader.
	void*		  m_pDrmHeader;				    // Drm Header Arrary Pointer 
//	unsigned char m_abyDrmHeader[10000];		// Drm Header Array
} strd_chunk_t;

typedef struct dd_chunk_t
{
	unsigned short   m_usIsThisFrameEncrypted;     // Specifies whether this video frame is encrypted or not. (1 or 0)
	unsigned short   m_usFrameKeyIndex;			   // Specifies the index of the frame key that is used to encrypt the video frame in the stream.
	unsigned int     m_ulEncryptionOffset;         // Specifies the offset from where the data of the video frame is encrypted.
	unsigned int     m_ulEncryptionLength;         // Specifies the length of the portion of the video frame that is encrypted
} dd_chunk_t;

/*!
 ============================================================
 *
 *	Demuxer Initialize
 *
 ============================================================
*/
//! avi_dmx_init_t.m_ulOption flags
#define AVIOPT_DEFAULT			  0x0000	//!< Sequential Read / Use Internal Buffer
#define AVIOPT_SELECTIVE		  0x0001	//!< Video/Audio Selective Read
#define AVIOPT_USERBUFF			  0x0002	//!< Use User buffer
#define AVIOPT_MAKEINDEX		  0x0004    //!< Make Index by Sys (USE THIS FOR SEEK)
#define AVIOPT_STREAMING          0x0008    //!< Streaming Play Mode (Sequential Mode will be enforced regardless of 'AVIOPT_SELECTIVE' option)
#define AVIOPT_DO_NOT_SEEK_TO_END 0x0010    //!< Do not permit to seek to EOF
#define AVIOPT_DISABLE_INDEX_CHECK 0x0020    //!< Disable index checking routine to maximize performance (this option would be useful for streaming condition)

//! Demuxer Initialize - input
typedef struct avi_dmx_init_t
{
	// common
	av_memory_func_t m_stMemoryFuncs;		//!< [in] callback functions for memory
	av_file_func_t	 m_stFileFuncs;			//!< [in] callback functions for file
	char			*m_pszOpenFileName;		//!< [in] open file name

	//! avi specific info.
	long			m_lIoCacheSize;
	unsigned long	m_ulOption;				//!< option flags
} avi_dmx_init_t;

//! Demuxer Initialize - output
typedef struct avi_dmx_info_t
{
	// file info
	avi_dmx_file_info_t	    *m_pstFileInfo;				//!< pointer to file information

	// video stream info
	long				 	 m_lVideoStreamTotal;		//!< the number of video stream
	avi_dmx_video_info_t    *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	avi_dmx_video_info_t    *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	long				 	 m_lAudioStreamTotal;		//!< the number of audio stream
	avi_dmx_audio_info_t    *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	avi_dmx_audio_info_t    *m_pstDefaultAudioInfo;		//!< pointer to default audio information

	// Divx DRM info (special case)
	long					 m_lIsThisDivxDRM;			//!< If this value is 1, this contents is a Divx DRM.
	strd_chunk_t			*m_pstStrd;                 //!< Divx DRM STRD Chunk.  
} avi_dmx_info_t;


/*!
 ============================================================
 *
 *	Demuxer Running..
 *
 ============================================================
*/
typedef av_dmx_getstream_t avi_dmx_getstream_t;
typedef av_dmx_outstream_t avi_dmx_outstream_t;


/*!
 ============================================================
 *
 *	Channel Selection
 *
 ============================================================
*/

//! Select stream - input
typedef struct avi_dmx_sel_stream_t
{
	long					 m_lSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	long					 m_lVideoID;		//!< [in] Video ID
	long					 m_lAudioID;		//!< [in] Audio ID
} avi_dmx_sel_stream_t;

//! Select stream - output
typedef struct avi_dmx_sel_info_t
{
	avi_dmx_video_info_t	*m_pstVideoInfo;	//!< pointer to video information
	avi_dmx_audio_info_t	*m_pstAudioInfo;	//!< pointer to audio information
} avi_dmx_sel_info_t;



/*!
============================================================
*
*	Make Index by System Code
*
============================================================
*/

typedef struct avi_index_node_t  // AVI Index Linked-List (embedded in avi_dmx_index_t)
{
	unsigned int		     m_ulChunkID;		    // Type of current chunk. (Video / Audio / ETC)
	unsigned int		     m_ulChunkSize;			// Size of current chunk
	U64					     m_ullFilePosition;	    // Position of current chunk in file
	struct avi_index_node_t *pstNextIndex;			// Pointer to next index (Linked List)
} avi_index_node_t;


typedef struct avi_dmx_index_t  // AVI Index Structure
{
	int			      m_IsFinished;		   // Making index completed (if this value is 1, it means whole file scan is finished)
	unsigned int      m_ulCurrentNum;      // Total Number of Indexes that made until now 
	avi_index_node_t *pstHeadNode;         // pointer to the start position of indexes (Linked List)
	avi_index_node_t *pstTailNode;		   // pointer to the last position of indexes (Linked List)
} avi_dmx_index_t;


/*!
 ***********************************************************************
 * \brief
 *		TCC_AVI_DMX		: main api function of avi demuxer library
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: handle of avi demuxer library
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_AVI_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
avi_dmx_result_t 
TCC_AVI_DMX( int Op, avi_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );


/*!
===========================
	Get version of Library
===========================
*/
#define AVIDOP_GET_VERSION 01051641605
#define AVI_VER_BUFF_SIZE  32


/*!
===========================
	last error codes
===========================
*/
//! error none
#define	AVIERR_NONE							0	//

//! system error	(-1 ~ -999)
#define AVIERR_BASE_SYSTEM_ERROR					(0)
#define AVIERR_SYSTEM_ERROR					(AVIERR_BASE_SYSTEM_ERROR - 1)
#define	AVIERR_MEMORY_ALLOCATION_FAILED		(AVIERR_BASE_SYSTEM_ERROR - 2)
#define	AVIERR_MEMORY_REALLOCATION_FAILED	(AVIERR_BASE_SYSTEM_ERROR - 3)
#define	AVIERR_BUFFER_NOT_ENOUGH			(AVIERR_BASE_SYSTEM_ERROR - 4)
#define	AVIERR_FILE_OPEN_FAILED				(AVIERR_BASE_SYSTEM_ERROR - 5)
#define	AVIERR_END_OF_STREAM				(AVIERR_BASE_SYSTEM_ERROR - 6)
#define AVIERR_MEMORY_LEAKAGE				(AVIERR_BASE_SYSTEM_ERROR - 7)

//! broken file (-1000 ~ -1999)
#define AVIERR_BASE_BROKEN_FILE				(-1000)
#define AVIERR_BROKEN_FILE					(AVIERR_BASE_BROKEN_FILE - 0 )
#define	AVIERR_INVALID_SUPER_INDEX			(AVIERR_BASE_BROKEN_FILE - 1 )
#define	AVIERR_INVALID_STANDARD_INDEX		(AVIERR_BASE_BROKEN_FILE - 2 )
#define	AVIERR_INVALID_FIELD_INDEX			(AVIERR_BASE_BROKEN_FILE - 3 )
#define	AVIERR_SUPER_FCC_NOT_MATCHED		(AVIERR_BASE_BROKEN_FILE - 4 )
#define	AVIERR_INDEX_FCC_NOT_MATCHED		(AVIERR_BASE_BROKEN_FILE - 5 )

#define	AVIERR_RIFF_SIGNAL_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 6 )
#define	AVIERR_MAIN_HEADER_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 7 )
#define	AVIERR_STREAM_INFO_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 8 )
#define	AVIERR_STREAM_HEADER_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 9 )
#define	AVIERR_STREAM_FORMAT_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 10)
#define	AVIERR_MOVI_LIST_NOT_FOUND			(AVIERR_BASE_BROKEN_FILE - 11)
#define	AVIERR_STREAM_CHUNK_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 12)
#define	AVIERR_INDEX_CHUNK_NOT_FOUND		(AVIERR_BASE_BROKEN_FILE - 13)

#define	AVIERR_FILE_IS_BROKEN				(AVIERR_BASE_BROKEN_FILE - 14)
#define	AVIERR_INDEX_CHUNK_NOT_EXIST		(AVIERR_BASE_BROKEN_FILE - 15)

//! seek failed (-2000 ~ -2999)
#define AVIERR_BASE_SEEK_FAILED				(-2000)
#define AVIERR_SEEK_FAILED					(AVIERR_BASE_SEEK_FAILED - 0 )
#define	AVIERR_VIDEO_SCAN_SEEK_FAILED		(AVIERR_BASE_SEEK_FAILED - 1 )
#define	AVIERR_OLD_VIDEO_SEEK_FAILED		(AVIERR_BASE_SEEK_FAILED - 2 )
#define	AVIERR_ODML_VIDEO_SEEK_FAILED		(AVIERR_BASE_SEEK_FAILED - 3 )
#define	AVIERR_AUDIO_SCAN_SEEK_FAILED		(AVIERR_BASE_SEEK_FAILED - 4 )
#define	AVIERR_OLD_AUDIO_SEEK_FAILED		(AVIERR_BASE_SEEK_FAILED - 5 )
#define	AVIERR_ODML_AUDIO_SEEK_FAILED		(AVIERR_BASE_SEEK_FAILED - 6 )
#define	AVIERR_SEEK_INFO_NOT_EXIST			(AVIERR_BASE_SEEK_FAILED - 7 )
#define	AVIERR_INDEX_NOT_FOUND				(AVIERR_BASE_SEEK_FAILED - 8 )
#define	AVIERR_VBR_AUDIO					(AVIERR_BASE_SEEK_FAILED - 9 )
#define	AVIERR_SEEK_FAILED_EOF				(AVIERR_BASE_SEEK_FAILED - 10)
#define	AVIERR_SEEK_FAILED_AUDIO_EOF		(AVIERR_BASE_SEEK_FAILED - 11) // when 'seek video' had succeed, but 'seek audio' have failed
#define	AVIERR_MEMORY_SEEK_FAILED_OLDINDEX	(AVIERR_BASE_SEEK_FAILED - 12)
#define	AVIERR_MEMORY_SEEK_FAILED_ODMLINDEX	(AVIERR_BASE_SEEK_FAILED - 13)

//! not supported format error (-3000 ~ -3999)
#define AVIERR_BASE_NOT_SUPPORTED_FORMAT	(-3000)
#define AVIERR_NOT_SUPPORTED_FORMAT			(AVIERR_BASE_NOT_SUPPORTED_FORMAT - 0)
#define	AVIERR_MIDI_STREAM_NOT_SUPPORTED	(AVIERR_BASE_NOT_SUPPORTED_FORMAT - 1)
#define	AVIERR_TEXT_STREAM_NOT_SUPPORTED	(AVIERR_BASE_NOT_SUPPORTED_FORMAT - 2)
#define AVIERR_MUSTUSEINDEX_NOT_SUPPORTED	(AVIERR_BASE_NOT_SUPPORTED_FORMAT - 3)
#define AVIERR_INDEX_INFO_NOT_EXIST			(AVIERR_BASE_NOT_SUPPORTED_FORMAT - 4)

//! invalid parameter error (-4000 ~ -4999)
#define AVIERR_BASE_INVALID_PARAM			(-4000)
#define AVIERR_INVALID_PARAM				(AVIERR_BASE_INVALID_PARAM - 0)
#define	AVIERR_INVALID_VIDEO_ID				(AVIERR_BASE_INVALID_PARAM - 1)
#define	AVIERR_INVALID_AUDIO_ID				(AVIERR_BASE_INVALID_PARAM - 2)
#define	AVIERR_INVALID_SEEK_TIME			(AVIERR_BASE_INVALID_PARAM - 3)
#define	AVIERR_INVALID_STREAM_TYPE			(AVIERR_BASE_INVALID_PARAM - 4)
#define AVIERR_VIDEO_STREAM_NOT_EXIST		(AVIERR_BASE_INVALID_PARAM - 5)
#define AVIERR_AUDIO_STREAM_NOT_EXIST		(AVIERR_BASE_INVALID_PARAM - 6)

//! internal error
#define AVIERR_BASE_INTERNAL_ERROR			(-5000)
#define AVIERR_VIDEO_OFFSET_NOT_FOUND		(AVIERR_BASE_INTERNAL_ERROR - 0)
#define AVIERR_AUDIO_OFFSET_NOT_FOUND		(AVIERR_BASE_INTERNAL_ERROR - 1)
#define AVIERR_CHECK_CHUNK_BY_ODML_MEMORY   (AVIERR_BASE_INTERNAL_ERROR - 2)


#define IS_SYSTEM_ERROR(code)	(code <      0 && code > -1000)	// system error	(-1 ~ -999)
#define IS_FILE_ERROR(code)		(code <= -1000 && code > -2000)	// broken file (-1000 ~ -1999)
#define IS_SEEK_ERROR(code)		(code <= -2000 && code > -3000)	// seek failed (-2000 ~ -2999)
#define IS_FORMAT_ERROR(code)	(code <= -3000 && code > -4000)	// not supported format (-3000 ~ -3999)
#define IS_PARAM_ERROR(code)	(code <= -4000 && code > -5000)	// invalid parameter error (-4000 ~ -4999)
#define IS_INTERNAL_ERROR(code)	(code <= -5000 && code > -6000)	// demuxer internal error (-5000 ~ -5999)


#endif/*_AVI_DEMUXER_H_*/
