LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/omx_videoenc_component.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(OMX_INCLUDE) \
	$(OMX_TOP)/omx_base/include \
	$(OMX_TOP)/omx_videoenc_interface/include \
	hardware/telechips/common/libcamera/include \
	hardware/telechips/common \
	hardware/libhardware/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	$(TCC_OMX_INCLUDES) \
	$(PV_INCLUDES)

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libOMX.TCC.base \
	libOMX.TCC.VPUEnc \
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

ifeq ($(USE_MEDIASERVER_ALT),true)
LOCAL_CFLAGS += -DOPENMAX1_2
endif

LOCAL_MODULE := libOMX.TCC.VideoEnc
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

