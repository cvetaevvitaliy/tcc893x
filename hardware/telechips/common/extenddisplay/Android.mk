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
	$(TOP)/kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	$(LOCAL_PATH)/libs
	
LOCAL_CFLAGS := -DTELECHIPS \
			$(TARGET_BOOTLOADER_BOARD_CFLAGS) \
			$(BOARD_CFLAGS) \
			$(BOARD_REV_FLAGS) \
			$(BOARD_HDMI_UI_SIZE_FLAGS) \
			$(BOARD_OVERLAY_EXT_FLAGS) \
			$(BOARD_DUAL_HDMI_DISPLAY_FLAGS) \
			$(BOARD_HDMI_720P_FIXED_FLAGS) \
			$(BOARD_TV_COMPOSITE) \
			$(BOARD_TV_COMPONENT) \
			$(BOARD_COMPONENT_CHIP_FLAGS) \
			$(BOARD_DISPLAY_OUTPUT_FLAGS) \
			$(BOARD_HDMI_CEC) \
			$(BOARD_HDMI_HDCP) \
			$(BOARD_HDMI_CLK)


LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libbinder \
	libui \
	libhardware \
	libandroid_runtime \
	liblog \
	libmedia

ifeq ($(BOARD_HAVE_TV_ANALOG_OUTPUT),true)
LOCAL_CFLAGS += -DHAVE_TV_ANALOG_OUTPUT

LOCAL_SRC_FILES += \
	composite.cpp \
	component.cpp
endif

ifeq ($(strip $(BOARD_HAVE_HDMI_OUTPUT)),true)
LOCAL_CFLAGS += -DHAVE_HDMI_OUTPUT

LOCAL_SRC_FILES += \
	hdmi.cpp \
	extenddisplay.cpp

LOCAL_SHARED_LIBRARIES += \
	libpmap \
	libhdmi

ifeq ($(BOARD_HDMI_VERSION), HDMI_V1_3)
LOCAL_CFLAGS += -DHDMI_V1_3
endif
ifeq ($(BOARD_HDMI_VERSION), HDMI_V1_4)
LOCAL_CFLAGS += -DHDMI_V1_4
endif

endif

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif
ifeq ($(BOARD_VIDEO_DEINTERLACE_SUPPORT_FLAG), true)
LOCAL_CFLAGS += -DTCC_VIDEO_DEINTERLACE_SUPPORT
endif

LOCAL_MODULE := libextenddisplay
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(call first-makefiles-under,$(LOCAL_PATH))
