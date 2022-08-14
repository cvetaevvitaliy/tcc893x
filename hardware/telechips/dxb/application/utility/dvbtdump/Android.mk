LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog \
	libOMX.TCC.LinuxTV.Tuner \
	libOMX.TCC.MxL101SF.Tuner \
	libOMX.TCC.MN88472.Tuner \
	libOMX.TCC.TCC351X.Tuner

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(TCCDXB_TOP)/framework/dxb_components/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/framework/dxb_components/omx_engine/dxb_omx_include

LOCAL_CFLAGS :=

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_SRC_FILES := \
	dumpmain.c \
	tcc_tsif.c

LOCAL_CFLAGS    += -g -O0
LOCAL_MODULE := dvbtdump
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_EXECUTABLE)
