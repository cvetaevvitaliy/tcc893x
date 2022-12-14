LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=       \
        AAMRAssembler.cpp           \
        AAVCAssembler.cpp           \
        AH263Assembler.cpp          \
        AMPEG2TSAssembler.cpp       \
        AMPEG4AudioAssembler.cpp    \
        AMPEG4ElementaryAssembler.cpp \
        APacketSource.cpp           \
        ARawAudioAssembler.cpp      \
        ARTPAssembler.cpp           \
        ARTPConnection.cpp          \
        ARTPSource.cpp              \
        ARTPWriter.cpp              \
        ARTSPConnection.cpp         \
        ASessionDescription.cpp     \
        SDPLoader.cpp               \

LOCAL_C_INCLUDES:= \
	$(TOP)/frameworks/av/media/libstagefright/include \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/external/openssl/include

LOCAL_MODULE:= libstagefright_rtsp

LOCAL_WHOLE_STATIC_LIBRARIES := libtccnuplayer_rtsp

###################################################
# Telechips
ifeq ($(USE_TCC_NUPLAYER), true)
LOCAL_CPPFLAGS += -D_USE_TCC_NUPLAYER_=1
else
LOCAL_CPPFLAGS += -D_USE_TCC_NUPLAYER_=0
endif
# ###################################################

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_STATIC_LIBRARY)

################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES:=         \
        rtp_test.cpp

LOCAL_SHARED_LIBRARIES := \
	libstagefright liblog libutils libbinder libstagefright_foundation

LOCAL_STATIC_LIBRARIES := \
        libstagefright_rtsp

LOCAL_C_INCLUDES:= \
	frameworks/av/media/libstagefright \
	$(TOP)/frameworks/native/include/media/openmax

LOCAL_CFLAGS += -Wno-multichar

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= rtp_test

# include $(BUILD_EXECUTABLE)
