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

#ifndef NU_HTTP_DATA_SOURCE_H_

#define NU_HTTP_DATA_SOURCE_H_

#include <utils/String8.h>
#include <utils/threads.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/ABuffer.h>

#include "HTTPBase.h"
#include "HTTPStream.h"

namespace android {

struct ALooper;

/////////////////////////////////////////////////////////////////////////

struct NuHTTPDataSource : public HTTPBase {
    NuHTTPDataSource(
            int32_t connTimeoutSecs = -1, 
            int32_t recvTimeoutSecs = -1, 
            bool fakingUA = false);

    sp<ABuffer> mRangedBuffer;
    FILE *mFileHandle;

    virtual status_t connect(
            const char *uri,
            const KeyedVector<String8, String8> *headers = NULL,
            off64_t offset = 0);

    virtual void disconnect(bool finalState = false);

    virtual int32_t getMyName() { return TCC_HTTP; }
    virtual void setDefaultTimeoutSecs(int32_t connTimeoutSecs, int32_t recvTimeoutSecs);
    virtual status_t connectMore(uint32_t type);
    virtual int32_t getAppendedSourceState() const { return mRangedSourceState; }

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);
    virtual status_t skipAt(off64_t size);

    virtual status_t getSize(off64_t *size);
    virtual status_t setSize(off64_t size);
    virtual void setFlags(uint32_t value);
    virtual uint32_t flags();

    virtual sp<DecryptHandle> DrmInitialization(const char *mime);

    virtual void getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client);

    virtual String8 getMIMEType() const;

    virtual status_t reconnectAtOffset(off64_t offset);
    virtual status_t reconnectAtTime(int64_t timeUs);
    virtual void adjustOffset(off64_t offset) { mOffset = offset; }

    virtual bool isChunked() const { return mChunked; }
    virtual int32_t getResponseStatusCode() const { return mResponseStatusCode; }; 
    virtual status_t getAcceptRanges(String8* value);

    virtual void teardown();
    virtual void cancelSeekIfNecessary();
    virtual bool canExecuteQueuedSeek();
    virtual sp<ABuffer> getRangedCache() { return mRangedBuffer; }
    virtual bool isRangedConnAvail() const;
    virtual status_t getUriString(String8& token, int32_t mode);
    virtual String8 getRedirectedLocation() { return mRedirectedLocation; }
    virtual void setServiceType(int32_t type);
    virtual int32_t getDumpState(off64_t* dumpSize);

    status_t getContentType(String8* value);
    status_t GetModH264Streaming(String8* value);

    // Getter & Setter
    inline bool hasContentLengthMissed() const     { return mMissingContentLength; }

protected:
    virtual ~NuHTTPDataSource();

private:
    friend struct AHandlerReflector<NuHTTPDataSource>;
    struct RangedSource;
    sp<RangedSource> mRangedDataSource;

    struct DumpedSource;
    sp<DumpedSource> mDumpedDataSource;   

    enum State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED
    };

	enum {
        kWhatRangeConn       = 'rgcn',
        kWhatRangeFetch      = 'rgfc',
        kWhatRangeSkip       = 'rgsk',
        kWhatRangeSkipDone   = 'rgsd',
        kWhatRangeSkipCancel = 'rgsc',
        kWhatRangeDisconn    = 'rgdc',

        kWhatDumpConn      = 'dpcn',
        kWhatDumpFetch     = 'dpfc',
        kWhatDumpReconnect = 'dprc',
        kWhatDumpDisconn   = 'dpdc',
    };

    sp<AHandlerReflector<NuHTTPDataSource> > mReflector;
    sp<ALooper> mLooper;
    bool mIsLooperStarted;

    Mutex mSerializer;

    Mutex mLock;
    Condition mCondition;
    sp<AMessage> mAsyncResult;

    Mutex mMiscLock;
    Condition mMiscCondition;
    bool mMiscResult;

    State mState;
    int32_t mRangedSourceState;
    int32_t mDumpedSourceState;

    bool mTeardown;
    bool mPortBasedHostURL;

    uint32_t mExtraFlags;

    String8 mURI;
    Vector<String8> mCookie;
    bool mUseFakeUserAgent; 

    String8 mHost;
    unsigned mMasterPort;
    unsigned mPort;
    String8 mPath;
    String8 mHeaders;

    HTTPStream mHTTP;
    off64_t mOffset;
    off64_t mChunkedOffset;
    off64_t mConnectionOffset;

    size_t mNumSkipBytesCount;
    size_t mNowSkipBytesCount;
    size_t mSkipBytesRemained;
    bool   mNeedToSkipBytes;
    bool   mSkipBytesStopped;
    bool   mCanExecuteQueuedSeek;

    ssize_t mAccumulatedSkipBytes;
    ssize_t mSkipBytesRemnant;
    size_t mSkippingByteSize;

    String8 mRedirectedLocation;
    off64_t mContentLength;
    bool mContentLengthValid;
    bool mContentLengthNotified;
    int32_t mResponseStatusCode;
    bool mChunked;
    bool mMissingContentLength;
    bool mAcceptRange;

    // The number of data bytes in the current chunk before any subsequent
    // chunk header (or -1 if no more chunks).
    ssize_t mChunkDataBytesLeft;

    int32_t mMaxReconnectCount;

    bool mEnableHttpMsgLog;
    bool mEnableSkipAt;
    bool mEnableZeroByteRangeRequest;

    sp<DecryptHandle> mDecryptHandle;
    DrmManagerClient *mDrmManagerClient;

    KeyedVector<String8, String8> mOverrides;

    status_t connectAt(int64_t timeUs);

    status_t connect(
            const char *uri, const String8 &headers, off64_t offset);

    status_t connect(
            const char *host, unsigned port, const char *path,
            const String8 &headers,
            off64_t offset);

    // Read up to "size" bytes of data, respect transfer encoding.
    ssize_t internalRead(void *data, size_t size);

    void applyTimeoutResponse();

    void makeFullHeaders(
            const KeyedVector<String8, String8> *overrides,
            String8 *headers);

    NuHTTPDataSource(const NuHTTPDataSource &);
    NuHTTPDataSource &operator=(const NuHTTPDataSource &);

    static bool ParseURL(
        const char *url, String8 *host, unsigned *port, String8 *path);

    void onMessageReceived(const sp<AMessage> &msg);
    void onRangeConn();
    void onRangeFetch();
    void onRangeSkip(const sp<AMessage> &msg);
    void onRangeSkipDone();
    void onRangeSkipCancel();
    void onRangeDisconn();

    void onDumpConn();
    void onDumpFetch();
    void onDumpReconnect();
    void onDumpDisconn();

    void clearDRMState_l();
};

}  // namespace android

#endif  // NU_HTTP_DATA_SOURCE_H_
