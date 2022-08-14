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

#define LOG_TAG "TCCMediaExtractor"
#include <utils/Log.h>

#include "TCCMediaExtractor.h"
#include "TCCStagefrightDefine.h"

#include "include/NuCachedSource2.h"

#include <arpa/inet.h>

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <binder/ProcessState.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaBufferGroup.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaSource.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#ifdef OPENMAX1_2
#include <media/MetaDataExt.h>
#endif
#include <utils/String8.h>

#include "tcc_video_config_data.h"
#include "albumart_extractor.h"
#include "cdk_android.h"

#include <cutils/properties.h>

//#define DEBUG
#ifdef DEBUG
#define LOGMSG(x...) ALOGD(x)
#else
#define LOGMSG(x...)
#endif

//#define PRINT_DEBUG_LOG
#ifdef PRINT_DEBUG_LOG
#define DLOGD(x...) ALOGD(x)
#define DLOGE(x...) ALOGE(x)
#define DLOGI(x...) ALOGI(x)
#else
#define DLOGD(x...)
#define DLOGE(x...)
#define DLOGI(x...)
#endif

#define PRINT_FILE_TYPE
#ifdef PRINT_FILE_TYPE
#define FILETYPEMSG(x...) ALOGI(x)
#else
#define FILETYPEMSG(x...)
#endif

#define dim(x) (sizeof(x) / sizeof(x[0]))

namespace android {

static const int64_t THUMBNAIL_TIME_SEC = 0;
static const int64_t WFD_LPCM_FRAME_SIZE = 1924;

enum {
	kTCCCodecUnknown = 0,
	kTCCCodecAVC = 1,
	kTCCCodecMPEG4,
	kTCCCodecH263,
	kTCCCodecVC1,
	kTCCCodecWMV12,
	kTCCCodecRV,
	kTCCCodecMPEG2,
	kTCCCodecFLV1,
	kTCCCodecMJPEG,
	kTCCCodecAVS,
	kTCCCodecVP8,
	kTCCCodecVP9,
	kTCCCodecDIVX,
	kTCCCodecMVC,

	kTCCCodecMP3 = 20,
	kTCCCodecAAC,
	kTCCCodecAC3,
	kTCCCodecTRUEHD,
	kTCCCodecDDP,
	kTCCCodecDTS,
	kTCCCodecDTSHD,
	kTCCCodecPCM,
	kTCCCodecVORBIS,
	kTCCCodecFLAC,
	kTCCCodecAPE,
	kTCCCodecWMA,
	kTCCCodecRA,
	kTCCCodecMP2,
	kTCCCodecAMR,
	kTCCCodecAMRWB,

	kTCCCodecText = 40,
	kTCCCodecGraphic,

