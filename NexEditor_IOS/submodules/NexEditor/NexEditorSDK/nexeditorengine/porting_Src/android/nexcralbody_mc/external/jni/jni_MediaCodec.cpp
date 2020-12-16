/******************************************************************************
* File Name        : jni_MediaCodec.cpp
* Description      : MediaCodec accessors through JNI for NexCRALBody_MC
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
#define LOCATION_TAG "jni_MediaCodec"

#include <pthread.h>

#include "./jni_MediaCodec.h"

#include "./jni_MediaFormat.h"
#include "./jni_SurfaceTexture.h"

#include "utils/utils.h"
#include "utils/jni.h"

// for differentiating between encoder and decoder in logs (U == undefined)
//#define NEX_MC_TYPE ('U' - 'D')
#define NEX_MC_TYPE 0

namespace { // (anon101)
	jint g_codecCount = -1;
}

namespace Nex_MC {
namespace JNI {
	namespace JCLASS {
		jclass MediaCodec = NULL;
		jclass BufferInfo = NULL;
		jclass CryptoInfo = NULL;
		jclass MediaCrypto = NULL;
		jclass UUID = NULL;
		jclass MediaCodecList = NULL;
		jclass MediaCodecInfo = NULL;
		jclass MediaCodecInfo_CodecCapabilities = NULL;
		jclass MediaCodecInfo_CodecProfileLevel = NULL;
		jclass MediaCodecInfo_VideoCapabilities = NULL;
	}; // namespace JCLASS

	namespace JMETHODS {
		JavaMediaCodecMethods MediaCodec = {
			  NULL // jmethodID createDecoderByType
			, NULL // jmethodID createEncoderByType
			, NULL // jmethodID createByCodecName
			, NULL // jmethodID release
			, NULL // jmethodID configure
			, NULL // jmethodID start
			, NULL // jmethodID stop
			, NULL // jmethodID flush
			, NULL // jmethodID queueInputBuffer
			, NULL // jmethodID queueSecureInputBuffer
			, NULL // jmethodID dequeueInputBuffer
			, NULL // jmethodID dequeueOutputBuffer
			, NULL // jmethodID releaseOutputBuffer
			, NULL // jmethodID getInputFormat // api 21
			, NULL // jmethodID getOutputFormat
			, NULL // jmethodID getInputBuffers
			, NULL // jmethodID getOutputBuffers
			, NULL // jmethodID setVideoScalingMode
			, NULL // jmethodID getBuffers
			, NULL // jmethodID createInputSurface // api 18
			, NULL // jmethodID signalEndOfInputStream // api 18
			, NULL // jmethodID captureFrame // only for samsung (full-seg projects)
			, NULL // jmethodID getCodecInfo // api 18
		};

		JavaMediaCodecBufferInfoMethods BufferInfo = {
			  NULL // jmethodID constructor
		};

		JavaMediaCodecCryptoInfoMethods CryptoInfo = {
			  NULL // jmethodID constructor
			, NULL // jmethodID set
		};

		JavaMediaCryptoMethods MediaCrypto = {
			  NULL // jmethodID constructor
			, NULL // jmethodID isCryptoSchemeSupported
		};

		JavaUUIDMethods UUID = {
			  NULL // jmethodID constructor
		};

		JavaMediaCodecListMethods MediaCodecList = {
			  NULL // jmethodID getCodecCount
			, NULL // jmethodID getCodecInfoAt
		};

		JavaMediaCodecInfoMethods MediaCodecInfo = {
			  NULL // jmethodID getName
			, NULL // jmethodID isEncoder
			, NULL // jmethodID getSupportedTypes
			, NULL // jmethodID getCapabilitiesForType
		};

		JavaCodecCapabilitiesMethods CodecCapabilities = {
			  NULL // jmethodID isFeatureSupported // api 19
			, NULL // jmethodID getVideoCapabilities //api 23 
		};

		JavaVideoCapabilitiesMethods VideoCapabilities = {
			  NULL // jmethodID getWidthAlignment  // api 23
			, NULL // jmethodID getHeightAlignment // api 23
		};
	}; // namespace JMETHODS

	namespace JFIELDS {
		JavaMediaCodecBufferInfoFields BufferInfo = {
			  NULL // jfieldID offset
			, NULL // jfieldID size
			, NULL // jfieldID presentationTimeUs
			, NULL // jfieldID flags
		};

		JavaMediaCodecCryptoInfoFields CryptoInfo = {
			  NULL // jfieldID iv
			, NULL // jfieldID key
			, NULL // jfieldID mode
			, NULL // jfieldID numBytesOfClearData
			, NULL // jfieldID numBytesOfEncryptedData
			, NULL // jfieldID numSubSamples
		};

		JavaMediaCodecInfo_CodecCapabilitiesFields MediaCodecInfo_CodecCapabilities = {
			  NULL // jfieldID colorFormats
			, NULL // jfieldID profileLevels
		};

		JavaMediaCodecInfo_CodecProfileLevelFields MediaCodecInfo_CodecProfileLevel = {
			  NULL // jfieldID level
			, NULL // jfieldID profile
		};
	}; // namespace JFIELDS
}; // namespace JNI

using Utils::JNI::JNIEnvWrapper;
using Utils::JNI::JNILocalRefWrapper;

NexMediaCodec_using_jni::NexMediaCodec_using_jni()
	: jobjMediaCodec(NULL)
	, inputBuffers(NULL)
	, outputBuffers(NULL)
	, inputFormat(NULL)
	, outputFormat(NULL)
	, numOutputBuffers(0)
	, name(NULL)
{
}

NexMediaCodec_using_jni::~NexMediaCodec_using_jni()
{
	if (NULL != inputBuffers)
	{
		free(inputBuffers);
		inputBuffers = NULL;
	}

	if (NULL != outputBuffers)
	{
		free(outputBuffers);
		outputBuffers = NULL;
		numOutputBuffers = 0;
	}

	if (NULL != outputFormat)
	{
		delete ((NexMediaFormat_using_jni *)outputFormat);
		outputFormat = NULL;
	}

	if (NULL != inputFormat)
	{
		delete ((NexMediaFormat_using_jni *)inputFormat);
		inputFormat = NULL;
	}

	if (NULL != name)
	{
		free(name);
	}

	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			if (NULL != jobjMediaCodec)
			{
				env->DeleteGlobalRef(jobjMediaCodec);
				jobjMediaCodec = NULL;
			}
		}
		else
		{
			MC_ERR("couldn't get JNIEnv for deleting global refs");
		}
	}
}

/*static*/ int NexMediaCodec_using_jni::createDecoderByType(const char *mimeType, NexMediaCodec * &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstrMimeType = env->NewStringUTF(mimeType);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while creating new String");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	jobject jobjMediaCodec = env->CallStaticObjectMethod(JNI::JCLASS::MediaCodec, JNI::JMETHODS::MediaCodec.createDecoderByType, jstrMimeType);

	if (NULL == jobjMediaCodec)
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jstrMimeType);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling createDecoderByType");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -3;
		}
		return -4;
	}

	NexMediaCodec_using_jni *tempResult = new NexMediaCodec_using_jni;
	tempResult->jobjMediaCodec = env->NewGlobalRef(jobjMediaCodec);

	env->DeleteLocalRef(jstrMimeType);
	env->DeleteLocalRef(jobjMediaCodec);

	result = tempResult;

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::createEncoderByType(const char *mimeType, NexMediaCodec * &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstrMimeType = env->NewStringUTF(mimeType);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while creating new String");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	jobject jobjMediaCodec = env->CallStaticObjectMethod(JNI::JCLASS::MediaCodec, JNI::JMETHODS::MediaCodec.createEncoderByType, jstrMimeType);

	if (NULL == jobjMediaCodec)
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jstrMimeType);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling createEncoderByType");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -3;
		}
		return -4;
	}

	NexMediaCodec_using_jni *tempResult = new NexMediaCodec_using_jni;
	tempResult->jobjMediaCodec = env->NewGlobalRef(jobjMediaCodec);

	env->DeleteLocalRef(jstrMimeType);
	env->DeleteLocalRef(jobjMediaCodec);

	result = tempResult;

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::createByCodecName(const char *codecName, NexMediaCodec * &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstrCodecName = env->NewStringUTF(codecName);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while creating new String");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	jobject jobjMediaCodec = env->CallStaticObjectMethod(JNI::JCLASS::MediaCodec, JNI::JMETHODS::MediaCodec.createByCodecName, jstrCodecName);

	if (NULL == jobjMediaCodec)
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jstrCodecName);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling createByCodecName");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -3;
		}
		return -4;
	}

	NexMediaCodec_using_jni *tempResult = new NexMediaCodec_using_jni;
	tempResult->jobjMediaCodec = env->NewGlobalRef(jobjMediaCodec);

	env->DeleteLocalRef(jstrCodecName);
	env->DeleteLocalRef(jobjMediaCodec);

	result = tempResult;

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::releaseNexMediaCodec(NexMediaCodec *mediaCodec)
{
	delete static_cast<NexMediaCodec_using_jni *>(mediaCodec);
	return 0;
}

