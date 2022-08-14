LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_COPY_HEADERS := \
		tcc_resizer.h \
		TCC_GS_DECODER.h \
        tcc_InterSubtitle.h

include $(BUILD_COPY_HEADERS)
