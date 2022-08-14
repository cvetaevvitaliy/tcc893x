/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef HTTP_BASE_H_

#define HTTP_BASE_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaErrors.h>
#include <utils/threads.h>

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/ABuffer.h>
#endif

namespace android {

struct HTTPBase : public DataSource {
    enum Flags {
        // Don't log any URLs.
        kFlagIncognito = 1,
        kFlagLegacy = 2,
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        kFlagFakeUserAgent = 4,
#endif
    };

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    enum {
        HTTP_VOD     = 0, 
		HTTP_LIVE    = 1,
		HTTP_DLNA    = 2,
		HTTP_DTCP    = 4,
    };

    enum {
        AUX_SESSION_NONE = 0,
        AUX_SESSION_META = 1,
        AUX_SESSION_DUMP = 2,
        AUX_SESSION_MAX
    };

    enum {
        AUX_SESSION_DEACTIVATED, 
		AUX_SESSION_ACTIVATED, 
		AUX_SESSION_FETCHING, 
		AUX_SESSION_FETCHED, 
		AUX_SESSION_INACTIVE
    };

    enum {
        HTTP_PLAY_ONLY = 0,
        HTTP_DUMP_ONLY = 1,
        HTTP_DUMP_AND_PLAY = 2,
    };

    enum {
        HTTP_GET_AUTHORIZATION = 1,
        HTTP_GET_CONNECTION    = 2,
    };

    enum {
		URI_TOKEN_FULL = 0,
		URI_TOKEN_SCHEME = 1,
		URI_TOKEN_HOST = 2,
		URI_TOKEN_PORT = 4,
		URI_TOKEN_PATH = 8,
    };

    enum {
		CHROMIUM = 1,
	    TCC_HTTP,	
    };
#endif

    HTTPBase();

    virtual status_t connect(
            const char *uri,
            const KeyedVector<String8, String8> *headers = NULL,
            off64_t offset = 0) = 0;

    virtual void disconnect(bool finalState = false) = 0;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
	virtual int32_t getMyName() { return CHROMIUM; }
    virtual void teardown() { return; }
    virtual void cancelSeekIfNecessary() { return; }
    virtual bool canExecuteQueuedSeek() { return true; }
	virtual void setDefaultTimeoutSecs(int32_t connTimeoutSecs, int32_t recvTimeoutSecs) { return; }
	virtual status_t connectMore(uint32_t type);
    virtual int32_t getAppendedSourceState() const { return AUX_SESSION_INACTIVE; }
	virtual sp<ABuffer> getRangedCache() { return NULL; }
    virtual bool isRangedConnAvail() const { return true; }
	virtual status_t getUriString(String8 &token, int32_t mode) { return INVALID_OPERATION; }
    virtual String8 getRedirectedLocation() { return String8(""); }
	virtual void setServiceType(int32_t type) { mServiceType = type; }
    virtual int32_t getDumpState(off64_t *dumpSize) { return AUX_SESSION_INACTIVE; }

    virtual status_t skipAt(off64_t size) { return ERROR_END_OF_STREAM;}
#endif

    // Returns true if bandwidth could successfully be estimated,
    // false otherwise.
    virtual bool estimateBandwidth(int32_t *bandwidth_bps);

    virtual status_t getEstimatedBandwidthKbps(int32_t *kbps);

    virtual status_t setBandwidthStatCollectFreq(int32_t freqMs);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual bool isChunked() const = 0;
    virtual status_t getAcceptRanges(String8* value) { return !OK; }
    virtual int32_t getResponseStatusCode() const { return 0; }
#endif
    static status_t UpdateProxyConfig(
            const char *host, int32_t port, const char *exclusionList);

    void setUID(uid_t uid);
    bool getUID(uid_t *uid) const;

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    static sp<HTTPBase> Create(
        uint32_t flags = 0, int32_t connTimeoutSecs = -1, int32_t recvTimeoutSecs = -1);
#else
    static sp<HTTPBase> Create(uint32_t flags = 0);
#endif

    static void RegisterSocketUserTag(int sockfd, uid_t uid, uint32_t kTag);
    static void UnRegisterSocketUserTag(int sockfd);

    static void RegisterSocketUserMark(int sockfd, uid_t uid);
    static void UnRegisterSocketUserMark(int sockfd);

protected:
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    int32_t mServiceType;
#endif
    void addBandwidthMeasurement(size_t numBytes, int64_t delayUs);

private:
    struct BandwidthEntry {
        int64_t mDelayUs;
        size_t mNumBytes;
    };

    Mutex mLock;

    List<BandwidthEntry> mBandwidthHistory;
    size_t mNumBandwidthHistoryItems;
    int64_t mTotalTransferTimeUs;
    size_t mTotalTransferBytes;

    enum {
        kMinBandwidthCollectFreqMs = 1000,   // 1 second
        kMaxBandwidthCollectFreqMs = 60000,  // one minute
    };

    int64_t mPrevBandwidthMeasureTimeUs;
    int32_t mPrevEstimatedBandWidthKbps;
    int32_t mBandWidthCollectFreqMs;

    bool mUIDValid;
    uid_t mUID;

    DISALLOW_EVIL_CONSTRUCTORS(HTTPBase);
};

}  // namespace android

#endif  // HTTP_BASE_H_
