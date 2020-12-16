#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#define NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexTheme.h"
#undef NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexThemeRenderer.h"
#include <pthread.h>
#include <android/native_window_jni.h>

#define  LOG_TAG    "NXVidEdTest"

#if 0
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#else
#define  LOGI(...)  
#endif

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C" {
#ifdef FOR_PROJECT_Kinemaster
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_init(JNIEnv * env, jobject obj, jobject bitmapManager);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_prepareSurface(JNIEnv * env, jobject obj, jobject surface);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_surfaceChange(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_render(JNIEnv * env, jobject obj );
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setCTS(JNIEnv * env, jobject obj, jint cts);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_deinit(JNIEnv * env, jobject obj, jboolean isDetachedContext);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_loadThemes(JNIEnv * env, jobject obj, jstring themeBody);
    JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearRenderItems(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_loadRenderItem(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData);

//    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setEffect(JNIEnv * env, jobject obj, jstring effectName, jstring effectOptions);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setTransitionEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint effectStartTime, jint effectEndTime );
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setClipEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint clipStartTime, jint clipEndTime, jint effectStartTime, jint effectEndTime );
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearTransitionEffect(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearClipEffect(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setPlaceholders(JNIEnv * env, jobject obj, jstring primary, jstring secondary);
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_recycleRenderer(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_releaseContext(JNIEnv * env, jobject obj, jboolean swapBuffers );
    JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_aquireContext(JNIEnv * env, jobject obj );
    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearSurface(JNIEnv * env, jobject obj, jint tag );

    JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setForceRTT(JNIEnv *env, jobject obj, jint val);
#else
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_init(JNIEnv * env, jobject obj, jobject bitmapManager);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_prepareSurface(JNIEnv * env, jobject obj, jobject surface);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_surfaceChange(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_render(JNIEnv * env, jobject obj );
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setCTS(JNIEnv * env, jobject obj, jint cts);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_deinit(JNIEnv * env, jobject obj, jboolean isDetachedContext);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_loadThemes(JNIEnv * env, jobject obj, jstring themeBody);
    JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearRenderItems(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_loadRenderItem(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData);

//    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setEffect(JNIEnv * env, jobject obj, jstring effectName, jstring effectOptions);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setTransitionEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint effectStartTime, jint effectEndTime );
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setClipEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint clipStartTime, jint clipEndTime, jint effectStartTime, jint effectEndTime );
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearTransitionEffect(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearClipEffect(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setPlaceholders(JNIEnv * env, jobject obj, jstring primary, jstring secondary);
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_recycleRenderer(JNIEnv * env, jobject obj);
    JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_releaseContext(JNIEnv * env, jobject obj, jboolean swapBuffers );
    JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_aquireContext(JNIEnv * env, jobject obj );
    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearSurface(JNIEnv * env, jobject obj, jint tag );

    JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setForceRTT(JNIEnv *env, jobject obj, jint val);
#endif


};



typedef struct JNIThemeInstanceData_ {
//    NXT_HThemeSet  hThemeSet;
//    NXT_HTheme hTheme;
//    NXT_HEffect hEffect;
    NXT_HThemeRenderer_Context hContext;
    NXT_HThemeRenderer hRenderer;
    jobject bitmapManager;
    char *placeholder1;
    char *placeholder2;
    JNIEnv *pEnv;
} JNIThemeInstanceData;

//#define TEXBUF_MAX 2
//static TexBuf g_texbuf[TEXBUF_MAX] = {NXT_TextureID_Video_1, NXT_PixelFormat_Luminance, 0};

static int loadThemeFileCallback(char** ppOutputData, int* pLength, char* path, void* cbdata);
static int loadRenderItemThemeFileCallback(char** ppFileData, int* pLength, char* path, void* cbdata);
static NXT_Error loadImageCB( NXT_ImageInfo* pinfo, char* path, int asyncmode, void* cbdata );
static NXT_Error freeImageCB( NXT_ImageInfo* pinfo, void* cbdata );

static void setThemeInstanceHandle( JNIEnv *env, jobject obj, jlong handle ) {
    LOGI("[%s %d] setting 0x%08X", __func__, __LINE__, (unsigned int)handle);
    jclass cls = env->GetObjectClass(obj);
    LOGI("[%s %d] cls=0x%08X", __func__, __LINE__, (unsigned int)cls);
    jmethodID mid = env->GetMethodID(cls, "setThemeInstanceHandle","(J)V");
    LOGI("[%s %d] mid=0x%08X", __func__, __LINE__, (unsigned int)mid);
    env->CallVoidMethod(obj, mid, handle);
    LOGI("[%s %d] done setting 0x%08X", __func__, __LINE__, (unsigned int)handle);
}

static jlong getThemeInstanceHandle( JNIEnv *env, jobject obj ) {
    LOGI("[%s %d]", __func__, __LINE__);
    jclass cls = env->GetObjectClass(obj);
    LOGI("[%s %d] cls=0x%08X", __func__, __LINE__, (unsigned int)cls);
    jmethodID mid = env->GetMethodID(cls, "getThemeInstanceHandle","()J");
    LOGI("[%s %d] mid=0x%08X", __func__, __LINE__, (unsigned int)mid);
    jlong instanceHandle = env->CallLongMethod(obj, mid);
    LOGI("[%s %d] returning 0x%08X", __func__, __LINE__, (unsigned int)instanceHandle);
    return instanceHandle;
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_init(JNIEnv * env, jobject obj, jobject bitmapManager)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_init(JNIEnv * env, jobject obj, jobject bitmapManager)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Instance data null; need to create", __func__, __LINE__);
        th = (JNIThemeInstanceData*)malloc( sizeof(JNIThemeInstanceData) );
        memset(th, 0, sizeof(JNIThemeInstanceData) );
        setThemeInstanceHandle( env, obj, (jlong)th );
    }
    
    LOGI("[%s %d] (@0)", __func__, __LINE__);
    
    if( th->bitmapManager ) {
        LOGI("[%s %d] need to free old bitmap manager", __func__, __LINE__);
        env->DeleteGlobalRef(th->bitmapManager);
        th->bitmapManager = NULL;
    }
    LOGI("[%s %d] (@1)", __func__, __LINE__);
    th->bitmapManager = env->NewGlobalRef(bitmapManager);
    LOGI("[%s %d] END", __func__, __LINE__);
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_loadThemes(JNIEnv * env, jobject obj, jstring themeBody) 
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_loadThemes(JNIEnv * env, jobject obj, jstring themeBody) 
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        return;
    }
    
    const char *str;
    str = env->GetStringUTFChars(themeBody, NULL);
    if (str == NULL) {
        return;
    }

    if( th->hRenderer ) {
		NXT_ThemeRenderer_LoadThemesAndEffects(th->hRenderer, str, LOAD_THEME_PREIVEW);
    }
    env->ReleaseStringUTFChars(themeBody, str);
    LOGI("[%s %d] END", __func__, __LINE__);
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearRenderItems(JNIEnv * env, jobject obj)
#else
JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearRenderItems(JNIEnv * env, jobject obj)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th) {
        return 1;
    }

    NXT_ThemeRenderer_AquireContext(th->hRenderer);
    NXT_ThemeRenderer_ClearRenderItems(th->hRenderer);
    NXT_ThemeRenderer_ReleaseContext(th->hRenderer, 0);

    return 0;
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_loadRenderItem(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData)
#else
JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_loadRenderItem(JNIEnv * env, jobject obj, jstring strEffectID, jstring strEffectData)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th || !strEffectID ) {
        return 1;
    }

    const char *strData = env->GetStringUTFChars(strEffectData, NULL);
    if( strData == NULL )
    {
        LOGI("[nexTheme_jni.cpp %d] Invalid Theme Resource file", __LINE__);
        return 1;
    }

    const char *strID = env->GetStringUTFChars(strEffectID, NULL);
    if( strID == NULL )
    {
        LOGI("[nexTheme_jni.cpp %d] Invalid Theme Resource file", __LINE__);
        env->ReleaseStringUTFChars(strEffectData, strData);     
        return 1;
    }

    NXT_ThemeRenderer_AquireContext(th->hRenderer);
    //NXT_ThemeRenderer_GetRenderItem(th->hRenderer, strID, strData, 0, loadThemeFileCallback, th);
    NXT_ThemeRenderer_GetRenderItem(th->hRenderer, strID, NULL, strData, 0, loadRenderItemThemeFileCallback, th);
    
    NXT_ThemeRenderer_ReleaseContext(th->hRenderer, 0);

    env->ReleaseStringUTFChars(strEffectData, strData);
    env->ReleaseStringUTFChars(strEffectID, strID);

    return 0;
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setPlaceholders(JNIEnv * env, jobject obj, jstring primary, jstring secondary) 
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setPlaceholders(JNIEnv * env, jobject obj, jstring primary, jstring secondary) 
#endif
{
    
    LOGI("[%s %d] START", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        return;
    }
    
    const char *szPrimary = env->GetStringUTFChars(primary, NULL);
    const char *szSecondary = env->GetStringUTFChars(secondary, NULL);
    LOGI("[%s %d] szPrimary='%s'; szSecondary='%s'", __func__, __LINE__, szPrimary, szSecondary);

    if( th->placeholder1 ) {
        free(th->placeholder1);
        th->placeholder1 = 0;
    }
    
    if( th->placeholder2 ) {
        free(th->placeholder2);
        th->placeholder2 = 0;
    }
    
    if( szPrimary ) {
        th->placeholder1 = (char*)malloc(strlen(szPrimary)+1);
        strcpy( th->placeholder1, szPrimary );
    }
    
    if( szSecondary ) {
        th->placeholder2 = (char*)malloc(strlen(szSecondary)+1);
        strcpy( th->placeholder2, szSecondary );
    }
    
    NXT_ThemeRenderer_SetVideoPlaceholders(th->hRenderer, th->placeholder1, th->placeholder2 );

    if( szPrimary )
        env->ReleaseStringUTFChars(primary, szPrimary);
    
    if( szSecondary )
        env->ReleaseStringUTFChars(secondary, szSecondary);
    LOGI("[%s %d] END", __func__, __LINE__);
    
}


#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setTransitionEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint effectStartTime, jint effectEndTime )
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setTransitionEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint effectStartTime, jint effectEndTime )
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th || !effectId ) {
        return;
    }
    
    const char *pEffectId = effectId == NULL ? NULL : env->GetStringUTFChars(effectId, NULL);
    const char *pEffectOptions = effectOptions == NULL ? NULL : env->GetStringUTFChars(effectOptions, NULL);

    NXT_ThemeRenderer_SetTransitionEffect(th->hRenderer,pEffectId,pEffectOptions,clipIndex,totalClipCount,effectStartTime,effectEndTime);
    
    if( pEffectId ) {
        env->ReleaseStringUTFChars(effectId, pEffectId);
    }
    
    if( pEffectOptions ) {
        env->ReleaseStringUTFChars(effectOptions, pEffectOptions);
    }
    
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setClipEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint clipStartTime, jint clipEndTime, jint effectStartTime, jint effectEndTime )
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setClipEffect(JNIEnv * env, jobject obj, jstring effectId, jstring effectOptions, jint clipIndex, jint totalClipCount, jint clipStartTime, jint clipEndTime, jint effectStartTime, jint effectEndTime )
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th || !effectId ) {
        return;
    }

    const char *pEffectId = effectId == NULL ? NULL : env->GetStringUTFChars(effectId, NULL);
    const char *pEffectOptions = effectOptions == NULL ? NULL : env->GetStringUTFChars(effectOptions, NULL);

    NXT_ThemeRenderer_SetClipEffect(th->hRenderer, pEffectId, pEffectOptions, clipIndex, totalClipCount, clipStartTime, clipEndTime, effectStartTime, effectEndTime );
    
    if( pEffectId ) {
        env->ReleaseStringUTFChars(effectId, pEffectId);
    }
    
    if( pEffectOptions ) {
        env->ReleaseStringUTFChars(effectOptions, pEffectOptions);
    }

}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearTransitionEffect(JNIEnv * env, jobject obj)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearTransitionEffect(JNIEnv * env, jobject obj)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        return;
    }
    
    NXT_ThemeRenderer_ClearTransitionEffect(th->hRenderer);
    
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearClipEffect(JNIEnv * env, jobject obj)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearClipEffect(JNIEnv * env, jobject obj)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        return;
    }
    
    NXT_ThemeRenderer_ClearClipEffect(th->hRenderer);
    
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setEffect(JNIEnv * env, jobject obj, jstring effectName, jstring effectOptions)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setEffect(JNIEnv * env, jobject obj, jstring effectName, jstring effectOptions)
#endif
{
    LOGI("[%s %d] NexThemeRenderer_setEffect(0x%x)", __func__, __LINE__, effectName);
    
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !effectName || !th ) {
        return;
    }
    
    const char *str;
    str = env->GetStringUTFChars(effectOptions, NULL);
    if (str == NULL) {
        return;
    }
	
    //	NXT_ThemeRenderer_SetEffectOptions( th->hRenderer, str  );
    
    env->ReleaseStringUTFChars(effectOptions, str);
    LOGI("[%s %d] END", __func__, __LINE__);


