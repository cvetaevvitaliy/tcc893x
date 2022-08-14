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
			$(BOARD_REV_FLAGS) 

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libcutils \
	libbinder \
	libui \
	libhardware \
	libandroid_runtime \
	liblog

LOCAL_SRC_FILES += \
	display.cpp


LOCAL_MODULE := libdisplay
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
