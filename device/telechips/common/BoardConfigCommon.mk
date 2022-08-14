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

BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := device/generic/common/bluetooth

USE_TELECHIPS_PROPRIETARY_LIBRARIES := true
TARGET_GLOBAL_CFLAGS += -D_TCC_PROPRIETARY_DEPENDENCIES_ -DTCC_OMX
TARGET_GLOBAL_CPPFLAGS += -D_TCC_PROPRIETARY_DEPENDENCIES_ -DTCC_OMX

######################################################################################################################
# If you don't want to use TCC solution, please set this flag from "true" to "false".
# But, definitely we will not take responsibility for any problems caused by using your own solution instead of TCC's,
USE_TCC_AWESOMEPLAYER  := true
USE_TCC_MEDIAEXTRACTOR := true
USE_TCC_NUPLAYER       := true
######################################################################################################################

USE_TCC_SUBTITLE := true

BOARD_USES_PMAP := true

USE_QUICKBOOT := true 

USE_WIFI_DISPLAY := true

BOARD_HAVE_SPDIF := true
