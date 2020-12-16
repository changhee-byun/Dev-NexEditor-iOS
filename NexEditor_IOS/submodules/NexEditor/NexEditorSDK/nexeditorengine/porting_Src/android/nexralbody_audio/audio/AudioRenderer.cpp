#if defined(_USE_SINK_THREAD_)
USE_SINK_THREAD_not_yet_supported
#endif

#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
#include <dlfcn.h>
#endif // !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)

#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include <queue>
#include <sys/system_properties.h>
#include <pthread.h>

#include "NexSAL_Internal.h"
#include "NexRAL.h"
#include "NexMediaDef.h" // #include "NexOTIs.h"
#include "nexSALClasses/nexSALClasses.h"

#include "nexRALBody_Audio_API.h"

#include "external/Audio.h"
#include "external/jni/jni_Audio.h"

#include "MemoryBuffer.h"

#include "utils/utils.h"
#include "utils/jni.h"
#include "version.h"

#define MAX_INSTANCES 10

#define PROPERTY_VALUE_MAX	64

typedef short			Int16;
typedef unsigned short	Uint16;
typedef int				Int32;
typedef unsigned int	Uint32;

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#define _NEX_DEFINE_64_
#endif

namespace { // (anon1)
	class NexAudioRendererInstance
	{
		typedef NXINT32 /*android::status_t*/ (*AudioSystemGetOutputLatency_t)(NXUINT32 *latency, int streamType);
		typedef NXINT32 /*android::status_t*/ (*AudioSystemGetOutputFrameCount_t)(int *frameCount, int stream);
		typedef NXINT32 /*android::status_t*/ (*AudioSystemGetOutputSamplingRate_t)(int *samplingRate, int stream);
	public:
		NexAudioRendererInstance(FNRALCALLBACK ralCallback, void *cbUserData, void *jobjAudioManager, bool _useAudioEffect)
			: refCount(1)
#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
			, libmediaHandle(NULL)
			, fnGetOutputLatency(NULL)
			, fnGetOutputFrameCount(NULL)
			, fnGetOutputSamplingRate(NULL)
#endif // !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
			, fnCallback(ralCallback)
			, callbackUD(cbUserData)
			, nexAudio(NULL)
			, requestedMavenMode(0)
			, requestedMavenStrength(0)
			, requestedBassStrength(0)
			, requestedAutoVolumeEnable(0)
			, requestedAutoVolumeStrength(0)
			, requestedAutoVolumeReleaseTime(0)
			, requestedSpeed(0x7FFFFFFF)
			, requestedSoundPath(-1)
			, requestedVolume(-1.0f)
			, useAudioEffect(_useAudioEffect)
			, audioSessionId(-1)
			, lastRegisteredAudioRenderer(NULL)
		{
		    int apiver = 0; //yoon
		    {
		        char nexSysPropertys[PROPERTY_VALUE_MAX];
			    int nexPropertyLen = 0;
			    memset( nexSysPropertys, 0x00, PROPERTY_VALUE_MAX );
			    nexPropertyLen = Nex_AR::Utils::GetSystemProperty("ro.build.version.sdk", nexSysPropertys);
			    if(nexPropertyLen > 0)
				apiver = atoi(nexSysPropertys);
    			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 0, "Android API level: %d", apiver);
			
	    	}
	    	
		    if(apiver >= 23 )
		    {
				isMarshmallowOrAbove = true;
			}
			else
			{
				isMarshmallowOrAbove = false;
#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
			libmediaHandle = dlopen("libmedia.so", RTLD_LAZY);
			if (NULL != libmediaHandle)
			{
				void *sym = dlsym(libmediaHandle,"_ZN7android11AudioSystem16getOutputLatencyEPj19audio_stream_type_t");//high version of android ,>= 4.1

				if (NULL != sym)
				{
					fnGetOutputLatency = (AudioSystemGetOutputLatency_t)sym;
				}
				else
				{
					sym = dlsym(libmediaHandle,"_ZN7android11AudioSystem16getOutputLatencyEPji"); //low version android , <4.1
					if (NULL != sym)
					{
						fnGetOutputLatency = (AudioSystemGetOutputLatency_t)sym;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "cannot find symbol in libmedia.so\n");
						dlclose(libmediaHandle);
						libmediaHandle = NULL;
					}
				}

				if (NULL != libmediaHandle)
				{
					sym = dlsym(libmediaHandle, "_ZN7android11AudioSystem19getOutputFrameCountEPii"); // may not exist past 4.2
					if (NULL != sym)
					{
						fnGetOutputFrameCount = (AudioSystemGetOutputFrameCount_t)sym;
					}

					sym = dlsym(libmediaHandle, "_ZN7android11AudioSystem21getOutputSamplingRateEPii"); // may not exist past 4.2
					if (NULL != sym)
					{
						fnGetOutputSamplingRate = (AudioSystemGetOutputSamplingRate_t)sym;
					}
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "cannot load libmedia.so\n");
			}
#endif // !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
            }
			nexAudio = Nex_AR::NexAudio_using_jni::create((jobject)jobjAudioManager);
		}

		~NexAudioRendererInstance()
		{
#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
			if (NULL != libmediaHandle)
			{
				dlclose(libmediaHandle);
				libmediaHandle = NULL;
			}
#endif // !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
			if (NULL != nexAudio)
			{
				Nex_AR::NexAudio_using_jni::destroy(nexAudio);
				nexAudio = NULL;
			}
		}

		void RALCallback(int msg, int ext1, int ext2, int ext3, void * ext4)
		{
			if (NULL != fnCallback)
			{
				fnCallback(msg, ext1, ext2, ext3, ext4, callbackUD);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "RAL Callback function not set\n");
			}
		}

        //yoon		
		// Available from API lv.23, Marshmallow.
		int32_t AudioManagerGetOutputLatency(uint32_t *latency, int streamType)
		{
			*latency = nexAudio->getOutputLatency(streamType);
			return 0;
		}
		
		int32_t AudioManagerGetOutputFrameCount(int *frameCount, int stream)
		{
			return nexAudio->getOutputFramesPerBuffer();
		}
		
		int32_t AudioManagerGetOutputSamplingRate(int *samplingRate, int stream)
		{
			return nexAudio->getOutputSampleRate();
		}

#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
		NXINT32 /*android::status_t*/ AudioSystemGetOutputLatency(NXUINT32 *latency, int streamType)
		{
			if (NULL != fnGetOutputLatency)
			{
				return fnGetOutputLatency(latency, streamType);
			}
			return -ENODEV; /*NO_INIT*/
		}

		NXINT32 /*android::status_t*/ AudioSystemGetOutputFrameCount(int *frameCount, int stream)
		{
			if (NULL != fnGetOutputFrameCount)
			{
				return fnGetOutputFrameCount(frameCount, stream);
			}
			return -ENODEV; /*NO_INIT*/
		}

		NXINT32 /*android::status_t*/ AudioSystemGetOutputSamplingRate(int *samplingRate, int stream)
		{
			if (NULL != fnGetOutputSamplingRate)
			{
				return fnGetOutputSamplingRate(samplingRate, stream);
			}
			return -ENODEV; /*NO_INIT*/
		}
#endif // !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)

		bool isValid()
		{
#if defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
			return NULL != nexAudio;
#else // defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
            return ((NULL != libmediaHandle && (NULL != fnGetOutputFrameCount && NULL != fnGetOutputSamplingRate)) || NULL != nexAudio);
#endif // else !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
		}

		bool decRef()
		{
			int ret = nexSAL_AtomicDec(&refCount);
			if (1 == ret)
			{
				delete this;
				return true;
			}
			return false;
		}

		void incRef()
		{
			nexSAL_AtomicInc(&refCount);
		}

		NXUINT32 SetParam(NXUINT32 uiMavenMode, NXUINT32 uiMavenStrength, NXUINT32 uiBassStrength)
		{
			CSALMutex::Auto autolock(lock);
			requestedMavenMode = uiMavenMode;
			requestedMavenStrength = uiMavenStrength;
			requestedBassStrength = uiBassStrength;
			return 0;
		}

		void GetRequestedParams(Int16 &mavenMode, Uint32 &mavenStrength, Uint32 &bassStrength)
		{
			CSALMutex::Auto autolock(lock);
			mavenMode = requestedMavenMode;
			mavenStrength = requestedMavenStrength;
			bassStrength = requestedBassStrength;
		}

		NXUINT32 SetAutoVolumeParam(NXUINT32 uiEnable, NXUINT32 uiStrength, NXUINT32 uiReleaseTime)
		{
			CSALMutex::Auto autolock(lock);
			requestedAutoVolumeEnable = uiEnable;
			requestedAutoVolumeStrength = uiStrength;
			requestedAutoVolumeReleaseTime = uiReleaseTime;
			return 0;
		}

		void GetRequestedAutoVolumeParams(Int16 &enable, Uint32 &strength, Uint32 &releaseTime)
		{
			CSALMutex::Auto autolock(lock);
			enable = requestedAutoVolumeEnable;
			strength = requestedAutoVolumeStrength;
			releaseTime = requestedAutoVolumeReleaseTime;
		}

		NXUINT32 SetRequestedSpeed(int speed)
		{
			CSALMutex::Auto autolock(lock);
			requestedSpeed = speed;
			return 0;
		}

		int GetRequestedSpeed()
		{
			CSALMutex::Auto autolock(lock);
			return requestedSpeed;
		}

		NXUINT32 SetRequestedSoundPath(int soundPath)
		{
			CSALMutex::Auto autolock(lock);
			requestedSoundPath = soundPath;
			return 0;
		}

		Int16 GetRequestedSoundPath()
		{
			CSALMutex::Auto autolock(lock);
			return requestedSoundPath;
		}

		NXUINT32 SetRequestedVolume(float fGain)
		{
			CSALMutex::Auto autolock(lock);
			requestedVolume = fGain;
			return 0;
		}

		float GetRequestedVolume()
		{
			CSALMutex::Auto autolock(lock);
			return requestedVolume;
		}

	private:
		CSALMutex lock;
		int refCount;
#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
		void * libmediaHandle;
		AudioSystemGetOutputLatency_t fnGetOutputLatency;
		AudioSystemGetOutputFrameCount_t fnGetOutputFrameCount;
		AudioSystemGetOutputSamplingRate_t fnGetOutputSamplingRate;
