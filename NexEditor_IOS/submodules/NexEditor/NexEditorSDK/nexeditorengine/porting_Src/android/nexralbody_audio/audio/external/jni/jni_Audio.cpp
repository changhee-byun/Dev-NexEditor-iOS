#define LOG_TAG "Nex_jni_audio"

// #include "utils/debug.h"

#include "./jni_Audio.h"

#include "NexSAL_Internal.h"

#include "utils/utils.h"
#include "utils/jni.h"

#include "nexSALClasses/nexSALClasses.h"

#include <cstdlib>

namespace Nex_AR {
namespace JNI {
	namespace JCLASS {
		jclass AudioManager = NULL;
		jclass AudioFormat = NULL;
		jclass AudioTrack = NULL;
		jclass AudioTimestamp = NULL;
	} // namespace JCLASS

	namespace JMETHODS {
		JavaAudioManagerMethods AudioManager = {
			  NULL // jmethodID getProperty /* api 17 */
			  , NULL // jmethodID getOutputLatency /* Hidden API */ //yoon
		};

		JavaAudioTrackMethods AudioTrack = {
			  NULL // jmethodID constructor /* api 3 */
			, NULL // jmethodID constructor2 /* api 9 */			
			, NULL // jmethodID flush /* api 3 */
			, NULL // jmethodID getAudioSessionId /* api 9 */
			, NULL // jmethodID getChannelCount /* api 3 */
			, NULL // jmethodID getMinBufferSize /* api 3 */
			, NULL // jmethodID getNativeFrameCount /* api 3 */
			, NULL // jmethodID getNativeOutputSampleRate /* api 3 */
			, NULL // jmethodID getPlaybackHeadPosition /* api 3 */
			, NULL // jmethodID getPlayState /* api 3 */
			, NULL // jmethodID getState /* api 3 */
			, NULL // jmethodID pause /* api 3 */
			, NULL // jmethodID play /* api 3 */
			, NULL // jmethodID release /* api 3 */
			, NULL // jmethodID setNotificationMarkerPosition /* api 3 */
			, NULL // jmethodID setStereoVolume /* api 3 */
			, NULL // jmethodID stop /* api 3 */
			, NULL // jmethodID write_s /* api 3 */
			, NULL // jmethodID write_b /* api 3 */
			, NULL // jmethodID getTimestamp /* api 19 */
		};

		JavaAudioTimestampMethods AudioTimestamp = {
			  NULL // jmethodID constructor /* api 19 */
		};
	} // namespace JMETHODS

	namespace JFIELDS {
		JavaAudioManagerFields AudioManager = {
			  NULL // jfieldID PROPERTY_OUTPUT_FRAMES_PER_BUFFER /* api 17 */
			, NULL // jfieldID PROPERTY_OUTPUT_SAMPLE_RATE /* api 17 */
		};

		JavaAudioFormatFields AudioFormat = {
			  NULL // jfieldID CHANNEL_OUT_5POINT1 /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_7POINT1 /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_BACK_CENTER /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_BACK_LEFT /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_BACK_RIGHT /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_DEFAULT /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_FRONT_CENTER /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_FRONT_LEFT /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_FRONT_LEFT_OF_CENTER /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_FRONT_RIGHT /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_FRONT_RIGHT_OF_CENTER /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_LOW_FREQUENCY /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_MONO /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_QUAD /* api 5 */
			, NULL // jfieldID CHANNEL_OUT_STEREO /* api 5 */
			, NULL // jfieldID ENCODING_DEFAULT /* api 3 */
			, NULL // jfieldID ENCODING_PCM_16BIT /* api 3 */
			, NULL // jfieldID ENCODING_PCM_8BIT /* api 3 */
		};

		JavaAudioTrackFields AudioTrack = {
			  NULL // jfieldID MODE_STREAM /* api 3 */
			, NULL // jfieldID STATE_INITIALIZED /* api 3 */
			, NULL // jfieldID mNativeTrackInJavaObj /* api 3 */
			, NULL // jfieldID mNativeTrackInJavaObj_long /* api ?? */
		};

