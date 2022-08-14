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

#ifndef A_RTSP_CONTROLLER_H_

#define A_RTSP_CONTROLLER_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/MediaExtractor.h>

#ifndef _TCC_RTSP_
#define _TCC_RTSP_
#endif

namespace android {

struct ALooper;
struct MyHandler;

struct ARTSPController : public MediaExtractor {
    ARTSPController(const sp<ALooper> &looper);

    status_t connect(const char *url);
    void disconnect();

    void seekAsync(int64_t timeUs, void (*seekDoneCb)(void *), void *cookie
		#ifdef _TCC_RTSP_
		, void (*seekSuccessDoneCb)(void *)
		#endif
		);
#ifdef _TCC_RTSP_
	bool isSeekable();
	bool seekPending();
	int64_t getSeekTime();
    void pauseAsync();
    void playAsync();
    bool isPausing();
#endif
    virtual uint32_t getMyType() const;
    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);

    virtual sp<MetaData> getTrackMetaData(
            size_t index, uint32_t flags);

    int64_t getNormalPlayTimeUs();
    int64_t getQueueDurationUs(bool *eos);
    int64_t getLastBufferTime();
#ifdef _TCC_RTSP_
	int64_t getBufferEndTimeUs();
#endif

    void onMessageReceived(const sp<AMessage> &msg);

    virtual uint32_t flags() const {
        // Seeking 10secs forward or backward is a very expensive operation
        // for rtsp, so let's not enable that.
        // The user can always use the seek bar.
#ifdef _TCC_RTSP_
		return CAN_PAUSE | CAN_SEEK | CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD;
#else
        return CAN_PAUSE | CAN_SEEK;
#endif
    }

protected:
    virtual ~ARTSPController();

private:
    enum {
        kWhatConnectDone    = 'cdon',
        kWhatDisconnectDone = 'ddon',
        kWhatSeekDone       = 'sdon',
        kWhatSeekSuccessDone= 'ssnd',
    };

    enum State {
        DISCONNECTED,
        CONNECTED,
        CONNECTING,
    };

    Mutex mLock;
    Condition mCondition;

    State mState;
    status_t mConnectionResult;

    sp<ALooper> mLooper;
    sp<MyHandler> mHandler;
    sp<AHandlerReflector<ARTSPController> > mReflector;

    void (*mSeekDoneCb)(void *);
    void *mSeekDoneCookie;
#ifdef _TCC_RTSP_	
    void (*mSeekSuccessDoneCb)(void *);
    void *mSeekSuccessDoneCookie;

	bool mSeeking;
#endif
    int64_t mLastSeekCompletedTimeUs;

    DISALLOW_EVIL_CONSTRUCTORS(ARTSPController);
};

}  // namespace android

#endif  // A_RTSP_CONTROLLER_H_
