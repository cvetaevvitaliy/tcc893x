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
#define LOG_TAG "NuCachedSource2"
#include <utils/Log.h>

#include "include/NuCachedSource2.h"
#include "include/HTTPBase.h"

#include <cutils/properties.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>

namespace android {

struct PageCache {
    PageCache(size_t pageSize);
    ~PageCache();

    struct Page {
        void *mData;
        size_t mSize;
    };

    Page *acquirePage();
    void releasePage(Page *page);

    void appendPage(Page *page);
    size_t releaseFromStart(size_t maxBytes);

    size_t totalSize() const {
        return mTotalSize;
    }

    void copy(size_t from, void *data, size_t size);

private:
    size_t mPageSize;
    size_t mTotalSize;
    size_t mTotalPage;

	bool mProfilingMode;

    List<Page *> mActivePages;
    List<Page *> mFreePages;

    void freePages(List<Page *> *list);

    DISALLOW_EVIL_CONSTRUCTORS(PageCache);
};

PageCache::PageCache(size_t pageSize)
    : mPageSize(pageSize),
      mTotalSize(0),
      mTotalPage(0) {
}

PageCache::~PageCache() {
    freePages(&mActivePages);
    freePages(&mFreePages);
}

void PageCache::freePages(List<Page *> *list) {
    List<Page *>::iterator it = list->begin();
    while (it != list->end()) {
        Page *page = *it;

        free(page->mData);
        delete page;
        page = NULL;

        ++it;
    }
}

PageCache::Page *PageCache::acquirePage() {
    if (!mFreePages.empty()) {
        List<Page *>::iterator it = mFreePages.begin();
        Page *page = *it;
        mFreePages.erase(it);

        return page;
    }

    Page *page = new Page;
    page->mData = malloc(mPageSize);
    page->mSize = 0;
    mTotalPage++;

    return page;
}

void PageCache::releasePage(Page *page) {
    page->mSize = 0;
    mFreePages.push_back(page);
}

void PageCache::appendPage(Page *page) {
    mTotalSize += page->mSize;
    mActivePages.push_back(page);
}

size_t PageCache::releaseFromStart(size_t maxBytes) {
    size_t bytesReleased = 0;

    while (maxBytes > 0 && !mActivePages.empty()) {
        List<Page *>::iterator it = mActivePages.begin();

        Page *page = *it;

        if (maxBytes < page->mSize) {
            break;
        }

        mActivePages.erase(it);

        maxBytes -= page->mSize;
        bytesReleased += page->mSize;

        releasePage(page);
    }

    if (mTotalSize < bytesReleased) {
        mTotalSize = 0;
    } else {
        mTotalSize -= bytesReleased;
    }
    return bytesReleased;
}

void PageCache::copy(size_t from, void *data, size_t size) {
    ALOGV("copy from %d size %d", from, size);

    if (size == 0) {
        return;
    }

    CHECK_LE(from + size, mTotalSize);

    size_t offset = 0;
    List<Page *>::iterator it = mActivePages.begin();
    while (from >= offset + (*it)->mSize) {
        offset += (*it)->mSize;
        ++it;
    }

    size_t delta = from - offset;
    size_t avail = (*it)->mSize - delta;

    if (avail >= size) {
        memcpy(data, (const uint8_t *)(*it)->mData + delta, size);
        return;
    }

    memcpy(data, (const uint8_t *)(*it)->mData + delta, avail);
    ++it;
    data = (uint8_t *)data + avail;
    size -= avail;

    while (size > 0) {
        size_t copy = (*it)->mSize;
        if (copy > size) {
            copy = size;
        }
        memcpy(data, (*it)->mData, copy);
        data = (uint8_t *)data + copy;
        size -= copy;
        ++it;
    }
}

////////////////////////////////////////////////////////////////////////////////

NuCachedSource2::NuCachedSource2(
        const sp<DataSource> &source,
        const char *cacheConfig,
        bool disconnectAtHighwatermark, 
        int32_t modes)
    : mSource(source),
#if _USE_TCC_AWESOMEPLAYER_
      mModes(modes),
      mReconnectWithNoDelay(false),
      mFetchingInPreparePhase(true),
      mUseRangedCache(false),
      mRangeByteConsumed(0),
#endif
      mReflector(new AHandlerReflector<NuCachedSource2>(this)),
      mLooper(new ALooper),
      mCache(new PageCache(kPageSize)),
      mCacheOffset(0),
      mFinalStatus(OK),
      mLastAccessPos(0),
      mFetching(true),
#if _USE_TCC_AWESOMEPLAYER_
      mSeeking(false),
      mSeekDoneCb(NULL),
      mSeekSuccessDoneCb(NULL),
      mSeekDoneCookie(NULL),
      mLastSeekCompletedTimeUs(-1),
      mPrefillDone(false),
      mStartOnFetchTimeUs(-1),
#endif
      mLastFetchTimeUs(-1),
      mNumRetriesLeft(kMaxNumRetries),
#if _USE_TCC_AWESOMEPLAYER_
      mConsumedBytesToSkip(0),
      mRemainedBytesToSkip(0),
      mTimeoutSkipping(false),
      mSkipStartTimeUs(-1),
      mTeardown(false),
      mSuspended(false),
      mStopped(false),
      mDisconnectedAtHighWatermark(false),
      mProfilingMode(false),
#else
      mHighwaterThresholdBytes(kDefaultHighWaterThreshold),
      mLowwaterThresholdBytes(kDefaultLowWaterThreshold),
      mKeepAliveIntervalUs(kDefaultKeepAliveIntervalUs),
#endif
      mDisconnectAtHighwatermark(disconnectAtHighwatermark) {
    // We are NOT going to support disconnect-at-highwatermark indefinitely
    // and we are not guaranteeing support for client-specified cache
    // parameters. Both of these are temporary measures to solve a specific
    // problem that will be solved in a better way going forward.

#if _USE_TCC_AWESOMEPLAYER_
    if (mModes & BROADCAST_FETCHING) {
        mHighwaterThresholdBytes = kBroadcastHighWaterThreshold;
        mLowwaterThresholdBytes = kBroadcastLowWaterThreshold;
    } else {
        mHighwaterThresholdBytes = kDefaultHighWaterThreshold;
        mLowwaterThresholdBytes = kDefaultLowWaterThreshold;
    }
	mKeepAliveIntervalUs = kDefaultKeepAliveIntervalUs / 5;
#else
    mKeepAliveIntervalUs = kDefaultKeepAliveIntervalUs;
#endif
    updateCacheParamsFromSystemProperty();

    if (cacheConfig != NULL) {
        updateCacheParamsFromString(cacheConfig);
    }

    if (mDisconnectAtHighwatermark) {
        ALOGW("Makes no sense to disconnect and do keep-alives...");
        mKeepAliveIntervalUs = 0;
    }

#if _USE_TCC_AWESOMEPLAYER_
	mPrevKeepAliveIntervalUs = mKeepAliveIntervalUs;

    char value[PROPERTY_VALUE_MAX];
    property_get("tcc.media.profile", value, "off");
    mProfilingMode = (!strcmp(value, "on")) ? true : false;
#endif

    mLooper->setName("NuCachedSource2");
    mLooper->registerHandler(mReflector);
    mLooper->start(false, false, PRIORITY_DISPLAY);

    Mutex::Autolock autoLock(mLock);
#if _USE_TCC_AWESOMEPLAYER_
    if (mModes & PARTIAL_FETCHING) {
        (new AMessage(kWhatFetchFile, mReflector->id()))->post();
    } else 
#endif
    (new AMessage(kWhatFetchMore, mReflector->id()))->post();
}

NuCachedSource2::~NuCachedSource2() {
    mLooper->stop();
    mLooper->unregisterHandler(mReflector->id());

    delete mCache;
    mCache = NULL;

    mReflector.clear();
    mLooper.clear();
}

bool NuCachedSource2::getEstimatedBandwidthbps(int32_t *bps) {
    if (mSource->flags() & (kIsHTTPBasedSource | kIsExtraHTTPBasedSource)) {
        HTTPBase* source = static_cast<HTTPBase *>(mSource.get());
        return source->estimateBandwidth(bps);
    }
    return false;
}

status_t NuCachedSource2::getEstimatedBandwidthKbps(int32_t *kbps) {
    if (mSource->flags() & (kIsHTTPBasedSource | kIsExtraHTTPBasedSource)) {
        HTTPBase* source = static_cast<HTTPBase *>(mSource.get());
        return source->getEstimatedBandwidthKbps(kbps);
    }
    return ERROR_UNSUPPORTED;
}

status_t NuCachedSource2::setCacheStatCollectFreq(int32_t freqMs) {
    if (mSource->flags() & (kIsHTTPBasedSource | kIsExtraHTTPBasedSource)) {
        HTTPBase *source = static_cast<HTTPBase *>(mSource.get());
        return source->setBandwidthStatCollectFreq(freqMs);
    }
    return ERROR_UNSUPPORTED;
}

status_t NuCachedSource2::initCheck() const {
    return mSource->initCheck();
}

status_t NuCachedSource2::getSize(off64_t *size) {
    return mSource->getSize(size);
}

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
status_t NuCachedSource2::setSize(off64_t size) {
    return mSource->setSize(size);
}

void NuCachedSource2::teardown() {
	ALOGE("teardown");
    mTeardown = true;
    mSource->teardown();
    stop();
    setFlags(flags() | kIsDataSourceToreDown);
}

void NuCachedSource2::cancelSeekIfNecessary() {
    mSource->cancelSeekIfNecessary();
}

bool NuCachedSource2::canExecuteQueuedSeek() {
    return mSource->canExecuteQueuedSeek();
}
#endif

uint32_t NuCachedSource2::flags() {
    // Remove HTTP related flags since NuCachedSource2 is not HTTP-based.
    uint32_t flags = mSource->flags() & ~(kWantsPrefetching | kIsHTTPBasedSource | kIsExtraHTTPBasedSource);
	setFlags(flags | kIsCachingDataSource);
    return DataSource::flags();
}

status_t NuCachedSource2::seekAsync(int64_t timeUs, 
        void (*seekDoneCb)(void *), void *cookie,
        void (*seekSuccessDoneCb)(void *)) {
    Mutex::Autolock autoSerializer(mSerializer);

    Mutex::Autolock autoLock(mLock);

    ALOGE("seekAsync time %lld", timeUs);

    CHECK(seekDoneCb != NULL);
    CHECK(mSeekDoneCb == NULL);

    bool tooEarly = 
        mLastSeekCompletedTimeUs >= 0
            && ALooper::GetNowUs() < mLastSeekCompletedTimeUs + 100000ll;

    if (tooEarly) {
        ALOGW("Cancel seek - too early !!");
        (*seekDoneCb)(cookie);
        return OK;
    }

    mSeeking = true;
    mSeekDoneCb = seekDoneCb;
    mSeekSuccessDoneCb = seekSuccessDoneCb;
    mSeekDoneCookie = cookie;

    sp<AMessage> msg = new AMessage(kWhatSeekDone, mReflector->id());
    msg->setInt64("time", timeUs);
    msg->post();

    return OK;
}

void NuCachedSource2::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
    #if _USE_TCC_AWESOMEPLAYER_
        case kWhatSkipMore:
    #endif
        case kWhatFetchMore:
        {
        #if _USE_TCC_AWESOMEPLAYER_
            if (mSuspended || mStopped) {
                break;
            }

            (mRemainedBytesToSkip) ? onSkip() :
        #endif
            onFetch();
            break;
        }

