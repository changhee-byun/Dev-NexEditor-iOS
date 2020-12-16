/******************************************************************************
* File Name        : NexCALBody_mc.cpp
* Description      : Generalized native codec for Android
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/


#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#define _NEX_DEFINE_64_
#endif


#define LOCATION_TAG "NexCALBody_mc"


#undef LOG_TAG
#define LOG_TAG "nexcral_mc"

#include <sys/types.h>
#include <string.h>

//#define NEXOTI_H264_SW_MC 0x200000C1
//#define eNEX_CODEC_V_H264_MC_S			 	0x50010300
//#define eNEX_CODEC_V_H264_MC_ENCODER_USING_FRAMEDATA 0x50010301

#include "mysystem/window.h"

#include "./NexCALBody_mc.h"
#include "./NexCRALBody_mc_impl.h"

using Nex_MC::Common::NexCAL_mc;

#include "version.h"

#include "utils/utils.h"
using Nex_MC::Utils::Time::GetRealTime;
using Nex_MC::Utils::Time::GetTickCount;
#include "utils/jni.h"

#include "NexMediaDef.h" //#include "nexOTIs.h"

#include "external/jni/jni_MediaCodec.h"
#include "external/jni/jni_MediaFormat.h"

#include "external/recvd/recvd_SurfaceTexture.h"

#define DECLARE_VIDEO_DECODER(NAME)                                                                                    \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Decoder {                                                                                                    \
	extern NEXCALGetProperty GetPropertyF;                                                                             \
	extern NEXCALSetProperty SetPropertyF;                                                                             \
	extern NEXCALVideoInit InitF;                                                                                      \
	extern NEXCALVideoDeinit DeinitF;                                                                                  \
	extern NEXCALVideoDecode DecodeF;                                                                                  \
}; /* namespace Decoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

#define DECLARE_VIDEO_SW_DECODER(NAME)                                                                                 \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Decoder {                                                                                                    \
    extern NEXCALGetProperty GetPropertySWF;                                                                           \
    extern NEXCALSetProperty SetPropertyF;                                                                             \
    extern NEXCALVideoInit InitSWF;                                                                                    \
}; /* namespace Decoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

#define DECLARE_VIDEO_PLAYREADY_DECODER(NAME)                                                                          \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Decoder {                                                                                                    \
    extern NEXCALSetProperty SetPropertyF;                                                                             \
    extern NEXCALVideoInit InitPRF;                                                                                    \
    extern NEXCALVideoDecode DecodePRF;                                                                                \
}; /* namespace Decoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

DECLARE_VIDEO_DECODER(H263);
DECLARE_VIDEO_DECODER(H264);
DECLARE_VIDEO_SW_DECODER(H264);
DECLARE_VIDEO_DECODER(HEVC);

namespace Nex_MC {
namespace H264 {
namespace Decoder {
    extern int QueryForSupport(uint8_t *dsi, size_t dsiLen);
    extern int QueryForSupportSW(uint8_t *dsi, size_t dsiLen);
}
}
}

namespace Nex_MC {
namespace HEVC {
namespace Decoder {
    extern int QueryForSupport(uint8_t *dsi, size_t dsiLen);
}
}
}

DECLARE_VIDEO_PLAYREADY_DECODER(H264);
DECLARE_VIDEO_DECODER(Mpeg4V);
DECLARE_VIDEO_DECODER(Mpeg2);
DECLARE_VIDEO_DECODER(VPX);

#define DECLARE_VIDEO_ENCODER(NAME)                                                                                    \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Encoder {                                                                                                    \
	extern NEXCALGetProperty GetPropertyF;                                                                             \
	extern NEXCALSetProperty SetPropertyF;                                                                             \
	extern NEXCALVideoEncoderInit InitF;                                                                               \
	extern NEXCALVideoEncoderInit InitUsingFrameDataF;                                                                 \
	extern NEXCALVideoEncoderDeinit DeinitF;                                                                           \
	extern NEXCALVideoEncoderEncode EncodeF;                                                                           \
	extern NEXCALVideoEncoderEncode EncodeUsingFrameDataF;                                                             \
}; /* namespace Encoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

DECLARE_VIDEO_ENCODER(H264);
DECLARE_VIDEO_ENCODER(HEVC);
DECLARE_VIDEO_ENCODER(Mpeg4V);

namespace Nex_MC {
namespace H264 {
namespace Encoder {
    extern size_t setCropToAchieveResolution(uint8_t *dsi_in_out, uint32_t maxLength, size_t reqWidth, size_t reqHeight);
}
}
}

namespace Nex_MC {
namespace HEVC {
namespace Encoder {
    extern size_t setCropToAchieveResolution(uint8_t *dsi_in_out, uint32_t maxLength, size_t reqWidth, size_t reqHeight);
}
}
}

#define DECLARE_AUDIO_DECODER(NAME)                                                                                    \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Decoder {                                                                                                    \
	extern NEXCALGetProperty GetPropertyF;                                                                             \
	extern NEXCALSetProperty SetPropertyF;                                                                             \
	extern NEXCALAudioInit InitF;                                                                                      \
	extern NEXCALAudioDeinit DeinitF;                                                                                  \
	extern NEXCALAudioDecode DecodeF;                                                                                  \
}; /* namespace Decoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

#define DECLARE_AUDIO_PLAYREADY_DECODER(NAME)                                                                          \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Decoder {                                                                                                    \
	extern NEXCALAudioInit InitPRF;                                                                                    \
	extern NEXCALAudioDecode DecodePRF;                                                                                \
}; /* namespace Decoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

DECLARE_AUDIO_DECODER(AAC);
DECLARE_AUDIO_PLAYREADY_DECODER(AAC);
DECLARE_AUDIO_DECODER(MP3);
DECLARE_AUDIO_PLAYREADY_DECODER(MP3);
DECLARE_AUDIO_DECODER(AMRNB);
DECLARE_AUDIO_DECODER(AMRWB);
DECLARE_AUDIO_DECODER(OGG);

#define DECLARE_AUDIO_ENCODER(NAME)                                                                                    \
namespace Nex_MC {                                                                                                     \
namespace NAME {                                                                                                       \
namespace Encoder {                                                                                                    \
	extern NEXCALGetProperty GetPropertyF;                                                                             \
	extern NEXCALAudioEncoderInit InitF;                                                                               \
	extern NEXCALAudioEncoderDeinit DeinitF;                                                                           \
	extern NEXCALAudioEncoderEncode EncodeF;                                                                           \
}; /* namespace Encoder */                                                                                             \
}; /* namespace NAME */                                                                                                \
}

DECLARE_AUDIO_ENCODER(AAC);
DECLARE_AUDIO_ENCODER(MP3);
DECLARE_AUDIO_ENCODER(AMRNB);
DECLARE_AUDIO_ENCODER(AMRWB);

namespace Nex_MC {

namespace { // anon
	typedef enum BufferState {
		  BUFFER_REMOTE = 0
		, BUFFER_LOCAL
		, ENUM_PADDING = 0x7FFFFFFF
	} BufferState;
};

namespace Common {

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE (mc->avtype)

#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE (mc->type)

void* InputThread(void* pArg)
{
	NexCAL_mc *mc = (NexCAL_mc *)pArg;

	IDENTITY("++", mc->codecName, mc, "");

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	mc->inAtExternal = false;
	mc->inIndex = -1;
	mc->inWaiting = false;
	mc->inThreadRunning = TRUE;

	STATUS(Log::MUTEX, Log::DEBUG, "( ( (%sCond) ) )", "eng");
	pthread_cond_broadcast(&mc->engCond);
	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "inLock");
	pthread_mutex_lock(&mc->inLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "inLock");
	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);
	sched_yield();

	STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "engLock");

	long result = -1000;
	int locked = 0;    

	do
	{
		result = -1000;

		if (0 <= mc->inIndex)
		{
			if (mc->engWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "eng");
				pthread_cond_broadcast(&mc->engCond);

				mc->engWaiting = false;
			}

			mc->inWaiting = true;

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			locked = 0;

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, inLock)", "in");
			pthread_cond_wait(&mc->inCond, &mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, inLock)>>", "in");
		}
		else if (mc->engWaiting && !mc->eosSent)
		{
			mc->inAtExternal = true;

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			locked = 0;

			// the following timeout value needs to be tuned... smaller values lead to faster reset/deinit but more power consumption
			EXTERNAL_CALL(dequeueInputBuffer, Log::DEBUG, mc->mc->dequeueInputBuffer(mc->inTimeout, result));

			if (0 <= result)
			{
				STATUS(Log::FLOW, Log::VERBOSE, "got index(%ld)", result);
			}
			else if (-1 /* INFO_TRY_AGAIN_LATER */ == result)
			{
				STATUS(Log::FLOW, Log::VERBOSE, "timed out");
			}
			else
			{
				MC_ERR("unexpected result(%ld)", result);
			}
		}
		else
		{
			mc->inWaiting = true;

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			locked = 0;

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, inLock)", "in");
			pthread_cond_wait(&mc->inCond, &mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, inLock)>>", "in");
		}

		if( !mc->shuttingDown)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "engLock");
			pthread_mutex_lock(&mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "engLock");
			locked = 1;            
		}

		if (0 <= result)
		{
			mc->inIndex = result;
		}
		else if (-1 /* INFO_TRY_AGAIN_LATER */ == result)
		{
			if (mc->engWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "eng");
				pthread_cond_broadcast(&mc->engCond);

				mc->engWaiting = false;
			}
		}

		mc->inAtExternal = false;
		mc->inWaiting = false;
	} while (!mc->shuttingDown && !mc->exceptionState);

	STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
	if( locked)
	pthread_mutex_unlock(&mc->engLock);
	// I know this is redundant. This is on purpose to match locks and log levels
	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	mc->inWaiting = false;
	mc->inThreadRunning = FALSE;

	STATUS(Log::MUTEX, Log::DEBUG, "( ( (%sCond) ) )", "eng");
	pthread_cond_broadcast(&mc->engCond);
	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "inLock");
	pthread_mutex_unlock(&mc->inLock);

	IDENTITY("--", mc->codecName, mc, "");
	return 0;
}

namespace { // (anon8)
inline int DequeueOutput(NexCAL_mc *mc, int64_t timeout)
{
	long result = -1;

	// the following timeout value needs to be tuned... smaller values lead to faster reset/deinit but more power consumption
	EXTERNAL_CALL(dequeueOutputBuffer, Log::DEBUG, mc->mc->dequeueOutputBuffer(mc->bufferInfo, timeout, result));

	if (0 <= result)
	{
		STATUS(Log::FLOW, Log::VERBOSE, "got index(%ld)", result);

		Nex_MC::Utils::JNI::JNIEnvWrapper env;

		if (env != NULL)
		{
			mc->lastOutputOffset = env->GetIntField(mc->bufferInfo, JNI::JFIELDS::BufferInfo.offset);
			mc->lastOutputSize = env->GetIntField(mc->bufferInfo, JNI::JFIELDS::BufferInfo.size);
			mc->lastOutputPTS = env->GetLongField(mc->bufferInfo, JNI::JFIELDS::BufferInfo.presentationTimeUs);
			mc->lastOutputFlags = env->GetIntField(mc->bufferInfo, JNI::JFIELDS::BufferInfo.flags);
		}
	}
	else if (-1 /* INFO_TRY_AGAIN_LATER */ == result)
	{
		STATUS(Log::FLOW, Log::VERBOSE, "timed out");
	}
	else if (-2 /* INFO_OUTPUT_FORMAT_CHANGED */ == result)
	{
		STATUS(Log::FLOW, Log::VERBOSE, "INFO_OUTPUT_FORMAT_CHANGED");

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		NexMediaFormat *format = NULL;

		EXTERNAL_CALL(getOutputFormat, Log::DEBUG, mc->mc->regetOutputFormat(format));

		if (mc->outputFormatChanged(mc))
		{
			STATUS(Log::FLOW, Log::VERBOSE, "output format change detected");
			mc->outIndex = -2;
		}
		else
		{
			STATUS(Log::FLOW, Log::VERBOSE, "no output format change detected");
			result = -1;
			mc->outIndex = -1;
		}

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);
	}
	else if (-3 /* INFO_OUTPUT_BUFFERS_CHANGED */ == result)
	{
		STATUS(Log::FLOW, Log::VERBOSE, "INFO_OUTPUT_BUFFERS_CHANGED");

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		result = -1;
		mc->outIndex = -1;

		bool doesntNeedBufferPointers = (NEX_MC_TYPE_DECODER == mc->type) && (NEX_MC_AVTYPE_VIDEO == mc->avtype) && (NULL != mc->vd.mcvr);

		EXTERNAL_CALL(getOutputBuffers, Log::DEBUG, mc->mc->getOutputBuffers(doesntNeedBufferPointers, mc->outputBuffers));

		if (doesntNeedBufferPointers)
		{
			for (int i = 0; i < mc->mc->getNumOutputBuffers(); ++i)
			{
				mc->outputBuffers[i] = (void *)BUFFER_REMOTE;
			}
		}

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);
	}
	else
	{
		MC_ERR("unexpected result(%ld)", result);
	}

	return result;
}
}; // namespace (anon8)

//Nex_MC::Common::OutputThread
void* OutputThread(void* pArg)
{
	NexCAL_mc *mc = (NexCAL_mc *)pArg;

	IDENTITY("++", mc->codecName, mc, "");

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	mc->outAtExternal = false;
	mc->outIndex = -1;
	mc->outWaiting = false;
	mc->outThreadRunning = TRUE;

	STATUS(Log::MUTEX, Log::DEBUG, "( ( (%sCond) ) )", "eng");
	pthread_cond_broadcast(&mc->engCond);
	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "outLock");
	pthread_mutex_lock(&mc->outLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "outLock");
	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);
	sched_yield();

	STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "engLock");

	int result = -1000;
	int locked = 0;    

	do
	{
		result = -1000;

		if (-1 != mc->outIndex)
		{
			if (mc->engWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "eng");
				pthread_cond_broadcast(&mc->engCond);

				mc->engWaiting = false;
			}

			mc->outWaiting = true;

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			locked = 0;            

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, outLock)", "out");
			pthread_cond_wait(&mc->outCond, &mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, outLock)>>", "out");
		}
		else if (mc->engWaiting && !mc->eosReceived)
		{
			mc->outAtExternal = true;

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			locked = 0;            

			result = DequeueOutput(mc, mc->outTimeout);
		}
		else
		{
			mc->outWaiting = true;

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			locked = 0;            

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, outLock)", "out");
			pthread_cond_wait(&mc->outCond, &mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, outLock)>>", "out");
		}

		if( !mc->shuttingDown)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "engLock");
			pthread_mutex_lock(&mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "engLock");
			locked = 1;            
		}

		if (-1 /* INFO_TRY_AGAIN_LATER */ == result)
		{
			if (mc->engWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "eng");
				pthread_cond_broadcast(&mc->engCond);

				mc->engWaiting = false;
			}
		}
		else if (-1000 != result)
		{
			mc->outIndex = result;
		}

		mc->outAtExternal = false;
		mc->outWaiting = false;
	} while (!mc->shuttingDown && !mc->exceptionState);

	STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "engLock");
	if( locked)
	pthread_mutex_unlock(&mc->engLock);
	// I know this is redundant. This is on purpose to match locks and log levels
	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	mc->outWaiting = false;
	mc->outThreadRunning = FALSE;

	STATUS(Log::MUTEX, Log::DEBUG, "( ( (%sCond) ) )", "eng");
	pthread_cond_broadcast(&mc->engCond);
	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "outLock");
	pthread_mutex_unlock(&mc->outLock);

	IDENTITY("--", mc->codecName, mc, "");
	return 0;
}

typedef struct MCConfigParams
{
	NexSurfaceTexture *surface;
	jobject crypto;
	int flags;
} MCConfigParams;

//Nex_MC::Common::Init
int Init( NexCAL_mc *mc, bool needInputThread = true )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	mc->defaultInTimeout = NEX_DEFAULT_INPUT_TIMEOUT;

	if (Utils::ChipType::EDEN == Utils::GetChipType())
	{
		mc->defaultOutTimeout = 100000; // 100 ms
	}
	else
	{
		mc->defaultOutTimeout = NEX_DEFAULT_OUTPUT_TIMEOUT;
	}

	mc->inTimeout = mc->defaultInTimeout;
	mc->outTimeout = mc->defaultOutTimeout;

	mc->shuttingDown = false;

	STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "engLock");
	pthread_mutex_init(&mc->engLock, NULL);
	STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "inLock");
	pthread_mutex_init(&mc->inLock, NULL);
	STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "outLock");
	pthread_mutex_init(&mc->outLock, NULL);

	STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "engCond");
	pthread_cond_init(&mc->engCond, NULL);
	STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "inCond");
	pthread_cond_init(&mc->inCond, NULL);
	STATUS(Log::MUTEX, Log::DEBUG, "O(%s)", "outCond");
	pthread_cond_init(&mc->outCond, NULL);

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (needInputThread)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "O(in_t)");
		pthread_create(&mc->inThreadId, NULL, &Common::InputThread, mc);

		while (FALSE == mc->inThreadRunning && !mc->exceptionState)
		{
			STATUS(Log::MUTEX, Log::DEBUG, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%sCond, engLock)>>", "eng");
		}
	}
	else
	{
		mc->inWaiting = true;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "O(out_t)");
	pthread_create(&mc->outThreadId, NULL, &Common::OutputThread, mc);

	while (FALSE == mc->outThreadRunning && !mc->exceptionState)
	{
		STATUS(Log::MUTEX, Log::DEBUG, ">>(%sCond, engLock)", "eng");
		pthread_cond_wait(&mc->engCond, &mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%sCond, engLock)>>", "eng");
	}

	while ((!mc->inWaiting || !mc->outWaiting) && !mc->exceptionState)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);
		sched_yield();
		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", 0);
	return 0;
}

//Nex_MC::Common::Deinit
int Deinit( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	int retValue = 0;

	mc->shuttingDown = true;
    
	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (0 != mc->inThreadId)
	{
		while (TRUE == mc->inThreadRunning)
		{
			if (mc->inWaiting)
			{
				STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "inLock");
				pthread_mutex_lock(&mc->inLock);
				STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "inLock");

				STATUS(Log::MUTEX, Log::DEBUG, "( ( (%sCond) ) )", "in");
				pthread_cond_broadcast(&mc->inCond);

				STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "inLock");
				pthread_mutex_unlock(&mc->inLock);
			}

			STATUS(Log::MUTEX, Log::DEBUG, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%sCond, engLock)>>", "eng");
		}

		STATUS(Log::MUTEX, Log::DEBUG, ">-(in_t)");
		pthread_join(mc->inThreadId, NULL);
		STATUS(Log::MUTEX, Log::DEBUG, "(in_t)>-");
	}

	while (TRUE == mc->outThreadRunning)
	{
		if (mc->outWaiting)
		{
			STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "outLock");

			STATUS(Log::MUTEX, Log::DEBUG, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		STATUS(Log::MUTEX, Log::DEBUG, ">>(%sCond, engLock)", "eng");
		pthread_cond_wait(&mc->engCond, &mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%sCond, engLock)>>", "eng");
	}

	STATUS(Log::MUTEX, Log::DEBUG, ">-(out_t)");
	pthread_join(mc->outThreadId, NULL);
	STATUS(Log::MUTEX, Log::DEBUG, "(out_t)>-");

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "engCond");
	pthread_cond_destroy(&mc->engCond);
	STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "inCond");
	pthread_cond_destroy(&mc->inCond);
	STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "outCond");
	pthread_cond_destroy(&mc->outCond);

	STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "engLock");
	pthread_mutex_destroy(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "inLock");
	pthread_mutex_destroy(&mc->inLock);
	STATUS(Log::MUTEX, Log::DEBUG, "X(%s)", "outLock");
	pthread_mutex_destroy(&mc->outLock);

	if(mc->inIndex >= 0)
	{
		EXTERNAL_CALL_WITH_ACTION(queueInputBuffer, Log::DEBUG
			, mc->mc->queueInputBuffer(mc->inIndex, 0, 0, 0, 0)
			, retValue = -1);
		mc->inIndex = -1;	 		
 	}
			
	if(mc->outIndex >= 0)
	{
		EXTERNAL_CALL_WITH_ACTION(releaseOutputBuffer, Log::DEBUG
			, mc->mc->releaseOutputBuffer(mc->outIndex, false)
			, retValue = -1);
		mc->outIndex = -1;	 					
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

namespace Decoder {

//Nex_MC::Common::Decoder::Init
int Init( NexCAL_mc *mc, MCConfigParams *mccp )
{
	STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

	int retValue = Nex_MC::Common::Init(mc);

	if (NULL != mc->preferredCodecName)
	{

		STATUS(Log::INFO, Log::DEBUG, "preferred codec name specified [%s]", mc->preferredCodecName);
		EXTERNAL_CALL_WITH_ACTION(createByCodecName, Log::DEBUG
			, NexMediaCodec_using_jni::createByCodecName(mc->preferredCodecName, mc->mc)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -20)
			; return -20);
	}
	else
	{
		EXTERNAL_CALL_WITH_ACTION(createDecoderByType, Log::DEBUG
			, NexMediaCodec_using_jni::createDecoderByType(mc->mimeType, mc->mc)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -10)
			; return -10);
	}

	if (NULL == mc->mc)
	{
		MC_ERR("failed to create MediaCodec instance");
		STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -1);
		return -1;
	}

	EXTERNAL_CALL_WITH_ACTION(configure, Log::DEBUG
		, mc->mc->configure(mc->inputFormat, mccp->surface, mccp->crypto, mccp->flags)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -12)
		; return -12);

	EXTERNAL_CALL_WITH_ACTION(createBufferInfo, Log::DEBUG
		, NexMediaCodec_using_jni::createBufferInfo(mc->bufferInfo)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -11)
		; return -11);

	if (NULL == mc->bufferInfo)
	{
		MC_ERR("failed to create MediaCodec$BufferInfo instance");
		STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -2);
		return -2;
	}

	EXTERNAL_CALL_WITH_ACTION(start, Log::DEBUG
		, mc->mc->start()
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -13)
		; return -13);

	EXTERNAL_CALL_WITH_ACTION(getInputBuffers, Log::DEBUG
		, mc->mc->getInputBuffers(mc->inputBuffers)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -14)
		; return -14);

	bool doesntNeedBufferPointers = (NEX_MC_AVTYPE_VIDEO == mc->avtype) && (NULL != mc->vd.mcvr);

	EXTERNAL_CALL_WITH_ACTION(getOutputBuffers, Log::DEBUG
		, mc->mc->getOutputBuffers(doesntNeedBufferPointers, mc->outputBuffers)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -15)
		; return -15);

	if (doesntNeedBufferPointers)
	{
		for (int i = 0; i < mc->mc->getNumOutputBuffers(); ++i)
		{
			mc->outputBuffers[i] = (void *)BUFFER_REMOTE;
		}
	}

	STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", retValue);
	return retValue;
}

}; // namespace Decoder

