#-------------------------------------------------------------#
# Copyright (c) 2009 Telechips Inc.                           #
# ALL RIGHTS RESERVED                                         #
#-------------------------------------------------------------#

DEF_TARGET := libtnftl_v8_$(TCC_ARCH)_KN.lo 

DEF_OBJECT_tnftl := tnftl_v8.o
DEF_OBJECT_tnftl += tnftl_v8_fw.o
DEF_OBJECT_tnftl += tnftl_v8_rand.o
DEF_OBJECT_tnftl += tnftl_v8_debug.o

include ../rules_base.mk
include ./rules_header.mk

$(DEF_TARGET): $(DEF_OBJECT_tnftl)
