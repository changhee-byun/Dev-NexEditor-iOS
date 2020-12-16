#ifndef JNI_AUDIO_H_
#define JNI_AUDIO_H_

#include <jni.h>

#include "external/Audio.h"

#ifndef NULL
#define NULL (0)
#endif

namespace Nex_AR {
namespace JNI {
	namespace JCLASS {
		extern jclass AudioManager;
		extern jclass AudioFormat;
		extern jclass AudioTrack;
		extern jclass AudioTimestamp;
	} // namespace JCLASS

	namespace JMETHODS {
		typedef struct JavaAudioManagerMethods {
			jmethodID getProperty /* api 17 */ ;
			jmethodID getOutputLatency /* api 18 Hidden property */ ; //yoon
		} JavaAudioManagerMethods;
		extern JavaAudioManagerMethods AudioManager;

		typedef struct JavaAudioTrackMethods {
			jmethodID constructor /* api 3 */ ;
			jmethodID constructor2 /* api 9 */ ;			
			jmethodID flush /* api 3 */ ;
			jmethodID getAudioSessionId /* api 9 */ ;
			jmethodID getChannelCount /* api 3 */ ;
			jmethodID getMinBufferSize /* api 3 */ ;
			jmethodID getNativeFrameCount /* api 3 */ ;
			jmethodID getNativeOutputSampleRate /* api 3 */ ;
			jmethodID getPlaybackHeadPosition /* api 3 */ ;
			jmethodID getPlayState /* api 3 */;
			jmethodID getState /* api 3 */ ;
			jmethodID pause /* api 3 */ ;
			jmethodID play /* api 3 */ ;
			jmethodID release /* api 3 */ ;
			jmethodID setNotificationMarkerPosition /* api 3 */;
			jmethodID setStereoVolume /* api 3 */ ;
			jmethodID stop /* api 3 */ ;
			jmethodID write_s /* api 3 */ ;
			jmethodID write_b /* api 3 */ ;
			jmethodID getTimestamp /* api 19 */ ;
		} JavaAudioTrackMethods;
		extern JavaAudioTrackMethods AudioTrack;

		typedef struct JavaAudioTimestampMethods {
			jmethodID constructor /* api 19 */ ;
		} JavaAudioTimestampMethods;
		extern JavaAudioTimestampMethods AudioTimestamp;
	} // namespace JMETHODS

	namespace JFIELDS {
		typedef struct JavaAudioManagerFields {
			jfieldID PROPERTY_OUTPUT_FRAMES_PER_BUFFER /* api 17 */ ;
			jfieldID PROPERTY_OUTPUT_SAMPLE_RATE /* api 17 */ ;
		} JavaAudioManagerFields;
		extern JavaAudioManagerFields AudioManager;

		typedef struct JavaAudioFormatFields {
			jfieldID CHANNEL_OUT_5POINT1 /* api 5 */ ;
			jfieldID CHANNEL_OUT_7POINT1 /* api 5 */ ;
			jfieldID CHANNEL_OUT_BACK_CENTER /* api 5 */ ;
			jfieldID CHANNEL_OUT_BACK_LEFT /* api 5 */ ;
			jfieldID CHANNEL_OUT_BACK_RIGHT /* api 5 */ ;
			jfieldID CHANNEL_OUT_DEFAULT /* api 5 */ ;
			jfieldID CHANNEL_OUT_FRONT_CENTER /* api 5 */ ;
			jfieldID CHANNEL_OUT_FRONT_LEFT /* api 5 */ ;
			jfieldID CHANNEL_OUT_FRONT_LEFT_OF_CENTER /* api 5 */ ;
			jfieldID CHANNEL_OUT_FRONT_RIGHT /* api 5 */ ;
			jfieldID CHANNEL_OUT_FRONT_RIGHT_OF_CENTER /* api 5 */ ;
			jfieldID CHANNEL_OUT_LOW_FREQUENCY /* api 5 */ ;
			jfieldID CHANNEL_OUT_MONO /* api 5 */ ;
			jfieldID CHANNEL_OUT_QUAD /* api 5 */ ;
			jfieldID CHANNEL_OUT_STEREO /* api 5 */ ;
			jfieldID ENCODING_DEFAULT /* api 3 */ ;
			jfieldID ENCODING_PCM_16BIT /* api 3 */ ;
			jfieldID ENCODING_PCM_8BIT /* api 3 */ ;
		} JavaAudioFormatFields;
		extern JavaAudioFormatFields AudioFormat;

		typedef struct JavaAudioTrackFields {
			jfieldID MODE_STREAM /* api 3 */ ;
			jfieldID STATE_INITIALIZED /* api 3 */ ;
			jfieldID mNativeTrackInJavaObj /* api 3 */ ;
			jfieldID mNativeTrackInJavaObj_long /* api ?? */ ;
		} JavaAudioTrackFields;
		extern JavaAudioTrackFields AudioTrack;

		typedef struct JavaAudioTimestampFields {
			jfieldID framePosition /* api 19 */ ;
			jfieldID nanoTime /* api 19 */ ;
		} JavaAudioTimestampFields;
		extern JavaAudioTimestampFields AudioTimestamp;
	} // namespace JFIELDS
} // namespace JNI

class NexAudio_using_jni : public NexAudio {
private:
	NexAudio_using_jni();
	virtual ~NexAudio_using_jni();
public:
	static NexAudio * create(jobject audioManager = NULL);
	static void destroy(NexAudio *);
	virtual int getOutputSampleRate();
	virtual int getOutputFramesPerBuffer();
	virtual int getOutputLatency(int streamType); //yoon
private:
	jobject jobjAudioManager;

private:
	NexAudio_using_jni(const NexAudio_using_jni &);
	NexAudio_using_jni &operator=(const NexAudio_using_jni &);
}; // class NexAudio_using_jni

class NexAudioTrack_using_jni : public NexAudioTrack {
private:
	NexAudioTrack_using_jni();
	virtual ~NexAudioTrack_using_jni();
public:
	// TODO: new create option
	static NexAudioTrack * create(int streamType, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode);
	static NexAudioTrack * create(int streamType, int sampleRateInHz, int channelConfig, int audioFormat, int bufferSizeInBytes, int mode, int sessionId);
	static void destroy(NexAudioTrack *);
	virtual void flush();
	virtual int getAudioSessionId();
	virtual int getChannelCount();
	static int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat);
	virtual int getNativeFrameCount();
	static int getNativeOutputSampleRate(int streamType);
	virtual unsigned int getPlaybackHeadPosition();
	virtual int getState();
	virtual int getPlayState();
	virtual void pause();
	virtual void play();
	virtual void release();
	virtual void stop();
	virtual int write(const short *audioData, int offsetInShorts, int sizeInShorts);
	virtual int write(const unsigned char *audioData, int offsetInBytes, int sizeInBytes);
	virtual int setStereoVolume(float leftVolume, float rightVolume);
	virtual void * getNativeAudioTrack();

	virtual bool getTimestamp(long long &framePosition, long long &nanoTime);

	virtual int setNotificationMarkerPosition(int markerInFrames);

	// static void OnLoad();
	// static void OnUnload();
private:
	jobject jobjAudioTrack;
	jobject jobjAudioTimestamp;

private:
	NexAudioTrack_using_jni(const NexAudio_using_jni &);
	NexAudioTrack_using_jni &operator=(const NexAudio_using_jni &);
}; // class NexAudioTrack_using_jni

} // namespace Nex_AR

#endif /* JNI_AUDIO_H_ */