	kTCCCodecFirstAudio = kTCCCodecMP3,
	kTCCCodecLastAudio = kTCCCodecAMRWB,
	kTCCCodecFirstVideo = kTCCCodecAVC,
	kTCCCodecLastVideo = kTCCCodecMVC,
};

enum {
	kTCCContainerNONE = 0,
	kTCCContainerMKV,
	kTCCContainerMP4,
	kTCCContainerAVI,
	kTCCContainerMPG,
	kTCCContainerTS,
	kTCCContainerASF,
	kTCCContainerRMFF,
	kTCCContainerAUDIO,
	kTCCContainerOGG,
	kTCCContainerFLV,
};

typedef struct {
	const char *ext;
	const char *mimeType;
	const float confidence;
	const int checkType;
	const char *fileTypeMsg;
} T_CDK_SNIFF_WITH_EXT;

const static T_CDK_SNIFF_WITH_EXT tSniffListWithExt[] = {
	// MPG file
	{".MPG",  MEDIA_MIMETYPE_CONTAINER_MPG,      0.41f, 0, "mpg file"}, 
	{".VOB",  MEDIA_MIMETYPE_CONTAINER_MPG,      0.41f, 0, "mpg file"}, 
	{".DAT",  MEDIA_MIMETYPE_CONTAINER_MPG,      0.41f, 0, "mpg file"}, 
	{".MPEG", MEDIA_MIMETYPE_CONTAINER_MPG,      0.41f, 0, "mpg file"}, 
	// MP4 file
	{".MP4",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".M4A",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".M4P",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".M4B",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".M4R",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".M4V",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".MOV",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	{".3GP",  MEDIA_MIMETYPE_CONTAINER_MPEG4,    0.3f,  0, "mp4/mov file"}, 
	// AVI file
	{".AVI",  MEDIA_MIMETYPE_CONTAINER_AVI,      0.4f,  0, "avi file"}, 
	// OGM file
	{".OGM",  MEDIA_MIMETYPE_CONTAINER_OGM,      0.4f,  1, "ogm file"}, 
	// RMVB file
	{".RMVB", MEDIA_MIMETYPE_CONTAINER_RM,       0.4f,  0, "rmvb file"}, 
	// Windows Media file
	{".WMV",  MEDIA_MIMETYPE_CONTAINER_WMV,      1.0f,  0, "windows media file"}, 
	// MKV file
	{".MKV",  MEDIA_MIMETYPE_CONTAINER_MATROSKA, 0.7f,  0, "mkv file"}, 
	// FLAC file
	{".FLAC", MEDIA_MIMETYPE_CONTAINER_FLAC,     0.4f,  0, "flac file"}, 
	// APE file
	{".APE",  MEDIA_MIMETYPE_CONTAINER_APE,      0.4f,  0, "ape file"}, 
	// FLV file
	{".FLV",  MEDIA_MIMETYPE_CONTAINER_FLV,      0.4f,  0, "flv file"}, 
	{".F4V",  MEDIA_MIMETYPE_CONTAINER_FLV,      0.4f,  0, "flv file"}, 
	{".F4P",  MEDIA_MIMETYPE_CONTAINER_FLV,      0.4f,  0, "flv file"}, 
	{".F4A",  MEDIA_MIMETYPE_CONTAINER_FLV,      0.4f,  0, "flv file"}, 
	{".F4B",  MEDIA_MIMETYPE_CONTAINER_FLV,      0.4f,  0, "flv file"}, 
	// TS file
	{".TS",   MEDIA_MIMETYPE_CONTAINER_TS,       0.2f,  0, "ts file"}, 
	{".TP",   MEDIA_MIMETYPE_CONTAINER_TS,       0.2f,  0, "ts file"}, 
	{".TRP",  MEDIA_MIMETYPE_CONTAINER_TS,       0.2f,  0, "ts file"}, 
	{".M2TS", MEDIA_MIMETYPE_CONTAINER_TS,       0.2f,  0, "ts file"}, 
    {".SSIF", MEDIA_MIMETYPE_CONTAINER_SSIF,     0.2f,  0, "ts file"}, 
};
  
class TCCMediaSource : public MediaSource {
public:
    TCCMediaSource(const sp<MetaData> &format,
                const sp<DataSource> &dataSource,
                const sp<TCCMediaExtractor> &extractor,
				const sp<CDKWrapperStagefright> &CDK,
				const uint32_t codec);

#ifdef USE_HDCP_DECRYPTION
    virtual status_t setBuffers(const Vector<MediaBuffer *> &buffers);
#endif
    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();

    virtual sp<MetaData> getFormat();

    status_t setSkimmingOptions(const ReadOptions *options);
    status_t checkSeekRangeValidity(int64_t *seekTime);

    virtual status_t read(
            MediaBuffer **buffer, const ReadOptions *options = NULL);

protected:
    virtual ~TCCMediaSource();

private:
	enum {
		THUMBNAIL_RETRY_COUNT = 50,
	};

    Mutex mLock;

    sp<MetaData> mFormat;
    sp<DataSource> mDataSource;
	sp<TCCMediaExtractor> mExtractor;
	sp<CDKWrapperStagefright> mCDK;

    bool mStarted;
    bool mParsingStarted;
	bool mSourceDone;//JS Baek
	uint32_t mThumbFrameTryCount;
	int64_t mSkimInterval;//JS Baek

    MediaBufferGroup *mGroup;

	uint32_t mCodecType;
	uint32_t mStreamType;
	int32_t mPlayRate;
	int32_t mPrevPlayMode;
	int32_t mInterSubtitleType; // INTERNAL_SUBTITLE_INCLUDE
#ifdef USE_HDCP_DECRYPTION
    bool mNeedDecryption;
#endif
    size_t parseNALSize(const uint8_t *data) const;

    TCCMediaSource(const TCCMediaSource &);
    TCCMediaSource &operator=(const TCCMediaSource &);
};

// SSG, below class is maybe used for progressive play(HTTP)

// This custom data source wraps an existing one and satisfies requests
// falling entirely within a cached range from the cache while forwarding
// all remaining requests to the wrapped datasource.
// This is used to cache the full sampletable metadata for a single track,
// possibly wrapping multiple times to cover all tracks, i.e.
// Each MPEG4DataSource caches the sampletable metadata for a single track.

struct TCCMediaDataSource : public DataSource {
    TCCMediaDataSource(const sp<DataSource> &source);

    virtual status_t initCheck() const;
    virtual ssize_t readAt(off_t offset, void *data, size_t size);
    virtual status_t getSize(off64_t *size);
    virtual uint32_t flags();

    status_t setCachedRange(off_t offset, size_t size);

protected:
    virtual ~TCCMediaDataSource();

private:
    Mutex mLock;

    sp<DataSource> mSource;
    off_t mCachedOffset;
    size_t mCachedSize;
    uint8_t *mCache;

    void clearCache();

    TCCMediaDataSource(const TCCMediaDataSource &);
    TCCMediaDataSource &operator=(const TCCMediaDataSource &);
};

TCCMediaDataSource::TCCMediaDataSource(const sp<DataSource> &source)
    : mSource(source),
      mCachedOffset(0),
      mCachedSize(0),
      mCache(NULL) {
}

TCCMediaDataSource::~TCCMediaDataSource() {
    clearCache();
}

void TCCMediaDataSource::clearCache() {
    if (mCache) {
        free(mCache);
        mCache = NULL;
    }

    mCachedOffset = 0;
    mCachedSize = 0;
}

status_t TCCMediaDataSource::initCheck() const {
    return mSource->initCheck();
}

ssize_t TCCMediaDataSource::readAt(off_t offset, void *data, size_t size) {
    Mutex::Autolock autoLock(mLock);

    if (offset >= mCachedOffset
            && offset + size <= mCachedOffset + mCachedSize) {
        memcpy(data, &mCache[offset - mCachedOffset], size);
        return size;
    }

    return mSource->readAt(offset, data, size);
}

status_t TCCMediaDataSource::getSize(off64_t *size) {
    return mSource->getSize(size);
}

uint32_t TCCMediaDataSource::flags() {
    return mSource->flags();
}

status_t TCCMediaDataSource::setCachedRange(off_t offset, size_t size) {
    Mutex::Autolock autoLock(mLock);

    clearCache();

    mCache = (uint8_t *)malloc(size);

    if (mCache == NULL) {
        return -ENOMEM;
    }

    mCachedOffset = offset;
    mCachedSize = size;

    ssize_t err = mSource->readAt(mCachedOffset, mCache, mCachedSize);

    if (err < (ssize_t)size) {
        clearCache();

        return ERROR_IO;
    }

    return OK;
}

////////////////////////////////////////////////////////////////////////////////

static void hexdump(const void *_data, size_t size) {
    const uint8_t *data = (const uint8_t *)_data;
    size_t offset = 0;
    while (offset < size) {
        printf("0x%04x  ", offset);

        size_t n = size - offset;
        if (n > 16) {
            n = 16;
        }

        for (size_t i = 0; i < 16; ++i) {
            if (i == 8) {
                printf(" ");
            }

            if (offset + i < size) {
                printf("%02x ", data[offset + i]);
            } else {
                printf("   ");
            }
        }

        printf(" ");

        for (size_t i = 0; i < n; ++i) {
            if (isprint(data[offset + i])) {
                printf("%c", data[offset + i]);
            } else {
                printf(".");
            }
        }

        printf("\n");

        offset += 16;
    }
}

////////////////////////////////////////////////////////////////////////////////

TCCMediaExtractor::TCCMediaExtractor(const sp<DataSource> &source, const char *notifiedMIME)
    : mDataSource(source),
      mCDK(NULL),  
      mHaveMetadata(false),
      mHasVideo(false),
      mInitResult(true),
      mFirstTrack(NULL),
      mLastTrack(NULL),
      mFileMetaData(new MetaData),
	  mIsDrm(false) {

    ALOGI("loading TCCMediaExtractor");

    if (notifiedMIME) {
        mNotifiedMIME = notifiedMIME;
    }
}

TCCMediaExtractor::~TCCMediaExtractor() {
    Track *track = mFirstTrack;
    while (track) {
        Track *next = track->next;

        delete track;
        track = next;
    }
    mFirstTrack = mLastTrack = NULL;
}

sp<MetaData> TCCMediaExtractor::getMetaData() {
	return getMetaData(false);
}

sp<MetaData> TCCMediaExtractor::getMetaData(bool withThumbnail) {
	LOGMSG("%s called", __func__);
    status_t err;
    if ((err = readMetaData(withThumbnail ? kMediaScanMetaWithFrame : getDrmFlag() ? kMediaScanNo : kMediaScanMetaOnly)) != OK) {
        return new MetaData;
    }

    return mFileMetaData;
}

uint32_t TCCMediaExtractor::flags() const {
	if (mCDK->GetFileInfo()->iSeekable) {
		return CAN_PAUSE | CAN_SEEK_FORWARD | CAN_SEEK_BACKWARD | CAN_SEEK;
	} else {
		ALOGW("flags: NO SEEKABLE !!"); // DBG, HEES
		return CAN_PAUSE;
	}
}

void TCCMediaExtractor::feedMore(
        int64_t videoThresholdUs, int64_t audioThresholdUs) {
    if (audioThresholdUs == 0) {
        audioThresholdUs = videoThresholdUs;
    }
	mCDK->feedMore(videoThresholdUs, audioThresholdUs);
}

int32_t TCCMediaExtractor::checkQueuedDurationUs(int64_t *durationUs, bool *fulled) {
	return (mCDK->checkQueuedDurationUs(durationUs, fulled) == TCC_CDK_WRAPPER_OK)
           ? OK : INVALID_OPERATION;
}

void TCCMediaExtractor::cancelSeekIfNecessary(bool cancelled) {
    mCDK->cancelSeekIfNecessary(cancelled);
}

void TCCMediaExtractor::signalUnderrun(bool underrun, int64_t thresholdUs) {
    mCDK->signalUnderrun(underrun, thresholdUs);
}

size_t TCCMediaExtractor::countTracks() {
	LOGMSG("%s called", __func__);
    status_t err;
    if ((err = readMetaData(kMediaScanNo)) != OK) {
        return 0;
    }

    size_t n = 0;
    Track *track = mFirstTrack;
    while (track) {
        ++n;
        track = track->next;
    }

    return n;
}

sp<MetaData> TCCMediaExtractor::getTrackMetaData(
        size_t index, uint32_t flags) {
	LOGMSG("%s called", __func__);
    status_t err;
    if ((err = readMetaData(kMediaScanNo)) != OK) {
        return NULL;
    }

    Track *track = mFirstTrack;
    while (index > 0) {
        if (track == NULL) {
            return NULL;
        }

        track = track->next;
        --index;
    }

    if (track == NULL) {
        return NULL;
    }

    if ((flags & kIncludeExtensiveMetaData)
            && !track->includes_expensive_metadata) {
        track->includes_expensive_metadata = true;
		LOGMSG("%s, include extensive meta data", __func__);

        const char *mime;
        CHECK(track->meta->findCString(kKeyMIMEType, &mime));
		if (!strncasecmp("video/", mime, 6)) {
			mFileMetaData->findCString(kKeyMIMEType, &mime);

			if (mCDK->GetFileInfo()->iSeekable && 
					(!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AVI)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_ASF)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WMV)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4)
					|| !strcasecmp(mime, "video/mp4")
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MATROSKA)
                    || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WEBM)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPG)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_TS)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)
                    || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_SSIF)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_FLV)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGM)
					|| !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_RM))) {
				int64_t target_offset;
				target_offset = mCDK->GetFileInfo()->iDuration * 100;// 1/10 of duration in microsec
				LOGMSG("%s, set thumbnail time to %d secs", __func__, target_offset);
				track->meta->setInt64(kKeyThumbnailTime, target_offset);
			} else {
				LOGMSG("%s, set thumbnail time to 0 sec", __func__);
				track->meta->setInt64(kKeyThumbnailTime, 0);
			}
		}
    }
    if(flags & 0x2)
    {
		LOGMSG("%s, set thumbnail time to 0 sec", __func__);
		track->meta->setInt64(kKeyThumbnailTime, 0);
    }

    return track->meta;
}

