LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS := \
        Display/include/tcc_display.h \
        Parser/include/tcc_subtitle_interface.h

include $(BUILD_COPY_HEADERS)
