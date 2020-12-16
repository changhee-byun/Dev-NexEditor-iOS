LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional


LOCAL_SRC_FILES := commonlib/lua-5.3.1/src/lapi.c\
    commonlib/lua-5.3.1/src/lauxlib.c\
    commonlib/lua-5.3.1/src/lbaselib.c\
    commonlib/lua-5.3.1/src/lbitlib.c\
    commonlib/lua-5.3.1/src/lcode.c\
    commonlib/lua-5.3.1/src/lcorolib.c\
    commonlib/lua-5.3.1/src/lctype.c\
    commonlib/lua-5.3.1/src/ldblib.c\
    commonlib/lua-5.3.1/src/ldebug.c\
    commonlib/lua-5.3.1/src/ldo.c\
    commonlib/lua-5.3.1/src/ldump.c\
    commonlib/lua-5.3.1/src/lfunc.c\
    commonlib/lua-5.3.1/src/lgc.c\
    commonlib/lua-5.3.1/src/linit.c\
    commonlib/lua-5.3.1/src/liolib.c\
    commonlib/lua-5.3.1/src/llex.c\
    commonlib/lua-5.3.1/src/lmathlib.c\
    commonlib/lua-5.3.1/src/lmem.c\
    commonlib/lua-5.3.1/src/loadlib.c\
    commonlib/lua-5.3.1/src/lobject.c\
    commonlib/lua-5.3.1/src/lopcodes.c\
    commonlib/lua-5.3.1/src/loslib.c\
    commonlib/lua-5.3.1/src/lparser.c\
    commonlib/lua-5.3.1/src/lstate.c\
    commonlib/lua-5.3.1/src/lstring.c\
    commonlib/lua-5.3.1/src/lstrlib.c\
    commonlib/lua-5.3.1/src/ltable.c\
    commonlib/lua-5.3.1/src/ltablib.c\
    commonlib/lua-5.3.1/src/ltm.c\
    commonlib/lua-5.3.1/src/lundump.c\
    commonlib/lua-5.3.1/src/lutf8lib.c\
    commonlib/lua-5.3.1/src/lvm.c\
    commonlib/lua-5.3.1/src/lzio.c\
    renderitem/AnimatedValue.cpp\
    renderitem/Animation.cpp\
    renderitem/glext_loader.cpp\
    renderitem/img.cpp\
    renderitem/lua_extension_for_renderItem.cpp\
    renderitem/maya/material.cpp\
    renderitem/Matrices.cpp\
    renderitem/maya/mesh.cpp\
    renderitem/NexMath.cpp\
    renderitem/poly34.cpp\
    renderitem/RenderItem.cpp\
    renderitem/RenderTarget.cpp\
    renderitem/ritmsimplexnoise1234.cpp\
    renderitem/soil/image_DXT.c\
    renderitem/soil/image_helper.c\
    renderitem/soil/SOIL.c\
    renderitem/soil/stb_image_aug.c\
    renderitem/util.cpp\
    renderitem/nxtRenderItem.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/commonlib/rapidxml-1.13/rapidxml-1.13/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/commonlib/lua-5.3.1/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../nexSAL/$(NEX_SAL_VERSION)/inc \
					$(LOCAL_PATH)/../NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
					$(LOCAL_PATH)/../NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
					$(LOCAL_PATH)/../NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
					$(LOCAL_PATH)/../../nexEditorUtil/inc \


ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS += -Wno-format -D_FIXED_ -D_x86
else
LOCAL_CFLAGS += -Wno-format -D_FIXED_ -DARM
LOCAL_ARM_MODE := arm
endif

# To hide symbol
LOCAL_CFLAGS += -fvisibility=hidden

LOCAL_MODULE    := librenderitem

include $(BUILD_STATIC_LIBRARY)