//    const char *str;
    str = env->GetStringUTFChars(effectName, NULL);
    if (str == NULL) {
        return;
    }
    
//    NXT_ThemeRenderer_Set
	
//	NXT_ThemeRenderer_SetEffect( th->hRenderer, str, 0, 0);

	
    /*unsigned int numEffects = NXT_Theme_GetEffectCount( th->hTheme );
    for( unsigned int i=0; i<numEffects; i++ ) {
        NXT_HEffect hEffect = NXT_Theme_GetEffect( th->hTheme,i );
        char *effect_id = NXT_Effect_GetID(hEffect);
        if( effect_id && strcmp(effect_id,str)==0 ) {
            th->hEffect = hEffect;
            if( th->hRenderer ) {
                NXT_ThemeRenderer_SetEffect( th->hRenderer, th->hEffect, 0, 0);
            }
            break;
        }
    }*/
    
    env->ReleaseStringUTFChars(effectName, str);
    LOGI("[%s %d] END", __func__, __LINE__);
}

#include <NEXVIDEOEDITOR_SupportDevices.h>

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_prepareSurface(JNIEnv * env, jobject obj, jobject surface)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_prepareSurface(JNIEnv * env, jobject obj, jobject surface)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Unable to process surface change; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return;
    }
    
    ANativeWindow* nativeWindow = NULL;
    if( surface != NULL )
    {
        nativeWindow = ANativeWindow_fromSurface(env, surface);
    }

    if( !th->hRenderer ) {
        LOGI("[%s %d] Making renderer", __func__, __LINE__ );
        th->hContext = NXT_ThemeRendererContext_Create(DEFAULT_MAX_TEXTURE_CACHE_SIZE, FALSE, 0);
        th->hRenderer = NXT_ThemeRenderer_Create(NXT_RendererType_ExternalContext, NULL, th->hContext);
        if(isSDM660Device())
            NXT_ThemeRenderer_SetForceRTT(th->hRenderer, TRUE);
        LOGI("[%s %d] Made renderer; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        NXT_ThemeRenderer_RegisterImageCallbacks(th->hRenderer,
                                                 loadImageCB,
                                                 freeImageCB,
                                                 loadRenderItemThemeFileCallback,
                                                 (void*)th);

        th->pEnv = env;
        if( th->placeholder1 || th->placeholder2 ) {
            NXT_ThemeRenderer_SetVideoPlaceholders(th->hRenderer, th->placeholder1, th->placeholder2 );
            LOGI("[%s %d] SetVideoPlaceholders", __func__, __LINE__);
        }
    }
    int result = NXT_ThemeRenderer_SetNativeWindow( th->hRenderer, nativeWindow, 0, 0);
    LOGI("[%s %d] Result %d", __func__, __LINE__, result);

}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_surfaceChange(JNIEnv * env, jobject obj,  jint width, jint height)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_surfaceChange(JNIEnv * env, jobject obj,  jint width, jint height)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Unable to process surface change; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return;
    }
    
    if( !th->hRenderer ) {
        LOGI("[%s %d] Making renderer", __func__, __LINE__ );
        th->hContext = NXT_ThemeRendererContext_Create(DEFAULT_MAX_TEXTURE_CACHE_SIZE, FALSE, 0);
        th->hRenderer = NXT_ThemeRenderer_Create(NXT_RendererType_ExternalContext, NULL, th->hContext);
        if(isSDM660Device())
            NXT_ThemeRenderer_SetForceRTT(th->hRenderer, TRUE);
        LOGI("[%s %d] Made renderer; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        NXT_ThemeRenderer_RegisterImageCallbacks(th->hRenderer,
                                                 loadImageCB,
                                                 freeImageCB,
                                                 loadRenderItemThemeFileCallback,
                                                 (void*)th);

        th->pEnv = env;
        
//        if( th->hEffect ) {
//            NXT_ThemeRenderer_SetEffect( th->hRenderer, th->hEffect, 0, 0);
//            LOGI("[%s %d] SetEffect", __func__, __LINE__);
//        }
        if( th->placeholder1 || th->placeholder2 ) {
            NXT_ThemeRenderer_SetVideoPlaceholders(th->hRenderer, th->placeholder1, th->placeholder2 );
            LOGI("[%s %d] SetVideoPlaceholders", __func__, __LINE__);
        }
    }
    NXT_ThemeRenderer_SurfaceChanged( th->hRenderer, width, height );
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setCTS(JNIEnv * env, jobject obj, jint cts)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setCTS(JNIEnv * env, jobject obj, jint cts)
#endif
{
    LOGI("[%s %d] cts=%d", __func__, __LINE__, cts);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Unable to set cts; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return;
    }
    if( !th->hRenderer) {
        LOGI("[%s %d] Unable to set cts; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        return;
    }
    
    NXT_ThemeRenderer_SetCTS( th->hRenderer, cts );
    
    LOGI("[%s %d] END", __func__, __LINE__);
}


