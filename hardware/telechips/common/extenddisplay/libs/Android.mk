#
# Copyright (C) 2008, 2009 Telechips, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_C_INCLUDES := \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include 

LOCAL_CFLAGS := -DTELECHIPS \
		$(BOARD_REV_FLAGS) \
		$(BOARD_HDMI_UI_SIZE_FLAGS) \
		$(BOARD_DISPLAY_OUTPUT_FLAGS) \
		$(TARGET_BOOTLOADER_BOARD_CFLAGS) \
		$(BOARD_HDMI_CLK) \

LOCAL_SRC_FILES := \
	libcec/libcec.c \
	libddc/libddc.c \
	libedid/libedid.c \
	libedid/edidparse.c \
	libhpd/libhpd.c \
	libiic/libiic.c \
	libsmp/hdmi_sha1.c \
	libsmp/smp_add.c \
	libsmp/smp_div.c \
	libsmp/smp_err.c \
	libsmp/smp_exp.c \
	libsmp/smp_mod.c \
	libsmp/smp_mont.c \
	libsmp/smp_mul.c \
	libsmp/smp_shift.c \
	libsmp/smp_sqr.c \
	libsmp/smp_util.c \
	libsmp/arm/smp_asm.S \
	libsrm/libsrm.c

ifeq ($(BOARD_HDMI_VERSION), HDMI_V1_3)
LOCAL_CFLAGS += -DHDMI_V1_3
LOCAL_SRC_FILES += \
	libhdmi/libhdmi_v1_3.c \
	libphy/libphy_v1_3.c
endif

ifeq ($(BOARD_HDMI_VERSION), HDMI_V1_4)
LOCAL_CFLAGS += -DHDMI_V1_4
LOCAL_SRC_FILES += \
	libhdmi/libhdmi_v1_4.c \
	libphy/libphy_v1_4.c
endif
LOCAL_SHARED_LIBRARIES := \
	libutils \
	libui \
	liblog

LOCAL_MODULE := libhdmi

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
