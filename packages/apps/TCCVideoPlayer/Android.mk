#ifdef _DO_NOT_BUILD_
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := VideoPlayer

LOCAL_CERTIFICATE := platform

# We mark this out until Mtp and MediaMetadataRetriever is unhidden.
# LOCAL_SDK_VERSION := current

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
#endif
