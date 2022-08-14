LOCAL_PATH:= $(call my-dir)

#
# libSPDIFHardwareService
#

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -D_POSIX_SOURCE

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES :=           \
	SPDIFHardwareStub.cpp    
	

LOCAL_SHARED_LIBRARIES := \
	libtinyalsa \
	libutils  \
	libcutils \
	libbinder \
	libc      


LOCAL_C_INCLUDES := \
	$(TOP)/kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	external/tinyalsa/include

ifeq ($(BOARD_HDMI_VERSION), HDMI_V1_3)
LOCAL_CFLAGS += -DHDMI_V1_3
endif

ifeq ($(BOARD_HDMI_VERSION), HDMI_V1_4)
LOCAL_CFLAGS += -DHDMI_V1_4
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libspdifsound

include $(BUILD_SHARED_LIBRARY)
