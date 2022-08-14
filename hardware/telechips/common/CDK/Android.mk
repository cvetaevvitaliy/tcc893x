LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := \
	cdk/cdk_sys.c \
	cdk/cdk_aenc.c \

LOCAL_MODULE := libTCC_CDK_AUDIO
LOCAL_MODULE_TAGS := optional

-include external/opencore/Config.mk
-include $(PV_TOP)/Android_system_extras.mk

LOCAL_CFLAGS :=  $(PV_CFLAGS_MINUS_VISIBILITY) -D__ANDROID__

LOCAL_C_INCLUDES := \
 	$(PV_INCLUDES) \
	$(LOCAL_PATH)/cdk \
	$(LOCAL_PATH)/container \

LOCAL_SHARED_LIBRARIES += \
	libutils \
	liblog

include $(BUILD_SHARED_LIBRARY)

#############################################################################
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := \
	cdk/cdk_sys.c \

LOCAL_MODULE := libTCC_CDK_LIB
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D__ANDROID__

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_CFLAGS +=  -DTCC_892X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
LOCAL_CFLAGS += -DTCC_8925S_INCLUDE
endif
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_CFLAGS += -DTCC_893X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
LOCAL_CFLAGS += -DTCC_8935S_INCLUDE
endif
endif

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
 	$(PV_INCLUDES) \
	$(LOCAL_PATH)/cdk \
	$(LOCAL_PATH)/container \

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libcdmx

LOCAL_STATIC_LIBRARIES := \
					   
LOCAL_SHARED_LIBRARIES += \
	libutils \
	liblog \
	libdl \
	libcutils \
	libHDCP2api

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/container/audio \
	-lTCCxxxx_AACDMX_ANDROID_V1.07.02 \
	-lTCCxxxx_FLACDMX_ANDROID_V1.05.03 \
	-lTCC892x_APEDMX_ANDROID_V1.00.01 \
	-lTCCxxxx_MP3DMX_ANDROID_V1.00.03
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/container/audio \
	-lTCCxxxx_AACDMX_ANDROID_V1.07.02 \
	-lTCCxxxx_FLACDMX_ANDROID_V1.05.03 \
	-lTCC892x_APEDMX_ANDROID_V1.00.01 \
	-lTCCxxxx_MP3DMX_ANDROID_V1.00.03
endif

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \

include $(BUILD_SHARED_LIBRARY)
include $(CLEAR_VARS)

#############################################################################

include $(CLEAR_VARS)
include $(call first-makefiles-under,$(LOCAL_PATH))
