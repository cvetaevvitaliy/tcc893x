#
# Makefile for nfc devices
#

obj-y				+= proc_nfc.o
obj-$(CONFIG_PN544_NFC)		+= pn544.o
obj-$(CONFIG_NFC_PN533)		+= pn533.o
obj-$(CONFIG_NFC_WILINK)	+= nfcwilink.o

ccflags-$(CONFIG_NFC_DEBUG) := -DDEBUG
