/******************************************************************************
* File Name        : jni_MediaFormat.h
* Description      : MediaFormat accessors through JNI for NexCRALBody_MC
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

#ifndef _NEXCRAL_JNI_MEDIAFORMAT_H
#define _NEXCRAL_JNI_MEDIAFORMAT_H

#include <jni.h>

#include "external/NexMediaFormat.h"

#ifndef LOG_TAG
#define LOG_TAG "nexcral_mc"
#endif

namespace Nex_MC {
namespace JNI {
	namespace JCLASS {
		extern jclass MediaFormat;
	}; // namespace JCLASS

	namespace JMETHODS {
		typedef struct JavaMediaFormatMethods {
			jmethodID createAudioFormat;
			jmethodID createVideoFormat;
			jmethodID getInteger;
			jmethodID setInteger;
			jmethodID setByteBuffer;
			jmethodID toString;
		} JavaMediaFormatMethods;
		extern JavaMediaFormatMethods MediaFormat;
	}; // namespace JMETHODS
}; // namespace JNI

class NexMediaFormat_using_jni : public NexMediaFormat {
private:
	NexMediaFormat_using_jni();
	virtual ~NexMediaFormat_using_jni();
public:
	static int createAudioFormat(const char *mimeType, int sampleRate, int channelCount, NexMediaFormat * &result);
	static int createVideoFormat(const char *mimeType, int width, int height, NexMediaFormat * &result);

	static int releaseNexMediaFormat(NexMediaFormat *mediaFormat);

	virtual int setInteger(const char *key, int value);
	virtual int getInteger(const char *key, int &result);

	virtual int setByteBuffer(const char *key, jobject byteBuffer);

	virtual int toString(char *buffer, size_t bufferSize, size_t &result);
private:
	friend class NexMediaCodec_using_jni;
	jobject jobjMediaFormat;

private:
	NexMediaFormat_using_jni(const NexMediaFormat_using_jni &);
	NexMediaFormat_using_jni &operator=(const NexMediaFormat_using_jni &);
}; // class NexMediaFormat_using_jni

}; // namespace Nex_MC

#endif //#ifndef _NEXCRAL_JNI_MEDIAFORMAT_H

