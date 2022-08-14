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

#include "cdk_wrapper_stagefright.h"
#include "utils/Log.h"
#include "media_io_wrapper_stagefright.h"

#undef	LOG_TAG
#define LOG_TAG	"TCCCDKWrapper"

//#define DEBUG

// -----------------------------------------------------------------
// class member functions
// -----------------------------------------------------------------


// constructor
CDKWrapperStagefright::CDKWrapperStagefright() :
	TCC_CDK_Wrapper()
	,mFile(NULL)
	,mEnableLogging(false)
	,mAudioSeekDone(false)
	,mVideoSeekDone(false)
	,mAudioTrackAvail(false)
	,mVideoTrackAvail(false)
#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
	,mAudioBuffer(NULL)
	,mVideoBuffer(NULL)
	,mLastAudioTime(0)
	,mLastVideoTime(0)
#endif
{
}

// destructor
CDKWrapperStagefright::~CDKWrapperStagefright()
{
	if (mFile) {
		delete mFile;
		mFile = NULL;
	}

#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
	if (mAudioBuffer) {
		delete[] mAudioBuffer;
	}

	if (mVideoBuffer) {
		delete[] mVideoBuffer;
	}
#endif
}

int32_t CDKWrapperStagefright::SetCallbackFunctions(uint32_t aType)
{
	int32_t error = 0;

	if ((pCallback = new cdk_callback_func_t()) == NULL) {
		return TCC_CDK_WRAPPER_ERROR;
	}

    pCallback->m_pfMalloc           = (void* (*) ( unsigned int ))malloc;
    pCallback->m_pfFree             = (void  (*) ( void* ))free;
    pCallback->m_pfNonCacheMalloc   = (void* (*) ( unsigned int ))malloc;
    pCallback->m_pfNonCacheFree     = (void  (*) ( void* ))free;
    pCallback->m_pfMemcpy           = (void* (*) ( void*, const void*, unsigned int ))memcpy;
    pCallback->m_pfMemset           = (void* (*) ( void*, int, unsigned int ))memset;
    pCallback->m_pfRealloc          = (void* (*) ( void*, unsigned int ))realloc;
    pCallback->m_pfMemmove          = (void* (*) ( void*, const void*, unsigned int ))memmove;
    pCallback->m_pfMemcmp           = (int (*) ( const void*, const void*, unsigned int ))memcmp;

    pCallback->m_pfPhysicalAlloc    = NULL;
    pCallback->m_pfPhysicalFree     = NULL;
    pCallback->m_pfVirtualAlloc     = NULL;
    pCallback->m_pfVirtualFree      = NULL;

    pCallback->m_pfFopen            = (void* (*) (const char *, const char *))WRAPPER_fopen;
    pCallback->m_pfFread            = (unsigned int (*) (void*, unsigned int, unsigned int, void* ))WRAPPER_fread;
    pCallback->m_pfFseek            = (aType & STREAMING_SEEK_FUNCTION)
                                    ? (int  (*) (void*, long, int ))WRAPPER_sseek
                                    : (int  (*) (void*, long, int ))WRAPPER_fseek;
    pCallback->m_pfFtell            = (long (*) (void* ))WRAPPER_ftell;
    pCallback->m_pfFwrite           = (unsigned int (*) (const void*, unsigned int, unsigned int, void*))fwrite;
    pCallback->m_pfFclose           = (int  (*) (void *))WRAPPER_fclose;
    pCallback->m_pfUnlink           = NULL;
    pCallback->m_pfFeof             = (aType & STREAMING_EOF_FUNCTION) 
                                    ? (unsigned int (*) ( void * ))WRAPPER_seof
                                    : (unsigned int (*) ( void * ))WRAPPER_feof;
    pCallback->m_pfFflush           = (unsigned int (*) ( void * ))WRAPPER_fflush;
    pCallback->m_pfFseek64          = (aType & STREAMING_SEEK_FUNCTION)
                                    ? (int (*) (void*, int64_t, int ))WRAPPER_sseek64
                                    : (int (*) (void*, int64_t, int ))WRAPPER_fseek64;
    pCallback->m_pfFtell64          = (int64_t (*) (void * ))WRAPPER_ftell64;
	
	// set io files
	iCdk.m_sCore.m_psCallback = pCallback;
	if (callback_func_init(pCallback) < 0) {
		PrintError("cdk_func_init Error !!\n"); 
		return TCC_CDK_WRAPPER_ERROR;
	}

	return TCC_CDK_WRAPPER_OK;
}