status_t TCCMediaExtractor::readMetaData(int32_t getMetadataMode) {
	LOGMSG("%s in - getMetadataMode:%d", __func__, getMetadataMode);

	if (!mInitResult) {
		return NO_INIT;
	}

    if (mHaveMetadata) {
        return OK;
    }

    off_t offset = 0;
    status_t err;

	mCDK = new CDKWrapperStagefright();
	mCDK->CDKInit(mDataSource);

	err = parseFile(getMetadataMode);

	if (mHaveMetadata) {
		switch (mCDK->GetFileInfo()->iContainerType)
		{
			case kTCCContainerMKV:
				mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_MATROSKA);
				break;
			case kTCCContainerMP4:
				if (mHasVideo) {
					mFileMetaData->setCString(kKeyMIMEType, "video/mp4");
				} else {
					mFileMetaData->setCString(kKeyMIMEType, "audio/mp4");
				}
				break;
			case kTCCContainerAVI:
				mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_AVI);
				break;
			case kTCCContainerMPG:
				mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_MPG);
				break;
			case kTCCContainerTS:
				mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_MPEG2TS);
				break;
			case kTCCContainerASF:
				if (mHasVideo) {
					mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_WMV);
				} else {
					mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_WMA);
				}
				break;
			case kTCCContainerRMFF:
				if (mHasVideo) {
					mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_RM);
				} else {
					mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_RA);
				}
				break;
			case kTCCContainerAUDIO:
				if (mCDK->GetFileInfo()->iNumAudStreams > 0)
				{
					switch (mFirstTrack->codec)
					{
						case kTCCCodecMP3:
							mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_MPEG);
							break;
						case kTCCCodecAAC:
							mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_AAC);
							break;
						case kTCCCodecPCM:
							mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_WAV);
							break;
						case kTCCCodecFLAC:
							mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_FLAC);
							break;
						case kTCCCodecAPE:
							mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_APE);
							break;
						case kTCCCodecMP2:
							mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_MP2);
							break;
						default: 
							mFileMetaData->setCString(kKeyMIMEType, "audio/unknown");
							break;
					}
				}
				break;
			case kTCCContainerOGG:
			    if (mHasVideo) {
			    	mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_OGM);
			    } else {
			    	mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_OGG);
			    }
				break;
			case kTCCContainerFLV:
				mFileMetaData->setCString(kKeyMIMEType, MEDIA_MIMETYPE_CONTAINER_FLV);
				break;
			default: break;
		}

		LOGMSG("%s out", __func__);
		return OK;
	}

	mInitResult = false;
	LOGMSG("%s out ret %d", __func__, err);
    return err;
}

int32_t 
TCCMediaExtractor::SetTCCCodecInfo(void) 
{
    int32_t ret = 0;

    TCC_CDK_FileInfo* fileinfo;
    Vector<TCC_CDK_VideoInfo> *vinfo;
    Vector<TCC_CDK_AudioInfo> *ainfo;
    Vector<TCC_CDK_SubtitleInfo> *sinfo;

    fileinfo = mCDK->GetFileInfo();
    vinfo = mCDK->GetVideoInfo();
    ainfo = mCDK->GetAudioInfo();
    sinfo = mCDK->GetSubtitleInfo();

    if (fileinfo->iNumVidStreams > 0) {
        mHasVideo = true;
    }

    uint32_t count;

    LOGMSG("%s extract video tracks", __func__);
    // retrieve video track info
    for (count = 0 ; count < fileinfo->iNumVidStreams ; count++) {
        Track *track = new Track;
        track->next = NULL;

        if (mLastTrack) {
            mLastTrack->next = track;
        } else {
            mFirstTrack = track;
        }
        mLastTrack = track;

        track->meta = new MetaData;

        track->meta->setCString(kKeyMIMEType, (*vinfo)[count].mimetype);

        if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_MPEG4)) {
            track->codec = kTCCCodecMPEG4;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_VC1)) {
            track->codec = kTCCCodecVC1;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_AVC)) {
            track->codec = kTCCCodecAVC;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_H263)) {
            track->codec = kTCCCodecH263;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_MPEG2)) {
            track->codec = kTCCCodecMPEG2;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_DIVX)) {
            track->codec = kTCCCodecDIVX;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_RV)) {
            track->codec = kTCCCodecRV;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_FLV1)) {
            track->codec = kTCCCodecFLV1;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_WMV12)) {
            track->codec = kTCCCodecWMV12;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_MJPEG)) {
            track->codec = kTCCCodecMJPEG;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_AVS)) {
            track->codec = kTCCCodecAVS;
        } else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_MVC)) {
            track->codec = kTCCCodecMVC;
		} else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_VP8)) {
			track->codec = kTCCCodecVP8;
		} else if (!strcasecmp((*vinfo)[count].mimetype, MEDIA_MIMETYPE_VIDEO_VP9)) {
			track->codec = kTCCCodecVP9;
        } else {
            track->codec = kTCCCodecUnknown;
        }

        track->meta->setInt32(kKeyWidth, (*vinfo)[count].iWidth);
        track->meta->setInt32(kKeyHeight, (*vinfo)[count].iHeight);
        //track->meta->setInt32(kKeyFrameRate, (*vinfo)[count].iFrameRate / 1000);
        track->meta->setInt32(kKeyFrameRate, (*vinfo)[count].iFrameRate); // *1000
		track->meta->setInt64(kKeyDuration, static_cast<int64_t>(fileinfo->iDuration) * 1000); // usec
        track->meta->setInt32(kKeyMaxInputSize, VIDEO_BUFFER_SIZE);
        track->meta->setInt32(kKeyRotation, (*vinfo)[count].iRotateDegree);
        track->includes_expensive_metadata = false;

        // set codec config data
        uint8_t *config;
        uint32_t config_size;
        config_size = (*vinfo)[count].iCodecExtraDataLen + sizeof(TCCVideoConfigData);
        config = (uint8_t*)malloc(config_size);
        memcpy(config, (*vinfo)[count].pCodecExtraData, (*vinfo)[count].iCodecExtraDataLen);

        TCCVideoConfigData config_data;

        memset(&config_data, 0, sizeof(TCCVideoConfigData));
        config_data.iWidth = (*vinfo)[count].iWidth;
        config_data.iHeight = (*vinfo)[count].iHeight;
        sprintf(config_data.id, TCC_VIDEO_CONFIG_ID);
        config_data.iContainerType = fileinfo->iContainerType;
        config_data.iBitRate = fileinfo->iBitRate / 1024;
        config_data.iFrameRate = (*vinfo)[count].iFrameRate;

        memcpy(config+(*vinfo)[count].iCodecExtraDataLen, (void*)&config_data, sizeof(TCCVideoConfigData));

        track->meta->setData(kKeyTCCCodecInfo, (uint32_t)0, config, config_size);
#ifdef OPENMAX1_2
        track->meta->setData(kKeyCodecSpecific, (uint32_t)0, config, config_size);
#endif
        free(config);
    }

    LOGMSG("%s extract audio tracks", __func__);
    // retrieve audio track info
    for (count = 0 ; count < fileinfo->iNumAudStreams ; count++) {
        Track *track = new Track;
        track->next = NULL;

        if (mLastTrack) {
            mLastTrack->next = track;
        } else {
            mFirstTrack = track;
        }
        mLastTrack = track;

        track->meta = new MetaData;
        if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
            track->meta->setCString(kKeyMIMEType, MEDIA_MIMETYPE_AUDIO_VORBIS_TCC);
            track->codec = kTCCCodecVORBIS;
        } else {
            track->meta->setCString(kKeyMIMEType, (*ainfo)[count].mimetype);

            if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_AMR_WB)) {
                track->codec = kTCCCodecAMRWB;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_WMA)) {
                track->codec = kTCCCodecWMA;
#ifdef OPENMAX1_2
				track->meta->setInt32(kKeyWMABlockAlign, 0);
				track->meta->setInt32(kKeyWMAFormatTag, (*ainfo)[count].iFormatId);
#endif
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_MPEG_TCC)) {
                track->codec = kTCCCodecMP3;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_AAC_TCC)) {
                track->codec = kTCCCodecAAC;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_AMR_NB)) {
                track->codec = kTCCCodecAMR;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_PCM)) {
                track->codec = kTCCCodecPCM;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_MP2)) {
                track->codec = kTCCCodecMP2;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_DTS)) {
                track->codec = kTCCCodecDTS;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_DTSHD)) {
                track->codec = kTCCCodecDTSHD;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_AC3)) {
                track->codec = kTCCCodecAC3;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_DDP)) {
                track->codec = kTCCCodecAC3;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_TRUEHD)) {
                track->codec = kTCCCodecTRUEHD;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_RA)) {
                track->codec = kTCCCodecRA;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_FLAC)) {
                track->codec = kTCCCodecFLAC;
            } else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_APE)) {
                track->codec = kTCCCodecAPE;
			} else if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_AAC)) { // for google aac decoder
                track->codec = kTCCCodecAAC;
            } else {
                track->codec = kTCCCodecUnknown;
            }
        }

        track->meta->setInt32(kKeyChannelCount, (*ainfo)[count].iChannels);
        track->meta->setInt32(kKeySampleRate, (*ainfo)[count].iSamplesPerSec);
        track->meta->setInt64(kKeyDuration, static_cast<int64_t>(fileinfo->iDuration) * 1000); // usec
        track->meta->setInt32(kKeyMaxInputSize, AUDIO_BUFFER_SIZE);

        if( (*ainfo)[count].pszLanguageInfo != NULL)
        {
            track->meta->setCString(kKeyMediaLanguage, (*ainfo)[count].pszLanguageInfo);
        }
        track->includes_expensive_metadata = false;

        // AMR codec does not need config data
        if (track->codec != kTCCCodecAMR && track->codec != kTCCCodecAMRWB) {
            // set codec config data
            uint8_t *config;
            uint32_t config_size;
			bool is_tcc_audiodecoder = true;
			
			if (!strcasecmp((*ainfo)[count].mimetype, MEDIA_MIMETYPE_AUDIO_AAC)) {
				// use google aac-decoder
				is_tcc_audiodecoder = false;
			}
			
			config_size = (*ainfo)[count].iCodecExtraDataLen;
			if (is_tcc_audiodecoder == true) {
				config_size += sizeof(uint8_t);
			}
            config = (uint8_t*)malloc(config_size);
            memcpy(config, (*ainfo)[count].pCodecExtraData, (*ainfo)[count].iCodecExtraDataLen);
			if (is_tcc_audiodecoder == true) {
            	*(config+(*ainfo)[count].iCodecExtraDataLen) = (uint8_t)fileinfo->iContainerType;
			}
           	track->meta->setData(kKeyTCCCodecInfo, (uint32_t)0, config, config_size);
#ifdef OPENMAX1_2
           	track->meta->setData(kKeyCodecSpecific, (uint32_t)0, config, config_size);
#endif
            free(config);
        }
    }

    LOGMSG("%s extract subtitle tracks", __func__);
    // retrieve subtitle track info
    for (count = 0 ; count < fileinfo->iNumSubtitleStreams ; count++) {
        Track *track = new Track;
        track->next = NULL;

        if (mLastTrack) {
            mLastTrack->next = track;
        } else {
            mFirstTrack = track;
        }
        mLastTrack = track;

        track->meta = new MetaData;

        track->meta->setCString(kKeyMIMEType, (*sinfo)[count].mimetype);

        if (!strcasecmp((*sinfo)[count].mimetype, MEDIA_MIMETYPE_TEXT_3GPP)) {
            track->codec = kTCCCodecText;
        } else if (!strcasecmp((*sinfo)[count].mimetype, MEDIA_MIMETYPE_TEXT_GRAPHIC)) {
            track->codec = kTCCCodecGraphic;
        } else {
            track->codec = kTCCCodecUnknown;
        }

        track->meta->setInt32(kKeyMaxInputSize, (*sinfo)[count].iMaxInputSize);

        if( (*sinfo)[count].pszLanguageInfo != NULL)
        {
            track->meta->setCString(kKeyMediaLanguage, (*sinfo)[count].pszLanguageInfo);
        }
    }

    return ret;
}

