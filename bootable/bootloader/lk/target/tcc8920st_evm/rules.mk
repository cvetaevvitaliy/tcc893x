LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LK_TOP_DIR)/platform/tcc_shared

PLATFORM := tcc892x

MEMBASE	:= 0x82000000
MEMSIZE := 0x01000000 # 16MB

#------------------------------------------------------------------
# Define board revision
# 0x6000 : STB    /TCC8920 /DDR3 512MB (16BIT) /None
# 0x6020 : STB    /TCC8920 /DDR3 512MB (32BIT) /None
# 0x6030 : STB    /TCC8920 /DDR3 1024MB(32BIT) /None
# 0x6040 : STB    /TCC8920 /DDR2 512MB (32BIT) /None
# 0x6050 : STB    /TCC8920 /DDR2 1024MB(32BIT) /None
# 0x7100 : YAOJIN /TCC8925 /DDR3 512MB (16BIT) /None(1CS)
# 0x7110 : YAOJIN /TCC8925 /DDR3 1024MB(16BIT) /None(2CS)
# 0x8100 : UPC    /TCC8925 /DDR3 512MB (16BIT) /None
# 0x8110 : UPC    /TCC8925 /DDR3 1024MB(16BIT) /None
# 0x9100 : DONGLE /TCC8925 /DDR3 512MB (16BIT) /None
# 0x9101 : DONGLE /TCC8925 /DDR3 512MB (16BIT) /'S' version
# 0x9110 : DONGLE /TCC8925 /DDR3 1024MB(16BIT) /None
# 0x9111 : DONGLE /TCC8925 /DDR3 1024MB (16BIT) /'S' version
# 0x9112 : DONGLE /TCC8925 /DDR3 1024MB (16BIT) /eMMC(BROADCOM)
HW_REV=0x6030
#HW_REV=0x7100
#HW_REV=0x8110
#HW_REV=0x9100
#HW_REV=0x9112


#==================================================================
# Chipset Information
# #==================================================================
# #------------------------------------------------------------------
# # CHIPSET TYPE
# #------------------------------------------------------------------
ifneq ($(filter $(HW_REV),0x6000, 0x6020 0x6030 0x6040 0x6050),)
DEFINES += CONFIG_CHIP_TCC8920
endif

ifneq ($(filter $(HW_REV),0x7100 0x7110 0x8100 0x8110 0x9100 0x9110 0x9112),)
DEFINES += CONFIG_CHIP_TCC8925
endif

ifneq ($(filter $(HW_REV),0x9101 0x9111),)
DEFINES += CONFIG_CHIP_TCC8925S
endif

#------------------------------------------------------------------
# SDRAM CONTROLLER TYPE
#------------------------------------------------------------------
ifneq ($(filter $(HW_REV),0x6040 0x6050),)
 TCC_MEM_TYPE := DRAM_DDR2
else
 ifneq ($(filter $(HW_REV),0x6020 0x6030 0x6000 0x7100 0x7110 0x9100 0x9101 0x9112 0x9110 0x9111 0x8100 0x8110),)
  TCC_MEM_TYPE := DRAM_DDR3
 else
  #TCC_MEM_TYPE := DRAM_LPDDR2
  TCC_MEM_TYPE := DRAM_DDR2
  #TCC_MEM_TYPE := DRAM_DDR3
 endif
endif

#------------------------------------------------------------------
# Define memory bus width

ifneq ($(filter $(HW_REV),0x6000 0x7100 0x7110 0x9100 0x9101 0x9112 0x9110 0x9111 0x8100 0x8110),)
 DEFINES += CONFIG_DRAM_16BIT_USED
else
 DEFINES += CONFIG_DRAM_32BIT_USED
endif


#------------------------------------------------------------------
# Define memory size in MB
ifneq ($(filter $(HW_REV),0x6040 0x6020 0x6000 0x7100 0x9100 0x9101 0x8100),)
 TCC_MEM_SIZE := 512
else
 ifneq ($(filter $(HW_REV),0x6050 0x6030 0x7110 0x9110 0x9111 0x9112 0x8110),)
  TCC_MEM_SIZE := 1024
 else
  TCC_MEM_SIZE := 512
 endif
