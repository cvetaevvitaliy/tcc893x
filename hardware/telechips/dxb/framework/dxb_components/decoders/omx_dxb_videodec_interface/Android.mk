LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
        src/vdec_k.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/cdk \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/container \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/video_codec \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/video_codec/vpu \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(DXB_C_INCLUDES)

LOCAL_SHARED_LIBRARIES := \
	libc \
	liblog \
	libOMX.TCC.DxB.base

LOCAL_CFLAGS := -fno-short-enums \
	$(BOARD_REV_FLAGS) \
	$(BOARD_HDMI_UI_SIZE_FLAGS) \
	$(DXB_CFLAGS)

LOCAL_LDFLAGS := \
	-L$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/video_codec/vpu

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS += -lTCC89xx_JPUCODEC_ANDROID_V1.2.0.3_Temp
LOCAL_LDFLAGS += -lTCC93_88xx_VPUCODEC_ANDROID_V0.0.53
endif

LOCAL_MODULE := libOMX.TCC.DxB.VPUDec
LOCAL_MODULE_TAGS := eng debug
LOCAL_SHARED_LIBRARIES += $(DXB_SHARED_LIBRARIES)

include $(BUILD_SHARED_LIBRARY)
