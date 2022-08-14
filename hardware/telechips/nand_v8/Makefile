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

ifneq ($(LK_TOP_DIR),)
# LK part of makefile
LOCAL_DIR := $(GET_LOCAL_DIR)
else
LOCAL_DIR :=
endif

INCLUDES += -I$(LOCAL_DIR)

OBJS += \
	$(LOCAL_DIR)/tnftl_v8_glue.o \
	$(LOCAL_DIR)/nand_utils.o \
	$(LOCAL_DIR)/nand_io_v8.o \
	$(LOCAL_DIR)/nand_drv_v8.o \
	$(LOCAL_DIR)/nand_board_v8.o \
	$(LOCAL_DIR)/fwupgrade.o

#OBJS += \
#	$(LOCAL_DIR)/TNFTL_V8/tnftl_v8.o \
#	$(LOCAL_DIR)/TNFTL_V8/tnftl_v8_fw.o

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

ifneq ($(CONFIG_ARCH_TCC88XX),)
TCC_ARCH := TCC880X
DEFINES += -DTCC88XX -DTCC88X
endif

ifneq ($(CONFIG_ARCH_TCC92XX),)
TCC_ARCH := TCC8900
DEFINES += -DTCC89XX -DTCC89X
endif

ifneq ($(CONFIG_ARCH_TCC93XX),)
TCC_ARCH := TCC9300
DEFINES += -DTCC93XX -DTCC93X
endif

ifneq ($(CONFIG_ARCH_TCC892X),)
TCC_ARCH := TCC892X
DEFINES += -DTCC892X
endif

ifneq ($(CONFIG_CHIP_TCC8930),)
TCC_ARCH := TCC893X
DEFINES += -DTCC8930
endif

ifneq ($(CONFIG_CHIP_TCC8935),)
TCC_ARCH := TCC893X
DEFINES += -DTCC8935
endif

ifneq ($(CONFIG_CHIP_TCC8933),)
TCC_ARCH := TCC893X
DEFINES += -DTCC8933
endif

ifneq ($(CONFIG_CHIP_TCC8935S),)
TCC_ARCH := TCC893X
DEFINES += -DTCC8935
endif

ifneq ($(CONFIG_CHIP_TCC8933S),)
TCC_ARCH := TCC893X
DEFINES += -DTCC8933
endif

ifneq ($(CONFIG_CHIP_TCC8937S),)
TCC_ARCH := TCC893X
DEFINES += -DTCC8937
endif


DEFINES += \
	-D_LINUX_ \
	-DNAND_INCLUDE \
	-DNAND_BOOT_INCLUDE \
	-DTNFTL_V8_INCLUDE \
	-D$(TCC_ARCH) \
	-DANDROID_KERNEL \
	-DKERNEL_DRIVER

LIBS += \
	libtnftl/libtnftl_v8_$(TCC_ARCH)_KN.lo

EXTRA_CFLAGS += $(DEFINES)
EXTRA_CFLAGS += -I$(ANDROID_BUILD_TOP)/bootable/bootloader/lk/platform/tcc_shared/include/fwdn
ifeq ($(TCC_ARCH),TCC893X)
EXTRA_CFLAGS += -I$(ANDROID_BUILD_TOP)/bootable/bootloader/lk/platform/tcc893x/include
else
EXTRA_CFLAGS += -I$(ANDROID_BUILD_TOP)/bootable/bootloader/lk/platform/tcc892x/include
endif
EXTRA_CFLAGS += -I$(ANDROID_BUILD_TOP)/hardware/telechips/nand_v8

obj-m := tcc_nand_core.o tcc_nand.o

tcc_nand_core-objs := \
	$(OBJS) $(LIBS)

tcc_nand-objs := \
	tcc_ndd.o 

else
ifneq ($(LK_TOP_DIR),)
# LK part of makefile

else
# Normal Makefile

# Define host system directory
ifneq ($(ANDROID_BUILD_TOP),)
KDIR ?= $(ANDROID_BUILD_TOP)/kernel
else
KDIR ?= /lib/modules/$(shell uname -r)/build
endif

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
endif
