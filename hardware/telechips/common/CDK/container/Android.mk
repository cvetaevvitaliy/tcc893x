LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := \
	cdmx.c

LOCAL_MODULE := libcdmx
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -D__ANDROID__

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_CFLAGS += -DTCC_892X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
LOCAL_CFLAGS += -DTCC_8925S_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
else
LOCAL_CFLAGS += -DTCC_VPU_C7_INCLUDE
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
LOCAL_CFLAGS += -DTCC_893X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
LOCAL_CFLAGS += -DTCC_8935S_INCLUDE
LOCAL_CFLAGS += -DTCC_VPU_C5_INCLUDE
else
LOCAL_CFLAGS += -DTCC_VPU_C7_INCLUDE
endif
endif


ifeq ($(BOARD_DIVXDRM_FLAGS),true)
LOCAL_CFLAGS += \
	-DDIVX_DRM5
endif

LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/cdk \
	$(LOCAL_PATH)/container \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libcdmx_mkv \
	libcdmx_mp4 \
	libcdmx_avi \
	libcdmx_mpg \
	libcdmx_ts \
	libcdmx_asf \
	libcdmx_audio \
	libcdmx_ogg \
	libcdmx_flv \
	libcdmx_ext

include $(BUILD_STATIC_LIBRARY)

#############################################################################

include $(CLEAR_VARS)
include $(call first-makefiles-under,$(LOCAL_PATH))