#endif // !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
		FNRALCALLBACK fnCallback;
		Nex_AR::NexAudio *nexAudio;
		Int16 requestedMavenMode;
		Uint32 requestedMavenStrength;
		Uint32 requestedBassStrength;
		Int16 requestedAutoVolumeEnable;
		Uint32 requestedAutoVolumeStrength;
		Uint32 requestedAutoVolumeReleaseTime;
		int requestedSpeed;
		Int16 requestedSoundPath;

		float requestedVolume;
	public:
		bool useAudioEffect;
		void * callbackUD;
		int audioSessionId;
		void * lastRegisteredAudioRenderer;
		bool isMarshmallowOrAbove; //yoon
	};

	void * g_instance[MAX_INSTANCES] = { NULL, };

	pthread_mutex_t g_instancesLock;

	class InstancesInitializer
	{
	public:
		InstancesInitializer()
		{
			pthread_mutex_init(&g_instancesLock, NULL);
		}
		~InstancesInitializer()
		{
			pthread_mutex_destroy(&g_instancesLock);
		}
	};

	InstancesInitializer instancesInitializer;
} // namespace (anon1)

namespace { // (anon2)
	class NexAudioRenderer
	{
	private:
		typedef enum State
		{
			  Stopped = 1
			, Paused = 2
			, Playing = 3
		} State;
	public:
		NexAudioRenderer(
				  NexAudioRendererInstance *nari
				, NXUINT32 uCodecType
				, NXUINT32 uSamplingRate
				, NXUINT32 uNumOfChannels
				, NXUINT32 uBitsPerSample
				, NXUINT32 uNumOfSamplesPerChannel )
			: nexAudioRendererInstance(nari)
			, codecType(uCodecType)
			, samplingRate(uSamplingRate)
			, numChannels(uNumOfChannels)
			, bitsPerSample(uBitsPerSample)
			, numSamplesPerChannel(uNumOfSamplesPerChannel)
			, msecsPerFrame(0.0f)
			, frameSize(0)
			, oneFrameSizeInBytes(0)
			, memoryBuffer(NULL)
			, firstCTS(0xFFFFFFFF)
			, currentCTS(0)
			, nextExpectedCTS(0)
			, endCTS(0xFFFFFFFF)
			, nexAudioTrack(NULL)
			, scratchOutputBuffer(NULL)
			, numInputSamplesPerChannelRequired(1152)
			, numFramesWrittenToAudioTrack(0)
			, currentSpeed(0)
			, requestedSpeed(0)
			, currentState(Paused)
			, requestedState(Playing)
			, isFirstDrain(true)
			, currentCTSAdjustment(0)
			, mavenMode(0)
			, mavenStrength(0)
			, bassStrength(0)
			, soundPath(0)
			, requestedSoundPath(0)
			, autoVolumeEnable(0)
			, autoVolumeStrength(0)
			, autoVolumeReleaseTime(0)
			, cachedTSFramePos(0LL)
			, cachedTSExpectedPTSTime(0LL)
			, cachedTSExists(false)
			, cachedIsArtificiallyShifted(false)
			, trustedTimeStamp(true)
			, maxCTS(0xFFFFFFFFLL)
			, preservedCurrentTime(0LL)
			, pausedTime(0xFFFFFFFF)
			, lastGetPlaybackHeadPositionTick(0)
			, lastPlaybackHeadPosition(0)
			, numWraps(0)
			, receivedEndFrame(false)
			, getCurrentCTS_func(NULL)
			, getPlaybackHeadPosition_func(NULL)
			, systemOutputLatency(0)
			, nativeFrameCount(0)
			, previousSystemCurrentTime(0LL)
			, bufferSizeFactor(1)
			, decreasedHeadPosition(false)
			, arm_emulator_mode(false)
		{
			if (NULL != nexAudioRendererInstance)
			{
				nexAudioRendererInstance->incRef();
				nexAudioRendererInstance->lastRegisteredAudioRenderer = this;
			}

			if ( 768 > numSamplesPerChannel &&
				(  eNEX_CODEC_A_WMA == codecType
				|| eNEX_CODEC_A_WMA_SPEECH == codecType
				|| eNEX_CODEC_A_APE == codecType
				|| eNEX_CODEC_A_FLAC == codecType
				|| eNEX_CODEC_A_VORBIS == codecType ) )
			{
				numSamplesPerChannel = 2048;
			}

			msecsPerFrame = 1.e3 / (float)samplingRate;
			frameSize = numChannels * (bitsPerSample>>3);

			oneFrameSizeInBytes = numSamplesPerChannel * frameSize;

#ifdef FOR_PROJECT_Tinno
			int numBuffers = 600;
#else
			int numBuffers = 240;
#endif

			while (NULL == memoryBuffer && numBuffers >= 15)
			{
				memoryBuffer = new CircularMemoryBuffer(numBuffers, 4*1024, 4); // 960k cap, 16k extra
				numBuffers /= 2;
			}

			if (NULL == memoryBuffer)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Couldn't create memory buffer!\n");
			}

			int channelConfig = 0;

			switch (numChannels)
			{
			case 1:
				channelConfig = 4; /* AudioFormat.CHANNEL_OUT_MONO */
				break;
			case 2:
				channelConfig = 0xC; /* AudioFormat.CHANNEL_OUT_STEREO */
				break;
			case 6:
				channelConfig = 0xFC; /* AudioFormat.CHANNEL_OUT_5POINT1 */
				break;
			case 8:
				channelConfig = 0x3FC; /* AudioFormat.CHANNEL_OUT_7POINT1 */
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "found unexpected channel count(%u)\n", numChannels);
				channelConfig = 1; /* AudioFormat.CHANNEL_OUT_DEFAULT */
				break;
			}

			numInputSamplesPerChannelRequired = 1152;
#ifdef FOR_PROJECT_Tinno
			bufferSizeFactor = 9;
#else
			bufferSizeFactor = 4;
#endif

			bool kindleKitkatDevice = false;
			
#if 1 //for kindle kitkat
			char nexSysPropertys[PROPERTY_VALUE_MAX];
			int nexPropertyLen = 0;
			memset( nexSysPropertys, 0x00, PROPERTY_VALUE_MAX );

			nexPropertyLen = Nex_AR::Utils::GetSystemProperty("ro.product.manufacturer", nexSysPropertys);
			if (0 < nexPropertyLen)
			{
				if (!strncasecmp("Amazon", nexSysPropertys, sizeof("Amazon") - 1))
				{
					memset( nexSysPropertys, 0x00, PROPERTY_VALUE_MAX );
					Nex_AR::Utils::GetSystemProperty("ro.build.version.release", nexSysPropertys);
					if (!strncasecmp("4.4", nexSysPropertys, sizeof("4.4") - 1))
					{
						kindleKitkatDevice = true;
						bufferSizeFactor = 2;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 0, "Amazon Kitkat(%s) devices\n", nexSysPropertys);
					}						
				}			
			}	
#endif	//#if 1
			int minBufferSize = Nex_AR::NexAudioTrack_using_jni::getMinBufferSize(samplingRate, channelConfig, 0x2 /* AudioFormat.ENCODING_PCM_16BIT */);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "AudioTrack minBufferSize(%d)\n", minBufferSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "before AudioTrack audioSessionId(%d)\n", nexAudioRendererInstance->audioSessionId);
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 0, "AudioTrack buffer size(%d) : minBufferSize(%d), bufferSizeFactor(%d), memoryBuffer count(%d)\n", minBufferSize * bufferSizeFactor, minBufferSize, bufferSizeFactor, numBuffers);
			if(0 > nexAudioRendererInstance->audioSessionId)
				nexAudioTrack = Nex_AR::NexAudioTrack_using_jni::create(3 /* AudioManager.STREAM_MUSIC */, samplingRate, channelConfig, 0x2 /* AudioFormat.ENCODING_PCM_16BIT */, minBufferSize * bufferSizeFactor, 0x1 /* AudioTrack.MODE_STREAM */);
			else
				nexAudioTrack = Nex_AR::NexAudioTrack_using_jni::create(3 /* AudioManager.STREAM_MUSIC */, samplingRate, channelConfig, 0x2 /* AudioFormat.ENCODING_PCM_16BIT */, minBufferSize * bufferSizeFactor, 0x1 /* AudioTrack.MODE_STREAM */, nexAudioRendererInstance->audioSessionId);

			nexAudioRendererInstance->audioSessionId = nexAudioTrack->getAudioSessionId();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "after AudioTrack audioSessionId(%d)\n", nexAudioRendererInstance->audioSessionId);
			float requestedVolume = nexAudioRendererInstance->GetRequestedVolume();
			if (nexAudioRendererInstance->audioSessionId != 0)
			{
				if (-1.0f != requestedVolume)
				{
					SetVolume(requestedVolume);
				}

				lastTimeInfo.speed = 0.0f;
				lastTimeInfo.playbackHeadPosition = 0;
				lastTimeInfo.pts = 0;

				bufferSizeCreated = minBufferSize * bufferSizeFactor;
				if (NULL != Nex_AR::JNI::JCLASS::AudioTimestamp)
				{
					getCurrentCTS_func = &NexAudioRenderer::getCurrentCTS_kk;
					getPlaybackHeadPosition_func = &NexAudioRenderer::getPlaybackHeadPosition_kk;
					if(kindleKitkatDevice)
					{
						getCurrentCTS_func = &NexAudioRenderer::getCurrentCTS_Kindle_kk;
					}

					nativeFrameCount = nexAudioTrack->getNativeFrameCount() / bufferSizeFactor; // should simply be minBufferSize / sizeOfSample...
					if (-1 == nativeFrameCount)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "couldn't get native frame count\n");
						nativeFrameCount = 0;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "AudioTrack nativeFrameCount(%d)\n", nativeFrameCount);
					}				
				}
				else
				{
					getCurrentCTS_func = &NexAudioRenderer::getCurrentCTS_jb_and_below;
					getPlaybackHeadPosition_func = &NexAudioRenderer::getPlaybackHeadPosition_jb_and_below;

					nativeFrameCount = nexAudioTrack->getNativeFrameCount() / bufferSizeFactor; // should simply be minBufferSize / sizeOfSample...
					if (-1 == nativeFrameCount)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "couldn't get native frame count\n");
						nativeFrameCount = 0;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "AudioTrack nativeFrameCount(%d)\n", nativeFrameCount);
					}
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "AudioTrack audioSessionId(%d)\n", nexAudioRendererInstance->audioSessionId);
			}
		}

		~NexAudioRenderer()
		{
			NexAudioRendererInstance *nari = NULL;

			if (NULL != nexAudioRendererInstance)
			{
				if (isValid())
				{
					nari = nexAudioRendererInstance;
					nari->incRef();
				}
				if (this == nexAudioRendererInstance->lastRegisteredAudioRenderer)
				{
					nexAudioRendererInstance->lastRegisteredAudioRenderer = NULL;
				}
				nexAudioRendererInstance->decRef();
				nexAudioRendererInstance = NULL;
			}

			if (NULL != memoryBuffer)
			{
				delete memoryBuffer;
				memoryBuffer = NULL;
			}

			if (NULL != nexAudioTrack)
			{
				if ( isValid() )
					nexAudioTrack->stop();
				nexAudioTrack->release();
				Nex_AR::NexAudioTrack_using_jni::destroy(nexAudioTrack);
				nexAudioTrack = NULL;
			}

			if (NULL != scratchOutputBuffer)
			{
				nexSAL_MemFree(scratchOutputBuffer);
				scratchOutputBuffer = NULL;
			}

			if (NULL != nari)
			{
				nari->decRef();
			}
		}

		bool isValid()
		{
			return NULL != nexAudioRendererInstance && nexAudioRendererInstance->isValid() && NULL != memoryBuffer && NULL != nexAudioTrack && 0 != nexAudioRendererInstance->audioSessionId;
		}

		NXUINT32 getEmptyBuffer(void **ppEmptyBuffer, NXINT32 *nMaxBufferSize)
		{
			void *ret = NULL;

#if defined(_USE_SINK_THREAD_)
			NXUINT32 startTick = nexSAL_GetTickCount();

			do
			{
				if (1000 > millisBuffered()) // don't buffer more than 1 second
				{
#endif // defined(_USE_SINK_THREAD_)
					ret = memoryBuffer->GetWriteBuffer(384 * 1024);

					if (NULL != ret)
					{
						*ppEmptyBuffer = ret;
						*nMaxBufferSize = 384*1024;
						return 0;
					}
#if defined(_USE_SINK_THREAD_)
				}
				nexSAL_TaskSleep(5);
			} while (1000 > nexSAL_GetTickCount() - startTick); // TODO: rethink this timeout
#endif // defined(_USE_SINK_THREAD_)

			*ppEmptyBuffer = NULL;
			*nMaxBufferSize = 0;
			return 0;
		}

		NXUINT32 consumeBuffer(void * pBuffer, int nBufferLen, NXUINT32 uCTS, int isDecodeSuccess, int bEndFrame)
		{
			if (0xFFFFFFFF == firstCTS)
			{
				firstCTS = uCTS;
			}

			if (!receivedEndFrame)
			{
				NXUINT32 writtenTime = memoryBuffer->TotalWritten() * msecsPerFrame / frameSize;
				NXUINT32 expectedCTS = firstCTS + writtenTime;

				const int DIFF_EPSILON = 2;

				int diff = (int)expectedCTS - (int)uCTS;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "DIFF_EPSILON expectedCTS(%d), uCTS(%d), diff(%d)\n",expectedCTS, uCTS, diff);
				if (DIFF_EPSILON < ABS(diff) || DIFF_EPSILON < ABS((int)nextExpectedCTS - (int)uCTS))
				{
					CTSAdjustmentElement d = { memoryBuffer->TotalWritten(), diff };
					{
						CSALMutex::Auto autolock(lock);
						ctsAdjustmentQueue.push(d);
					}
				}
			}

			currentCTS = uCTS;
            
			if (!receivedEndFrame)
			{
				NXUINT32 written = memoryBuffer->Write(pBuffer, nBufferLen);
				nextExpectedCTS = currentCTS + written * msecsPerFrame / frameSize;

				if (0 != bEndFrame)
				{
					receivedEndFrame = true;
					endCTS = nextExpectedCTS + 1;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "received end frame: endCTS(%u)\n", endCTS);
				}
			}

