/****************************************************************************
 *   FileName    : cdmx_instance_ext.h
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
#ifndef _CDMX_INSTANCE_EXT_H_
#define _CDMX_INSTANCE_EXT_H_

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// ext1 instance
//
typedef struct ext1_cdmx_file_info_t
{
	// common info
	char		    *m_pszFileName;	
	unsigned long	 m_ulDuration;		// duration
	S64				 m_llFileSize;		// filesize

	// ext1 specific info
	unsigned long	 m_ulTimeScale;
	unsigned long	 m_ulUserDataPos;
	unsigned long	 m_ulUserDataLen;
	char			*m_pszCopyright;
	char			*m_pszCreationTime;
	unsigned long 	 m_bSeekable;
	unsigned long 	 m_ulLastKeyFrameTime;
	unsigned long 	 m_ulTotalKeyFrameNum;
} ext1_cdmx_file_info_t;

typedef struct ext1_cdmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// ext1 specific info
	char			*m_pszCodecName;
	char			*m_pszCodecVendorName;
	unsigned long    m_ulLength;
	unsigned long    m_ulMOFTag;
	unsigned long    m_ulSubMOFTag;
	unsigned short   m_usBitCount;
	unsigned short   m_usPadWidth;
	unsigned short   m_usPadHeight;
	unsigned short   m_usReserved;
	unsigned long 	 m_ulFramesPerSecond;
	unsigned long    m_ulOpaqueDataSize;
	unsigned char	*m_pbyOpaqueData;
	unsigned long 	 m_ulMaxBitRate;
	unsigned long	 m_ulAvgBitRate;
} ext1_cdmx_video_info_t;

typedef struct ext1_cdmx_audio_info_t
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

	// ext1 specific info
	char			*m_pszCodecName;
	char			*m_pszCodecVendorName;
	unsigned long	 m_ulTotalFrameNumber;
	unsigned long	 m_ulActualRate;
	unsigned short	 m_usAudioQuality;
	unsigned short	 m_usFlavorIndex;
	unsigned long	 m_ulBitsPerFrame;
	unsigned long	 m_ulGranularity;
	unsigned long	 m_ulOpaqueDataSize;
	unsigned char	*m_pbyOpaqueData;
	unsigned long	 m_ulSamplesPerFrame;
	unsigned short	 m_usRegions;
	unsigned long	 m_ulFrameSizeInBits;
	unsigned long	 m_ulSampleRate; //?
	unsigned short	 m_usCplStart;
	unsigned short	 m_usCplQBits;
	unsigned short	 m_usDummy; //?
	unsigned long 	 m_ulMaxBitRate;
	unsigned long	 m_ulAvgBitRate;
} ext1_cdmx_audio_info_t;

typedef struct ext1_cdmx_info_t
{
	// file info
	ext1_cdmx_file_info_t	*m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			 m_ulVideoStreamTotal;		//!< number of video stream
	ext1_cdmx_video_info_t   *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	ext1_cdmx_video_info_t   *m_pstDefaultVideoInfo;	//!< pointer to default video information

	// audio stream info
	unsigned long			 m_ulAudioStreamTotal;		//!< number of audio stream
	ext1_cdmx_audio_info_t   *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	ext1_cdmx_audio_info_t   *m_pstDefaultAudioInfo;	//!< pointer to default audio information

} ext1_cdmx_info_t;


typedef struct ext1_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for EXT1 demuxer
	av_dmx_handle_t			hDmxHandle;
	ext1_dmx_init_t			stDmxInit;
	ext1_dmx_info_t			stExt1Info;
	ext1_dmx_getstream_t	stDmxGetStream;
	ext1_dmx_outstream_t	stDmxOutStream;
	ext1_dmx_seek_t			stDmxSeek;

	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

	ext1_cdmx_info_t		stDmxInfo;
} ext1_dmx_inst_t;

#endif//_CDMX_INSTANCE_EXT_H_

