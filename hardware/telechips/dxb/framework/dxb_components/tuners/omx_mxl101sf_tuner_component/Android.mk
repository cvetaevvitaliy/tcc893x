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
	src/omx_mxl101sf_tuner_component.c \
	src/MxL101SF_OEM_Drv.cpp \
	src/MxL101SF_PhyCfg.cpp \
	src/MxL101SF_PhyCtrlApi.cpp \
	src/MxL101SF_dvbt.cpp
	
	
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(TCCDXB_TOP)/../../../ \
	kernel/include/linux/dvb

LOCAL_MODULE := libOMX.TCC.MxL101SF.Tuner
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
