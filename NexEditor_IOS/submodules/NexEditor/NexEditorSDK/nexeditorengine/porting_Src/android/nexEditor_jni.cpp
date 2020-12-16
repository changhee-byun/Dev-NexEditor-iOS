/******************************************************************************
* File Name   :	nexEditor_jni.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#include <jni.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <android/native_window_jni.h>
#include "NexPKGVerifier.h"

#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_Interface.h"
#include "nexEditorEventHandler.h"

#define NEXEDITOR_MAJOR_VERSION	1
#define NEXEDITOR_MINOR_VERSION	1
#define NEXEDITOR_PATCH_VERSION	41

#include "SALBody_File.h"
#include "KISA_lsh.h"
#define  LOG_TAG    "NEXEDITOR"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static INexVideoEditor* g_VideoEditorHandle	= NULL;

#define MAKE_JNI_FUNC_NAME(package_prefix,function_name) package_prefix##function_name
#ifdef FOR_PROJECT_Kinemaster
#define _EDITOR_JNI_(f) MAKE_JNI_FUNC_NAME(Java_com_nextreaming_nexvideoeditor_NexEditor_,f)
#define INTERNAL_SDK_PACKAGE_PATH "com/nextreaming/nexvideoeditor/"
#define SDK_PACKAGE_PATH "com/nextreaming/nexvideoeditor/"
#else
#define _EDITOR_JNI_(f) MAKE_JNI_FUNC_NAME(Java_com_nexstreaming_kminternal_nexvideoeditor_NexEditor_,f)
#define INTERNAL_SDK_PACKAGE_PATH "com/nexstreaming/kminternal/nexvideoeditor/"
#define SDK_PACKAGE_PATH "com/nexstreaming/nexeditorsdk/"
#endif

extern "C" {
	
JNIEXPORT jint JNICALL _EDITOR_JNI_(initUserData)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(check4km)(JNIEnv * env, jobject obj, jobject context);
JNIEXPORT jint JNICALL _EDITOR_JNI_(createEditor)(JNIEnv * env, jobject obj, jstring strLibPath, jstring strModel, jint iAPILevel, jint iUserData, jintArray properties );
JNIEXPORT jint JNICALL _EDITOR_JNI_(destroyEditor)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(commandMarker)(JNIEnv * env, jobject obj, jint iTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(prepareSurface)(JNIEnv * env, jobject obj, jobject surface);
JNIEXPORT jint JNICALL _EDITOR_JNI_(changeSurface)(JNIEnv * env, jobject obj, jint iWidth, jint iHeight);

JNIEXPORT jint JNICALL _EDITOR_JNI_(createProject)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(closeProject)(JNIEnv * env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectEffect)(JNIEnv * env, jobject obj, jstring strEffectID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(encodeProject)(JNIEnv * env, jobject obj, jstring strFilePath, jint iWidth, jint iHeight, jint iBitrate, jlong iMaxFileSize, jint iMaxFileDuration, jint iFPS, jint iProjectWidth, jint iProjectHeight, jint iSamplingRate, jint iAudioBitrate, jint iEncodeProfile, jint iEncodeLevel, jint iVideoCodecType, jint iFlag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(encodeProjectJpeg)(JNIEnv * env, jobject obj, jobject surface, jstring strFilePath, jint iWidth, jint iHeight, jint iQuality, jint iFlag);

//#ifndef FOR_PROJECT_Kinemaster
JNIEXPORT jint JNICALL _EDITOR_JNI_(encodePause)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(encodeResume)(JNIEnv * env, jobject obj);
//#endif

JNIEXPORT jint JNICALL _EDITOR_JNI_(addVisualClip)(JNIEnv * env, jobject obj, jint iNextToClipID, jstring strFilePath, jint iNewClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(addAudioClip)(JNIEnv * env, jobject obj, jint iStartTime, jstring strFilePath, jint iNewClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(moveVisualClip)(JNIEnv * env, jobject obj, jint iNextToClipID, jint iMovedClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(moveAudioClip)(JNIEnv * env, jobject obj, jint iStartTime, jint iMovedClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(deleteClipID)(JNIEnv * env, jobject obj, jint iClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(updateVisualClip)(JNIEnv * env, jobject obj, jobject clipInfo);
JNIEXPORT jint JNICALL _EDITOR_JNI_(updateAudioClip)(JNIEnv * env, jobject obj, jobject clipInfo);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setBackgroundMusic)(JNIEnv * env, jobject obj, jstring strFilePath, jint iNewClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setBackgroundMusicVolume)(JNIEnv * env, jobject obj, int iVolume, int iFadeInTime, int iFadeOutTime);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectVolumeFade)(JNIEnv * env, jobject obj, int iFadeInTime, int iFadeOutTime);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectVolume)(JNIEnv * env, jobject obj, int iProjectVolume);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectManualVolumeControl)(JNIEnv * env, jobject obj, int iManualVolumeControl);

JNIEXPORT jint JNICALL _EDITOR_JNI_(getDuration)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setTime)(JNIEnv * env, jobject obj, jint iTime, jint iDisplay, jint iIDRFrame);
JNIEXPORT jbyteArray JNICALL _EDITOR_JNI_(getTimeThumbData)(JNIEnv * env, jobject obj, jint iTime);
JNIEXPORT jint JNICALL _EDITOR_JNI_(captureCurrentFrame)(JNIEnv * env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(startPlay)(JNIEnv * env, jobject obj, jint iMuteAudio);
JNIEXPORT jint JNICALL _EDITOR_JNI_(pausePlay)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(resumePlay)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(stopPlay)(JNIEnv * env, jobject obj, jint iFlag);

JNIEXPORT jobject JNICALL _EDITOR_JNI_(getVisualClip)(JNIEnv * env, jobject obj, jint iClipID);
JNIEXPORT jobject JNICALL _EDITOR_JNI_(getAudioClip)(JNIEnv * env, jobject obj, jint iClipID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipList)(JNIEnv * env, jobject obj, jobject manager);
JNIEXPORT jint JNICALL _EDITOR_JNI_(loadClipList)(JNIEnv * env, jobject obj, jobjectArray arrayVisualClip, jobjectArray arrayAudioClip);
JNIEXPORT jint JNICALL _EDITOR_JNI_(loadTheme)(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData, jint iFlags);
JNIEXPORT jint JNICALL _EDITOR_JNI_(clearRenderItems)(JNIEnv * env, jobject obj, jint iFlags);
JNIEXPORT jint JNICALL _EDITOR_JNI_(loadRenderItem)(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData, jint iFlags);
JNIEXPORT jstring JNICALL _EDITOR_JNI_(getLoadThemeError)(JNIEnv * env, jobject obj, jint iFlags);
JNIEXPORT jstring JNICALL _EDITOR_JNI_(getThemeID)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setThemeID)(JNIEnv * env, jobject obj, jstring strThemeID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getThemeList)(JNIEnv * env, jobject obj, jobject manager);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipEffectList)(JNIEnv * env, jobject obj, jobject manager);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getTitleEffectList)(JNIEnv * env, jobject obj, jobject manager);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setEventHandler)(JNIEnv * env, jobject obj, jobject eventListener);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipInfo)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jobject clipInfo, jint isVideoThumbnail, jint isAudioPCMLevel, jint isBackground, jint iUserTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipInfoSync)(JNIEnv * env, jobject obj, jstring clipPath, jobject clipInfo, jint iFlag, jint iUserTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipVideoThumb)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iWidth, jint iHeight, jint iStartTime, jint iEndTime, jint iCount, jint iFlag, jint iUserTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipVideoThumbWithTimeTable)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iWidth, jint iHeight, int iSize, jintArray arrayTables, jint iFlag, jint iUserTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipAudioThumb)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iUserTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipAudioThumbPartial)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iStartTime, jint iEndTime, jint iUseCount, jint iSkipCount, jint iUserTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipStopThumb)(JNIEnv * env, jobject obj, jint iUserTag);

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearScreen)(JNIEnv * env, jobject obj, jint iTag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(fastOptionPreview)(JNIEnv * env, jobject obj, jstring strOption, jint iDisplay);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getVersionNumber)(JNIEnv * env, jobject obj, jint iVersion);
JNIEXPORT jint JNICALL _EDITOR_JNI_(clearTrackCache)(JNIEnv * env, jobject obj);
JNIEXPORT jstring JNICALL _EDITOR_JNI_(getSystemProperty)(JNIEnv * env, jclass clazz, jstring strPropertyName);

JNIEXPORT jint JNICALL _EDITOR_JNI_(startVoiceRecorder)(JNIEnv * env, jobject obj, jstring strFilePath, jint iSampleRate, jint iChannels, jint iBitForSample);
JNIEXPORT jint JNICALL _EDITOR_JNI_(processVoiceRecorder)(JNIEnv * env, jobject obj, jbyteArray arrayPCM, jint iPCMLen);
JNIEXPORT jint JNICALL _EDITOR_JNI_(endVoiceRecorder)(JNIEnv * env, jobject obj, jobject clipInfo);

JNIEXPORT jstring JNICALL _EDITOR_JNI_(getProperty)(JNIEnv * env, jobject obj, jstring strPropertyName);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setProperty)(JNIEnv * env, jobject obj, jstring strPropertyName, jstring strPropertyValue);

JNIEXPORT jint JNICALL _EDITOR_JNI_(transcodingStart)(JNIEnv* env, jobject obj, jstring strSrcClipPath, jstring strDstClipPath,  jint iWidth, jint iHeight, jint iDisplayWidth, jint iDisplayHeight, jint iBitrate, jlong lMaxFileSize, jint iFPS, jint iFlag, jstring strUserData);
JNIEXPORT jint JNICALL _EDITOR_JNI_(transcodingStop)(JNIEnv* env, jobject obj);

#ifdef FOR_LAYER_FEATURE
JNIEXPORT jint JNICALL _EDITOR_JNI_(addLayerItem)(JNIEnv* env, jobject obj, jobject layer);
JNIEXPORT jint JNICALL _EDITOR_JNI_(deleteLayerItem)(JNIEnv* env, jobject obj, jint iLayerID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(clearLayerItem)(JNIEnv* env, jobject obj);
#endif	

#ifdef FOR_LOADLIST_FEATURE
JNIEXPORT jint JNICALL _EDITOR_JNI_(loadList)(JNIEnv * env, jobject obj, jobjectArray arrayVisualClip, jobjectArray arrayAudioClip, jint iOption);
JNIEXPORT jint JNICALL _EDITOR_JNI_(asyncLoadList)(JNIEnv * env, jobject obj, jobjectArray arrayVisualClip, jobjectArray arrayAudioClip, jint iOption);
JNIEXPORT jint JNICALL _EDITOR_JNI_(asyncEffectList)(JNIEnv * env, jobject obj, jobjectArray arrayEffectClip, jint iOption);
JNIEXPORT jint JNICALL _EDITOR_JNI_(asyncDrawInfoList)(JNIEnv * env, jobject obj, jobjectArray master, jobjectArray sub);
JNIEXPORT jint JNICALL _EDITOR_JNI_(updateDrawInfo)(JNIEnv * env, jobject obj, jobject drawinfo);
JNIEXPORT jint JNICALL _EDITOR_JNI_(clearList)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(clearProject)(JNIEnv * env, jobject obj);
#endif	

JNIEXPORT jint JNICALL _EDITOR_JNI_(highlightStart)(JNIEnv* env, jobject obj, jstring strSrcClipPath, jint iIndexMode, jint iRequestInterval, jint iRequestCount, jint iOutputMode, jstring strDstClipPath, jint iWidth, jint iHeight, jint iBitrate, jlong lMaxFileSize, jint iDecodeMode);
JNIEXPORT jint JNICALL _EDITOR_JNI_(highlightStop)(JNIEnv* env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(checkDirectExport)(JNIEnv* env, jobject obj, jint option);
JNIEXPORT jint JNICALL _EDITOR_JNI_(directExport)(JNIEnv * env, jobject obj, jstring strFilePath, jlong iMaxFileSize, jlong iMaxFileDuration, jstring strUserData, jint iFlag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(checkPFrameDirectExportSync)(JNIEnv* env, jobject obj, jstring strFilePath);

JNIEXPORT jint JNICALL _EDITOR_JNI_(fastPreviewStart)(JNIEnv * env, jobject obj, jint iStartTime, jint iEndTime, jint displayWidth, jint displayHeight);
JNIEXPORT jint JNICALL _EDITOR_JNI_(fastPreviewStop)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(fastPreviewTime)(JNIEnv * env, jobject obj, jint iSetTime);

JNIEXPORT jint JNICALL _EDITOR_JNI_(checkIDRStart)(JNIEnv * env, jobject obj, jstring strFilePath);
JNIEXPORT jint JNICALL _EDITOR_JNI_(checkIDREnd)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(checkIDRTime)(JNIEnv * env, jobject obj, jint iTime);

JNIEXPORT jint JNICALL _EDITOR_JNI_(reverseStart)(JNIEnv* env, jobject obj, jstring strSrcClipPath, jstring strDstClipPath, jstring strTempClipPath, jint iWidth, jint iHeight, jint iBitrate, jlong lMaxFileSize, jint iStartTime, jint iEndTime, jint iDecodeMode);
JNIEXPORT jint JNICALL _EDITOR_JNI_(reverseStop)(JNIEnv* env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setVolumeWhilePlay)(JNIEnv * env, jobject obj, jint iMasterVolume, jint iSlaveVolume);
JNIEXPORT jintArray JNICALL _EDITOR_JNI_(getVolumeWhilePlay)(JNIEnv * env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(addUDTA)(JNIEnv * env, jobject obj, jint iType, jstring strUDTA);
JNIEXPORT jint JNICALL _EDITOR_JNI_(clearUDTA)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getNativeSDKInfoWM)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setBaseFilterRenderItem)(JNIEnv * env, jobject obj, jstring path);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForClipID)(JNIEnv * env, jobject obj, jint export_flag, jint clipid);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setTexNameForLUT)(JNIEnv * env, jobject obj, jint export_flag, jint clipid, jfloat x, jfloat y);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForMask)(JNIEnv * env, jobject obj, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForBlend)(JNIEnv * env, jobject obj, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(swapBlendMain)(JNIEnv * env, jobject obj, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForWhite)(JNIEnv * env, jobject obj, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(cleanupMaskWithWhite)(JNIEnv * env, jobject obj, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(createRenderItem)(JNIEnv * env, jobject obj, jstring effect_id, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(releaseRenderItem)(JNIEnv * env, jobject obj, jint effect_id, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(drawRenderItemOverlay)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jstring usersettings, jint export_flag, jint current_time, jint start_time, jint end_time, jfloatArray matrix, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled);
JNIEXPORT jint JNICALL _EDITOR_JNI_(createNexEDL)(JNIEnv * env, jobject obj, jstring effect_id, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(releaseNexEDL)(JNIEnv * env, jobject obj, jint effect_id, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(drawNexEDLOverlay)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint export_flag, jint current_time, jint start_time, jint end_time, jfloatArray matrix, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jint mode);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setRenderToMask)(JNIEnv * env, jobject obj, jint export_flag);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setRenderToDefault)(JNIEnv * env, jobject obj, jint export_flag);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setOutputFile)(JNIEnv *env, jobject thiz, jobject fileDescriptor, jlong offset, jlong length);
JNIEXPORT jint JNICALL _EDITOR_JNI_(closeOutputFile)(JNIEnv *env, jobject thiz, jint setOutputFD);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setInputFile)(JNIEnv *env, jobject thiz, jobject fileDescriptor, jint predef ,jlong offset, jlong length);
JNIEXPORT jint JNICALL _EDITOR_JNI_(closeInputFile)(JNIEnv *env, jobject thiz, jint predef, jint setInputFD);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setVideoTrackUUID)(JNIEnv *env, jobject thiz, jint iMode, jbyteArray arrayUUID);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getAudioSessionID)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(set360VideoTrackPosition)(JNIEnv * env, jobject obj, jint iXAngle , jint iYAngle , jint iflags);
JNIEXPORT jint JNICALL _EDITOR_JNI_(releaseLUTTexture)(JNIEnv * env, jobject obj, jint resource_id);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setTaskSleep)(JNIEnv * env, jobject obj, jint sleep);
JNIEXPORT jintArray JNICALL _EDITOR_JNI_(createCubeLUT)(JNIEnv * env, jobject obj, jbyteArray stream);
JNIEXPORT jintArray JNICALL _EDITOR_JNI_(createLGLUT)(JNIEnv * env, jobject obj, jbyteArray stream);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setPreviewScaleFactor)(JNIEnv * env, jobject obj, jfloat fScaleFactor);
JNIEXPORT jfloat JNICALL _EDITOR_JNI_(getPreviewScaleFactor)(JNIEnv * env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(pushLoadedBitmap)(JNIEnv * env, jobject obj, jstring path, jintArray pixels, jint width, jint height, jint loadedtype);
JNIEXPORT jint JNICALL _EDITOR_JNI_(removeBitmap)(JNIEnv * env, jobject obj, jstring path);
JNIEXPORT jint JNICALL _EDITOR_JNI_(updateRenderInfo)(JNIEnv * env, jobject obj, jint clipid, jint start_left, jint start_top, jint start_right, jint start_bottom, jint end_left, jint end_top, jint end_right, jint end_bottom, jint face_detected, jint face_left, jint face_top, jint face_right, jint face_bottom);
JNIEXPORT jint JNICALL _EDITOR_JNI_(resetFaceDetectInfo)(JNIEnv * env, jobject obj, jint clipid);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setBrightness)(JNIEnv * env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getBrightness)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setContrast)(JNIEnv * env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getContrast)(JNIEnv * env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setSaturation)(JNIEnv * env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getSaturation)(JNIEnv * env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setDeviceLightLevel)(JNIEnv * env, jobject obj, jint lightLevel);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setDeviceGamma)(JNIEnv * env, jobject obj, jfloat gamma);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setForceRTT)(JNIEnv * env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setThumbnailRoutine)(JNIEnv * env, jobject obj, jint value);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setVignette)(JNIEnv *env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getVignette)(JNIEnv *env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setVignetteRange)(JNIEnv *env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getVignetteRange)(JNIEnv *env, jobject obj);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setSharpness)(JNIEnv *env, jobject obj, jint value);
JNIEXPORT jint JNICALL _EDITOR_JNI_(getSharpness)(JNIEnv *env, jobject obj);

JNIEXPORT jint JNICALL _EDITOR_JNI_(setGIFMode)(JNIEnv *env, jobject obj, jint mode);
JNIEXPORT jint JNICALL _EDITOR_JNI_(setEncInfo)(JNIEnv * env, jobject obj,  jobjectArray info);
JNIEXPORT void JNICALL _EDITOR_JNI_(setPacakgeName4Protection)(JNIEnv * env, jobject obj, jstring package_name);
JNIEXPORT jlong JNICALL _EDITOR_JNI_(getPerformanceCount)(JNIEnv *env, jobject obj, jint type);
};

jobject createVisualClip(JNIEnv * env, IClipItem* pClip);
jobject createAudioClip(JNIEnv * env, IClipItem* pClip);
int setVisualClip(JNIEnv * env, jobject clipInfo, IClipItem* pClip);
int setAudioClip(JNIEnv * env, jobject clipInfo, IClipItem* pClip);
int setLayerInfo(JNIEnv * env, jobject clipInfo, ILayerItem* pLayer);

int setDrawInfo(JNIEnv * env, jobject drawInfo, IDrawInfo* pDrawInfo);

#include "nexProtection.h"

#define NEXVIDEOEDITOR_VERSION_Majer 3
#define NEXVIDEOEDITOR_VERSION_Minor 4
#define NEXVIDEOEDITOR_VERSION_Patch 1

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#define NEXVIDEOEDITOR_ABI 64
#else
#define NEXVIDEOEDITOR_ABI 32
#endif


#ifdef FOR_TEST_ENCODER_PROFILE
#define NEXVIDEOEDITOR_VERSION_DESC "encoderProfile"
#else
#define NEXVIDEOEDITOR_VERSION_DESC "none"
#endif

static jclass gclassSTL;

JNIEXPORT void JNICALL _EDITOR_JNI_(setPacakgeName4Protection)(JNIEnv * env, jobject obj, jstring package_name)
{
	const char* packageName = env->GetStringUTFChars(package_name, NULL);
	LOGI("[nexEDitor_jni.cpp %d] packageName:%s", __LINE__, packageName);
#ifndef FOR_PROJECT_Kinemaster
	setPackageName4Protection(packageName);	
#endif
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(initUserData)(JNIEnv * env, jobject obj)
{
	return InitUserData();
}

static char* jstringTostring(JNIEnv* env, jstring jstr, const char * pend )
{
        char* rtn = NULL;
        int pend_len = 0;

        jclass clsstring = env->FindClass( "java/lang/String");
        jstring strencode = env->NewStringUTF("utf-8");
        jmethodID mid = env->GetMethodID( clsstring, "getBytes", "(Ljava/lang/String;)[B");
        jbyteArray barr= (jbyteArray)env->CallObjectMethod( jstr, mid, strencode);
        jsize alen = env->GetArrayLength( barr);
        jbyte* ba = env->GetByteArrayElements( barr, JNI_FALSE);


        if( pend != NULL ){
            pend_len = strlen(pend);
        }

        if (alen > 0)
        {
                rtn = (char*)malloc(pend_len + alen + 1);
                if( pend_len > 0 ){
                    memcpy(rtn, pend, pend_len);
                }
                memcpy(rtn+pend_len, ba, alen);
                rtn[pend_len + alen] = 0;
        }
        env->ReleaseByteArrayElements( barr, ba, 0);
        return rtn;
}

#define NX_CERT_VAL 0xf82376ad
static int s_cert_mismatch = NX_CERT_VAL;

/*
static void printHexaLog(const char *pTitle, const unsigned char *pData, unsigned int uSize)
{
	LOGI("[nexEDitor_jni.cpp %d] [%s] Data size = %u, address=%p\n", __LINE__, pTitle?pTitle:"", uSize, pData);

 	int i=0, j=0;
 	#define HEXA_UNIT	32
 	
	for (i = 0; i<uSize; i += HEXA_UNIT) {
		char pBuf[10240];
		unsigned int uOffset = 0;
		memset(pBuf, 0x00, sizeof(pBuf));

		unsigned int uTarget = uSize - i;
		uTarget = uTarget>HEXA_UNIT ? HEXA_UNIT : uTarget;

		for (j = 0; j<uTarget; j++) {
			sprintf(&pBuf[uOffset], "0x%02X, ", pData[i + j]);
			uOffset = strlen(pBuf);
		}

 		LOGI("[nexEDitor_jni.cpp %d] [%d,%d] %s\n", __LINE__, i, j, pBuf);
 	}
}
*/
JNIEXPORT jint JNICALL _EDITOR_JNI_(check4km)(JNIEnv * env, jobject thiz, jobject context)
{
#ifdef    NX_CERT_DEBUG_MODE

#else
    if( s_cert_mismatch != NX_CERT_VAL )
    {
        return 0;
    }
jclass context_clazz = env->GetObjectClass( context);
   jmethodID methodID_func = env->GetMethodID( context_clazz,"getPackageManager", "()Landroid/content/pm/PackageManager;");
   jobject package_manager = env->CallObjectMethod( context, methodID_func);

    jmethodID mid_packagename = env->GetMethodID( context_clazz, "getPackageName", "()Ljava/lang/String;");
    jstring packageName = (jstring) env->CallObjectMethod( context, mid_packagename);
    /*
    char * pn = jstringTostring(env,packageName,NULL);
    LOGI("[nexEDitor_jni.cpp %d] checkValidate package name(%s)", __LINE__,pn);
    free(pn);
    */

   jclass pm_clazz = env->GetObjectClass( package_manager);
   jmethodID methodID_pm = env->GetMethodID( pm_clazz,    "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
   jobject package_info = env->CallObjectMethod(package_manager,methodID_pm, packageName, 64);

   jclass pi_cls = env->GetObjectClass( package_info);
    jfieldID fid = env->GetFieldID( pi_cls, "signatures", "[Landroid/content/pm/Signature;");
    jobjectArray signatures = (jobjectArray)env->GetObjectField( package_info, fid);
    jobject sign = env->GetObjectArrayElement( signatures, 0);

    jclass sign_cls = env->GetObjectClass( sign);
    jmethodID mid = env->GetMethodID(sign_cls, "toCharsString", "()Ljava/lang/String;");
    if(mid == NULL){
        LOGE("[nexEDitor_jni.cpp %d] toCharsString fail!", __LINE__);
        return -1;
     }

    jstring signString = (jstring)env->CallObjectMethod( sign, mid);
    char * ss = jstringTostring(env, signString,"7G*_}edU4#WM}P9:{X}5QoW");
    //printHexaLog("Signature String", (const unsigned char *)ss, strlen(ss));
    
    unsigned char szDigest[MD_SIZE];
    lsh512_encrypt( (const unsigned char *)ss, strlen(ss), szDigest );
    free(ss);
    //printHexaLog("Signature MD", szDigest, sizeof(szDigest));

    unsigned char validDigest[] ={
    			0xB4, 0xE5, 0x3F, 0x75, 0xBB, 0xF0, 0xFC, 0x28, 0xD4, 0x89, 0x93, 0x1A, 0x1E, 0x50, 0x4F, 0xF0,
    			0xE6, 0xB3, 0xD5, 0x86, 0x98, 0x6F, 0xE1, 0x8F, 0x3D, 0x67, 0x80, 0xB9, 0xA8, 0x46, 0xDE, 0x50,
    			0x36, 0x78, 0x08, 0xA5, 0x0A, 0xE6, 0x30, 0xEC, 0xD4, 0x49, 0xEF, 0x44, 0x9C, 0xF7, 0xA7, 0x00,
    			0xD4, 0x12, 0x82, 0xAA, 0xED, 0xA0, 0xE5, 0xC5, 0xB5, 0xC9, 0x40, 0x8B, 0x6A, 0x94, 0xD8, 0x21};

    srand(NX_CERT_VAL);
    s_cert_mismatch = rand();
    
	for( int j = 0 ; j < MD_SIZE ; j++ ){
		if( szDigest[j] != validDigest[j] ){
			s_cert_mismatch++;
		}
	}
#endif
   return 0;

}

#if defined(_ANDROID) && defined(FOR_PROJECT_Kinemaster) && !defined(NX_CERT_DEBUG_MODE)
unsigned char g_DexMDs2[DEX_FILES_CNT][ENC_SIZE+MD_SIZE/2] = {
	{0xFA,
		0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,
		0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA},
	{0xFA,
		0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,
		0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA},
	{0xFA,
		0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,
		0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA,	0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA}
};

unsigned char g_DexMDs1[DEX_FILES_CNT][ENC_SIZE+MD_SIZE/2] = {
	{0xFB,
		0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,
		0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB},
	{0xFB,
		0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,
		0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB},
	{0xFB,
		0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,
		0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB,	0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB, 0xFB}
};

unsigned char g_DexMDs[DEX_FILES_CNT][ENC_SIZE+MD_SIZE/2] = {
	{0xF1,
		0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1,	0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1,
		0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1,	0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1, 0xF1},
	{0xF2,
		0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,	0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
		0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,	0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2},
	{0xF3,
		0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,	0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,
		0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,	0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3}
};

unsigned char g_SoMD1[ENC_SIZE+MD_SIZE+ENC_SIZE+(4*4)] = {
	0xFE, 
		0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
		0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
		0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,
		0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE,

	0xFE, 
		0xFE, 0xFE, 0xFE, 0xFE,
		0xFE, 0xFE, 0xFE, 0xFE,
		0xFE, 0xFE, 0xFE, 0xFE,
		0xFE, 0xFE, 0xFE, 0xFE};
#endif

JNIEXPORT jint JNICALL _EDITOR_JNI_(createEditor)(JNIEnv * env, jobject obj,
    jstring strLibPath, jstring strModel, jint iAPILevel, jint iUserData, jintArray properties )
{
	LOGI("[nexEDitor_jni.cpp %d] createEditor (0x%p) Abi(%d) Version : %d.%d.%d(%s), %d.%d.%d", __LINE__,
		 g_VideoEditorHandle, NEXVIDEOEDITOR_ABI, NEXVIDEOEDITOR_VERSION_Majer, NEXVIDEOEDITOR_VERSION_Minor, NEXVIDEOEDITOR_VERSION_Patch, NEXVIDEOEDITOR_VERSION_DESC, NPV_MAJOR_VERSION, NPV_MINOR_VERSION, NPV_PATCH_VERSION);
#ifdef FOR_PROJECT_Kinemaster
#ifndef NX_CERT_DEBUG_MODE
    srand(NX_CERT_VAL);
    int r = rand();

    if( s_cert_mismatch != r )
    {
        LOGE("[nexEDitor_jni.cpp %d] createEditor mismatch count(%x, %x)", __LINE__,s_cert_mismatch, r);
        return 1;
    }
#endif
#endif

#ifndef FOR_PROJECT_Kinemaster
	if(checkSDKProtection())
	{
		return 1;
	}
#endif

	if( g_VideoEditorHandle )
	{
		SAFE_RELEASE(g_VideoEditorHandle);
	}

	const char *strPath = env->GetStringUTFChars(strLibPath, NULL);
	if( strPath == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] createEditor failed because did not get lib path", __LINE__);
		return 1;
	}
	
	const char *strModelName = env->GetStringUTFChars(strModel, NULL);
	if( strModelName == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] createEditor failed because did not get lib path", __LINE__);
		env->ReleaseStringUTFChars(strLibPath, strPath);
		return 1;
	}

	jint*pProperties = NULL;
	if( properties )
	{
		pProperties = env->GetIntArrayElements(properties, NULL);	
	}

	jclass      lclassSTL;
	lclassSTL = env->FindClass(SDK_PACKAGE_PATH "WrapperForSurfaceTextureListener");

	if( lclassSTL == NULL )
	{
		LOGE("[%s %d] Failed to find WrapperForSurfaceTextureListener class222", __func__, __LINE__);
		return 1;
	}

	if ( gclassSTL )
	{
		env->DeleteGlobalRef(gclassSTL);
	}
	gclassSTL = reinterpret_cast<jclass>(env->NewGlobalRef(lclassSTL));
	LOGE("[%s %d] WrapperForSurfaceTextureListener class was finded,%p", __func__, __LINE__,gclassSTL);


	env->DeleteLocalRef( lclassSTL );



	JavaVM* pVM = NULL;
	env->GetJavaVM(&pVM);
	g_VideoEditorHandle = CreateNexVideoEditor(	NEXEDITOR_INERFACE_MAJOR_VERSION, 
												NEXEDITOR_INERFACE_MINOR_VERSION,
												(char*)strPath, (char*)strModelName, iAPILevel, iUserData, pProperties, pVM,gclassSTL);

	if( pProperties )
	{
		env->ReleaseIntArrayElements(properties, pProperties, 0);
	}

	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] createEditor failed because did not create editor handle", __LINE__);
		env->ReleaseStringUTFChars(strLibPath, strPath);
		env->ReleaseStringUTFChars(strModel, strModelName);
		return 1;
	}

