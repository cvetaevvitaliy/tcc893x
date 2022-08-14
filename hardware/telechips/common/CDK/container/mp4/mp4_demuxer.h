/****************************************************************************
 *   FileName    : mp4_demuxer.h
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
#ifndef _MP4_DEMUXER_H_
#define _MP4_DEMUXER_H_

#if defined(_WIN32_WCE)					// ARM WinCE
	typedef unsigned __int64  UI64;
	typedef __int64 I64;
#else
	typedef unsigned long long UI64;
	typedef signed long long I64;
#endif

#define MP4_DMX_VERSION "3.02.08"

//! Op Code 
#define MP4_GET_AVC_INFO 100 //!< opcode : get sps/pps info
#define MP4DMX_CALC_REAL_DURATION 	(1)
#define MP4DMX_SEARCH_LASTKEY_FRAME	(1<<1)
#define MP4DMX_SAVE_STBL_INTERNAL	(1<<2)
#define MP4DMX_PREVENT_EOF_FSEEK	(1<<3)
#define MP4DMX_MEDIA_SCAN_MODE_ON	(1<<4)
#define MP4DMX_SEQUENTIAL_GETSTREAM	(1<<5)
#define MP4DMX_SEEK_KEY_FRAME_ENABLE	(1<<6)


//! Return value
#ifndef ERR_NONE
#define ERR_NONE				 0
#endif
#ifndef ERR_END_OF_FILE
#define ERR_END_OF_FILE			-1
#endif
#ifndef ERR_END_OF_VIDEO_FILE
#define ERR_END_OF_VIDEO_FILE	-2
#endif
#ifndef ERR_END_OF_AUDIO_FILE
#define ERR_END_OF_AUDIO_FILE	-3
#endif
#ifndef ERR_END_OF_SUBTITLE_FILE
#define ERR_END_OF_SUBTITLE_FILE -4
#endif

#define MP4DMX_ERROR_BASE (-27000)
#define MP4DMX_ERROR_INVALID_AUDIOID		(MP4DMX_ERROR_BASE-21)
#define MP4DMX_ERROR_INVALID_VIDEOID		(MP4DMX_ERROR_BASE-22)
#define MP4DMX_ERROR_INVALID_SUBTITLEID		(MP4DMX_ERROR_BASE-23)

#ifdef MP4_GET_AVC_INFO

//! sps/pps parameter set (demuxer) : 8 bytes
typedef struct mp4_avc_parameter_set_t
{
	void* m_pData;		//!< pointer of sps or pps 
	int m_iDataLength;	//!< size of sps or pps 
} mp4_avc_parameter_set_t;

//! avcC info(demuxer) : AVCDecoderConfigurationRecord in avc file format(ISO/IEC 14496-15) : 28 bytes
typedef struct mp4_avcC_t 
{
	int m_iSpsNum;						//!< numbers of sps
	mp4_avc_parameter_set_t* m_pSpsArray;	//!< array of sps data
	int m_iPpsNum;						//!< numbers of pps
	mp4_avc_parameter_set_t* m_pPpsArray;	//!< array of pps data
	int m_iNalLenSize;					//!< size of nal length
} mp4_avcC_t;

//! avc info interface : 36 bytes
typedef struct mp4_dmx_avc_info_t
{
	void* m_pExtraData;			//!< [IN] pointer of extra data
	int m_iExtraDataLength;		//!< [IN] length of extra data
	mp4_avcC_t* m_pAvcC;			//!< [OUT] pointer of avcC info
} mp4_dmx_avc_info_t;

#endif//MP4_GET_AVC_INFO

//! common callback function : 128 bytes
typedef struct mp4_dmx_callback_func_t
{
	void* (*m_pMalloc		 ) ( unsigned int );					//!< malloc
	void* (*m_pNonCacheMalloc) ( unsigned int );					//!< non-cacheable malloc 
	void  (*m_pFree			 ) ( void* );							//!< free
	void  (*m_pNonCacheFree	 ) ( void* );							//!< non-cacheable free
	void* (*m_pMemcpy		 ) ( void*, const void*, unsigned int );//!< memcpy
	void* (*m_pMemset		 ) ( void*, int, unsigned int );		//!< memset
	
	void* (*m_pRealloc		 ) ( void*, unsigned int );				//!< realloc
	void* (*m_pMemmove		 ) ( void*, const void*, unsigned int );//!< memmove
	int m_Reserved1[16-8];

	void*		 (*m_pFopen	) ( const char *, const char * );						//!< fopen
	unsigned int (*m_pFread	) ( void*, unsigned int, unsigned int, void* );			//!< fread
	int			 (*m_pFseek	) ( void*, long, int );									//!< fseek
	long		 (*m_pFtell	) ( void* );											//!< ftell
	unsigned int (*m_pFwrite) ( const void*, unsigned int, unsigned int, void* );	//!< fwrite
	int			 (*m_pFclose) ( void* );											//!< fclose
	int			 (*m_pUnlink) ( const char* );										//!< _unlink
	unsigned int (*m_pFeof  ) ( void* );											//!< feof
	unsigned int (*m_pFflush) ( void* );											//!< fflush
	int			 (*m_pFseek64) ( void*, I64, int );								//!< fseek
	I64		 (*m_pFtell64) ( void* );											//!< ftell
	int m_Reserved2[16-11];
} mp4_dmx_callback_func_t;

typedef int mp4_dmx_handle_t;	//!< handle of libmp4
typedef int mp4_dmx_result_t;	//!< return value of libmp4

//! init parameter of TCC_MP4_DMX() : 128+32+96 bytes
typedef struct mp4_dmx_init_t 
{
	mp4_dmx_callback_func_t m_sCallbackFunc;//!< [in] callback functions of libmp4
	void* 	m_pOpenFileName;	//!< [in] open file name( when Op is INIT )
							//!< [in] handle of opened file( when Op is INIT_FROM_FILE_HANDLE )
	int		m_iRealDurationCheckEnable;
	int		m_iLastkeySearchEnable;
	int		m_iSaveInternalIndex;
	int		m_iPreventFseekToEof;
	int		m_iSequentialGetStream;
	int		m_iSeekKeyframeEnable;
	int     m_Reserved[8-7];

	//! 96 Bytes
	union 
	{
		int m_ReservedExtra[24];
		struct 
		{
			int m_iVideoFileCacheSize;	//!< [in] video file cache(buffer) size
			int m_iAudioFileCacheSize;	//!< [in] audio file cache(buffer) size
			int m_iSubtitleFileCacheSize;	//!< [in] audio file cache(buffer) size			int m_iSTSCCacheSize;		//!< [in] cache(buffer) size of STSC chunk
			int m_iSTSCCacheSize;		//!< [in] cache(buffer) size of STSC chunk
			int m_iSTSSCacheSize;		//!< [in] cache(buffer) size of STSS chunk
			int m_iSTSZCacheSize;		//!< [in] cache(buffer) size of STSZ chunk
			int m_iSTCOCacheSize;		//!< [in] cache(buffer) size of STCO chunk
			int m_iSTTSCacheSize;		//!< [in] cache(buffer) size of STTS chunk
			int m_iDefaultCacheSize;	//!< [in] cache(buffer) base file cache. On sequential mode, only this cache can be used.
			int m_ReservedExtra[24-9];
		} m_sCache;
	} m_uExtra;

} mp4_dmx_init_t;

typedef struct mp4_dmx_info_t
{
	//! file information : 128 Bytes
	struct 
	{
		/* common */
		char* 	m_pszOpenFileName;	//!< open file name
		char* 	m_pszCopyright;		//!< copyright
		char* 	m_pszCreationTime;	//!< creation time
		int 	m_iRunningtime;			//!< runing time * 1000
		I64 	m_lFileSize;			//!< total file size

		I64 	m_lUserDataPos;		//!< user data position
		int 	m_iTimeScale;			//!< timescale of file
		int 	m_iUserDataLen;			//!< user data length  40 bytes
		
		int					m_iNumTotalTracks;
		int					m_iSeekable;			// if(==1) seek is possible or ..
		char*				m_pcTitle;				
		char*				m_pcArtist;
		char*				m_pcAlbum;				//!<80 bytes
		char*				m_pcAlbumArtist;
		char*				m_pcYear;
		char*				m_pcWriter;
		char*				m_pcAlbumArt;
		int					m_iAlbumArtSize;
		char*				m_pcGenre;				//!< 100 bytes
		char				m_pcCompilation[4];		//!< 104 bytes
		char				m_pcCDTrackNumber[16];	//!< 120 bytes
		char				m_pcDiscNumber[16];		//!< 136 bytes
		char*				m_pcLocation;			//!< 140 bytes
		int					m_bStandardGenre;
		int					m_iFragmentedMp4;
		int 				m_Reserved[48-33];
	} m_sFileInfo;

	//! video information : 128 Bytes
	struct 
	{
		int					m_iVideoTrackCount;			// total track count
		int					m_iDefaultVideoTrackIndex; // if -1 no video track available
		int					m_iVideoTrackIndexCache;
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

		int m_iTotalNumber;			//!< total frame number
		int m_iKeyFrameNumber;		//!< key frame number
		int m_bAvcC;				//!< avcC flag for H264
		int m_iTrackTimeScale;		//!< timescale of video
		int m_iLastKeyTime;			//!< time of last key frame

		int	m_iMaxBitrate;			//!< maximum bitrate
		int m_iAvgBitrate;			//!< average bitrate
		int m_iRotateDegree;		//!< rotation degree
		unsigned int		m_uiAspectRatioX;
		unsigned int		m_uiAspectRatioY;		
		int m_Reserved[32-21];			
	} m_sVideoInfo;

	//! audio information : 128 Bytes
	struct 
	{
		int					m_iAudioTrackCount;			// total track count
		int					m_iDefaultAudioTrackIndex; // if -1 no audio track available
		int					m_iAudioTrackIndexCache;
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

		/* mp4 info */
		int m_iFramesPerSample;		//!< fps
		int m_iTrackTimeScale;		//!< timescale of audio
		int m_iSamplesPerPacket;	//!< samples / packet

		int	m_iMaxBitrate;			//!< maximum bitrate
		int m_iAvgBitrate;			//!< average bitrate
		int m_Reserved[32-17];			
	} m_sAudioInfo;

	struct
	{
		int				m_iSubtitleTrackCount;
		int				m_iCurrentSubtitleTrackIndex;
		int				m_iSubtitleTrackIndexCache;
		char			*m_pszLanguage;

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
	} m_stSubtitleInfo;

} mp4_dmx_info_t;

