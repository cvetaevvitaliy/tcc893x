/*
 * Copyright (C) 2009 The Android Open Source Project
 * Copyright (C) 2012 Telechips 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCC_AWESOME_PLAYER_H_

#define TCC_AWESOME_PLAYER_H_

#include <utils/threads.h>
#include "AwesomePlayer.h"
#include <media/stagefright/MediaSource.h>

namespace android {
struct ALooper;
struct NuDtcpAgent;
struct SfLiveSource;
struct WFDRTSPController;

struct TCCAwesomePlayer : public AwesomePlayer {
    TCCAwesomePlayer();
    virtual ~TCCAwesomePlayer();
    virtual status_t getNumSubtitleStreams(int *numStreams);
    virtual status_t getCurrentSubtitleStream(int *numCurrStream);	
    virtual status_t setParameter(int key, const Parcel &request);
    virtual status_t getParameter(int key, Parcel *reply);

    virtual status_t invoke(const Parcel &request, Parcel *reply);
    virtual status_t selectTrack(size_t trackIndex, bool select);
    virtual status_t selectAudioTrack_l(const sp<MediaSource>& source, size_t trackIndex);

    int setSkimming(MediaSource::ReadOptions * options);

private:
    sp<MediaExtractor> mMediaExtractor;
    sp<SfLiveSource> mLiveSource;
    sp<WFDRTSPController> mWFDRTSPController;

    int32_t     mWFDBufferInitFlag;
    int32_t     mWFDFPS;
    int32_t     mWFDFrameInterval;
    int32_t     mWFDConsecutiveDropCount;
    int64_t     mWFDFirstArriveTime;
    int64_t     mWFDAudioSkipBaseTime;
    int64_t     mWFDFirstVideoPTS;
    int64_t     mWFDRealTime;
    int64_t     mWFDSystemClock;

    sp<ALooper> mLooper;
    sp<NuDtcpAgent> mDtcpAgent;

    String8     mProtectedHost;
    String8     mProtectedPort;

#ifdef TCC_VIDEO_DISPLAY_BY_VSYNC_INT
    int notifyVideoFrameSkip(MediaBuffer *pVideoBuffer, int handleFB);
#endif

    int         mFbHandle;
    int         mFirstBufferId_AfterSeek;
    int         mVsyncFrameDropNum;
    int         mCheckCountToDrop;
    int64_t     mTimeSourcePrevDeltaUs;
    int64_t     mTimeSourceDeltaGapUs;
    int32_t     mVideoWidth;
    int32_t     mVideoHeight;
    int32_t     mFrameRate;
    uint32_t    mSetPositionFlags;
    int64_t     mSetPositionPreviousClockUs;
    int         mVsyncSupport;

    // Fast play & Skimming
    int32_t mPlayRate;
    int32_t mPlayMode;
    bool mAudioPaused;
    int32_t mPrevPlayRate;
    bool mAudioRateChangeable;
	int64_t mSeekInterval;
	int64_t mPrevVideoTimeUs;
	int64_t mLatenessUs;

    SeekType mSeekPending;
    bool mTeardown;

    int32_t mVideoBufferFilledTolerance;
    int64_t mCachedDurationUsAtUnderrun;
    int64_t mQueuedDurationUsAtUnderrun;

    int64_t mWFDLastDisplayClock;
    int64_t mWFDLastVideoPTS;
    bool    bWFDInfoSend;

    int64_t mLastQueuedPacketTimeUs;
    Condition mSeekRequest;

    uint32_t mCodec_id;

    static int32_t GetHeaderSpecificInfo(
            KeyedVector<String8, String8> &header,
            const char *keyString, String8 *valueString = NULL);

    static void UpdateExtraSpecificInfo(
            KeyedVector<String8, String8> &from, const char *fromCategory,
            KeyedVector<String8, String8> &to, const char *toCategory,
            const char *fieldName);

    static void OnWFDInfoWrapper(void *cookie, int arg1, int arg2);

    virtual bool     connectAuxSession(uint32_t sessionType);
    virtual status_t disconnectAuxSession(bool *auxSessionStatus, size_t cachedSize);
    virtual status_t finishSetHLSSource_l();
    virtual status_t finishSetRTSPSource_l();
    virtual status_t finishSetWFDSource_l();
    virtual status_t setPlayerBeforeConnection(bool *disconnectAtHighwatermark);
    virtual status_t connectAuxSessionIfRequested(bool &useMoreHttpSession);
    virtual bool     needToChangeDataSource(status_t &err);
    virtual status_t setPlayerAfterConnection(bool *disconnectAtHighwatermark);
    virtual void     changeMIMEIfNecessary(AString &sniffedMIME, String8 contentType);
    virtual status_t changeDataSourceIfNecessary(const sp<DataSource> &dataSource,
                                                 sp<MediaExtractor> &extractor,
                                                 const char *mime);

    virtual status_t setDataSource_l(const sp<MediaExtractor> &extractor);
    virtual status_t play_l();
    virtual void     onVideoEvent();

    virtual void     playAsync();
    virtual void     pauseAsync();

    virtual void     teardown();
    virtual void     disconnectAndClear();

    virtual void     getApplicationSpecificHeaders();
    virtual void     addHTTPSpecificHeadersIfRequired(int32_t flags);

    virtual status_t seekTo_l(int64_t timeUs);

    virtual void     onBufferingUpdateHls();
    virtual void     onBufferingUpdateHttp();
    virtual void     onBufferingUpdateWFD();

    virtual bool     isStreamingWFD() const;
    virtual void     setDRMInfoIfWFD(int32_t flag);
    void    finishSeekIfNecessary(int64_t videoTimeUs);

    // Multi-audio selection
    status_t    changeAudioTrack(const int32_t newAudioIndex);
    status_t    renewAudioSource(const int32_t newAudioIndex);

    status_t    AdjustTimeIfNecessary(int64_t &iNowUs);
    bool        isCachingHTTP() const;


    bool        prefillCache(const sp<MediaExtractor> &extractor, bool needToPostfill);


    void onWFDInfo(int arg1, int arg2);

    TCCAwesomePlayer(const TCCAwesomePlayer &);
    TCCAwesomePlayer &operator=(const TCCAwesomePlayer &);
};

}  // namespace android

#endif  // TCC_AWESOME_PLAYER_H_