#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_render(JNIEnv * env, jobject obj )
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_render(JNIEnv * env, jobject obj )
#endif
{
    LOGI("[%s %d] IN ", __func__, __LINE__ );
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Unable to render; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return;
    }
    if( !th->hRenderer) {
        LOGI("[%s %d] Unable to render; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        return;
    }
    
//    NXT_ThemeRenderer_SetCTS( th->hRenderer, cts );
//    NXT_ThemeRenderer_SetTime( th->hRenderer, progress );
//    LOGI("[%s %d] completed SetTime", __func__, __LINE__);
    
    NXT_ThemeRenderer_GLDraw( th->hRenderer, NXT_RendererOutputType_RGBA_8888, 1 );
    LOGI("[%s %d] END", __func__, __LINE__);
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_clearSurface(JNIEnv * env, jobject obj, jint tag )
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_clearSurface(JNIEnv * env, jobject obj, jint tag )
#endif
{
    LOGI("[%s %d] IN ", __func__, __LINE__ );
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Failed; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return;
    }
    if( !th->hRenderer) {
        LOGI("[%s %d] Failed; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        return;
    }

    NXT_ThemeRenderer_ClearScreen(th->hRenderer, tag);
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_aquireContext(JNIEnv * env, jobject obj )
#else
JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_aquireContext(JNIEnv * env, jobject obj )
#endif
{
    LOGI("[%s %d] IN ", __func__, __LINE__ );
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Failed; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return NXT_Error_Internal;
    }
    if( !th->hRenderer) {
        LOGI("[%s %d] Failed; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        return NXT_Error_Internal;
    }
    
    NXT_Error result = NXT_ThemeRenderer_AquireContext(th->hRenderer);
    LOGI("[%s %d] END", __func__, __LINE__);
    return result;
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT jint JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_releaseContext(JNIEnv * env, jobject obj, jboolean swapBuffers )
#else
JNIEXPORT jint JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_releaseContext(JNIEnv * env, jobject obj, jboolean swapBuffers )
#endif
{
    LOGI("[%s %d] IN ", __func__, __LINE__ );
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th ) {
        LOGI("[%s %d] Failed; th=0x%08X", __func__, __LINE__, (unsigned int)th );
        return NXT_Error_Internal;
    }
    if( !th->hRenderer) {
        LOGI("[%s %d] Failed; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer );
        return NXT_Error_Internal;
    }
    
    NXT_Error result = NXT_ThemeRenderer_ReleaseContext(th->hRenderer,swapBuffers?1:0);
    LOGI("[%s %d] END", __func__, __LINE__);
    return result;
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_recycleRenderer(JNIEnv * env, jobject obj) 
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_recycleRenderer(JNIEnv * env, jobject obj) 
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th )
        return;
    if( th->hRenderer ) {
        NXT_ThemeRenderer_Destroy( th->hRenderer, 0 );
        NXT_ThemeRendererContext_Destroy(th->hContext);
        th->hRenderer = NULL;
        LOGI("[%s %d] Recycled renderer", __func__, __LINE__);
    }    
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_deinit(JNIEnv * env, jobject obj, jboolean isDetachedContext)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_deinit(JNIEnv * env, jobject obj, jboolean isDetachedContext)
#endif
{
    LOGI("[%s %d]", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)getThemeInstanceHandle(env, obj);
    if( !th )
        return;
    
    if( th->placeholder1 ) {
        free(th->placeholder1);
        th->placeholder1 = 0;
    }
    
    if( th->placeholder2 ) {
        free(th->placeholder2);
        th->placeholder2 = 0;
    }

    if( th->hRenderer ) {
        NXT_ThemeRenderer_Destroy( th->hRenderer, isDetachedContext );
        NXT_ThemeRendererContext_Destroy(th->hContext);
        th->hRenderer = NULL;
    }
    if( th->bitmapManager ) {
        env->DeleteGlobalRef(th->bitmapManager);
        th->bitmapManager = NULL;
    }
}

#ifdef FOR_PROJECT_Kinemaster
JNIEXPORT void JNICALL Java_com_nextreaming_nexvideoeditor_NexThemeRenderer_setForceRTT(JNIEnv *env, jobject obj, jint val)
#else
JNIEXPORT void JNICALL Java_com_nexstreaming_kminternal_nexvideoeditor_NexThemeRenderer_setForceRTT(JNIEnv *env, jobject obj, jint val)
#endif
{
    LOGI("[%s %d] IN ", __func__, __LINE__);
    JNIThemeInstanceData *th = (JNIThemeInstanceData *)getThemeInstanceHandle(env, obj);
    if (!th)
    {
        LOGI("[%s %d] Failed; th=0x%08X", __func__, __LINE__, (unsigned int)th);
        return;
    }
    if (!th->hRenderer)
    {
        LOGI("[%s %d] Failed; th->hRenderer=0x%08X", __func__, __LINE__, (unsigned int)th->hRenderer);
        return;
    }

    NXT_ThemeRenderer_SetForceRTT(th->hRenderer, val);
}

static int loadThemeFileCallback(char** ppFileData, int* pLength, char* path, void* cbdata)
{
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)cbdata;
    JNIEnv *env = th->pEnv;
    
    jclass cls = env->GetObjectClass(th->bitmapManager);
    jmethodID mid;
    
    mid = env->GetMethodID(cls, "openThemeFile", "(Ljava/lang/String;)[B");

    *ppFileData = NULL;

    LOGI("[nexTheme_jni.cpp  %d] openThemeFile", __LINE__);
    
    jstring strPath = env->NewStringUTF(path);
    if( strPath == NULL ){

        return 0;
    }

    jbyteArray arr = (jbyteArray)env->CallObjectMethod(th->bitmapManager, mid, strPath);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile arr:0x%x", __LINE__, arr);
    jboolean isCopy = 0;
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);
    jbyte* contents = env->GetByteArrayElements(arr, &isCopy);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile contents:0x%x isCopy:%d", __LINE__, contents, isCopy);
    *pLength = env->GetArrayLength(arr);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile length:%d", __LINE__, *pLength);
    *ppFileData = new char[*pLength + 1];
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);
    memcpy(*ppFileData, contents, *pLength);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);
    (*ppFileData)[*pLength] = 0;
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);
    env->ReleaseByteArrayElements(arr, contents, 0);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);
    env->DeleteLocalRef(arr);
    LOGI("[nexTheme_jni.cpp  %d] callbackGetThemeFile", __LINE__);

    return 0;   
}

