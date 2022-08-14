LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	src/tcc_font.c \
	src/tcc_fontDraw.c \
	src/tcc_fontFT.c 
	
LOCAL_LDFLAGS := \
        -L$(LOCAL_PATH)/lib -lft2

#LOCAL_STATIC_LIBRARIES := \
#	libft2

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
#	libft2

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(TCC_OMX_FLAGS) \
	$(DXB_CFLAGS)

LOCAL_MODULE := libdxbfont
LOCAL_MODULE_TAGS := eng debug

#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)

