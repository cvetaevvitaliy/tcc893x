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

#ifndef CHROME_HTTP_DATA_SOURCE_H_

#define CHROME_HTTP_DATA_SOURCE_H_

#include <media/stagefright/foundation/AString.h>
#include <utils/threads.h>

#include "HTTPBase.h"

namespace android {

struct SfDelegate;

struct ChromiumHTTPDataSource : public HTTPBase {
    ChromiumHTTPDataSource(uint32_t flags = 0);

    virtual status_t connect(
            const char *uri,
            const KeyedVector<String8, String8> *headers = NULL,
            off64_t offset = 0);

    virtual void disconnect(bool finalState = false);

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);
    virtual status_t getSize(off64_t *size);
    virtual uint32_t flags();

    virtual sp<DecryptHandle> DrmInitialization(const char *mime);

    virtual void getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client);

    virtual String8 getUri();

    virtual String8 getMIMEType() const;

    virtual status_t reconnectAtOffset(off64_t offset);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    virtual void teardown(); 
    virtual bool isChunked() const; 
    virtual int32_t getResponseStatusCode() const; 
#endif

    static status_t UpdateProxyConfig(
            const char *host, int32_t port, const char *exclusionList);

protected:
    virtual ~ChromiumHTTPDataSource();

private:
    friend struct SfDelegate;

    enum State {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        READING,
        DISCONNECTING
    };

    const uint32_t mFlags;

    mutable Mutex mLock;
    Condition mCondition;

    State mState;

    SfDelegate *mDelegate;

    AString mURI;
    KeyedVector<String8, String8> mHeaders;

    off64_t mCurrentOffset;

    // Any connection error or the result of a read operation
    // (for the lattter this is the number of bytes read, if successful).
    ssize_t mIOResult;

    int64_t mContentSize;

    String8 mContentType;
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    int32_t mResponseStatusCode;
    String8 mTransferEncoding;
    bool mChunked;
	bool mTeardown;
#endif

    sp<DecryptHandle> mDecryptHandle;
    DrmManagerClient *mDrmManagerClient;

    void disconnect_l();

    status_t connect_l(
            const char *uri,
            const KeyedVector<String8, String8> *headers,
            off64_t offset);

    static void InitiateRead(
            ChromiumHTTPDataSource *me, void *data, size_t size);

    void initiateRead(void *data, size_t size);

    void onConnectionEstablished(
            int64_t contentSize, const char *contentType
        #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
            , const char *transferEncoding
            , int32_t httpStatus = 200
        #endif
            );

    void onConnectionFailed(status_t err
        #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
            , int32_t httpStatus = 0
        #endif
            );
    void onReadCompleted(ssize_t size);
    void onDisconnectComplete();

    void clearDRMState_l();

    DISALLOW_EVIL_CONSTRUCTORS(ChromiumHTTPDataSource);
};

}  // namespace android

#endif  // CHROME_HTTP_DATA_SOURCE_H_