#if defined(_ANDROID) && defined(FOR_PROJECT_Kinemaster) && !defined(NX_CERT_DEBUG_MODE)
	if ( startPKGVerifier(env, 3, strPath, _EDITOR_JNI_(destroyEditor)) != NPV_ERROR_NONE )
		_EDITOR_JNI_(destroyEditor)(env, obj);
#endif

	env->ReleaseStringUTFChars(strLibPath, strPath);
	env->ReleaseStringUTFChars(strModel, strModelName);
	return 0;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(destroyEditor)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] destroyEditor", __LINE__);
	if( g_VideoEditorHandle != NULL )
		LOGI("[nexEDitor_jni.cpp %d] destroyEditor(%d)", __LINE__, g_VideoEditorHandle->GetRefCnt());

	SAFE_RELEASE(g_VideoEditorHandle);

	if ( gclassSTL )
	{
		env->DeleteGlobalRef(gclassSTL);
		gclassSTL = NULL;
	}
	return 0;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(commandMarker)(JNIEnv * env, jobject obj, jint iTag)
{
	LOGI("[nexEDitor_jni.cpp %d] commandMarker", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	g_VideoEditorHandle->commandMarker(iTag);
	return 0;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(prepareSurface)(JNIEnv * env, jobject obj, jobject surface)
{
	LOGI("[nexEDitor_jni.cpp %d] prepareSurface 0x%p", __LINE__, surface);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	ANativeWindow* nativeWindow = NULL;
	if( surface != NULL )
	{
		nativeWindow = ANativeWindow_fromSurface(env, surface);
	}

	jint ret = g_VideoEditorHandle->prepareSurface((void*)nativeWindow);
	if(nativeWindow) ANativeWindow_release(nativeWindow);

	return ret;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(changeSurface)(JNIEnv * env, jobject obj, jint iWidth, jint iHeight)
{
	LOGI("[nexEDitor_jni.cpp %d] changeSurface %d %d", __LINE__, iWidth, iHeight);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setVideoPosition(0, 0, iWidth, iHeight);
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(createProject)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] createProject", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->createProject();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(closeProject)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] closeProject", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->closeProject();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectEffect)(JNIEnv * env, jobject obj, jstring strEffectID)
{
	LOGI("[nexEDitor_jni.cpp %d] setProjectEffect", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	const char *str = env->GetStringUTFChars(strEffectID, NULL);
	if( str == NULL )
	{
		return 1;
	}

	int iRet = g_VideoEditorHandle->setProjectEffect((char*)str);

	env->ReleaseStringUTFChars(strEffectID, str);		
	return iRet;	
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(encodeProject)(JNIEnv * env, jobject obj, jstring strFilePath,
	jint iWidth, jint iHeight, jint iBitrate, jlong iMaxFileSize, jint iMaxFileDuration, jint iFPS, jint iProjectWidth, jint iProjectHeight, jint iSamplingRate, jint iAudioBitrate, jint iEncodeProfile, jint iEncodeLevel, jint iVideoCodecType, jint iFlag)
{
	LOGI("[nexEDitor_jni.cpp %d] encodeProject", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	const char *str = env->GetStringUTFChars(strFilePath, NULL);
	if( str == NULL )
	{
		return 1;
	}

	LOGI("[nexEDitor_jni.cpp %d] encodeProject(%s w:%d h:%d b:%d maxFile:%lld dur:%d fps:%d codec:%d) (%d %d) (%d %d) (0x%x)", __LINE__, str, iWidth, iHeight, iBitrate, iMaxFileSize, iMaxFileDuration, iFPS, iVideoCodecType, iProjectWidth, iProjectHeight, iEncodeProfile, iEncodeLevel, iFlag);
	g_VideoEditorHandle->setSaturation(g_VideoEditorHandle->getSaturation());
	g_VideoEditorHandle->setBrightness(g_VideoEditorHandle->getBrightness());
	g_VideoEditorHandle->setContrast(g_VideoEditorHandle->getContrast());
	int iRet = g_VideoEditorHandle->encodeProject((char*)str, iWidth, iHeight, iBitrate, iMaxFileSize, iMaxFileDuration, iFPS, iProjectWidth, iProjectHeight, iSamplingRate, iAudioBitrate, iEncodeProfile, iEncodeLevel, iVideoCodecType, iFlag);

	env->ReleaseStringUTFChars(strFilePath, str);
	return iRet;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(encodeProjectJpeg)(JNIEnv * env, jobject obj, jobject surface, jstring strFilePath, jint iWidth, jint iHeight, jint iQuality, jint iFlag)
{
	LOGI("[nexEDitor_jni.cpp %d] encodeProjectJpeg", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	ANativeWindow* nativeWindow = NULL;
	if( surface != NULL )
	{
		nativeWindow = ANativeWindow_fromSurface(env, surface);
	}

	if( nativeWindow == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] encodeProjectJpeg native window error", __LINE__);
		return 1;
	}

	const char *str = env->GetStringUTFChars(strFilePath, NULL);
	if( str == NULL )
	{
		return 1;
	}

	LOGI("[nexEDitor_jni.cpp %d] encodeProjectJpeg(%s w:%d h:%d q:%d f(0x%x)", __LINE__, str, iWidth, iHeight, iQuality, iFlag);
	g_VideoEditorHandle->setSaturation(g_VideoEditorHandle->getSaturation());
	g_VideoEditorHandle->setBrightness(g_VideoEditorHandle->getBrightness());
	g_VideoEditorHandle->setContrast(g_VideoEditorHandle->getContrast());
	int iRet = g_VideoEditorHandle->encodeProjectJpeg((void*)nativeWindow, (char*)str, iWidth, iHeight, iQuality, iFlag);
	// if(nativeWindow) ANativeWindow_release(nativeWindow);

	env->ReleaseStringUTFChars(strFilePath, str);
	return iRet;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(encodePause)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] encodePause", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->pauseExport();	
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(encodeResume)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] encodeResume", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->resumeExport();
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(addVisualClip)(JNIEnv * env, jobject obj, jint iNextToClipID, jstring strFilePath, jint iNewClipID)
{
	LOGI("[nexEDitor_jni.cpp %d] addVisualClip(%d)", __LINE__, iNextToClipID);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	if( strFilePath == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] addVisualClip failed because strFilePath was null", __LINE__);
		return 1;
	}

	const char *str = env->GetStringUTFChars(strFilePath, NULL);
	if( str == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] addVisualClip failed because strFilePath was null", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->addVisualClip(iNextToClipID, (char*)str, iNewClipID);

	env->ReleaseStringUTFChars(strFilePath, str);		
	return iRet;	
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(addAudioClip)(JNIEnv * env, jobject obj, jint iStartTime, jstring strFilePath, jint iNewClipID)
{
	LOGI("[nexEDitor_jni.cpp %d] addAudioClip StartTime(%d)", __LINE__, iStartTime);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	if( strFilePath == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] addAudioClip failed because strFilePath was null", __LINE__);
		return 1;
	}
	
	const char *str = env->GetStringUTFChars(strFilePath, NULL);
	if( str == NULL )
	{
		return 1;
	}

	int iRet = g_VideoEditorHandle->addAudioClip((unsigned int)iStartTime, (char*)str, iNewClipID);

	env->ReleaseStringUTFChars(strFilePath, str);		
	return iRet;	
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(moveVisualClip)(JNIEnv * env, jobject obj, jint iNextToClipID, jint iMovedClipID)
{
	LOGI("[nexEDitor_jni.cpp %d] moveVisualClip", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->moveVisualClip(iNextToClipID, iMovedClipID);
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(moveAudioClip)(JNIEnv * env, jobject obj, jint iStartTime, jint iMovedClipID)
{
	LOGI("[nexEDitor_jni.cpp %d] moveAudioClip(Clip : %d Time : %d)", __LINE__, iMovedClipID, iStartTime);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->moveAudioClip(iStartTime, iMovedClipID);
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(deleteClipID)(JNIEnv * env, jobject obj, jint iClipID)
{
	LOGI("[nexEDitor_jni.cpp %d] deleteClipID(%d)", __LINE__, iClipID);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->deleteClip(iClipID);
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(updateVisualClip)(JNIEnv * env, jobject obj, jobject clipInfo)
{
	LOGI("[nexEDitor_jni.cpp %d] updateVisualClip", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	jfieldID		id;	
	jclass		clipInfo_ref_class;
	clipInfo_ref_class = env->GetObjectClass(clipInfo);
	if( clipInfo_ref_class == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipType", "I");
	CLIP_TYPE clipType = (CLIP_TYPE)env->GetIntField(clipInfo, id);
	switch(clipType)
	{
		case CLIPTYPE_IMAGE:
		case CLIPTYPE_VIDEO:
			break;
		default:
			return 1;
	}

	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
	jint iClipID = env->GetIntField(clipInfo, id);

	IClipItem* pClip = pClipList->createEmptyClipUsingID((unsigned int)iClipID);
	if( pClip == NULL )
	{
		SAFE_RELEASE(pClipList);
		return 1;
	}

	pClip->setClipType(clipType);

	switch(clipType)
	{
		case CLIPTYPE_IMAGE:
		{
			id = env->GetFieldID(clipInfo_ref_class, "mTotalTime", "I");
			pClip->setTotalTime((unsigned int)env->GetIntField(clipInfo, id));

			id = env->GetFieldID(clipInfo_ref_class, "mClipPath", "Ljava/lang/String;");
			jstring mClipPath = (jstring)env->GetObjectField(clipInfo, id);
			if( mClipPath )
			{
				const char *str = env->GetStringUTFChars(mClipPath, NULL);
				if( str )
				{
					LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Image File Path(%s)", __LINE__, pClip->getClipPath());
					LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Image File Path(%s)", __LINE__, str);
					pClip->setClipPath((char*)str);
					env->ReleaseStringUTFChars(mClipPath, str);
				}
			}
			break;
		}
		case CLIPTYPE_VIDEO:
		{
			id = env->GetFieldID(clipInfo_ref_class, "mStartTrimTime", "I");
			pClip->setStartTrimTime((unsigned int)env->GetIntField(clipInfo, id));

			id = env->GetFieldID(clipInfo_ref_class, "mEndTrimTime", "I");
			pClip->setEndTrimTime((unsigned int)env->GetIntField(clipInfo, id));
			break;
		}
		default:
			break;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mExistAudio", "I");
	jint isAudio = (CLIP_TYPE)env->GetIntField(clipInfo, id);
	if( isAudio )
	{
		id = env->GetFieldID(clipInfo_ref_class, "mAudioOnOff", "I");
		pClip->setAudioOnOff((int)env->GetIntField(clipInfo, id));

		id = env->GetFieldID(clipInfo_ref_class, "mClipVolume", "I");
		pClip->setAudioVolume((int)env->GetIntField(clipInfo, id));

		id = env->GetFieldID(clipInfo_ref_class, "mBGMVolume", "I");
		pClip->setBGMVolume((int)env->GetIntField(clipInfo, id));
		id = env->GetFieldID(clipInfo_ref_class, "mPanLeft", "I");
		pClip->setPanLeftFactor((int)env->GetIntField(clipInfo, id));
		id = env->GetFieldID(clipInfo_ref_class, "mPanRight", "I");
		pClip->setPanRightFactor((int)env->GetIntField(clipInfo, id));

		// RYU 20130604. set need to set AudioEnvelop when using it.
		//id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeOnOff", "I");
		//pClip->setAudioEnvelopOnOff((int)env->GetIntField(clipInfo, id));

		jint* pVolumeEnvelopLevel = NULL;
		int nVolumeEnvelopeSize = 0;
		jint* pVolumeEnvelopTime = NULL;

		id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeLevel", "[I");
		 jintArray arrVolumeEnvelopeLevel = (jintArray) env->GetObjectField(clipInfo, id);
		 if(arrVolumeEnvelopeLevel != NULL)
		 {
			pVolumeEnvelopLevel = env->GetIntArrayElements(arrVolumeEnvelopeLevel, NULL);
			nVolumeEnvelopeSize = env->GetArrayLength( arrVolumeEnvelopeLevel);

		 }
		id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeTime", "[I");
		jintArray arrVolumeEnvelopeTime = (jintArray) env->GetObjectField(clipInfo, id);
		if( arrVolumeEnvelopeTime != NULL)
		{
			pVolumeEnvelopTime = env->GetIntArrayElements(arrVolumeEnvelopeTime, NULL);
		}

		if( nVolumeEnvelopeSize > 0 && pVolumeEnvelopTime != NULL && pVolumeEnvelopLevel != NULL )
			pClip->setAudioEnvelop(nVolumeEnvelopeSize, (unsigned int*)pVolumeEnvelopTime, (unsigned int*)pVolumeEnvelopLevel);
		
		// Release Envelope Array.
		if(pVolumeEnvelopTime != NULL)
			env->ReleaseIntArrayElements(arrVolumeEnvelopeTime, pVolumeEnvelopTime, 0);

		if(pVolumeEnvelopLevel != NULL)
			env->ReleaseIntArrayElements(arrVolumeEnvelopeLevel, pVolumeEnvelopLevel, 0);
		
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEffectDuration", "I");
	pClip->setClipEffectDuration((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEffectOffset", "I");
	pClip->setClipEffectOffset((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEffectOverlap", "I");
	pClip->setClipEffectOverlap((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mClipEffectID", "Ljava/lang/String;");
	jstring mClipEffectID = (jstring)env->GetObjectField(clipInfo, id);
	if( mClipEffectID )
	{
		const char *str = env->GetStringUTFChars(mClipEffectID, NULL);
		if( str == NULL )
		{
			SAFE_RELEASE(pClip);
			SAFE_RELEASE(pClipList);
			return 1;
		}
		pClip->setClipEffectID((char*)str);
		env->ReleaseStringUTFChars(mClipEffectID, str);
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Clip Effect ID(%s) (Duration: %d, Offset:%d, Overlap: %d)", __LINE__, pClip->getClipEffectID(), pClip->getClipEffectDuration(), pClip->getClipEffectOffset(), pClip->getClipEffectOverlap());
	}
	else
	{
		pClip->setClipEffectID((char*)"");
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Clip Effect ID(%s)", __LINE__, pClip->getClipEffectID());
	}

	id = env->GetFieldID(clipInfo_ref_class, "mTitleEffectID", "Ljava/lang/String;");
	jstring mTitleEffectID = (jstring)env->GetObjectField(clipInfo, id);
	if( mTitleEffectID )
	{
		const char *str = env->GetStringUTFChars(mTitleEffectID, NULL);
		if( str == NULL )
		{
			SAFE_RELEASE(pClip);
			SAFE_RELEASE(pClipList);
			return 1;
		}
		pClip->setTitleEffectID((char*)str);
		env->ReleaseStringUTFChars(mTitleEffectID, str);
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Title Effect ID(%s)", __LINE__, pClip->getTitleEffectID());
	}
	else
	{
		pClip->setTitleEffectID((char*)"");
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Title Effect ID(%s)", __LINE__, pClip->getTitleEffectID());
	}

	id = env->GetFieldID(clipInfo_ref_class, "mFilterID", "Ljava/lang/String;");
	jstring mFilterID = (jstring)env->GetObjectField(clipInfo, id);
	if( mFilterID )
	{
		const char *str = env->GetStringUTFChars(mFilterID, NULL);
		if( str == NULL )
		{
			SAFE_RELEASE(pClip);
			SAFE_RELEASE(pClipList);
			return 1;
		}
		pClip->setFilterID((char*)str);
		env->ReleaseStringUTFChars(mFilterID, str);
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Filter ID(%s)", __LINE__, pClip->getFilterID());
	}
	else
	{
		pClip->setFilterID((char*)"");
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Filter ID(%s)", __LINE__, pClip->getFilterID());
	}

	id = env->GetFieldID(clipInfo_ref_class, "mRotateState", "I");
	int iRotate = (int)env->GetIntField(clipInfo, id);
	pClip->setRotateState(iRotate);

	id = env->GetFieldID(clipInfo_ref_class, "mBrightness", "I");
	pClip->setBrightness((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mContrast", "I");
	pClip->setContrast((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mSaturation", "I");
	pClip->setSaturation((int)env->GetIntField(clipInfo, id));

    id = env->GetFieldID(clipInfo_ref_class, "mHue", "I");
	pClip->setHue((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTintcolor", "I");
	pClip->setTintcolor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mLUT", "I");
	pClip->setLUT((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCustomLUT_A", "I");
	pClip->setCustomLUTA((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCustomLUT_B", "I");
	pClip->setCustomLUTB((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCustomLUT_Power", "I");
	pClip->setCustomLUTPower((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mVignette", "I");
	pClip->setVignette((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mSpeedControl", "I");
	pClip->setSpeedCtlFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mKeepPitch", "I");
	pClip->setKeepPitch((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mVoiceChanger", "I");
	pClip->setVoiceChangerFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCompressor", "I");
	pClip->setCompressorFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mPitchFactor", "I");
	pClip->setPitchFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mMusicEffector", "I");
	pClip->setMusicEffector((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mProcessorStrength", "I");
	pClip->setProcessorStrength((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mBassStrength", "I");
	pClip->setBassStrength((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTitleStyle", "I");
	pClip->setTitleStyle((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mTitleStartTime", "I");
	pClip->setTitleStartTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTitleEndTime", "I");
	pClip->setTitleEndTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEnhancedAudioFilter", "Ljava/lang/String;");
	jstring sEnhancedAudioFilter = (jstring)env->GetObjectField(clipInfo, id);
	if( sEnhancedAudioFilter )
	{
		const char *str = env->GetStringUTFChars(sEnhancedAudioFilter, NULL);
		if( str == NULL )
		{
			SAFE_RELEASE(pClip);
			SAFE_RELEASE(pClipList);
			return 1;
		}
		pClip->setEnhancedAudioFilter((char*)str);
		env->ReleaseStringUTFChars(sEnhancedAudioFilter, str);
	}
	else
	{
		pClip->setEnhancedAudioFilter((char*)"");
	}
	//LOGI("[nexEDitor_jni.cpp %d] updateVisualClip EnhancedAudioFilter (%s, %s)", __LINE__, sEnhancedAudioFilter, pClip->getEnhancedAudioFilter());

	id = env->GetFieldID(clipInfo_ref_class, "mEqualizer", "Ljava/lang/String;");
	jstring sEqualizer = (jstring)env->GetObjectField(clipInfo, id);
	if( sEqualizer )
	{
		const char *str = env->GetStringUTFChars(sEqualizer, NULL);
		if( str == NULL )
		{
			SAFE_RELEASE(pClip);
			SAFE_RELEASE(pClipList);
			return 1;
		}
		pClip->setEqualizer((char*)str);
		env->ReleaseStringUTFChars(sEqualizer, str);
	}
	else
	{
		pClip->setEqualizer((char*)"");
	}
	//LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Equalizer (%s, %s)", __LINE__, sEqualizer, pClip->getEqualizer());

	id = env->GetFieldID(clipInfo_ref_class, "mTitle", "Ljava/lang/String;");
	jstring title = (jstring)env->GetObjectField(clipInfo, id);
	if( title )
	{
		const char *str = env->GetStringUTFChars(title, NULL);
		if( str == NULL )
		{
			SAFE_RELEASE(pClip);
			SAFE_RELEASE(pClipList);
			return 1;
		}
		pClip->setTitle((char*)str);
		env->ReleaseStringUTFChars(title, str);
	}
	else
	{
		pClip->setTitle((char*)"");
		// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip Title (%s)", __LINE__, pClip->getTitle());
	}


	jfloatArray arrMatrix = NULL;
	jfloat* pMatrix = NULL;
	jint iMatrixSize = 0;

	id = env->GetFieldID(clipInfo_ref_class, "mStartMatrix", "[F");
	arrMatrix = (jfloatArray) env->GetObjectField(clipInfo, id);
	if( arrMatrix != NULL)
	{
		iMatrixSize = env->GetArrayLength( arrMatrix);
		pMatrix = env->GetFloatArrayElements(arrMatrix, NULL);
		if( pMatrix != NULL )
		{
			pClip->setStartMatrix(pMatrix, iMatrixSize);
			env->ReleaseFloatArrayElements(arrMatrix, pMatrix, 0);
		}
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEndMatrix", "[F");
	arrMatrix = (jfloatArray) env->GetObjectField(clipInfo, id);
	if( arrMatrix != NULL)
	{
		iMatrixSize = env->GetArrayLength( arrMatrix);
		pMatrix = env->GetFloatArrayElements(arrMatrix, NULL);
		if( pMatrix != NULL )
		{
			pClip->setEndMatrix(pMatrix, iMatrixSize);
			env->ReleaseFloatArrayElements(arrMatrix, pMatrix, 0);
		}
	}
	
	jint iLeft		= 0;
	jint iTop		= 0;
	jint iRight	= 0;
	jint iBottom	= 0;
	
	id = env->GetFieldID(clipInfo_ref_class, "mStartRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");

	jobject objRect = env->GetObjectField(clipInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			IRectangle* pRect = pClip->getStartPosition();
			if( pRect )
			{
				pRect->setRect(iLeft, iTop, iRight, iBottom);
				pRect->Release();
			}
			env->DeleteLocalRef(rectangle_ref_class);
			// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip StartRect(%d %d %d %d)", __LINE__, iLeft, iTop, iRight, iBottom);
		}
		env->DeleteLocalRef(objRect);		
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEndRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");
	
	objRect = env->GetObjectField(clipInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			IRectangle* pRect = pClip->getEndPosition();
			if( pRect )
			{
				pRect->setRect(iLeft, iTop, iRight, iBottom);
				pRect->Release();
			}
			env->DeleteLocalRef(rectangle_ref_class);

			// LOGI("[nexEDitor_jni.cpp %d] updateVisualClip EndRect(%d %d %d %d)", __LINE__, iLeft, iTop, iRight, iBottom);
		}
		env->DeleteLocalRef(objRect);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mDestRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");

	objRect = env->GetObjectField(clipInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			IRectangle* pRect = pClip->getDstPosition();
			if( pRect )
			{
				pRect->setRect(iLeft, iTop, iRight, iBottom);
				pRect->Release();
			}
			env->DeleteLocalRef(rectangle_ref_class);			
		}
		env->DeleteLocalRef(objRect);		
	}	

	id = env->GetFieldID(clipInfo_ref_class, "mSlowMotion", "I");
	pClip->setSlowMotion((int)env->GetIntField(clipInfo, id));

	g_VideoEditorHandle->updateClipInfo((IClipItem*)pClip);
	SAFE_RELEASE(pClip);
	SAFE_RELEASE(pClipList);

	return 0;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(updateAudioClip)(JNIEnv * env, jobject obj, jobject clipInfo)
{
	LOGI("[nexEDitor_jni.cpp %d] updateAudioClip", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	jfieldID		id;	
	jclass		clipInfo_ref_class;
	clipInfo_ref_class = env->GetObjectClass(clipInfo);
	if( clipInfo_ref_class == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipType", "I");
	CLIP_TYPE clipType = (CLIP_TYPE)env->GetIntField(clipInfo, id);
	switch(clipType)
	{
		case CLIPTYPE_AUDIO:
			break;
		default:
			LOGE("[nexEDitor_jni.cpp %d] updateAudioClip invalid audio type Clip type", __LINE__);
			return 1;
	}

	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
	int iClipID = (int)env->GetIntField(clipInfo, id);

	IClipItem* pClip = pClipList->createEmptyClipUsingID((unsigned int)iClipID);
	if( pClip == NULL )
	{
		LOGE("[nexEDitor_jni.cpp %d] Can not find audio Clip ID(%d)", __LINE__, iClipID);
		SAFE_RELEASE(pClipList);
		return 1;
	}

	pClip->setClipType(clipType);

	jint iStartTime		= 0;
	jint iEndTime			= 0;
	jint iStartTrimTime	= 0;
	jint iEndTrimTime	= 0;
	jint iAudioOnOff		= 0;
	jint iClipVolume		= 0;
	jintArray arrVolumeEnvelopeLevel = NULL;
	jintArray arrVolumeEnvelopeTime = NULL;
	jint* pVolumeEnvelopLevel = NULL;
	jint* pVolumeEnvelopTime = NULL;
	int nVolumeEnvelopeSize = 0;
	//int nVolumeEnvelopeOnOff = 0;

	jint iPanLeftFactor	= -1000;
	jint iPanRightFactor	= -1000;

	id = env->GetFieldID(clipInfo_ref_class, "mPanLeft", "I");
	iPanLeftFactor = env->GetIntField(clipInfo, id);
	id = env->GetFieldID(clipInfo_ref_class, "mPanRight", "I");
	iPanRightFactor = env->GetIntField(clipInfo, id);

	id = env->GetFieldID(clipInfo_ref_class, "mStartTime", "I");
	iStartTime = env->GetIntField(clipInfo, id);

	id = env->GetFieldID(clipInfo_ref_class, "mEndTime", "I");
	iEndTime = env->GetIntField(clipInfo, id);

	id = env->GetFieldID(clipInfo_ref_class, "mStartTrimTime", "I");
	iStartTrimTime = env->GetIntField(clipInfo, id);

	id = env->GetFieldID(clipInfo_ref_class, "mEndTrimTime", "I");
	iEndTrimTime = env->GetIntField(clipInfo, id);
	
	id = env->GetFieldID(clipInfo_ref_class, "mAudioOnOff", "I");
	iAudioOnOff = env->GetIntField(clipInfo, id);

	id = env->GetFieldID(clipInfo_ref_class, "mClipVolume", "I");
	iClipVolume = env->GetIntField(clipInfo, id);

	// RYU 20130604. set need to set AudioEnvelop when using it.
	//id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeOnOff", "I");
	//nVolumeEnvelopeOnOff = env->GetIntField(clipInfo, id);

	id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeLevel", "[I");
	arrVolumeEnvelopeLevel = (jintArray) env->GetObjectField(clipInfo, id);

	if( arrVolumeEnvelopeLevel != NULL)
	{
		pVolumeEnvelopLevel = env->GetIntArrayElements(arrVolumeEnvelopeLevel, NULL);
		nVolumeEnvelopeSize = env->GetArrayLength( arrVolumeEnvelopeLevel);
	}
		
	id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeTime", "[I");
	arrVolumeEnvelopeTime = (jintArray) env->GetObjectField(clipInfo, id);
	if( arrVolumeEnvelopeTime != NULL)
	{
		pVolumeEnvelopTime = env->GetIntArrayElements(arrVolumeEnvelopeTime, NULL);
	}


	pClip->setStartTime((unsigned int)iStartTime);
	pClip->setEndTime((unsigned int)iEndTime);
	pClip->setStartTrimTime((unsigned int)iStartTrimTime);
	pClip->setEndTrimTime((unsigned int)iEndTrimTime);
	pClip->setAudioOnOff((int)iAudioOnOff);
	pClip->setAudioVolume((int)iClipVolume);

	pClip->setPanLeftFactor((int)iPanLeftFactor);
	pClip->setPanRightFactor((int)iPanRightFactor);

	if( nVolumeEnvelopeSize > 0 && pVolumeEnvelopTime != NULL && pVolumeEnvelopLevel != NULL )
		pClip->setAudioEnvelop(nVolumeEnvelopeSize, (unsigned int*)pVolumeEnvelopTime, (unsigned int*)pVolumeEnvelopLevel);
	//pClip->setAudioEnvelopOnOff(nVolumeEnvelopeOnOff);

    //TODO: yoon speed contorl
   	id = env->GetFieldID(clipInfo_ref_class, "mSpeedControl", "I");
	pClip->setSpeedCtlFactor((int)env->GetIntField(clipInfo, id));


	// Release Envelope Array.
	if(pVolumeEnvelopTime != NULL)
		env->ReleaseIntArrayElements(arrVolumeEnvelopeTime, pVolumeEnvelopTime, 0);

	if(pVolumeEnvelopLevel != NULL)
		env->ReleaseIntArrayElements(arrVolumeEnvelopeLevel, pVolumeEnvelopLevel, 0);



	LOGE("[nexEDitor_jni.cpp %d] updateAudioClip Info(id:%d Time(%d %d) TrimTime(%d %d) OnOff(%d) Volume(%d))", 
		__LINE__, iClipID, iStartTime, iEndTime, iStartTrimTime, iEndTrimTime, iAudioOnOff, iClipVolume);

	g_VideoEditorHandle->updateClipInfo(pClip);
	SAFE_RELEASE(pClip);
	SAFE_RELEASE(pClipList);

	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setBackgroundMusic)(JNIEnv * env, jobject obj, jstring strFilePath, jint iNewClipID)
{
	LOGI("[nexEDitor_jni.cpp %d] setBackgroundMusic", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	
	if( strFilePath == NULL )
	{
		int iRet = g_VideoEditorHandle->setBackgroundMusic("", -1);
		LOGI("[nexEDitor_jni.cpp %d] clear setBackgroundMusic ", __LINE__);
		return 0;
	}

	const char *str = env->GetStringUTFChars(strFilePath, NULL);
	int iRet = g_VideoEditorHandle->setBackgroundMusic(str, iNewClipID);
	env->ReleaseStringUTFChars(strFilePath, str);
	LOGI("[nexEDitor_jni.cpp %d] setBackgroundMusic Out", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setBackgroundMusicVolume)(JNIEnv * env, jobject obj, int iVolume, int iFadeInTime, int iFadeOutTime)
{
	LOGI("[nexEDitor_jni.cpp %d] setBackgroundMusicVolume", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->setBackgroundMusicVolume(iVolume, iFadeInTime, iFadeOutTime);
	LOGI("[nexEDitor_jni.cpp %d] setBackgroundMusicVolume Out", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectVolumeFade)(JNIEnv * env, jobject obj, int iFadeInTime, int iFadeOutTime)
{
	LOGI("[nexEDitor_jni.cpp %d] setProjectVolumeFade(%d %d)", __LINE__, iFadeInTime, iFadeOutTime);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->setProjectVolumeFade(iFadeInTime, iFadeOutTime);
	LOGI("[nexEDitor_jni.cpp %d] setProjectVolumeFade Out", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectVolume)(JNIEnv * env, jobject obj, int iProjectVolume)
{
	LOGI("[nexEDitor_jni.cpp %d] setProjectVolume(%d)", __LINE__, iProjectVolume);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->setProjectVolume(iProjectVolume);
	LOGI("[nexEDitor_jni.cpp %d] setProjectVolume Out", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setProjectManualVolumeControl)(JNIEnv * env, jobject obj, int iManualVolumeControl)
{
	LOGI("[nexEDitor_jni.cpp %d] setProjectManualVolumeControl(%d)", __LINE__, iManualVolumeControl);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->setProjectManualVolumeControl(iManualVolumeControl);
	LOGI("[nexEDitor_jni.cpp %d] setProjectManualVolumeControl Out", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getDuration)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d]", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 0;
	}
	return (int)(g_VideoEditorHandle->getDuration()/1000);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setTime)(JNIEnv * env, jobject obj, jint iTime, jint iDisplay, jint iIDRFrame)
{
	LOGI("[nexEDitor_jni.cpp %d] setTime(Time:%d Display:%d IDRFrame:%d)", __LINE__, iTime, iDisplay, iIDRFrame);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->setTime((unsigned int)iTime, iDisplay, iIDRFrame);
}

JNIEXPORT jbyteArray JNICALL _EDITOR_JNI_(getTimeThumbData)(JNIEnv * env, jobject obj, jint iTime)
{
	LOGI("[nexEDitor_jni.cpp %d]getTimeThumbData(Time:%d)", __LINE__, iTime);
	jbyteArray arr;
	
	if( g_VideoEditorHandle )
	{
		unsigned char* pData = g_VideoEditorHandle->setTimeThumbData((unsigned int)iTime);
		if(pData)
		{
			int iSize = 320*240*3/2;
			arr = env->NewByteArray(iSize);
			env->SetByteArrayRegion(arr, 0, iSize, (const jbyte*)pData);
  		//env->DeleteLocalRef(arr);
		}
		else
		{
			int iSize = 0;
			arr = env->NewByteArray(iSize);
			LOGI("[nexEDitor_jni.cpp %d] getTimeThumbData() data is null", __LINE__);
  		//	env->DeleteLocalRef(arr);
		}
	}
	else
	{
		LOGI("[nexEDitor_jni.cpp %d] getTimeThumbData() Invalid VideoEditor handle", __LINE__);
		arr = env->NewByteArray(0);
		//env->DeleteLocalRef(arr);
	}
	return arr;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(captureCurrentFrame)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] captureCurrentFrame", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	return g_VideoEditorHandle->captureCurrentFrame();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(startPlay)(JNIEnv * env, jobject obj, jint iMuteAudio)
{
	LOGI("[nexEDitor_jni.cpp %d] StartPlay In(%d)",__LINE__, iMuteAudio);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	LOGI("[nexEDitor_jni.cpp %d] StartPlay Out",__LINE__);
	return g_VideoEditorHandle->startPlay(iMuteAudio);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(pausePlay)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] pausePlay In",__LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	LOGI("[nexEDitor_jni.cpp %d] pausePlay Out",__LINE__);
	return g_VideoEditorHandle->pausePlay();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(resumePlay)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] resumePlay In",__LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	LOGI("[nexEDitor_jni.cpp %d] resumePlay Out",__LINE__);
	return g_VideoEditorHandle->resumePlay();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(stopPlay)(JNIEnv * env, jobject obj, jint iFlag)
{
	LOGI("[nexEDitor_jni.cpp %d] stopPlay In(%d)", __LINE__,iFlag);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}
	int iRet = g_VideoEditorHandle->stopPlay(iFlag);
	LOGI("[nexEDitor_jni.cpp %d] stopPlay Out(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jobject JNICALL _EDITOR_JNI_(getVisualClip)(JNIEnv * env, jobject obj, jint iClipID)
{
	LOGI("[nexEditor_jni.cpp %d] getVisualClip",__LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return NULL;
	}

	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getVisualClip failed(clip list is null)", __LINE__);
		return NULL;
	}

	IClipItem* pClip = pClipList->getClipUsingID(iClipID);
	if( pClip == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getVisualClip failed(clip not found(ClipID:%d)", __LINE__, iClipID);
		return NULL;
	}

	SAFE_RELEASE(pClipList);

	jobject visualObj = createVisualClip(env, pClip);
	SAFE_RELEASE(pClip);
	return visualObj;
}

JNIEXPORT jobject JNICALL _EDITOR_JNI_(getAudioClip)(JNIEnv * env, jobject obj, jint iClipID)
{
	LOGI("[nexEditor_jni.cpp %d] getAudioClip",__LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return NULL;
	}

	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		return NULL;
	}

	IClipItem* pClip = pClipList->getClipUsingID(iClipID);
	if( pClip == NULL )
	{
		return NULL;
	}

	jobject audioObj = createAudioClip(env, pClip);
	pClip->Release();
	return audioObj;
	
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipList)(JNIEnv * env, jobject obj, jobject manager)
{
	LOGI("[nexEditor_jni.cpp %d] getClipList",__LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	LOGI("[nexEditor_jni %d] getClipList", __LINE__);
	jclass manager_class_ref = env->GetObjectClass(manager);
	if( manager_class_ref == NULL )
	{
		LOGI("[getClipList %d] GetManager Class failed",__LINE__);
		return 1;
	}

	LOGI("[nexEditor_jni %d] getClipList", __LINE__);
	jmethodID clearClipMethodID = env->GetMethodID(manager_class_ref, "clearAllClip", "()V");
	if( clearClipMethodID == NULL )
	{
		LOGI("[getClipList %d] get addClipMethodID failed",__LINE__);
		return 1;
	}
	env->CallVoidMethod(manager, clearClipMethodID);

	LOGI("[nexEditor_jni %d] getClipList", __LINE__);

	jmethodID addClipMethodID = env->GetMethodID(manager_class_ref, "addClip", "(L" INTERNAL_SDK_PACKAGE_PATH "NexVisualClip;)I");
	
	if( addClipMethodID == NULL )
	{
		LOGI("[getClipList %d] get addClipMethodID failed",__LINE__);
		return 1;
	}

	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL ) return 1;

	LOGI("[nexEditor_jni %d] getClipList", __LINE__);
	for( int i = 0; i < pClipList->getClipCount(); i++)
	{
		IClipItem* pClip = pClipList->getClip(i);
		if( pClip == NULL ) continue;

		jobject obj = createVisualClip(env, pClip);
		if( obj == NULL )
		{
			SAFE_RELEASE(pClip);
			LOGI("[nexEditor_jni.cpp %d] Create VideoClip Object failed",__LINE__);
			continue;
		}

		LOGI("[nexEditor_jni %d] getClipList Addclip(0x%p)", __LINE__, addClipMethodID);
		env->CallIntMethod(manager, addClipMethodID, obj);
		LOGI("[nexEditor_jni %d] getClipList", __LINE__);
		
		if( pClip->getAudioClipCount() > 0 )
		{
			for(int j = 0; j < pClip->getAudioClipCount(); j++)
			{
				IClipItem* pAudioClip = pClip->getAudioClip(j);
				if( pAudioClip == NULL )
				{
					continue;
				}
				jobject objaudio = createAudioClip(env, pAudioClip);
				if( objaudio ==  NULL )
				{
					LOGI("[nexEditor_jni.cpp %d] Create AudioClip Object failed",__LINE__);
					continue;
				}
				jclass cls = env->GetObjectClass(obj);

				jmethodID addMethodID = env->GetMethodID(cls, "addAudioClip", "(L" INTERNAL_SDK_PACKAGE_PATH "NexAudioClip;)I");
				
				if( addMethodID == NULL )
				{
					SAFE_RELEASE(pAudioClip);
					continue;
				}
				env->CallIntMethod(obj, addMethodID, objaudio);
				SAFE_RELEASE(pAudioClip);

				env->DeleteLocalRef(objaudio);
			}
		}

		env->DeleteLocalRef(obj);
		SAFE_RELEASE(pClip);
	}

	SAFE_RELEASE(pClipList);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(loadClipList)(JNIEnv * env, jobject obj, jobjectArray arrayVisualClip, jobjectArray arrayAudioClip)
{
	LOGI("[nexEDitor_jni.cpp %d] loadClipList", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] GetClipList failed", __LINE__);
		return 1;
	}

	pClipList->lockClipList();
	pClipList->clearClipList();

	g_VideoEditorHandle->clearProject();

	if( arrayVisualClip == NULL  )
	{
		pClipList->unlockClipList();
		SAFE_RELEASE(pClipList);
		g_VideoEditorHandle->updateClipInfo();
		LOGI("[nexEDitor_jni.cpp %d] loadClipList(Cliplist clear because Visual Clip array is null)", __LINE__);
		return 0;		
	}

	int iArrayCount = env->GetArrayLength(arrayVisualClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayVisualClip, i);
		if( obj == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);

		IClipItem* pItem = pClipList->createClipUsingID(iClipID);
		if( pItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			continue;			
		}
		
		setVisualClip(env, objClip, pItem);
		pItem->printClipInfo();
		pItem->Release();
		env->DeleteLocalRef(objClip);
		env->DeleteLocalRef(clipInfo_ref_class);
	}

	if( arrayAudioClip == NULL )
	{
		pClipList->unlockClipList();
		SAFE_RELEASE(pClipList);
		g_VideoEditorHandle->updateClipInfo();
		LOGI("[nexEDitor_jni.cpp %d] loadClipList End", __LINE__);
		return 0;
	}

	iArrayCount = env->GetArrayLength(arrayAudioClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayAudioClip, i);
		if( obj == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);

		id = env->GetFieldID(clipInfo_ref_class, "mStartTime", "I");
		jint iStartTime = env->GetIntField(objClip, id);

		id = env->GetFieldID(clipInfo_ref_class, "mVisualClipID", "I");
		jint iVisualClipID = env->GetIntField(objClip, id);

		IClipItem* pVisualItem = pClipList->getClipUsingID(iVisualClipID);
		if( pVisualItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			continue;
		}

		IClipItem* pItem = (IClipItem*)pClipList->createClipUsingID(iClipID, 1);
		if( pItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			pVisualItem->Release();
			continue;
		}

		setAudioClip(env, objClip, pItem);

		pVisualItem->addAudioClip(pItem);
		pVisualItem->Release();
		
		pItem->printClipInfo();
		pItem->Release();
		
		env->DeleteLocalRef(objClip);
		env->DeleteLocalRef(clipInfo_ref_class);
	}
	
	pClipList->unlockClipList();
	SAFE_RELEASE(pClipList);
	g_VideoEditorHandle->updateClipInfo();
	LOGI("[nexEDitor_jni.cpp %d] loadClipList End", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(loadTheme)(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData, jint iFlags)
{
	LOGI("[nexEDitor_jni.cpp %d] loadTheme iFlags(%d)", __LINE__, iFlags);


	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	const char *strData = env->GetStringUTFChars(strEffectData, NULL);
	if( strData == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Theme Resource file", __LINE__);
		return 1;
	}

	/*
	const char *strID = env->GetStringUTFChars(strEffectID, NULL);
	if( strID == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Theme Resource file", __LINE__);
		env->ReleaseStringUTFChars(strEffectData, strData);		
		return 1;
	}
	*/

	int iRet = g_VideoEditorHandle->loadTheme((char*)strData, iFlags);

	env->ReleaseStringUTFChars(strEffectData, strData);		
	// env->ReleaseStringUTFChars(strEffectID, strID);		
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearRenderItems)(JNIEnv * env, jobject obj, jint iFlags)
{
	LOGI("[nexEDitor_jni.cpp %d] clearRenderItems iFlags(%d)", __LINE__, iFlags);


	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->clearRenderItems(iFlags);

	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(loadRenderItem)(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData, jint iFlags)
{
	LOGI("[nexEDitor_jni.cpp %d] loadTheme iFlags(%d)", __LINE__, iFlags);


	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	const char *strData = env->GetStringUTFChars(strEffectData, NULL);
	if( strData == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Theme Resource file", __LINE__);
		return 1;
	}

	const char *strID = env->GetStringUTFChars(strEffectID, NULL);
	if( strID == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Theme Resource file", __LINE__);
		env->ReleaseStringUTFChars(strEffectData, strData);		
		return 1;
	}

	int iRet = g_VideoEditorHandle->loadRenderItem((char*)strID, (char*)strData, iFlags);

	env->ReleaseStringUTFChars(strEffectData, strData);		
	env->ReleaseStringUTFChars(strEffectID, strID);		
	return iRet;
}

JNIEXPORT jstring JNICALL _EDITOR_JNI_(getLoadThemeError)(JNIEnv * env, jobject obj, jint iFlags)
{
	LOGI("[nexEDitor_jni.cpp %d] getThemeLoadError", __LINE__);

	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		jstring jstrValue = env->NewStringUTF("Invalid editor handle");
		return jstrValue;	
	}

	char* pStr = g_VideoEditorHandle->getLoadThemeError(iFlags);
	jstring jstrValue = env->NewStringUTF(pStr);
	return jstrValue;	
}

JNIEXPORT jstring JNICALL _EDITOR_JNI_(getThemeID)(JNIEnv * env, jobject obj)
{
	// RYU 20130529 remove functions related with theme.
	LOGI("[nexEDitor_jni.cpp %d]getThemeID is not supported!", __LINE__);
	return NULL;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setThemeID)(JNIEnv * env, jobject obj, jstring strThemeID)
{
	// RYU 20130529 remove functions related with theme.
	LOGI("[nexEDitor_jni.cpp %d] setThemeID is not supported.", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getThemeList)(JNIEnv * env, jobject obj, jobject manager)
{
	// RYU 20130529 remove functions related with theme.
	LOGI("[nexEDitor_jni.cpp %d] getThemeList is not supported!", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipEffectList)(JNIEnv * env, jobject obj, jobject manager)
{
	// RYU 20130529 remove functions related with theme.
	LOGI("[nexEDitor_jni.cpp %d] getClipEffectList is not supported!", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getTitleEffectList)(JNIEnv * env, jobject obj, jobject manager)
{
	// RYU 20130529 remove functions related with theme.
	LOGI("[nexEDitor_jni.cpp%d] getTitleEffectList is not supported!", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setEventHandler)(JNIEnv * env, jobject obj, jobject eventListener)
{
	LOGI("[nexEditor_jni.cpp %d] setEventHandler In", __LINE__);
	if( g_VideoEditorHandle == NULL || eventListener == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	CNexVideoEditoerEventHandler* pEventHandler = new CNexVideoEditoerEventHandler;
	if( pEventHandler == NULL )
	{
		return 1;
	}

	if( pEventHandler->setEventObject(eventListener) !=  0 )
	{
		LOGI("[nexEditor_jni.cpp %d] set EventListener failed", __LINE__);
		return 1;
	}
	int iRet = g_VideoEditorHandle->setEventHandler((INexVideoEditorEventHandler *) pEventHandler);
	SAFE_RELEASE(pEventHandler);
	LOGI("[nexEditor_jni.cpp %d] setEventHandler Out(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipInfo)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jobject clipInfo, jint isVideoThumbnail, jint isAudioPCMLevel, jint isBackground, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipInfo option(%d %d %d %d)", __LINE__, isVideoThumbnail, isAudioPCMLevel, isBackground, iUserTag);

	if( g_VideoEditorHandle == NULL )
	{
		return 1;
	}
	const char *str = env->GetStringUTFChars(clipPath, NULL);
	if( str == NULL )
	{
		return 1;
	}

	const char *strthumb= env->GetStringUTFChars(pThumbFilePath, NULL);
	if( strthumb == NULL )
	{
		env->ReleaseStringUTFChars(clipPath, str);
		return 1;
	}	

	IClipInfo* pClipInfo = g_VideoEditorHandle->getClipInfo((char*)str, (char*)strthumb, isVideoThumbnail, isAudioPCMLevel, isBackground, iUserTag);
	env->ReleaseStringUTFChars(clipPath, str);
	env->ReleaseStringUTFChars(pThumbFilePath, strthumb);

	if( pClipInfo == NULL )
	{
		if( isBackground )
		{
			return 0;
		}
		return 1;
	}
	
	jfieldID id;
	jclass cls = env->GetObjectClass(clipInfo);

	if( pClipInfo->existVideo() )
	{
		id = env->GetFieldID(cls, "mExistVideo", "I");
		env->SetIntField(clipInfo, id, pClipInfo->existVideo());

		id = env->GetFieldID(cls, "mVideoCodecType", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoCodecType());
		
		id = env->GetFieldID(cls, "mVideoWidth", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getWidth());

		id = env->GetFieldID(cls, "mVideoHeight", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getHeight());

		id = env->GetFieldID(cls, "mDisplayVideoWidth", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getDisplayWidth());

		id = env->GetFieldID(cls, "mDisplayVideoHeight", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getDisplayHeight());
	}

	id = env->GetFieldID(cls, "mExistAudio", "I");
	env->SetIntField(clipInfo, id, pClipInfo->existAudio());

	id = env->GetFieldID(cls, "mAudioCodecType", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getAudioCodecType());

	id = env->GetFieldID(cls, "mAudioDuration", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getClipAudioDuration());

	id = env->GetFieldID(cls, "mVideoDuration", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getClipVideoDuration());

	id = env->GetFieldID(cls, "mSeekPointCount", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getSeekPointCount());

	if( strlen(pClipInfo->getThumbnailPath()) > 0 )
	{
		jstring strPath = env->NewStringUTF(pClipInfo->getThumbnailPath());
		if( strPath == NULL )
		{
			SAFE_RELEASE(pClipInfo);
			return 1;
		}
		
		id = env->GetFieldID(cls, "mThumbnailPath", "Ljava/lang/String;");
		env->SetObjectField(clipInfo, id, strPath);
	}

	SAFE_RELEASE(pClipInfo);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipInfoSync)(JNIEnv * env, jobject obj, jstring clipPath, jobject clipInfo, jint iFlag, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipInfo_Sync(iFlag:%d Tag:%d)", __LINE__, iFlag, iUserTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *strclip = env->GetStringUTFChars(clipPath, NULL);
	if( strclip == NULL )
	{
		return 1;
	}

	int iRet = 0;
	IClipInfo* pClipInfo = g_VideoEditorHandle->getClipInfo_Sync((char*)strclip, iFlag, iUserTag, &iRet);
	env->ReleaseStringUTFChars(clipPath, strclip);
	if( pClipInfo == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getClipInfo_Sync error(Tag : %d iRet : %d)", __LINE__, iUserTag, iRet);
		return iRet;
	}

	jfieldID id;
	jclass cls = env->GetObjectClass(clipInfo);

	if( pClipInfo->existVideo() )
	{
		id = env->GetFieldID(cls, "mExistVideo", "I");
		env->SetIntField(clipInfo, id, pClipInfo->existVideo());

		id = env->GetFieldID(cls, "mVideoCodecType", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoCodecType());
		
		id = env->GetFieldID(cls, "mVideoWidth", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getWidth());

		id = env->GetFieldID(cls, "mVideoHeight", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getHeight());

		id = env->GetFieldID(cls, "mDisplayVideoWidth", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getDisplayWidth());

		id = env->GetFieldID(cls, "mDisplayVideoHeight", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getDisplayHeight());

		id = env->GetFieldID(cls, "mFPS", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoFPS());

		id = env->GetFieldID(cls, "mfFPS", "F");
		env->SetFloatField(clipInfo, id, pClipInfo->getVideoFPSFloat());

		id = env->GetFieldID(cls, "mVideoH264Profile", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoH264Profile());
		
		id = env->GetFieldID(cls, "mVideoH264Level", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoH264Level());

		id = env->GetFieldID(cls, "mVideoH264Interlaced", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoH264Interlaced());

		id = env->GetFieldID(cls, "mVideoOrientation", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoOrientation());
 
		id = env->GetFieldID(cls, "mVideoBitRate", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoBitRate());

		id = env->GetFieldID(cls, "mVideoRenderType", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoRenderType());
		
		id = env->GetFieldID(cls, "mVideoHDRType", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getVideoHDRType());
		
		int size = 0;
		const char *puuid = pClipInfo->getVideoUUID(&size);
    	if( size > 0 )
    	{
    	    jbyteArray arr = env->NewByteArray(size);
		    jbyte* pBuffer = (jbyte *)env->GetByteArrayElements(arr, 0);

		    if( pBuffer != NULL )
		    {
		        memcpy(pBuffer,puuid,size);
			    env->ReleaseByteArrayElements(arr, pBuffer, 0);
		    }

    		id = env->GetFieldID(cls, "mVideoUUID", "[B");
    		env->SetObjectField(clipInfo, id, arr);
    	}

	}

	if( pClipInfo->existAudio() )
	{
		id = env->GetFieldID(cls, "mExistAudio", "I");
		env->SetIntField(clipInfo, id, pClipInfo->existAudio());

		id = env->GetFieldID(cls, "mAudioCodecType", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getAudioCodecType());

		id = env->GetFieldID(cls, "mAudioSampleRate", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getAudioSampleRate());
		
		id = env->GetFieldID(cls, "mAudioChannels", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getAudioChannels());

		id = env->GetFieldID(cls, "mAudioBitRate", "I");
		env->SetIntField(clipInfo, id, pClipInfo->getAudioBitRate());
	}

	id = env->GetFieldID(cls, "mAudioDuration", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getClipAudioDuration());

	id = env->GetFieldID(cls, "mVideoDuration", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getClipVideoDuration());

	unsigned int uiAudioEditBoxtime = 0, uiVideoEditBoxtime = 0;
	pClipInfo->getEditBoxTIme(&uiAudioEditBoxtime, &uiVideoEditBoxtime);
	id = env->GetFieldID(cls, "mAudioEditBoxTime", "I");
	env->SetIntField(clipInfo, id, (int)uiAudioEditBoxtime);

	id = env->GetFieldID(cls, "mVideoEditBoxTime", "I");
	env->SetIntField(clipInfo, id, (int)uiVideoEditBoxtime);


	id = env->GetFieldID(cls, "mSeekPointCount", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getSeekPointCount());

	if( strlen(pClipInfo->getThumbnailPath()) > 0 )
	{
		jstring strPath = env->NewStringUTF(pClipInfo->getThumbnailPath());
		if( strPath == NULL )
		{
			SAFE_RELEASE(pClipInfo);
			return 1;
		}
		
		id = env->GetFieldID(cls, "mThumbnailPath", "Ljava/lang/String;");
		env->SetObjectField(clipInfo, id, strPath);
	}

	if( (iFlag & GET_CLIPINFO_INCLUDE_SEEKTABLE) ==  GET_CLIPINFO_INCLUDE_SEEKTABLE )
	{
		LOGI("[nexEDitor_jni.cpp %d] SeekTable size(%d)", __LINE__, pClipInfo->getSeekTableCount());
		jintArray arr = env->NewIntArray(pClipInfo->getSeekTableCount());
		int* pBuffer = (int*)env->GetIntArrayElements(arr, 0);
		if( pBuffer != NULL )
		{
			for( int i = 0; i < pClipInfo->getSeekTableCount(); i++)
			{
				// LOGI("[nexEDitor_jni.cpp %d] Add SeekTable(%d)", __LINE__, pClipInfo->getSeekTableValue(i));
				pBuffer[i] = pClipInfo->getSeekTableValue(i);
			}
			env->ReleaseIntArrayElements(arr, pBuffer, 0);
		}

		id = env->GetFieldID(cls, "mSeekTable", "[I");
		env->SetObjectField(clipInfo, id, arr);
	}

	SAFE_RELEASE(pClipInfo);
	return 0;
	
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipVideoThumb)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iWidth, jint iHeight, jint iStartTime, jint iEndTime, jint iCount, jint iFlag, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipVideoThumb with options(%d %d %d %d %d %d %d)", __LINE__, iWidth, iHeight, iStartTime, iEndTime, iCount, iFlag, iUserTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *strclip = env->GetStringUTFChars(clipPath, NULL);
	if( strclip == NULL )
	{
		return 1;
	}

	const char *strthumb= env->GetStringUTFChars(pThumbFilePath, NULL);
	if( strthumb == NULL )
	{
		env->ReleaseStringUTFChars(clipPath, strclip);
		return 1;
	}	

	g_VideoEditorHandle->getClipVideoThumb((char*)strclip, (char*)strthumb, iWidth, iHeight, iStartTime, iEndTime, iCount, iFlag, iUserTag);
	env->ReleaseStringUTFChars(clipPath, strclip);
	env->ReleaseStringUTFChars(pThumbFilePath, strthumb);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipVideoThumbWithTimeTable)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iWidth, jint iHeight, int iSize, jintArray arrayTables, jint iFlag, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipVideoThumbWithTimeTable with options(%d %d %d 0x%p %d %d)", __LINE__, iWidth, iHeight, iSize, arrayTables, iFlag, iUserTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *strclip = env->GetStringUTFChars(clipPath, NULL);
	if( strclip == NULL )
	{
		return 1;
	}

	const char *strthumb= env->GetStringUTFChars(pThumbFilePath, NULL);
	if( strthumb == NULL )
	{
		env->ReleaseStringUTFChars(clipPath, strclip);
		return 1;
	}

	jint*pTimeTable = NULL;
	if( arrayTables )
	{
		pTimeTable = env->GetIntArrayElements(arrayTables, NULL);	
	}

	g_VideoEditorHandle->getClipVideoThumbWithTimeTable((char*)strclip, (char*)strthumb, iWidth, iHeight, iSize, pTimeTable, iFlag, iUserTag);
	env->ReleaseStringUTFChars(clipPath, strclip);
	env->ReleaseStringUTFChars(pThumbFilePath, strthumb);
	if( pTimeTable )
	{
		env->ReleaseIntArrayElements(arrayTables, pTimeTable, 0);
	}
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipAudioThumb)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipAudioThumb(%d)", __LINE__, iUserTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *strclip = env->GetStringUTFChars(clipPath, NULL);
	if( strclip == NULL )
	{
		return 1;
	}

	const char *strthumb= env->GetStringUTFChars(pThumbFilePath, NULL);
	if( strthumb == NULL )
	{
		env->ReleaseStringUTFChars(clipPath, strclip);
		return 1;
	}	

	g_VideoEditorHandle->getClipAudioThumb((char*)strclip, (char*)strthumb, iUserTag);
	env->ReleaseStringUTFChars(clipPath, strclip);
	env->ReleaseStringUTFChars(pThumbFilePath, strthumb);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipAudioThumbPartial)(JNIEnv * env, jobject obj, jstring clipPath, jstring pThumbFilePath, jint iStartTime, jint iEndTime, jint iUseCount, jint iSkipCount, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipAudioThumb(%d)", __LINE__, iUserTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *strclip = env->GetStringUTFChars(clipPath, NULL);
	if( strclip == NULL )
	{
		return 1;
	}

	const char *strthumb= env->GetStringUTFChars(pThumbFilePath, NULL);
	if( strthumb == NULL )
	{
		env->ReleaseStringUTFChars(clipPath, strclip);
		return 1;
	}	

	g_VideoEditorHandle->getClipAudioThumbPartial((char*)strclip, (char*)strthumb, iStartTime, iEndTime, iUseCount, iSkipCount, iUserTag);
	env->ReleaseStringUTFChars(clipPath, strclip);
	env->ReleaseStringUTFChars(pThumbFilePath, strthumb);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getClipStopThumb)(JNIEnv * env, jobject obj, jint iUserTag)
{
	LOGI("[nexEDitor_jni.cpp %d] getClipStopThumb(%d)", __LINE__, iUserTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	g_VideoEditorHandle->getClipStopThumb(iUserTag);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearScreen)(JNIEnv * env, jobject obj, jint iTag)
{
	LOGI("[nexEDitor_jni.cpp %d] clearScreen(%d)", __LINE__, iTag);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	g_VideoEditorHandle->clearScreen(iTag);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(fastOptionPreview)(JNIEnv * env, jobject obj, jstring strOption, jint iDisplay)
{
	LOGI("[nexEDitor_jni.cpp %d] fastOptionPreview", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *str = env->GetStringUTFChars(strOption, NULL);
	if( str == NULL )
	{
		return 1;
	}

	int iRet = g_VideoEditorHandle->fastOptionPreview(str, iDisplay);
	env->ReleaseStringUTFChars(strOption, str);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getVersionNumber)(JNIEnv * env, jobject obj, jint iVersion)
{
	LOGI("[nexEDitor_jni.cpp %d] getVersionNumber(%d)", __LINE__, iVersion);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	return g_VideoEditorHandle->getVersionNumber(iVersion);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearTrackCache)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] clearTrackCache", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		return 0;
	}
	return g_VideoEditorHandle->clearTrackCache();
}

JNIEXPORT jstring JNICALL _EDITOR_JNI_(getSystemProperty)(JNIEnv * env, jclass clazz, jstring strPropertyName)
{
	LOGI("[nexEDitor_jni.cpp %d] getSystemProperty", __LINE__);

	const char *strName = env->GetStringUTFChars(strPropertyName, NULL);
	if( strName == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getSystemProperty failed because invalid parameter", __LINE__);
		return NULL;
	}

	char strValue[93];
	strcpy(strValue, "");

	if( getSystemProperty(strName, strValue) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		strcpy(strValue, "");
	}

	env->ReleaseStringUTFChars(strPropertyName, strName);
	
	jstring jstrValue = env->NewStringUTF(strValue);
	return jstrValue;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(startVoiceRecorder)(JNIEnv * env, jobject obj, jstring strFilePath, jint iSampleRate, jint iChannels, jint iBitForSample)
{
	LOGI("[nexEDitor_jni.cpp %d] startVoiceRecorder", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		return -1;
	}

	const char *str = env->GetStringUTFChars(strFilePath, NULL);
	if( str == NULL )
	{
		return 1;
	}

	int iRet = g_VideoEditorHandle->startVoiceRecorder((char*)str, iSampleRate, iChannels, iBitForSample);
	env->ReleaseStringUTFChars(strFilePath, str);

	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(processVoiceRecorder)(JNIEnv * env, jobject obj, jbyteArray arrayPCM, jint iPCMLen)
{
	LOGI("[nexEDitor_jni.cpp %d] processVoiceRecoder In PcmLen(%d)", __LINE__, iPCMLen);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] processVoiceRecoder failed because g_VideoEditorHandle handle is null", __LINE__);
		return -1;
	}

	if( arrayPCM == NULL || iPCMLen <= 0 )
	{
		LOGI("[nexEDitor_jni.cpp %d] processVoiceRecoder failed because arrayPCM is null(0x%p %d)", __LINE__, arrayPCM, iPCMLen);
		return -1;
	}

	const jbyte* bytes = env->GetByteArrayElements(arrayPCM, NULL);
	jsize size = env->GetArrayLength(arrayPCM);

	if( bytes == NULL || size <= 0 )
	{
		LOGI("[nexEDitor_jni.cpp %d] processVoiceRecoder failed because arrayPCM hadle is failed(0x%p %d)", __LINE__, bytes, size);
		return -1;
	}

	int iRet = g_VideoEditorHandle->processVoiceRecorder((int)iPCMLen, (unsigned char*)bytes);
	env->ReleaseByteArrayElements(arrayPCM, const_cast<jbyte*>(bytes), JNI_ABORT);
	LOGI("[nexEDitor_jni.cpp %d] processVoiceRecoder Out(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(endVoiceRecorder)(JNIEnv * env, jobject obj, jobject clipInfo)
{
	LOGI("[nexEDitor_jni.cpp %d] endVoiceRecorder", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] endVoiceRecorder failed because g_VideoEditorHandle handle is null", __LINE__);
		return -1;
	}

	IClipInfo* pClipInfo = g_VideoEditorHandle->endVoiceRecorder();
	if( pClipInfo == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] endVoiceRecorder failed because stopVoiceRecorder is failed", __LINE__);
		return -1;
	}

	jfieldID id;
	jclass cls = env->GetObjectClass(clipInfo);

	id = env->GetFieldID(cls, "mExistAudio", "I");
	env->SetIntField(clipInfo, id, pClipInfo->existAudio());

	id = env->GetFieldID(cls, "mAudioDuration", "I");
	env->SetIntField(clipInfo, id, pClipInfo->getClipAudioDuration());

	// setClipInfo
	pClipInfo->Release();
	return 0;
}

JNIEXPORT jstring JNICALL _EDITOR_JNI_(getProperty)(JNIEnv * env, jobject obj, jstring strPropertyName)
{
	LOGI("[nexEDitor_jni.cpp %d] getProperty", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getProperty failed because g_VideoEditorHandle handle is null", __LINE__);
		return NULL;
	}

	const char *strName = env->GetStringUTFChars(strPropertyName, NULL);
	if( strName == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getSystemProperty failed because invalid parameter", __LINE__);
		return NULL;
	}

	char strValue[93];
	strcpy(strValue, "");

	g_VideoEditorHandle->getProperty(strName, strValue);
	
	env->ReleaseStringUTFChars(strPropertyName, strName);
	
	jstring jstrValue = env->NewStringUTF(strValue);
	return jstrValue;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setProperty)(JNIEnv * env, jobject obj, jstring strPropertyName, jstring strPropertyValue)
{
	// LOGI("[nexEDitor_jni.cpp %d] setProperty", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] setProperty failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	const char *strName = env->GetStringUTFChars(strPropertyName, NULL);
	if( strName == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getSystemProperty failed because invalid parameter", __LINE__);
		return 1;
	}

	const char *strValue = env->GetStringUTFChars(strPropertyValue, NULL);
	if( strName == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] getSystemProperty failed because invalid parameter", __LINE__);
		env->ReleaseStringUTFChars(strPropertyName, strName);
		return 1;
	}

	LOGI("[nexEDitor_jni.cpp %d] setProperty(%s %s)", __LINE__, strName, strValue);

	g_VideoEditorHandle->setProperty(strName, strValue);

	env->ReleaseStringUTFChars(strPropertyName, strName);
	env->ReleaseStringUTFChars(strPropertyValue, strValue);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(transcodingStart)(JNIEnv* env, jobject obj, jstring strSrcClipPath, jstring strDstClipPath,  jint iWidth, jint iHeight, jint iDisplayWidth, jint iDisplayHeight, jint iBitrate, jlong lMaxFileSize, jint iFPS, jint iFlag, jstring strUserData)
{
	LOGI("[nexEDitor_jni.cpp %d] TranscodingStart", __LINE__);
	int ret = 0;
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] TranscodingStart failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	const char* strSrc = env->GetStringUTFChars(strSrcClipPath, NULL);
	if( strSrc == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] TranscodingStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	const char* strDst = env->GetStringUTFChars(strDstClipPath, NULL);
	if( strDst == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] TranscodingStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	const char* strUser = env->GetStringUTFChars(strUserData, NULL);
	if( strUser == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] TranscodingStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	ret = g_VideoEditorHandle->transcodingStart(strSrc, strDst, iWidth, iHeight, iDisplayWidth, iDisplayHeight, iBitrate, lMaxFileSize, iFPS, iFlag, 100, strUser);

	env->ReleaseStringUTFChars(strSrcClipPath, strSrc);
	env->ReleaseStringUTFChars(strDstClipPath, strDst);
	env->ReleaseStringUTFChars(strUserData, strUser);
	LOGI("[nexEDitor_jni.cpp %d] TranscodingStart End", __LINE__);

	return ret;

}

JNIEXPORT jint JNICALL _EDITOR_JNI_(transcodingStop)(JNIEnv* env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] TranscodingStop", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] TranscodingStop failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	g_VideoEditorHandle->transcodingStop();
	LOGI("[nexEDitor_jni.cpp %d] TranscodingStop End", __LINE__);

	return 0;
}

#ifdef FOR_LAYER_FEATURE


JNIEXPORT jint JNICALL _EDITOR_JNI_(addLayerItem)(JNIEnv* env, jobject obj, jobject layer)
{
	LOGI("[nexEDitor_jni.cpp %d] addLayerItem", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] addLayerItem failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	ILayerItem* pLayer = g_VideoEditorHandle->createLayerItem();
	if( pLayer == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] addLayerItem failed because create layer item failed", __LINE__);
		return 1;
	}

	setLayerInfo(env, layer, pLayer);

	g_VideoEditorHandle->addLayerItem(pLayer);
	SAFE_RELEASE(pLayer);
	LOGI("[nexEDitor_jni.cpp %d] addLayerItem End", __LINE__);
	return 0;
	
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(deleteLayerItem)(JNIEnv* env, jobject obj, jint iLayerID)
{
	LOGI("[nexEDitor_jni.cpp %d] deleteLayerItem", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] deleteLayerItem failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	g_VideoEditorHandle->deleteLayerItem(iLayerID);
	LOGI("[nexEDitor_jni.cpp %d] deleteLayerItem End", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearLayerItem)(JNIEnv* env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] clearLayerItem", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] clearLayerItem failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	g_VideoEditorHandle->clearLayerItem();
	LOGI("[nexEDitor_jni.cpp %d] clearLayerItem End", __LINE__);
	return 0;
}
#endif

#ifdef FOR_LOADLIST_FEATURE

JNIEXPORT jint JNICALL _EDITOR_JNI_(loadList)(JNIEnv * env, jobject obj, jobjectArray arrayVisualClip, jobjectArray arrayAudioClip, jint iOption)
{
	LOGI("[nexEDitor_jni.cpp %d] loadList opt(%d)", __LINE__, iOption);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] loadList failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}


	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] GetClipList failed", __LINE__);
		return 1;
	}

	pClipList->lockClipList();
	pClipList->clearClipList();

	g_VideoEditorHandle->clearProject(iOption);

	if( arrayVisualClip == NULL  )
	{
		pClipList->unlockClipList();
		SAFE_RELEASE(pClipList);
		LOGI("[nexEDitor_jni.cpp %d] loadClipList(Cliplist clear because Visual Clip array is null)", __LINE__);
		return 0;		
	}

	int iArrayCount = env->GetArrayLength(arrayVisualClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayVisualClip, i);
		if( obj == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);

		IClipItem* pItem = pClipList->createEmptyClipUsingID(iClipID);
		if( pItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			continue;			
		}
		
		setVisualClip(env, objClip, pItem);
		pClipList->addClipItem(pItem);
		// pItem->printClipInfo();
		pItem->Release();
		env->DeleteLocalRef(objClip);
		env->DeleteLocalRef(clipInfo_ref_class);
	}

	if( arrayAudioClip == NULL )
	{
		pClipList->unlockClipList();
		SAFE_RELEASE(pClipList);
		g_VideoEditorHandle->updateLoadList();
		LOGI("[nexEDitor_jni.cpp %d] loadClipList End", __LINE__);
		return 0;
	}

	iArrayCount = env->GetArrayLength(arrayAudioClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayAudioClip, i);
		if( obj == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);

		IClipItem* pItem = (IClipItem*)pClipList->createEmptyClipUsingID(iClipID);
		if( pItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			continue;
		}

		setAudioClip(env, objClip, pItem);

		pClipList->addClipItem(pItem);
		// pItem->printClipInfo();
		pItem->Release();
		
		env->DeleteLocalRef(objClip);
		env->DeleteLocalRef(clipInfo_ref_class);
	}

	pClipList->unlockClipList();
	SAFE_RELEASE(pClipList);

	g_VideoEditorHandle->updateLoadList();

	LOGI("[nexEDitor_jni.cpp %d] loadList End", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(asyncLoadList)(JNIEnv * env, jobject obj, jobjectArray arrayVisualClip, jobjectArray arrayAudioClip, jint iOption)
{
	LOGI("[nexEDitor_jni.cpp %d] ayncLoadList opt(%d)", __LINE__, iOption);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] loadList failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	IClipList* pClipList = g_VideoEditorHandle->createClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] GetClipList failed", __LINE__);
		return 1;
	}

	pClipList->lockClipList();
	pClipList->clearClipList();

	if( arrayVisualClip == NULL  )
	{
		pClipList->unlockClipList();
		int iRet = g_VideoEditorHandle->updateClipList(pClipList, iOption);
		SAFE_RELEASE(pClipList);
		LOGI("[nexEDitor_jni.cpp %d] loadClipList(Cliplist clear because Visual Clip array is null)", __LINE__);
		return iRet;		
	}

	int iArrayCount = env->GetArrayLength(arrayVisualClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayVisualClip, i);
		if( objClip == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);

		IClipItem* pItem = pClipList->createEmptyClipUsingID(iClipID);
		if( pItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			continue;			
		}
		
		setVisualClip(env, objClip, pItem);
		pClipList->addClipItem(pItem);
		// pItem->printClipInfo();
		pItem->Release();
		env->DeleteLocalRef(objClip);
		env->DeleteLocalRef(clipInfo_ref_class);
	}

	if( arrayAudioClip == NULL )
	{
		pClipList->unlockClipList();
		int iRet = g_VideoEditorHandle->updateClipList(pClipList, iOption);
		SAFE_RELEASE(pClipList);
		LOGI("[nexEDitor_jni.cpp %d] loadClipList End(%d)", __LINE__, iRet);
		return iRet;
	}

	iArrayCount = env->GetArrayLength(arrayAudioClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayAudioClip, i);
		if( objClip == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);

		IClipItem* pItem = (IClipItem*)pClipList->createEmptyClipUsingID(iClipID);
		if( pItem == NULL )
		{
			env->DeleteLocalRef(objClip);
			env->DeleteLocalRef(clipInfo_ref_class);
			continue;
		}

		setAudioClip(env, objClip, pItem);

		pClipList->addClipItem(pItem);
		// pItem->printClipInfo();
		pItem->Release();
		
		env->DeleteLocalRef(objClip);
		env->DeleteLocalRef(clipInfo_ref_class);
	}

	pClipList->unlockClipList();

	int iRet = g_VideoEditorHandle->updateClipList(pClipList, iOption);
	SAFE_RELEASE(pClipList);

	LOGI("[nexEDitor_jni.cpp %d] ayncLoadList End(%d)", __LINE__, iRet);
	return iRet;
}
JNIEXPORT jint JNICALL _EDITOR_JNI_(asyncEffectList)(JNIEnv * env, jobject obj, jobjectArray arrayEffectClip, jint iOption)
{
	LOGI("[nexEDitor_jni.cpp %d] asyncEffectList opt(%d)", __LINE__, iOption);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] loadList failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	IClipList* pClipList = g_VideoEditorHandle->createClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] GetClipList failed", __LINE__);
		return 1;
	}

	pClipList->lockClipList();
	pClipList->clearClipList();

	int iArrayCount = env->GetArrayLength(arrayEffectClip);
	for(int i = 0; i < iArrayCount; i++)
	{
		jobject objClip = env->GetObjectArrayElement(arrayEffectClip, i);
		if( obj == NULL )
		{
			continue;
		}
		
		jfieldID		id;	
		jclass		clipInfo_ref_class;
		const char *str1 = NULL;
		const char *str2 = NULL;
		const char *str3 = NULL;		
		clipInfo_ref_class = env->GetObjectClass(objClip);
		if( clipInfo_ref_class == NULL )
		{
			env->DeleteLocalRef(objClip);
			continue;
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipID", "I");
		jint iClipID = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList iClipID(%d)", __LINE__, iClipID);

		id = env->GetFieldID(clipInfo_ref_class, "mStartTime", "I");
		jint mStartTime = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mStartTime(%d)", __LINE__, mStartTime);

		id = env->GetFieldID(clipInfo_ref_class, "mEndTime", "I");
		jint mEndTime = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mEndTime(%d)", __LINE__, mEndTime);

		id = env->GetFieldID(clipInfo_ref_class, "mTitleStyle", "I");
		jint mTitleStyle = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mTitleStyle(%d)", __LINE__, mTitleStyle);

		id = env->GetFieldID(clipInfo_ref_class, "mTitleStartTime", "I");
		jint mTitleStartTime = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mTitleStartTime(%d)", __LINE__, mTitleStartTime);

		id = env->GetFieldID(clipInfo_ref_class, "mTitleEndTime", "I");
		jint mTitleEndTime = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mTitleEndTime(%d)", __LINE__, mTitleEndTime);

		id = env->GetFieldID(clipInfo_ref_class, "mTitle", "Ljava/lang/String;");
		jstring mTitle = (jstring)env->GetObjectField(objClip, id);
		if( mTitle )
		{
			str1 = env->GetStringUTFChars(mTitle, NULL);
			if( str1 == NULL )
			{
				env->DeleteLocalRef(mTitle);
				env->DeleteLocalRef(clipInfo_ref_class);
				return 1;
			}
			LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mTitle(%s)", __LINE__, str1);
		}
		
		id = env->GetFieldID(clipInfo_ref_class, "mEffectDuration", "I");
		jint mEffectDuration = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mEffectDuration(%d)", __LINE__, mEffectDuration);
		
		id = env->GetFieldID(clipInfo_ref_class, "mEffectOffset", "I");
		jint mEffectOffset = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mEffectOffset(%d)", __LINE__, mEffectOffset);
		
		id = env->GetFieldID(clipInfo_ref_class, "mEffectOverlap", "I");
		jint mEffectOverlap = env->GetIntField(objClip, id);
		LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mEffectOverlap(%d)", __LINE__, mEffectOverlap);
		
		id = env->GetFieldID(clipInfo_ref_class, "mTitleEffectID", "Ljava/lang/String;");
		jstring mTitleEffectID = (jstring)env->GetObjectField(objClip, id);
		if( mTitleEffectID )
		{
			str2 = env->GetStringUTFChars(mTitleEffectID, NULL);
			if( str2 == NULL )
			{
				env->DeleteLocalRef(mTitleEffectID);
				env->DeleteLocalRef(clipInfo_ref_class);
				return 1;
			}
			LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mTitleEffectID(%s)", __LINE__, str2);
		}

		id = env->GetFieldID(clipInfo_ref_class, "mClipEffectID", "Ljava/lang/String;");
		jstring mClipEffectID = (jstring)env->GetObjectField(objClip, id);
		if( mClipEffectID )
		{
			str3 = env->GetStringUTFChars(mClipEffectID, NULL);
			if( str3 == NULL )
			{
				env->DeleteLocalRef(mClipEffectID);
				env->DeleteLocalRef(clipInfo_ref_class);
				return 1;
			}
			LOGI("[nexEDitor_jni.cpp %d] asyncEffectList mClipEffectID(%s)", __LINE__, str3);
		}

		IEffectItem* pEffectItem = g_VideoEditorHandle->createEffectItem();
		if( pEffectItem == NULL )
		{
			LOGI("[nexEDitor_jni.cpp %d] createEffectItem failed", __LINE__);
			return 1;
		}

		unsigned int uiEffectStartTime = mEndTime - (mEffectDuration * mEffectOffset / 100);
		pEffectItem->setEffectInfo((unsigned int)mStartTime, (unsigned int)mEndTime, (unsigned int)uiEffectStartTime, (unsigned int)mEffectDuration, (int)mEffectOffset, (int)mEffectOverlap, str3, str1, (int)i, (int)iArrayCount);
		pEffectItem->setTitleEffectInfo((unsigned int)mStartTime, (unsigned int)mEndTime, (unsigned int)mTitleStartTime, (unsigned int)mTitleEndTime, str2, str1, (int)i, (int)iArrayCount);

		if(mTitle)
		{
			env->ReleaseStringUTFChars(mTitle, str1);
			env->DeleteLocalRef(mTitle);
		}

		if(mTitleEffectID)
		{
			env->ReleaseStringUTFChars(mTitleEffectID, str2);
			env->DeleteLocalRef(mTitleEffectID);
		}

		if(mClipEffectID)
		{
			env->ReleaseStringUTFChars(mClipEffectID, str3);
			env->DeleteLocalRef(mClipEffectID);
		}

		pClipList->addEffectItem(pEffectItem);
	}
	
	pClipList->unlockClipList();

	int iRet = g_VideoEditorHandle->updateEffectList(pClipList, iOption);
	SAFE_RELEASE(pClipList);

	LOGI("[nexEDitor_jni.cpp %d] asyncEffectList End(%d)", __LINE__, 0);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(asyncDrawInfoList)(JNIEnv * env, jobject obj, jobjectArray master, jobjectArray sub)
{
	LOGI("[nexEDitor_jni.cpp %d] asyncDrawInfoList", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] loadList failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	IDrawInfoList* masterList = NULL;
	IDrawInfoList* subList = NULL;

	if( master != NULL )
	{
		masterList = g_VideoEditorHandle->createDrawInfoList();

		int iArrayCount = env->GetArrayLength(master);
		for(int i = 0; i < iArrayCount; i++)
		{
			jobject drawInfo = env->GetObjectArrayElement(master, i);
			if( drawInfo == NULL )
			{
				continue;
			}

			IDrawInfo* pItem = masterList->createDrawInfo();
			if( pItem == NULL )
			{
				env->DeleteLocalRef(drawInfo);
				continue;
			}

			setDrawInfo(env, drawInfo, pItem);
			masterList->addDrawInfo(pItem);
			SAFE_RELEASE(pItem);
			env->DeleteLocalRef(drawInfo);
		}
	}

	if( master != NULL && sub != NULL )
	{
		subList = g_VideoEditorHandle->createDrawInfoList();

		int iArrayCount = env->GetArrayLength(sub);
		for(int i = 0; i < iArrayCount; i++)
		{
			jobject drawInfo = env->GetObjectArrayElement(sub, i);
			if( drawInfo == NULL )
			{
				continue;
			}

			IDrawInfo* pItem = subList->createDrawInfo();
			if( pItem == NULL )
			{
				env->DeleteLocalRef(drawInfo);
				continue;
			}

			setDrawInfo(env, drawInfo, pItem);
			subList->addDrawInfo(pItem);
			SAFE_RELEASE(pItem);
			env->DeleteLocalRef(drawInfo);
		}
	}

	int iRet = g_VideoEditorHandle->setDrawInfoList(masterList, subList);
	SAFE_RELEASE(masterList);
	SAFE_RELEASE(subList);
	LOGI("[nexEDitor_jni.cpp %d] asyncDrawInfoList End", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(updateDrawInfo)(JNIEnv * env, jobject obj, jobject drawinfo)
{
	LOGI("[nexEDitor_jni.cpp %d] updateDrawInfo", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] loadList failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	IDrawInfo* pDrawInfo = g_VideoEditorHandle->createDrawInfo();

	if( drawinfo == NULL || pDrawInfo == NULL )
	{
		SAFE_RELEASE(pDrawInfo);
		return 1;
	}

	setDrawInfo(env, drawinfo, pDrawInfo);

	int iRet = g_VideoEditorHandle->setDrawInfo(pDrawInfo);
	SAFE_RELEASE(pDrawInfo);
	LOGI("[nexEDitor_jni.cpp %d] updateDrawInfo End", __LINE__);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearList)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] clearList", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] clearList failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}


	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] GetClipList failed", __LINE__);
		return 1;
	}

	pClipList->lockClipList();
	pClipList->clearClipList();
	pClipList->unlockClipList();
	
	g_VideoEditorHandle->updateLoadList();
	g_VideoEditorHandle->clearProject();
	
	SAFE_RELEASE(pClipList);
	LOGI("[nexEDitor_jni.cpp %d] loadList End", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearProject)(JNIEnv * env, jobject obj)
{
	int rval = 0;
	LOGI("[nexEDitor_jni.cpp %d] clearProject", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] clearProject failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	

	rval = g_VideoEditorHandle->clearProject();
	LOGI("[nexEDitor_jni.cpp %d] clearProject End(%d)", __LINE__,rval);
	return rval;
}

#endif

JNIEXPORT jint JNICALL _EDITOR_JNI_(highlightStart)(JNIEnv* env, jobject obj, jstring strSrcClipPath, jint iIndexMode, jint iRequestInterval, jint iRequestCount, jint iOutputMode, jstring strDstClipPath, jint iWidth, jint iHeight, jint iBitrate, jlong lMaxFileSize, jint iDecodeMode)
{
	LOGI("[nexEDitor_jni.cpp %d] HighlightStart", __LINE__);

	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] HighlightStart failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	if(iRequestInterval < 1000)
	{
		LOGI("[nexEDitor_jni.cpp %d] HighlightStart failed, Please check RequestInterval %d", __LINE__, iRequestInterval);
		return 1;
	}

	if(iRequestCount < 1)
	{
		LOGI("[nexEDitor_jni.cpp %d] HighlightStart failed, Please check RequestCount %d", __LINE__, iRequestCount);
		return 1;
	}

	const char* strSrc = env->GetStringUTFChars(strSrcClipPath, NULL);
	if( strSrc == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] HighlightStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	const char* strDst = NULL;

	if(iOutputMode == 1)	// mode = 0 ; indexmode, mode = 1 make highlight mode
	{
		env->GetStringUTFChars(strDstClipPath, NULL);
		if( strDst == NULL)
		{
			LOGI("[nexEDitor_jni.cpp %d] HighlightStart() failed because invalid parameter", __LINE__);
			return 1;
		}
	}
	g_VideoEditorHandle->highlightStart(strSrc, iIndexMode, iRequestInterval, iRequestCount, iOutputMode, strDst, iWidth, iHeight, iBitrate, lMaxFileSize, iDecodeMode);

	env->ReleaseStringUTFChars(strSrcClipPath, strSrc);
	if(iOutputMode == 1)
	{
		env->ReleaseStringUTFChars(strDstClipPath, strDst);
	}
	LOGI("[nexEDitor_jni.cpp %d] HighlightStart End", __LINE__);

	return 0;

}

JNIEXPORT jint JNICALL _EDITOR_JNI_(highlightStop)(JNIEnv* env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] HighlightStartStop", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] HighlightStartStop failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	g_VideoEditorHandle->highlightStop();
	LOGI("[nexEDitor_jni.cpp %d] HighlightStartStop End", __LINE__);

	return 0;
}

/* iRet 
	0 : support direct encode
	1 : Invalid editor handle
	2 : Invalid clip list handle
	3 : Clip count is 0 or clip error
	4 : Video information was different.
	5 : Video codec unmatch
	6 : Video not start idr frame
	7 : Has image clip.
	8 : Has video layer.
	9 : Enable transition effect
	10 : Enable title effect
	11 : Encoder dsi unmatch with dsi of clip
	12 : applied speed control
	13 : Unsupport codec.
	14 : Rotate invalid.
	15 : Multi track content.
	16 : Has audio tracks.
*/	
JNIEXPORT jint JNICALL _EDITOR_JNI_(checkDirectExport)(JNIEnv* env, jobject obj, jint option)
{
	LOGI("[nexEDitor_jni.cpp %d] checkDirectExport", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] checkDirectExport failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

#if 1	
	int iRet = g_VideoEditorHandle->checkDirectExport(option);
#else
	IClipList* pClipList = g_VideoEditorHandle->getClipList();
	if( pClipList == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] checkDirectExport GetClipList failed", __LINE__);
		return 1;
	}

	pClipList->lockClipList();

	int iRet = pClipList->checkDirectExport();

	pClipList->unlockClipList();
	SAFE_RELEASE(pClipList);
#endif	
	LOGI("[nexEDitor_jni.cpp %d] checkDirectExport End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(directExport)(JNIEnv * env, jobject obj, jstring strFilePath, jlong iMaxFileSize, jlong iMaxFileDuration, jstring strUserData, jint iFlag)
{
	LOGI("[nexEDitor_jni.cpp %d] directExport(0x%p, %lld %lld, 0x%p %d)", __LINE__, strFilePath, iMaxFileSize, iMaxFileDuration, strUserData, iFlag);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] directExport failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	const char* strPath = env->GetStringUTFChars(strFilePath, NULL);
	if( strPath == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] directExport failed because invalid parameter", __LINE__);
		return 1;
	}

	const char* strData = NULL;
	if( strUserData != NULL )
	{
		strData = env->GetStringUTFChars(strUserData, NULL);
		if( strData == NULL)
		{
			env->ReleaseStringUTFChars(strFilePath, strPath);
			LOGI("[nexEDitor_jni.cpp %d] directExport failed because invalid parameter", __LINE__);
			return 1;
		}
	}

	int iRet = g_VideoEditorHandle->directExport(strPath, iMaxFileSize, iMaxFileDuration, strData, iFlag);

	env->ReleaseStringUTFChars(strFilePath, strPath);
	if( strData != NULL )
		env->ReleaseStringUTFChars(strUserData, strData);
	LOGI("[nexEDitor_jni.cpp %d] directExport End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(checkPFrameDirectExportSync)(JNIEnv* env, jobject obj, jstring strFilePath)
{
	LOGI("[nexEDitor_jni.cpp %d] checkPFrameDirectExportSync(0x%p)", __LINE__, strFilePath);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] checkPFrameDirectExportSync failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
 }

	const char* strPath = env->GetStringUTFChars(strFilePath, NULL);
	if( strPath == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] checkPFrameDirectExportSync failed because invalid parameter", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->checkPFrameDirectExportSync(strPath);

	env->ReleaseStringUTFChars(strFilePath, strPath);
	LOGI("[nexEDitor_jni.cpp %d] checkPFrameDirectExportSync End(%d)", __LINE__, iRet);
	return iRet;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(fastPreviewStart)(JNIEnv * env, jobject obj, jint iStartTime, jint iEndTime, jint displayWidth, jint displayHeight)
{
	LOGI("[nexEDitor_jni.cpp %d] fastPreview(%d %d)", __LINE__, (unsigned int)iStartTime, (unsigned int)iEndTime);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] fastPreview failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->fastPreviewStart((unsigned int)iStartTime, (unsigned int)iEndTime, displayWidth, displayHeight);

	LOGI("[nexEDitor_jni.cpp %d] fastPreview End(%d)", __LINE__, iRet);
	
	return iRet;
 }

JNIEXPORT jint JNICALL _EDITOR_JNI_(fastPreviewStop)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] fastPreviewEnd", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] fastPreviewEnd failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->fastPreviewStop();

	LOGI("[nexEDitor_jni.cpp %d] fastPreviewEnd(%d)", __LINE__, iRet);
	
	return iRet;
 }

JNIEXPORT jint JNICALL _EDITOR_JNI_(fastPreviewTime)(JNIEnv * env, jobject obj, jint iSetTime)
{
	LOGI("[nexEDitor_jni.cpp %d] fastPreviewSetTime START(%d)", __LINE__, (unsigned int)iSetTime);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] fastPreview failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->fastPreviewTime((unsigned int)iSetTime);

	LOGI("[nexEDitor_jni.cpp %d] fastPreviewSetTime End(%d)", __LINE__, iRet);
	
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(checkIDRStart)(JNIEnv * env, jobject obj, jstring strFilePath)
{
	LOGI("[nexEDitor_jni.cpp %d] checkIDRStart(0x%p)", __LINE__, strFilePath);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] checkIDRStart failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	const char* strPath = env->GetStringUTFChars(strFilePath, NULL);
	if( strPath == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] directExport failed because invalid parameter", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->checkIDRStart(strPath);

	env->ReleaseStringUTFChars(strFilePath, strPath);
	LOGI("[nexEDitor_jni.cpp %d] checkIDRStart End(%d)", __LINE__, iRet);
	return iRet;	
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(checkIDREnd)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] checkIDREnd In", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] checkIDREnd failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	int iRet = g_VideoEditorHandle->checkIDREnd();

	LOGI("[nexEDitor_jni.cpp %d] checkIDREnd End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(checkIDRTime)(JNIEnv * env, jobject obj, jint iTime)
{
	LOGI("[nexEDitor_jni.cpp %d] checkIDRTime In", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] checkIDRTime failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	int iRet = g_VideoEditorHandle->checkIDRTime(iTime);

	LOGI("[nexEDitor_jni.cpp %d] checkIDRTime End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(reverseStart)(JNIEnv* env, jobject obj, jstring strSrcClipPath, jstring strDstClipPath, jstring strTempClipPath, jint iWidth, jint iHeight, jint iBitrate, jlong lMaxFileSize, jint iStartTime, jint iEndTime, jint iDecodeMode)
{
	LOGI("[nexEDitor_jni.cpp %d] ReverseStart", __LINE__);

	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] ReverseStart failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	const char* strSrc = env->GetStringUTFChars(strSrcClipPath, NULL);
	if( strSrc == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] ReverseStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	const char* strDst = env->GetStringUTFChars(strDstClipPath, NULL);
	if( strDst == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] ReverseStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	const char* strTemp = env->GetStringUTFChars(strTempClipPath, NULL);
	if( strTemp == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] ReverseStart() failed because invalid parameter", __LINE__);
		return 1;
	}
	
	g_VideoEditorHandle->reverseStart(strSrc, strDst, strTemp, iWidth, iHeight, iBitrate, lMaxFileSize, iStartTime, iEndTime, iDecodeMode);

	env->ReleaseStringUTFChars(strSrcClipPath, strSrc);
	env->ReleaseStringUTFChars(strDstClipPath, strDst);
	env->ReleaseStringUTFChars(strTempClipPath, strTemp);	
	LOGI("[nexEDitor_jni.cpp %d] ReverseStart End", __LINE__);

	return 0;

}


JNIEXPORT jint JNICALL _EDITOR_JNI_(reverseStop)(JNIEnv* env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] ReverseStop", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] ReverseStop failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	g_VideoEditorHandle->reverseStop();
	LOGI("[nexEDitor_jni.cpp %d] ReverseStop End", __LINE__);

	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setVolumeWhilePlay)(JNIEnv * env, jobject obj, jint iMasterVolume, jint iSlaveVolume)
{
	LOGI("[nexEDitor_jni.cpp %d] setVolumeWhilePlay(%d %d)", __LINE__, iMasterVolume, iSlaveVolume);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] setVolumeWhilePlay failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	int iRet = g_VideoEditorHandle->setVolumeWhilePlay(iMasterVolume, iSlaveVolume);
	LOGI("[nexEDitor_jni.cpp %d] setVolumeWhilePlay End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jintArray JNICALL _EDITOR_JNI_(getVolumeWhilePlay)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getVolumeWhilePlay", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] getVolumeWhilePlay failed because g_VideoEditorHandle handle is null", __LINE__);
		return NULL;
	}

	jintArray arr = env->NewIntArray(2);;
	int sVolume[2] = {0,0};
	g_VideoEditorHandle->getVolumeWhilePlay(&sVolume[0], &sVolume[1]);
	env->SetIntArrayRegion(arr, 0, 2, sVolume);
	
	LOGI("[nexEDitor_jni.cpp %d] getVolumeWhilePlay End", __LINE__);
	return arr;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(addUDTA)(JNIEnv * env, jobject obj, jint iType, jstring strUDTA)
{
	LOGI("[nexEDitor_jni.cpp %d] addUDTA In", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] addUDTA failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}

	const char* strData = env->GetStringUTFChars(strUDTA, NULL);
	if( strData == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] ReverseStart() failed because invalid parameter", __LINE__);
		return 1;
	}

	int iRet = g_VideoEditorHandle->addUDTA(iType, strData);
	env->ReleaseStringUTFChars(strUDTA, strData);
	LOGI("[nexEDitor_jni.cpp %d] addUDTA End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(clearUDTA)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] clearUDTA In", __LINE__);
	if( g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] clearUDTA failed because g_VideoEditorHandle handle is null", __LINE__);
		return 1;
	}
	
	int iRet = g_VideoEditorHandle->clearUDTA();
	LOGI("[nexEDitor_jni.cpp %d] clearUDTA End(%d)", __LINE__, iRet);
	return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getNativeSDKInfoWM)(JNIEnv * env, jobject obj)
{
    NEXSDKInformation stSDKInfo;
    
    if(getSDKInfo(&stSDKInfo))
        return 0;

    if(stSDKInfo.bCheckWaterMark)
    {
        LOGI("[nexEDitor_jni.cpp %d] enable WaterMark", __LINE__);
        return 1;
    }
    else    
    {
        return 0;
    }
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForClipID)(JNIEnv * env, jobject obj, jint export_flag, jint clipid)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	jint ret = g_VideoEditorHandle->getTexNameForClipID(export_flag, clipid);

    return ret;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setBaseFilterRenderItem)(JNIEnv * env, jobject obj, jstring path)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	const char *str = env->GetStringUTFChars(path, NULL);
	if(str == NULL)
		return -1;
	int ret = g_VideoEditorHandle->setBaseFilterRenderItem(str);
	env->ReleaseStringUTFChars(path, str);
	return ret;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setTexNameForLUT)(JNIEnv * env, jobject obj, jint export_flag, jint clipid, jfloat x, jfloat y)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->setTexNameForLUT(export_flag, clipid, x, y);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForMask)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->getTexNameForMask(export_flag);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForBlend)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->getTexNameForBlend(export_flag);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(swapBlendMain)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->swapBlendMain(export_flag);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getTexNameForWhite)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->getTexNameForWhite(export_flag);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(cleanupMaskWithWhite)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->cleanupMaskWithWhite(export_flag);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(createRenderItem)(JNIEnv * env, jobject obj, jstring effect_id, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}

	const char *str = env->GetStringUTFChars(effect_id, NULL);

	if( str == NULL )
	{
		return -1;
	}	

	int ret = g_VideoEditorHandle->createRenderItem(export_flag, str);

	env->ReleaseStringUTFChars(effect_id, str);
	return ret;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(releaseRenderItem)(JNIEnv * env, jobject obj, jint effect_id, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return 1;
	}

	return g_VideoEditorHandle->releaseRenderItem(export_flag, effect_id);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(drawRenderItemOverlay)(JNIEnv * env, jobject obj, jint effect_id, jint tex_id_for_second, jstring usersettings, jint export_flag, jint current_time, jint start_time, jint end_time, jfloatArray matrix, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jboolean mask_enabled)
{
	if (!g_VideoEditorHandle){
		return 1;
	}
	
	const char *str = env->GetStringUTFChars(usersettings, NULL);

	if( str == NULL )
	{
		return 1;
	}

	if (matrix != NULL){

		float* pmatrix = (float*)env->GetFloatArrayElements(matrix, 0);
		if( pmatrix != NULL )
		{
			g_VideoEditorHandle->drawRenderItemOverlay(effect_id, tex_id_for_second, export_flag, (char*)str, current_time, start_time, end_time, pmatrix, left, top, right, bottom, alpha, mask_enabled?1:0);
			env->ReleaseFloatArrayElements(matrix, pmatrix, 0);
		}
	}

	env->ReleaseStringUTFChars(usersettings, str);

	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(createNexEDL)(JNIEnv * env, jobject obj, jstring effect_id, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}

	const char *str = env->GetStringUTFChars(effect_id, NULL);

	if( str == NULL )
	{
		return -1;
	}	

	int ret = g_VideoEditorHandle->createNexEDL(export_flag, str);

	env->ReleaseStringUTFChars(effect_id, str);
	return ret;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(releaseNexEDL)(JNIEnv * env, jobject obj, jint effect_id, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return 1;
	}

	return g_VideoEditorHandle->releaseNexEDL(export_flag, effect_id);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(drawNexEDLOverlay)(JNIEnv * env, jobject obj, jint effect_id, jstring usersettings, jint export_flag, jint current_time, jint start_time, jint end_time, jfloatArray matrix, jfloat left, jfloat top, jfloat right, jfloat bottom, jfloat alpha, jint mode)
{
	if (!g_VideoEditorHandle){
		return 1;
	}
	
	const char *str = env->GetStringUTFChars(usersettings, NULL);

	if( str == NULL )
	{
		return 1;
	}

	if (matrix != NULL){

		float* pmatrix = (float*)env->GetFloatArrayElements(matrix, 0);
		if( pmatrix != NULL )
		{
			g_VideoEditorHandle->drawNexEDLOverlay(effect_id, export_flag, (char*)str, current_time, start_time, end_time, pmatrix, left, top, right, bottom, alpha, mode);
			env->ReleaseFloatArrayElements(matrix, pmatrix, 0);
		}
	}

	env->ReleaseStringUTFChars(usersettings, str);

	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setRenderToMask)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->setRenderToMask(export_flag);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setRenderToDefault)(JNIEnv * env, jobject obj, jint export_flag)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	return g_VideoEditorHandle->setRenderToDefault(export_flag);
}

//yoon

static int nexjniGetFDFromFileDescriptor(JNIEnv * env, jobject fileDescriptor) 
{ 
    jint fd = -1; jclass fdClass = env->FindClass("java/io/FileDescriptor"); 
    if (fdClass != NULL) 
    { 
        jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClass, "descriptor", "I"); 
        if (fdClassDescriptorFieldID != NULL && fileDescriptor != NULL) 
        { 
            fd = env->GetIntField(fileDescriptor, fdClassDescriptorFieldID); 
        } 
    } 
    return fd; 
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setOutputFile)(JNIEnv *env, jobject thiz, jobject fileDescriptor, jlong offset, jlong length)
{
    LOGI("[nexEDitor_jni.cpp %d] setOutputFile In", __LINE__);
	if (!g_VideoEditorHandle){
		return -1;
	}
   
    if (fileDescriptor == NULL) {
        return -2;
    }
    int fd = nexjniGetFDFromFileDescriptor(env, fileDescriptor);
    LOGI("[nexEDitor_jni.cpp %d] setOutputFile fd=%d", __LINE__,fd);
    int iRet = regOutPutFD(fd);
    LOGI("[nexEDitor_jni.cpp %d] setOutputFile End(%d)", __LINE__, iRet);
    return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(closeOutputFile)(JNIEnv *env, jobject thiz, jint setOutputFD)
{
    LOGI("[nexEDitor_jni.cpp %d] closeOutputFile In(%d)", __LINE__,setOutputFD);
	if (!g_VideoEditorHandle){
		return -1;
	}
   
    int iRet = unregOutPutFD(setOutputFD);
    LOGI("[nexEDitor_jni.cpp %d] closeOutputFile End(%d)", __LINE__, iRet);
    return iRet;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setInputFile)(JNIEnv *env, jobject thiz, jobject fileDescriptor, jint predef ,jlong offset, jlong length)
{
    LOGI("[nexEDitor_jni.cpp %d] setInputFile In", __LINE__);
   
    if (fileDescriptor == NULL) {
    	LOGI("[nexEDitor_jni.cpp %d] setInputFile fileDescriptor is null", __LINE__);
        return -2;
    }
    int fd = nexjniGetFDFromFileDescriptor(env, fileDescriptor);

    struct stat sb;
    int ret = fstat(fd, &sb);
    if (ret != 0) {
        LOGI("[nexEDitor_jni.cpp %d]setInputFile fstat(%d) failed: %d, %s", __LINE__, fd, ret, strerror(errno));
        return -3;
    }

		if( predef == 1 ){ //asset
			LOGI("[nexEDitor_jni.cpp %d] setInputFile Asset End(%d)", __LINE__, fd);
			return regAssetFD(fd);
		}

    LOGI("[nexEDitor_jni.cpp %d] setInputFile not supported predef(%d)", __LINE__, predef);
    return -4;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(closeInputFile)(JNIEnv *env, jobject thiz, jint predef, jint setInputFD)
{
    LOGI("[nexEDitor_jni.cpp %d] closeInputFile In(%d,%d)", __LINE__,predef,setInputFD);
		if (!g_VideoEditorHandle){
			return -1;
		}
   	int iRet = -4;
   	if( predef == 1 ){ //asset
    	iRet = unregAssetFD(setInputFD);
  	}
    LOGI("[nexEDitor_jni.cpp %d] closeInputFile End(%d)", __LINE__, iRet);
    return iRet;
}


JNIEXPORT jint JNICALL _EDITOR_JNI_(setVideoTrackUUID)(JNIEnv *env, jobject thiz, jint iMode, jbyteArray arrayUUID)
{
	LOGI("[nexEDitor_jni.cpp %d] setVideoTrackUUID In(%d)", __LINE__, iMode);
	unsigned char *pUUID = NULL;
	if (!g_VideoEditorHandle){
		return -1;
	}

	if (arrayUUID != NULL){
		jsize size = env->GetArrayLength(arrayUUID);
		jbyte *jarr = env->GetByteArrayElements(arrayUUID, NULL);
		pUUID = (unsigned char *)malloc(size);
		memcpy(pUUID, jarr, size);
		env->ReleaseByteArrayElements(arrayUUID, jarr, JNI_ABORT);
	}

	g_VideoEditorHandle->setVideoTrackUUID(iMode, pUUID);
	if (pUUID != NULL){
		free(pUUID);
	}
	LOGI("[nexEDitor_jni.cpp %d] setVideoTrackUUID End", __LINE__);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getAudioSessionID)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d]", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 0;
	}
	return (int)g_VideoEditorHandle->getAudioSessionID();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(set360VideoTrackPosition)(JNIEnv * env, jobject obj, jint iXAngle , jint iYAngle , jint iflags)
{
	LOGI("[nexEDitor_jni.cpp %d] set360VideoPosition(%d,%d)", __LINE__,iXAngle,iYAngle);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 0;
	}
	return g_VideoEditorHandle->set360VideoTrackPosition(iXAngle,iYAngle, iflags);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(releaseLUTTexture)(JNIEnv * env, jobject obj, jint resource_id)
{
	LOGI("[nexEDitor_jni.cpp %d] releaseLUTTexture(%d)", __LINE__,resource_id);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 0;
	}
	return g_VideoEditorHandle->releaseLUTTexture(resource_id);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setTaskSleep)(JNIEnv * env, jobject obj, jint sleep)
{
	LOGI("[nexEDitor_jni.cpp %d] setTaskSleep(%d)", __LINE__, sleep);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 0;
	}
	return g_VideoEditorHandle->setTaskSleep(sleep);
}

JNIEXPORT jintArray JNICALL _EDITOR_JNI_(createCubeLUT)(JNIEnv * env, jobject obj, jbyteArray stream)
{
	LOGE("[nexEditor_jni.cpp %d] createCubeLUT", __LINE__);

	jsize size = env->GetArrayLength(stream);
	jbyte *jarr = env->GetByteArrayElements(stream, NULL);
	int* lut = g_VideoEditorHandle->createCubeLUT((const char*)jarr);
	env->ReleaseByteArrayElements(stream, jarr, JNI_ABORT);

	jintArray arr;
	
	if( g_VideoEditorHandle )
	{
		if(lut)
		{
			int size = 64 * 4096;
			arr = env->NewIntArray(size);
			env->SetIntArrayRegion(arr, 0, size, (const jint*)lut);
		}
	}
	return arr;
}

JNIEXPORT jintArray JNICALL _EDITOR_JNI_(createLGLUT)(JNIEnv * env, jobject obj, jbyteArray stream)
{
	LOGE("[nexEditor_jni.cpp %d] createLGLUT", __LINE__);

	jsize size = env->GetArrayLength(stream);
	jbyte *jarr = env->GetByteArrayElements(stream, NULL);
	int* lut = g_VideoEditorHandle->createLGLUT((const char*)jarr);
	env->ReleaseByteArrayElements(stream, jarr, JNI_ABORT);

	jintArray arr;
	
	if( g_VideoEditorHandle )
	{
		if(lut)
		{
			int size = 64 * 4096;
			arr = env->NewIntArray(size);
			env->SetIntArrayRegion(arr, 0, size, (const jint*)lut);
		}
	}
	return arr;
}




jobject createVisualClip(JNIEnv * env, IClipItem* pClip)
{
	jfieldID		id;	
	jmethodID	methodID;	
	jobject 		objVisualClip;
	jmethodID	nexVisualClip_construct;	
	jclass		visualClipInfo_ref_class;

	visualClipInfo_ref_class = env->FindClass(INTERNAL_SDK_PACKAGE_PATH "NexVisualClip");
	
	if( visualClipInfo_ref_class == NULL )
	{
		return NULL;
	}
	nexVisualClip_construct = env->GetMethodID(visualClipInfo_ref_class, "<init>", "()V");
	if( nexVisualClip_construct == NULL )
	{
		env->DeleteLocalRef(visualClipInfo_ref_class);
		return NULL;
	}

	objVisualClip = env->NewObject(visualClipInfo_ref_class, nexVisualClip_construct);
	LOGI("[nexEditor_jni %d] createVisualClip(0x%p)", __LINE__, objVisualClip);
	if( objVisualClip == NULL )
	{
		env->DeleteLocalRef(visualClipInfo_ref_class);
		return NULL;
	}

	id = env->GetFieldID(visualClipInfo_ref_class, "mClipID", "I");
	env->SetIntField(objVisualClip, id, pClip->getClipID());

	id = env->GetFieldID(visualClipInfo_ref_class, "mClipType", "I");
	env->SetIntField(objVisualClip, id, pClip->getClipType());

	id = env->GetFieldID(visualClipInfo_ref_class, "mTotalAudioTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getTotalAudioTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mTotalVideoTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getTotalVideoTime());
	
	id = env->GetFieldID(visualClipInfo_ref_class, "mTotalTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getTotalTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mStartTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getStartTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mEndTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getEndTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mStartTrimTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getStartTrimTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mEndTrimTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getEndTrimTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mWidth", "I");
	env->SetIntField(objVisualClip, id, pClip->getWidth());

	id = env->GetFieldID(visualClipInfo_ref_class, "mHeight", "I");
	env->SetIntField(objVisualClip, id, pClip->getHeight());

	id = env->GetFieldID(visualClipInfo_ref_class, "mExistVideo", "I");
	env->SetIntField(objVisualClip, id, pClip->isVideoExist());

	id = env->GetFieldID(visualClipInfo_ref_class, "mExistAudio", "I");
	env->SetIntField(objVisualClip, id, pClip->isAudioExist());

	id = env->GetFieldID(visualClipInfo_ref_class, "mTitleStyle", "I");
	env->SetIntField(objVisualClip, id, pClip->getTitleStyle());

	id = env->GetFieldID(visualClipInfo_ref_class, "mTitleStartTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getTitleStartTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mTitleEndTime", "I");
	env->SetIntField(objVisualClip, id, pClip->getTitleEndTime());

	id = env->GetFieldID(visualClipInfo_ref_class, "mAudioOnOff", "I");
	env->SetIntField(objVisualClip, id, pClip->getAudioOnOff());

	id = env->GetFieldID(visualClipInfo_ref_class, "mClipVolume", "I");
	env->SetIntField(objVisualClip, id, pClip->getAudioVolume());

	id = env->GetFieldID(visualClipInfo_ref_class, "mBGMVolume", "I");
	env->SetIntField(objVisualClip, id, pClip->getBGMVolume());

	id = env->GetFieldID(visualClipInfo_ref_class, "mEffectDuration", "I");
	env->SetIntField(objVisualClip, id, pClip->getClipEffectDuration());

	id = env->GetFieldID(visualClipInfo_ref_class, "mEffectOffset", "I");
	env->SetIntField(objVisualClip, id, pClip->getClipEffectOffset());

	id = env->GetFieldID(visualClipInfo_ref_class, "mEffectOverlap", "I");
	env->SetIntField(objVisualClip, id, pClip->getClipEffectOverlap());

	id = env->GetFieldID(visualClipInfo_ref_class, "mRotateState", "I");
	env->SetIntField(objVisualClip, id, pClip->getRotateState());
	LOGI("[nexEditor_jni %d] Visual clip Rotate State(%d)", __LINE__, pClip->getRotateState());

	id = env->GetFieldID(visualClipInfo_ref_class, "mBrightness", "I");
	env->SetIntField(objVisualClip, id, pClip->getBrightness());

	id = env->GetFieldID(visualClipInfo_ref_class, "mContrast", "I");
	env->SetIntField(objVisualClip, id, pClip->getContrast());

	id = env->GetFieldID(visualClipInfo_ref_class, "mSaturation", "I");
	env->SetIntField(objVisualClip, id, pClip->getSaturation());

    id = env->GetFieldID(visualClipInfo_ref_class, "mHue", "I");
	env->SetIntField(objVisualClip, id, pClip->getHue());

	id = env->GetFieldID(visualClipInfo_ref_class, "mTintcolor", "I");
	env->SetIntField(objVisualClip, id, pClip->getTintcolor());

	id = env->GetFieldID(visualClipInfo_ref_class, "mSpeedControl", "I");
	env->SetIntField(objVisualClip, id, pClip->getSpeedCtlFactor());

	id = env->GetFieldID(visualClipInfo_ref_class, "mKeepPitch", "I");
	env->SetIntField(objVisualClip, id, pClip->getKeepPitch());

	id = env->GetFieldID(visualClipInfo_ref_class, "mVoiceChanger", "I");
	env->SetIntField(objVisualClip, id, pClip->getVoiceChangerFactor());

	id = env->GetFieldID(visualClipInfo_ref_class, "mCompressor", "I");
	env->SetIntField(objVisualClip, id, pClip->getCompressorFactor());

	id = env->GetFieldID(visualClipInfo_ref_class, "mPitchFactor", "I");
	env->SetIntField(objVisualClip, id, pClip->getPitchFactor());

	id = env->GetFieldID(visualClipInfo_ref_class, "mMusicEffector", "I");
	env->SetIntField(objVisualClip, id, pClip->getMusicEffector());

	id = env->GetFieldID(visualClipInfo_ref_class, "mProcessorStrength", "I");
	env->SetIntField(objVisualClip, id, pClip->getProcessorStrength());

	id = env->GetFieldID(visualClipInfo_ref_class, "mBassStrength", "I");
	env->SetIntField(objVisualClip, id, pClip->getBassStrength());

	if( pClip->getEnhancedAudioFilter() != NULL && strlen(pClip->getEnhancedAudioFilter()) > 0 )
	{
		jstring strEnhancedAudioFilter = env->NewStringUTF(pClip->getEnhancedAudioFilter());
		if( strEnhancedAudioFilter )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mEnhancedAudioFilter", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strEnhancedAudioFilter);
			env->DeleteLocalRef(strEnhancedAudioFilter);
		}
		//LOGI("[nexEDitor_jni.cpp %d] createVisualClip mEnhancedAudioFilter (%s)", __LINE__, pClip->getEnhancedAudioFilter());
	}

	if( pClip->getEqualizer() != NULL && strlen(pClip->getEqualizer()) > 0 )
	{
		jstring strEqualizer = env->NewStringUTF(pClip->getEqualizer());
		if( strEqualizer )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mEqualizer", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strEqualizer);
			env->DeleteLocalRef(strEqualizer);
		}
		//LOGI("[nexEDitor_jni.cpp %d] createVisualClip mEqualizer (%s)", __LINE__, pClip->getEqualizer());
	}

	if( pClip->getClipPath() != NULL && strlen(pClip->getClipPath()) > 0 )
	{
		jstring strClipPath = env->NewStringUTF(pClip->getClipPath());
		if( strClipPath )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mClipPath", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strClipPath);
			env->DeleteLocalRef(strClipPath);
		}
	}

	if( pClip->getClipEffectID() != NULL && strlen(pClip->getClipEffectID()) > 0 )
	{
		jstring strClipEffectID = env->NewStringUTF(pClip->getClipEffectID());
		if( strClipEffectID )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mClipEffectID", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strClipEffectID);
			env->DeleteLocalRef(strClipEffectID);
		}
	}

	if( pClip->getTitleEffectID() != NULL && strlen(pClip->getTitleEffectID()) > 0 )
	{
		jstring strTitleEffectID = env->NewStringUTF(pClip->getTitleEffectID());
		if( strTitleEffectID )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mTitleEffectID", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strTitleEffectID);
			env->DeleteLocalRef(strTitleEffectID);
		}
	}	

	if( pClip->getFilterID() != NULL && strlen(pClip->getFilterID()) > 0 )
	{
		jstring strFilterID = env->NewStringUTF(pClip->getFilterID());
		if( strFilterID )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mFilterID", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strFilterID);
			env->DeleteLocalRef(strFilterID);
		}
	}

	LOGI("[nexEditor_jni %d] createVisualClip(0x%p)", __LINE__, objVisualClip);
	if( pClip->getTitle() != NULL && strlen(pClip->getTitle()) > 0 )
	{
		jstring strPath = env->NewStringUTF(pClip->getTitle());
		if( strPath )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mTitle", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strPath);
			env->DeleteLocalRef(strPath);
		}
	}
	if( strlen(pClip->getThumbnailPath()) > 0 )
	{
		jstring strPath = env->NewStringUTF(pClip->getThumbnailPath());
		if( strPath )
		{
			id = env->GetFieldID(visualClipInfo_ref_class, "mThumbnailPath", "Ljava/lang/String;");
			env->SetObjectField(objVisualClip, id, strPath);
			env->DeleteLocalRef(strPath);
		}
	}

	int iCount = 0;
	float* pMatrix = NULL;

	pMatrix = pClip->getStartMatrix(&iCount);
	if( pMatrix != NULL )
	{
		jfloatArray arr = env->NewFloatArray(iCount);
		float* pBuffer = (float*)env->GetFloatArrayElements(arr, 0);
		if( pBuffer != NULL )
		{
			memcpy(pBuffer, pMatrix, sizeof(float)*iCount);
			env->ReleaseFloatArrayElements(arr, pBuffer, 0);
		}

		id = env->GetFieldID(visualClipInfo_ref_class, "mStartMatrix", "[F");
		env->SetObjectField(objVisualClip, id, arr);
	}

	pMatrix = pClip->getEndMatrix(&iCount);
	if( pMatrix != NULL )
	{
		jfloatArray arr = env->NewFloatArray(iCount);
		float* pBuffer = (float*)env->GetFloatArrayElements(arr, 0);
		if( pBuffer != NULL )
		{
			memcpy(pBuffer, pMatrix, sizeof(float)*iCount);
			env->ReleaseFloatArrayElements(arr, pBuffer, 0);
		}

		id = env->GetFieldID(visualClipInfo_ref_class, "mEndMatrix", "[F");
		env->SetObjectField(objVisualClip, id, arr);
	}	

	IRectangle* pRect = pClip->getStartPosition();
	if( pRect )
	{
		methodID = env->GetMethodID(visualClipInfo_ref_class, "setStartRect", "(IIII)V");
		if( methodID == NULL )
		{
			SAFE_RELEASE(pRect);
			env->DeleteLocalRef(objVisualClip);
			env->DeleteLocalRef(visualClipInfo_ref_class);
			LOGI("[getClipList %d] get methodID failed",__LINE__);
			return NULL;
		}
		env->CallVoidMethod(objVisualClip, methodID, pRect->getLeft(), pRect->getTop(), pRect->getRight(), pRect->getBottom());
		SAFE_RELEASE(pRect);
	}

	pRect = pClip->getEndPosition();
	if( pRect )
	{
		methodID = env->GetMethodID(visualClipInfo_ref_class, "setEndRect", "(IIII)V");
		if( methodID == NULL )
		{
			SAFE_RELEASE(pRect);
			env->DeleteLocalRef(objVisualClip);
			env->DeleteLocalRef(visualClipInfo_ref_class);
			LOGI("[getClipList %d] get methodID failed",__LINE__);
			return NULL;
		}
		env->CallVoidMethod(objVisualClip, methodID, pRect->getLeft(), pRect->getTop(), pRect->getRight(), pRect->getBottom());
		SAFE_RELEASE(pRect);
	}

	pRect = pClip->getDstPosition();
	if( pRect )
	{
		methodID = env->GetMethodID(visualClipInfo_ref_class, "setDestRect", "(IIII)V");
		if( methodID == NULL )
		{
			SAFE_RELEASE(pRect);
			env->DeleteLocalRef(objVisualClip);
			env->DeleteLocalRef(visualClipInfo_ref_class);
			LOGI("[getClipList %d] get methodID failed",__LINE__);
			return NULL;
		}
		env->CallVoidMethod(objVisualClip, methodID, pRect->getLeft(), pRect->getTop(), pRect->getRight(), pRect->getBottom());
		SAFE_RELEASE(pRect);
	}

	env->DeleteLocalRef(visualClipInfo_ref_class);
	LOGI("[nexEditor_jni %d] createVisualClip(0x%p)", __LINE__, objVisualClip);
	return objVisualClip;
}

jobject createAudioClip(JNIEnv * env, IClipItem* pClip)
{
	jfieldID		id;	
	jobject 		objAudioClip;
	jmethodID	nexAudioClip_construct;
	jclass		audioClipInfo_ref_class;
	
	audioClipInfo_ref_class = env->FindClass(INTERNAL_SDK_PACKAGE_PATH "NexAudioClip");
	
	if( audioClipInfo_ref_class == NULL )
	{
		return NULL;
	}
	nexAudioClip_construct = env->GetMethodID(audioClipInfo_ref_class, "<init>", "()V");
	if( nexAudioClip_construct == NULL )
	{
		env->DeleteLocalRef(audioClipInfo_ref_class);
		return NULL;
	}
	
	objAudioClip = env->NewObject(audioClipInfo_ref_class, nexAudioClip_construct);
	LOGI("[nexEditor_jni %d] createAudioClip(0x%p)", __LINE__, objAudioClip);
	if( objAudioClip == NULL )
	{
		env->DeleteLocalRef(audioClipInfo_ref_class);
		return NULL;
	}

	id = env->GetFieldID(audioClipInfo_ref_class, "mClipID", "I");
	env->SetIntField(objAudioClip, id, pClip->getClipID());

	id = env->GetFieldID(audioClipInfo_ref_class, "mVisualClipID", "I");
	env->SetIntField(objAudioClip, id, pClip->getVisualClipID());
	
	id = env->GetFieldID(audioClipInfo_ref_class, "mClipType", "I");
	env->SetIntField(objAudioClip, id, pClip->getClipType());

	id = env->GetFieldID(audioClipInfo_ref_class, "mTotalTime", "I");
	env->SetIntField(objAudioClip, id, pClip->getTotalTime());

	id = env->GetFieldID(audioClipInfo_ref_class, "mStartTime", "I");
	env->SetIntField(objAudioClip, id, pClip->getStartTime());

	id = env->GetFieldID(audioClipInfo_ref_class, "mEndTime", "I");
	env->SetIntField(objAudioClip, id, pClip->getEndTime());

	id = env->GetFieldID(audioClipInfo_ref_class, "mStartTrimTime", "I");
	env->SetIntField(objAudioClip, id, pClip->getStartTrimTime());

	id = env->GetFieldID(audioClipInfo_ref_class, "mEndTrimTime", "I");
	env->SetIntField(objAudioClip, id, pClip->getEndTrimTime());

	id = env->GetFieldID(audioClipInfo_ref_class, "mAudioOnOff", "I");
	env->SetIntField(objAudioClip, id, pClip->getAudioOnOff());	

	id = env->GetFieldID(audioClipInfo_ref_class, "mClipVolume", "I");
	env->SetIntField(objAudioClip, id, pClip->getAudioVolume());

	if( pClip->getClipPath() != NULL && strlen(pClip->getClipPath()) > 0 )
	{
		jstring strClipPath = env->NewStringUTF(pClip->getClipPath());
		if( strClipPath )
		{
			id = env->GetFieldID(audioClipInfo_ref_class, "mClipPath", "Ljava/lang/String;");
			env->SetObjectField(objAudioClip, id, strClipPath);
		}
	}

	env->DeleteLocalRef(audioClipInfo_ref_class);
	return objAudioClip;
}

int setVisualClip(JNIEnv * env, jobject clipInfo, IClipItem* pClip)
{
	if( pClip == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Clip handle", __LINE__);
		return 1;
	}

	jfieldID		id;	
	jclass		clipInfo_ref_class;
	clipInfo_ref_class = env->GetObjectClass(clipInfo);
	if( clipInfo_ref_class == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipType", "I");
	CLIP_TYPE clipType = (CLIP_TYPE)env->GetIntField(clipInfo, id);
	pClip->setClipType(clipType);

	id = env->GetFieldID(clipInfo_ref_class, "mTotalTime", "I");
	pClip->setTotalTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mStartTime", "I");
	pClip->setStartTime((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mEndTime", "I");
	pClip->setEndTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mStartTrimTime", "I");
	pClip->setStartTrimTime((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEndTrimTime", "I");
	pClip->setEndTrimTime((unsigned int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mWidth", "I");
	pClip->setWidth((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mHeight", "I");
	pClip->setHeight((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mExistVideo", "I");
	pClip->setVideoExist((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mExistAudio", "I");
	pClip->setAudioExist((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mClipPath", "Ljava/lang/String;");
	jstring clipPath = (jstring)env->GetObjectField(clipInfo, id);
	if( clipPath )
	{
		const char *str = env->GetStringUTFChars(clipPath, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(clipPath);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setClipPath((char*)str);
		env->ReleaseStringUTFChars(clipPath, str);
		env->DeleteLocalRef(clipPath);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mThumbnailPath", "Ljava/lang/String;");
	jstring thumbnailPath = (jstring)env->GetObjectField(clipInfo, id);
	if( thumbnailPath )
	{
		const char *str = env->GetStringUTFChars(thumbnailPath, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(thumbnailPath);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setThumbnailPath((const char*)str);
		env->ReleaseStringUTFChars(thumbnailPath, str);
		env->DeleteLocalRef(thumbnailPath);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mTitleStyle", "I");
	pClip->setTitleStyle((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mTitleStartTime", "I");
	pClip->setTitleStartTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTitleEndTime", "I");
	pClip->setTitleEndTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTitle", "Ljava/lang/String;");
	jstring title = (jstring)env->GetObjectField(clipInfo, id);
	if( title )
	{
		const char *str = env->GetStringUTFChars(title, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(title);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setTitle((char*)str);
		env->ReleaseStringUTFChars(title, str);
		env->DeleteLocalRef(title);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mAudioOnOff", "I");
	pClip->setAudioOnOff((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mClipVolume", "I");
	pClip->setAudioVolume((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mBGMVolume", "I");
	pClip->setBGMVolume((int)env->GetIntField(clipInfo, id));


	id = env->GetFieldID(clipInfo_ref_class, "mPanLeft", "I");
	pClip->setPanLeftFactor((int)env->GetIntField(clipInfo, id));
	id = env->GetFieldID(clipInfo_ref_class, "mPanRight", "I");
	pClip->setPanRightFactor((int)env->GetIntField(clipInfo, id));

	if( clipType == CLIPTYPE_AUDIO || clipType == CLIPTYPE_VIDEO || clipType == CLIPTYPE_VIDEO_LAYER )
	{
		jint* pVolumeEnvelopLevel = NULL;
		int nVolumeEnvelopeSize = 0;
		jint* pVolumeEnvelopTime = NULL;


		id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeLevel", "[I");
		jintArray arrVolumeEnvelopeLevel = (jintArray) env->GetObjectField(clipInfo, id);
		if(arrVolumeEnvelopeLevel != NULL)
		{
			pVolumeEnvelopLevel = env->GetIntArrayElements(arrVolumeEnvelopeLevel, NULL);
			nVolumeEnvelopeSize = env->GetArrayLength( arrVolumeEnvelopeLevel);
		}
		id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeTime", "[I");
		jintArray arrVolumeEnvelopeTime = (jintArray) env->GetObjectField(clipInfo, id);
		if( arrVolumeEnvelopeTime != NULL)
		{
			pVolumeEnvelopTime = env->GetIntArrayElements(arrVolumeEnvelopeTime, NULL);
		}

		if( nVolumeEnvelopeSize > 0 && pVolumeEnvelopTime != NULL && pVolumeEnvelopLevel != NULL )
			pClip->setAudioEnvelop(nVolumeEnvelopeSize, (unsigned int*)pVolumeEnvelopTime, (unsigned int*)pVolumeEnvelopLevel);
		
		// Release Envelope Array.
		if(pVolumeEnvelopTime != NULL)
		{
			env->ReleaseIntArrayElements(arrVolumeEnvelopeTime, pVolumeEnvelopTime, 0);
			env->DeleteLocalRef(arrVolumeEnvelopeTime);
		}

		if(pVolumeEnvelopLevel != NULL)
		{
			env->ReleaseIntArrayElements(arrVolumeEnvelopeLevel, pVolumeEnvelopLevel, 0);
			env->DeleteLocalRef(arrVolumeEnvelopeLevel);
		}
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEffectDuration", "I");
	pClip->setClipEffectDuration((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEffectOffset", "I");
	pClip->setClipEffectOffset((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEffectOverlap", "I");
	pClip->setClipEffectOverlap((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mRotateState", "I");
	int iRotate = (int)env->GetIntField(clipInfo, id);
	pClip->setRotateState(iRotate);
	// LOGI("[nexEditor_jni %d] Visual clip Rotate State(%d)", __LINE__, iRotate);

	id = env->GetFieldID(clipInfo_ref_class, "mBrightness", "I");
	pClip->setBrightness((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mContrast", "I");
	pClip->setContrast((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mSaturation", "I");
	pClip->setSaturation((int)env->GetIntField(clipInfo, id));

    id = env->GetFieldID(clipInfo_ref_class, "mHue", "I");
	pClip->setHue((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mLUT", "I");
	pClip->setLUT((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCustomLUT_A", "I");
	pClip->setCustomLUTA((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCustomLUT_B", "I");
	pClip->setCustomLUTB((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCustomLUT_Power", "I");
	pClip->setCustomLUTPower((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mVignette", "I");
	pClip->setVignette((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTintcolor", "I");
	pClip->setTintcolor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mSpeedControl", "I");
	pClip->setSpeedCtlFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mKeepPitch", "I");
	pClip->setKeepPitch((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mVoiceChanger", "I");
	pClip->setVoiceChangerFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCompressor", "I");
	pClip->setCompressorFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mPitchFactor", "I");
	pClip->setPitchFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mMusicEffector", "I");
	pClip->setMusicEffector((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mProcessorStrength", "I");
	pClip->setProcessorStrength((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mBassStrength", "I");
	pClip->setBassStrength((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mMotionTracked", "I");
	pClip->setMotionTracked((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mFreezeDuration", "I");
	pClip->setFreezeDuration((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEnhancedAudioFilter", "Ljava/lang/String;");
	jstring sEnhancedAudioFilter = (jstring)env->GetObjectField(clipInfo, id);
	if( sEnhancedAudioFilter )
	{
		const char *str = env->GetStringUTFChars(sEnhancedAudioFilter, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(sEnhancedAudioFilter);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setEnhancedAudioFilter((char*)str);
		env->ReleaseStringUTFChars(sEnhancedAudioFilter, str);
		env->DeleteLocalRef(sEnhancedAudioFilter);
		//LOGI("[nexEDitor_jni.cpp %d] setVisualClip mEnhancedAudioFilter (%s)", __LINE__, pClip->getEnhancedAudioFilter());
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEqualizer", "Ljava/lang/String;");
	jstring sEqualizer = (jstring)env->GetObjectField(clipInfo, id);
	if( sEqualizer )
	{
		const char *str = env->GetStringUTFChars(sEqualizer, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(sEqualizer);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setEqualizer((char*)str);
		env->ReleaseStringUTFChars(sEqualizer, str);
		env->DeleteLocalRef(sEqualizer);
		//LOGI("[nexEDitor_jni.cpp %d] setVisualClip mEqualizer (%s)", __LINE__, pClip->getEqualizer());
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipEffectID", "Ljava/lang/String;");
	jstring mClipEffectID = (jstring)env->GetObjectField(clipInfo, id);
	if( mClipEffectID )
	{
		const char *str = env->GetStringUTFChars(mClipEffectID, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(mClipEffectID);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setClipEffectID((char*)str);
		// LOGI("[nexEDitor_jni.cpp %d] setVisualClip ClipEffectID(%s)", __LINE__, str);
		env->ReleaseStringUTFChars(mClipEffectID, str);
		env->DeleteLocalRef(mClipEffectID);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mTitleEffectID", "Ljava/lang/String;");
	jstring mTitleEffectID = (jstring)env->GetObjectField(clipInfo, id);
	if( mTitleEffectID )
	{
		const char *str = env->GetStringUTFChars(mTitleEffectID, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(mTitleEffectID);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setTitleEffectID((char*)str);
		// LOGI("[nexEDitor_jni.cpp %d] setVisualClip TitleEffectID(%s)", __LINE__, str);
		env->ReleaseStringUTFChars(mTitleEffectID, str);
		env->DeleteLocalRef(mTitleEffectID);
	}	

	jint iLeft		= 0;
	jint iTop		= 0;
	jint iRight	= 0;
	jint iBottom	= 0;

	id = env->GetFieldID(clipInfo_ref_class, "mStartRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");
	
	jobject objRect = env->GetObjectField(clipInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			IRectangle* pRect = pClip->getStartPosition();
			if( pRect )
			{
				pRect->setRect(iLeft, iTop, iRight, iBottom);
				pRect->Release();
			}
			env->DeleteLocalRef(rectangle_ref_class);
			
		}
		env->DeleteLocalRef(objRect);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEndRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");
	
	objRect = env->GetObjectField(clipInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			IRectangle* pRect = pClip->getEndPosition();
			if( pRect )
			{
				pRect->setRect(iLeft, iTop, iRight, iBottom);
				pRect->Release();
			}
			env->DeleteLocalRef(rectangle_ref_class);
		}
		env->DeleteLocalRef(objRect);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mDestRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");
	
	objRect = env->GetObjectField(clipInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);
			
			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			IRectangle* pRect = pClip->getDstPosition();
			if( pRect )
			{
				pRect->setRect(iLeft, iTop, iRight, iBottom);
				pRect->Release();
			}
			env->DeleteLocalRef(rectangle_ref_class);

		}
		env->DeleteLocalRef(objRect);
	}	

	id = env->GetFieldID(clipInfo_ref_class, "mSlowMotion", "I");
	pClip->setSlowMotion((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mIframePlay", "I");
	pClip->setIframePlay((int)env->GetIntField(clipInfo, id));

//	id = env->GetFieldID(clipInfo_ref_class, "mReverse", "I");
//	pClip->setReverse((int)env->GetIntField(clipInfo, id));

	//LOGI("[nexEDitor_jni.cpp %d] setVisualClip Done(Clip ID : %d)", __LINE__, pClip->getClipID());

	env->DeleteLocalRef(clipInfo_ref_class);
	return 0;
}

int setAudioClip(JNIEnv * env, jobject clipInfo, IClipItem* pClip)
{
	if( pClip == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Clip handle", __LINE__);
		return 1;
	}

	jfieldID		id;	
	jclass		clipInfo_ref_class;
	clipInfo_ref_class = env->GetObjectClass(clipInfo);
	if( clipInfo_ref_class == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipType", "I");
	CLIP_TYPE clipType = (CLIP_TYPE)env->GetIntField(clipInfo, id);
	pClip->setClipType(clipType);

	id = env->GetFieldID(clipInfo_ref_class, "mTotalTime", "I");
	pClip->setTotalTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mStartTime", "I");
	pClip->setStartTime((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mEndTime", "I");
	pClip->setEndTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mStartTrimTime", "I");
	pClip->setStartTrimTime((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEndTrimTime", "I");
	pClip->setEndTrimTime((unsigned int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mClipPath", "Ljava/lang/String;");
	jstring clipPath = (jstring)env->GetObjectField(clipInfo, id);
	if( clipPath )
	{
		const char *str = env->GetStringUTFChars(clipPath, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(clipPath);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setClipPath((char*)str);
		env->ReleaseStringUTFChars(clipPath, str);
		env->DeleteLocalRef(clipPath);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mAudioOnOff", "I");
	pClip->setAudioOnOff((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mAutoEnvelop", "I");
	pClip->setAutoEnvelop((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mClipVolume", "I");
	pClip->setAudioVolume((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mVoiceChanger", "I");
	pClip->setVoiceChangerFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mCompressor", "I");
	pClip->setCompressorFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mPitchFactor", "I");
	pClip->setPitchFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mMusicEffector", "I");
	pClip->setMusicEffector((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mProcessorStrength", "I");
	pClip->setProcessorStrength((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mBassStrength", "I");
	pClip->setBassStrength((int)env->GetIntField(clipInfo, id));
	id = env->GetFieldID(clipInfo_ref_class, "mPanLeft", "I");
	pClip->setPanLeftFactor((int)env->GetIntField(clipInfo, id));
	id = env->GetFieldID(clipInfo_ref_class, "mPanRight", "I");
	pClip->setPanRightFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEnhancedAudioFilter", "Ljava/lang/String;");
	jstring sEnhancedAudioFilter = (jstring)env->GetObjectField(clipInfo, id);
	if( sEnhancedAudioFilter )
	{
		const char *str = env->GetStringUTFChars(sEnhancedAudioFilter, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(sEnhancedAudioFilter);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setEnhancedAudioFilter((char*)str);
		env->ReleaseStringUTFChars(sEnhancedAudioFilter, str);
		env->DeleteLocalRef(sEnhancedAudioFilter);
		//LOGI("[nexEDitor_jni.cpp %d] setAudioClip mEnhancedAudioFilter (%s)", __LINE__, pClip->getEnhancedAudioFilter());
	}

	id = env->GetFieldID(clipInfo_ref_class, "mEqualizer", "Ljava/lang/String;");
	jstring sEqualizer = (jstring)env->GetObjectField(clipInfo, id);
	if( sEqualizer )
	{
		const char *str = env->GetStringUTFChars(sEqualizer, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(sEqualizer);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pClip->setEqualizer((char*)str);
		env->ReleaseStringUTFChars(sEqualizer, str);
		env->DeleteLocalRef(sEqualizer);
		//LOGI("[nexEDitor_jni.cpp %d] setAudioClip mEqualizer (%s)", __LINE__, pClip->getEqualizer());
	}

	jint* pVolumeEnvelopLevel = NULL;
	int nVolumeEnvelopeSize = 0;
	jint* pVolumeEnvelopTime = NULL;

	id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeLevel", "[I");
	jintArray arrVolumeEnvelopeLevel = (jintArray) env->GetObjectField(clipInfo, id);
	if(arrVolumeEnvelopeLevel != NULL)
	{
	    pVolumeEnvelopLevel = env->GetIntArrayElements(arrVolumeEnvelopeLevel, NULL);
	    nVolumeEnvelopeSize = env->GetArrayLength( arrVolumeEnvelopeLevel);
	    
	}
	id = env->GetFieldID(clipInfo_ref_class, "mVolumeEnvelopeTime", "[I");
	jintArray arrVolumeEnvelopeTime = (jintArray) env->GetObjectField(clipInfo, id);
	if( arrVolumeEnvelopeTime != NULL)
	{
	    pVolumeEnvelopTime = env->GetIntArrayElements(arrVolumeEnvelopeTime, NULL);
	}

	if( nVolumeEnvelopeSize > 0 && pVolumeEnvelopTime != NULL && pVolumeEnvelopLevel != NULL )
	    pClip->setAudioEnvelop(nVolumeEnvelopeSize, (unsigned int*)pVolumeEnvelopTime, (unsigned int*)pVolumeEnvelopLevel);

    //Todo : set speed
    //pClip->setSpeedCtlFactor(100);
   	id = env->GetFieldID(clipInfo_ref_class, "mSpeedControl", "I");
	pClip->setSpeedCtlFactor((int)env->GetIntField(clipInfo, id));

	// Release Envelope Array.
	if(pVolumeEnvelopTime != NULL)
		env->ReleaseIntArrayElements(arrVolumeEnvelopeTime, pVolumeEnvelopTime, 0);

	if(pVolumeEnvelopLevel != NULL)
		env->ReleaseIntArrayElements(arrVolumeEnvelopeLevel, pVolumeEnvelopLevel, 0);



	//for( int i = 0 ; i < nVolumeEnvelopeSize ; i++)
	//{
	//	LOGI("[nexEDitor_jni.cpp %d] setAudioClip VOL ENV(%d, %d)", __LINE__,pVolumeEnvelopTime[i], pVolumeEnvelopLevel[i]);
	//}

	
	LOGI("[nexEDitor_jni.cpp %d] setAudioClip Done(Clip ID : %d)", __LINE__, pClip->getClipID());
	env->DeleteLocalRef(clipInfo_ref_class);
	return 0;
}

int setLayerInfo(JNIEnv * env, jobject clipInfo, ILayerItem* pLayer)
{
	if( pLayer == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid Clip handle", __LINE__);
		return 1;
	}

	jfieldID		id;	
	jclass		clipInfo_ref_class;
	clipInfo_ref_class = env->GetObjectClass(clipInfo);
	if( clipInfo_ref_class == NULL )
	{
		return 1;
	}

	id = env->GetFieldID(clipInfo_ref_class, "mID", "I");
	pLayer->setID((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mType", "I");
	CLIP_TYPE clipType = (CLIP_TYPE)env->GetIntField(clipInfo, id);
	pLayer->setType(clipType);

	id = env->GetFieldID(clipInfo_ref_class, "mTotalTime", "I");
	pLayer->setTotalTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mStartTime", "I");
	pLayer->setStartTime((int)env->GetIntField(clipInfo, id));
	
	id = env->GetFieldID(clipInfo_ref_class, "mEndTime", "I");
	pLayer->setEndTime((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mStartTrimTime", "I");
	pLayer->setStartTrimTime((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mEndTrimTime", "I");
	pLayer->setEndTrimTime((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mWidth", "I");
	pLayer->setWidth((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mHeight", "I");
	pLayer->setHeight((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mExistVideo", "I");
	pLayer->setVideoExist((unsigned int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mRotateState", "I");
	pLayer->setRotateState((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mBrightness", "I");
	pLayer->setBrightness((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mContrast", "I");
	pLayer->setContrast((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mSaturation", "I");
	pLayer->setSaturation((int)env->GetIntField(clipInfo, id));

    id = env->GetFieldID(clipInfo_ref_class, "mHue", "I");
	pLayer->setHue((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mTintcolor", "I");
	pLayer->setTintcolor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mLUT", "I");
	pLayer->setLUT((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mVignette", "I");
	pLayer->setVignette((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mSpeedControl", "I");
	pLayer->setSpeedCtlFactor((int)env->GetIntField(clipInfo, id));

	id = env->GetFieldID(clipInfo_ref_class, "mDisplayOptions", "Ljava/lang/String;");
	jstring displayoption = (jstring)env->GetObjectField(clipInfo, id);
	if( displayoption )
	{
		const char *str = env->GetStringUTFChars(displayoption, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(displayoption);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pLayer->setDisplayOption((char*)str);
		env->ReleaseStringUTFChars(displayoption, str);
		env->DeleteLocalRef(displayoption);
	}

	id = env->GetFieldID(clipInfo_ref_class, "mClipPath", "Ljava/lang/String;");
	jstring clippath = (jstring)env->GetObjectField(clipInfo, id);
	if( clippath )
	{
		const char *str = env->GetStringUTFChars(clippath, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(clippath);
			env->DeleteLocalRef(clipInfo_ref_class);
			return 1;
		}
		pLayer->setDisplayOption((char*)str);
		env->ReleaseStringUTFChars(clippath, str);
		env->DeleteLocalRef(clippath);
	}		
	
	LOGI("[nexEDitor_jni.cpp %d] setLayerInfo Done(Layer ID : %d)", __LINE__, pLayer->getID());
	env->DeleteLocalRef(clipInfo_ref_class);
	return 0;
}

int setDrawInfo(JNIEnv * env, jobject drawInfo, IDrawInfo* pDrawInfo)
{
	if( pDrawInfo == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid DrawInfo handle", __LINE__);
		return 1;
	}

	jfieldID		id;
	jclass			drawinfo_ref_class;
	drawinfo_ref_class = env->GetObjectClass(drawInfo);
	if( drawinfo_ref_class == NULL )
	{
		return 1;
	}

//	public int			mID;
//	public int			mTrackID;
//
//	public String       mEffectID;
//
//	public int			mStartTime;
//	public int			mEndTime;
//
//	public int			mRotateState;
//	public int			mBrightness;
//	public int			mContrast;
//	public int			mSaturation;
//	public int			mTintcolor;
//	public int			mLUT;
//
//	public NexRectangle    mStartRect;
//	public NexRectangle    mEndRect;

	id = env->GetFieldID(drawinfo_ref_class, "mID", "I");
	jint drawID = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mTrackID", "I");
	jint trackID = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mSubEffectID", "I");
	jint subEffectID = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mIsTransition", "I");
	jint isTransition = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mStartTime", "I");
	jint startTime = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mEndTime", "I");
	jint endTime = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mRotateState", "I");
	jint rotate = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mUserRotateState", "I");
	jint user_rotate = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mTranslateX", "I");
	jint translate_x = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mTranslateY", "I");
	jint translate_y = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mBrightness", "I");
	jint bright = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mContrast", "I");
	jint contrast = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mSaturation", "I");
	jint satuation = env->GetIntField(drawInfo, id);

    id = env->GetFieldID(drawinfo_ref_class, "mHue", "I");
	jint hue = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mTintcolor", "I");
	jint tintcolor = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mLUT", "I");
	jint lut = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mCustomLUT_A", "I");
	jint custom_lut_a = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mCustomLUT_B", "I");
	jint custom_lut_b = env->GetIntField(drawInfo, id);

	id = env->GetFieldID(drawinfo_ref_class, "mCustomLUT_Power", "I");
	jint custom_lut_power = env->GetIntField(drawInfo, id);

	pDrawInfo->setDrawInfo(drawID, trackID, subEffectID, isTransition, startTime, endTime, rotate, user_rotate, translate_x, translate_y, lut, custom_lut_a, custom_lut_b, custom_lut_power);

	pDrawInfo->setColor(bright, satuation, contrast, tintcolor, hue);

	id = env->GetFieldID(drawinfo_ref_class, "mEffectID", "Ljava/lang/String;");
	jstring clipPath = (jstring)env->GetObjectField(drawInfo, id);

	if( clipPath )
	{
		const char *str = env->GetStringUTFChars(clipPath, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(clipPath);
			env->DeleteLocalRef(drawinfo_ref_class);
			return 1;
		}

		pDrawInfo->setEffect(str);

		env->ReleaseStringUTFChars(clipPath, str);
		env->DeleteLocalRef(clipPath);
	}

	id = env->GetFieldID(drawinfo_ref_class, "mTitle", "Ljava/lang/String;");
	jstring title = (jstring)env->GetObjectField(drawInfo, id);

	if( title )
	{
		const char *str = env->GetStringUTFChars(title, NULL);
		if( str == NULL )
		{
			env->DeleteLocalRef(title);
			env->DeleteLocalRef(drawinfo_ref_class);
			return 1;
		}

		pDrawInfo->setTitle(str);

		env->ReleaseStringUTFChars(title, str);
		env->DeleteLocalRef(title);
	}


	jint iLeft		= 0;
	jint iTop		= 0;
	jint iRight		= 0;
	jint iBottom	= 0;

	id = env->GetFieldID(drawinfo_ref_class, "mStartRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");

	jobject objRect = env->GetObjectField(drawInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			pDrawInfo->setStartRect(iLeft, iTop, iRight, iBottom);

			env->DeleteLocalRef(rectangle_ref_class);

		}
		env->DeleteLocalRef(objRect);
	}

	id = env->GetFieldID(drawinfo_ref_class, "mEndRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");

	objRect = env->GetObjectField(drawInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			pDrawInfo->setEndRect(iLeft, iTop, iRight, iBottom);

			env->DeleteLocalRef(rectangle_ref_class);
		}
		env->DeleteLocalRef(objRect);
	}

	id = env->GetFieldID(drawinfo_ref_class, "mFaceRect", "L" INTERNAL_SDK_PACKAGE_PATH "NexRectangle;");

	objRect = env->GetObjectField(drawInfo, id);
	if( objRect )
	{
		jclass rectangle_ref_class = env->GetObjectClass(objRect);
		if( rectangle_ref_class )
		{
			id = env->GetFieldID(rectangle_ref_class, "mLeft", "I");
			iLeft = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mTop", "I");
			iTop = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mRight", "I");
			iRight = env->GetIntField(objRect, id);

			id = env->GetFieldID(rectangle_ref_class, "mBottom", "I");
			iBottom = env->GetIntField(objRect, id);

			pDrawInfo->setFaceRect(iLeft, iTop, iRight, iBottom);

			env->DeleteLocalRef(rectangle_ref_class);
		}
		env->DeleteLocalRef(objRect);
	}
	
	env->DeleteLocalRef(drawinfo_ref_class);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setPreviewScaleFactor)(JNIEnv * env, jobject obj, jfloat fScaleFactor)
{

	LOGI("[nexEDitor_jni.cpp %d] setPreviewScaleFactor %f", __LINE__, fScaleFactor);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setPreviewScaleFactor(fScaleFactor);
}

JNIEXPORT jfloat JNICALL _EDITOR_JNI_(getPreviewScaleFactor)(JNIEnv * env, jobject obj)
{

	LOGI("[nexEDitor_jni.cpp %d] getPreviewScaleFactor", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getPreviewScaleFactor();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(pushLoadedBitmap)(JNIEnv * env, jobject obj, jstring path, jintArray pixels, jint width, jint height, jint loadedtype)
{
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] pushLoadedBitmap failed because did not create editor handle", __LINE__);
		return 1;
	}

	const char *strPath = env->GetStringUTFChars(path, NULL);
	if(strPath == NULL){

		LOGI("[nexEDitor_jni.cpp %d] pushLoadedBitmap failed because did not image path", __LINE__);
		return 1;
	}
	
	jint* ppixels = NULL;
	if(pixels){

		ppixels = env->GetIntArrayElements(pixels, NULL);	
	}

	if(NULL == ppixels){

		LOGI("[nexEDitor_jni.cpp %d] pushLoadedBitmap failed because no pixels", __LINE__);
		env->ReleaseStringUTFChars(path, strPath);
		return 1;
	}

	
	int ret = g_VideoEditorHandle->pushLoadedBitmap(strPath, ppixels, width, height, loadedtype);
	env->ReleaseStringUTFChars(path, strPath);
	env->ReleaseIntArrayElements(pixels, ppixels, 0);
	return ret;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(removeBitmap)(JNIEnv * env, jobject obj, jstring path)
{
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] removeBitmap failed because did not create editor handle", __LINE__);
		return 1;
	}

	const char *strPath = env->GetStringUTFChars(path, NULL);
	if(strPath == NULL){

		LOGI("[nexEDitor_jni.cpp %d] removeBitmap failed because did not image path", __LINE__);
		return 1;
	}
	
	
	g_VideoEditorHandle->removeBitmap(strPath);
	env->ReleaseStringUTFChars(path, strPath);
	return 0;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(updateRenderInfo)(JNIEnv * env, jobject obj, jint clipid, jint start_left, jint start_top, jint start_right, jint start_bottom, jint end_left, jint end_top, jint end_right, jint end_bottom, jint face_detected, jint face_left, jint face_top, jint face_right, jint face_bottom)
{

	LOGI("[nexEDitor_jni.cpp %d] updateRenderInfo %d", __LINE__, clipid);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	LOGE("[nexEditor_jni.cpp %d] updateRenderInfo FaceRect:%d %d %d %d", __LINE__, face_left, face_top, face_right, face_bottom);

	RECT start = {start_left, start_top, start_right, start_bottom};
	RECT end  = {end_left, end_top, end_right, end_bottom};
	RECT face = {face_left, face_top, face_right, face_bottom};

	return g_VideoEditorHandle->updateRenderInfo(clipid, face_detected, start, end, face);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(resetFaceDetectInfo)(JNIEnv * env, jobject obj, jint clipid)
{
	LOGI("[nexEDitor_jni.cpp %d] resetFaceDetectInfo %d", __LINE__, clipid);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->resetFaceDetectInfo(clipid);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setBrightness)(JNIEnv * env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setBrightness %d", __LINE__,  value);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setBrightness(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setContrast)(JNIEnv * env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setContrast %d", __LINE__,  value);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setContrast(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setSaturation)(JNIEnv * env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setSaturation %d", __LINE__,  value);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setSaturation(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getBrightness)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getBrightness", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getBrightness();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getContrast)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getContrast", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getContrast();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getSaturation)(JNIEnv * env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getSaturation", __LINE__);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getSaturation();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setDeviceLightLevel)(JNIEnv * env, jobject obj, jint lightLevel)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	g_VideoEditorHandle->setDeviceLightLevel(lightLevel);
	return 1;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setDeviceGamma)(JNIEnv * env, jobject obj, jfloat gamma)
{
	if (!g_VideoEditorHandle){
		return -1;
	}
	g_VideoEditorHandle->setDeviceGamma(gamma);
	return 1;
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setForceRTT)(JNIEnv * env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setForceRTT %d", __LINE__,  value);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setForceRTT(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setThumbnailRoutine)(JNIEnv * env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setThumbnailRoutin %d", __LINE__,  value);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setThumbnailRoutine(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setVignette)(JNIEnv *env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setVignette %d", __LINE__, value);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setVignette(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getVignette)(JNIEnv *env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getVignette", __LINE__);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getVignette();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setVignetteRange)(JNIEnv *env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setVignetteRange %d", __LINE__, value);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setVignetteRange(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getVignetteRange)(JNIEnv *env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getVignetteRange", __LINE__);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getVignetteRange();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setSharpness)(JNIEnv *env, jobject obj, jint value)
{
	LOGI("[nexEDitor_jni.cpp %d] setSharpness %d", __LINE__, value);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setSharpness(value);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(getSharpness)(JNIEnv *env, jobject obj)
{
	LOGI("[nexEDitor_jni.cpp %d] getSharpness", __LINE__);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->getSharpness();
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setGIFMode)(JNIEnv *env, jobject obj, jint mode)
{
	LOGI("[nexEDitor_jni.cpp %d] setGIFMode(%d)", __LINE__, mode);
	if (g_VideoEditorHandle == NULL)
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 1;
	}

	return g_VideoEditorHandle->setGIFMode(mode);
}

JNIEXPORT jint JNICALL _EDITOR_JNI_(setEncInfo)(JNIEnv * env, jobject obj,  jobjectArray info)
{
	const int strCnt = env->GetArrayLength(info);
	const char* pInfo[strCnt];
	jstring jstr[strCnt];

	if ( g_VideoEditorHandle == NULL ) {
		LOGI("[nexEDitor_jni.cpp %d] Video editor is not created!", __LINE__);
		return 1;
	}

	LOGI("[nexEDitor_jni.cpp %d] setEncInfo(%d)", __LINE__, strCnt);
	for (int i=0; i<strCnt; i++) {
		jstr[i] = (jstring) (env->GetObjectArrayElement(info, i));
		pInfo[i] = env->GetStringUTFChars(jstr[i], 0);
	}

	g_VideoEditorHandle->setEncInfo(strCnt, pInfo);

	for ( int i=0 ; i<strCnt ; i++ ) {
		env->ReleaseStringUTFChars(jstr[i], pInfo[i]);
	}

	return 0;
}

extern long PerformanceChecker_GetErrorCount(int);
JNIEXPORT jlong JNICALL _EDITOR_JNI_(getPerformanceCount)(JNIEnv *env, jobject obj, jint type)
{
	LOGI("[nexEDitor_jni.cpp %d] getPerformanceCount(%d)", __LINE__, type);
	if( g_VideoEditorHandle == NULL )
	{
		LOGI("[nexEDitor_jni.cpp %d] Invalid VideoEditor handle", __LINE__);
		return 0;
	}
	return PerformanceChecker_GetErrorCount((int)type);
}


// in order to print so file's version
class _just_for_internal_print_version_nexstreaming17_
{
public:
	_just_for_internal_print_version_nexstreaming17_() {
		 
		// __android_log_print(ANDROID_LOG_INFO, "NEXEDITOR", "this so file version is (%d,%d,%d)", NEXEDITOR_MAJOR_VERSION, NEXEDITOR_MINOR_VERSION, NEXEDITOR_PATCH_VERSION);
		LOGE("this so(eric:1) file version is (%d,%d,%d)", NEXEDITOR_MAJOR_VERSION, NEXEDITOR_MINOR_VERSION, NEXEDITOR_PATCH_VERSION);
	}

};

_just_for_internal_print_version_nexstreaming17_ _temp_internal_only_variable_;

