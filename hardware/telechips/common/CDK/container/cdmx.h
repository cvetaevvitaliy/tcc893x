/****************************************************************************
 *   FileName    : cdmx.h
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
#ifndef _CDMX_H_
#define _CDMX_H_


#include "../cdk/cdk_pre_define.h"
#include "../cdk/cdk_port.h"
#ifdef INCLUDE_MP4_DMX
	#include "mp4/mp4_demuxer.h"
#endif
#ifdef INCLUDE_MKV_DMX
	#include "mkv/mkv_demuxer.h"
#endif
#ifdef INCLUDE_MPG_DMX
	#include "mpg/mpg_demuxer.h"
#endif
#ifdef INCLUDE_AVI_DMX
	#include "avi/avi_demuxer.h"
#endif
#ifdef INCLUDE_TS_DMX
	#include "ts/TCC_TS_DEMUXER.h"
	#include "ts/TCC_TS_DEMUXER_EXT.h"
#endif
#ifdef INCLUDE_OGG_DMX
	#include "ogg/OGG_demuxer.h"
#endif
#ifdef INCLUDE_AUDIO_DMX
	#include "audio/audio_demuxer.h"
#endif
#ifdef INCLUDE_FLV_DMX
	#include "flv/flv_demuxer.h"
#endif
#ifdef INCLUDE_ASF_DMX
	#include "asf/asf_demuxer.h"
#endif
#ifdef INCLUDE_EXT1_DMX
	#include "ext/ext_demuxer.h"
#endif

#include "secure_av_parser.h"

#define CDMX_PTS_MODE		0	//! Presentation Timestamp (Display order)
#define CDMX_DTS_MODE		1	//! Decode Timestamp (Decode order)

#define CDMX_INIT			0
#define CDMX_GET_INFO		1
#define CDMX_GET_SEQ_HEADER	2
#define CDMX_GETSTREAM		3
#define CDMX_CLOSE			4
#define CDMX_SEL_STREAM		5
#define CDMX_SEEK			6
#define CDMX_PRE_INIT       10
#define CDMX_GET_INFO_SEL_STREAM 11


#define CDMX_SEEKMODE_RELATIVE_TIME		0
#define CDMX_SEEKMODE_ABSOLUTE_TIME		1

#define SKIMMING_MODE_DIRECTION_FORWARD		0x00000001
#define SKIMMING_MODE_DIRECTION_BACKWARD	0x00000002

#define CDMX_SECURE_TYPE_HDCP	1

#ifdef HAVE_ANDROID_OS
#define CDMX_MARKER_DEFAULT_PLAY_MODE_BIT       0x00000000 
#define CDMX_MARKER_SEQUENTIAL_PLAY_MODE_BIT    0x00000010 
#define CDMX_MARKER_PROGRESSIVE_PLAY_MODE_BIT   0x00000020 
#define CDMX_MARKER_PLAYBACK_ONLY_MODE_BIT      0x00000040 
#define CDMX_MARKER_MEDIA_SCAN_MODE_BIT  		0x00000080 
#define CDMX_MARKER_PREINIT_DONE_BIT            0x00000100 
#define CDMX_MARKER_IGNORE_FILE_SIZE_BIT        0x00000200
#define CDMX_MARKER_WFD_PLAYBACK_MODE_BIT       0x00000400
#define CDMX_MARKER_METADATA_MODE_BIT  			0x00000800 

#define ENABLE_MEDIASCAN_OPCODE
#endif

typedef struct cdmx_stream_id_list_t
{
	long	*plVideoIdList;
	long	*plAudioIdList;
	long	*plSubtitleIdList;
	long	*plGraphicsIdList;
} cdmx_stream_id_list_t;

typedef struct cdmx_select_stream_t
{
	long					lSelType;
	long					lVideoID;
	long					lAudioID;
	long					lSubtitleID;
	long					lGraphicsID;
} cdmx_select_stream_t;

typedef struct cdmx_seq_header_t
{
	unsigned char* m_pSeqHeaderData;	//!< Seqeunce header pointer
	int m_iSeqHeaderDataLen;			//!< Seqeunce header len (bytes)
} cdmx_seq_header_t;

typedef struct cdmx_input_t 
{
	unsigned char* m_pPacketBuff;		//!< [in] allocated packet(video or audio) buffer pointer
	int m_iPacketBuffSize;				//!< [in] allocated packet(video or audio) buffer size
	int m_iPacketType;					//!< [in] PACKET_NONE or PACKET_VIDEO or PACKET_AUDIO
	int m_iUsedBytes;					//!< used bytes
	int m_iInterestingCaptionIdx;		//!< caption index
//	int m_iSkimmingModeFlags;			//!< infomation for skimming mode such as speed and direction...
//	float m_iSkimmingPlaySpeed;			//!< infomation for skimming mode such as speed and direction...
//	int m_iSkimmingLateTime;
#ifdef HAVE_ANDROID_OS
	unsigned char* m_pPacketBuff_ext1;	//!< [in] allocated packet buffer pointer for the designated stream type only
	unsigned char* m_pPacketBuff_ext2;	//!< [in] allocated packet buffer pointer for the designated stream type only
#else
	int	m_Reserved[8-6];
#endif
} cdmx_input_t;

typedef struct cdmx_seek_t 
{
	int m_iSeekTime;				//!< seek time in millisec.(could be below "0". That means backward seeking in Relative-Seek-Mode)
	unsigned int m_uiSeekMode;		//!< "0" : Relative time Seek Mode, "1" : Absolute time Seek Mode
} cdmx_seek_t;

typedef struct cdmx_info_t
{
	//! File information : 128 Bytes
	struct 
	{
		/* common */
		char* m_pszOpenFileName;	//!< open file name
		char* m_pszCopyright;		//!< copyright
		char* m_pszCreationTime;	//!< creation time
		int m_iRunningtime;			//!< runing time * 1000
		int64_t m_lFileSize;		//!< total file size

		/* AVI info */
		int m_bHasIndex;
		int m_iSuggestedBufferSize;
		int m_iTotalStreams;

		/* mp4 info */
		int m_iTimeScale;			//!< timescale of file
		i64_t m_lUserDataPos;		//!< user data position
		int m_iUserDataLen;			//!< user data length
		char* m_pcTitle;				
		char* m_pcArtist;
		char* m_pcAlbum;				//!<80 bytes
		char* m_pcAlbumArtist;
		char* m_pcYear;
		char* m_pcWriter;
		char* m_pcAlbumArt;
		int m_iAlbumArtSize;
		char* m_pcGenre;				//!< 100 bytes
		char* m_pcCompilation;		//!< 104 bytes
		char* m_pcCDTrackNumber;	//!< 120 bytes
		char* m_pcDiscNumber;		//!< 136 bytes
		char* m_pcLocation;			//!< 140 bytes

		int m_iSeekable;			//!< flag of file seekable

		/* security info */
		uint32_t m_ulSecureType;
		int		m_iFragmentedMp4;
		int m_Reserved[32-29];
	} m_sFileInfo;

	//! Video information : 48*4 = 192 Bytes
	struct 
	{
		/* common */
		int m_iWidth;				//!< width
		int m_iHeight;				//!< height
		int m_iFrameRate;			//!< framerate * 1000;
		int m_iFourCC;				//!< fourcc

		unsigned int m_ulmax_bit_rate;        //!< maximum bit rate of stream 
		unsigned int m_ulavg_bit_rate;        //!< average bit rate of stream

		/* extra info (common) */
		char* m_pszCodecName;		//!< codec name
		char* m_pszCodecVendorName;	//!< codec vendor
		unsigned char* m_pExtraData;//!< extra data
		int m_iExtraDataLen;		//!< extra data length

		/* AVI info */
		int m_iNumVideoStream;		//!< number of video stream
		int m_iCurrVideoIdx;		//!< current video stream index
		int m_iBitsPerSample;		//!< bits per sample

		/* mp4 info */
		int m_iTotalNumber;			//!< total frame number
		int m_iKeyFrameNumber;		//!< key frame number
		int m_bAvcC;				//!< avcC flag for H264
		int m_iTrackTimeScale;		//!< timescale of video
		int m_iLastKeyTime;			//!< time of last key frame
		int m_iRotateDegree;

		/* asf info */
		int m_iVideoStreamID;			//!< video stream ID
		int	m_iHasPayloadAspectRatio;	//!< payload extension pixel aspect ratio
		int m_iAspectRatioX;			//!< pixel aspect ratio X
		int m_iAspectRatioY;			//!< pixel aspect ratio Y
		int m_lAvgBitRate;				//!< average bit rate

		/*mpg info*/
		int m_iAspectRatio;
		int m_iBitRate;

		/* EXTV1 info */	
		unsigned int   m_ulLength;				//!< The length of the video
		unsigned int   m_ulEXTV1_type1;				//!< The type of media contained inf the bitstream
		unsigned int   m_ulEXTV1_type2;			//!< The video format FOURCC code
		unsigned short int m_usEXTV1Cnt;		//!< bit number of the video
		unsigned short int   m_usPadWidth;		//!< padded width, in pixels, of a video frame
		unsigned short int   m_usPadHeight;		//!< padded height, in pixels, of a video frame
		unsigned short int   m_sdummy;			//!< dummy byte for 4byte align	
		unsigned int m_ulFramesPerSecond;		//!< Number of frames per second in the video
		unsigned int   m_ulEXTV1Size;		//!< extraDataSize
		char*    m_ulEXTV1Data;					//!< frame data
		// etc
		unsigned long		m_ulLastKeyTimestamp;
		unsigned long		m_ulStereoMode;
		int m_Reserved[48-36];
	} m_sVideoInfo;

	//! Audio information = 192 Bytes
	struct 
	{
		/* common */
		int m_iTotalNumber;						//!< total audio stream number
		int m_iSamplePerSec;					//!< samples/sec
		int m_iBitsPerSample;					//!< bits/sample
		int m_iChannels;						//!< channels
		int m_iFormatId;						//!< format id

		unsigned int m_ulmax_bit_rate;			//!< maximum bit rate of stream 
		unsigned int m_ulavg_bit_rate;			//!< average bit rate of stream

		/* extra info (common) */
		char* m_pszCodecName;					//!< codec name
		char* m_pszCodecVendorName;				//!< codec vendor
		unsigned char* m_pExtraData;			//!< extra data
		int m_iExtraDataLen;					//!< extra data length
		char* m_pszlanguageInfo;				//!< language information

		/* mp4 info */
		int m_iFramesPerSample;					//!< fps
		int m_iTrackTimeScale;					//!< timescale of audio
		int m_iSamplesPerPacket;				//!< samples / packet

		/* AVI info */
		int m_iNumAudioStream;					//!< number of audio stream
		int m_iCurrAudioIdx;					//!< current audio stream index
		int m_iBlockAlign;						//!< block align
		int m_iAvgBytesPerSec;					//!< average bytes per second

		/* ASF info */
//		ASF_AUDIOINFO pAudInfo[1];
		int m_iAudioStreamID;

		/* MPG info & Audio Only Parser */
		int m_iBitRate;

		/* RM info */
		int m_iActualRate;						//!< If the codec flavor supports it, this is the extended sample rate
		short m_sAudioQuality;					//!< This number is obsolete. It always contains a value of 100	
		short m_sFlavorIndex;					//!< Codec flavor used in this substream
		int m_iBitsPerFrame;					//!< Number of bits per audio frame
		int m_iGranularity;						//!< The total size of the block of compressed audio data
		int m_iOpaqueDataSize;					//!< Size, in bytes, of the codec-specific opaque data
		char*  m_pOpaqueData;					//!< Used to initialize the decoder

		int m_iSamplesPerFrame;					//!< Number of samples per audio frame
		int m_iframeSizeInBits;					//!< Sampling Frequency
		int m_iSampleRate;						//!< Sample rate of this substream
		short m_scplStart;						//!< Starting bin coded with coupling
		short m_scplQBits;						//!< allocated bit num for coupling
		short m_snRegions;						//!< number of region
		short m_sdummy;							//!< dummy byte for 4byte align

		/* TS info */
		unsigned long m_ulSubType;

		/* for Audio Only Path */
		int m_iMinDataSize;						//!< minimum data size
		
		int m_Reserved[48-34];
	} m_sAudioInfo;

	//! Subtitle information = 128 Bytes
	struct 
	{
		int m_iIsText;							//!< type
		int m_iNumSubtitleStream;				//!< number of subtitle stream
		int m_iCurrSubtitleIdx;					//!< current subtitle strea index
		char *m_pszLanguage;					//!< language

		/* ts info */
		int m_iPID;								//!< subtitle stream PID
		int m_iCharEnc;							//!< character encoding
		int m_iGraphicsType;					//!< graphics type (PRESENTATION / INTERACTIVE)

		/* mkv, mp4 info */
		int m_iTotalNumber;						//!< total subtitle stream number
		int m_iTrackIndexCache;					//!< track index cache
		void *m_pInfo;							//!< if exist
		int m_iInfoSize;						//!< if exist

		int m_Reserved[32-11];
	} m_sSubtitleInfo;
} cdmx_info_t;

