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
	main.cpp \
	MxL101SF_PhyCtrlApi.cpp \
	MxL101SF_PhyCfg.cpp \
	MxL101SF_OEM_Drv.cpp

LOCAL_CFLAGS    += -g -O0
LOCAL_MODULE := mxl101sf
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_EXECUTABLE)
include $(call first-makefiles-under,$(LOCAL_PATH))