        case kWhatRead:
        {
            onRead(msg);
            break;
        }

    #if _USE_TCC_AWESOMEPLAYER_
        case kWhatFetchFile:
        {
            onFetchFile();
            break;
        }

        case kWhatSeekDone:
        {
            ALOGD("kWhatSeekDone");
            onSeekDone(msg);
            break;
        }
        case kWhatSuspend:
        {
            ALOGD("kWhatSuspend");
            onSuspend();
            break;
        }

		case kWhatStop:
        {
            ALOGD("kWhatStop");
            onStop();
            break;
        }
    #endif

        default:
            TRESPASS();
    }
}

void NuCachedSource2::fetchInternal() {
    ALOGV("fetchInternal");

#if _USE_TCC_AWESOMEPLAYER_
    if (mSource->getMyName() == HTTPBase::CHROMIUM) 
#endif
    {
        bool reconnect = false;

        {
            Mutex::Autolock autoLock(mLock);
            CHECK(mFinalStatus == OK || mNumRetriesLeft > 0);

            if (mFinalStatus != OK) {
                --mNumRetriesLeft;

                reconnect = true;
            }
        }

        if (reconnect) {
            status_t err =
                mSource->reconnectAtOffset(mCacheOffset + mCache->totalSize());

            Mutex::Autolock autoLock(mLock);

            if (err == ERROR_UNSUPPORTED || err == -EPIPE) {
                // These are errors that are not likely to go away even if we
                // retry, i.e. the server doesn't support range requests or similar.
                mNumRetriesLeft = 0;
                return;
            } else if (err != OK) {
                ALOGI("The attempt to reconnect failed, %d retries remaining",
                     mNumRetriesLeft);

                return;
            }
        }
    }

#if _USE_TCC_AWESOMEPLAYER_
    if (mSource->getMyName() == HTTPBase::TCC_HTTP) {
        if (mDisconnectedAtHighWatermark) {
            status_t err =
                mSource->reconnectAtOffset(mCacheOffset + mCache->totalSize());

            Mutex::Autolock autoLock(mLock);

            if (err == ERROR_UNSUPPORTED) {
                mNumRetriesLeft = 0;
                return;
            } else if (err != OK) {
                ALOGI("The attempt to reconnect failed, %d retries remaining",
                        mNumRetriesLeft);

                return;
            }
            mDisconnectedAtHighWatermark = false;
        }
    }
#endif

    PageCache::Page *page = mCache->acquirePage();

    ssize_t n = mSource->readAt(
            mCacheOffset + mCache->totalSize(), page->mData, kPageSize);

    Mutex::Autolock autoLock(mLock);

#if _USE_TCC_AWESOMEPLAYER_
    if (mTeardown) {
        ALOGE("fetchInternal: tear down");
        mFinalStatus = ERROR_END_OF_STREAM;
        mCache->releasePage(page);
    } else
#endif
    if (n == 0 || n == ERROR_END_OF_STREAM || n == ERROR_CONNECTION_LOST) {
        ALOGI("ERROR_END_OF_STREAM: n = %d", n);

        mNumRetriesLeft = 0;
        mFinalStatus = ERROR_END_OF_STREAM;

        mCache->releasePage(page);
    } else if (n < 0) {
        if (n == ERROR_UNSUPPORTED || n == -EPIPE) {
            // These are errors that are not likely to go away even if we
            // retry, i.e. the server doesn't support range requests or similar.
            mNumRetriesLeft = 0;
        }
        ALOGE("source returned error %ld, %d retries left", n, mNumRetriesLeft);
        mFinalStatus = n;
        mCache->releasePage(page);
    } else {
        if (mFinalStatus != OK) {
            ALOGI("retrying a previously failed read succeeded.");
        }
        mNumRetriesLeft = kMaxNumRetries;
        mFinalStatus = OK;

        page->mSize = n;
        mCache->appendPage(page);
    }
}