#ifdef DIVX_DRM5//ndef DIVX_DRM
typedef struct av_dmx_divxdrm_info_t
{
	unsigned short	 m_usIsThisFrameEncrypted;	   // Specifies whether this video frame is encrypted or not. (1 or 0)
	unsigned short	 m_usFrameKeyIndex; 		   // Specifies the index of the frame key that is used to encrypt the video frame in the stream.
	unsigned int	 m_ulEncryptionOffset;		   // Specifies the offset from where the data of the video frame is encrypted.
	unsigned int	 m_ulEncryptionLength;		   // Specifies the length of the portion of the video frame that is encrypted
} av_dmx_divxdrm_info_t;
#endif

typedef struct cdmx_output_t
{
	unsigned char* m_pPacketData;			//!< pointer of output data
	int m_iPacketSize;						//!< length of output data
	int m_iPacketType;						//!< packet type of output data
	int m_iTimeStamp;						//!< timestamp(msec) of output data
	int m_bKeyFrame;						//!< key flag of output data
	int m_iKeyCount;						//!< current key count
	// INTERNAL_SUBTITLE_INCLUDE
	unsigned int	m_iEndTimeStamp ;
	unsigned int	m_iSubtitleType ;

	/* For Audio Only Path */
	int m_iEndOfFile;						//!< end of file
	int m_iDecodedSamples;					//!< Decoded samples
	unsigned int m_uiUseCodecSpecific;		//!< variable use in several codec
//	int m_iSkimmingTimeStamp;				//!< DTS is varied since skimming mode is performed

	#ifdef DIVX_DRM5
	av_dmx_divxdrm_info_t	m_divxdrm_info;
	#endif

	unsigned long   m_ulSecureType;
	union {
		struct {
			unsigned long		ulStreamCounter;
			unsigned long long	ullInputCounter;
		} stHdcpInfo;
	} m_enSecureInfo;
	long m_bPacketDiscont;
	int m_Reserved[32-15];					//!< reserved...
} cdmx_output_t;