		JavaAudioTimestampFields AudioTimestamp = {
			  NULL // jfieldID framePosition /* api 19 */
			, NULL // jfieldID nanoTime /* api 19 */
		};
	} // namespace JFIELDS
} // namespace JNI

using Utils::JNI::JNIEnvWrapper;

NexAudio_using_jni::NexAudio_using_jni()
	: jobjAudioManager(NULL)
{
}

NexAudio_using_jni::~NexAudio_using_jni()
{
	if (NULL != jobjAudioManager)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->DeleteGlobalRef(jobjAudioManager);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't get JNIEnv for deleting global ref to AudioManager object\n");
		}
		jobjAudioManager = NULL;
	}
}

/*static*/ NexAudio * NexAudio_using_jni::create(jobject audioManager)
{
	NexAudio_using_jni *ret = NULL;

	ret = new NexAudio_using_jni;

	if (NULL != ret && NULL != audioManager)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret->jobjAudioManager = env->NewGlobalRef(audioManager);
		}
	}

	return ret;
}

/*static*/ void NexAudio_using_jni::destroy(NexAudio *instance)
{
	delete ((NexAudio_using_jni *)instance);
}

int NexAudio_using_jni::getOutputLatency(int streamType)
{
	if (NULL != jobjAudioManager)
	{
		JNIEnvWrapper env;
		
		if (env != NULL && NULL != Nex_AR::JNI::JMETHODS::AudioManager.getOutputLatency)
		{
			jint latency = (jint)env->CallIntMethod(jobjAudioManager, Nex_AR::JNI::JMETHODS::AudioManager.getOutputLatency, streamType);
			
			return latency;
		}
	}
	
	return -1;
}

int NexAudio_using_jni::getOutputSampleRate()
{
	if (NULL != jobjAudioManager)
	{
		JNIEnvWrapper env;

		if (env != NULL && NULL != Nex_AR::JNI::JMETHODS::AudioManager.getProperty && NULL != Nex_AR::JNI::JFIELDS::AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE)
		{
			jstring property = (jstring)env->GetStaticObjectField(Nex_AR::JNI::JCLASS::AudioManager, Nex_AR::JNI::JFIELDS::AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);

			jstring value = (jstring)env->CallObjectMethod(jobjAudioManager, Nex_AR::JNI::JMETHODS::AudioManager.getProperty, property);

			if (NULL != value)
			{
				const char * cvalue = env->GetStringUTFChars(value, NULL);

				if (NULL != cvalue)
				{
					int ret = atoi(cvalue);

					env->ReleaseStringUTFChars(value, cvalue);

					return ret;
				}
			}
		}
	}

	return -1;
}

int NexAudio_using_jni::getOutputFramesPerBuffer()
{
	if (NULL != jobjAudioManager)
	{
		JNIEnvWrapper env;

		if (env != NULL && NULL != Nex_AR::JNI::JMETHODS::AudioManager.getProperty && NULL != Nex_AR::JNI::JFIELDS::AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER)
		{
			jstring property = (jstring)env->GetStaticObjectField(Nex_AR::JNI::JCLASS::AudioManager, Nex_AR::JNI::JFIELDS::AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);

			jstring value = (jstring)env->CallObjectMethod(jobjAudioManager, Nex_AR::JNI::JMETHODS::AudioManager.getProperty, property);

			if (NULL != value)
			{
				const char * cvalue = env->GetStringUTFChars(value, NULL);

				if (NULL != cvalue)
				{
					int ret = atoi(cvalue);

					env->ReleaseStringUTFChars(value, cvalue);

					return ret;
				}
			}
		}
	}

	return -1;
}

NexAudioTrack_using_jni::NexAudioTrack_using_jni()
	: jobjAudioTrack(NULL)
	, jobjAudioTimestamp(NULL)
{
}

