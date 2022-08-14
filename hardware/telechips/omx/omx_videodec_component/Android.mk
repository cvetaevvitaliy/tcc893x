LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/omx_videodec_component.c \
	src/omx_vpudec_component.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	hardware/telechips/common/opencore/opencorehw/main \
	hardware/telechips/common/opencore/opencorehw/tccrenderer \
	$(OMX_INCLUDE) \
	$(OMX_TOP)/omx_base/include \
	$(OMX_TOP)/omx_videodec_interface/include \
	$(TCC_OMX_INCLUDES) \
	hardware/telechips/common \
	hardware/telechips/common/cdk_wrapper/main/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	$(PV_INCLUDES) \
	hardware/libhardware/include \
  	hardware/telechips/common/stagefright/tccmediaextractor

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libOMX.TCC.base \
	libOMX.TCC.VPUDec \
	libhardware \
	libpmap
	
LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(TCC_OMX_FLAGS) \
	-DSYS_LINUX \
	$(TARGET_BOOTLOADER_BOARD_CFLAGS) \
	$(BOARD_HDMI_UI_SIZE_FLAGS)

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif
ifeq ($(BOARD_LCD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DUSE_LCD_VIDEO_VSYNC
endif
ifeq ($(BOARD_VIDEO_DEINTERLACE_SUPPORT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DEINTERLACE_SUPPORT
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc92xx)
LOCAL_CFLAGS += -DTCC_89XX_INCLUDE
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc93xx)
LOCAL_CFLAGS += -DTCC_93XX_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc88xx)
LOCAL_CFLAGS += -DTCC_88XX_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_CFLAGS += -DTCC_892X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
LOCAL_CFLAGS += -DTCC_8925S_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
else
LOCAL_CFLAGS += -DTCC_VPU_C7_INCLUDE
LOCAL_CFLAGS += -DRING_BUFFER_MODULE_ENABLE
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_CFLAGS += -DTCC_893X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
LOCAL_CFLAGS += -DTCC_8935S_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
else
LOCAL_CFLAGS += -DTCC_VPU_C7_INCLUDE
LOCAL_CFLAGS += -DRING_BUFFER_MODULE_ENABLE
endif
endif

ifeq ($(BOARD_DIVXDRM_FLAGS),true)
LOCAL_SHARED_LIBRARIES += \
	libpvdivxdrm5plugin \
	
LOCAL_CFLAGS += \
	-DDIVX_DRM5
endif

ifeq ($(USE_MEDIASERVER_ALT),true)
LOCAL_CFLAGS += -DOPENMAX1_2
endif

ifeq ($(TARGET_USE_TZOS), true)
LOCAL_SHARED_LIBRARIES += \
	libteesmapi

LOCAL_CFLAGS += \
	-DTC_SECURE_MEMORY_COPY
endif

LOCAL_MODULE := libOMX.TCC.VideoDec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

