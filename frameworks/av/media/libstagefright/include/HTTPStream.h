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

#ifndef HTTP_STREAM_H_

#define HTTP_STREAM_H_

#include <sys/types.h>

#include <media/stagefright/MediaErrors.h>
#include <utils/KeyedVector.h>
#include <utils/threads.h>
#include <utils/String8.h>

#include <sys/time.h>

#include <sys/socket.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

namespace android {

class HTTPStream {
public:
    HTTPStream();
    ~HTTPStream();

	void setDefaultTimeoutSecs(int32_t connTimeoutSecs = -1, 
                               int32_t recvTimeoutSecs = -1);

    status_t connect(const char *server, int port = 80);

    status_t disconnect(bool finalState = false);

    status_t send(const char *data, size_t size);

    // Assumes data is a '\0' terminated string.
    status_t send(const char *data);

    // Receive up to "size" bytes of data.
    ssize_t receive(void *data, size_t size);

    status_t receive_header(int *http_status);

    // The header key used to retrieve the status line.
    const char *kStatusKey;

    bool find_header_value(
            const String8 &key, String8 *value) const;

    size_t get_cookie_num() const;
    bool find_cookie_value(size_t index, String8 *value) const;

    // Pass a negative value to disable the timeout.
    void setReceiveTimeout(int seconds);

    // Receive a line of data terminated by CRLF, line will be '\0' terminated
    // _excluding_ the termianting CRLF.
    status_t receive_line(char *line, size_t size);

    /*static*/ int64_t getNowUs();

    inline int64_t getConnTimeoutUs() { return mSelectInConnectExpiredUs; }
    inline int64_t getRecvTimeoutUs() { return mSelectInSendRecvExpiredUs; }
    inline void signalMissingContentLength(bool flag) { mIsContentLengthMissing = flag;}
    inline void signalToIgnoreOpNowInProgress(bool flag) { mIgnoreOpNowInProgress = flag;}
    inline void setContentLengthValidity(bool flag) { mIsContentLengthValid = flag;}

private:
    enum State {
        READY,
        CONNECTING,
        CONNECTED
    };

    int64_t mSelectInConnectExpiredUs;
    int64_t mSelectInSendRecvExpiredUs;

    State mState;
    bool mFinalState;
    bool mUnreachableState;
    bool mEnableHttpMsgLog;

    bool mConnTimeoutPrefixed;
	int64_t mDefaultConnTimeoutUs;
    bool mRecvTimeoutPrefixed;
	int64_t mDefaultRecvTimeoutUs;
	bool mIsContentLengthMissing;
	bool mIgnoreOpNowInProgress;
	bool mIsContentLengthValid;

    Mutex mLock;
    int mSocket;

    KeyedVector<String8, String8> mHeaders;
    KeyedVector<size_t, String8> mCookies;

	bool MakeSocketBlocking(int s, bool blocking); 
	status_t MyConnect(
	        int s, const struct sockaddr *addr, socklen_t addrlen); 
	ssize_t MySendReceive(
        int s, void *data, size_t size, int flags, bool sendData,
        int64_t entranceTimeUs = -1); 
	ssize_t MySend(int s, const void *data, size_t size, int flags, int64_t entranceTimeUs = -1);
	ssize_t MyReceive(int s, void *data, size_t size, int flags, int64_t entranceTimeUs = -1);

    HTTPStream(const HTTPStream &);
    HTTPStream &operator=(const HTTPStream &);
};

}  // namespace android

#endif  // HTTP_STREAM_H_
