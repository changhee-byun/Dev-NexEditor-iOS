LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
SALBODYDEBUG_TOP := $(LOCAL_PATH)

LOCAL_MODULE_TAGS := eng

ifdef USE_NDK

LOCAL_SRC_FILES := \
	SALBody_Debug.cpp \
	SALBody_File.cpp \
	SALBody_Mem.cpp \
	SALBody_SyncObj.cpp \
	SALBody_Task.cpp \
	SALBody_Time.cpp
#	SALBody_Sock_SSL.cpp \
	
else

LOCAL_SRC_FILES := \
	SALBody_Debug.cpp \
	SALBody_File.cpp \
	SALBody_Mem.cpp \
	SALBody_Sock.cpp \
	SALBody_SyncObj.cpp \
	SALBody_Task.cpp \
	SALBody_Time.cpp

endif

ifdef USE_NDK
	LOCAL_CFLAGS += -Wno-format -DHAVE_CONFIG_H
	LOCAL_CFLAGS += -DUSE_NDK -fvisibility=hidden
else
	LOCAL_CFLAGS += -Wno-format -DHAVE_CONFIG_H
endif


ifeq ($(TARGET_ARCH),arm)
LOCAL_ARM_MODE := arm
else ifeq ($(TARGET_ARCH),arm64)
LOCAL_ARM_MODE := arm
else
endif

LOCAL_C_INCLUDES := \
	$(SALBODYDEBUG_TOP)/inc \
 	$(LOCAL_PATH)/../../../extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
 	$(LOCAL_PATH)/../../../extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
 	$(LOCAL_PATH)/../../../extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
	$(LOCAL_PATH)/../../../extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc

LOCAL_PRELINK_MODULE := false

LOCAL_STATIC_LIBRARIES := \
	libnexsal

LOCAL_MODULE := libnexsalbody

include $(BUILD_STATIC_LIBRARY)