void NuCachedSource2::onFetch() {
    ALOGV("onFetch");

    if (mFinalStatus != OK && mNumRetriesLeft == 0) {
        ALOGV("EOS reached, done prefetching for now");
    #if _USE_TCC_AWESOMEPLAYER_
        if (mFetching == true && mProfilingMode) {
            ALOGE("[PROFILE] Pause caching !! - onFetch(), EOS reached");
        }
    #endif
        mFetching = false;
    }

    bool keepAlive =
        !mFetching
            && mFinalStatus == OK
            && mKeepAliveIntervalUs > 0
        #if _USE_TCC_AWESOMEPLAYER_
            && (mStartOnFetchTimeUs = ALooper::GetNowUs())
        #else
            && ALooper::GetNowUs() 
        #endif
                >= mLastFetchTimeUs + mKeepAliveIntervalUs;

    if (mFetching || keepAlive) {
        if (keepAlive) {
        #if _USE_TCC_AWESOMEPLAYER_
            if (mProfilingMode) {
                ALOGI("[PROFILE] Fetch is paused but fetch 1 page for 'Keep alive'");
            } else 
        #endif
            ALOGI("Keep alive");
        }

        fetchInternal();

        mLastFetchTimeUs = ALooper::GetNowUs();
    #if _USE_TCC_AWESOMEPLAYER_
        if ((mModes & EXTRA_CONNECTING) && !mPrefillDone) {
            int32_t appendedSourceState = (static_cast<HTTPBase *>(mSource.get())
                    ->getAppendedSourceState());
            if (mFetching && appendedSourceState != HTTPBase::AUX_SESSION_ACTIVATED 
                          && appendedSourceState != HTTPBase::AUX_SESSION_INACTIVE) {
                mFetchingInPreparePhase = false;
            }
        } else
#endif
        if (mFetching && mCache->totalSize() >= mHighwaterThresholdBytes) {
            ALOGI("Cache full, done prefetching for now - cache total size %d", mCache->totalSize());
        #if _USE_TCC_AWESOMEPLAYER_
            if (mFetching == true && mProfilingMode) {
                ALOGE("[PROFILE] Pause caching !! - onFetch(), Cache full !!");
            }
        #endif
            mFetching = false;

        #if _USE_TCC_AWESOMEPLAYER_
            if (mDisconnectAtHighwatermark && !mDisconnectedAtHighWatermark
                    && (mSource->flags() & DataSource::kWantsPrefetching) 
                    && !(mSource->flags() & DataSource::kIsHTTPLiveSource)) {

                off64_t lastBytePosCached = mCacheOffset + mCache->totalSize();
                if ((mLastAccessPos < lastBytePosCached) 
                        && (lastBytePosCached - mLastAccessPos > kPrefillHighWaterMarkBytes)) {
                    ALOGW("Disconnecting at high watermark");
                    static_cast<HTTPBase *>(mSource.get())->disconnect();
                    mDisconnectedAtHighWatermark = true;
                }
            }
        #endif
        }
    } else 
#if _USE_TCC_AWESOMEPLAYER_
    if (!mTeardown)
#endif
    {
        Mutex::Autolock autoLock(mLock);
        restartPrefetcherIfNecessary_l();
    }

    int64_t delayUs = 0;
    if (mFetching) {
        if (mFinalStatus != OK && mNumRetriesLeft > 0) {
        #if _USE_TCC_AWESOMEPLAYER_
            if (mReconnectWithNoDelay || mFinalStatus == INFO_RETRY) {
                ALOGW("We failed this time and will try again right now. "
                     "(mFinalStatus: %d)", mFinalStatus);
            } else if (!mTeardown)
        #endif
            {
                // We failed this time and will try again in 3 seconds.
                ALOGW("We failed this time and will try again in 3 seconds. "
                     "(mFinalStatus: %d, mNumRetriesLeft: %d)", mFinalStatus, mNumRetriesLeft);
                delayUs = 3000000ll;
            }
        }
    } else {
        delayUs = 100000ll;
    }

#if _USE_TCC_AWESOMEPLAYER_
    if (!mTeardown) 
#endif
    {
        (new AMessage(kWhatFetchMore, mReflector->id()))->post(delayUs);
    }
}

