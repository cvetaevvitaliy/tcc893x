LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := \
    libc \
    libutils \
    libcutils \
    liblog

LOCAL_LDLIBS += \
    -lpthread \
    -ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
    -D_POSIX_SOURCE \
    $(DXB_CFLAGS)

LOCAL_LDFLAGS :=

LOCAL_STATIC_LIBRARIES := 

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(DXB_C_INCLUDES)

LOCAL_SRC_FILES := \
    src/DVBTSpace.c

LOCAL_SHARED_LIBRARIES += $(DXB_SHARED_LIBRARIES)

LOCAL_MODULE := libdvbt_freqspace
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)	
include $(call first-makefiles-under,$(LOCAL_PATH))
