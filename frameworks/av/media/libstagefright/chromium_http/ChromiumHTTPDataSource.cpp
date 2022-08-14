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

//#define LOG_NDEBUG 0
#define LOG_TAG "ChromiumHTTPDataSource"
#include <media/stagefright/foundation/ADebug.h>

#include "include/ChromiumHTTPDataSource.h"

#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/MediaErrors.h>

#include "support.h"

#include <cutils/properties.h> // for property_get

namespace android {

static int64_t kReadDeletageTimeOutNs = 30000000000LL; // 30 secs.
static int64_t kConnDeletageTimeOutNs = 10000000000LL; // 10 secs.

ChromiumHTTPDataSource::ChromiumHTTPDataSource(uint32_t flags)
    : mFlags(flags),
      mState(DISCONNECTED),
      mDelegate(new SfDelegate),
      mCurrentOffset(0),
      mIOResult(OK),
      mContentSize(-1),
#if _USE_TCC_AWESOMEPLAYER_
      mChunked(false),
      mTeardown(false),
      mResponseStatusCode(0),
#endif
      mDecryptHandle(NULL),
      mDrmManagerClient(NULL) {
    mDelegate->setOwner(this);
}

ChromiumHTTPDataSource::~ChromiumHTTPDataSource() {
    disconnect();

    delete mDelegate;
    mDelegate = NULL;

    clearDRMState_l();

    if (mDrmManagerClient != NULL) {
        delete mDrmManagerClient;
        mDrmManagerClient = NULL;
    }
}

status_t ChromiumHTTPDataSource::connect(
        const char *uri,
        const KeyedVector<String8, String8> *headers,
        off64_t offset) {
    Mutex::Autolock autoLock(mLock);

    uid_t uid;
    if (getUID(&uid)) {
        mDelegate->setUID(uid);
    }

#if defined(LOG_NDEBUG) && !LOG_NDEBUG
    LOG_PRI(ANDROID_LOG_VERBOSE, LOG_TAG, "connect on behalf of uid %d", uid);
#endif

    return connect_l(uri, headers, offset);
}

status_t ChromiumHTTPDataSource::connect_l(
        const char *uri,
        const KeyedVector<String8, String8> *headers,
        off64_t offset) {
    if (mState != DISCONNECTED) {
        disconnect_l();
		mTeardown = false;
    }

#if defined(LOG_NDEBUG) && !LOG_NDEBUG
    LOG_PRI(ANDROID_LOG_VERBOSE, LOG_TAG,
                "connect to <URL suppressed> @%lld", offset);
#endif

    mURI = uri;
    mContentType = String8("application/octet-stream");

    if (headers != NULL) {
        mHeaders = *headers;
    } else {
        mHeaders.clear();
    }

    mState = CONNECTING;
    mContentSize = -1;
    mCurrentOffset = offset;

    mDelegate->initiateConnection(mURI.c_str(), &mHeaders, offset);

    while ((mState == CONNECTING || mState == DISCONNECTING)
#if _USE_TCC_AWESOMEPLAYER_
	   && !mTeardown
#endif
	   ) {
    #if _USE_TCC_AWESOMEPLAYER_
        status_t ret = mCondition.waitRelative(mLock, kConnDeletageTimeOutNs);
        if (ret != OK) {
            continue;
        }
    #else
        mCondition.wait(mLock);
    #endif
    }

#if _USE_TCC_AWESOMEPLAYER_
    if (mTeardown) {
        LOG_PRI(ANDROID_LOG_ERROR, LOG_TAG, "connect_l: stopped by teardown");
        return ERROR_END_OF_STREAM;
    }

	if (mState == CONNECTED) {
		mChunked = (!strncasecmp(mTransferEncoding.string(), "chunked", 7)) 
		         ? true : false;
		return OK;
	} else if (mIOResult == ERROR_UNSUPPORTED) {
		return connect_l(uri, headers, offset);
	}
	return mIOResult;
#else
    return mState == CONNECTED ? OK : mIOResult;
#endif
}

void ChromiumHTTPDataSource::onConnectionEstablished(
        int64_t contentSize, const char *contentType
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
		, const char *transferEncoding
        , int32_t httpStatus
    #endif
		) {
    Mutex::Autolock autoLock(mLock);

    if (mState != CONNECTING) {
        // We may have initiated disconnection.
        CHECK_EQ(mState, DISCONNECTING);
        return;
    }

    mState = CONNECTED;
    mContentSize = (contentSize < 0) ? -1 : contentSize + mCurrentOffset;
    mContentType = String8(contentType);
#if _USE_TCC_AWESOMEPLAYER_
    mTransferEncoding = String8(transferEncoding);
    mResponseStatusCode = httpStatus;
#endif
    mCondition.broadcast();
}

void ChromiumHTTPDataSource::onConnectionFailed(status_t err
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        , int32_t httpStatus
    #endif
        ) {
    Mutex::Autolock autoLock(mLock);
    mState = DISCONNECTED;
    mCondition.broadcast();

    // mURI.clear();

#if _USE_TCC_AWESOMEPLAYER_
    LOG_PRI(ANDROID_LOG_ERROR, LOG_TAG, "connect failed with http status code %d", httpStatus);
    mResponseStatusCode = httpStatus;
#endif
    mIOResult = err;
}

void ChromiumHTTPDataSource::disconnect(bool finalState) {
    Mutex::Autolock autoLock(mLock);
    disconnect_l();
}

void ChromiumHTTPDataSource::disconnect_l() {
    if (mState == DISCONNECTED) {
        return;
    }

    mState = DISCONNECTING;
    mIOResult = -EINTR;

    mDelegate->initiateDisconnect();

    while (mState == DISCONNECTING) {
        mCondition.wait(mLock);
    }

    CHECK_EQ((int)mState, (int)DISCONNECTED);
}

status_t ChromiumHTTPDataSource::initCheck() const {
    Mutex::Autolock autoLock(mLock);

    return mState == CONNECTED ? OK : NO_INIT;
}

ssize_t ChromiumHTTPDataSource::readAt(off64_t offset, void *data, size_t size) {
    Mutex::Autolock autoLock(mLock);

    if (mState != CONNECTED) {
        return INVALID_OPERATION;
    }

#if 0
    char value[PROPERTY_VALUE_MAX];
    if (property_get("media.stagefright.disable-net", value, 0)
            && (!strcasecmp(value, "true") || !strcmp(value, "1"))) {
        LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "Simulating that the network is down.");
        disconnect_l();
        return ERROR_IO;
    }
#endif

#if _USE_TCC_AWESOMEPLAYER_
    bool needsToReconnect = false;

