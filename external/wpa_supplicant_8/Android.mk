ifeq ($(BOARD_USES_REALTEK_WIFI),true)
    include $(CURDIR)/external/wpa_supplicant_8/realtek/wpa_supplicant/Android.mk
    include $(CURDIR)/external/wpa_supplicant_8/realtek/hostapd/Android.mk
else
ifeq ($(BOARD_USES_BROADCOM_WIFI),true)
    include $(CURDIR)/external/wpa_supplicant_8/wpa_supplicant/Android.mk
    include $(CURDIR)/external/wpa_supplicant_8/hostapd/Android.mk
endif
endif

ifeq ($(WPA_SUPPLICANT_VERSION),VER_2_1_DEVEL)
include $(call all-subdir-makefiles)
endif