#if !defined(_USE_SINK_THREAD_)
			drainToAudioTrack(); // blocking
#else // !defined(_USE_SINK_THREAD_)
			// TODO:
#endif // else defined(_USE_SINK_THREAD_)

			return 0;
		}

		NXUINT32 setBufferMute(void * pBuffer, bool bSetPCMSize, int *piWrittenPCMSize)
		{
			if (NULL == pBuffer)
			{
				*piWrittenPCMSize = 0;
				return 0;
			}

                    if(!bSetPCMSize) {
			*piWrittenPCMSize = oneFrameSizeInBytes;
                    }

			memset( pBuffer, 0x00, *piWrittenPCMSize );
			return 0;
		}

		NXUINT32 getCurrentCTS(NXUINT32 *puCTS)
		{
			int playState = nexAudioTrack->getPlayState();

			if (0xFFFFFFFF != pausedTime && 0x2 /* AudioTrack.PLAYSTATE_PAUSED */ == playState)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "returning paused time (%u)\n", pausedTime);
				*puCTS = pausedTime;
				return 0;
			}

			NXINT64 playbackHeadPosition = getPlaybackHeadPosition();

			if (0 == playbackHeadPosition)
			{
				*puCTS = (NXUINT32)(MIN(endCTS, (MIN(maxCTS, (MAX(0LL, lastTimeInfo.pts)))))); // need this many brackets because of poor MIN/MAX defitions in nexSal_internal.h
				return 0;
			}

			return CALL_MEMBER_FN(*this,getCurrentCTS_func)(playState, playbackHeadPosition, puCTS);
		}

		NXUINT32 setProperty(NXUINT32 uProperty, NXUINT32 uValue)
		{
			switch (uProperty)
			{
			case NEXRAL_PROPERTY_AUDIO_ARM_EMULATOR_MODE:
				if (arm_emulator_mode != (0 != uValue))
				{
					CSALMutex::Auto autolock(lock);
					arm_emulator_mode = 0 != uValue;
					systemOutputLatency = 0;
				}
				break;
			default:
				break;
			}
			return 0;
		}

	private:
		void getSystemOutputLatencyWithoutLibmedia()
		{
			int nativeOutputSampleRate = Nex_AR::NexAudioTrack_using_jni::getNativeOutputSampleRate(3 /* AudioManager.STREAM_MUSIC */);
			if (-1 == nativeOutputSampleRate)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "couldn't get native output sample rate\n");
				systemOutputLatency = 0;
			}
			else
			{
				nativeFrameCount = nexAudioTrack->getNativeFrameCount() / bufferSizeFactor;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "AudioTrack nativeOutputSampleRate(%d)\n", nativeOutputSampleRate);
				systemOutputLatency = (NXUINT32)((nativeFrameCount / (nativeOutputSampleRate / 1000.0f)) + 0.5f);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "Calculated AudioSystem systemOutputLatency(%u)\n", systemOutputLatency);
			}
		}

		NXUINT32 getSystemOutputLatency()
		{
			NXINT64 currentTime = getMonotonicClockInNanos(); 
			NXINT64 diffTime = (currentTime-previousSystemCurrentTime)/1000000LL;
			if(diffTime > 1000) // to check the systemoutput latency per 1sec.
			{
				previousSystemCurrentTime = currentTime;
				systemOutputLatency = 0;
			}

			NXINT32 ret = systemOutputLatency;
			if (0 == ret)
			{
				CSALMutex::Auto autolock(lock);

				if (arm_emulator_mode)
				{
					getSystemOutputLatencyWithoutLibmedia();
				}
				else if(nexAudioRendererInstance->isMarshmallowOrAbove) //yoon
				{
					ret = nexAudioRendererInstance->AudioManagerGetOutputLatency(&systemOutputLatency, 3);
				}
				else
				{
#if !defined(_REMOVE_LIBMEDIA_DEPENDENCY_)
					NXINT32 ret = 0;
					if (NULL != nexAudioRendererInstance)
					{
						ret = nexAudioRendererInstance->AudioSystemGetOutputLatency(&systemOutputLatency, 3 /* AudioSystem::MUSIC or AUDIO_STREAM_MUSIC */);
					}
					if (-ENODEV == ret)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "couldn't get system output latency\n");
						systemOutputLatency = 0;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "AudioSystem systemOutputLatency(%u)\n", systemOutputLatency);
					}
#else
					getSystemOutputLatencyWithoutLibmedia();
