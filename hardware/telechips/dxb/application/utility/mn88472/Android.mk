LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	liblog

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/mn88472_sample\
	$(LOCAL_PATH)/common \
	$(LOCAL_PATH)/mn88472 \
	$(LOCAL_PATH)/tuner_template

LOCAL_CFLAGS :=

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_SRC_FILES := \
	mn88472_sample/MN88472_sample.c \
	mn88472_sample/MN88472_UserFunction.c \
	common/MN_DMD_common.c \
	common/MN_DMD_console.c \
	common/MN_DMD_driver.c \
	mn88472/MN88472_autoctrl.c \
	mn88472/MN88472_register.c \
	mn88472/MN_DMD_device.c \
	mn88472/MN_TCB.c \
	i2c_template/MN_I2C.c \
	tuner_template/MN_Tuner.c \
	tuner_template/MxL603_Main.c \
	tuner_template/MxL603_OEM_Drv.c\
	tuner_template/MxL603_TunerApi.c \
	tuner_template/MxL603_TunerCfg.c


LOCAL_CFLAGS    += -g -O0
LOCAL_MODULE := mn88472test
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_EXECUTABLE)
include $(call first-makefiles-under,$(LOCAL_PATH))