NexAudioTrack_using_jni::~NexAudioTrack_using_jni()
{
	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->DeleteGlobalRef(jobjAudioTrack);

			if (NULL != jobjAudioTimestamp)
			{
				env->DeleteGlobalRef(jobjAudioTimestamp);
				jobjAudioTimestamp = NULL;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't get JNIEnv for deleting global ref to AudioTrack object\n");
		}
		jobjAudioTrack = NULL;
	}
}

/*static*/ NexAudioTrack * NexAudioTrack_using_jni::create(int streamType, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode)
{
	NexAudioTrack_using_jni *ret = NULL;

	JNIEnvWrapper env;

	jobject globalledAudioTrack = NULL;

	if (env != NULL)
	{
		jobject audioTrack = env->NewObject(JNI::JCLASS::AudioTrack, JNI::JMETHODS::AudioTrack.constructor, (jint)streamType, (jint)sampleRateInHz, (jint)channelConfig, (jint)audioFormat, (jint)bufferSizeInBytes, (jint)mode);

		globalledAudioTrack = env->NewGlobalRef(audioTrack);

		env->DeleteLocalRef(audioTrack);
	}

	if (globalledAudioTrack != NULL)
	{
		ret = new NexAudioTrack_using_jni;

		if (NULL != ret)
		{
			ret->jobjAudioTrack = globalledAudioTrack;

			if (NULL != JNI::JCLASS::AudioTimestamp)
			{
				jobject audioTimestamp = env->NewObject(JNI::JCLASS::AudioTimestamp, JNI::JMETHODS::AudioTimestamp.constructor);

				if (NULL != audioTimestamp)
				{
					jobject globalledAudioTimestamp = env->NewGlobalRef(audioTimestamp);
					ret->jobjAudioTimestamp = globalledAudioTimestamp;
					env->DeleteLocalRef(audioTimestamp);
				}
			}
		}
	}

	return ret;
}

/*static*/ NexAudioTrack * NexAudioTrack_using_jni::create(int streamType, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode, int sessionId)
{
	NexAudioTrack_using_jni *ret = NULL;

	JNIEnvWrapper env;

	jobject globalledAudioTrack = NULL;

	if (env != NULL)
	{
		jobject audioTrack = NULL;
		if(JNI::JMETHODS::AudioTrack.constructor2)
			audioTrack = env->NewObject(JNI::JCLASS::AudioTrack, JNI::JMETHODS::AudioTrack.constructor2, (jint)streamType, (jint)sampleRateInHz, (jint)channelConfig, (jint)audioFormat, (jint)bufferSizeInBytes, (jint)mode, (jint)sessionId);
		else
			audioTrack = env->NewObject(JNI::JCLASS::AudioTrack, JNI::JMETHODS::AudioTrack.constructor, (jint)streamType, (jint)sampleRateInHz, (jint)channelConfig, (jint)audioFormat, (jint)bufferSizeInBytes, (jint)mode);
		
		globalledAudioTrack = env->NewGlobalRef(audioTrack);

		env->DeleteLocalRef(audioTrack);
	}

	if (globalledAudioTrack != NULL)
	{
		ret = new NexAudioTrack_using_jni;

		if (NULL != ret)
		{
			ret->jobjAudioTrack = globalledAudioTrack;

			if (NULL != JNI::JCLASS::AudioTimestamp)
			{
				jobject audioTimestamp = env->NewObject(JNI::JCLASS::AudioTimestamp, JNI::JMETHODS::AudioTimestamp.constructor);

				if (NULL != audioTimestamp)
				{
					jobject globalledAudioTimestamp = env->NewGlobalRef(audioTimestamp);
					ret->jobjAudioTimestamp = globalledAudioTimestamp;
					env->DeleteLocalRef(audioTimestamp);
				}
			}
		}
	}

	return ret;
}

/*static*/ void NexAudioTrack_using_jni::destroy(NexAudioTrack *instance)
{
	delete ((NexAudioTrack_using_jni *)instance);
}

