//
// Created by jeongwook.yoon on 2019-05-07.
//

#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define NEXEDITOR_MAJOR_VERSION	1
#define NEXEDITOR_MINOR_VERSION	1
#define NEXEDITOR_PATCH_VERSION	41

//#include "KISA_SHA256.h"
#define  LOG_TAG    "NEXEDITORUTIL"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#include "KenBurnsRectanglesGenerator.h"

#define MAKE_JNI_FUNC_NAME(package_prefix,function_name) package_prefix##function_name
#ifdef FOR_PROJECT_Kinemaster
#define _EDITOR_JNI_(f) MAKE_JNI_FUNC_NAME(Java_com_nextreaming_nexvideoeditor_nexEditorUtils_,f)
#else
#define _EDITOR_JNI_(f) MAKE_JNI_FUNC_NAME(Java_com_nexstreaming_kminternal_nexvideoeditor_nexEditorUtils_,f)
#endif

extern "C" {
JNIEXPORT jint JNICALL _EDITOR_JNI_(getKenBurnsRectsN)(JNIEnv *env, jobject obj, jint width, jint height, jint faceCount, jobjectArray faceRects, jint aspectWidth, jint aspectHeight, jint duration , jobjectArray resultRects){
    RECT * prFace = NULL;

    float dim=100000.0;
    int iArrayCount = 0;

	if (faceCount != 0)
	{
		iArrayCount = env->GetArrayLength(faceRects);
	    LOGI("[nexUtils_jni.cpp %d] getKenBurnsRects(%d,%d,%d,%d,%d,%d)", __LINE__,width,height,iArrayCount,aspectWidth,aspectHeight,duration);
	    if( iArrayCount > 0 ){
	        prFace = new RECT[iArrayCount];
	    }

	    for(int i = 0; i < iArrayCount; i++)
	    {
	        jobject objClip = env->GetObjectArrayElement(faceRects, i);
	        if( objClip == NULL )
	        {
	            LOGI("[nexUtils_jni.cpp %d] getKenBurnsRects facs(%d)objClip is null", __LINE__,i);
	            continue;
	        }

	        jfieldID		id;
	        jclass		clipInfo_ref_class;
	        clipInfo_ref_class = env->GetObjectClass(objClip);
	        if( clipInfo_ref_class == NULL )
	        {
	            env->DeleteLocalRef(objClip);
	            LOGI("[nexUtils_jni.cpp %d] getKenBurnsRects facs(%d)clipInfo_ref_class is null", __LINE__,i);
	            continue;
	        }
	        int value;
	        id = env->GetFieldID(clipInfo_ref_class, "mLeft", "I");
	        value = env->GetIntField(objClip, id);
	        LOGI("[nexUtils_jni.cpp %d] getKenBurnsRects facs(%d)mLeft=%d", __LINE__,i,value);
	        prFace[i].left = (int)(width *  (value / dim));
	        id = env->GetFieldID(clipInfo_ref_class, "mTop", "I");
	        value = env->GetIntField(objClip, id);
	        prFace[i].top = (int)(height *  (value / dim));

	        id = env->GetFieldID(clipInfo_ref_class, "mRight", "I");
	        value = env->GetIntField(objClip, id);
	        prFace[i].right = (int)(width *  (value / dim));

	        id = env->GetFieldID(clipInfo_ref_class, "mBottom", "I");
	        value = env->GetIntField(objClip, id);
	        prFace[i].bottom = (int)(height *  (value / dim));

	        env->DeleteLocalRef(objClip);
	        env->DeleteLocalRef(clipInfo_ref_class);
	    }

	    for(int i = 0; i < iArrayCount; i++){
	        LOGI("[nexUtils_jni.cpp %d] getKenBurnsRects facs(%d)(%d,%d,%d,%d)", __LINE__,i,prFace[i].left,prFace[i].top,prFace[i].right,prFace[i].bottom);
	    }
	}
 
    RECT src = { 0,0,width,height };
    RECT output[2];

    KenBurnsRectanglesGenerator gen;
    gen.generate(&src, prFace, iArrayCount, aspectWidth, aspectHeight, duration, output);

    for(int i = 0; i < 2; i++){
        LOGI("[nexUtils_jni.cpp %d] getKenBurnsRects result(%d)(%d,%d,%d,%d)", __LINE__,i,output[i].left,output[i].top,output[i].right,output[i].bottom);
    }

    iArrayCount = env->GetArrayLength(resultRects);
    for(int i = 0; i < iArrayCount; i++){
        if( i > 1 )
            break;
        jobject objClip = env->GetObjectArrayElement(resultRects, i);
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

        id = env->GetFieldID(clipInfo_ref_class, "mLeft", "I");
        env->SetIntField(objClip, id,output[i].left);

        id = env->GetFieldID(clipInfo_ref_class, "mTop", "I");
        env->SetIntField(objClip, id,output[i].top);

        id = env->GetFieldID(clipInfo_ref_class, "mRight", "I");
        env->SetIntField(objClip, id,output[i].right);

        id = env->GetFieldID(clipInfo_ref_class, "mBottom", "I");
        env->SetIntField(objClip, id,output[i].bottom);

        env->DeleteLocalRef(objClip);
        env->DeleteLocalRef(clipInfo_ref_class);
    }

    //clean
    if(prFace){
        delete[] prFace;
    }
    return 0;
}
}

