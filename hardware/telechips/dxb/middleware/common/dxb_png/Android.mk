LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := \
	lib/TCCxxxx_PNG_DEC_LIB_ANDROID_V200.a

LOCAL_MODULE_TAGS := eng debug	
	
include $(BUILD_MULTI_PREBUILT)