void NuCachedSource2::onRead(const sp<AMessage> &msg) {
    ALOGV("onRead");

    int64_t offset;
    CHECK(msg->findInt64("offset", &offset));

    void *data;
    CHECK(msg->findPointer("data", &data));

    size_t size;
    CHECK(msg->findSize("size", &size));

    ssize_t result =
#if _USE_TCC_AWESOMEPLAYER_
    (mTeardown || mStopped) ? ERROR_END_OF_STREAM :
#endif
    readInternal(offset, data, size);

    if (result == -EAGAIN) {
    #if _USE_TCC_AWESOMEPLAYER_
        if (mTeardown && result != ERROR_END_OF_STREAM) {
            ALOGW("tore down during readInternal");
            result = ERROR_END_OF_STREAM;
        } else 
    #endif
        {
            msg->post(10000); // android : 50000
            return;
        }
    }

    Mutex::Autolock autoLock(mLock);

    CHECK(mAsyncResult == NULL);

    mAsyncResult = new AMessage;
    mAsyncResult->setInt32("result", result);

    mCondition.signal();
}

#if _USE_TCC_AWESOMEPLAYER_
void NuCachedSource2::onFetchFile() {
    ALOGV("onFetchFile");

    if (mFinalStatus != OK) {
        ALOGV("EOS reached, done prefetching for now");
        mFetching = false;
    }

    off64_t sourceSize = 0;
    if (getSize(&sourceSize) == OK) {
        if (mFetching) {
            fetchInternal();

            ALOGV("onFetchFile: total size %d / %d", mCache->totalSize(), sourceSize - (sourceSize % kPageSize));

            if (mFetching && mCache->totalSize() >= sourceSize - (sourceSize % kPageSize)) {
                ALOGI("onFetchFile: Cache full, done prefetching file now" 
                        " - cache total size %d", mCache->totalSize());
                mFetching = false;
            }
        }

        int64_t delayUs = 0;
        if (mFetching) {
            if (mFinalStatus != OK) {
                // We failed this time and will try again in 3 seconds.
                ALOGW("We failed this time and will try again in 3 seconds. "
                        "(mFinalStatus: %d)", mFinalStatus);
                delayUs = 3000000ll;
            } else {
                delayUs = 0;
            }
        }

        if (mFetching 
        #if _USE_TCC_AWESOMEPLAYER_
            && !mTeardown
        #endif
            ) {
            (new AMessage(kWhatFetchFile, mReflector->id()))->post(delayUs);
        }
    } else {
        mFetching = false;
    }
}