static int loadRenderItemThemeFileCallback(char** ppFileData, int* pLength, char* path, void* cbdata)
{
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)cbdata;
    JNIEnv *env = th->pEnv;
    
    jclass cls = env->GetObjectClass(th->bitmapManager);
    jmethodID mid;
    
    mid = env->GetMethodID(cls, "callbackReadAssetItemFile", "(Ljava/lang/String;Ljava/lang/String;)[B");

    *ppFileData = NULL;

    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback (%s)", __LINE__,path);

    char szId[1024];
    char szPath[1024];
    char *p = strchr(path,'/');
    if( p != NULL ){
        size_t len = p-path;
        strncpy(szId,path,len);
        szId[len] = 0x00;
        p++;
        strcpy(szPath, p );
    }else{

        strcpy(szId, path);
        szPath[0] = 0;
    }
    
    jstring strID = env->NewStringUTF(szId);
    if( strID == NULL ){
        return 0;
    }
    
    jstring strPath = env->NewStringUTF(szPath);
    if( strPath == NULL ){

        return 0;
    }


    jbyteArray arr = (jbyteArray)env->CallObjectMethod(th->bitmapManager, mid, strID,strPath);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback arr:0x%x", __LINE__, arr);
    jboolean isCopy = 0;
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);
    jbyte* contents = env->GetByteArrayElements(arr, &isCopy);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback contents:0x%x isCopy:%d", __LINE__, contents, isCopy);
    *pLength = env->GetArrayLength(arr);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback length:%d", __LINE__, *pLength);
    *ppFileData = new char[*pLength + 1];
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);
    memcpy(*ppFileData, contents, *pLength);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);
    (*ppFileData)[*pLength] = 0;
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);
    env->ReleaseByteArrayElements(arr, contents, 0);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);
    env->DeleteLocalRef(arr);
    LOGI("[nexTheme_jni.cpp  %d] loadRenderItemThemeFileCallback", __LINE__);

    return 0;   
}

