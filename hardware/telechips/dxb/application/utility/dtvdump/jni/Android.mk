#
# Copyright (C) 2012 Telechips, Inc.  All rights reserved.
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

LOCAL_SRC_FILES := com_telechips_android_dtvdump.cpp \
	dtvdump.c \
	linuxtv_utils.c \
	tcc_rtp.c \
	tcc_socket.c \
	tcc_socket_util.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(TCCDXB_TOP)/framework/dxb_components/omx_engine/dxb_omx_base/include \
	$(TCCDXB_TOP)/framework/dxb_components/omx_engine/dxb_omx_include \
	kernel/include/linux/dvb

LOCAL_SHARED_LIBRARIES := \
	libandroid_runtime \
	libcutils \
	libutils \
	liblog \
	libTCCDxBInterface \
	libOMX.TCC.tcc353x.Tuner \
	libOMX.TCC.LinuxTV.Tuner \
	libOMX.TCC.TCC351X.Tuner \
	libOMX.TCC.ATSCTuner \
	libOMX.TCC.MxL101SF.Tuner \
	libOMX.TCC.MN88472.Tuner \
	libOMX.TCC.DxB.base

LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_MODULE := libdtvdump

LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)
