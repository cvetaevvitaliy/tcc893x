/*
 * Copyright (C) 2009 The Android Open Source Project
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

#ifndef AWESOME_PLAYER_H_

#define AWESOME_PLAYER_H_

#include "HTTPBase.h"
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "NuHTTPDataSource.h"
#endif
#include "TimedEventQueue.h"

#include <media/MediaPlayerInterface.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/OMXClient.h>
#include <media/stagefright/TimeSource.h>
#include <media/stagefright/MetaData.h>
#include <utils/threads.h>
#include <drm/DrmManagerClient.h>

namespace android {

struct AudioPlayer;
struct DataSource;
struct MediaBuffer;
struct MediaExtractor;
struct MediaSource;
struct NuCachedSource2;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
struct NuCacheBase;
struct ARTSPController;
#endif
struct IGraphicBufferProducer;

class DrmManagerClinet;
class DecryptHandle;

class TimedTextDriver;
struct WVMExtractor;

struct AwesomeRenderer : public RefBase {
    AwesomeRenderer() {}

    virtual void render(MediaBuffer *buffer) = 0;

private:
    AwesomeRenderer(const AwesomeRenderer &);
    AwesomeRenderer &operator=(const AwesomeRenderer &);
};

struct AwesomePlayer {
    AwesomePlayer();
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    ~AwesomePlayer();

    void setListener(const wp<MediaPlayerBase> &listener);
    void setUID(uid_t uid);

    status_t setDataSource(
            const char *uri,
            const KeyedVector<String8, String8> *headers = NULL);

    status_t setDataSource(int fd, int64_t offset, int64_t length);

    status_t setDataSource(const sp<IStreamSource> &source);

    void reset();

    status_t prepare();
    status_t prepare_l();
    status_t prepareAsync();
    status_t prepareAsync_l();

    status_t play();
    status_t pause();
    status_t stop();

    bool isPlaying() const;

    status_t setSurfaceTexture(const sp<IGraphicBufferProducer> &bufferProducer);
    void setAudioSink(const sp<MediaPlayerBase::AudioSink> &audioSink);
    status_t setLooping(bool shouldLoop);

    status_t getDuration(int64_t *durationUs);
    status_t getPosition(int64_t *positionUs);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t setParameter(int key, const Parcel &request);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t getParameter(int key, Parcel *reply);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t invoke(const Parcel &request, Parcel *reply);
    status_t setCacheStatCollectFreq(const Parcel &request);

    status_t seekTo(int64_t timeUs);

    // when select is true, the given track is selected.
    // otherwise, the given track is unselected.
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t selectTrack(size_t trackIndex, bool select);

    // This is a mask of MediaExtractor::Flags.
    uint32_t flags() const;

    void postAudioEOS(int64_t delayUs = 0ll);
    void postAudioSeekComplete();
    void postAudioTearDown();
    status_t dump(int fd, const Vector<String16> &args) const;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
protected:
#else
private:
#endif
    friend struct AwesomeEvent;
    friend struct PreviewPlayer;

    enum {
        PLAYING             = 0x01,
        LOOPING             = 0x02,
        FIRST_FRAME         = 0x04,
        PREPARING           = 0x08,
        PREPARED            = 0x10,
        AT_EOS              = 0x20,
        PREPARE_CANCELLED   = 0x40,
        CACHE_UNDERRUN      = 0x80,
        AUDIO_AT_EOS        = 0x0100,
        VIDEO_AT_EOS        = 0x0200,
        AUTO_LOOPING        = 0x0400,

        // We are basically done preparing but are currently buffering
        // sufficient data to begin playback and finish the preparation phase
        // for good.
        PREPARING_CONNECTED = 0x0800,

        // We're triggering a single video event to display the first frame
        // after the seekpoint.
        SEEK_PREVIEW        = 0x1000,

        AUDIO_RUNNING       = 0x2000,
        AUDIOPLAYER_STARTED = 0x4000,

        INCOGNITO           = 0x8000,

        TEXT_RUNNING        = 0x10000,
        TEXTPLAYER_INITIALIZED  = 0x20000,

        SLOW_DECODER_HACK   = 0x40000,
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        LEGACY_HTTPIMPL     = 0x80000,
        PLAYING_READY       = 0x100000,
    #endif
    };

    mutable Mutex mLock;
    Mutex mMiscStateLock;
    mutable Mutex mStatsLock;
    Mutex mAudioLock;

    OMXClient mClient;
    TimedEventQueue mQueue;
    bool mQueueStarted;
    wp<MediaPlayerBase> mListener;
    bool mUIDValid;
    uid_t mUID;

    sp<ANativeWindow> mNativeWindow;
    sp<MediaPlayerBase::AudioSink> mAudioSink;

    SystemTimeSource mSystemTimeSource;
    TimeSource *mTimeSource;

    String8 mUri;
    KeyedVector<String8, String8> mUriHeaders;
    KeyedVector<String8, String8> mSourceSpecificInfos;

    sp<DataSource> mFileSource;

    sp<MediaSource> mVideoTrack;
    sp<MediaSource> mVideoSource;
    sp<AwesomeRenderer> mVideoRenderer;
    bool mVideoRenderingStarted;
    bool mVideoRendererIsPreview;
    int32_t mMediaRenderingStartGeneration;
    int32_t mStartGeneration;

    ssize_t mActiveAudioTrackIndex;
    sp<MediaSource> mAudioTrack;
    sp<MediaSource> mOmxSource;
    sp<MediaSource> mAudioSource;
    AudioPlayer *mAudioPlayer;
    int64_t mDurationUs;

    int32_t mDisplayWidth;
    int32_t mDisplayHeight;
    int32_t mVideoScalingMode;

    uint32_t mFlags;
    uint32_t mExtractorFlags;
    uint32_t mExtractorType;
    uint32_t mSinceLastDropped;

    int64_t mTimeSourceDeltaUs;
    int64_t mVideoTimeUs;

    enum SeekType {
        NO_SEEK,
        SEEK,
        SEEK_VIDEO_ONLY
    };
    SeekType mSeeking;

    bool mSeekNotificationSent;
    int64_t mSeekTimeUs;

    int64_t mBitrate;

    bool mWatchForAudioSeekComplete;
    bool mWatchForAudioEOS;

    sp<TimedEventQueue::Event> mVideoEvent;
    bool mVideoEventPending;
    sp<TimedEventQueue::Event> mStreamDoneEvent;
    bool mStreamDoneEventPending;
    sp<TimedEventQueue::Event> mBufferingEvent;
    bool mBufferingEventPending;
    int64_t mBufferingEventDelayUs;
    sp<TimedEventQueue::Event> mCheckAudioStatusEvent;
    bool mAudioStatusEventPending;
    sp<TimedEventQueue::Event> mVideoLagEvent;
    bool mVideoLagEventPending;
    sp<TimedEventQueue::Event> mAudioTearDownEvent;
    bool mAudioTearDownEventPending;
    sp<TimedEventQueue::Event> mAsyncPrepareEvent;
    Condition mPreparedCondition;
    bool mIsAsyncPrepare;
    status_t mPrepareResult;
    status_t mStreamDoneStatus;

    void postVideoEvent_l(int64_t delayUs = -1);
    void postBufferingEvent_l();
    void postStreamDoneEvent_l(status_t status);
    void postCheckAudioStatusEvent(int64_t delayUs);
    void postVideoLagEvent_l();
    void postAudioTearDownEvent(int64_t delayUs);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t play_l();

    MediaBuffer *mVideoBuffer;

    sp<HTTPBase> mConnectingDataSource;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    sp<NuCacheBase>
#else
    sp<NuCachedSource2>
#endif
    mCachedSource;

    DrmManagerClient *mDrmManagerClient;
    sp<DecryptHandle> mDecryptHandle;

    int64_t mLastVideoTimeUs;
    TimedTextDriver *mTextDriver;

    sp<WVMExtractor> mWVMExtractor;
    sp<MediaExtractor> mExtractor;

    status_t setDataSource_l(
            const char *uri,
            const KeyedVector<String8, String8> *headers = NULL);

    status_t setDataSource_l(const sp<DataSource> &dataSource);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t setDataSource_l(const sp<MediaExtractor> &extractor);
    void reset_l();
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    status_t seekTo_l(int64_t timeUs);
    status_t pause_l(bool at_eos = false);
    void initRenderer_l();
    void notifyVideoSize_l();
    void seekAudioIfNecessary_l();

    void cancelPlayerEvents(bool keepNotifications = false);

    status_t initDecoders(bool video = true, bool audio = true);

    void setAudioSource(sp<MediaSource> source);
    status_t initAudioDecoder();

    void setVideoSource(sp<MediaSource> source);
    status_t initVideoDecoder(uint32_t flags = 0);

    void addTextSource_l(size_t trackIndex, const sp<MediaSource>& source);

    void onStreamDone();

    void notifyListener_l(int msg, int ext1 = 0, int ext2 = 0);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual
#endif
    void onVideoEvent();
    void onBufferingUpdate();

    void onCheckAudioStatus();
    void onPrepareAsyncEvent();
    void abortPrepare(status_t err);
    void finishAsyncPrepare_l();
    void onVideoLagUpdate();
    void onAudioTearDownEvent();

    void beginPrepareAsync_l();

    bool getCachedDuration_l(int64_t *durationUs, bool *eos
                         #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
                             , bool *isCacheFull = NULL
                         #endif
                            );

    status_t finishSetDataSource_l();

    static bool ContinuePreparation(void *cookie);

    bool getBitrate(int64_t *bitrate);

    void finishSeekIfNecessary(int64_t videoTimeUs);
    void ensureCacheIsFetching_l();

    void notifyIfMediaStarted_l();
    void createAudioPlayer_l();
    status_t startAudioPlayer_l(bool sendErrorNotification = true);

    void shutdownVideoDecoder_l();
    status_t setNativeWindow_l(const sp<ANativeWindow> &native);

    bool isStreamingHTTP() const;

    void sendCacheStats();
    void checkDrmStatus(const sp<DataSource>& dataSource);

    enum FlagMode {
        SET,
        CLEAR,
        ASSIGN
    };
    void modifyFlags(unsigned value, FlagMode mode);

    struct TrackStat {
        String8 mMIME;
        String8 mDecoderName;
    };

    // protected by mStatsLock
    struct Stats {
        int mFd;
        String8 mURI;
        int64_t mBitrate;

        // FIXME:
        // These two indices are just 0 or 1 for now
        // They are not representing the actual track
        // indices in the stream.
        ssize_t mAudioTrackIndex;
        ssize_t mVideoTrackIndex;

        int64_t mNumVideoFramesDecoded;
        int64_t mNumVideoFramesDropped;
        int32_t mVideoWidth;
        int32_t mVideoHeight;
        uint32_t mFlags;
        Vector<TrackStat> mTracks;
    } mStats;

    bool    mOffloadAudio;
    bool    mAudioTearDown;
    bool    mAudioTearDownWasPlaying;
    int64_t mAudioTearDownPosition;

    status_t setVideoScalingMode(int32_t mode);
    status_t setVideoScalingMode_l(int32_t mode);
    status_t getTrackInfo(Parcel* reply) const;

    status_t selectAudioTrack_l(const sp<MediaSource>& source, size_t trackIndex);

    size_t countTracks() const;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    enum {
        NONE = 0,
        RTSP = 1,
        HTTP = 2,
        HLS  = 4,
        DLNA = 8,
        WVM  = 16,
        WFD  = 32,
    };
    uint32_t mStreamingMode;

    enum {
        NOTIFY_ON_RUNNING  = 0x01,
        NOTIFY_ON_UNDERRUN = 0x02,
    };
    int32_t mBufferingUpdateType;

    int32_t mHTTPTimeoutSecs;
    bool mSeekable;
    bool mSeekInvokable;
    Condition mSeekedCondition;

    bool mHasItsOwnCache;
    bool mUseAnotherCachedSource;

    size_t  mLowWaterMarkBytes;
    size_t  mHighWaterMarkBytes;
    int64_t mEntryConnectionTimeUs;
    int64_t mPrefillWaitTimeUs;
    int64_t mPrerollWaterMarkUs;
    int64_t mPauseWaterMarkUs;
    int64_t mResumeWaterMarkUs;

    bool mProfilingMode;
    off64_t mPrefillCacheSize;

    sp<ARTSPController> mRTSPController;
    int32_t mNumKeepingUnderrunNearEOS;
    int64_t mSeekToEndTimeMarginUs;
    bool mSeekAsyncDonePending;
    bool mCheckEOSPossibility;
    bool mEOSReached;
    bool mEOSFinalized;

    bool isSeekable() const;

    static void OnSeekDoneWrapper(void *cookie);
    static void OnSeekSuccessDoneWrapper(void *cookie);

    void onSeekDone();
    void onSeekSuccessDone();

    void updateBufferingParamsFromSystemProperty();
    void showBufferingParams();

    status_t finishSetDataSource();
    status_t setVideoSynchronousMode(int32_t mode);
    void modifyStreamingMode(unsigned value, FlagMode mode);

    void onBufferingUpdateRtsp();
    virtual void onBufferingUpdateHls()  { onBufferingUpdate(); }
    virtual void onBufferingUpdateHttp() { onBufferingUpdate(); }
    virtual void onBufferingUpdateWFD()  { onBufferingUpdate(); }

    bool isStreamingRTSP() const;
    virtual bool isStreamingWFD() const { return false; }

    virtual status_t finishSetHLSSource_l() {
        return finishSetDataSource_l();
    }
    virtual status_t finishSetRTSPSource_l() {
        return finishSetDataSource_l();
    }
    virtual status_t finishSetWFDSource_l() {
        return finishSetDataSource_l();
    }
    virtual status_t setPlayerBeforeConnection(bool *disconnectAtHighwatermark) {
        return OK;
    }
    virtual status_t connectAuxSessionIfRequested(bool &useMoreHttpSession) {
        return OK;
    }
    virtual bool needToChangeDataSource(status_t &err) { 
        return false;
    }
    virtual status_t setPlayerAfterConnection(bool *disconnectAtHighwatermark) {
        return OK;
    }
    virtual void changeMIMEIfNecessary(AString &sniffedMIME, String8 contentType) {}
    virtual status_t changeDataSourceIfNecessary(const sp<DataSource> &dataSource,
                                                 sp<MediaExtractor> &extractor,
                                                 const char *mime) {
        return ERROR_UNSUPPORTED;
    }

    virtual void playAsync() {}
    virtual void pauseAsync() {}

    virtual void teardown() {}
    virtual void disconnectAndClear() {}
    virtual void signalReset() {}
    virtual void getApplicationSpecificHeaders() {}
    virtual void addHTTPSpecificHeadersIfRequired(int32_t flags) {}

    virtual bool connectAuxSession(uint32_t sessionType) { return false; }
    virtual status_t disconnectAuxSession(
                bool *auxSessionStatus, size_t cachedSize) { return OK; }
    virtual void setDRMInfoIfWFD(int32_t flag) {}
#endif

    AwesomePlayer(const AwesomePlayer &);
    AwesomePlayer &operator=(const AwesomePlayer &);
};

}  // namespace android

#endif  // AWESOME_PLAYER_H_
