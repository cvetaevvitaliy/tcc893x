LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) 

LOCAL_CFLAGS :=

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_SRC_FILES := \
	MxL603_Main.c \
	MxL603_OEM_Drv.c\
	MxL603_TunerApi.c \
	MxL603_TunerCfg.c

LOCAL_CFLAGS    += -g -O0
LOCAL_MODULE := mxl603
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_EXECUTABLE)
include $(call first-makefiles-under,$(LOCAL_PATH))
