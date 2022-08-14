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

USE_MASS_STORAGE = false


ifeq ($(USE_MASS_STORAGE),true)
DEVICE_PACKAGE_OVERLAYS := device/telechips/tcc8920st/ums/overlay
endif

DEVICE_PACKAGE_OVERLAYS += device/telechips/tcc8920st/overlay

PRODUCT_AAPT_CONFIG := normal mdpi

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.hdmi_max_resolution = fullhd \
	ro.system.hdmi_active = true \
	persist.sys.hdmi_resolution = 125 \
	persist.sys.hdmi_resize_up = 0 \
	persist.sys.hdmi_resize_dn = 0 \
	persist.sys.hdmi_resize_lt = 0 \
	persist.sys.hdmi_resize_rt = 0 \
	tcc.hdmi.uisize = 1280x720 \
	persist.sys.renderer_onthefly = true \
	persist.sys.hdmi_color_space = 1

#PRODUCT_PROPERTY_OVERRIDES += ro.QB.enable=1
#PRODUCT_PACKAGES += quickboot

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.composite_active = true \
	persist.sys.composite_mode = 0 \
	persist.sys.composite_resize_up = 3 \
	persist.sys.composite_resize_dn = 3 \
	persist.sys.composite_resize_lt = 3 \
	persist.sys.composite_resize_rt = 3 \

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.component_active = true \
	persist.sys.component_mode = 1 \
	persist.sys.component_resize_up = 3 \
	persist.sys.component_resize_dn = 3 \
	persist.sys.component_resize_lt = 3 \
	persist.sys.component_resize_rt = 3 \
	persist.sys.auto_resolution = 0

PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.hdmi_portable = false \
	ro.system.osd_active = true

ifdef TARGET_BOARD_HDMI_DONGLE
PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.audio_active = false
else
PRODUCT_PROPERTY_OVERRIDES += \
	ro.system.audio_active = true \
	persist.sys.spdif_setting = 0
endif

# Define TCC set-top/media box solution
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.solution.mbox = 1 \
	tcc.solution.video = 1 \
	tcc.solution.preview = 0 \
	tcc.solution.mbox.sleep = 1 \

# this property invoke deskclock Application after the value(minutes) at Home Screen.  ( 0: disable , value : enable)
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.solution.deskclock = 0

# this property don't get thumbnail image using video decoder but get static image from Resource. ( 0:displable , 1:enable)
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.solution.nothumb = 1

# this property play next video preview after the value(minutes).  ( 0: disable ,  value : enable )
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.solution.previewduration = 0

# Define TCC internal subtitle
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.internal.subtitle = 1

# Define skip meta of video & image
#
PRODUCT_PROPERTY_OVERRIDES += \
        tcc.solution.skipVideoMeta = 1 \
        tcc.solution.skipImageMeta = 1
	
PRODUCT_COPY_FILES := \
	$(LOCAL_KERNEL):kernel \
	device/telechips/tcc8920st/init.tcc8920st.setupfs.rc:root/init.tcc8920st.setupfs.rc \
	device/telechips/tcc8920st/init.recovery.tcc8920st.rc:root/init.recovery.tcc8920st.rc \
	device/telechips/tcc8920st/init.tcc8920st.nand.rc:root/init.tcc8920st.nand.rc \
	device/telechips/tcc8920st/init.tcc8920st.nand.rc:root/init.recovery.tcc8920st.nand.rc \
	device/telechips/tcc8920st/init.tcc8920st.emmc.rc:root/init.tcc8920st.emmc.rc \
	device/telechips/tcc8920st/init.tcc8920st.emmc.rc:root/init.recovery.tcc8920st.emmc.rc \
	device/telechips/tcc8920st/fstab.tcc8920st:root/fstab.tcc8920st \
	device/telechips/tcc8920st/fstab.quickboot.tcc8920st:root/fstab.quickboot.tcc8920st \
	device/telechips/tcc8920st/ueventd.tcc8920st.rc:root/ueventd.tcc8920st.rc \
	device/telechips/tcc8920st/init.tcc8920st.usb.rc:root/init.tcc8920st.usb.rc \
	device/telechips/tcc8920st/init.tcc8920st.wifi.realtek.rc:root/init.tcc8920st.wifi.realtek.rc \
	device/telechips/tcc8920st/init.tcc8920st.wifi.broadcom.rc:root/init.tcc8920st.wifi.broadcom.rc

ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920st/ums/init.tcc8920st.rc:root/init.tcc8920st.rc
else
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920st/init.tcc8920st.rc:root/init.tcc8920st.rc
endif

PRODUCT_COPY_FILES += \
	device/telechips/tcc8920st/recovery.fstab:root/factory.fstab

# Define TCC video vsync mode
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.video.vsync.support = 1 \
	tcc.video.vsync.threshold = 0 \
	tcc.video.surface.support = 1

