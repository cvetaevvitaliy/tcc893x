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

#ifndef _CDK_WRAPPER_STAGEFRIGHT_H_
#define _CDK_WRAPPER_STAGEFRIGHT_H_

#include "tcc_cdk_wrapper.h"
#include "media_io_wrapper_stagefright.h"
#include <media/stagefright/DataSource.h>
#include "TCCStagefrightDefine.h"

// if you want to seek to the nearest front key frame of the requested time, enable this define
#ifdef ENABLE_ACCURATE_SEEK
#define ALWAYS_SEEK_TO_FRONT_KEY_FRAME
#endif

class CDKWrapperStagefright : public TCC_CDK_Wrapper, public RefBase
{
	public:
		CDKWrapperStagefright();
		virtual ~CDKWrapperStagefright();

		virtual int32_t SetCallbackFunctions(uint32_t aType);
		int32_t CDKInit(const sp<DataSource> &source);
		int32_t Seek(uint32_t type, uint32_t targetTS);
		int32_t GetData(uint32_t aType, uint8_t* arBuffer, uint32_t& aSize, uint32_t& aTimeStamp, uint32_t& aExtra, int32_t &iParam, int32_t &iPacketDisCont);

#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
		int32_t PeekData(uint32_t aType, uint32_t& aSize, uint32_t& aTimeStamp, int32_t& iPacketDisCont);
#endif
		void SetAvailableTrack(uint32_t type);

		// INTERNAL_SUBTITLE_INCLUDE
		int GetSubtitleType(void);
		int SelectSubtitleStream(int num);
		int GetCurrentSubtitleStream(void);
		int SetCurrentPlaybackPosition(int msec);
		int32_t GetFirstSubtitle();
		int32_t SetSubtitleSeek(int msec);
		int32_t get_inter_subtitle_type();
		int32_t numFileStream();
		void ClearAudioBuffer();
		int32_t GetLastKeyFramePTS();
#ifdef USE_HDCP_DECRYPTION
		bool GetDecryptionNeed();
#endif
        void ClearStreamBuffer();
        void SkipAudioStream();
        uint32_t GetVideoBufferedTime();

	private:
		Mutex mLock;

		sp<DataSource> mSource;
		SF_MEDIA_FILE*	mFile;
		bool mEnableLogging;
		bool mAudioSeekDone;
		bool mVideoSeekDone;

		// check whether this track is supported by extractor or not
		bool mAudioTrackAvail;
		bool mVideoTrackAvail;

#ifdef ALWAYS_SEEK_TO_FRONT_KEY_FRAME
		// for PeekData()
		uint8_t* mAudioBuffer;
		uint8_t* mVideoBuffer;
		uint32_t mLastAudioTime;
		uint32_t mLastVideoTime;
#endif

		virtual void GetCurrentFileSize(uint64_t& aSize);
		virtual void PrintError(const char *fmt, ...);
		virtual void PrintInfo(const char *fmt, ...);

		void ReadLogSetting();
};

#endif // _CDK_WRAPPER_STAGEFRIGHT_H_