void NuCachedSource2::onSeekDone(const sp<AMessage> &msg) {

    if (mAsyncResult != NULL) {
        ALOGW("seems onRead should be invoke before me ...");

        msg->post(10000); // android : 50000
        return;
    }

    int64_t timeUs;
    CHECK(msg->findInt64("time", &timeUs));

    ALOGD("onSeekDone: timeUs %lld, %.2f sec", timeUs, timeUs/1E6);

    status_t err = mSource->reconnectAtTime(timeUs);

    mLastSeekCompletedTimeUs = ALooper::GetNowUs();

    void (*seekDoneCb)(void *) = mSeekDoneCb;
    void (*seekSuccessDoneCb)(void *) = mSeekSuccessDoneCb;
    mSeekDoneCb = NULL;
    mSeekSuccessDoneCb = NULL;

    if (err == OK) {
        size_t totalSize = mCache->totalSize();
        mCacheOffset = mLastAccessPos;
        mSource->adjustOffset(mCacheOffset);
        CHECK_EQ(mCache->releaseFromStart(totalSize), totalSize);

        ALOGD("onSeekDone: LastAccessPos %lld Fetching %d, FinalStatus %d", 
                mLastAccessPos, mFetching, mFinalStatus);

        mFetching = true;
        mFinalStatus = OK;

        (*seekSuccessDoneCb)(mSeekDoneCookie);
    } else {
        (*seekDoneCb)(mSeekDoneCookie);
    }
            
    {
       Mutex::Autolock autoLock(mLock);
       mSeeking = false;
    }
}

bool NuCachedSource2::skipInternal(size_t alignmentSkipBytes) {
    CHECK_EQ(mFinalStatus, (status_t)OK);

    PageCache::Page *page = mCache->acquirePage();

	size_t size = (alignmentSkipBytes > 0) ? alignmentSkipBytes 
	            : ((mRemainedBytesToSkip > kPageSize) ? kPageSize 
	                                       : mRemainedBytesToSkip);

    ssize_t n = mSource->readAt(
            mCacheOffset + mCache->totalSize(), page->mData, size);

    Mutex::Autolock autoLock(mLock);

#if _USE_TCC_AWESOMEPLAYER_
    if (mTeardown) {
        ALOGE("skipInternal: tear down");
		mConsumedBytesToSkip = 0;
		mRemainedBytesToSkip = 0;
        mFinalStatus = ERROR_END_OF_STREAM;
        mCache->releasePage(page);
		return false;
	} 
#endif

    if (n < 0) {
        ALOGE("source returned error %ld", n);
        if(n == ERROR_CONNECTION_LOST) {
            ALOGW("ERROR_CONNECTION_LOST - ERROR_END_OF_STREAM");
            n = ERROR_END_OF_STREAM;
			mConsumedBytesToSkip = 0;
			mRemainedBytesToSkip = 0;
        }
        mFinalStatus = n;
		mCache->releasePage(page);
    } else if (n == 0) {
        ALOGI("ERROR_END_OF_STREAM");
		if(mRemainedBytesToSkip > kPageSize) {
			mFinalStatus = ERROR_END_OF_STREAM;
		}
		mCache->releasePage(page);
		mConsumedBytesToSkip = 0;
        mRemainedBytesToSkip = 0;
    } else {
        mConsumedBytesToSkip += n;
		if (alignmentSkipBytes > 0) {
			mRemainedBytesToSkip = 0;
		} else {
            mRemainedBytesToSkip -= n;
		}
        page->mSize = n;
		mCache->appendPage(page);
    }

	if(mRemainedBytesToSkip > 0) {
		ALOGW("skipInternal: skip more - Consume %d bytes, Remain %d bytes", 
			  mConsumedBytesToSkip, mRemainedBytesToSkip);
		return true; 
	} else {
		ALOGI("skipInternal: skip end - Consume %d bytes, Remain %d bytes", 
			  mConsumedBytesToSkip, mRemainedBytesToSkip);
		restartPrefetcherIfNecessary_l(true);
		return false; 
	}
}

void NuCachedSource2::onSkip() {
    ALOGV("onSkip: mFetching %d", mFetching);

    if (mFinalStatus != OK) {
        ALOGV("EOS reached, done prefetching for now");
    #if _USE_TCC_AWESOMEPLAYER_
        if (mFetching == true && mProfilingMode) {
            ALOGE("[PROFILE] Pause caching !! - onSkip()");
        }
    #endif
        mFetching = false;
	}

	bool moreSkip = false;
    bool keepSkipping =	
            !mSuspended
            && mFinalStatus == OK
            && ALooper::GetNowUs() <= mSkipStartTimeUs + kSkipDataTimeoutUs;

    if (mFetching) {
        if (keepSkipping) {
	        moreSkip = skipInternal();

	        mLastFetchTimeUs = ALooper::GetNowUs();

	        if (mFetching && mCache->totalSize() >= mHighwaterThresholdBytes) {
				ALOGI("onSkip: Cache full, clear all");
				Mutex::Autolock autoLock(mLock);
				restartPrefetcherIfNecessary_l(true);
	        }
        } else {
            ALOGW("onSkip: timeout !! - Consume %d bytes, Remain %d bytes", 
				  mConsumedBytesToSkip, mRemainedBytesToSkip);
			
            mRemainedBytesToSkip = 0;

			size_t alignBytes = mConsumedBytesToSkip % kLiveSourceUnitSize;
			moreSkip = skipInternal(kLiveSourceUnitSize - alignBytes);
			
            moreSkip = 0;
			mTimeoutSkipping = true;			
        }
    } else if (!mSuspended) {
		Mutex::Autolock autoLock(mLock);
		restartPrefetcherIfNecessary_l();
	}

#if _USE_TCC_AWESOMEPLAYER_
    if (!mTeardown) 
#endif
    {
        (new AMessage((moreSkip ? kWhatSkipMore : kWhatFetchMore), mReflector->id()))->post();
    }
}

