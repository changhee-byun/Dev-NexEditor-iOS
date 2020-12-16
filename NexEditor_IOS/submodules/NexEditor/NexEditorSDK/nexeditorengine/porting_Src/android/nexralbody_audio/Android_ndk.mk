LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

USE_NDK := 1

ifeq ($(TARGET_ARCH),arm)
ARCH_DIR := armeabi
else
ARCH_DIR := x86
endif

TOP_PATH := $(LOCAL_PATH)

include $(TOP_PATH)/../../../extern_lib_version.mk

NEXTREAMING_INCLUDES := \
	$(TOP_PATH)/../../../extern_lib/nexCAL/$(NEX_CAL_VERSION)/inc \
	$(TOP_PATH)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc \
	$(TOP_PATH)/../../../extern_lib/nexRAL/$(NEX_RAL_VERSION)/inc \
	$(TOP_PATH)/../../../extern_lib/NexSound/$(NEX_SOUND_VERSION) \
	$(TOP_PATH)/../../../extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
	$(TOP_PATH)/../../../extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
	$(TOP_PATH)/../../../extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
	$(TOP_PATH)/../nexsalbody/inc \
	$(TOP_PATH)/inc

include $(CLEAR_VARS)

include $(TOP_PATH)/audio/Android.mk