typedef struct mp4_dmx_input_t
{
	unsigned char* m_pPacketBuff;		//!< [in] allocated packet(video or audio) buffer pointer
	int m_iPacketBuffSize;				//!< [in] allocated packet(video or audio) buffer size
	int m_iPacketType;					//!< [in] PACKET_NONE or PACKET_VIDEO or PACKET_AUDIO
	int m_iUsedBytes;					//!< used bytes
	int	m_Reserved[8-4];
} mp4_dmx_input_t;

typedef struct mp4_dmx_output_t
{
	unsigned char* m_pPacketData;	//!< pointer of output data
	int m_iPacketSize;				//!< length of output data
	int m_iPacketType;				//!< packet type of output data
	int m_iTimeStamp;				//!< timestamp(msec) of output data
	int m_bKeyFrame;				//!< key flag of output data
	int m_iKeyCount;				//!< current key count
	I64 m_llOffset;					//!< current key count
	int m_iEndTimeStamp;			//!< end timestamp(msec) of output data
	int m_Reserved[32-9];			//!< reserved...
} mp4_dmx_output_t;

typedef struct mp4_dmx_seek_t
{
	long			m_lSeekTime;	//!< millisecond unit
	unsigned long	m_ulSeekMode;	//!< mode flags
} mp4_dmx_seek_t;

