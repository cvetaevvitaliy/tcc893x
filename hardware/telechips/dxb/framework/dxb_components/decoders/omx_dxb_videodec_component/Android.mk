LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false
 
LOCAL_SRC_FILES := \
	src/video_user_data.c \
	src/vpu_ringbuffer_util.c \
	src/omx_videodec_component.c \
	src/omx_vpu_c7_ringbuffer_dec_component.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/cdk \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/container \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/video_codec \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/video_codec/vpu \
	$(TCCDXB_OMX_TOP)/decoders/omx_dxb_videodec_interface/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(TCCDXB_TOP)/../../../ \
	$(DXB_C_INCLUDES)

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libpmap \
	libOMX.TCC.DxB.base \
	libOMX.TCC.DxB.VPUDec

	
LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(TCC_OMX_FLAGS) \
	-DSYS_LINUX \
	$(BOARD_HDMI_UI_SIZE_FLAGS) \
	$(DXB_CFLAGS)

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif
ifeq ($(BOARD_LCD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DUSE_LCD_VIDEO_VSYNC
endif

LOCAL_MODULE := libOMX.TCC.DxB.VideoDec
LOCAL_MODULE_TAGS := eng debug
LOCAL_SHARED_LIBRARIES += $(DXB_SHARED_LIBRARIES)

include $(BUILD_SHARED_LIBRARY)

