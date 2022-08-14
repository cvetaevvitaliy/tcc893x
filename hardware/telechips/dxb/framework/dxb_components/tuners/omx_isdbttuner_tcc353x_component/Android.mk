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

LOCAL_CFLAGS    :=



#========= TCC353X Driver Build Guide =============
#=                                                =
#=                  TCC353X                       =
#=                                                =
#==================================================


#==================================================
# 1. Select TCC353X chip (TCC3530/TCC3531/TCC3532)
#==================================================

#LOCAL_CFLAGS    += -D_USE_TCC3530_
LOCAL_CFLAGS    += -D_USE_TCC3531_
#LOCAL_CFLAGS    += -D_USE_TCC3532_

#==================================================
# 2. Select Target Standard (1seg/fullseg(default))
#==================================================

#LOCAL_CFLAGS    += -D_USE_ONLY_1_SEGMENT_ 

#==================================================
# 3. Select Diversity Type (single/2~4Diversity)
#==================================================

LOCAL_CFLAGS    += -D_USE_TCC353X_SINGLE_  
#LOCAL_CFLAGS    += -D_USE_TCC353X_2DIVERSITY_
#LOCAL_CFLAGS    += -D_USE_TCC353X_3DIVERSITY_
#LOCAL_CFLAGS    += -D_USE_TCC353X_4DIVERSITY_

#==================================================
# 4. Select TUNER Type (DUAL BAND(default)/TRIPLE BAND)
#==================================================

#LOCAL_CFLAGS    += -D_USE_TCC353X_TRIPLE_BAND_


#==================================================
# 5. Select Tcc353x Power control type (Default _DXB_PWRCTRL_1_3_)
#
# [_DXB_PWRCTRL_1_3_]
#   DxbPower Control 0 : Master
#   DxbPower Control 1 : Slave1, Slave2, Slave3
#
# [_DXB_PWRCTRL_4_]
#   DxbPower Control 0 : Master, Slave1, Slave2, Slave3
#   DxbPower Control 1 : None
#
# [_DXB_PWRCTRL_2_2_]
#   DxbPower Control 0 : Master, Slave1
#   DxbPower Control 1 : Slave2, Slave3
#
#==================================================

LOCAL_CFLAGS    += -D_DXB_PWRCTRL_1_3_
#LOCAL_CFLAGS    += -D_DXB_PWRCTRL_4_
#LOCAL_CFLAGS    += -D_DXB_PWRCTRL_2_2_

#==================================================
# 6. Select Tcc353x Debug Log
#
#   DBG_MONITOR : enable monitoring log
#   USE_MONITORING_LOG : enable monitoring debug log
#==================================================

#LOCAL_CFLAGS    += -DDBG_MONITOR
#LOCAL_CFLAGS    += -DUSE_MONITORING_LOG

#====== End of TCC353X Driver Build Guide =========
#=                                                =
#=                  TCC353X                       =
#=                                                =
#==================================================

LOCAL_CFLAGS    += -v

#LOCAL_LDFLAGS := \
#	-L$(LOCAL_PATH)/tcc353xdriver/driver -lTcc353xDriver

LOCAL_SRC_FILES := \
	tcc353xdriver/api/src/tcc353x_api.c \
	tcc353xdriver/Linux_Adapt/tcc353x_linux_i2c.c \
	tcc353xdriver/Linux_Adapt/tcc353x_linux_tccspi.c \
	tcc353xdriver/PAL/tcc353x_pal_i2c.c \
	tcc353xdriver/PAL/tcc353x_pal_tccspi.c \
	tcc353xdriver/PAL/tcpal_android.c \
	tcc353xdriver/sample/main/tcc353x_main.c \
	tcc353xdriver/sample/monitoring/tcc353x_monitoring.c \
	tcc353xdriver/sample/monitoring/tcc353x_monitoring_calculate.c \
	src/isdbt_tcc353x_tuner.c \
	src/isdbt_tuner_space.c \
	src/omx_isdbttuner_tcc353x_component.c\
	tcc353xdriver/driver/src/tcc353x_command_control.c\
	tcc353xdriver/driver/src/tcc353x_core.c\
	tcc353xdriver/driver/src/tcc353x_dpll_19200osc.c\
	tcc353xdriver/driver/src/tcc353x_dpll_38400osc.c\
	tcc353xdriver/driver/src/tcc353x_dpll_tcc3535.c\
	tcc353xdriver/driver/src/tcc353x_dpll_tcc3536.c\
	tcc353xdriver/driver/src/tcc353x_isdb.c\
	tcc353xdriver/driver/src/tcc353x_mailbox.c\
	tcc353xdriver/driver/src/tcc353x_register_control.c\
	tcc353xdriver/driver/src/tcc353x_rf.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/tcc353xdriver/common \
	$(LOCAL_PATH)/tcc353xdriver/Linux_Adapt \
	$(LOCAL_PATH)/tcc353xdriver/PAL \
	$(LOCAL_PATH)/tcc353xdriver/sample/main \
	$(LOCAL_PATH)/tcc353xdriver/sample/monitoring \
	$(LOCAL_PATH)/tcc353xdriver/api/inc \
	$(LOCAL_PATH)/tcc353xdriver/driver/inc \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
	$(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/middleware/lib_isdbt/include

LOCAL_MODULE := libOMX.TCC.tcc353x.Tuner
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