/*============================================
	Common Demuxer API
============================================*/
//! File information
typedef struct av_dmx_file_info_t
{
	// common info
	char		   *m_pszFileName;
	long			m_lDuration;
	int64_t			m_llFileSize;
} av_dmx_file_info_t;

//! Video stream information
typedef struct av_dmx_video_info_t
{
	// common info
	long			 m_lStreamID;
	long			 m_lWidth;
	long			 m_lHeight;
	long			 m_lFrameRate;
	unsigned long	 m_ulFourCC;
	void			*m_pExtraData;
	long			 m_lExtraLength;
} av_dmx_video_info_t;

//! Audio stream information
typedef struct av_dmx_audio_info_t
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
} av_dmx_audio_info_t;

//! Subtitle stream information
typedef struct av_dmx_subtitle_info_t
{	
	// common info
	long			 m_lStreamID;
	char			*m_pszLanguage;
} av_dmx_subtitle_info_t;

//! Graphics stream information
typedef struct av_dmx_graphics_info_t
{	
	// common info
	long			 m_lStreamID;
	char			*m_pszLanguage;
} av_dmx_graphics_info_t;

//! Demuxer Initialize Information
typedef struct av_dmx_info_t
{
	// file info
	av_dmx_file_info_t	   *m_pstFileInfo;				//!< pointer to file information

	// video stream info
	long				 	m_lVideoStreamTotal;		//!< the number of video stream
	av_dmx_video_info_t    *m_pstVideoInfoList;			//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	av_dmx_video_info_t    *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	long				    m_lAudioStreamTotal;		//!< the number of audio stream
	av_dmx_audio_info_t    *m_pstAudioInfoList;			//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	av_dmx_audio_info_t    *m_pstDefaultAudioInfo;		//!< pointer to default audio information

	// text subtitle stream info
	long				    m_lSubtitleStreamTotal;		//!< the number of subtitle stream
	av_dmx_subtitle_info_t *m_pstSubtitleInfoList;		//!< pointer to subtitle information list ( m_lSubtitleStreamTotal x sizeof(m_pstSubtitleInfoList) )
	av_dmx_subtitle_info_t *m_pstDefaultSubtitleInfo;	//!< pointer to default audio information

	// graphics stream info
	long				    m_lGraphicsStreamTotal;		//!< the number of graphics stream
	av_dmx_graphics_info_t *m_pstGraphicsInfoList;		//!< pointer to graphics information list ( m_lGraphicsStreamTotal x sizeof(m_pstGraphicsInfoList) )
	av_dmx_graphics_info_t *m_pstDefaultGraphicsInfo;	//!< pointer to default audio information
} av_dmx_info_t;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#define CDMX_MAX_FRAME_LEN (2*1024*1024)

