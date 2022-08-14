# Copyright (C) 2012, Telechips Limited
# by ZzaU(B070371)
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

USE_STAGEFRIGHT_IF = 0

LOCAL_SRC_FILES := \
    ext_player_wrapper.cpp

LOCAL_C_INCLUDES:= \
	hardware/telechips/omx/omx_include \
	hardware/telechips/omx/omx_videodec_interface/include \
	hardware/telechips/omx/omx_videoenc_interface/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	hardware/telechips/common/libcamera/include \
	frameworks/base/include \
	frameworks/base/media/libstagefright/include \
	hardware/libhardware/include

ifeq ($(USE_STAGEFRIGHT_IF), 1)
LOCAL_C_INCLUDES += hardware/telechips/common/tcc-interface/tccif
else
LOCAL_C_INCLUDES += hardware/telechips/common/tcc-interface/tccif-direct
endif

LOCAL_SHARED_LIBRARIES := \
	libbinder \
    libutils \
    libcutils \
    libgui

ifeq ($(USE_STAGEFRIGHT_IF), 1)
LOCAL_SHARED_LIBRARIES += libtcc.video.call.interface
else
LOCAL_SHARED_LIBRARIES += libtcc.video.call.direct.interface
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_CFLAGS += -DTCC_892X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
LOCAL_CFLAGS += -DTCC_8925S_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
else
LOCAL_CFLAGS += -DTCC_VPU_C7_INCLUDE
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_CFLAGS += -DTCC_893X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
LOCAL_CFLAGS += -DTCC_8935S_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
else
LOCAL_CFLAGS += -DTCC_VPU_C7_INCLUDE
endif
endif

LOCAL_MODULE := libDirect.Player.Wrapper
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

