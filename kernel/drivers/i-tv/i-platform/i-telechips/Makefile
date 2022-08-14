# 
# Telechips platform
#
ifneq ($(CONFIG_ARCH_TCC893X),)
itv-tccxxxx-objs := itccxxxx.o itccxxxx_tsif_cm3_p.o
else
itv-tccxxxx-objs := itccxxxx.o itccxxxx_tsif_p.o
endif

obj-$(CONFIG_iTV_PLATFORM_TCCXXXX) += itv-tccxxxx.o

EXTRA_CFLAGS += -Idrivers/spi
