LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libdxbservice \
	libsqlite

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(TCCDXB_TOP)/middleware/lib_tdmb/include \
	$(TCCDXB_TOP)/middleware/lib_tdmb/include/player \
	$(TCCDXB_TOP)/service/libdxbservice/include \
	external/sqlite/dist

LOCAL_CFLAGS := \
	$(DXB_CFLAGS)
	
LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_SRC_FILES := \
	tpegmain.cpp

LOCAL_MODULE := tdmbtpeg
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_EXECUTABLE)
