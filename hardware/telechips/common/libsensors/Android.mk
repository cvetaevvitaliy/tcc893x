# Copyright (C) 2009 Telechips, Inc.
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

#ifeq ($(BOARD_TCC_SENSOR), true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

# HAL module implemenation, not prelinked and stored in
# hw/<BACKLIGHT_HARDWARE_MODULE_ID>.<ro.product.board>.so
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include 

LOCAL_CFLAGS := -fno-short-enums \
        -DLOG_TAG=\"Sensors\"

LOCAL_SRC_FILES := 						\
				sensors.c 				\
				nusensors.cpp 			\
				InputEventReader.cpp	\
				SensorBase.cpp			\
				AkmSensor.cpp	\
				AccelSensor.cpp
				
LOCAL_SRC_FILES += 						\
				LightSensor.cpp			\
				ProximitySensor.cpp
				
LOCAL_MODULE_TAGS := optional
				
LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)

include $(BUILD_SHARED_LIBRARY)

#endif
