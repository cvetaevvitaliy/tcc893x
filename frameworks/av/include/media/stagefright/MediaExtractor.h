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

#ifndef MEDIA_EXTRACTOR_H_

#define MEDIA_EXTRACTOR_H_

#include <utils/RefBase.h>

namespace android {

class DataSource;
class MediaSource;
class MetaData;

class MediaExtractor : public RefBase {
public:
    static sp<MediaExtractor> Create(
            const sp<DataSource> &source, const char *mime = NULL
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
            , bool useOriginal = false, const char *notifiedMIME = NULL // TELECHIPS
#endif
    );

    virtual size_t countTracks() = 0;
    virtual size_t countTracks(int32_t& error) { return countTracks(); }; // TELECHIPS
    virtual sp<MediaSource> getTrack(size_t index) = 0;

    enum GetTrackMetaDataFlags {
        kIncludeExtensiveMetaData = 1
    };
    virtual sp<MetaData> getTrackMetaData(
            size_t index, uint32_t flags = 0) = 0;

    // Return container specific meta-data. The default implementation
    // returns an empty metadata object.
    virtual sp<MetaData> getMetaData();

    // TELECHIPS
    virtual int32_t getNumSubtitleStreams();
    virtual int32_t getCurrentSubtitleStream(void);
    virtual char*   getLangSubtitleStream(void);
    virtual int32_t GetSubtitleType(void);
    virtual int32_t SelectSubtitleStream(int32_t num);
    virtual int32_t GetSubtitleData();
    virtual int32_t SetSubtitleSeek(int msec);
    virtual int32_t get_inter_subtitle_type();
    virtual void SetCurrentPlaybackPosition(int64_t usec);

    virtual int32_t postInitIfNecessary(int64_t prefillUs = 0);
    virtual void feedMore(
                 int64_t videoThresholdUs = 3000000ll, 
                 int64_t audioThresholdUs = 0) {}
    virtual int32_t checkQueuedDurationUs(int64_t *durationUs, bool *fulled);
    virtual void cancelSeekIfNecessary(bool cancelled) {}
    virtual void signalUnderrun(bool underrun, int64_t thresholdUs = 0) {}

    virtual sp<MetaData> getMetaData(bool withThumbnail); 
    virtual void ClearStreamQueue() { return; };
    virtual void SkipAudioStream() { return; };
    virtual uint32_t GetVideoBufferedTime();

    enum Flags {
        CAN_SEEK_BACKWARD  = 1,  // the "seek 10secs back button"
        CAN_SEEK_FORWARD   = 2,  // the "seek 10secs forward button"
        CAN_PAUSE          = 4,
        CAN_SEEK           = 8,  // the "seek bar"
    };

    // TELECHIPS, HEES
    // Some omx video decoders might want to know it 
    // to assign appropriate timestamp to output frame.
    // CAUSION : enum constants should be synchronized with 
    // OMX_BUFFERFLAG_EXTRACTORTYPE_(ExtractorType) in OMX_Core.h
    enum ExractorType {
        UNASSIGNED = 0,
        MATROSKA   = 1 << 16, 
        MPEG2TS    = 1 << 17,
        MPEG4      = 1 << 18,
        TCC        = 1 << 19,
        RTSP       = 1 << 20,
        WFD        = 1 << 21,
    };

    virtual uint32_t getMyType() const { return UNASSIGNED; }

    // If subclasses do _not_ override this, the default is
    // CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_SEEK | CAN_PAUSE
    virtual uint32_t flags() const;

    // for DRM
    void setDrmFlag(bool flag) {
        mIsDrm = flag;
    };
    bool getDrmFlag() {
        return mIsDrm;
    }
    virtual char* getDrmTrackInfo(size_t trackID, int *len) {
        return NULL;
    }

protected:
    MediaExtractor() : mIsDrm(false) {}
    virtual ~MediaExtractor() {}

private:
    bool mIsDrm;

    MediaExtractor(const MediaExtractor &);
    MediaExtractor &operator=(const MediaExtractor &);
};

}  // namespace android

#endif  // MEDIA_EXTRACTOR_H_
