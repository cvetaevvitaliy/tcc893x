LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false


LOCAL_SHARED_LIBRARIES := \
        libc \
        libutils \
        libcutils \
        liblog \
        libmedia \
        libdl \
        libhardware_legacy \
        libspdifsound \
        libOMX.TCC.DxB.base
#        libasound \

LOCAL_STATIC_LIBRARIES += \
#        libaudiointerface

LOCAL_LDLIBS += \
        -lpthread \
        -ldl

LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib -lSimpleBuffer

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -D_POSIX_SOURCE
#LOCAL_CFLAGS := \
        $(TCC_OMX_FLAGS)


LOCAL_SRC_FILES := \
	src/omx_alsasink_component.cpp

LOCAL_C_INCLUDES := \
    $(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
    $(LOCAL_PATH)/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	external/alsa-lib/include

LOCAL_MODULE := libOMX.TCC.alsasink
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)

