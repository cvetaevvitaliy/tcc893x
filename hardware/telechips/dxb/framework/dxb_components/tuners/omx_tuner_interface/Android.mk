LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false


LOCAL_SHARED_LIBRARIES := \
				libcutils \
				libutils \
				libbinder \
				liblog \
				libOMX.TCC.TCC351X.Tuner \
				libOMX.TCC.LinuxTV.Tuner \
				libOMX.TCC.tcc353x.Tuner \
				libOMX.TCC.tcc317x.Tuner \
				libOMX.TCC.MxL101SF.Tuner \
				libOMX.TCC.MN88472.Tuner \
				libOMX.TCC.LinuxTV.Tuner \
				libOMX.TCC.ATSCTuner 

LOCAL_LDLIBS += \
        -lpthread \
        -ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS :=	\
	$(DXB_CFLAGS)

LOCAL_CFLAGS    += -v

LOCAL_SRC_FILES := \
	src/omx_tuner_interface.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include\oem \
	$(LOCAL_PATH)/include\drivers \
	$(LOCAL_PATH)/oem \
	$(LOCAL_PATH)/drivers \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/middleware/lib_isdbt/include \
	$(LOCAL_PATH)/include

LOCAL_MODULE := libOMX.TCC.Tuner.interface
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
