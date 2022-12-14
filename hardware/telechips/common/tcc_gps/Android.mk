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
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(BOARD_GPS_MAKER),tcc_gps)

# for gps copy

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/cgvuart.ko:root/lib/modules/cgvuart.ko

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/cgxdrv.ko:system/TccGps/cgxdrv.ko

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/tcc_gps:system/TccGps/tcc_gps

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/rungps.sh:system/TccGps/rungps.sh

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/HiMap.db:system/TccGps/HiMap.db

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/Almanac.cg:system/TccGps/Almanac.cg

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/Hca.cg:system/TccGps/Hca.cg

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/Ionosphere.cg:system/TccGps/Ionosphere.cg

endif