void NexAudioTrack_using_jni::flush()
{
	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->CallVoidMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.flush);
		}
	}
}

int NexAudioTrack_using_jni::getAudioSessionId()
{
	int ret = 0;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL && NULL != JNI::JMETHODS::AudioTrack.getAudioSessionId)
		{
			ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getAudioSessionId));
		}
	}

	return ret;
}

int NexAudioTrack_using_jni::getChannelCount()
{
	int ret = 0;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getChannelCount));
		}
	}

	return ret;
}

/*static*/ int NexAudioTrack_using_jni::getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat)
{
	int ret = -1;

	JNIEnvWrapper env;

	if (env != NULL)
	{
		ret = (int)(env->CallStaticIntMethod(JNI::JCLASS::AudioTrack, JNI::JMETHODS::AudioTrack.getMinBufferSize, (jint)sampleRateInHz, (jint)channelConfig, (jint)audioFormat));
	}

	return ret;
}

int NexAudioTrack_using_jni::getNativeFrameCount()
{
	int ret = -1;

	JNIEnvWrapper env;

	if (env != NULL)
	{
		ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getNativeFrameCount));
	}

	return ret;
}

/*static*/ int NexAudioTrack_using_jni::getNativeOutputSampleRate(int streamType)
{
	int ret = -1;

	JNIEnvWrapper env;

	if (env != NULL)
	{
		ret = (int)(env->CallStaticIntMethod(JNI::JCLASS::AudioTrack, JNI::JMETHODS::AudioTrack.getNativeOutputSampleRate, (jint)streamType));
	}

	return ret;
}

unsigned int NexAudioTrack_using_jni::getPlaybackHeadPosition()
{
	int ret = 0;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (unsigned int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getPlaybackHeadPosition));
		}
	}

	return ret;
}

int NexAudioTrack_using_jni::getPlayState()
{
	int ret = 0;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getPlayState));
		}
	}

	return ret;
}

int NexAudioTrack_using_jni::getState()
{
	int ret = 0;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getState));
		}
	}

	return ret;
}

void NexAudioTrack_using_jni::pause()
{
	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->CallVoidMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.pause);
		}
	}
}

void NexAudioTrack_using_jni::play()
{
	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->CallVoidMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.play);
		}
	}
}

void NexAudioTrack_using_jni::release()
{
	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->CallVoidMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.release);
		}
	}
}

void NexAudioTrack_using_jni::stop()
{
	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			env->CallVoidMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.stop);
		}
	}
}

int NexAudioTrack_using_jni::write(const short *audioData, int offsetInShorts, int sizeInShorts)
{
	int ret = -3;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			jshortArray shortarray = env->NewShortArray(sizeInShorts);

			if (NULL != shortarray)
			{
				env->SetShortArrayRegion(shortarray, 0, sizeInShorts, (jshort *)audioData);
				ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.write_s, shortarray, (jint)offsetInShorts, (jint)sizeInShorts));
				env->DeleteLocalRef(shortarray);
			}
		}
	}

	return ret;
}

int NexAudioTrack_using_jni::write(const unsigned char *audioData, int offsetInBytes, int sizeInBytes)
{
	int ret = -3;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			jbyteArray bytearray = env->NewByteArray(sizeInBytes);

			if (NULL != bytearray)
			{
				env->SetByteArrayRegion(bytearray, 0, sizeInBytes, (jbyte *)audioData);
				ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.write_b, bytearray, (jint)offsetInBytes, (jint)sizeInBytes));
				env->DeleteLocalRef(bytearray);
			}
		}
	}

	return ret;
}

int NexAudioTrack_using_jni::setStereoVolume(float leftVolume, float rightVolume)
{
	int ret = -1;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.setStereoVolume, (jfloat)leftVolume, (jfloat)rightVolume));
		}
	}

	return ret;
}

