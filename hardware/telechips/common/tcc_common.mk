# Copyright (C) 2011 Telechips, Inc.
# #
# # Licensed under the Apache License, Version 2.0 (the "License");
# # you may not use this file except in compliance with the License.
# # You may obtain a copy of the License at
# #
# #      http://www.apache.org/licenses/LICENSE-2.0
# #
# # Unless required by applicable law or agreed to in writing, software
# # distributed under the License is distributed on an "AS IS" BASIS,
# # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# # See the License for the specific language governing permissions and
# # limitations under the License.
#
# # This file lists the modules that are specific to all TCC devices.

PRODUCT_PACKAGES := \
	libstagefrighthw \
	libvoipsound \
	libortp \
	libTCCWifiDisplaySourceClient \
	libwfdsource \
	libwfdomxbasecomp \
	libwfdomxtranscodercomp \
	wfd_source_service

PRODUCT_PACKAGES += \
	libTCC_OMXCore \
	libtcc.video.call.interface \
	libtcc.video.call.direct.interface \
	libpmap \
	libDirect.Decoder.Wrapper \
	libDirect.Encoder.Wrapper \
	libDirect.Render.Wrapper \
	libDirect.Player.Wrapper

PRODUCT_PACKAGES += \
	libTCCsubtitle \
	libTCCInterSubtitle

PRODUCT_PACKAGES += \
	libTCC_CDK_AUDIO \
	libTCC_CDK_LIB \
	libTCC_CDK_CONFIG \
	libTCC_CDK_WRAPPER

include hardware/telechips/common/tcc_container_support.mk
include hardware/telechips/common/tcc_video_support.mk
include hardware/telechips/common/tcc_audio_support.mk