status_t TCCMediaExtractor::parseFile(int32_t getMetadataMode) {
    LOGMSG("%s in - getMetadataMode:%d", __func__, getMetadataMode);

    int32_t ret;

    int32_t sourceScheme;
    if (mDataSource->flags() & DataSource::kIsWFDSource) {
        if(mDataSource->flags() & DataSource::kIsHDCP2Enable) {
            sourceScheme = TCC_CDK_Wrapper::HDCP2_SOURCE_SCHEME;
        } else {
           sourceScheme = TCC_CDK_Wrapper::RTSP_SOURCE_SCHEME;
        }
    } else if (mDataSource->flags() & (DataSource::kIsCachingDataSource |
                                       DataSource::kWantsPrefetching)) {
        sourceScheme = TCC_CDK_Wrapper::HTTP_SOURCE_SCHEME;
    } else {
        sourceScheme = TCC_CDK_Wrapper::FILE_SOURCE_SCHEME;
    }

    uint32_t mode;

    switch (getMetadataMode) {
        case kMediaScanMetaOnly :
            mode = TCC_CDK_WRAPPER_PARSER_GETMETADATAMODE;
            break;
        case kMediaScanMetaWithFrame :
            mode = TCC_CDK_WRAPPER_PARSER_GETMETADATAMODE
                | TCC_CDK_WRAPPER_PARSER_GETTHUMBNAILMODE;
            break;
        case kMediaScanNo :
        default:
            mode = 0;
            if (sourceScheme != TCC_CDK_Wrapper::FILE_SOURCE_SCHEME) {
                off64_t contentLength = 0;
                if ((mDataSource->getSize(&contentLength) != OK)) {
                    status_t err = ERROR_UNSUPPORTED;
                    // Check if file size can be extracted from app. specific setting.
                    ssize_t index = mDataSource->
                        getExternalSourceSpecificInfos().indexOfKey(String8("media/size"));
                    if (index >= 0) {
                        String8 value = mDataSource->
                            getExternalSourceSpecificInfos().valueAt(index);
                        mDataSource->getExternalSourceSpecificInfos().removeItemsAt(index);
                        err = mDataSource->setSize((off64_t)(atoi(value.string())));
                    }

                    if (err != OK) {
                        ALOGI("ignore file size");
                        mode |= TCC_CDK_WRAPPER_PARSER_IGNORE_FILE_SIZE;
                    }
                }

                bool byteSeekAccepted = true;
                if (DataSource::kIsWFDSource & mDataSource->flags()) {
                    byteSeekAccepted = false;
                    mode |= TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE;
                    ALOGE("TCCMediaExtractor TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE");
                } else if (DataSource::kHttpByteRangeDenied & mDataSource->flags()) {
                    byteSeekAccepted = false;
                }

                if (!byteSeekAccepted) {
                    mode |= TCC_CDK_WRAPPER_PARSER_PROGRESSIVE_READ_MODE;
                    if (!(DataSource::kHttpTimeRangeRequestable & mDataSource->flags())) {
                        // we can't operate 'seek' from this data source.
                        ALOGI("can't seek.. playback only");
                    }
                }
                if(DataSource::kIsWFDSource & mDataSource->flags()) {
                    ALOGE("TCCMediaExtractor TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE");
                    mode |= TCC_CDK_WRAPPER_PARSER_WFDPLAYBACKMODE;
                }
            }
            break;
    }

    String8 ext;
    if (mDataSource->flags() & DataSource::kLocalFileSource) {
        String8 filePath = static_cast<FileSource*>(mDataSource.get())->getFilePath();
        //ALOGD("FILENAME: %s", filePath.string());
        ext = filePath.getPathExtension();
        ext.toUpper();
    } else if (sourceScheme != TCC_CDK_Wrapper::FILE_SOURCE_SCHEME
            && !strncasecmp("audio/", mNotifiedMIME.string(), 6)) {
        // Hack to avoid demuxer from redundant re-init..
        ext = ".audio"; 
    }

	if(mCDK->GetFileInfo()->m_iFragmentedMp4) 
		mode |= TCC_CDK_WRAPPER_PARSER_FRAGMENTED_MP4;
    ret = mCDK->ContainerParse(mode, sourceScheme,
            (ext.length() >= 3) ? (char *)(ext.string()+1) : NULL);

    if (ret != TCC_CDK_WRAPPER_OK) {
        LOGMSG("%s: parse file error %d", __func__, ret);
        return NO_INIT;
    }

    ret = SetTCCCodecInfo();
    if (ret < 0) {
        LOGMSG("%s: SetTCCCodecInfo error %d", __func__, ret);
        return NO_INIT;
    }

    parseMetaData();

    mHaveMetadata = true;

    LOGMSG("%s out", __func__);
    return OK;
}

#define MetadataBufsize		512

#ifdef META_MIN
#undef META_MIN
#endif
#define META_MIN(a,b) ((a)>(b)?(b):(a))