#endif
				}

				ret = systemOutputLatency;
			}

			return ret;
		}

		NXUINT32 getCurrentCTS_jb_and_below(int playState, NXINT64 playbackHeadPosition /* != 0 */, NXUINT32 *puCTS)
		{
			NXINT64 retValue = 0LL;

			//int latencyInFrames = nativeFrameCount + (NXUINT32)(getSystemOutputLatency() / msecsPerFrame);
			int latencyInFrames = (NXUINT32)(getSystemOutputLatency() / msecsPerFrame);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "playbackHeadPosition(%lld) latencyInFrames(%d)", playbackHeadPosition, latencyInFrames);


			if (!timeInfoQueue.empty())
			{
				CSALMutex::Auto autolock(lock);

				while (!timeInfoQueue.empty())
				{
					AudioTrackTimeInformation atti = timeInfoQueue.front();

					if (atti.playbackHeadPosition <= playbackHeadPosition)
					{
						timeInfoQueue.pop();

						lastTimeInfo = atti;
					}
					else
					{
						break;
					}
				}
			}

			// we simply assume a latency of nativeFrameCount frames:
			// playbackHeadPosition -= nativeFrameCount;

			// + a system output latency (converted to frame count):
			// playbackHeadPosition -= (getSystemOutputLatency() / msecsPerFrame);

			playbackHeadPosition -= latencyInFrames;

			if (0 > playbackHeadPosition)
			{
				playbackHeadPosition = 0;
			}

			NXINT64 phpDiff = playbackHeadPosition - lastTimeInfo.playbackHeadPosition;

			retValue = (NXINT64)(phpDiff / (double)samplingRate * 1000LL * lastTimeInfo.speed);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "lti_pts(%d) lti_php(%lld) lti_speed(%f) phpDiff(%lld) diffDuration(%lld)\n", lastTimeInfo.pts, lastTimeInfo.playbackHeadPosition, lastTimeInfo.speed, phpDiff, retValue);
			// nexSAL_DebugPrintf("lti_pts(%d) lti_php(%lld) lti_speed(%f) phpDiff(%lld) diffDuration(%lld)\n", lastTimeInfo.pts, lastTimeInfo.playbackHeadPosition, lastTimeInfo.speed, phpDiff, retValue);

			retValue += lastTimeInfo.pts;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "maxCTS(%lld) diff(%lld)\n", maxCTS, maxCTS - retValue);

			*puCTS = (NXUINT32)(MIN(endCTS, (MIN(maxCTS, (MAX(0LL, retValue)))))); // need this many brackets because of poor MIN/MAX defitions in nexSal_internal.h
			return 0;
		}

		NXUINT32 getCurrentCTS_kk(int playState, NXINT64 playbackHeadPosition /* != 0 */, NXUINT32 *puCTS)
		{
			NXINT64 retValue = 0LL;

			int phpInMillis = (int)(playbackHeadPosition / (double)samplingRate * 1000LL * lastTimeInfo.speed);

			NXINT64 currentTime = getMonotonicClockInNanos();
			NXINT64 diffTime = 0LL;

			NXINT64 tsFramePos = 0LL, tsExpectedPTSTime = 0LL;
			bool tsExists = false;

			int framePosOffset = 0LL;

			if (0x3 /* AudioTrack.PLAYSTATE_PLAYING */ == playState && cachedIsArtificiallyShifted)
			{
				tsExists = nexAudioTrack->getTimestamp(tsFramePos, tsExpectedPTSTime);

				if (tsExists)
				{
					currentTime = getMonotonicClockInNanos(); // in case getTimestamp() takes a long time

					diffTime = (currentTime-tsExpectedPTSTime)/1000000LL;

					if (500LL /*500ms*/ > diffTime) // legit
					{
						// we need to start heading towards the correct time... smoothly...

						int calcedPlaybackHeadPosition = (int)((currentTime-tsExpectedPTSTime)/1000LL*(double)samplingRate/1000000LL) + tsFramePos;

						int cachedCalcedPlaybackHeadPosition = (int)((currentTime-cachedTSExpectedPTSTime)/1000LL*(double)samplingRate/1000000LL) + cachedTSFramePos;

						int cachedDiff = (currentTime - cachedTSExpectedPTSTime)/1000000LL;

						// we want the cached time to gradually reach actual time -- equal when cachedDiff==500ms

						float offsetFactor = cachedDiff/500.0;
						framePosOffset = (calcedPlaybackHeadPosition - cachedCalcedPlaybackHeadPosition) * offsetFactor;
					}
				}
			}

			if (0x2 /* AudioTrack.PLAYSTATE_PAUSED */ == playState || (cachedTSExists && ((500000000LL /*500ms*/ > (currentTime - cachedTSExpectedPTSTime)) || (Stopped == currentState))))
			{
				tsExists = cachedTSExists;
				tsFramePos = cachedTSFramePos;
				tsExpectedPTSTime = cachedTSExpectedPTSTime;

				diffTime = (currentTime-tsExpectedPTSTime)/1000000LL;
			}
			else
			{
				tsExists = nexAudioTrack->getTimestamp(tsFramePos, tsExpectedPTSTime);

				// if (receivedEndFrame)
				// {
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5,"tsExists(%s) tsFramePos(%lld) tsExpectedPTSTime(%lld)\n", tsExists ? "true" : "false", tsFramePos, tsExpectedPTSTime);
				// }

				if(tsExists)
				{
					int tsFramePosInMillis = (int)(tsFramePos / (double)samplingRate * 1000LL * lastTimeInfo.speed);
					int latancy = (int)(nativeFrameCount / (double)samplingRate * 1000LL) + getSystemOutputLatency();
					int DIFF_MAX = latancy * 2; 
					if( DIFF_MAX < ABS(phpInMillis-tsFramePosInMillis))
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "not trusted tsExists-phpInMillis(%d) tsFramePosInMillis(%d) framediff(%d)\n",
				 			phpInMillis, tsFramePosInMillis, phpInMillis-tsFramePosInMillis);						
						trustedTimeStamp = false;
					}
					else
					{
						trustedTimeStamp = true;
					}
				}
				if (false == trustedTimeStamp || playbackHeadPosition < tsFramePos)
				{
					tsExists = false;
					tsFramePos = 0;
					tsExpectedPTSTime = 0;
				}

				if (tsExists)
				{
					currentTime = getMonotonicClockInNanos(); // in case getTimestamp() takes a long time

					diffTime = (currentTime-tsExpectedPTSTime)/1000000LL;

					if (500LL /*500ms*/ < diffTime)
					{
						if (cachedTSExists)
						{
							tsExists = cachedTSExists;
							tsFramePos = cachedTSFramePos;
							tsExpectedPTSTime = cachedTSExpectedPTSTime;
						}
						else
						{
							tsExists = false;
							tsFramePos = 0;
							tsExpectedPTSTime = 0;
						}
					}
					else
					{
						cachedTSExists = tsExists;
						cachedTSFramePos = tsFramePos;
						cachedTSExpectedPTSTime = tsExpectedPTSTime;
						cachedIsArtificiallyShifted = false;
						framePosOffset = 0LL;
					}
				}
			}

			if (tsExists && tsFramePos != 0)
			{
				int tsFramePosInMillis = (int)(tsFramePos / (double)samplingRate * 1000LL * lastTimeInfo.speed);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "gcp2-php(%lld) tsFramePos(%lld)\n", playbackHeadPosition, tsFramePos);

				NXINT64 calcedPlaybackHeadPosition = (NXINT64)((currentTime-tsExpectedPTSTime)/1000LL*(double)samplingRate/1000000LL) + tsFramePos + framePosOffset;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "gcp-phpInMillis(%d) tsFramePosInMillis(%d) framediff(%d) diff(%lld) calcedPhp(%lld)\n",
				 			phpInMillis, tsFramePosInMillis, phpInMillis-tsFramePosInMillis, diffTime, calcedPlaybackHeadPosition);

				playbackHeadPosition = calcedPlaybackHeadPosition;

			}
			else
			{
				int latencyInFrames = (NXUINT32)(getSystemOutputLatency() / msecsPerFrame);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "tsExists(false). playbackHeadPosition(%lld) latencyInFrames(%d)", playbackHeadPosition, latencyInFrames);
				playbackHeadPosition = MAX(0LL, playbackHeadPosition - latencyInFrames);

				phpInMillis = (int)(playbackHeadPosition / (double)samplingRate * 1000LL * lastTimeInfo.speed);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "gcp-phpInMillis(%d)\n", phpInMillis);
			}

			if (!timeInfoQueue.empty())
			{
				CSALMutex::Auto autolock(lock);

				while (!timeInfoQueue.empty())
				{
					AudioTrackTimeInformation atti = timeInfoQueue.front();

					if (atti.playbackHeadPosition <= playbackHeadPosition)
					{
						timeInfoQueue.pop();

						lastTimeInfo = atti;
					}
					else
					{
						break;
					}
				}
			}

			NXINT64 phpDiff = playbackHeadPosition - lastTimeInfo.playbackHeadPosition;

			retValue = (NXINT64)(phpDiff / (double)samplingRate * 1000LL * lastTimeInfo.speed);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5,"lti_pts(%d) lti_php(%lld) lti_speed(%f) phpDiff(%lld) diffDuration(%lld) maxCTS(%lld)\n", lastTimeInfo.pts, lastTimeInfo.playbackHeadPosition, lastTimeInfo.speed, phpDiff, retValue, maxCTS);
			retValue += lastTimeInfo.pts;

			*puCTS = (NXUINT32)(MIN(endCTS, (MIN(maxCTS, (MAX(0LL, retValue)))))); // need this many brackets because of poor MIN/MAX defitions in nexSal_internal.h
			return 0;
		}
		NXUINT32 getCurrentCTS_Kindle_kk(int playState, NXINT64 playbackHeadPosition /* != 0 */, NXUINT32 *puCTS)
		{
			NXINT64 retValue = 0LL;

			int phpInMillis = (int)(playbackHeadPosition / (double)samplingRate * 1000LL * lastTimeInfo.speed);

			NXINT64 currentTime = getMonotonicClockInNanos();
			NXINT64 diffTime = 0LL;

			NXINT64 tsFramePos = 0LL, tsExpectedPTSTime = 0LL;
			bool tsExists = false;

			int framePosOffset = 0LL;

			if (0x3 /* AudioTrack.PLAYSTATE_PLAYING */ == playState && cachedIsArtificiallyShifted)
			{
				tsExists = nexAudioTrack->getTimestamp(tsFramePos, tsExpectedPTSTime);

				if (tsExists)
				{
					currentTime = getMonotonicClockInNanos(); // in case getTimestamp() takes a long time

					diffTime = (currentTime-tsExpectedPTSTime)/1000000LL;

					if (500LL /*500ms*/ > diffTime) // legit
					{
						// we need to start heading towards the correct time... smoothly...

						int calcedPlaybackHeadPosition = (int)((currentTime-tsExpectedPTSTime)/1000LL*(double)samplingRate/1000000LL) + tsFramePos;

						int cachedCalcedPlaybackHeadPosition = (int)((currentTime-cachedTSExpectedPTSTime)/1000LL*(double)samplingRate/1000000LL) + cachedTSFramePos;

						int cachedDiff = (currentTime - cachedTSExpectedPTSTime)/1000000LL;

						// we want the cached time to gradually reach actual time -- equal when cachedDiff==500ms

						float offsetFactor = cachedDiff/500.0;
						framePosOffset = (calcedPlaybackHeadPosition - cachedCalcedPlaybackHeadPosition) * offsetFactor;
					}
				}
			}

			if (0x2 /* AudioTrack.PLAYSTATE_PAUSED */ == playState || (cachedTSExists && ((500000000LL /*500ms*/ > (currentTime - cachedTSExpectedPTSTime)) || (Stopped == currentState))))
			{
				tsExists = cachedTSExists;
				tsFramePos = cachedTSFramePos;
				tsExpectedPTSTime = cachedTSExpectedPTSTime;

				diffTime = (currentTime-tsExpectedPTSTime)/1000000LL;
			}
			else
			{
				tsExists = nexAudioTrack->getTimestamp(tsFramePos, tsExpectedPTSTime);

				// if (receivedEndFrame)
				// {
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5,"tsExists(%s) tsFramePos(%lld) tsExpectedPTSTime(%lld)\n", tsExists ? "true" : "false", tsFramePos, tsExpectedPTSTime);
				// }

				
				if(tsExists)
				{
					int tsFramePosInMillis = (int)(tsFramePos / (double)samplingRate * 1000LL * lastTimeInfo.speed);
					int latancy = (int)(nativeFrameCount / (double)samplingRate * 1000LL) + getSystemOutputLatency();
					int DIFF_MAX = latancy * 2; 
					if( DIFF_MAX < ABS(phpInMillis-tsFramePosInMillis))
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "not trusted tsExists-phpInMillis(%d) tsFramePosInMillis(%d) framediff(%d)\n",
				 			phpInMillis, tsFramePosInMillis, phpInMillis-tsFramePosInMillis);						
						trustedTimeStamp = false;
					}
					else
					{
						trustedTimeStamp = true;
					}
				}

				
				if (false == trustedTimeStamp) // || playbackHeadPosition < tsFramePos)
				{
					tsExists = false;
					tsFramePos = 0;
					tsExpectedPTSTime = 0;
				}

				if (tsExists)
				{
					currentTime = getMonotonicClockInNanos(); // in case getTimestamp() takes a long time

					diffTime = (currentTime-tsExpectedPTSTime)/1000000LL;

					if (500LL /*500ms*/ < diffTime)
					{
						if (cachedTSExists)
						{
							tsExists = cachedTSExists;
							tsFramePos = cachedTSFramePos;
							tsExpectedPTSTime = cachedTSExpectedPTSTime;
						}
						else
						{
							tsExists = false;
							tsFramePos = 0;
							tsExpectedPTSTime = 0;
						}
					}
					else
					{
						cachedTSExists = tsExists;
						cachedTSFramePos = tsFramePos;
						cachedTSExpectedPTSTime = tsExpectedPTSTime;
						cachedIsArtificiallyShifted = false;
						framePosOffset = 0LL;
					}
				}
			}

			if (tsExists)
			{
				int latencyInFrames2 = (NXUINT32)(getSystemOutputLatency() / msecsPerFrame);
				int latencyInFrames3 = bufferSizeCreated /(2*numChannels);
				//int latencyInFrames = latencyInFrames2 -(latencyInFrames3);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "Kindle playbackHeadPosition(%lld) latencyInFrames(%d + %d)", playbackHeadPosition, latencyInFrames2,latencyInFrames3);

				int tsFramePosInMillis = (int)(tsFramePos / (double)samplingRate * 1000LL * lastTimeInfo.speed);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "gcp2-php(%lld) tsFramePos(%lld)\n", playbackHeadPosition, tsFramePos);

				NXINT64 calcedPlaybackHeadPosition = (NXINT64)((currentTime-tsExpectedPTSTime)/1000LL*(double)samplingRate/1000000LL) + tsFramePos + framePosOffset;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "gcp-phpInMillis(%d) tsFramePosInMillis(%d) framediff(%d) diff(%lld) calcedPhp(%lld)\n",
				 			phpInMillis, tsFramePosInMillis, phpInMillis-tsFramePosInMillis, diffTime, calcedPlaybackHeadPosition);

				playbackHeadPosition = calcedPlaybackHeadPosition;

			}
			else
			{

				int latencyInFrames2 = (NXUINT32)(getSystemOutputLatency() / msecsPerFrame);
				int latencyInFrames3 = bufferSizeCreated /(2*numChannels);
				int latencyInFrames = latencyInFrames2 -(latencyInFrames3);
				//int latencyInFrames = -(latencyInFrames3);
	
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "Kindle playbackHeadPosition(%lld) latencyInFrames(%d + %d -> %d)", playbackHeadPosition, latencyInFrames2,latencyInFrames3,latencyInFrames);
				if(playbackHeadPosition > 0)
					playbackHeadPosition = MAX(0LL, playbackHeadPosition - latencyInFrames);

				phpInMillis = (int)(playbackHeadPosition / (double)samplingRate * 1000LL * lastTimeInfo.speed);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "gcp-phpInMillis(%d)\n", phpInMillis);

			}

			if (!timeInfoQueue.empty())
			{
				CSALMutex::Auto autolock(lock);

				while (!timeInfoQueue.empty())
				{
					AudioTrackTimeInformation atti = timeInfoQueue.front();

					if (atti.playbackHeadPosition <= playbackHeadPosition)
					{
						timeInfoQueue.pop();

						lastTimeInfo = atti;
					}
					else
					{
						break;
					}
				}
			}

			NXINT64 phpDiff = MAX(0LL, playbackHeadPosition - lastTimeInfo.playbackHeadPosition);

			retValue = (NXINT64)(phpDiff / (double)samplingRate * 1000LL * lastTimeInfo.speed);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5,"lti_pts(%d) lti_php(%lld) lti_speed(%f) phpDiff(%lld) diffDuration(%lld) maxCTS(%lld)\n", lastTimeInfo.pts, lastTimeInfo.playbackHeadPosition, lastTimeInfo.speed, phpDiff, retValue, maxCTS);
			retValue += lastTimeInfo.pts;

			//*puCTS = (NXUINT32)(MIN(endCTS, (MIN(maxCTS, (MAX(0LL, retValue)))))); // need this many brackets because of poor MIN/MAX defitions in nexSal_internal.h
			*puCTS = (NXUINT32)(MIN(endCTS, (MAX(0LL, retValue)))); // need this many brackets because of poor MIN/MAX defitions in nexSal_internal.h
			return 0;
		}
	public:
		NXUINT32 clearBuffer()
		{
			CSALMutex::Auto autolock(lock);
			NXUINT32 cts = 0;
			getCurrentCTS(&cts);
			flush(cts);
			return 0;
		}

		NXUINT32 pause()
		{
			CSALMutex::Auto autolock(lock); // re-entrant... thankfully

			if (currentState != Playing)
			{
				return 0;
			}

			requestedState = Paused;

			NXUINT32 playbackHeadPosition = getPlaybackHeadPosition();
			int phpInMillis = (int)(playbackHeadPosition / (double)samplingRate * 1000LL * lastTimeInfo.speed);
			NXUINT32 firstPHP = playbackHeadPosition;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "pause: php pre-pause: %u -- %d\n", playbackHeadPosition, phpInMillis);

			bool tempCachedTSExists = cachedTSExists;
			cachedTSExists = false;
			NXUINT32 cts;
			getCurrentCTS(&cts);

			if (!cachedTSExists)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 3, "pause: no cache exists\n");
				cachedTSExists = tempCachedTSExists;
			}

			if (cachedTSExists)
			{
				preservedCurrentTime = getMonotonicClockInNanos();
				pausedTime = cts;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "preservations made (%lld) pausedTime(%u)\n", preservedCurrentTime, pausedTime);
			}

			pauseAudioTrack();
			while (0x3 /* AudioTrack.PLAYSTATE_PLAYING */ == nexAudioTrack->getPlayState())
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "waiting for audio track to go to paused state\n");
				nexSAL_TaskSleep(1);
			}

			const NXUINT32 timeout = 100; // 100ms
			NXUINT32 startTick = nexSAL_GetTickCount();
			do
			{
				playbackHeadPosition = getPlaybackHeadPosition(true);
				phpInMillis = (int)(playbackHeadPosition / (double)samplingRate * 1000LL * lastTimeInfo.speed);
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "pause: changing php after pause: %u -- %d\n", playbackHeadPosition, phpInMillis);
				if (firstPHP != playbackHeadPosition || timeout <= nexSAL_GetTickCount() - startTick)
					break;
				nexSAL_TaskSleep(1);
			} while (true);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "pause: php post pause: %u -- %d\n", playbackHeadPosition, phpInMillis);
			return 0;
		}

		NXUINT32 resume()
		{
			CSALMutex::Auto autolock(lock); // re-entrant... thankfully

			requestedState = Playing;

			if (cachedTSExists)
			{
				NXINT64 currentTime = getMonotonicClockInNanos();
				cachedTSExpectedPTSTime += currentTime - preservedCurrentTime;
				cachedIsArtificiallyShifted = true;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "cached expected presentation time shifted up (%lld) nanos\n", currentTime - preservedCurrentTime);
				pausedTime = 0xFFFFFFFF;
			}

			drainToAudioTrack();
			return 0;
		}

		NXUINT32 flush(NXUINT32 uCTS)
		{
			CSALMutex::Auto autolock(lock);

			memoryBuffer->Reset();
			firstCTS = uCTS;
			currentCTS = uCTS;
			nextExpectedCTS = uCTS;
			endCTS = 0xFFFFFFFF;
			nexAudioTrack->flush();
			numFramesWrittenToAudioTrack = 0;
			numWraps = 0;
			isFirstDrain = true;

			while (!ctsAdjustmentQueue.empty())
			{
				ctsAdjustmentQueue.pop();
			}

			currentCTSAdjustment = 0;
			cachedTSFramePos = 0LL;
			cachedTSExpectedPTSTime = 0LL;
			cachedTSExists = false;
			cachedIsArtificiallyShifted = false;
			trustedTimeStamp = true;
			maxCTS = 0xFFFFFFFFLL;
			preservedCurrentTime = 0LL;
			pausedTime = 0xFFFFFFFF;

			while (!timeInfoQueue.empty())
			{
				timeInfoQueue.pop();
			}

			lastTimeInfo.playbackHeadPosition = 0;
			lastTimeInfo.pts = uCTS;

			lastGetPlaybackHeadPositionTick = 0;
			lastPlaybackHeadPosition = 0;
			decreasedHeadPosition = false;

			receivedEndFrame = false;

			return 0;
		}

		NXUINT32 setTime(NXUINT32 uCTS)
		{
			CSALMutex::Auto autolock(lock);
			firstCTS = uCTS;
			currentCTS = uCTS;
			nextExpectedCTS = uCTS;

			while (!ctsAdjustmentQueue.empty())
			{
				ctsAdjustmentQueue.pop();
			}

			currentCTSAdjustment = 0;

			while (!timeInfoQueue.empty())
			{
				timeInfoQueue.pop();
			}
			lastTimeInfo.playbackHeadPosition = 0;
			lastTimeInfo.pts = uCTS;
			return 0;
		}

		NXUINT32 setPlaybackRate(NXINT32 iRate)
		{
			CSALMutex::Auto autolock(lock);
			requestedSpeed = iRate;
			return 0;
		}

		NXUINT32 SetSoundPath(NXINT32 iPath)
		{
			CSALMutex::Auto autolock(lock);
			requestedSoundPath = 0;
			return 0;
		}

		NXUINT32 SetVolume(float fGain)
		{
			if (NULL == nexAudioTrack)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "audio track is NULL\n");
				return 0;
			}

			return nexAudioTrack->setStereoVolume(fGain, fGain);
		}

		NXUINT32 GetAudioSessionId()
		{
			if (NULL == nexAudioTrack)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "audio track is NULL\n");
				return 0;
			}

			return nexAudioTrack->getAudioSessionId();
		}
	private:
		typedef struct AudioTrackTimeInformation
		{
			float speed;
			NXINT64 playbackHeadPosition;
			int pts;

			AudioTrackTimeInformation& operator=(const AudioTrackTimeInformation &rhs)
			{
				if (this != &rhs)
				{
					speed = rhs.speed;
					playbackHeadPosition = rhs.playbackHeadPosition;
					pts = rhs.pts;
				}
				return *this;
			}
		} AudioTrackTimeInformation;

		typedef struct CTSAdjustmentElement {
			NXUINT64 byteOffset;
			NXINT32 ctsAdj; // in millisec

			CTSAdjustmentElement& operator=(const CTSAdjustmentElement &rhs)
			{
				if (this == &rhs)
				{
					byteOffset = rhs.byteOffset;
					ctsAdj = rhs.ctsAdj;
				}
				return *this;
			}
		} CTSAdjustmentElement;

		NXUINT32 millisBuffered()
		{
			return memoryBuffer->Size() * msecsPerFrame / frameSize;
		}

		void startAudioTrack()
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "startAudioTrack+\n");

			CSALMutex::Auto autolock(lock);

			if (NULL != nexAudioTrack && 0x3 /* AudioTrack.PLAYSTATE_PLAYING */ != nexAudioTrack->getPlayState())
			{
				nexAudioTrack->play();
			}
			lastPlaybackHeadPosition = 0;
			lastGetPlaybackHeadPositionTick = 0;
			getPlaybackHeadPosition();

			currentState = Playing;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "startAudioTrack-\n");
		}

		void pauseAudioTrack()
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "pauseAudioTrack+\n");

			CSALMutex::Auto autolock(lock);

			if (NULL != nexAudioTrack && 0x3 /* AudioTrack.PLAYSTATE_PLAYING */ == nexAudioTrack->getPlayState())
			{
				nexAudioTrack->pause();
			}

			currentState = Paused;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "pauseAudioTrack-\n");
		}

		void stopAudioTrack()
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "stopAudioTrack+\n");

			CSALMutex::Auto autolock(lock);

			if (NULL != nexAudioTrack && 0x1 /* AudioTrack.PLAYSTATE_STOPPED */ != nexAudioTrack->getPlayState())
			{
 				// AudioTrack.html: When used on an instance created in MODE_STREAM mode, audio will stop playing after the last buffer that was written has been played.
				nexAudioTrack->stop();
			}

			currentState = Stopped;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "stopAudioTrack-\n");
		}

		void fulfillPlayStateChangeRequest_l() // locked
		{
			if (currentState != requestedState)
			{
				switch (requestedState)
				{
				case Stopped:
					stopAudioTrack();
					break;
				case Paused:
					pauseAudioTrack();
					break;
				case Playing:
					startAudioTrack();
					break;
				}
			}
		}

		void checkFirstDrain_l() // locked
		{
			if (isFirstDrain)
			{
				while (!timeInfoQueue.empty())
				{
					timeInfoQueue.pop();
				}

				numFramesWrittenToAudioTrack = getPlaybackHeadPosition(true);

				if (0x7FFFFFFF == requestedSpeed)
				{
					requestedSpeed = currentSpeed;
				}

				isFirstDrain = false;
			}
		}

		void checkForAdjustments_l()
		{
			while (!ctsAdjustmentQueue.empty())
			{
				CTSAdjustmentElement adj = ctsAdjustmentQueue.front();
				if (adj.byteOffset > memoryBuffer->TotalRead())
				{
					break;
				}

				if (0x7FFFFFFF == requestedSpeed)
				{
					requestedSpeed = currentSpeed;
				}
				currentCTSAdjustment = adj.ctsAdj;
				ctsAdjustmentQueue.pop();
			}
		}

		void setSpeedIfNecessary_l()
		{
			if (0x7FFFFFFF != requestedSpeed)
			{
				AudioTrackTimeInformation atti;
				atti.playbackHeadPosition = numFramesWrittenToAudioTrack;
				atti.pts = firstCTS + memoryBuffer->TotalRead()*msecsPerFrame/frameSize - currentCTSAdjustment;
				atti.speed = numInputSamplesPerChannelRequired / 1152.0f;

				timeInfoQueue.push(atti);

				currentSpeed = requestedSpeed;

				requestedSpeed = 0x7FFFFFFF;
			}
		}

		void drainToAudioTrack()
		{
			void * tempInputBuffer = NULL;
			size_t tempInputBufferSize = 0;

			int writeSize = 1152 * frameSize;

			lock.Lock(); // for checking memoryBuffer size
			while (0 < memoryBuffer->Size())
			{
				{
					CSALMutex::Auto autolock(lock); // remember: this is a re-entrant lock
					lock.Unlock(); // was for checking memoryBuffer size

					fulfillPlayStateChangeRequest_l();

					if (Playing != currentState)
					{
						break;
					}

					checkFirstDrain_l();

					checkForAdjustments_l();

					setSpeedIfNecessary_l();
				}

				NXUINT32 numBytesRequiredForProcess = numInputSamplesPerChannelRequired * frameSize;
				NXUINT32 bytesToRead = numBytesRequiredForProcess;
				void *readBuffer = memoryBuffer->GetReadBuffer(bytesToRead);

				if (NULL == readBuffer)
				{
					if (!receivedEndFrame)
						break;

					readBuffer = memoryBuffer->GetReadBuffer(bytesToRead); // just in case more was written (we are not holding a lock at this point)

					if (NULL == readBuffer)
					{
						NXUINT32 bufferSize = memoryBuffer->Size();
						if (0 == bufferSize)
							break;

						if (bufferSize > numBytesRequiredForProcess)
						{
							// this should never happen.
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "audio: end reached but buffer size(%u) > required size(%u)\n", bufferSize, numBytesRequiredForProcess);
							break;
						}

						if (NULL != tempInputBuffer && tempInputBufferSize < bytesToRead)
						{
							nexSAL_MemFree(tempInputBuffer);
							tempInputBuffer = NULL;
						}

						if (NULL == tempInputBuffer)
						{
							tempInputBuffer = nexSAL_MemAlloc(numBytesRequiredForProcess);
							tempInputBufferSize = numBytesRequiredForProcess;
						}

						if (NULL == tempInputBuffer)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "audio: end reached but couldn't allocate buffer to output the last bit(%u)\n", numBytesRequiredForProcess);
							break;
						}

						memset(tempInputBuffer, 0x00, tempInputBufferSize);
						memoryBuffer->Read(tempInputBuffer, bufferSize);
						bytesToRead = 0;

						readBuffer = tempInputBuffer;

						writeSize = bufferSize;
						
					}
				}

				void *outputBuffer = NULL;
				outputBuffer = readBuffer;
				

				writeSize -= (writeSize % frameSize);
				numFramesWrittenToAudioTrack += (writeSize / frameSize);
				nexAudioTrack->write((unsigned char *)outputBuffer, 0, writeSize);

				if (0 < bytesToRead)
				{
					memoryBuffer->Read(readBuffer, bytesToRead);
				}
				else // eos written
				{
					stopAudioTrack();
				}

				lock.Lock(); // for checking memoryBuffer size
			}
			lock.Unlock(); // was for checking memoryBuffer size

			if (NULL != tempInputBuffer)
			{
				nexSAL_MemFree(tempInputBuffer);
			}
		}

		NXINT64 getPlaybackHeadPosition(bool force = false)
		{
			return CALL_MEMBER_FN(*this, getPlaybackHeadPosition_func)(force);
		}
		NXINT64 getPlaybackHeadPosition_jb_and_below(bool force)
		{
			NXINT64 ret = 0;
			NXUINT32 currentTick = nexSAL_GetTickCount();

			NXUINT32 uGapInFrames = 0 != lastGetPlaybackHeadPositionTick ? (currentTick - lastGetPlaybackHeadPositionTick) * (double)samplingRate / 1000LL : 0;
			NXINT64 audiotrackFrameCount = nativeFrameCount * (bufferSizeFactor + 1);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "getphp: cur(%u) lasttick(%u)\n", currentTick, lastGetPlaybackHeadPositionTick);
			if (!force && 10 > currentTick - lastGetPlaybackHeadPositionTick) // call only once every 10ms
			{
				return lastPlaybackHeadPosition + uGapInFrames;
			}

			if (NULL != nexAudioTrack)
			{
				if (0 /* STATE_UNINITIALIZED */ != nexAudioTrack->getState())
				{
					ret = nexAudioTrack->getPlaybackHeadPosition();
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "getPlaybackHeadPosition(%lld), lastPlaybackHeadPosition(%lld), audioTrackFrameCount(%lld), uGapInFrames(%u)\n", ret, lastPlaybackHeadPosition, audiotrackFrameCount, uGapInFrames );
					if (0 != ret || (0 != numWraps && (!receivedEndFrame || 1 /* AudioTrack.PLAYSTATE_STOPPED */ != nexAudioTrack->getPlayState())))
					{
						ret += (numWraps * 0x100000000LL);
						CSALMutex::Auto autolock(lock);

						if (lastPlaybackHeadPosition > ret && 0xFFF00000 < (lastPlaybackHeadPosition - ret))
						{
							++numWraps;
							ret += 0x100000000LL;
						}

						//for kindle fire hdx
						if(!decreasedHeadPosition && lastPlaybackHeadPosition > ret)
						{
							if(audiotrackFrameCount >= lastPlaybackHeadPosition)
							{
								decreasedHeadPosition = true;
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 3, "playbackHeadPosition was shrunk : lastPlaybackHeadPosition(%lld), getPlaybackHeadPosition(%lld) \n",lastPlaybackHeadPosition, ret);
							}
						}

						if(decreasedHeadPosition)
						{
							ret += audiotrackFrameCount;
						}
						if (0 == lastPlaybackHeadPosition || lastPlaybackHeadPosition < ret)
						{
							maxCTS = lastTimeInfo.pts + (int)((ret - lastTimeInfo.playbackHeadPosition) / (double)samplingRate * 1000LL * lastTimeInfo.speed);

							//if ((ret - lastPlaybackHeadPosition) >= uGapInFrames) // only update if it seems more accurate
							if(1)
							{
								lastGetPlaybackHeadPositionTick = currentTick;
								lastPlaybackHeadPosition = ret;
							}
							else
							{
								ret = lastPlaybackHeadPosition + uGapInFrames;
							}
						}
						else
						{
							ret = lastPlaybackHeadPosition + uGapInFrames;
						}
					}

					if (0 == ret && receivedEndFrame && 1 /* AudioTrack.PLAYSTATE_STOPPED */ == nexAudioTrack->getPlayState())
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "audiotrack is stopped. got 0 for php. returning 0x7FFFFFFFFFFFFFFFLL instead.\n");
						ret = 0x7FFFFFFFFFFFFFFFLL;
						maxCTS = endCTS;
					}
				}
			}
			return ret;
		}
		NXINT64 getPlaybackHeadPosition_kk(bool force)
		{
			NXINT64 ret = 0;
			NXUINT32 currentTick = nexSAL_GetTickCount();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 5, "getphp: cur(%u) lasttick(%u)\n", currentTick, lastGetPlaybackHeadPositionTick);
			if (!force && 10 > currentTick - lastGetPlaybackHeadPositionTick) // call only once every 10ms
			{
				return lastPlaybackHeadPosition;
			}

			if (NULL != nexAudioTrack)
			{
				if (0 /* STATE_UNINITIALIZED */ != nexAudioTrack->getState())
				{
					ret = nexAudioTrack->getPlaybackHeadPosition();

					if (0 != ret || (0 != numWraps && (!receivedEndFrame || 1 /* AudioTrack.PLAYSTATE_STOPPED */ != nexAudioTrack->getPlayState())))
					{
						ret += (numWraps * 0x100000000LL);
						CSALMutex::Auto autolock(lock);

						if (lastPlaybackHeadPosition > ret && 0xFFF00000 < (lastPlaybackHeadPosition - ret))
						{
							++numWraps;
							ret += 0x100000000LL;
						}

						if (lastPlaybackHeadPosition < ret)
						{
							maxCTS = lastTimeInfo.pts + (int)((ret - lastTimeInfo.playbackHeadPosition) / (double)samplingRate * 1000LL * lastTimeInfo.speed);

							lastGetPlaybackHeadPositionTick = currentTick;
							lastPlaybackHeadPosition = ret;
						}
						else
						{
							ret = lastPlaybackHeadPosition;
						}
					}

					if (0 == ret && receivedEndFrame && 1 /* AudioTrack.PLAYSTATE_STOPPED */ == nexAudioTrack->getPlayState())
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "audiotrack is stopped. got 0 for php. returning 0x7FFFFFFFFFFFFFFFLL instead.\n");
						ret = 0x7FFFFFFFFFFFFFFFLL;
						maxCTS = endCTS;
					}
				}
			}
			return ret;
		}

		CSALMutex lock; // re-entrant lock necessary
		NexAudioRendererInstance *nexAudioRendererInstance;
		NXUINT32 codecType;
		NXUINT32 samplingRate;
		NXUINT32 numChannels;
		NXUINT32 bitsPerSample;
		NXUINT32 numSamplesPerChannel;
		float msecsPerFrame;
		size_t frameSize;
		NXUINT32 oneFrameSizeInBytes;
		MemoryBuffer *memoryBuffer;

		NXUINT32 firstCTS;

		NXUINT32 currentCTS;
		NXUINT32 nextExpectedCTS;

		NXUINT32 endCTS;

		Nex_AR::NexAudioTrack *nexAudioTrack;
		Int16 *scratchOutputBuffer;

		Int32 numInputSamplesPerChannelRequired;
		NXINT64 numFramesWrittenToAudioTrack;
		int currentSpeed;
		int requestedSpeed;

		State currentState;
		State requestedState;

		bool isFirstDrain;

		AudioTrackTimeInformation lastTimeInfo;
		std::queue<AudioTrackTimeInformation> timeInfoQueue;
		std::queue<CTSAdjustmentElement> ctsAdjustmentQueue;
		NXINT32 currentCTSAdjustment;

		Int16 mavenMode;
		Uint32 mavenStrength;
		Uint32 bassStrength;

		Int16 soundPath;
		Int16 requestedSoundPath;

		Int16 autoVolumeEnable;
		Uint32 autoVolumeStrength;
		Uint32 autoVolumeReleaseTime;

		NXINT64 cachedTSFramePos;
		NXINT64 cachedTSExpectedPTSTime;
		bool cachedTSExists;
		bool cachedIsArtificiallyShifted;
		bool trustedTimeStamp; //martin
		NXINT64 maxCTS;
		NXINT64 preservedCurrentTime;
		NXUINT32 pausedTime;

		NXUINT32 lastGetPlaybackHeadPositionTick;
		NXINT64 lastPlaybackHeadPosition;

		int numWraps;

		bool receivedEndFrame;

		typedef NXUINT32 (NexAudioRenderer::*getCurrentCTS_Fn)(int playState, NXINT64 playbackHeadPosition /* != 0 */, NXUINT32 *puCTS);
		getCurrentCTS_Fn getCurrentCTS_func;

		typedef NXINT64 (NexAudioRenderer::*getPlaybackHeadPosition_Fn)(bool force);
		getPlaybackHeadPosition_Fn getPlaybackHeadPosition_func;

		NXUINT32 systemOutputLatency;
		NXINT32 nativeFrameCount;
		NXINT64 previousSystemCurrentTime;

		NXINT32 bufferSizeFactor;
		bool decreasedHeadPosition;
		bool arm_emulator_mode;
		NXINT32 bufferSizeCreated;
	};
} // namespace (anon2)

