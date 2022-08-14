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


# Inherit from those products. Most specific first.
$(call inherit-product, device/telechips/tcc8920st/device.mk)
$(call inherit-product, hardware/telechips/nand_v8/nand_v8.mk)
# This is where we'd set a backup provider if we had one
#$(call inherit-product, device/sample/products/backup_overlay.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# How this product is called in the build system
PRODUCT_NAME := full_tcc8920st
PRODUCT_DEVICE := tcc8920st
PRODUCT_BRAND := Android
PRODUCT_MANUFACTURER := telechips

# Define the name of target board
TARGET_BOARD_8920_EV := true
#TARGET_BOARD_8925_YJ8925T := true
#TARGET_BOARD_8925_UPC := true
#TARGET_BOARD_8925_DONGLE := true

PRODUCT_CHARACTERISTICS := tablet,sdcard
PRODUCT_TAGS += nand_v8

# The user-visible product name
ifeq ($(TARGET_BOARD_8920_EV),true)
PRODUCT_MODEL := TCC8920_STB_EV
endif
ifeq ($(TARGET_BOARD_8925_YJ8925T),true)
PRODUCT_MODEL := TCC8925_YJ8925T
endif
ifeq ($(TARGET_BOARD_8925_UPC),true)
PRODUCT_MODEL := TCC8925_UPC
endif
ifeq ($(TARGET_BOARD_8925_DONGLE),true)
PRODUCT_MODEL := TCC8925_HDMI_DONGLE
endif