void TCCMediaExtractor::parseMetaData() {
	char *buffer;
	buffer = new char[MetadataBufsize];

	String16 *temp;

	mFileMetaData->setInt32(kKeyVideoTrackCount, mCDK->GetFileInfo()->iNumVidStreams );
	mFileMetaData->setInt32(kKeyAudioTrackCount, mCDK->GetFileInfo()->iNumAudStreams );
    // 2012/03/15
	mFileMetaData->setInt32(kKeyGetStreamMode, mCDK->iGetStreamMode );
	// 2013/10/24
	mFileMetaData->setInt32(kKeyBitRate, mCDK->GetFileInfo()->iBitRate*1000 );	// bps

	if(mCDK->GetFileInfo()->m_pcLocation)
		mFileMetaData->setCString(kKeyLocation, mCDK->GetFileInfo()->m_pcLocation );

	if (mCDK->GetFileInfo()->iContainerType == kTCCContainerMP4) 
	{
		if(mCDK->GetFileInfo()->m_pcTitle)
		{
			mFileMetaData->setCString(kKeyTitle, mCDK->GetFileInfo()->m_pcTitle );	
		}

		if(mCDK->GetFileInfo()->m_pcArtist)
		{
			mFileMetaData->setCString(kKeyArtist, mCDK->GetFileInfo()->m_pcArtist );
		}

		if(mCDK->GetFileInfo()->m_pcAlbum)
		{
			mFileMetaData->setCString(kKeyAlbum, mCDK->GetFileInfo()->m_pcAlbum );
		}

		if(mCDK->GetFileInfo()->m_pcAlbumArtist)
		{
			mFileMetaData->setCString(kKeyAlbumArtist, mCDK->GetFileInfo()->m_pcAlbumArtist );
		}

		if(mCDK->GetFileInfo()->m_pcYear)
		{
			mFileMetaData->setCString(kKeyYear, mCDK->GetFileInfo()->m_pcYear );
		}

		if(mCDK->GetFileInfo()->m_pcWriter)
		{
			mFileMetaData->setCString(kKeyWriter, mCDK->GetFileInfo()->m_pcWriter );
		}

		if(mCDK->GetFileInfo()->m_pcAlbumArt)
		{
			mFileMetaData->setData(kKeyAlbumArt, MetaData::TYPE_NONE, mCDK->GetFileInfo()->m_pcAlbumArt , mCDK->GetFileInfo()->m_iAlbumArtSize );
		}

		if(mCDK->GetFileInfo()->m_pcGenre)
		{
			mFileMetaData->setCString(kKeyGenre, mCDK->GetFileInfo()->m_pcGenre );
		}

		if(mCDK->GetFileInfo()->m_pcCompilation[0])
		{
			mFileMetaData->setCString(kKeyCompilation, mCDK->GetFileInfo()->m_pcCompilation );
		}

		if(mCDK->GetFileInfo()->m_pcCDTrackNumber[0])
		{
			mFileMetaData->setCString(kKeyCDTrackNumber, mCDK->GetFileInfo()->m_pcCDTrackNumber );
		}

		if(mCDK->GetFileInfo()->m_pcDiscNumber[0])
		{
			mFileMetaData->setCString(kKeyDiscNumber, mCDK->GetFileInfo()->m_pcDiscNumber );
		}
	}


	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_AUTHOR)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_AUTHOR);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("author %s", buffer);

        mFileMetaData->setCString(kKeyArtist, buffer);
	} else if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_ARTIST)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_ARTIST);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("artist %s", buffer);

        mFileMetaData->setCString(kKeyArtist, buffer);
	}

	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_ALBUM)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_ALBUM);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("album %s", buffer);

        mFileMetaData->setCString(kKeyAlbum, buffer);
	}
	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_TITLE)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_TITLE);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("title %s", buffer);

        mFileMetaData->setCString(kKeyTitle, buffer);
	}
	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_TRACK)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_TRACK);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("track %s", buffer);

        mFileMetaData->setCString(kKeyCDTrackNumber, buffer);
	}
	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_GENRE)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_GENRE);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("genre %s", buffer);

        mFileMetaData->setCString(kKeyGenre, buffer);
	}
	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_YEAR)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_YEAR);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("year %s", buffer);

        mFileMetaData->setCString(kKeyYear, buffer);
	}
	if (mCDK->ExistMetadataValue(TCC_CDK_WRAPPER_METADATA_COMPOSER)) {
		temp = mCDK->GetMetadataString(TCC_CDK_WRAPPER_METADATA_COMPOSER);

		memset(buffer, 0, MetadataBufsize);
		utf16_to_utf8(temp->string(), META_MIN(MetadataBufsize,temp->size()), buffer); 
		LOGMSG("composer %s", buffer);

        mFileMetaData->setCString(kKeyComposer, buffer);
	}

	// only for WMA
	if (mCDK->GetFileInfo()->iContainerType == kTCCContainerASF && !mHasVideo) 
	{
		TCCAlbumartExtractor extractor;
		if (extractor.ExistWMAAlbumart(mDataSource))
		{
			uint32_t albumartsize;
			uint8_t* albumart;

			extractor.GetWMAAlbumart(albumartsize, albumart);

			mFileMetaData->setData(kKeyAlbumArt, MetaData::TYPE_NONE, albumart, albumartsize);
		}
	}

    delete[] buffer;
    buffer = NULL;
}

sp<MediaSource> TCCMediaExtractor::getTrack(size_t index) {
    status_t err;
    if ((err = readMetaData(kMediaScanNo)) != OK) {
        return NULL;
    }

	size_t track_num = index;
    Track *track = mFirstTrack;
    while (index > 0) {
        if (track == NULL) {
            return NULL;
        }

        track = track->next;
        --index;
    }

    if (track == NULL) {
        return NULL;
    }

	if (track->codec >= kTCCCodecFirstAudio && track->codec <= kTCCCodecLastAudio) {
		selectStream(track_num);
	}

    return new TCCMediaSource(track->meta, mDataSource, this, mCDK, track->codec);
}


// INTERNAL_SUBTITLE_INCLUDE
int32_t TCCMediaExtractor::GetSubtitleType(void)
{
	return mCDK->GetSubtitleType();
}

int32_t TCCMediaExtractor::SelectSubtitleStream(int32_t num)
{
	return mCDK->SelectSubtitleStream(num);
}

void TCCMediaExtractor::SetCurrentPlaybackPosition(int64_t usec) {
	unsigned int msec = usec/1000;
	mCDK->SetCurrentPlaybackPosition(msec);
}

int32_t TCCMediaExtractor::GetSubtitleData()
{
	LOGMSG("TCCMediaSource::GetSubtitleData");
	return mCDK->GetFirstSubtitle() ;
}

int32_t TCCMediaExtractor::SetSubtitleSeek(int msec)
{
	LOGMSG("TCCMediaSource::SetSubtitleSeek");
	return mCDK->SetSubtitleSeek(msec) ;
}

int32_t TCCMediaExtractor::get_inter_subtitle_type() {
    return mCDK->get_inter_subtitle_type();
}

void TCCMediaExtractor::selectStream(int num) {
    TCC_CDK_FileInfo* fileinfo = mCDK->GetFileInfo();
	if (num < (int)fileinfo->iNumStreams) {
        int32_t ret = mCDK->SelectAudioStream(num-fileinfo->iNumVidStreams);
		if (ret != TCC_CDK_WRAPPER_OK) {
            ALOGE("fail changing audio stream");
            return;
		}
        mCDK->ClearAudioBuffer();  
    }
}

int32_t TCCMediaExtractor::postInitIfNecessary(int64_t prefillUs) {
	LOGMSG("%s in", __func__);
	int32_t ret;
	ret = mCDK->PrepareDemuxerWithAppropriateMode(prefillUs);
	if (ret != TCC_CDK_WRAPPER_OK) {
		return UNKNOWN_ERROR;
	}
	return OK;
}

int32_t TCCMediaExtractor::getNumSubtitleStreams() {
	TCC_CDK_FileInfo* fileinfo;

	fileinfo = mCDK->GetFileInfo();
	return fileinfo->iNumSubtitleStreams;
}

int32_t TCCMediaExtractor::getCurrentSubtitleStream(void) {
	return mCDK->GetCurrentSubtitleStream();
}

char* TCCMediaExtractor::getLangSubtitleStream() {
	return mCDK->GetLangSubtitleStream();
}

void TCCMediaExtractor::ClearStreamQueue() {
    mCDK->ClearStreamBuffer();
    return;
}
void TCCMediaExtractor::SkipAudioStream() {
    mCDK->SkipAudioStream();
    return;
}

uint32_t TCCMediaExtractor::GetVideoBufferedTime()
{
    return mCDK->GetVideoBufferedTime();
}

////////////////////////////////////////////////////////////////////////////////

TCCMediaSource::TCCMediaSource(
        const sp<MetaData> &format,
        const sp<DataSource> &dataSource,
        const sp<TCCMediaExtractor> &extractor,
		const sp<CDKWrapperStagefright> &CDK,
		const uint32_t codec)
    : mFormat(format),
      mDataSource(dataSource),
      mExtractor(extractor),
	  mCDK(CDK),
      mStarted(false),
      mParsingStarted(false),
	  mSourceDone(false),//JS Baek
	  mThumbFrameTryCount(0),
	  mSkimInterval(0),//JS Baek
#ifdef USE_HDCP_DECRYPTION
	  mNeedDecryption(false),
#endif
      mGroup(NULL),
      mCodecType(codec),
	  mPlayRate(PLAYRATE_DEFAULT),
	  mPrevPlayMode(CDK_ANDROID_VIDEO_EXTRA_NORMAL_MODE) {
	if (mCodecType >= kTCCCodecFirstAudio && mCodecType <= kTCCCodecLastAudio) {
		mStreamType = STREAM_TYPE_AUDIO;
		mCDK->SetAvailableTrack(STREAM_TYPE_AUDIO);
	} else if (mCodecType >= kTCCCodecFirstVideo && mCodecType <= kTCCCodecLastVideo) {
		mStreamType = STREAM_TYPE_VIDEO;
		mCDK->SetAvailableTrack(STREAM_TYPE_VIDEO);
	} else if (mCodecType == kTCCCodecText) {
		mStreamType = STREAM_TYPE_TEXT;
		mCDK->SetAvailableTrack(STREAM_TYPE_TEXT);
	} else {
		mStreamType = STREAM_TYPE_UNKNOWN;
	}
}

