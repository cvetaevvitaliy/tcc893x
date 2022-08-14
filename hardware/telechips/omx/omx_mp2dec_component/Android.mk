LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/omx_mp2dec_component.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(OMX_TOP)/omx_audio_interface/include \
	$(CDK_DIR)/cdk \
	$(CDK_DIR)/audio_codec \
	$(CDK_DIR)/container \
	$(OMX_INCLUDE) \
	$(OMX_TOP)/omx_base/include \

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libOMX.TCC.base \
	libOMX.TCC.audio

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(TCC_OMX_FLAGS) \
	-DSYS_LINUX

ifeq ($(USE_MEDIASERVER_ALT),true)
LOCAL_CFLAGS += -DOPENMAX1_2
endif

LOCAL_MODULE := libOMX.TCC.mp2dec
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

