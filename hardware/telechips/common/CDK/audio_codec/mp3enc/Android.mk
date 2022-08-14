LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          MP3
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	mp3enc.c

LOCAL_C_INCLUDES := \
	hardware/telechips/common/CDK/audio_codec
	
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils

LOCAL_CFLAGS := \
	-DTCC_MP3_ENCODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_MP3ENC_ANDROID_V04.11.06_CTS
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC893x_MP3ENC_ANDROID_V04.11.06_CTS
endif
LOCAL_MODULE := libTCC.mp3enc
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)