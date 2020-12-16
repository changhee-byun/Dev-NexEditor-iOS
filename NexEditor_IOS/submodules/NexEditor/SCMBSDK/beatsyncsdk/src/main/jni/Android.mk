LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

BEAT_TOP := $(LOCAL_PATH)
NDK_LOCAL_TOP := $(ANDROID_NDK_ROOT)
TARGET_PLATFORM := android-21

LOCAL_CFLAGS += -Wno-multichar -Wno-format -D_USE_LONGLONG_TO_INT64 -DBUILD_NDK
LOCAL_CPPFLAGS += -std=c++11

ifeq ($(TARGET_ARCH),arm)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/lib
LOCAL_MULTILIB := 32
endif

ifeq ($(TARGET_ARCH),arm64)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/lib

endif

LOCAL_C_INCLUDES += $(NDK_PLATFORM_PATH) $(BEAT_TOP)/inc $(NDK_LOCAL_TOP)/sources/cxx-stl/stlport/stlport

LOCAL_MODULE := libnxBeatSyncjni
LOCAL_SRC_FILES := native-jni.cpp

LOCAL_LDLIBS := -L$(NDK_PLATFORM_LIB) -llog -lstdc++ -landroid -lm

ifeq ($(TARGET_ARCH),arm)
LOCAL_LDFLAGS += 	-L$(BEAT_TOP)/libs -lMusicHL_Android_ARMv7a_Release -lBeatSyncAssetMaker_ARMv7a_Release -lBeatTrackerRF_ARMv7a_Release -lNexBeatDetector_ARMv7a_Release
else
LOCAL_LDFLAGS += 	-L$(BEAT_TOP)/libs -lMusicHL_Android_ARMv8a_Release -lBeatSyncAssetMaker_ARMv8a_Release -lBeatTrackerRF_ARMv8a_Release -lNexBeatDetector_ARMv8a_Release
endif

include $(BUILD_SHARED_LIBRARY)