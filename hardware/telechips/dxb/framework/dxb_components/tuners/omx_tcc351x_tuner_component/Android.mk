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

LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/lib -lTCC351XCore_0_11_25 -lFICParser_2_0_12_63 -lSimpleBuffer

LOCAL_CFLAGS := \
	$(DXB_CFLAGS)

LOCAL_SRC_FILES := \
	src/omx_tcc351x_tuner_component.c \
	src/SPI.c \
	src/TCC351X/TC3X_API.c \
	src/TCC351X/TC3X_Sub_API.c \
	src/TCC351X/IO/TC3X_IO.c \
	src/TCC351X/IO/TC3X_IO_CSPI.c \
	src/TCC351X/IO/TC3X_IO_HPI.c \
	src/TCC351X/IO/TC3X_IO_I2C.c \
	src/TCC351X/IO/TC3X_IO_SDIO.c \
	src/TCC351X/IO/TC3X_IO_SRAMLIKE.c \
	src/TCC351X/IO/TC3X_IO_UTIL.c \
	src/TCC351X_antenna.c \
	src/TCC351X_dvbt.c \
	src/TCC351X_isdbt.c \
	src/TCC351X_tdmb.c \
	src/TCC351X_ISDBTSpace.c \
	src/TCC351X_TDMBSpace.c \
	src/FICParser/TDMBInfo.c \
	src/FICParser/DMBManage.c \
	src/tcc_tsif.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/FICParser \
	$(LOCAL_PATH)/include/TCC351X \
	$(LOCAL_PATH)/include/TCC351X/IO \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(TCCDXB_TOP)/../../../ \
	kernel/include/linux/dvb

LOCAL_MODULE := libOMX.TCC.TCC351X.Tuner
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