//! Select stream - input
typedef struct mp4_dmx_sel_stream_t
{
	unsigned long	 m_ulSelType;		//!< [in] DMXTYPE_VIDEO | DMXTYPE_AUDIO | DMXTYPE_SUBTITLE
	unsigned long	 m_ulVideoID;		//!< [in] Video ID (if m_ulSelType has DMXTYPE_VIDEO)
	unsigned long	 m_ulAudioID;		//!< [in] Audio ID (if m_ulSelType has DMXTYPE_AUDIO)
	unsigned long	 m_ulSubtitleID;	//!< [in] Subtitle ID (if m_ulSelType has DMXTYPE_SUBTITLE)
} mp4_dmx_sel_stream_t;

/*!
 ***********************************************************************
 * \brief
 *		TCC_MP4_DMX		: main api function of libmp4
 * \param
 *		[in]Op			: decoder operation 
 * \param
 *		[in,out]pHandle	: libmp4's handle
 * \param
 *		[in]pParam1		: init or input parameter
 * \param
 *		[in]pParam2		: output or information parameter
 * \return
 *		If successful, TCC_MP4_DMX returns 0 or plus. Otherwise, it returns a minus value.
 ***********************************************************************
 */
mp4_dmx_result_t 
TCC_MP4_DMX( int Op, mp4_dmx_handle_t* pHandle, void* pParam1, void* pParam2 );

#endif//_MP4_DEMUXER_H_