namespace Encoder {

//Nex_MC::Common::Encoder::Init
int Init( NexCAL_mc *mc, MCConfigParams *mccp )
{
	STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

	int retValue = Nex_MC::Common::Init(mc, NEX_MC_AVTYPE_VIDEO != mc->avtype || 0x7f000789 /* COLOR_FormatSurface */ != mc->ve.colorFormat);

	if (NULL != mc->preferredCodecName)
	{
        STATUS(Log::INFO, Log::DEBUG, "preferred codec name specified [%s]", mc->preferredCodecName);
		EXTERNAL_CALL_WITH_ACTION(createByCodecName, Log::DEBUG
			, NexMediaCodec_using_jni::createByCodecName(mc->preferredCodecName, mc->mc)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -20)
			; return -20);
	}
	else
	{
		EXTERNAL_CALL_WITH_ACTION(createEncoderByType, Log::DEBUG
			, NexMediaCodec_using_jni::createEncoderByType(mc->mimeType, mc->mc)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -10)
			; return -10);
	}

	if (NULL == mc->mc)
	{
		MC_ERR("failed to create MediaCodec instance");
		STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -1);
		return -1;
	}

	EXTERNAL_CALL_WITH_ACTION(configure, Log::DEBUG
		, mc->mc->configure(mc->inputFormat, NULL, NULL, mccp->flags)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -12)
		; return -12);

	EXTERNAL_CALL_WITH_ACTION(createBufferInfo, Log::DEBUG
		, NexMediaCodec_using_jni::createBufferInfo(mc->bufferInfo)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -11)
		; return -11);

	if (NULL == mc->bufferInfo)
	{
		MC_ERR("failed to create MediaCodec$BufferInfo instance");
		STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -2);
		return -2;
	}

	if (NEX_MC_AVTYPE_VIDEO == mc->avtype && 0x7f000789 /* COLOR_FormatSurface */ == mc->ve.colorFormat && NULL != Nex_MC::JNI::JMETHODS::MediaCodec.createInputSurface)
	{
		jobject surface;

		EXTERNAL_CALL_WITH_ACTION(createInputSurface, Log::DEBUG
			, mc->mc->createInputSurface(surface)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -13)
			; return -13);

		if (NULL != surface)
		{
			STATUS(Log::FLOW, Log::DEBUG, "surface(%p)", surface);
			// exception cannot occur for the following function (as of yet)
			NexSurfaceTexture_using_recvd::createSurfaceTexture(surface, mccp->surface);
		}

		EXTERNAL_CALL_WITH_ACTION(start, Log::DEBUG
			, mc->mc->start()
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -14)
			; return -15);
	}
	else
	{
		EXTERNAL_CALL_WITH_ACTION(start, Log::DEBUG
			, mc->mc->start()
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -15)
			; return -15);

		EXTERNAL_CALL_WITH_ACTION(getInputBuffers, Log::DEBUG
			, mc->mc->getInputBuffers(mc->inputBuffers)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -16)
			; return -16);
	}

	EXTERNAL_CALL_WITH_ACTION(getOutputBuffers, Log::DEBUG
		, mc->mc->getOutputBuffers(false, mc->outputBuffers)
		, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -17)
		; return -17);

	STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", retValue);
	return retValue;
}

}; // namespace Encoder

}; // namespace Common

namespace Video {
#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

namespace Decoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

void SetDebugLevel( int types, int max_level )
{
	Log::SetDebugLevel(NEX_MC_TYPE, NEX_MC_AVTYPE, types, max_level);
}

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%d) ud(%p)", uProperty, pUserData);

	int retValue = 0;

	NXINT64 input = *puValue;

	switch ( uProperty )
	{
#if 0 // this is for kinemaster
	case NEXCAL_PROPERTY_AVC_GET_POSTDISPLAY_CALLBACK_FUNC_USERDATA:
		if(0 == pUserData)
		{
			OC_ERR("uUserData is zero");
		}

		if(NULL != NexOCDecoder::spCALUserData)
		{
			*puValue = (unsigned int)(size_t)(NexOCDecoder::spCALUserData);
		}
		else
		{
			*puValue = 0;
		}
		break;

	case NEXCAL_PROPERTY_AVC_GET_POSTDISPLAY_CALLBACK_FUNC:
		if(NULL == pUserData)
		{
			OC_ERR("uUserData is zero");
		}

		*puValue = (NXINT64)(size_t)(NexOCDecoder::HANDLERPostDisplayCallback);
		break;
#endif
	case NEXCAL_PROPERTY_CODEC_IMPLEMENT :
		*puValue = NEXCAL_PROPERTY_CODEC_IMPLEMENT_HW;
		break;

	case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
		*puValue = NEXCAL_PROPERTY_ANSWERIS_YES;
		break;

	case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
	//	if (0 == uUserData)
		if (false)
		{
			if (NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 == *puValue)
			{
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
			}
			else if (NEXCAL_PROPERTY_VIDEO_BUFFER_MERGED_YUV420 == *puValue)
			{
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_MERGED_YUV420;
			}
			else if (NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420 == *puValue)
			{
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420;
			}
			else if (NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420 == *puValue)
			{
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420;
			}
			else if (NEXCAL_PROPERTY_VIDEO_BUFFER_TILED == *puValue)
			{
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_TILED;
			}
			else
			{
				*puValue 	= NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420
					| NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420
					| NEXCAL_PROPERTY_VIDEO_BUFFER_MERGED_YUV420
					| NEXCAL_PROPERTY_VIDEO_BUFFER_NV12
					| NEXCAL_PROPERTY_VIDEO_BUFFER_TILED
					| NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
			}
		}

		if (true)//0 != uUserData)
		{ // TODO:
			*puValue 	= NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
		}
		break;
	case NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT:
		*puValue = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES;
		break;

	case NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_SUPPORTED:
		*puValue = NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_INTERNAL | NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_EXTERNAL;
		break;

	case NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR:
		//if (0 == uUserData)
		{
			*puValue = NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_INTERNAL;
 		}// TODO:
		break;

	case NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT:
		*puValue = NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT_NO;
		break;

	default:
		*puValue = 0;
		break;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}

NXINT32 SetProperty( NXUINT32 uProperty, NXINT64 uValue,  NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%d) value(%lld) ud(%p)", uProperty, uValue, pUserData);

	NXINT32 retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

namespace { // anon9
	bool outputFormatChangedChecker(NexCAL_mc *mc)
	{
		STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

		bool result = false;

		int ret = 0;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
			; return false);

		if (NULL != outputFormat)
		{
			int newWidth = 0;

			EXTERNAL_CALL_WITH_ACTION(getInteger(width), Log::DEBUG
				, outputFormat->getInteger("width", newWidth)
				, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
				; return false);

			if (newWidth != mc->vd.width)
			{
				result = true;
			}
			else
			{
				int newHeight;

				EXTERNAL_CALL_WITH_ACTION(getInteger(height), Log::DEBUG
					, outputFormat->getInteger("height", newHeight)
					, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
					; return false);

				if (newHeight != mc->vd.height)
				{
					result = true;
				}
			}

			int newColorFormat;
			EXTERNAL_CALL_WITH_ACTION(getInteger(color-format), Log::DEBUG
				, outputFormat->getInteger("color-format", newColorFormat)
				, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
				; return false);

			result = true;
		}

		STATUS(Log::FLOW, Log::DEBUG, "- result(%s)", result ? "true" : "false");
		return result;
	}

	void ReleaseOutputBuffer(NexCAL_mc *mc, int outIndex, bool render)
	{
		STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		if (NULL == mc->mc)
		{
			MC_WARN("media codec already released!");

			STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);

			STATUS(Log::FLOW, Log::DEBUG, "-");
			return;
		}

		if (0 <= outIndex && outIndex < mc->mc->getNumOutputBuffers())
		{
#ifdef _NEX_DEFINE_64_
			if (BUFFER_LOCAL == static_cast<BufferState>((int)(NXINT64)(mc->outputBuffers[outIndex])))
#else
			if (BUFFER_LOCAL == static_cast<BufferState>((int)(mc->outputBuffers[outIndex])))
#endif
			{
				EXTERNAL_CALL(releaseOutputBuffer, Log::DEBUG, mc->mc->releaseOutputBuffer(outIndex, render));

				mc->outputBuffers[outIndex] = (void *)BUFFER_REMOTE;
			}
			else
			{
				MC_WARN("wrong buffer state(0x%p)", mc->outputBuffers[outIndex]);
			}
		}
		else
		{
			MC_ERR("invalid outIndex(%d)", outIndex);
		}

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		STATUS(Log::FLOW, Log::DEBUG, "-");
	}
}; // namespace (anon9)

NexCAL_mc * CreateNexCAL_mc(const char *codecName, const char *mimeType, int width, int height, unsigned int uUserDataType, NXVOID **ppUserData)
{
	STATUS(Log::FLOW, Log::INFO, "+");

	NexMediaFormat *inputFormat = NULL;

	STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->createVideoFormat");
	int ret = NexMediaFormat_using_jni::createVideoFormat(mimeType, width, height, inputFormat);
	STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "createVideoFormat->");

	if (0 != ret)
	{
		MC_ERR("Exception occurred while creating video format (%d)", ret);
		return NULL;
	}

	if (NULL == inputFormat)
	{
		MC_ERR("failed to create MediaFormat instance");
		return NULL;
	}

	NexCAL_mc *mc = (NexCAL_mc *)malloc(sizeof(NexCAL_mc));
	if (NULL == mc)
	{
		MC_ERR("malloc failed!");
		NexMediaFormat_using_jni::releaseNexMediaFormat(inputFormat);
		return NULL;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(NexCAL_mc));

	memset(mc, 0x00, sizeof(NexCAL_mc));

	mc->type = NEX_MC_TYPE;
	mc->avtype = NEX_MC_AVTYPE;
	mc->codecName = codecName;

	mc->mimeType = mimeType;

	mc->inIndex = -1;
	mc->outIndex = -1;

	// let's trust mw
	mc->vd.width = width;
	mc->vd.height = height;
	mc->vd.pitch = width;

	mc->inputFormat = inputFormat;

	if ('NRMC' == uUserDataType)
	{
		Nex_MC::Video::Renderer::NexRAL_mcvr *mcvr = (Nex_MC::Video::Renderer::NexRAL_mcvr *)(*ppUserData);
		mcvr->mc = mc;
		mc->vd.mcvr = mcvr;
		mc->vd.releaseOutputBuffer = ReleaseOutputBuffer;
		STATUS(Log::FLOW, Log::DEBUG, "detected mcvr");
	}
    else if ('NSMC' == uUserDataType)
    {
        Nex_MC::Video::Renderer::NexRAL_mcvr *mcvr = (Nex_MC::Video::Renderer::NexRAL_mcvr *)(*ppUserData);
        mcvr->mc = mc;
        mc->vd.mcvr = mcvr;
        mc->vd.releaseOutputBuffer = ReleaseOutputBuffer;
        STATUS(Log::FLOW, Log::DEBUG, "detected surface mcvr");
    }
	else if ('NKMS' == uUserDataType)
	{
		mc->vd.created_mcvr = true;
		mc->vd.mcvr = (Nex_MC::Video::Renderer::NexRAL_mcvr *)malloc(sizeof(Nex_MC::Video::Renderer::NexRAL_mcvr));
		if (NULL == mc->vd.mcvr)
		{
			MC_ERR("malloc failed!");
			NexMediaFormat_using_jni::releaseNexMediaFormat(inputFormat);
			free(mc);
			return NULL;
		}
		STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(Nex_MC::Video::Renderer::NexRAL_mcvr));
		mc->vd.mcvr->mc = mc;

		// exception cannot occur for the following function (as of yet)
		NexSurfaceTexture_using_recvd::createSurfaceTexture((jobject)(*ppUserData), mc->vd.mcvr->surfaceTexture);
		mc->vd.releaseOutputBuffer = ReleaseOutputBuffer;
		STATUS(Log::FLOW, Log::DEBUG, "detected km");
	}
#if defined(NEX_ADD_OC_DEFS)
	else if ('ALuD' == uUserDataType)
	{
		Nex_MC::Video::Renderer::NexRAL_mcvr *mcvr = (Nex_MC::Video::Renderer::NexRAL_mcvr *)(*ppUserData);
		mcvr->mc = mc;
		mc->vd.mcvr = mcvr;
		mc->vd.releaseOutputBuffer = ReleaseOutputBuffer;
		STATUS(Log::FLOW, Log::DEBUG, "detected mcvr");
	}
#endif

	STATUS(Log::FLOW, Log::INFO, "- mc(%p)", mc);
	return mc;
}

int Init( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);
	int sdk_level = Nex_MC::Utils::GetBuildVersionSDK();

	if (NULL == mc->inputFormat)
	{
		EXTERNAL_CALL_WITH_ACTION(createVideoFormat, Log::DEBUG
			, NexMediaFormat_using_jni::createVideoFormat(mc->mimeType, mc->vd.width, mc->vd.height, mc->inputFormat)
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -10)
			; return -10);

		if (NULL == mc->inputFormat)
		{
			MC_ERR("failed to create MediaFormat instance");
			STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -1);
			return -1;
		}
	}

	// the following is causing too many problems because android uses assert to check whether this value works (i.e. crash if it doesn't)
	// we know that at least marvell requires a bigger value than the default (800kb seems to work)
#if 0
	//int maxInputSize = ROUNDUP(mc->vd.width, 16) * ROUNDUP(mc->vd.height, 16) * 3 / 4 + 30*1024; // similar to nvidia's formula
	int maxInputSize = ROUNDUP(mc->vd.width, 16) * ROUNDUP(mc->vd.height, 16) / 2 + 30*1024;
	maxInputSize = MIN(maxInputSize, ROUNDUP(1920, 16) * ROUNDUP(1080, 16) / 2);
#endif
    if (0 == mc->vd.maxInputSize)
    {
	    Utils::ChipType::ChipType chipType = Utils::GetChipType();

		mc->vd.maxInputSize = ROUNDUP(mc->vd.width, 16) * ROUNDUP(mc->vd.height, 16) * 3 / 4 + 30*1024; // similar to nvidia's formula
        if (Utils::ChipType::MARVELL == chipType)
        {
            mc->vd.maxInputSize = 819200;
        }
    }

    if (0 != mc->vd.maxInputSize)
    {
        mc->vd.maxInputSize = ROUNDUP(mc->vd.maxInputSize, 4096);
        EXTERNAL_CALL_WITH_ACTION(setInteger(max-input-size), Log::DEBUG
            , mc->inputFormat->setInteger("max-input-size", mc->vd.maxInputSize)
            , STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -11)
            ; return -11);
    }

#if 0
    if (0 != mc->vd.fps)
    {
        EXTERNAL_CALL_WITH_ACTION(setInteger(frame-rate), Log::DEBUG
            , mc->inputFormat->setInteger("frame-rate" /* KEY_FRAME_RATE */, mc->vd.fps)
            , STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -11)
            ; return -11);
    }
