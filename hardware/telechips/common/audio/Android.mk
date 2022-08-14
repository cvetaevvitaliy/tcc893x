# Copyright (C) 2012 The Android Open Source Project
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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SRC_FILES := \
	audio_hw.c \
	audio_route.c \
        usb_audio.c
LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	external/expat/lib \
	$(call include-path-for, audio-utils)
LOCAL_SHARED_LIBRARIES := liblog libcutils libtinyalsa libaudioutils libexpat
LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_BOARD_AUDIO_CODEC),es8388)
LOCAL_CFLAGS += -DTCC_AUDIO_ES8388
endif

ifeq ($(TARGET_BOARD_AUDIO_CODEC),wm8731)
LOCAL_CFLAGS += -DTCC_AUDIO_WM8731
endif

ifeq ($(TARGET_BOARD_AUDIO_CODEC),wm8524)
LOCAL_CFLAGS += -DTCC_AUDIO_WM8524
endif

ifeq ($(TARGET_BOARD_AUDIO_CODEC),wm8988)
LOCAL_CFLAGS += -DTCC_AUDIO_WM8988
endif

ifeq ($(TARGET_BOARD_AUDIO_CODEC),rt5633)
LOCAL_CFLAGS += -DTCC_AUDIO_RT5633
endif

ifeq ($(TARGET_BOARD_AUDIO_CODEC),rt5631)
LOCAL_CFLAGS += -DTCC_AUDIO_RT5631
endif

ifeq ($(TARGET_BOARD_AUDIO_CODEC),rt5625)
LOCAL_CFLAGS += -DTCC_AUDIO_RT5625
endif

include $(BUILD_SHARED_LIBRARY)


#include $(CLEAR_VARS)

#LOCAL_MODULE:= usb_audio
#LOCAL_SHARED_LIBRARIES := liblog libcutils
#LOCAL_C_INCLUDES += \
#	external/tinyalsa/include \

#LOCAL_SRC_FILES:= usb_audio.c

#LOCAL_MODULE_TAGS := optional

#include $(BUILD_EXECUTABLE)