typedef struct cdmx_pre_init_t
{
#define PRE_INIT_NUM 4
	long 	m_lParam[PRE_INIT_NUM];
} cdmx_pre_init_t;

typedef struct cdmx_init_t
{
	char		    		*pszFileName;	
	void					*pstCallback;
	unsigned long			 ulParsingMode;
#define CDMXINIT_SPDIF_MULTIMODE	1
	unsigned long			 ulOptions;
} cdmx_init_t;

//////////////////////////////////////////////////////////////////////////
// 
// buffers
//
typedef struct cdmx_buff_t
{
	// for seq. header
	unsigned char	*pbySeqHeaderBuff;
	unsigned long 	 ulSeqHeaderBuffSize;
	// for stream buffer
	unsigned char	*pbyStreamBuff;
	unsigned long 	 ulStreamBuffSize;
	// stream id list
	cdmx_stream_id_list_t stStrmIdList;
} cdmx_buff_t;
//////////////////////////////////////////////////////////////////////////
// 
// control variables
//
typedef struct cdmx_ctrl_t
{
	// cdmx common
#define CDMX_VCODEC_AVC			1
#define CDMX_VCODEC_MPEG4		2
#define CDMX_VCODEC_VC1_SP_MP 	3
#define CDMX_VCODEC_VC1_AP 		4
#define CDMX_VCODEC_WMV78 		5
#define CDMX_VCODEC_RV89 		6
#define CDMX_VCODEC_DIV3		7
#define CDMX_VCODEC_VP8			8
	long			lCodec;
	long			bHasIdxTable;
	long			bSeqHeaderDone;
	long			lAvcNalLengthSize;
	long			lLastKeyTimeStamp;
	long			bIsNotFirstFrame;
	long			bPreInitDone;

//	long			lVideoOutCount;
//	long			lPrevVideoPTS;
	long 	 		bIsKeyframe;

	unsigned long 	ulParsingMode;

	// extra information
	unsigned long 	ulTimeScale;
	unsigned long 	bSeekable;
	unsigned long 	ulKeyFrameTotal;

	// security info
	unsigned long 	ulSecureType;

	//
	long 			lSpdifMultiMode;
	long 			lSpdifTrueHDSub;

} cdmx_ctrl_t;