void * NexAudioTrack_using_jni::getNativeAudioTrack()
{
	void * ret = NULL;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			if (NULL != JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj)
			{
				ret = (void *)(env->GetIntField(jobjAudioTrack, JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj));
			}
			else if (NULL != JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj_long)
			{
				ret = (void *)(env->GetLongField(jobjAudioTrack, JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj_long));
			}
		}
	}
	return ret;
}

bool NexAudioTrack_using_jni::getTimestamp(long long &framePosition, long long &nanoTime)
{
	bool ret = false;

	if (NULL != jobjAudioTimestamp && NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (bool)(env->CallBooleanMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.getTimestamp, jobjAudioTimestamp));
			if (ret)
			{
				framePosition = (long long)(env->GetLongField(jobjAudioTimestamp, JNI::JFIELDS::AudioTimestamp.framePosition));
				nanoTime = (long long)(env->GetLongField(jobjAudioTimestamp, JNI::JFIELDS::AudioTimestamp.nanoTime));
			}
		}
	}
	return ret;
}

int NexAudioTrack_using_jni::setNotificationMarkerPosition(int markerInFrames)
{
	int ret = -1;

	if (NULL != jobjAudioTrack)
	{
		JNIEnvWrapper env;

		if (env != NULL)
		{
			ret = (int)(env->CallIntMethod(jobjAudioTrack, JNI::JMETHODS::AudioTrack.setNotificationMarkerPosition, (jint)markerInFrames));
		}
	}

	return ret;
}

} // namespace Nex_AR


namespace { // anon1
	using Nex_AR::Utils::JNI::GetMethodID;
	using Nex_AR::Utils::JNI::GetStaticMethodID;
	using Nex_AR::Utils::JNI::GetFieldID;
	using Nex_AR::Utils::JNI::GetStaticFieldID;
	using Nex_AR::Utils::JNI::jniThrowException;

	class InitJNI : Nex_AR::Utils::Initializer
	{
	public:
		InitJNI()
			: initialized(false)
			, instanceCount(0)
		{
			Nex_AR::Utils::registerInitializer(this);
		}

		virtual ~InitJNI()
		{
		}

		virtual void initialize()
		{
			CSALMutex::Auto autolock(lock);

			if (0 == instanceCount)
			{
				OnLoad();
			}

			++instanceCount;
		}

		virtual void deinitialize()
		{
			CSALMutex::Auto autolock(lock);
			--instanceCount;

			if (0 == instanceCount)
			{
				OnUnload();
			}
		}

	private:
		void OnLoad()
		{
			Nex_AR::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				jclass audioManagerClass = env->FindClass("android/media/AudioManager");

				if (NULL == audioManagerClass)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "JNI: Couldn't find class \"android/media/AudioManager\"\n");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_AR::JNI::JCLASS::AudioManager = (jclass)env->NewGlobalRef(audioManagerClass);