endif

ifeq ($(TCC_MEM_SIZE), 256)
 DEFINES += CONFIG_TCC_MEM_256MB
endif
ifeq ($(TCC_MEM_SIZE), 512)
 DEFINES += CONFIG_TCC_MEM_512MB
endif
ifeq ($(TCC_MEM_SIZE), 1024)
 DEFINES += CONFIG_TCC_MEM_1024MB
endif

#------------------------------------------------------------------
# SNAPSHOT BOOT Option
#------------------------------------------------------------------
ifeq ($(TCC_SNAPSHOT_USE),1)
DEFINES += CONFIG_SNAPSHOT_BOOT
endif

#------------------------------------------------------------------ 
# GPU Scaling Option 
#------------------------------------------------------------------
#DEFINES += CONFIG_GPU_BUS_SCALING

#------------------------------------------------------------------
# Define target board
ifneq ($(filter $(HW_REV),0x6000, 0x6020 0x6030 0x6040 0x6050),)
 DEFINES += TARGET_TCC8920_EV
endif

ifneq ($(filter $(HW_REV),0x7100 0x7110),)
 DEFINES += TARGET_TCC8925_HDB892F
endif

ifneq ($(filter $(HW_REV),0x8100 0x8110),)
  DEFINES += TARGET_TCC8925_UPC
# DEFINES += HDMI_USE_INTERNAL_PLL
endif
ifneq ($(filter $(HW_REV),0x9100 0x9101 0x9110 0x9111 0x9112),)
 DEFINES += TARGET_TCC8925_STB_DONGLE
endif

ifneq ($(filter $(HW_REV),0x9100 0x9101 0x9110 0x9111 0x9112),)
# DEFINES += HDMI_USE_INTERNAL_PLL
endif

# Define if target board is STB
TARGET_BOARD_STB := true
DEFINES += BOARD_TCC892X_STB_DEMO

BASE_ADDR       := 0x80000000

#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR2)
 ifneq ($(filter $(HW_REV),0x6040),)
  DEFINES += CONFIG_DDR2_HY5PS1G831CFPS6
 else
  ifneq ($(filter $(HW_REV),0x6050),)
   DEFINES += CONFIG_DDR2_HY5PS1G831CFPS6 #should be change to fit some memory..
  else
   #DEFINES += CONFIG_DDR2_HXB18T2G160AF
   #DEFINES += CONFIG_DDR2_HY5PS1G1631CFPS6
   DEFINES += CONFIG_DDR2_HY5PS1G831CFPS6
   #DEFINES += CONFIG_DDR2_K4T1G164QE_HCF7
   #DEFINES += CONFIG_DDR2_K4T1G084QF_BCE7
   #DEFINES += CONFIG_DDR2_CT83486C1
   #DEFINES += CONFIG_DDR2_MT47H256M8EB25E
  endif
 endif

 DEFINES += DRAM_DDR2
endif

#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_DDR3)
 ifneq ($(filter $(HW_REV),0x6020),)
  DEFINES += CONFIG_DDR3_H5TQ1G83BFR_H9C
 else
  ifneq ($(filter $(HW_REV),0x6030 0x6000 0x9100 0x9101),)
   DEFINES += CONFIG_DDR3_H5TQ2G83BFR_H9C
   #DEFINES += CONFIG_DDR3_H5TQ2G83BFR_PBC
  else
   ifneq ($(filter $(HW_REV), 0x9110 0x9111 0x9112),)
     #DEFINES += CONFIG_DDR3_H5TC4G83MFR_H9A
     DEFINES += CONFIG_DDR3_H5TQ4G83AFR_PBC
   else
    ifneq ($(filter $(HW_REV), 0x7100 0x7110 0x8100 0x8110),)
     DEFINES += CONFIG_DDR3_H5TQ2G83BFR_PBC
    else
     DEFINES += CONFIG_DDR3_K4B2G1646C_HCK0
     #DEFINES += CONFIG_DDR3_H5TQ2G63BFR_H9C
     #DEFINES += CONFIG_DDR3_K4B1G1646E_HCH9
     #DEFINES += CONFIG_DDR3_H5TQ2G83BFR_H9C
     #DEFINES += CONFIG_DDR3_H5TQ2G83BFR_PBC
     #DEFINES += CONFIG_DDR3_H5TQ1G83BFR_H9C
     #DEFINES += CONFIG_DDR3_IN5CB256M8BN_CG
     #DEFINES += CONFIG_DDR3_NT5CB256M8GN_CG
     #DEFINES += CONFIG_DDR3_H5TC4G83MFR_H9A
    endif
   endif
  endif
 endif

 DEFINES += DRAM_DDR3
