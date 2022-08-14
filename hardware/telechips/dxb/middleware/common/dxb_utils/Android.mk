LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/tcc_dxb_memory.c \
	src/tcc_dxb_queue.c \
	src/tcc_dxb_semaphore.c \
	src/tcc_dxb_thread.c \
	src/tcc_msg.cpp

	
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \

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
	$(TCC_OMX_FLAGS) \
	$(DXB_CFLAGS)

#LOCAL_LDFLAGS := \
#	-L$(OMX_TOP)/omx_lib \
#	-lOMX.TCC.common ## test

LOCAL_MODULE := libdxbutils
LOCAL_MODULE_TAGS := eng debug

#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)

