LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          AAC
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	aacenc.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS := \
	-DTCC_AAC_ENCODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_AACENC_ANDROID_V2.02.01
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_AACENC_ANDROID_V2.02.01
endif

LOCAL_MODULE := libTCC.DxB.aacenc
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)