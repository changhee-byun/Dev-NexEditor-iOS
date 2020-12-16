LOCAL_PATH:= $(call my-dir)

#####################################
# libnexralbody_audio
#####################################

include $(CLEAR_VARS)

RALBODY_TOP := $(LOCAL_PATH)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	utils/utils.cpp \
	ALManager.cpp \
	utils/jni.cpp \
	external/jni/jni_Audio.cpp \
	AudioRenderer.cpp

LOCAL_SHARED_LIBRARIES :=

LOCAL_C_INCLUDES := \
	$(RALBODY_TOP) \
	$(RALBODY_TOP)/external \
	$(RALBODY_TOP)/external/jni \
	$(NEXTREAMING_INCLUDES)

LOCAL_CFLAGS := -fvisibility=hidden

LOCAL_LDFLAGS += -L$(NDK_PLATFORM_LIB) \
					-ldl \
					-llog

LOCAL_STATIC_LIBRARIES += \
	libnexsound_new \
	libnexsal \
	libnexsalbody \
	libstlport_static

ifneq ($(NEX_REMOVE_LIBMEDIA_DEPENDENCY),0)
	LOCAL_CFLAGS += -D_REMOVE_LIBMEDIA_DEPENDENCY_=1
endif

ifneq ($(NEX_USE_SINK_THREAD),0)
	LOCAL_CFLAGS += -D_USE_SINK_THREAD_
endif

ifneq ($(NEX_EXPORT_MORE),0)
	LOCAL_CFLAGS += -DNEX_EXPORT_MORE=1
endif

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libnexralbody_audio

include $(BUILD_SHARED_LIBRARY)

# If you build only nexralbody_audio module, you must include salbody and sal.
#include $(RALBODY_TOP)/../../nexsalbody/Android_ndk.mk
#include $(RALBODY_TOP)/../../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/build/android/Android_ndk.mk