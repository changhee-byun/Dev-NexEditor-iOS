LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

NEXRALBODY_AUDIO_PATH := $(LOCAL_PATH)

#####################################
# libnexralbody_audio
#####################################

ifeq ($(NEX_REMOVE_LIBMEDIA_DEPENDENCY),)
NEX_REMOVE_LIBMEDIA_DEPENDENCY := 0
endif

ifeq ($(NEX_USE_SINK_THREAD),)
NEX_USE_SINK_THREAD := 0
endif

ifeq ($(NEX_EXPORT_MORE),)
NEX_EXPORT_MORE := 0
endif

NEX_STATIC_BUILD :=
include $(NEXRALBODY_AUDIO_PATH)/Android-libnexralbody_audio.mk