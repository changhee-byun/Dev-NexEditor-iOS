LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

LOCAL_ARM_MODE := arm

WRAPNEXCALBODY_TOP := $(LOCAL_PATH)


LOCAL_SRC_FILES := \
	nexCalBody_PCM.cpp \
	nexCalBody.cpp \
	nexWrapCalBody_api.cpp

LOCAL_C_INCLUDES :=	$(NDK_PLATFORM_INCLUDE)
LOCAL_C_INCLUDES += \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/nexUtil/$(NEX_UTILS_VERSION)/inc \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/nexCAL/$(NEX_CAL_VERSION)/inc \
	 	$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
	 	$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
	 	$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc

LOCAL_CFLAGS += -DPCM_CALBODY

LOCAL_LDFLAGS +=	-L$(NDK_PLATFORM_LIB) \
					-llog \
					-landroid	

LOCAL_STATIC_LIBRARIES += 	libcutils \
							libc

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libwrapnexpcmcalbody

include $(BUILD_SHARED_LIBRARY)

