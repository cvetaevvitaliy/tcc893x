LOCAL_PATH       := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE     := libjniTccif
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES  := \
	jniTccif.cpp \
	tccif_direct_sample.cpp \
	tccif_modified_stagefright_sample.cpp


LOCAL_C_INCLUDES := \
	hardware/telechips/common/tcc-interface/tccif \
	hardware/telechips/common/tcc-interface/tccif-direct \
	hardware/telechips/omx/omx_videodec_interface/include \
	hardware/telechips/omx/omx_videoenc_interface/include \
	kernel/arch/arm/mach-$(TARGET_BOARD_PLATFORM)/include \
	hardware/telechips/common/libcamera/include \
	hardware/libhardware/include \
	frameworks/base/include \
	frameworks/av/include/media \
	frameworks/av/media/libstagefright/include \
	frameworks/native/include/media/openmax \
	frameworks/base/include/media/stagefright

LOCAL_SHARED_LIBRARIES += \
	libbinder \
	libutils \
	libstagefright \
	libtcc.video.call.interface \
	libtcc.video.call.direct.interface \
	libcutils \
	libgui \
	libandroid_runtime

include $(BUILD_SHARED_LIBRARY)
