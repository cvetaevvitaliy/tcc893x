/****************************************************************************
 *   FileName    : mkv_demuxer.h
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided "AS IS" and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¡¯s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
#ifndef _MKV_DEMUXER_H_
#define _MKV_DEMUXER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "../av_common_dmx.h"

//! MKV Demuxer OP Code
#define MKVDOP_OPEN_FROM_FILE_HANDLE	3000 //!< opcode : this is special operation to open from file handle
#define MKVDOP_GET_AVC_INFO				3001 //!< opcode : get sps/pps info
#define MKVDOP_CREATE_INDEX				3002 //!< opcode : this means that when index table does not exist, we can create it.
											 //				it doesn't work if MKVDOP_OPEN_FROM_FILE_HANDLE or MKVDF_DISABLE_SEEK_TO_END_OF_FILE is set.
#define MKVDOP_SET_OPTIONS				4000
#define MKVDOP_GET_VERSION				4001 //!< opcode : if you want to get version info. of current demuxer, use this opcode.

/*!
 ============================================================
 *
 *	File/Video/Audio/Subtitle Information
 *		- information about video, audio or subtitle into one file.
 *
 ============================================================
*/
//! file information [out]
typedef struct mkv_dmx_file_info_t
{
	// common info
	char		    *m_pszFileName;	
	unsigned long	 m_ulDuration;		// duration
	S64				 m_llFileSize;		// filesize

	// mkv specific info
#define MKVDINFO_SEEKABLE	(1<<0)		// this means that demuxer is able to seek.
	unsigned long	 m_ulInfoFlags; // Additional Output Information Flags
} mkv_dmx_file_info_t;

//! video stream information
typedef struct mkv_dmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// mkv specific info
	unsigned long 	 m_ulLastKeyFrameTimeStamp;
	unsigned long 	 m_ulTimeStampType; // if it is set, this means that demuxer's timestamp is decoding order(DTS)
										// it is not used for MKV, mkv's default value is PTS(zero)(Presentation TimeStamp)
	unsigned long	 m_ulStereoMode;
	// Stereo-3D video mode
	//  0: mono or unknown
	//  1: side by side (left eye is first)
	//  2: top-bottom (right eye is first) 
	//  3: top-bottom (left eye is first)
	//  4: checkboard (right is first)
	//  5: checkboard (left is first)
	//  6: row interleaved (right is first)
	//  7: row interleaved (left is first)
	//  8: column interleaved (right is first)
	//  9: column interleaved (left is first)
	// 10: anaglyph (cyan/red)
	// 11: side by side (right eye is first)
	// 12: anaglyph (green/magenta)
	// 13: both eyes laced in one Block (left eye is first)
	// 14: both eyes laced in one Block (right eye is first))
} mkv_dmx_video_info_t;

//! audio stream information
typedef struct mkv_dmx_audio_info_t
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

	// mkv specific info
	unsigned long	 m_ulBlockAlign;		// for WMA 2011/01/20
	unsigned long	 m_ulAvgBytesPerSec;	// for WMA 2011/01/20
} mkv_dmx_audio_info_t;

//! subtitle stream information
typedef struct mkv_dmx_subtitle_info_t
{	
	// common info
	unsigned long	 m_ulStreamID;
	char			*m_pszLanguage;

	// mkv specific info.
//http://www.matroska.org/technical/specs/subtitles/index.html
#define SUBTITLETYPE_SRT	0 // SRT subtitle
#define SUBTITLETYPE_SSA	1 // SSA subtitle
#define SUBTITLETYPE_USF	2 // Universal Subtitle Format
#define SUBTITLETYPE_BMP	3 // BMP
#define SUBTITLETYPE_VOBSUB	4 // VOBSUB
#define SUBTITLETYPE_KATE	5 // KATE
	unsigned long	 m_ulSubtitleType;
	void			*m_pSubtitleInfo;		// if exist
	unsigned long	 m_ulSubtitleInfoSize;
} mkv_dmx_subtitle_info_t;

typedef av_dmx_handle_t mkv_dmx_handle_t;
typedef av_dmx_result_t mkv_dmx_result_t;
/*!
 ============================================================
 *
 *	Demuxer Initialize
 *
 ============================================================
*/
// these flags are used to set m_ulInitFlags.
#define MKVDF_DISABLE_SEEK_TO_END_OF_FILE	(1<<0)	/* It prevents the demuxer from seeking to end of file. 
													   therefore host cannot get some info. (duration, last key) */
