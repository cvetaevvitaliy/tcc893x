LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          APE
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	apedec.c

LOCAL_C_INCLUDES := \
	hardware/telechips/common/CDK/audio_codec
	
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils

LOCAL_CFLAGS := \
	-DTCC_APE_DECODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_APEDEC_ANDROID_V1.1.5
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC893x_APEDEC_ANDROID_V1.1.5
endif

LOCAL_MODULE := libTCC.apedec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)