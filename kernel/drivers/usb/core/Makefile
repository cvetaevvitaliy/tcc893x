#
# Makefile for USB Core files and filesystem
#

ccflags-$(CONFIG_USB_DEBUG) := -DDEBUG

usbcore-y := usb.o hub.o hcd.o urb.o message.o driver.o
usbcore-y += config.o file.o buffer.o sysfs.o endpoint.o
usbcore-y += devio.o notify.o generic.o quirks.o devices.o

usbcore-$(CONFIG_PCI)		+= hcd-pci.o
usbcore-$(CONFIG_USB_DEVICEFS)	+= inode.o

obj-$(CONFIG_TCC_USB_DRD)   += drd_otg.o
obj-$(CONFIG_USB)		+= usbcore.o