#define MKVDF_USE_FREAD_DIRECTLY			(1<<1)	// If it is set, your callback fread function will be called directly without buffering.
#define MKVDF_DISABLE_AUDIO_STREAM			(1<<2)	// If you don't need to get audio streams, set it. 
#define MKVDF_DISABLE_VIDEO_STREAM			(1<<3)	// If you don't need to get video streams, set it.
#define MKVDF_DISABLE_SUBTITLE_STREAM		(1<<4)	// If you don't need to get subtitle streams, set it.

//! options
typedef struct mkv_dmx_options_t
{
	unsigned long m_ulOptions;
} mkv_dmx_options_t;

//! Demuxer Initialize - input
typedef struct mkv_dmx_init_t
{
	// common
	av_memory_func_t m_stMemoryFuncs;			//!< [in] callback functions for memory
	av_file_func_t	 m_stFileFuncs;				//!< [in] callback functions for file
	char			*m_pszOpenFileName;			/*!< [in] open file name( when Op is DMXOP_OPEN ) or
														  handle of opened file( when Op is MKVDOP_OPEN_FROM_FILE_HANDLE ) */
	// mkv specific info.
	unsigned long	 m_ulInitFlags;				//!< [in] options or flags

	// cache settings
	unsigned long	 m_ulVideoFileCacheSize;	//!< [in] video file cache(buffer) size( default:520833 bytes )
	unsigned long	 m_ulAudioFileCacheSize;	//!< [in] audio file cache(buffer) size( default:520833 bytes )
	unsigned long	 m_ulSeekFileCacheSize;		//!< [in] file cache for seek routine( default:1024 bytes )
} mkv_dmx_init_t;

//! Demuxer Initialize - output
typedef struct mkv_dmx_info_t
{
	// file info
	mkv_dmx_file_info_t	    *m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			 m_ulVideoStreamTotal;		//!< number of video stream
	mkv_dmx_video_info_t    *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	mkv_dmx_video_info_t    *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	unsigned long			 m_ulAudioStreamTotal;		//!< number of audio stream
	mkv_dmx_audio_info_t    *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	mkv_dmx_audio_info_t    *m_pstDefaultAudioInfo;		//!< pointer to default audio information

	// subtitle stream info
	unsigned long			 m_ulSubtitleStreamTotal;	//!< number of subtitle stream
	mkv_dmx_subtitle_info_t *m_pstSubtitleInfoList;		//!< pointer to subtitle information list ( m_lAudioStreamTotal x sizeof(m_pstSubtitleInfoList) )
	mkv_dmx_subtitle_info_t *m_pstDefaultSubtitleInfo;	//!< pointer to default subtitle information
} mkv_dmx_info_t;

typedef struct mkv_dmx_metadata_t
{
	unsigned long	 m_ulMetaTotal;
	av_metadata_t	*m_pstMetaList;
} mkv_dmx_metadata_t;

/*!
 ============================================================
 *
 *	Demuxer Running..
 *		- mkv has no a specific data
 *
 ============================================================
*/
//#define USE_AV_COMMON_DMX

//! Running - getstream
#ifdef USE_AV_COMMON_DMX
typedef av_dmx_getstream_t	mkv_dmx_getstream_t;
#else
typedef struct mkv_dmx_getstream_t
{
	unsigned char  *m_pbyStreamBuff;		//!< [in] the pointer to getstream buffer
	unsigned long	m_ulStreamBuffSize;		//!< [in] the size of getstream buffer
	unsigned long	m_ulStreamType;			//!< [in] the type of getstream
	void		   *m_pSpecificData;		//!< [in] the pointer to specific input of demuxer(if demuxer has a specific data)
} mkv_dmx_getstream_t;
#endif

//! Running - outstream
#ifdef USE_AV_COMMON_DMX
typedef av_dmx_outstream_t	mkv_dmx_outstream_t;
#else
typedef struct mkv_dmx_outstream_t
{
	unsigned char  *m_pbyStreamData;		//!< [out] the pointer to outstream data
	unsigned long	m_ulStreamDataSize;		//!< [out] the size to outstream data
	unsigned long	m_ulStreamType;			//!< [out] the type of outstream
	unsigned long	m_ulTimeStamp;			//!< [out] the timestamp of outstream
	unsigned long	m_ulEndTimeStamp;		//!< [out] the end timestamp of outstream: This is not mandatory except in the case of text-subtitle
	void		   *m_pSpecificData;		//!< [out] the pointer to specific output of demuxer(if demuxer has a specific data)
} mkv_dmx_outstream_t;
#endif

