LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          MP3
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	mp3dec.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS := \
	-DTCC_MP3_DECODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_MP3DEC_ANDROID_V5.00.00
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC893x_MP3DEC_ANDROID_V5.00.00
endif

LOCAL_MODULE := libTCC.DxB.mp3dec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)