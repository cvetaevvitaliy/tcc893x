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

#ifndef NU_CRYPT_CACHED_SOURCE_H_

#define NU_CRYPT_CACHED_SOURCE_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>

#include "NuCacheBase.h"

namespace android {

struct ALooper;
struct APageCache;

struct NuDtcpCachedSource : public NuCacheBase {
    NuDtcpCachedSource(
            const sp<DataSource> &source,
            const char *cacheConfig = NULL);

    enum {
        kPCPHeadSize = 14,
        kPCPBodySize = 2032,
        kPCPUnitSize = 2046,
    };

    enum {
        DTCP_STACK_DISCONNECTED = 0,       // TCP disconnected
        DTCP_STACK_CONNECTED,              // TCP connected to other device
        DTCP_STACK_AUTHENTICATED,          // TCP connected and Autenticated (AKE)
        DTCP_STACK_UNAUTHENTICATED,        // Restarted Authentication
        DTCP_STACK_ALIVE,                  // polling for 'keep running' (1x/sec)
        DTCP_STACK_ERROR,                  // unexpected termination
        DTCP_STACK_KEY_NOT_CONFIRMED,      // Key not confirmed
        DTCP_STACK_KEY_EXPIRED,            // Key expired
        DTCP_STACK_MAKE_CONTENT_USABLE,    // Move Commitment event
        DTCP_STACK_DISCARD_CONTENT,        // idem
        DTCP_STACK_COMMIT_FAIL,            // idem
        DTCP_STACK_COMMIT_DONE             // idem
    };

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);
    virtual status_t setSize(off64_t size);
    virtual void teardown();
    virtual uint32_t flags();

    virtual String8 getUri();

    virtual String8 getMIMEType() const;

    ////////////////////////////////////////////////////////////////////////////

    size_t cachedSize();
    size_t approxDataRemaining(status_t *finalStatus, off64_t *lastBytePos = NULL);
    void resumeFetchingIfNecessary();

    // The following methods are supported only if the
    // data source is HTTP-based; otherwise, ERROR_UNSUPPORTED
    // is returned.
    status_t getEstimatedBandwidthKbps(int32_t *kbps);
    status_t setCacheStatCollectFreq(int32_t freqMs);

    static void RemoveCacheSpecificHeaders(
            KeyedVector<String8, String8> *headers,
            String8 *cacheConfig,
            bool *disconnectAtHighwatermark);

	bool doesKeepFetching();

	void suspend();
    void stop();
    void resume();

protected:
    virtual ~NuDtcpCachedSource();

private:
    friend struct AHandlerReflector<NuDtcpCachedSource>;

    enum {
        kPageSize                       = 1024 * 16,
        kFetchSize                      = 32768,
        kDefaultHighWaterThreshold      = 40 * 1024 * 1024,
        kDefaultLowWaterThreshold       = 4 * 1024 * 1024,

        // Read data after a 15 sec timeout whether we're actively
        // fetching or not.
        kDefaultKeepAliveIntervalUs     = 15000000,

        kWataerMarkBytesUnit = 1024 * 1024, 
        kPrefillHighWaterMarkBytes = kPageSize * 12,

	    // when we need to run stream-skip, limit its timeout to 1 sec
	    // to avoid the possibility of ANR.
	    kSkipDataTimeoutUs = 1000000,
    };

    enum {
        kWhatInit       = 'iNit',
        kWhatFetchMore  = 'fetc',
        kWhatMultiFetch  = 'mfet',
        kWhatRead       = 'read',
        kWhatSuspend    = 'susp',
        kWhatStop       = 'stop',
    };

    enum {
        kMaxNumRetries = 10,
    };

    enum {
        DTCP_OK,
        DTCP_MORE_OUTPUT,
        DTCP_MORE_INPUT,
    };

    void * mLibHandle;
    int32_t (*mInitFunc) (const char *addr, int32_t port);
    int32_t (*mStatusFunc) ();
    int32_t (*mHeaderFunc) (const uint8_t * inHeader);
    int32_t (*mProcessFunc) (const uint8_t * inData, int32_t inSize, uint8_t * outData);
	
    void (*mCloseFunc) ();

    bool mLibLoadSanityCheck;

    sp<ALooper> mDummyLooper;

    sp<DataSource> mSource;
	off64_t mSourceSize;

    sp<AHandlerReflector<NuDtcpCachedSource> > mReflector;
    sp<ALooper> mLooper;

    Mutex mSerializer;
    Mutex mLock;
    Mutex mCacheLock;
    Condition mCondition;

	bool mSuspended;
    Condition mSuspendCondition;
    Condition mStopCondition;

    APageCache *mCache;
    off64_t mCacheOffset;
    off64_t mRecvOffset;
    off64_t mNonCachedOffset;
    status_t mFinalStatus;
    off64_t mLastAccessPos;
    sp<AMessage> mAsyncResult;
    bool mFetching;

    int64_t mStartOnFetchTimeUs;
    int64_t mLastFetchTimeUs;

    int32_t mNumRetriesLeft;

    bool mTeardown;
    bool mStopped;

    size_t mHighwaterThresholdBytes;
    size_t mLowwaterThresholdBytes;

    // If the keep-alive interval is 0, keep-alives are disabled.
    int64_t mKeepAliveIntervalUs;

    void onMessageReceived(const sp<AMessage> &msg);
    void onInit();
    void onFetch();
    void onMultiFetch();
    void onSuspend();
    void onStop();
    void onRead(const sp<AMessage> &msg);

    void fetchInternal();
	ssize_t fetchMulti();

    ssize_t readInternal(off64_t offset, void *data, size_t size);
    status_t seekInternal_l(off64_t offset);

    size_t approxDataRemaining_l(status_t *finalStatus, off64_t *lastBytePos = NULL);

    void restartPrefetcherIfNecessary_l(
            bool ignoreLowWaterThreshold = false, bool force = false);

    void updateCacheParamsFromSystemProperty();
    void updateCacheParamsFromString(const char *s);

    DISALLOW_EVIL_CONSTRUCTORS(NuDtcpCachedSource);
};

}  // namespace android

#endif  // NU_CRYPT_CACHED_SOURCE_H_
