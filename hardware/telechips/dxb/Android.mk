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

ifneq ($(filter tcc%,$(TARGET_BOARD_PLATFORM)),)

LOCAL_PATH := $(call my-dir)
TCCDXB_TOP := $(call my-dir)
TCCDXB_OMX_TOP := $(LOCAL_PATH)/framework/dxb_components

include $(CLEAR_VARS)

#$(info PLATFORM_VERSION=$(PLATFORM_VERSION))

SUPPORT_RINGBUFFER := true

DXB_CFLAGS += \
	$(TARGET_BOOTLOADER_BOARD_CFLAGS) $(BOARD_HDMI_UI_SIZE_FLAGS)

DXB_C_INCLUDES += \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include/mach

ifneq ($(filter 2.3%,$(PLATFORM_VERSION)),)
DXB_SHARED_LIBRARIES += \
	libsurfaceflinger_client \
	libpmap
DXB_CFLAGS += -DGINGERBREAD_PLATFORM
endif
ifneq ($(filter 4.0%,$(PLATFORM_VERSION)),)
DXB_SHARED_LIBRARIES += \
	libsurfaceflinger_client \
	libpmap
DXB_CFLAGS += -DICS_PLATFORM
endif
ifneq ($(filter 4.1% 4.2%,$(PLATFORM_VERSION)),)
DXB_SHARED_LIBRARIES += \
	libsurfaceflinger \
	libpmap
DXB_CFLAGS += -DJB_PLATFORM
endif
ifneq ($(filter 4.4%,$(PLATFORM_VERSION)),)
DXB_SHARED_LIBRARIES += \
	libsurfaceflinger \
	libpmap
DXB_CFLAGS += -DKITKAT_PLATFORM
endif

# platform configuration
ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
DXB_CFLAGS += -DTCC_892X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
DXB_CFLAGS += -DTCC_8925S_INCLUDE
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
DXB_CFLAGS += -DTCC_893X_INCLUDE
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
DXB_CFLAGS += -DTCC_8935S_INCLUDE
endif
endif

# ringbuffer configuration
ifeq ($(SUPPORT_RINGBUFFER),true)
ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
ifeq ($(TARGET_BOARD_SOC),tcc892xS)
SUPPORT_RINGBUFFER:=false
else
DXB_CFLAGS += -DRING_BUFFER_MODULE_ENABLE
endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
ifeq ($(TARGET_BOARD_SOC),tcc893xS)
SUPPORT_RINGBUFFER:=false
else
DXB_CFLAGS += -DRING_BUFFER_MODULE_ENABLE
endif
endif
endif

include $(call first-makefiles-under,$(LOCAL_PATH))

endif
