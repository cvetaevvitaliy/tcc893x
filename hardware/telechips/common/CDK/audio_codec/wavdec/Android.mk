LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          WAV
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	wavdec.c

LOCAL_C_INCLUDES := \
	hardware/telechips/common/CDK/audio_codec
	
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils

LOCAL_CFLAGS := \
	-DTCC_WAV_DECODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc92xx)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCCxxxx_WAVDEC_ANDROID_V4.20.00
endif
	
ifeq ($(TARGET_BOARD_PLATFORM),tcc93xx)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCCxxxx_WAVDEC_ANDROID_V4.20.00
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc88xx)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCCxxxx_WAVDEC_ANDROID_V4.20.00
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCCxxxx_WAVDEC_ANDROID_V4.20.00
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCCxxxx_WAVDEC_ANDROID_V4.20.00
endif

LOCAL_MODULE := libTCC.pcmdec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)