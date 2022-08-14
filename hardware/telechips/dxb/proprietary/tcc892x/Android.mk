#
# Copyright (C) 2011 Telechips, Inc.
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

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libTeletext_v2.a
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libisdbt_subtitle.a
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.DxB.base.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.bsacdec.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.ATSCTuner.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.LinuxTV.Demux.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.LinuxTV.PVR.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.IPTVDemux.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.tdmbdemux.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.PADParser.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libOMX.TCC.tdmbrec.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libTCC_CDK_DXB_LIB.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libTCCDxBInterface.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libdxbsc.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libdxbcipher.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libdvbt.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libtdmb.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libisdbt.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_LIBS := libatsc.so
LOCAL_MODULE_TAGS := eng debug
include $(BUILD_MULTI_PREBUILT)

endif
