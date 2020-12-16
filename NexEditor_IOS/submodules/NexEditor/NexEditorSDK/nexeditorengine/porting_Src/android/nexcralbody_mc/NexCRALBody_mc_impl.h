/******************************************************************************
* File Name        : NexCRALBody_mc_impl.h
* Description      : Internal definitions for NexCRALBody_mc
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

#ifndef _NEXCAL_MC_IMPL_H_
#define _NEXCAL_MC_IMPL_H_

#include <pthread.h>
#include <semaphore.h>

#include <jni.h>

#include "utils/utils.h"

#if (0 < _NEX_INPUT_DUMP_) || (0 < _NEX_OUTPUT_DUMP_)
#include <stdio.h>
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#define NEX_MC_TYPE_DECODER 0
#define NEX_MC_TYPE_ENCODER 1
#define NEX_MC_TYPE_RENDERER 2
#if defined(NEX_TEST)
#define NEX_MC_TYPE_TEST 3
#endif

#define NEX_MC_AVTYPE_VIDEO 0
#define NEX_MC_AVTYPE_AUDIO 1

#define NEX_DEFAULT_INPUT_TIMEOUT 300000 // 300ms
#define NEX_DEFAULT_OUTPUT_TIMEOUT 300000 // 300ms

#define NEXCAL_PROPERTY_VIDEO_FPS						(NEXCAL_PROPERTY_USER_START + 0x100)
#define NEXCAL_PROPERTY_VIDEO_ENCODER_OPERATING_RATE	(NEXCAL_PROPERTY_USER_START + 0x101)
#define NEXCAL_PROPERTY_VIDEO_DECODER_MODE				(NEXCAL_PROPERTY_USER_START + 0x102)
#define NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2			(NEXCAL_PROPERTY_USER_START + 0x103)

namespace Nex_MC {

class NexMediaCodec;
class NexMediaFormat;
class NexSurfaceTexture;

namespace Common {
	typedef struct NexCAL_mc NexCAL_mc;

	typedef struct MediaCryptoInfo
	{
		int64_t mostSignificantBits; /* uuid's 64 most significant bits */
		int64_t leastSignificantBits; /* uuid's 64 least significant bits */
		uint8_t *initData; /* opaque initialization data specific to the crypto scheme */
		int initDataLength; /* length of the opaque initialization data */
	} MediaCryptoInfo;

	typedef struct MediaCodecCryptoInfo
	{
		unsigned char* pData; /* identical to pData for unencrypted content */
		int iLen; /* identical to iLen for unencrypted content */
		uint8_t *iv; /* 16-byte initialization vector */
		uint8_t *key; /* 16-byte opaque key */
		int mode; /* type of encryption: CRYPTO_MODE_AES_CTR=1; CRYPTO_MODE_UNENCRYPTED=0 */
		int *numBytesOfClearData; /* number of leading unencrypted bytes in each subSample */
		int *numBytesOfEncryptedData; /* number of trailing encrypted bytes in each subSample */
		int numSubSamples; /* number of subSamples that make up the buffer's contents */
	} MediaCodecCryptoInfo;
}; // namespace Common

namespace Video {
namespace Renderer {
	typedef void (*ralCallback_t)(int msg, int ext1, int ext2, int ext3, void *ext4, void *userData);

