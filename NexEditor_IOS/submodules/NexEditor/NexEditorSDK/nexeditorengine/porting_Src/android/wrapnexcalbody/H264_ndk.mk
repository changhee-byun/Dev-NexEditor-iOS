LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

WRAPNEXCALBODY_TOP := $(LOCAL_PATH)


LOCAL_SRC_FILES := \
	nexCalBody_H264.cpp \
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
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/NexCodecUtils \
		$(WRAPNEXCALBODY_TOP)/../../../extern_lib/H264_Decoder/trunk

LOCAL_CFLAGS += -DH264_CALBODY

# To hide symbol
LOCAL_CFLAGS += -fvisibility=hidden

ifeq ($(TARGET_ARCH),x86)

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/H264_Decoder/trunk/ \
					-lNxAVCDec_linux_x86_release
			
else ifeq ($(TARGET_ARCH),x86_64)			

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/H264_Decoder/trunk/ \
					-lNxAVCDec_linux_x86_64_release		
			
else ifeq ($(TARGET_ARCH),arm64)			

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/H264_Decoder/trunk/ \
					-lNxAVCDec_android_armv8_lv51_release

else 

LOCAL_ARM_MODE := arm

LOCAL_LDFLAGS +=	-L$(WRAPNEXCALBODY_TOP)/../../../extern_lib/H264_Decoder/trunk/ \
					-lNxAVCDec_android_armv7_lv51_release
					
endif

LOCAL_LDFLAGS +=	-L$(NDK_PLATFORM_LIB) \
					-llog \
					-landroid	

LOCAL_STATIC_LIBRARIES += 	libcutils \
							libc
							
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libwrapnexh264calbody

include $(BUILD_SHARED_LIBRARY)

