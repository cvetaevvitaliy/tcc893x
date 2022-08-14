LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          MP2
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	mp2dec.c

LOCAL_C_INCLUDES := \
	hardware/telechips/common/CDK/audio_codec
	
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils

LOCAL_CFLAGS := \
	-DTCC_MP2_DECODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_MP2DEC_ANDROID_V4.07.03
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC893x_MP2DEC_ANDROID_V4.07.05
endif

LOCAL_MODULE := libTCC.mp2dec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)