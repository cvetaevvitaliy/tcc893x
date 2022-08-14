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

#ifndef TCC_LIVE_SOURCE_H_

#define TCC_LIVE_SOURCE_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AHandlerReflector.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/DataSource.h>
#include <utils/Vector.h>

namespace android {

struct ALooper;
struct ABuffer;
struct HTTPBase;
struct SfM3UParser;
struct SfSegmentDataSource;

struct SfLiveSource : public DataSource {
    SfLiveSource(const char *url, const sp<HTTPBase> &source, 
                 const KeyedVector<String8, String8> &headers,
                 uint32_t flags);

    enum {
        BANDWIDTH_NO_MARGIN = 10,
        BANDWIDTH_MARGIN_PERCENT_10 = 9,
        BANDWIDTH_MARGIN_PERCENT_20 = 8,
        BANDWIDTH_MARGIN_PERCENT_30 = 7,
        BANDWIDTH_MARGIN_PERCENT_40 = 6,
        BANDWIDTH_MARGIN_PERCENT_50 = 5,
        BANDWIDTH_MARGIN_PERCENT_60 = 4,
        BANDWIDTH_MARGIN_PERCENT_70 = 3,
        BANDWIDTH_MARGIN_PERCENT_80 = 2,
        BANDWIDTH_MARGIN_PERCENT_90 = 1,
    };

    enum {
        NORMAL_INIT = 0,
        PRE_INIT,
        POST_INIT,
    };

	virtual status_t initCheck() const;

    virtual ssize_t readAt(off64_t offset, void *data, size_t size);

    virtual status_t getSize(off64_t *size);
    virtual uint32_t flags() {
        return kWantsPrefetching | kIsHTTPLiveSource;
    }

	virtual void teardown();

    bool hasReloadablePlaylist() const;
    bool hasVariantPlaylist() const;

	status_t init(int32_t mode = 0);

    void unloadExpirationChecker(bool halted = false);
    void pauseAsync();
    bool doesStreamExpired();

	bool IsAES128Encrypted();
	bool checkStartSegmentFetched();
	size_t getBandwidthIndex(size_t margin = BANDWIDTH_MARGIN_PERCENT_20);

    size_t getPrevBandwidthIndex();

    bool removeBandwidthItem(size_t index);
    status_t setProperPlaylist(bool force = false, size_t reqIndex = 0);

    const char *getNotifiableMediaURL();

    bool getDuration(int64_t *durationUs) const;
    bool getBitrate(int64_t *bitrate);
    bool isSeekable() const;
    status_t checkCipherKey(sp<AMessage> itemMeta = NULL);
    void startWithSwitchedPlaylist();

    bool seekTo(int64_t& seekTimeUs, 
	            int64_t* pBaseTimeUs = NULL, 
	            int64_t* pLBiasTimeUs = NULL, 
				int64_t* pRBiasTimeUs = NULL,
				int64_t* pSeekTimeOffet = NULL);

	void onMessageReceived(const sp<AMessage> &msg);

protected:
    virtual ~SfLiveSource();

private:
    enum {
        kWhatReload        = 'reLd',
        kWhatReloadWaiting = 'rLwt',
        kWhatReloadDone    = 'rLdn',
	};

    enum {
        EXPIRE_CHECK_OFF  = 0,
        EXPIRE_CHECK_ON   = 1,
        EXPIRE_CHECK_STOP = 2,
	};

	int32_t mExpireCheckStatus;
    int64_t mExpireDurationUs;

	status_t mReloadResult;
    int32_t mPlaylistReloadCount;
    int32_t mReloadWaitingCount;
    int32_t mReloadWaitingEntranceCount;

    bool mIsLivePlaylist;
    bool mEnableAccurateSeek;

	struct BandwidthItem {
        AString mURI;
        AString mURI_1;
        AString mURI_2;
        AString mURI_3;
        unsigned long mBandwidth;
    };
    Vector<BandwidthItem> mBandwidthItems;

	struct DurationItem {
        size_t mIndex;
		int64_t mDurationUs;
		int64_t mAbsoluteDurationUs;
    };
    Vector<DurationItem> mDurationItems;

    Mutex mLock;
    Condition mCondition;

    Mutex mSerializer;
    Condition mSerializedCondition;
    bool mReadAtDone;

    sp<ALooper> mLooper;
    sp<AHandlerReflector<SfLiveSource> > mReflector;
    sp<AMessage> mAsyncResult;

    AString mMasterURL;
    AString mURL;
    status_t mInitCheck;
    int32_t mInitMode;
    int64_t mDurationUs;
    int64_t mSeekClipStartTimeUs;
    int64_t mSeekClipEndTimeUs;

    bool mIsM3UConnected;

	int32_t mTargetDuration;
	bool mTargetDurationValid;

    sp<SfM3UParser> mTCCPlaylist;
    int32_t mLastItemSequenceNumber;
    int64_t mLastItemEndTimestamp;
    size_t mPlaylistIndex;
    int32_t mStartPlaylistIndex;
    int64_t mLastSegmentFetchTimeUs;
    int64_t mLastPlaylistFetchTimeUs;
    int64_t mPlaylistFetchDelayTimeUs;

    int64_t mBitrate;

    bool mAES128Applied;
    bool mUseAnotherAESKeyUrl;
    KeyedVector<AString, sp<ABuffer> > mAESKeyForURI;

    sp<HTTPBase> mHTTPDataSource;
    sp<SfSegmentDataSource> mSegmentDataSource;

    KeyedVector<String8, String8> mExtraHeaders;
    uint32_t mFlags;

    off64_t mSourceSize;
    off64_t mStartSourceSegmentSize;
    off64_t mOffsetBias;
    off64_t mEndOffsetBias;

    ssize_t mPrevBandwidthIndex;
    size_t mPrevPlaylistIndex;
    int32_t mPlaylistStartIndex;

	int64_t mSeekTimeUs;
	bool mSeeking;
	bool mUseURIFullPath;
	bool mUseSecondaryURI;
	bool mWayToMakeURLFixed;

	bool mEndOfStream;

    bool mContentLengthOmitted;
	bool mTeardown;

    bool probeDiscontinuityTag(int64_t& lastDiscontinuityIndex);
    status_t fetchM3U(const char *url, sp<ABuffer> *buffer, bool checkOption = false);
    static int SortByBandwidth(const BandwidthItem *a, const BandwidthItem *b);

    status_t reloadPlaylistFile();
    status_t switchToFirst(bool fetchMaster = true, bool checkOption = false);
    status_t switchToNext(
		    bool seeking = false, int64_t seekTimeUs = 0, 
            int64_t* pEstimatedSeekOffset = NULL, bool reloaded = false);

    status_t loadPlaylist(bool fetchMaster, bool checkOption = false);
    status_t fetchSegment(const char *url, sp<ABuffer> *out);
    status_t decryptBuffer(
            size_t playlistIndex, const sp<ABuffer> &buffer);
	void determineSeekability();

    DISALLOW_EVIL_CONSTRUCTORS(SfLiveSource);
};

}  // namespace android

#endif  // TCC_LIVE_SOURCE_H_
