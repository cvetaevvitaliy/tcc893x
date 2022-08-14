LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false


LOCAL_SHARED_LIBRARIES := \
        libc \
        libutils \
        libcutils \
        liblog \
        libOMX.TCC.DxB.base

LOCAL_LDLIBS += \
        -lpthread \
        -ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	$(DXB_CFLAGS)

LOCAL_SRC_FILES := \
	src/omx_linuxtv_tuner_component.c \
	src/LinuxTV_Frontend.c
	
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(TCCDXB_TOP)/../../../ \
	kernel/include/linux/dvb

LOCAL_MODULE := libOMX.TCC.LinuxTV.Tuner
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
