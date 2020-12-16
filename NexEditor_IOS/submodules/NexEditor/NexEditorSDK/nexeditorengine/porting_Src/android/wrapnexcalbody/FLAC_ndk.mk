LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_ARM_MODE := arm

WRAPNEXCALBODY_TOP := $(LOCAL_PATH)


LOCAL_SRC_FILES := \
	nexCalBody_FLAC.cpp \
	nexCalBody.cpp \
	nexWrapCalBody_api.cpp

LOCAL_C_INCLUDES :=	$(NDK_PLATFORM_INCLUDE)
LOCAL_C_INCLUDES += \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/nexUtil/$(NEX_UTILS_VERSION)/inc \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/nexCAL/$(NEX_CAL_VERSION)/inc \
	 	$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
	 	$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
	 	$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/FLAC_Decoder

LOCAL_CFLAGS += -DFLAC_CALBODY

ifeq ($(TARGET_ARCH),x86)

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/FLAC_Decoder/ \
					-lNxFLACDec_Linux_x86_release

else ifeq ($(TARGET_ARCH),x86_64)			

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/FLAC_Decoder/ \
					-lNxFLACDec_android_x86_64_release
			
else ifeq ($(TARGET_ARCH),arm64)			

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/FLAC_Decoder/ \
					-lNxFLACDec_android_arm64-v8a_release

else 

LOCAL_ARM_MODE := arm

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/FLAC_Decoder/ \
					-lNxFLACDec_Android_armv7_release
					
endif

LOCAL_LDFLAGS +=	-L$(NDK_PLATFORM_LIB) \
					-llog \
					-landroid	

LOCAL_STATIC_LIBRARIES += 	libcutils \
							libc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libwrapnexflaccalbody

include $(BUILD_SHARED_LIBRARY)

