/******************************************************************************
* File Name   :	NexJNIEnv.cpp
* Description : JNI Manager for jni call from Engine.
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2016 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
eric	2016/12/07	Draft.
-----------------------------------------------------------------------------*/

#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include "string.h"

#include "NexJNIEnvStack.h"

#define DEBIGING_ING

#define  LOG_TAG    "NEXJNI"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

namespace { // anonymous
	JavaVM *g_javaVM;
	pthread_key_t g_envKey;
	pthread_once_t once_control = PTHREAD_ONCE_INIT;

	void destroy_tls_value(void * env)
	{
		if (NULL == g_javaVM)
		{
			LOGE("(ERIC) There is no VM. (destroy_tls_value)\n");
		}
		else
		{
#ifdef DEBIGING_ING
            LOGE("(ERIC) destroy_tls_value called.");
#endif
			g_javaVM->DetachCurrentThread();
		}
	}

	void make_tls_key()
	{
#ifdef DEBIGING_ING
        LOGE("(ERIC) make_tls_key called.");
#endif
		(void)pthread_key_create(&g_envKey, destroy_tls_value);
	}
}


//static
void NexJNIEnvStack::setJavaVM(void* _vm)
{
    LOGE("(ERIC) set JAVA VM");
	g_javaVM = (JavaVM*)_vm;
}

NexJNIEnvStack::NexJNIEnvStack()
: env(NULL)
, list(&default_list[0])
, listMax(DEFAULT_MAX_LIST_FOR_DELETEREF)
, listCount(0)
{
	if ( !g_javaVM ) {
		LOGE("(ERIC) There is no VM.");
		return;	
	}

	bool needsDetach = false;
	int get_env_ret = g_javaVM->GetEnv((void **)&env, JNI_VERSION_1_4);
	if (get_env_ret != JNI_OK || env == NULL) {

		JavaVMAttachArgs args = {JNI_VERSION_1_4, NULL, NULL};
		int result = g_javaVM->AttachCurrentThread(&env, (void*) &args);
		if (result != JNI_OK || env == NULL)
		{
			LOGE("VM attach failed: %#x", result);
			return;
		}
		needsDetach = true;
	}

	if ( needsDetach ) {
		(void) pthread_once(&once_control, make_tls_key);
		void *ptr;
		if (NULL == (ptr = pthread_getspecific(g_envKey)))
		{
			(void) pthread_setspecific(g_envKey, (void *)env);
#ifdef DEBIGING_ING
            LOGE("VM attached called.");
#endif

		}
	}

	if ( env != NULL && JNI_TRUE == env->ExceptionCheck() )
	{
		LOGE("NexJNIEnvStack : env already in exception state");
		env->ExceptionDescribe();
		env->ExceptionClear();
	}
}

NexJNIEnvStack::~NexJNIEnvStack()
{
	for ( int i = 0 ; i < listCount ; i++ ) {
		if ( list[i] != NULL ) {
			env->DeleteLocalRef((jobject)list[i]);
			//LOGE("(ERIC) auto delete local ref.");
		} else {
			LOGE("(ERIC) auto delete local ref. ref is null. what happens.");
		}
	}

	if ( listMax > DEFAULT_MAX_LIST_FOR_DELETEREF )
		free(list);
}

void NexJNIEnvStack::addAutoDeleteLocalRef(void* obj)
{
	if ( listCount > listMax-1 ) {
		void** oldlist = list;
		list = (void**)malloc( listMax*2*sizeof(void*) );

		for ( int i = 0 ; i < listCount ; i++ )
			list[i] = oldlist[i];

		if ( listMax > DEFAULT_MAX_LIST_FOR_DELETEREF ) {
			free( oldlist );
		}
		listMax *= 2;
	}

	list[listCount] = obj;
	listCount++;
}
