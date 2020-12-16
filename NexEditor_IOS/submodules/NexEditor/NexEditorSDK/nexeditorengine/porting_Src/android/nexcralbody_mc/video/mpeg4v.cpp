/******************************************************************************
* File Name        : mpeg4v.cpp
* Description      : mpeg4v codec for Android
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#define LOCATION_TAG "mpeg4v"

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

namespace Mpeg4V {

const char *MIMETYPE = "video/mp4v-es";

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
#define CODEC_NAME "Mpeg4VD"

using Nex_MC::Video::Decoder::NexCAL_mcvd;

namespace { // (anon201)
	using Nex_MC::Utils::WrapSetProperty;
    
	pthread_mutex_t g_preferredDecoderMutex;
	char *g_preferredDecoderName = NULL;
	bool g_checkedForPreferredDecoder = false;

	WrapSetProperty g_wrapSetProperty;

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

NXINT32  GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	unsigned int retValue = Video::Decoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	unsigned int input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
		*puValue = (unsigned long)"TODO:"; // TODO:
		break;

	// TODO: find real values
	case NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE:
		*puValue = NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE_VOP;
		break;

	case NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME:
		*puValue = NEXCAL_PROPERTY_VIDEO_INIT_WITH_PFRAME_NOT_OK;
		break;

	case NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT:
		*puValue = NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_YES;
		break;

	case NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE:
		*puValue = NEXCAL_PROPERTY_MPEG4_SUPPORT_COREPROFILE_YES;
		break;

	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC:
		*puValue = NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_NO;
		break;

	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT:
		*puValue = NEXCAL_PROPERTY_MPEG4V_SUPPORT_GMC_WRAPPOINT_NO;
		break;

	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE:
		*puValue = NEXCAL_PROPERTY_MPEG4V_SUPPORT_INTERLACE_NO;
		break;

	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL:
		*puValue = NEXCAL_PROPERTY_MPEG4V_SUPPORT_QPEL_NO;
		break;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

static NXINT32 _SetPropertyReal(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	NexCAL_mc *mc = (NexCAL_mc *)a_pUserData;

	switch ( a_uProperty )
	{
	case NEXCAL_PROPERTY_VIDEO_FPS :
			mc->vd.fps = (NXINT32)a_qValue;
		break;
	case NEXCAL_PROPERTY_VIDEO_DECODER_MODE :	
			mc->vd.useSWCodec = (NXINT32)a_qValue;
		break;
	}
	return 0;
}

NXINT32 SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
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
	{
		_SetPropertyReal( a_uProperty, a_qValue, a_pUserData );
	}
	
	//bool bMC = CheckRegistedMC( a_pUserData );
	return 0;
}

NEXCALSetProperty SetPropertyF = SetProperty;

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

	NXUINT32 uProperty;
	NXINT64 qValue;
	WrapSetProperty::PopRet pret;
	while ( WrapSetProperty::VALID_PAIR == ( pret = g_wrapSetProperty.PopPropertyAndValuePair( *ppUserData, &uProperty, &qValue ) ) )
	{
		_SetPropertyReal( uProperty, qValue, (NXVOID *)mc );
	}

	pthread_mutex_lock(&g_preferredDecoderMutex);
	if (!g_checkedForPreferredDecoder)
	{
		NexMediaCodec::CodecClassRequest req = NexMediaCodec::HW;
		if (mc->vd.useSWCodec == true)
		{
			if ((*piWidth)*(*piHeight) > 1920*1088)
			{
				IDENTITY("-", CODEC_NAME, *ppUserData, "Init: it works with google s/w codec (w:%d,h:%d)", *piWidth, *piHeight);
				req = NexMediaCodec::GOOGLE_SW;
			}
		}

		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, req, g_preferredDecoderName);

		if (NULL == g_preferredDecoderName)
		{
			ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::ANY, g_preferredDecoderName);
		}
		//g_checkedForPreferredDecoder = true;
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

	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );
    
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

	g_wrapSetProperty.UnregisterUserData( pUserData );

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

namespace Encoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "Mpeg4VE"

using Nex_MC::Video::Encoder::NexCAL_mcve;

namespace { // (anon101)
	using Nex_MC::Utils::WrapSetProperty;
	
	pthread_mutex_t g_preferredEncoderMutex;
	char *g_preferredEncoderName = NULL;
	bool g_checkedForPreferredEncoder = false;
	int *g_supportedColorFormats = NULL;

	int *g_encoderProfileLevels = NULL;

	WrapSetProperty g_wrapSetProperty;
	
	class CleanUpCrewEncoder {
	public:
		CleanUpCrewEncoder()
		{
			pthread_mutex_init(&g_preferredEncoderMutex, NULL);
		}

		~CleanUpCrewEncoder()
		{
			if (NULL != g_supportedColorFormats)
			{
				free(g_supportedColorFormats);
				g_supportedColorFormats = NULL;
			}

			pthread_mutex_lock(&g_preferredEncoderMutex);
			if (NULL != g_preferredEncoderName)
			{
				free(g_preferredEncoderName);
				g_preferredEncoderName = NULL;
			}
			if (NULL != g_encoderProfileLevels)
			{
				free(g_encoderProfileLevels);
				g_encoderProfileLevels = NULL;
			}
			g_checkedForPreferredEncoder = false;
			pthread_mutex_unlock(&g_preferredEncoderMutex);

			pthread_mutex_destroy(&g_preferredEncoderMutex);
		}
	};

	CleanUpCrewEncoder cleanUp;
} // namespace (anon101)

namespace { // (anon22)
typedef struct CodecSpecificData {
	void * csd0;
} CodecSpecificData;
}; // namespace (anon22)

//namespace { // (anotheranon2)
//	using Common::TranslatedOMXProfile;
//	using Common::TranslatedOMXLevel;
//}

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = Video::Encoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	NXINT64 input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_PROFILE:
		{
			pthread_mutex_lock(&g_preferredEncoderMutex);
			if (!g_checkedForPreferredEncoder)
			{
				int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels);
				g_checkedForPreferredEncoder = true;
			}
			pthread_mutex_unlock(&g_preferredEncoderMutex);
		}
		break;
	}

	bool isLevelCheck = false;
	int profileForLevelCheck = 0;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
		{
			if (NULL == g_preferredEncoderName)
			{
				MC_ERR("couldn't find hw MPEG4V encoder");
				*puValue = (NXINT64)"not found";
			}
			else
			{
				*puValue = (NXINT64)g_preferredEncoderName;
			}
		}
		break;

	case NEXCAL_PROPERTY_MPEG4V_SUPPORT_PROFILE:
		{
			int highestProfile = 0;

			for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
			{
				highestProfile = MAX(highestProfile, g_encoderProfileLevels[i*2 + 1]/*TranslatedOMXProfile(g_encoderProfileLevels[i*2 + 1])*/);
			}

			if (0 == highestProfile)
			{
				highestProfile = 0x01;
			}

			*puValue = highestProfile;
		}
		break;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 GetTranslatedOMXProfileValue(NXINT32 iValue)
{
	NXINT32 iRet = 0x01;
#if 0
	switch (iValue)
	{
		case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN:
			iRet = 0x01/*HEVCProfileMain*/;
			break;
		case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN10:
			iRet = 0x02/*HEVCProfileMain10*/;
			break;
		case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAINSTILLPICTURE:
			iRet = 0x1000/*HEVCProfileMain10HDR10*/;
			break;
	}
#endif
	return iRet;
}

