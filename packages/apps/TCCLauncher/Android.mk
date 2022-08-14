LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := TelechipsLauncher

LOCAL_OVERRIDES_PACKAGES := Launcher2

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
