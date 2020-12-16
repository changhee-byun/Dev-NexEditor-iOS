/******************************************************************************
* File Name   :	NexThemeRenderer_SurfaceTexture.cpp
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

#if defined(ANDROID)
#include <jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <android/native_window_jni.h>
#endif

#undef LOG_TAG
#define  LOG_TAG    "NexThemeRenderer_SurfaceTexture"

#include "NexThemeRenderer.h"
#include "NexThemeRenderer_Internal.h"
#include "NexThemeRenderer_SurfaceTexture.h"
#include "NexTheme_Math.h"
#include "NexJNIEnvStack.h"
#include "nexTexturesLogger.h"

static NXT_Error NXT_ThemeRenderer_GLTexNameFromSurfaceTexture(NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow, GLuint *pTexName );
static NXT_Error NXT_ThemeRenderer_SurfaceTextureUpdateTexture(NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow, NXT_Matrix4f* pSurfaceMatrix );
static NXT_Error internalSetSurfaceTexture(NXT_HThemeRenderer renderer,
                                           unsigned int track_id,
                                           NXT_TextureID texture_id,
                                           int width,
                                           int height,
                                           int width_pitch,
                                           int height_pitch,
                                           ANativeWindow *pNativeWindow,
                                           unsigned int bDiscard,
                                           int convert_to_rgb_flag,
                                           int tex_id_for_customlut_a,
                                           int tex_id_for_customlut_b,
                                           int tex_id_for_customlut_power,
                                           int tex_id_for_lut,
                                           int color_transfer_function_id,
                                           float max_cll);

static jobject createSurfaceTexture(JNIEnv * env, GLuint texname, jclass jclass_WSTL )
{
    jmethodID   methodMakeSurfaceTexture;
    jobject     objSurfaceTexture;
    
    methodMakeSurfaceTexture = env->GetStaticMethodID(jclass_WSTL, "makeSurfaceTexture", "(I)Landroid/graphics/SurfaceTexture;");
    if( methodMakeSurfaceTexture == NULL )
    {
        return NULL;
    }

    objSurfaceTexture  = env->CallStaticObjectMethod(jclass_WSTL, methodMakeSurfaceTexture, (jint) texname);
    if( objSurfaceTexture == NULL )
    {
        return NULL;
    }
    
    return objSurfaceTexture;
}


static jobject createSurface(JNIEnv * env, jobject surfaceTexture )
{
	jfieldID    id;
	jobject     objSurface;
	jmethodID   classSurface_construct;
	jclass      classSurface;
    
    classSurface = env->FindClass("android/view/Surface");
	if( classSurface == NULL )
    {
		return NULL;
    }
    
    classSurface_construct = env->GetMethodID(classSurface, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
	if( classSurface_construct == NULL )
    {
        env->DeleteLocalRef(classSurface);
        return NULL;
    }
    
    objSurface = env->NewObject(classSurface, classSurface_construct, surfaceTexture);
    if( objSurface == NULL )
    {
        env->DeleteLocalRef(classSurface);
        return NULL;
    }
    
    env->DeleteLocalRef(classSurface);
    return objSurface;
}

static jobject createWrapperForSurfaceTextureListener(JNIEnv * env, jclass jclass_WSTL)
{


    jmethodID WSTL_Construct;
    WSTL_Construct = env->GetMethodID(jclass_WSTL, "<init>", "(I)V");
    if( WSTL_Construct == NULL )
    {
        LOGE("[%s %d] Failed to get WrapperForSurfaceTextureListener class's construct", __func__, __LINE__);
        return NULL;
    }

    jobject     objWSTL;
    objWSTL = env->NewObject(jclass_WSTL, WSTL_Construct, (jint) 312);
    if( objWSTL == NULL )
    {
        LOGE("[%s %d] Failed to get WrapperForSurfaceTextureListener class's construct", __func__, __LINE__);
        return NULL;
    }
    
    
    return objWSTL;
}


static void connectSurfaceTextureToListener( JNIEnv * env, jobject surfaceTexture, jclass classWSTL, jobject objWSTL )
{
    jmethodID jm_connectListener = env->GetMethodID(classWSTL, "connectListener", "(Landroid/graphics/SurfaceTexture;)V");
    if( jm_connectListener == NULL )
    {
        LOGE("[%s %d] Failed to bind method: connectListener", __func__, __LINE__);
        return;
    }

    env->CallVoidMethod(objWSTL,jm_connectListener,surfaceTexture);
}

static void disconnectSurfaceTextureToListener( JNIEnv * env, jobject surfaceTexture, jclass classWSTL, jobject objWSTL )
{
    jmethodID jm_disconnectListener = env->GetMethodID(classWSTL, "disconnectListener", "(Landroid/graphics/SurfaceTexture;)V");
    if( jm_disconnectListener == NULL )
    {
        LOGE("[%s %d] Failed to bind method: jm_disconnectListener", __func__, __LINE__);
        return;
    }

    env->CallVoidMethod(objWSTL,jm_disconnectListener,surfaceTexture);
}

static int waitForSurfaceTextureToBeAvailable(NXT_HThemeRenderer renderer, JNIEnv *env, jclass classWSTL, jobject objWSTL, int nTimeOutInMS )
{
    return env->CallIntMethod(objWSTL, renderer->methodWSTL_waitFrameAvailable, nTimeOutInMS);
}

static void surfaceUpdateTexture(NXT_HThemeRenderer renderer,JNIEnv * env, jobject surfaceTexture, NXT_Matrix4f* pSurfaceMatrix )
{
    jfloatArray matrix;

    env->CallVoidMethod(surfaceTexture, renderer->methodSurfaceTexture_updateTexImage);
    
    matrix = env->NewFloatArray(16);
    if( matrix==NULL ) {
        LOGE("[%s %d] Failed to create float array", __func__, __LINE__);
    } else {
        env->CallVoidMethod(surfaceTexture, renderer->methodSurfaceTexture_getTransformMatrix, matrix);
        env->GetFloatArrayRegion(matrix, 0, 16, pSurfaceMatrix->e);
        LOGD("[%s %d] SurfaceTexture Matrix:"
             "{%f %f %f %f} "
             "{%f %f %f %f} "
             "{%f %f %f %f} "
             "{%f %f %f %f} ", __func__, __LINE__,
             pSurfaceMatrix->e[0],  pSurfaceMatrix->e[1],  pSurfaceMatrix->e[2],  pSurfaceMatrix->e[3],
             pSurfaceMatrix->e[4],  pSurfaceMatrix->e[5],  pSurfaceMatrix->e[6],  pSurfaceMatrix->e[7],
             pSurfaceMatrix->e[8],  pSurfaceMatrix->e[9],  pSurfaceMatrix->e[10], pSurfaceMatrix->e[11],
             pSurfaceMatrix->e[12], pSurfaceMatrix->e[13], pSurfaceMatrix->e[14], pSurfaceMatrix->e[15]
             );

        env->DeleteLocalRef(matrix);
    }
}

static void releaseSurfaceTexture(JNIEnv * env, jobject surfaceTexture ) {
    
	jfieldID    id;
	jmethodID   classSurfaceTexture_release;
	jmethodID   classSurfaceTexture_finalize;
	jclass      classSurfaceTexture;
    
    classSurfaceTexture = env->FindClass("android/graphics/SurfaceTexture");
	if( classSurfaceTexture == NULL )
    {
		return;
    }
    
    classSurfaceTexture_release = env->GetMethodID(classSurfaceTexture, "release", "()V");
	if( classSurfaceTexture_release == NULL )
    {
        LOGE("[%s %d] Failed to bind method: release", __func__, __LINE__);
        env->DeleteLocalRef(classSurfaceTexture);
        return;
    }

    env->CallVoidMethod(surfaceTexture, classSurfaceTexture_release);

    /*	
    classSurfaceTexture_finalize = env->GetMethodID(classSurfaceTexture, "finalize", "()V");
	if( classSurfaceTexture_finalize == NULL )
    {
        LOGE("[%s %d] Failed to bind method: release", __func__, __LINE__);
        env->DeleteLocalRef(classSurfaceTexture);
        return;
    }	
    
    env->CallVoidMethod(surfaceTexture, classSurfaceTexture_finalize);
    */
    env->DeleteLocalRef(classSurfaceTexture);
}

