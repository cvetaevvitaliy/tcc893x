# Copyright 2009 Telechips, Inc.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_CFLAGS := -fno-short-enums \
		$(TARGET_BOOTLOADER_BOARD_CFLAGS) \
		$(BOARD_HDMI_UI_SIZE_FLAGS) \
		-DTCC_DUAL_HDMI_DIAPLAY

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif

LOCAL_SRC_FILES := \
	src/TelechipsCameraHardware.cpp \
	src/TCC_V4l2_Camera.cpp \
	src/MessageQueue.cpp \
	src/TelechipsCameraHal_Module.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include

ifeq ($(filter-out tcc92xx,$(TARGET_BOARD_PLATFORM)),)
# don't need to do
else
LOCAL_SRC_FILES += \
        vpudec/src/jpu_jpeg.c

LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/../CDK/video_codec/vpu \
	-lTCC89xx_JPUCODEC_ANDROID_V1.4.0.2
endif


ifeq ($(filter-out tcc92xx,$(TARGET_BOARD_PLATFORM)),)
# don't need to do
else
LOCAL_SRC_FILES += \
	vpudec/src/vpudec_k.c

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../../omx/omx_base/include \
	$(LOCAL_PATH)/vpudec/include \
        $(TOP)/kernel/include/generated        \
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc93xx)
LOCAL_CFLAGS += -DTCC_93XX_INCLUDE
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc88xx)
LOCAL_CFLAGS += -DTCC_88XX_INCLUDE
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_CFLAGS += -DTCC_892X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
LOCAL_CFLAGS += -DTCC_8925S_INCLUDE
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_CFLAGS += -DTCC_893X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
LOCAL_CFLAGS += -DTCC_8935S_INCLUDE
endif
endif

endif

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libui \
	libbinder \
	liblog \
	libcamera_client \
	libpmap

#LOCAL_MODULE := libcamera
#LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS:= optional

include $(BUILD_SHARED_LIBRARY)