# Define TCC video deinteralce mode, this is a sub item of vsync mode, so you have to enable vsync mode first to use deinterlace mode.
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.video.deinterlace.support = 0

# Gallery is used only image
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.solution.onlyimage = 1

# Define EXCLUSIVE UI
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.exclusive.ui.enable = 0

# Define 3D UI mode
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.3d.ui.enable = 0

# Define display mode for external output (0:normal, 1:auto-detection(HDMI<->CVBS), 2:HDMI/CVBS<->CVBS, 3:HDMI/CVBS<->CVBS/Component)  
PRODUCT_PROPERTY_OVERRIDES += \
	persist.sys.display.mode = 0

# Define display path for mouse in composite output (0:framework, 1:kernel)
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.composite.mouse.path = 1

# Define properties releated STB for using files which will be release library
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.display.output.stb = 1 \
	tcc.component.chip = ths8200

# Define key long press power shutdown popery 
PRODUCT_PROPERTY_OVERRIDES += \
        tcc.shutdown.active = 0

# Define wifi & bluetooth for STB
PRODUCT_PROPERTY_OVERRIDES += \
        tcc.solution.wifi.mbox = 1 

PRODUCT_PROPERTY_OVERRIDES += tcc.solution.bluetooth.mbox = 0

# Define cec enable for STB
PRODUCT_PROPERTY_OVERRIDES += \
        ro.system.cec_active = true \
	persist.sys.hdmi_cec = 1

# Define NTP default timezone
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.default.timezone = Asia/Shanghai

# Define FAT Partition Number of Internal Storage
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.internal.storage.fat.number = 12

ifdef TARGET_BOARD_HDMI_DONGLE
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.wifi.forced.enable = 1
endif

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15

PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160

PRODUCT_COPY_FILES += \
        device/telechips/common/initlogo1280x720.rle:root/initlogo.rle

# Set default USB interface
ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mass_storage
else
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mtp
endif
# Input device calibration files
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920st/tcc-ts.idc:system/usr/idc/tcc-ts.idc

# These are the hardware-specific features
PRODUCT_COPY_FILES += \
	frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
	frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
	frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
	frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
	frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
	frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
	frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
	frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
	frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
	frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
	frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml

PRODUCT_CHARACTERISTICS := tablet,sdcard

PRODUCT_TAGS += dalvik.gc.type-precise

# Live Wallpapers
PRODUCT_PACKAGES += \
	LiveWallpapers \
	LiveWallpapersPicker \
	VisualizationWallpapers \
	PhaseBeam \
	librs_jni \
	Camera \
	TelechipsSystemUpdater \
	VideoPlayer	\
	Gallery \
	TelechipsLauncher \
	TelechipsApkInstaller \
	TelechipsAppsManager

# Initial SettingScreen
PRODUCT_PACKAGES += \
        SettingScreen

ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920st/ums/vold.fstab:system/etc/vold.fstab\
	device/telechips/tcc8920st/ums/vold.emmc.fstab:system/etc/vold.emmc.fstab
else ifeq ($(USE_MASS_STORAGE),false)
#PRODUCT_COPY_FILES += \
	device/telechips/tcc8920st/vold.fstab:system/etc/vold.fstab\
	device/telechips/tcc8920st/vold.emmc.fstab:system/etc/vold.emmc.fstab
endif


# File system management packages
PRODUCT_PACKAGES += \
	setup_fs \
	make_ext4fs \
	e2fsck\
	mkmtdimg

#Making Splash Image packages
PRODUCT_PACKAGES += \
				mksplashimg \
				mksplash

PRODUCT_PACKAGES += \
	librs_jni \
	com.android.future.usb.accessory

# AKMD8975 sensor
PRODUCT_PACKAGES += \
	akmd8975

PRODUCT_PACKAGES += TelechipsWFDSink

#Telechips Security Solution 
$(call inherit-product, device/telechips/tcc8920st/tcsec.tcc8920st.mk)

$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)

# Realtek Wi-Fi module
$(call inherit-product, device/telechips/tcc892x-common/wifi/realtek.mk)

# Broadcom Wi-Fi module
#$(call inherit-product, device/telechips/tcc892x-common/wifi/bcm4334.mk)
#$(call inherit-product, device/telechips/tcc892x-common/wifi/bcm4335.mk)
#$(call inherit-product, device/telechips/tcc892x-common/wifi/ap6234.mk)

# Telechips remote(IR)
$(call inherit-product, device/telechips/common/telechips-remote.mk)

# Google TV remote
#$(call inherit-product, device/telechips/common/googletv-remote.mk)

# stuff common to all Telechips TCC892x devices
$(call inherit-product, device/telechips/tcc892x-common/device_tcc892x-common.mk)