NXT_Error NXT_ThemeRenderer_InitForSurfaceTexture( NXT_HThemeRenderer renderer )
{
	jclass      classSurfaceTexture;
   	jmethodID   methodSurfaceTexture_updateTexImage;
	jmethodID   methodSurfaceTexture_getTransformMatrix;
    jmethodID   methodWSTL_waitFrameAvailable;

    NexJNIEnvStack env;

    classSurfaceTexture = env->FindClass("android/graphics/SurfaceTexture");
	if( classSurfaceTexture == NULL )
		return NXT_Error_Internal;
    
    methodSurfaceTexture_updateTexImage = env->GetMethodID(classSurfaceTexture, "updateTexImage", "()V");
	if( methodSurfaceTexture_updateTexImage == NULL ) {
        LOGE("[%s %d] Failed to bind method: updateTexImage", __func__, __LINE__);
        env->DeleteLocalRef(classSurfaceTexture);
        return NXT_Error_Internal;
    }
    
    methodSurfaceTexture_getTransformMatrix = env->GetMethodID(classSurfaceTexture, "getTransformMatrix", "([F)V");
	if( methodSurfaceTexture_getTransformMatrix == NULL ) {
        LOGE("[%s %d] Failed to bind method: getTransformMatrix", __func__, __LINE__);
        env->DeleteLocalRef(classSurfaceTexture);
        return NXT_Error_Internal;
    }

    // for WSTL class
    methodWSTL_waitFrameAvailable = env->GetMethodID(renderer->jclass_wrapperforSurfaceTextureListener, "waitFrameAvailable", "(I)I");
    if ( methodWSTL_waitFrameAvailable == NULL ) {
        LOGE("[%s %d] Failed to bind method: waitFrameAvailable", __func__, __LINE__);
        env->DeleteLocalRef(classSurfaceTexture);
        return NXT_Error_Internal;
    }

    // success, let's move them to renderer
    renderer->classSurfaceTexture = reinterpret_cast<jclass>(env->NewGlobalRef(classSurfaceTexture));
    env->DeleteLocalRef(classSurfaceTexture);

    renderer->methodSurfaceTexture_updateTexImage = methodSurfaceTexture_updateTexImage;
    renderer->methodSurfaceTexture_getTransformMatrix = methodSurfaceTexture_getTransformMatrix;
    renderer->methodWSTL_waitFrameAvailable = methodWSTL_waitFrameAvailable;

    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_DeinitForSurfaceTexture( NXT_HThemeRenderer renderer )
{
    if ( renderer->classSurfaceTexture ) {
        NexJNIEnvStack env;

        env->DeleteGlobalRef(renderer->classSurfaceTexture);

        renderer->classSurfaceTexture = NULL;
        renderer->methodSurfaceTexture_updateTexImage = NULL;
        renderer->methodSurfaceTexture_getTransformMatrix = NULL;
        renderer->methodWSTL_waitFrameAvailable = NULL;
    }

    return NXT_Error_None;
}

NXT_Error NXT_ThemeRenderer_CreateSurfaceTexture( NXT_HThemeRenderer renderer, ANativeWindow** ppNativeWindow ) {

    LOGD("[%s %d] Begin NXT_ThemeRenderer_CreateSurfaceTexture", __func__, __LINE__);

    int i;
    
    NXT_Error error = NXT_Error_None;
    NXT_SurfaceTextureInfo* pSurfTexInfo = NULL;
    
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( renderer->java_vm == NULL ) {
        LOGE("[%s %d] no JVM", __func__, __LINE__);
        return NXT_Error_NoJVM;
    }
    
    for( i=0; i<NXT_SURFACE_TEXTURE_MAX; i++) {
        if( !renderer->surfaceTextures[i].bUsed ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
    }
    if( !pSurfTexInfo ) {
        LOGE("[%s %d] Too many surface textures; max of %d allowed", __func__, __LINE__, NXT_SURFACE_TEXTURE_MAX);
        return NXT_Error_TooManySurfaceTextures;
    }

    error = NXT_ThemeRenderer_AquireContext(renderer);
    if( error != NXT_Error_None )
        return error;

//	error = NXT_ThemeRenderer_CheckUploadOk(renderer);
//	if( error != NXT_Error_None ) {
//		NXT_ThemeRenderer_ReleaseContext(renderer, 0);
//		return error;
//	}

    JNIEnv *env = NULL;

    NexJNIEnvStack envStack;
    env = envStack.get();

    GLuint texname = 0;
    GL_GenTextures(1, &texname);
    CHECK_GL_ERROR();
    
    /*android::SurfaceTexture *pSurfaceTexture = new android::SurfaceTexture(texname);
    android::SurfaceTextureClient *pSurfaceTextureClient = new android::SurfaceTextureClient(pSurfaceTexture);
    pSurfaceTextureClient->incStrong(&pSurfTexInfo->hSurfaceTextureClient);
    */
    
    jobject objSurfaceTexture = createSurfaceTexture(env, texname, renderer->jclass_wrapperforSurfaceTextureListener);
    if( objSurfaceTexture==NULL ) {
        LOGE("[%s %d] Failed constructing surface texture", __func__, __LINE__);
        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
        return NXT_Error_ConstructorFail;
    }
    jobject objSurface = createSurface(env, objSurfaceTexture);
    if( objSurface==NULL ) {
        LOGE("[%s %d] Failed constructing surface", __func__, __LINE__);
        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
        return NXT_Error_ConstructorFail;
    }
    
   jobject objWSTL = createWrapperForSurfaceTextureListener(env, renderer->jclass_wrapperforSurfaceTextureListener);
   
    ANativeWindow* pNativeWindow = ANativeWindow_fromSurface(env, objSurface);
    // ANativeWindow_acquire(pNativeWindow);
    
    pSurfTexInfo->objSurfaceTexture = env->NewGlobalRef(objSurfaceTexture);
    pSurfTexInfo->objSurface = env->NewGlobalRef(objSurface);
    pSurfTexInfo->objWSTL = env->NewGlobalRef(objWSTL);
    pSurfTexInfo->pNativeWindow = pNativeWindow;
    pSurfTexInfo->surfaceTextureName = texname;
    pSurfTexInfo->bUsed = 1;
    pSurfTexInfo->matrix = NXT_Matrix4f_Identity();

    env->DeleteLocalRef(objSurfaceTexture);
    env->DeleteLocalRef(objSurface);
    env->DeleteLocalRef(objWSTL);

    connectSurfaceTextureToListener( env, pSurfTexInfo->objSurfaceTexture, renderer->jclass_wrapperforSurfaceTextureListener, pSurfTexInfo->objWSTL );

    NXT_ThemeRenderer_ReleaseContext(renderer, 0);

#ifdef FOR_TEST_MEDIACODEC_DEC
    *ppNativeWindow = (ANativeWindow *)pSurfTexInfo->objSurface;
#else
    *ppNativeWindow = pNativeWindow;
#endif
    return NXT_Error_None;
    
}

NXT_Error NXT_ThemeRenderer_DestroySurfaceTexture( NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow )
{
    LOGD("[%s %d] Begin NXT_ThemeRenderer_DestroySurfaceTexture", __func__, __LINE__);
    
    int i;
    NXT_Error error = NXT_Error_None;
    NXT_SurfaceTextureInfo* pSurfTexInfo = NULL;

    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( !pNativeWindow ) {
        LOGE("[%s %d] null pNativeWindow", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( renderer->java_vm == NULL ) {
        LOGE("[%s %d] no JVM", __func__, __LINE__);
        return NXT_Error_NoJVM;
    }

    for( i=0; i<NXT_SURFACE_TEXTURE_MAX; i++) {
#ifdef FOR_TEST_MEDIACODEC_DEC
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].objSurface/*pNativeWindow*/ == (jobject)pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#else
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].pNativeWindow == pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#endif
    }
    if( !pSurfTexInfo ) {
        LOGE("[%s %d] Native window not owned by renderer (0x%08X)", __func__, __LINE__, pNativeWindow );
        return NXT_Error_WrongOwner;
    }
    
    error = NXT_ThemeRenderer_AquireContext(renderer);
    if( error != NXT_Error_None )
        return error;
    
    JNIEnv *env = NULL;

    NexJNIEnvStack envStack;
    env = envStack.get();

    ANativeWindow_release(pSurfTexInfo->pNativeWindow);
    // ANativeWindow_release(pSurfTexInfo->pNativeWindow);

    disconnectSurfaceTextureToListener( env, pSurfTexInfo->objSurfaceTexture, renderer->jclass_wrapperforSurfaceTextureListener, pSurfTexInfo->objWSTL );
    
    releaseSurfaceTexture(env,pSurfTexInfo->objSurfaceTexture);
    env->DeleteGlobalRef(pSurfTexInfo->objSurface);
    env->DeleteGlobalRef(pSurfTexInfo->objSurfaceTexture);
    env->DeleteGlobalRef(pSurfTexInfo->objWSTL);

    GLuint texname = pSurfTexInfo->surfaceTextureName;
    renderer->keepSurfaceResource(texname);
    // renderer->findTexIDAndClear(texname);
    // GL_DeleteTextures(1, &texname);
    CHECK_GL_ERROR();
    
    pSurfTexInfo->bUsed = 0;

    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
    
    return NXT_Error_None;

}

NXT_Error NXT_ThemeRenderer_WaitForFrameToBeAvailable( NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow, int nTimeOutInMS )
{
    LOGD("[%s %d] Begin NXT_ThemeRenderer_WaitForFrameToBeAvailable", __func__, __LINE__);

    int i;
    NXT_SurfaceTextureInfo* pSurfTexInfo = NULL;
    NXT_Error error = NXT_Error_None;
    
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( renderer->java_vm == NULL ) {
        LOGE("[%s %d] no JVM", __func__, __LINE__);
        return NXT_Error_NoJVM;
    }
    
    JNIEnv *env = NULL;

    NexJNIEnvStack envStack;
    env = envStack.get();

    for( i=0; i<NXT_SURFACE_TEXTURE_MAX; i++) {
#ifdef FOR_TEST_MEDIACODEC_DEC
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].objSurface/*pNativeWindow*/ == (jobject)pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#else
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].pNativeWindow == pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#endif
    }
    if( !pSurfTexInfo ) {
        LOGE("[%s %d] Native window not owned by renderer (0x%08X)", __func__, __LINE__, pNativeWindow );
        return NXT_Error_WrongOwner;
    }

    int result = waitForSurfaceTextureToBeAvailable( renderer, env, renderer->jclass_wrapperforSurfaceTextureListener, pSurfTexInfo->objWSTL, nTimeOutInMS );
    if( result & 0x4 ) {
        error = NXT_Error_Timeout;
    }
    return error;
    
}

