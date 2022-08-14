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

#ifndef NU_CACHED_SOURCE_2_H_

#define NU_CACHED_SOURCE_2_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "NuCacheBase.h"
#else
#include <media/stagefright/DataSource.h>
#endif

namespace android {

struct ALooper;
struct PageCache;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
struct NuCachedSource2 : public NuCacheBase {
#else
struct NuCachedSource2 : public DataSource {
#endif
    NuCachedSource2(
            const sp<DataSource> &source,
            const char *cacheConfig = NULL,
            bool disconnectAtHighwatermark = false, 
            int32_t modes = 0);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    enum {
        EXTRA_CONNECTING = 0x01,
        PARTIAL_FETCHING = 0x02,
        BROADCAST_FETCHING = 0x04,
    };
#endif

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual status_t setSize(off64_t size);
    virtual void teardown();
    virtual void cancelSeekIfNecessary();
    virtual bool canExecuteQueuedSeek();

    virtual status_t seekAsync(int64_t timeUs, void (*seekDoneCb)(void *), 
                           void *cookie, void (*seekSuccessDoneCb)(void *));
#endif
    virtual uint32_t flags();

    virtual sp<DecryptHandle> DrmInitialization(const char* mime);
    virtual void getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client);
    virtual String8 getUri();

    virtual String8 getMIMEType() const;

    ////////////////////////////////////////////////////////////////////////////

    size_t cachedSize();
    size_t approxDataRemaining(status_t *finalStatus, off64_t *lastBytePos = NULL);

    void resumeFetchingIfNecessary();

    // The following methods are supported only if the
    // data source is HTTP-based; otherwise, ERROR_UNSUPPORTED
    // is returned.
    bool getEstimatedBandwidthbps(int32_t *bps);
    status_t getEstimatedBandwidthKbps(int32_t *kbps);
    status_t setCacheStatCollectFreq(int32_t freqMs);

    static void RemoveCacheSpecificHeaders(
            KeyedVector<String8, String8> *headers,
            String8 *cacheConfig,
            bool *disconnectAtHighwatermark);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    inline void signalReconnectWithoutDelay(bool flag) { mReconnectWithNoDelay = flag; }
    inline void signalPrefillDone() { mPrefillDone = true; }
    void clearCacheAndResume(off64_t skipBytesAtResume = 0);

    inline bool isSkipTimedout() { return mTimeoutSkipping; }
    inline void resetSkipTimeout() { mTimeoutSkipping = false; }

    inline off64_t getConsumedBytesToSkip() { return mConsumedBytesToSkip; }
    inline off64_t getRemainedBytesToSkip() { return mRemainedBytesToSkip; }
    sp<DataSource>& getConnectedDataSource() { return mSource; }

    bool doesKeepFetching();

    void suspend();
    void stop();
    void resume();
#endif

protected:
    virtual ~NuCachedSource2();

private:
    friend struct AHandlerReflector<NuCachedSource2>;

    enum {
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        kPageSize                       = 32712, // 188 * 174
    #else
        kPageSize                       = 65536,
    #endif
        kDefaultHighWaterThreshold      = 20 * 1024 * 1024,
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        kPoorlyInterleavedThreshold     = 8 * 1024 * 1024,
        kDefaultLowWaterThreshold       = kDefaultHighWaterThreshold - kPoorlyInterleavedThreshold,
    #else
        kDefaultLowWaterThreshold       = 4 * 1024 * 1024,
    #endif
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        kBroadcastHighWaterThreshold    = 8 * 1024 * 1024,
        kBroadcastLowWaterThreshold     = 6 * 1024 * 1024,
    #endif

        // Read data after a 15 sec timeout whether we're actively
        // fetching or not.
        kDefaultKeepAliveIntervalUs     = 15000000,

	#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        kWataerMarkBytesUnit = 1024 * 1024, 
        kLiveSourceUnitSize = 188,
        kDefaultAppendedSourceSize = 256 * 1024,
        kPrefillHighWaterMarkBytes = kPageSize * 12,

        // when we need to run stream-skip, limit its timeout to 1 sec
        // to avoid the possibility of ANR.
        kSkipDataTimeoutUs = 1000000,
	#endif
    };

    enum {
        kWhatFetchMore  = 'fetc',
        kWhatRead       = 'read',
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        kWhatFetchFile  = 'fetf',
        kWhatSkipMore   = 'skip',
        kWhatSeekDone   = 'skdn',
        kWhatSuspend    = 'susp',
        kWhatStop       = 'stop',
    #endif
    };

    enum {
        kMaxNumRetries = 100000, /* 10 -> 100000, TELECHIPS */
    };

    sp<DataSource> mSource;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    int32_t mModes;
	bool mReconnectWithNoDelay;
    bool mPrefillDone;
    bool mFetchingInPreparePhase;

    bool mUseRangedCache;
    size_t mRangeByteConsumed;
#endif

    sp<AHandlerReflector<NuCachedSource2> > mReflector;
    sp<ALooper> mLooper;

    Mutex mSerializer;
    mutable Mutex mLock;
    Mutex mCacheLock;
    Condition mCondition;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    bool mSuspended;
    Condition mSuspendCondition;
    Condition mStopCondition;
#endif

    PageCache *mCache;
    off64_t mCacheOffset;
    status_t mFinalStatus;
    off64_t mLastAccessPos;
    sp<AMessage> mAsyncResult;
    bool mFetching;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    void (*mSeekDoneCb)(void *);
    void *mSeekDoneCookie;
    
    void (*mSeekSuccessDoneCb)(void *);
    void *mSeekSuccessDoneCookie;

    bool mSeeking;

    int64_t mStartOnFetchTimeUs;
#endif

    int64_t mLastFetchTimeUs;

    int32_t mNumRetriesLeft;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    bool mTimeoutSkipping;
    int64_t mSkipStartTimeUs;

    off64_t mConsumedBytesToSkip;
    off64_t mRemainedBytesToSkip;

    bool mTeardown;
    bool mStopped;
#endif

    size_t mHighwaterThresholdBytes;
    size_t mLowwaterThresholdBytes;

    // If the keep-alive interval is 0, keep-alives are disabled.
    int64_t mKeepAliveIntervalUs;
    int64_t mPrevKeepAliveIntervalUs;

    bool mDisconnectAtHighwatermark;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
	bool mDisconnectedAtHighWatermark;
    bool mProfilingMode;
    int64_t mLastSeekCompletedTimeUs;
#endif

    void onMessageReceived(const sp<AMessage> &msg);
    void onFetch();
    void onRead(const sp<AMessage> &msg);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    void onFetchFile();
    void onSeekDone(const sp<AMessage> &msg);
    void onSuspend();
    void onStop();
    void onSkip();

    bool skipInternal(size_t alignmentSkipBytes = 0);
#endif
    void fetchInternal();
    ssize_t readInternal(off64_t offset, void *data, size_t size);
    status_t seekInternal_l(off64_t offset);

    size_t approxDataRemaining_l(status_t *finalStatus, off64_t *lastBytePos = NULL);

    void restartPrefetcherIfNecessary_l(
            bool ignoreLowWaterThreshold = false, bool force = false);

    void updateCacheParamsFromSystemProperty();
    void updateCacheParamsFromString(const char *s);

    DISALLOW_EVIL_CONSTRUCTORS(NuCachedSource2);
};

}  // namespace android

#endif  // NU_CACHED_SOURCE_2_H_
