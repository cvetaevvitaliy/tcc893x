/* ------------------------------------------------------------------
 * Copyright (C) 2009-2010 Telechips 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */

#ifndef _TCC_CDK_WRAPPER_H_
#define _TCC_CDK_WRAPPER_H_

#include "cdk.h"
#include <tcc_metadata_parser.h>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AHandlerReflector.h>

#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include "stream_queue.h"
#include "MessageQueue.h"
#include "tcc_InterSubtitle.h"

#define USE_HDCP_DECRYPTION //Telechips, ZzaU :: for Secure Input-buffer.

#define INTERNAL_SUBTITLE_INCLUDE
#define PGS_CAPTION_SUPPORT_INCLUDE

//Sync with omx_base_component.h!!
#define VIDEO_BUFFER_SIZE (4 * 1024 * 1024)
#define AUDIO_BUFFER_SIZE 404 * 1024 //30000
#define SUBTITLE_BUFFER_SIZE (512 * 1024) 

#define VIDEO_QUEUE_BUFFER_SIZE   (32 * 1024 * 1024)
#define AUDIO_QUEUE_BUFFER_SIZE   (4 * 1024 * 1024)
#define SUBTITLE_QUEUE_BUFFER_SIZE   (1 * 1024 * 1024)

#define VIDEO_ELEMENT_SIZE     VIDEO_BUFFER_SIZE
#define AUDIO_ELEMENT_SIZE     AUDIO_BUFFER_SIZE 
#define SUBTITLE_ELEMENT_SIZE  SUBTITLE_BUFFER_SIZE 

#define STREAM_TYPE_VIDEO 0
#define STREAM_TYPE_AUDIO 1
#define STREAM_TYPE_TEXT  2
#define STREAM_TYPE_ALL 8
#define STREAM_TYPE_UNKNOWN 9

#define NORMAL_FUNCTIONS              0x00000000
#define FILE_OPEN_ONLY_ONCE_FUNCTIONS 0x00000001
#define STREAMING_SEEK_FUNCTION       0x00000002
#define STREAMING_EOF_FUNCTION        0x00000004

//#define SUPPORT_AAC_ELD_PROFILE		  // support decoding aac-ld/eld profile by using google soft aac decoder component.

#if defined(TCC_8925S_INCLUDE) || defined(TCC_8935S_INCLUDE)
#define MERGE_FRAME_AVG // for defragmentation
#endif

