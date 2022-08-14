#
# Copyright (C) 2012 Telechips, Inc.
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

PRODUCT_COPY_FILES := \
	device/telechips/tcc893x-common/bluetooth/key_921600.psr:system/key_921600.psr

PRODUCT_COPY_FILES += \
 vendor/telechips/proprietary/tcc893x/ba_svc:system/bin/ba_svc

ifeq ($(BOARD_ANTENNA_SHARING_BT_WIFI), true)
PRODUCT_COPY_FILES += \
	device/telechips/tcc893x-common/bluetooth/key_3000000coex.psr:system/key_3000000.psr
else
 ifeq ($(BOARD_BT_HOST_WAKE_UP), true)
 PRODUCT_COPY_FILES += \
	 device/telechips/tcc893x-common/bluetooth/key_3000000hwake.psr:system/key_3000000.psr
 else
 PRODUCT_COPY_FILES += \
	 device/telechips/tcc893x-common/bluetooth/key_3000000.psr:system/key_3000000.psr
 endif
endif

PRODUCT_COPY_FILES += \
	device/telechips/tcc893x-common/bluetooth/bcm4330.hcd:system/vendor/firmware/bcm4330.hcd

PRODUCT_COPY_FILES += \
       device/telechips/tcc893x-common/bluetooth/BCM4334B0.hcd:system/vendor/firmware/bcm4334.hcd

PRODUCT_COPY_FILES += \
       device/telechips/tcc893x-common/bluetooth/BCM4335B0.hcd:system/vendor/firmware/bcm4335.hcd

PRODUCT_COPY_FILES += \
       device/telechips/tcc893x-common/bluetooth/bcm43341b0.hcd:system/vendor/firmware/bcm43341.hcd

#for bluetooth
PRODUCT_PACKAGES += \
	avinfo \
	hciconfig \
	hcitool \
	l2ping \
	bccmd \
	bcmtool \
	rfcomm \
	bt_vendor.conf

PRODUCT_PACKAGES += \
	audio.a2dp.default \
	audio.usb.default

# Bluetooth config file
#PRODUCT_COPY_FILES += \
    system/bluetooth/data/main.nonsmartphone.conf:system/etc/bluetooth/main.conf \

# audio mixer paths
#PRODUCT_COPY_FILES += \
#    device/asus/grouper/mixer_paths.xml:system/etc/mixer_paths.xml

# audio policy configuration
PRODUCT_COPY_FILES += \
    device/telechips/tcc893x-common/audio_policy.conf:system/etc/audio_policy.conf

# Prebuilted NTFS driver modules
#
PRODUCT_COPY_FILES += \
	device/telechips/tcc893x-common/ufsd_$(TARGET_BOARD_SOC).ko:root/lib/modules/ufsd.ko

# Prebuilt NTFS tools
#
PRODUCT_COPY_FILES += \
	device/telechips/tcc893x-common/mkntfs:system/bin/mkntfs \
	device/telechips/tcc893x-common/chkntfs:system/bin/chkntfs\
	device/telechips/tcc893x-common/chkntfs:system/bin/chkufsd

# VPU(Video Manager/Encoder/Decoder)
PRODUCT_COPY_FILES += \
	device/telechips/tcc893x-common/media_codecs_$(TARGET_BOARD_SOC).xml:system/etc/media_codecs.xml \
	$(LOCAL_PATH)/vpu_$(TARGET_BOARD_SOC)_core.ko:system/lib/modules/vpu_core.ko \
	$(LOCAL_PATH)/vpu_$(TARGET_BOARD_SOC)_device.ko:system/lib/modules/vpu_device.ko

# WDMA device driver for Wifi display 
#PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/tcc_wdma.ko:system/lib/modules/tcc_wdma.ko

# DXB(Telechips DTV Broadcasting) LinuxTV Driver
#
ifeq ($(TARGET_BOARD_8935_UPC), true)
else
 ifeq ($(TARGET_BOARD_8935_DONGLE), true)
 else
  PRODUCT_COPY_FILES += \
   kernel/drivers/spi/tcc_dxb_drv/tcc_dxb_drv.ko:system/lib/modules/tcc_dxb_drv.ko
  ifneq ($(TARGET_BOARD_SOC), tcc893xS)
   PRODUCT_COPY_FILES += \
    kernel/drivers/spi/tcc_dxb_drv/AVL6211/avl6211_fe.ko:system/lib/modules/avl6211_fe.ko
  endif
 endif
endif

# Hardware Demux Firmware
ifneq ($(TARGET_BOARD_SOC), tcc893xS)
PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/dxb/HWDemux.bin:system/etc/firmware/tcc_tsif/HWDemux.bin
endif

# Media Profiles
PRODUCT_COPY_FILES += \
	device/telechips/tcc893x-common/media_profiles.xml:system/etc/media_profiles.xml

#tinyalsa utilities
PRODUCT_PACKAGES += \
	tinyplay \
	tinycap \
	tinymix

# Audio Mixer Path config
PRODUCT_COPY_FILES += \
 hardware/telechips/common/audio/mixer_paths_wm8731.xml:system/etc/mixer_paths_wm8731.xml \
 hardware/telechips/common/audio/mixer_paths_es8388.xml:system/etc/mixer_paths_es8388.xml \
 hardware/telechips/common/audio/mixer_paths_wm8988.xml:system/etc/mixer_paths_wm8988.xml \
 hardware/telechips/common/audio/mixer_paths_wm8524.xml:system/etc/mixer_paths_wm8524.xml \
 hardware/telechips/common/audio/mixer_paths_rt5633.xml:system/etc/mixer_paths_rt5633.xml \
 hardware/telechips/common/audio/mixer_paths_rt5631.xml:system/etc/mixer_paths_rt5631.xml \
 hardware/telechips/common/audio/mixer_paths_rt5625.xml:system/etc/mixer_paths_rt5625.xml

BOARD_SEPOLICY_DIRS := \
		     device/telechips/tcc893x-common/sepolicy
# The list below is order dependent
BOARD_SEPOLICY_UNION := \
                     device.te \
                     app.te \
                     camera.te \
                     system.te \
                     surfaceflinger.te \
                     file_contexts

# stuff common to all Telechips devices
$(call inherit-product, hardware/telechips/common/tcc893x.mk)
$(call inherit-product-if-exists, vendor/telechips/proprietary/tcc-vendor.mk)
$(call inherit-product, device/telechips/common/common.mk)