typedef struct cdmx_meta_t
{
	char			*pszTitle;				
	char			*pszArtist;
	char			*pszAlbum;			// 80 bytes
	char			*pszAlbumArtist;
	char			*pszYear;
	char			*pszWriter;
	char			*pszAlbumArt;
	unsigned long 	 ulAlbumArtSize;
	char			*pszGenre;			// 100 bytes
	char			*pszCompilation;	// 104 bytes
	char			*pszCDTrackNumber;	// 120 bytes
	char			*pszDiscNumber;		// 136 bytes
	char			*pszLocation;		// 140 bytes
	unsigned long 	 bFragmentedMp4;
} cdmx_meta_t;

//////////////////////////////////////////////////////////////////////////
// 
// common instance
//
typedef struct cdmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;
} cdmx_inst_t;


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// mkv instance
//
typedef struct mkv_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for MKV demuxer
	mkv_dmx_handle_t		hDmxHandle;
	mkv_dmx_init_t			stDmxInit;
	mkv_dmx_info_t			stDmxInfo;
//	mkv_dmx_sel_stream_t	stDmxSelStream;
//	mkv_dmx_sel_info_t		stDmxSelInfo;
	mkv_dmx_metadata_t		stDmxMeta;
	mkv_dmx_getstream_t		stDmxGetStream;
	mkv_dmx_outstream_t		stDmxOutStream;
	mkv_dmx_seek_t			stDmxSeek;
	mkv_dmx_error_t			stDmxError;
	mkv_dmx_getavc_t		stDmxGetAvc;
	mkv_dmx_outavc_t		stDmxOutAvc;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

} mkv_dmx_inst_t;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// avi instance
//
typedef struct avi_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for AVI demuxer
	avi_dmx_handle_t		hDmxHandle;
	avi_dmx_init_t			stDmxInit;
	avi_dmx_info_t			stDmxInfo;
	avi_dmx_getstream_t		stDmxGetStream;
	avi_dmx_outstream_t		stDmxOutStream;
	av_dmx_seek_t			stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

} avi_dmx_inst_t;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// audio instance
//
typedef struct audio_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for Audio demuxer
	av_dmx_handle_t			hDmxHandle;
	audio_dmx_init_t		stDmxInit;
	audio_dmx_info_t		stDmxInfo;
	audio_dmx_input_t		stDmxGetStream;
	audio_dmx_output_t		stDmxOutStream;
	audio_dmx_seek_t		stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

} audio_dmx_inst_t;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// flv instance
//
typedef struct flv_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for FLV demuxer
	av_dmx_handle_t			hDmxHandle;
	flv_dmx_init_t			stDmxInit;
	flv_dmx_info_t			stDmxInfo;
	flv_dmx_getstream_t		stDmxGetStream;
	flv_dmx_outstream_t		stDmxOutStream;
	av_dmx_seek_t			stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

} flv_dmx_inst_t;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// mp4 instance
//
typedef struct mp4_cdmx_file_info_t
{
	// common info
	char		    *m_pszFileName;	
	unsigned long	 m_ulDuration;		// duration
	S64				 m_llFileSize;		// filesize

	// mp4 specific info
	unsigned long	 m_ulTimeScale;
	unsigned long	 m_ulUserDataPos;
	unsigned long	 m_ulUserDataLen;
	unsigned long	 m_ulTotalStreams;

	char			*m_pszCopyright;
	char			*m_pszCreationTime;
	char			*m_pszTitle;				
	char			*m_pszArtist;
	char			*m_pszAlbum;			//!<80 bytes
	char			*m_pszAlbumArtist;
	char			*m_pszYear;
	char			*m_pszWriter;
	char			*m_pszAlbumArt;
	unsigned long 	 m_ulAlbumArtSize;
	char			*m_pszGenre;			//!< 100 bytes
	char			*m_pszCompilation;		//!< 104 bytes
	char			*m_pszCDTrackNumber;	//!< 120 bytes
	char			*m_pszDiscNumber;		//!< 136 bytes
	char			*m_pszLocation;			//!< 140 bytes

	unsigned long 	 m_bSeekable;
	unsigned long 	 m_bFragmented;

} mp4_cdmx_file_info_t;

