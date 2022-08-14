LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/tcc_shared

PLATFORM := tcc892x

#------------------------------------------------------------------
# Define board revision
# 0x1000 : TCC8920_D2_08X4_SV01 - DDR2 512MB(32Bit)
# 0x1001 : TCC8920_D2_08X4_SV01 - DDR2 1024MB(32Bit)
# 0x1002 : TCC8920_D3_08X4_SV01 - DDR3 512MB(32Bit)
# 0x1003 : TCC8920_D3_08X4_SV01 - DDR3 1024MB(32Bit)
# 0x1004 : TCC8920_D3_08X4_SV01 - DDR3 512MB(16Bit)
# 0x1005 : TCC8920_D3_16X4_2CS_SV01 - DDR3 1024MB (32Bit)
# 0x1006 : TCC8925_D3_08X4_2CS_SV01 - DDR3 1024MB (16Bit)
# 0x1007 : TCC8920_D3_08X4_SV60 - DDR3 1024MB(32Bit)
# 0x1008 : TCC8923_D3_08X4_SV01 - DDR3 1024MB(32Bit)
#HW_REV=0x1000
#HW_REV=0x1001
#HW_REV=0x1002
#HW_REV=0x1003
#HW_REV=0x1004
#HW_REV=0x1005
#HW_REV=0x1006
HW_REV=0x1007
#HW_REV=0x1008

#==================================================================
# Chipset Information
#==================================================================
#------------------------------------------------------------------
# CHIPSET TYPE
#------------------------------------------------------------------
DEFINES += CONFIG_CHIP_TCC8920
#DEFINES += CONFIG_CHIP_TCC8925
#DEFINES += CONFIG_CHIP_TCC8925S

#------------------------------------------------------------------
# BASE Address
#------------------------------------------------------------------
BASE_ADDR        := 0x80000000
MEMBASE          := 0x82000000
MEMSIZE          := 0x01000000 # 16MB

TAGS_ADDR        := BASE_ADDR+0x00000100
KERNEL_ADDR      := BASE_ADDR+0x00008000
RAMDISK_ADDR     := BASE_ADDR+0x01000000
SKERNEL_ADDR     := BASE_ADDR+0x04000000
QB_SCRATCH_ADDR  := BASE_ADDR+0x06000000 # 2MB
SCRATCH_ADDR     := BASE_ADDR+0x06200000


#==================================================================
# SDRAM Setting
#==================================================================
#------------------------------------------------------------------
# SDRAM CONTROLLER TYPE
#------------------------------------------------------------------
#TCC_MEM_TYPE := DRAM_LPDDR2
#TCC_MEM_TYPE := DRAM_DDR2
#TCC_MEM_TYPE := DRAM_DDR3
ifneq ($(filter $(HW_REV),0x1000 0x1001),)
TCC_MEM_TYPE := DRAM_DDR2
else
TCC_MEM_TYPE := DRAM_DDR3
endif

#------------------------------------------------------------------
# Define memory bus width
#------------------------------------------------------------------
ifneq ($(filter $(DEFINES),CONFIG_CHIP_TCC8925 CONFIG_CHIP_TCC8925S),)
 DEFINES += CONFIG_DRAM_16BIT_USED
else
 DEFINES += CONFIG_DRAM_32BIT_USED
endif

#------------------------------------------------------------------
# Define memory size in MB
#------------------------------------------------------------------
#TCC_MEM_SIZE := 512
TCC_MEM_SIZE := 1024
#TCC_MEM_SIZE := 2048

ifeq ($(TCC_MEM_SIZE), 256)
 DEFINES += CONFIG_TCC_MEM_256MB
endif
ifeq ($(TCC_MEM_SIZE), 512)
 DEFINES += CONFIG_TCC_MEM_512MB
endif
ifeq ($(TCC_MEM_SIZE), 1024)
 DEFINES += CONFIG_TCC_MEM_1024MB
endif
ifeq ($(TCC_MEM_SIZE), 2048)
 DEFINES += CONFIG_TCC_MEM_2048MB
endif

