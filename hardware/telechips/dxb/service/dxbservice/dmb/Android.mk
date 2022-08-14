LOCAL_PATH:= $(call my-dir)

#=============================================================================
# libdmbserver
#=============================================================================
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE    := false

LOCAL_C_INCLUDES        += $(TCCDXB_TOP)/middleware/lib_tdmb/include/player
LOCAL_C_INCLUDES        += $(TCCDXB_TOP)/service/libdxbservice/include
LOCAL_C_INCLUDES        += frameworks/native/include/media/hardware
LOCAL_C_INCLUDES        += frameworks/native/include/media/openmax

LOCAL_MODULE            := libdmbserver
LOCAL_MODULE_TAGS       := eng debug

LOCAL_SHARED_LIBRARIES  += libcutils
LOCAL_SHARED_LIBRARIES  += libutils
LOCAL_SHARED_LIBRARIES  += libbinder
LOCAL_SHARED_LIBRARIES  += libdxbservice
LOCAL_SHARED_LIBRARIES  += libtdmb

LOCAL_SRC_FILES         += DMBService.cpp
LOCAL_SRC_FILES         += main_dmbserver.cpp

include $(BUILD_SHARED_LIBRARY)
#=============================================================================
