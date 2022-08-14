#ifdef _DO_NOT_BUILD_
LOCAL_PATH := $(call my-dir)

ifeq ($(BOARD_GPS_LIBRARIES), libgps)

ifneq ($(TARGET_PRODUCT),sim)
# HAL module implemenation, not prelinked and stored in
# # hw/<GPS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_CFLAGS += -DHAVE_GPS_HARDWARE
LOCAL_SHARED_LIBRARIES := liblog libcutils libhardware libc libutils
LOCAL_SRC_FILES := gps_tcc.c
LOCAL_MODULE := gps.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
endif

endif
#endif
