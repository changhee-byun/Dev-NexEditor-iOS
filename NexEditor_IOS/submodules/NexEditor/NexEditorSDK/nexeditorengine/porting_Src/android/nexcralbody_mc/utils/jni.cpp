/******************************************************************************
* File Name        : jni.cpp
* Description      : JNI utilities for NexCALBody_MC
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

#define LOG_TAG "nexcral_mc"
#include "./utils.h"
#include "./jni.h"

#include <pthread.h>
#include <dlfcn.h>

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#define _NEX_DEFINE_64_
#endif

// for differentiating between encoder and decoder in logs (U == undefined)
#define NEX_MC_TYPE ('U' - 'D')

namespace { // anon0
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
			MC_ERR("couldn't open libandroid_runtime.so! can't get java vm... (%s)\n", dlerr);
			return;
		}

		void *dlvm = dlsym(art_handle, "_ZN7android14AndroidRuntime7mJavaVME");

		if (NULL == dlvm)
		{
			const char *dlerr = dlerror();
			MC_ERR("couldn't get java vm symbol! (%s)\n", dlerr);
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
} // namespace (anon0)

namespace { // anon1
	pthread_key_t envKey;
	pthread_once_t envKey_once = PTHREAD_ONCE_INIT;

	void destroy_env(void * env)
	{
		JavaVM *vm = g_vm;
		if (NULL == vm)
		{
			MC_ERR("getJavaVM returned NULL (destroy_env)\n");
		}
		else
		{
			vm->DetachCurrentThread();
		}
	}

	void make_envKey()
	{
		(void) pthread_key_create(&envKey, destroy_env);
	}

	JNIEnv* getJNIEnv(JavaVM** _vm, bool &needsDetach)
	{
		if (NULL == g_vm)
		{
			getJavaVMGlobal();
			if (NULL == g_vm)
			{
				MC_ERR("can't get vm!");
				return NULL;
			}
		}
		JNIEnv* env = NULL;
		g_vm->GetEnv((void **)&env, JNI_VERSION_1_4);
		if (env == NULL)
		{
			JavaVMAttachArgs args = {JNI_VERSION_1_4, NULL, NULL};
			*_vm = g_vm;
			int result = (*_vm)->AttachCurrentThread(&env, (void*) &args);
			if (result != JNI_OK)
			{
				MC_ERR("thread attach failed: %#x", result);
				return NULL;
			}
			needsDetach = true;
		}
		return env;
	}
}; // namespace (anon1)

namespace Nex_MC {
namespace Utils {
namespace JNI {
	namespace JCLASS {
		jclass String = NULL;
	}

	namespace JMETHODS {
		JavaStringMethods String = {
		  NULL //jmethodID equals;
		};
	}

	void setJavaVMfromExternal(JavaVM* javaVM)
	{
		setJavaVMGlobal(javaVM); 	
	}
	
	JavaVM * getJavaVM()
	{
		if (NULL == g_vm)
		{
			getJavaVMGlobal();
		}
		return g_vm;
	}

	JNIEnvWrapper::JNIEnvWrapper()
		: needsDetach(false)
		, vm(NULL)
	{
		env = getJNIEnv(&vm, needsDetach);

		if (needsDetach)
		{
			(void) pthread_once(&envKey_once, make_envKey);
			void *ptr;
			if (NULL == (ptr = pthread_getspecific(envKey)))
			{
				(void) pthread_setspecific(envKey, (void *)env);
			}
			needsDetach = false;
		}

		if (env == NULL)
		{
			MC_ERR("could not get jni-env");
		}
		else
		{
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("env already in exception state");
				env->ExceptionDescribe();
				env->ExceptionClear();
				//env = NULL;
			}
		}
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
		if (NULL == address)
		{
			MC_ERR("malloc failed!");
			return NULL;
		}
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
}; // namespace Nex_MC


namespace { // anon1
	using Nex_MC::Utils::JNI::GetMethodID;
	using Nex_MC::Utils::JNI::jniThrowException;

	class InitJNI : Nex_MC::Utils::Initializer
	{
	public:
		InitJNI()
			: instanceCount(0)
		{
			pthread_mutex_init(&lock, NULL);
			Nex_MC::Utils::registerInitializer(this);
		}

		virtual ~InitJNI()
		{
			Nex_MC::Utils::unregisterInitializer(this);
			deinitialize(true);
			pthread_mutex_destroy(&lock);
		}

		virtual void initialize()
		{
			pthread_mutex_lock(&lock);

			if (0 == instanceCount)
			{
				OnLoad();
			}

			++instanceCount;

			pthread_mutex_unlock(&lock);
		}

		virtual void deinitialize(bool force)
		{
			pthread_mutex_lock(&lock);

			--instanceCount;

			if (0 == instanceCount)
			{
				OnUnload();
			}
			else if (force && 0 < instanceCount)
			{
				OnUnload();
				instanceCount = 0;
			}

			pthread_mutex_unlock(&lock);
		}
	private:
		void OnLoad()
		{
			Nex_MC::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				jclass stringClass = env->FindClass("java/lang/String");

				if (NULL == stringClass)
				{
					MC_ERR("JNI: Couldn't find class \"java/lang/String\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::Utils::JNI::JCLASS::String = (jclass)env->NewGlobalRef(stringClass);

				Nex_MC::Utils::JNI::JMETHODS::String.equals = GetMethodID(env, stringClass
					, "equals", "(Ljava/lang/Object;)Z");
			}
		}
		void OnUnload()
		{
			Nex_MC::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				if (NULL != Nex_MC::Utils::JNI::JCLASS::String)
				{
					env->DeleteGlobalRef(Nex_MC::Utils::JNI::JCLASS::String);
					Nex_MC::Utils::JNI::JCLASS::String = NULL;
				}
			}
		}
	private:
		pthread_mutex_t lock;
		int instanceCount;
	};
	InitJNI initJNI;
}
/*=============================================================================
                                Revision History
===============================================================================
    Author         Date        Version       API Version
===============================================================================
    benson         2013/04/09      1                 1
-------------------------------------------------------------------------------
 Initial version
=============================================================================*/

