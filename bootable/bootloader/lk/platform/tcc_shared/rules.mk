LOCAL_DIR := $(GET_LOCAL_DIR)

DEFINES += PLATFORM_TCC=1
DEFINES += ANDROID_BOOT=1
DEFINES += WITH_DMA_ZONE=1
DMA_SIZE := 0x00800000 # 8MB
DEFINES += _DMA_SIZE=$(DMA_SIZE)

DEFINES += $(TARGET_XRES)
DEFINES += $(TARGET_YRES)

INCLUDES += -I$(LOCAL_DIR)/include -I$(LOCAL_DIR)/include/fwdn -I$(LOCAL_DIR)/include/tnftl -I$(LOCAL_DIR)/include/tcc_remocon  -I$(LOCAL_DIR)/include/usb3.0 -I$(LOCAL_DIR)/include/tcsb -I$(LOCAL_DIR)/include/splash

OBJS += \
	$(LOCAL_DIR)/debug.o \
	$(LOCAL_DIR)/timer.o \
	$(LOCAL_DIR)/tca_tco.o \
	$(LOCAL_DIR)/sfl.o\
	$(LOCAL_DIR)/partition_parser.o \
	$(LOCAL_DIR)/component_cs4954.o \
	$(LOCAL_DIR)/component_ths8200.o 

OBJS += \
	$(LOCAL_DIR)/usb_device.o \
	$(LOCAL_DIR)/usbdev_class.o \
	$(LOCAL_DIR)/usbphy.o \
	$(LOCAL_DIR)/vtc.o \
	$(LOCAL_DIR)/fastbootusb.o \
	$(LOCAL_DIR)/hsusb.o \
	$(LOCAL_DIR)/usb_manager.o

ifeq ($(TCC_USB_30_USE), 1)
OBJS += \
	$(LOCAL_DIR)/usb3.0/usb30dev.o \
	$(LOCAL_DIR)/usb3.0/usb30reset.o \
	$(LOCAL_DIR)/usb3.0/pcd.o \
	$(LOCAL_DIR)/usb3.0/pcd_intr.o \
	$(LOCAL_DIR)/usb3.0/cil.o \
	$(LOCAL_DIR)/usb3.0/cil_intr.o
else
OBJS += \
	$(LOCAL_DIR)/otgcore.o \
	$(LOCAL_DIR)/otgdev_io.o 
endif

OBJS += \
	$(LOCAL_DIR)/nand.o

OBJS += \
	$(LOCAL_DIR)/fwdn_drv_v7.o \
	$(LOCAL_DIR)/fwdn_main.Ao \
	$(LOCAL_DIR)/fwdn_nand.o \
	$(LOCAL_DIR)/disk.o \
	$(LOCAL_DIR)/fwdn_protocol_v7.o

OBJS += \
	$(LOCAL_DIR)/lcd/lcd_lms480kf01.o \
	$(LOCAL_DIR)/lcd/lcd_tw8816.o \
	$(LOCAL_DIR)/lcd/lcd_dx08d11vm0aaa.o \
	$(LOCAL_DIR)/lcd/lcd_lms350df01.o \
	$(LOCAL_DIR)/lcd/lcd_td043mgeb1.o \
	$(LOCAL_DIR)/lcd/lcd_td070rdh.o \
	$(LOCAL_DIR)/lcd/lcd_at070tn93.o \
	$(LOCAL_DIR)/lcd/lcd_claa104xa01cw.o\
	$(LOCAL_DIR)/lcd/lcd_n101l6.o\
	$(LOCAL_DIR)/lcd/lcd_ht121wx2.o\
	$(LOCAL_DIR)/lcd/lcd_claa102na0dcw.o \
	$(LOCAL_DIR)/lcd/lcd_ED090NA.o\
	$(LOCAL_DIR)/lcd/lcd_kr080pa2s.o\
	$(LOCAL_DIR)/lcd/lcd_hv070wsa.o\
	$(LOCAL_DIR)/lcd/lcd_claa070np01.o\
	$(LOCAL_DIR)/lcd/lcd_FLD0800.o\
	$(LOCAL_DIR)/lcd/lcd_CLAA070WP03.o\
	$(LOCAL_DIR)/lcd/lcd_lms700kf23.o\
	$(LOCAL_DIR)/lcd/lcd_EJ070NA.o\
	$(LOCAL_DIR)/lcd/lcd_lp101wx1.o

ifeq ($(TCC_HDMI_USE), 1)
OBJS += \
	$(LOCAL_DIR)/hdmi_1280x720.o
endif

ifneq ($(filter true,$(BOOTING_BY_REMOTE_KEY) $(RECOVERY_BY_REMOTE_KEY) $(START_FWDN_BY_REMOTE_KEY)),)
OBJS+= \
	$(LOCAL_DIR)/tcc_remocon.o
endif

ifeq ($(EMMC_BOOT),1)
OBJS+= \
	$(LOCAL_DIR)/emmc.o \
	$(LOCAL_DIR)/sd_memory.o \
	$(LOCAL_DIR)/sd_hw.o \
	$(LOCAL_DIR)/sd_bus.o \
	$(LOCAL_DIR)/sd_slot.o \
	$(LOCAL_DIR)/sd_update.o \
	$(LOCAL_DIR)/fwdn_sdmmc.o \
	$(LOCAL_DIR)/update.o
endif

ifeq ($(TCC_SPLASH_USE), 1)

OBJS += \
	$(LOCAL_DIR)/splashimg.o

endif

include ../../../hardware/telechips/nand_v8/Makefile

include platform/tcc_shared/tools/Makefile
include platform/tcc_shared/tcsb/Makefile
