LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

SCMB_TOP := $(LOCAL_PATH)
NDK_LOCAL_TOP := $(ANDROID_NDK_ROOT)
TARGET_PLATFORM := android-21

LOCAL_CFLAGS += -Wno-format -D_USE_LONGLONG_TO_INT64 -DUSER_LITTLE_ENDIAN

ifeq ($(TARGET_ARCH),arm)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/lib
LOCAL_MULTILIB := 32
endif

ifeq ($(TARGET_ARCH),arm64)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/lib

endif


LOCAL_C_INCLUDES += $(SCMB_TOP)/inc

LOCAL_MODULE := libnxSCMBjni
LOCAL_SRC_FILES := native-jni.cpp NexSAL.c NexSAL_Trace.c SALBody_File.cpp SALBody_Debug.cpp SALBody_Mem.cpp SALBody_SyncObj.cpp SALBody_Task.cpp SALBody_Time.cpp KISA_SHA256.c
LOCAL_LDLIBS := -llog

ifeq ($(TARGET_ARCH),arm)
LOCAL_LDFLAGS += 	-L$(SCMB_TOP)/libs -lNxMETA_android_arm_release
else
LOCAL_LDFLAGS += 	-L$(SCMB_TOP)/libs -lNxMETA_android_armv8a_release
endif
include $(BUILD_SHARED_LIBRARY)