TCCMediaSource::~TCCMediaSource() {
    if (mStarted) {
        stop();
    }
}

#ifdef USE_HDCP_DECRYPTION
status_t TCCMediaSource::setBuffers(const Vector<MediaBuffer *> &buffers)
{
    delete mGroup;
    mGroup = new MediaBufferGroup;
    for (size_t i = 0; i < buffers.size(); ++i) {
        mGroup->add_buffer(buffers.itemAt(i));
    }

    return OK;
}
#endif

status_t TCCMediaSource::start(MetaData *params) {
    Mutex::Autolock autoLock(mLock);

    CHECK(!mStarted);

    mGroup = new MediaBufferGroup;

    int32_t max_size;
    CHECK(mFormat->findInt32(kKeyMaxInputSize, &max_size));

    mGroup->add_buffer(new MediaBuffer(max_size));

	if( mCDK->IsPrepared() == false ) {
        bool b_mode_changed = false;
		if (mCDK->PrepareDemuxerWithSelectiveMode(&b_mode_changed) != TCC_CDK_WRAPPER_OK)
			return UNKNOWN_ERROR;
		if (b_mode_changed == true) {
			if (mExtractor->SetTCCCodecInfo() < 0)
                return UNKNOWN_ERROR;
        }
	}

    mStarted = true;

    return OK;
}

status_t TCCMediaSource::stop() {
    Mutex::Autolock autoLock(mLock);

    CHECK(mStarted);

    delete mGroup;
    mGroup = NULL;

    mStarted = false;

    return OK;
}

sp<MetaData> TCCMediaSource::getFormat() {
    Mutex::Autolock autoLock(mLock);

#ifdef USE_HDCP_DECRYPTION
    mNeedDecryption = mCDK->GetDecryptionNeed();

    if( mStreamType == STREAM_TYPE_VIDEO && mNeedDecryption ){
        mFormat->setInt32(kKeyRequiresSecureBuffers, true);
    }
#endif

    return mFormat;
}

status_t TCCMediaSource::setSkimmingOptions(const ReadOptions *options)
{
	int32_t playRate = 0;
	int64_t seekTimeUs = -1;
	int32_t ret = OK;
	
	if (options && options->getPlayRate(&playRate))
	{
		if((playRate & PLAYMODE_SKIPPING) || (playRate & PLAYMODE_SKIMMING))
			mCDK->bIsUnderSkimming = true;
		else
			mCDK->bIsUnderSkimming = false;
	}

	if( mCDK->bIsUnderSkimming == true && mStreamType == STREAM_TYPE_AUDIO )
		return ERROR_END_OF_STREAM;

	return ret;
}

status_t TCCMediaSource::checkSeekRangeValidity(int64_t *seekTime)
{
     int32_t last_key_pts_in_ms = mCDK->GetLastKeyFramePTS();  // GET time-stamp of the last key frame

     if(last_key_pts_in_ms <= 5000)
         return OK;  // skip the rest. last_key_pts_in_ms value from Demuxer can't be trusted because it's too short.

     int32_t seek_time_is_ms = (int32_t)(*seekTime / 1000);

     if(seek_time_is_ms > last_key_pts_in_ms) {
         *seekTime = (int64_t)last_key_pts_in_ms * 1000;   // modify seekTime not to prevent beyond "LAST KEYFRAME PTS"
         ALOGD("Seek time is adjusted not to seek beyond the last key frame's time stamp.");
     }

     return OK;
}

status_t TCCMediaSource::read(
        MediaBuffer **out, const ReadOptions *options) {
    Mutex::Autolock autoLock(mLock);

    CHECK(mStarted);
    LOGMSG("[TCCMediaSource::read] in");

    *out = NULL;
    MediaBuffer* buffer = NULL;

    int64_t seekTimeUs = -1;
    bool isSyncSample = false;
    ReadOptions::SeekMode mode;

    if (mStreamType != STREAM_TYPE_TEXT) {
        if (options && options->getSeekTo(&seekTimeUs, &mode)) {
            uint32_t findFlags = 0;
            switch (mode) {
                case ReadOptions::SEEK_PREVIOUS_SYNC:
                    //findFlags = SampleTable::kFlagBefore;
                    break;
                case ReadOptions::SEEK_NEXT_SYNC:
                    //findFlags = SampleTable::kFlagAfter;
                    break;
                case ReadOptions::SEEK_CLOSEST_SYNC:
                case ReadOptions::SEEK_CLOSEST:
                    //findFlags = SampleTable::kFlagClosest;
                    break;
                default:
                    CHECK(!"Should not be here.");
                    break;
            }

        #if 0 // it needs more testing.
            if (options->getParamPreventSeekBeyondLastKey() == true) {
                int ret = this->checkSeekRangeValidity(&seekTimeUs);
                if (ret != OK)
                    LOGMSG("Error occured while run checkSeekRangeValidity");
            }
        #endif

            LOGMSG("[TCCMediaSource::read] exist seek option, seek to %lld us", seekTimeUs);

            int32_t ret = mCDK->Seek(mStreamType, seekTimeUs / 1000);
            // if seeking is failed, just continue playing.

            isSyncSample = true;

            // fall through
        }

        status_t ret = setSkimmingOptions(options);
        if (ret != OK) {
            LOGMSG("Skimming is done");
            return ERROR_END_OF_STREAM;
        }
    }

    off_t offset;
    int32_t size;
    uint32_t dts;
    uint32_t extra = 0;
    int32_t param = TCC_MEDIAEVENT_NONE; // INTERNAL_SUBTITLE_INCLUDE
    bool newBuffer = false;
    const int32_t loopcnt = 2;

    for (int32_t i = 0; i < loopcnt; i++) {
        if (buffer == NULL) {
            newBuffer = true;

            status_t err = mGroup->acquire_buffer(&buffer);

            if (err != OK) {
                ALOGE("[TCCMediaSource::read] can't acquire buffer!");
                //CHECK_EQ(buffer, NULL);
                return err;
            }
        }

        if (newBuffer) {
            mFormat->findInt32(kKeyMaxInputSize, &size);
            int32_t packetDisCont = 0;
            int32_t ret = mCDK->GetData(mStreamType, (uint8_t *)buffer->data(), (uint32_t&)size, dts, extra, param, packetDisCont);

            if((DataSource::kIsWFDSource & mDataSource->flags())
                    && (mCodecType == kTCCCodecPCM)
                    && WFD_LPCM_FRAME_SIZE != size) {
                ret = TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME;
            }
            if (ret < 0) {
                if (ret == TCC_CDK_WRAPPER_END_OF_STREAM || ret == TCC_CDK_WRAPPER_END_OF_FILE) {
                    mSourceDone = true;//JS Baek
                    buffer->release();
                    buffer = NULL;

                    ALOGE("read: end of %s stream",
                            mStreamType == STREAM_TYPE_AUDIO ? "audio" :
                            mStreamType == STREAM_TYPE_VIDEO ? "video" :
                            mStreamType == STREAM_TYPE_TEXT  ? "text"  : "other");
                    return ERROR_END_OF_STREAM;
                } else if (ret == TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME || ret == TCC_CDK_WRAPPER_GET_FRAME_CANCELLED) {
                    bool resetted = false;
                    if (options && options->getReset(&resetted) && resetted) {
                        ALOGW("read: %s - TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME -> ERROR_END_OF_STREAM",
                                mStreamType == STREAM_TYPE_AUDIO ? "audio" :
                                mStreamType == STREAM_TYPE_VIDEO ? "video" :
                                mStreamType == STREAM_TYPE_TEXT  ? "text"  : "other");
                        return ERROR_END_OF_STREAM;
                    }

                    if ((DataSource::kIsCachingDataSource | DataSource::kWantsPrefetching) 
                            & mDataSource->flags()) {
                        buffer->release();
                        buffer = NULL;
                        if (DataSource::kIsDataSourceToreDown & mDataSource->flags()) {
                            return ERROR_IO;
                        }

                        if (ret == TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME) {
                            ALOGW("read: %s - INFO_RETRY", 
                                    mStreamType == STREAM_TYPE_AUDIO ? "audio" :
                                    mStreamType == STREAM_TYPE_VIDEO ? "video" :
                                    mStreamType == STREAM_TYPE_TEXT  ? "text"  : "other");
                            usleep(500000);
                        } else {
                            usleep(50);
                        }
                        return INFO_RETRY;
                    }

                    if ((DataSource::kIsWFDSource & mDataSource->flags())
                            && (DataSource::kIsDataSourceToreDown & mDataSource->flags())) {
                        buffer->release();
                        buffer = NULL;
                        return ERROR_IO;
                    }

                    usleep(5000);
                    LOGMSG("read: %s - TCC_CDK_WRAPPER_GET_FRAME_NEXT_TIME",
                            mStreamType == STREAM_TYPE_AUDIO ? "audio" :
                            mStreamType == STREAM_TYPE_VIDEO ? "video" :
                            mStreamType == STREAM_TYPE_TEXT  ? "text"  : "other");

                    if (i > 0) i--;
                    continue;
                } else if (ret == TCC_CDK_WRAPPER_DEMUXER_ERROR) {
                    buffer->release();
                    buffer = NULL;

                    ALOGE("[TCCMediaSource::read] demux error! ERROR_END_OF_STREAM");
                    return ERROR_END_OF_STREAM;
                } else if (ret == TCC_CDK_WRAPPER_WAIT_SEEK_REQUEST) {
                    buffer->release();
                    buffer = NULL;
                    DLOGD("[WAIT-step01] return WAIT_SEEK_REQUEST (%s)",
                            mStreamType == STREAM_TYPE_AUDIO ? "audio" :
                            mStreamType == STREAM_TYPE_VIDEO ? "video" :
                            mStreamType == STREAM_TYPE_TEXT  ? "text"  : "other");
                    return WAIT_SEEK_REQUEST;
                } else {
                    buffer->release();
                    buffer = NULL;

                    ALOGE("[TCCMediaSource::read] GetData fail! ret %d", ret);
                    return ERROR_IO;
                }
            }

            CHECK(buffer != NULL);

            // APE needs extra info to seek
            if (mCodecType == kTCCCodecAPE && (isSyncSample || !mParsingStarted)) {
                mParsingStarted = true;
                *((uint32_t*)((char*)buffer->data()+size)) = 0x12345678; // identifier
                memcpy((char*)buffer->data()+size+4, &extra, 4);
                size += 8;
            }

            buffer->set_range(0, size);
            buffer->meta_data()->clear();
            buffer->meta_data()->setInt64(kKeyTime, ((int64_t)dts * 1000)); // convert to usec
            buffer->meta_data()->setInt32(kKeyPacketDisCont, ((int32_t)packetDisCont));

#if 0 // TODO
            if( param == TCC_MEDIAEVENT_SYNC_FRAME || isSyncSample == true )
                buffer->meta_data()->setInt32(kKeyIsDiscontinuity, true);
            else
                buffer->meta_data()->setInt32(kKeyIsDiscontinuity, false);
#endif

            LOGMSG("read done, type %s, size %d, time %lld",
                    mStreamType == STREAM_TYPE_AUDIO ? "audio" :
                    mStreamType == STREAM_TYPE_VIDEO ? "video" :
                    mStreamType == STREAM_TYPE_TEXT  ? "text"  : "other",
                    size, (int64_t)dts * 1000);

            int64_t thumb_time;
            if (mFormat->findInt64(kKeyThumbnailTime, &thumb_time)) {
                buffer->meta_data()->setInt64(kKeyThumbnailTime, (THUMBNAIL_TIME_SEC * 1000000)); // convert to usec

                if (mThumbFrameTryCount == THUMBNAIL_RETRY_COUNT && thumb_time != 0) {
                    // fail to create thumbnail at key frame time is after 1 sec
                    // so, reset to 0 sec
                    seekTimeUs = 0;
                    ret = mCDK->Seek(mStreamType, seekTimeUs);

                    if (buffer != NULL) {
                        buffer->release();
                        buffer = NULL;
                    }

                    if (ret != TCC_CDK_WRAPPER_OK) {
                        if (ret == TCC_CDK_WRAPPER_END_OF_FILE) {
                            return ERROR_END_OF_STREAM;
                        }
                        ALOGE("[TCCMediaSource::read] seek to 0 sec fail! %d", ret);
                        return UNKNOWN_ERROR;
                    }

                    mThumbFrameTryCount++;
                    continue;
                } else if ((mThumbFrameTryCount == THUMBNAIL_RETRY_COUNT * 2) && thumb_time == 0) {
                    ALOGE("thumbnail would not be created");
                    return ERROR_UNSUPPORTED;
                } else {
                    mThumbFrameTryCount++;
                }
            }
        }
        break;
    }

    *out = buffer;
    buffer = NULL;

    LOGMSG("[TCCMediaSource::read] out");
    return OK;
}