namespace android {

struct AMessage;
struct ALooper;

typedef struct cdk_t
{
	int CDK_SIZE[1024]; //! 1KBytes
	cdk_core_t m_sCore;
} cdk_t;

typedef enum
{
	TCC_CDK_WRAPPER_OK                       = 0,
	TCC_CDK_WRAPPER_ERROR                    = -1,
	TCC_CDK_WRAPPER_NO_SEQUENCE_HEADER       = -2,
	TCC_CDK_WRAPPER_END_OF_STREAM            = -3,
	TCC_CDK_WRAPPER_END_OF_FILE              = -4,
	TCC_CDK_WRAPPER_DEMUXER_ERROR            = -5,
	TCC_CDK_WRAPPER_SEEK_ERROR               = -6,
	TCC_CDK_WRAPPER_INVALID_STREAM_ID        = -7,
	TCC_CDK_WRAPPER_INSUFFICIENT_BUFFER_SIZE = -8,
	TCC_CDK_WRAPPER_SELECT_STREAM_ERROR      = -9,
	TCC_CDK_WRAPPER_SKIP_CORRUPT_PACKET      = -10,
	TCC_CDK_WRAPPER_UNKNOWN_FILE_TYPE        = -11,
	TCC_CDK_WRAPPER_INSUFFICIENT_DATA_SIZE   = -12,
	TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME      = -13,
	TCC_CDK_WRAPPER_PAUSE_REQUEST            = -14,
	TCC_CDK_WRAPPER_WAIT_SEEK_REQUEST        = -15,
	TCC_CDK_WRAPPER_GET_FRAME_CANCELLED      = -16,
	TCC_CDK_WRAPPER_DECRYPTION_FAILED        = -17,
	TCC_CDK_WRAPPER_SECURE_COPY_FAILED       = -18,

	TCC_CDK_WRAPPER_UNKNOWN_ERROR            = -100,
} TCC_CDK_WRAPPER_ERROR_TYPE;

typedef enum
{
	TCC_CDK_WRAPPER_METADATA_ARTIST = 0,
	TCC_CDK_WRAPPER_METADATA_ALBUM,
	TCC_CDK_WRAPPER_METADATA_TITLE,
	TCC_CDK_WRAPPER_METADATA_TRACK,
	TCC_CDK_WRAPPER_METADATA_GENRE,
	TCC_CDK_WRAPPER_METADATA_YEAR,
	TCC_CDK_WRAPPER_METADATA_COMMENT,
	TCC_CDK_WRAPPER_METADATA_AUTHOR,
	TCC_CDK_WRAPPER_METADATA_DESCRIPTION,
	TCC_CDK_WRAPPER_METADATA_RATING,
	TCC_CDK_WRAPPER_METADATA_COMPOSER,
	TCC_CDK_WRAPPER_METADATA_COPYRIGHT
} TCC_CDK_WRAPPER_METADATA;

typedef enum
{
	TCC_CDK_WRAPPER_FIRST_FRAME_GET = 0,	
	TCC_CDK_WRAPPER_FIRST_I_FRAME_GET
} TCC_CDK_WRAPPER_FIRST_VIDEO_FRAME_GET_MODE;

typedef enum
{
	TCC_CDK_WRAPPER_VIDEO_NORMAL_PLAY = 0,
	TCC_CDK_WRAPPER_VIDEO_FRAME_NO_SKIP,	
	TCC_CDK_WRAPPER_VIDEO_FRAME_SKIP,
	TCC_CDK_WRAPPER_VIDEO_FRAME_SKIMMING
} TCC_CDK_WRAPPER_VIDEO_FRAME_SKIP_MODE;

// INTERNAL_SUBTITLE_INCLUDE
typedef enum
{
  TCC_MEDIAEVENT_NONE					= 0,
  TCC_MEDIAEVENT_READY_TO_DISPLAY_TIMEDTEXT,
  TCC_MEDIAEVENT_READY_TO_DISPLAY_PGS,
  TCC_MEDIAEVENT_SYNC_FRAME,
} TCC_MEDIAEVENT ;

#define TCC_CDK_WRAPPER_PARSER_GETMETADATAMODE  0x00000001
#define TCC_CDK_WRAPPER_PARSER_GETTHUMBNAILMODE 0x00000002
#define TCC_CDK_WRAPPER_PARSER_PROGRESSIVE_READ_MODE 0x00000004
#define TCC_CDK_WRAPPER_PARSER_IGNORE_FILE_SIZE 0x00000008
#define TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE  0x00000010
#define TCC_CDK_WRAPPER_PARSER_FRAGMENTED_MP4  	0x00000020

//#define TCC_MIME_FORMAT_UNKNOWN 	"FORMATUNKNOWN"
#define TCC_MIME_VIDEO_UNKNOWN 		"video-unknown"
#define TCC_MIME_VIDEO_NOTSUPPORTED	"video/notsupported"
#define TCC_MIME_AUDIO_UNKNOWN 		"audio-unknown"
#define TCC_MIME_TEXT_UNKNOWN 		"text-unknown"
#define TCC_MIME_H264			  	"video/avc"
#define TCC_MIME_H263	    		"video/3gpp"
#define TCC_MIME_MPEG4         		"video/mp4v-es"// MPEG4 Video
#define TCC_MIME_REAL_VIDEO			"video/vnd.rn-realvideo"
#define TCC_MIME_DIVX				"video/x-divx"
#define TCC_MIME_MPEG2				"video/mpeg2"
#define TCC_MIME_VC1				"video/vc1" // WMV Video
#define TCC_MIME_WMV_1_2			"video/x-wmv-1-2" // WMV7,8 Video
#define TCC_MIME_MJPEG    	     	"video/x-jpeg"
#define TCC_MIME_FLV1  				"video/x-flv"
#define TCC_MIME_AVS  				"video/avs-video"
#define TCC_MIME_VP8                "video/x-vnd.on2.vp8"
#define TCC_MIME_VP9                "video/x-vnd.on2.vp9"
#define TCC_MIME_MVC                "video/x-mvc"

#define TCC_MIME_AMR_NB				"audio/3gpp" // Streaming AMR format, aka IETF_COMBINED_TOC
#define TCC_MIME_AMR_WB             "audio/amr-wb" // AMR Wide Band
#define TCC_MIME_EVRC				"audio/EVRC" // Streaming EVRC format
#define TCC_MIME_QCELP				"audio/qcelp"
#define TCC_MIME_MP3				"audio/mpeg"
#define TCC_MIME_AAC_TCC 		 	"audio/x-mp4a-latm-tcc"
#define TCC_MIME_AC3	      		"audio/ac3"
#define TCC_MIME_TRUEHD	      		"audio/x-true-hd"
#define TCC_MIME_FLAC				"audio/flac"
#define TCC_MIME_APE				"audio/x-ape"
#define TCC_MIME_REAL_AUDIO			"audio/vnd.rn-realaudio"
#define TCC_MIME_MP2				"audio/mpeg2"
#define TCC_MIME_PCM_TCC			"audio/x-pcm"
#define TCC_MIME_MP3_TCC			"audio/x-mpeg-tcc"
#define TCC_MIME_WMA				"audio/x-ms-wma"
#define TCC_MIME_VORBIS				"audio/vorbis"
#define TCC_MIME_DTS            	"audio/vnd.dts"
#define TCC_MIME_DTSHD            	"audio/vnd.dts.hd"
#define TCC_MIME_DDP            	"audio/e-ac3"
#define TCC_MIME_AAC_GOOGLE			"audio/mp4a-latm"

#define TCC_MIME_TEXT_3GPP			"text/3gpp-tt"
#define TCC_MIME_TEXT_SUBRIP		"application/x-subrip"
#define TCC_MIME_TEXT_GRAPHIC		"text/ts"

#define CODEC_VP9 1000


class TCC_CDK_VideoInfo
{
	public:
		TCC_CDK_VideoInfo() :
			iID(-1),
			i_vtype(0),
			iWidth(0),
			iHeight(0),
			iFrameRate(0),
			iBitRate(0),
			pExtraData(NULL),
			iExtraDataLen(0),
			iFourCC(0),
			iRotateDegree(0),
			pCodecExtraData(NULL),
			iCodecExtraDataLen(0)
		{}

