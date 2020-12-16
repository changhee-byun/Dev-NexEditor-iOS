/******************************************************************************
* File Name        : NexMediaCodec.h
* Description      : MediaCodec interface declarations for NexCRALBody_MC
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

#ifndef _NEXCRAL_NEXMEDIACODEC_H
#define _NEXCRAL_NEXMEDIACODEC_H

#include <stdlib.h>
#include <jni.h>

namespace Nex_MC {

class NexMediaFormat;
class NexSurfaceTexture;

class NexMediaCodec {
public:
	typedef enum CodecClassRequest {
		  GOOGLE_SW = 1
		, OTHER_SW = 2
		, ANY_SW = 3
		, HW = 4
		, ANY = 7
		, CCR_FOURBYTE = 0x7FFFFFFF
	} CodecClassRequest;
protected:
	NexMediaCodec();
	~NexMediaCodec();
public:
	virtual int configure(NexMediaFormat *format, NexSurfaceTexture *surface, jobject crypto, int flags) = 0;

	virtual int start() = 0;
	virtual int stop() = 0;

	virtual int release() = 0;

	virtual int flush() = 0;

	virtual int queueInputBuffer(size_t idx, size_t offset, size_t size, int64_t pts, int flags) = 0;
	virtual int queueSecureInputBuffer(size_t idx, size_t offset, jobject cryptoInfo, int64_t pts, int flags) = 0;

	virtual int dequeueInputBuffer(int64_t timeout, long &result) = 0;
	virtual int dequeueOutputBuffer(jobject bufferInfo, int64_t timeout, long &result) = 0;

	virtual int releaseOutputBuffer(size_t idx, bool render) = 0;

	virtual int getInputFormat(NexMediaFormat * &result) = 0;
	virtual int getOutputFormat(NexMediaFormat * &result) = 0;
	virtual int regetOutputFormat(NexMediaFormat * &result) = 0;

	virtual int getInputBuffers(void ** &result) = 0;
	virtual int getOutputBuffers(bool withoutPointers, void ** &result) = 0;
	virtual int getNumOutputBuffers() = 0;

	virtual int setVideoScalingMode(int mode) = 0;

	virtual int createInputSurface(jobject &result) = 0;
	virtual int signalEndOfInputStream() = 0;

	virtual int captureFrame(bool thumbnailMode) = 0;

	virtual int getName(const char * &result) = 0;
protected:
	static int getSurfaceObjectFromSurfaceTexture(NexSurfaceTexture *surface, jobject &result);
};
};

#endif //#ifndef _NEXCRAL_NEXMEDIACODEC_H
