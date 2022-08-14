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

# This file lists the modules that are specific to DxB.

# DTV library
PRODUCT_PACKAGES := \
	libOMX.TCC.DxB.base \
	libOMX.TCC.fbdevsink \
	libOMX.TCC.tcc317x.Tuner \
	libOMX.TCC.TCC351X.Tuner \
	libOMX.TCC.tcc353x.Tuner \
	libOMX.TCC.DxB.VPUDec \
	libOMX.TCC.DxB.VideoDec \
	libOMX.TCC.DxB.AudioDec \
	libOMX.TCC.bsacdec \
	libOMX.TCC.alsasink \
	libOMX.TCC.LinuxTV.Demux \
	libOMX.TCC.LinuxTV.PVR \
	libOMX.TCC.IPTVDemux \
	libOMX.TCC.tdmbdemux \
	libOMX.TCC.PADParser \
	libOMX.TCC.tdmbrec \
	libOMX.TCC.ISDBTTuner \
	libOMX.TCC.ATSCTuner \
	libOMX.TCC.MxL101SF.Tuner \
	libOMX.TCC.MN88472.Tuner \
	libOMX.TCC.LinuxTV.Tuner \
	libOMX.TCC.Tuner.interface \
	libdxbutils \
	libTCC_CDK_DXB_LIB \
	libTCCDxBInterface \
	libdxbsc \
	libdxbfont \
	libisdbt_region \
	libdxbcipher \
	libdvbt \
	libtdmb \
	libtdmb_jni \
	libiptv \
	libiptv_jni \
	libisdbt \
	libisdbt_jni \
	libdvrdvbt_jni \
	libdvbt_freqspace \
	libatsc \
	libTCC.DxB.aacdec \
	libTCC.DxB.ac3dec \
	libTCC.DxB.bsacdec \
	libTCC.DxB.mp2dec \
	libTCC.DxB.mp3dec \
	libTCC.DxB.aacenc \
	libTCC.DxB.mp3enc \
	libTCC.DxB.latmdmx \


# DTV xml
PRODUCT_PACKAGES += \
	com.telechips.android.iptv.xml \
	com.telechips.android.isdbt.xml

# DTV jar
PRODUCT_PACKAGES += \
	com.telechips.android.iptv \
	com.telechips.android.isdbt

# DTV dump
PRODUCT_PACKAGES += \
	iptvdump \
	dvbtdump \
	tdmbdump \
	isdbtdump \
	tdmbtpeg \

# DTV service
PRODUCT_PACKAGES += \
	tcc_dxb_service

# DTV apk
PRODUCT_PACKAGES += \
	TDMBPlayer \
	SampleIPTVPlayer \
	ISDBTPlayer \
	DVBPlayer	\
	dtvDumpActivity \
	SampleATSCPlayer

# tcc.dxb.dvbt.baseband : Define dvbt baseband type 1:dvbs2(linuxtv fe) 2:tcc351x_spi 3:tcc351x_i2c 4:mxl101sf
# tcc.dxb.linuxtv.enable : 1:Enbale linuxtv 0:Disable linuxtv
PRODUCT_PROPERTY_OVERRIDES += \
	tcc.dxb.dvbt.baseband = 1 \
	tcc.dxb.linuxtv.enable = 1 \
	tcc.dxb.ringbuffer.enable = 0 \
	tcc.dxb.standard = 2
