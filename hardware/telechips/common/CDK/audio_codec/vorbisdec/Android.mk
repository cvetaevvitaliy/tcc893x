LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          VORBIS
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	vorbisdec.c \
	TCC_VORBIS_DEC.c

LOCAL_C_INCLUDES := \
  $(TOP)/external/tremolo/Tremolo \
	hardware/telechips/common/CDK/audio_codec 
	
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libvorbisidec


LOCAL_CFLAGS := \
	-DTCC_VORBIS_DECODE


LOCAL_MODULE := libTCC.vorbisdec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)