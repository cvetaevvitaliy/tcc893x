LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := 			\
	libTSDecoderATSC_002.a		\
	libTSProcATSC_006.a			\
	libTCCClosedCaption_004.a


LOCAL_MODULE_TAGS := eng debug	
	
include $(BUILD_MULTI_PREBUILT)
