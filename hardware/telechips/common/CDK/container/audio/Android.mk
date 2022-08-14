LOCAL_PATH := $(call my-dir)
ifeq ($(USE_INTERNAL_LIBS),true)
include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := \
	libTCC892x_APEDMX_ANDROID_V1.00.01.a 	\
	libTCCxxxx_AACDMX_ANDROID_V1.07.02.a 	\
	libTCCxxxx_FLACDMX_ANDROID_V1.05.03.a 	\
	libTCCxxxx_MP3DMX_ANDROID_V1.00.03.a

LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)
endif
#############################################################################
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := cdmx_audio.c

LOCAL_MODULE := libcdmx_audio
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D__ANDROID__

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
	$(CDK_DIR)/cdk \
	$(CDK_DIR)/container \
	$(CDK_DIR)/audio_codec \

ifeq ($(USE_INTERNAL_LIBS),true)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_APEDMX_ANDROID_V1.00.01 	\
	libTCCxxxx_AACDMX_ANDROID_V1.07.02 	\
	libTCCxxxx_FLACDMX_ANDROID_V1.05.03 	\
	libTCCxxxx_MP3DMX_ANDROID_V1.00.03
endif

include $(BUILD_STATIC_LIBRARY)

#############################################################################