void NuCachedSource2::onSuspend() {
    Mutex::Autolock autoLock(mLock);
#if _USE_TCC_AWESOMEPLAYER_
    if (mFetching == true && mProfilingMode) {
        ALOGE("[PROFILE] Pause caching !! - onSuspend()");
    }
#endif

    mFetching = false;
    mSuspended = true;

    mSuspendCondition.signal();
}

void NuCachedSource2::onStop() {
    Mutex::Autolock autoLock(mLock);
#if _USE_TCC_AWESOMEPLAYER_
    if (mFetching == true && mProfilingMode) {
        ALOGE("[PROFILE] Pause caching !! - onStop()");
    }
#endif

    mFetching = false;
    mStopped = true;
    mKeepAliveIntervalUs = 0;

    mStopCondition.signal();
}
#endif

void NuCachedSource2::restartPrefetcherIfNecessary_l(
        bool ignoreLowWaterThreshold, bool force) {
    static const size_t kGrayArea = 1024 * 1024;

    if (mFetching || (mFinalStatus != OK && mNumRetriesLeft == 0)) {
        return;
    }

    if (!ignoreLowWaterThreshold && !force
            && mCacheOffset + mCache->totalSize() - mLastAccessPos
                >= mLowwaterThresholdBytes) {
        return;
    }

    size_t maxBytes = mLastAccessPos - mCacheOffset;

    if (!force) {
        if (maxBytes < kGrayArea) {
            return;
        }

        maxBytes -= kGrayArea;
    }

    size_t actualBytes = mCache->releaseFromStart(maxBytes);
    mCacheOffset += actualBytes;

    ALOGI("restarting prefetcher, totalSize = %d", mCache->totalSize());
    mFetching = true;

#if _USE_TCC_AWESOMEPLAYER_
    if (mProfilingMode) {
        ALOGE("[PROFILE] cache w offset %lld, cache r offset %lld, avail %lld < low water mark %d",
                mCacheOffset + mCache->totalSize(), mLastAccessPos, 
                mCacheOffset + mCache->totalSize() - mLastAccessPos, mLowwaterThresholdBytes);

        if (mFetching == false) {
            ALOGE("[PROFILE] Resume caching !! - restartPrefetcherIfNecessary_l()");
        }
    }

    if (mDisconnectedAtHighWatermark
            && (mSource->flags() & DataSource::kWantsPrefetching) 
            && !(mSource->flags() & DataSource::kIsHTTPLiveSource)) {
        ALOGW("connect at low watermark");

        status_t err =
                mSource->reconnectAtOffset(mCacheOffset + mCache->totalSize());
        mDisconnectedAtHighWatermark = false;
    }
#endif
}

ssize_t NuCachedSource2::readAt(off64_t offset, void *data, size_t size) {
    Mutex::Autolock autoSerializer(mSerializer);

    ALOGV("readAt offset %lld, size %d", offset, size);

    Mutex::Autolock autoLock(mLock);

#if _USE_TCC_AWESOMEPLAYER_
    if((mModes & EXTRA_CONNECTING) && !mPrefillDone) {
        ALOGV("kPrepareStage:readAt offset %lld, size %d, mCacheOffset %lld, Cache total size %d", 
                offset, size, mCacheOffset, mCache->totalSize());

        HTTPBase* source = static_cast<HTTPBase *>(mSource.get());
        sp<ABuffer> rangedCache = source->getRangedCache();

        off64_t sourceSize = 0;
        CHECK(getSize(&sourceSize) == OK);
        if (rangedCache != NULL && sourceSize) {
            if (offset >= sourceSize - kDefaultAppendedSourceSize) {
                if (!mUseRangedCache) {
                    mUseRangedCache = true;
                    ALOGI("reatAt: Start range cache - offset:%lld, cache size:%d, start offset:%d", 
                            offset, rangedCache->size(), sourceSize - kDefaultAppendedSourceSize);
                }
            } else if (mUseRangedCache) {
                ALOGI("reatAt: Stop range cache - offset:%lld, mRangeByteConsumed:%d,", 
                        offset, mRangeByteConsumed);
                mUseRangedCache = false;
                mRangeByteConsumed = 0;
            }
        }

        if (mUseRangedCache) {
            ALOGD("reatAt: range cache mode (offset:%lld, size:%d, consumed %d)",
                    offset, size, mRangeByteConsumed);
            ssize_t n;
            if (rangedCache->capacity() - mRangeByteConsumed >= size) {
                memcpy(data, rangedCache->data() + mRangeByteConsumed, size);
                mRangeByteConsumed += size;
                n = size;
            } else if (rangedCache->capacity() - mRangeByteConsumed > 0) {
                memcpy(data, rangedCache->data() + mRangeByteConsumed,
                        rangedCache->capacity() - mRangeByteConsumed);
                n = rangedCache->capacity() - mRangeByteConsumed;
            } else {
                ALOGI("kPrepareStage: ERROR_END_OF_STREAM");
                n = ERROR_END_OF_STREAM;
            }
            return n;
        }
    }
#endif

    // If the request can be completely satisfied from the cache, do so.

    if (offset >= mCacheOffset
            && offset + size <= mCacheOffset + mCache->totalSize()) {
        size_t delta = offset - mCacheOffset;
        mCache->copy(delta, data, size);

        mLastAccessPos = offset + size;

        return size;
    }

    sp<AMessage> msg = new AMessage(kWhatRead, mReflector->id());
    msg->setInt64("offset", offset);
    msg->setPointer("data", data);
    msg->setSize("size", size);

    CHECK(mAsyncResult == NULL);
    msg->post();

    while (mAsyncResult == NULL) {
        mCondition.wait(mLock);
    }

    int32_t result;
    CHECK(mAsyncResult->findInt32("result", &result));

    mAsyncResult.clear();

    if (result > 0) {
        mLastAccessPos = offset + result;
    }

    return (ssize_t)result;
}

