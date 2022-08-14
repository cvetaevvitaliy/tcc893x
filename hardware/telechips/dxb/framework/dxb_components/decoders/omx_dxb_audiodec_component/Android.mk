LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PRELINK_MODULE := false

SUPPORT_DDP := false

ifeq ($(SUPPORT_DDP),false)
  SUPPORT_AC3 := false
endif

LOCAL_SRC_FILES := \
    src/omx_spdif_component.c \
    src/omx_mp2dec_component.c \
    src/omx_aacdec_component.c
	


LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/spdif/include \
    $(TCCDXB_OMX_TOP)/decoders/omx_dxb_audiodec_interface/include \
    $(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/cdk \
    $(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/audio_codec \
    $(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/container \
    $(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_include \
    $(TCCDXB_OMX_TOP)/omx_engine/dxb_omx_base/include \

LOCAL_SHARED_LIBRARIES := \
    libc \
    libutils \
    libcutils \
    liblog \
    libOMX.TCC.DxB.base \
    libTCC_CDK_DXB_LIB \

LOCAL_WHOLE_STATIC_LIBRARIES := \
    libOMX.TCC.DxB.audio

LOCAL_STATIC_LIBRARIES := \
    libtccspdifparser

LOCAL_LDLIBS += \
    -lpthread \
    -ldl

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS := \
    $(TCC_OMX_FLAGS) \
    -DSYS_LINUX \
    $(DXB_CFLAGS)
	
LOCAL_LDFLAGS := \
    -L$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/audio_codec/mp2dec/lib \
    -L$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/audio_codec/aacdec/lib \
    -L$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/audio_codec/ac3dec/lib \
    -L$(TCCDXB_OMX_TOP)/decoders/dxb_cdk_library/audio_codec/ddpdec/lib

ifeq ($(TARGET_BOARD_PLATFORM),tcc92xx)	
LOCAL_LDFLAGS += \
    -lTCC89_92xx_MP2DEC_ANDROID_V4.07.03_CTS \
    -lTCC89_92xx_AACDEC_ANDROID_V3.18.03_CTS
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc93xx)	
LOCAL_LDFLAGS += \
    -lTCC93xx_MP2DEC_ANDROID_V4.07.03_CTS \
    -lTCC93xx_AACDEC_ANDROID_V3.18.03_CTS
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc88xx)	
LOCAL_LDFLAGS += \
    -lTCC88xx_MP2DEC_ANDROID_V4.07.03_CTS \
    -lTCC88xx_AACDEC_ANDROID_V3.18.03_Neon_CTS
endif

ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)	
LOCAL_LDFLAGS += \
    -lTCC892x_MP2DEC_ANDROID_V4.07.03 \
    -lTCC892x_AACDEC_ANDROID_V3.21.02_Neon
endif
ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)	
LOCAL_LDFLAGS += \
    -lTCC893x_MP2DEC_ANDROID_V4.07.05_NOPT \
    -lTCC893x_AACDEC_ANDROID_V3.21.02_Neon
endif

ifeq ($(SUPPORT_DDP),true)
  LOCAL_CFLAGS += -DSUPPORT_DDP_CODEC
  ifeq ($(TARGET_BOARD_PLATFORM),tcc88xx)
    LOCAL_LDFLAGS += -lTCC88xx_DDPDCV_ANDROID_V1.00.04
  endif

  ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
    LOCAL_LDFLAGS += -lTCC892x_DDPDCV_ANDROID_V1.00.04
  endif
  ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
    LOCAL_LDFLAGS += -lTCC892x_DDPDCV_ANDROID_V1.00.04
  endif
else
  ifeq ($(SUPPORT_AC3),true)
    LOCAL_CFLAGS += -DSUPPORT_AC3
      ifeq ($(TARGET_BOARD_PLATFORM),tcc92xx)
        LOCAL_LDFLAGS += \
          -lTCC89_92xx_AC3DEC_ANDROID_V2.12.02 \
          -lTCC92xx_AUDIO_OMFFT_ANDROID_V0.00	
      endif

      ifeq ($(TARGET_BOARD_PLATFORM),tcc93xx)
        LOCAL_LDFLAGS += \
          -lTCC93xx_AC3DEC_ANDROID_V2.12.02 \
          -lTCC_ARMv6_AUDIO_OMFFT_LINUX_V0.00
      endif

      ifeq ($(TARGET_BOARD_PLATFORM),tcc88xx)
        LOCAL_LDFLAGS += \
          -lTCC93xx_AC3DEC_ANDROID_V2.12.02 \
          -lTCC_ARMv6_AUDIO_OMFFT_LINUX_V0.00
      endif

      ifeq ($(TARGET_BOARD_PLATFORM),tcc892x)
        LOCAL_LDFLAGS += \
		  -lTCC892x_AC3DEC_ANDROID_V2.12.11_NoPT \
          -lTCC89_92xx_AUDIO_OMFFT_ANDROID_V0.00
      endif    
      ifeq ($(TARGET_BOARD_PLATFORM),tcc893x)
        LOCAL_LDFLAGS += \
		  -lTCC892x_AC3DEC_ANDROID_V2.12.11_NoPT \
          -lTCC89_92xx_AUDIO_OMFFT_ANDROID_V0.00
      endif    
  endif	
endif

LOCAL_MODULE := libOMX.TCC.DxB.AudioDec
LOCAL_MODULE_TAGS := eng debug

#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)

include $(call first-makefiles-under,$(LOCAL_PATH))