#endif

	if (Utils::ChipType::EXYNOS4 == Utils::GetChipType()
		|| Utils::ChipType::EXYNOS == Utils::GetChipType())
	{   
		// the following is to prevent queueInputBuffer error during Adaptive playback on Nexus 10
		if(mc->vd.width < 640 || mc->vd.height < 480)
		{ 
			int max_width = MAX(mc->vd.width, 640);
			int max_height = MAX(mc->vd.height, 480);

	        EXTERNAL_CALL_WITH_ACTION(setInteger(width), Log::DEBUG
	            , mc->inputFormat->setInteger("width", max_width)
	            , STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -12)
	            ; return -12);
	        EXTERNAL_CALL_WITH_ACTION(setInteger(height), Log::DEBUG
	            , mc->inputFormat->setInteger("height", max_height)
	            , STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -13)
	            ; return -13);        

	        STATUS(Log::FLOW, Log::INFO, "inputformat changed into (%d,%d)", max_width, max_height);
    	}
    }

	if( sdk_level >= 24 )
	{
		EXTERNAL_CALL_WITH_ACTION(setInteger(color-range), Log::DEBUG
			, mc->inputFormat->setInteger("color-range" /* KEY_COLOR_RANGE */, 1 /*COLOR_RANGE_FULL*/) //KM-6037
			, STATUS(Log::FLOW, Log::DEBUG, "- ret(0x%X)", -14)
			; return -14);
	}

	if (NULL == mc->outputFormatChanged)
	{
		mc->outputFormatChanged = outputFormatChangedChecker;
	}

	Common::MCConfigParams mccp;
	memset(&mccp, 0x00, sizeof(Common::MCConfigParams));
	mccp.surface = NULL != mc->vd.mcvr ? mc->vd.mcvr->surfaceTexture : NULL;
	mccp.crypto = mc->vd.crypto;
	mccp.flags = 0;

	// initializes mutex vars and starts threads;
	// creates MediaCodec obj;
	// configures, starts, gets in/out buffers;
	int retValue = Nex_MC::Common::Decoder::Init(mc, &mccp);

	if (0 != retValue)
	{
		MC_ERR("Common::Decoder::Init failed");
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

namespace { // anon-release
	void releaseMediaCodec( NexCAL_mc *mc )
	{
		STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

		if (NULL != mc->mc)
		{
			if (NULL != mc->bufferInfo) // if this was made, start was probably called at some point
			{
				bool bCallStopRelease = true;
				STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseBufferInfo");
				NexMediaCodec_using_jni::releaseBufferInfo(mc->bufferInfo);
				STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseBufferInfo->");
				mc->bufferInfo = NULL;

				if ( Utils::ChipType::EXYNOS4 == Utils::GetChipType() && mc->vd.bInitFailed )
					bCallStopRelease = false;

				if (bCallStopRelease)
				{
					EXTERNAL_CALL(stop, Log::DEBUG, mc->mc->stop());
					EXTERNAL_CALL(release, Log::DEBUG, mc->mc->release());
				}
			}
			else if (Utils::ChipType::EXYNOS4 == Utils::GetChipType())
			{
				EXTERNAL_CALL(release, Log::DEBUG, mc->mc->release());
			}

			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaCodec");
			NexMediaCodec_using_jni::releaseNexMediaCodec(mc->mc);
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaCodec->");
			mc->mc = NULL;
		}

		STATUS(Log::FLOW, Log::DEBUG, "-");
	}
} // namespace (anon-release)

int Deinit( NexCAL_mc *mc )
{
	MC_DEBUG("+ mc(%p)", mc);
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	if (NEX_MC_TYPE_DECODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
	{
		if(mc->eosSent && !mc->eosReceived)
		{
			unsigned int uiFlag;
			unsigned int ticks = GetTickCount();

			while(GetTickCount() - ticks < 100)
			{
				uiFlag = NEXCAL_VDEC_FLAG_END_OF_STREAM;

				Nex_MC::Video::Decoder::Decode(NULL, 0, 0, 0, uiFlag, &uiFlag, (NXVOID*)mc);

				if( NEXCAL_CHECK_VDEC_RET(uiFlag, NEXCAL_VDEC_DECODING_SUCCESS) )
				{
					if( NEXCAL_CHECK_VDEC_RET(uiFlag, NEXCAL_VDEC_OUTPUT_EXIST) )
					{
						unsigned char*	pY = NULL;
						unsigned char*	pU = NULL;
						unsigned char*	pV = NULL;
						unsigned int uiDecOutCTS = 0;                    
				
						if( Nex_MC::Video::Decoder::GetOutput(&pY, NULL, NULL, &uiDecOutCTS, (NXVOID*)mc) != 0 )
						{
							MC_DEBUG("MC GetOutput failed(%d)", uiDecOutCTS);
							break;                            
						}

						if (NULL != mc->vd.mcvr)
						{                          
							Nex_MC::Video::Decoder::ReleaseOutputBuffer(mc, (long)pY, 0);
						}                            
					}
					else
					{
						usleep(5000);
					}
            
					if( NEXCAL_CHECK_VDEC_RET(uiFlag, NEXCAL_VDEC_EOS) )
					{
						MC_DEBUG("MC Deinit Eos Detected");
						break;
					}
				}
				else
				{
					MC_DEBUG("MC Deinit Decoding Failed");
					break;
				}
			}
		}
	}

	int retValue = Nex_MC::Common::Deinit(mc); // joins threads and destroys mutex vars

	if (NULL != mc->lastOutputBytes)
	{
		free(mc->lastOutputBytes);
		mc->lastOutputBytes = NULL;
		mc->lastOutputBytesSize = 0;
	}

	MC_DEBUG("+ releaseMediaCodec");
	releaseMediaCodec(mc);
	MC_DEBUG("- releaseMediaCodec");

	if (NULL != mc->inputFormat)
	{
		MC_DEBUG("->releaseNexMediaFormat");
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaFormat");
		NexMediaFormat_using_jni::releaseNexMediaFormat(mc->inputFormat);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaFormat->");
		MC_DEBUG("releaseNexMediaFormat->");
		mc->inputFormat = NULL;
	}

	if (NULL != mc->vd.mcvr && mc->vd.created_mcvr)
	{
		NexSurfaceTexture_using_recvd::releaseNexSurfaceTexture(mc->vd.mcvr->surfaceTexture, false, false);
		mc->vd.mcvr->surfaceTexture = NULL;
		mc->vd.mcvr->mc = NULL;
		free(mc->vd.mcvr);
		mc->vd.mcvr = NULL;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	MC_DEBUG("- ret(0x%X)", retValue);
	return retValue;
}

int Decode( 
		unsigned char* pData
		, int iLen
		, unsigned int uDTS
		, unsigned int uPTS
		, int nFlag
		, unsigned int* puDecodeResult
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ len(%d) ts(%u/%u) flag(0x%X) ud(%p)", iLen, uDTS, uPTS, nFlag, pUserData);

	int retValue = 0;
	bool eosDetected = false;

	if(IS_BINARY_FLAG_SET(nFlag, NEXCAL_VDEC_FLAG_OUTPUT_ONLY) == FALSE)
	{
		eosDetected = IS_BINARY_FLAG_SET(nFlag, NEXCAL_VDEC_FLAG_END_OF_STREAM);
	}

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (eosDetected && !mc->sentValidInputFrame)
	{
		retValue = 0;
		NEXCAL_INIT_VDEC_RET(*puDecodeResult);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);

		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
		return retValue;
	}

	NEXCAL_INIT_VDEC_RET(*puDecodeResult);

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	const unsigned int TIMEOUT = (eosDetected && Utils::ChipType::EDEN == Utils::GetChipType()) ? 100 : 3000;

	bool formatChanged = false;

	while (0 > mc->inIndex && 0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && TIMEOUT > GetTickCount() - ticks && !mc->exceptionState)
	{ // no input buffer to fill && no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (mc->inWaiting && !mc->eosSent)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "inLock");
			pthread_mutex_lock(&mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "inLock");

			mc->inWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "in");
			pthread_cond_broadcast(&mc->inCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "inLock");
			pthread_mutex_unlock(&mc->inLock);
		}

		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outIndex = -1; // it could have been -2 or -3
			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		// there's a reason this is done after waking both threads
		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	if(IS_BINARY_FLAG_SET(nFlag, NEXCAL_VDEC_FLAG_OUTPUT_ONLY) == FALSE)
	{
		if (0 > mc->inIndex)
		{
			retValue = -1;
		}
		else
		{
			int flags = 0;

			if (eosDetected)
			{
				STATUS(Log::FLOW, Log::DEBUG, "EOS detected");

				flags |= 4 /* BUFFER_FLAG_END_OF_STREAM */;
				mc->eosSent = true;
				iLen = 0;
				uPTS = 0;
			}
			else
			{
				mc->sentValidInputFrame = true;
			}

			if (NULL != pData && 0 < iLen)
			{
				STATUS(Log::INPUT, Log::VERBOSE, "pts(%lld) flag(0x%X) ud(%p)", (int64_t)(uPTS * 1000LL), flags, pUserData);
				MEM_DUMP(Log::INPUT, Log::VERBOSE, pData, iLen, 32);
				STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], pData, iLen);
				memcpy(mc->inputBuffers[mc->inIndex], pData, iLen);
			}

			EXTERNAL_CALL_WITH_BOTH_ACTIONS(queueInputBuffer, Log::DEBUG
				, mc->mc->queueInputBuffer(mc->inIndex, 0, iLen, (int64_t)(uPTS * 1000LL), flags)
				,
				, retValue = 0
				; NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS)
				; NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME));

			mc->inIndex = -1;
		}
	}
	else
	{
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if(IS_BINARY_FLAG_SET(nFlag, NEXCAL_VDEC_FLAG_DECODE_ONLY) == FALSE)
	{
		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}
		
		if (mc->eosSent && !mc->eosReceived)
		{
			while (0 > mc->outIndex && !mc->eosReceived && TIMEOUT > GetTickCount() - ticks && !mc->exceptionState)
			{ // no output buffer received && not timed out yet
				mc->engWaiting = true;

				if (-2 == mc->outIndex)
				{
					formatChanged = true;
					mc->outIndex = -1;
				}

				if (mc->outWaiting)
				{
					STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
					pthread_mutex_lock(&mc->outLock);
					STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

					mc->outIndex = -1; // it could have been -2 or -3
					mc->outWaiting = false;

					STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
					pthread_cond_broadcast(&mc->outCond);

					STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
					pthread_mutex_unlock(&mc->outLock);
				}

				STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
				pthread_cond_wait(&mc->engCond, &mc->engLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

				mc->engWaiting = false;
			}

			if (0 > mc->outIndex && !mc->eosReceived && TIMEOUT <= GetTickCount() - ticks && !mc->exceptionState)
			{
				retValue = 0;
				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_TIMEOUT);
			}
		}

		if (formatChanged)
		{
			retValue = 0;
			int ret = 0;

			NexMediaFormat *outputFormat = NULL;

			EXTERNAL_CALL(getOutputFormat, Log::DEBUG, mc->mc->getOutputFormat(outputFormat));

			if (!mc->exceptionState)
			{
				EXTERNAL_CALL(getInteger(width), Log::DEBUG, outputFormat->getInteger("width", mc->vd.width));
			}

			if (!mc->exceptionState)
			{
				EXTERNAL_CALL(getInteger(height), Log::DEBUG, outputFormat->getInteger("height", mc->vd.height));
			}

			if (!mc->exceptionState)
			{
				EXTERNAL_CALL(getInteger(color-format), Log::DEBUG, outputFormat->getInteger("color-format", mc->vd.colorformat));
			}

			if (!mc->exceptionState)
			{
				STATUS(Log::FLOW, Log::DEBUG, "width(%d) height(%d)", mc->vd.width, mc->vd.height);

				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_SETTING_CHANGE);              
			}
		}
		else
		{
			if (0 <= mc->outIndex)
			{ // there's output to return
				retValue = 0;
				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);              

				if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
				{
					mc->eosReceived = true;
				}

				if (NULL != mc->vd.releaseOutputBuffer && mc->outIndex < mc->mc->getNumOutputBuffers())
				{
					mc->outputBuffers[mc->outIndex] = (void *)BUFFER_LOCAL;
				}
			}
			else
			{
				mc->outIndex = -1; // it could have been -2 or -3
			}
		}

		if (mc->eosReceived)
		{
			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);              
		}
	}

	if (!mc->exceptionState)
	{
		if ((unsigned int)-1 == retValue)
		{
			// retValue == -1 if there was no input and there was no output... i.e. timeout.
			retValue = 0;
			if(IS_BINARY_FLAG_SET(nFlag, NEXCAL_VDEC_FLAG_DECODE_ONLY) == FALSE)
				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_TIMEOUT);
			else            
				NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);               
		}
	}
	else
	{
		// really only want to return -1 if there was an exception during decoding
		STATUS(Log::FLOW, Log::VERBOSE, "exception state");
		retValue = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}

int DecodePR( 
		unsigned char* pData
		, int iLen
		, unsigned int uDTS
		, unsigned int uPTS
		, int nFlag
		, unsigned int* puDecodeResult
		, NXVOID *pUserData)
{
	STATUS(Log::FLOW, Log::INFO, "+ len(%d) ts(%u/%u) flag(0x%X) ud(%p)", iLen, uDTS, uPTS, nFlag, pUserData);

	int retValue = 0;

	bool eosDetected = IS_BINARY_FLAG_SET(nFlag, NEXCAL_VDEC_FLAG_END_OF_STREAM);

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (eosDetected && !mc->sentValidInputFrame)
	{
		retValue = 0;
		NEXCAL_INIT_VDEC_RET(*puDecodeResult);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME);
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);

		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
		return retValue;
	}

	NEXCAL_INIT_VDEC_RET(*puDecodeResult);

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	const unsigned int TIMEOUT = (eosDetected && Utils::ChipType::EDEN == Utils::GetChipType()) ? 100 : 3000;

	bool formatChanged = false;

	while (0 > mc->inIndex && 0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && TIMEOUT > GetTickCount() - ticks && !mc->exceptionState)
	{ // no input buffer to fill && no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (mc->inWaiting && !mc->eosSent)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "inLock");
			pthread_mutex_lock(&mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "inLock");

			mc->inWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "in");
			pthread_cond_broadcast(&mc->inCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "inLock");
			pthread_mutex_unlock(&mc->inLock);
		}

		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outIndex = -1; // it could have been -2 or -3
			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		// there's a reason this is done after waking both threads
		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	if (0 > mc->inIndex)
	{
		retValue = -1;
	}
	else
	{
		int flags = 0;

		if (eosDetected)
		{
			STATUS(Log::FLOW, Log::DEBUG, "EOS detected");

			flags |= 4 /* BUFFER_FLAG_END_OF_STREAM */;
			mc->eosSent = true;
		}
		else
		{
			mc->sentValidInputFrame = true;
		}

		if (NULL != pData)
		{
			Nex_MC::Common::MediaCodecCryptoInfo *mcci = (Nex_MC::Common::MediaCodecCryptoInfo *)pData;

			EXTERNAL_CALL(setCryptoInfo, Log::DEBUG, Nex_MC::NexMediaCodec_using_jni::setCryptoInfo(mc->vd.cryptoInfo, mcci->numSubSamples, mcci->numBytesOfClearData, mcci->numBytesOfEncryptedData, mcci->key, mcci->iv, mcci->mode));

			if (NULL != mcci->pData && 0 < mcci->iLen)
			{
				STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], mcci->pData, mcci->iLen);
				memcpy(mc->inputBuffers[mc->inIndex], mcci->pData, mcci->iLen);
			}
		}

		EXTERNAL_CALL_WITH_BOTH_ACTIONS(queueSecureInputBuffer, Log::DEBUG
			, mc->mc->queueSecureInputBuffer(mc->inIndex, 0, mc->vd.cryptoInfo, (int64_t)(uPTS * 1000LL), flags)
			,
			, retValue = 0
			; NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS)
			; NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_NEXT_FRAME));

		mc->inIndex = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (mc->eosSent && !mc->eosReceived)
	{
		while (0 > mc->outIndex && !mc->eosReceived && TIMEOUT > GetTickCount() - ticks && !mc->exceptionState)
		{ // no output buffer received && not timed out yet
			mc->engWaiting = true;

			if (-2 == mc->outIndex)
			{
				formatChanged = true;
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outIndex = -1; // it could have been -2 or -3
				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

			mc->engWaiting = false;
		}

		if (0 > mc->outIndex && !mc->eosReceived && TIMEOUT <= GetTickCount() - ticks)
        {
            retValue = 0;
            NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_TIMEOUT);
        }
	}

	if (formatChanged)
	{
		retValue = 0;
		int ret = 0;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL(getOutputFormat, Log::DEBUG, mc->mc->getOutputFormat(outputFormat));

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL(getInteger(width), Log::DEBUG, outputFormat->getInteger("width", mc->vd.width));
		}

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL(getInteger(height), Log::DEBUG, outputFormat->getInteger("height", mc->vd.height));
		}

		if (!mc->exceptionState)
		{
			STATUS(Log::FLOW, Log::DEBUG, "width(%d) height(%d)", mc->vd.width, mc->vd.height);

			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_SETTING_CHANGE);              
		}
	}
	else
	{
		if (0 <= mc->outIndex)
		{ // there's output to return
			retValue = 0;
			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_DECODING_SUCCESS);
			NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_EXIST);              

			if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
			{
				mc->eosReceived = true;
			}

			if (NULL != mc->vd.releaseOutputBuffer && mc->outIndex < mc->mc->getNumOutputBuffers())
			{
				mc->outputBuffers[mc->outIndex] = (void *)BUFFER_LOCAL;
			}
		}
		else
		{
			mc->outIndex = -1; // it could have been -2 or -3
		}
	}

	if (mc->eosReceived)
	{
		NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_EOS);
	}

	if ((unsigned int)-1 == retValue)
    {
        // retValue == -1 if there was no input and there was no output... i.e. timeout.
        retValue = 0;
        NEXCAL_SET_VDEC_RET(*puDecodeResult, NEXCAL_VDEC_OUTPUT_TIMEOUT);
    }

    if (mc->exceptionState)
    {
        // really only want to return -1 if there was an exception during decoding
        STATUS(Log::FLOW, Log::VERBOSE, "exception state");
        retValue = -1;
    }

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}

int GetOutput(
		 unsigned char** ppBits1
		, unsigned char** ppBits2
		, unsigned char** ppBits3
		, unsigned int *pDTS
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (0 <= mc->outIndex)
	{
		STATUS(Log::OUTPUT, Log::DEBUG, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

		if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
		{
			mc->eosReceived = true;
		}

		if (NULL != pDTS)
			*pDTS = mc->lastOutputPTS / 1000;

		if (NULL != mc->vd.mcvr)
		{
			if (NULL != ppBits1)
			{
				*ppBits1 = (unsigned char *)mc->outIndex;
			}

			if (NULL != ppBits2)
			{
				*ppBits2 = (unsigned char *)mc->outIndex;
			}

			if (NULL != ppBits3)
			{
				*ppBits3 = (unsigned char *)mc->outIndex;
			}

			mc->outIndex = -1;
		}
		else
		{
			if (NULL != mc->lastOutputBytes && mc->lastOutputSize > mc->lastOutputBytesSize)
			{
				free(mc->lastOutputBytes);
				mc->lastOutputBytes = NULL;
				mc->lastOutputBytesSize = 0;
			}

			if (NULL == mc->lastOutputBytes)
			{
				mc->lastOutputBytes = (unsigned char *)memalign(32, mc->lastOutputSize);
				if (NULL == mc->lastOutputBytes)
				{
					MC_ERR("memalign failed!");
					STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
					pthread_mutex_unlock(&mc->engLock);
					retValue = -1;
					if (NULL != ppBits1)
						*ppBits1 = NULL;
					if (NULL != ppBits2)
						*ppBits2 = NULL;
					if (NULL != ppBits3)
						*ppBits3 = NULL;
					if (NULL != pDTS)
						*pDTS = 0;
					STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) ts(%u)", retValue, 0 != pDTS ? *pDTS : 0xFFFFFFFF);
					return retValue;
				}
				STATUS(Log::FLOW, Log::INFO, "%p = memalign(32, %d)", mc->lastOutputBytes, mc->lastOutputSize);
				mc->lastOutputBytesSize = mc->lastOutputSize;
			}
			memcpy(mc->lastOutputBytes, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);

#if defined(NEX_WRITE_OUTPUT_TO_DISK)
			char filename[1024];
			memset(filename, 0x00, 1024);
			int stringlength = snprintf(filename, 1023, "/sdcard/yuvdump-%lld_.yuv", mc->lastOutputPTS);
			FILE * out = fopen(filename, "w");
			fwrite(((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize, 1, out);
			fclose(out);
			out = NULL;
			STATUS(Log::FLOW, Log::INFO, "wrote to %s", filename);
#endif

			if (1)//mcvd->lastOutputFlags != 2)
			{
				if (NULL != ppBits1)
					*ppBits1 = mc->lastOutputBytes;
				if (NULL != ppBits2)
					*ppBits2 = mc->lastOutputBytes + mc->vd.width*mc->vd.height;
				if (NULL != ppBits3)
					*ppBits3 = *ppBits2 + (mc->vd.width*mc->vd.height>>2);
				// TODO: color format conversion will be required
			}

			STATUS(Log::FLOW, Log::INFO, "ppBits: 1(%p)", *ppBits1);

			EXTERNAL_CALL(releaseOutputBuffer, Log::DEBUG, mc->mc->releaseOutputBuffer(mc->outIndex, false));

			mc->outIndex = -1;
		}
	}
	else
	{
		retValue = -1;
		if (NULL != ppBits1)
			*ppBits1 = NULL;
		if (NULL != ppBits2)
			*ppBits2 = NULL;
		if (NULL != ppBits3)
			*ppBits3 = NULL;
		if (NULL != pDTS)
			*pDTS = 0;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) ts(%u)", retValue, 0 != pDTS ? *pDTS : 0xFFFFFFFF);
	return retValue;
}

int Reset( NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (!mc->sentValidInputFrame)
	{ // no need to reset
		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
		return retValue;
	}

	if (mc->eosSent)
	{
		STATUS(Log::FLOW, Log::DEBUG, "eos has been sent. flush may not work. releasing and recreating mc");

		if (NEX_MC_TYPE_DECODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
		{
			if(mc->eosSent && !mc->eosReceived)
			{
				unsigned int uiFlag;
				unsigned int ticks = GetTickCount();

				while(GetTickCount() - ticks < 100)
				{
					uiFlag = NEXCAL_VDEC_FLAG_END_OF_STREAM;

					Nex_MC::Video::Decoder::Decode(NULL, 0, 0, 0, uiFlag, &uiFlag, (NXVOID*)mc);

					if( NEXCAL_CHECK_VDEC_RET(uiFlag, NEXCAL_VDEC_DECODING_SUCCESS) )
					{
						if( NEXCAL_CHECK_VDEC_RET(uiFlag, NEXCAL_VDEC_OUTPUT_EXIST) )
						{
							unsigned char*	pY = NULL;
							unsigned char*	pU = NULL;
							unsigned char*	pV = NULL;
							unsigned int uiDecOutCTS = 0;                    
				
							if( Nex_MC::Video::Decoder::GetOutput(&pY, NULL, NULL, &uiDecOutCTS, (NXVOID*)mc) != 0 )
							{
								MC_DEBUG("MC GetOutput failed(%d)", uiDecOutCTS);
								break;                            
							}

							if (NULL != mc->vd.mcvr)
							{                          
								Nex_MC::Video::Decoder::ReleaseOutputBuffer(mc, (long)pY, 0);
							}                            
						}
						else
						{
							usleep(5000);
						}
                    
						if( NEXCAL_CHECK_VDEC_RET(uiFlag, NEXCAL_VDEC_EOS) )
						{
							MC_DEBUG("MC Deinit Eos Detected");
							break;
						}
					}
					else
					{
						MC_DEBUG("MC Deinit Decoding Failed");
						break;
					}
				}
			}
		}

		retValue = Nex_MC::Common::Deinit(mc); // joins threads and destroys mutex vars

		if (0 != retValue)
		{
			MC_WARN("error occurred during deinit (0x%X)", retValue);
		}

		releaseMediaCodec(mc);

		retValue = Nex_MC::Video::Decoder::Init(mc);

		mc->inIndex = -1;
		mc->outIndex = -1;

		mc->eosSent = false;
		mc->eosReceived = false;

		mc->sentValidInputFrame = false;

		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
		return retValue;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	while ((mc->inAtExternal || mc->outAtExternal) && !mc->exceptionState)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		usleep(1000); // 1 ms -- should only have to sleep up to MAX(INPUT_TIMEOUT,OUTPUT_TIMEOUT)

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");
	}

	mc->inIndex = -1;
	mc->outIndex = -1;

	EXTERNAL_CALL(flush, Log::DEBUG, mc->mc->flush());

	mc->eosSent = false;
	mc->eosReceived = false;

	mc->sentValidInputFrame = false;

	for (int i = 0; i < mc->mc->getNumOutputBuffers(); ++i)
	{
		mc->outputBuffers[i] = (void *)BUFFER_REMOTE;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int GetInfo( unsigned int uIndex, unsigned int* puResult, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ idx(%d) ud(%p)", uIndex, pUserData);

	unsigned int retValue = NEXCAL_ERROR_NONE;

	if (0 == pUserData)
	{
		MC_ERR("uUserData is zero");
		retValue = NEXCAL_ERROR_INVALID_PARAMETER;
	}
	else
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		switch (uIndex)
		{
		case NEXCAL_VIDEO_GETINFO_WIDTH :
			*puResult = mc->vd.width;
			break;
		case NEXCAL_VIDEO_GETINFO_HEIGHT :
			*puResult = mc->vd.height;
			break;
		case NEXCAL_VIDEO_GETINFO_WIDTHPITCH :
			*puResult = mc->vd.pitch;
			break;
		case NEXCAL_VIDEO_GETINFO_RENDERER_RESET :
			*puResult = NEX_CAL_VIDEO_GETINFO_USE_RENDERER_RESET;
			break;
		case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
			*puResult = mc->vd.colorformat;
			break;            
		default :
			MC_ERR("Invalid index(0x%X)", uIndex);
			retValue = NEXCAL_ERROR_INVALID_PARAMETER;
			break;
		}
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) result(0x%X)", retValue, *puResult);
	return retValue;
}

}; // namespace Decoder

namespace Encoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

namespace { // (anon119)
	bool outputFormatChangedChecker(NexCAL_mc *mc)
	{
		return false;
	}
}; // namespace (anon119)

void SetDebugLevel( int types, int max_level )
{
	Log::SetDebugLevel(NEX_MC_TYPE, NEX_MC_AVTYPE, types, max_level);
}

int GetProperty( unsigned int uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%d) ud(%p)", uProperty, pUserData);

	unsigned int retValue = 0;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_VIDEO_INPUT_BUFFER_TYPE :
		{
			*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_UNKNOWN;
			switch (Utils::GetChipType())
			{
			case Utils::ChipType::QUALCOMM_8974:
			case Utils::ChipType::QUALCOMM_8XXX:
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
				break;
			case Utils::ChipType::QUALCOMM_7XXX:
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_NV21;
				break;
			case Utils::ChipType::EXYNOS4:
			case Utils::ChipType::EXYNOS:
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_NV21;
				break;
			case Utils::ChipType::MTK:
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420;
				break;
			default:
				*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420;
				break;
			}
		}
		break;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}

int SetProperty( unsigned int uProperty, NXINT64 uValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%u) value(%lld) ud(%p)", uProperty, uValue, pUserData);

	unsigned int retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int Init( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	if (NULL == mc->inputFormat)
	{
		EXTERNAL_CALL_WITH_ACTION(createVideoFormat, Log::DEBUG
			, NexMediaFormat_using_jni::createVideoFormat(mc->mimeType, mc->ve.width, mc->ve.height, mc->inputFormat)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -10)
			; return -10);

		if (NULL == mc->inputFormat)
		{
			MC_ERR("failed to create MediaFormat instance");
			STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -1);
			return -1;
		}

		EXTERNAL_CALL_WITH_ACTION(setInteger(color-format), Log::DEBUG
			, mc->inputFormat->setInteger("color-format" /* KEY_COLOR_FORMAT */, mc->ve.colorFormat)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -11)
			; return -11);

		EXTERNAL_CALL_WITH_ACTION(setInteger(bitrate), Log::DEBUG
			, mc->inputFormat->setInteger("bitrate" /* KEY_BIT_RATE */, mc->ve.bitrate /* bits per sec */)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -12)
			; return -12);

		EXTERNAL_CALL_WITH_ACTION(setInteger(frame-rate), Log::DEBUG
			, mc->inputFormat->setInteger("frame-rate" /* KEY_FRAME_RATE */, mc->ve.fps /* frames per sec */)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -13)
			; return -13);

		EXTERNAL_CALL_WITH_ACTION(setInteger(i-frame-interval), Log::DEBUG
			, mc->inputFormat->setInteger("i-frame-interval" /* KEY_I_FRAME_INTERVAL */, 1 /* 1 second per i-frame */)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -14)
			; return -14);

		int sdk_level = Nex_MC::Utils::GetBuildVersionSDK();
		LOGW("mc->ve.profile == %d, mc->ve.level==%d, sdk level=%d\n",mc->ve.profile, mc->ve.level, sdk_level );