endif

#------------------------------------------------------------------
ifeq ($(TCC_MEM_TYPE), DRAM_LPDDR2)
 DEFINES += CONFIG_LPDDR2_K4P4G324EB_AGC1

 DEFINES += DRAM_LPDDR2
endif

#------------------------------------------------------------------
ifeq ($(TARGET_BOARD_STB),true)
 DEFINES += TARGET_BOARD_STB
 DISP_DEFINES := DISPLAY_STB
else
 DISP_DEFINES := DISPLAY_NORMAL
endif

#------------------------------------------------------------------
# Support video displaying by hw vsyn interrupt
#------------------------------------------------------------------
TCC_VIDEO_DISPLAY_BY_VSYNC_INT := true
#TCC_VIDEO_DISPLAY_BY_VSYNC_INT := false

ifeq ($(TCC_VIDEO_DISPLAY_BY_VSYNC_INT), true)
DEFINES += TCC_VIDEO_DISPLAY_BY_VSYNC_INT
endif

#------------------------------------------------------------------
TAGS_ADDR        := BASE_ADDR+0x00000100
KERNEL_ADDR      := BASE_ADDR+0x00008000
RAMDISK_ADDR     := BASE_ADDR+0x01000000
SKERNEL_ADDR     := BASE_ADDR+0x04000000
QB_SCRATCH_ADDR  := BASE_ADDR+0x06000000 # 2MB
SCRATCH_ADDR     := BASE_ADDR+0x06200000

#------------------------------------------------------------------
# Keypad Type
#------------------------------------------------------------------
KEYS_USE_GPIO_KEYPAD := 1
#KEYS_USE_ADC_KEYPAD := 1

#------------------------------------------------------------------
MODULES += \
	dev/keys \
	dev/pmic/axp192 \
	dev/gpio/pca953x \
	lib/ptable \
	lib/libfdt


DEFINES += \
	SDRAM_SIZE=$(MEMSIZE) \
	MEMSIZE=$(MEMSIZE) \
	MEMBASE=$(MEMBASE) \
	BASE_ADDR=$(BASE_ADDR) \
	TAGS_ADDR=$(TAGS_ADDR) \
	KERNEL_ADDR=$(KERNEL_ADDR) \
	RAMDISK_ADDR=$(RAMDISK_ADDR) \
	SCRATCH_ADDR=$(SCRATCH_ADDR) \
	QB_SCRATCH_ADDR=$(QB_SCRATCH_ADDR) \
	SKERNEL_ADDR=$(SKERNEL_ADDR) \
	TCC_MEM_SIZE=$(TCC_MEM_SIZE) \
	HW_REV=$(HW_REV)

# Defines debug level
DEBUG := 1

#------------------------------------------------------------------
# Defines Default Display
ifeq ($(DISP_DEFINES), DISPLAY_STB)
DEFINES += DEFAULT_DISPLAY_OUTPUT_DUAL
DEFINES += HDMI_1280X720
else
DEFINES += DEFAULT_DISPLAY_LCD
endif

#------------------------------------------------------------------
# Defines Display Type
ifeq ($(DISP_DEFINES), DISPLAY_STB)
 ifneq ($(filter $(HW_REV),0x6000, 0x6020 0x6030 0x6040 0x6050),)
  DEFINES += DISPLAY_STB_NORMAL
  #DEFINES += DISPLAY_STB_AUTO_HDMI_CVBS
  #DEFINES += DISPLAY_STB_ATTACH_HDMI_CVBS
  #DEFINES += DISPLAY_STB_ATTACH_DUAL_AUTO
 endif

 ifneq ($(filter $(HW_REV),0x7100 0x7110),)
  DEFINES += DISPLAY_STB_NORMAL
 endif

 ifneq ($(filter $(HW_REV),0x8100 0x8110),)
  DEFINES += DISPLAY_STB_NORMAL
 endif

 ifneq ($(filter $(HW_REV),0x9100 0x9101 0x9110 0x9111 0x9112),)
  DEFINES += DISPLAY_STB_NORMAL
 endif
