/******************************************************************************
* File Name        : NexSurfaceTexture.h
* Description      : SurfaceTexture interface declarations for NexCRALBody_MC
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

#ifndef _NEXCRAL_NEXSURFACETEXTURE_H
#define _NEXCRAL_NEXSURFACETEXTURE_H

#include <jni.h>

#include <stdlib.h>
#include <GLES2/gl2.h>

class ANativeWindow;

namespace Nex_MC {

class NexMediaCodec;

class NexSurfaceTexture {
protected:
	NexSurfaceTexture() {};
	~NexSurfaceTexture() {};
public:
	virtual int attachToGLContext(int texName) = 0;
	virtual int detachFromGLContext() = 0;
	virtual int getTransformMatrix(GLfloat *mtx) = 0;
	virtual int updateTexImage() = 0;
	virtual int getSurfaceObject(jobject &result) = 0;
	virtual int getANativeWindowFromSurfaceObject(ANativeWindow *&result) = 0;
	virtual int releaseAcquiredANativeWindow() = 0;
}; // class NexSurfaceTexture

}; // namespace Nex_MC

#endif //#ifndef _NEXCRAL_NEXSURFACETEXTURE_H
