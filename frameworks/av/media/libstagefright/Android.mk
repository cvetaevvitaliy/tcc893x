LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include frameworks/av/media/libstagefright/codecs/common/Config.mk

LOCAL_SRC_FILES:=                         \
        ACodec.cpp                        \
        AACExtractor.cpp                  \
        AACWriter.cpp                     \
        AMRExtractor.cpp                  \
        AMRWriter.cpp                     \
        AudioPlayer.cpp                   \
        AudioSource.cpp                   \
        AwesomePlayer.cpp                 \
        CameraSource.cpp                  \
        CameraSourceTimeLapse.cpp         \
        DataSource.cpp                    \
        DRMExtractor.cpp                  \
        ESDS.cpp                          \
        FileSource.cpp                    \
        FLACExtractor.cpp                 \
        HTTPBase.cpp                      \
        JPEGSource.cpp                    \
        MP3Extractor.cpp                  \
        MP3Writer.cpp                     \
        MPEG2TSWriter.cpp                 \
        MPEG4Extractor.cpp                \
        MPEG4Writer.cpp                   \
        MediaAdapter.cpp                  \
        MediaBuffer.cpp                   \
        MediaBufferGroup.cpp              \
        MediaCodec.cpp                    \
        MediaCodecList.cpp                \
        MediaDefs.cpp                     \
        MediaExtractor.cpp                \
        MediaMuxer.cpp                    \
        MediaSource.cpp                   \
        MetaData.cpp                      \
        NuCachedSource2.cpp               \
        NuMediaExtractor.cpp              \
        OMXClient.cpp                     \
        OMXCodec.cpp                      \
        OggExtractor.cpp                  \
        SampleIterator.cpp                \
        SampleTable.cpp                   \
        SkipCutBuffer.cpp                 \
        StagefrightMediaScanner.cpp       \
        StagefrightMetadataRetriever.cpp  \
        SurfaceMediaSource.cpp            \
        ThrottledSource.cpp               \
        TimeSource.cpp                    \
        TimedEventQueue.cpp               \
        Utils.cpp                         \
        VBRISeeker.cpp                    \
        WAVExtractor.cpp                  \
        WVMExtractor.cpp                  \
        XINGSeeker.cpp                    \
        avc_utils.cpp                     \
        mp4/FragmentedMP4Parser.cpp       \
        mp4/TrackFragment.cpp             \

ifeq ($(USE_TELECHIPS_PROPRIETARY_LIBRARIES),true)
LOCAL_SRC_FILES += \
        NuCacheBase.cpp
endif

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/av/include/media/stagefright/timedtext \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/frameworks/native/include/media/openmax \
        $(TOP)/frameworks/native/services/connectivitymanager \
        $(TOP)/external/flac/include \
        $(TOP)/external/tremolo \
        $(TOP)/external/openssl/include \

LOCAL_C_INCLUDES+= \
		$(TOP)/hardware/telechips/common/stagefright/tccmediaextractor \
		$(TOP)/hardware/telechips/common/cdk_wrapper/main/include \
		$(TOP)/hardware/telechips/common/CDK/cdk \

LOCAL_SHARED_LIBRARIES := \
        libbinder \
        libcamera_client \
        libconnectivitymanager \
        libcrypto \
        libcutils \
        libdl \
        libdrmframework \
        libexpat \
        libgui \
        libicui18n \
        libicuuc \
        liblog \
        libmedia \
        libsonivox \
        libssl \
        libstagefright_omx \
        libstagefright_yuv \
        libsync \
        libui \
        libutils \
        libvorbisidec \
        libz \
        libpowermanager

LOCAL_STATIC_LIBRARIES := \
        libstagefright_color_conversion \
        libstagefright_aacenc \
        libstagefright_matroska \
        libstagefright_timedtext \
        libvpx \
        libwebm \
        libstagefright_mpeg2ts \
        libstagefright_id3 \
        libFLAC \
        libmedia_helper

LOCAL_SRC_FILES += \
        chromium_http_stub.cpp
LOCAL_CPPFLAGS += -DCHROMIUM_AVAILABLE=1

################################################################################
# Telechips
ifeq ($(USE_TCC_AWESOMEPLAYER),true)
LOCAL_CPPFLAGS += -D_USE_TCC_AWESOMEPLAYER_=1
LOCAL_CPPFLAGS += -DSF_HTTP_AVAILABLE=1
else
LOCAL_CPPFLAGS += -D_USE_TCC_AWESOMEPLAYER_=0
LOCAL_CPPFLAGS += -DSF_HTTP_AVAILABLE=0
endif

ifeq ($(USE_TCC_MEDIAEXTRACTOR),true)
LOCAL_CPPFLAGS += -D_USE_TCC_MEDIAEXTRACTOR_=1
else
LOCAL_CPPFLAGS += -D_USE_TCC_MEDIAEXTRACTOR_=0
endif
################################################################################

LOCAL_SHARED_LIBRARIES += libstlport
include external/stlport/libstlport.mk

LOCAL_SHARED_LIBRARIES += \
        libstagefright_enc_common \
        libstagefright_avc_common \
        libstagefright_foundation \
        libdl

# TELECHIPS
ifeq ($(USE_TELECHIPS_PROPRIETARY_LIBRARIES),true)
LOCAL_WHOLE_STATIC_LIBRARIES += \
	libstagefright_rtsp_tcc \
	libstagefright_mpeg2ts_tcc \
	libstagefright_httplive_tcc \
	libstagefright_http_tcc \
	libstagefright_dtcp_tcc \
	libTCC.MEDIA.extractor \
	libTCCAwesomePlayer \
	libTCC.v2ip.codec \
	libstagefright_wfd_tcc

LOCAL_SHARED_LIBRARIES += \
	libTCC_CDK_LIB \
	libTCC_CDK_WRAPPER \
	libpmap \
	libvoipsound \
	libTCCWifiDisplaySourceClient \
	libHDCP2api
endif

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE:= libstagefright

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
