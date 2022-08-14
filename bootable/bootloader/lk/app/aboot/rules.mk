LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LK_TOP_DIR)/platform/tcc_shared/include

OBJS += \
	$(LOCAL_DIR)/aboot.o \
	$(LOCAL_DIR)/fastboot.o \
	$(LOCAL_DIR)/recovery.o

ifeq ($(TCC_SNAPSHOT_USE_LZ4),1)
OBJS += \
	$(LOCAL_DIR)/lz4/tcc_memcpy.Ao \
	$(LOCAL_DIR)/lz4/lz4_decompress.o \
	$(LOCAL_DIR)/lz4/lz4_load.o 

LIBS += $(LOCAL_DIR)/../../snapshot/libsnapshot.a
#LDFLAGS += $(LOCAL_DIR)/../../snapshot/libsnapshot.a
endif
