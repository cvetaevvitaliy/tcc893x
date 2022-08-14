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

##############################################################
# CFLAGS OPTIONS
#
# [TDMB Only]
# 1. disable _SUPPORT_LBAND_
#
# [TDMB, DAB]
# 1. enable _SUPPORT_LBAND_
#
# [Select Xtal]
# use 19.2Mhz 	: enable _OSC_19200kHz_
# use 24.576Mhz	: enable _OSC_24576kHz_
# use 38.4Mhz 	: enable _OSC_38400kHz_
#
# [Select Dual board mode]
# 1. use chain mode : enable _DUAL_CHAIN_MODE_
# 2. others : disable _DUAL_CHAIN_MODE_
#
# [Signal State]
# Signal State Debug Enable: _SIGNAL_STATE_
# Transmission Mode info Enable: _STATE_GET_TRANSMISSIONMODE_
#
# [LNA]
# LG LNA CUSTOM1: _RSSI_EXT_LNA_CUSTOM1_
#
# [Multi Channel]
# Multi Channel: _MULTI_SERVICE_
##############################################################

LOCAL_CFLAGS    :=
LOCAL_CFLAGS    += -v
#LOCAL_CFLAGS    += -D_SUPPORT_LBAND_

LOCAL_CFLAGS	+= -D_OSC_19200kHz_
#LOCAL_CFLAGS	+= -D_OSC_24576kHz_
#LOCAL_CFLAGS	+= -D_OSC_38400kHz_

LOCAL_CFLAGS	+= -D_DUAL_CHAIN_MODE_

#LOCAL_CFLAGS	+= -D_SIGNAL_STATE_
#LOCAL_CFLAGS	+= -D_STATE_GET_TRANSMISSIONMODE_
#LOCAL_CFLAGS	+= -D_RSSI_EXT_LNA_CUSTOM1_

LOCAL_CFLAGS	+= -D_MULTI_SERVICE_

LOCAL_LDFLAGS := \
	-L$(LOCAL_PATH)/tcc317xdriver/lib -lFICParser_2_0_12_63 -lSimpleBuffer

LOCAL_SRC_FILES := \
	tcc317xdriver/api/src/tcc317x_api.c \
	tcc317xdriver/PAL/tcc317x_pal_i2c.c \
	tcc317xdriver/PAL/tcc317x_pal_tccspi.c \
	tcc317xdriver/PAL/tcpal_android.c \
	tcc317xdriver/sample/main/tcc317x_main.c \
	tcc317xdriver/sample/monitoring/tcc317x_monitoring.c \
	tcc317xdriver/sample/monitoring/tcc317x_monitoring_calculate.c \
	tcc317xdriver/Linux_Adapt/tcc317x_linux_i2c.c \
	tcc317xdriver/Linux_Adapt/tcc317x_linux_tccspi.c \
	src/omx_tcc317x_tuner_component.c \
	src/tdmb_tuner_space.c \
	src/tdmb_tcc317x_tuner.c \
	src/FICParser/TDMBInfo.c \
	src/FICParser/DMBManage.c \
	src/tcc_tsif.c \
	tcc317xdriver/driver/src/tcc317x_command_control.c \
	tcc317xdriver/driver/src/tcc317x_core.c \
	tcc317xdriver/driver/src/tcc317x_tdmb.c \
	tcc317xdriver/driver/src/tcc317x_mailbox.c \
	tcc317xdriver/driver/src/tcc317x_register_control.c \
	tcc317xdriver/driver/src/tcc317x_rf.c \
	tcc317xdriver/sample/streamProcess/tcc317x_stream_process.c 

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/FICParser \
	$(LOCAL_PATH)/tcc317xdriver/common \
	$(LOCAL_PATH)/tcc317xdriver/Linux_Adapt \
	$(LOCAL_PATH)/tcc317xdriver/PAL \
	$(LOCAL_PATH)/tcc317xdriver/sample/main \
	$(LOCAL_PATH)/tcc317xdriver/sample/monitoring \
	$(LOCAL_PATH)/tcc317xdriver/sample/streamProcess \
	$(LOCAL_PATH)/tcc317xdriver/api/inc \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/middleware/lib_tdmb/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(TCCDXB_TOP)/../../../ \
	kernel/include/linux/dvb \
	$(LOCAL_PATH)/tcc317xdriver/common \
	$(LOCAL_PATH)/tcc317xdriver/driver \
	$(LOCAL_PATH)/tcc317xdriver/driver/inc \
	$(LOCAL_PATH)/tcc317xdriver/PAL \
	$(LOCAL_PATH)/tcc317xdriver/sample/monitoring

LOCAL_MODULE := libOMX.TCC.tcc317x.Tuner
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)

