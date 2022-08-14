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
	src/omx_mn88472_tuner_component.c \
	src/MN88472_dvbt.cpp \
	src/common/MN_DMD_common.c \
	src/common/MN_DMD_driver.c \
	src/mn88472/MN88472_autoctrl.c \
	src/mn88472/MN88472_register.c \
	src/mn88472/MN_DMD_device.c \
	src/i2c_template/MN_I2C.c \
	src/tuner_template/MN_Tuner.c \
	src/tuner_template/MxL603_Main.c \
	src/tuner_template/MxL603_OEM_Drv.c\
	src/tuner_template/MxL603_TunerApi.c \
	src/mn88472_sample/MN88472_UserFunction.c \
	src/tuner_template/MxL603_TunerCfg.c
	
	
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/src/common \
	$(LOCAL_PATH)/src/mn88472 \
	$(LOCAL_PATH)/src/tuner_template \
	$(LOCAL_PATH)/src/mn88472_sample \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(TCCDXB_TOP)/../../../ \
	kernel/include/linux/dvb

LOCAL_MODULE := libOMX.TCC.MN88472.Tuner
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
