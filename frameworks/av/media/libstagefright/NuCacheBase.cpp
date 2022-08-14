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

//#define LOG_NDEBUG 0
#define LOG_TAG "NuCacheBase"
#include <utils/Log.h>

#include "include/NuCacheBase.h"
#include "include/NuCachedSource2.h"
#include "include/NuDtcpCachedSource.h"

#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

// static
sp<NuCacheBase> NuCacheBase::Create(
            uint32_t implTypeFlag,
            const sp<DataSource> &source,
            const char *cacheConfig,
            bool disconnectAtHighwatermark,
			bool isMultiConnected) {

    if (implTypeFlag == kAnotherImpl) {
        return new NuDtcpCachedSource(source, cacheConfig);
    } else {
        return new NuCachedSource2(source, cacheConfig, 
                       disconnectAtHighwatermark, isMultiConnected);
    }
}

bool NuCacheBase::getEstimatedBandwidthbps(int32_t *bps) {
    *bps = 0;
    return false;
}

status_t NuCacheBase::getEstimatedBandwidthKbps(int32_t *kbps) {
    *kbps = 0;
    return ERROR_UNSUPPORTED;
}

status_t NuCacheBase::setCacheStatCollectFreq(int32_t freqMs) {
    return ERROR_UNSUPPORTED;
}

status_t NuCacheBase::initCheck() const {
    return ERROR_UNSUPPORTED;
}

status_t NuCacheBase::getSize(off64_t *size) {
    *size = 0;
    return ERROR_UNSUPPORTED;
}

status_t NuCacheBase::setSize(off64_t size) {
    return ERROR_UNSUPPORTED;
}

ssize_t NuCacheBase::readAt(off64_t offset, void *data, size_t size) {
    return (ssize_t)ERROR_END_OF_STREAM;
}

}  // namespace android
