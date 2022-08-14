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

$(call inherit-product, hardware/telechips/common/tcc_common.mk)
$(call inherit-product, hardware/telechips/omx/tcc_omx.mk)
$(call inherit-product-if-exists, hardware/telechips/dxb/tcc_dxb.mk)
$(call inherit-product-if-exists, hardware/telechips/common/remote_player/remote_player.mk)

PRODUCT_PACKAGES += \
	busybox

PRODUCT_PACKAGES += \
	tcsb_mkimg \
	tcsb_signing\
	tcsb_signtool \
	tcsb_otapackage

#need to make splash image
PRODUCT_PACKAGES += \
	rgb2565\
	rgbto888

PRODUCT_PROPERTY_OVERRIDES += tcc.all.hdmi.720p.fixed = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.all.video.call.enable = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.all.camera.no.display = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.video.lplayback.mode = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.all.web.full.video = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.show.video.fps = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.hwc.dynamic.disable = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.hwc.lcd.disable = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.hwc.no.use.external = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.hwc.fb.update.async = 1
PRODUCT_PROPERTY_OVERRIDES += tcc.hwc.use.fence.sync = 1
PRODUCT_PROPERTY_OVERRIDES += tcc.all.output.support.camera = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.media.rtsp.cusset = 1
PRODUCT_PROPERTY_OVERRIDES += tcc.media.rtsp.starttime = 10
PRODUCT_PROPERTY_OVERRIDES += tcc.media.rtsp.autimeout = 10
PRODUCT_PROPERTY_OVERRIDES += tcc.media.rtsp.eos = 1
PRODUCT_PROPERTY_OVERRIDES += tcc.media.rtsp.seekend = 3
PRODUCT_PROPERTY_OVERRIDES += tcc.extenddisplay.detected = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.wfd.display.level = 0

PRODUCT_PROPERTY_OVERRIDES += ro.QB.builtin.windowManager = 2
#PRODUCT_PROPERTY_OVERRIDES += ro.QB.builtin.launcher = 1
PRODUCT_PROPERTY_OVERRIDES += ro.QB.builtin.widget = 1
#PRODUCT_PROPERTY_OVERRIDES += ro.QB.builtin.wallpaper = 1
PRODUCT_PROPERTY_OVERRIDES += tcc.QB.profile.dlog = 0
PRODUCT_PROPERTY_OVERRIDES += tcc.QB.profile.dtime = 10

#PRODUCT_PROPERTY_OVERRIDES += persist.sys.QB.aging.dump = on
#PRODUCT_PROPERTY_OVERRIDES += persist.sys.QB.reboot.delay = 60
# erase / off / reset
#PRODUCT_PROPERTY_OVERRIDES += persist.sys.QB.aging = reset

#
# tcc.indicate.buff.percentage.at
# running : normal indication of buffering percentage.
# underrun: notifying buffering percentage
#           from 0% buffering at underrun-start
#           until 100% buffering at high water mark.
#PRODUCT_PROPERTY_OVERRIDES += tcc.indicate.buff.percentage.at = underrun
#
#PRODUCT_PROPERTY_OVERRIDES += tcc.parser.sequential = 3

# 
# http media streaming reconnection property
# - if disalbed, default is as below :
#
#PRODUCT_PROPERTY_OVERRIDES += tcc.http.conn.timeout.sec = 5
#PRODUCT_PROPERTY_OVERRIDES += tcc.http.recv.timeout.sec = 6
#PRODUCT_PROPERTY_OVERRIDES += tcc.http.reconn.trial.num = 100
PRODUCT_PROPERTY_OVERRIDES += tcc.http.disconn.at.full = 0

PRODUCT_COPY_FILES := \
	device/telechips/common/factory/init.factory.rc:root/init.factory.rc

PRODUCT_COPY_FILES += \
	device/telechips/common/factory/res/images/test_image0.png:root/res/images/test_image0.png	\
	device/telechips/common/factory/res/images/test_image1.png:root/res/images/test_image1.png	\
	device/telechips/common/factory/res/images/test_image2.png:root/res/images/test_image2.png	\
	device/telechips/common/factory/res/images/test_image3.png:root/res/images/test_image3.png	\
	device/telechips/common/factory/res/images/test_image4.png:root/res/images/test_image4.png	\
	device/telechips/common/factory/res/images/test_image5.png:root/res/images/test_image5.png	\
	device/telechips/common/factory/res/images/test_image6.png:root/res/images/test_image6.png

PRODUCT_COPY_FILES += \
	device/telechips/common/factory/res/fonts/HMFMOLD_18.bdf:system/fonts/HMFMOLD_18.bdf			\
	device/telechips/common/factory/res/fonts/VeraMonoMCC_16.bdf:system/fonts/VeraMonoMCC_16.bdf