static void swapRB(unsigned char* pbuf, int width, int height){

	#if defined(ANDROID)
		for(int i = 0; i < width*height; ++i){

			unsigned char r = pbuf[i*4+0];
			pbuf[i*4+0] = pbuf[i*4+2];
			pbuf[i*4+2] = r;
		}
	#endif
}

static NXT_Error loadImageCB( NXT_ImageInfo* pinfo, char* path, int asyncmode, void* cbdata ) {
    
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)cbdata;
    JNIEnv *env = th->pEnv;
    
    jclass cls = env->GetObjectClass(th->bitmapManager);
    jmethodID mid;
    
    /* Ask the image manager for a bitmap */
#ifdef FOR_PROJECT_Kinemaster
    mid = env->GetMethodID(cls, "openThemeImage", "(Ljava/lang/String;)Lcom/nextreaming/nexvideoeditor/NexImage;");
#else
    mid = env->GetMethodID(cls, "openThemeImage", "(Ljava/lang/String;)Lcom/nexstreaming/kminternal/nexvideoeditor/NexImage;");
#endif
    
    jstring name = env->NewStringUTF(path);
    jobject loadedBitmap = env->CallObjectMethod(th->bitmapManager, mid, name);
    env->DeleteLocalRef(name);
    
    jclass bmclass = env->GetObjectClass(loadedBitmap);
    
    /* Get image dimensions */
    mid = env->GetMethodID(bmclass, "getWidth", "()I");
    int width = env->CallIntMethod(loadedBitmap, mid);
    mid = env->GetMethodID(bmclass, "getHeight", "()I");
    int height = env->CallIntMethod(loadedBitmap, mid);
    
    /* Get pixels */
    jintArray array = env->NewIntArray(width * height);
    
    mid = env->GetMethodID(bmclass, "getPixels", "([I)V");
    env->CallVoidMethod(loadedBitmap, mid, array);
    
    jint *pixels = env->GetIntArrayElements(array, 0);

    swapRB((unsigned char*)pixels, width, height);
    
    pinfo->width = width;
    pinfo->height = height;
    pinfo->pitch = width;
    pinfo->pixels = pixels;
    pinfo->freeImageCallback = freeImageCB;
    pinfo->cbprivate1 = (void*)env->NewGlobalRef(array);;
    
    return NXT_Error_None;
}

static NXT_Error freeImageCB( NXT_ImageInfo* pinfo, void* cbdata ) {
    
    JNIThemeInstanceData *th = (JNIThemeInstanceData*)cbdata;
    JNIEnv *env = th->pEnv;
    jintArray array = (jintArray)pinfo->cbprivate1;
    
//    jclass cls = env->GetObjectClass(th->bitmapManager);
//    jmethodID mid;
    
    env->ReleaseIntArrayElements(array, pinfo->pixels, 0);
    env->DeleteGlobalRef(array);
    
    /* Free image */
//    mid = env->GetMethodID(cls, "close", "()V");
//    env->CallVoidMethod(th->bitmapManager,mid);
    
    pinfo->pixels = NULL;
    pinfo->freeImageCallback = NULL;
    
    return NXT_Error_None;
}
