#-------------------------------------------------------------#
# Copyright (c) 2009 Telechips Inc.                           #
# ALL RIGHTS RESERVED                                         #
#-------------------------------------------------------------#

# CROSS COMPILER
#CROSS_PREFIX:=/opt/armv6/codesourcery/bin/arm-none-linux-gnueabi-
#CROSS_PREFIX:= /opt/armv7/codesourcery/bin/arm-none-linux-gnueabi-
CROSS_PREFIX:= ../../../../../prebuilts/gcc/linux-x86/arm/arm-eabi-4.6/bin/arm-eabi-

# - [ Command rules ] --------------------------------------
ECHO:=echo
CC:=$(CROSS_PREFIX)gcc
LD:=$(CROSS_PREFIX)ld
AR:=$(CROSS_PREFIX)ar
AS:=$(CROSS_PREFIX)as

# - [ Option rules ] --------------------------------------
#DEF_ARM_ARCH := -march=armv6k -mtune=arm1176jzf-s -mfloat-abi=soft #-mfpu=vfp -mfloat-abi=softfp
DEF_CODE_GEN := -fno-strict-aliasing -fno-builtin -fno-common -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-short-enums
DEF_WARN := -Wundef -Wstrict-prototypes -Wno-trigraphs -Wdeclaration-after-statement

DEF_NAND := -DUSE_V_ADDRESS
#DEF_NAND += -DNAND_INCLUDE
#DEF_NAND += -DNAND_BOOT_INCLUDE
#DEF_NAND += -D_NAND_DEVICE_CONFIG_
#DEF_NAND += -DINTERNAL_HIDDEN_STORAGE_INCLUDE
#DEF_NAND += -DECC_TEST
DEF_NAND += -D_LINUX_

ifeq ($(TCC_ARCH), TCC880X)
DEF_NAND += -DTCC88XX
else ifeq ($(TCC_ARCH), TCC892X)
DEF_NAND += -DTCC892X
else ifeq ($(TCC_ARCH), TCC8925S)
DEF_NAND += -DTCC8925S
else ifeq ($(TCC_ARCH), TCC893X)
DEF_NAND += -DTCC893X
endif

DEF_CFLAGS_COMMON := $(DEF_ARM_ARCH) $(DEF_CODE_GEN) $(DEF_WARN) $(DEF_NAND) #-D__LINUX_ARM_ARCH__=6
DEF_CFLAGS_COMMON += -I../..
DEF_CFLAGS_COMMON += -Wall #-Werror
DEF_CFLAGS_COMMON += -O2 #-g
DEF_CFLAGS_COMMON += -fno-short-enums

DEF_CFLAGS_GNUC := $(DEF_CFLAGS_COMMON) $(DEF_INCLUDE_PATH) $(DEF_DEFINE) -c
DEF_CFLAGS_ANSI := $(DEF_CFLAGS_COMMON) -ansi $(DEF_INCLUDE_PATH) $(DEF_DEFINE) -c
DEF_LDFLAGS_SHARED := -s -shared
DEF_LDFLAGS_OBJECT := -s -r

DEF_ARFLAGS_STATIC := rc

# - [ Default rules ] --------------------------------------
ifeq ($(DEF_TARGET),none)

else

all: $(DEF_TARGET)
	cp $(DEF_TARGET) ../../libtnftl
	@$(ECHO) -e "[\x1b[1;33mOK\x1b[0m] $^"

clean:
	@$(ECHO) -e "[\x1b[1;31mRM\x1b[0m] *.o *.lo *.a"
	@find . -type f \( -name '*.o' -or -name '*.lo' -or -name '*.a' \) -print | xargs rm -f

endif


# End of make 
