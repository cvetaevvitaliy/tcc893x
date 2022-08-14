LOCAL_PATH:= $(call my-dir)

#
# libmediaplayerservice
#

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=               \
    ActivityManager.cpp         \
    Crypto.cpp                  \
    Drm.cpp                     \
    HDCP.cpp                    \
    MediaPlayerFactory.cpp      \
    MediaPlayerService.cpp      \
    MediaRecorderClient.cpp     \
    MetadataRetrieverClient.cpp \
    MidiFile.cpp                \
    MidiMetadataRetriever.cpp   \
    RemoteDisplay.cpp           \
    SharedLibrary.cpp           \
    StagefrightPlayer.cpp       \
    StagefrightRecorder.cpp     \
    TestPlayerStub.cpp          \

LOCAL_SHARED_LIBRARIES :=       \
    libbinder                   \
    libcamera_client            \
    libcutils                   \
    liblog                      \
    libdl                       \
    libgui                      \
    libmedia                    \
    libsonivox                  \
    libstagefright              \
    libstagefright_foundation   \
    libstagefright_httplive     \
    libstagefright_omx          \
    libstagefright_wfd          \
    libutils                    \
    libvorbisidec               \

LOCAL_STATIC_LIBRARIES :=       \
    libstagefright_nuplayer     \
    libstagefright_rtsp         \

###################################################
# Telechips
ifeq ($(USE_TCC_AWESOMEPLAYER),true)
LOCAL_CPPFLAGS += -D_USE_TCC_AWESOMEPLAYER_=1
LOCAL_CPPFLAGS += -D_STAGEFRIGHT_HTTPLIVE_=1
LOCAL_CPPFLAGS += -D_STAGEFRIGHT_RTSP_=1
else
LOCAL_CPPFLAGS += -D_USE_TCC_AWESOMEPLAYER_=0
LOCAL_CPPFLAGS += -D_STAGEFRIGHT_HTTPLIVE_=0
LOCAL_CPPFLAGS += -D_STAGEFRIGHT_RTSP_=0
endif

ifeq ($(BOARD_HAVE_SPDIF),true)
LOCAL_CFLAGS += -DTCC_SPDIF_BITSTREAM
LOCAL_SHARED_LIBRARIES += libspdifsound
endif
###################################################

LOCAL_C_INCLUDES :=                                                 \
    $(call include-path-for, graphics corecg)                       \
    $(TOP)/frameworks/av/media/libstagefright/include               \
    $(TOP)/frameworks/av/media/libstagefright/rtsp                  \
    $(TOP)/frameworks/av/media/libstagefright/wifi-display          \
    $(TOP)/frameworks/native/include/media/openmax                  \
    $(TOP)/external/tremolo/Tremolo                                 \
    $(TOP)/kernel/include/generated				    \

LOCAL_MODULE:= libmediaplayerservice

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