int32_t CDKWrapperStagefright::CDKInit(const sp<DataSource> &source)
{
	/////////////////////////////////////////////////
	// start initializing
	/////////////////////////////////////////////////
	
	TCC_CDK_Wrapper::CDKInit();

	mSource = source;
	mFile = new SF_MEDIA_FILE();
	mFile->offset = 0;
	mFile->file = mSource;
	if (mFile->file->getSize(&mFile->size) != OK)
		mFile->size = 0;

	iFilePtr = reinterpret_cast<void*>(mFile);

	// set open file
	memset(pFilename, 0, 512);
	sprintf(pFilename, "%x", reinterpret_cast<unsigned int>(iFilePtr)); // convert hex value of address for file pointer to string
	iCdk.m_sCore.m_pcOpenFileName = pFilename;

	ReadLogSetting();

#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
	if (!mAudioBuffer) {
		mAudioBuffer = new uint8_t[AUDIO_BUFFER_SIZE];
	}

	if (!mVideoBuffer) {
		mVideoBuffer = new uint8_t[VIDEO_BUFFER_SIZE];
	}
#endif

	return TCC_CDK_WRAPPER_OK;
}

int32_t CDKWrapperStagefright::Seek(uint32_t type, uint32_t targetTS)
{
	Mutex::Autolock autoLock(mLock);

	if (mVideoTrackAvail == true && mAudioTrackAvail == true) {
		if( type == STREAM_TYPE_VIDEO ) {
			if( mVideoSeekDone == false ) {
				if( mAudioSeekDone == true ) {
					mAudioSeekDone = false;
					return TCC_CDK_WRAPPER_OK;
				}
				mVideoSeekDone = true;
			}
		}
		else if( type == STREAM_TYPE_AUDIO ) {
			if( mAudioSeekDone == false ) {
				if( mVideoSeekDone == true ) {
					mVideoSeekDone = false;
					return TCC_CDK_WRAPPER_OK;
				}
				mAudioSeekDone = true;
			}
		}
		else if( type == STREAM_TYPE_TEXT ) {
			// None
		}
		else
			return TCC_CDK_WRAPPER_SEEK_ERROR;
	}

	int32_t ret;
#ifndef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
	ret = MediaSeek(targetTS);
#else
	bool forward;

	if (mVideoTrackAvail) {
		// video file
		if (targetTS >= mLastVideoTime && mLastVideoTime > 0) {
			forward = true;
		} else {
			forward = false;
		}

		if(forward)
			ret = MediaSeek(targetTS, SKIP_OPERATION_RESUME);
		else
			ret = MediaSeek(targetTS);

		if (ret != TCC_CDK_WRAPPER_OK || !forward) {
			return ret;
		}
	} else {
		// audio file
		ret = MediaSeek(targetTS);
		return ret;
	}

	uint32_t size;
	uint32_t time;

	// jump to the nearest front key frame of requested position
	size = VIDEO_BUFFER_SIZE;
	ret = PeekData(STREAM_TYPE_VIDEO, size, time);

	if (ret == TCC_CDK_WRAPPER_OK) {
		ret = MediaSeek(time-1, SKIP_OPERATION_PAUSE);
	}
#endif

	return ret;
}

#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
int32_t CDKWrapperStagefright::PeekData(
		uint32_t aType, 
		uint32_t& aSize, 
		uint32_t& aTimeStamp,
		int32_t& iPacketDisCont)
{
	int32_t ret = TCC_CDK_WRAPPER_ERROR;
	uint32_t extra = SKIP_OPERATION_RESUME;
	int32_t param;

	if (aType == STREAM_TYPE_AUDIO) {
		ret = GetStreamData(aType, mAudioBuffer, aSize, aTimeStamp, extra, param, iPacketDisCont);
		mLastAudioTime = aTimeStamp;
	} else if (aType == STREAM_TYPE_VIDEO) {
		ret = GetStreamData(aType, mVideoBuffer, aSize, aTimeStamp, extra, param, iPacketDisCont);
		mLastVideoTime = aTimeStamp;
	}

	return ret;
}
#endif

int32_t CDKWrapperStagefright::GetData(
		uint32_t aType, 
		uint8_t* arBuffer, 
		uint32_t& aSize, 
		uint32_t& aTimeStamp, 
		uint32_t& aExtra, 
		int32_t &iParam,
		int32_t& iPacketDisCont)
{
	Mutex::Autolock autoLock(mLock);
 
#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
	int32_t ret;
	
	ret = GetStreamData(aType, arBuffer, aSize, aTimeStamp, aExtra, iParam, iPacketDisCont);

	if (aType == STREAM_TYPE_AUDIO) {
		mLastAudioTime = aTimeStamp;
	} else if (aType == STREAM_TYPE_VIDEO) {
		mLastVideoTime = aTimeStamp;
	}

	return ret;
#else
	if( aType == STREAM_TYPE_VIDEO ) {
		if( mAudioSeekDone == true ) {
			return TCC_CDK_WRAPPER_WAIT_SEEK_REQUEST;
		}
	}
	else if( aType == STREAM_TYPE_AUDIO ) {
		if( mVideoSeekDone == true ) {
			return TCC_CDK_WRAPPER_WAIT_SEEK_REQUEST;
		}
	}
	else if( aType == STREAM_TYPE_TEXT ) {
		if( mAudioSeekDone == true || mVideoSeekDone == true) {
			return TCC_CDK_WRAPPER_WAIT_SEEK_REQUEST;
		}
	}
	else
		return TCC_CDK_WRAPPER_SEEK_ERROR;

	return GetStreamData(aType, arBuffer, aSize, aTimeStamp, aExtra, iParam, iPacketDisCont);
#endif
}