int NexMediaCodec_using_jni::configure(NexMediaFormat *format, NexSurfaceTexture *surface, jobject crypto, int flags)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject jobjSurface = NULL;
	if (NULL != surface)
	{
		int ret = getSurfaceObjectFromSurfaceTexture(surface, jobjSurface);

		if (0 != ret)
		{
			MC_ERR("exception occurred while calling getSurfaceObjectFromSurfaceTexture (%d)", ret);
			return -2;
		}
	}
	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.configure, ((NexMediaFormat_using_jni *)format)->jobjMediaFormat, jobjSurface, crypto, (jint)flags);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling configure");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -3;
	}

	return 0;
}

int NexMediaCodec_using_jni::start()
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.start);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling start");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::stop()
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.stop);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling stop");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::release()
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.release);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling release");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::flush()
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.flush);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling flush");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::queueInputBuffer(size_t idx, size_t offset, size_t size, int64_t pts, int flags)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.queueInputBuffer, (jint)idx, (jint)offset, (jint)size, (jlong)pts, (jint)flags);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling queueInputBuffer");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::queueSecureInputBuffer(size_t idx, size_t offset, jobject cryptoInfo, int64_t pts, int flags)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.queueSecureInputBuffer, (jint)idx, (jint)offset, cryptoInfo, (jlong)pts, (jint)flags);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling queueSecureInputBuffer");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::dequeueInputBuffer(int64_t timeout, long &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jint tempResult = env->CallIntMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.dequeueInputBuffer, (jlong)timeout);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling dequeueInputBuffer");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}
	result = (long)tempResult;

	return 0;
}

int NexMediaCodec_using_jni::dequeueOutputBuffer(jobject bufferInfo, int64_t timeout, long &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jint tempResult = env->CallIntMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.dequeueOutputBuffer, bufferInfo, (jlong)timeout);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling dequeueOutputBuffer");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}
	result = (long)tempResult;

	return 0;
}

int NexMediaCodec_using_jni::releaseOutputBuffer(size_t idx, bool render)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.releaseOutputBuffer, (jint)idx, (jboolean)render);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling releaseOutputBuffer");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::getInputFormat(NexMediaFormat * &result)
{
	JNIEnvWrapper env;
	jobject jobjInputFormat = NULL;

	if (env == NULL)
		return -1;

	if (JNI::JMETHODS::MediaCodec.getInputFormat)
		jobjInputFormat = env->CallObjectMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.getInputFormat);
	else
	{
		return -1;
	}

	if (NULL == jobjInputFormat)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling jobjInputFormat");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	if (NULL != inputFormat)
	{
		delete ((NexMediaFormat_using_jni *)inputFormat);
		inputFormat = NULL;
	}

	inputFormat = new NexMediaFormat_using_jni;
	((NexMediaFormat_using_jni *)inputFormat)->jobjMediaFormat = env->NewGlobalRef(jobjInputFormat);
	env->DeleteLocalRef(jobjInputFormat);

	result = inputFormat;

	return 0;
}

int NexMediaCodec_using_jni::getOutputFormat(NexMediaFormat * &result)
{
	if (NULL != outputFormat)
	{
		result = outputFormat;
		return 0;
	}

	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject jobjOutputFormat = env->CallObjectMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.getOutputFormat);


	if (NULL == jobjOutputFormat)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling getOutputFormat");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	if (NULL != outputFormat)
	{
		delete ((NexMediaFormat_using_jni *)outputFormat);
		outputFormat = NULL;
	}

	outputFormat = new NexMediaFormat_using_jni;
	((NexMediaFormat_using_jni *)outputFormat)->jobjMediaFormat = env->NewGlobalRef(jobjOutputFormat);
	env->DeleteLocalRef(jobjOutputFormat);

	result = outputFormat;

	return 0;
}

int NexMediaCodec_using_jni::regetOutputFormat(NexMediaFormat * &result)
{
	if (NULL != outputFormat)
	{
		delete ((NexMediaFormat_using_jni *)outputFormat);
		outputFormat = NULL;
	}

	return getOutputFormat(result);
}

int NexMediaCodec_using_jni::getInputBuffers(void ** &result)
{
	result = NULL;
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobjectArray jArray = (jobjectArray)env->CallObjectMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.getInputBuffers);
	if (NULL == jArray)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling getInputBuffers");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	jsize arrayLength = env->GetArrayLength(jArray);
	STATUST(0, 0, Log::FLOW, Log::VERBOSE, "%zu", arrayLength);
	if (0 < arrayLength)
	{
		if (NULL != inputBuffers)
		{
			free(inputBuffers);
		}

		inputBuffers = (void **)malloc(sizeof(void*) * arrayLength);
		if (NULL == inputBuffers)
		{
			MC_ERR("malloc failed!");
			return -6;
		}
		STATUST(0, 0, Log::FLOW, Log::INFO, "%p = malloc(%d)", inputBuffers, sizeof(void*) * arrayLength);

		jsize i = 0;
		for (i = 0; i < arrayLength; ++i)
		{
			jobject jBuffer = env->GetObjectArrayElement(jArray, i);
			if (NULL == jBuffer)
			{
				break;
			}
			void *addr = env->GetDirectBufferAddress(jBuffer);
			env->DeleteLocalRef(jBuffer);
			STATUST(0, 0, Log::FLOW, Log::VERBOSE, "[%zu](%p)", i, addr);
			inputBuffers[i] = addr;
		}

		if (i < arrayLength)
		{
			MC_ERR("failed at idx(%zu)", i);
			env->DeleteLocalRef(jArray);
			return -4;
		}
	}
	else
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jArray);
		return -5;
	}

	env->DeleteLocalRef(jArray);

	result = inputBuffers;
	return 0;
}

