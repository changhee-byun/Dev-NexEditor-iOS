LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

CALBODY_TOP := $(LOCAL_PATH)

LOCAL_SRC_FILES := \
	nexCalBody.cpp \
	nexQueue.cpp

LOCAL_CFLAGS += -Wno-format -DHAVE_CONFIG_H

# To hide symbol
LOCAL_CFLAGS += -fvisibility=hidden

ifeq ($(TARGET_ARCH),arm)
LOCAL_ARM_MODE := arm
else ifeq ($(TARGET_ARCH),arm64)
LOCAL_ARM_MODE := arm
else
endif


LOCAL_C_INCLUDES := \
		$(CALBODY_TOP)/../../../extern_lib/nexUtil/$(NEX_UTILS_VERSION)/inc \
		$(CALBODY_TOP)/../../../extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
		$(CALBODY_TOP)/../../../extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
		$(CALBODY_TOP)/../../../extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
		$(CALBODY_TOP)/../../../extern_lib/NexCodecUtils \
		$(CALBODY_TOP)/../../../extern_lib/nexCAL/$(NEX_CAL_VERSION)/inc \
		$(CALBODY_TOP)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc \
		$(CALBODY_TOP)/../../inc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libnexcalbody

include $(BUILD_STATIC_LIBRARY)

