/******************************************************************************
* File Name        : jni_MediaCodec.h
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

#ifndef _NEXCRAL_JNI_MEDIACODEC_H
#define _NEXCRAL_JNI_MEDIACODEC_H

#include <jni.h>

#include "external/NexMediaCodec.h"

#ifndef LOG_TAG
#define LOG_TAG "nexcral_mc"
#endif

namespace Nex_MC {
namespace JNI {

	const int MC_Feat_AdaptivePlayback = 1;
	namespace JCLASS {
		extern jclass MediaCodec;
		extern jclass BufferInfo;
		extern jclass CryptoInfo;
		extern jclass MediaCrypto;
		extern jclass UUID;
		extern jclass MediaCodecList;
		extern jclass MediaCodecInfo;
		extern jclass MediaCodecInfo_CodecCapabilities;
		extern jclass MediaCodecInfo_CodecProfileLevel;
		extern jclass MediaCodecInfo_VideoCapabilities;
	}; // namespace JCLASS

	namespace JMETHODS {
		typedef struct JavaMediaCodecMethods {
			jmethodID createDecoderByType;
			jmethodID createEncoderByType;
			jmethodID createByCodecName;
			jmethodID release;
			jmethodID configure;
			jmethodID start;
			jmethodID stop;
			jmethodID flush;
			jmethodID queueInputBuffer;
			jmethodID queueSecureInputBuffer;
			jmethodID dequeueInputBuffer;
			jmethodID dequeueOutputBuffer;
			jmethodID releaseOutputBuffer;
			jmethodID getInputFormat;
			jmethodID getOutputFormat;
			jmethodID getInputBuffers;
			jmethodID getOutputBuffers;
			jmethodID setVideoScalingMode;
			jmethodID getBuffers;
			jmethodID createInputSurface;
			jmethodID signalEndOfInputStream;
			jmethodID captureFrame; // only for samsung (full-seg projects)
			jmethodID getCodecInfo; // api 18
		} JavaMediaCodecMethods;
		extern JavaMediaCodecMethods MediaCodec;

		typedef struct JavaMediaCodecBufferInfoMethods {
			jmethodID constructor;
		} JavaMediaCodecBufferInfoMethods;
		extern JavaMediaCodecBufferInfoMethods BufferInfo;

		typedef struct JavaMediaCodecCryptoInfoMethods {
			jmethodID constructor;
			jmethodID set;
		} JavaMediaCodecCryptoInfoMethods;
		extern JavaMediaCodecCryptoInfoMethods CryptoInfo;

		typedef struct JavaMediaCryptoMethods {
			jmethodID constructor;
			jmethodID isCryptoSchemeSupported;
		} JavaMediaCryptoMethods;
		extern JavaMediaCryptoMethods MediaCrypto;

		typedef struct JavaUUIDMethods {
			jmethodID constructor;
		} JavaUUIDMethods;
		extern JavaUUIDMethods UUID;

		typedef struct JavaMediaCodecListMethods {
			jmethodID getCodecCount;
			jmethodID getCodecInfoAt;
		} JavaMediaCodecListMethods;
		extern JavaMediaCodecListMethods MediaCodecList;

		typedef struct JavaMediaCodecInfoMethods {
			jmethodID getName;
			jmethodID isEncoder;
			jmethodID getSupportedTypes;
			jmethodID getCapabilitiesForType;
		} JavaMediaCodecInfoMethods;
		extern JavaMediaCodecInfoMethods MediaCodecInfo;

		typedef struct JavaCodecCapabilitiesMethods {
			jmethodID isFeatureSupported;
			jmethodID getVideoCapabilities; // api 23
		} JavaCodecCapabilitiesMethods;
		extern JavaCodecCapabilitiesMethods CodecCapabilities;		

		typedef struct JavaVideoCapabilitiesMethods {
			jmethodID getWidthAlignment;
			jmethodID getHeightAlignment;
		} JavaVideoCapabilitiesMethods;
		extern JavaVideoCapabilitiesMethods VideoCapabilities;
	}; // namespace JMETHODS

	namespace JFIELDS {
		typedef struct JavaMediaCodecBufferInfoFields {
			jfieldID offset;
			jfieldID size;
			jfieldID presentationTimeUs;
			jfieldID flags;
		} JavaMediaCodecBufferInfoFields;
		extern JavaMediaCodecBufferInfoFields BufferInfo;

		typedef struct JavaMediaCodecCryptoInfoFields {
			jfieldID iv;
			jfieldID key;
			jfieldID mode;
			jfieldID numBytesOfClearData;
			jfieldID numBytesOfEncryptedData;
			jfieldID numSubSamples;
		} JavaMediaCodecCryptoInfoFields;
		extern JavaMediaCodecCryptoInfoFields CryptoInfo;

		typedef struct JavaMediaCodecInfo_CodecCapabilitiesFields {
			jfieldID colorFormats;
			jfieldID profileLevels;
		} JavaMediaCodecInfo_CodecCapabilitiesFields;
		extern JavaMediaCodecInfo_CodecCapabilitiesFields MediaCodecInfo_CodecCapabilities;

		typedef struct JavaMediaCodecInfo_CodecProfileLevelFields {
			jfieldID level;
			jfieldID profile;
		} JavaMediaCodecInfo_CodecProfileLevelFields;
		extern JavaMediaCodecInfo_CodecProfileLevelFields MediaCodecInfo_CodecProfileLevel;
	}; // namespace JFIELDS
}; // namespace JNI

class NexMediaCodec_using_jni : public NexMediaCodec {
private:
	NexMediaCodec_using_jni();
	virtual ~NexMediaCodec_using_jni();
public:
	static int createDecoderByType(const char *mimeType, NexMediaCodec * &result);
	static int createEncoderByType(const char *mimeType, NexMediaCodec * &result);
	static int createByCodecName(const char *codecName, NexMediaCodec * &result);

	static int releaseNexMediaCodec(NexMediaCodec *mediaCodec);

	virtual int configure(NexMediaFormat *format, NexSurfaceTexture *surface, jobject crypto, int flags);

	virtual int start();
	virtual int stop();

	virtual int release();

	virtual int flush();

	virtual int queueInputBuffer(size_t idx, size_t offset, size_t size, int64_t pts, int flags);
	virtual int queueSecureInputBuffer(size_t idx, size_t offset, jobject cryptoInfo, int64_t pts, int flags);

	virtual int dequeueInputBuffer(int64_t timeout, long &result);
	virtual int dequeueOutputBuffer(jobject bufferInfo, int64_t timeout, long &result);

	virtual int releaseOutputBuffer(size_t idx, bool render);

	virtual int getInputFormat(NexMediaFormat * &result);
	virtual int getOutputFormat(NexMediaFormat * &result);
	virtual int regetOutputFormat(NexMediaFormat * &result);

	virtual int getInputBuffers(void ** &result);
	virtual int getOutputBuffers(bool withoutPointers, void ** &result);
	virtual int getNumOutputBuffers();

	virtual int setVideoScalingMode(int mode);

	virtual int createInputSurface(jobject &result);
	virtual int signalEndOfInputStream();

	virtual int captureFrame(bool thumbnailMode);

	virtual int getName(const char * &result);

	static int createBufferInfo(jobject &result);
	static int releaseBufferInfo(jobject bufferInfo);

	static int createCryptoInfo(jobject &result);
	static int releaseCryptoInfo(jobject cryptoInfo);
	static int setCryptoInfo(jobject cryptoInfo, int newNumSubSamples, int *newNumBytesOfClearData, int *newNumBytesOfEncryptedData, uint8_t *newKey, uint8_t *newIV, int newMode);

	static int createMediaCrypto(int64_t msb, int64_t lsb, uint8_t *initData, int initDataLength, jobject &result);
	static int releaseMediaCrypto(jobject mediaCrypto);

	static int getSupportedInputColorFormats(const char *type, int *&supportedList);

	static int findPreferredCodec(const char *type, bool encoder, CodecClassRequest wantedCodecClass, char *&preferredCodecName, int **profileLevels = NULL, int *supportedFeature = NULL, int *piWidthAlignment = NULL, int *piHeightAlignment = NULL);
private:
	jobject jobjMediaCodec;
	void ** inputBuffers;
	void ** outputBuffers;
	NexMediaFormat *inputFormat;
	NexMediaFormat *outputFormat;
	int numOutputBuffers;
	char *name;

private:
	NexMediaCodec_using_jni(const NexMediaCodec_using_jni &);
	NexMediaCodec_using_jni &operator=(const NexMediaCodec_using_jni &);
}; // class NexMediaCodec_using_jni

}; // namespace Nex_MC

#endif //#ifndef _NEXCRAL_JNI_MEDIACODEC_H

