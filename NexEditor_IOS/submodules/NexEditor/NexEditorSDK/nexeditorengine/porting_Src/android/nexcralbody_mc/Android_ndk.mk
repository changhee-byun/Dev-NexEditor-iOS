LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

NEXCRALBODY_MC_DEC_PATH := $(LOCAL_PATH)

#####################################
# libnexcralbody_mc
#####################################

ifeq ($(filter $(NEX_JNI_ACCESSORS), 1),)
NEX_JNI_ACCESSORS := 1
endif

ifeq ($(NEX_ADD_ALL),1)
NEX_ADD_OC := 1
NEX_ADD_NW := 1
NEX_ADD_AAC_SF := 1
NEX_ADD_MP3_SF := 1
NEX_ADD_AMRNB_SF := 1
NEX_ADD_AMRWB_SF := 1
NEX_ADD_OGG_SF := 1
endif

ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk

ifeq ($(NEX_ADD_OC),1)
BUILD_AS_OC := 1
ifeq ($(NEX_ADD_NW),1)
BUILD_AS_OC := 2
endif
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_OC :=
endif

ifeq ($(NEX_ADD_NW),1)
BUILD_AS_NW := 1
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_NW :=
endif

ifeq ($(NEX_ADD_AAC_SF),1)
BUILD_AS_AAC_SF := 1
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_AAC_SF :=
endif

ifeq ($(NEX_ADD_MP3_SF),1)
BUILD_AS_MP3_SF := 1
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_MP3_SF :=
endif

ifeq ($(NEX_ADD_AMRNB_SF),1)
BUILD_AS_AMRNB_SF := 1
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_AMRNB_SF :=
endif

ifeq ($(NEX_ADD_AMRWB_SF),1)
BUILD_AS_AMRWB_SF := 1
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_AMRWB_SF :=
endif

ifeq ($(NEX_ADD_OGG_SF),1)
BUILD_AS_OGG_SF := 1
ifeq ($(NEX_ADD_STATIC),1)
NEX_STATIC_BUILD := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_STATIC_BUILD :=
endif
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
BUILD_AS_OGG_SF :=
endif

ifeq ($(NEX_ADD_TEST),1)
NEX_BUILD_TEST := 1
include $(NEXCRALBODY_MC_DEC_PATH)/Android-libnexcralbody_mc.mk
NEX_BUILD_TEST :=
endif

