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

#ifndef NU_CACHE_BASE_H_

#define NU_CACHE_BASE_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/DataSource.h>

namespace android {

struct NuCacheBase : public DataSource {
    enum ImplType {
        kDefaultImpl,
        kAnotherImpl,
    };

    NuCacheBase() { mImplType = 0; }

    static sp<NuCacheBase> Create(
            uint32_t implTypeFlag,
            const sp<DataSource> &source,
            const char *cacheConfig = NULL,
            bool disconnectAtHighwatermark = false, 
            bool isMultiConnected = false);

    virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);
    virtual status_t setSize(off64_t size);

    virtual status_t seekAsync(int64_t timeUs, void (*seekDoneCb)(void *),
            void * cookie, void (*seekSuccessDoneCb)(void *)) { 
        return INVALID_OPERATION; 
    }

    virtual void teardown() {}
    virtual void cancelSeekIfNecessary() {}
    virtual bool canExecuteQueuedSeek() { return true; }
    virtual void stop() {}
    virtual void resume() {}

    ////////////////////////////////////////////////////////////////////////////

    virtual size_t cachedSize() {
        return 0;
    }
    virtual size_t approxDataRemaining(status_t *finalStatus, off64_t *lastBytePos = NULL) {
        return 0;
    }
    virtual void resumeFetchingIfNecessary() {}

    // The following methods are supported only if the
    // data source is HTTP-based; otherwise, ERROR_UNSUPPORTED
    // is returned.
    virtual bool getEstimatedBandwidthbps(int32_t *bps); 
    virtual status_t getEstimatedBandwidthKbps(int32_t *kbps);
    virtual status_t setCacheStatCollectFreq(int32_t freqMs);

    virtual inline void signalReconnectWithoutDelay(bool flag) { return; }
    virtual inline void signalPrefillDone() { return; }
    virtual void clearCacheAndResume(off64_t skipBytesAtResume = 0) { return; }
	virtual bool doesKeepFetching() {
        return false;
    }

private:
    uint32_t mImplType;
    
    DISALLOW_EVIL_CONSTRUCTORS(NuCacheBase);
};

}  // namespace android

#endif  // NU_CACHE_BASE_H_
