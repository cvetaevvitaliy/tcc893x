LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/tcc_shared

PLATFORM := tcc892x

#------------------------------------------------------------------
# Define board revision
# 0x2000 = TCC8803_M805S_D3_08x4_V0.1(Change to TCC89 - DDR3_512MB  (32Bit)
# 0x2001 = TCC8803/8923_M805S_D3_08x4_V1.1            - DDR3_1024MB (32Bit)
# 0x2002 = TCC8925_M805S_D3_08x4_2CS_V0.1             - DDR3_1024MB (16Bit)
# 0x2003 = TCC8925_M805S_D3_08X4_2CS_V1.2 2012.03.15  - DDR3_1024MB (16Bit)
# 0x2004 = TCC8925_M805S_D3_08x2_4L_V0.1A             - DDR3_512MB  (16Bit)
# 0x2005 = TCC8925_M805S_D3_08X4_2CS_V1.4A 2012.05.08 - DDR3_1024MB (16Bit)
# 0x2006 = TCC8925_M805S_D3_08X2_4L_V1.0 2012.06.26   - DDR3_512MB  (16Bit)
# 0x2007 = TCC8925_M805S_D3_08X4(2CS)_4L_V0.1 2012.7.3- DDR3_1024MB (16Bit)
# 0x2008 = TCC8925X_35_M805S_D3_08X4_V0.1 2012.8.17   - DDR3_1024MB (16Bit)
# 0x2009 = TCC8925X_35_M805S_D3_08X4_V0.11 2012.9.20  - DDR3_1024MB (16Bit)
# 0x2009 = TCC8925X_35_M805S_D3_08X4_V1.2 2013.03.26  - DDR3_1024MB (16Bit)
#HW_REV=0x2000
#HW_REV=0x2001
#HW_REV=0x2002
#HW_REV=0x2003
#HW_REV=0x2004
#HW_REV=0x2005
#HW_REV=0x2006
#HW_REV=0x2007
#HW_REV=0x2008
HW_REV=0x2009

#==================================================================
# Chipset Information
#==================================================================
#------------------------------------------------------------------
# CHIPSET TYPE
#------------------------------------------------------------------
#DEFINES += CONFIG_CHIP_TCC8920
DEFINES += CONFIG_CHIP_TCC8925
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
SCRATCH_ADDR     := BASE_ADDR+0x06000000


#==================================================================
# SDRAM Setting
#==================================================================
#------------------------------------------------------------------
# SDRAM CONTROLLER TYPE
#------------------------------------------------------------------
#TCC_MEM_TYPE := DRAM_LPDDR2
#TCC_MEM_TYPE := DRAM_DDR2
TCC_MEM_TYPE := DRAM_DDR3

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
	dev/pmic/axp202 \
	dev/pmic/tcc270 \
	lib/ptable \
	lib/libfdt

DEFINES += \
	MEMSIZE=$(MEMSIZE) \
	MEMBASE=$(MEMBASE) \
	BASE_ADDR=$(BASE_ADDR) \
	TAGS_ADDR=$(TAGS_ADDR) \
	KERNEL_ADDR=$(KERNEL_ADDR) \
	RAMDISK_ADDR=$(RAMDISK_ADDR) \
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

#DEFINES += AXP192_PMIC
DEFINES += AXP202_PMIC
#DEFINES += TCC270_PMIC

