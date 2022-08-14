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

# BoardConfig.mk
#
# Product-specific compile-time definitions.
#

BOARD_HAVE_NAND_FLASH := true
TARGET_NAND_TNFTL_VERSION := 8
TARGET_USERIMAGES_USE_EXT4 := true

TARGET_BOOTLOADER_BOARD_NAME := tcc893x_evm

TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true
TARGET_UPDATE_ONLY_SPLASH := false
TARGET_UPDATE_ONLY_BOOTLOADER := false

# Splash Partition Use
TARGET_SPLASH_USE := true

KERNEL_DEFCONFIG := tcc893x_defconfig

BOARD_CFLAGS := -DTCC893X
TARGET_BOOTLOADER_BOARD_CFLAGS := -D_TCC8930_

BOARD_FLASH_BLOCK_SIZE := 4096
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 681574400 #650MB System
BOARD_USERDATAIMAGE_PARTITION_SIZE := 1073741824 #1GB UserData

BOARD_CACHEIMAGE_PARTITION_SIZE :=  157286400
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4

BOARD_QBDATAIMAGE_PARTITION_SIZE :=  209715200 #200MByte
BOARD_QBDATAIMAGE_FILE_SYSTEM_TYPE := ext4

# Wi-Fi defines
BOARD_USES_REALTEK_WIFI := true
BOARD_USES_BROADCOM_WIFI := false

ifeq ($(BOARD_USES_REALTEK_WIFI), true)
    REALTEK_MODULE_NAME := rt8188cu
    WPA_SUPPLICANT_VERSION := VER_0_8_X
    BOARD_WPA_SUPPLICANT_DRIVER := NL80211
    BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_rtl
    BOARD_HOSTAPD_DRIVER        := NL80211
    BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_rtl
    CONFIG_DRIVER_WEXT 			:=y
    WIFI_DRIVER_MODULE_NAME   := wlan
    WIFI_DRIVER_MODULE_PATH   := "/system/wifi/wlan.ko"

    WIFI_DRIVER_MODULE_ARG    := "ifname=wlan0 if2name=p2p0"
    WIFI_FIRMWARE_LOADER      := ""
    WIFI_DRIVER_FW_PATH_STA   := ""
    WIFI_DRIVER_FW_PATH_AP    := ""
    WIFI_DRIVER_FW_PATH_P2P   := ""
    WIFI_DRIVER_FW_PATH_PARAM := ""
endif

ifeq ($(BOARD_USES_BROADCOM_WIFI), true)
    BOARD_WPA_SUPPLICANT_DRIVER 	:= NL80211
    WPA_SUPPLICANT_VERSION      	:= VER_0_8_X
    BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
    BOARD_HOSTAPD_DRIVER        	:= NL80211
    BOARD_HOSTAPD_PRIVATE_LIB   	:= lib_driver_cmd_bcmdhd
    BOARD_WLAN_DEVICE           	:= bcmdhd
    WIFI_DRIVER_MODULE_NAME 		:= bcmdhd
    WIFI_DRIVER_FW_PATH_PARAM   	:= "/data/misc/wifi/firmware_path"
    WIFI_DRIVER_MODULE_PATH 		:= "/system/wifi/bcmdhd.ko"
    WIFI_DRIVER_MODULE_ARG			:= "iface_name=wlan firmware_path=/system/wifi/fw_bcmdhd.bin nvram_path=/system/wifi/bcmdhd.cal"
    WIFI_DRIVER_FW_PATH_STA     	:= "/system/wifi/fw_bcmdhd.bin"
    WIFI_DRIVER_FW_PATH_P2P     	:= "/system/wifi/fw_bcmdhd_p2p.bin"
    WIFI_DRIVER_FW_PATH_AP      	:= "/system/wifi/fw_bcmdhd_apsta.bin"
endif

# Bluetooth defines
#
BOARD_HAVE_BLUETOOTH := false
#CUSTOM_BLUETOOTH_VENDOR := csr
#CUSTOM_BLUETOOTH_VENDOR := bcm
BOARD_HAVE_BLUETOOTH_BCM := false

BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG := true
#BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG := false 
BOARD_VIDEO_DEINTERLACE_SUPPORT_FLAG := true
#BOARD_VIDEO_DEINTERLACE_SUPPORT_FLAG := false
BOARD_SKIP_UI_UPDATE_DURING_VIDEO_FLAG := true

ifeq ($(BOARD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG), true)
BOARD_LCD_VIDEO_DISPLAY_BY_VSYNC_INT_FLAG := true
endif

#if you want use bluetooth and wifi coex , you should check below option.
#BOARD_ANTENNA_SHARING_BT_WIFI := true

#if you want use csr bluetooth host wake up, you should check below option.
#BOARD_BT_HOST_WAKE_UP := true
# 
# /Bluetooth defines

# TV output defines
BOARD_HAVE_HDMI_OUTPUT := true
## HDMI Version select
ifeq ($(TARGET_BOARD_SOC), tcc893x)
BOARD_HDMI_VERSION := HDMI_V1_4
else
BOARD_HDMI_VERSION := HDMI_V1_3
endif

BOARD_HDMI_CLK := -DTCC_HDMI_USE_XIN_24MHZ
#BOARD_HDMI_CLK := -DTCC_HDMI_USE_INTERNAL_PLL
#BOARD_HDMI_CLK := -DTCC_HDMI_USE_EXTERNAL_CRYSTAL

#BOARD_HDMI_HDCP := -DTCC_HDMI_HDCP

BOARD_HAVE_TV_ANALOG_OUTPUT := true
BOARD_TV_COMPOSITE := -DTCC_OUTPUT_COMPOSITE
#BOARD_TV_COMPONENT := -DTCC_OUTPUT_COMPONENT


# GPS define
BOARD_GPS_LIBRARIES := libgps
# #BOARD_GPS_MAKER := tcc_gps
BOARD_GPS_MAKER := surf_gps

# Audio Codec Chip
TARGET_BOARD_AUDIO_CODEC := wm8731

BOARD_USES_VOIP := true

# Touch Calibration - use only pressure sensitive touch screen
#TARGET_HAVE_TSLIB := true

TARGET_RECOVERY_UI_LIB := librecovery_ui_tcc893x
TARGET_RECOVERY_UPDATER_LIBS += librecovery_updater_telechips

include device/telechips/tcc893x-common/BoardConfigCommon.mk
