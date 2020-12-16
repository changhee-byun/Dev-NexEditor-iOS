/******************************************************************************
* File Name        : vorbis.cpp
* Description      : vorbis codec for Android
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
#define LOCATION_TAG "vorbis"

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

namespace OGG {

const char *MIMETYPE = "audio/vorbis";

namespace { // (anon1)

inline unsigned char * find_csd(int target, unsigned char *pConfig, int iConfigLen, int &csdLen)
{
	csdLen = 0;
	unsigned char *ret = NULL;

	if (7 < iConfigLen)
	{
		int i = 0;

		int header = 0;
		header |= pConfig[i++] << 24;
		header |= pConfig[i++] << 16;
		header |= pConfig[i++] << 8;
		header |= pConfig[i++];

		while (target != header && i < iConfigLen)
		{
			header = (header << 8) | pConfig[i++];
		}

		if (target == header)
		{
			ret = pConfig + i - 4;

			if (7 > (iConfigLen - 7 - (ret - pConfig)))
			{
				csdLen = iConfigLen - (ret - pConfig);
			}
			else
			{
				header = 0;
				header |= pConfig[i++] << 24;
				header |= pConfig[i++] << 16;
				header |= pConfig[i++] << 8;
				header |= pConfig[i++];

				while (0x766f72 != (header & 0xFFFFFF) && i < iConfigLen)
				{
					header = (header << 8) | pConfig[i++];
				}

				if (0x766f72 == (header & 0xFFFFFF))
				{
					unsigned char *end = pConfig + i - 4;
					csdLen = end - ret;
				}
				else
				{
					csdLen = iConfigLen - (ret - pConfig);
				}
			}
		}
	}

	return ret;
}

typedef struct CodecSpecificData {
	jobject jobj_csd0;
	jobject jobj_csd1;
} CodecSpecificData;

}; // namespace (anon1)

namespace Decoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "OGGD"

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
	case NEXCAL_PROPERTY_COMPONENT_NAME:
		*puValue = (NXINT64)"TODO:"; // TODO:
		break;

	case NEXCAL_PROPERTY_SUPPORT_ES_AUDIO:
		*puValue = NEXCAL_PROPERTY_SUPPORT_ES_AUDIO_NO;
		break;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 Deinit( NXVOID *pUserData );

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
			MC_WARN("couldn't find non-google vorbis decoder");
		}
		else
		{
			mc->preferredCodecName = (const char *)g_preferredDecoderName;
		}
	}

#if 1 == _NEX_INPUT_DUMP_
	mc->inFile = fopen("/sdcard/cralbody_mc_test/vorbisd_in.mct", "wb");
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
	mc->outFile = fopen("/sdcard/cralbody_mc_test/vorbisd_out.mct", "wb");
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
	mc->outFile = fopen("/sdcard/cralbody_mc_test/vorbisd_pcm_out.mct", "wb");
#endif

	CodecSpecificData *csd = NULL;

	if (0 < iConfigLen && NULL != pConfig)
	{
		unsigned char *csd_0 = NULL;
		int csd_0_len = 0;
		unsigned char *csd_1 = NULL;
		int csd_1_len = 0;
		csd_0 = find_csd(0x01766f72, pConfig, iConfigLen, csd_0_len);
		csd_1 = find_csd(0x05766f72, pConfig, iConfigLen, csd_1_len);

		if (NULL != csd_0 || NULL != csd_1)
		{
			csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
			if (NULL == csd)
			{
				MC_ERR("malloc failed!");
				*ppUserData = 0;
				Deinit((NXVOID *)mc);
				IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -7);
				return -7;
			}
			STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
			memset(csd, 0x00, sizeof(CodecSpecificData));

			if (NULL != csd_0)
			{
				void *csd0;
				STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->NewDirectByteBuffer");
				csd->jobj_csd0 = Utils::JNI::NewDirectByteBuffer(csd_0_len, csd0);
				STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "NewDirectByteBuffer->");
				memcpy(csd0, csd_0, csd_0_len);
			}

			if (NULL != csd_1)
			{
				void *csd1;
				STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->NewDirectByteBuffer");
				csd->jobj_csd1 = Utils::JNI::NewDirectByteBuffer(csd_1_len, csd1);
				STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "NewDirectByteBuffer->");
				memcpy(csd1, csd_1, csd_1_len);
			}

			mc->ad.codecSpecific = (void *)csd;
		}
	}

	if (NULL != csd && NULL != csd->jobj_csd0)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->setByteBuffer");
		mc->inputFormat->setByteBuffer("csd-0", csd->jobj_csd0);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "setByteBuffer->");
	}

	if (NULL != csd && NULL != csd->jobj_csd1)
	{
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->setByteBuffer");
		mc->inputFormat->setByteBuffer("csd-1", csd->jobj_csd1);
		STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "setByteBuffer->");
	}

	*ppUserData = (NXVOID *)mc;

	//retValue = Audio::Decoder::Init(mc, pFrame, iFrameLen);
	retValue = Audio::Decoder::Init(mc, NULL, 0);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID *)mc);
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

}; // namespace Decoder

}; // namespace OGG

}; // namespace Nex_MC

