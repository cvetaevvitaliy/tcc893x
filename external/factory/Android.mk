ifneq ($(filter tcc%,$(TARGET_BOARD_PLATFORM)),)

LOCAL_PATH := $(call my-dir)
HAVE_CUST_FOLDER := $(shell test -d device/telechips/common/factory && echo yes)
CUSTOM_PATH := device/telechips/common/factory
$(info $(CUSTOM_PATH) $(TARGET_DEVICE))

commands_factory_local_path := $(LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/factory.c \
	src/volume/roots.c \
	src/ui/ui.c \
	src/ui/factory_ui.c	\
	src/items/lcd_test.c	\
	src/items/item_reference.c

LOCAL_MODULE := factory

LOCAL_MODULE_TAGS := eng

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_STATIC_LIBRARIES :=

ifeq ($(TARGET_USERIMAGES_USE_EXT4), true)
LOCAL_CFLAGS += -DUSE_EXT4
LOCAL_C_INCLUDES += system/extras/ext4_utils \

LOCAL_WHOLE_STATIC_LIBRARIES += libext4_utils_static libz libsparse_static
endif

ifeq ($(TARGET_BOARD_HDMI_DONGLE),true)
LOCAL_CFLAGS += -DTCC_FACTORY_RESET_SUPPORT
endif

LOCAL_STATIC_LIBRARIES += libminzip libunz libmtdutils_factory libmincrypt
LOCAL_STATIC_LIBRARIES += libminui_factory libpixelflinger_static libpng libcutils liblog
LOCAL_STATIC_LIBRARIES += libstdc++ libc

LOCAL_C_INCLUDES += system/extras/ext4_utils \
	$(LOCAL_PATH)/src/include \
	$(LOCAL_PATH)/src/items 	\
	$(LOCAL_PATH)/src/minui	\
	$(LOCAL_PATH)/src/mtdutils

include $(BUILD_EXECUTABLE)

#copy etc/init.rc to rootfs/factory_init.rc for non-factory image mode
#$(shell cp $(CUSTOM_PATH)/init.rc $(LOCAL_PATH)/etc/init.rc)
#$(shell chmod 777 $(LOCAL_PATH)/etc/init.rc)

$(shell echo -e "\nservice factory /system/bin/logwrapper /system/bin/factory\n oneshot" >> $(LOCAL_PATH)/etc/init.rc)
include $(CLEAR_VARS)
LOCAL_MODULE := init.factory.rc
LOCAL_MODULE_TAGS := eng
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := $(TARGET_ROOT_OUT)

include $(commands_factory_local_path)/src/minui/Android.mk
include $(commands_factory_local_path)/src/mtdutils/Android.mk

############################################################################
endif
