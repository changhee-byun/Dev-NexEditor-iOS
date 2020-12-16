/******************************************************************************
* File Name        : jni_MediaFormat.cpp
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
#define LOCATION_TAG "jni_MediaFormat"

#include <pthread.h>

#include "./jni_MediaFormat.h"

#include "utils/utils.h"
#include "utils/jni.h"

// for differentiating between encoder and decoder in logs (U == undefined)
#define NEX_MC_TYPE ('U' - 'D')

namespace Nex_MC {
namespace JNI {
	namespace JCLASS {
		jclass MediaFormat = NULL;
	}; // namespace JCLASS

	namespace JMETHODS {
		JavaMediaFormatMethods MediaFormat = {
			  NULL // jmethodID createAudioFormat
			, NULL // jmethodID createVideoFormat
			, NULL // jmethodID getInteger
			, NULL // jmethodID setInteger
			, NULL // jmethodID setByteBuffer
			, NULL // jmethodID toString
		};
	}; // namespace JMETHODS
}; // namespace JNI

using Utils::JNI::JNIEnvWrapper;

NexMediaFormat_using_jni::NexMediaFormat_using_jni()
	: jobjMediaFormat(NULL)
{
}

NexMediaFormat_using_jni::~NexMediaFormat_using_jni()
{
	if (NULL != jobjMediaFormat)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->DeleteGlobalRef(jobjMediaFormat);
		}
		else
		{
			MC_ERR("couldn't get JNIEnv for deleting global ref to MediaFormat object");
		}
		jobjMediaFormat = NULL;
	}
}

/*static*/ int NexMediaFormat_using_jni::createAudioFormat(const char *mimeType, int sampleRate, int channelCount, NexMediaFormat * &result)
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

	jobject jobjMediaFormat = env->CallStaticObjectMethod(JNI::JCLASS::MediaFormat, JNI::JMETHODS::MediaFormat.createAudioFormat, jstrMimeType, (jint)sampleRate, (jint)channelCount);

	if (NULL == jobjMediaFormat)
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jstrMimeType);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling createAudioFormat");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -3;
		}
		return -4;
	}

	NexMediaFormat_using_jni *tempResult = new NexMediaFormat_using_jni;
	tempResult->jobjMediaFormat = env->NewGlobalRef(jobjMediaFormat);

	env->DeleteLocalRef(jstrMimeType);
	env->DeleteLocalRef(jobjMediaFormat);

	result = tempResult;

	return 0;
}

/*static*/ int NexMediaFormat_using_jni::createVideoFormat(const char *mimeType, int width, int height, NexMediaFormat * &result)
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

	jobject jobjMediaFormat = env->CallStaticObjectMethod(JNI::JCLASS::MediaFormat, JNI::JMETHODS::MediaFormat.createVideoFormat, jstrMimeType, (jint)width, (jint)height);

	if (NULL == jobjMediaFormat)
	{
		MC_ERR("failed");
		env->DeleteLocalRef(jstrMimeType);

		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling createVideoFormat");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -3;
		}
		return -4;
	}

	NexMediaFormat_using_jni *tempResult = new NexMediaFormat_using_jni;
	tempResult->jobjMediaFormat = env->NewGlobalRef(jobjMediaFormat);

	env->DeleteLocalRef(jstrMimeType);
	env->DeleteLocalRef(jobjMediaFormat);

	result = tempResult;

	return 0;
}

/*static*/ int NexMediaFormat_using_jni::releaseNexMediaFormat(NexMediaFormat *mediaFormat)
{
	delete static_cast<NexMediaFormat_using_jni *>(mediaFormat);
	return 0;
}

int NexMediaFormat_using_jni::setInteger(const char *key, int value)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstrKey = env->NewStringUTF(key);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while creating new String");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	env->CallVoidMethod(jobjMediaFormat, JNI::JMETHODS::MediaFormat.setInteger, jstrKey, (jint)value);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling setInteger");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -3;
	}

	env->DeleteLocalRef(jstrKey);

	return 0;
}

