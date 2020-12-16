#ifndef AUDIO_H_
#define AUDIO_H_

namespace Nex_AR {
class NexAudio {
protected:
	NexAudio() {};
	~NexAudio() {};
public:
    virtual int getOutputLatency(int streamType) = 0; //yoon
	virtual int getOutputSampleRate() = 0;
	virtual int getOutputFramesPerBuffer() = 0;
}; // class NexAudio

class NexAudioTrack {
protected:
	NexAudioTrack() {};
	~NexAudioTrack() {};
public:
	virtual void flush() = 0;
//	static int getMinBufferSize(int sampleRateInHz, int channelConfig, int audioFormat) = 0;
	virtual int getAudioSessionId() = 0;
	virtual int getChannelCount() = 0;
	virtual unsigned int getPlaybackHeadPosition() = 0;
	virtual int getNativeFrameCount() = 0;
//	static int getNativeOutputSampleRate(int streamType) = 0;
	virtual int getState() = 0;
	virtual int getPlayState() = 0;
	virtual void pause() = 0;
	virtual void play() = 0;
	virtual void release() = 0;
	virtual void stop() = 0;
	virtual int write(const short *audioData, int offsetInShorts, int sizeInShorts) = 0;
	virtual int write(const unsigned char *audioData, int offsetInBytes, int sizeInBytes) = 0;
	virtual int setStereoVolume(float leftVolume, float rightVolume) = 0;
	virtual int setNotificationMarkerPosition(int markerInFrames) = 0;
	virtual void * getNativeAudioTrack() = 0;

	virtual bool getTimestamp(long long &framePosition, long long &expectedPTSTime) = 0;
}; // class NexAudioTrack
} // namespace Nex_AR

#endif /* AUDIO_H_ */
