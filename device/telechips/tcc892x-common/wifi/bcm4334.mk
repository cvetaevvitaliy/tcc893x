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
$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/config/config-bcm.mk)	

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/broadcom/bcm4334/bcm4334.ko:system/wifi/bcmdhd.ko \
	$(LOCAL_PATH)/broadcom/bcm4334/fw_bcmdhd_apsta.bin:system/wifi/fw_bcmdhd_apsta.bin \
	$(LOCAL_PATH)/broadcom/bcm4334/fw_bcmdhd.bin:system/wifi/fw_bcmdhd.bin

ifeq ($(TARGET_BOARD_8925_DONGLE),true)
PRODUCT_COPY_FILES += $(LOCAL_PATH)/broadcom/bcm4334/bcmdhd_koine.cal:system/wifi/bcmdhd.cal
else
PRODUCT_COPY_FILES += $(LOCAL_PATH)/broadcom/bcm4334/bcmdhd.cal:system/wifi/bcmdhd.cal
endif

PRODUCT_PROPERTY_OVERRIDES += \
    tcc.wifi.p2p.persistent = 1
