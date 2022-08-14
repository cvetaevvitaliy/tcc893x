/*
 * Copyright (C) 2010 Telechips, Inc.
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

#ifndef TCCMEDIAEXTRACTOR_H_

#define TCCMEDIAEXTRACTOR_H_

#include <media/stagefright/MediaExtractor.h>
#include <utils/Vector.h>
#include "cdk_wrapper_stagefright.h"

namespace android {

class DataSource;
class SampleTable;

class TCCMediaExtractor : public MediaExtractor {
public:
    // Extractor assumes ownership of "source".
    TCCMediaExtractor(const sp<DataSource> &source, const char *notifiedMIME = NULL);

    virtual size_t countTracks();
    virtual sp<MediaSource> getTrack(size_t index);
    virtual sp<MetaData> getTrackMetaData(size_t index, uint32_t flags);

    virtual sp<MetaData> getMetaData();
    virtual sp<MetaData> getMetaData(bool withThumbnail);
	virtual uint32_t flags() const;
	virtual uint32_t getMyType() const { return MediaExtractor::TCC; }
    virtual void feedMore(
                 int64_t videoThresholdUs = 3000000ll, 
                 int64_t audioThresholdUs = 0);
    virtual int32_t checkQueuedDurationUs(int64_t *durationUs, bool *fulled);
    virtual void cancelSeekIfNecessary(bool cancelled);
    virtual void signalUnderrun(bool underrun, int64_t thresholdUs = 0);
	
	virtual int32_t postInitIfNecessary(int64_t prefillUs = 0);
	virtual int32_t getNumSubtitleStreams();
	virtual int32_t getCurrentSubtitleStream(void);
	virtual char* getLangSubtitleStream(void);
	// INTERNAL_SUBTITLE_INCLUDE
	virtual int32_t GetSubtitleType(void);
	virtual int32_t SelectSubtitleStream(int32_t num);
	virtual void  SetCurrentPlaybackPosition(int64_t usec); 
    virtual int32_t GetSubtitleData(); 		
    virtual int32_t SetSubtitleSeek(int32_t msec);
	virtual int32_t get_inter_subtitle_type();
    virtual void ClearStreamQueue();
    virtual void SkipAudioStream();
    virtual uint32_t GetVideoBufferedTime();

    int32_t SetTCCCodecInfo(void);

protected:
    virtual ~TCCMediaExtractor();

private:
	enum {
		kMediaScanNo = 0,
		kMediaScanMetaOnly = 1,
		kMediaScanMetaWithFrame = 2,
	};

    struct Track {
        Track *next;
        sp<MetaData> meta;
        bool includes_expensive_metadata;
        bool skipTrack;
		uint32_t codec;
    };

    mutable Mutex mLock;

    sp<DataSource> mDataSource;
    String8 mNotifiedMIME;
	sp<CDKWrapperStagefright> mCDK;

    bool mHaveMetadata;
    bool mHasVideo;
	bool mInitResult;

    Track *mFirstTrack, *mLastTrack;

    sp<MetaData> mFileMetaData;

    status_t readMetaData(int32_t getMetadataMode);
    status_t parseFile(int32_t getMetadataMode);
    void parseMetaData();

    void selectStream(int num);

    bool mIsDrm;
		
    TCCMediaExtractor(const TCCMediaExtractor &);
    TCCMediaExtractor &operator=(const TCCMediaExtractor &);
};

bool SniffTCCMedia(
        const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> *);

}  // namespace android

#endif  // TCCMEDIAEXTRACTOR_H_
