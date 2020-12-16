LOCAL_PATH:= $(call my-dir)

#####################################
# libnexcralbody_mc
#####################################

include $(CLEAR_VARS)

include $(NEXCRALBODY_MC_DEC_PATH)/nex_android_version.mk

ifeq ($(_NEX_ANDROID_PLATFORM_CODENAME_),)
_NEX_ANDROID_PLATFORM_CODENAME_ := DEFAULT
endif

APP_ALLOW_MISSING_DEPS=true

# this is temporary define.
_NEX_ANDROID_PLATFORM_CODENAME_ := JELLYBEAN

ifeq ($(_NEX_ANDROID_PLATFORM_CODENAME_), DEFAULT)
_TARGET_EXTENSION_ :=
endif
ifeq ($(_NEX_ANDROID_PLATFORM_CODENAME_), KITKAT)
_TARGET_EXTENSION_ := _kitkat
endif
ifeq ($(_NEX_ANDROID_PLATFORM_CODENAME_), JELLYBEAN_PLUS_422)
_TARGET_EXTENSION_ := _jbp422
endif
ifeq ($(_NEX_ANDROID_PLATFORM_CODENAME_), JELLYBEAN_PLUS)
_TARGET_EXTENSION_ := _jbp
endif
ifeq ($(_NEX_ANDROID_PLATFORM_CODENAME_), JELLYBEAN)
_TARGET_EXTENSION_ := _jb
endif

NDK_LOCAL_TOP := $(ANDROID_NDK_ROOT)

#####################################

LOCAL_MODULE_TAGS := optional

ifeq ($(NEX_DEBUG),)
NEX_DEBUG=7
endif

# utils/jni.cpp must be at the very top!
LOCAL_SRC_FILES := \
	utils/utils.cpp \
	utils/jni.cpp

ifeq ($(NEX_JNI_ACCESSORS),1)
LOCAL_SRC_FILES += \
	external/jni/jni_MediaCodec.cpp \
	external/jni/jni_MediaFormat.cpp \
	external/jni/jni_SurfaceTexture.cpp
endif

LOCAL_SRC_FILES += \
	utils/time.cpp \
	utils/debug.cpp \
	NexCALBody_mc.cpp \
	NexRALBody_mc.cpp \
	video/h263.cpp \
	video/h264.cpp \
	video/hevc.cpp \
	video/mpeg4v.cpp \
	video/mpeg2.cpp \
	video/vpx.cpp \
	audio/aac.cpp \
	audio/mp3.cpp \
	audio/amrnb.cpp \
	audio/amrwb.cpp \
	audio/vorbis.cpp \
	external/NexMediaCodec.cpp \
	external/recvd/recvd_SurfaceTexture.cpp

LOCAL_SHARED_LIBRARIES :=

LOCAL_LDFLAGS += -L$(NEXCRALBODY_MC_DEC_PATH)

ifneq ($(_NEX_ANDROID_PLATFORM_VERSION_),)
LOCAL_CFLAGS += -DNEX_ANDROID_PLATFORM_VERSION=$(_NEX_ANDROID_PLATFORM_VERSION_)
else
LOCAL_CFLAGS += -DNEX_ANDROID_PLATFORM_VERSION=0
endif

LOCAL_C_INCLUDES += \
	$(NEXCALBODY_MC_DEC_PATH)

# for <list>: (may not need for ndk)
LOCAL_C_INCLUDES += \
	bionic \
	external/stlport/stlport

LOCAL_C_INCLUDES += \
	$(NEXCRALBODY_MC_DEC_PATH)/../../../extern_lib/nexCAL/$(NEX_CAL_VERSION)/inc \
	$(NEXCRALBODY_MC_DEC_PATH)/../../../extern_lib/nexRAL/$(NEX_RAL_VERSION)/inc \
	$(NEXCRALBODY_MC_DEC_PATH)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc \
	$(NEXCRALBODY_MC_DEC_PATH)/inc

ifeq ($(TARGET_ARCH_VARIANT),armv7-a)
    LOCAL_CFLAGS += -DHAVE_NEON=1
#    LOCAL_LDFLAGS += -lneon_colorconversion
else
	LOCAL_CFLAGS += -DHAVE_NEON=0
endif

ifeq ($(NO_RAL),1)
LOCAL_CFLAGS += -DNO_RAL=1
endif

TMP_NEX_DEBUG_E := 0
TMP_NEX_DEBUG_D := 0
TMP_NEX_DEBUG_R := 0
TMP_NEX_DEBUG_T := 0

ifneq ($(NEX_DEBUG),)
TMP_NEX_DEBUG_E := $(NEX_DEBUG)
TMP_NEX_DEBUG_D := $(NEX_DEBUG)
TMP_NEX_DEBUG_R := $(NEX_DEBUG)
TMP_NEX_DEBUG_T := $(NEX_DEBUG)
endif

ifneq ($(NEX_DEBUG_E),)
TMP_NEX_DEBUG_E := $(NEX_DEBUG_E)
ifeq ($(NEX_DEBUG),)
NEX_DEBUG := 1
endif
endif

ifneq ($(NEX_DEBUG_D),)
TMP_NEX_DEBUG_D := $(NEX_DEBUG_D)
ifeq ($(NEX_DEBUG),)
NEX_DEBUG := 1
endif
endif