		TCC_CDK_VideoInfo(const TCC_CDK_VideoInfo& aSrc) 
		{
			iID = aSrc.iID;
			i_vtype = aSrc.i_vtype;
			mimetype = aSrc.mimetype;
			iWidth = aSrc.iWidth;
			iHeight = aSrc.iHeight;
			iFrameRate = aSrc.iFrameRate;
			iBitRate = aSrc.iBitRate;
			iRotateDegree = aSrc.iRotateDegree;

			if (aSrc.iExtraDataLen > 0 && aSrc.pExtraData != NULL)
			{
				pExtraData = (uint8_t*)malloc(aSrc.iExtraDataLen);
				if ( pExtraData == NULL ) {
					ALOGE( "[Err:%d] malloc failed in %s(%d) \n", -1, __FILE__, __LINE__ );
				}
				//ALOGE("[Line:%04d] pExtraData: 0x%x\n", __LINE__, pExtraData);
				memcpy(pExtraData,	aSrc.pExtraData, aSrc.iExtraDataLen);
			}
			else
			{
				pExtraData = NULL;
			}
			iExtraDataLen = aSrc.iExtraDataLen;
			iFourCC = aSrc.iFourCC;

			if (aSrc.iCodecExtraDataLen > 0 && aSrc.pCodecExtraData != NULL)
			{
				pCodecExtraData = (uint8_t*)malloc(aSrc.iCodecExtraDataLen);
				if ( pCodecExtraData == NULL ) {
					ALOGE( "[Err:%d] malloc failed in %s(%d) \n", -1, __FILE__, __LINE__ );
				}
				//ALOGE("[Line:%04d] pCodecExtraData: 0x%x\n", __LINE__, pCodecExtraData);
				memcpy(pCodecExtraData, aSrc.pCodecExtraData, aSrc.iCodecExtraDataLen);
			}
			else
			{
				pCodecExtraData = NULL;
			}
			iCodecExtraDataLen = aSrc.iCodecExtraDataLen;
		}

		~TCC_CDK_VideoInfo()
		{
			if (pCodecExtraData)
			{
				//ALOGE("[Line:%04d] pCodecExtraData: 0x%x\n", __LINE__, pCodecExtraData);
				free(pCodecExtraData);
				pCodecExtraData = NULL;
			}

			if (pExtraData)
			{
				//ALOGE("[Line:%04d] pExtraData: 0x%x\n", __LINE__, pExtraData);
				free(pExtraData);
				pExtraData = NULL;
			}
		}

		int32_t		iID;
		int32_t		i_vtype;
		String8		mimetype; 
		uint32_t 	iWidth;
		uint32_t	iHeight;
		uint32_t	iFrameRate;
		uint32_t	iBitRate; // bps
		uint8_t*	pExtraData;		// do not free()
		uint32_t	iExtraDataLen;
		uint32_t	iFourCC;
		int32_t		iRotateDegree;
		uint8_t*	pCodecExtraData; // for component init
		uint32_t	iCodecExtraDataLen; // for component init
};

class TCC_CDK_AudioInfo
{
	public:
		TCC_CDK_AudioInfo() : 
			iID(-1), 
			i_atype(0), 
			iSamplesPerSec(0), 
			iBitsPerSample(0),
			iBitRate(0),
			iChannels(0), 
			iFormatId(0), 
			iAvgBytesPerSec(0), 
			pCodecExtraData(NULL),
			iCodecExtraDataLen(0),
			pszLanguageInfo(0) {
		}

