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

#include "ARTPSource.h"
//#include "ASessionDescipiton.h"
namespace android {
struct TCCARTPConnection {
    TCCARTPConnection(void* _baseClass);
    ~TCCARTPConnection();
    status_t parseRTP(void *s, const sp<ABuffer> &buffer);
    status_t parseSR(void *s, const uint8_t *data, size_t size);
    void     onTimeResetD();
    void     onRTPBufferClearD();

private:
    void* baseClass;

	uint32_t mPrevVideoRTPTime;
	uint32_t mPrevAudioRTPTime;
	uint32_t mVideoRTPTimediff;
	uint32_t mAudioRTPTimediff;
	uint32_t mPrevVideoPacketNum;
	uint32_t mPrevAudioPacketNum;

	bool mTryTimeUpVideo;
	bool mTryTimeUpAudio;
	bool mSeek;
};

}  // namespace android