bool SniffByTargetObject(void *pObject, int32_t checkType) {
    bool retVal = false;
    switch (checkType) {
        case 0:
        {
            CDKWrapperStagefright *pCDK = (CDKWrapperStagefright *)pObject;
            retVal = (pCDK->numFileStream() > 0) ? true : false;
        }
        break;

        case 1:
        {
            CDKWrapperStagefright *pCDK = (CDKWrapperStagefright *)pObject;
            retVal = (pCDK->GetFileInfo()->iNumVidStreams > 0) ? true : false;
        }
        break;

        default:
            break;
    }

    return retVal;
}

bool SniffTCCMedia(
        const sp<DataSource> &source, String8 *mimeType, float *confidence, sp<AMessage> *) {
    uint8_t header[16];

    ssize_t n = source->readAt(0, header, sizeof(header));
    if (n < (ssize_t)sizeof(header)) {
//		ALOGE("SniffTCCMedia - source->readAt(0, header, sizeof(header)) error!!!");
//		return false;
    }

    int32_t sourceScheme;
    if (source->flags() & (DataSource::kIsCachingDataSource |
                DataSource::kWantsPrefetching)) {
        sourceScheme = TCC_CDK_Wrapper::HTTP_SOURCE_SCHEME;
    } else {
        sourceScheme = TCC_CDK_Wrapper::FILE_SOURCE_SCHEME;
    }

    if ( !memcmp(header+4, "ftyp", 4) || !memcmp(header+4, "moov", 4) || 
         !memcmp(header+4, "mdat", 4) || !memcmp(header+4, "skip", 4) )  {

		// for gapless
        CDKWrapperStagefright *pCDK = NULL;
		pCDK = new CDKWrapperStagefright();
		pCDK->CDKInit(source);
		if ( pCDK->ContainerParse(TCC_CDK_WRAPPER_PARSER_GETMETADATAMODE, sourceScheme, "mp4") < 0 )
		{
			*mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;
			*confidence = 0.3f;
			FILETYPEMSG("mp4/mov file");
			delete pCDK;
			pCDK = NULL;
			return true;
		}

		if (pCDK->GetFileInfo()->iNumVidStreams > 0) 
		{
			*mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;
			*confidence = 0.3f;
			FILETYPEMSG("mp4/mov file");
		}
		else
		{
			*mimeType = "audio/mp4";
			*confidence = 1.0f;
			FILETYPEMSG("m4a file");
		}

		delete pCDK;
		pCDK = NULL;
        return true;

    } else if (!memcmp(header, "RIFF", 4)) {
        if (!memcmp(header+8, "AVI ", 4) || !memcmp(header+8, "AVIX", 4)) {
            *mimeType = MEDIA_MIMETYPE_CONTAINER_AVI;
            *confidence = 0.4f;
            FILETYPEMSG("avi file");
            return true;
        }
	#if 0
        if(!memcmp(header+8, "WAVE", 4)) {
            //ret = true;
        }
    #endif
    } else if (!memcmp(header, ".RMF", 4)) {
        *mimeType = MEDIA_MIMETYPE_CONTAINER_RM;
        *confidence = 0.4f;
        FILETYPEMSG("rmff file");
        return true;
    } else if (header[0] == 0x30) { // Check Sync Byte ASF file
        uint32_t rebuf[4];
        for (uint32_t i = 0; i < 4; i++) {
            rebuf[i] = (unsigned long) (header[4*i]&0xFF)<<24  | (unsigned long) (header[4*i+1]&0xFF)<<16 |
                       (unsigned long) (header[4*i+2]&0xFF)<<8 | (unsigned long) (header[4*i+3]&0xFF);
        }

        if ((rebuf[0] == 0x3026b275) && (rebuf[1] == 0x8e66cf11) && 
            (rebuf[2] == 0xa6d900aa) && (rebuf[3] == 0x0062ce6c)) {
            *mimeType = MEDIA_MIMETYPE_CONTAINER_WMV;
            *confidence = 1.0f;
			FILETYPEMSG("window media file");
            return true;
        }
    } else if (header[0] == 0x1a && header[1] == 0x45 && header[2] == 0xdf && header[3] == 0xa3) {
        uint8_t p[50];
    	n = source->readAt(4, p, sizeof(p));
        if (n < (ssize_t)sizeof(p)) {
            //ALOGE("SniffTCCMedia - source->readAt(4, p, sizeof(p)) - error!!!");
        } else {
            for (uint32_t i = 0; i < 40; i++) {
                if (p[i] == 0x42 && p[i+1] == 0x82) {
					if (!memcmp(p+i+3, "matroska", 8)) {
						*mimeType = MEDIA_MIMETYPE_CONTAINER_MATROSKA;
						*confidence = 0.7f;
						FILETYPEMSG("mkv file");
						return true;
					} else if (!memcmp(p+i+3, "webm", 4)) {
						*mimeType = MEDIA_MIMETYPE_CONTAINER_WEBM;
						*confidence = 0.7f;
						FILETYPEMSG("webm file");
						return true;
					}
                }
            }
        }
    } else if (!memcmp(header, "fLaC", 4)) {
        // Planet 2011.4.26 "fLaC" only search
    #if 0
        if (header[4] == 0 && header[5] == 0 && header[6] == 0 && header[7] == 0x22)
    #endif
        {
            *mimeType = MEDIA_MIMETYPE_CONTAINER_FLAC;
            *confidence = 0.4f;
            FILETYPEMSG("flac file");
            return true;
        }
    } else if (!memcmp(header, "MAC ", 4)) {
        *mimeType = MEDIA_MIMETYPE_CONTAINER_APE;
        *confidence = 0.4f;
        FILETYPEMSG("ape file");
        return true;
    } else if (!memcmp(header+4, "ftypqt", 6)) {
        *mimeType = MEDIA_MIMETYPE_CONTAINER_MPEG4;
        *confidence = 0.09;
        FILETYPEMSG("mov file");
        return true;
    } else if (!memcmp("\x00\x00\x01\xba", header, 4)) {
        *mimeType = MEDIA_MIMETYPE_CONTAINER_MPG;
        *confidence = 0.3;
        FILETYPEMSG("mpg file");
        return true;
    } else if (!memcmp(header, "FLV", 3)) {
        *mimeType = MEDIA_MIMETYPE_CONTAINER_FLV;
        *confidence = 0.4f;
        FILETYPEMSG("FLV file");
        return true;
    } else {
        bool sniffed = false;
        bool sniffingWeakly = false;
        if ((DataSource::kIsCachingDataSource | DataSource::kWantsPrefetching) 
                & source->flags()) {
            loff_t size = 0;
            if (source->getSize(&size) != OK || size <= 0) {
                sniffingWeakly = true;
            }
        }

        uint8_t p[1088];
        memset(p, 0, 1088);
    	n = source->readAt(0, p, sizeof(p));
        if (n < (ssize_t)sizeof(p)) {
            //ALOGE("SniffTCCMedia - source->readAt(0, p, sizeof(p)) - error!!!");
            //return false;
        } else {
			// skip id3 tag
			unsigned int id3tagsize = 0;
			if (!strncmp((char*)p, "ID3", 3)) {
				id3tagsize = (p[6] << 21) | (p[7] << 14) | (p[8] <<  7) | (p[9] <<  0);
				id3tagsize += 10;
				n = source->readAt(id3tagsize, p, sizeof(p)); 
			}
			if (!strncmp((char*)p, "fLaC", 4)) {
                *mimeType = MEDIA_MIMETYPE_CONTAINER_FLAC;
                *confidence = 0.4f;
                FILETYPEMSG("flac file"); 
				return true;
			}	

            for (uint32_t i = 1; i < 1024-3; i++) {
                if (p[i] == 0 && p[i+1] == 0 && p[i+2] == 0x01 && p[i+3] == 0xba) {
                    *mimeType = MEDIA_MIMETYPE_CONTAINER_MPG;
                    *confidence = 0.3f;
                    FILETYPEMSG("mpg file");
                    return true;
                }

                if (p[i] == 'R' && p[i+1] == 'I' && p[i+2] == 'F' && p[i+3] == 'F') {
                    *mimeType = MEDIA_MIMETYPE_CONTAINER_AVI;
                    *confidence = 0.4f;
                    FILETYPEMSG("avi file"); // Microsoft format AVI, WAV
					return true;
                }

                if (sniffingWeakly && 0x47 == p[i] && i < 512) {
                    sniffed = (((0x47 == p[i + (188 * 1)]) 
                                    && (0x47 == p[i + (188 * 2)]) 
                                    && (0x47 == p[i + (188 * 3)])) 
                                != ((0x47 == p[i + (192 * 1)]) 
                                    && (0x47 == p[i + (192 * 2)])
                                    && (0x47 == p[i + (192 * 3)])));

                    if (sniffed) {
                        *mimeType = MEDIA_MIMETYPE_CONTAINER_TS;
                        *confidence = 0.2f;
                        FILETYPEMSG("ts file");
                        return true;
                    }
                }
            }
        }
#if 0	// replaced by above code ==> //skip id3 tag flac
		//20110609 LeeJH flac search code add
        uint8_t *fpp = (uint8_t*)calloc(1, 5120);
        n = source->readAt(0, fpp, 5120);
        if (n < (ssize_t)(5120)) {
            free(fpp);
            //ALOGE("SniffTCCMedia - source->readAt(0, fpp, 5120) - error!!!");
            //return false;
        } else {
            if (fpp[0] == 'I' && fpp[1] == 'D' && fpp[2] == '3') {
                for (uint32_t i = 1 ; i < 5120-3 ; i++) {
                    if (fpp[i] == 'f' && fpp[i+1] == 'L' && fpp[i+2] == 'a' && fpp[i+3] == 'C') {
                        *mimeType = MEDIA_MIMETYPE_CONTAINER_FLAC;
                        *confidence = 0.4f;
                        FILETYPEMSG("flac file");
                        free(fpp);
                        return true;
                    }
                }
            }
            free(fpp);
        }
#endif

        if (!sniffingWeakly) {
            uint8_t *pp = (uint8_t*)calloc(1, 20480+512);
            n = source->readAt(0, pp, 20480+512);
            if (n < (ssize_t)(20480+512)) {
                free(pp);
                //ALOGE("SniffTCCMedia - source->readAt(0, pp, 20480+512) - error!!!");
                //return false;
            } else {
                // this condition must be exclusive
                for (uint32_t ii = 0; ii < 512; ii++) {/* up to 1024 */
                    if (0x47 == pp[ii]) {
                        sniffed = (((0x47 == pp[ii + (188 * 1)]) 
                                        && (0x47 == pp[ii + (188 * 54)]) 
                                        && (0x47 == pp[ii + (188 * 108)]))
                                    != ((0x47 == pp[ii + (192 * 1)]) 
                                        && (0x47 == pp[ii + (192 * 53)]) 
                                        && (0x47 == pp[ii + (192 * 106)])));

                        if (sniffed) {
                            *mimeType = MEDIA_MIMETYPE_CONTAINER_TS;
                            *confidence = 0.2f;
                            FILETYPEMSG("ts file");
                            free(pp);
                            return true;
                        }
                    }
                }
                free(pp);
            }
        }
    }

    // CDK sniff with data source extension
    String8 ext;
    if (source->flags() & DataSource::kLocalFileSource) {
		String8 filePath = static_cast<FileSource*>(source.get())->getFilePath();
		ext = filePath.getPathExtension();
    }
    ext.toUpper();

    if (ext.length() >= 3) {
        CDKWrapperStagefright *pCDK = NULL;
        for (int32_t i = 0; i < dim(tSniffListWithExt); i++) {
            if (strncasecmp(ext.string(), tSniffListWithExt[i].ext, 
                            strlen(tSniffListWithExt[i].ext))) {
                if (pCDK) delete pCDK;
				pCDK = NULL;
				continue;
            }

            pCDK = new CDKWrapperStagefright();
            pCDK->CDKInit(source);
            if (!pCDK->ContainerParse(
                           TCC_CDK_WRAPPER_PARSER_GETMETADATAMODE, sourceScheme, (char *)(ext.string()+1))
                      && SniffByTargetObject((void *)pCDK, tSniffListWithExt[i].checkType)) {
                *mimeType = tSniffListWithExt[i].mimeType;
                *confidence = tSniffListWithExt[i].confidence;
                FILETYPEMSG("ext. %s", tSniffListWithExt[i].fileTypeMsg);
                delete pCDK;
                return true;
            }
            delete pCDK;
			pCDK = NULL;
        }
    }

    // CDK sniff without data source extension
    CDKWrapperStagefright *pCDK = new CDKWrapperStagefright();
    pCDK->CDKInit(source);
    if (pCDK->IsAACFile() == TCC_CDK_WRAPPER_OK) {
        // AAC ADTS audio
        *mimeType = MEDIA_MIMETYPE_CONTAINER_AAC;
        *confidence = 0.19f;
        FILETYPEMSG("aac file");
        delete pCDK;
        return true;
	} else if (pCDK->IsMP2File() == TCC_CDK_WRAPPER_OK) {
        // MP2 audio
        *mimeType = MEDIA_MIMETYPE_CONTAINER_MP2;
        *confidence = 0.4f;
        FILETYPEMSG("mp2 file");
        delete pCDK;
        return true;
    }

	delete pCDK;

	//ALOGE("SniffTCCMedia - not supportted file format!!!");
    return false;
}

}  // namespace android