		TCC_CDK_AudioInfo(const TCC_CDK_AudioInfo& aSrc)
		{
			iID = aSrc.iID;
			i_atype = aSrc.i_atype;
			mimetype = aSrc.mimetype;
			iSamplesPerSec = aSrc.iSamplesPerSec;
			iBitsPerSample = aSrc.iBitsPerSample;
			iBitRate = aSrc.iBitRate;
			iChannels = aSrc.iChannels;
			iFormatId = aSrc.iFormatId;
			iAvgBytesPerSec = aSrc.iAvgBytesPerSec;
			pszLanguageInfo = aSrc.pszLanguageInfo;
			
			if (aSrc.iCodecExtraDataLen > 0 && aSrc.pCodecExtraData != NULL)
			{
				pCodecExtraData = (uint8_t*)malloc(aSrc.iCodecExtraDataLen);
				if ( pCodecExtraData == NULL ) {
					ALOGE( "[Err:%d] malloc failed in %s(%d) \n", -1, __FILE__, __LINE__ );
				}
				//ALOGE("[Line:%04d] pCodecExtraData: 0x%x\n", __LINE__, pCodecExtraData);
				memcpy(pCodecExtraData, aSrc.pCodecExtraData, aSrc.iCodecExtraDataLen);
			}
			else
			{
				pCodecExtraData = NULL;
			}
			iCodecExtraDataLen = aSrc.iCodecExtraDataLen;
		}

		~TCC_CDK_AudioInfo()
		{
			if (pCodecExtraData)
			{
				//ALOGE("[Line:%04d] pCodecExtraData: 0x%x\n", __LINE__, pCodecExtraData);
				free(pCodecExtraData);
				pCodecExtraData = NULL;
			}
		}
		
		int32_t		iID;
		int32_t		i_atype;
		String8		mimetype;
		uint32_t	iSamplesPerSec;
		uint32_t	iBitsPerSample;
		uint32_t	iBitRate; // bps, not kbps
		uint32_t	iChannels;
		uint32_t	iFormatId;
		uint32_t	iAvgBytesPerSec;
		uint8_t*	pCodecExtraData; // for component init
		uint32_t	iCodecExtraDataLen; // for component init
		char*		pszLanguageInfo;
};

class TCC_CDK_SubtitleInfo
{
	public:
		TCC_CDK_SubtitleInfo() : 
			iID(-1), 
			i_stype(0),
			iMaxInputSize(0),			
			pszLanguageInfo(0) 
		{}

		TCC_CDK_SubtitleInfo(const TCC_CDK_SubtitleInfo& aSrc) 
		{
			iID = aSrc.iID;
			i_stype = aSrc.i_stype;
			mimetype = aSrc.mimetype;
			iMaxInputSize = aSrc.iMaxInputSize;
			pszLanguageInfo = aSrc.pszLanguageInfo;
		}

		~TCC_CDK_SubtitleInfo()
		{
		}

		int32_t		iID;
		int32_t		i_stype;
		String8		mimetype;
		uint32_t	iMaxInputSize;
		char*		pszLanguageInfo;
};

class TCC_CDK_FileInfo
{
	public:
		TCC_CDK_FileInfo() :
			iDuration(0),
			iBitRate(0),
			iTimeScale(0),
			iNumStreams(0),
			iNumVidStreams(0),
			iNumAudStreams(0),
			iNumSubtitleStreams(0),
			iContainerType(0),
			iSeekable(false),
			bHasIndex(false),
			m_pcTitle(0),
			m_pcArtist(0),
			m_pcAlbum(0),
			m_pcAlbumArtist(0),
			m_pcYear(0),
			m_pcWriter(0),
			m_pcAlbumArt(0),
			m_iAlbumArtSize(0),
			m_pcGenre(0),
			m_pcCompilation(0),
			m_pcCDTrackNumber(0),
			m_pcDiscNumber(0),
			m_pcLocation(0){
		}

		uint32_t	iDuration;
		uint32_t	iBitRate; //kbps
		uint32_t	iTimeScale;
		uint32_t	iNumStreams;
		uint32_t	iNumVidStreams;
		uint32_t	iNumAudStreams;
		uint32_t	iNumSubtitleStreams;
		uint8_t		iContainerType;
		bool		iSeekable;
		bool		bHasIndex;
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
		int			m_iFragmentedMp4;
};

typedef void* CDK_ITEM_PTR;

class TCC_CDK_Wrapper
{
	public:
		enum { 
            FILE_SOURCE_SCHEME = 0,
            HTTP_SOURCE_SCHEME,
            RTSP_SOURCE_SCHEME,
            HDCP2_SOURCE_SCHEME
        };

        enum { 
		       CDK_GETSTREAM_SELECTIVE_MODE  = 1,
		       CDK_GETSTREAM_SEQUENTIAL_MODE = 2, 
		       DEMUXER_THREAD_ENABLED_MODE   = 4,
		       DEMUXER_LOOPER_ENABLED_MODE   = 8 
		};

