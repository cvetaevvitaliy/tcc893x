LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          AAC
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	aacdec_latm.c

LOCAL_C_INCLUDES := \
	hardware\telechips\common\CDK\container\audio

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils

LOCAL_CFLAGS := \
	-DTCC_LATM_PARSER

LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCCxxxx_LATMDMX_ANDROID_V2.00.00

LOCAL_MODULE := libTCC.latmdmx
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)