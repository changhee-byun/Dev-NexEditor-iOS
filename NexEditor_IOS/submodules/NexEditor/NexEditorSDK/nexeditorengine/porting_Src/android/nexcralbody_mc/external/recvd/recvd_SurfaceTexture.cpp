/******************************************************************************
* File Name        : recvd_SurfaceTexture.cpp
* Description      : SurfaceTexture accessors through recvd for NexCRALBody_MC
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
#define LOCATION_TAG "recvd_SurfaceTexture"

#include "./recvd_SurfaceTexture.h"
#include "../jni/jni_SurfaceTexture.h"

#include "utils/utils.h"
#include "utils/jni.h"

#include <android/native_window_jni.h>

// for differentiating between encoder and decoder in logs (U == undefined)
//#define NEX_MC_TYPE ('U' - 'D')
#define NEX_MC_TYPE 0

namespace Nex_MC {

NexSurfaceTexture_using_recvd::NexSurfaceTexture_using_recvd()
	: jobjSurface(NULL)
	, aNativeWindow(NULL)
{
}

NexSurfaceTexture_using_recvd::~NexSurfaceTexture_using_recvd()
{
	jobjSurface = NULL;

	if (NULL != aNativeWindow)
	{
		ANativeWindow_release(aNativeWindow);
		aNativeWindow = NULL;
	}
}

/*static*/ int NexSurfaceTexture_using_recvd::createSurfaceTexture(jobject surface, NexSurfaceTexture * &result)
{
	NexSurfaceTexture_using_recvd *ret = new NexSurfaceTexture_using_recvd;
	ret->jobjSurface = surface;

	result = ret;

	return 0;
}

#define RET_IF_EQ(x, y) if ((x) == (y)) return (x)
#define RET_IF_NEQ(type, x, y) do { \
	type tempVar;                   \
	if ((x) != (tempVar = (y))) {   \
		return tempVar;             \
	}                               \
} while(0)

/*static*/ int NexSurfaceTexture_using_recvd::releaseNexSurfaceTexture(NexSurfaceTexture *surfaceTexture, bool callRelease, bool deleteGlobalRef)
{
	int ret = 0;
	jobject surface = NULL;

	RET_IF_NEQ(int, 0, surfaceTexture->getSurfaceObject(surface));

	if (callRelease)
	{
		RET_IF_NEQ(int, 0, NexSurfaceTexture_using_jni::releaseSurfaceObject(surface));
	}

	if (deleteGlobalRef)
	{
		Nex_MC::Utils::JNI::JNIEnvWrapper env;

		if (env != NULL)
		{
			env->DeleteGlobalRef(surface);
		}

		if (JNI_TRUE == env->ExceptionCheck())
		{
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			ret = -1;
		}
	}

	delete static_cast<NexSurfaceTexture_using_recvd *>(surfaceTexture);

	return ret;
}

int NexSurfaceTexture_using_recvd::attachToGLContext(int texName)
{
	return 0;
}

int NexSurfaceTexture_using_recvd::detachFromGLContext()
{
	return 0;
}

int NexSurfaceTexture_using_recvd::getTransformMatrix(GLfloat *mtx)
{
	return 0;
}

int NexSurfaceTexture_using_recvd::updateTexImage()
{
	return 0;
}

int NexSurfaceTexture_using_recvd::getSurfaceObject(jobject &result)
{
	result = jobjSurface;
	return 0;
}

int NexSurfaceTexture_using_recvd::getANativeWindowFromSurfaceObject(ANativeWindow *&result)
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

int NexSurfaceTexture_using_recvd::releaseAcquiredANativeWindow()
{
	if (NULL != aNativeWindow)
	{
		ANativeWindow_release(aNativeWindow);
		aNativeWindow = NULL;
	}

	return 0;
}


}; // namespace Nex_MC