int NexMediaCodec_using_jni::getOutputBuffers(bool withoutPointers, void ** &result)
{
	result = NULL;
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobjectArray jArray = (jobjectArray)env->CallObjectMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.getOutputBuffers);
	if (NULL == jArray)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling getOutputBuffers");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	jsize arrayLength = env->GetArrayLength(jArray);
	STATUST(0, 0, Log::FLOW, Log::VERBOSE, "%zu", arrayLength);
	if (0 < arrayLength)
	{
		if (NULL != outputBuffers)
		{
			free(outputBuffers);
		}

		numOutputBuffers = arrayLength;
		outputBuffers = (void **)malloc(sizeof(void*) * numOutputBuffers);
		if (NULL == outputBuffers)
		{
			MC_ERR("malloc failed!");
			return -6;
		}
		STATUST(0, 0, Log::FLOW, Log::INFO, "%p = malloc(%d)", outputBuffers, sizeof(void*) * numOutputBuffers);

		if (!withoutPointers)
		{
			jsize i = 0;
			for (i = 0; i < numOutputBuffers; ++i)
			{
				jobject jBuffer = env->GetObjectArrayElement(jArray, i);
				if (NULL == jBuffer)
				{
					break;
				}
				void *addr = env->GetDirectBufferAddress(jBuffer);
				env->DeleteLocalRef(jBuffer);
				STATUST(0, 0, Log::FLOW, Log::VERBOSE, "[%zu](%p)", i, addr);
				outputBuffers[i] = addr;
			}

			if (i < arrayLength)
			{
				MC_ERR("failed at idx(%zu)", i);
				env->DeleteLocalRef(jArray);
				return -4;
			}
		}
	}
	else
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jArray);
		return -5;
	}

	env->DeleteLocalRef(jArray);

	result = outputBuffers;
	return 0;
}

int NexMediaCodec_using_jni::getNumOutputBuffers()
{
	return numOutputBuffers;
}

int NexMediaCodec_using_jni::setVideoScalingMode(int mode)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.setVideoScalingMode, (jint)mode);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling setVideoScalingMode");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	return 0;
}

int NexMediaCodec_using_jni::createInputSurface(jobject &result)
{
	result = NULL;

	if (NULL == JNI::JMETHODS::MediaCodec.createInputSurface)
	{
		MC_WARN("createInputSurface not supported");
		return 0;
	}

	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject surface = env->CallObjectMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.createInputSurface);

	if (NULL == surface)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling createInputSurface");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	result = env->NewGlobalRef(surface);
	env->DeleteLocalRef(surface);

	return 0;
}

int NexMediaCodec_using_jni::signalEndOfInputStream()
{
	if (NULL == JNI::JMETHODS::MediaCodec.signalEndOfInputStream)
	{
		MC_WARN("signalEndOfInputStream not supported");
		return 0;
	}

	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.signalEndOfInputStream);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling signalEndOfInputStream");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}
	return 0;
}

int NexMediaCodec_using_jni::captureFrame(bool thumbnailMode)
{
	if (NULL == JNI::JMETHODS::MediaCodec.captureFrame)
	{
		MC_WARN("captureFrame not supported");
		return 0;
	}

	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->CallVoidMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.captureFrame, (jboolean)thumbnailMode);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling captureFrame");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}
	return 0;
}

int NexMediaCodec_using_jni::getName(const char * &result)
{
	if (NULL != name)
	{
		result = name;
		return 0;
	}

	if (NULL == JNI::JMETHODS::MediaCodec.getCodecInfo)
	{
		MC_WARN("getCodecInfo not supported");
		result = "";
		return 0;
	}

	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject mediaCodecInfo = env->CallObjectMethod(jobjMediaCodec, JNI::JMETHODS::MediaCodec.getCodecInfo);

	if (NULL == mediaCodecInfo)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling getCodecInfo");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}

		MC_WARN("no codec info was returned");
		result = "";
		return 0;
	}

	JNILocalRefWrapper mediaCodecInfo_wrapped(env.get(), mediaCodecInfo);

	jstring jstrName = (jstring)env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getName);
	if (NULL == jstrName)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling getName");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -15;
		}

		MC_WARN("no name was returned");
		result = "";
		return 0;
	}
	JNILocalRefWrapper name_wrapped(env.get(), jstrName);

	const char *cstrName = env->GetStringUTFChars(jstrName, 0);

	name = strdup(cstrName);
	result = name;

	env->ReleaseStringUTFChars(jstrName, cstrName);

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::createBufferInfo(jobject &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject bufferInfo = env->NewObject(JNI::JCLASS::BufferInfo, JNI::JMETHODS::BufferInfo.constructor);

	if (NULL == bufferInfo)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling BufferInfo constructor");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	result = env->NewGlobalRef(bufferInfo);

	env->DeleteLocalRef(bufferInfo);

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::releaseBufferInfo(jobject bufferInfo)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->DeleteGlobalRef(bufferInfo);

	return 0;
}


/*static*/ int NexMediaCodec_using_jni::createCryptoInfo(jobject &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject cryptoInfo = env->NewObject(JNI::JCLASS::CryptoInfo, JNI::JMETHODS::CryptoInfo.constructor);

	if (NULL == cryptoInfo)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling CryptoInfo constructor");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	result = env->NewGlobalRef(cryptoInfo);

	env->DeleteLocalRef(cryptoInfo);

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::releaseCryptoInfo(jobject cryptoInfo)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->DeleteGlobalRef(cryptoInfo);

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::setCryptoInfo(jobject cryptoInfo, int newNumSubSamples, int *newNumBytesOfClearData, int *newNumBytesOfEncryptedData, uint8_t *newKey, uint8_t *newIV, int newMode)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jintArray jintarrayNewNumBytesOfClearData = NULL;
	jintArray jintarrayNewNumBytesOfEncryptedData = NULL;
	jbyteArray jbytearrayNewKey = NULL;
	jbyteArray jbytearrayNewIV = NULL;

	if (NULL != newNumBytesOfClearData)
	{
		if (0 < newNumSubSamples)
		{
			jintarrayNewNumBytesOfClearData = env->NewIntArray(newNumSubSamples);

			if (NULL == jintarrayNewNumBytesOfClearData)
			{
				MC_ERR("failed");
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while creating new int array");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -2;
				}
				return -3;
			}
			env->SetIntArrayRegion(jintarrayNewNumBytesOfClearData, 0, newNumSubSamples, newNumBytesOfClearData);
		}
	}

	if (NULL != newNumBytesOfEncryptedData)
	{
		if (0 < newNumSubSamples)
		{
			jintarrayNewNumBytesOfEncryptedData = env->NewIntArray(newNumSubSamples);

			if (NULL == jintarrayNewNumBytesOfEncryptedData)
			{
				MC_ERR("failed");
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while creating new int array");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -4;
				}
				return -5;
			}
			env->SetIntArrayRegion(jintarrayNewNumBytesOfEncryptedData, 0, newNumSubSamples, newNumBytesOfEncryptedData);
		}
	}

	if (NULL != newKey)
	{
		jbytearrayNewKey = env->NewByteArray(16);

		if (NULL == jbytearrayNewKey)
		{
			MC_ERR("failed");
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while creating new byte array");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -6;
			}
			return -7;
		}
		env->SetByteArrayRegion(jbytearrayNewKey, 0, 16, (jbyte *)newKey);
	}

	if (NULL != newIV)
	{
		jbytearrayNewIV = env->NewByteArray(16);

		if (NULL == jbytearrayNewIV)
		{
			MC_ERR("failed");
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while creating new byte array");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -8;
			}
			return -9;
		}
		env->SetByteArrayRegion(jbytearrayNewIV, 0, 16, (jbyte *)newIV);
	}

	env->CallVoidMethod(cryptoInfo, JNI::JMETHODS::CryptoInfo.set
		, (jint)newNumSubSamples
		, jintarrayNewNumBytesOfClearData
		, jintarrayNewNumBytesOfEncryptedData
		, jbytearrayNewKey
		, jbytearrayNewIV
		, (jint)newMode
		);
	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling set");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -10;
	}

	if (NULL != jintarrayNewNumBytesOfClearData)
	{
		env->DeleteLocalRef(jintarrayNewNumBytesOfClearData);
	}

	if (NULL != jintarrayNewNumBytesOfEncryptedData)
	{
		env->DeleteLocalRef(jintarrayNewNumBytesOfEncryptedData);
	}

	if (NULL != jbytearrayNewKey)
	{
		env->DeleteLocalRef(jbytearrayNewKey);
	}

	if (NULL != jbytearrayNewIV)
	{
		env->DeleteLocalRef(jbytearrayNewIV);
	}

	return 0;
}