NXT_Error NXT_ThemeRenderer_CreateOutputSurface( NXT_HThemeRenderer renderer, ANativeWindow** ppNativeWindow )
{
    LOGD("[%s %d] Begin NXT_ThemeRenderer_CreateSurfaceTexture", __func__, __LINE__);

    int i;
    NXT_Error error = NXT_Error_None;
    
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( renderer->java_vm == NULL ) {
        LOGE("[%s %d] no JVM", __func__, __LINE__);
        return NXT_Error_NoJVM;
    }
    
    JNIEnv *env = NULL;

    NexJNIEnvStack envStack;
    env = envStack.get();

    jobject objSurfaceTexture = createSurfaceTexture(env, 0, renderer->jclass_wrapperforSurfaceTextureListener);
    if( objSurfaceTexture==NULL ) {
        LOGE("[%s %d] Failed constructing surface texture", __func__, __LINE__);
        return NXT_Error_ConstructorFail;
    }
    jobject objSurface = createSurface(env, objSurfaceTexture);
    if( objSurface==NULL ) {
        LOGE("[%s %d] Failed constructing surface", __func__, __LINE__);
        return NXT_Error_ConstructorFail;
    }
    
    ANativeWindow* pNativeWindow = ANativeWindow_fromSurface(env, objSurface);
    // ANativeWindow_acquire(pNativeWindow);
    
    renderer->outputSurfaceTexture.objSurfaceTexture = env->NewGlobalRef(objSurfaceTexture);
    renderer->outputSurfaceTexture.objSurface = env->NewGlobalRef(objSurface);
    renderer->outputSurfaceTexture.pNativeWindow = pNativeWindow;
    renderer->outputSurfaceTexture.surfaceTextureName = 0;
    renderer->outputSurfaceTexture.bUsed = 1;
    renderer->outputSurfaceTexture.matrix = NXT_Matrix4f_Identity();

    env->DeleteLocalRef(objSurfaceTexture);
    env->DeleteLocalRef(objSurface);
    
    *ppNativeWindow = pNativeWindow;
    return NXT_Error_None;
    
}