static NXINT32 _SetPropertyReal(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	NexCAL_mc *mc = (NexCAL_mc *)a_pUserData;

	switch ( a_uProperty )
	{
	case NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_PROFILE:
			mc->ve.profile = (NXINT32)a_qValue;
		break;

	case NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_LEVEL:
			mc->ve.level = (NXINT32)a_qValue;
		break;
	}
	return 0;
}

NXINT32 SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
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
	{
		_SetPropertyReal( a_uProperty, a_qValue, a_pUserData );
	}
	
	//bool bMC = CheckRegistedMC( a_pUserData );
	return 0;
}

NEXCALSetProperty SetPropertyF = SetProperty;

NXINT32 Deinit(NXVOID *pUserData);

NXINT32 Init( 
		NEX_CODEC_TYPE eCodecType
		, NXUINT8** ppConfig
		, NXINT32* piConfigLen
		, NXINT32 iQuality
		, NXINT32 iWidth
		, NXINT32 iHeight
		, NXINT32 iPitch
		, NXINT32 iFPS
		, NXINT32 bCBR
		, NXINT32 iBitRate
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X)", eCodecType);

	NXINT32 retValue = 0;
	int	backup_profile=NEXSAL_INFINITE, backup_level=NEXSAL_INFINITE;

	NexCAL_mc *mc = (NexCAL_mc *)malloc(sizeof(NexCAL_mc));
	if (NULL == mc)
	{
		MC_ERR("malloc failed!");
		*ppUserData = 0;
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -1);
		return -1;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(NexCAL_mc));

