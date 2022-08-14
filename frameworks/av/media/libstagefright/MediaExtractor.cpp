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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaExtractor"
#include <utils/Log.h>

#include "include/AMRExtractor.h"
#include "include/MP3Extractor.h"
#include "include/MPEG4Extractor.h"
#include "include/WAVExtractor.h"
#include "include/OggExtractor.h"
#include "include/MPEG2PSExtractor.h"
#include "include/MPEG2TSExtractor.h"
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "include/TCCMPEG2TSExtractor.h"
#endif
#include "include/DRMExtractor.h"
#include "include/WVMExtractor.h"
#include "include/FLACExtractor.h"
#include "include/AACExtractor.h"

#include "matroska/MatroskaExtractor.h"

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "TCCMediaExtractor.h"
#endif

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MetaData.h>
#include <utils/String8.h>

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include <cutils/properties.h>
#endif

namespace android {

sp<MetaData> MediaExtractor::getMetaData() {
    return new MetaData;
}

uint32_t MediaExtractor::flags() const {
    return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK;
}

// static
sp<MediaExtractor> MediaExtractor::Create(
        const sp<DataSource> &source, const char *mime
    #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
        , bool useOriginal, const char *notifiedMIME
    #endif
        ) {
    sp<AMessage> meta;

    String8 tmp;
    float confidence;
    if (mime == NULL) {

        if (source->flags() & DataSource::kLocalFileSource) {
            String8 filePath = static_cast<FileSource*>(source.get())->getFilePath();
            ALOGD("FILENAME: %s", filePath.string());
        }

        if (!source->sniff(&tmp, &confidence, &meta)) {
            ALOGV("FAILED to autodetect media content.");

            return NULL;
        }

        mime = tmp.string();
        ALOGV("Autodetected media content as '%s' with confidence %.2f",
             mime, confidence);
    }

    bool isDrm = false;
    // DRM MIME type syntax is "drm+type+original" where
    // type is "es_based" or "container_based" and
    // original is the content's cleartext MIME type
    if (!strncmp(mime, "drm+", 4)) {
        const char *originalMime = strchr(mime+4, '+');
        if (originalMime == NULL) {
            // second + not found
            return NULL;
        }
        ++originalMime;
        if (!strncmp(mime, "drm+es_based+", 13)) {
            // DRMExtractor sets container metadata kKeyIsDRM to 1
            return new DRMExtractor(source, originalMime);
        } else if (!strncmp(mime, "drm+container_based+", 20)) {
            mime = originalMime;
            isDrm = true;
        } else {
            return NULL;
        }
    }

    // Planet 20130930 HBR Mode(Audio and be played.) Start
    char value[PROPERTY_VALUE_MAX];
    property_get("persist.sys.spdif_setting", value, "");

    if ( strcmp(value, "4") == 0 ) {
        if(!strncmp(mime, "audio", 5) || !strncmp(mime, "application", 11)) {
            ALOGE("~~~~~!!!!! Audio File : type=1 mime: %s !!!!!~~~~~",mime);
            property_set("tcc.hdmi.audio_type", "3");
        } else {
            ALOGE("~~~~~!!!!! Not Audio File : type=0 mime: %s !!!!!~~~~~",mime);
        }
    }
    // Planet 20130930 HBR Mode(Audio and be played.) End

    MediaExtractor *ret = NULL;
    if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG4)
            || !strcasecmp(mime, "audio/mp4")) {
#if _USE_TCC_MEDIAEXTRACTOR_
        char value[PROPERTY_VALUE_MAX];
        property_get("tcc.use.extractor.mp4", value, "1");
        if (atoi(value) == 0 || !strcasecmp(mime, "audio/mp4")) {
            ALOGI("Use Google's MPEG4Extractor");
            useOriginal = true;
        }

        if (useOriginal == false) {
            ret = new TCCMediaExtractor(source, notifiedMIME);
        }
        else
#endif//_USE_TCC_MEDIAEXTRACTOR_
        {
            ret = new MPEG4Extractor(source);
        }
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_MPEG)) {
        ret = new MP3Extractor(source, meta);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_NB)
            || !strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AMR_WB)) {
        ret = new AMRExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_FLAC)) {
        ret = new FLACExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WAV)) {
        ret = new WAVExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGG)) {
        ret = new OggExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MATROSKA)) {
#if _USE_TCC_MEDIAEXTRACTOR_
		char value[PROPERTY_VALUE_MAX];
		property_get("tcc.use.extractor.mkv", value, "1");
		if (atoi(value) == 0) {
			ALOGI("Use Google's MatroskaExtractor");
			useOriginal = true;
		}

		if (useOriginal == false) 
		{
			ret = new TCCMediaExtractor(source, notifiedMIME);
		}
		else
#endif//_USE_TCC_MEDIAEXTRACTOR_
		{
			ret = new MatroskaExtractor(source);
		}
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)) {
#if _USE_TCC_MEDIAEXTRACTOR_
		char value[PROPERTY_VALUE_MAX];
		property_get("tcc.use.extractor.ts", value, "1");
		if (atoi(value) == 0) {
			ALOGI("Use Google's MPEG2TSExtractor");
			useOriginal = true;
		}

		if (useOriginal == false) 
		{
			if (source->flags() & DataSource::kIsCachingDataSource)
				ret = new MPEG2TSExtractor(source);
			else
				ret = new TCCMediaExtractor(source, notifiedMIME);
		}
		else
#endif//_USE_TCC_MEDIAEXTRACTOR_
		{
			ret = new MPEG2TSExtractor(source);
		}
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WVM)) {
        // Return now.  WVExtractor should not have the DrmFlag set in the block below.
        return new WVMExtractor(source);
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_AAC_ADTS)) {
#if _USE_TCC_MEDIAEXTRACTOR_
		char value[PROPERTY_VALUE_MAX];
		property_get("tcc.use.extractor.aac", value, "1");
		if (atoi(value) == 0) {
			ALOGI("Use Google's AACExtractor");
			useOriginal = true;
		}

		if (useOriginal == false) 
		{
			ret = new TCCMediaExtractor(source);
		}
		else
#endif//_USE_TCC_MEDIAEXTRACTOR_
		{
			ret = new AACExtractor(source, meta);
		}
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2PS)) {
        ret = new MPEG2PSExtractor(source);
    }
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WEBM)) {
		char value[PROPERTY_VALUE_MAX];
		property_get("tcc.use.extractor.webm", value, "1");
		if (atoi(value) == 0) {
			ALOGI("Use Google's MatroskaExtractor");
			useOriginal = true;
		}

		if (useOriginal == false) {
			ret = new TCCMediaExtractor(source);
		} else {
			ret = new MatroskaExtractor(source);
		}
	}
