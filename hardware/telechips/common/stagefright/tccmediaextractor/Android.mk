LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	liblog \
	libTCC_CDK_WRAPPER

LOCAL_SRC_FILES := \
	TCCMediaExtractor.cpp \
	cdk_wrapper_stagefright.cpp \
	media_io_wrapper_stagefright.cpp \
	albumart_extractor.cpp

LOCAL_MODULE := libTCC.MEDIA.extractor

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
LOCAL_CFLAGS += -DTCC_892X_INCLUDE
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

LOCAL_LDFLAGS += -S 

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
	$(TOP)/frameworks/av/media/libstagefright \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/frameworks/native/include/media/hardware \
	$(TOP)/frameworks/native/include \
	$(TCC_HARDWARE_COMMON_TOP)/CDK/cdk \
	$(TCC_HARDWARE_COMMON_TOP)/cdk_wrapper/main/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include

ifeq ($(USE_MEDIASERVER_ALT),true)
LOCAL_C_INCLUDES += \
	vendor/tv/frameworks/av/include

LOCAL_CFLAGS += -DOPENMAX1_2
endif

LOCAL_WHOLE_STATIC_LIBRARIES := \

include $(BUILD_STATIC_LIBRARY)
