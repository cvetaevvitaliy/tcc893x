LOCAL_PATH:= $(call my-dir)

# the quickboot library
# ============================================================
include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := quickboot_service_lib:obf-kk_quickboot_service_lib_v1033.jar
include $(BUILD_MULTI_PREBUILT)

# the library
# ============================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
            $(call all-subdir-java-files) \
	    com/android/server/EventLogTags.logtags \
	    com/android/server/am/EventLogTags.logtags

LOCAL_MODULE:= services

#ifeq ($(USE_QUICKBOOT),true)
LOCAL_STATIC_JAVA_LIBRARIES := quickboot_service_lib
#endif

LOCAL_JAVA_LIBRARIES := android.policy conscrypt telephony-common

include $(BUILD_JAVA_LIBRARY)

include $(BUILD_DROIDDOC)
