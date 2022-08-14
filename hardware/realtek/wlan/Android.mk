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
#
LOCAL_PATH := $(call my-dir)

ifeq ($(BOARD_USES_REALTEK_WIFI),true)

include $(CLEAR_VARS)
LOCAL_MODULE := dhcpcd.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/dhcpcd
LOCAL_SRC_FILES := android_dhcpcd.conf
include $(BUILD_PREBUILT)

WIFI_DRIVER_SOCKET_IFACE := wlan0
ifeq ($(strip $(WPA_SUPPLICANT_VERSION)),VER_0_8_X)
  include external/wpa_supplicant_8/wpa_supplicant/wpa_supplicant_conf.mk
else
ifeq ($(strip $(WPA_SUPPLICANT_VERSION)),VER_0_6_X)
  include external/wpa_supplicant_6/wpa_supplicant/wpa_supplicant_conf.mk
else
  include external/wpa_supplicant/wpa_supplicant_conf.mk
endif
endif

ifneq ($(filter $(REALTEK_MODULE_NAME),rt8188cu),)
    include $(CURDIR)/hardware/realtek/wlan/rt8188cu/Android.mk
endif
ifneq ($(filter $(REALTEK_MODULE_NAME),rt8188eus),)
    include $(CURDIR)/hardware/realtek/wlan/rt8188eus/Android.mk
endif
ifneq ($(filter $(REALTEK_MODULE_NAME),rt8189es),)
    include $(CURDIR)/hardware/realtek/wlan/rt8189es/Android.mk
endif
ifneq ($(filter $(REALTEK_MODULE_NAME),rt8192du),)
    include $(CURDIR)/hardware/realtek/wlan/rt8192du/Android.mk
endif
endif

