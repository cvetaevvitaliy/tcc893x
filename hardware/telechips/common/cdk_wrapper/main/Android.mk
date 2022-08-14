LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false 

LOCAL_SRC_FILES := \
	src/tcc_cdk_wrapper.cpp \
	src/stream_queue.cpp \
	src/MessageQueue.cpp \
	src/tcc_metadata_parser.cpp \

LOCAL_MODULE := libTCC_CDK_WRAPPER
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := $(TARGET_BOOTLOADER_BOARD_CFLAGS) \

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

ifeq ($(BOARD_DIVXDRM_FLAGS),true)
LOCAL_CFLAGS += \
	-DDIVX_DRM5
endif
								 
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := \
	frameworks/base/include \
	$(LOCAL_PATH)/include \
	$(TCC_HARDWARE_COMMON_TOP)/CDK/cdk \
	$(TCC_HARDWARE_COMMON_TOP)/CDK/container \
	$(TCC_HARDWARE_COMMON_TOP)/libtccsubtitle/IntSubtitle \

LOCAL_SHARED_LIBRARIES += \
	libutils \
	libcutils \
	libdl \
	liblog \
	libTCC_CDK_LIB \
	libTCCInterSubtitle \
	libstagefright_foundation \
	libHDCP2api

LOCAL_LDFLAGS := \

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \

include $(BUILD_SHARED_LIBRARY)