size_t NuCachedSource2::cachedSize() {
    Mutex::Autolock autoLock(mLock);
    return mCacheOffset + mCache->totalSize();
}

size_t NuCachedSource2::approxDataRemaining(status_t *finalStatus, off64_t *lastBytePos) {
    //Mutex::Autolock autoLock(mLock);
    return approxDataRemaining_l(finalStatus, lastBytePos);
}

size_t NuCachedSource2::approxDataRemaining_l(status_t *finalStatus, off64_t *lastBytePos) {
    *finalStatus = mFinalStatus;

    if (mFinalStatus != OK && mNumRetriesLeft > 0) {
		ALOGW("approxDataRemaining_l: mFinalStatus = %d", mFinalStatus);
        // Pretend that everything is fine until we're out of retries.
        *finalStatus = OK;
    }

#if _USE_TCC_AWESOMEPLAYER_
    if (!mFetching && *finalStatus == OK) {
        *finalStatus = ERROR_BUFFER_TOO_SMALL;
	}
#endif

    off64_t lastBytePosCached = mCacheOffset + mCache->totalSize();
#if _USE_TCC_AWESOMEPLAYER_
    if (lastBytePos != NULL) {
        *lastBytePos = lastBytePosCached;
    }
#endif
    if (mLastAccessPos < lastBytePosCached) {
        return (size_t)(lastBytePosCached - mLastAccessPos);
    }
    return 0;
}

ssize_t NuCachedSource2::readInternal(off64_t offset, void *data, size_t size) {
    CHECK_LE(size, (size_t)mHighwaterThresholdBytes);

    ALOGV("readInternal offset %lld size %d", offset, size);

    Mutex::Autolock autoLock(mLock);
#if _USE_TCC_AWESOMEPLAYER_
    if (mFetchingInPreparePhase == false) {
	    if ((mModes & EXTRA_CONNECTING) && mPrefillDone) {
			ALOGI("readInternal: Turn off preparing fetch state !!");
			mFetchingInPreparePhase = true;

            if(mFetching == false && mProfilingMode) {
                ALOGE("[PROFILE] Resume caching !! - readInternal()");
            }

			mFetching = true;
            mModes &= ~EXTRA_CONNECTING;
		}
	} else
#endif
    if (!mFetching) {
        mLastAccessPos = offset;
        restartPrefetcherIfNecessary_l(
                false, // ignoreLowWaterThreshold
                true); // force
    }

    if (offset < mCacheOffset
            || offset > /*>=*/ (off64_t)(mCacheOffset + mCache->totalSize())) {
        static const off64_t kPadding = 256 * 1024;

        // In the presence of multiple decoded streams, once of them will
        // trigger this seek request, the other one will request data "nearby"
        // soon, adjust the seek position so that that subsequent request
        // does not trigger another seek.
        off64_t seekOffset = (offset > kPadding) ? offset - kPadding : 0;

        seekInternal_l(seekOffset);
    }

    size_t delta = offset - mCacheOffset;

#if _USE_TCC_AWESOMEPLAYER_
    if (mFinalStatus == INFO_RETRY) {
        return -EAGAIN;
    }
#endif
    if (mFinalStatus != OK) {
        if (delta >= mCache->totalSize()) {
            return mFinalStatus;
        }

        size_t avail = mCache->totalSize() - delta;

        if (avail > size) {
            avail = size;
        }

        mCache->copy(delta, data, avail);

        return avail;
    }

    if (offset + size <= mCacheOffset + mCache->totalSize()) {
        mCache->copy(delta, data, size);

        return size;
    }

    ALOGV("deferring read");

    return -EAGAIN;
}

status_t NuCachedSource2::seekInternal_l(off64_t offset) {
    mLastAccessPos = offset;

    if (offset >= mCacheOffset
            && offset <= (off64_t)(mCacheOffset + mCache->totalSize())) {
        return OK;
    }

    ALOGI("new range: offset= %lld", offset);

    mCacheOffset = offset;

    size_t totalSize = mCache->totalSize();
    CHECK_EQ(mCache->releaseFromStart(totalSize), totalSize);

    mNumRetriesLeft = kMaxNumRetries;
    mFetching = true;
    mFinalStatus = OK;

#if _USE_TCC_AWESOMEPLAYER_
    if (mFetching == false && mProfilingMode) {
        ALOGE("[PROFILE] Resume caching !! - seekInternal_l()");
    }
#endif

    return OK;
}

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
bool NuCachedSource2::doesKeepFetching() {
    Mutex::Autolock autoLock(mLock);
	return mFetching;
}

void NuCachedSource2::suspend() {
    Mutex::Autolock autoLock(mLock);

    if (!mSuspended) {
        ALOGD("Suspending cache...");
        (new AMessage(kWhatSuspend, mReflector->id()))->post/*AtFront*/();
        while (!mSuspended) {
            mSuspendCondition.wait(mLock);
        }
    }
    ALOGD("cache is now suspended");
}

void NuCachedSource2::stop() {
    if (mStopped) return;

    Mutex::Autolock autoLock(mLock);

    mSuspended = true;
	(new AMessage(kWhatStop, mReflector->id()))->post/*AtFront*/();
    ALOGD("request to stop cache...");
	while (!mStopped) {
		mStopCondition.wait(mLock);
	}
    ALOGD("cache is now stopped");
}

