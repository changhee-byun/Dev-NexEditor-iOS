LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ANIGIF_TOP := $(LOCAL_PATH)
NDK_LOCAL_TOP := $(ANDROID_NDK_ROOT)
TARGET_PLATFORM := android-21

LOCAL_CFLAGS += -Wno-format -D_USE_LONGLONG_TO_INT64

ifeq ($(TARGET_ARCH),arm)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/lib
LOCAL_MULTILIB := 32
endif

ifeq ($(TARGET_ARCH),arm64)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/lib

endif


LOCAL_C_INCLUDES += $(ANIGIF_TOP)/inc

LOCAL_MODULE := libnxAniGifjni
LOCAL_SRC_FILES := native-jni.cpp
LOCAL_LDLIBS := -llog

ifeq ($(TARGET_ARCH),arm)
LOCAL_LDFLAGS += 	-L$(ANIGIF_TOP)/libs -lNxAnimatedGIF_release_armv7  -lNxYUVtoRGB_android_armv7_release
else
LOCAL_LDFLAGS += 	-L$(ANIGIF_TOP)/libs -lNxAnimatedGIF_release_armv8 -lNxYUVtoRGB_android_armv8_release
endif
include $(BUILD_SHARED_LIBRARY)