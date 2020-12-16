﻿/******************************************************************************
* File Name        : mpeg2.cpp
* Description      : mpeg2 codec for Android
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
#define LOCATION_TAG "mpeg2"

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
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

namespace Nex_MC {

using Common::NexCAL_mc;

namespace Mpeg2 {

const char *MIMETYPE = "video/mpeg2";

namespace { // (anon1)

typedef struct CodecSpecificData {
	jobject jobj_csd0;
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
#define CODEC_NAME "Mpeg2D"

using Nex_MC::Video::Decoder::NexCAL_mcvd;

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

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = Video::Decoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	unsigned int input = *puValue;

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
		, NXUINT8* pConfig
		, NXINT32 iLen
		, NXUINT8* pConfigEnhance
		, NXINT32 iEnhLen
		, NXVOID *pInitInfo
		, NXVOID *pExtraInfo
		, NXINT32 iNALHeaderLengthSize
		, NXINT32* piWidth
		, NXINT32* piHeight
		, NXINT32* piPitch
		, NXUINT32 uMode
		, NXUINT32 uUserDataType
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X) udtype(0x%X)", eCodecType, uUserDataType);

	NXINT32 retValue = 0;

	NexCAL_mc *mc = Video::Decoder::CreateNexCAL_mc(CODEC_NAME, MIMETYPE, *piWidth, *piHeight, uUserDataType, ppUserData);

	if (NULL == mc)
	{
		MC_ERR("failed to create userData");
		return -1;
	}

	pthread_mutex_lock(&g_preferredDecoderMutex);
	if (!g_checkedForPreferredDecoder)
	{
		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::HW, g_preferredDecoderName);

		if (NULL == g_preferredDecoderName)
		{
			ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::ANY, g_preferredDecoderName);
		}
		g_checkedForPreferredDecoder = true;
	}
	pthread_mutex_unlock(&g_preferredDecoderMutex);

	mc->preferredCodecName = (const char *)g_preferredDecoderName;

	CodecSpecificData *csd = NULL;

	if (0 < iLen)
	{
		csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
		if (NULL == csd)
		{
			MC_ERR("malloc failed!");
			*ppUserData = 0;
			free(mc);
			IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -21);
			return -21;
		}
		STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
		memset(csd, 0x00, sizeof(CodecSpecificData));
		void *csd0;
		csd->jobj_csd0 = Utils::JNI::NewDirectByteBuffer(iLen, csd0);
		memcpy(csd0, pConfig, iLen);

		mc->vd.codecSpecific = (void *)csd;
	}

	if (NULL != csd && NULL != csd->jobj_csd0)
	{
		mc->inputFormat->setByteBuffer("csd-0", csd->jobj_csd0);
	}

	*ppUserData = (void*)mc;

	retValue = Video::Decoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((void*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*piWidth = mc->vd.width;
	*piHeight = mc->vd.height;
	*piPitch = mc->vd.pitch;

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoInit InitF = Init;

NXINT32 Deinit( NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	// call common Deinit
	NXINT32 retValue = Video::Decoder::Deinit(mc);

	CodecSpecificData *csd = (CodecSpecificData *)mc->vd.codecSpecific;

	if (NULL != csd)
	{
		if (NULL != csd->jobj_csd0)
		{
			Utils::JNI::DeleteDirectByteBuffer(csd->jobj_csd0);
		}

		free(csd);
		csd = NULL;
		mc->vd.codecSpecific = NULL;
	}

	free(mc);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoDeinit DeinitF = Deinit;

NXINT32 Decode
		( NXUINT8* pData
		, NXINT32 iLen
		, NXVOID *pExtraInfo		
		, NXUINT32 uDTS
		, NXUINT32 uPTS
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "len(%d) ts(%u/%u) flag(0x%X)", iLen, uDTS, uPTS, nFlag);

	NXINT32 retValue = 0;

	retValue = Video::Decoder::Decode(pData, iLen, uDTS, uPTS, nFlag, puDecodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}
NEXCALVideoDecode DecodeF = Decode;

}; // namespace Decoder

}; // namespace Mpeg2

}; // namespace Nex_MC