namespace { // (anon3)

size_t findInstanceIndexWithPlayerID(void* uUserData)
{
	for (size_t i = 0; i < MAX_INSTANCES; ++i)
	{
		NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[i];
		if (NULL != nari && nari->isValid())
		{
			if (nari->callbackUD == uUserData)
			{
				return i;
			}
		}
	}
	return (size_t)-1;
}

size_t findInstanceIndexWithAudioRenderer(void* uUserData)
{
	for (size_t i = 0; i < MAX_INSTANCES; ++i)
	{
		NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[i];
		if (NULL != nari && nari->isValid())
		{
			if (nari->lastRegisteredAudioRenderer == uUserData)
			{
				return i;
			}
		}
	}
	return (size_t)-1;
}

} // namespace (anon3)

EXPORTED2
unsigned int nexRALBody_Audio_getProperty( unsigned int uProperty, unsigned int *puValue, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "getProperty+ property(0x%x) userData(0x%x)\n", uProperty, uUserData);
	switch(uProperty)
	{
	case NEXRAL_PROPERTY_AUDIO_RENDERER_MODE:
		*puValue = NEXRAL_PROPERTY_AUDIO_RENDERER_MODE_NORMAL;
		break;

	default:
		*puValue = 0;
		break;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "getProperty- property(0x%x) value(0x%x) userData(0x%x)\n", uProperty, *puValue, uUserData);
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_setProperty( unsigned int uProperty, unsigned int uValue, void* uUserData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "setProperty+ property(0x%x) value(0x%x) userData(0x%x)\n", uProperty, uValue, uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->setProperty(uProperty, uValue);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "setProperty- property(0x%x) value(0x%x) userData(0x%x) ret(0x%x)\n", uProperty, uValue, uUserData, ret);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_init ( unsigned int uCodecType
                                   , unsigned int uSamplingRate
                                   , unsigned int uNumOfChannels
                                   , unsigned int uBitsPerSample
                                   , unsigned int uNumOfSamplesPerChannel
                                   , void** puUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "init+ codecType(0x%x) samplingRate(%u) numOfChannels(%u) bitsPerSample(%u) numSamplesPerCh(%u)\n", uCodecType, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	size_t instanceNo = findInstanceIndexWithPlayerID(*puUserData);
	*puUserData = NULL;
	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return 1;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "instanceNo(%d)\n",instanceNo);
	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return 1;
	}

	NexAudioRenderer *audioRenderer = new NexAudioRenderer(nari, uCodecType, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	if (NULL == audioRenderer)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't initialize audio renderer instance\n");
		return 1;
	}
	else if (!audioRenderer->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't initialize audio renderer properly\n");
		delete audioRenderer;
		return 1;
	}

	*puUserData = audioRenderer;

	nari->RALCallback( 0x60001, uSamplingRate, uNumOfChannels, 0, NULL );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "init- ud(0x%x)\n", *puUserData);
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_deinit( void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "deinit+ ud(0x%x)\n", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	size_t instanceNo = findInstanceIndexWithAudioRenderer(uUserData);

	delete audioRenderer;

	((NexAudioRendererInstance *)g_instance[instanceNo])->RALCallback( 0x60002, 0, 0, 0, NULL );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "deinit- ud(0x%x)\n", uUserData);
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_getEmptyBuffer( void** ppEmptyBuffer, int* nMaxBufferSize, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "getEmptyBuffer+ ud(0x%x)\n", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->getEmptyBuffer(ppEmptyBuffer, nMaxBufferSize);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "getEmptyBuffer- emptyBuf(%p) maxBufSize(%d) ret(0x%x) ud(0x%x)\n", *ppEmptyBuffer, *nMaxBufferSize, ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_consumeBuffer( void* pBuffer, int nBufferLen, unsigned int uCTS, int isDecodeSuccess, int bEndFrame, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "consumeBuffer+ buffer(%p) bufferLen(%d) uCTS(%d) decSuccess(%s) endFrame(%s) ud(0x%x)\n", pBuffer, nBufferLen, uCTS, 0 != isDecodeSuccess ? "true" : "false", 0 != bEndFrame ? "true" : "false", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->consumeBuffer(pBuffer, nBufferLen, uCTS, isDecodeSuccess, bEndFrame);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "consumeBuffer- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_setBufferMute( void* pBuffer, bool bSetPCMSize, int* piWrittenPCMSize, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "setBufferMute+ buffer(%p) ud(0x%x)\n", pBuffer, uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	if (NULL == piWrittenPCMSize)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "piWrittenPCMSize is NULL\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->setBufferMute(pBuffer, bSetPCMSize, piWrittenPCMSize);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "setBufferMute- ret(0x%x) writtenSize(%d) ud(0x%x)\n", ret, *piWrittenPCMSize, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_getCurrentCTS( unsigned int* puCTS, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 3, "getCurrentCTS+ ud(0x%x)\n", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->getCurrentCTS(puCTS);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 4, "getCurrentCTS- ret(0x%x) cts(%u) ud(0x%x)\n", ret, *puCTS, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_clearBuffer( void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "clearBuffer+ ud(0x%x)\n", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->clearBuffer();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "clearBuffer- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_pause( void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "pause+ ud(0x%x)\n", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->pause();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "pause- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_resume( void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "resume+ ud(0x%x)\n", uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->resume();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "resume- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_flush( unsigned int uCTS, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "flush+ cts(%u) ud(0x%x)\n", uCTS, uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->flush(uCTS);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "flush- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_setTime( unsigned int uCTS, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "setTime+ cts(%u) ud(0x%x)\n", uCTS, uUserData);

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "invalid userdata given\n");
		return -1;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	NXUINT32 ret = audioRenderer->setTime(uCTS);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "setTime- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_setPlaybackRate( int iRate, void* uUserData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "setPlaybackRate+ rate(%d) ud(0x%x)\n", iRate, uUserData);

	size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);

	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -1;
	}

	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -2;
	}

	uUserData = nari->lastRegisteredAudioRenderer;

	if ((unsigned int)NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "none init'd yet. Caching requested playback rate...\n");

		NXUINT32 ret = nari->SetRequestedSpeed(iRate);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "setPlaybackRate- ret(0x%x) ud(0x%x)\n", ret, uUserData);
		return ret;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	nari->SetRequestedSpeed(iRate);
	NXUINT32 ret = audioRenderer->setPlaybackRate(iRate);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "setPlaybackRate- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_SetSoundPath( int iPath, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetSoundPath+ path(%d) ud(0x%x)\n", iPath, uUserData);

	size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);

	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -1;
	}

	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -2;
	}

	uUserData = nari->lastRegisteredAudioRenderer;

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "none init'd yet. Caching requested sound path...\n");

		NXUINT32 ret = nari->SetRequestedSoundPath(iPath);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "SetSoundPath- ret(0x%x) ud(0x%x)\n", ret, uUserData);
		return ret;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	nari->SetRequestedSoundPath(iPath);
	NXUINT32 ret = audioRenderer->SetSoundPath(iPath);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "SetSoundPath- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_create(int nLogLevel, unsigned int bUseAudioEffect, void * /*jobject*/ audioManager,  FNRALCALLBACK fnCallback, void *pUserData)
{
	// nLogLevel = 5;
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_FLOW, 		nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_AUDIO, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_VIDEO, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_AUDIO, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_VIDEO, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_SYS, 		nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_SYS, 		nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_INFO, 		nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_WARNING, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_ERR, 		nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_F_READER, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_F_WRITER, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_CRAL, 		nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_SOURCE, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_TARGET, 	nLogLevel);
	nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_NONE, 		nLogLevel);

	nexSAL_DebugPrintf("========================================================\n");
	nexSAL_DebugPrintf("++++++++++++NexRALBody Audio Information %d.%d.%d.%s\n",	NEXRALBODY_VERSION_MAJOR,
																					NEXRALBODY_VERSION_MINOR,
																					NEXRALBODY_VERSION_PATCH,
																					xstr(NEXRALBODY_VERSION_BUILD));
	nexSAL_DebugPrintf("SDK Information : %s\n", 									NEXRALBODY_SDK_INFORMATION);
	nexSAL_DebugPrintf(" + nLogLevel(%d)\n", 										nLogLevel);
	nexSAL_DebugPrintf("========================================================\n");

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "create+ useAudioEffect(%s)\n", FALSE != bUseAudioEffect ? "true" : "false");

	NexAudioRendererInstance *instance = NULL;

	size_t instanceNo = (size_t)-1;

	pthread_mutex_lock(&g_instancesLock);
	for (size_t i = 0; MAX_INSTANCES > i; ++i)
	{
		if (NULL == g_instance[i])
		{
			instanceNo = i;
			break;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "instanceNo(%d) \n", instanceNo);
	if (MAX_INSTANCES <= instanceNo)
	{
		pthread_mutex_unlock(&g_instancesLock);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "no handles available (max %d). close some first\n", MAX_INSTANCES);
		return -1;
	}

	if (NULL == g_instance[instanceNo])
	{
		void *cbUserData = NULL;
		if (NULL != pUserData)
		{
			void* hPlayer = pUserData;
			cbUserData = hPlayer;
		}
		instance = new NexAudioRendererInstance(fnCallback, cbUserData, audioManager, FALSE != bUseAudioEffect);
		if (!instance->isValid())
		{
			delete instance;
			instance = NULL;
			pthread_mutex_unlock(&g_instancesLock);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't create audio ralbody instance\n");
			return -1;
		}
		g_instance[instanceNo] = (void *)instance;
	}
	else
	{
		instance = (NexAudioRendererInstance *)g_instance[instanceNo];
		instance->incRef();
	}

	pthread_mutex_unlock(&g_instancesLock);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "create-\n");
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_delete(void* uUserData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "delete+\n");

	pthread_mutex_lock(&g_instancesLock);

	size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);

	if (MAX_INSTANCES <= instanceNo || NULL == g_instance[instanceNo])
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "received invalid instanceNo to delete\n");
	}
	else
	{
		NexAudioRendererInstance *instance = (NexAudioRendererInstance *)g_instance[instanceNo];
		bool deleted = instance->decRef();
		if (deleted)
		{
			g_instance[instanceNo] = NULL;
		}
	}

	pthread_mutex_unlock(&g_instancesLock);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "delete-\n");
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_getAudioSessionId(unsigned int* puSessionId, void* uUserData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "GetAudioSessionId+ ud(0x%x)\n", uUserData);

	//size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);
	size_t instanceNo = findInstanceIndexWithAudioRenderer(uUserData);

	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -1;
	}

	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -2;
	}

	uUserData = nari->lastRegisteredAudioRenderer;

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "none init'd yet. returning 0...\n");

		*puSessionId = 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "GetAudioSessionId- Id(0x%x) ud(0x%x)\n", *puSessionId, uUserData);
		return 0;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	*puSessionId = audioRenderer->GetAudioSessionId();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "GetAudioSessionId- Id(0x%x) ud(0x%x)\n", *puSessionId, uUserData);
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_prepareAudioTrack(void* pAudioTrack, void* uUserData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "prepareAudioTrack+- not doing anything...\n");
	// TODO:
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_SetVolume(float fGain, void* uUserData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetVolume+ gain(%f) ud(0x%x)\n", fGain, uUserData);

	size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);

	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -1;
	}

	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -2;
	}

	uUserData = nari->lastRegisteredAudioRenderer;

	if (NULL == uUserData)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "none init'd yet. Caching requested volume...\n");

		NXUINT32 ret = nari->SetRequestedVolume(fGain);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "SetVolume- ret(0x%x) ud(0x%x)\n", ret, uUserData);
		return ret;
	}

	NexAudioRenderer *audioRenderer = (NexAudioRenderer *)uUserData;

	nari->SetRequestedVolume(fGain);
	NXUINT32 ret = audioRenderer->SetVolume(fGain);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "SetVolume- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return 0;
}

