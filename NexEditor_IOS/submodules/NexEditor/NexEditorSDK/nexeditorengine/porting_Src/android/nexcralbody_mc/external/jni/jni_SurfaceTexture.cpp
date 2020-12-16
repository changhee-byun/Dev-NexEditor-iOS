/******************************************************************************
* File Name        : jni_SurfaceTexture.cpp
* Description      : SurfaceTexture accessors through JNI for NexCRALBody_MC
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
#define LOCATION_TAG "jni_SurfaceTexture"

#include <pthread.h>

#include "./jni_SurfaceTexture.h"

#include "utils/utils.h"
#include "utils/jni.h"

#include <android/native_window_jni.h>

// for differentiating between encoder and decoder in logs (U == undefined)
//#define NEX_MC_TYPE ('U' - 'D')
#define NEX_MC_TYPE 0

namespace Nex_MC {
namespace JNI {
	namespace JCLASS {
		jclass SurfaceTexture = NULL;
		jclass Surface = NULL;
	}; // namespace JCLASS

	namespace JMETHODS {
		JavaSurfaceTextureMethods SurfaceTexture = {
			  NULL // jmethodID constructor
			, NULL // jmethodID attachToGLContext
			, NULL // jmethodID detachFromGLContext
			, NULL // jmethodID getTransformMatrix
			, NULL // jmethodID updateTexImage
			, NULL // jmethodID release
		};

		JavaSurfaceMethods Surface = {
			  NULL // jmethodID constructor
			, NULL // jmethodID release
		};
	}; // namespace JMETHODS
}; // namespace JNI

using Utils::JNI::JNIEnvWrapper;

NexSurfaceTexture_using_jni::NexSurfaceTexture_using_jni()
	: jobjSurfaceTexture(NULL)
	, jobjSurface(NULL)
	, aNativeWindow(NULL)
{
}

NexSurfaceTexture_using_jni::~NexSurfaceTexture_using_jni()
{
	if (NULL != jobjSurfaceTexture)
	{
		JNIEnvWrapper env;
		if (env != NULL)
		{
			MC_INFO("SurfaceTesture.release ++");
			env->CallVoidMethod(jobjSurfaceTexture, JNI::JMETHODS::SurfaceTexture.release);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while calling release");
			}
			MC_INFO("SurfaceTesture.release --");	
			env->DeleteGlobalRef(jobjSurface);
			env->DeleteGlobalRef(jobjSurfaceTexture);
		}
		else
		{
			MC_ERR("couldn't get JNIEnv for deleting global ref to SurfaceTexture & Surface objects");
		}
		jobjSurface = NULL;
		jobjSurfaceTexture = NULL;
	}

	if (NULL != aNativeWindow)
	{
		ANativeWindow_release(aNativeWindow);
		aNativeWindow = NULL;
	}
}

/*static*/ int NexSurfaceTexture_using_jni::createSurfaceTexture(int texName, NexSurfaceTexture * &result)
{
	int ret = 0;
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject jobjSurfaceTexture = env->NewObject(JNI::JCLASS::SurfaceTexture, JNI::JMETHODS::SurfaceTexture.constructor, (jint)texName);

	if (NULL == jobjSurfaceTexture)
	{
		MC_ERR("failed");

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling SurfaceTexture constructor");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	jobject jobjSurface = env->NewObject(JNI::JCLASS::Surface, JNI::JMETHODS::Surface.constructor, jobjSurfaceTexture);

	if (NULL == jobjSurface)
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jobjSurfaceTexture);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling Surface constructor");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -4;
		}
		return -5;
	}

	NexSurfaceTexture_using_jni *tempResult = new NexSurfaceTexture_using_jni;
	tempResult->jobjSurfaceTexture = env->NewGlobalRef(jobjSurfaceTexture);
	tempResult->jobjSurface = env->NewGlobalRef(jobjSurface);

	env->DeleteLocalRef(jobjSurface);
	env->DeleteLocalRef(jobjSurfaceTexture);

	result = tempResult;

	return 0;
}

/*static*/ int NexSurfaceTexture_using_jni::releaseNexSurfaceTexture(NexSurfaceTexture *surfaceTexture)
{
	delete static_cast<NexSurfaceTexture_using_jni *>(surfaceTexture);
	return 0;
}

