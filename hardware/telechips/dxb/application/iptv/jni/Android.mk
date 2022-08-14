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
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	com_telechips_android_IPTVPlayer.cpp

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE) \
	$(TCCDXB_TOP)/middleware/lib_iptv/include \
	$(TCCDXB_TOP)/middleware/lib_iptv/include/player

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libnativehelper \
	libcutils \
	libutils \
	libgui \
	libiptv

LOCAL_CFLAGS := \
	$(DXB_CFLAGS)	

LOCAL_MODULE := libiptv_jni
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
