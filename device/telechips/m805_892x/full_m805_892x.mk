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
$(call inherit-product, device/telechips/m805_892x/device.mk)
$(call inherit-product, hardware/telechips/nand_v8/nand_v8.mk)
# This is where we'd set a backup provider if we had one
#$(call inherit-product, device/sample/products/backup_overlay.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# How this product is called in the build system
PRODUCT_NAME := full_m805_892x
PRODUCT_DEVICE := m805_892x
PRODUCT_BRAND := Android
PRODUCT_MANUFACTURER := telechips

PRODUCT_CHARACTERISTICS := tablet,sdcard

# The user-visible product name
PRODUCT_MODEL := TCC892x_M805S
