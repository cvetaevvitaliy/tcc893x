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

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/Utils.h>

namespace android {
struct TCCMyHandler : public RefBase {

    struct TrackInfo {
        AString mURL;
        int mRTPSocket;
        int mRTCPSocket;
        bool mUsingInterleavedTCP;
        uint32_t mFirstSeqNumInSegment;
        bool mNewSegment;

        uint32_t mRTPAnchor;
        int64_t mNTPAnchorUs;
        int32_t mTimeScale;
        bool mEOSReceived;

        uint32_t mNormalPlayTimeRTP;
        int64_t mNormalPlayTimeUs;

        sp<APacketSource> mPacketSource;

        // Stores packets temporarily while no notion of time
        // has been established yet.
        List<sp<ABuffer> > mPackets;
    };

    TCCMyHandler(void* _baseClass);
    virtual ~TCCMyHandler();
    void onMessageReceivedD(const sp<AMessage> &msg);
    void postAccessUnitTimeoutCheck();
    void parsePlayResponse(const sp<ARTSPResponse>&);
    void connectDefaultTypeD(bool type);

private:
    uint64_t mStartupTimeoutUs;
    float mNTP1, mNTP2;
    int64_t mReqSeekTimeUs;
    int64_t mResSeekTimeUs;
    bool mPausing;
    uint32_t mFramerate;
    bool mTransportsSwitched;
    uint64_t mAccessUnitTimeoutUs;
    bool mTransportChange;
    int32_t mErrorBackUp;
    void* baseClass;
};

}  // namespace android