EXPORTED2
unsigned int nexRALBody_Audio_MavenSetParam( unsigned int uiMavenMode, unsigned int uiMavenStrength, unsigned int uiBassStrength, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetParam+ mode(%u) strength(%u) bassStrength(%u) ud(0x%x)\n", uiMavenMode, uiMavenStrength, uiBassStrength, uUserData);

	size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);

	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -1;
	}

	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -2;
	}

	NXUINT32 ret = nari->SetParam(uiMavenMode, uiMavenStrength, uiBassStrength);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "SetParam- ret(0x%x) ud(0x%x)\n", ret, uUserData);
	return ret;
}

EXPORTED2
unsigned int nexRALBody_Audio_MavenSetAutoVolumeParam( unsigned int uiEnable, unsigned int uiStrength, unsigned int uiReleaseTime, void* uUserData )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetAutoVolumeParam+\n");

	size_t instanceNo = findInstanceIndexWithPlayerID(uUserData);

	if (MAX_INSTANCES <= instanceNo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -1;
	}

	NexAudioRendererInstance *nari = (NexAudioRendererInstance *)g_instance[instanceNo];
	if (NULL == nari || !nari->isValid())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "couldn't find a valid created audio renderer instance\n");
		return -2;
	}

	NXUINT32 ret = nari->SetAutoVolumeParam(uiEnable, uiStrength, uiReleaseTime);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "SetAutoVolumeParam- ret(0x%x)\n", ret);
	return ret;
}