#ifdef FOR_PROJECT_LGE
		if( sdk_level >= 23 )
		{
			if ( mc->ve.profile == 1 )
			{
				EXTERNAL_CALL_WITH_ACTION(setInteger(profile), Log::DEBUG
				, mc->inputFormat->setInteger("profile" /* KEY_PROFILE */, 8 /* AVCProfileHigh */)
				, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -15)
				; return -15);

				EXTERNAL_CALL_WITH_ACTION(setInteger(level), Log::DEBUG
				, mc->inputFormat->setInteger("level" /* KEY_LEVEL */, 0x800 /* AVCLevel4 */)
				, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -16)
				; return -16);
			}
			else
			{
				EXTERNAL_CALL_WITH_ACTION(setInteger(profile), Log::DEBUG
				, mc->inputFormat->setInteger("profile" /* KEY_PROFILE */, 1 /* AVCProfileBaseline */)
				, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -17)
				; return -17);

				EXTERNAL_CALL_WITH_ACTION(setInteger(level), Log::DEBUG
				, mc->inputFormat->setInteger("level" /* KEY_LEVEL */, 512  /* AVCLevel4 */)
				, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -18)
				; return -18);
			}
		}
		else
		{
			if ( mc->ve.profile == 1 )
			{
				EXTERNAL_CALL_WITH_ACTION(setInteger(profile), Log::DEBUG
				, mc->inputFormat->setInteger("profile" /* KEY_PROFILE */, 8 /* AVCProfileHigh */)
				, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -15)
				; return -15);
			}            
		}
#else
		if (Utils::ChipType::ROCK != Utils::GetChipType() && sdk_level >= 23 )
		{
			EXTERNAL_CALL_WITH_ACTION(setInteger(profile), Log::DEBUG
			, mc->inputFormat->setInteger("profile" /* KEY_PROFILE */, mc->ve.profile)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -15)
			; return -15);

			EXTERNAL_CALL_WITH_ACTION(setInteger(level), Log::DEBUG
			, mc->inputFormat->setInteger("level" /* KEY_LEVEL */, mc->ve.level)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -16)
			; return -16);

			if (mc->ve.profile >= 0x08/*AVCProfileHigh*/ )
			{
				//to set a high profile to encoder forcely
				EXTERNAL_CALL_WITH_ACTION(setInteger(recorder), Log::DEBUG
				, mc->inputFormat->setInteger("recorder" /* KEY_RECORDER */, 1)
				, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -17)
				; return -17);
			}
		}
#endif
	}

	if (NULL == mc->outputFormatChanged)
	{
		mc->outputFormatChanged = outputFormatChangedChecker;
	}

	Common::MCConfigParams mccp;
	memset(&mccp, 0x00, sizeof(Common::MCConfigParams));
	mccp.surface = NULL;
	mccp.crypto = NULL;
	mccp.flags = 1 /* CONFIGURE_FLAG_ENCODE */;

	// initializes mutex vars and starts threads;
	// creates MediaCodec obj;
	// configures, creates input surface, starts, gets out buffers;
	mc->inIndex = -1;
	unsigned int retValue = Nex_MC::Common::Encoder::Init(mc, &mccp);

	mc->ve.surface = mccp.surface;

	if (0 != retValue)
	{
		MC_ERR("Common::Encoder::Init failed");
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int Deinit( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	int retValue = Nex_MC::Common::Deinit(mc); // joins threads and destroys mutex vars


	if (NULL != mc->lastOutputBytes)
	{
		free(mc->lastOutputBytes);
		mc->lastOutputBytes = NULL;
		mc->lastOutputBytesSize = 0;
	}

	if (NULL != mc->mc)
	{
		if (NULL != mc->bufferInfo) // if this was made, start was probably called at some point
		{
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseBufferInfo");
			NexMediaCodec_using_jni::releaseBufferInfo(mc->bufferInfo);
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseBufferInfo->");
			mc->bufferInfo = NULL;

			EXTERNAL_CALL(stop, Log::DEBUG, mc->mc->stop());

			EXTERNAL_CALL(release, Log::DEBUG, mc->mc->release());
		}

		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaCodec");
		NexMediaCodec_using_jni::releaseNexMediaCodec(mc->mc);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaCodec->");
		mc->mc = NULL;
	}

	if (NULL != mc->inputFormat)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaFormat");
		NexMediaFormat_using_jni::releaseNexMediaFormat(mc->inputFormat);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaFormat->");
		mc->inputFormat = NULL;
	}

	if (NULL != mc->ve.surface)
	{
		jobject surface = NULL;
		// exception cannot occur for the following function (as of yet)
		mc->ve.surface->getSurfaceObject(surface);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexSurfaceTexture");
		NexSurfaceTexture_using_recvd::releaseNexSurfaceTexture(mc->ve.surface, true, true);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexSurfaceTexture->");
		mc->ve.surface = NULL;

	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int Encode
		( unsigned char* pData1
		, unsigned char* pData2
		, unsigned char* pData3
		, unsigned int uPTS
		, unsigned int* puEncodeResult
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ pData(%p,%p,%p) uPTS(%u) ud(%p)", pData1, pData2, pData3, uPTS, pUserData);

	int retValue = -1;

	bool eosDetected = NULL == pData1;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	NEXCAL_INIT_VENC_RET(*puEncodeResult);
	NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_ENCODING_SUCCESS);
	NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_NEXT_INPUT);

	int ret = 0;

	if (eosDetected)
	{
		if (!mc->eosSent)
		{
			EXTERNAL_CALL(signalEndOfInputStream, Log::DEBUG, mc->mc->signalEndOfInputStream());
			mc->eosSent = true;
		}

		if (!mc->sentValidInputFrame)
		{
			retValue = 0;
			NEXCAL_INIT_VENC_RET(*puEncodeResult);
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_ENCODING_SUCCESS);
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_EOS);

			STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
			return retValue;
		}
	}
	else
	{
		mc->sentValidInputFrame = true;
		NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_NEXT_INPUT);
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	bool formatChanged = false;

	mc->outTimeout = 5000; // 5ms

	while (0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && 5 > GetTickCount() - ticks && !mc->exceptionState)
	{ // no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outIndex = -1; // it could have been -2 or -3
			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (mc->eosSent && !mc->eosReceived)
	{
		retValue = -1; // we err unless we get output

		mc->outTimeout = 300000; // 300ms

		while (0 > mc->outIndex && !mc->eosReceived && 3000 > GetTickCount() - ticks && !mc->exceptionState)
		{ // no output buffer received && not timed out yet
			mc->engWaiting = true;

			if (-2 == mc->outIndex)
			{
				formatChanged = true;
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outIndex = -1; // it could have been -2 or -3
				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

			mc->engWaiting = false;
		}
	}

	if (formatChanged)
	{
		retValue = 0;
		int ret = 0;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, retValue = -1);

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(width), Log::DEBUG
				, outputFormat->getInteger("width", mc->ve.width)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(height), Log::DEBUG
				, outputFormat->getInteger("height", mc->ve.height)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			STATUS(Log::FLOW, Log::DEBUG, "width(%d) height(%d)", mc->ve.width, mc->ve.height);

			// TODO: need encoder equivalent:
			//NEXCAL_VDEC_SET_RET_DECODING(*puDecodeResult, NEXCAL_VDEC_RET_DECODING_SUCCESS);
			//NEXCAL_VDEC_SET_RET_SETTING_CHANGE(*puDecodeResult, NEXCAL_VDEC_RET_SETTINGS_CHANGED);
		}
	}
	else
	{
		if (0 <= mc->outIndex)
		{ // there's output to return
			retValue = 0;
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_OUTPUT_EXIST);

			if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
			{
				mc->eosReceived = true;
			}
		}
		else
		{
			mc->outIndex = -1; // it could have been -2 or -3
		}
	}

	if (mc->eosReceived)
	{
		NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_EOS);
	}

	if (!mc->exceptionState)
	{
		if ((unsigned int)-1 == retValue)
		{
			// retValue == -1 if there was no input and there was no output... i.e. timeout.
			retValue = 0;
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_OUTPUT_TIMEOUT);
		}
	}
	else
	{
		// really only want to return -1 if there was an exception during decoding
		STATUS(Log::FLOW, Log::VERBOSE, "exception state");
		NEXCAL_INIT_VENC_RET(*puEncodeResult);
		retValue = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
	return retValue;
}

int EncodeUsingFrameData
		( unsigned char* pData1
		, unsigned char* pData2
		, unsigned char* pData3
		, unsigned int uPTS
		, unsigned int* puEncodeResult
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ pData(%p,%p,%p) uPTS(%u) ud(%p)", pData1, pData2, pData3, uPTS, pUserData);

	int retValue = 0;

	bool eosDetected = NULL == pData1;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (eosDetected && !mc->sentValidInputFrame)
	{
		retValue = 0;
		NEXCAL_INIT_VENC_RET(*puEncodeResult);
		NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_ENCODING_SUCCESS);
		NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_EOS);

		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
		return retValue;
	}

	NEXCAL_INIT_VENC_RET(*puEncodeResult);
	NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_ENCODING_SUCCESS);

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	const unsigned int TIMEOUT = 3000;

	bool formatChanged = false;

	while (0 > mc->inIndex && 0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && TIMEOUT > GetTickCount() - ticks && !mc->exceptionState)
	{ // no input buffer to fill && no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (mc->inWaiting && !mc->eosSent)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "inLock");
			pthread_mutex_lock(&mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "inLock");

			mc->inWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "in");
			pthread_cond_broadcast(&mc->inCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "inLock");
			pthread_mutex_unlock(&mc->inLock);
		}

		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outIndex = -1; // it could have been -2 or -3
			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		// there's a reason this is done after waking both threads
		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	if (0 > mc->inIndex)
	{
		retValue = -1;
	}
	else
	{
		int flags = 0;

		if (eosDetected)
		{
			STATUS(Log::FLOW, Log::DEBUG, "EOS detected");

			flags |= 4 /* BUFFER_FLAG_END_OF_STREAM */;
			mc->eosSent = true;
		}
		else
		{
			mc->sentValidInputFrame = true;
		}

		size_t size = 0;

		if (NULL != pData1)
		{
			size_t inSizeY = mc->ve.width * mc->ve.height;

			switch (Utils::GetChipType())
			{
			case Utils::ChipType::QUALCOMM_8974:
				if (1920 <= mc->ve.width && 1080 <= mc->ve.height)
				{
					size_t width16 = (mc->ve.width + 15) & -16;
					size_t height16 = (mc->ve.height + 15) & -16;
					size_t sizeY = width16 * height16;
					size_t sizeC = sizeY >> 2;
					size = sizeY + sizeC*2;
					if (width16 == (size_t)mc->ve.width)
					{
						STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, mc->inputBuffers[mc->inIndex], pData1, inSizeY);
						memcpy(mc->inputBuffers[mc->inIndex], pData1, inSizeY);
						STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, (uint8_t *)mc->inputBuffers[mc->inIndex] + sizeY, pData1 + inSizeY, inSizeY >> 1);
						memcpy((uint8_t *)mc->inputBuffers[mc->inIndex] + sizeY, pData1 + inSizeY, inSizeY >> 1);
					}
					else
					{
						for (int i = 0; i < mc->ve.height; ++i)
						{
							STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, (uint8_t *)mc->inputBuffers[mc->inIndex] + width16*i, pData1 + mc->ve.width*i, mc->ve.width);
							memcpy((uint8_t *)mc->inputBuffers[mc->inIndex] + width16*i, pData1 + mc->ve.width*i, mc->ve.width);
						}
						for (int i = 0; i < mc->ve.height/2; ++i)
						{
							STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, (uint8_t *)mc->inputBuffers[mc->inIndex] + sizeY + width16*i, pData1 + mc->ve.width*(i+mc->ve.height), mc->ve.width);
							memcpy((uint8_t *)mc->inputBuffers[mc->inIndex] + sizeY + width16*i, pData1 + mc->ve.width*(i+mc->ve.height), mc->ve.width);
						}
					}
					break;
				}
				// else fall through:
			case Utils::ChipType::QUALCOMM_8XXX:
				{
					size_t sizeY = mc->ve.width * mc->ve.height;
					size_t sizeC = sizeY >> 2;
					size_t sizeY_2048 = (sizeY + 2047) & -2048;
					size = ((sizeY_2048 + ((2*sizeC + 2047) & -2048)) + 4095) & -4096;

					STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, mc->inputBuffers[mc->inIndex], pData1, sizeY);
					memcpy(mc->inputBuffers[mc->inIndex], pData1, sizeY);
					STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, (uint8_t *)mc->inputBuffers[mc->inIndex] + sizeY_2048, pData1 + sizeY, sizeC*2);
					memcpy((uint8_t *)mc->inputBuffers[mc->inIndex] + sizeY_2048, pData1 + sizeY, sizeC*2);
				}
				break;
			case Utils::ChipType::MTK:
			default:
				{
					size_t sizeY = mc->ve.width * mc->ve.height;
					size_t sizeC = sizeY >> 2;
					size = sizeY + sizeC*2;
					STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %zu)", mc->inIndex, mc->inputBuffers[mc->inIndex], pData1, size);
					memcpy(mc->inputBuffers[mc->inIndex], pData1, size);
				}
				break;
			}
		}

		EXTERNAL_CALL_WITH_BOTH_ACTIONS(queueInputBuffer, Log::DEBUG
			, mc->mc->queueInputBuffer(mc->inIndex, 0, size, (int64_t)(uPTS * 1000LL), flags)
			,
			, retValue = 0
			; NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_ENCODING_SUCCESS)
			; NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_NEXT_INPUT));

		mc->inIndex = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (mc->eosSent && !mc->eosReceived)
	{
		retValue = -1; // we err unless we get output

		mc->outTimeout = 300000; // 300ms

		while (0 > mc->outIndex && !mc->eosReceived && TIMEOUT > GetTickCount() - ticks && !mc->exceptionState)
		{ // no output buffer received && not timed out yet
			mc->engWaiting = true;

			if (-2 == mc->outIndex)
			{
				formatChanged = true;
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outIndex = -1; // it could have been -2 or -3
				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

			mc->engWaiting = false;
		}

		if (0 > mc->outIndex && !mc->eosReceived && TIMEOUT <= GetTickCount() - ticks && !mc->exceptionState)
		{
			retValue = 0;
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_OUTPUT_TIMEOUT);
		}
	}

	if (formatChanged)
	{
		retValue = 0;
		int ret = 0;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, retValue = -1);

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(width), Log::DEBUG
				, outputFormat->getInteger("width", mc->ve.width)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(height), Log::DEBUG
				, outputFormat->getInteger("height", mc->ve.height)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			STATUS(Log::FLOW, Log::DEBUG, "width(%d) height(%d)", mc->ve.width, mc->ve.height);

			// TODO: need encoder equivalent:
			//NEXCAL_VDEC_SET_RET_DECODING(*puDecodeResult, NEXCAL_VDEC_RET_DECODING_SUCCESS);
			//NEXCAL_VDEC_SET_RET_SETTING_CHANGE(*puDecodeResult, NEXCAL_VDEC_RET_SETTINGS_CHANGED);
		}
	}
	else
	{
		if (0 <= mc->outIndex)
		{ // there's output to return
			retValue = 0;
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_OUTPUT_EXIST);

			if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
			{
				mc->eosReceived = true;
			}
		}
		else
		{
			mc->outIndex = -1; // it could have been -2 or -3
		}
	}

	if (mc->eosReceived)
	{
		NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_EOS);
	}

	if (!mc->exceptionState)
	{
		if ((unsigned int)-1 == retValue)
		{
			// retValue == -1 if there was no input and there was no output... i.e. timeout.
			retValue = 0;
			NEXCAL_SET_VENC_RET(*puEncodeResult, NEXCAL_VENC_OUTPUT_TIMEOUT);
		}
	}
	else
	{
		// really only want to return -1 if there was an exception during encoding
		STATUS(Log::FLOW, Log::VERBOSE, "exception state");
		retValue = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
	return retValue;
}

