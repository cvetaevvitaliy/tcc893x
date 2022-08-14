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
	$(LOCAL_PATH)/broadcom/ap6234/ap6234.ko:/system/wifi/bcmdhd.ko \
	$(LOCAL_PATH)/broadcom/ap6234/fw_bcm43341b0_ag.bin:/system/wifi/fw_bcmdhd.bin \
	$(LOCAL_PATH)/broadcom/ap6234/fw_bcm43341b0_ag_apsta.bin:/system/wifi/fw_bcmdhd_apsta.bin \
	$(LOCAL_PATH)/broadcom/ap6234/nvram_ap6234.txt:/system/wifi/bcmdhd.cal

PRODUCT_PROPERTY_OVERRIDES += \
    tcc.wifi.p2p.persistent = 1
