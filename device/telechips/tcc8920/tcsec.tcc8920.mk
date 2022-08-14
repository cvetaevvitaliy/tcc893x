#
# Copyright (C) 2014 Telechips, Inc.
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

##### Defines for supporting DRM #####
TC_DRMS_NOT_SUPPRORT := 0
TC_DRMS_WITH_HIDDEN := 1
TC_DRMS_WITH_TEE := 2

# HDCP1.X
TARGET_USE_HDCP1X := TC_DRMS_NOT_SUPPRORT
#TARGET_USE_HDCP1X := TC_DRMS_WITH_HIDDEN

# HDCP2.X
TARGET_USE_HDCP2X := TC_DRMS_NOT_SUPPRORT
#TARGET_USE_HDCP2X := TC_DRMS_WITH_HIDDEN
#TARGET_USE_HDCP2X := TC_DRMS_WITH_TEE

# DTCP-IP
TARGET_USE_DTCPIP := TC_DRMS_NOT_SUPPRORT
#TARGET_USE_DTCPIP := TC_DRMS_WITH_HIDDEN
#TARGET_USE_DTCPIP := TC_DRMS_WITH_TEE

# WIDEVINE L3
TARGET_USE_WIDEVINE := TC_DRMS_NOT_SUPPRORT
#TARGET_USE_WIDEVINE := TC_DRMS_WITH_HIDDEN

# PLAYREADY
TARGET_USE_PLAYREADY := TC_DRMS_NOT_SUPPRORT
#TARGET_USE_PLAYREADY := TC_DRMS_WITH_HIDDEN

#<-- Telechips Security Module for MP
TARGET_USE_TCQRTM := false



##<--- Do not modify the below
### TARGET_USE_HIDDEN : DRM with Hidden Partition ###
TARGET_USE_HIDDEN := false

ifeq ($(TARGET_USE_HDCP1X), TC_DRMS_WITH_HIDDEN)		#HDCP1.x Module
TARGET_USE_HIDDEN := true
PRODUCT_PACKAGES += \
	libhcdcp1xapi
PRODUCT_COPY_FILES += \
	hardware/telechips/hciph/tcc_hcdcp1x_892x.ko:system/lib/modules/tcc_hcdcp1x.ko
endif
ifeq ($(TARGET_USE_HDCP2X), TC_DRMS_WITH_HIDDEN)		#HDCP2.x Module
TARGET_USE_HIDDEN := true
PRODUCT_PACKAGES += \
	libhdcp2hal \
	libhdcp2
endif
ifeq ($(TARGET_USE_DTCPIP), TC_DRMS_WITH_HIDDEN)		#DTCP-IP Module
TARGET_USE_HIDDEN := true
endif
ifeq ($(TARGET_USE_WIDEVINE), TC_DRMS_WITH_HIDDEN)		#WideVine Module
TARGET_USE_HIDDEN := true
endif
ifeq ($(TARGET_USE_PLAYREADY), TC_DRMS_WITH_HIDDEN)		#PlayReady Module
TARGET_USE_HIDDEN := true
endif

ifeq ($(TARGET_USE_HIDDEN), true)	    #Common Hidden Partition Module
PRODUCT_PACKAGES += \
	libhciphapi
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920/init.tcc8920.hidden.rc:root/init.tcc8920.hidden.rc \
	device/telechips/tcc8920/init.tcc8920.hidden.nand.rc:root/init.tcc8920.hidden.nand.rc \
	device/telechips/tcc8920/init.tcc8920.hidden.emmc.rc:root/init.tcc8920.hidden.emmc.rc \
	hardware/telechips/hciph/tcc_hciph_nand_892x.ko:system/lib/modules/tcc_hciph_nand.ko \
	hardware/telechips/hciph/tcc_hciph_emmc_892x.ko:system/lib/modules/tcc_hciph_emmc.ko
else
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920/init.tcc8920.wodrm.rc:root/init.tcc8920.hidden.rc
endif


### TARGET_USE_TZOS : DRM with TEE ###
TARGET_USE_TZOS := false

ifeq ($(TARGET_USE_HDCP2X), TC_DRMS_WITH_TEE)		#HDCP2.x Module
TARGET_USE_TZOS := true
PRODUCT_PACKAGES += \
	libhdcp2
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920/init.tcc8920.tztee.hdcp2.rc:root/init.tcc8920.tztee.hdcp2.rc
endif
ifeq ($(TARGET_USE_DTCPIP), TC_DRMS_WITH_TEE)		#DTCP-IP Module
TARGET_USE_TZOS := true
PRODUCT_PACKAGES += \
	libdtcpip
endif

ifeq ($(TARGET_USE_TZOS), true)
PRODUCT_PACKAGES += \
	libteecapi \
	libteesmapi

PRODUCT_COPY_FILES += \
	device/telechips/tcc8920/init.tcc8920.tztee.rc:root/init.tcc8920.tztee.rc \
	device/telechips/tcc8920/init.tcc8920.tztee.tzos.rc:root/init.tcc8920.tztee.tzos.rc \
	hardware/telechips/tzos/tzdrv_892x.ko:system/lib/modules/tzdrv.ko \
	device/telechips/tcc8920/fstab.tcc892x.sest:root/fstab.tcc892x.sest

else

PRODUCT_COPY_FILES += \
	device/telechips/tcc8920/init.tcc8920.wodrm.rc:root/init.tcc8920.tztee.rc

ifeq ($(TARGET_USE_TCQRTM), true)
PRODUCT_PACKAGES += \
	libtcqrtmapi
PRODUCT_COPY_FILES += \
	device/telechips/tcc8920/init.tcc8920.tcqrtm.rc:root/init.tcc8920.tztee.rc \
	hardware/telechips/tcqrt/tcc_cqrtm_892x.ko:system/lib/modules/tcc_cqrtm.ko
endif
endif
##--->

