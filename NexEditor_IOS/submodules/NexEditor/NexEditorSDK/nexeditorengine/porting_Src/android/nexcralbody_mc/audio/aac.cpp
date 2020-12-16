/******************************************************************************
* File Name        : aac.cpp
* Description      : aac codec for Android
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#define LOCATION_TAG "aac"

#undef LOG_TAG
#define LOG_TAG "nexcral_mc"

#include "NexCAL.h"

#include "NexCRALBody_mc_impl.h"
#include "external/NexMediaCodec.h"

#include "external/jni/jni_MediaCodec.h"
#include "external/jni/jni_MediaFormat.h"

#include "utils/utils.h"
#include "utils/jni.h"

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_AUDIO

namespace Nex_MC {

using Common::NexCAL_mc;

namespace AAC {

const char *MIMETYPE = "audio/mp4a-latm";

namespace Decoder {

#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "AACD"

namespace { // (anon201)
	pthread_mutex_t g_preferredDecoderMutex;
	char *g_preferredDecoderName = NULL;
	bool g_checkedForPreferredDecoder = false;

	class CleanUpCrewDecoder {
	public:
		CleanUpCrewDecoder()
		{
			pthread_mutex_init(&g_preferredDecoderMutex, NULL);
		}

		~CleanUpCrewDecoder()
		{
			pthread_mutex_lock(&g_preferredDecoderMutex);
			if (NULL != g_preferredDecoderName)
			{
				free(g_preferredDecoderName);
				g_preferredDecoderName = NULL;
			}
			g_checkedForPreferredDecoder = false;
			pthread_mutex_unlock(&g_preferredDecoderMutex);

			pthread_mutex_destroy(&g_preferredDecoderMutex);
		}
	};

	CleanUpCrewDecoder cleanUp;
} // namespace (anon201)

namespace { // (anon1)

typedef struct CodecSpecificData {
	jobject jobj_csd0;
} CodecSpecificData;

}; // namespace (anon1)

NXINT32 SetProperty(NXUINT32 uProperty, NXINT64 lValue, NXVOID *pUserData)
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = Audio::Decoder::SetProperty(uProperty, lValue, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, lValue);
	return 0;
}
NEXCALSetProperty SetPropertyF = SetProperty;

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = Audio::Decoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	NXINT64 input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME :
		*puValue = (NXINT64)"TODO:"; // TODO:
		break;


	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME :
		*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_NO;
		break;

	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER :
		*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_YES;
		break;

	case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO :
		*puValue = NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_NO;
		break;

	case NEXCAL_PROPERTY_AAC_SUPPORT_MAIN :
		*puValue = 0;
		break;

	case NEXCAL_PROPERTY_AAC_SUPPORT_LTP :
		*puValue = 1;
		break;

	case NEXCAL_PROPERTY_AAC_SUPPORT_PS :
		*puValue = 1;
		break;

	case NEXCAL_PROPERTY_AAC_SUPPORT_SBR :
		*puValue = 1;
		break;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 Deinit(NXVOID *pUserData);

NXINT32 Init
		( NEX_CODEC_TYPE eCodecType
		, NXUINT8* pConfig
		, NXINT32 iConfigLen
		, NXUINT8* pFrame
		, NXINT32 iFrameLen
		, NXVOID *pInitInfo
		, NXVOID *pExtraInfo
		, NXUINT32* puSamplingRate
		, NXUINT32* puNumOfChannels
		, NXUINT32* puBitsPerSample
		, NXUINT32* puNumOfSamplesPerChannel
		, NXUINT32 uMode
		, NXUINT32 uUserDataType
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X)", eCodecType);

	NXINT32 retValue = 0;

	NexCAL_mc *mc = Audio::Decoder::CreateNexCAL_mc(CODEC_NAME, MIMETYPE, *puSamplingRate, *puNumOfChannels, *puNumOfSamplesPerChannel, uUserDataType, ppUserData);

	if (NULL == mc)
	{
		MC_ERR("failed to create userData");
		return -1;
	}

	if (Utils::Manufacturer::SAMSUNG == Utils::GetManufacturer())
	{
		pthread_mutex_lock(&g_preferredDecoderMutex);
		if (!g_checkedForPreferredDecoder)
		{
			int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::HW, g_preferredDecoderName);
			g_checkedForPreferredDecoder = true;
		}
		pthread_mutex_unlock(&g_preferredDecoderMutex);

		if (NULL == g_preferredDecoderName)
		{
			MC_WARN("couldn't find non-google aac decoder");
		}
		else
		{
			mc->preferredCodecName = (const char *)g_preferredDecoderName;
		}
	}

#if 1 == _NEX_INPUT_DUMP_
	mc->inFile = fopen("/sdcard/cralbody_mc_test/aacd_in.mct", "wb");
	if (NULL != mc->inFile)
	{
		int hexa1 = 1;
		fwrite(&hexa1, sizeof(int), 1, mc->inFile);
		fwrite(&mc->avtype, sizeof(int), 1, mc->inFile);
		fwrite(&mc->type, sizeof(int), 1, mc->inFile);
		fwrite(&eCodecType, sizeof(unsigned int), 1, mc->inFile);
		fwrite(puSamplingRate, sizeof(unsigned int), 1, mc->inFile);
		fwrite(puNumOfChannels, sizeof(unsigned int), 1, mc->inFile);
		fwrite(puBitsPerSample, sizeof(unsigned int), 1, mc->inFile);
		fwrite(puNumOfSamplesPerChannel, sizeof(unsigned int), 1, mc->inFile);
		if (0 < iConfigLen && NULL != pConfig)
		{
			fwrite(&iConfigLen, sizeof(int), 1, mc->inFile);
			fwrite(pConfig, sizeof(unsigned char), (size_t)iConfigLen, mc->inFile);
		}
	}
#endif

#if 1 == _NEX_OUTPUT_DUMP_
	mc->outFile = fopen("/sdcard/cralbody_mc_test/aacd_out.mct", "wb");
	if (NULL != mc->outFile)
	{
		int hexa1 = 1;
		fwrite(&hexa1, sizeof(int), 1, mc->outFile);
		fwrite(&mc->avtype, sizeof(int), 1, mc->outFile);
		int type = NEX_MC_TYPE_ENCODER;
		fwrite(&type, sizeof(int), 1, mc->outFile);
		fwrite(&eCodecType, sizeof(unsigned int), 1, mc->outFile);
	}
#elif 2 == _NEX_OUTPUT_DUMP_
	mc->outFile = fopen("/sdcard/cralbody_mc_test/aacd_pcm_out.mct", "wb");
#endif

	CodecSpecificData *csd = NULL;

	if (0 < iConfigLen)
	{
		csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
		if (NULL == csd)
		{
			MC_ERR("malloc failed!");
			*ppUserData = 0;

#if 1 == _NEX_INPUT_DUMP_
			if (NULL != mc->inFile)
			{
				fclose(mc->inFile);
				mc->inFile = NULL;
			}
#endif
#if 1 == _NEX_OUTPUT_DUMP_
			if (NULL != mc->outFile)
			{
				fclose(mc->outFile);
				mc->outFile = NULL;
			}
#endif

			free(mc);

			IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -2);
			return -2;
		}
		STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
		memset(csd, 0x00, sizeof(CodecSpecificData));
		void *csd0;
		csd->jobj_csd0 = Utils::JNI::NewDirectByteBuffer(iConfigLen, csd0);
		memcpy(csd0, pConfig, iConfigLen);
		MEM_DUMP(Log::INPUT, Log::VERBOSE, pConfig, (size_t)iConfigLen, (size_t)iConfigLen);

		mc->ad.codecSpecific = (void *)csd;
	}

	bool isADTS = true;

	if (3 <= iFrameLen)
	{
		// pDest[0]	== 0xFF;									// 1111 1111
		// pDest[1]	== 0xF0 | (mpeg4||mpeg2)<<3 | (!crc);		// 1111 x00x
		// pDest[2]	== 0x40 | sampling_frequency_index<<2;		// 01xx xx00
		if (0xFF != pFrame[0] || 0xF0 != (0xF6 & pFrame[1]) || 0x40 != (0x40 & pFrame[2]))
		{
			isADTS = false;
		}
	}

	mc->inputFormat->setInteger("is-adts", isADTS ? 1 : 0);

	if (NULL != csd && NULL != csd->jobj_csd0)
	{
		mc->inputFormat->setByteBuffer("csd-0", csd->jobj_csd0);
	}

	*ppUserData = (NXVOID*)mc;

	//retValue = Audio::Decoder::Init(mc, pFrame, iFrameLen);
	retValue = Audio::Decoder::Init(mc, NULL, 0);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*puSamplingRate = mc->ad.sampleRate;
	*puNumOfChannels = mc->ad.channelCount;
	*puBitsPerSample = mc->ad.bitsPerSample;
	*puNumOfSamplesPerChannel = mc->ad.numSamplesPerChannel;

#if 1 == _NEX_OUTPUT_DUMP_
	if (NULL != mc->outFile)
	{
		fwrite(puSamplingRate, sizeof(unsigned int), 1, mc->outFile);
		fwrite(puNumOfChannels, sizeof(unsigned int), 1, mc->outFile);
		int bitRate = 1024*1024;
		fwrite(&bitRate, sizeof(int), 1, mc->outFile);
	}
#endif

	const char *codec_name = NULL;

	int getname_res = mc->mc->getName(codec_name);

	if (0 != getname_res)
	{
		MC_WARN("getName failed(%d)", getname_res);
	}

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X) codecName(%s)", retValue, codec_name);
	return retValue;
}
NEXCALAudioInit InitF = Init;

NXINT32 Deinit( NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	// call common Deinit
	NXINT32 retValue = Audio::Decoder::Deinit(mc);

	CodecSpecificData *csd = (CodecSpecificData *)mc->ad.codecSpecific;

	if (NULL != csd)
	{
		if (NULL != csd->jobj_csd0)
		{
			Utils::JNI::DeleteDirectByteBuffer(csd->jobj_csd0);
		}

		free(csd);
		csd = NULL;
		mc->ad.codecSpecific = NULL;
	}

#if 1 == _NEX_INPUT_DUMP_
	if (NULL != mc->inFile)
	{
		fclose(mc->inFile);
		mc->inFile = NULL;
	}
#endif
#if 1 == _NEX_OUTPUT_DUMP_
	if (NULL != mc->outFile)
	{
		fclose(mc->outFile);
		mc->outFile = NULL;
	}
#endif

	free(mc);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALAudioDeinit DeinitF = Deinit;

NXINT32 Decode
		( NXUINT8* pSource
		, NXINT32 iLen
		, NXVOID *pExtraInfo		
		, NXVOID* pDest
		, NXINT32* piWrittenPCMSize
		, NXUINT32 uDTS
		, NXUINT32 *puOutputTime
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "len(%d) ts(%u) flag(0x%X)", iLen, uDTS, nFlag);

	NXINT32 retValue = 0;

	retValue = Audio::Decoder::Decode(pSource, iLen, pDest, piWrittenPCMSize, uDTS, puOutputTime, nFlag, puDecodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}
NEXCALAudioDecode DecodeF = Decode;

NXINT32 InitPR
		( NEX_CODEC_TYPE eCodecType
		, NXUINT8* pConfig
		, NXINT32 iConfigLen
		, NXUINT8* pFrame
		, NXINT32 iFrameLen
		, NXVOID *a_pInitInfo
		, NXVOID *a_pExtraInfo
		, NXUINT32* puSamplingRate
		, NXUINT32* puNumOfChannels
		, NXUINT32* puBitsPerSample
		, NXUINT32* puNumOfSamplesPerChannel
		, NXUINT32 uMode
		, NXUINT32 uUserDataType
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X)", eCodecType);

	NXINT32 retValue = 0;

	NexCAL_mc *mc = Audio::Decoder::CreateNexCAL_mc(CODEC_NAME, MIMETYPE, *puSamplingRate, *puNumOfChannels, *puNumOfSamplesPerChannel, uUserDataType, ppUserData);

	if (NULL == mc)
	{
		MC_ERR("failed to create userData");
		return -1;
	}

	CodecSpecificData *csd = NULL;

	if (0 < iConfigLen)
	{
		csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
		if (NULL == csd)
		{
			MC_ERR("malloc failed!");
			*ppUserData = 0;

#if 1 == _NEX_INPUT_DUMP_
			if (NULL != mc->inFile)
			{
				fclose(mc->inFile);
				mc->inFile = NULL;
			}
#endif
#if 1 == _NEX_OUTPUT_DUMP_
			if (NULL != mc->outFile)
			{
				fclose(mc->outFile);
				mc->outFile = NULL;
			}
#endif

			free(mc);

			IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -2);
			return -2;
		}
		STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
		memset(csd, 0x00, sizeof(CodecSpecificData));
		void *csd0;
		csd->jobj_csd0 = Utils::JNI::NewDirectByteBuffer(iConfigLen, csd0);
		memcpy(csd0, pConfig, iConfigLen);

		mc->ad.codecSpecific = (void *)csd;
	}

	mc->inputFormat->setInteger("is-adts", 1);

	if (NULL != csd && NULL != csd->jobj_csd0)
	{
		mc->inputFormat->setByteBuffer("csd-0", csd->jobj_csd0);
	}

	Common::MediaCryptoInfo *mci = (Common::MediaCryptoInfo *)pFrame;

	int ret = 0;
	ret = NexMediaCodec_using_jni::createMediaCrypto(mci->mostSignificantBits, mci->leastSignificantBits, mci->initData, mci->initDataLength, mc->ad.crypto);
	if (0 != ret)
	{
		MC_ERR("exception occurred while creating media crypto (%d)", ret);
		return -2;
	}
	ret = NexMediaCodec_using_jni::createCryptoInfo(mc->ad.cryptoInfo);
	if (0 != ret)
	{
		MC_ERR("exception occurred while creating crypto info (%d)", ret);
		return -3;
	}

	*ppUserData = (NXVOID*)mc;

	retValue = Audio::Decoder::Init(mc, NULL, 0);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*puSamplingRate = mc->ad.sampleRate;
	*puNumOfChannels = mc->ad.channelCount;
	*puBitsPerSample = mc->ad.bitsPerSample;
	*puNumOfSamplesPerChannel = mc->ad.numSamplesPerChannel;

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALAudioInit InitPRF = InitPR;

NXINT32 DecodePR
		( NXUINT8* pSource
		, NXINT32 iLen
		, NXVOID* pDest
		, NXVOID *a_pExtraInfo
		, NXINT32* piWrittenPCMSize
		, NXUINT32 uDTS
		, NXUINT32 *puOutputTime
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "len(%d) ts(%u) flag(0x%X)", iLen, uDTS, nFlag);

	NXINT32 retValue = 0;

	retValue = Audio::Decoder::DecodePR(pSource, iLen, pDest, piWrittenPCMSize, uDTS, puOutputTime, nFlag, puDecodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}
NEXCALAudioDecode DecodePRF = DecodePR;

}; // namespace Decoder

namespace Encoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "AACE"

namespace { // (anon101)
	pthread_mutex_t g_preferredEncoderMutex;
	char *g_preferredEncoderName = NULL;
	bool g_checkedForPreferredEncoder = false;

	class CleanUpCrewEncoder {
	public:
		CleanUpCrewEncoder()
		{
			pthread_mutex_init(&g_preferredEncoderMutex, NULL);
		}

		~CleanUpCrewEncoder()
		{
			pthread_mutex_lock(&g_preferredEncoderMutex);
			if (NULL != g_preferredEncoderName)
			{
				free(g_preferredEncoderName);
				g_preferredEncoderName = NULL;
			}
			g_checkedForPreferredEncoder = false;
			pthread_mutex_unlock(&g_preferredEncoderMutex);

			pthread_mutex_destroy(&g_preferredEncoderMutex);
		}
	};

	CleanUpCrewEncoder cleanUp;
} // namespace (anon101)

namespace { // (anon2)

typedef struct CodecSpecificData {
	void * csd0;
} CodecSpecificData;

}; // namespace (anon2)

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = Audio::Encoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	NXINT64 input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
		*puValue = (unsigned long)"TODO:"; // TODO:
		break;

#if 0
	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME :
		*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_NO;
		break;

	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER :
		*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_YES;
		break;

	case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO:
		*puValue = NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_NO;
		break;
#endif
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 Deinit(NXVOID *pUserData);

int MakeADTSHeader(unsigned char *pDSI, int nSample, unsigned char *pDest)
{
	unsigned char sampling_frequency_index = 0;

	sampling_frequency_index = pDSI[0];
	sampling_frequency_index &= 0x07;
	sampling_frequency_index <<=1;
	sampling_frequency_index |= (pDSI[1] & 0x80 ? 1 : 0);

	pDest[0]	= 0xFF;									// 1111 1111
	pDest[1]	= 0xF9;									// 1111 1001
	pDest[2]	= 0x40 | sampling_frequency_index<<2;	// 01xx xx00
	pDest[3]	= ((pDSI[1]>>3)&0x0F)<<6; 				// xx00 0000

	pDest[3]	&= (unsigned char)0xFC;
	pDest[3]	|= (unsigned char)(((nSample+7)&0x1800)>>11);	// 0001 1000 0000 0000
	pDest[4]	= (unsigned char)(((nSample+7)&0x7F8)>>3);		// 0000 0111 1111 1000
	pDest[5]	= (unsigned char)(((nSample+7)&0x7)<<5);		// 0000 0000 0000 0111
	pDest[5]	|= (unsigned char)0x1F;							// xxx1 1111
	pDest[6]	= (unsigned char)0xFC;							// 1111 1100

	return 0;
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

	if (NULL != mc->lastOutputBytes && (mc->lastOutputSize + 7) > mc->lastOutputBytesSize)
	{
		free(mc->lastOutputBytes);
		mc->lastOutputBytes = NULL;
		mc->lastOutputBytesSize = 0;
	}
	if (NULL == mc->lastOutputBytes)
	{
		unsigned int bytesToAlloc = mc->lastOutputSize + 7 - 1;
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
	CodecSpecificData *csd = (CodecSpecificData *)mc->ae.codecSpecific;
	MakeADTSHeader((unsigned char *)csd->csd0, mc->lastOutputSize, mc->lastOutputBytes);
	memcpy(mc->lastOutputBytes + 7, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);
	*ppDest = mc->lastOutputBytes;
	ret = mc->lastOutputSize + 7;

	STATUS(Log::FLOW, Log::DEBUG, "- ret(%d)", ret);
	return ret;
}

NXINT32 Init
		( NEX_CODEC_TYPE eCodecType
		, NXUINT8** ppConfig
		, NXINT32 * piConfigLen
		, NXUINT32 uSamplingRate
		, NXUINT32 uNumOfChannels
		, NXINT32		   iBitRate
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X)", eCodecType);

	NXINT32 retValue = 0;

	NexCAL_mc *mc = (NexCAL_mc *)malloc(sizeof(NexCAL_mc));
	if (NULL == mc)
	{
		MC_ERR("malloc failed!");
		*ppUserData = 0;
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -1);
		return -1;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(NexCAL_mc));

	memset(mc, 0x00, sizeof(NexCAL_mc));

	mc->type = NEX_MC_TYPE;
	mc->avtype = NEX_MC_AVTYPE;
	mc->codecName = CODEC_NAME;

	mc->mimeType = MIMETYPE;

	mc->ae.sampleRate = uSamplingRate;
	mc->ae.channelCount = uNumOfChannels;
	mc->ae.bitRate = iBitRate;

	mc->ae.writeOutputFrame = WriteOutputFrame;

#if 1 == _NEX_INPUT_DUMP_
	mc->inFile = fopen("/sdcard/cralbody_mc_test/aace_in.mct", "wb");
	if (NULL != mc->inFile)
	{
		int hexa1 = 1;
		fwrite(&hexa1, sizeof(int), 1, mc->inFile);
		fwrite(&mc->avtype, sizeof(int), 1, mc->inFile);
		fwrite(&mc->type, sizeof(int), 1, mc->inFile);
		fwrite(&eCodecType, sizeof(unsigned int), 1, mc->inFile);
		fwrite(&uSamplingRate, sizeof(unsigned int), 1, mc->inFile);
		fwrite(&uNumOfChannels, sizeof(unsigned int), 1, mc->inFile);
		fwrite(&iBitRate, sizeof(int), 1, mc->inFile);
	}
#elif 2 == _NEX_INPUT_DUMP_
	mc->inFile = fopen("/sdcard/cralbody_mc_test/aace_pcm_in.mct", "wb");
#endif

#if 1 == _NEX_OUTPUT_DUMP_
	mc->outFile = fopen("/sdcard/cralbody_mc_test/aace_out.mct", "wb");
	if (NULL != mc->outFile)
	{
		int hexa1 = 1;
		fwrite(&hexa1, sizeof(int), 1, mc->outFile);
		fwrite(&mc->avtype, sizeof(int), 1, mc->outFile);
		int type = NEX_MC_TYPE_ENCODER;
		fwrite(&type, sizeof(int), 1, mc->outFile);
		fwrite(&eCodecType, sizeof(unsigned int), 1, mc->outFile);
	}
#elif 2 == _NEX_OUTPUT_DUMP_
	mc->outFile = fopen("/sdcard/cralbody_mc_test/aace_aac_adts_out.mct", "wb");
#endif


	pthread_mutex_lock(&g_preferredEncoderMutex);
	if (!g_checkedForPreferredEncoder)
	{
		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName);
		g_checkedForPreferredEncoder = true;
	}
	pthread_mutex_unlock(&g_preferredEncoderMutex);

	mc->preferredCodecName = (const char *)g_preferredEncoderName;

	*ppUserData = (NXVOID*)mc;

	retValue = Audio::Encoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	{
		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		mc->outTimeout = 1000; // 1ms

		int giveInputAt0 = 3;

		while (0 > mc->outIndex)
		{
			mc->engWaiting = true;

			if (0 >= giveInputAt0)
			{
				// send first frame
				EXTERNAL_CALL_WITH_ACTION(dequeueInputBuffer, Log::DEBUG
					, mc->mc->dequeueInputBuffer(1, mc->inIndex)
					, *ppUserData = 0
					; Deinit((NXVOID*)mc)
					; IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -2)
					; return -2);

				if (0 <= mc->inIndex)
				{
					int frameLen = mc->ae.channelCount * 2 /* bytespersample */ * 1024 /* default samplesperchannel for aac */;
					STATUS(Log::FLOW, Log::DEBUG, "memset([%ld](%p), 0x00, %d)", mc->inIndex, mc->inputBuffers[mc->inIndex], frameLen);
					memset(mc->inputBuffers[mc->inIndex], 0x00, frameLen);

					EXTERNAL_CALL_WITH_ACTION(queueInputBuffer, Log::DEBUG
						, mc->mc->queueInputBuffer(mc->inIndex, 0, frameLen, 0LL, 0)
						, *ppUserData = 0
						; Deinit((NXVOID*)mc)
						; IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -3)
						; return -3);

					++mc->ae.dummyCount;
				}
				mc->inIndex = -1;
			}

			if (-2 == mc->outIndex)
			{
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
			//sched_yield();
			STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
			pthread_mutex_lock(&mc->engLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

			if (mc->outWaiting == false)
			{
				STATUS(Log::MUTEX, Log::DEBUG, ">>(%sCond, engLock)", "eng");
				pthread_cond_wait(&mc->engCond, &mc->engLock);
				STATUS(Log::MUTEX, Log::DEBUG, "(%sCond, engLock)>>", "eng");
			}
			mc->engWaiting = false;

			--giveInputAt0;
		}

		STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

		if (2 != (mc->lastOutputFlags & 2))
		{
			MC_WARN("first output from encoder was not CODEC_CONFIG! No DSI found!");

			--mc->ae.dummyCount;
		}
		else
		{
			CodecSpecificData *csd = NULL;

			csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
			if (NULL == csd)
			{
				MC_ERR("malloc failed!");
				*ppUserData = 0;
				Deinit((NXVOID*)mc);
				IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -7);
				return -7;
			}
			STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
			memset(csd, 0x00, sizeof(CodecSpecificData));
			csd->csd0 = malloc(mc->lastOutputSize);
			if (NULL == csd->csd0)
			{
				MC_ERR("malloc failed!");
				*ppUserData = 0;
				Deinit((NXVOID*)mc);
				IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -8);
				return -8;
			}
			STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd->csd0, mc->lastOutputSize);

			memcpy(csd->csd0, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);

			mc->ae.codecSpecific = (void *)csd;

			*ppConfig = (unsigned char *)csd->csd0;
			*piConfigLen = mc->lastOutputSize;
		}

		EXTERNAL_CALL_WITH_ACTION(releaseOutputBuffer, Log::DEBUG
			, mc->mc->releaseOutputBuffer(mc->outIndex, false)
			, *ppUserData = 0
			; Deinit((NXVOID*)mc)
			; IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -4)
			; return -4);

		mc->outIndex = -1;

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		// TODO: check for exception?
		// Nex_MC::Audio::Encoder::Reset((unsigned int)mc);

		mc->outTimeout = mc->defaultOutTimeout;
	}

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALAudioEncoderInit InitF = Init;

NXINT32 Deinit( NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	// call common Deinit
	NXINT32 retValue = Audio::Encoder::Deinit(mc);

	CodecSpecificData *csd = (CodecSpecificData *)mc->ae.codecSpecific;

	if (NULL != csd)
	{
		if (NULL != csd->csd0)
		{
			free(csd->csd0);
		}

		free(csd);
		csd = NULL;
		mc->ae.codecSpecific = NULL;
	}

	free(mc);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALAudioEncoderDeinit DeinitF = Deinit;

NXINT32 Encode
		( NXUINT8* pData
		, NXINT32 iLen
		, NXUINT8** ppOutData
		, NXUINT32* piOutLen
		, NXUINT32* puEncodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "len(%d)", iLen);

	NXINT32 retValue = 0;

	retValue = Audio::Encoder::Encode(pData, iLen, ppOutData, piOutLen, puEncodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
	return retValue;
}
NEXCALAudioEncoderEncode EncodeF = Encode;

}; // namespace Encoder

}; // namespace AAC

}; // namespace Nex_MC