	typedef struct NexRAL_mcvr
	{
		void* hPlayer;
		Common::NexCAL_mc *mc;
		NexSurfaceTexture *surfaceTexture;
		ralCallback_t ralCallbackFunction;
		void * ralCallbackUserData;
		/*GLuint*/unsigned int texName;
		/*GLfloat*/float mvp[16];
		bool hasNoFrame;
		bool displayingLogo;
		bool surfaceTextureDetached;

		bool on;

		unsigned int surfaceWidth;
		unsigned int surfaceHeight;

		bool drawingBlank;
		bool drawingBlank2;
		bool drawingBlank3;
#if defined(_SHOW_LOGO_)
		bool drawingLogo;
		bool drawingLogo2;
		bool drawingLogo3;
#endif
		pthread_mutex_t glDrawStatusLock;
		pthread_cond_t glDrawStatusCondition;
		bool glDrawWaiting;

		/*EGLContext*/ void * eglcontext;

		/*GLuint*/ unsigned int program;

		/*GLuint*/ unsigned int u_mvp_matrix;
		/*GLuint*/ unsigned int u_tex_matrix;
		/*GLuint*/ unsigned int u_textureSampler;
		/*GLuint*/ unsigned int u_alpha;
		/*GLuint*/ unsigned int a_position;
		/*GLuint*/ unsigned int a_texCoord;

#if defined(_SHOW_LOGO_)
		/*GLuint*/ unsigned int logoProgram;
		/*GLuint*/ unsigned int logou_mvp_matrix;
		/*GLuint*/ unsigned int logou_tex_matrix;
		/*GLuint*/ unsigned int logou_textureSampler;
		/*GLuint*/ unsigned int logou_alpha;
		/*GLuint*/ unsigned int logoa_position;
		/*GLuint*/ unsigned int logoa_texCoord;
		/*GLuint*/ unsigned int logoTexName;
#endif
	} NexRAL_mcvr;
}; // namespace Renderer

namespace Decoder {
	typedef void (*OutputBufferReleaser)(Common::NexCAL_mc *mc, int outIndex, bool render);
	typedef struct NexCAL_mcvd
	{
		int width;
		int height;
		int pitch;
		void *codecSpecific;
		Renderer::NexRAL_mcvr *mcvr;
		jobject crypto;
		jobject cryptoInfo;
		bool created_mcvr;
		OutputBufferReleaser releaseOutputBuffer;
		int maxInputSize;
		int fps;
		int useSWCodec;
		bool bInitFailed;
		int colorformat;
	} NexCAL_mcvd;
}; // namespace Decoder

namespace Encoder {
	//typedef int (*OutputWriter)(Common::NexCAL_mc *mc, unsigned char **ppDest);
	typedef struct NexCAL_mcve
	{
		int quality;
		int width;
		int height;
		int fps;
		bool cbr;
		int bitrate;
		int profile;
		int level;
		int colorFormat;
		//OutputWriter writeOutputFrame;
		void *codecSpecific;
		NexSurfaceTexture *surface;
	} NexCAL_mcve;
}; // namespace Encoder
}; // namespace Video

namespace Audio {
namespace Decoder {
	typedef struct NexCAL_mcad
	{
		int sampleRate;
		int channelCount;
		int bitsPerSample;
		int numSamplesPerChannel;
		void *codecSpecific;
		jobject crypto;
		jobject cryptoInfo;

		int64_t maxPTS;
	} NexCAL_mcad;
}; // namespace Decoder

namespace Encoder {
	typedef int (*OutputWriter)(Common::NexCAL_mc *mc, unsigned char **ppDest);
	typedef struct NexCAL_mcae
	{
		int sampleRate;
		int channelCount;
		int bitRate;
		OutputWriter writeOutputFrame;
		void *codecSpecific;
		int dummyCount;
	} NexCAL_mcae;
}; // namespace Encoder
}; // namespace Audio

namespace Common {
	typedef bool (*OutputFormatChangedChecker)(Common::NexCAL_mc *mc);

	struct NexCAL_mc
	{
		int type; // NEX_MC_TYPE_DECODER or NEX_MC_TYPE_ENCODER
		int avtype; // NEX_MC_AVTYPE_VIDEO or NEX_MC_AVTYPE_AUDIO
		const char *codecName;

		const char *preferredCodecName;

		union
		{
			Nex_MC::Video::Decoder::NexCAL_mcvd vd;
			Nex_MC::Video::Encoder::NexCAL_mcve ve;
			Nex_MC::Audio::Decoder::NexCAL_mcad ad;
			Nex_MC::Audio::Encoder::NexCAL_mcae ae;
		};

		const char *mimeType;

		NexMediaCodec *mc;
		NexMediaFormat *inputFormat;

		OutputFormatChangedChecker outputFormatChanged;

		struct
		{ // related to threading
			pthread_t inThreadId;
			pthread_t outThreadId;

			pthread_mutex_t engLock;
			pthread_mutex_t inLock;
			pthread_mutex_t outLock;

			struct
			{ // acquire engLock for these
				volatile sig_atomic_t inThreadRunning;
				volatile sig_atomic_t outThreadRunning;

				pthread_cond_t engCond;

				long inIndex;
				bool inWaiting;
				bool inAtExternal;
				int64_t inTimeout;
				int64_t defaultInTimeout;

				long outIndex;
				bool outWaiting;
				bool outAtExternal;
				int64_t outTimeout;
				int64_t defaultOutTimeout;

				bool eosSent;
				bool eosReceived;

				bool engWaiting;

				bool shuttingDown;

				void ** inputBuffers;
				void ** outputBuffers;

				int lastOutputOffset;
				int lastOutputSize;
				long long lastOutputPTS;
				int lastOutputFlags;
			};

			pthread_cond_t inCond; // inLock
			pthread_cond_t outCond; // outLock

			jobject bufferInfo;
		};

		unsigned char *lastOutputBytes;
		int lastOutputBytesSize;

		bool sentValidInputFrame;

		bool exceptionState;

#if 0 < _NEX_INPUT_DUMP_
		FILE *inFile;
#endif
#if 0 < _NEX_OUTPUT_DUMP_
		FILE *outFile;
#endif
	};
}; // namespace Common