void CDKWrapperStagefright::SetAvailableTrack(uint32_t type)
{
	if (type == STREAM_TYPE_VIDEO)
	{
		mVideoTrackAvail = true;
	}
	else if (type == STREAM_TYPE_AUDIO)
	{
		mAudioTrackAvail = true;
	}
}

void CDKWrapperStagefright::GetCurrentFileSize(uint64_t& aSize)
{
	off64_t size;

	if (mSource->getSize(&size) != OK)
	{
		size = 0;
	}
	aSize = static_cast<uint64_t>(size);
}

void CDKWrapperStagefright::PrintError(const char* fmt, ...)
{
	if (mEnableLogging)
	{
		va_list arg;
		va_start(arg, fmt);
		LOG_PRI_VA(ANDROID_LOG_DEBUG, LOG_TAG, fmt, arg);
		va_end(arg);
	}
}

void CDKWrapperStagefright::PrintInfo(const char* fmt, ...)
{
	if (mEnableLogging)
	{
		va_list arg;
		va_start(arg, fmt);
		LOG_PRI_VA(ANDROID_LOG_INFO, LOG_TAG, fmt, arg);
		va_end(arg);
	}
}

void CDKWrapperStagefright::ReadLogSetting()
{
#ifdef DEBUG
	FILE *fp;
	fp = fopen("/sdcard/pvlogger.txt", "r");

	if (fp == NULL)
	{
		fp = fopen("/sdcard/sdcard/pvlogger.txt", "r");
	}

	if (fp == NULL)
	{
		fp = fopen("/mnt/nand/sdcard/pvlogger.txt", "r");
	}

	if (fp != NULL)
	{
		char buf[1024];

		while (0 < fscanf(fp, "%s", buf))
		{
			if (buf[0] == '8')
			{
				if (!strncmp(buf+2, "TCCCDKWrapper", 13))
				{
					mEnableLogging = true;
				}
			}
		}

		fclose(fp);
	}
#endif
}

// INTERNAL_SUBTITLE_INCLUDE
int CDKWrapperStagefright::GetSubtitleType(void)
{
	int ret = TCC_CDK_Wrapper::GetSubtitleType() ; 
	return ret; 
}
int CDKWrapperStagefright::SelectSubtitleStream(int num)
{
	return TCC_CDK_Wrapper::SelectSubtitleStream( num ) ; 
}

int CDKWrapperStagefright::GetCurrentSubtitleStream(void)
{
	return TCC_CDK_Wrapper::GetCurrentSubtitleStream();
}

int CDKWrapperStagefright::SetCurrentPlaybackPosition(int msec)
{
	return TCC_CDK_Wrapper::SetCurrentPlaybackPosition( msec ) ; 
}

int32_t CDKWrapperStagefright::GetFirstSubtitle()
{
	return TCC_CDK_Wrapper::GetFirstSubtitle(); 
}

int32_t CDKWrapperStagefright::SetSubtitleSeek(int msec)
{
	return TCC_CDK_Wrapper::SetSubtitleSeek(msec); 
}

int32_t CDKWrapperStagefright::get_inter_subtitle_type()
{
	return TCC_CDK_Wrapper::get_inter_subtitle_type(); 
}
int32_t CDKWrapperStagefright::numFileStream()
{
	return TCC_CDK_Wrapper::GetFileInfo()->iNumVidStreams + 
			TCC_CDK_Wrapper::GetFileInfo()->iNumAudStreams;
}
void CDKWrapperStagefright::ClearAudioBuffer()     
{
	TCC_CDK_Wrapper::ClearAudioBuffer();
}
void CDKWrapperStagefright::ClearStreamBuffer()
{
	TCC_CDK_Wrapper::ClearStreamBuffer(STREAM_TYPE_ALL);
}
void CDKWrapperStagefright::SkipAudioStream()
{
	TCC_CDK_Wrapper::SkipAudioStream();
}

uint32_t CDKWrapperStagefright::GetVideoBufferedTime()
{
	return TCC_CDK_Wrapper::GetVideoBufferedTime();
}

int32_t CDKWrapperStagefright::GetLastKeyFramePTS()
{
    return TCC_CDK_Wrapper::GetLastKeyFramePTS();

}
#ifdef USE_HDCP_DECRYPTION
bool CDKWrapperStagefright::GetDecryptionNeed()
{
    return TCC_CDK_Wrapper::GetDecryptionNeed();

}
#endif
