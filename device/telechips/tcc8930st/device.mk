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

ifeq ($(TARGET_PREBUILT_KERNEL),)
LOCAL_KERNEL := kernel/arch/arm/boot/Image
else
LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

TARGET_BOARD_SOC := tcc893x
#TARGET_BOARD_SOC := tcc893xS

USE_MASS_STORAGE := false

USE_FB_1080P := false

ifeq ($(USE_MASS_STORAGE),true)
DEVICE_PACKAGE_OVERLAYS := device/telechips/tcc8930st/ums/overlay
endif

DEVICE_PACKAGE_OVERLAYS += device/telechips/tcc8930st/overlay

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

ifeq ($(TARGET_BOARD_8935_DONGLE),true)
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
	device/telechips/tcc8930st/init.tcc8930st.setupfs.rc:root/init.tcc8930st.setupfs.rc \
	device/telechips/tcc8930st/init.recovery.tcc8930st.rc:root/init.recovery.tcc8930st.rc \
	device/telechips/tcc8930st/init.tcc8930st.nand.rc:root/init.tcc8930st.nand.rc \
	device/telechips/tcc8930st/init.tcc8930st.nand.rc:root/init.recovery.tcc8930st.nand.rc \
	device/telechips/tcc8930st/init.tcc8930st.emmc.rc:root/init.tcc8930st.emmc.rc \
	device/telechips/tcc8930st/init.tcc8930st.emmc.rc:root/init.recovery.tcc8930st.emmc.rc \
	device/telechips/tcc8930st/fstab.tcc8930st:root/fstab.tcc8930st \
	device/telechips/tcc8930st/fstab.tcc8930st.emmc:root/fstab.tcc8930st.emmc \
	device/telechips/tcc8930st/fstab.quickboot.tcc8930st:root/fstab.quickboot.tcc8930st \
	device/telechips/tcc8930st/ueventd.tcc8930st.rc:root/ueventd.tcc8930st.rc \
	device/telechips/tcc8930st/init.tcc8930st.usb.rc:root/init.tcc8930st.usb.rc \
	device/telechips/tcc8930st/init.tcc8930st.wifi.realtek.rc:root/init.tcc8930st.wifi.realtek.rc \
	device/telechips/tcc8930st/init.tcc8930st.wifi.broadcom.rc:root/init.tcc8930st.wifi.broadcom.rc

ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/ums/init.tcc8930st.rc:root/init.tcc8930st.rc
else
PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/init.tcc8930st.rc:root/init.tcc8930st.rc
endif

PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/recovery.fstab:root/factory.fstab

# Define TCC video vsync mode
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.video.vsync.support = 1 \
	tcc.video.vsync.threshold = 0 \
	tcc.video.surface.support = 1 \
	tcc.video.mvc.support = 0 \
	tcc.video.mvc.enable = 0

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

ifeq ($(TARGET_BOARD_8935_DONGLE),true)
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.wifi.forced.enable = 1
endif

PRODUCT_PROPERTY_OVERRIDES += \
	wifi.interface=wlan0 \
	wifi.supplicant_scan_interval=15

# Set default USB interface
ifeq ($(USE_MASS_STORAGE),true)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
	persist.sys.usb.config=mass_storage
else
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
	TelechipsSystemUpdater \
	VideoPlayer	\
	Gallery \
	TelechipsLauncher \
	TelechipsApkInstaller \
	TelechipsAppsManager

# Input device calibration files
PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/tcc-ts.idc:system/usr/idc/tcc-ts.idc

PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/tcc-ts-sitronix.idc:system/usr/idc/tcc-ts-sitronix.idc

PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/tcc-ts-goodix-cap.idc:system/usr/idc/tcc-ts-goodix-cap.idc

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
	device/telechips/tcc8930st/ums/vold.fstab:system/etc/vold.fstab\
	device/telechips/tcc8930st/ums/vold.emmc.fstab:system/etc/vold.emmc.fstab
else ifeq ($(USE_MASS_STORAGE),false)
#PRODUCT_COPY_FILES += \
	device/telechips/tcc8930st/vold.fstab:system/etc/vold.fstab\
	device/telechips/tcc8930st/vold.emmc.fstab:system/etc/vold.emmc.fstab
ifeq ($(TARGET_BOARD_SOC),tcc893x)
PRODUCT_COPY_FILES += \
   kernel/drivers/usb/host/xhci-hcd.ko:root/lib/modules/xhci-hcd.ko
PRODUCT_COPY_FILES += \
   kernel/drivers/usb/dwc3/dwc_usb3.ko:root/lib/modules/dwc_usb3.ko
PRODUCT_COPY_FILES += \
   kernel/drivers/usb/dwc3/dwc3-tcc.ko:root/lib/modules/dwc3-tcc.ko
PRODUCT_COPY_FILES += \
   kernel/drivers/usb/gadget/g_android.ko:root/lib/modules/g_android.ko
PRODUCT_COPY_FILES += \
   kernel/drivers/usb/gadget/udc-core.ko:root/lib/modules/udc-core.ko
else
PRODUCT_PROPERTY_OVERRIDES += \
	ro.chip.product=tcc8935s
endif
endif

#ifeq ($(TARGET_HAVE_TSLIB),true)
PRODUCT_COPY_FILES += \
         $(LOCAL_PATH)/../../../external/tslib/ts.conf:system/ts.conf
#endif

PRODUCT_PROPERTY_OVERRIDES += \
	ro.opengles.version=131072

ifeq ($(USE_FB_1080P),true)
PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=240
else
PRODUCT_PROPERTY_OVERRIDES += \
	ro.sf.lcd_density=160
endif

PRODUCT_TAGS += dalvik.gc.type-precise

# Initial SettingScreen
PRODUCT_PACKAGES += \
        SettingScreen

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

ifeq ($(USE_FB_1080P),true)
PRODUCT_COPY_FILES += \
	device/telechips/common/initlogo1920x1080.rle:root/initlogo.rle
else
PRODUCT_COPY_FILES += \
	device/telechips/common/initlogo1280x720.rle:root/initlogo.rle
endif

#File system management packages
PRODUCT_PACKAGES += \
		    setup_fs\
		    make_ext4fs\
		    e2fsck\
		    mkmtdimg

#Making Splash Image packages
PRODUCT_PACKAGES += \
				mksplashimg \
				mksplash

PRODUCT_PACKAGES += TelechipsWFDSink

#Telechips Security Solution 
$(call inherit-product, device/telechips/tcc8930st/tcsec.tcc8930st.mk)

$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)

# Realtek Wi-Fi module
$(call inherit-product, device/telechips/tcc893x-common/wifi/realtek.mk)

# Broadcom Wi-Fi module
#$(call inherit-product, device/telechips/tcc893x-common/wifi/bcm4330.mk)
#$(call inherit-product, device/telechips/tcc893x-common/wifi/bcm4334.mk)
#$(call inherit-product, device/telechips/tcc893x-common/wifi/bcm4335.mk)
#$(call inherit-product, device/telechips/tcc893x-common/wifi/ap6234.mk)
#$(call inherit-product, device/telechips/tcc893x-common/wifi/ap6210.mk)

# stuff common to all Telechips TCC8930ST devices
$(call inherit-product, device/telechips/tcc893x-common/device_tcc893x-common.mk)

# Telechips remote(IR)
$(call inherit-product, device/telechips/common/telechips-remote.mk)

# Google TV remote
#$(call inherit-product, device/telechips/common/googletv-remote.mk)