ifneq ($(NEX_DEBUG_R),)
TMP_NEX_DEBUG_R := $(NEX_DEBUG_R)
ifeq ($(NEX_DEBUG),)
NEX_DEBUG := 1
endif
endif

ifneq ($(NEX_DEBUG_T),)
TMP_NEX_DEBUG_T := $(NEX_DEBUG_T)
ifeq ($(NEX_DEBUG),)
NEX_DEBUG := 1
endif
endif

$(NEX_DEBUG = $(NEX_DEBUG))

ifneq ($(NEX_DEBUG),)
LOCAL_CFLAGS += -DNEX_DEBUG_MODE=$(NEX_DEBUG) -DNEX_DEBUG_E_MODE=$(TMP_NEX_DEBUG_E) -DNEX_DEBUG_D_MODE=$(TMP_NEX_DEBUG_D) -DNEX_DEBUG_R_MODE=$(TMP_NEX_DEBUG_R) -DNEX_DEBUG_T_MODE=$(TMP_NEX_DEBUG_T)
endif

ifneq ($(NEX_NO_LOGO),1)
LOCAL_CFLAGS += -D_SHOW_LOGO_
endif

ifeq ($(NEX_INPUT_DUMP),1)
LOCAL_CFLAGS += -D_NEX_INPUT_DUMP_=1
else
ifeq ($(NEX_INPUT_DUMP),2)
LOCAL_CFLAGS += -D_NEX_INPUT_DUMP_=2
endif
endif
ifeq ($(NEX_OUTPUT_DUMP),1)
LOCAL_CFLAGS += -D_NEX_OUTPUT_DUMP_=1
else
ifeq ($(NEX_OUTPUT_DUMP),2)
LOCAL_CFLAGS += -D_NEX_OUTPUT_DUMP_=2
endif
endif

LOCAL_LDFLAGS += -Wl,--exclude-libs,ALL


LOCAL_STATIC_LIBRARIES += \
	libstlport_static


LOCAL_LDFLAGS += \
	-landroid \
	-llog \
	-lGLESv2 \
	-lEGL \
	-ldl


LOCAL_CFLAGS += -Wno-multichar -Wint-to-pointer-cast 

LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libnexcralbody_mc$(_TARGET_EXTENSION_)

ifeq ($(BUILD_AS_OC),1)
LOCAL_CFLAGS += -DNEX_ADD_OC_DEFS=1
LOCAL_MODULE := libnexcal_oc$(_TARGET_EXTENSION_)
else
ifeq ($(BUILD_AS_OC),2)
LOCAL_CFLAGS += -DNEX_ADD_OC_DEFS=1
LOCAL_CFLAGS += -DNEX_ADD_NW_DEFS=1
LOCAL_MODULE := libnexcal_oc$(_TARGET_EXTENSION_)
endif
endif

ifeq ($(BUILD_AS_NW),1)
LOCAL_CFLAGS += -DNEX_ADD_NW_DEFS=1
LOCAL_MODULE := libnexral_nw$(_TARGET_EXTENSION_)
endif

ifeq ($(BUILD_AS_AAC_SF),1)
LOCAL_CFLAGS += -DNEX_ADD_AAC_SF_DEFS=1
LOCAL_MODULE := libnexcal_in_aac_armv7
endif

ifeq ($(BUILD_AS_MP3_SF),1)
LOCAL_CFLAGS += -DNEX_ADD_MP3_SF_DEFS=1
LOCAL_MODULE := libnexcal_in_mp3_armv7
endif

ifeq ($(BUILD_AS_AMRNB_SF),1)
LOCAL_CFLAGS += -DNEX_ADD_AMRNB_SF_DEFS=1
LOCAL_MODULE := libnexcal_in_amr_armv7
endif

ifeq ($(BUILD_AS_AMRWB_SF),1)
LOCAL_CFLAGS += -DNEX_ADD_AMRWB_SF_DEFS=1
LOCAL_MODULE := libnexcal_in_amrwb_armv7
endif

ifeq ($(BUILD_AS_OGG_SF),1)
LOCAL_CFLAGS += -DNEX_ADD_OGG_SF_DEFS=1
LOCAL_MODULE := libnexcal_in_ogg_armv7
endif

ifeq ($(NEX_BUILD_TEST),1)
LOCAL_MODULE := libnexcralbody_mc_test$(_TARGET_EXTENSION_)
LOCAL_SRC_FILES += test/test_libnexcralbody_mc.cpp
LOCAL_CFLAGS += -DNEX_TEST=1
include $(BUILD_SHARED_LIBRARY)
else
ifeq ($(NEX_STATIC_BUILD),1)
LOCAL_MODULE := $(LOCAL_MODULE)_static
include $(BUILD_STATIC_LIBRARY)
STATIC_LIBRARY_NAME := $(LOCAL_MODULE)
include $(CLEAR_VARS)
LOCAL_STATIC_LIBRARIES := $(STATIC_LIBRARY_NAME)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := $(STATIC_LIBRARY_NAME)_maker
include $(BUILD_SHARED_LIBRARY)
else
include $(BUILD_SHARED_LIBRARY)
endif
endif
        