        enum { 
            SKIP_OPERATION_NONE     = 0x00000000,
            SKIP_OPERATION_PAUSE    = 0x00000001,
            SKIP_OPERATION_SEEK     = 0x00000002,
            SKIP_OPERATION_RESUME   = 0x00000004 
		};

		TCC_CDK_Wrapper();
		virtual ~TCC_CDK_Wrapper();

		int32_t CDKClose();
		int32_t CDKCloseCore();

		// Clear Audio stream-queue when audio-selection is run (for sequential mode demuxers)
		void ClearAudioBuffer();  

		virtual int32_t SetCallbackFunctions(uint32_t aType) = 0; 
		virtual void GetCurrentFileSize(uint64_t& aSize) = 0;

		// Parsing container's info
        int32_t ContainerParse(uint32_t aMode = 0, int32_t aSourceScheme = FILE_SOURCE_SCHEME, const char *pszExt = NULL);
		
		// Retrieve stream data
		int32_t GetStreamData(uint32_t aType, uint8_t* arBuffer, uint32_t& aSize, uint32_t& aTimeStamp, uint32_t& aExtra, int32_t &iParam, int32_t &iPacketDisCont );

		// to support fast video skip mode
		// aRate = 10 usec scale
		void SetVideoSkipModeOptions(uint32_t aMode, float aRate);
		
		Vector<TCC_CDK_VideoInfo> * GetVideoInfo();
		Vector<TCC_CDK_AudioInfo> * GetAudioInfo();
		Vector <TCC_CDK_SubtitleInfo> *GetSubtitleInfo();
		TCC_CDK_FileInfo*	GetFileInfo();

		// to support multi stream media
		int32_t SelectVideoStream(int32_t aID);
		int32_t SelectAudioStream(int32_t aID);	
		int32_t SelectSubtitleStream(int32_t sID);	
		int32_t GetCurrentSubtitleStream(void);
		char* GetLangSubtitleStream(void);
		int32_t PrepareDemuxerWithAppropriateMode(int64_t prefillUs = 0);
		int32_t PrepareDemuxerWithSelectiveMode(bool *pbModeChanged = NULL);
		bool IsPrepared() { return bPrepared; };
		void ResetFirstFrameFlag(); // related with stream select

		void Reset();
		String16* GetMetadataString(int32_t key);
		bool ExistMetadataValue(int32_t key);

		int32_t IsMP2File();
		int32_t IsAACFile();

		void feedMore(int64_t videoThresholdUs = 3000000ll, 
                      int64_t audioThresholdUs = 3000000ll);
		int32_t checkQueuedDurationUs(int64_t *durationUs, bool *fulled = NULL);
		void cancelSeekIfNecessary(bool cancelled);
		void signalUnderrun(bool underrun, int64_t thresholdUs = 0);

		// INTERNAL_SUBTITLE_INCLUDE
		void SetSentSubtitleEvent(bool value);
		bool GetSentSubtitleEvent(void);
		void SetDetectIntSubtitle(bool value);
		bool GetDetectIntSubtitle(void);
		int32_t GetFirstSubtitle(void);
		int32_t SetSubtitleSeek(int msec);
		int SetCurrentPlaybackPosition(int msec);
		void SetSentPGSEvent(bool value);
		bool GetSentPGSEvent(void);
		void set_inter_subtitle_type(int32_t type);
		int32_t get_inter_subtitle_type(void);
		void PushSubtitleData(void);

		int32_t GetSubtitleType(void);  //  External Or Internal ??
		inline uint32_t GetAudPacketNum() const { return iAudPacketNum; };
		inline uint32_t GetVidPacketNum() const { return iVidPacketNum; };

		int32_t iGetStreamMode;
		bool bIsUnderSkimming;//JS Baek
		inline int32_t GetLastVideoPTS() const { return mLastVideoPTS; };

		int32_t GetLastKeyFramePTS();

        int32_t MediaSeekCore(uint32_t targetTS);
        void ClearStreamBuffer(int32_t packet_num);
        void SkipAudioStream(void);
        uint32_t GetVideoBufferedTime(void);
        int32_t FillStreamQueues(bool *queueFulled = NULL);
        bool CheckBufferingConditionByTime(uint32_t& aVideoTime, uint32_t& aAudioTime);
        bool CheckBufferingConditionByClipTime(uint32_t& aTargetClipNum, uint32_t& aClipTime);

#ifdef USE_HDCP_DECRYPTION
		bool GetDecryptionNeed(void);
#endif

	protected:
		void CDKInit();
		int callback_func_init( cdk_callback_func_t* psFunc );
		virtual int32_t MediaSeek(uint32_t targetTS, uint32_t aOperationMakrker = 0);

