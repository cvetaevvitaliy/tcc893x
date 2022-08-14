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

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := kernel/arch/arm/boot/Image
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

TARGET_BOARD_SOC := tcc892x
#TARGET_BOARD_SOC := tcc892xS

USE_MASS_STORAGE := false

DEVICE_PACKAGE_OVERLAYS := device/telechips/m805_892x/overlay

PRODUCT_AAPT_CONFIG := normal large mdpi

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15 \
	ro.carrier=wifi-only

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.hdmi_max_resolution = fullhd \
	persist.sys.renderer_onthefly = true

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.hdmi_active = true \
	persist.sys.output_mode = 1 \
        persist.sys.hdmi_resize_x = 0 \
        persist.sys.hdmi_resize_y = 0 \
	persist.demo.hdmirotationlock = true

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.composite_active = false \
	persist.sys.composite_resize_x = 3 \
	persist.sys.composite_resize_y = 3

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.component_active = false \
	tcc.output.component_mode = 0

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.hdmi_portable = true \
	ro.system.osd_active = true

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15

PRODUCT_PROPERTY_OVERRIDES += \
        ro.system.audio_active = false

# Disable navigation bar
PRODUCT_PROPERTY_OVERRIDES += \
	qemu.hw.mainkeys = 0

# Disable Strict mode
PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.strictmode.disable = true \
	persist.sys.strictmode.visual = false

PRODUCT_COPY_FILES := \
	$(LOCAL_KERNEL):kernel \
	device/telechips/m805_892x/init.m805_892x.rc:root/init.m805_892x.rc \
	device/telechips/m805_892x/init.m805_892x.setupfs.rc:root/init.m805_892x.setupfs.rc \
	device/telechips/m805_892x/init.recovery.m805_892x.rc:root/init.recovery.m805_892x.rc \
	device/telechips/m805_892x/init.m805_892x.nand.rc:root/init.m805_892x.nand.rc \
	device/telechips/m805_892x/init.m805_892x.nand.rc:root/init.recovery.m805_892x.nand.rc \
	device/telechips/m805_892x/init.m805_892x.emmc.rc:root/init.m805_892x.emmc.rc \
	device/telechips/m805_892x/init.m805_892x.emmc.rc:root/init.recovery.m805_892x.emmc.rc \
	device/telechips/m805_892x/fstab.m805_892x:root/fstab.m805_892x \
	device/telechips/m805_892x/ueventd.m805_892x.rc:root/ueventd.m805_892x.rc \
	device/telechips/m805_892x/init.m805_892x.usb.rc:root/init.m805_892x.usb.rc \
	device/telechips/m805_892x/init.m805_892x.wifi.realtek.rc:root/init.m805_892x.wifi.realtek.rc \
	device/telechips/m805_892x/init.m805_892x.wifi.broadcom.rc:root/init.m805_892x.wifi.broadcom.rc

PRODUCT_COPY_FILES += \
	device/telechips/m805_892x/recovery.fstab:root/factory.fstab

# Define TCC video vsync mode
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.video.vsync.support = 1 \
	tcc.video.vsync.threshold = 0 \
	tcc.video.surface.support = 1

# Define TCC video deinteralce mode.
PRODUCT_PROPERTY_OVERRIDES += \
         tcc.video.deinterlace.support = 1

# Define NTP default timezone
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.default.timezone = Asia/Shanghai

# Define FAT Partition Number of Internal Storage
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.internal.storage.fat.number = 11

# Set default USB interface
ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mass_storage
else ifeq ($(USE_MASS_STORAGE),false)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mtp
endif


# Live Wallpapers
PRODUCT_PACKAGES += \
	LiveWallpapers \
	LiveWallpapersPicker \
	VisualizationWallpapers \
	PhaseBeam \
	librs_jni \
	Camera	\
	Gallery2

# Input device calibration files
PRODUCT_COPY_FILES += \
	device/telechips/m805_892x/tcc-ts.idc:system/usr/idc/tcc-ts.idc

PRODUCT_COPY_FILES += \
	device/telechips/m805_892x/tcc-ts-sitronix.idc:system/usr/idc/tcc-ts-sitronix.idc

PRODUCT_COPY_FILES += \
	device/telechips/m805_892x/tcc-ts-goodix-cap.idc:system/usr/idc/tcc-ts-goodix-cap.idc

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
	frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
	frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
	frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
	frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml

ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_COPY_FILES += \
	device/telechips/m805_892x/ums/vold.fstab:system/etc/vold.fstab\
	device/telechips/m805_892x/ums/vold.emmc.fstab:system/etc/vold.emmc.fstab
else ifeq ($(USE_MASS_STORAGE),false)
#PRODUCT_COPY_FILES += \
	device/telechips/m805_892x/vold.fstab:system/etc/vold.fstab\
	device/telechips/m805_892x/vold.emmc.fstab:system/etc/vold.emmc.fstab
endif

#ifeq ($(TARGET_HAVE_TSLIB),true)
PRODUCT_COPY_FILES += \
         $(LOCAL_PATH)/../../../external/tslib/ts.conf:system/ts.conf
#endif

PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=131072

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=120
#	ro.sf.lcd_density=100

PRODUCT_PROPERTY_OVERRIDES += \
	debug.hwui.render_dirty_regions=false

PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_PACKAGES += \
	librs_jni \
	com.android.future.usb.accessory

#ifeq ($(TARGET_HAVE_TSLIB),true)
PRODUCT_PACKAGES += \
        TSCalibration \
        libtslib \
        inputraw \
        pthres \
        dejitter \
        linear
#endif

# AKMD8975 sensor
PRODUCT_PACKAGES += \
	akmd8975

PRODUCT_COPY_FILES += \
	device/telechips/common/initlogo1024x600.rle:root/initlogo.rle
#	device/telechips/common/initlogo800x480.rle:root/initlogo.rle
#	device/telechips/common/initlogo1280x720.rle:root/initlogo.rle

#Telechips system update apk
PRODUCT_PACKAGES  += \
	TelechipsSystemUpdater

#File system management packages
PRODUCT_PACKAGES += \
		    setup_fs\
		    make_ext4fs\
		    e2fsck \
		    mkmtdimg

#Making Splash Image packages
PRODUCT_PACKAGES += \
		mksplashimg \
		mksplash

PRODUCT_PACKAGES += TelechipsWFDSink 
				
#Telechips Security Solution 
$(call inherit-product, device/telechips/m805_892x/tcsec.m805_892x.mk)

$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)

# Realtek Wi-Fi module
$(call inherit-product, device/telechips/tcc892x-common/wifi/realtek.mk)

# Broadcom Wi-Fi module
#$(call inherit-product, device/telechips/tcc893x-common/wifi/bcm4334.mk)
#$(call inherit-product, device/telechips/tcc893x-common/wifi/bcm4335.mk)
#$(call inherit-product, device/telechips/tcc893x-common/wifi/ap6234.mk)

# stuff common to all Telechips TCC892x devices
$(call inherit-product, device/telechips/tcc892x-common/device_tcc892x-common.mk)

