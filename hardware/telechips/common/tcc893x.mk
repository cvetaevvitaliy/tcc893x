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

# This file lists the modules that are specific to all TCC devices.

PRODUCT_PACKAGES := \
	audio.primary.tcc893x \
	libaudioutils

PRODUCT_PACKAGES += \
	gralloc.tcc893x \
	hwcomposer.tcc893x \
	sensors.tcc893x \
	nfc.tcc893x \
	lights.tcc893x \
	power.tcc893x \
	camera.tcc893x	\
	gps.tcc893x
	