void NuCachedSource2::resume() {
    Mutex::Autolock autoLock(mLock);
#if _USE_TCC_AWESOMEPLAYER_
    if (mFetching == false && mProfilingMode) {
        ALOGE("[PROFILE] Resume caching !! - resume()");
    }
#endif
    mFinalStatus = OK;
    mFetching = true;
    mSuspended = false;
    mStopped = false;
    mModes &= ~PARTIAL_FETCHING;

	ALOGI("resume");

    (new AMessage(kWhatFetchMore, mReflector->id()))->post();
}

void NuCachedSource2::clearCacheAndResume(off64_t skipBytesAtResume) {
    Mutex::Autolock autoLock(mLock);

    ALOGD("clearCacheAndResume:mFetching(%d), mSuspended(%d), mFinalStatus(%d)",
         mFetching, mSuspended, mFinalStatus);

    mCacheOffset = 0;
    mFinalStatus = OK;
    mLastAccessPos = 0;
    mLastFetchTimeUs = -1;

    size_t totalSize = mCache->totalSize();
    CHECK_EQ(mCache->releaseFromStart(totalSize), totalSize);

#if _USE_TCC_AWESOMEPLAYER_
    mSuspended = false;
    mStopped = false;
#endif

    if (skipBytesAtResume) {
        ALOGI("skipBytesAtResume %d bytes", skipBytesAtResume);
        mConsumedBytesToSkip = 0;
        mRemainedBytesToSkip = skipBytesAtResume;
        mSkipStartTimeUs = ALooper::GetNowUs();
        (new AMessage(kWhatSkipMore, mReflector->id()))->post();
    } else {
        mFetching = true;
        (new AMessage((mModes & PARTIAL_FETCHING) ? kWhatFetchFile : kWhatFetchMore, 
                      mReflector->id()))->post();
    }

#if _USE_TCC_AWESOMEPLAYER_
    if (mFetching == false && mProfilingMode) {
        ALOGE("[PROFILE] Resume caching !! - clearCacheAndResume()");
    }
#endif
    if (!mFetching) mFetching = true;
    if (mKeepAliveIntervalUs == 0) {
        mKeepAliveIntervalUs = mPrevKeepAliveIntervalUs;
    }
}
#endif

void NuCachedSource2::resumeFetchingIfNecessary() {
    Mutex::Autolock autoLock(mLock);

    restartPrefetcherIfNecessary_l(true /* ignore low water threshold */);
}

sp<DecryptHandle> NuCachedSource2::DrmInitialization(const char* mime) {
    return mSource->DrmInitialization(mime);
}

void NuCachedSource2::getDrmInfo(sp<DecryptHandle> &handle, DrmManagerClient **client) {
    mSource->getDrmInfo(handle, client);
}

String8 NuCachedSource2::getUri() {
    return mSource->getUri();
}

String8 NuCachedSource2::getMIMEType() const {
    return mSource->getMIMEType();
}

void NuCachedSource2::updateCacheParamsFromSystemProperty() {
    char value[PROPERTY_VALUE_MAX];
    if (!property_get("media.stagefright.cache-params", value, NULL)) {
        return;
    }

    updateCacheParamsFromString(value);
}

void NuCachedSource2::updateCacheParamsFromString(const char *s) {
    ssize_t lowwaterMarkKb, highwaterMarkKb;
    int keepAliveSecs;

    if (sscanf(s, "%ld/%ld/%d",
               &lowwaterMarkKb, &highwaterMarkKb, &keepAliveSecs) != 3) {
        ALOGE("Failed to parse cache parameters from '%s'.", s);
        return;
    }

    if (lowwaterMarkKb >= 0) {
        mLowwaterThresholdBytes = lowwaterMarkKb * 1024;
    } else {
        mLowwaterThresholdBytes = kDefaultLowWaterThreshold;
    }

    if (highwaterMarkKb >= 0) {
        mHighwaterThresholdBytes = highwaterMarkKb * 1024;
    } else {
        mHighwaterThresholdBytes = kDefaultHighWaterThreshold;
    }

    if (mLowwaterThresholdBytes >= mHighwaterThresholdBytes) {
        ALOGE("Illegal low/highwater marks specified, reverting to defaults.");

        mLowwaterThresholdBytes = kDefaultLowWaterThreshold;
        mHighwaterThresholdBytes = kDefaultHighWaterThreshold;
    }

    if (keepAliveSecs >= 0) {
        mKeepAliveIntervalUs = keepAliveSecs * 1000000ll;
    } else {
        mKeepAliveIntervalUs = kDefaultKeepAliveIntervalUs;
    }

    ALOGV("lowwater = %d bytes, highwater = %d bytes, keepalive = %lld us",
         mLowwaterThresholdBytes,
         mHighwaterThresholdBytes,
         mKeepAliveIntervalUs);
}

// static
void NuCachedSource2::RemoveCacheSpecificHeaders(
        KeyedVector<String8, String8> *headers,
        String8 *cacheConfig,
        bool *disconnectAtHighwatermark) {
    *cacheConfig = String8();
    *disconnectAtHighwatermark = false;

    if (headers == NULL) {
        return;
    }

    ssize_t index;
    if ((index = headers->indexOfKey(String8("x-cache-config"))) >= 0) {
        *cacheConfig = headers->valueAt(index);

        headers->removeItemsAt(index);

        ALOGV("Using special cache config '%s'", cacheConfig->string());
    }

    if ((index = headers->indexOfKey(
                    String8("x-disconnect-at-highwatermark"))) >= 0) {
        *disconnectAtHighwatermark = true;
        headers->removeItemsAt(index);

        ALOGV("Client requested disconnection at highwater mark");
    }
}

}  // namespace android