		virtual void PrintError(const char *fmt, ...) {}
		virtual void PrintInfo(const char *fmt, ...) {}

		void*					iFilePtr;
		char					pFilename[512];
		cdk_callback_func_t*	pCallback;
		cdk_t					iCdk;

		TCC_CDK_FileInfo		iFileInfo;

	private:
		int32_t CreateCdmxInstance(unsigned long ulCdmxType);
		int32_t ResetCdmxInstance(void);
		int32_t DeleteCdmxInstance(void);
		int32_t CopyMediaInfo( unsigned long ulParsingMode, void* pvStrmIdList );
		String8	ConvertVidTypeToMimeType(unsigned long ulType);
		String8	ConvertAudTypeToMimeType(unsigned long ulType);
		String8	ConvertSubTypeToMimeType(unsigned long ulType);
		int32_t InitCdmx (unsigned long ulParsingMode = 0);
		int32_t CloseCdmx (void);
		int32_t SelectCdkDemuxMode(void);

		int32_t LoadExternalSetting();
		int32_t LoadParserLibrary(int32_t ctype);
		void GetMIMEType();
		void ConfirmMediaInfo();
		int identify_audio_get_stream(void* pCDK, unsigned int loopCount);
		int identify_audio_seek_zero(void* pCDK);
		int identify_ac3_stream_sync(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize);
		int identify_mpeg_stream(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize, int iDmxType, int iAudioType);
		int identify_dts_stream_sync(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize);
		int Recheck_AC3_DTS(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize, int iDmxType, int type);
		int Recheck_MP123(void* pCDK, unsigned char* pucBitstreamBufAddr, int iBitstreamBufSize, int iDmxType, int type);
		int get_input_container_type(char* pFileName, const char *pszExt = NULL);
		int get_video_codec_type( int iFourCC );
		int get_audio_codec_type( void* pCDK, int iFourId, int iDmxType, uint32_t iOpenMode );
		void ReadMetadata();
		void CDMXErrorProcess(int32_t& ret, int32_t packetNum = -1);
		int CheckAudioCodecUse(int32_t iCheckAll, int32_t iCheckIndex);

		int32_t CheckProperty(unsigned long ulType, const char *pszExt, const char *pszDef = "1" );
		void MakeAACSpecificConfigData(uint8_t *config, uint32_t profile, uint32_t samplerate, uint32_t numChannel);

	#ifdef USE_HDCP_DECRYPTION
        int32_t Stream_Decryption_process(const uint8_t * inData, uint8_t * outData, uint32_t& dataSize, uint32_t streamCtr, uint64_t inputCtr);
		int32_t Stream_Copy_process(const uint8_t * inData, uint8_t * outData, uint32_t& dataSize);
	#endif
		bool                    mProfilingMode;
        int32_t                 mSourceScheme;
		int32_t                 i_ctype;
		int32_t                 iAudID; // in wrapper 
		int32_t                 iVidID; // in wrapper
		int32_t                 iSubID; // in wrapper

		int32_t                 iAudPacketNum; 
		int32_t                 iVidPacketNum;
		int32_t                 iPacketNumUnderrun;

		int32_t                 audio_end;
		int32_t                 video_end;
		int32_t                 codec_exist;
		bool                    bSentFirstVideoFrame;

		Vector<TCC_CDK_VideoInfo>		iVidInfo;
		Vector<TCC_CDK_AudioInfo>		iAudInfo;
		Vector<TCC_CDK_SubtitleInfo>	iSubInfo;

		int32_t* 	iVidIDList;
		int32_t* 	iAudIDList;
		int32_t* 	iSubIDList;
		int32_t 	iDefaultVidStreamID; // in parser
		int32_t 	iDefaultAudStreamID; // in parser
		int32_t 	iDefaultSubStreamID; // in parser
		
		String16	sMetadataArtist;
		String16	sMetadataAlbum;
		String16	sMetadataTitle;
		String16	sMetadataTrack;
		String16	sMetadataGenre;
		String16	sMetadataComment;
		String16	sMetadataAuthor;
		String16	sMetadataDescription;
		String16	sMetadataRating;
		String16	sMetadataComposer;
		String16	sMetadataCopyright;
		String16	sMetadataYear;

		uint8_t*				pStreamBuffer;
		CDK_ITEM_PTR			pItem;

		long (*m_pfnCdmx) ( unsigned long, unsigned long*, void*, void* );
		void  *m_pvCdmxInst;
		unsigned long m_ulCdmxInstSize;

		bool					iGetMetadataMode;
		bool					iGetThumbnailMode;
		bool					bFirstGetStreamAfterSeek;
        bool					mSeekCancelled;
		uint8_t					ipParserLibExist[11];