namespace {
AUDIO_RALBODY_FUNCTION_ST g_audioRendererHandle =
{
      /* audioRendererHandle.fnNexRALBody_Audio_getProperty             = */ nexRALBody_Audio_getProperty
    , /* audioRendererHandle.fnNexRALBody_Audio_setProperty             = */ nexRALBody_Audio_setProperty
    , /* audioRendererHandle.fnNexRALBody_Audio_init                    = */ nexRALBody_Audio_init
    , /* audioRendererHandle.fnNexRALBody_Audio_deinit                  = */ nexRALBody_Audio_deinit
    , /* audioRendererHandle.fnNexRALBody_Audio_getEmptyBuffer          = */ nexRALBody_Audio_getEmptyBuffer
    , /* audioRendererHandle.fnNexRALBody_Audio_consumeBuffer           = */ nexRALBody_Audio_consumeBuffer
    , /* audioRendererHandle.fnNexRALBody_Audio_setBufferMute           = */ nexRALBody_Audio_setBufferMute
    , /* audioRendererHandle.fnNexRALBody_Audio_getCurrentCTS           = */ nexRALBody_Audio_getCurrentCTS
    , /* audioRendererHandle.fnNexRALBody_Audio_clearBuffer             = */ nexRALBody_Audio_clearBuffer
    , /* audioRendererHandle.fnNexRALBody_Audio_pause                   = */ nexRALBody_Audio_pause
    , /* audioRendererHandle.fnNexRALBody_Audio_resume                  = */ nexRALBody_Audio_resume
    , /* audioRendererHandle.fnNexRALBody_Audio_flush                   = */ nexRALBody_Audio_flush
    , /* audioRendererHandle.fnNexRALBody_Audio_setTime                 = */ nexRALBody_Audio_setTime
    , /* audioRendererHandle.fnNexRALBody_Audio_setPlaybackRate         = */ nexRALBody_Audio_setPlaybackRate
    , /* audioRendererHandle.fnNexRALBody_Audio_SetSoundPath            = */ nexRALBody_Audio_SetSoundPath
    , /* audioRendererHandle.fnNexRALBody_Audio_create                  = */ nexRALBody_Audio_create
    , /* audioRendererHandle.fnNexRALBody_Audio_delete                  = */ nexRALBody_Audio_delete
    , /* audioRendererHandle.fnNexRALBody_Audio_getAudioSessionId       = */ nexRALBody_Audio_getAudioSessionId
    , /* audioRendererHandle.fnNexRALBody_Audio_prepareAudioTrack       = */ nexRALBody_Audio_prepareAudioTrack
    , /* audioRendererHandle.fnNexRALBody_Audio_SetVolume               = */ nexRALBody_Audio_SetVolume
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenInit               = */ NULL/*nexRALBody_Audio_MavenInit*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetVolume          = */ NULL/*nexRALBody_Audio_MavenSetVolume*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetOutput          = */ NULL/*nexRALBody_Audio_MavenSetOutput*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetParam           = */ nexRALBody_Audio_MavenSetParam
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetAutoVolumeParam = */ nexRALBody_Audio_MavenSetAutoVolumeParam
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSEQSetParam        = */ NULL/*nexRALBody_Audio_MavenSEQSetParam*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenVMS2SetParam       = */ NULL/*nexRALBody_Audio_MavenVMS2SetParam*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenVMS2FilterSetMode  = */ NULL/*nexRALBody_Audio_MavenVMS2FilterSetMode*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenVMS2FilterSetParam = */ NULL/*nexRALBody_Audio_MavenVMS2FilterSetParam*/
};
}