typedef struct mp4_cdmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// mp4 specific info
	char			*m_pszCodecName;
	char			*m_pszCodecVendorName;
	unsigned long	 m_ulTotalFrameNumber;
	unsigned long	 m_ulTotalKeyFrameNumber;
	unsigned long	 m_bIsAvcC;
	unsigned long	 m_ulTrackTimeScale;
	unsigned long	 m_ulRotateDegree;
	unsigned long	 m_ulAspectRatioX;
	unsigned long	 m_ulAspectRatioY;

} mp4_cdmx_video_info_t;

typedef struct mp4_cdmx_audio_info_t
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

	// mp4 specific info
	unsigned long	 m_ulTotalFrameNumber;
	char			*m_pszCodecName;
	char			*m_pszCodecVendorName;
	unsigned long	 m_ulFramesPerSample;
	unsigned long	 m_ulTrackTimeScale;
	unsigned long	 m_ulSamplesPerPacket;

} mp4_cdmx_audio_info_t;

typedef struct mp4_cdmx_subtitle_info_t
{	
	// common info
	unsigned long	 m_ulStreamID;
	char			*m_pszLanguage;

	// mp4 specific info.
	unsigned long	 m_ulSubtitleType;
	void			*m_pSubtitleInfo;		// if exist
	unsigned long	 m_ulSubtitleInfoSize;
} mp4_cdmx_subtitle_info_t;


typedef struct mp4_cdmx_info_t
{
	// file info
	mp4_cdmx_file_info_t	*m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			 m_ulVideoStreamTotal;		//!< number of video stream
	mp4_cdmx_video_info_t   *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	mp4_cdmx_video_info_t   *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	unsigned long			 m_ulAudioStreamTotal;		//!< number of audio stream
	mp4_cdmx_audio_info_t   *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	mp4_cdmx_audio_info_t   *m_pstDefaultAudioInfo;		//!< pointer to default audio information

	// subtitle stream info
	unsigned long			 m_ulSubtitleStreamTotal;	//!< number of subtitle stream
	mp4_cdmx_subtitle_info_t *m_pstSubtitleInfoList;		//!< pointer to subtitle information list ( m_lAudioStreamTotal x sizeof(m_pstSubtitleInfoList) )
	mp4_cdmx_subtitle_info_t *m_pstDefaultSubtitleInfo;	//!< pointer to default subtitle information

} mp4_cdmx_info_t;

typedef struct mp4_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for MP4 demuxer
	av_dmx_handle_t			hDmxHandle;
	mp4_dmx_init_t			stDmxInit;
	mp4_dmx_info_t			stMp4Info;
	mp4_dmx_input_t			stDmxGetStream;
	mp4_dmx_output_t		stDmxOutStream;
	av_dmx_seek_t			stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

	mp4_cdmx_info_t			stDmxInfo;
} mp4_dmx_inst_t;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// mpg instance
//
typedef struct mpg_cdmx_file_info_t
{
	// common info
	char		    *m_pszFileName;	
	unsigned long	 m_ulDuration;		// duration
	S64				 m_llFileSize;		// filesize

	// mpg specific info
} mpg_cdmx_file_info_t;

typedef struct mpg_cdmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// mpg specific info
	unsigned long	 m_ulAspectRatio;
	unsigned long	 m_ulBitRate;
	long	 		 m_lStartPTS;
	long	 		 m_lEndPTS;
} mpg_cdmx_video_info_t;

typedef struct mpg_cdmx_audio_info_t
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

	// mpg specific info
	unsigned long	 m_ulBitRate;
	unsigned long	 m_ulSyncWord;
	unsigned long	 m_ulChunkSize;
	long	 		 m_lStartPTS;
	long	 		 m_lEndPTS;

} mpg_cdmx_audio_info_t;

typedef struct mpg_cdmx_info_t
{
	// file info
	mpg_cdmx_file_info_t	    *m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			 m_ulVideoStreamTotal;		//!< number of video stream
	mpg_cdmx_video_info_t   	*m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	mpg_cdmx_video_info_t    *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	unsigned long			 m_ulAudioStreamTotal;		//!< number of audio stream
	mpg_cdmx_audio_info_t    *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	mpg_cdmx_audio_info_t    *m_pstDefaultAudioInfo;		//!< pointer to default audio information

} mpg_cdmx_info_t;