NXT_Error NXT_ThemeRenderer_DestroyOutputSurface( NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow )
{
    LOGD("[%s %d] Begin NXT_ThemeRenderer_DestroySurfaceTexture", __func__, __LINE__);
    
    int i;
    NXT_Error error = NXT_Error_None;

    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( !pNativeWindow ) {
        LOGE("[%s %d] null pNativeWindow", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( renderer->java_vm == NULL ) {
        LOGE("[%s %d] no JVM", __func__, __LINE__);
        return NXT_Error_NoJVM;
    }

    JNIEnv *env = NULL;

    NexJNIEnvStack envStack;
    env = envStack.get();

    ANativeWindow_release(renderer->outputSurfaceTexture.pNativeWindow);
    // ANativeWindow_release(pSurfTexInfo->pNativeWindow);
	
    releaseSurfaceTexture(env,renderer->outputSurfaceTexture.objSurfaceTexture);
    env->DeleteGlobalRef(renderer->outputSurfaceTexture.objSurface);
    env->DeleteGlobalRef(renderer->outputSurfaceTexture.objSurfaceTexture);
    
    renderer->outputSurfaceTexture.bUsed = 0;
    
    return NXT_Error_None;

}

NXT_Error NXT_ThemeRenderer_UnsetSurfaceTexture(NXT_HThemeRenderer renderer,
                                                NXT_TextureID texture_id)
{
    NXT_Error error = NXT_Error_None;
    LOGI("[%s %d] start", __func__, __LINE__);
    
    // Confirm renderer is valid and initialized
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        LOGE("[%s %d] Renderer is not initialized", __func__, __LINE__);
        return NXT_Error_NotInit;
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);
    
    if( tex->bUseSurfaceTexture ) {
        tex->texNameInitCount = 0;
        tex->bUseSurfaceTexture = 0;
    }
    
    return NXT_Error_None;
}

static NXT_Error NXT_ThemeRenderer_GLTexNameFromSurfaceTexture(NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow, GLuint *pTexName) {
    NXT_SurfaceTextureInfo* pSurfTexInfo = NULL;
    int i;
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( !pNativeWindow ) {
        LOGE("[%s %d] null pNativeWindow", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    for( i=0; i<NXT_SURFACE_TEXTURE_MAX; i++) {
#ifdef FOR_TEST_MEDIACODEC_DEC
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].objSurface/*pNativeWindow*/ == (jobject)pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#else
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].pNativeWindow == pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#endif
    }
    if( !pSurfTexInfo ) {
        LOGE("[%s %d] pNativeWindow not owned by renderer (0x%08X)", __func__, __LINE__, pNativeWindow );
        return NXT_Error_WrongOwner;
    }
    
    *pTexName = pSurfTexInfo->surfaceTextureName;
    return NXT_Error_None;
}

static NXT_Error NXT_ThemeRenderer_SurfaceTextureUpdateTexture(NXT_HThemeRenderer renderer, ANativeWindow* pNativeWindow, NXT_Matrix4f* pSurfaceMatrix ) {
    int i;
    NXT_SurfaceTextureInfo* pSurfTexInfo = NULL;
    if( !renderer ) {
        LOGE("[%s %d] null pointer", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( !pNativeWindow ) {
        LOGE("[%s %d] null pNativeWindow", __func__, __LINE__);
        return NXT_Error_BadParam;
    }
    if( renderer->java_vm == NULL ) {
        LOGE("[%s %d] no JVM", __func__, __LINE__);
        return NXT_Error_NoJVM;
    }
    for( i=0; i<NXT_SURFACE_TEXTURE_MAX; i++) {
#ifdef FOR_TEST_MEDIACODEC_DEC
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].objSurface/*pNativeWindow*/ == (jobject)pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#else
        if( renderer->surfaceTextures[i].bUsed && renderer->surfaceTextures[i].pNativeWindow == pNativeWindow ) {
            pSurfTexInfo = &renderer->surfaceTextures[i];
            break;
        }
#endif
    }
    if( !pSurfTexInfo ) {
        LOGE("[%s %d] pNativeWindow not owned by renderer (0x%08X)", __func__, __LINE__, pNativeWindow );
        return NXT_Error_WrongOwner;
    }

    JNIEnv *env = NULL;

    NexJNIEnvStack envStack;
    env = envStack.get();

    surfaceUpdateTexture(renderer,env,pSurfTexInfo->objSurfaceTexture, &pSurfTexInfo->matrix);
    
    if( pSurfaceMatrix )
        *pSurfaceMatrix = pSurfTexInfo->matrix;

    return NXT_Error_None;
}

void insetSurfaceMatrix( NXT_Matrix4f *pMatrix, int width, int height, int insetWidthSize, int insetHeightSize ) {
    
    NXT_Matrix4f m = *pMatrix;
    
    NXT_Matrix4f sm = NXT_Matrix4f_Identity();
    float pixelwidth = 1.0/(float)width;
    float pixelheight = 1.0/(float)height;
    float xfactor = 1.0/(1.0-pixelwidth*((float)insetWidthSize));
    float yfactor = 1.0/(1.0-pixelheight*((float)insetHeightSize));

    LOGD("[%s %d]XYFactor(%f, %f)",__func__, __LINE__, xfactor, yfactor);
    
    m.e[5] = -m.e[5];
    m.e[13] = 0.0;
    
    sm = NXT_Matrix4f_MultMatrix(NXT_Matrix4f_Translate(NXT_Vector4f(-0.5, -0.5, 0.0, 0.0)), sm);
    sm = NXT_Matrix4f_MultMatrix(NXT_Matrix4f_Scale(xfactor, yfactor, 1.0), sm);
    sm = NXT_Matrix4f_MultMatrix(NXT_Matrix4f_Translate(NXT_Vector4f(+0.5, +0.5, 0.0, 0.0)), sm);

    LOGD("[%s %d] INSET Matrix:\n"
         "I{%f %f %f %f} \n"
         "I{%f %f %f %f} \n"
         "I{%f %f %f %f} \n"
         "I{%f %f %f %f} \n", __func__, __LINE__,
         sm.e[0],  sm.e[1],  sm.e[2],  sm.e[3],
         sm.e[4],  sm.e[5],  sm.e[6],  sm.e[7],
         sm.e[8],  sm.e[9],  sm.e[10], sm.e[11],
         sm.e[12], sm.e[13], sm.e[14], sm.e[15]
         );
    
    m = NXT_Matrix4f_MultMatrix(sm, m);

    LOGD("[%s %d] RESULT INSET Matrix: \n"
         "R{%f %f %f %f} \n"
         "R{%f %f %f %f} \n"
         "R{%f %f %f %f} \n"
         "R{%f %f %f %f} \n", __func__, __LINE__,
         m.e[0],  m.e[1],  m.e[2],  m.e[3],
         m.e[4],  m.e[5],  m.e[6],  m.e[7],
         m.e[8],  m.e[9],  m.e[10], m.e[11],
         m.e[12], m.e[13], m.e[14], m.e[15]
         );
    
    *pMatrix = m;
}

NXT_Error NXT_ThemeRenderer_SetSurfaceTexture(NXT_HThemeRenderer renderer,
                                              unsigned int track_id,
                                              NXT_TextureID texture_id,
                                              int width,
                                              int height,
                                              int width_pitch,
                                              int height_pitch,
                                              ANativeWindow *pNativeWindow,
                                              int convert_to_rgb_flag,
                                              int tex_id_for_lut,
                                              int tex_id_for_customlut_a,
                                              int tex_id_for_customlut_b,
                                              int tex_id_for_customlut_power,
                                              int color_transfer_function_id,
                                              float max_cll)
{
    return internalSetSurfaceTexture(renderer,track_id,texture_id,width,height,width_pitch,height_pitch,pNativeWindow,0, convert_to_rgb_flag, tex_id_for_customlut_a, tex_id_for_customlut_b, tex_id_for_customlut_power, tex_id_for_lut, color_transfer_function_id, max_cll);
}


NXT_Error NXT_ThemeRenderer_DiscardSurfaceTexture(NXT_HThemeRenderer renderer,
                                              ANativeWindow* pNativeWindow)
{
    NXT_Error error = NXT_Error_None;
    LOGI("[%s %d] in", "DiscardSurfaceTexture", __LINE__ );
    
    // Confirm renderer is valid and initialized
    if( !renderer ) {
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        LOGE("[%s %d] Renderer is not initialized", __func__, __LINE__);
        return NXT_Error_NotInit;
    }
    
    if( renderer->bIsInternalContext ) {
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
            LOGE("[%s %d] NO EGL CONTEXT - Set SurfaceTexture to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
    }
    
    // Set active texture unit
    
    glActiveTexture(GL_TEXTURE0);
    
    GLuint texName = 0;
    
    error = NXT_ThemeRenderer_GLTexNameFromSurfaceTexture( renderer, pNativeWindow, &texName );
    if( error != NXT_Error_None )
        return error;
    
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, texName);
    CHECK_GL_ERROR();
    
    // Begin profiling
    NXT_BEGIN_PROFILE("discardSurfaceTexture");
    
    // Update the texture
    NXT_Matrix4f surfaceMatrix;
    error = NXT_ThemeRenderer_SurfaceTextureUpdateTexture(renderer, pNativeWindow, &surfaceMatrix);
    
    // Done - end profiling and return
    NXT_END_PROFILE();
    
    LOGI("[DiscardSurfaceTexture %d] done", __LINE__ );
    
    return error;
}

static NXT_Error internalSetSurfaceTexture(NXT_HThemeRenderer renderer,
                                           unsigned int track_id,
                                           NXT_TextureID texture_id,
                                           int width,
                                           int height,
                                           int width_pitch,
                                           int height_pitch,
                                           ANativeWindow *pNativeWindow,
                                           unsigned int bDiscard,
                                           int convert_to_rgb_flag,
                                           int customlut_a_id,
                                           int customlut_b_id,
                                           int customlut_power,
                                           int lut_id,
                                           int color_transfer_function_id,
                                           float max_cll)
{
    NXT_Error error = NXT_Error_None;
    
    // Confirm renderer is valid and initialized
    if( !renderer ) {
		LOGE("0x%x", renderer);
        LOGE("[%s %d] Renderer is null", __func__, __LINE__);
        return NXT_Error_MissingParam;
    }
    
    if( !renderer->bInitialized ) {
        LOGE("[%s %d] Renderer is not initialized 0x%x", __func__, __LINE__, renderer);
        return NXT_Error_NotInit;
    }
    
    // Validate parameters
    if( texture_id < 0 || texture_id >= NXT_TextureID_COUNT ) {
		LOGE("0x%x", renderer);
        LOGE("[%s %d] NXT_Error_ParamRange texture_id=%d", __func__, __LINE__, (int)texture_id);
        return NXT_Error_ParamRange;
    }
    
    if( renderer->bIsInternalContext ) {
        if( eglGetCurrentContext()==EGL_NO_CONTEXT ) {
            CHECK_EGL_ERROR();
			LOGE("0x%x", renderer);
            LOGE("[%s %d] NO EGL CONTEXT - Set SurfaceTexture to texture failed", __func__, __LINE__ );
            return NXT_Error_NoContext;
        }
    }
    
    // Get pointer to texture we will create/update
    NXT_TextureInfo *tex = &renderer->getRawVideoSrc(texture_id);

    tex->track_id_ = track_id;
    tex->custom_lut_a = customlut_a_id;
    tex->custom_lut_b = customlut_b_id;
    tex->custom_lut_power = customlut_power;
    tex->color_transfer_function_id_ = color_transfer_function_id;
    tex->max_cll_ = max_cll;
    tex->serial_++;

    NXT_ThemeRenderer_ResetTexture(renderer, texture_id,width,height);
    
    glActiveTexture(GL_TEXTURE0);
    
    error = NXT_ThemeRenderer_GLTexNameFromSurfaceTexture( renderer, pNativeWindow, &tex->texName[0] );
    if( error != NXT_Error_None )
        return error;
    tex->texNameInitCount = 1;

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex->texName[0]);
    CHECK_GL_ERROR();
    
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/ );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    CHECK_GL_ERROR();
    
    // Begin profiling
    NXT_BEGIN_PROFILE("setSurfaceTexture");
    
    // Update the texture
    error = NXT_ThemeRenderer_SurfaceTextureUpdateTexture(renderer, pNativeWindow, &tex->surfaceMatrix);
    
    // tex->surfaceMatrix = NXT_Matrix4f_Transpose(tex->surfaceMatrix);	

    // Improve inset function
#if 0 // original code    
    insetSurfaceMatrix( &tex->surfaceMatrix, width, height, -3, -3 );
#else // Improved code
    NXT_Matrix4f m = tex->surfaceMatrix;
    float tmp = m.e[3];
    m.e[3] = m.e[12];
    m.e[12] = tmp;
    m.e[7] = m.e[13] + m.e[5];
    m.e[13] = 0;
    m.e[5] = -m.e[5];
    tex->surfaceMatrix = m;
#endif
    // Done - end profiling and return
    NXT_END_PROFILE();
    
    tex->srcWidth = width;
    tex->srcHeight = height;
    tex->textureWidth = width_pitch;
    tex->textureHeight = height_pitch;
    tex->origin_textureformat_ = NXT_PixelFormat_EGLImage;
    tex->textureFormat = NXT_PixelFormat_EGLImage;
    tex->bValidTexture = 1;
    tex->bUseSurfaceTexture = 1;
    
    if( !bDiscard ) {
        renderer->last_upload_tex_id = texture_id;
    }
    
    LOGD("[%s %d] ADJUSTED SurfaceTexture Matrix:"
         "A{%f %f %f %f} "
         "A{%f %f %f %f} "
         "A{%f %f %f %f} "
         "A{%f %f %f %f} ", __func__, __LINE__,
         tex->surfaceMatrix.e[0],  tex->surfaceMatrix.e[1],  tex->surfaceMatrix.e[2],  tex->surfaceMatrix.e[3],
         tex->surfaceMatrix.e[4],  tex->surfaceMatrix.e[5],  tex->surfaceMatrix.e[6],  tex->surfaceMatrix.e[7],
         tex->surfaceMatrix.e[8],  tex->surfaceMatrix.e[9],  tex->surfaceMatrix.e[10], tex->surfaceMatrix.e[11],
         tex->surfaceMatrix.e[12], tex->surfaceMatrix.e[13], tex->surfaceMatrix.e[14], tex->surfaceMatrix.e[15]
         );

    
    LOGI("[%s %d] done -> texture_id(%d/%s) srcWidth(%d) srcHeight(%d) textureWidth(%d) textureHeight(%d) textureFormat(%d) valid(%d) usesfc(%d)", "SetSurfaceTexture", __LINE__,
         texture_id, (texture_id==NXT_TextureID_Video_1?"NXT_TextureID_Video_1":(texture_id==NXT_TextureID_Video_2?"NXT_TextureID_Video_2":"???")),
         tex->srcWidth, tex->srcHeight, tex->textureWidth, tex->textureHeight, tex->textureFormat, tex->bValidTexture, tex->bUseSurfaceTexture );
    //Jeff
    //Jeff
    if (convert_to_rgb_flag || lut_id || customlut_a_id > 0 || customlut_b_id > 0 || color_transfer_function_id > 0){

        int custom_lut_a_texture = 0;
        int custom_lut_b_texture = 0;

        if(customlut_a_id > 0){

            custom_lut_a_texture = NXT_ThemeRenderer_GetLUTTexture(renderer, customlut_a_id);
            if(custom_lut_a_texture == 0){

                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, customlut_a_id);
                if(tex_id != 0)
                    custom_lut_a_texture = tex_id;
            }
        }

        if(customlut_b_id > 0){

            custom_lut_b_texture = NXT_ThemeRenderer_GetLUTTexture(renderer, customlut_b_id);
            if(custom_lut_b_texture == 0){

                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, customlut_b_id);
                if(tex_id != 0)
                    custom_lut_b_texture = tex_id;
            }
        }

        int tex_id_for_lut = 0;
        if(lut_id != 0){
            LOGI("[%s %d]", __func__, __LINE__);
           tex_id_for_lut = NXT_ThemeRenderer_GetLUTTexture(renderer, lut_id);
           LOGI("[%s %d]", __func__, __LINE__);
           if(tex_id_for_lut == 0){

                int tex_id = NXT_ThemeRenderer_GetLutTexWithID(renderer, lut_id);
                if(tex_id != 0)
                    tex_id_for_lut = NXT_ThemeRenderer_SetLUTTexture(renderer, lut_id, tex_id);
           }
        }
        /*
        if(texture_id == NXT_TextureID_Video_1 || texture_id == NXT_TextureID_Video_2){

            if(lut_id || renderitem_effect_id > -1)
                NXT_ThemeRenderer_ConvertTextureToRGB(renderer, texture_id, tex_id_for_lut, renderitem_effect_id, progress);
        }
        else
        */
        NXT_ThemeRenderer_ConvertTextureToRGB(renderer, tex, custom_lut_a_texture, custom_lut_b_texture, customlut_power, tex_id_for_lut, NULL, color_transfer_function_id, max_cll);        
    }
    return error;
    
}
