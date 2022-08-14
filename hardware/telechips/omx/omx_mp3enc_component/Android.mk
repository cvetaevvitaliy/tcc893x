LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
	src/omx_mp3enc_component.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
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
	libTCC_CDK_AUDIO \
	
LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(TCC_OMX_FLAGS) \
	-D__ANDROID__ \
#	-D__FILE_INPUT__ 
	
ifeq ($(USE_MEDIASERVER_ALT),true)
LOCAL_CFLAGS += -DOPENMAX1_2
endif

LOCAL_MODULE := libOMX.TCC.mp3enc
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

