# Copyright (C) 2013 Telechips, Inc.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	ext_render_wrapper.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	hardware/telechips/common/tcc-interface/tccif-direct \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	frameworks/base/media/libstagefright/include \
	frameworks/base/include \
	$(OMX_TOP)/omx_base/include

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libgui	\
	libutils \
	libcutils \
	libtcc.video.call.direct.interface

LOCAL_MODULE := libDirect.Render.Wrapper
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