/*static*/ int NexMediaCodec_using_jni::createMediaCrypto(int64_t msb, int64_t lsb, uint8_t *initData, int initDataLength, jobject &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jobject UUID = env->NewObject(JNI::JCLASS::UUID, JNI::JMETHODS::UUID.constructor, (jlong)msb, (jlong)lsb);
	if (NULL == UUID)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling UUID constructor");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	jboolean supported = env->CallStaticBooleanMethod(JNI::JCLASS::MediaCrypto, JNI::JMETHODS::MediaCrypto.isCryptoSchemeSupported, UUID);
	if (JNI_FALSE == supported)
	{
		MC_ERR("crypto scheme not supported");
		env->DeleteLocalRef(UUID);
		return -4;
	}

	jbyteArray jbytearrayInitData = NULL;
	if (NULL != initData)
	{
		if (0 < initDataLength)
		{
			jbytearrayInitData = env->NewByteArray(initDataLength);

			if (NULL == jbytearrayInitData)
			{
				MC_ERR("failed");
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while creating new byte array");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -5;
				}
				env->DeleteLocalRef(UUID);
				return -6;
			}

			env->SetByteArrayRegion(jbytearrayInitData, 0, initDataLength, (jbyte *)initData);
		}
	}

	jobject mediaCrypto = env->NewObject(JNI::JCLASS::MediaCrypto, JNI::JMETHODS::MediaCrypto.constructor, UUID, jbytearrayInitData);

	if (NULL == mediaCrypto)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while creating new byte array");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -7;
		}
		env->DeleteLocalRef(UUID);
		if (NULL != jbytearrayInitData)
		{
			env->DeleteLocalRef(jbytearrayInitData);
		}
		return -8;
	}

	result = env->NewGlobalRef(mediaCrypto);

	env->DeleteLocalRef(mediaCrypto);
	env->DeleteLocalRef(UUID);

	if (NULL != jbytearrayInitData)
	{
		env->DeleteLocalRef(jbytearrayInitData);
	}

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::releaseMediaCrypto(jobject mediaCrypto)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	env->DeleteGlobalRef(mediaCrypto);

	return 0;
}

/*static*/ int NexMediaCodec_using_jni::getSupportedInputColorFormats(const char *type, int *&supportedList)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	{
		jstring reqType = env->NewStringUTF(type);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while creating new String");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		JNILocalRefWrapper reqType_wrapped(env.get(), reqType);

		if (-1 == g_codecCount)
		{
			g_codecCount = env->CallStaticIntMethod(JNI::JCLASS::MediaCodecList, JNI::JMETHODS::MediaCodecList.getCodecCount);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting codec count");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -3;
			}
		}

		for (jint i = 0; i < g_codecCount; ++i)
		{
			jobject mediaCodecInfo = env->CallStaticObjectMethod(JNI::JCLASS::MediaCodecList, JNI::JMETHODS::MediaCodecList.getCodecInfoAt, i);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting codec info at %zu", (size_t)i);
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -4;
			}
			if (NULL == mediaCodecInfo)
			{
				continue;
			}
			JNILocalRefWrapper mediaCodecInfo_wrapped(env.get(), mediaCodecInfo);

			jboolean isEncoder = env->CallBooleanMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.isEncoder);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while checking if encoder");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -5;
			}
			if (JNI_FALSE == isEncoder)
			{
				continue;
			}

			jobject supportedTypes = env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getSupportedTypes);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting supported types");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -6;
			}
			if (NULL == supportedTypes)
			{
				continue;
			}
			JNILocalRefWrapper supportedTypes_wrapped(env.get(), supportedTypes);

			jsize numSupportedTypes = env->GetArrayLength((jobjectArray)supportedTypes);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting supported types count");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -7;
			}
			STATUST(0, 0, Log::FLOW, Log::VERBOSE, "%zu", numSupportedTypes);

			for (jsize j = 0; j < numSupportedTypes; ++j)
			{
				jstring supportedType = (jstring)env->GetObjectArrayElement((jobjectArray)supportedTypes, j);
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while getting supported type string");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -8;
				}
				if (NULL == supportedType)
				{
					break;
				}
				JNILocalRefWrapper supportedType_wrapped(env.get(), supportedType);

				jboolean equals = env->CallBooleanMethod(supportedType, Nex_MC::Utils::JNI::JMETHODS::String.equals, reqType);
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while comparing two strings");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -9;
				}

				if (JNI_TRUE == equals)
				{
					jobject codecCapabilities = env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getCapabilitiesForType, reqType);
					if (JNI_TRUE == env->ExceptionCheck())
					{
						MC_ERR("exception occurred while getting codec capabilities");
						JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
						return -10;
					}
					JNILocalRefWrapper codecCapabilities_wrapped(env.get(), codecCapabilities);

					jobject supportedColorFormats = (jobject)env->GetObjectField(codecCapabilities, JNI::JFIELDS::MediaCodecInfo_CodecCapabilities.colorFormats);
					if (JNI_TRUE == env->ExceptionCheck())
					{
						MC_ERR("exception occurred while getting supported color formats array");
						JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
						return -11;
					}
					JNILocalRefWrapper supportedColorFormats_wrapped(env.get(), supportedColorFormats);

					jint *native_supportedColorFormats = env->GetIntArrayElements(*reinterpret_cast<jintArray *>(&supportedColorFormats), NULL);
					if (JNI_TRUE == env->ExceptionCheck())
					{
						MC_ERR("exception occurred while getting array elements");
						JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
						env->ReleaseIntArrayElements(*reinterpret_cast<jintArray *>(&supportedColorFormats), native_supportedColorFormats, JNI_ABORT);
						if (JNI_TRUE == env->ExceptionCheck())
						{
							MC_ERR("exception occurred while releasing array elements");
							JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
							return -17;
						}
						return -12;
					}

					jsize numSupportedColorFormats = env->GetArrayLength(*reinterpret_cast<jintArray *>(&supportedColorFormats));
					if (JNI_TRUE == env->ExceptionCheck())
					{
						MC_ERR("exception occurred while getting array size");
						JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
						return -14;
					}

					if (NULL != supportedList)
					{
						free(supportedList);
					}
					supportedList = (int *)malloc((numSupportedColorFormats + 1) * sizeof(int));
					if (NULL == supportedList)
					{
						MC_ERR("malloc failed!");
						return -21;
					}
					STATUST(1, 0, Log::FLOW, Log::INFO, "%p = malloc(%d)", supportedList, (numSupportedColorFormats + 1) * sizeof(int));

					for (int k = 0; k < numSupportedColorFormats; ++k)
					{
						supportedList[k] = native_supportedColorFormats[k];
						STATUST(1, 0, Log::FLOW, Log::INFO, "%d th colorformat = %x", k, supportedList[k] );
					}
					supportedList[numSupportedColorFormats] = -1;

					env->ReleaseIntArrayElements(*reinterpret_cast<jintArray *>(&supportedColorFormats), native_supportedColorFormats, JNI_ABORT);
					if (JNI_TRUE == env->ExceptionCheck())
					{
						MC_ERR("exception occurred while releasing array elements");
						JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
						return -14;
					}

					// DEBUG:
					jstring name = (jstring)env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getName);
					if (NULL == name)
					{
						MC_ERR("failed");
						if (JNI_TRUE == env->ExceptionCheck())
						{
							MC_ERR("exception occurred while calling getName");
							JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
							return -15;
						}
						return -16;
					}
					JNILocalRefWrapper name_wrapped(env.get(), name);

					const char *cstrName = env->GetStringUTFChars(name, 0);