namespace Video {
namespace Decoder {
	extern Nex_MC::Common::NexCAL_mc * CreateNexCAL_mc(const char *codecName, const char *mimeType, NXINT32 width, NXINT32 height, NXUINT32 uUserDataType, NXVOID **ppUserData);
	extern NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData );
	extern NXINT32 SetProperty( NXUINT32 uProperty, NXINT64 uValue, NXVOID *pUserData );
	extern NXINT32 Init( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Deinit( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Decode
		( NXUINT8* pData
		, NXINT32 iLen
		, NXUINT32 uDTS
		, NXUINT32 uPTS
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData );
	extern NXINT32 DecodePR
		( NXUINT8* pData
		, NXINT32 iLen
		, NXUINT32 uDTS
		, NXUINT32 uPTS
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData );
	extern NXINT32 GetOutput
		( NXUINT8** ppBits1
		, NXUINT8** ppBits2
		, NXUINT8** ppBits3
		, NXUINT32 *pDTS
		, NXVOID *pUserData );
	extern NXINT32 Reset( NXVOID *pUserData );
	extern NXINT32 GetInfo( NXUINT32 uIndex, NXUINT32* puResult, NXVOID *pUserData );
}; // namespace Decoder

namespace Encoder {
	extern NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData );
	extern NXINT32 SetProperty( NXUINT32 uProperty, NXINT64 uValue, NXVOID *pUserData );
	extern NXINT32 Init( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Deinit( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Encode
		( NXUINT8* pData1
		, NXUINT8* pData2
		, NXUINT8* pData3
	//	, unsigned char** ppOutData
	//	, unsigned int* piOutLen
		, NXUINT32 uPTS
		, NXUINT32* puEncodeResult
		, NXVOID *pUserData );
	extern NXINT32 EncodeUsingFrameData
		( NXUINT8* pData1
		, NXUINT8* pData2
		, NXUINT8* pData3
	//	, NXUINT8** ppOutData
	//	, unsigned int* piOutLen
		, NXUINT32 uPTS
		, NXUINT32* puEncodeResult
		, NXVOID *pUserData );
	extern NXINT32 GetOutput
		( NXUINT8** ppOutData
		, NXINT32* piOutLen
		, NXUINT32* puPTS
		, NXVOID *pUserData );
	extern NXINT32 Skip
		( NXUINT8** ppOutData
		, NXUINT32* piOutLen
		, NXVOID *pUserData );
	extern NXINT32 Reset( NXVOID *pUserData );
}; // namespace Decoder
}; // namespace Video

namespace Audio {
namespace Decoder {
	extern Nex_MC::Common::NexCAL_mc * CreateNexCAL_mc(const char *codecName, const char *mimeType, NXINT32 sampleRate, NXINT32 channelCount, NXINT32 numSamplesPerChannel, NXUINT32 uUserDataType, NXVOID **ppUserData);
	extern NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData );
	extern NXINT32 SetProperty( NXUINT32 uProperty, NXINT64 uValue, NXVOID *pUserData );
	extern NXINT32 Init( Nex_MC::Common::NexCAL_mc *mc, NXUINT8 *pFrame, NXINT32 iFrameLen );
	extern NXINT32 Deinit( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Decode
		( NXUINT8* pSource
		, NXINT32 iLen
		, NXVOID* pDest
		, NXINT32* piWrittenPCMSize
		, NXUINT32 uDTS
		, NXUINT32 *puOutputTime
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData );
	extern NXINT32 DecodePR
		( NXUINT8* pSource
		, NXINT32 iLen
		, NXVOID* pDest
		, NXINT32* piWrittenPCMSize
		, NXUINT32 uDTS
		, NXUINT32 *puOutputTime
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData );
	extern NXINT32 Reset( NXVOID *pUserData );
}; // namespace Decoder

namespace Encoder {
	extern NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData );
	extern NXINT32 SetProperty( NXUINT32 uProperty, NXINT64 uValue, NXVOID *pUserData );
	extern NXINT32 Init( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Deinit( Nex_MC::Common::NexCAL_mc *mc );
	extern NXINT32 Encode
		( NXUINT8* pData
		, NXINT32 iLen
		, NXUINT8** ppOutData
		, NXUINT32* piOutLen
		, NXUINT32* puEncodeResult
		, NXVOID *pUserData );
	extern NXINT32 Reset( NXVOID *pUserData );

	extern NXINT32 WriteOutputFrame(Nex_MC::Common::NexCAL_mc *mc, NXUINT8 **ppDest);
}; // namespace Encoder
}; // namespace Audio


}; // namespace Nex_MC

#endif	//#ifndef _NEXCAL_MC_IMPL_H_