				Nex_AR::JNI::JMETHODS::AudioManager.getProperty = GetMethodID(env, audioManagerClass
					, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;", false);
					
        		Nex_AR::JNI::JMETHODS::AudioManager.getOutputLatency = GetMethodID(env, audioManagerClass //yoon
		        	, "getOutputLatency", "(I)I", false);
		        	
				Nex_AR::JNI::JFIELDS::AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER = GetStaticFieldID(env, audioManagerClass
					, "PROPERTY_OUTPUT_FRAMES_PER_BUFFER", "Ljava/lang/String;", false);
				Nex_AR::JNI::JFIELDS::AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE = GetStaticFieldID(env, audioManagerClass
					, "PROPERTY_OUTPUT_SAMPLE_RATE", "Ljava/lang/String;", false);


				jclass audioFormatClass = env->FindClass("android/media/AudioFormat");

				if (NULL == audioFormatClass)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "JNI: Couldn't find class \"android/media/AudioFormat\"\n");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_AR::JNI::JCLASS::AudioFormat = (jclass)env->NewGlobalRef(audioFormatClass);

				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_5POINT1 = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_5POINT1", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_7POINT1 = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_7POINT1", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_BACK_CENTER = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_BACK_CENTER", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_BACK_LEFT = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_BACK_LEFT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_BACK_RIGHT = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_BACK_RIGHT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_DEFAULT = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_DEFAULT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_FRONT_CENTER = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_FRONT_CENTER", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_FRONT_LEFT = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_FRONT_LEFT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_FRONT_LEFT_OF_CENTER = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_FRONT_LEFT_OF_CENTER", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_FRONT_RIGHT = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_FRONT_RIGHT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_FRONT_RIGHT_OF_CENTER", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_LOW_FREQUENCY = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_LOW_FREQUENCY", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_MONO = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_MONO", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_QUAD = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_QUAD", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.CHANNEL_OUT_STEREO = GetStaticFieldID(env, audioFormatClass
					, "CHANNEL_OUT_STEREO", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.ENCODING_DEFAULT = GetStaticFieldID(env, audioFormatClass
					, "ENCODING_DEFAULT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.ENCODING_PCM_16BIT = GetStaticFieldID(env, audioFormatClass
					, "ENCODING_PCM_16BIT", "I");
				Nex_AR::JNI::JFIELDS::AudioFormat.ENCODING_PCM_8BIT = GetStaticFieldID(env, audioFormatClass
					, "ENCODING_PCM_8BIT", "I");


				jclass audioTrackClass = env->FindClass("android/media/AudioTrack");

