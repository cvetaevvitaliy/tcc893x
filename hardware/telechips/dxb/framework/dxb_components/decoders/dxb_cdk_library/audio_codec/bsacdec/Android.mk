LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------
#          BSAC
# -----------------------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	bsacdec.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../

LOCAL_CFLAGS := \
	-DTCC_BSAC_DECODE

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	 -L$(LOCAL_PATH)/lib \
	 -lTCC892x_BSACDEC_ANDROID_V3.01.07
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	 -L$(LOCAL_PATH)/lib \
	 -lTCC893x_BSACDEC_ANDROID_V3.01.07
endif

LOCAL_MODULE := libTCC.DxB.bsacdec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)