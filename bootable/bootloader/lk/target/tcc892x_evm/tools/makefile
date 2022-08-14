#Makefile to generate lk.rom

TOOLS_DIR  := target/$(TARGET)/tools
KEYS_DIR := ../../../vendor/telechips/tools/tcsb/sign_key
SIGNTOOLS_DIR := ../../../vendor/telechips/tools/tcsb

ifeq ($(TSBM_INCLUDE),1)
ifeq ($(EMMC_BOOT),1)
BOOTLOADER_FILE := $(BUILDDIR)/EMMC_TCSB.rom
else
BOOTLOADER_FILE := $(BUILDDIR)/NAND_TCSB.rom
endif
else
BOOTLOADER_FILE := $(BUILDDIR)/lk.rom
endif

# For Secure Boot Start
DRAMINIT := $(BUILDDIR)/dram.rom
NAND_BL1 := $(BUILDDIR)/nand_bl1.rom
SEOUT_FILE := $(BUILDDIR)/lkse.rom
SIGNTOOL :=$(SIGNTOOLS_DIR)/tcsb_signtool
KEYPAIR :=$(KEYS_DIR)/tcsb_keypair.der
WRAPKEY:=$(KEYS_DIR)/tcsb_sbcr.bin
EMMC_HEADER := $(BUILDDIR)/emmc_header.tsi
NAND_HEADER := $(BUILDDIR)/nand_header.tsi
# For Secure Boot End

APPSBOOTHEADER: $(BOOTLOADER_FILE)

ifeq ($(TSBM_INCLUDE),1)
ifeq ($(TRUSTZONE_ENABLE),1)

TZOS := platform/tcc892x/tzos/tzos.rom

ifeq ($(EMMC_BOOT),1)
$(BUILDDIR)/EMMC_TCSB.rom: $(OUTBIN) $(BUILDDIR)/tcc_crc $(BUILDDIR)/tcsb_mkblimg
	$(NOECHO)echo generating $@
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg extract $(OUTBIN) $(DRAMINIT)
	$(NOECHO) cat $(TZOS) >> $(OUTBIN)
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg tzmend $(OUTBIN) $(TZOS)
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(DRAMINIT) -outfile $(BUILDDIR)/secure_dram.tsi
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(OUTBIN) -outfile $(BUILDDIR)/secure_bootloader.tsi
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg mkheader $(OUTBIN) $(BUILDDIR)/secure_dram.tsi $(BUILDDIR)/secure_bootloader.tsi  $(EMMC_HEADER)
	$(NOECHO) cat $(EMMC_HEADER) > $(BUILDDIR)/EMMC_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_dram.tsi  >> $(BUILDDIR)/EMMC_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_bootloader.tsi >> $(BUILDDIR)/EMMC_TCSB.rom
	$(NOECHO) rm -f $(DRAMINIT) $(BUILDDIR)/*.tsi
endif # EMMC BOOT END

ifeq ($(NAND_BOOT),1)
$(BUILDDIR)/NAND_TCSB.rom: $(OUTBIN) $(BUILDDIR)/tcc_crc $(BUILDDIR)/tcsb_mkblimg
	$(NOECHO)echo generating $@
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg extract $(OUTBIN) $(DRAMINIT) $(NAND_BL1)
	$(NOECHO) cat $(TZOS) >> $(OUTBIN)
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg tzmend $(OUTBIN) $(TZOS)
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(NAND_BL1) -outfile $(BUILDDIR)/secure_nand_bl1.tsi
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(DRAMINIT) -outfile $(BUILDDIR)/secure_dram.tsi
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(OUTBIN) -outfile $(BUILDDIR)/secure_bootloader.tsi
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg mkheader $(OUTBIN) $(NAND_BL1) $(BUILDDIR)/secure_dram.tsi $(BUILDDIR)/secure_bootloader.tsi $(BUILDDIR)/secure_nand_bl1.tsi $(NAND_HEADER)
	$(NOECHO) cat $(NAND_HEADER) > $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_dram.tsi  >> $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_bootloader.tsi >> $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_nand_bl1.tsi  >> $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) rm -f $(DRAMINIT) $(BUILDDIR)/*.tsi
endif # NAND BOOT END

else # !TRUSTZONE

ifeq ($(EMMC_BOOT),1)
$(BUILDDIR)/EMMC_TCSB.rom: $(OUTBIN) $(BUILDDIR)/tcc_crc $(BUILDDIR)/tcsb_mkblimg
	$(NOECHO)echo generating $@
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg extract $(OUTBIN) $(DRAMINIT) 
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(DRAMINIT) -outfile $(BUILDDIR)/secure_dram.tsi 
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(OUTBIN) -outfile $(BUILDDIR)/secure_bootloader.tsi 
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg mkheader $(OUTBIN) $(BUILDDIR)/secure_dram.tsi $(BUILDDIR)/secure_bootloader.tsi $(EMMC_HEADER)
	$(NOECHO) cat $(EMMC_HEADER) > $(BUILDDIR)/EMMC_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_dram.tsi  >> $(BUILDDIR)/EMMC_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_bootloader.tsi >> $(BUILDDIR)/EMMC_TCSB.rom
	$(NOECHO) rm -f $(DRAMINIT) $(BUILDDIR)/*.tsi
endif # EMMC BOOT END

ifeq ($(NAND_BOOT),1)
$(BUILDDIR)/NAND_TCSB.rom: $(OUTBIN) $(BUILDDIR)/tcc_crc $(BUILDDIR)/tcsb_mkblimg
	$(NOECHO)echo generating $@
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg extract $(OUTBIN) $(DRAMINIT) $(NAND_BL1)
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(NAND_BL1) -outfile $(BUILDDIR)/secure_nand_bl1.tsi 
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(DRAMINIT) -outfile $(BUILDDIR)/secure_dram.tsi 
	$(NOECHO)$(SIGNTOOL) sign -type BLw -signalgo ecdsa224 -hashalgo sha224 -prikey $(KEYPAIR) -wrapkey $(WRAPKEY) -infile $(OUTBIN) -outfile $(BUILDDIR)/secure_bootloader.tsi 
	$(NOECHO)$(BUILDDIR)/tcsb_mkblimg mkheader $(OUTBIN) $(NAND_BL1) $(BUILDDIR)/secure_dram.tsi $(BUILDDIR)/secure_bootloader.tsi $(BUILDDIR)/secure_nand_bl1.tsi $(NAND_HEADER)
	$(NOECHO) cat $(NAND_HEADER) > $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_dram.tsi  >> $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_bootloader.tsi >> $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) cat $(BUILDDIR)/secure_nand_bl1.tsi  >> $(BUILDDIR)/NAND_TCSB.rom
	$(NOECHO) rm -f $(DRAMINIT) $(BUILDDIR)/*.tsi

endif # NAND BOOT END

endif # TRUSTZONE_END

else
$(BUILDDIR)/lk.rom: $(OUTBIN) $(BUILDDIR)/tcc_crc
	$(NOECHO)echo generating $@
	$(NOECHO)$(BUILDDIR)/tcc_crc -v __BL -o $(BOOTLOADER_FILE) $(OUTBIN)
endif

							    