int NexMediaFormat_using_jni::getInteger(const char *key, int &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstrKey = env->NewStringUTF(key);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while creating new String");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	jint value = env->CallIntMethod(jobjMediaFormat, JNI::JMETHODS::MediaFormat.getInteger, jstrKey);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling getInteger");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -3;
	}

	env->DeleteLocalRef(jstrKey);

	result = (int)value;

	return 0;
}

int NexMediaFormat_using_jni::setByteBuffer(const char *key, jobject byteBuffer)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstrKey = env->NewStringUTF(key);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while creating new String");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -2;
	}

	env->CallVoidMethod(jobjMediaFormat, JNI::JMETHODS::MediaFormat.setByteBuffer, jstrKey, byteBuffer);

	if (JNI_TRUE == env->ExceptionCheck())
	{
		MC_ERR("exception occurred while calling setByteBuffer");
		JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
		return -3;
	}

	env->DeleteLocalRef(jstrKey);

	return 0;
}

int NexMediaFormat_using_jni::toString(char *buffer, size_t bufferSize, size_t &result)
{
	JNIEnvWrapper env;

	if (env == NULL)
		return -1;

	jstring jstr = (jstring)env->CallObjectMethod(jobjMediaFormat, JNI::JMETHODS::MediaFormat.toString);

	if (NULL == jstr)
	{
		MC_ERR("failed");
		if (JNI_TRUE == env->ExceptionCheck())
		{
			MC_ERR("exception occurred while calling toString");
			JNI_EXCEPTION_DESCRIBE_AND_CLEAR(env);
			return -2;
		}
		return -3;
	}

	const char *cstr = env->GetStringUTFChars(jstr, 0);
	result = strlen(cstr);
	strncpy(buffer, cstr, bufferSize);
	env->ReleaseStringUTFChars(jstr, cstr);
	env->DeleteLocalRef(jstr);

	return 0;
}

}; // namespace Nex_MC

namespace { // anon1
	using Nex_MC::Utils::JNI::GetMethodID;
	using Nex_MC::Utils::JNI::GetStaticMethodID;
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
				jclass mediaFormatClass = env->FindClass("android/media/MediaFormat");

				if (NULL == mediaFormatClass)
				{
					MC_ERR("JNI: Couldn't find class \"android/media/MediaFormat\"");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_MC::JNI::JCLASS::MediaFormat = (jclass)env->NewGlobalRef(mediaFormatClass);

				Nex_MC::JNI::JMETHODS::MediaFormat.createAudioFormat = GetStaticMethodID(env, mediaFormatClass
					, "createAudioFormat", "(Ljava/lang/String;II)Landroid/media/MediaFormat;");
				Nex_MC::JNI::JMETHODS::MediaFormat.createVideoFormat = GetStaticMethodID(env, mediaFormatClass
					, "createVideoFormat", "(Ljava/lang/String;II)Landroid/media/MediaFormat;");
				Nex_MC::JNI::JMETHODS::MediaFormat.getInteger = GetMethodID(env, mediaFormatClass
					, "getInteger", "(Ljava/lang/String;)I");
				Nex_MC::JNI::JMETHODS::MediaFormat.setInteger = GetMethodID(env, mediaFormatClass
					, "setInteger", "(Ljava/lang/String;I)V");
				Nex_MC::JNI::JMETHODS::MediaFormat.setByteBuffer = GetMethodID(env, mediaFormatClass
					, "setByteBuffer", "(Ljava/lang/String;Ljava/nio/ByteBuffer;)V");
				Nex_MC::JNI::JMETHODS::MediaFormat.toString = GetMethodID(env, mediaFormatClass
					, "toString", "()Ljava/lang/String;");
			}
		}
		void OnUnload()
		{
			Nex_MC::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				if (NULL != Nex_MC::JNI::JCLASS::MediaFormat)
				{
					env->DeleteGlobalRef(Nex_MC::JNI::JCLASS::MediaFormat);
					Nex_MC::JNI::JCLASS::MediaFormat = NULL;
				}
			}
		}
	private:
		pthread_mutex_t lock;
		int instanceCount;
	};

	InitJNI initJNI;
}; // namespace (anon1)