//					MC_INFO("found encoder(%s) for type(%s)", cstrName, type);
					env->ReleaseStringUTFChars(name, cstrName);
					// :GUBED

					return 0;
				}
			}
		}
	}

	return 0;
}

/* static */ int NexMediaCodec_using_jni::findPreferredCodec(const char *type, bool encoder, CodecClassRequest wantedCodecClass, char *&preferredCodecName, int **profileLevels, int *supportedFeature, int *piWidthAlignment, int *piHeightAlignment)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	if (profileLevels ) {
		if ( NULL != *profileLevels)
			free(*profileLevels);

		*profileLevels = (int *)malloc(sizeof(int));
		(**profileLevels) = 0;
	}

	int bPreferredCodec = 0, nRetryIndex = -1;

	{
		jstring reqType = env->NewStringUTF(type);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while creating new String");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		JNILocalRefWrapper reqType_wrapped(env.get(), reqType);

		if (-1 == g_codecCount)
		{
			g_codecCount = env->CallStaticIntMethod(JNI::JCLASS::MediaCodecList, JNI::JMETHODS::MediaCodecList.getCodecCount);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting codec count");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -3;
			}
		}

		for (jint i = 0; i < g_codecCount; ++i)
		{
			jobject mediaCodecInfo = env->CallStaticObjectMethod(JNI::JCLASS::MediaCodecList, JNI::JMETHODS::MediaCodecList.getCodecInfoAt, i);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting codec info at %zu", (size_t)i);
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -4;
			}
			if (NULL == mediaCodecInfo)
			{
				continue;
			}
			JNILocalRefWrapper mediaCodecInfo_wrapped(env.get(), mediaCodecInfo);

			jboolean isEncoder = env->CallBooleanMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.isEncoder);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while checking if encoder");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -5;
			}
			if ((JNI_TRUE == isEncoder) != encoder)
			{
				continue;
			}

			jobject supportedTypes = env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getSupportedTypes);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting supported types");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -6;
			}
			if (NULL == supportedTypes)
			{
				continue;
			}
			JNILocalRefWrapper supportedTypes_wrapped(env.get(), supportedTypes);

			jsize numSupportedTypes = env->GetArrayLength((jobjectArray)supportedTypes);
			if (JNI_TRUE == env->ExceptionCheck())
			{
				MC_ERR("exception occurred while getting supported types count");
				JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
				return -7;
			}
			STATUST(0, 0, Log::FLOW, Log::VERBOSE, "%zu", numSupportedTypes);

			for (jsize j = 0; j < numSupportedTypes; ++j)
			{
				jstring supportedType = (jstring)env->GetObjectArrayElement((jobjectArray)supportedTypes, j);
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while getting supported type string");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -8;
				}
				if (NULL == supportedType)
				{
					break;
				}
				JNILocalRefWrapper supportedType_wrapped(env.get(), supportedType);

				jboolean equals = env->CallBooleanMethod(supportedType, Nex_MC::Utils::JNI::JMETHODS::String.equals, reqType);
				if (JNI_TRUE == env->ExceptionCheck())
				{
					MC_ERR("exception occurred while comparing two strings");
					JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
					return -9;
				}

				if (JNI_TRUE == equals)
				{
					jstring codecName = (jstring)env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getName);
					if (JNI_TRUE == env->ExceptionCheck())
					{
						MC_ERR("exception occurred while getting codec name");
						JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
						return -10;
					}
					JNILocalRefWrapper codecName_wrapped(env.get(), codecName);

					const char *cstr = env->GetStringUTFChars(codecName, NULL);

					STATUST(0, 0, Log::FLOW, Log::VERBOSE, "codecName(%s)", cstr);

					//if (!strncmp("OMX.", cstr, sizeof("OMX.") - 1))
					{
						int codecClass = 0; // this will be 1, 2, or 4
						if (NULL != (strstr(cstr, ".google.")))
						{
							codecClass = (int)GOOGLE_SW;
						}
						else if (NULL != (strstr(cstr, ".sw.")))
						{
							codecClass = (int)OTHER_SW;
						}
						else if (
									0 == strncmp("OMX.ittiam.", cstr, sizeof("OMX.ittiam.") - 1) // ittiam is really horrible sw....
								 || 0 == strncmp("OMX.ARICENT.", cstr, sizeof("OMX.ARICENT.") - 1) // misbehaving sw codec from aricent....
						        )
						{
							if (
									0 != strncmp("OMX.ARICENT.", cstr, sizeof("OMX.ARICENT.") - 1) // blacklisted aricent because it behaves weird
							   )
							codecClass = (int)OTHER_SW;
						}
						else
						{
							if (
								( !encoder || (
									0 != strncmp("OMX.SEC.AVC.Encoder", cstr, sizeof("OMX.SEC.AVC.Encoder") - 1) // blacklisted for Galaxy S3
								) )
							   )
							{
								codecClass = (int)HW;
							}
						}

						bPreferredCodec = 0;
						if (0 != (((int)wantedCodecClass) & codecClass))
						{
							bPreferredCodec = 1;

							if ( 18 > Nex_MC::Utils::GetBuildVersionSDK() 
								&& Utils::Manufacturer::SAMSUNG == Utils::GetManufacturer() )
							{
								// In this case, we want a component whose name is "OMX.SEC.mp3.dec com".
								if ( 0 == strncmp("OMX.SEC.MP3.Decoder", cstr, sizeof("OMX.SEC.MP3.Decoder")-1 ) )
								{
									bPreferredCodec = 0;
									nRetryIndex = i;
								}
							}
						}

						if ( bPreferredCodec )
						{
							size_t nameLen = strlen(cstr);
							if (NULL != preferredCodecName)
							{
								free(preferredCodecName);
							}
							preferredCodecName = (char *)malloc(nameLen+1);
							if (NULL == preferredCodecName)
							{
								MC_ERR("malloc failed!");
								return -21;
							}
							STATUST(encoder ? 1 : 0, 0, Log::FLOW, Log::INFO, "%p = malloc(%d)", preferredCodecName, nameLen+1);
							strncpy(preferredCodecName, cstr, nameLen+1);
							env->ReleaseStringUTFChars(codecName, cstr);

							if (NULL != profileLevels)
							{
								jobject codecCapabilities = env->CallObjectMethod(mediaCodecInfo, JNI::JMETHODS::MediaCodecInfo.getCapabilitiesForType, reqType);
								if (JNI_TRUE == env->ExceptionCheck())
								{
									MC_ERR("exception occurred while getting codec capabilities");
									JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
									return -10;
								}
								JNILocalRefWrapper codecCapabilities_wrapped(env.get(), codecCapabilities);


								jstring reqFeat = env->NewStringUTF("adaptive-playback");

								if (JNI_TRUE == env->ExceptionCheck())
								{
									MC_ERR("exception occurred while creating new String");
									JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
									return -2;
								}

								if(JNI::JMETHODS::CodecCapabilities.isFeatureSupported)
								{
									JNILocalRefWrapper reqFeat_wrapped(env.get(), reqFeat);
									jboolean isAdaptivePlayback = env->CallBooleanMethod(codecCapabilities, JNI::JMETHODS::CodecCapabilities.isFeatureSupported, reqFeat);
									if (JNI_TRUE == env->ExceptionCheck())
									{
										MC_ERR("exception occurred while checking if FeatureSupported");
										JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
										return -23;
									}
									if (JNI_TRUE == isAdaptivePlayback && supportedFeature)
									{
										*supportedFeature |= JNI::MC_Feat_AdaptivePlayback;
									}
								}
								
								jobject supportedProfileLevels = (jobject)env->GetObjectField(codecCapabilities, JNI::JFIELDS::MediaCodecInfo_CodecCapabilities.profileLevels);
								if (JNI_TRUE == env->ExceptionCheck())
								{
									MC_ERR("exception occurred while getting supported profiles & levels array");
									JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
									return -11;
								}
								JNILocalRefWrapper supportedProfileLevels_wrapped(env.get(), supportedProfileLevels);

								jsize numSupportedProfileLevels = env->GetArrayLength(*reinterpret_cast<jobjectArray *>(&supportedProfileLevels));

								if (NULL != *profileLevels)
								{
									free(*profileLevels);
								}

								*profileLevels = (int *)malloc(sizeof(int) * (numSupportedProfileLevels * 2 + 1));
								if (NULL == *profileLevels)
								{
									MC_ERR("malloc failed!");
									return -22;
								}
								STATUST(encoder ? 1 : 0, 0, Log::FLOW, Log::INFO, "%p = malloc(%d)", *profileLevels, sizeof(int) * (numSupportedProfileLevels * 2 + 1));
								(*profileLevels)[0] = numSupportedProfileLevels;

								for (int k = 0; k < numSupportedProfileLevels; ++k)
								{
									jobject supportedProfileLevel = env->GetObjectArrayElement(*reinterpret_cast<jobjectArray *>(&supportedProfileLevels), k);

									if (JNI_TRUE == env->ExceptionCheck())
									{
										MC_ERR("exception occurred while getting an object element at (%d) from array", k);
										JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
										return -12;
									}

									JNILocalRefWrapper supportedProfileLevel_wrapped(env.get(), supportedProfileLevel);

									(*profileLevels)[k*2 + 1] = (int)env->GetIntField(supportedProfileLevel, JNI::JFIELDS::MediaCodecInfo_CodecProfileLevel.profile);
									(*profileLevels)[k*2 + 2] = (int)env->GetIntField(supportedProfileLevel, JNI::JFIELDS::MediaCodecInfo_CodecProfileLevel.level);
								}

								if (23 <= Nex_MC::Utils::GetBuildVersionSDK() && piWidthAlignment && piHeightAlignment)
								{
									jobject videoCapabilities = env->CallObjectMethod(codecCapabilities, JNI::JMETHODS::CodecCapabilities.getVideoCapabilities);
									if (JNI_TRUE == env->ExceptionCheck())
									{
										MC_ERR("exception occurred while getting video capabilities");
										JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
										return -13;
									}

									JNILocalRefWrapper videoCapabilities_wrapped(env.get(), videoCapabilities);

									if (JNI::JMETHODS::VideoCapabilities.getWidthAlignment && JNI::JMETHODS::VideoCapabilities.getHeightAlignment)
									{
										int iWidthAlignment = (int)env->CallIntMethod(videoCapabilities, JNI::JMETHODS::VideoCapabilities.getWidthAlignment);
										if (JNI_TRUE == env->ExceptionCheck())
										{
											MC_ERR("exception occurred while checking if getWidthAlignment");
											JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
											return -14;
										}

										jint iHeightAlignment = (int)env->CallIntMethod(videoCapabilities, JNI::JMETHODS::VideoCapabilities.getHeightAlignment);
										if (JNI_TRUE == env->ExceptionCheck())
										{
											MC_ERR("exception occurred while checking if getHeightAlignment");
											JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
											return -15;
										}

										*piWidthAlignment = iWidthAlignment;
										*piHeightAlignment = iHeightAlignment;
									}
								}
							}

							return 0;
						} // end of "if (0 != (((int)wantedCodecClass) & codecClass))"
					} // end of "if (!strncmp("OMX.", cstr, sizeof("OMX.") - 1))"

					env->ReleaseStringUTFChars(codecName, cstr);
					
				} // end of "if (supportedMime Type == request MimeType)"
			} // end of "for (jsize j = 0; j < numSupportedTypes; ++j)"

			if ( ( i == g_codecCount-1 ) && ( -1 != nRetryIndex ) )
			{
				i = nRetryIndex-1;
			}
		} // for (jint i = 0; i < g_codecCount; ++i)
	}

	return 0;
}
}; // namespace Nex_MC