#------------------------------------------------------------------
# SDRAM DDR2 Vender
#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR2)
 #DEFINES += CONFIG_DDR2_CT83486C1
 DEFINES += CONFIG_DDR2_HY5PS1G831CFPS6
 #DEFINES += CONFIG_DDR2_HY5PS1G1631CFPS6
 #DEFINES += CONFIG_DDR2_HXB18T2G160AF
 #DEFINES += CONFIG_DDR2_K4T1G084QF_BCE7
 #DEFINES += CONFIG_DDR2_K4T1G164QE_HCF7
 #DEFINES += CONFIG_DDR2_MT47H256M8EB25E

 DEFINES += DRAM_DDR2
endif

#------------------------------------------------------------------
# SDRAM DDR3 Vender
#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR3)
 #DEFINES += CONFIG_DDR3_H5TQ4G83AFR_PBC
 #DEFINES += CONFIG_DDR3_H5TC4G83MFR_H9A
 #DEFINES += CONFIG_DDR3_H5TQ1G83BFR_H9C
 #DEFINES += CONFIG_DDR3_H5TQ2G63BFR_H9C
 #DEFINES += CONFIG_DDR3_H5TQ2G63BFR_PBC
 #DEFINES += CONFIG_DDR3_H5TQ2G83BFR_H9C
 DEFINES += CONFIG_DDR3_H5TQ2G83BFR_PBC
 #DEFINES += CONFIG_DDR3_IN5CB256M8BN_CG
 #DEFINES += CONFIG_DDR3_K4B1G1646E_HCH9
 #DEFINES += CONFIG_DDR3_K4B2G1646C_HCK0
 #DEFINES += CONFIG_DDR3_NT5CB256M8GN_CG
 #DEFINES += CONFIG_DDR3_PRN256M8V89CG8GQF_15E

 DEFINES += DRAM_DDR3
endif

#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_LPDDR2)
 DEFINES += CONFIG_LPDDR2_K4P4G324EB_AGC1

 DEFINES += DRAM_LPDDR2
endif

DEFINES += DEFAULT_DISPLAY_LCD
DEFINES += DISPLAY_SPLASH_SCREEN=1
DEFINES += DISPLAY_SPLASH_SCREEN_DIRECT=1
DEFINES += DISPLAY_TYPE_MIPI=1
#DEFINES += AT070TN93      # 800x480
DEFINES += FLD0800	  # 1024x600 
#DEFINES += LP101WX1	  # 1280x800
DEFINES += DISPLAY_TYPE_DSI6G=1


TCC_SPLASH_USE := 1

#------------------------------------------------------------------
# Keypad Type
#------------------------------------------------------------------
KEYS_USE_GPIO_KEYPAD := 1
#KEYS_USE_ADC_KEYPAD := 1

MODULES += \
	dev/keys \
	dev/pmic/axp192 \
	dev/gpio/pca953x \
	lib/ptable \
	lib/libfdt

DEFINES += \
	MEMSIZE=$(MEMSIZE) \
	MEMBASE=$(MEMBASE) \
	BASE_ADDR=$(BASE_ADDR) \
	TAGS_ADDR=$(TAGS_ADDR) \
	KERNEL_ADDR=$(KERNEL_ADDR) \
	RAMDISK_ADDR=$(RAMDISK_ADDR) \
	QB_SCRATCH_ADDR=$(QB_SCRATCH_ADDR) \
	SCRATCH_ADDR=$(SCRATCH_ADDR) \
	SKERNEL_ADDR=$(SKERNEL_ADDR) \
	TCC_MEM_SIZE=$(TCC_MEM_SIZE) \
	HW_REV=$(HW_REV)

#------------------------------------------------------------------
OBJS += \
	$(LOCAL_DIR)/clock.o \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/keypad.o \
	$(LOCAL_DIR)/gpio.o \
	$(LOCAL_DIR)/target_display.o \
	$(LOCAL_DIR)/atags.o

DEFINES += AXP192_PMIC
DEFINES += TCC_PCA953X_USE

#PM_SUSPEND_USE := 1

#------------------------------------------------------------------
# Snapshot Boot Option
#------------------------------------------------------------------
#TCC_QUICKBOOT_USE := 1

ifeq ($(TCC_QUICKBOOT_USE),1)
DEFINES += CONFIG_SNAPSHOT_BOOT
DEFINES += CONFIG_SNAPSHOT_CHECKSUM

TCC_SNAPSHOT_USE_LZ4 := 1
endif