//! Running - seek
#define MKVDSEEK_TIME_BASED_GETFRAME 0x100		/* this means that demuxer seek to only desired time (does not find keyframe)
													it would be a good selection for long keyframe intervals */
#ifdef USE_AV_COMMON_DMX
typedef av_dmx_seek_t		mkv_dmx_seek_t;
#else
typedef struct mkv_dmx_seek_t
{
	long			m_lSeekTime;	//!< millisecond unit
	unsigned long	m_ulSeekMode;	//!< mode flags
} mkv_dmx_seek_t;
#endif

/*!
 ============================================================
 *
 *	Channel Selection
 *
 ============================================================
*/

//! Select stream - input
typedef struct mkv_dmx_sel_stream_t
{
	unsigned long	 m_ulSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	unsigned long	 m_ulVideoID;		//!< [in] Video ID (if m_ulSelType has DMXTYPE_VIDEO)
	unsigned long	 m_ulAudioID;		//!< [in] Audio ID (if m_ulSelType has DMXTYPE_AUDIO)
	unsigned long	 m_ulSubtitleID;	//!< [in] Subtitle ID (if m_ulSelType has DMXTYPE_SUBTITLE)
} mkv_dmx_sel_stream_t;

//! Select stream - output
typedef struct mkv_dmx_sel_info_t
{
	mkv_dmx_video_info_t	*m_pstVideoInfo;	//!< [out] pointer to video information
	mkv_dmx_audio_info_t	*m_pstAudioInfo;	//!< [out] pointer to audio information
	mkv_dmx_subtitle_info_t	*m_pstSubtitleInfo;	//!< [out] pointer to subtitle information
} mkv_dmx_sel_info_t;

/*!
 ============================================================
 *
 *	Get AvcC Information(for H.264)
 *
 ============================================================
*/
#ifdef MKVDOP_GET_AVC_INFO

//! sps/pps parameter set
typedef struct mkv_dmx_avc_parameter_set_t
{
	unsigned char	*m_pbyData;
	unsigned long	 m_ulDataLength;
} mkv_dmx_avc_parameter_set_t;

//! output interface, avcC info(demuxer) : AVCDecoderConfigurationRecord in avc file format(ISO/IEC 14496-15) : 28 bytes
typedef struct mkv_dmx_avcC_t 
{
	unsigned long				 m_ulSpsNum;		//!< [out] numbers of sps
	mkv_dmx_avc_parameter_set_t	*m_pstSpsArray;		//!< [out] array of sps data
	unsigned long				 m_ulPpsNum;		//!< [out] numbers of pps
	mkv_dmx_avc_parameter_set_t* m_pstPpsArray;		//!< [out] array of pps data
	unsigned long				 m_ulNalLenSize;	//!< [out] size of nal length
} mkv_dmx_avcC_t;

//! input interface
typedef struct mkv_dmx_getavc_t
{
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;
} mkv_dmx_getavc_t;

//! output interface
typedef struct mkv_dmx_outavc_t
{
	mkv_dmx_avcC_t	*m_pstAvcC;			//!< [out] pointer to avcC info
} mkv_dmx_outavc_t;

#endif//MKVDOP_GET_AVC_INFO


/*!
 ============================================================
 *
 *	Get Errors
 *
 ============================================================
*/
typedef struct mkv_dmx_error_t
{
	long	 m_lErrCode;
	char	*m_pszErrStatus;
	S64		 m_llLastStatus;
} mkv_dmx_error_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_MKV_DMX			: main api function of mkv demuxer library
 * \param
 *		[in]ulOpCode		: demuxer operation 
 * \param
 *		[in,out]ptHandle	: handle of mkv demuxer library
 * \param
 *		[in]pParam1			: init or input parameter
 * \param
 *		[in,out]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MKV_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
mkv_dmx_result_t 
TCC_MKV_DMX( unsigned long ulOpCode, mkv_dmx_handle_t* ptHandle, void* pParam1, void* pParam2 );


#ifdef __cplusplus
}
#endif
#endif/*_MKV_DEMUXER_H_*/