int GetOutput
		( unsigned char** ppOutData
		, int* piOutLen
		, unsigned int* puPTS
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	unsigned int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (0 <= mc->outIndex)
	{
		STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

		if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
		{
			mc->eosReceived = true;
		}

		*puPTS = mc->lastOutputPTS / 1000;

		if (NULL != mc->lastOutputBytes && mc->lastOutputSize > mc->lastOutputBytesSize)
		{
			free(mc->lastOutputBytes);
			mc->lastOutputBytes = NULL;
			mc->lastOutputBytesSize = 0;
		}

		if (NULL == mc->lastOutputBytes)
		{
			mc->lastOutputBytes = (unsigned char *)memalign(32, mc->lastOutputSize);
			if (NULL == mc->lastOutputBytes)
			{
				MC_ERR("memalign failed!");
				STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
				pthread_mutex_unlock(&mc->engLock);
				retValue = -1;
				if (NULL != ppOutData)
					*ppOutData = NULL;
				if (NULL != piOutLen)
					*piOutLen = 0;
				if (NULL != puPTS)
					*puPTS = 0;
				STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) outLen(%u) ts(%u)", retValue, 0 != piOutLen ? *piOutLen : 0xFFFFFFFF, 0 != puPTS ? *puPTS : 0xFFFFFFFF);
				return retValue;
			}
			STATUS(Log::FLOW, Log::INFO, "%p = memalign(32, %d)", mc->lastOutputBytes, mc->lastOutputSize);
			mc->lastOutputBytesSize = mc->lastOutputSize;
		}
		memcpy(mc->lastOutputBytes, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);

#if defined(NEX_WRITE_OUTPUT_TO_DISK)
		char filename[1024];
		memset(filename, 0x00, 1024);
		int stringlength = snprintf(filename, 1023, "/sdcard/encdump-%lld_.dump", mc->lastOutputPTS);
		FILE * out = fopen(filename, "w");
		fwrite(((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize, 1, out);
		fclose(out);
		out = NULL;
		STATUS(Log::FLOW, Log::INFO, "wrote to %s", filename);
#endif

		if (1)
		{
			*ppOutData = mc->lastOutputBytes;
			*piOutLen = mc->lastOutputSize;
		}

		STATUS(Log::FLOW, Log::INFO, "ppOutData(%p)", *ppOutData);

		EXTERNAL_CALL(releaseOutputBuffer, Log::DEBUG, mc->mc->releaseOutputBuffer(mc->outIndex, false));

		mc->outIndex = -1;
	}
	else
	{
		retValue = -1;
		*ppOutData = NULL;
		*piOutLen = 0;
		*puPTS = 0;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) outLen(%u) ts(%u)", retValue, 0 != piOutLen ? *piOutLen : 0xFFFFFFFF, 0 != puPTS ? *puPTS : 0xFFFFFFFF);
	return retValue;
}

int Skip
		( unsigned char** ppOutData
		, unsigned int* piOutLen
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	unsigned int retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) outLen(%u)", retValue, 0 != piOutLen ? *piOutLen : 0xFFFFFFFF);
	return retValue;
}

int Reset( NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (!mc->sentValidInputFrame)
	{ // no need to reset
		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
		return retValue;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	while ((mc->inAtExternal || mc->outAtExternal) && !mc->exceptionState)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		usleep(1000); // 1 ms -- should only have to sleep up to MAX(INPUT_TIMEOUT,OUTPUT_TIMEOUT)

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");
	}

	mc->inIndex = -1;
	mc->outIndex = -1;

	EXTERNAL_CALL(flush, Log::DEBUG, mc->mc->flush());

	mc->eosSent = false;
	mc->eosReceived = false;

	mc->sentValidInputFrame = false;

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

}; // namespace Encoder
}; // namespace Video

namespace Audio {
#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_AUDIO

namespace Decoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

void SetDebugLevel( int types, int max_level )
{
	Log::SetDebugLevel(NEX_MC_TYPE, NEX_MC_AVTYPE, types, max_level);
}

int GetProperty( unsigned int uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%u) ud(%p)", uProperty, pUserData);

	int retValue = 0;

	unsigned int input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_CODEC_IMPLEMENT :
		*puValue = NEXCAL_PROPERTY_CODEC_IMPLEMENT_HW;
		break;

	case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
		*puValue = NEXCAL_PROPERTY_ANSWERIS_YES;
		break;

//	case NEXCAL_PROPERTY_SET_RIGHT_OUTPUT_INFORMATION :
//		*puValue = NEXCAL_PROPERTY_ANSWERIS_NO;
//		break;

	case NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE :
		*puValue = NEXCAL_PROPERTY_AUDIO_BUFFER_PCM;
		break;

	case NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_SUPPORTED:
		*puValue = NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_INTERNAL | NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_EXTERNAL;
		break;

	case NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR:
		//if (0 == uUserData)
		{
			*puValue = NEXCAL_PROPERTY_OUTPUTBUFFER_ALLOCATOR_INTERNAL;
 		}// TODO:
		break;

	case NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT:
		*puValue = NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT_NO;
		break;
		
	default:
		*puValue = 0;
		break;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}

int SetProperty( unsigned int uProperty, NXINT64 uValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%u) value(%lld) ud(%p)", uProperty, uValue, pUserData);

	int retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

namespace { // anon10
	bool outputFormatChangedChecker(NexCAL_mc *mc)
	{
		STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

		bool result = false;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
			; return false);

		if (NULL != outputFormat)
		{
			int newSampleRate = 0;

			EXTERNAL_CALL_WITH_ACTION(getInteger(sample-rate), Log::DEBUG
				, outputFormat->getInteger("sample-rate", newSampleRate)
				, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
				; return false);

			if (newSampleRate != mc->ad.sampleRate)
			{
				result = true;
			}
			else
			{
				int newChannelCount = 0;

				EXTERNAL_CALL_WITH_ACTION(getInteger(channel-count), Log::DEBUG
					, outputFormat->getInteger("channel-count", newChannelCount)
					, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
					; return false);

				if (newChannelCount != mc->ad.channelCount)
				{
					result = true;
				}
			}
		}

		STATUS(Log::FLOW, Log::DEBUG, "- result(%s)", result ? "true" : "false");
		return result;
	}
}; // namespace (anon10)

NexCAL_mc * CreateNexCAL_mc(const char *codecName, const char *mimeType, int sampleRate, int channelCount, int numSamplesPerChannel, unsigned int uUserDataType, NXVOID **ppUserData)
{
	STATUS(Log::FLOW, Log::INFO, "+");

	NexMediaFormat *inputFormat = NULL;
	STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->createAudioFormat");
	int ret = NexMediaFormat_using_jni::createAudioFormat(mimeType, sampleRate, channelCount, inputFormat);
	STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "createAudioFormat->");

	if (0 != ret)
	{
		MC_ERR("Exception occurred while creating audio format (%d)", ret);
		return NULL;
	}

	if (NULL == inputFormat)
	{
		MC_ERR("failed to create MediaFormat instance");
		return NULL;
	}

	NexCAL_mc *mc = (NexCAL_mc *)malloc(sizeof(NexCAL_mc));
	if (NULL == mc)
	{
		MC_ERR("malloc failed!");
		NexMediaFormat_using_jni::releaseNexMediaFormat(inputFormat);
		return NULL;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(NexCAL_mc));

	memset(mc, 0x00, sizeof(NexCAL_mc));

	mc->type = NEX_MC_TYPE;
	mc->avtype = NEX_MC_AVTYPE;
	mc->codecName = codecName;

	mc->mimeType = mimeType;

	mc->inIndex = -1;
	mc->outIndex = -1;

	// let's trust mw
	mc->ad.sampleRate = sampleRate;
	mc->ad.channelCount = channelCount;
	mc->ad.bitsPerSample = 16;
	mc->ad.numSamplesPerChannel = numSamplesPerChannel;

	mc->inputFormat = inputFormat;

	STATUS(Log::FLOW, Log::INFO, "- mc(%p)", mc);
	return mc;
}

//Nex_MC::Audio::Init
int Init( NexCAL_mc *mc, unsigned char *pFrame, int iFrameLen )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	int ret = 0;

	if (NULL == mc->inputFormat)
	{
		EXTERNAL_CALL_WITH_ACTION(createAudioFormat, Log::DEBUG
			, NexMediaFormat_using_jni::createAudioFormat(mc->mimeType, mc->ad.sampleRate, mc->ad.channelCount, mc->inputFormat)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -10)
			; return -10);

		if (NULL == mc->inputFormat)
		{
			MC_ERR("failed to create MediaFormat instance");
			STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -1);
			return -1;
		}
	}

	if (NULL == mc->outputFormatChanged)
	{
		mc->outputFormatChanged = outputFormatChangedChecker;
	}

	Common::MCConfigParams mccp;
	memset(&mccp, 0x00, sizeof(Common::MCConfigParams));
	mccp.surface = NULL;
	mccp.crypto = mc->ad.crypto;
	mccp.flags = 0;

	// initializes mutex vars and starts threads;
	// creates MediaCodec obj;
	// configures, starts, gets in/out buffers;
	int retValue = Nex_MC::Common::Decoder::Init(mc, &mccp);

	if (0 != retValue)
	{
		MC_ERR("Common::Decoder::Init failed");
		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
		return retValue;
	}

	if (0 < iFrameLen && NULL != pFrame)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		mc->outTimeout = 1000; // 1ms

		bool formatChanged = false;

		while (0 > mc->outIndex && !mc->exceptionState)
		{
			mc->engWaiting = true;

			// send first frame
			EXTERNAL_CALL(dequeueInputBuffer, Log::DEBUG, mc->mc->dequeueInputBuffer(1, mc->inIndex));

			if (0 <= mc->inIndex)
			{
				STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], pFrame, iFrameLen);
				memcpy(mc->inputBuffers[mc->inIndex], pFrame, iFrameLen);

				EXTERNAL_CALL(queueInputBuffer, Log::DEBUG, mc->mc->queueInputBuffer(mc->inIndex, 0, iFrameLen, 0LL, 1 /* BUFFER_FLAG_SYNC_FRAME */));
			}
			mc->inIndex = -1;

			if (-2 == mc->outIndex)
			{
				formatChanged = true;
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			sched_yield();
			STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
			pthread_mutex_lock(&mc->engLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

			mc->engWaiting = false;
		}

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		Nex_MC::Audio::Decoder::Reset((void*)mc);

		mc->outTimeout = mc->defaultOutTimeout;

		if (formatChanged || 0 == mc->ad.numSamplesPerChannel)
		{
			NexMediaFormat *outputFormat = NULL;

			EXTERNAL_CALL(getOutputFormat, Log::DEBUG, mc->mc->getOutputFormat(outputFormat));

			if (!mc->exceptionState)
			{
				EXTERNAL_CALL(getInteger(sample-rate), Log::DEBUG, outputFormat->getInteger("sample-rate", mc->ad.sampleRate));
			}

			if (!mc->exceptionState)
			{
				EXTERNAL_CALL(getInteger(channel-count), Log::DEBUG, outputFormat->getInteger("channel-count", mc->ad.channelCount));
			}

			if (!mc->exceptionState)
			{
				mc->ad.numSamplesPerChannel = (mc->lastOutputOffset + mc->lastOutputSize)/mc->ad.channelCount/(mc->ad.bitsPerSample>>3);
				STATUS(Log::FLOW, Log::DEBUG, "channelCount(%d) sampleRate(%d) numOfSamplesPerChannel(%d)", mc->ad.channelCount, mc->ad.sampleRate, mc->ad.numSamplesPerChannel);
			}
		}
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

//Nex_MC::Audio::Deinit
int Deinit( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	int retValue = Nex_MC::Common::Deinit(mc); // joins threads and destroys mutex vars

	if (NULL != mc->lastOutputBytes)
	{
		free(mc->lastOutputBytes);
		mc->lastOutputBytes = NULL;
		mc->lastOutputBytesSize = 0;
	}

	if (NULL != mc->mc)
	{
		if (NULL != mc->bufferInfo) // if this was made, start was probably called at some point
		{
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseBufferInfo");
			NexMediaCodec_using_jni::releaseBufferInfo(mc->bufferInfo);
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseBufferInfo->");
			mc->bufferInfo = NULL;

			//EXTERNAL_CALL(stop, Log::DEBUG, mc->mc->stop());

			EXTERNAL_CALL(release, Log::DEBUG, mc->mc->release());
		}

		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaCodec");
		NexMediaCodec_using_jni::releaseNexMediaCodec(mc->mc);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaCodec->");
		mc->mc = NULL;
	}

	if (NULL != mc->inputFormat)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaFormat");
		NexMediaFormat_using_jni::releaseNexMediaFormat(mc->inputFormat);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaFormat->");
		mc->inputFormat = NULL;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}


//Nex_MC::Audio::Decode
int Decode
		( unsigned char* pSource
		, int iLen
		, void* pDest
		, int* piWrittenPCMSize
		, unsigned int uDTS
		, unsigned int *puOutputTime
		, int nFlag
		, unsigned int* puDecodeResult
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ len(%d) ts(%u) flag(0x%X) ud(%p)", iLen, uDTS, nFlag, pUserData);

	int retValue = 0;

	bool eosDetected = IS_BINARY_FLAG_SET(nFlag, NEXCAL_ADEC_FLAG_END_OF_STREAM);

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (eosDetected && !mc->sentValidInputFrame)
	{
		retValue = 0;

		NEXCAL_INIT_ADEC_RET(*puDecodeResult);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME);
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_EOS);

		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
		return retValue;
	}

	NEXCAL_INIT_ADEC_RET(*puDecodeResult);

	int maxWritablePCMSize = *piWrittenPCMSize;
	*piWrittenPCMSize = 0;

#if 1 == _NEX_INPUT_DUMP_
	if (NULL != mc->inFile)
	{
		if (0 < iLen && NULL != pSource)
		{
			fwrite(&iLen, sizeof(int), 1, mc->inFile);
			fwrite(pSource, sizeof(unsigned char), (size_t)iLen, mc->inFile);
		}
	}
