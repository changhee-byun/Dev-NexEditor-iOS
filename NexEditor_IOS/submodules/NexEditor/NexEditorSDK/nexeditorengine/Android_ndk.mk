#####################################
#        Make  for  BUILD           #
#####################################
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := eng

NEXEDITOR_TOP := $(LOCAL_PATH)
NDK_LOCAL_TOP := $(ANDROID_NDK_ROOT)

include $(NEXEDITOR_TOP)/extern_lib_version.mk
include $(NEXEDITOR_TOP)/extern_lib_config.mk

LOCAL_CFLAGS += -Wno-multichar -Wno-format -D_USE_LONGLONG_TO_INT64 -D_ANDROID $(NEXEDITOR_CFLAGS) -DBUILD_NDK 
LOCAL_CPPFLAGS += -std=c++11

# To hide symbol
LOCAL_CPPFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -fvisibility=hidden

ifeq ($(TARGET_ARCH),arm)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm/usr/lib
LOCAL_MULTILIB := 32
endif

ifeq ($(TARGET_ARCH),arm64)

NDK_PLATFORM_PATH := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-arm64/usr/lib

endif

ifeq ($(TARGET_ARCH),x86)

NDK_PLATFORM_INCLUDE := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-x86/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-x86/usr/lib
LOCAL_MULTILIB := 32
LOCAL_CFLAGS += -D_x86
endif

ifeq ($(TARGET_ARCH),x86_64)

NDK_PLATFORM_INCLUDE := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-x86_64/usr/include
NDK_PLATFORM_LIB := $(NDK_LOCAL_TOP)/platforms/$(TARGET_PLATFORM)/arch-x86_64/usr/lib64
LOCAL_CFLAGS += -D_x86
endif



LOCAL_SRC_FILES := \
					nexVideoEditor/src/NEXVIDEOEDITOR_Android_AudioRenderTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_AudioTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_Clip.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_Layer.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ClipList.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_CodecManager.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_CodecWrap.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_Rect.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_PlayClipTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ProjectManager.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ThreadBase.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_Thumbnail.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ThumbnailTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_TranscodingTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoEditor.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_GL_VideoRenderTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_WrapFileReader.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_WrapFileWriter.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ExportWriter.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_WrapFileReverse.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_WrapOutSource.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoFrame.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoTrack.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoFrameWriteTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_FileMissing.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_AudioGain.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_SupportDevices.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_FrameTimeChecker.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_Util.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_Property.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_EffectItem.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_EffectItemVec.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_HighlightDiffChecker.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_PFrameProcessTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_CalcTime.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_FastPreviewTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_SetTimeTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_DirectExportTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoThumbTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_PreviewThumb.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_VideoPreviewTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ReverseTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_UDTA.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_DrawInfo.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_DrawInfoVec.cpp \
					nexVideoEditor/src/EnvelopDetector.cpp \
					nexVideoEditor/src/NxResizeYUV.c \
					nexVideoEditor/src/RGB_To_YUV420.cpp \
					nexVideoEditor/src/nexYYUV2YUV420.c \
					nexVideoEditor/src/nexNV12TileToNV12.cpp \
					nexVideoEditor/src/nextreaming_configuration_reader.cpp \
					nexVideoEditor/porting_src/porting_android.cpp \
					porting_Src/android/nexEditor_jni.cpp \
					porting_Src/android/nexEditorUtils_jni.cpp \
					porting_Src/android/nexEditorEventHandler.cpp \
					porting_Src/android/NexJNIEnvStack.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_ImageTask.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_PlaybackCacheWorker.cpp \
					nexVideoEditor/src/NexSDKInfo.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_CodecCacheInfo.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_PerformanceChecker.cpp \
					nexVideoEditor/src/NEXVIDEOEDITOR_FrameYUVInfo.cpp
					
#					nexVideoEditor/src/YUV420_To_RGB16.cpp 

