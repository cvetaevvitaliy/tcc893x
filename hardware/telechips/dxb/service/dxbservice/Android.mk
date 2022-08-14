LOCAL_PATH:= $(call my-dir)

#=============================================================================
# dxbserver
#=============================================================================
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE    := false

LOCAL_MODULE            := tcc_dxb_service
LOCAL_MODULE_TAGS       := eng debug

LOCAL_SHARED_LIBRARIES  += libutils
LOCAL_SHARED_LIBRARIES  += libbinder
LOCAL_SHARED_LIBRARIES  += libcutils
LOCAL_SHARED_LIBRARIES  += libdmbserver
#LOCAL_SHARED_LIBRARIES  += libdvbserver
#LOCAL_SHARED_LIBRARIES  += libisdbserver

LOCAL_SRC_FILES         += main_dxbserver.cpp

include $(BUILD_EXECUTABLE)
#=============================================================================

include $(call first-makefiles-under, $(LOCAL_PATH))
