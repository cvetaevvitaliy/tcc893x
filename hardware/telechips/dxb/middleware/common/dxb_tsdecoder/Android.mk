LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := \
	lib/libTSDecoder.a

LOCAL_MODULE_TAGS := eng debug	
	
include $(BUILD_MULTI_PREBUILT)