namespace { // (anon5)

void * InternalGetHandle()
{
	Nex_AR::Utils::initializeAll();
	return &g_audioRendererHandle;
}

void InternalSetJavaVM(JavaVM* javaVM) //yoon
{
	Nex_AR::Utils::JNI::setJavaVMfromExternal(javaVM);
}

void InternalCloseHandle( void *pHandle )
{
	Nex_AR::Utils::deinitializeAll();
}

} // namespace (anon5)

#ifdef __cplusplus
extern "C"
{
#endif

EXPORTED
void * GetHandle()
{
	return InternalGetHandle();
}

void SetJavaVM(JavaVM* javaVM) //yoon
{
	return InternalSetJavaVM(javaVM);
}

EXPORTED
void CloseHandle( void *pHandle )
{
	InternalCloseHandle(pHandle);
}

EXPORTED
void SetJavaVMforRender(JavaVM* javaVM)
{
	Nex_AR::Utils::JNI::setJavaVMfromExternal(javaVM);
}

EXPORTED
unsigned int nexRALBody_Audio_Get_RALBody( void *pFunctions )
{
    AUDIO_RALBODY_FUNCTION_ST *pRal = (AUDIO_RALBODY_FUNCTION_ST *)pFunctions;

    AUDIO_RALBODY_FUNCTION_ST *pGotten =  (AUDIO_RALBODY_FUNCTION_ST *)InternalGetHandle();

    if (NULL == pGotten)
    {
    	return -1;
    }

    memcpy(pRal, pGotten, sizeof(AUDIO_RALBODY_FUNCTION_ST));

    return 0;
}

EXPORTED
unsigned int nexRALBody_Audio_Close_RALBody( void *pFunctions )
{
	if (NULL == pFunctions)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "pFunctions is NULL!\n");
		return -1;
	}

	InternalCloseHandle(&g_audioRendererHandle);
	return 0;
}

#ifdef __cplusplus
}
#endif
