/******************************************************************************
* File Name        : recvd_SurfaceTexture.h
* Description      : SurfaceTexture accessors through RECVD for NexCRALBody_MC
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

#ifndef _NEXCRAL_RECVD_SURFACETEXTURE_H
#define _NEXCRAL_RECVD_SURFACETEXTURE_H

#include "external/NexSurfaceTexture.h"

#ifndef LOG_TAG
#define LOG_TAG "nexcral_mc"
#endif

namespace Nex_MC {

class NexSurfaceTexture_using_recvd : public NexSurfaceTexture {
private:
	NexSurfaceTexture_using_recvd();
	virtual ~NexSurfaceTexture_using_recvd();
public:
	static int createSurfaceTexture(jobject surface, NexSurfaceTexture * &result);

	static int releaseNexSurfaceTexture(NexSurfaceTexture *surfaceTexture, bool callRelease, bool deleteGlobalRef);

	// these don't do anything!
	virtual int attachToGLContext(int texName);
	virtual int detachFromGLContext();
	virtual int getTransformMatrix(GLfloat *mtx);
	virtual int updateTexImage();
	virtual int getSurfaceObject(jobject &result);
	virtual int getANativeWindowFromSurfaceObject(ANativeWindow *&result);
	virtual int releaseAcquiredANativeWindow();

private:
	jobject jobjSurface;
	ANativeWindow *aNativeWindow;

private:
	NexSurfaceTexture_using_recvd(const NexSurfaceTexture_using_recvd &);
	NexSurfaceTexture_using_recvd &operator=(const NexSurfaceTexture_using_recvd &);
}; // class NexSurfaceTexture_using_recvd

}; // namespace Nex_MC

#endif //#ifndef _NEXCRAL_RECVD_SURFACETEXTURE_H