		// INTERNAL_SUBTITLE_INCLUDE, for subtitle event, it will occur once for a file.
		tccInterSubtitle        m_InterSubtitle;
		bool		            bSentSubtitleEvent;
		bool		            bDetectIntSubtitle;
		bool					bSentPGSEvent ;
		int32_t					mInterSubtitleType;

		int32_t					mLastVideoPTS;
		int32_t					mLastAudioPTS;
		bool					bIsQueueEmpty;
		bool					bIsWFDPlayMode;
		bool					bIsDecryptionNeeded;
		int32_t					mEnableHdcp2;
		bool                    bPrepared;
#ifdef MERGE_FRAME_AVG
		// for Defragmentation
		int32_t DefragmentFrame ( uint8_t*	pTempBuff,
								  StreamQueue*	pStreamQueue, 
								  uint8_t*  arBuffer, 
								  uint32_t& aSize, 
								  uint32_t& aTimeStamp, 
								  uint32_t& aExtra );

		int32_t DefragmentFrame_Selective ( uint8_t*	pTempBuff,
								  void* CDK, 
								  uint8_t*  arBuffer, 
								  uint32_t& aSize, 
								  uint32_t& aTimeStamp, 
								  uint32_t& aExtra );
		long	m_lPrevStatus;
		long	m_lSpsPpsDect;
		int32_t MergeVideoFrameAVC ( unsigned char *pbyDst, long lDstBuffSize,
									 unsigned char *pbySrc, long lSrcDataSize,
									 long* plUsedBytes, long* plReadBytes, long *pbUsingAUD );
		int m_after_seq;
		int m_after_pic;
		int32_t TSD_GetMpegFrameStartPos(unsigned char *pbyDst, int lDstBuffSize,
										 unsigned char *pbySrc, int lSrcDataSize,
										 long *plUsedBytes, long *plReadBytes,
										 unsigned long picture_header_type,
										 unsigned long picture_header_type2,
										 unsigned long sequence_header_type);
		unsigned long 	m_ulPreviousTime;
		unsigned long m_ulCurrentTime;
		uint8_t*		m_pbyTempBuffBackup;
		bool			m_bSeekFlag;
		bool			m_bMultiplePacketFrame;
		bool			m_bQueueEmpty;

		//add for test 20121016 skimming
		long m_lFrame_complete;
		long m_lUsed_bytes;
		long m_lRead_bytes;
		long m_lFrame_size;
		unsigned long m_ulSync;

		long	m_lStartPos;
		long	m_lEndPos;
		long	m_lDataLen;
		long	m_lAudUsed;

		unsigned long m_ulSeqHeaderType;
		unsigned long m_ulPicHeaderType;
		unsigned long m_ulPicHeaderType2;	//for AVS PB frame
		long m_lVideoCodecType;
		uint8_t m_MergeFrame_Temp[4*1024*1024];
#endif//MERGE_FRAME_AVG

		KeyedVector<uint32_t, StreamQueue*>     iDemuxStreamQueues;

		typedef bool (TCC_CDK_Wrapper::*buffering_check_func_ptr) (uint32_t&, uint32_t&);
		buffering_check_func_ptr fpCheckBufferingCondition;

		bool CheckIfMoreBufferingIsNeeded(uint32_t& aParam1, uint32_t& aParam2);
		bool CheckBufferingConditionByCount(uint32_t& aAmount, uint32_t& iCount);
		bool DoNotCheckBufferingCondition(uint32_t& aParam1, uint32_t& aParam2);

        void SendReqMsgToFeedDataIfUnderruned(uint32_t aDemuxerProcCmd, uint32_t index = 0, uint32_t aParam1 = 0, uint32_t aParam2 = 0, uint32_t aCmdOption = TCCDemuxerThread::MSG_SYNC_OFF);
        void PrintHexData(unsigned int type, unsigned int process, unsigned char* p);

		bool SanityCheckBufferSpaceAvailability();

        // Retrieve sequential stream data
        int32_t FillStreamQueues(uint32_t op_code, uint32_t& aParam1, uint32_t& aParam2, int32_t packet_num = 0, uint8_t* pPacketBuffer = NULL, secure_info_t *pstSecureInfo = NULL);
		int32_t FillStreamQueue(uint32_t op_code, uint32_t& aParam1, uint32_t& aParam2);

        void createAndRunDemuxerThread();
        void destroyDemuxerThread();

        bool DoesPrevReqMsgToFeedDataRemain(uint32_t aCmdOption = TCCDemuxerThread::MSG_SYNC_OFF);

        int demuxerThread();
		class TCCDemuxerThread : public Thread 
		{
		public:
			enum DemuxerCmdSyncOption { MSG_SYNC_OFF, MSG_SYNC_ON };
			enum DemuxerThreadState { STATE_READY, STATE_PAUSED, STATE_RUNNING, STATE_FINISHED };
			enum DemuxerThreadCmd { CMD_FEED, CMD_FEEDMORE, CMD_PAUSE, CMD_SEEK, CMD_RESUME, CMD_CLOSE };