    do {
        if (needsToReconnect || (offset != mCurrentOffset
            // Prevent reduadant reconnection when play list is switched.
            && (!(mChunked && mCurrentOffset == 0)))) {
            LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, 
                    "readAt: %s - offset %lld mCurrentOffset %lld", 
                    needsToReconnect ? "reconnect" : "connect", offset, mCurrentOffset);
#else
        if (offset != mCurrentOffset) {
#endif
            AString tmp = mURI;
            KeyedVector<String8, String8> tmpHeaders = mHeaders;

            disconnect_l();

            status_t err = connect_l(tmp.c_str(), &tmpHeaders, offset);

        #if _USE_TCC_AWESOMEPLAYER_
            if (err == ERROR_CONNECTION_LOST) {
                LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "ERROR_CONNECTION_LOST");
                usleep(3000000); // wait 3 secs.
                continue;
            }
        #endif
            if (err != OK) {
                return err;
            }
        }

        mState = READING;

        int64_t startTimeUs = ALooper::GetNowUs();

        mDelegate->initiateRead(data, size);

        while (mState == READING
#if _USE_TCC_AWESOMEPLAYER_
	       && !mTeardown
#endif
	       ) {
        #if _USE_TCC_AWESOMEPLAYER_
            status_t ret = mCondition.waitRelative(mLock, kReadDeletageTimeOutNs);
            if (mTeardown) {
                LOG_PRI(ANDROID_LOG_ERROR, LOG_TAG, "readAt: stopped by teardown");
                return ERROR_END_OF_STREAM;
            } 
            
            if (ret != OK) {
                LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "readAt: timeout - ret: %d", ret);
                needsToReconnect = true;
                break;
            }
            needsToReconnect = false;
        #else
            mCondition.wait(mLock);
        #endif
        }

    #if _USE_TCC_AWESOMEPLAYER_
        if (!needsToReconnect)
    #endif
        {
            if (mIOResult < OK) {
                return mIOResult;
            }

            if (mState == CONNECTED) {
                int64_t delayUs = ALooper::GetNowUs() - startTimeUs;

                // The read operation was successful, mIOResult contains
                // the number of bytes read.
                addBandwidthMeasurement(mIOResult, delayUs);

                mCurrentOffset += mIOResult;
                return mIOResult;
            }
        }