namespace { // anon1
	using Nex_MC::Utils::JNI::GetMethodID;
	using Nex_MC::Utils::JNI::GetFieldID;
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
				jclass mediaCodecClass = env->FindClass("android/media/MediaCodec");

				if (NULL == mediaCodecClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodec\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::MediaCodec = (jclass)env->NewGlobalRef(mediaCodecClass);

				/* don't support full-seg any more.
				Nex_MC::JNI::JMETHODS::MediaCodec.captureFrame = GetMethodID(env, mediaCodecClass
					, "captureFrame", "(Z)V", false);
				*/

				Nex_MC::JNI::JMETHODS::MediaCodec.configure = GetMethodID(env, mediaCodecClass
					, "configure", "(Landroid/media/MediaFormat;Landroid/view/Surface;Landroid/media/MediaCrypto;I)V");
				Nex_MC::JNI::JMETHODS::MediaCodec.createDecoderByType = GetStaticMethodID(env, mediaCodecClass
					, "createDecoderByType", "(Ljava/lang/String;)Landroid/media/MediaCodec;");
				Nex_MC::JNI::JMETHODS::MediaCodec.createEncoderByType = GetStaticMethodID(env, mediaCodecClass
					, "createEncoderByType", "(Ljava/lang/String;)Landroid/media/MediaCodec;");
				Nex_MC::JNI::JMETHODS::MediaCodec.createByCodecName = GetStaticMethodID(env, mediaCodecClass
					, "createByCodecName", "(Ljava/lang/String;)Landroid/media/MediaCodec;");
				Nex_MC::JNI::JMETHODS::MediaCodec.dequeueInputBuffer = GetMethodID(env, mediaCodecClass
					, "dequeueInputBuffer", "(J)I");
				Nex_MC::JNI::JMETHODS::MediaCodec.dequeueOutputBuffer = GetMethodID(env, mediaCodecClass
					, "dequeueOutputBuffer", "(Landroid/media/MediaCodec$BufferInfo;J)I");
				Nex_MC::JNI::JMETHODS::MediaCodec.flush = GetMethodID(env, mediaCodecClass
					, "flush", "()V");
				Nex_MC::JNI::JMETHODS::MediaCodec.getBuffers = GetMethodID(env, mediaCodecClass
					, "getBuffers", "(Z)[Ljava/nio/ByteBuffer;");
				Nex_MC::JNI::JMETHODS::MediaCodec.getInputBuffers = GetMethodID(env, mediaCodecClass
					, "getInputBuffers", "()[Ljava/nio/ByteBuffer;");
				Nex_MC::JNI::JMETHODS::MediaCodec.getOutputBuffers = GetMethodID(env, mediaCodecClass
					, "getOutputBuffers", "()[Ljava/nio/ByteBuffer;");

				if (21 <= Nex_MC::Utils::GetBuildVersionSDK())
					Nex_MC::JNI::JMETHODS::MediaCodec.getInputFormat = GetMethodID(env, mediaCodecClass
						, "getInputFormat", "()Landroid/media/MediaFormat;");

				Nex_MC::JNI::JMETHODS::MediaCodec.getOutputFormat = GetMethodID(env, mediaCodecClass
					, "getOutputFormat", "()Landroid/media/MediaFormat;");
				Nex_MC::JNI::JMETHODS::MediaCodec.queueInputBuffer = GetMethodID(env, mediaCodecClass
					, "queueInputBuffer", "(IIIJI)V");
				Nex_MC::JNI::JMETHODS::MediaCodec.queueSecureInputBuffer = GetMethodID(env, mediaCodecClass
					, "queueSecureInputBuffer", "(IILandroid/media/MediaCodec$CryptoInfo;JI)V");
				Nex_MC::JNI::JMETHODS::MediaCodec.release = GetMethodID(env, mediaCodecClass
					, "release", "()V");
				Nex_MC::JNI::JMETHODS::MediaCodec.releaseOutputBuffer = GetMethodID(env, mediaCodecClass
					, "releaseOutputBuffer", "(IZ)V");
				Nex_MC::JNI::JMETHODS::MediaCodec.setVideoScalingMode = GetMethodID(env, mediaCodecClass
					, "setVideoScalingMode", "(I)V");
				Nex_MC::JNI::JMETHODS::MediaCodec.start = GetMethodID(env, mediaCodecClass
					, "start", "()V");
				Nex_MC::JNI::JMETHODS::MediaCodec.stop = GetMethodID(env, mediaCodecClass
					, "stop", "()V");

				if (18 <= Nex_MC::Utils::GetBuildVersionSDK())
					Nex_MC::JNI::JMETHODS::MediaCodec.createInputSurface = GetMethodID(env, mediaCodecClass
						, "createInputSurface", "()Landroid/view/Surface;", false);

				if (18 <= Nex_MC::Utils::GetBuildVersionSDK())
					Nex_MC::JNI::JMETHODS::MediaCodec.signalEndOfInputStream = GetMethodID(env, mediaCodecClass
						, "signalEndOfInputStream", "()V", false);

				if (18 <= Nex_MC::Utils::GetBuildVersionSDK())
					Nex_MC::JNI::JMETHODS::MediaCodec.getCodecInfo = GetMethodID(env, mediaCodecClass
						, "getCodecInfo", "()Landroid/media/MediaCodecInfo;", false);


				jclass mediaCodecBufferInfoClass = env->FindClass("android/media/MediaCodec$BufferInfo");

				if (NULL == mediaCodecBufferInfoClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodec$BufferInfo\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::BufferInfo = (jclass)env->NewGlobalRef(mediaCodecBufferInfoClass);
				Nex_MC::JNI::JMETHODS::BufferInfo.constructor = GetMethodID(env, mediaCodecBufferInfoClass
					, "<init>", "()V");
				Nex_MC::JNI::JFIELDS::BufferInfo.offset = GetFieldID(env, mediaCodecBufferInfoClass
					, "offset", "I");
				Nex_MC::JNI::JFIELDS::BufferInfo.size = GetFieldID(env, mediaCodecBufferInfoClass
					, "size", "I");
				Nex_MC::JNI::JFIELDS::BufferInfo.presentationTimeUs = GetFieldID(env, mediaCodecBufferInfoClass
					, "presentationTimeUs", "J");
				Nex_MC::JNI::JFIELDS::BufferInfo.flags = GetFieldID(env, mediaCodecBufferInfoClass
					, "flags", "I");

				jclass mediaCodecCryptoInfoClass = env->FindClass("android/media/MediaCodec$CryptoInfo");

				if (NULL == mediaCodecCryptoInfoClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodec$CryptoInfo\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::CryptoInfo = (jclass)env->NewGlobalRef(mediaCodecCryptoInfoClass);
				Nex_MC::JNI::JMETHODS::CryptoInfo.constructor = GetMethodID(env, mediaCodecCryptoInfoClass
					, "<init>", "()V");
				Nex_MC::JNI::JMETHODS::CryptoInfo.set = GetMethodID(env, mediaCodecCryptoInfoClass
					, "set", "(I[I[I[B[BI)V");
				Nex_MC::JNI::JFIELDS::CryptoInfo.iv = GetFieldID(env, mediaCodecCryptoInfoClass
					, "iv", "[B");
				Nex_MC::JNI::JFIELDS::CryptoInfo.key = GetFieldID(env, mediaCodecCryptoInfoClass
					, "key", "[B");
				Nex_MC::JNI::JFIELDS::CryptoInfo.mode = GetFieldID(env, mediaCodecCryptoInfoClass
					, "mode", "I");
				Nex_MC::JNI::JFIELDS::CryptoInfo.numBytesOfClearData = GetFieldID(env, mediaCodecCryptoInfoClass
					, "numBytesOfClearData", "[I");
				Nex_MC::JNI::JFIELDS::CryptoInfo.numBytesOfEncryptedData = GetFieldID(env, mediaCodecCryptoInfoClass
					, "numBytesOfEncryptedData", "[I");
				Nex_MC::JNI::JFIELDS::CryptoInfo.numSubSamples = GetFieldID(env, mediaCodecCryptoInfoClass
					, "numSubSamples", "I");

				jclass mediaCryptoClass = env->FindClass("android/media/MediaCrypto");

				if (NULL == mediaCryptoClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCrypto\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::MediaCrypto = (jclass)env->NewGlobalRef(mediaCryptoClass);
				Nex_MC::JNI::JMETHODS::MediaCrypto.constructor = GetMethodID(env, mediaCryptoClass
					, "<init>", "(Ljava/util/UUID;[B)V");
				Nex_MC::JNI::JMETHODS::MediaCrypto.isCryptoSchemeSupported = GetStaticMethodID(env, mediaCryptoClass
					, "isCryptoSchemeSupported", "(Ljava/util/UUID;)Z");

				jclass UUIDClass = env->FindClass("java/util/UUID");

				if (NULL == UUIDClass)
				{
					MC_ERR("JNI: Couldn't find class \"java/util/UUID\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::UUID = (jclass)env->NewGlobalRef(UUIDClass);
				Nex_MC::JNI::JMETHODS::UUID.constructor = GetMethodID(env, UUIDClass
					, "<init>", "(JJ)V");

				jclass MediaCodecListClass = env->FindClass("android/media/MediaCodecList");

				if (NULL == MediaCodecListClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodecList\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::MediaCodecList = (jclass)env->NewGlobalRef(MediaCodecListClass);
				Nex_MC::JNI::JMETHODS::MediaCodecList.getCodecCount = GetStaticMethodID(env, MediaCodecListClass
					, "getCodecCount", "()I", NULL);
				Nex_MC::JNI::JMETHODS::MediaCodecList.getCodecInfoAt = GetStaticMethodID(env, MediaCodecListClass
					, "getCodecInfoAt", "(I)Landroid/media/MediaCodecInfo;", NULL);


				jclass MediaCodecInfoClass = env->FindClass("android/media/MediaCodecInfo");

				if (NULL == MediaCodecInfoClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodecInfo\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::MediaCodecInfo = (jclass)env->NewGlobalRef(MediaCodecInfoClass);
				Nex_MC::JNI::JMETHODS::MediaCodecInfo.getName = GetMethodID(env, MediaCodecInfoClass
					, "getName", "()Ljava/lang/String;", NULL);
				Nex_MC::JNI::JMETHODS::MediaCodecInfo.isEncoder = GetMethodID(env, MediaCodecInfoClass
					, "isEncoder", "()Z", NULL);
				Nex_MC::JNI::JMETHODS::MediaCodecInfo.getSupportedTypes = GetMethodID(env, MediaCodecInfoClass
					, "getSupportedTypes", "()[Ljava/lang/String;", NULL);
				Nex_MC::JNI::JMETHODS::MediaCodecInfo.getCapabilitiesForType = GetMethodID(env, MediaCodecInfoClass
					, "getCapabilitiesForType", "(Ljava/lang/String;)Landroid/media/MediaCodecInfo$CodecCapabilities;", NULL);

				jclass MediaCodecInfo_CodecCapabilitiesClass = env->FindClass("android/media/MediaCodecInfo$CodecCapabilities");

				if (NULL == MediaCodecInfo_CodecCapabilitiesClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodecInfo$CodecCapabilities\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::MediaCodecInfo_CodecCapabilities = (jclass)env->NewGlobalRef(MediaCodecInfo_CodecCapabilitiesClass);
				Nex_MC::JNI::JFIELDS::MediaCodecInfo_CodecCapabilities.colorFormats = GetFieldID(env, MediaCodecInfo_CodecCapabilitiesClass
					, "colorFormats", "[I");
				Nex_MC::JNI::JFIELDS::MediaCodecInfo_CodecCapabilities.profileLevels = GetFieldID(env, MediaCodecInfo_CodecCapabilitiesClass
					, "profileLevels", "[Landroid/media/MediaCodecInfo$CodecProfileLevel;");
				Nex_MC::JNI::JMETHODS::CodecCapabilities.isFeatureSupported = GetMethodID(env, MediaCodecInfo_CodecCapabilitiesClass
					, "isFeatureSupported", "(Ljava/lang/String;)Z", NULL);


				jclass MediaCodecInfo_CodecProfileLevelClass = env->FindClass("android/media/MediaCodecInfo$CodecProfileLevel");

				if (NULL == MediaCodecInfo_CodecProfileLevelClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaCodecInfo$CodecProfileLevel\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}

				Nex_MC::JNI::JCLASS::MediaCodecInfo_CodecProfileLevel = (jclass)env->NewGlobalRef(MediaCodecInfo_CodecProfileLevelClass);
				Nex_MC::JNI::JFIELDS::MediaCodecInfo_CodecProfileLevel.level = GetFieldID(env, MediaCodecInfo_CodecProfileLevelClass
					, "level", "I");
				Nex_MC::JNI::JFIELDS::MediaCodecInfo_CodecProfileLevel.profile = GetFieldID(env, MediaCodecInfo_CodecProfileLevelClass
					, "profile", "I");

				if (23 <= Nex_MC::Utils::GetBuildVersionSDK())
				{
					Nex_MC::JNI::JMETHODS::CodecCapabilities.getVideoCapabilities = GetMethodID(env, MediaCodecInfo_CodecCapabilitiesClass
						, "getVideoCapabilities", "()Landroid/media/MediaCodecInfo$VideoCapabilities;", NULL);

					jclass MediaCodecInfo_VideoCapabilitiesClass = env->FindClass("android/media/MediaCodecInfo$VideoCapabilities");

					if (NULL == MediaCodecInfo_VideoCapabilitiesClass)
					{
						MC_ERR("JNI: Couldn't find class \"android/media/MediaCodecInfo$VideoCapabilities\"");
						jniThrowException(env.get(), "java/lang/Exception", NULL);
						return;
					}

					Nex_MC::JNI::JCLASS::MediaCodecInfo_VideoCapabilities = (jclass)env->NewGlobalRef(MediaCodecInfo_VideoCapabilitiesClass);
					Nex_MC::JNI::JMETHODS::VideoCapabilities.getWidthAlignment = GetMethodID(env, MediaCodecInfo_VideoCapabilitiesClass
						, "getWidthAlignment", "()I", NULL);
					Nex_MC::JNI::JMETHODS::VideoCapabilities.getHeightAlignment = GetMethodID(env, MediaCodecInfo_VideoCapabilitiesClass
						, "getHeightAlignment", "()I", NULL);
				}
			}
		}

		void OnUnload()
		{
			Nex_MC::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				if (NULL != Nex_MC::JNI::JCLASS::MediaCodec)
				{
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::MediaCodec);
					Nex_MC::JNI::JCLASS::MediaCodec = NULL;
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::BufferInfo);
					Nex_MC::JNI::JCLASS::BufferInfo = NULL;
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::CryptoInfo);
					Nex_MC::JNI::JCLASS::CryptoInfo = NULL;
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::MediaCrypto);
					Nex_MC::JNI::JCLASS::MediaCrypto = NULL;
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::UUID);
					Nex_MC::JNI::JCLASS::UUID = NULL;
				}
			}
		}
	private:
		pthread_mutex_t lock;
		int instanceCount;
	};

	InitJNI initJNI;
}; // namespace (anon1)
