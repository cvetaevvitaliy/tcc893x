LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := \
	libc \
	libutils \
	libcutils \
	libbinder \
	libgui \
	liblog \
	libmedia \
	libdl \
	libui \
	libandroid_runtime \
	libnativehelper \
	libhardware_legacy \
	libdxbutils \
	libsqlite \
	libdxbcipher \
	libTCCDxBInterface


LOCAL_LDLIBS += \
	-lpthread \
	-ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
	-D_POSIX_SOURCE \
	$(DXB_CFLAGS)

LOCAL_LDFLAGS := \
	-L$(TCCDXB_TOP)/middleware/common/dxb_alticas/lib -lalticas
	
LOCAL_STATIC_LIBRARIES := \


LOCAL_C_INCLUDES := \
	$(JNI_H_INCLUDE) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/include/manager \
	$(LOCAL_PATH)/include/player \
	$(LOCAL_PATH)/include/socket \
	external/sqlite/dist \
	$(TCCDXB_TOP)/middleware/common/dxb_utils/include \
	$(TCCDXB_TOP)/middleware/common/dxb_tsdecoder/include \
	$(TCCDXB_TOP)/middleware/common/dxb_alticas/include \
	$(TCCDXB_TOP)/middleware/common/dxb_cipher/include \
	$(TCCDXB_TOP)/framework/dxb_interface/include/interface \
	$(DXB_C_INCLUDES)


LOCAL_SRC_FILES := \
	src/manager/tcc_iptv_manager_demux.c \
	src/manager/tcc_iptv_manager_audio.c \
	src/manager/tcc_iptv_manager_video.c \
	src/manager/tcc_iptv_manager_socket.c \
	src/socket/tcc_rtp.c \
	src/socket/tcc_socket.c \
	src/socket/tcc_socket_util.c \
	../common/dxb_alticas/src/tcc_alticas.c \
	../common/dxb_alticas/src/tcc_dxb_manager_alticas.c \
	src/tcc_iptv_proc.cpp \
	src/tcc_iptv_event.cpp \
	src/player/IPTVPlayer.cpp

LOCAL_SHARED_LIBRARIES += $(DXB_SHARED_LIBRARIES)


LOCAL_MODULE := libiptv
LOCAL_MODULE_TAGS := eng debug

include $(BUILD_SHARED_LIBRARY)	
include $(call first-makefiles-under,$(LOCAL_PATH))
