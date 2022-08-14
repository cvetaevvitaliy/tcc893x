LOCAL_PATH:= $(call my-dir)

#=============================================================================
# libdxbservice
#=============================================================================
include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE    := false

LOCAL_C_INCLUDES        += $(LOCAL_PATH)/include

LOCAL_MODULE            := libdxbservice
LOCAL_MODULE_TAGS       := optional

LOCAL_SHARED_LIBRARIES  += libbinder
LOCAL_SHARED_LIBRARIES  += libgui
LOCAL_SHARED_LIBRARIES  += libutils
LOCAL_SHARED_LIBRARIES  += liblog

LOCAL_SRC_FILES         += src/IDxbPlayerService.cpp
LOCAL_SRC_FILES         += src/IDxbPlayer.cpp
LOCAL_SRC_FILES         += src/IDxbPlayerClient.cpp
LOCAL_SRC_FILES         += src/DxbPlayerClient.cpp
LOCAL_SRC_FILES         += src/DMBPlayerClient.cpp
#LOCAL_SRC_FILES         += src/DVBPlayerClient.cpp
#LOCAL_SRC_FILES         += src/ISDBPlayerClient.cpp

include $(BUILD_SHARED_LIBRARY)
#=============================================================================