#endif
#if _USE_TCC_MEDIAEXTRACTOR_
    else if (!strcasecmp(mime, MEDIA_MIMETYPE_INDEX_M3U8)) {
        ret = new TCCMPEG2TSExtractor(source);
	} else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AVI)) {
        ret = new TCCMediaExtractor(source);
	} else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_TS)) {
#if _USE_TCC_MEDIAEXTRACTOR_
		char value[PROPERTY_VALUE_MAX];
		property_get("tcc.use.extractor.ts", value, "1");
		if (atoi(value) == 0) {
			ALOGI("Use Google's MPEG2TSExtractor");
			useOriginal = true;
		}

		if (useOriginal == false) 
		{
			ret = new TCCMediaExtractor(source, notifiedMIME);
		}
		else
#endif//_USE_TCC_MEDIAEXTRACTOR_
		{
			ret = new MPEG2TSExtractor(source);
		}
	} else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPG)) {
#if _USE_TCC_MEDIAEXTRACTOR_
		char value[PROPERTY_VALUE_MAX];
		property_get("tcc.use.extractor.mpg", value, "1");
		if (atoi(value) == 0) {
			ALOGI("Use Google's MPEG2PSExtractor");
			useOriginal = true;
		}

		if (useOriginal == false) 
		{
			ret = new TCCMediaExtractor(source, notifiedMIME);
		}
		else
#endif//_USE_TCC_MEDIAEXTRACTOR_
		{
			ret = new MPEG2PSExtractor(source);
		}
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_FLAC)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_APE)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MP2)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_RA)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WMA)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_AAC)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_WMV)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_ASF)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MOV)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_FLV)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_DIVX)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_RM)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_OGM)
            || !strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_SSIF)) {
        ret = new TCCMediaExtractor(source, notifiedMIME);
    }
#endif//_USE_TCC_MEDIAEXTRACTOR_

    if (ret != NULL) {
       if (isDrm) {
           ret->setDrmFlag(true);
       } else {
           ret->setDrmFlag(false);
       }
    }

    return ret;
}

// TELECHIPS, SSG
sp<MetaData> MediaExtractor::getMetaData(bool withThumbnail) {
	return getMetaData();
}

// INTERNAL_SUBTITLE_INCLUDE
int32_t MediaExtractor::getNumSubtitleStreams() {
    return 0;
}

int32_t MediaExtractor::getCurrentSubtitleStream(void) {
    return 0;
}

char* MediaExtractor::getLangSubtitleStream(void) {
    return NULL;
}

int32_t MediaExtractor::SelectSubtitleStream(int32_t num) {
    return 0;
}

int32_t  MediaExtractor::GetSubtitleData() {
    return 0;
}

int32_t  MediaExtractor::SetSubtitleSeek(int msec) {
    return 0;
}

int32_t MediaExtractor::get_inter_subtitle_type() {
    return 0;
}

int32_t MediaExtractor::GetSubtitleType(void) {
    return 0;
}

void  MediaExtractor::SetCurrentPlaybackPosition(int64_t usec) {
    return;
}

int32_t MediaExtractor::postInitIfNecessary(int64_t prefillUs) {
    return OK;
}

uint32_t MediaExtractor::GetVideoBufferedTime(){
    return 0;
}

int32_t MediaExtractor::checkQueuedDurationUs(int64_t *durationUs, bool *fulled) {
    *durationUs = 0;
	*fulled = false;
    return INVALID_OPERATION;
}

}  // namespace android
