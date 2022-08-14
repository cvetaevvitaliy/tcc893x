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

#ifndef OMX_TCC_CODEC_H_

#define OMX_TCC_CODEC_H_

#include <android/native_window.h>
#include <media/IOMX.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaSource.h>
#include <utils/threads.h>
#include <media/stagefright/OMXCodec.h>

namespace android {

struct TCCOMXCodecObserver;

struct TCCOMXCodec : public OMXCodec {
    static sp<MediaSource> Create(
            const sp<IOMX> &omx,
            const sp<MetaData> &meta, bool createEncoder,
            const sp<MediaSource> &source,
            const char *matchComponentName = NULL,
            uint32_t flags = 0,
            const sp<ANativeWindow> &nativeWindow = NULL,
            bool isForceLocally = true);

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();
    virtual status_t read(
            MediaBuffer **buffer, const ReadOptions *options = NULL);

    virtual status_t pause();

    // from MediaBufferObserver
    virtual void signalBufferReturned(MediaBuffer *buffer);
protected:
    virtual ~TCCOMXCodec();

private:

    // Make sure mLock is accessible to OMXCodecObserver
    friend class TCCOMXCodecObserver;

    // Call this with mLock hold
    virtual void on_message(const omx_message &msg);
    virtual status_t waitForBufferFilled_l();

	bool mMapInformed;
	bool mReadWaiting;
	int mFd;
	void* mMapInfo;
	Mutex mTipLock;

    TCCOMXCodec(const sp<IOMX> &omx, IOMX::node_id node,
             uint32_t quirks, uint32_t flags,
             bool isEncoder, const char *mime, const char *componentName,
             const sp<MediaSource> &source,
             const sp<ANativeWindow> &nativeWindow, bool isForceLocally);

// TELECHIPS, ZzaU
	bool TCCPrepareCodec();
	void TCCStopCodec();
	uint32_t getNodeId();
    void drainInput_firstBuffer();
    void fillOutput_firstBuffer();

    TCCOMXCodec(const TCCOMXCodec &);
    TCCOMXCodec &operator=(const TCCOMXCodec &);
};

}  // namespace android

#endif  // OMX_TCC_CODEC_H_
