LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	libbinder \
	liblog \
	libmedia \
	libdl \
	libui \
	libandroid_runtime \
	libnativehelper \
	libhardware_legacy

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	-D_POSIX_SOURCE \
	-DTCC_89XX_INCLUDE \
	$(DXB_CFLAGS)

ifeq ($(TARGET_BOARD_PLATFORM),tcc93xx)
	LOCAL_CFLAGS += -D$(TARGET_BOARD_STYLE)
endif	

LOCAL_LDFLAGS := 

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(DXB_C_INCLUDES)


LOCAL_SRC_FILES := \
	src/ISDBT_Region.c

LOCAL_SHARED_LIBRARIES += $(DXB_SHARED_LIBRARIES)

LOCAL_MODULE := libisdbt_region
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
include $(call first-makefiles-under,$(LOCAL_PATH))
