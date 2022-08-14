#ifdef _DO_NOT_BUILD_
LOCAL_PATH:= $(call my-dir)

#
# libvoipsound
#

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -D_POSIX_SOURCE

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES :=           \
	VoipHardwareStub.cpp    
	

LOCAL_SHARED_LIBRARIES := liblog libmedia libcutils libtinyalsa libaudioutils libdl
	
	
LOCAL_C_INCLUDES := \
	external/tinyalsa/include


LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libvoipsound

include $(BUILD_SHARED_LIBRARY)
#endif
