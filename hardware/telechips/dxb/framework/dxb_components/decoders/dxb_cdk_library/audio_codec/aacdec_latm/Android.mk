LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          AAC
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	aacdec_latm.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS := \
	-DTCC_LATM_PARSER

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC89xx_LATMDMX_ANDROID_V2.00.00
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib \
	-lTCC89xx_LATMDMX_ANDROID_V2.00.00
endif

LOCAL_MODULE := libTCC.DxB.latmdmx
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)