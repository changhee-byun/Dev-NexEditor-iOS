/******************************************************************************
* File Name        : jni_SurfaceTexture.h
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

#ifndef _NEXCRAL_JNI_SURFACETEXTURE_H
#define _NEXCRAL_JNI_SURFACETEXTURE_H

#include <jni.h>

#include "external/NexSurfaceTexture.h"

#ifndef LOG_TAG
#define LOG_TAG "nexcral_mc"
#endif

namespace Nex_MC {
namespace JNI {
	namespace JCLASS {
		extern jclass SurfaceTexture;
		extern jclass Surface;
	}; // namespace JCLASS

	namespace JMETHODS {
		typedef struct JavaSurfaceTextureMethods {
			jmethodID constructor;
			jmethodID attachToGLContext;
			jmethodID detachFromGLContext;
			jmethodID getTransformMatrix;
			jmethodID updateTexImage;
			jmethodID release;
		} JavaSurfaceTextureMethods;
		extern JavaSurfaceTextureMethods SurfaceTexture;

		typedef struct JavaSurfaceMethods {
			jmethodID constructor;
			jmethodID release;
		} JavaSurfaceMethods;
		extern JavaSurfaceMethods Surface;
	}; // namespace JMETHODS
}; // namespace JNI

class NexSurfaceTexture_using_jni : public NexSurfaceTexture {
private:
	NexSurfaceTexture_using_jni();
	virtual ~NexSurfaceTexture_using_jni();
public:
	static int createSurfaceTexture(int texName, NexSurfaceTexture * &result);

	static int releaseNexSurfaceTexture(NexSurfaceTexture *surfaceTexture);

	virtual int attachToGLContext(int texName);
	virtual int detachFromGLContext();
	virtual int getTransformMatrix(GLfloat *mtx);
	virtual int updateTexImage();
	virtual int getSurfaceObject(jobject &result);
	virtual int getANativeWindowFromSurfaceObject(ANativeWindow *&result);
	virtual int releaseAcquiredANativeWindow();

	static int releaseSurfaceObject(jobject surface);
private:
	jobject jobjSurfaceTexture;
	jobject jobjSurface;
	ANativeWindow *aNativeWindow;

private:
	NexSurfaceTexture_using_jni(const NexSurfaceTexture_using_jni &);
	NexSurfaceTexture_using_jni &operator=(const NexSurfaceTexture_using_jni &);
}; // class NexSurfaceTexture_using_jni

}; // namespace Nex_MC

#endif //#ifndef _NEXCRAL_JNI_SURFACETEXTURE_H

