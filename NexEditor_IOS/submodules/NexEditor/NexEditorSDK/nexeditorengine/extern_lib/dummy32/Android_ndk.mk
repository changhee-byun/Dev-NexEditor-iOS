
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := dummy32bit.c 

LOCAL_CFLAGS += -Wno-format -DHAVE_CONFIG_H

# To hide symbol
LOCAL_CFLAGS += -fvisibility=hidden

ifeq ($(TARGET_ARCH),arm)
LOCAL_ARM_MODE := arm
else ifeq ($(TARGET_ARCH),arm64)
LOCAL_ARM_MODE := arm
else
endif


LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE := libdummy32

include $(BUILD_SHARED_LIBRARY)