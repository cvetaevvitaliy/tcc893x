# Copyright (C) 2013 Telechips, Inc.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	ext_encoder_wrapper.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	hardware/telechips/omx/omx_include \
	hardware/telechips/omx/omx_videodec_interface/include \
	hardware/telechips/omx/omx_videoenc_interface/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	hardware/telechips/common/libcamera/include \
	hardware/libhardware/include \
    hardware/telechips/common/tcc-interface/tccif-direct


LOCAL_SHARED_LIBRARIES := \
	liblog \
	libutils \
	libcutils \
	libtcc.video.call.direct.interface

LOCAL_MODULE := libDirect.Encoder.Wrapper
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