int NexSurfaceTexture_using_jni::attachToGLContext(int texName)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjSurfaceTexture, JNI::JMETHODS::SurfaceTexture.attachToGLContext, (jint)texName);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling attachToGLContext");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexSurfaceTexture_using_jni::detachFromGLContext()
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjSurfaceTexture, JNI::JMETHODS::SurfaceTexture.detachFromGLContext);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling detachFromGLContext");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexSurfaceTexture_using_jni::getTransformMatrix(GLfloat *mtx)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jfloatArray matrix = env->NewFloatArray(16);
	if (NULL == matrix)
	{
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while making new float array");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
    		return -2;
		}
		return -3;
	}
	env->CallVoidMethod(jobjSurfaceTexture, JNI::JMETHODS::SurfaceTexture.getTransformMatrix, matrix);
	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling getTransformMatrix");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -4;
	}
	env->GetFloatArrayRegion(matrix, 0, 16, mtx);
	env->DeleteLocalRef(matrix);

	return 0;
}

int NexSurfaceTexture_using_jni::updateTexImage()
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjSurfaceTexture, JNI::JMETHODS::SurfaceTexture.updateTexImage);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling updateTexImage");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexSurfaceTexture_using_jni::getSurfaceObject(jobject &result)
{
	result = jobjSurface;
	return 0;
}

int NexSurfaceTexture_using_jni::getANativeWindowFromSurfaceObject(ANativeWindow *&result)
{
	if (NULL == jobjSurface)
	{
		MC_ERR("surface object is null");
		return -1;
	}

	if (NULL == aNativeWindow)
	{
		Nex_MC::Utils::JNI::JNIEnvWrapper env;

		if (env == NULL)
			return -2;

		aNativeWindow = ANativeWindow_fromSurface(env.get(), jobjSurface);
	}

	result = aNativeWindow;

	return 0;
}

int NexSurfaceTexture_using_jni::releaseAcquiredANativeWindow()
{
	if (NULL != aNativeWindow)
	{
		ANativeWindow_release(aNativeWindow);
		aNativeWindow = NULL;
	}

	return 0;
}

/* static */ int NexSurfaceTexture_using_jni::releaseSurfaceObject(jobject surface)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(surface, JNI::JMETHODS::Surface.release);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling release");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

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
				jclass surfaceTextureClass = env->FindClass("android/graphics/SurfaceTexture");

				if (NULL == surfaceTextureClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/graphics/SurfaceTexture\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::SurfaceTexture = (jclass)env->NewGlobalRef(surfaceTextureClass);

				Nex_MC::JNI::JMETHODS::SurfaceTexture.constructor = GetMethodID(env, surfaceTextureClass
					, "<init>", "(I)V");
				Nex_MC::JNI::JMETHODS::SurfaceTexture.attachToGLContext = GetMethodID(env, surfaceTextureClass
					, "attachToGLContext", "(I)V");
				Nex_MC::JNI::JMETHODS::SurfaceTexture.detachFromGLContext = GetMethodID(env, surfaceTextureClass
					, "detachFromGLContext", "()V");
				Nex_MC::JNI::JMETHODS::SurfaceTexture.getTransformMatrix = GetMethodID(env, surfaceTextureClass
					, "getTransformMatrix", "([F)V");
				Nex_MC::JNI::JMETHODS::SurfaceTexture.updateTexImage = GetMethodID(env, surfaceTextureClass
					, "updateTexImage", "()V");
				Nex_MC::JNI::JMETHODS::SurfaceTexture.release = GetMethodID(env, surfaceTextureClass
					, "release", "()V", false);

				jclass surfaceClass = env->FindClass("android/view/Surface");

				if (NULL == surfaceClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/view/Surface\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::Surface = (jclass)env->NewGlobalRef(surfaceClass);

				Nex_MC::JNI::JMETHODS::Surface.constructor = GetMethodID(env, surfaceClass
					, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
				Nex_MC::JNI::JMETHODS::Surface.release = GetMethodID(env, surfaceClass
					, "release", "()V");
			}
		}
		void OnUnload()
		{
			Nex_MC::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				if (NULL != Nex_MC::JNI::JCLASS::SurfaceTexture)
				{
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::SurfaceTexture);
					Nex_MC::JNI::JCLASS::SurfaceTexture = NULL;
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::Surface);
					Nex_MC::JNI::JCLASS::Surface = NULL;
				}
			}
		}
	private:
		pthread_mutex_t lock;
		int instanceCount;
	};

	InitJNI initJNI;
}; // namespace (anon1)