#endif

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	bool formatChanged = false;

	while (0 > mc->inIndex && 0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && 3000 > GetTickCount() - ticks && !mc->exceptionState)
	{ // no input buffer to fill && no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (mc->inWaiting && !mc->eosSent)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "inLock");
			pthread_mutex_lock(&mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "inLock");

			mc->inWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "in");
			pthread_cond_broadcast(&mc->inCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "inLock");
			pthread_mutex_unlock(&mc->inLock);
		}

		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		// there's a reason this is done after waking both threads
		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	if (0 > mc->inIndex)
	{
		retValue = -1;
	}
	else
	{
		int flags = 0;

		if (eosDetected)
		{
			STATUS(Log::FLOW, Log::DEBUG, "EOS detected");

			flags |= 4 /* BUFFER_FLAG_END_OF_STREAM */;
			mc->eosSent = true;
		}
		else
		{
			mc->sentValidInputFrame = true;
		}

		if (NULL != pSource && 0 < iLen)
		{
			STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], pSource, iLen);
			memcpy(mc->inputBuffers[mc->inIndex], pSource, iLen);
		}

		EXTERNAL_CALL_WITH_BOTH_ACTIONS(queueInputBuffer, Log::DEBUG
			, mc->mc->queueInputBuffer(mc->inIndex, 0, iLen, (int64_t)(uDTS * 1000LL), flags)
			, retValue = -1
			, retValue = 0
			; mc->ad.maxPTS = MAX((int64_t)(uDTS * 1000LL), mc->ad.maxPTS)
			; NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS)
			; NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME););

		mc->inIndex = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (mc->eosSent && !mc->eosReceived)
	{
		retValue = -1; // we err unless we get output

		while (0 > mc->outIndex && !mc->eosReceived && 3000 > GetTickCount() - ticks && !mc->exceptionState)
		{ // no output buffer received && not timed out yet
			mc->engWaiting = true;

			if (-2 == mc->outIndex)
			{
				formatChanged = true;
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

			mc->engWaiting = false;
		}
	}

	if (formatChanged)
	{
		retValue = 0;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, retValue = -1);

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(sample-rate), Log::DEBUG
				, outputFormat->getInteger("sample-rate", mc->ad.sampleRate)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(channel-count), Log::DEBUG
				, outputFormat->getInteger("channel-count", mc->ad.channelCount)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			STATUS(Log::FLOW, Log::DEBUG, "channelCount(%d) sampleRate(%d)", mc->ad.channelCount, mc->ad.sampleRate);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_CHANGED);
		}
	}

	while (0 <= mc->outIndex && !mc->exceptionState)
	{ // there's output to return
		retValue = 0;
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);

		if (0 != (mc->lastOutputOffset + mc->lastOutputSize))
		{
			int newNumSamplesPerChannel = (mc->lastOutputOffset + mc->lastOutputSize)/mc->ad.channelCount/(mc->ad.bitsPerSample>>3);
			if (newNumSamplesPerChannel != mc->ad.numSamplesPerChannel)
			{
				mc->ad.numSamplesPerChannel = newNumSamplesPerChannel;
				// b109T
				//NEXCAL_ADEC_SET_RET_NEXT_FRAME(*puDecodeResult, NEXCAL_ADEC_RET_NEXT_FRAME_EXIST);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_CHANGED);
				STATUS(Log::FLOW, Log::DEBUG, "numOfSamplesPerChannel(%d)", mc->ad.numSamplesPerChannel);
				break;
			}
		}

		if (mc->lastOutputSize <= (maxWritablePCMSize - (*piWrittenPCMSize)))
		{
			if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
			{
				mc->eosReceived = true;
			}
			else if (mc->eosSent && mc->lastOutputPTS == mc->ad.maxPTS)
			{
				STATUS(Log::FLOW, Log::INFO, "last frame received");
				mc->eosReceived = true;
			}

			STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

			memcpy(((unsigned char *)pDest) + (*piWrittenPCMSize), ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);
			*piWrittenPCMSize += mc->lastOutputSize;

#if 1 == _NEX_OUTPUT_DUMP_
			if (NULL != mc->outFile)
			{
				fwrite(&mc->lastOutputSize, sizeof(int), 1, mc->outFile);
				fwrite(((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, sizeof(unsigned char), (size_t)mc->lastOutputSize, mc->outFile);
			}
#elif 2 == _NEX_OUTPUT_DUMP_
			if (NULL != mc->outFile)
			{
				fwrite(((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, sizeof(unsigned char), (size_t)mc->lastOutputSize, mc->outFile);
			}
#endif

			EXTERNAL_CALL_WITH_ACTION(releaseOutputBuffer, Log::DEBUG
				, mc->mc->releaseOutputBuffer(mc->outIndex, false)
				, retValue = -1);

			if (mc->eosReceived)
			{
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_EOS);
				mc->outIndex = -1;
				break;
			}
			else
			{
				STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
				pthread_mutex_unlock(&mc->engLock);
				mc->outIndex = Nex_MC::Common::DequeueOutput(mc, 0);
				STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
				pthread_mutex_lock(&mc->engLock);
				STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");
			}
		}
		else
		{
			break;
		}
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	if (0 != piWrittenPCMSize && 0 != (*piWrittenPCMSize))
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) writtenPCMSize(%d) decRet(0x%X)", retValue, 0 != piWrittenPCMSize ? *piWrittenPCMSize : 0xFFFFFFFF, *puDecodeResult);
	return retValue;
}

int DecodePR
		( unsigned char* pSource
		, int iLen
		, void* pDest
		, int* piWrittenPCMSize
		, unsigned int uDTS
		, unsigned int *puOutputTime
		, int nFlag
		, unsigned int* puDecodeResult
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ len(%d) ts(%u) flag(0x%X) ud(%p)", iLen, uDTS, nFlag, pUserData);

	int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	NEXCAL_INIT_ADEC_RET(*puDecodeResult);

	int maxWritablePCMSize = *piWrittenPCMSize;
	*piWrittenPCMSize = 0;

#if 1 == _NEX_INPUT_DUMP_
	if (NULL != mc->inFile)
	{
		if (0 < iLen && NULL != pSource)
		{
			fwrite(&iLen, sizeof(int), 1, mc->inFile);
			fwrite(pSource, sizeof(unsigned char), (size_t)iLen, mc->inFile);
		}
	}
#endif

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	bool formatChanged = false;

	while (0 > mc->inIndex && 0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && 3000 > GetTickCount() - ticks && !mc->exceptionState)
	{ // no input buffer to fill && no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (mc->inWaiting && !mc->eosSent)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "inLock");
			pthread_mutex_lock(&mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "inLock");

			mc->inWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "in");
			pthread_cond_broadcast(&mc->inCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "inLock");
			pthread_mutex_unlock(&mc->inLock);
		}

		if (-2 == mc->outIndex)
		{
			formatChanged = true;
			mc->outIndex = -1;
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		// there's a reason this is done after waking both threads
		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	if (0 > mc->inIndex)
	{
		retValue = -1;
	}
	else
	{
		int flags = 0;

		if (IS_BINARY_FLAG_SET(nFlag, NEXCAL_ADEC_FLAG_END_OF_STREAM))
		{
			STATUS(Log::FLOW, Log::DEBUG, "EOS detected");

			flags |= 4 /* BUFFER_FLAG_END_OF_STREAM */;
			mc->eosSent = true;
		}

		if (NULL != pSource)
		{
			Nex_MC::Common::MediaCodecCryptoInfo *mcci = (Nex_MC::Common::MediaCodecCryptoInfo *)pSource;

			EXTERNAL_CALL(setCryptoInfo, Log::DEBUG, Nex_MC::NexMediaCodec_using_jni::setCryptoInfo(mc->ad.cryptoInfo, mcci->numSubSamples, mcci->numBytesOfClearData, mcci->numBytesOfEncryptedData, mcci->key, mcci->iv, mcci->mode));

			if (NULL != mcci->pData && 0 < mcci->iLen)
			{
				STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], mcci->pData, mcci->iLen);
				memcpy(mc->inputBuffers[mc->inIndex], mcci->pData, mcci->iLen);
			}
		}

		EXTERNAL_CALL_WITH_BOTH_ACTIONS(queueSecureInputBuffer, Log::DEBUG
			, mc->mc->queueSecureInputBuffer(mc->inIndex, 0, mc->ad.cryptoInfo, (int64_t)(uDTS * 1000LL), flags)
			, retValue = -1
			, retValue = 0
			; mc->ad.maxPTS = MAX((int64_t)(uDTS * 1000LL), mc->ad.maxPTS)
			; NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS)
			; NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_NEXT_FRAME));

		mc->inIndex = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (mc->eosSent && !mc->eosReceived)
	{
		retValue = -1; // we err unless we get output

		while (0 > mc->outIndex && !mc->eosReceived && 3000 > GetTickCount() - ticks && !mc->exceptionState)
		{ // no output buffer received && not timed out yet
			mc->engWaiting = true;

			if (-2 == mc->outIndex)
			{
				formatChanged = true;
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

			mc->engWaiting = false;
		}
	}

	if (formatChanged)
	{
		retValue = 0;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, retValue = -1);

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(sample-rate), Log::DEBUG
				, outputFormat->getInteger("sample-rate", mc->ad.sampleRate)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			EXTERNAL_CALL_WITH_ACTION(getInteger(channel-count), Log::DEBUG
				, outputFormat->getInteger("channel-count", mc->ad.channelCount)
				, retValue = -1);
		}

		if (!mc->exceptionState)
		{
			STATUS(Log::FLOW, Log::DEBUG, "channelCount(%d) sampleRate(%d)", mc->ad.channelCount, mc->ad.sampleRate);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);
			NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_CHANGED);            
		}
	}

	while (0 <= mc->outIndex && !mc->exceptionState)
	{ // there's output to return
		retValue = 0;
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_DECODING_SUCCESS);

		if (0 != (mc->lastOutputOffset + mc->lastOutputSize))
		{
			int newNumSamplesPerChannel = (mc->lastOutputOffset + mc->lastOutputSize)/mc->ad.channelCount/(mc->ad.bitsPerSample>>3);
			if (newNumSamplesPerChannel != mc->ad.numSamplesPerChannel)
			{
				mc->ad.numSamplesPerChannel = newNumSamplesPerChannel;
				// b109T
				//NEXCAL_ADEC_SET_RET_NEXT_FRAME(*puDecodeResult, NEXCAL_ADEC_RET_NEXT_FRAME_EXIST);
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_CHANGED);    
				STATUS(Log::FLOW, Log::DEBUG, "numOfSamplesPerChannel(%d)", mc->ad.numSamplesPerChannel);
				break;
			}
		}

		if (mc->lastOutputSize <= (maxWritablePCMSize - (*piWrittenPCMSize)))
		{
			if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
			{
				mc->eosReceived = true;
			}
			else if (mc->eosSent && mc->lastOutputPTS == mc->ad.maxPTS)
			{
				STATUS(Log::FLOW, Log::INFO, "last frame received");
				mc->eosReceived = true;
			}

			STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

			memcpy(((unsigned char *)pDest) + (*piWrittenPCMSize), ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);
			*piWrittenPCMSize += mc->lastOutputSize;

#if 1 == _NEX_OUTPUT_DUMP_
			if (NULL != mc->outFile)
			{
				fwrite(&mc->lastOutputSize, sizeof(int), 1, mc->outFile);
				fwrite(((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, sizeof(unsigned char), (size_t)mc->lastOutputSize, mc->outFile);
			}
#elif 2 == _NEX_OUTPUT_DUMP_
			if (NULL != mc->outFile)
			{
				fwrite(((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, sizeof(unsigned char), (size_t)mc->lastOutputSize, mc->outFile);
			}
#endif
			EXTERNAL_CALL_WITH_ACTION(releaseOutputBuffer, Log::DEBUG
				, mc->mc->releaseOutputBuffer(mc->outIndex, false)
				, retValue = -1);

			if (mc->eosReceived)
			{
				NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_CHANGED);    
				mc->outIndex = -1;
				break;
			}
			else
			{
				mc->outIndex = Nex_MC::Common::DequeueOutput(mc, 0);
			}
		}
		else
		{
			break;
		}
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	if (0 != piWrittenPCMSize && 0 != (*piWrittenPCMSize))
	{
		NEXCAL_SET_ADEC_RET(*puDecodeResult, NEXCAL_ADEC_OUTPUT_EXIST);    
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) writtenPCMSize(%d) decRet(0x%X)", retValue, 0 != piWrittenPCMSize ? *piWrittenPCMSize : 0xFFFFFFFF, *puDecodeResult);
	return retValue;
}

int Reset( NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (!mc->sentValidInputFrame)
	{ // no need to reset
		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
		return retValue;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	while ((mc->inAtExternal || mc->outAtExternal) && !mc->exceptionState)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		usleep(1000); // 1 ms -- should only have to sleep up to MAX(INPUT_TIMEOUT,OUTPUT_TIMEOUT)

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");
	}

	mc->inIndex = -1;
	mc->outIndex = -1;

	EXTERNAL_CALL_WITH_ACTION(flush, Log::DEBUG
		, mc->mc->flush()
		, retValue = -1);

	mc->eosSent = false;
	mc->eosReceived = false;

	mc->sentValidInputFrame = false;

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int GetInfo( unsigned int uIndex, unsigned int* puResult, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ idx(0x%X) ud(%p)", uIndex, pUserData);

	int retValue = NEXCAL_ERROR_NONE;

	if (0 == pUserData)
	{
		MC_ERR("uUserData is zero");
		retValue = NEXCAL_ERROR_INVALID_PARAMETER;
	}
	else
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		switch (uIndex)
		{
		case NEXCAL_AUDIO_GETINFO_SAMPLINGRATE :
			*puResult = mc->ad.sampleRate;
			break;
		case NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS :
			*puResult = mc->ad.channelCount;
			break;
		case NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE :
			*puResult = mc->ad.bitsPerSample;
			break;
		case NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL :
			*puResult = mc->ad.numSamplesPerChannel;
			break;
		default :
			MC_ERR("Invalid index(0x%X)", uIndex);
			retValue = NEXCAL_ERROR_INVALID_PARAMETER;
			break;
		}
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) result(0x%X)", retValue, *puResult);
	return retValue;
}

}; // namespace Decoder

namespace Encoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

void SetDebugLevel( int types, int max_level )
{
	Log::SetDebugLevel(NEX_MC_TYPE, NEX_MC_AVTYPE, types, max_level);
}

int GetProperty( unsigned int uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%u) ud(%p)", uProperty, pUserData);

	unsigned int retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}

int SetProperty( unsigned int uProperty, NXINT64 uValue, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ prop(%u) value(%lld) ud(%p)", uProperty, uValue, pUserData);

	unsigned int retValue = 0;

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

namespace { // anon11
	bool outputFormatChangedChecker(NexCAL_mc *mc)
	{
		STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p)", mc);

		bool result = false;

		NexMediaFormat *outputFormat = NULL;

		EXTERNAL_CALL_WITH_ACTION(getOutputFormat, Log::DEBUG
			, mc->mc->getOutputFormat(outputFormat)
			, STATUS(Log::FLOW, Log::DEBUG, "- result(false)")
			; return false);

		if (NULL != outputFormat)
		{
		}

		STATUS(Log::FLOW, Log::DEBUG, "- result(%s)", result ? "true" : "false");
		return result;
	}
}; // namespace (anon11)

int Init( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	int ret = 0;

	if (NULL == mc->inputFormat)
	{
		EXTERNAL_CALL_WITH_ACTION(createAudioFormat, Log::DEBUG
			, NexMediaFormat_using_jni::createAudioFormat(mc->mimeType, mc->ae.sampleRate, mc->ae.channelCount, mc->inputFormat)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -10)
			; return -10);

		if (NULL == mc->inputFormat)
		{
			MC_ERR("failed to create MediaFormat instance");
			return -1;
		}

		EXTERNAL_CALL_WITH_ACTION(setInteger(bitrate), Log::DEBUG
			, mc->inputFormat->setInteger("bitrate", mc->ae.bitRate)
			, STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", -11)
			; return -11);
	}

	if (NULL == mc->outputFormatChanged)
	{
		mc->outputFormatChanged = outputFormatChangedChecker;
	}

	Common::MCConfigParams mccp;
	memset(&mccp, 0x00, sizeof(Common::MCConfigParams));
	mccp.surface = NULL;
	mccp.crypto = NULL;
	mccp.flags = 1 /* CONFIGURE_FLAG_ENCODE */;

	// initializes mutex vars and starts threads;
	// creates MediaCodec obj;
	// configures, starts, gets in/out buffers;
	int retValue = Nex_MC::Common::Encoder::Init(mc, &mccp);

	if (0 != retValue)
	{
		MC_ERR("Common::Encoder::Init failed");
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int Deinit( NexCAL_mc *mc )
{
	STATUS(Log::FLOW, Log::INFO, "+ mc(%p)", mc);

	int retValue = Nex_MC::Common::Deinit(mc); // joins threads and destroys mutex vars


	if (NULL != mc->lastOutputBytes)
	{
		free(mc->lastOutputBytes);
		mc->lastOutputBytes = NULL;
		mc->lastOutputBytesSize = 0;
	}

	if (NULL != mc->mc)
	{
		if (NULL != mc->bufferInfo) // if this was made, start was probably called at some point
		{
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseBufferInfo");
			NexMediaCodec_using_jni::releaseBufferInfo(mc->bufferInfo);
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseBufferInfo->");
			mc->bufferInfo = NULL;

			//EXTERNAL_CALL(stop, Log::DEBUG, mc->mc->stop());

			EXTERNAL_CALL(release, Log::DEBUG, mc->mc->release());
		}

		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaCodec");
		NexMediaCodec_using_jni::releaseNexMediaCodec(mc->mc);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaCodec->");
		mc->mc = NULL;
	}

	if (NULL != mc->inputFormat)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseNexMediaFormat");
		NexMediaFormat_using_jni::releaseNexMediaFormat(mc->inputFormat);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseNexMediaFormat->");
		mc->inputFormat = NULL;
	}

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

int WriteOutputFrame(NexCAL_mc *mc, unsigned char **ppDest)
{
	STATUS(Log::FLOW, Log::DEBUG, "+ mc(%p) ppDest(%p)", mc, ppDest);
	int ret = 0;

    if (0 < mc->ae.dummyCount)
    {
        --mc->ae.dummyCount;
        STATUS(Log::FLOW, Log::DEBUG, "- ret(%d)", ret);
        return ret;
    }

	if (NULL != mc->lastOutputBytes && mc->lastOutputSize > mc->lastOutputBytesSize)
	{
		free(mc->lastOutputBytes);
		mc->lastOutputBytes = NULL;
		mc->lastOutputBytesSize = 0;
	}
	if (NULL == mc->lastOutputBytes)
	{
		unsigned int bytesToAlloc = mc->lastOutputSize - 1;
		bytesToAlloc |= bytesToAlloc >> 1;
		bytesToAlloc |= bytesToAlloc >> 2;
		bytesToAlloc |= bytesToAlloc >> 4;
		bytesToAlloc |= bytesToAlloc >> 8;
		bytesToAlloc |= bytesToAlloc >> 16;
		++bytesToAlloc;
		mc->lastOutputBytes = (unsigned char *)memalign(32, bytesToAlloc);
		if (NULL == mc->lastOutputBytes)
		{
			MC_ERR("memalign failed!");
			STATUS(Log::FLOW, Log::DEBUG, "- ret(%d)", 0);
			return 0;
		}
		STATUS(Log::FLOW, Log::INFO, "%p = memalign(32, %d)", mc->lastOutputBytes, bytesToAlloc);
		mc->lastOutputBytesSize = bytesToAlloc;
	}
	memcpy(mc->lastOutputBytes, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);
	*ppDest = mc->lastOutputBytes;
	ret = mc->lastOutputSize;

	STATUS(Log::FLOW, Log::DEBUG, "- ret(%d)", ret);
	return ret;
}

int Encode
		( unsigned char* pData
		, int iLen
		, unsigned char** ppOutData
		, unsigned int* piOutLen
		, unsigned int* puEncodeResult
		, NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ len(%d) ud(%p)", iLen, pUserData);

	int retValue = 0;

	bool eosDetected = 0 == iLen || NULL == pData;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (eosDetected && !mc->sentValidInputFrame)
	{
		retValue = 0;
		NEXCAL_INIT_AENC_RET(*puEncodeResult);
		NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_ENCODING_SUCCESS);
		NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_NEXT_INPUT);
		NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_EOS);

		*piOutLen = 0;

		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) piOutLen(%d) encRet(0x%X)", retValue, NULL != piOutLen ? *piOutLen : 0xFFFFFFFF, *puEncodeResult);
		return retValue;
	}

	NEXCAL_INIT_AENC_RET(*puEncodeResult);

	*piOutLen = 0;

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	unsigned int ticks = GetTickCount();

	while (0 > mc->inIndex && 0 > mc->outIndex && (!mc->eosSent || !mc->eosReceived) && 3000 > GetTickCount() - ticks && !mc->exceptionState)
	{ // no input buffer to fill && no output buffer received && not timed out yet
		mc->engWaiting = true;

		if (mc->inWaiting && !mc->eosSent)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "inLock");
			pthread_mutex_lock(&mc->inLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "inLock");

			mc->inWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "in");
			pthread_cond_broadcast(&mc->inCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "inLock");
			pthread_mutex_unlock(&mc->inLock);
		}

		if (mc->outWaiting && !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
			pthread_mutex_lock(&mc->outLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

			mc->outWaiting = false;

			STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
			pthread_cond_broadcast(&mc->outCond);

			STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
			pthread_mutex_unlock(&mc->outLock);
		}

		// there's a reason this is done after waking both threads
		if (!mc->eosSent || !mc->eosReceived)
		{
			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");
		}

		mc->engWaiting = false;
	}

	if (0 > mc->inIndex)
	{
		retValue = -1;
	}
	else
	{
		int flags = 0;

		if (eosDetected)
		{
			STATUS(Log::FLOW, Log::DEBUG, "EOS detected");

			flags |= 4 /* BUFFER_FLAG_END_OF_STREAM */;
			mc->eosSent = true;
		}
		else
		{
			mc->sentValidInputFrame = true;
			STATUS(Log::FLOW, Log::DEBUG, "memcpy([%ld](%p), %p, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], pData, iLen);
			memcpy(mc->inputBuffers[mc->inIndex], pData, iLen);
		}

#if 1 == _NEX_INPUT_DUMP_
    if (NULL != mc->inFile)
    {
        if (0 < iLen && NULL != pData)
        {
            fwrite(&iLen, sizeof(int), 1, mc->inFile);
            fwrite(pData, sizeof(unsigned char), (size_t)iLen, mc->inFile);
        }
    }
#elif 2 == _NEX_INPUT_DUMP_
    if (NULL != mc->inFile)
    {
        if (0 < iLen && NULL != pData)
        {
            fwrite(pData, sizeof(unsigned char), (size_t)iLen, mc->inFile);
        }
    }
#endif

		EXTERNAL_CALL_WITH_BOTH_ACTIONS(queueInputBuffer, Log::DEBUG
			, mc->mc->queueInputBuffer(mc->inIndex, 0, iLen, 0LL, flags)
			, retValue = -1
			, retValue = 0
			; NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_ENCODING_SUCCESS)
			; NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_NEXT_INPUT));

		mc->inIndex = -1;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	sched_yield();

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	if (mc->eosSent && !mc->eosReceived)
	{
		retValue = -1; // we err unless we get output

		while (0 > mc->outIndex && !mc->eosReceived && 3000 > GetTickCount() - ticks && !mc->exceptionState)
		{ // no output buffer received && not timed out yet
			mc->engWaiting = true;

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::VERBOSE, ">>(%sCond, engLock)", "eng");
			pthread_cond_wait(&mc->engCond, &mc->engLock);
			STATUS(Log::MUTEX, Log::VERBOSE, "(%sCond, engLock)>>", "eng");

			mc->engWaiting = false;
		}
	}

	if (0 <= mc->outIndex)
	{ // there's output to return
		STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);
		NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_ENCODING_SUCCESS);

		if (0 < mc->lastOutputSize)
		{ // really... there really is output to return
            retValue = 0;
			int written = mc->ae.writeOutputFrame(mc, ppOutData);
			if (0 < written)
			{
				NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_OUTPUT_EXIST);

				*piOutLen += written;
			}
		}

		if (IS_BINARY_FLAG_SET(mc->lastOutputFlags, 4 /* BUFFER_INFO_END_OF_STREAM */))
		{
			mc->eosReceived = true;
			retValue = 0;
		}

		EXTERNAL_CALL_WITH_ACTION(releaseOutputBuffer, Log::DEBUG
			, mc->mc->releaseOutputBuffer(mc->outIndex, false)
			, retValue = -1);

		mc->outIndex = -1;

		if (mc->eosReceived)
		{
			NEXCAL_SET_AENC_RET(*puEncodeResult, NEXCAL_AENC_EOS);
		}
	}

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

#if 1 == _NEX_OUTPUT_DUMP_
	if (NULL != mc->outFile)
	{
		if (NULL != piOutLen && 0 < *piOutLen)
		{
			fwrite(piOutLen, sizeof(int), 1, mc->outFile);
			fwrite((unsigned char *)*ppOutData, sizeof(unsigned char), (size_t)*piOutLen, mc->outFile);
		}
	}
#elif 2 == _NEX_OUTPUT_DUMP_
	if (NULL != mc->outFile)
	{
		if (NULL != piOutLen && 0 < *piOutLen)
		{
			fwrite((unsigned char *)*ppOutData, sizeof(unsigned char), (size_t)*piOutLen, mc->outFile);
		}
	}
#endif

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X) piOutLen(%d) encRet(0x%X)", retValue, NULL != piOutLen ? *piOutLen : 0xFFFFFFFF, *puEncodeResult);
	return retValue;
}

int Reset( NXVOID *pUserData )
{
	STATUS(Log::FLOW, Log::INFO, "+ ud(%p)", pUserData);

	int retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	if (!mc->sentValidInputFrame)
	{ // no need to reset
		STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
		return retValue;
	}

	STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
	pthread_mutex_lock(&mc->engLock);
	STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

	while ((mc->inAtExternal || mc->outAtExternal) && !mc->exceptionState)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		usleep(1000); // 1 ms -- should only have to sleep up to MAX(INPUT_TIMEOUT,OUTPUT_TIMEOUT)

		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");
	}

	mc->inIndex = -1;
	mc->outIndex = -1;

	EXTERNAL_CALL_WITH_ACTION(flush, Log::DEBUG
		, mc->mc->flush()
		, retValue = -1);

	mc->eosSent = false;
	mc->eosReceived = false;

	mc->sentValidInputFrame = false;

	STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
	pthread_mutex_unlock(&mc->engLock);

	STATUS(Log::FLOW, Log::INFO, "- ret(0x%X)", retValue);
	return retValue;
}

}; // namespace Encoder
}; // namespace Audio

}; // namespace Nex_MC