RETRY_INIT:
	memset(mc, 0x00, sizeof(NexCAL_mc));
	mc->type = NEX_MC_TYPE;
	mc->avtype = NEX_MC_AVTYPE;
	mc->codecName = CODEC_NAME;

	mc->mimeType = MIMETYPE;

	mc->ve.quality = iQuality;
	mc->ve.width = iWidth;
	mc->ve.height = iHeight;
	mc->ve.fps = iFPS;
	mc->ve.cbr = 0 != bCBR;
	mc->ve.bitrate = iBitRate;
	mc->ve.profile = 0;
	mc->ve.colorFormat = 0x13 /* COLOR_FormatYUV420Planar */;

	if (NULL == JNI::JMETHODS::MediaCodec.createInputSurface)
	{
		pthread_mutex_lock(&g_preferredEncoderMutex);
		if (!g_checkedForPreferredEncoder)
		{
			int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels);
			g_checkedForPreferredEncoder = true;
		}
		pthread_mutex_unlock(&g_preferredEncoderMutex);

		mc->preferredCodecName = (const char *)g_preferredEncoderName;

		if (NULL == g_supportedColorFormats)
		{
			NexMediaCodec_using_jni::getSupportedInputColorFormats(MIMETYPE, g_supportedColorFormats);
		}
		if (NULL != g_supportedColorFormats)
		{
			int i = 0;

			if (Nex_MC::Utils::ChipType::EXYNOS4 == Nex_MC::Utils::GetChipType()
				|| Nex_MC::Utils::ChipType::EXYNOS == Nex_MC::Utils::GetChipType())
			{
				for (i = 0; -1 != g_supportedColorFormats[i]; ++i)
				{
					if (0x15 == g_supportedColorFormats[i])
					{
						break;
					}
				}

				if (-1 == g_supportedColorFormats[i])
				{
					MC_WARN("couldn't find valid(0x15) input color format (%d)", i);
					i = 0;
				}
			}

			for ( ; -1 != g_supportedColorFormats[i]; ++i)
			{
				if (0x7f000789 /* COLOR_FormatSurface */ == g_supportedColorFormats[i]
					|| 0x7f000200 == g_supportedColorFormats[i])
				{
					continue;
				}
				else
				{
					mc->ve.colorFormat = g_supportedColorFormats[i];
					break;
				}
			}

			if (-1 == g_supportedColorFormats[i])
			{
				MC_WARN("couldn't find valid input color format (%d)", i);
			}
		}
		else
		{
			MC_WARN("couldn't find valid input color format");
		}
	}
	else
	{
		mc->ve.colorFormat = 0x7f000789 /* COLOR_FormatSurface */;
	}

	// MC_INFO("color selected: %d", mc->ve.colorFormat);

	//mc->ve.writeOutputFrame = WriteOutputFrame;


	// drain property_value pair queue
	NXUINT32 uProperty;
	NXINT64 qValue;
	WrapSetProperty::PopRet pret;
	while ( WrapSetProperty::VALID_PAIR == ( pret = g_wrapSetProperty.PopPropertyAndValuePair( *ppUserData, &uProperty, &qValue ) ) )
	{
		_SetPropertyReal( uProperty, qValue, (NXVOID *)mc );
	}

	if ( backup_profile == NEXSAL_INFINITE && backup_level == NEXSAL_INFINITE )
	{
		backup_profile = mc->ve.profile;
		backup_level = mc->ve.level;

		if ( g_encoderProfileLevels && (mc->ve.profile==0 || mc->ve.level==0) )
		{
			//in case that profile&level is not set
			if (mc->ve.profile == 0)
				mc->ve.profile = g_encoderProfileLevels[1];
			if (mc->ve.level == 0)
				mc->ve.level = g_encoderProfileLevels[2];

			for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
			{
				if (g_encoderProfileLevels[i*2+1] > mc->ve.profile)
				{
					break;
				}
				else if (g_encoderProfileLevels[i*2+1] == mc->ve.profile)
				{
					mc->ve.level = g_encoderProfileLevels[i*2+2];
				}
			}
			MC_WARN("change to mc->ve.profile == 0x%x, mc->ve.level=0x%x\n",mc->ve.profile, mc->ve.level );
		}
	}
	else
	{
		mc->ve.profile = backup_profile;
		mc->ve.level = backup_level;
	}

	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );
	
	*ppUserData = (NXVOID *)mc;

	retValue = Video::Encoder::Init(mc);

	if (0 != retValue)
	{
		if ( backup_profile == 0 ||  backup_level == 0 )
		{
			MC_WARN("video encoder init fail. retry 0x01/0x01. - ret(0x%X)", retValue);
			Video::Encoder::Deinit(mc);
			backup_profile = 0x01;//Simple Profile
			backup_level = 0x01;//Level 0
			goto RETRY_INIT;
		}
		else
		{
			*ppUserData = 0;
			Deinit((NXVOID *)mc);
			IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
			return retValue;
		}
	}

	if (false)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		mc->outTimeout = 1000; // 1ms

		int breakAt0 = 3;

		while (0 > mc->outIndex)
		{
			mc->engWaiting = true;

			if (0 >= breakAt0)
			{
				break;
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
			sched_yield();
			STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
			pthread_mutex_lock(&mc->engLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

			mc->engWaiting = false;

			--breakAt0;
		}

		STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

		*ppConfig = NULL;
		*piConfigLen = 0;

		if (0 > mc->outIndex)
		{
			MC_WARN("timed out waiting for DSI!");
		}
		else
		{
			if (2 != (mc->lastOutputFlags & 2))
			{
				MC_WARN("first output from encoder was not CODEC_CONFIG! No DSI found!");
			}
			else
			{
				CodecSpecificData *csd = NULL;

				csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
				if (NULL == csd)
				{
					MC_ERR("malloc failed!");
					*ppUserData = 0;
					Deinit((NXVOID *)mc);
					IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -21);
					return -21;
				}
				STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
				memset(csd, 0x00, sizeof(CodecSpecificData));
				csd->csd0 = malloc(mc->lastOutputSize);
				if (NULL == csd->csd0)
				{
					MC_ERR("malloc failed!");
					*ppUserData = 0;
					Deinit((NXVOID *)mc);
					IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -22);
					return -22;
				}
				STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd->csd0, mc->lastOutputSize);

				memcpy(csd->csd0, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);

				mc->ve.codecSpecific = (void *)csd;

				*ppConfig = (unsigned char *)csd->csd0;
				*piConfigLen = mc->lastOutputSize;
			}

			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseOutputBuffer");
			mc->mc->releaseOutputBuffer(mc->outIndex, false);
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseOutputBuffer->");
			mc->outIndex = -1;
		}

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		//Nex_MC::Video::Encoder::Reset((NXVOID *)mc);

		mc->outTimeout = mc->defaultOutTimeout;
	}

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoEncoderInit InitF = Init;

NXINT32 Deinit( NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;


	g_wrapSetProperty.UnregisterUserData( pUserData );


	// call common Deinit
	NXINT32 retValue = Video::Encoder::Deinit(mc);

	CodecSpecificData *csd = (CodecSpecificData *)mc->ve.codecSpecific;

	if (NULL != csd)
	{
		if (NULL != csd->csd0)
		{
			free(csd->csd0);
		}

		free(csd);
		csd = NULL;
		mc->ve.codecSpecific = NULL;
	}

	free(mc);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoEncoderDeinit DeinitF = Deinit;

NXINT32 Encode( 
		NXUINT8* pData1
		, NXUINT8* pData2
		, NXUINT8* pData3
	//	, NXUINT8** ppOutData
	//	, NXUINT32* piOutLen
		, NXUINT32 uPTS
		, NXUINT32* puEncodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "pData(%p,%p,%p) uPTS(%u)", pData1, pData2, pData3, uPTS);

	NXINT32 retValue = 0;

	retValue = Video::Encoder::Encode(pData1, pData2, pData3, uPTS, puEncodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
	return retValue;
}
NEXCALVideoEncoderEncode EncodeF = Encode;

} // namespace Encoder
}; // namespace Mpeg4V

}; // namespace Nex_MC
