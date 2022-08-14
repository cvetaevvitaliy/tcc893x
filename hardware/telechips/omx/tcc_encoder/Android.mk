LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/encoder.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(OMX_TOP)/omx_videoenc_interface/include \
	hardware/telechips/common \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libOMX.TCC.VPUEnc \
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

LOCAL_MODULE := libTCC_Encoder
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