typedef struct mpg_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for MPG demuxer
	mpg_handle_t			hDmxHandle;
	mpg_dec_init_t			stDmxInit;
	mpg_dec_info_t 			stMpgInfo;
	mpg_dec_input_t			stDmxGetStream;
	mpg_dec_output_t		stDmxOutStream;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

	mpg_cdmx_info_t			stDmxInfo;
} mpg_dmx_inst_t;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// ogg instance
//
typedef struct ogg_cdmx_file_info_t
{
	// common info
	char		    *m_pszFileName;	
	unsigned long	 m_ulDuration;		// duration
	S64				 m_llFileSize;		// filesize

	// ogg specific info
	char		    *m_pszOpenFileName;
	char		    *m_pszCopyright;
	char		    *m_pszCreationTime;
	unsigned long	 m_ulTotalBitRate;
	unsigned long 	 m_bSeekable;
} ogg_cdmx_file_info_t;

typedef struct ogg_cdmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// ogg specific info
	char		    *m_pszCodecName;
	char		    *m_pszCodecVendorName;
	unsigned long	 m_ulBitRate;
} ogg_cdmx_video_info_t;

typedef struct ogg_cdmx_audio_info_t
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

	// ogg specific info
	char		    *m_pszCodecName;
	char		    *m_pszCodecVendorName;
	unsigned long	 m_ulBitRate;
} ogg_cdmx_audio_info_t;

typedef struct ogg_cdmx_info_t
{
	// file info
	ogg_cdmx_file_info_t	*m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			 m_ulVideoStreamTotal;		//!< number of video stream
	ogg_cdmx_video_info_t   *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	ogg_cdmx_video_info_t   *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	unsigned long			 m_ulAudioStreamTotal;		//!< number of audio stream
	ogg_cdmx_audio_info_t   *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	ogg_cdmx_audio_info_t   *m_pstDefaultAudioInfo;		//!< pointer to default audio information
} ogg_cdmx_info_t;

typedef struct ogg_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for OGG demuxer
	av_dmx_handle_t			hDmxHandle;
	ogg_dmx_init_t			stDmxInit;
	ogg_dmx_info_t			stOggInfo;
	ogg_dmx_input_t			stDmxGetStream;
	ogg_dmx_output_t		stDmxOutStream;
	av_dmx_seek_t			stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

	ogg_cdmx_info_t			stDmxInfo;
} ogg_dmx_inst_t;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 
// ts instance
//
typedef struct ts_cdmx_file_info_t
{
	// common info
	char		    *m_pszFileName;	
	unsigned long	 m_ulDuration;		// duration
	S64				 m_llFileSize;		// filesize

	// ts specific info
	unsigned long	 m_ulSecureType;
	unsigned long	 m_ulBuffChangeStatus;
} ts_cdmx_file_info_t;

typedef struct ts_cdmx_video_info_t
{
	// common info
	unsigned long	 m_ulStreamID;
	unsigned long	 m_ulWidth;
	unsigned long	 m_ulHeight;
	unsigned long	 m_ulFrameRate;
	unsigned long	 m_ulFourCC;
	unsigned char	*m_pbyCodecPrivate;
	unsigned long	 m_ulCodecPrivateSize;

	// ts specific info
	unsigned long	 m_bIsStereo;
} ts_cdmx_video_info_t;

typedef struct ts_cdmx_audio_info_t
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

	// ts specific info
	unsigned long	 m_ulBitRate;
	unsigned long 	 m_ulSubType;
} ts_cdmx_audio_info_t;

typedef struct ts_cdmx_subtitle_info_t
{	
	// common info
	unsigned long	 m_ulStreamID;
	char			*m_pszLanguage;

	// ts specific info.
	unsigned long	 m_ulSubtitleType;
} ts_cdmx_subtitle_info_t;


typedef struct ts_cdmx_info_t
{
	// file info
	ts_cdmx_file_info_t		*m_pstFileInfo;				//!< pointer to file information

	// video stream info
	unsigned long			m_ulVideoStreamTotal;		//!< number of video stream
	ts_cdmx_video_info_t   *m_pstVideoInfoList;		//!< pointer to video information list ( m_lVideoStreamTotal x sizeof(m_pstVideoInfoList) )
	ts_cdmx_video_info_t   *m_pstDefaultVideoInfo;		//!< pointer to default video information

	// audio stream info
	unsigned long			m_ulAudioStreamTotal;		//!< number of audio stream
	ts_cdmx_audio_info_t   *m_pstAudioInfoList;		//!< pointer to audio information list ( m_lAudioStreamTotal x sizeof(m_pstAudioInfoList) )
	ts_cdmx_audio_info_t   *m_pstDefaultAudioInfo;		//!< pointer to default audio information

	// subtitle stream info
	unsigned long			m_ulSubtitleStreamTotal;
	ts_cdmx_subtitle_info_t *m_pstSubtitleInfoList;
	ts_cdmx_subtitle_info_t *m_pstDefaultSubtitleInfo;

} ts_cdmx_info_t;

