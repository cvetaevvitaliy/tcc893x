LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	tcc_omxcore.c \

LOCAL_MODULE := libTCC_OMXCore
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES += libutils libcutils liblog libdl

LOCAL_CFLAGS := $(TCC_OMX_CFLAGS)

ifeq ($(USE_MEDIASERVER_ALT),true)
LOCAL_CFLAGS += -DOPENMAX1_2
endif

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	frameworks/native/include/media/openmax

include $(BUILD_SHARED_LIBRARY)
