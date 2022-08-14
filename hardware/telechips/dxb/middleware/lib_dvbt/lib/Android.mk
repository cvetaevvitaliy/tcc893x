LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS :=
LOCAL_MODULE_TAGS := eng debug	
	
include $(BUILD_MULTI_PREBUILT)
include $(call first-makefiles-under,$(LOCAL_PATH))
