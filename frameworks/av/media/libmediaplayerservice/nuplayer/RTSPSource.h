/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef RTSP_SOURCE_H_

#define RTSP_SOURCE_H_

#include "NuPlayerSource.h"

#include "ATSParser.h"

#include <media/stagefright/foundation/AHandlerReflector.h>

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "TCCRTSPSource.h"
#endif

namespace android {

struct ALooper;
struct AnotherPacketSource;
struct MyHandler;
struct SDPLoader;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
struct TCCRTSPSource;
#endif

struct NuPlayer::RTSPSource : public NuPlayer::Source {
    RTSPSource(
            const sp<AMessage> &notify,
            const char *url,
            const KeyedVector<String8, String8> *headers,
            bool uidValid = false,
            uid_t uid = 0,
            bool isSDP = false);

    virtual void prepareAsync();
    virtual void start();
    virtual void stop();
    virtual void pause();
    virtual void resume();

    virtual status_t feedMoreTSData();
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual status_t dequeueAccessUnit_l(bool audio, sp<ABuffer> *accessUnit);
    status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);
#else
    virtual status_t dequeueAccessUnit(bool audio, sp<ABuffer> *accessUnit);
#endif


    virtual status_t getDuration(int64_t *durationUs);
    virtual status_t seekTo(int64_t seekTimeUs);

    void onMessageReceived(const sp<AMessage> &msg);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
public:
    friend struct TCCRTSPSource;

#if _USE_TCC_NUPLAYER_
    friend struct TCCSourceDelegate<TCCRTSPSource>;
    sp<TCCRTSPSource> mSubSource;
    sp<TCCSourceDelegate<TCCRTSPSource> > mDelegateClass;
#else
    friend struct TCCSourceDelegate<RTSPSource>;
    sp<RTSPSource> mSubSource;
    sp<TCCSourceDelegate<RTSPSource> > mDelegateClass;
#endif
#endif

protected:
    virtual ~RTSPSource();

    virtual sp<MetaData> getFormatMeta(bool audio);

private:
    enum {
        kWhatNotify          = 'noti',
        kWhatDisconnect      = 'disc',
        kWhatPerformSeek     = 'seek',
    };

    enum State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        SEEKING,
    };

    enum Flags {
        // Don't log any URLs.
        kFlagIncognito = 1,
    };

    struct TrackInfo {
        sp<AnotherPacketSource> mSource;

        int32_t mTimeScale;
        uint32_t mRTPTime;
        int64_t mNormalPlaytimeUs;
        bool mNPTMappingValid;
    };

    AString mURL;
    KeyedVector<String8, String8> mExtraHeaders;
    bool mUIDValid;
    uid_t mUID;
    uint32_t mFlags;
    bool mIsSDP;
    State mState;
    status_t mFinalResult;
    uint32_t mDisconnectReplyID;
    bool mBuffering;

    sp<ALooper> mLooper;
    sp<AHandlerReflector<RTSPSource> > mReflector;
    sp<MyHandler> mHandler;
    sp<SDPLoader> mSDPLoader;

    Vector<TrackInfo> mTracks;
    sp<AnotherPacketSource> mAudioTrack;
    sp<AnotherPacketSource> mVideoTrack;

    sp<ATSParser> mTSParser;

    int32_t mSeekGeneration;

    int64_t mEOSTimeoutAudio;
    int64_t mEOSTimeoutVideo;

    sp<AnotherPacketSource> getSource(bool audio);

    void onConnected();
    void onSDPLoaded(const sp<AMessage> &msg);
    void onDisconnected(const sp<AMessage> &msg);
    void finishDisconnectIfPossible();

    void performSeek(int64_t seekTimeUs);

    bool haveSufficientDataOnAllTracks();

    void setEOSTimeout(bool audio, int64_t timeout);

    DISALLOW_EVIL_CONSTRUCTORS(RTSPSource);
};

}  // namespace android

#endif  // RTSP_SOURCE_H_
