LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := \
        libc \
        libui \
        libutils \
        libcutils \
        liblog \
        libpmap \
        libhardware \
        libOMX.TCC.DxB.base \
        libOMX.TCC.DxB.VPUDec

LOCAL_LDLIBS += \
        -lpthread \
        -ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := $(DXB_CFLAGS)

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif

LOCAL_SRC_FILES := \
	src/jpu_jpeg.c \
	src/tcc_utils.cpp \
	src/tcc_dxb_surface.cpp \
	src/omx_fbdev_sink_component.cpp

LOCAL_C_INCLUDES := \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	$(TCCDXB_OMX_TOP) \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/cdk \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/container \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(LOCAL_PATH)/include \
	$(DXB_C_INCLUDES)

ifeq ($(BOARD_VIDEO_DEINTERLACE_SUPPORT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DEINTERLACE_SUPPORT
endif

LOCAL_LDFLAGS := \
    -L$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/video_codec/vpu

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
    LOCAL_LDFLAGS += -lTCC89xx_JPUCODEC_ANDROID_V1.3.0.0
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
    LOCAL_LDFLAGS += -lTCC89xx_JPUCODEC_ANDROID_V1.3.0.0
endif

LOCAL_MODULE := libOMX.TCC.fbdevsink
LOCAL_MODULE_TAGS := eng debug
LOCAL_SHARED_LIBRARIES += $(DXB_SHARED_LIBRARIES)

include $(BUILD_SHARED_LIBRARY)