LOCAL_C_INCLUDES :=	$(NDK_PLATFORM_INCLUDE)
LOCAL_C_INCLUDES +=	\
					$(NDK_LOCAL_TOP)/sources/cxx-stl/stlport/stlport \
					$(NEXEDITOR_TOP)/extern_lib/NexCommon/NexCommonDef/$(NEX_COMMON_DEF_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NexCommon/NexMediaDef/$(NEX_MEDIA_DEF_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NexCommon/NexTypeDef/$(NEX_TYPE_DEF_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/nexSAL/$(NEX_SAL_VERSION)/inc \
					$(NEXEDITOR_TOP)/extern_lib/nexCAL/$(NEX_CAL_VERSION)/inc \
					$(NEXEDITOR_TOP)/extern_lib/nexRAL/$(NEX_RAL_VERSION)/inc \
					$(NEXEDITOR_TOP)/extern_lib/nexUtil/$(NEX_UTILS_VERSION)/inc \
					$(NEXEDITOR_TOP)/extern_lib/NexChunkParser/$(NEX_CHUNKPARSER_VERSION)/inc \
					$(NEXEDITOR_TOP)/extern_lib/NexSecureware/inc \
					$(NEXEDITOR_TOP)/extern_lib/NexCodecUtils \
					$(NEXEDITOR_TOP)/extern_lib/NxXMLParser/$(NEX_XML_PARSER_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NxFFSubtitle/$(NEX_FFSUBTITLE_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NxFFReader/$(NEX_FFREADER_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NxFFWriter/$(NEX_FFWRITER_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NEXMixer/$(NEX_MIXER_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NEXResampler/$(NEX_RESAMPLER_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/NexSound/$(NEX_SOUND_VERSION) \
					$(NEXEDITOR_TOP)/extern_lib/RenderItemSDK/renderitem \
					$(NEXEDITOR_TOP)/extern_lib/AES/inc \
					$(NEXEDITOR_TOP)/extern_lib/KISA_LSH \
					$(NEXEDITOR_TOP)/extern_lib/NexPKGVerifier \
					$(NEXEDITOR_TOP)/nexEditorUtil/inc \
					$(NEXEDITOR_TOP)/nexVideoEditor/nex_inc \
					$(NEXEDITOR_TOP)/nexVideoEditor/inc \
					$(NEXEDITOR_TOP)/nexVideoEditor/src \
					$(NEXEDITOR_TOP)/nexTheme/inc \
					$(NEXEDITOR_TOP)/nexLayer/src \
					$(NEXEDITOR_TOP)/nexVideoEditor/porting_src \
					$(NEXEDITOR_TOP)/nexVideoEditor/porting_src/android \
					$(NEXEDITOR_TOP)/porting_Src/inc \
					$(NEXEDITOR_TOP)/porting_Src/android \
					$(NEXEDITOR_TOP)/porting_Src/android/nexsalbody/inc \
					$(NEXEDITOR_TOP)/porting_Src/android/nexcalbody \
					$(NEXEDITOR_TOP)/porting_Src/android/nexcalbody_jpeg \
					$(NEXEDITOR_TOP)/porting_Src/android/nexcralbody_mc \
					$(NEXEDITOR_TOP)/porting_Src/android/nexralbody_audio/inc
					
					
LOCAL_WHOLE_STATIC_LIBRARIES := \
					libnexsal \
					libnexcal \
					libnexdatastruct \
					libnexchunkparser \
					libnexsalbody \
					libnexcalbody \
					libnexcalbody_jpeg  \
					libnexstdutil \
					libnexeditorutil \
					librenderitem	\
					libnexaes \
					libnextheme \
					libnexglrender \
					libkisalsh \
					libnexsecureware \
					libnexlayer

LOCAL_STATIC_LIBRARIES += 	libcutils \
							libc

ifeq ($(JPEG_BUILD_ENABLE), $(NEXBOOL_TRUE))
LOCAL_CFLAGS += -DBUILD_JPEG_DECODER
endif

ifeq ($(TARGET_ARCH),x86)

LOCAL_LDFLAGS := 	-L$(NEXEDITOR_TOP)/extern_lib/NEXMixer/$(NEX_MIXER_VERSION) \
					-lNexMixer_Android_x86_Release
					
LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NEXResampler/$(NEX_RESAMPLER_VERSION) \
					-lNxSRC_android_x86

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexSound/$(NEX_SOUND_VERSION) \
					-lNexsound_Android_x86_Release

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexCodecUtils \
					-lNexCodecUtil_android_x86_release

ifeq ($(JPEG_BUILD_ENABLE), $(NEXBOOL_TRUE))
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/JPEG_Decoder \
					lNxJPEGDec_linux_x86_v5.5.11_release
endif

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFReader/$(NEX_FFREADER_VERSION) \
					-lNxFFReader_android_x86_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFSubtitle/$(NEX_FFSUBTITLE_VERSION) \
					-lNxFFSubtitle_android_x86_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxXMLParser/$(NEX_XMLPARSER_VERSION) \
					-lnxXMLParser_android_x86_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFWriter/$(NEX_FFWRITER_VERSION) \
					-lNxFFWriter_android_x86_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NexPKGVerifier/ \
					-lNexPKGVerifier_x86_release

else ifeq ($(TARGET_ARCH),x86_64)

LOCAL_LDFLAGS := 	-L$(NEXEDITOR_TOP)/extern_lib/NEXMixer/$(NEX_MIXER_VERSION) \
					-lNexMixer_Android_x64_Release
					
LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NEXResampler/$(NEX_RESAMPLER_VERSION) \
					-lNxSRC_Android_x86_64

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexSound/$(NEX_SOUND_VERSION) \
					-lNexsound_Android_x64_Release

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexCodecUtils \
					-lNexCodecUtil_android_x86_64_release

ifeq ($(JPEG_BUILD_ENABLE), $(NEXBOOL_TRUE))
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/JPEG_Decoder \
					lNxJPEGDec_linux_x64_v5.5.11_release
endif

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFReader/$(NEX_FFREADER_VERSION) \
					-lNxFFReader_android_x64_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFSubtitle/$(NEX_FFSUBTITLE_VERSION) \
					-lNxFFSubtitle_android_x64_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxXMLParser/$(NEX_XMLPARSER_VERSION) \
					-lnxXMLParser_android_x64_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFWriter/$(NEX_FFWRITER_VERSION) \
					-lNxFFWriter_android_x64_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NexPKGVerifier/ \
					-lNexPKGVerifier_x86_64_release
					
else ifeq ($(TARGET_ARCH),arm64)

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NEXMixer/$(NEX_MIXER_VERSION) \
					-lNexMixer_Android_ARMv8a_Release

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NEXResampler/$(NEX_RESAMPLER_VERSION) \
					-lNxSRC_Android_arm64

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexSound/$(NEX_SOUND_VERSION)  \
					-lNexsound_Android_ARMv8a_Release

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexCodecUtils \
					-lNexCodecUtil_android_arm64-v8a_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/JPEG_Decoder \
					-lNxJPEGDec_android_armv8_v5.5.11_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFReader/$(NEX_FFREADER_VERSION) \
					-lNxFFReader_android_armv8a_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFSubtitle/$(NEX_FFSUBTITLE_VERSION) \
					-lNxFFSubtitle_android_armv8a_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxXMLParser/$(NEX_XMLPARSER_VERSION) \
					-lnxXMLParser_android_armv8a_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFWriter/$(NEX_FFWRITER_VERSION) \
					-lNxFFWriter_android_armv8a_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NexPKGVerifier/ \
					-lNexPKGVerifier_arm64-v8a_release
else

LOCAL_LDFLAGS := 	-L$(NEXEDITOR_TOP)/nexVideoEditor/nex_lib/android \
					-lnexSeperatorByte

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NEXMixer/$(NEX_MIXER_VERSION) \
					-lNexMixer_Android_ARM_Release

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NEXResampler/$(NEX_RESAMPLER_VERSION) \
					-lNxSRC_android_arm

LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexSound/$(NEX_SOUND_VERSION) \
					-lNexsound_Android_ARM_Release
					
LOCAL_LDFLAGS += 	-L$(NEXEDITOR_TOP)/extern_lib/NexCodecUtils \
					-lNexCodecUtil_android_armeabi_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/JPEG_Decoder \
					-lNxJPEGDec_android_armv7_v5.5.11_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFReader/$(NEX_FFREADER_VERSION) \
					-lNxFFReader_android_arm_release

LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFSubtitle/$(NEX_FFSUBTITLE_VERSION) \
					-lNxFFSubtitle_android_arm_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxXMLParser/$(NEX_XMLPARSER_VERSION) \
					-lnxXMLParser_android_arm_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NxFFWriter/$(NEX_FFWRITER_VERSION) \
					-lNxFFWriter_android_arm_release
					
LOCAL_LDFLAGS +=	-L$(NEXEDITOR_TOP)/extern_lib/NexPKGVerifier/ \
					-lNexPKGVerifier_armeabi_release
endif

LOCAL_LDFLAGS    += 	-L$(NDK_PLATFORM_LIB) \
						-lGLESv2 \
						-lEGL \
						-landroid \
						-llog \
						-lstdc++ \
						-lz

#Symbol Hide during linking
LOCAL_LDFLAGS += -Wl,-version-script=$(NEXEDITOR_TOP)/libnexeditorsdk.ver

#Generate map file during linking
LOCAL_LDFLAGS += -Xlinker -Map=$(NEXEDITOR_TOP)/libs/$(TARGET_ARCH_ABI).map -fuse-ld=bfd
					


LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE := libnexeditorsdk

include $(BUILD_SHARED_LIBRARY)


include $(NEXEDITOR_TOP)/nexTheme/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/nexEditorUtil/build/android/Android_ndk.mk

include $(NEXEDITOR_TOP)/nexLayer/build/android/Android_ndk.mk

include $(NEXEDITOR_TOP)/extern_lib/KISA_LSH/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/dummy32/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/RenderItemSDK/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/AES/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/nexUtil/$(NEX_UTILS_VERSION)/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/nexSAL/$(NEX_SAL_VERSION)/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/nexCAL/$(NEX_CAL_VERSION)/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/NexDataStruct/$(NEX_DATASTRUCT_VERSION)/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/NexChunkParser/$(NEX_CHUNKPARSER_VERSION)/build/android/Android_ndk.mk
include $(NEXEDITOR_TOP)/extern_lib/NexSecureware/build/android/Android_ndk.mk


include $(NEXEDITOR_TOP)/porting_Src/android/nexsalbody/Android_ndk.mk
include $(NEXEDITOR_TOP)/porting_Src/android/nexcalbody/Android_ndk.mk

ifeq ($(JPEG_BUILD_ENABLE), $(NEXBOOL_TRUE))
include $(NEXEDITOR_TOP)/porting_Src/android/nexcalbody_jpeg/Android_ndk.mk
endif
include $(NEXEDITOR_TOP)/porting_Src/android/nexcralbody_mc/Android_ndk.mk
include $(NEXEDITOR_TOP)/porting_Src/android/nexralbody_audio/Android_ndk.mk
include $(NEXEDITOR_TOP)/porting_Src/android/wrapnexcalbody/PCM_ndk.mk
include $(NEXEDITOR_TOP)/porting_Src/android/wrapnexcalbody/FLAC_ndk.mk
#include $(NEXEDITOR_TOP)/porting_Src/android/wrapnexcalbody/AAC_ENC_ndk.mk
