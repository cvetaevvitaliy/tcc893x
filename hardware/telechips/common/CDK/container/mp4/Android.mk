LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_PREBUILT_LIBS := \
	libTCC892x_MP4DMX_ANDROID_V3.02.08.a
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_PREBUILT_LIBS := \
	libTCC893x_MP4DMX_ANDROID_V3.02.08.a
endif

LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

#############################################################################
ifeq ($(INSTALL_LIB_FF2),true)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 

LOCAL_MODULE := libff2
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS :=  $(PV_CFLAGS_MINUS_VISIBILITY) \

LOCAL_ARM_MODE := arm

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_MP4DMX_ANDROID_V3.02.08
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC893x_MP4DMX_ANDROID_V3.02.08
endif

include $(BUILD_SHARED_LIBRARY)
endif
#############################################################################
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := cdmx_mp4.c

LOCAL_MODULE := libcdmx_mp4
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D__ANDROID__

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
	$(CDK_DIR)/cdk \
	$(CDK_DIR)/container \

#====================================================
# dynamic
ifeq ($(INSTALL_LIB_FF2),true)
LOCAL_CFLAGS += -DCDMX_USE_DYNAMIC_LOADING
else
# static
ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_MP4DMX_ANDROID_V3.02.08
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC893x_MP4DMX_ANDROID_V3.02.08
endif
endif
#====================================================


include $(BUILD_STATIC_LIBRARY)

#############################################################################