typedef struct ts_dmx_inst_t
{
	// common
	cdmx_ctrl_t				stCdmxCtrl;
	cdmx_buff_t				stCdmxBuff;
	cdmx_seq_header_t		stSeqHeader;

	// for TS demuxer
	av_dmx_handle_t			hDmxHandle;
	tsd_init_t				stDmxInit;
	tsd_info_t				stTsdInfo;
	tsd_getstream_t			stDmxGetStreamBuff;
	tsd_outstream_t			stDmxOutStreamBuff;
	tsd_getframe_t			stDmxGetFrame;
	tsd_outframe_t			stDmxOutFrame;
	tsd_seek_t				stDmxSeek;
	av_result_t (*pfnDmxLib) ( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
	void					*pvLibHandle;

	tsdext_config_scan_endpts_t  stConfScanEndpts;
	tsd_setup_buffer_t		stDmxStrmBuff;
	ts_cdmx_info_t			stDmxInfo;

} ts_dmx_inst_t;


#ifdef INCLUDE_ASF_DMX
#include "asf/cdmx_instance_asf.h"
#endif

#ifdef INCLUDE_EXT1_DMX
#include "ext/cdmx_instance_ext.h"
#endif
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	demuxer information
//
void
cdmx_memory_func_init( 
    av_memory_func_t *pstMemoryFuncs
	);

void 
cdmx_print_data (
   unsigned char	*pbyData, 
   long 	 		 lDatalen,
   long 	 		 lDivisor,
   long 	 		 lMaxCount
   );

void
cdmx_print_info (
    void 	*pDmxInfo,
	int 	 lVideoInfoLength,
	int 	 lAudioInfoLength,
	int 	 lSubtitleInfoLength,
	int 	 lGraphicsInfoLength
	);

void
cdmx_print_sel_info (
    av_dmx_video_info_t		*pstVideoInfo,
	av_dmx_audio_info_t		*pstAudioInfo,
	av_dmx_subtitle_info_t 	*pstSubtitleInfo,
	av_dmx_graphics_info_t 	*pstGraphicsInfo
	);

long
cdmx_get_stream_id_list (
    cdmx_stream_id_list_t 	*pstStreamIdList,
    void 					*pDmxInfo,
	long 	 			 	 lVideoInfoLength,
	long 	 			 	 lAudioInfoLength,
	long 	 			 	 lSubtitleInfoLength,
	long 	 			 	 lGraphicsInfoLength
	);
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//	Video sequence header extractor
//
int // -1: not implemented, 0: none, 1: extracted / implemented
cdmx_get_seqhead(
    const unsigned char	 *pbyData, 
	long				  lDataSize,
	unsigned char		**ppbySeqHead,
	long				 *plHeadLength,
	unsigned long		  ulFourCC
	);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//  for VPU
//

// added some codes to fix a problem of VPU's MPEG-4 decoder when the codec_private_data doesn't exist.
long
make_mpeg4_vol_header( 
	unsigned char 	 	 *pbyStreamData,
	unsigned long  	  	  ulStreamSize,
	unsigned char		**ppbyDest,
	unsigned long	 	 *pulDestSize,
	unsigned long 		  ulWidth,
	unsigned long 		  ulHeight,
	unsigned long 		  bIsAsp 
	);

long
avcc_to_bytestream( 
    unsigned char	 *pbyPriv,
	unsigned long 	  ulPrivSize,
	unsigned char	**ppbyDest,
	unsigned long	 *pulDestSize
	);

long
attach_start_code(
    cdmx_inst_t			*pstInst,
	av_dmx_info_t		*pstDmxInfo,
	av_dmx_outstream_t 	*pstDmxOutStream, //[in, out]
    unsigned char       *pbyOutStreamData //[out]
	);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//  dynamic loading
//
long 
open_library(
    const char	 *pszLibName,		// so
	const char	 *pszLibFuncName,	// api
	void		**ppvLibFunc,
	void		**ppvLibHandle
	);

long 
close_library(
	void	*pvLibHandle
	);


av_result_t 
cdmx_mkv( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_avi( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_audio( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_flv( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_mp4( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_mpg( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_ogg( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_ts( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_asf( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );
av_result_t 
cdmx_ext1( unsigned long ulOpCode, av_handle_t* ptHandle, void* pParam1, void* pParam2 );




















#endif //_CDMX_H_