#if _USE_TCC_AWESOMEPLAYER_
    } while (needsToReconnect && !mTeardown);
#endif

    return ERROR_IO;
}

void ChromiumHTTPDataSource::onReadCompleted(ssize_t size) {
    Mutex::Autolock autoLock(mLock);

    mIOResult = size;

    if (mState == READING) {
        mState = CONNECTED;
        mCondition.broadcast();
    }
}

status_t ChromiumHTTPDataSource::getSize(off64_t *size) {
    Mutex::Autolock autoLock(mLock);

    if (mContentSize < 0) {
        return ERROR_UNSUPPORTED;
    }

    *size = mContentSize;

    return OK;
}

uint32_t ChromiumHTTPDataSource::flags() {
    return kWantsPrefetching | kIsHTTPBasedSource;
}

// static
void ChromiumHTTPDataSource::InitiateRead(
        ChromiumHTTPDataSource *me, void *data, size_t size) {
    me->initiateRead(data, size);
}

void ChromiumHTTPDataSource::initiateRead(void *data, size_t size) {
    mDelegate->initiateRead(data, size);
}

void ChromiumHTTPDataSource::onDisconnectComplete() {
    Mutex::Autolock autoLock(mLock);
    CHECK_EQ((int)mState, (int)DISCONNECTING);

    mState = DISCONNECTED;
    // mURI.clear();
    mIOResult = -ENOTCONN;

    mCondition.broadcast();
}

sp<DecryptHandle> ChromiumHTTPDataSource::DrmInitialization(const char* mime) {
    Mutex::Autolock autoLock(mLock);

    if (mDrmManagerClient == NULL) {
        mDrmManagerClient = new DrmManagerClient();
    }

    if (mDrmManagerClient == NULL) {
        return NULL;
    }

    if (mDecryptHandle == NULL) {
        /* Note if redirect occurs, mUri is the redirect uri instead of the
         * original one
         */
        mDecryptHandle = mDrmManagerClient->openDecryptSession(
                String8(mURI.c_str()), mime);
    }

    if (mDecryptHandle == NULL) {
        delete mDrmManagerClient;
        mDrmManagerClient = NULL;
    }

    return mDecryptHandle;
}

void ChromiumHTTPDataSource::getDrmInfo(
        sp<DecryptHandle> &handle, DrmManagerClient **client) {
    Mutex::Autolock autoLock(mLock);

    handle = mDecryptHandle;
    *client = mDrmManagerClient;
}

String8 ChromiumHTTPDataSource::getUri() {
    Mutex::Autolock autoLock(mLock);

    return String8(mURI.c_str());
}

String8 ChromiumHTTPDataSource::getMIMEType() const {
    Mutex::Autolock autoLock(mLock);

    return mContentType;
}

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
void ChromiumHTTPDataSource::teardown() {
    LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "teardown");
    mTeardown = true;
    mCondition.broadcast();
    disconnect();
}

bool ChromiumHTTPDataSource::isChunked() const {
    Mutex::Autolock autoLock(mLock);

    return mChunked;
}

int32_t ChromiumHTTPDataSource::getResponseStatusCode() const {
    Mutex::Autolock autoLock(mLock);

    return mResponseStatusCode;
}
#endif

void ChromiumHTTPDataSource::clearDRMState_l() {
    if (mDecryptHandle != NULL) {
        // To release mDecryptHandle
        CHECK(mDrmManagerClient);
        mDrmManagerClient->closeDecryptSession(mDecryptHandle);
        mDecryptHandle = NULL;
    }
}

status_t ChromiumHTTPDataSource::reconnectAtOffset(off64_t offset) {
    Mutex::Autolock autoLock(mLock);

    if (mURI.empty()) {
        return INVALID_OPERATION;
    }

    LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "Reconnecting...");
    status_t err = connect_l(mURI.c_str(), &mHeaders, offset);
    if (err != OK) {
        LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "Reconnect failed w/ err 0x%08x", err);
    }

    return err;
}

// static
status_t ChromiumHTTPDataSource::UpdateProxyConfig(
        const char *host, int32_t port, const char *exclusionList) {
    return SfDelegate::UpdateProxyConfig(host, port, exclusionList);
}

}  // namespace android

