#
# Copyright (C) 2008 Telechips, Inc.
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

OMX_TOP := $(LOCAL_PATH)

TCC_OMX_INCLUDES := \
	$(LOCAL_PATH)/omx_core/include

CDK_DIR := $(LOCAL_PATH)/../common/CDK

ifeq ($(USE_MEDIASERVER_ALT),true)
OMX_INCLUDE := \
	vendor/tv/frameworks/av/include/media/openmax_1.2 \
	$(OMX_TOP)/omx_include_1.2
else
OMX_INCLUDE := $(OMX_TOP)/omx_include
endif

include $(call first-makefiles-under,$(LOCAL_PATH))
