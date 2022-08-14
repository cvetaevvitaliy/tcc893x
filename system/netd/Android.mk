LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=                                      \
                  BandwidthController.cpp              \
                  ClatdController.cpp                  \
                  CommandListener.cpp                  \
                  DnsProxyListener.cpp                 \
                  FirewallController.cpp               \
                  IdletimerController.cpp              \
                  InterfaceController.cpp              \
                  MDnsSdListener.cpp                   \
                  NatController.cpp                    \
                  NetdCommand.cpp                      \
                  NetdConstants.cpp                    \
                  NetlinkHandler.cpp                   \
                  NetlinkManager.cpp                   \
                  PppController.cpp                    \
                  ResolverController.cpp               \
                  SecondaryTableController.cpp         \
                  TetherController.cpp                 \
                  oem_iptables_hook.cpp                \
                  UidMarkMap.cpp                       \
                  main.cpp                             \


ifeq ($(BOARD_USES_ATH_WIFI), true)
    LOCAL_SRC_FILES += AthSoftapController.cpp
else ifeq ($(BOARD_USES_REALTEK_WIFI), true)
    #LOCAL_SRC_FILES += SoftapController_realtek.cpp 
    LOCAL_SRC_FILES += SoftapController.cpp
else ifeq ($(BOARD_USES_BROADCOM_WIFI), true)
    LOCAL_SRC_FILES += SoftapController.cpp
	LOCAL_CFLAGS += -DFIRMWARE_PATH
ifdef WIFI_DRIVER_FW_PATH_PARAM
    LOCAL_CFLAGS += -DWIFI_DRIVER_FW_PATH_PARAM=\"$(WIFI_DRIVER_FW_PATH_PARAM)\"
endif
ifdef WIFI_DRIVER_FW_PATH_STA
    LOCAL_CFLAGS += -DWIFI_DRIVER_FW_PATH_STA=\"$(WIFI_DRIVER_FW_PATH_STA)\"
endif
else
    LOCAL_SRC_FILES += SoftapController.cpp
endif

LOCAL_MODULE:= netd

LOCAL_C_INCLUDES := $(KERNEL_HEADERS) \
                    external/mdnsresponder/mDNSShared \
                    external/openssl/include \
                    external/stlport/stlport \
                    bionic \
                    bionic/libc/private \
                    $(call include-path-for, libhardware_legacy)/hardware_legacy

LOCAL_CFLAGS += -Werror=format

LOCAL_SHARED_LIBRARIES := libstlport libsysutils liblog libcutils libnetutils \
                          libcrypto libhardware_legacy libmdnssd libdl \
                          liblogwrap

ifneq ($(BOARD_HOSTAPD_DRIVER),)
  LOCAL_CFLAGS += -DHAVE_HOSTAPD
ifeq ($(WLAN_VENDOR), 2)
  LOCAL_CFLAGS += -DATH_WLAN
endif
endif

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:=          \
                  ndc.c \

LOCAL_MODULE:= ndc

LOCAL_C_INCLUDES := $(KERNEL_HEADERS)

#LOCAL_CFLAGS := 

LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_EXECUTABLE)
