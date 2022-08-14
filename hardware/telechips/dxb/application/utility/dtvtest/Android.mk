LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \

LOCAL_CFLAGS :=

LOCAL_SRC_FILES := \
	dtvtest.c

LOCAL_MODULE := dtvtest
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_EXECUTABLE)
