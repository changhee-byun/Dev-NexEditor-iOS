#include <jni.h>
#include <android/log.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "IMusicHighlighter.h"
#include "MusicHighlighter_DataDefinition.h"


#define  LOG_TAG    "nxBeatSync"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define ECODE_None                  0
#define ECODE_INVALID_Argument      -1
#define ECODE_MAX_Handle            -2
#define ECODE_INVALID_Handle        -3
#define ECODE_FailOfCreate          -4
#define ECODE_FailOfInit            -5
#define ECODE_FailOfUpdate          -6

#define FrameFlag_EoS               1

extern "C" {

#define MAX_HANDLE_COUNT 16

#define STATE_IDEL 0
#define STATE_INIT 1
#define STATE_PROCCESS 2
#define STATE_END 3

typedef struct _jniHandle{
    unsigned char set;
    unsigned char state;
    unsigned char need_update;
    unsigned char rev;
    void * handle;
}jniHandle;

static jniHandle s_DB[MAX_HANDLE_COUNT];
static int s_handle_count = 0;

#include<math.h>

extern int __isnanf(float x)
{
    return isnanf(x);
}

static int _getEmptyHandle(){
    for(int i = 0 ; i < MAX_HANDLE_COUNT ; i++ ){
        if( s_DB[i].set == 0 ){
            s_DB[i].set = 1;
            s_DB[i].state = STATE_IDEL;
            return i;
        }
    }
    return ECODE_MAX_Handle;
}

static int _createHandle(){
    int i = _getEmptyHandle();
    if( i < 0 ){
        return i;
    }

    s_handle_count++;
    if( s_handle_count == 1 ){

    }
    return i;
}

static int _destoryHandle(int index){
    if( s_DB[index].set != 0 ){
        s_DB[index].set = 0;
        s_DB[index].state = STATE_END;
        s_handle_count--;
        if( s_handle_count == 0 ){

        }
        return ECODE_None;
    }
    return ECODE_INVALID_Handle;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_beatsyncsdk_nxBeatsyncSDK_createHandle(JNIEnv* env, jobject thiz, jint channel, jint sampling_rate,  jint bits_per_sample){
    int handle_index;
   	handle_index = _createHandle();
    if( handle_index < 0 ){
        LOGE("[nxBeatSyncjni.cpp %d] createHandle fail!(%d)", __LINE__,handle_index);
        return ECODE_INVALID_Handle;
    }

    LOGI("[nxBeatSyncjni.cpp %d] createHandle index(%d) param(%d,%d,%d)", __LINE__,handle_index,channel,sampling_rate,bits_per_sample);

	IMusicHighlighter_Ptr music_highlighter = CreateMusicHighlighter();

	if (music_highlighter == NULL)
	{
       LOGE("[nxBeatSyncjni.cpp %d] createHandle fail!(%d)", __LINE__,handle_index);
       _destoryHandle(handle_index);
       return ECODE_INVALID_Handle;
	}

	int return_value = music_highlighter->Initialize(channel, sampling_rate, bits_per_sample);

	if (return_value != MUSIC_HIGHLIGHTER_SUCCESS)
	{
       LOGE("[nxBeatSyncjni.cpp %d] openFile create handle fail!(%d)", __LINE__,handle_index);
       _destoryHandle(handle_index);
       return ECODE_INVALID_Handle;
	}
		
	s_DB[handle_index].handle = music_highlighter;

		//return_value = music_highlighter->SetMusicSource((char*)pOut, read, false);

    return handle_index;
}

JNIEXPORT jint JNICALL Java_com_nexstreaming_beatsyncsdk_nxBeatsyncSDK_destroyHandle(JNIEnv* env, jobject thiz, jint handle){
	IMusicHighlighter_Ptr music_highlighter = NULL;
    if( s_DB[handle].set == 0 ){
        LOGE("[nxBeatSyncjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }
    
    music_highlighter = (IMusicHighlighter_Ptr)s_DB[handle].handle;
    
    music_highlighter->Release();
	DestroyMusicHighlighter(music_highlighter);

    return _destoryHandle(handle);
}

JNIEXPORT jint  JNICALL Java_com_nexstreaming_beatsyncsdk_nxBeatsyncSDK_pushPCM(JNIEnv* env, jobject thiz,  jint handle,jint flags, jbyteArray pcm_data){
    jbyte *jarr;
    int length;
    int iRet;
    int eos = 0;
    IMusicHighlighter_Ptr music_highlighter = NULL;

    if(  s_DB[handle].set == 0  ){
        LOGE("[nxBeatSyncjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }

    if( flags == FrameFlag_EoS ){
        eos = 1;
        LOGE("[nxBeatSyncjni.cpp %d] EoS(%d)", __LINE__, handle);
    }

    music_highlighter =  (IMusicHighlighter_Ptr)s_DB[handle].handle;

    jsize size = env->GetArrayLength(pcm_data);
    jarr = env->GetByteArrayElements(pcm_data,NULL);
    if( jarr == NULL){
        LOGE("[nxBeatSyncjni.cpp %d] invalid input data! size(%d)", __LINE__, size);
        return ECODE_INVALID_Argument;
    }

    int return_value = music_highlighter->SetMusicSource((char*)jarr, size, eos);
    if (return_value != MUSIC_HIGHLIGHTER_SUCCESS){
        LOGE("[nxBeatSyncjni.cpp %d] SetMusicSource fail!(%d)", __LINE__, return_value);
        return ECODE_INVALID_Argument;
    }
    return return_value;
}

JNIEXPORT jint  JNICALL Java_com_nexstreaming_beatsyncsdk_nxBeatsyncSDK_processFind(JNIEnv* env, jobject thiz,jint handle ){
    if( s_DB[handle].set == 0 ){
        LOGE("[nxBeatSyncjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return ECODE_INVALID_Handle;
    }
    IMusicHighlighter_Ptr music_highlighter = (IMusicHighlighter_Ptr)s_DB[handle].handle;

    if( s_DB[handle].state != STATE_PROCCESS ){
        music_highlighter->ProcessBeatSync();
        s_DB[handle].state = STATE_PROCCESS;
    }
    return 0;
}

JNIEXPORT jintArray  JNICALL Java_com_nexstreaming_beatsyncsdk_nxBeatsyncSDK_popResult(JNIEnv* env, jobject thiz,jint handle, jint cmd ){

    if( s_DB[handle].set == 0 ){
        LOGE("[nxBeatSyncjni.cpp %d] invalid handle(%d)", __LINE__, handle);
        return NULL;
    }
    IMusicHighlighter_Ptr music_highlighter = (IMusicHighlighter_Ptr)s_DB[handle].handle;

	int array_size = 0;
	int* pos_list = NULL;
	MusicHL::Tag tag_size = MusicHL::BeatSync_SizeOfBeatPosList;
	MusicHL::Tag tag_list = MusicHL::BeatSync_ListOfBeatPos;

	switch ( cmd ){
	    case 1 :
	    tag_size = MusicHL::BeatSync_SizeOfTransitionPosList;
	    tag_list = MusicHL::BeatSync_ListOfTransitionPos;
	    break;

	    case 2 :
	    tag_size = MusicHL::BeatSync_SizeOfEffectPosList;
	    tag_list = MusicHL::BeatSync_ListOfEffectPos;
	    break;
	}

    music_highlighter->Get(tag_size, &array_size);
    pos_list = new int[array_size];
    music_highlighter->Get(tag_list, &pos_list);

	jintArray newArray = env->NewIntArray(array_size);
    jint *narr = env->GetIntArrayElements(newArray, NULL);
	for (int i = 0; i < array_size; i++)
	{
	    narr[i] = pos_list[i];
	}
	delete[] pos_list;
    env->ReleaseIntArrayElements(newArray, narr, NULL);
    return newArray;
}


}