endif

#------------------------------------------------------------------
# Define for composite output
ifneq ($(filter $(HW_REV),0x6000, 0x6020 0x6030 0x6040 0x6050 0x7100 0x7110),)
 DEFINES += DISPLAY_SUPPORT_COMPOSITE
endif

#------------------------------------------------------------------
# Define for component output
ifneq ($(filter $(HW_REV),0x6000, 0x6020 0x6030 0x6040 0x6050),)
 DEFINES += DISPLAY_SUPPORT_COMPONENT
endif

#------------------------------------------------------------------
# Define for component chip
ifeq ($(DISP_DEFINES), DISPLAY_STB)
DEFINES += COMPONENT_CHIP_THS8200
else
DEFINES += COMPONENT_CHIP_CS4954
endif

#------------------------------------------------------------------
# Define Default Splash
DEFINES += DISPLAY_SPLASH_SCREEN=1
#DEFINES += DISPLAY_SPLASH_SCREEN_DIRECT=1

TCC_SPLASH_USE := 1

#------------------------------------------------------------------
# Define scaler use for displaying logo
DEFINES += DISPLAY_SCALER_USE

#------------------------------------------------------------------
#Define Factory Reset
ifeq ($(DISP_DEFINES), DISPLAY_STB)
DEFINES += TCC_FACTORY_RESET_SUPPORT
endif

#------------------------------------------------------------------
OBJS += \
	$(LOCAL_DIR)/clock.o \
	$(LOCAL_DIR)/init.o \
	$(LOCAL_DIR)/keypad.o \
	$(LOCAL_DIR)/gpio.o \
	$(LOCAL_DIR)/target_display.o \
	$(LOCAL_DIR)/atags.o

#------------------------------------------------------------------
# Defines Remocon
#BOOTING_BY_REMOTE_KEY := true
RECOVERY_BY_REMOTE_KEY := true
#START_FWDN_BY_REMOTE_KEY := true

ifeq ($(BOOTING_BY_REMOTE_KEY), true)
DEFINES += BOOTING_BY_REMOTE_KEY
endif
ifeq ($(RECOVERY_BY_REMOTE_KEY), true)
DEFINES += RECOVERY_BY_REMOTE_KEY
endif
ifeq ($(START_FWDN_BY_REMOTE_KEY), true)
DEFINES += START_FWDN_BY_REMOTE_KEY
endif

ifneq ($(filter true,$(BOOTING_BY_REMOTE_KEY) $(RECOVERY_BY_REMOTE_KEY) $(START_FWDN_BY_REMOTE_KEY)),)
#DEFINES += CONFIG_CS_2000_IR_X_CANVAS
DEFINES += CONFIG_YAOJIN_IR

#DEFINES += PBUS_DIVIDE_CLOCK          # only tcc892x
DEFINES += PBUS_DIVIDE_CLOCK_WITH_XTIN # only tcc893x
endif

ifeq ($(BOOTING_BY_REMOTE_KEY), true)
PM_SUSPEND_USE := 1
else
PM_SUSPEND_USE := 0
endif

TCC_HDMI_USE := 1

ifeq ($(TCC_HDMI_USE), 1)
  DEFINES += TCC_HDMI_USE
endif

#------------------------------------------------------------------
# SNAPSHOT BOOT Option
#------------------------------------------------------------------
#TCC_QUICKBOOT_USE := 1

ifeq ($(TCC_QUICKBOOT_USE),1)
DEFINES += CONFIG_SNAPSHOT_BOOT
DEFINES += CONFIG_SNAPSHOT_CHECKSUM

TCC_SNAPSHOT_USE_LZ4 := 1
endif

#DEFINES += ENABLE_EMMC_TZOS
