LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/mute_strategy.c \
	src/omx_audiodec_component.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/cdk \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/audio_codec \
	$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/container \
	$(TCCDXB_OMX_TOP)/decoders/omx_dxb_audiodec_component/include \
	$(TCCDXB_OMX_TOP)/decoders/omx_dxb_audiodec_component/spdif/include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libOMX.TCC.DxB.base \
	libTCC_CDK_DXB_LIB
	
LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(TCC_OMX_FLAGS) \
	-D__ANDROID__ \
	$(DXB_CFLAGS)
	
LOCAL_LDFLAGS :=

LOCAL_MODULE := libOMX.TCC.DxB.audio
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)

