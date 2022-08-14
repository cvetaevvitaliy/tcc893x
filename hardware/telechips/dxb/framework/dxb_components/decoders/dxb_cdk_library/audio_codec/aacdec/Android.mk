LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          AAC
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	aacdec.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS := \
	-DTCC_AAC_DECODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC892x_AACDEC_ANDROID_V3.21.02_Neon
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC893x_AACDEC_ANDROID_V3.21.02_Neon
endif

LOCAL_MODULE := libTCC.DxB.aacdec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)