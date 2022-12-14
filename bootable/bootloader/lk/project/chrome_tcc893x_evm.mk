# top level project rules for the tcc893x_evm project
#
LOCAL_DIR := $(GET_LOCAL_DIR)

TARGET := tcc893x_evm

MODULES += app/aboot

DEBUG := 1
#DEFINES += WITH_DEBUG_DCC=1
DEFINES += WITH_DEBUG_UART=1
#DEFINES += WITH_DEBUG_FBCON=1
#DEFINES += DEVICE_TREE=1
#DEFINES += MMC_BOOT_BAM=1
#DEFINES += CRYPTO_BAM=1

#Disable thumb mode
ENABLE_THUMB := false

TCC893X := 1
EMMC_BOOT := 1

ifeq ($(EMMC_BOOT),1)
DEFINES += _EMMC_BOOT=1
endif

CHROME_BOOT := 1

ifeq ($(CHROME_BOOT),1)
DEFINES += _CHROME_BOOT=1
DEFINES += _CHROME_DUAL_BOOT=1
endif
