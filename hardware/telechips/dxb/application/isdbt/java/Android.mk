#
# Copyright (C) 2013 Telechips, Inc.
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

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := ISDBTPlayer

LOCAL_JNI_SHARED_LIBRARIES := \
	libOMX.TCC.MN88472.Tuner \
	libOMX.TCC.MxL101SF.Tuner \
	libOMX.TCC.tcc353x.Tuner \
	libOMX.TCC.TCC351X.Tuner \
	libOMX.TCC.Tuner.interface \
	libOMX.TCC.tdmbrec \
	libOMX.TCC.PADParser \
	libOMX.TCC.tdmbdemux \
	libOMX.TCC.IPTVDemux \
	libOMX.TCC.LinuxTV.PVR \
	libOMX.TCC.LinuxTV.Demux \
	libOMX.TCC.alsasink \
	libTCC_CDK_DXB_LIB \
	libOMX.TCC.DxB.AudioDec \
	libOMX.TCC.bsacdec \
	libOMX.TCC.DxB.VPUDec \
	libOMX.TCC.DxB.VideoDec \
	libOMX.TCC.fbdevsink \
	libTCCDxBInterface \
	libisdbt_region \
	libdxbutils \
	libisdbt \
	libisdbt_jni

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