			TCCDemuxerThread(TCC_CDK_Wrapper* hw) : Thread(false), mCDKWrapper(hw) { 
				mDemuxerThreadState = STATE_READY;

				pDemuxerReqCmdQ = new MessageQueue();
				pDemuxerRespCmdQ = new MessageQueue();

				iCommandQueues.add(MessageQueue::REQ, pDemuxerReqCmdQ);
				iCommandQueues.add(MessageQueue::RESP, pDemuxerRespCmdQ);
			}

			virtual bool threadLoop();

			void FeedData(uint32_t aCommand, int32_t aParam1, int32_t aParam2, int32_t aParam3, 
			              int32_t aCmdOption = TCCDemuxerThread::MSG_SYNC_OFF);
			void Pause(int32_t aCmdOption = TCCDemuxerThread::MSG_SYNC_OFF);
			int32_t SeekTo(int32_t targetTS, int32_t aCmdOption = TCCDemuxerThread::MSG_SYNC_ON);
			void Resume(uint32_t aDemuxerProcCmd, int32_t aParam1, int32_t aParam2, 
			            int32_t aCmdOption = TCCDemuxerThread::MSG_SYNC_OFF);
			int32_t Close(int32_t aCmdOption);

			inline DemuxerThreadState GetState() const { return mDemuxerThreadState; };
			inline void SetState(DemuxerThreadState nState) { mDemuxerThreadState = nState; };

			KeyedVector<uint32_t, MessageQueue*> iCommandQueues;

		private:
			TCC_CDK_Wrapper* mCDKWrapper;
			MessageQueue*	pDemuxerReqCmdQ;  
			MessageQueue*	pDemuxerRespCmdQ;
			DemuxerThreadState mDemuxerThreadState;	
		};

        sp<TCCDemuxerThread> mDemuxerThread;
        mutable Mutex       mLock;

        uint32_t    mDemuxerGetFrameReqCnt;
        uint32_t    mDemuxerGetFrameRespCnt;

    //////////////////////////////////////////////////////////////////////////////////////////
	class CDKDemuxHandler : public AHandler {
	public:
        CDKDemuxHandler(TCC_CDK_Wrapper* owner) 
            : mOwner(owner), mLooper(new ALooper),
			  mPausePending(false),
			  mPaused(false),
			  mPrefilled(false),
			  mFeedPending(false),
              mResumePending(false),
			  mStopped(false),
              mResumed(false),
              mUnderrun(false),
              mThreshold(kESLowWatermarkUs),
			  mApproxFeedCount(10) {
            mLooper->setName("CDKDemuxHandler"); 
		}

        static const size_t kESMinWatermarkUs  = 1000000ll;
        static const size_t kESLowWatermarkUs  = 2000000ll;
        static const size_t kESHighWatermarkUs = 5000000ll;

		bool isAlive();
		void signalUnderrun(bool underrun, int64_t thresholdUs = 0);
		void start(int32_t prefillTimeMs = 0);
		void stop();
		void feedOnce(int32_t feedMs = 0);
		void pause();
		void resume(int32_t prefillMs = 0);
		
		int32_t seekTo(int32_t targetTS);

	protected:
        virtual void onMessageReceived(const sp<AMessage> &msg);
        virtual ~CDKDemuxHandler() {
            mLooper->stop();
            mLooper->unregisterHandler(id());
            mLooper.clear();
        }

	private:
        enum {
		    kWhatStart = 'staR',
		    kWhatStop = 'stoP',
		    kWhatFeed = 'feeD',
		    kWhatFeedOnce = 'fdoC',
		    kWhatPause = 'pauS',
		    kWhatSeek = 'seeK',
		    kWhatResume = 'resU',
        };

		friend struct AHandlerReflector<CDKDemuxHandler>;

        TCC_CDK_Wrapper* mOwner;
        sp<ALooper> mLooper;

		Mutex mHandlerLock;
		Mutex mSerializer;
		Condition mHandlerCondition;
        
		sp<AMessage> mAsyncResult;

		bool mPausePending;
		bool mPaused;
		bool mPrefilled;
		bool mFeedPending;
		bool mStopped;
        bool mResumePending;
        bool mResumed;
        bool mUnderrun;

		int64_t mThreshold;
		int32_t mApproxFeedCount;

		DISALLOW_EVIL_CONSTRUCTORS(CDKDemuxHandler);
	};

	sp<CDKDemuxHandler> mCDKDemuxHandler;
    //////////////////////////////////////////////////////////////////////////////////////////
};

}  // namespace android

#endif // _TCC_CDK_WRAPPER_H_

