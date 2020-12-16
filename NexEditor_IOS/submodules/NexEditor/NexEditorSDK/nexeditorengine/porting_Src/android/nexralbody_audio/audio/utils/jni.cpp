/******************************************************************************
* File Name        : jni.cpp
* Description      : JNI utilities for NexAudioRenderer
* Revision History : Located at the bottom of this file
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nexstreaming Confidential Proprietary
*	Copyright (C) 2006~2013 Nexstreaming Corporation
*	All rights are reserved by Nexstreaming Corporation
*
******************************************************************************/
#define LOCATION_TAG "jni"

#define LOG_TAG "nex_ar"
#include "./jni.h"

#include "./utils.h"

#include "NexSAL_Internal.h"

#include <pthread.h>
#include <dlfcn.h>

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#define _NEX_DEFINE_64_
#endif


//namespace { // anon0
//	JavaVM *g_vm = NULL;
namespace Nex_AR {
namespace Utils {
namespace JNI {
	
	JavaVM *g_vm = NULL;
	
	void getJavaVMGlobal()
	{
#ifdef _NEX_DEFINE_64_
		void *art_handle = dlopen("/system/lib64/libandroid_runtime.so", RTLD_LAZY);
#else
		void *art_handle = dlopen("/system/lib/libandroid_runtime.so", RTLD_LAZY);
#endif

		if (NULL == art_handle)
		{
			const char *dlerr = dlerror();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't open libandroid_runtime.so! can't get java vm... (%s)\n", dlerr);
			return;
		}

		void *dlvm = dlsym(art_handle, "_ZN7android14AndroidRuntime7mJavaVME");

		if (NULL == dlvm)
		{
			const char *dlerr = dlerror();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't get java vm symbol! (%s)\n", dlerr);
			dlclose(art_handle);
			art_handle = NULL;
		}

		g_vm = *(JavaVM **)dlvm;

		dlclose(art_handle);
		art_handle = NULL;
	}

	void setJavaVMGlobal(JavaVM *vm)
	{
		g_vm = vm; 
	}	
//} // namespace (anon0)

//namespace { // anon1
	void setJavaVMfromExternal(JavaVM* javaVM)
	{
		if(javaVM)
		{
			g_vm = javaVM;
		}
	}
	
	JavaVM * getJavaVM()
	{
		if(NULL == g_vm)
			getJavaVMGlobal();
		return g_vm;
	}
	
	JNIEnv* getJNIEnv(bool &needsDetach)
	{
		if (NULL == g_vm)
		{
			getJavaVMGlobal();
			if (NULL == g_vm)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "can't get vm!\n");
				return NULL;
			}
		}
		JNIEnv* env = NULL;
		g_vm->GetEnv((void **)&env, JNI_VERSION_1_4);
		if (env == NULL)
		{
			JavaVMAttachArgs args = {JNI_VERSION_1_4, NULL, NULL};
			//*_vm = g_vm;
			//int result = (*_vm)->AttachCurrentThread(&env, (void*) &args);
			int result = g_vm->AttachCurrentThread(&env, (void*) &args);
			if (result != JNI_OK)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "thread attach failed: %#x\n", result);
				return NULL;
			}
			needsDetach = true;
		}
		return env;
	}
//}; // namespace (anon1)

//namespace Nex_AR {
//namespace Utils {
//namespace JNI {
//	void setJavaVMfromExternal(JavaVM* javaVM)
//	{
//		setJavaVMGlobal(javaVM); 	
//	}

//	JavaVM * getJavaVM()
//	{
//		if (NULL == g_vm)
//		{
//			getJavaVMGlobal();
//		}
//		return g_vm;
//	}

	JNIEnvWrapper::JNIEnvWrapper()
		: needsDetach(false)
		, vm(NULL)
	{
		//env = getJNIEnv(&vm, needsDetach);
		env = getJNIEnv(needsDetach);
		vm = g_vm;
	}

	JNIEnvWrapper::~JNIEnvWrapper()
	{
		if (NULL != env && needsDetach)
		{
			vm->DetachCurrentThread();
		}
	}

	jobject NewDirectByteBuffer(jlong capacity, void *&address)
	{
		JNIEnvWrapper env;
		jobject ret = NULL;
		address = malloc(capacity);
		jobject dbb = env->NewDirectByteBuffer(address, capacity);

		if (NULL != dbb)
		{
			ret = env->NewGlobalRef(dbb);
			env->DeleteLocalRef(dbb);
		}

		return ret;
	}

	void DeleteDirectByteBuffer(jobject directByteBuffer)
	{
		JNIEnvWrapper env;
		void *address = env->GetDirectBufferAddress(directByteBuffer);
		env->DeleteGlobalRef(directByteBuffer);
		free(address);
	}

	int jniThrowException(JNIEnv *env, const char *className, const char *msg)
	{
		jclass exceptionClass = env->FindClass(className);

		if (NULL != exceptionClass)
		{
			if (JNI_OK != env->ThrowNew(exceptionClass, msg))
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}

		return 0;
	}
}; // namespace JNI
}; // namespace Utils
}; // namespace Nex_AR

/*=============================================================================
                                Revision History
===============================================================================
    Author         Date        Version       API Version
===============================================================================
    benson         2013/06/24      1                 1
-------------------------------------------------------------------------------
 Initial version
=============================================================================*/

