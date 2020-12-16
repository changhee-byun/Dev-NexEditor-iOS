LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional


LOCAL_SRC_FILES := hmac.c \
		lsh.c \
		lsh256.c \
		lsh512.c

LOCAL_C_INCLUDES := .


ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS += -Wno-format  -D_x86
else
LOCAL_CFLAGS += -Wno-format  -DARM
LOCAL_ARM_MODE := arm
endif

# To hide symbol
LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_MODULE    := libkisalsh

include $(BUILD_STATIC_LIBRARY)