namespace { // anon1

unsigned int reserved_1(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_2(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_3(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_4(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_5(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_6(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_7(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_8(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_9(  void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_10( void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_11( void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_12( void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }
unsigned int reserved_13( void * res1, void * res2, void * res3, void * res4, void * res5
						, void * res6, void * res7, void * res8, void * res9, void * res10)
{ return 0; }

NEXCALCodec2 NexCAL_MC_VideoDecoder_h263 = { // "video/3gpp"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_h264 = { // "video/avc"
    { (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
    , (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
    , (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_h264_sw = { // "video/avc"
    { (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
    , (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
    , (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_h264_pr = { // "video/avc"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_hevc = { // "video/hevc"
    { (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
    , (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
    , (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_mpeg4v = { // "video/mp4v-es"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_mpeg2 = { // "video/mpeg2"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_VideoDecoder_vpx = { // "video/x-vnd.on2.vp8"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

void ReleaseOutputBuffer( void *pBuffer, bool render, NXVOID *pUserData )
{
	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "+ ud(%p)", pUserData);

	if (NULL != pUserData)
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		if (NULL == mc->mc)
		{
			MC_WARN("media codec already released!");
			STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "-");
			return;
		}

		if (NEX_MC_TYPE_DECODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
		{
			mc->vd.releaseOutputBuffer(mc, (long)pBuffer, render);
		}
		else
		{
			MC_ERR("invalid mc type");
		}
	}
	else
	{
		MC_ERR("uUserData is NULL");
	}

	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "-");
}

void CaptureFrame( bool thumbnailMode, NXVOID *pUserData )
{
	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "+ ud(%p)", pUserData);

	if ((unsigned int)NULL != pUserData)
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		if (NEX_MC_TYPE_DECODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
		{
			if (NULL != Nex_MC::JNI::JMETHODS::MediaCodec.captureFrame)
			{
				// assume no exceptions can occur.... not sure though.
				EXTERNAL_CALL(captureFrame, Nex_MC::Log::DEBUG, mc->mc->captureFrame(thumbnailMode));
			}
			else
			{
				MC_ERR("java method \"captureFrame\" was never found");
			}
		}
		else
		{
			MC_ERR("invalid mc type");
		}
	}
	else
	{
		MC_ERR("uUserData is NULL");
	}

	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "-");
}

NEXCALCodec2 NexCAL_MC_VideoDecoder_utility_functions = {
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_VideoEncoder_h264 = { // "video/avc"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_VideoEncoder_h264_using_frame_data = { // "video/avc"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_VideoEncoder_hevc = { // "video/hevc"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_VideoEncoder_mpeg4v = { // "video/mpeg4v"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

jobject GetInputSurface( NXVOID *pUserData )
{
	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "+ ud(%p)", pUserData);
	jobject surface = NULL;

	if (NULL != pUserData)
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		if (NEX_MC_TYPE_ENCODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
		{
			if (NULL != mc->ve.surface)
			{
				// exception cannot occur for the following function (as of yet)
				mc->ve.surface->getSurfaceObject(surface);
			}
			else
			{
				STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "no input surface available");
			}
		}
		else
		{
			MC_ERR("invalid mc type");
		}
	}
	else
	{
		MC_ERR("uUserData is NULL");
	}

	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "- surface(%p)", surface);
	return surface;
}

ANativeWindow * GetInputANativeWindow( NXVOID *pUserData )
{
	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "+ ud(%p)", pUserData);
	ANativeWindow *nw = NULL;

	if (NULL != pUserData)
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		if (NEX_MC_TYPE_ENCODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
		{
			if (NULL != mc->ve.surface)
			{
				// exception cannot occur for the following function (as of yet)
				mc->ve.surface->getANativeWindowFromSurfaceObject(nw);
			}
			else
			{
				STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "no input surface available");
			}
		}
		else
		{
			MC_ERR("invalid mc type");
		}
	}
	else
	{
		MC_ERR("uUserData is NULL");
	}

	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "- nativeWindow(%p)", nw);
	return nw;
}

void SetBuffersTimestamp( unsigned int uPTS, NXVOID *pUserData )
{
	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "+ ud(%p)", pUserData);
	ANativeWindow *nw = NULL;

	if (NULL != pUserData)
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		if (NEX_MC_TYPE_ENCODER == mc->type && NEX_MC_AVTYPE_VIDEO == mc->avtype)
		{
			if (NULL != mc->ve.surface)
			{
				// exception cannot occur for the following function (as of yet)
				mc->ve.surface->getANativeWindowFromSurfaceObject(nw);

				if (NULL != nw)
				{
					//native_window_set_buffers_timestamp(nw, (int64_t)(uPTS * 1000000LL));
					// equal to:
					nw->perform(nw, 7 /* NATIVE_WINDOW_SET_BUFFERS_TIMESTAMP */, (int64_t)(uPTS * 1000000LL));
				}
				else
				{
					MC_ERR("native window is NULL");
				}
			}
			else
			{
				MC_ERR("no input surface available");
			}
		}
		else
		{
			MC_ERR("invalid mc type");
		}
	}
	else
	{
		MC_ERR("uUserData is NULL");
	}

	STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "-");
}

int QueryForSupport(NEX_CODEC_TYPE eCodecType , unsigned char *pConfig, int nConfigLen)
{
    STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::INFO, "+ oti(0x%X)", eCodecType);

    int ret = NEXCAL_ERROR_NONE;

    switch ((unsigned int)eCodecType)
    {
        case eNEX_CODEC_V_H263:
            break;
        case eNEX_CODEC_V_H264:
//      case NEXOTI_H264_PR:
            ret = Nex_MC::H264::Decoder::QueryForSupport(pConfig, (size_t)nConfigLen);
            break;
        case eNEX_CODEC_V_H264_MC_S:
            ret = Nex_MC::H264::Decoder::QueryForSupportSW(pConfig, (size_t)nConfigLen);
            break;
        case eNEX_CODEC_V_HEVC:
            ret = Nex_MC::HEVC::Decoder::QueryForSupport(pConfig, (size_t)nConfigLen);
            break;
        case eNEX_CODEC_V_MPEG4V:
        case eNEX_CODEC_V_DIVX:
            break;
        case eNEX_CODEC_V_MPEG2V:
            break;
        case eNEX_CODEC_V_VP6:   
        case eNEX_CODEC_V_VP6A: 
        case eNEX_CODEC_V_VP6F: 
        case eNEX_CODEC_V_VP7:  
        case eNEX_CODEC_V_VP8:  
             break;
        case eNEX_CODEC_A_AAC:
        case eNEX_CODEC_A_AACPLUS:
//      case NEXOTI_MPEG2AAC:
            break;
//      case NEXOTI_AAC_PR:
//      case NEXOTI_AAC_PLUS_PR:
//      case NEXOTI_MPEG2AAC_PR:
            break;
        case eNEX_CODEC_A_MP2:
        case eNEX_CODEC_A_MP3:
        case eNEX_CODEC_A_MP4A:
            break;
//      case NEXOTI_MP3_PR:
//      case NEXOTI_MP3inMP4_PR:
            break;
        case eNEX_CODEC_A_AMR:
            break;
        case eNEX_CODEC_A_AMRWB:
            break;
        case eNEX_CODEC_A_VORBIS:
            break;
        default:
            break;
    }

    STATUS(Nex_MC::Log::FLOW, Nex_MC::Log::DEBUG, "- ret(0x%X)", ret);
    return ret;
}

NEXCALCodec2 NexCAL_MC_VideoEncoder_utility_functions = {
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_VideoEncoder2_utility_functions = {
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_VideoEncoder3_utility_functions = {
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

NEXCALCodec2 NexCAL_MC_AudioDecoder_amrnb = { // "audio/3gpp"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_amrwb = { // "audio/amr-wb"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_mp3 = { // "audio/mpeg"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_mp3_pr = { // "audio/mpeg"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_aac = { // "audio/mp4a-latm"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_aac_pr = { // "audio/mp4a-latm"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_ogg = { // "audio/vorbis"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_alaw = { // "audio/g711-alaw"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioDecoder_mlaw = { // "audio/g711-mlaw"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioEncoder_aac = { // "audio/mp4a-latm"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioEncoder_mp3 = { // "audio/mpeg"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioEncoder_amrnb = { // "audio/3gpp"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};
NEXCALCodec2 NexCAL_MC_AudioEncoder_amrwb = { // "audio/amr-wb"
	{ (void *)reserved_1, (void *)reserved_2, (void *)reserved_3, (void *)reserved_4, (void *)reserved_5
	, (void *)reserved_6, (void *)reserved_7, (void *)reserved_8, (void *)reserved_9, (void *)reserved_10
	, (void *)reserved_11, (void *)reserved_12, (void *)reserved_13 }
};

class InitNexCALCodecs
{
private:
	inline void Scrambler(NEXCALCodec2 *pRet, int numScrambles)
	{
		size_t curIdx = 0;

		for (int i = 0; i < numScrambles; ++i)
		{
			void *tmpFunc = pRet->func[curIdx];

			size_t shufKey = ((size_t)tmpFunc) >> 2;

			size_t toIdx = (curIdx + (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

			//LOGE("i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

			if (toIdx != curIdx)
			{
				if (toIdx < curIdx)
				{
					toIdx += 13;
				}
				for (size_t j = curIdx; j < toIdx; ++j)
				{
					pRet->func[j%13] = pRet->func[(j+1)%13];
				}
				pRet->func[toIdx%13] = tmpFunc;
			}
			curIdx = (toIdx + 1) % 13;
		}

		if (0 != curIdx)
		{
			size_t offset = curIdx;
			void *tmpFunc[2] = { NULL, pRet->func[curIdx] };
			for (int i = 0; i < 13; ++i)
			{
				curIdx = (curIdx + 13-offset) % 13;
				tmpFunc[i%2] = pRet->func[curIdx];
				pRet->func[curIdx] = tmpFunc[(i+1)%2];
			}
		}
	}

	void NEXCALCodecScrambler(NEXCALCodec2 *pRet, void *func2, void *func4, void *func5, void *func6, void *func7, void *func8, void *func9, void *func10, void *func11, void *func12, int numPreScrambles)
	{
		Scrambler(pRet, numPreScrambles);

		pRet->func[2] = func2;
		pRet->func[4] = func4;
		pRet->func[5] = func5;
		pRet->func[6] = func6;
		pRet->func[7] = func7;
		pRet->func[8] = func8;
		pRet->func[9] = func9;
		pRet->func[10] = func10;
		pRet->func[11] = func11;
		pRet->func[12] = func12;
		Scrambler(pRet, 10);
	}
public:

#define VIDEO_DEC_SCRAMBLER(ncc2, GetPropertyFunc, SetPropertyFunc, InitFunc, DeinitFunc, DecodeFunc, UtilityFuncs, numPreScrambles) \
    NEXCALCodecScrambler(&ncc2                                                                                      \
    , (void*)Nex_MC::Video::Decoder::SetDebugLevel                                                                  \
    , GetPropertyFunc                                                                                               \
    , SetPropertyFunc \
    , InitFunc                                                                                                      \
    , DeinitFunc                                                                                                    \
    , DecodeFunc                                                                                                    \
    , (void*)Nex_MC::Video::Decoder::GetOutput                                                                      \
    , (void*)Nex_MC::Video::Decoder::Reset                                                                          \
    , (void*)Nex_MC::Video::Decoder::GetInfo                                                                        \
    , UtilityFuncs                                                                                                  \
    , numPreScrambles)

#define VIDEO_ENC_SCRAMBLER(ncc2, GetPropertyFunc, SetPropertyFunc,InitFunc, DeinitFunc, EncodeFunc, UtilityFuncs, numPreScrambles) \
    NEXCALCodecScrambler(&ncc2                                                                                      \
    , (void*)Nex_MC::Video::Encoder::SetDebugLevel                                                                  \
    , GetPropertyFunc                                                                                               \
    , SetPropertyFunc \
    , InitFunc                                                                                                      \
    , DeinitFunc                                                                                                    \
    , EncodeFunc                                                                                                    \
    , (void*)Nex_MC::Video::Encoder::GetOutput                                                                      \
    , (void*)Nex_MC::Video::Encoder::Skip                                                                           \
    , ncc2.func[11]                                                                                                 \
    , UtilityFuncs                                                                                                  \
    , numPreScrambles)

#define AUDIO_DEC_SCRAMBLER(ncc2, GetPropertyFunc, SetPropertyFunc,InitFunc, DeinitFunc, DecodeFunc, UtilityFuncs, numPreScrambles) \
    NEXCALCodecScrambler(&ncc2                                                                                      \
    , (void*)Nex_MC::Audio::Decoder::SetDebugLevel                                                                  \
    , GetPropertyFunc                                                                                               \
    , SetPropertyFunc                                                                    \
    , InitFunc                                                                                                      \
    , DeinitFunc                                                                                                    \
    , DecodeFunc                                                                                                    \
    , (void*)Nex_MC::Audio::Decoder::Reset                                                                          \
    , (void*)Nex_MC::Audio::Decoder::GetInfo                                                                        \
    , ncc2.func[11]                                                                                                 \
    , UtilityFuncs                                                                                                  \
    , numPreScrambles)

#define AUDIO_ENC_SCRAMBLER(ncc2, GetPropertyFunc, InitFunc, DeinitFunc, EncodeFunc, UtilityFuncs, numPreScrambles) \
    NEXCALCodecScrambler(&ncc2                                                                                      \
    , (void*)Nex_MC::Audio::Encoder::SetDebugLevel                                                                  \
    , GetPropertyFunc                                                                                               \
    , (void*)Nex_MC::Audio::Encoder::SetProperty                                                                    \
    , InitFunc                                                                                                      \
    , DeinitFunc                                                                                                    \
    , EncodeFunc                                                                                                    \
    , (void*)Nex_MC::Audio::Encoder::Reset                                                                          \
    , ncc2.func[10]                                                                                                 \
    , ncc2.func[11]                                                                                                 \
    , UtilityFuncs                                                                                                  \
    , numPreScrambles)

	InitNexCALCodecs()
	{
		unsigned int tickCount = GetTickCount();
		tickCount = GetRealTime();

		NEXCALCodecScrambler(&NexCAL_MC_VideoDecoder_utility_functions
			, NexCAL_MC_VideoDecoder_utility_functions.func[2]
			, (void*)ReleaseOutputBuffer
			, (void*)CaptureFrame
			, NexCAL_MC_VideoDecoder_utility_functions.func[6]
			, NexCAL_MC_VideoDecoder_utility_functions.func[7]
			, (void*)QueryForSupport
			, NexCAL_MC_VideoDecoder_utility_functions.func[9]
			, NexCAL_MC_VideoDecoder_utility_functions.func[10]
			, NexCAL_MC_VideoDecoder_utility_functions.func[11]
			, NexCAL_MC_VideoDecoder_utility_functions.func[12]
			, (tickCount++ % 7) + 1 );

		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_h263, (void*)Nex_MC::H263::Decoder::GetPropertyF, (void*)Nex_MC::Video::Decoder::SetProperty, (void*)Nex_MC::H263::Decoder::InitF, (void*)Nex_MC::H263::Decoder::DeinitF, (void*)Nex_MC::H263::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_h264, (void*)Nex_MC::H264::Decoder::GetPropertyF, (void*)Nex_MC::H264::Decoder::SetPropertyF, (void*)Nex_MC::H264::Decoder::InitF, (void*)Nex_MC::H264::Decoder::DeinitF, (void*)Nex_MC::H264::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_h264_pr, (void*)Nex_MC::H264::Decoder::GetPropertyF, (void*)Nex_MC::H264::Decoder::SetPropertyF,(void*)Nex_MC::H264::Decoder::InitPRF, (void*)Nex_MC::H264::Decoder::DeinitF, (void*)Nex_MC::H264::Decoder::DecodePRF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_h264_sw, (void*)Nex_MC::H264::Decoder::GetPropertySWF, (void*)Nex_MC::H264::Decoder::SetPropertyF, (void*)Nex_MC::H264::Decoder::InitSWF, (void*)Nex_MC::H264::Decoder::DeinitF, (void*)Nex_MC::H264::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_mpeg4v, (void*)Nex_MC::Mpeg4V::Decoder::GetPropertyF, (void*)Nex_MC::Mpeg4V::Decoder::SetPropertyF, (void*)Nex_MC::Mpeg4V::Decoder::InitF, (void*)Nex_MC::Mpeg4V::Decoder::DeinitF, (void*)Nex_MC::Mpeg4V::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_mpeg2, (void*)Nex_MC::Mpeg2::Decoder::GetPropertyF, (void*)Nex_MC::Video::Decoder::SetProperty, (void*)Nex_MC::Mpeg2::Decoder::InitF, (void*)Nex_MC::Mpeg2::Decoder::DeinitF, (void*)Nex_MC::Mpeg2::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_vpx, (void*)Nex_MC::VPX::Decoder::GetPropertyF, (void*)Nex_MC::Video::Decoder::SetProperty, (void*)Nex_MC::VPX::Decoder::InitF, (void*)Nex_MC::VPX::Decoder::DeinitF, (void*)Nex_MC::VPX::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_DEC_SCRAMBLER(NexCAL_MC_VideoDecoder_hevc, (void*)Nex_MC::HEVC::Decoder::GetPropertyF, (void*)Nex_MC::HEVC::Decoder::SetPropertyF, (void*)Nex_MC::HEVC::Decoder::InitF, (void*)Nex_MC::HEVC::Decoder::DeinitF, (void*)Nex_MC::HEVC::Decoder::DecodeF, &NexCAL_MC_VideoDecoder_utility_functions, (tickCount++ % 7) + 1);        

		NEXCALCodecScrambler(&NexCAL_MC_VideoEncoder_utility_functions
			, NexCAL_MC_VideoEncoder_utility_functions.func[2]
			, (void*)GetInputSurface
			, (void*)GetInputANativeWindow
			, (void*)SetBuffersTimestamp
			, (void*)Nex_MC::Video::Encoder::Reset
			, NexCAL_MC_VideoEncoder_utility_functions.func[8]
			, NexCAL_MC_VideoEncoder_utility_functions.func[9]
			, NexCAL_MC_VideoEncoder_utility_functions.func[10]
			, (void *)Nex_MC::H264::Encoder::setCropToAchieveResolution
			, NexCAL_MC_VideoEncoder_utility_functions.func[12]
			, (tickCount++ % 7) + 1 );

		NEXCALCodecScrambler(&NexCAL_MC_VideoEncoder2_utility_functions
			, NexCAL_MC_VideoEncoder2_utility_functions.func[2]
			, (void*)GetInputSurface
			, (void*)GetInputANativeWindow
			, (void*)SetBuffersTimestamp
			, (void*)Nex_MC::Video::Encoder::Reset
			, NexCAL_MC_VideoEncoder2_utility_functions.func[8]
			, NexCAL_MC_VideoEncoder2_utility_functions.func[9]
			, NexCAL_MC_VideoEncoder2_utility_functions.func[10]
			, (void *)Nex_MC::HEVC::Encoder::setCropToAchieveResolution
			, NexCAL_MC_VideoEncoder2_utility_functions.func[12]
			, (tickCount++ % 7) + 1 );
        
		NEXCALCodecScrambler(&NexCAL_MC_VideoEncoder3_utility_functions
			, NexCAL_MC_VideoEncoder3_utility_functions.func[2]
			, (void*)GetInputSurface
			, (void*)GetInputANativeWindow
			, (void*)SetBuffersTimestamp
			, (void*)Nex_MC::Video::Encoder::Reset
			, NexCAL_MC_VideoEncoder3_utility_functions.func[8]
			, NexCAL_MC_VideoEncoder3_utility_functions.func[9]
			, NexCAL_MC_VideoEncoder3_utility_functions.func[10]
			, NexCAL_MC_VideoEncoder3_utility_functions.func[11]
			, NexCAL_MC_VideoEncoder3_utility_functions.func[12]
			, (tickCount++ % 7) + 1 );
        
		VIDEO_ENC_SCRAMBLER(NexCAL_MC_VideoEncoder_h264, (void*)Nex_MC::H264::Encoder::GetPropertyF, (void*)Nex_MC::H264::Encoder::SetPropertyF, (void*)Nex_MC::H264::Encoder::InitF, (void*)Nex_MC::H264::Encoder::DeinitF, (void*)Nex_MC::H264::Encoder::EncodeF, &NexCAL_MC_VideoEncoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_ENC_SCRAMBLER(NexCAL_MC_VideoEncoder_h264_using_frame_data, (void*)Nex_MC::H264::Encoder::GetPropertyF, (void*)Nex_MC::H264::Encoder::SetPropertyF, (void*)Nex_MC::H264::Encoder::InitUsingFrameDataF, (void*)Nex_MC::H264::Encoder::DeinitF, (void*)Nex_MC::H264::Encoder::EncodeUsingFrameDataF, &NexCAL_MC_VideoEncoder_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_ENC_SCRAMBLER(NexCAL_MC_VideoEncoder_hevc, (void*)Nex_MC::HEVC::Encoder::GetPropertyF, (void*)Nex_MC::HEVC::Encoder::SetPropertyF, (void*)Nex_MC::HEVC::Encoder::InitF, (void*)Nex_MC::HEVC::Encoder::DeinitF, (void*)Nex_MC::HEVC::Encoder::EncodeF, &NexCAL_MC_VideoEncoder2_utility_functions, (tickCount++ % 7) + 1);
		VIDEO_ENC_SCRAMBLER(NexCAL_MC_VideoEncoder_mpeg4v, (void*)Nex_MC::Mpeg4V::Encoder::GetPropertyF, (void*)Nex_MC::Mpeg4V::Encoder::SetPropertyF, (void*)Nex_MC::Mpeg4V::Encoder::InitF, (void*)Nex_MC::Mpeg4V::Encoder::DeinitF, (void*)Nex_MC::Mpeg4V::Encoder::EncodeF, &NexCAL_MC_VideoEncoder3_utility_functions, (tickCount++ % 7) + 1);

		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_aac, (void*)Nex_MC::AAC::Decoder::GetPropertyF, (void*)Nex_MC::AAC::Decoder::SetPropertyF, (void*)Nex_MC::AAC::Decoder::InitF, (void*)Nex_MC::AAC::Decoder::DeinitF, (void*)Nex_MC::AAC::Decoder::DecodeF, NexCAL_MC_AudioDecoder_aac.func[12], (tickCount++ % 7) + 1);
		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_aac_pr, (void*)Nex_MC::AAC::Decoder::GetPropertyF, (void*)Nex_MC::AAC::Decoder::SetPropertyF, (void*)Nex_MC::AAC::Decoder::InitPRF, (void*)Nex_MC::AAC::Decoder::DeinitF, (void*)Nex_MC::AAC::Decoder::DecodePRF, NexCAL_MC_AudioDecoder_aac_pr.func[12], (tickCount++ % 7) + 1);
		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_mp3, (void*)Nex_MC::MP3::Decoder::GetPropertyF, (void*)Nex_MC::MP3::Decoder::SetPropertyF,(void*)Nex_MC::MP3::Decoder::InitF, (void*)Nex_MC::MP3::Decoder::DeinitF, (void*)Nex_MC::MP3::Decoder::DecodeF, NexCAL_MC_AudioDecoder_mp3.func[12], (tickCount++ % 7) + 1);
		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_mp3_pr, (void*)Nex_MC::MP3::Decoder::GetPropertyF, (void*)Nex_MC::MP3::Decoder::SetPropertyF,(void*)Nex_MC::MP3::Decoder::InitPRF, (void*)Nex_MC::MP3::Decoder::DeinitF, (void*)Nex_MC::MP3::Decoder::DecodePRF, NexCAL_MC_AudioDecoder_mp3_pr.func[12], (tickCount++ % 7) + 1);
		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_amrnb, (void*)Nex_MC::AMRNB::Decoder::GetPropertyF, (void*)Nex_MC::AMRNB::Decoder::SetPropertyF,(void*)Nex_MC::AMRNB::Decoder::InitF, (void*)Nex_MC::AMRNB::Decoder::DeinitF, (void*)Nex_MC::AMRNB::Decoder::DecodeF, NexCAL_MC_AudioDecoder_amrnb.func[12], (tickCount++ % 7) + 1);
		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_amrwb, (void*)Nex_MC::AMRWB::Decoder::GetPropertyF, (void*)Nex_MC::AMRWB::Decoder::SetPropertyF,(void*)Nex_MC::AMRWB::Decoder::InitF, (void*)Nex_MC::AMRWB::Decoder::DeinitF, (void*)Nex_MC::AMRWB::Decoder::DecodeF, NexCAL_MC_AudioDecoder_amrwb.func[12], (tickCount++ % 7) + 1);
		AUDIO_DEC_SCRAMBLER(NexCAL_MC_AudioDecoder_ogg, (void*)Nex_MC::OGG::Decoder::GetPropertyF, (void*)Nex_MC::OGG::Decoder::SetPropertyF,(void*)Nex_MC::OGG::Decoder::InitF, (void*)Nex_MC::OGG::Decoder::DeinitF, (void*)Nex_MC::OGG::Decoder::DecodeF, NexCAL_MC_AudioDecoder_ogg.func[12], (tickCount++ % 7) + 1);

		AUDIO_ENC_SCRAMBLER(NexCAL_MC_AudioEncoder_aac, (void*)Nex_MC::AAC::Encoder::GetPropertyF, (void*)Nex_MC::AAC::Encoder::InitF, (void*)Nex_MC::AAC::Encoder::DeinitF, (void*)Nex_MC::AAC::Encoder::EncodeF, NexCAL_MC_AudioEncoder_aac.func[12], (tickCount++ % 7) + 1);
		AUDIO_ENC_SCRAMBLER(NexCAL_MC_AudioEncoder_mp3, (void*)Nex_MC::MP3::Encoder::GetPropertyF, (void*)Nex_MC::MP3::Encoder::InitF, (void*)Nex_MC::MP3::Encoder::DeinitF, (void*)Nex_MC::MP3::Encoder::EncodeF, NexCAL_MC_AudioEncoder_mp3.func[12], (tickCount++ % 7) + 1);
		AUDIO_ENC_SCRAMBLER(NexCAL_MC_AudioEncoder_amrnb, (void*)Nex_MC::AMRNB::Encoder::GetPropertyF, (void*)Nex_MC::AMRNB::Encoder::InitF, (void*)Nex_MC::AMRNB::Encoder::DeinitF, (void*)Nex_MC::AMRNB::Encoder::EncodeF, NexCAL_MC_AudioEncoder_amrnb.func[12], (tickCount++ % 7) + 1);
		AUDIO_ENC_SCRAMBLER(NexCAL_MC_AudioEncoder_amrwb, (void*)Nex_MC::AMRWB::Encoder::GetPropertyF, (void*)Nex_MC::AMRWB::Encoder::InitF, (void*)Nex_MC::AMRWB::Encoder::DeinitF, (void*)Nex_MC::AMRWB::Encoder::EncodeF, NexCAL_MC_AudioEncoder_amrwb.func[12], (tickCount++ % 7) + 1);
		#if 0
		VideoScrambler(&NexCAL_MC_VideoDecoder_vpx, (void *)NexMCDecVpx::NEXCALBody_GetPropertyVpx);
		AudioScrambler(&NexCAL_MC_AudioDecoder_amrnb, (void *)NexMCDecAmrnb::NEXCALBody_GetPropertyAmrnb);
		AudioScrambler(&NexCAL_MC_AudioDecoder_amrwb, (void *)NexMCDecAmrwb::NEXCALBody_GetPropertyAmrwb);
		AudioScrambler(&NexCAL_MC_AudioDecoder_mp3, (void *)NexMCDecMp3::NEXCALBody_GetPropertyMp3);
		AudioScrambler(&NexCAL_MC_AudioDecoder_aac, (void *)NexMCDecAac::NEXCALBody_GetPropertyAac);
		AudioScrambler(&NexCAL_MC_AudioDecoder_ogg, (void *)NexMCDecOgg::NEXCALBody_GetPropertyOgg);
		AudioScrambler(&NexCAL_MC_AudioDecoder_alaw, (void *)NexMCDecAlaw::NEXCALBody_GetPropertyAlaw);
		AudioScrambler(&NexCAL_MC_AudioDecoder_mlaw, (void *)NexMCDecMlaw::NEXCALBody_GetPropertyMlaw);
		#endif
	}
	~InitNexCALCodecs() {}
}; // class InitNexCALCodecs

InitNexCALCodecs initNexCALCodecs;

}; // namespace (anon1)


//##############################################################################
using namespace Nex_MC::Log;

EXPORTED
unsigned int getNexCAL_MC_Version()
{
	const char *build_number = xstr(NEXCRALBODY_MC_VERSION_BUILD);
	return (((NEXCRALBODY_MC_VERSION_MAJOR)*100 + NEXCRALBODY_MC_VERSION_MINOR)*100 + NEXCRALBODY_MC_VERSION_PATCH)*100 + (unsigned int)(build_number[0]);
}

#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

EXPORTED
void SetJavaVMforCodec(JavaVM* javaVM)
{
	Nex_MC::Utils::JNI::setJavaVMfromExternal(javaVM);
}

EXPORTED
NEXCALCodec2 *getNexCAL_MC_Decoder(
	  NEX_CODEC_TYPE eCodecType
	, unsigned int API_Version
	, unsigned int nexCAL_API_MAJOR
	, unsigned int nexCAL_API_MINOR
	, unsigned int nexCAL_API_PATCH_NUM
	)
{
	Nex_MC::Utils::initializeAll();

	LOGW("========================================================\n");
	LOGW("NexCALBody_MC Version : %d.%d.%d (%s)\n",	NEXCRALBODY_MC_VERSION_MAJOR,
													NEXCRALBODY_MC_VERSION_MINOR,
													NEXCRALBODY_MC_VERSION_PATCH,
													xstr(NEXCRALBODY_MC_VERSION_MAJOR)
													xstr(NEXCRALBODY_MC_VERSION_MINOR)
													xstr(NEXCRALBODY_MC_VERSION_PATCH)
													xstr(NEXCRALBODY_MC_VERSION_BUILD));
	LOGW("NexCALBody_MC Information : %s\n", NEXCRALBODY_MC_INFORMATION);
	LOGW("========================================================\n");

	// initialize time:
	GetTickCount();

	STATUS(FLOW, INFO, "+ oti(0x%X)", eCodecType);

	STATUS(INPUT, INFO
		, "incoming info API Version:%d, nexCAL_MAJOR:%d, nexCAL_MINOR:%d, nexCAL_PATCH_NUM:%d"
		, API_Version, nexCAL_API_MAJOR, nexCAL_API_MINOR, nexCAL_API_PATCH_NUM);

	if (NEXCAL_MC_API_VERSION != API_Version)
	{
		MC_WARN("WARNING: API Version does not match! (mine:%u yours:%u)", NEXCAL_MC_API_VERSION, API_Version);
	}

	if (   NEXCAL_VERSION_MAJOR != nexCAL_API_MAJOR
		|| NEXCAL_VERSION_MINOR != nexCAL_API_MINOR
		|| NEXCAL_VERSION_PATCH != nexCAL_API_PATCH_NUM )
	{
		MC_WARN("WARNING: CAL API Version does not match! (mine:%x.%x.%x yours:%x.%x.%x)"
			, NEXCAL_VERSION_MAJOR, NEXCAL_VERSION_MINOR, NEXCAL_VERSION_PATCH
			, nexCAL_API_MAJOR, nexCAL_API_MINOR, nexCAL_API_PATCH_NUM);
	}

	NEXCALCodec2 *pRet = NULL;

	switch ((unsigned int)eCodecType)
	{
		case eNEX_CODEC_V_H263:
			pRet = &NexCAL_MC_VideoDecoder_h263;
			break;
		case eNEX_CODEC_V_H264:
			pRet = &NexCAL_MC_VideoDecoder_h264;
			break;
        case eNEX_CODEC_V_H264_MC_S:
            pRet = &NexCAL_MC_VideoDecoder_h264_sw;
            break;
//case NEXOTI_H264_PR:
//	pRet = &NexCAL_MC_VideoDecoder_h264_pr;
//	break;
		case eNEX_CODEC_V_HEVC:
			pRet = &NexCAL_MC_VideoDecoder_hevc;
			break;
		case eNEX_CODEC_V_MPEG4V:
		case eNEX_CODEC_V_DIVX:
			pRet = &NexCAL_MC_VideoDecoder_mpeg4v;
			break;
		case eNEX_CODEC_V_MPEG2V:
			pRet = &NexCAL_MC_VideoDecoder_mpeg2;
			break;
		case eNEX_CODEC_V_VP6:  // need to confirm availability of
		case eNEX_CODEC_V_VP6A: // vp6 and vp7...
		case eNEX_CODEC_V_VP6F: // ...
		case eNEX_CODEC_V_VP7: // ...
		case eNEX_CODEC_V_VP8:  // ...
 			pRet = &NexCAL_MC_VideoDecoder_vpx;
			break;
		case eNEX_CODEC_A_AAC:
		case eNEX_CODEC_A_AACPLUS:
//case NEXOTI_MPEG2AAC:
			pRet = &NexCAL_MC_AudioDecoder_aac;
			break;
//case NEXOTI_AAC_PR:
//case NEXOTI_AAC_PLUS_PR:
//case NEXOTI_MPEG2AAC_PR:
//	pRet = &NexCAL_MC_AudioDecoder_aac_pr;
//	break;
		case eNEX_CODEC_A_MP2:
		case eNEX_CODEC_A_MP3:
		case eNEX_CODEC_A_MP4A:
			pRet = &NexCAL_MC_AudioDecoder_mp3;
			break;
//case NEXOTI_MP3_PR:
//case NEXOTI_MP3inMP4_PR:
//	pRet = &NexCAL_MC_AudioDecoder_mp3_pr;
//	break;
		case eNEX_CODEC_A_AMR:
			pRet = &NexCAL_MC_AudioDecoder_amrnb;
			break;
		case eNEX_CODEC_A_AMRWB:
			pRet = &NexCAL_MC_AudioDecoder_amrwb;
			break;
		case eNEX_CODEC_A_VORBIS:
			pRet = &NexCAL_MC_AudioDecoder_ogg;
			break;
		default:
			break;
	}

	if (NULL == pRet)
	{
		MC_ERR("this CALBody does not support eCodecType(0x%X)", eCodecType);
		return NULL;
	}

	STATUS(FLOW, DEBUG, "-");
	return pRet;
}

#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

EXPORTED
NEXCALCodec2 *getNexCAL_MC_Encoder(
	  NEX_CODEC_TYPE eCodecType
	, unsigned int API_Version
	, unsigned int nexCAL_API_MAJOR
	, unsigned int nexCAL_API_MINOR
	, unsigned int nexCAL_API_PATCH_NUM
	)
{
	Nex_MC::Utils::initializeAll();

	LOGW("========================================================\n");
	LOGW("NexCALBody_MC Version : %d.%d.%d (%s)\n",	NEXCRALBODY_MC_VERSION_MAJOR,
													NEXCRALBODY_MC_VERSION_MINOR,
													NEXCRALBODY_MC_VERSION_PATCH,
													xstr(NEXCRALBODY_MC_VERSION_MAJOR)
													xstr(NEXCRALBODY_MC_VERSION_MINOR)
													xstr(NEXCRALBODY_MC_VERSION_PATCH)
													xstr(NEXCRALBODY_MC_VERSION_BUILD));
	LOGW("NexCALBody_MC Information : %s\n", NEXCRALBODY_MC_INFORMATION);
	LOGW("========================================================\n");

	// initialize time:
	GetTickCount();

	STATUS(FLOW, INFO, "+ oti(0x%X)", eCodecType);

	STATUS(INPUT, INFO
		, "incoming info API Version:%d, nexCAL_MAJOR:%d, nexCAL_MINOR:%d, nexCAL_PATCH_NUM:%d"
		, API_Version, nexCAL_API_MAJOR, nexCAL_API_MINOR, nexCAL_API_PATCH_NUM);

	if (NEXCAL_MC_API_VERSION != API_Version)
	{
		MC_WARN("WARNING: API Version does not match! (mine:%u yours:%u)", NEXCAL_MC_API_VERSION, API_Version);
	}

	if (   NEXCAL_VERSION_MAJOR != nexCAL_API_MAJOR
		|| NEXCAL_VERSION_MINOR != nexCAL_API_MINOR
		|| NEXCAL_VERSION_PATCH != nexCAL_API_PATCH_NUM )
	{
		MC_WARN("WARNING: CAL API Version does not match! (mine:%x.%x.%x yours:%x.%x.%x)"
			, NEXCAL_VERSION_MAJOR, NEXCAL_VERSION_MINOR, NEXCAL_VERSION_PATCH
			, nexCAL_API_MAJOR, nexCAL_API_MINOR, nexCAL_API_PATCH_NUM);
	}

	NEXCALCodec2 *pRet = NULL;

	switch ((unsigned int)eCodecType)
	{
		case eNEX_CODEC_V_H264:
			if (NULL == Nex_MC::JNI::JMETHODS::MediaCodec.createInputSurface)
			{
				pRet = &NexCAL_MC_VideoEncoder_h264_using_frame_data;
			}
			else
			{
				pRet = &NexCAL_MC_VideoEncoder_h264;
			}
			break;
		case eNEX_CODEC_V_H264_MC_ENCODER_USING_FRAMEDATA:
			pRet = &NexCAL_MC_VideoEncoder_h264_using_frame_data;
			break;
//case NEXOTI_H264_PR: // currently 0x100000C1 (will want to add a new name for encoders)
//	pRet = &NexCAL_MC_VideoEncoder_h264_using_frame_data;
			break;
		case eNEX_CODEC_V_HEVC:
			pRet = &NexCAL_MC_VideoEncoder_hevc;
			break;

		case eNEX_CODEC_V_MPEG4V:
			pRet = &NexCAL_MC_VideoEncoder_mpeg4v;
			break;

		case eNEX_CODEC_A_AAC:
		case eNEX_CODEC_A_AACPLUS:
//case NEXOTI_MPEG2AAC:
			pRet = &NexCAL_MC_AudioEncoder_aac;
			break;
		case eNEX_CODEC_A_MP2:
		case eNEX_CODEC_A_MP3:
		case eNEX_CODEC_A_MP4A:
			pRet = &NexCAL_MC_AudioEncoder_mp3;
			break;
		case eNEX_CODEC_A_AMR:
			pRet = &NexCAL_MC_AudioEncoder_amrnb;
			break;
		case eNEX_CODEC_A_AMRWB:
			pRet = &NexCAL_MC_AudioEncoder_amrwb;
			break;
		default:
			break;
	}

	if (NULL == pRet)
	{
		MC_ERR("this CALBody does not support eCodecType(0x%X)", eCodecType);
		return NULL;
	}

	STATUS(FLOW, DEBUG, "-");
	return pRet;
}

#if defined(NEX_ADD_OC_DEFS)
extern "C" {
EXPORTED
NEXCALCodec2 *getNexCAL_OC_VideoDecoder(
	  NEX_CODEC_TYPE eCodecType
	, unsigned int API_Version
	, unsigned int nexCAL_API_MAJOR
	, unsigned int nexCAL_API_MINOR
	, unsigned int nexCAL_API_PATCH_NUM
	)
{
	return getNexCAL_MC_Decoder(eCodecType, API_Version, nexCAL_API_MAJOR, nexCAL_API_MINOR, nexCAL_API_PATCH_NUM);
}
};
#endif

#if defined(NEX_ADD_AAC_SF_DEFS)
extern "C" {
EXPORTED
void * GetHandle()
{
	return (void *)getNexCAL_MC_Decoder(NEXOTI_AAC, NEXCAL_MC_API_VERSION, NEXCAL_MAJOR_VERSION, NEXCAL_MINOR_VERSION, NEXCAL_PATCH_NUM);
}

EXPORTED
void CloseHandle( void *pHandle )
{
	//free( pHandle );
}
};
#endif

#if defined(NEX_ADD_MP3_SF_DEFS)
extern "C" {
EXPORTED
void * GetHandle()
{
	return (void *)getNexCAL_MC_Decoder(NEXOTI_MP3, NEXCAL_MC_API_VERSION, NEXCAL_MAJOR_VERSION, NEXCAL_MINOR_VERSION, NEXCAL_PATCH_NUM);
}

EXPORTED
void CloseHandle( void *pHandle )
{
	//free( pHandle );
}
};
#endif

#if defined(NEX_ADD_AMRNB_SF_DEFS)
extern "C" {
EXPORTED
void * GetHandle()
{
	return (void *)getNexCAL_MC_Decoder(NEXOTI_AMR, NEXCAL_MC_API_VERSION, NEXCAL_MAJOR_VERSION, NEXCAL_MINOR_VERSION, NEXCAL_PATCH_NUM);
}

EXPORTED
void CloseHandle( void *pHandle )
{
	//free( pHandle );
}
};
#endif

#if defined(NEX_ADD_AMRWB_SF_DEFS)
extern "C" {
EXPORTED
void * GetHandle()
{
	return (void *)getNexCAL_MC_Decoder(NEXOTI_AMRWB, NEXCAL_MC_API_VERSION, NEXCAL_MAJOR_VERSION, NEXCAL_MINOR_VERSION, NEXCAL_PATCH_NUM);
}

EXPORTED
void CloseHandle( void *pHandle )
{
	//free( pHandle );
}
};
#endif

#if defined(NEX_ADD_OGG_SF_DEFS)
extern "C" {
EXPORTED
void * GetHandle()
{
	return (void *)getNexCAL_MC_Decoder(NEXOTI_VORBIS, NEXCAL_MC_API_VERSION, NEXCAL_MAJOR_VERSION, NEXCAL_MINOR_VERSION, NEXCAL_PATCH_NUM);
}

EXPORTED
void CloseHandle( void *pHandle )
{
	//free( pHandle );
}
};
#endif

/*-----------------------------------------------------------------------------
Revision History 
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
...
-----------------------------------------------------------------------------*/
