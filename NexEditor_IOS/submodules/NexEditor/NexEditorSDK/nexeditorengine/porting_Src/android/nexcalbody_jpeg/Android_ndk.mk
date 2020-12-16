LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

JPEG_TOP := $(LOCAL_PATH)

LOCAL_SRC_FILES := \
	nexCalBody_JPEG.cpp 

LOCAL_CFLAGS += -Wno-format -DHAVE_CONFIG_H

# To hide symbol
LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_C_INCLUDES := \
		$(JPEG_TOP)/../../../extern_lib/nexUtil/$(NEX_UTILS_VERSION)/inc \
		$(JPEG_TOP)/../../../extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
		$(JPEG_TOP)/../../../extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
		$(JPEG_TOP)/../../../extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
		$(JPEG_TOP)/../../../extern_lib/NexCodecUtils \
		$(JPEG_TOP)/../../../extern_lib/NexCAL/$(NEX_CAL_VERSION)/inc \
		$(JPEG_TOP)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc \
		$(JPEG_TOP)/../../inc

ifeq ($(TARGET_ARCH),arm)
LOCAL_ARM_MODE := arm
else ifeq ($(TARGET_ARCH),arm64)
LOCAL_ARM_MODE := arm
else
endif

LOCAL_MODULE := libnexcalbody_jpeg

include $(BUILD_STATIC_LIBRARY)

