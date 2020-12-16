/******************************************************************************
* File Name        : mp3.cpp
* Description      : mp3 codec for Android
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
#define LOCATION_TAG "mp3"

#undef LOG_TAG
#define LOG_TAG "nexcral_mc"

#include "NexCAL.h"

#include "NexCRALBody_mc_impl.h"
#include "external/NexMediaCodec.h"

#include "external/jni/jni_MediaCodec.h"
#include "external/jni/jni_MediaFormat.h"

#include "utils/utils.h"

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_AUDIO

namespace Nex_MC {

using Common::NexCAL_mc;

namespace MP3 {

const char *MIMETYPE = "audio/mpeg";

namespace Decoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "MP3D"

namespace { // (anon201)
	pthread_mutex_t g_preferredDecoderMutex;
	char *g_preferredDecoderName = NULL;
	bool g_checkedForPreferredDecoder = false;

	//WrapSetProperty g_wrapSetProperty;

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

NXINT32 _SetPropertyReal(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	IDENTITY("+", CODEC_NAME, a_pUserData, "prop(0x%X)", a_uProperty);

	// call common GetProperty
	NXINT32 retValue = Audio::Decoder::SetProperty(a_uProperty, a_qValue, a_pUserData);

	switch ( a_uProperty )
	{
	case NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2:
		if(Utils::ChipType::MTK != Utils::GetChipType() && a_qValue == 1)
			MIMETYPE = "audio/mpeg-L2";
		else
			MIMETYPE = "audio/mpeg";
		break;
	}

	IDENTITY("-", CODEC_NAME, a_pUserData, "ret(0x%X) value(%lld)", retValue, a_qValue);
	return 0;
}

NXINT32 SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
#if 0
	WrapSetProperty::CheckRet ret = g_wrapSetProperty.CheckClientIdExist( a_pUserData );
	if ( WrapSetProperty::NONE_EXIST == ret )
	{
		g_wrapSetProperty.RegisterClientId( a_pUserData );
		ret = WrapSetProperty::ID_ONLY_EXIST;
	}

	if ( WrapSetProperty::ID_ONLY_EXIST == ret )
	{
		g_wrapSetProperty.PushPropertyAndValuePair( a_pUserData, a_uProperty, a_qValue );
	}
	else // ret == USERDATA_EXIST
#endif
	{
		_SetPropertyReal( a_uProperty, a_qValue, a_pUserData );
	}
	
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

isL2_ReTry:
	NexCAL_mc *mc = Audio::Decoder::CreateNexCAL_mc(CODEC_NAME, MIMETYPE, *puSamplingRate, *puNumOfChannels, *puNumOfSamplesPerChannel, uUserDataType, ppUserData);

	if (NULL == mc)
	{
		if(!strcmp(MIMETYPE, "audio/mpeg-L2"))
		{
			MIMETYPE = "audio/mpeg";
			MC_DEBUG("MC layer 2 failed try again, %s", MIMETYPE);
			goto isL2_ReTry;
		}
		else
		{
			MC_ERR("failed to create userData");
			return -1;
		}
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
			MC_WARN("couldn't find non-google mp3 decoder");
		}
		else
		{
			mc->preferredCodecName = (const char *)g_preferredDecoderName;
		}
	}

#if 1 == _NEX_INPUT_DUMP_
	mc->inFile = fopen("/sdcard/cralbody_mc_test/mp3d_in.mct", "wb");
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
	mc->outFile = fopen("/sdcard/cralbody_mc_test/mp3d_out.mct", "wb");
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
	mc->outFile = fopen("/sdcard/cralbody_mc_test/mp3d_pcm_out.mct", "wb");
#endif

#if 0
	NXUINT32 uProperty;
	NXINT64 qValue;
	WrapSetProperty::PopRet pret;
	while ( WrapSetProperty::VALID_PAIR == ( pret = g_wrapSetProperty.PopPropertyAndValuePair( *ppUserData, &uProperty, &qValue ) ) )
	{
		_SetPropertyReal( uProperty, qValue, (NXVOID *)mc );
	}
	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );
#endif
	*ppUserData = (NXVOID*)mc;

	//retValue = Audio::Decoder::Init(mc, pFrame, iFrameLen);
	retValue = Audio::Decoder::Init(mc, NULL, 0);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);

		if(!strcmp(MIMETYPE, "audio/mpeg-L2"))
		{
			MIMETYPE = "audio/mpeg";
			MC_DEBUG("MC layer 2 failed try again, %s", MIMETYPE);
			goto isL2_ReTry;
		}
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

	//g_wrapSetProperty.UnregisterUserData( pUserData );

	// call common Deinit
	NXINT32 retValue = Audio::Decoder::Deinit(mc);

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

	free((NexCAL_mc *)pUserData);

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
		, NXVOID *pExtraInfo
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
#define CODEC_NAME "MP3E"

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
		*puValue = (NXINT64)"TODO:"; // TODO:
		break;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 Deinit(NXVOID *pUserData);

NXINT32 Init
		( NEX_CODEC_TYPE eCodecType
		, NXUINT8** ppConfig
		, NXINT32 * piConfigLen
		, NXUINT32 uSamplingRate
		, NXUINT32 uNumOfChannels
		, NXINT32		   iBitRate
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "eCodecType(0x%X)", eCodecType);

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

	mc->ae.writeOutputFrame = Audio::Encoder::WriteOutputFrame;

	*ppUserData = (NXVOID*)mc;

	retValue = Audio::Encoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*ppConfig = (unsigned char *)NULL;
	*piConfigLen = 0;

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALAudioEncoderInit InitF = Init;

NXINT32 Deinit(NXVOID *pUserData)
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	// call common Deinit
	NXINT32 retValue = Audio::Encoder::Deinit(mc);

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

}; // namespace MP3

}; // namespace Nex_MC

