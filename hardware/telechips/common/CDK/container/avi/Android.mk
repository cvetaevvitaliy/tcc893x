LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_PREBUILT_LIBS := \
	libTCC892x_AVIDMX_ANDROID_V2.9.5.a
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_PREBUILT_LIBS := \
	libTCC892x_AVIDMX_ANDROID_V2.9.5.a
endif

LOCAL_MODULE_TAGS := optional
include $(BUILD_MULTI_PREBUILT)

#############################################################################
ifeq ($(INSTALL_LIB_FF3),true)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 

LOCAL_MODULE := libff3
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS :=  $(PV_CFLAGS_MINUS_VISIBILITY) \

LOCAL_ARM_MODE := arm

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_AVIDMX_ANDROID_V2.9.5
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_AVIDMX_ANDROID_V2.9.5
endif

include $(BUILD_SHARED_LIBRARY)
endif
#############################################################################
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := cdmx_avi.c

LOCAL_MODULE := libcdmx_avi
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D__ANDROID__

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
	$(CDK_DIR)/cdk \
	$(CDK_DIR)/container \

#====================================================
# dynamic
ifeq ($(INSTALL_LIB_FF3),true)
LOCAL_CFLAGS += -DCDMX_USE_DYNAMIC_LOADING
else
# static
ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_AVIDMX_ANDROID_V2.9.5
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libTCC892x_AVIDMX_ANDROID_V2.9.5
endif
endif
#====================================================


include $(BUILD_STATIC_LIBRARY)

#############################################################################

