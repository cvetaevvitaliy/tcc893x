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

#ifndef A_RTP_CONNECTION_H_

#define A_RTP_CONNECTION_H_

#include <media/stagefright/foundation/AHandler.h>
#include <utils/List.h>

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "TCCARTPConnectionDelegate.h"
#include "TCCARTPConnection.h"
#endif

namespace android {

struct ABuffer;
struct ARTPSource;
struct ASessionDescription;

struct ARTPConnection : public AHandler {
    enum Flags {
        kRegularlyRequestFIR = 2,
    };

    ARTPConnection(uint32_t flags = 0);

    void addStream(
            int rtpSocket, int rtcpSocket,
            const sp<ASessionDescription> &sessionDesc, size_t index,
            const sp<AMessage> &notify,
            bool injected);

    void removeStream(int rtpSocket, int rtcpSocket);

    void injectPacket(int index, const sp<ABuffer> &buffer);

    // Creates a pair of UDP datagram sockets bound to adjacent ports
    // (the rtpSocket is bound to an even port, the rtcpSocket to the
    // next higher port).
    static void MakePortPair(
            int *rtpSocket, int *rtcpSocket, unsigned *rtpPort);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    void onTimeReset();
    void onRTPBufferClear();
#endif
protected:
    virtual ~ARTPConnection();
    virtual void onMessageReceived(const sp<AMessage> &msg);

private:
    enum {
        kWhatAddStream,
        kWhatRemoveStream,
        kWhatPollStreams,
        kWhatInjectPacket,
    };
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    friend struct TCCARTPConnection;
#if _USE_TCC_NUPLAYER_
    friend struct TCCARTPConnectionDelegate<TCCARTPConnection>;
    TCCARTPConnection *mSubSource;
    TCCARTPConnectionDelegate<TCCARTPConnection>* mDelegateClass;
#else
    friend struct TCCARTPConnectionDelegate<ARTPConnection>;
    ARTPConnection* mSubSource;
    TCCARTPConnectionDelegate<ARTPConnection>* mDelegateClass;
#endif
#endif
    static const int64_t kSelectTimeoutUs;

    uint32_t mFlags;

    struct StreamInfo;
    List<StreamInfo> mStreams;

    bool mPollEventPending;
    int64_t mLastReceiverReportTimeUs;

    void onAddStream(const sp<AMessage> &msg);
    void onRemoveStream(const sp<AMessage> &msg);
    void onPollStreams();
    void onInjectPacket(const sp<AMessage> &msg);
    void onSendReceiverReports();

    status_t receive(StreamInfo *info, bool receiveRTP);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    status_t parseRTP(void *info, const sp<ABuffer> &buffer);
#else
    status_t parseRTP(StreamInfo *info, const sp<ABuffer> &buffer);
#endif
    status_t parseRTCP(StreamInfo *info, const sp<ABuffer> &buffer);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    status_t parseSR(void *info, const uint8_t *data, size_t size);
#else
    status_t parseSR(StreamInfo *info, const uint8_t *data, size_t size);
#endif
    status_t parseBYE(StreamInfo *info, const uint8_t *data, size_t size);
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    sp<ARTPSource> findSource(void *info, uint32_t id);
#else
    sp<ARTPSource> findSource(StreamInfo *info, uint32_t id);
#endif

    void postPollEvent();
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    void onTimeResetD() {return;};
    void onRTPBufferClearD() {return;};
#endif
    DISALLOW_EVIL_CONSTRUCTORS(ARTPConnection);
};

}  // namespace android

#endif  // A_RTP_CONNECTION_H_