				if (NULL == audioTrackClass)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "JNI: Couldn't find class \"android/media/AudioTrack\"\n");
					jniThrowException(env.get(), "java/lang/Exception", NULL);
					return;
				}
				Nex_AR::JNI::JCLASS::AudioTrack = (jclass)env->NewGlobalRef(audioTrackClass);

				Nex_AR::JNI::JMETHODS::AudioTrack.constructor = GetMethodID(env, audioTrackClass
					, "<init>", "(IIIIII)V");
				Nex_AR::JNI::JMETHODS::AudioTrack.constructor2 = GetMethodID(env, audioTrackClass
					, "<init>", "(IIIIIII)V", false);				
				Nex_AR::JNI::JMETHODS::AudioTrack.flush = GetMethodID(env, audioTrackClass
					, "flush", "()V");
				Nex_AR::JNI::JMETHODS::AudioTrack.getAudioSessionId = GetMethodID(env, audioTrackClass
					, "getAudioSessionId", "()I", false);
				Nex_AR::JNI::JMETHODS::AudioTrack.getChannelCount = GetMethodID(env, audioTrackClass
					, "getChannelCount", "()I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getMinBufferSize = GetStaticMethodID(env, audioTrackClass
					, "getMinBufferSize", "(III)I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getNativeFrameCount = GetMethodID(env, audioTrackClass
					, "getNativeFrameCount", "()I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getNativeOutputSampleRate = GetStaticMethodID(env, audioTrackClass
					, "getNativeOutputSampleRate", "(I)I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getPlaybackHeadPosition = GetMethodID(env, audioTrackClass
					, "getPlaybackHeadPosition", "()I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getPlayState = GetMethodID(env, audioTrackClass
					, "getPlayState", "()I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getState = GetMethodID(env, audioTrackClass
					, "getState", "()I");
				Nex_AR::JNI::JMETHODS::AudioTrack.pause = GetMethodID(env, audioTrackClass
					, "pause", "()V");
				Nex_AR::JNI::JMETHODS::AudioTrack.play = GetMethodID(env, audioTrackClass
					, "play", "()V");
				Nex_AR::JNI::JMETHODS::AudioTrack.release = GetMethodID(env, audioTrackClass
					, "release", "()V");
				Nex_AR::JNI::JMETHODS::AudioTrack.setNotificationMarkerPosition = GetMethodID(env, audioTrackClass
					, "setNotificationMarkerPosition", "(I)I");
				Nex_AR::JNI::JMETHODS::AudioTrack.setStereoVolume = GetMethodID(env, audioTrackClass
					, "setStereoVolume", "(FF)I");
				Nex_AR::JNI::JMETHODS::AudioTrack.stop = GetMethodID(env, audioTrackClass
					, "stop", "()V");
				Nex_AR::JNI::JMETHODS::AudioTrack.write_s = GetMethodID(env, audioTrackClass
					, "write", "([SII)I");
				Nex_AR::JNI::JMETHODS::AudioTrack.write_b = GetMethodID(env, audioTrackClass
					, "write", "([BII)I");
				Nex_AR::JNI::JMETHODS::AudioTrack.getTimestamp = GetMethodID(env, audioTrackClass
					, "getTimestamp", "(Landroid/media/AudioTimestamp;)Z", false);
				Nex_AR::JNI::JFIELDS::AudioTrack.MODE_STREAM = GetStaticFieldID(env, audioTrackClass
					, "MODE_STREAM", "I");
				Nex_AR::JNI::JFIELDS::AudioTrack.STATE_INITIALIZED = GetStaticFieldID(env, audioTrackClass
					, "STATE_INITIALIZED", "I");
				Nex_AR::JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj = GetFieldID(env, audioTrackClass
					, "mNativeTrackInJavaObj", "I", false);
				if (NULL == Nex_AR::JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj)
				{
					Nex_AR::JNI::JFIELDS::AudioTrack.mNativeTrackInJavaObj_long = GetFieldID(env, audioTrackClass
						, "mNativeTrackInJavaObj", "J", false);
				}


				jclass audioTimestampClass = env->FindClass("android/media/AudioTimestamp");

				if (NULL == audioTimestampClass)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "JNI: Couldn't find class \"android/media/AudioTimestamp\"\n");
					if (JNI_TRUE == env->ExceptionCheck())
						env->ExceptionClear();
				}
				else
				{
					Nex_AR::JNI::JCLASS::AudioTimestamp = (jclass)env->NewGlobalRef(audioTimestampClass);

					Nex_AR::JNI::JMETHODS::AudioTimestamp.constructor = GetMethodID(env, audioTimestampClass
						, "<init>", "()V");
					Nex_AR::JNI::JFIELDS::AudioTimestamp.framePosition = GetFieldID(env, audioTimestampClass
						, "framePosition", "J");
					Nex_AR::JNI::JFIELDS::AudioTimestamp.nanoTime = GetFieldID(env, audioTimestampClass
						, "nanoTime", "J");
				}

				initialized = true;
			}
		}

		void OnUnload()
		{
			Nex_AR::Utils::JNI::JNIEnvWrapper env;

			if (env != NULL)
			{
				if (NULL != Nex_AR::JNI::JCLASS::AudioManager)
				{
					env->DeleteGlobalRef(Nex_AR::JNI::JCLASS::AudioManager);
					Nex_AR::JNI::JCLASS::AudioManager = NULL;
				}
				if (NULL != Nex_AR::JNI::JCLASS::AudioFormat)
				{
					env->DeleteGlobalRef(Nex_AR::JNI::JCLASS::AudioFormat);
					Nex_AR::JNI::JCLASS::AudioFormat = NULL;
				}
				if (NULL != Nex_AR::JNI::JCLASS::AudioTrack)
				{
					env->DeleteGlobalRef(Nex_AR::JNI::JCLASS::AudioTrack);
					Nex_AR::JNI::JCLASS::AudioTrack = NULL;
				}

				initialized = false;
			}
		}
	private:
		CSALMutex lock;

		bool initialized;
		int instanceCount;
	}; // class InitJNI

	InitJNI initJNI;
} // namespace (anon1)

#if 0
namespace Nex_AR {
/* static */ void NexAudioTrack_using_jni::OnLoad()
{
	initJNI.OnLoad();
}

/* static */ void NexAudioTrack_using_jni::OnUnload()
{
	initJNI.OnUnload();
}
} // namespace Nex_AR
#endif
