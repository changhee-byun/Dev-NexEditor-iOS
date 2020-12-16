/*-----------------------------------------------------------------------------
	File Name   :	nexCAL.c
	Description :	See the following code. ^^
 ------------------------------------------------------------------------------

 NexStreaming Confidential Proprietary
 Copyright (C) 2008 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

#if (defined(_WIN8) || defined(WIN32) || defined(WINCE))
	#include "windows.h"
#endif

#include "NexCAL.h"
#include "only4CAL.h"
#include "NexCodecEntry.h"
#include "NexCodecUtil.h"
#include "NexCAL_Wrapper.h"
#include "NexSAL_Internal.h"

#if ( defined(_WIN8) || defined(WIN32) || defined(WINCE))
/*
	for DLL version, _NEX_AL_DLL_ should be defined.
*/
	#define _NEX_AL_DLL_

	#pragma comment(lib, "nexSAL.lib")   

	#if defined(_NEX_AL_DLL_)

BOOL APIENTRY DllMain(HANDLE a_hModule, 
					  DWORD  a_uReason, 
					  LPVOID a_pReserved
					 )
{
	a_hModule = NULL;
	a_pReserved = NULL;

	switch (a_uReason)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

	#endif

#endif

static NEXCALRegistrationInfo* g_pCodecs = 0;

#define VERSION_STRING_HELPER(a, b, c, d) #a "." #b "." #c "." d
#define VERSION_STRING_MAKER(a, b, c, d) VERSION_STRING_HELPER(a, b, c, d)
#define VERSION_STRING  VERSION_STRING_MAKER(NEXCAL_VERSION_MAJOR, NEXCAL_VERSION_MINOR, NEXCAL_VERSION_PATCH, NEXCAL_VERSION_BRANCH)
#define VERSION_INFO    "NexStreaming Codec Adaptation Layer Library Version Prx " VERSION_STRING " Build Date " __DATE__

NEXCAL_API const NXCHAR* nexCAL_GetVersionString()
{
	return VERSION_STRING;
}

NEXCAL_API NXINT32 nexCAL_GetMajorVersion()
{
	return NEXCAL_VERSION_MAJOR;
}

NEXCAL_API NXINT32 nexCAL_GetMinorVersion()
{
	return NEXCAL_VERSION_MINOR;
}

NEXCAL_API NXINT32 nexCAL_GetPatchVersion()
{
	return NEXCAL_VERSION_PATCH;
}

NEXCAL_API const NXCHAR* nexCAL_GetBranchVersion()
{
	return NEXCAL_VERSION_BRANCH;
}

NEXCAL_API const NXCHAR* nexCAL_GetVersionInfo()
{
	return VERSION_INFO;
}

NEXCAL_API NXBOOL nexCAL_CheckSameVersion(const NXINT32 a_nMajor, const NXINT32 a_nMinor, const NXINT32 a_nPatch, const NXCHAR *a_strBranch)
{
	if (a_nMajor != NEXCAL_VERSION_MAJOR)
	{
		return FALSE;
	}

	if (a_nMinor != NEXCAL_VERSION_MINOR)
	{
		return FALSE;
	}

	if (a_nPatch != NEXCAL_VERSION_PATCH)
	{
		return FALSE;
	}

	if ((NULL == a_strBranch) || (0 != strcmp(NEXCAL_VERSION_BRANCH, a_strBranch)))
	{
		return FALSE;
	}

	return TRUE;
}

NEXCAL_API NXBOOL nexCAL_CheckCompatibleVersion(const NXINT32 a_nCompatibilityNum)
{
	if (a_nCompatibilityNum != NEXCAL_COMPATIBILITY_NUM)
	{
		return FALSE;
	}

	return TRUE;
}

// Do Reference count in/dec need a critical section ?
NEXCAL_API NEXCALHandle nexCAL_GetHandle(NXINT32 a_nFrom, const NXINT32 a_nCompatibilityNum)
{
	a_nFrom = 0;

	if (FALSE == nexCAL_CheckCompatibleVersion(a_nCompatibilityNum))
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Failed to check version.(compatibility number : %d)\n", _FUNLINE_, NEXCAL_COMPATIBILITY_NUM);
		return(NXVOID*)0;
	}

	if (g_pCodecs == 0)
	{
		g_pCodecs = (NEXCALRegistrationInfo*)nexSAL_MemAlloc(sizeof(NEXCALRegistrationInfo));
		memset(g_pCodecs, 0, sizeof(NEXCALRegistrationInfo));
	}

	if (!g_pCodecs)	return(NXVOID*)0;

	g_pCodecs->m_uRefCnt++;

	return(NEXCALHandle)g_pCodecs;
}

NEXCAL_API NXVOID nexCAL_ReleaseHandle(NEXCALHandle a_hCAL)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;

	if (g_pCodecs != pCodecs)
	{
		return;
	}

	pCodecs->m_uRefCnt--;

	if (pCodecs->m_uRefCnt == 0)
	{
		NXINT32 i;

		_CE_FreeListAll(pCodecs->m_pStart);

		for (i = 0 ; i < NEX_MEDIA_VALUE ; i++)
		{
			if (pCodecs->m_ahDump[i])
			{
				nexSAL_FileClose(pCodecs->m_ahDump[i]);
				pCodecs->m_ahDump[i] = 0;
			}
		}

		SAFE_FREE(pCodecs);
		g_pCodecs = (NXVOID*)0;
	}

	return;
}

NEXCAL_API NXINT32 nexCAL_GetPolicy(NEXCALHandle a_hCAL, NEXCALPolicy *a_pePolicy)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;

	if (!pCodecs)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] nexCAL_GetPolicy() : Codec Handle is NULL\n", _FUNLINE_);
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	*a_pePolicy = pCodecs->m_ePolicy;

	return NEXCAL_ERROR_NONE;
}

NEXCAL_API NXINT32 nexCAL_SetPolicy(NEXCALHandle a_hCAL, const NEXCALPolicy a_ePolicy)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;
	if (!pCodecs)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] nexCAL_SetPolicy() : Codec Handle is NULL\n", _FUNLINE_);
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	pCodecs->m_ePolicy = a_ePolicy;	// need checking range
	return NEXCAL_ERROR_NONE;
}

NEXCAL_API NXINT32 nexCAL_RegisterCodec(NEXCALHandle a_hCAL, 
										NEXCALMediaType a_eMediaType, 
										NEXCALMode a_eMode, 
										NEX_CODEC_TYPE a_eCodecType, 
										NEXCALOnLoad a_pfnOnLoad, 
										NXVOID *a_pUserData, 
										NXUINT32 a_uStaticProperties)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;
	NEXCALCodecEntry *pNew = _CE_New();

	pNew->m_eType = a_eMediaType;
	pNew->m_eMode = a_eMode;
	pNew->m_eCodecType = a_eCodecType;
	pNew->m_uStaticProperties = a_uStaticProperties;

	if (a_pfnOnLoad)
	{
		pNew->m_pOnLoad = a_pfnOnLoad;
		pNew->m_pUserDataForOnLoad = a_pUserData;

		if (-1 == _CE_Add(&(pCodecs->m_pStart), pNew))
		{
			return NEXCAL_ERROR_NONE;
		}
	}

	return NEXCAL_ERROR_FAIL;
}

//kwangsik.lee 2012-02-03
NEXCAL_API NXINT32 nexCAL_UnregisterCodec(NEXCALHandle a_hCAL, 
										  NEXCALMediaType a_eMediaType, 
										  NEXCALMode a_eMode, 
										  NEX_CODEC_TYPE a_eCodecType, 
										  NEXCALOnLoad a_pfnOnLoad, 
										  NXVOID *a_pUserData)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;
	NEXCALCodecEntry *pEntry = NULL;

	while (1)
	{
		pEntry = _CE_Find((pEntry ? pEntry->m_pNext : pCodecs->m_pStart), a_eMediaType, a_eMode, a_eCodecType);

		if (!pEntry)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] nexCAL_GetCodec() : Codec Handle not be found [eMediaType = %d, eMode = %d, eCodecType = %d]\n", _FUNLINE_, a_eMediaType, a_eMode, a_eCodecType);
			return NEXCAL_ERROR_FAIL;
		}

		//check same entry
		if (pEntry->m_pOnLoad == a_pfnOnLoad && pEntry->m_pUserDataForOnLoad == a_pUserData)
		{
			NXINT32 iRet = _CE_Remove(&(pCodecs->m_pStart), pEntry);

			if (NEXCAL_ERROR_NONE != iRet)
			{
				return NEXCAL_ERROR_FAIL;
			}
			else
			{
				return NEXCAL_ERROR_NONE;
			}
		}
	}

	return NEXCAL_ERROR_FAIL;
}

NEXCAL_API NEXCALCodecHandle nexCAL_GetCodec(NEXCALHandle a_hCAL, 
											 NEXCALMediaType a_eMediaType, 
											 NEXCALMode a_eMode, 
											 NEX_CODEC_TYPE a_eCodecType , 
											 NXUINT8 *a_pConfig, 
											 NXINT32 a_nConfigLen, 
											 NXUINT32 a_uFlag, 
											 NEXCALMethod a_eMethod,
											 NEXCALCodecHandle a_hAfterThis, 
											 NXVOID *a_pClientID)
{
	NEXCALCodecHandle hCodec = NULL;
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;
	NEXCALCodecEntry *pEntry = NULL;
	NXINT32 nError;
	NXBOOL bDone = FALSE;
	NXBOOL bHWOverSpec = FALSE;
	NXUINT32 uCodecErrorInfo = 0;

	if (a_eMediaType == NEXCAL_MEDIATYPE_VIDEO || a_eMediaType == NEXCAL_MEDIATYPE_VIDEO_EXTRA)
	{
		pCodecs->m_uVideoCodecLastErrorInfo = 0;
	}

	if (a_hAfterThis)
	{
		pEntry = a_hAfterThis->m_pEntry;
	}

	do
	{
		pEntry = _CE_Find((pEntry ? pEntry->m_pNext : pCodecs->m_pStart), a_eMediaType, a_eMode, a_eCodecType);

		if (!pEntry)
		{
			SAFE_FREE(hCodec);

			if (a_eMediaType == NEXCAL_MEDIATYPE_VIDEO || a_eMediaType == NEXCAL_MEDIATYPE_VIDEO_EXTRA)
			{
				if (uCodecErrorInfo != 0)
				{
					pCodecs->m_uVideoCodecLastErrorInfo = uCodecErrorInfo;
				}
				else
				{
					pCodecs->m_uVideoCodecLastErrorInfo = NEXCAL_ERROR_NOT_SUPPORT_VIDEO_CODEC;
				}
			}

			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] nexCAL_GetCodec() : Codec Handle not be found [eMediaType = %d, eMode = %d, eCodecType = %d]\n", _FUNLINE_, a_eMediaType, a_eMode, a_eCodecType);

			return(NEXCALCodecHandle)0;
		}

		if (hCodec == NULL)
		{
			hCodec = (NEXCALCodecHandle)nexSAL_MemAlloc(sizeof(struct NEXCALCodecHandle_struct));
			memset(hCodec, 0x0, sizeof(struct NEXCALCodecHandle_struct));
		}

		bDone = TRUE;

		if (a_eMethod == NEXCAL_METHOD_HW)
		{
			if (!NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC))
			{
				bDone = FALSE;
				continue;
			}
		}
		else if (a_eMethod == NEXCAL_METHOD_PLATFORM_SW)
		{
			if (!NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC)
				|| !NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
			{
				bDone = FALSE;
				continue;	
			}
		}
		else if (a_eMethod == NEXCAL_METHOD_OWN_SW)
		{
			if (!NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC)
				|| NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
			{
				bDone = FALSE;
				continue;	
			}			
		}
		else if (a_eMethod == NEXCAL_METHOD_FOLLOW_POLICY)
		{
		if (NEXCAL_POLICY_HWSW == pCodecs->m_ePolicy
			&& TRUE == bHWOverSpec
				&& !NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC))
		{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] nexCAL_GetCodec() : Codec Handle was found.\
								But not use sw codec according to Codecs policy", _FUNLINE_);
			bDone = FALSE;
			continue;           
		}
		}

		if (0 != (nError = pEntry->m_pOnLoad(FALSE, a_eMediaType, a_eMode, a_eCodecType, pEntry->m_uStaticProperties,
											pEntry->m_pUserDataForOnLoad, a_pClientID, &hCodec->m_cfs, sizeof(hCodec->m_cfs), 
											&hCodec->m_fnQueryHandler, &hCodec->m_pUserDataForQueryHandler)))
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] nexCAL_GetCodec() : Codec Handle was found. But Loading failed. [nError = %d, m_pOnLoad= 0x%p, pUserData = 0x%p, eCodecType = %d]\n", _FUNLINE_, nError, pEntry->m_pOnLoad, pEntry->m_pUserDataForOnLoad, a_eCodecType);
			uCodecErrorInfo = nError; //error value
			bDone = FALSE;
			continue;
		}

		if (hCodec->m_fnQueryHandler)
		{
			if (a_uFlag & NEXCAL_GETCODEC_FLAG_INSTANT_OUTPUT)
			{
				if (0 == hCodec->m_fnQueryHandler(NEXCAL_QUERY_INSTANT_OUTPUT, a_eMediaType, a_eMode, a_eCodecType, hCodec->m_pUserDataForQueryHandler, a_pConfig, a_nConfigLen, 0, 0))
				{
					bDone = FALSE;
					continue;
				}
			}

			if (0 != (nError = hCodec->m_fnQueryHandler(NEXCAL_QUERY_CLUSTERED, a_eMediaType, a_eMode, a_eCodecType, hCodec->m_pUserDataForQueryHandler, a_pConfig, a_nConfigLen, 0, 0)))
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[%s %d] nexCAL_GetCodec() : Codec Handle was loaded. But BlackListHandler return failure. [nError = %d, eCodecType = %d]\n", _FUNLINE_, nError, a_eCodecType);

				if (NEXCAL_ERROR_OVER_SPEC == nError && NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC))
				{
					bHWOverSpec = TRUE;
				}
				else if (NEXCAL_ERROR_UNSUPPORTED_RESOLUTION == nError && NEXCAL_CHECK_STATIC_PROPERTY(pEntry->m_uStaticProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC))
				{
					pEntry->m_pOnLoad(TRUE, pEntry->m_eType, pEntry->m_eMode, pEntry->m_eCodecType, pEntry->m_uStaticProperties, 
									pEntry->m_pUserDataForOnLoad, a_pClientID, NULL, 0, NULL, NULL);
					pCodecs->m_uVideoCodecLastErrorInfo = nError;
					SAFE_FREE(hCodec->m_pUserFrame);
					SAFE_FREE(hCodec);

					return(NEXCALCodecHandle)0;
				}

				uCodecErrorInfo = nError; //error value
				bDone = FALSE;
			}
			else
			{
				hCodec->m_uOutBufferFormat = hCodec->m_fnQueryHandler(NEXCAL_QUERY_OUTPUTBUFFER_TYPE, a_eMediaType, a_eMode, a_eCodecType, hCodec->m_pUserDataForQueryHandler, a_pConfig, a_nConfigLen, 0, 0);
				hCodec->m_uAVCMaxProfile = hCodec->m_fnQueryHandler(NEXCAL_QUERY_AVC_MAX_PROFILE, a_eMediaType, a_eMode, a_eCodecType, hCodec->m_pUserDataForQueryHandler, a_pConfig, a_nConfigLen, 0, 0);
			}
		}

		if (bDone)
		{
			hCodec->m_eCodecType = a_eCodecType;
			hCodec->m_hCAL = a_hCAL;
			hCodec->m_uProperties = pEntry->m_uStaticProperties;
			hCodec->m_pEntry = (NXVOID*)pEntry;
		}

	} while (FALSE == bDone);

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[%s %d] nexCAL_GetCodec() : [eMediaType = %d]\n", _FUNLINE_, a_eMediaType);
	if (a_eMediaType == NEXCAL_MEDIATYPE_VIDEO || a_eMediaType == NEXCAL_MEDIATYPE_VIDEO_EXTRA)
	{
		if (uCodecErrorInfo != 0)
		{
			pCodecs->m_uVideoCodecLastErrorInfo = uCodecErrorInfo;
		}
	}
	return(NEXCALCodecHandle)hCodec;
}

NEXCAL_API NXVOID nexCAL_ReleaseCodec(NEXCALCodecHandle a_hCodec, NXVOID *a_pClientID)
{
	if (!a_hCodec)
	{
		return;
	}

	if (a_hCodec->m_pEntry)
	{
		NEXCALCodecEntry *pEntry = a_hCodec->m_pEntry;

		if (pEntry->m_pOnLoad)
		{
			pEntry->m_pOnLoad(TRUE, pEntry->m_eType, pEntry->m_eMode, pEntry->m_eCodecType, pEntry->m_uStaticProperties,
							pEntry->m_pUserDataForOnLoad, a_pClientID, NULL, 0, NULL, NULL);
		}
	}

	SAFE_FREE(a_hCodec->m_pUserFrame);
	SAFE_FREE(a_hCodec);
}

NEXCAL_API NXUINT32 nexCAL_GetCodecLastErrorInfo(NEXCALHandle a_hCAL ,NEXCALMediaType a_eMediaType)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;

	if (a_eMediaType == NEXCAL_MEDIATYPE_VIDEO || a_eMediaType == NEXCAL_MEDIATYPE_VIDEO_EXTRA)
	{
		return pCodecs->m_uVideoCodecLastErrorInfo;
	}
	else
	{
		return NEXCAL_ERROR_NONE;
	}
}

NEXCAL_API NXVOID nexCAL_DumpFrames(NEXCALHandle a_hCAL, const NXCHAR *a_strFileName, NEXCALMediaType a_eMediaType, NXVOID *a_pReserved)
{
	NEXCALRegistrationInfo* pCodecs = (NEXCALRegistrationInfo*)a_hCAL;

	a_pReserved = NULL;

	if (pCodecs->m_ahDump[a_eMediaType])
	{
		nexSAL_FileClose(pCodecs->m_ahDump[a_eMediaType]);
		pCodecs->m_ahDump[a_eMediaType] = 0;
	}

	if (a_strFileName)
	{
		pCodecs->m_ahDump[a_eMediaType] = nexSAL_FileOpenA((NXCHAR*)a_strFileName, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
	}
}

static const NXCHAR* _GetStringFromCodec(NEX_CODEC_TYPE a_eCodecType)
{
	switch (a_eCodecType)
	{
		case eNEX_CODEC_V_MPEG4V        : return "MPEG4V";
		case eNEX_CODEC_V_H263      : return "H.263";
		case eNEX_CODEC_V_H264      : return "H.264";
		case eNEX_CODEC_V_SORENSONH263      : return "S.263";
		case eNEX_CODEC_V_WMV       : return "WMV";
		case eNEX_CODEC_V_WMV1      : return "WMV1";
		case eNEX_CODEC_V_WMV2      : return "WMV2";
		case eNEX_CODEC_V_WMV3      : return "WMV3";
		case eNEX_CODEC_V_WVC1      : return "WVC1";
		case eNEX_CODEC_V_MSMPEG4V3     : return "MP43";
		case eNEX_CODEC_V_REALV         : return "RV";
		case eNEX_CODEC_V_REALV30       : return "RV30";
		case eNEX_CODEC_V_REALV40       : return "RV40";
		case eNEX_CODEC_V_DIVX      : return "DIVX";
		case eNEX_CODEC_V_MPEG2V        : return "MPEG2";
		case eNEX_CODEC_V_VP6           : return "VP6";
		case eNEX_CODEC_V_VP7           : return "VP7";
		case eNEX_CODEC_V_VP8           : return "VP8";
		case eNEX_CODEC_V_THEORA        : return "THEORA";
		case eNEX_CODEC_V_MJPEG     : return "MJPG";
		case eNEX_CODEC_V_MSMPEG4V2 : return "MSMPEG4V2";

		case eNEX_CODEC_V_JPEG      : return "JPEG";

		case eNEX_CODEC_A_AAC           : return "AAC";
		case eNEX_CODEC_A_AACPLUS   : return "AAC_PLUS";
		case eNEX_CODEC_A_AACPLUSV2 : return "AAC_PLUS_V2";
		case eNEX_CODEC_A_MP3           : return "MP3";
		case eNEX_CODEC_A_BSAC      : return "BSAC";
		case eNEX_CODEC_A_WMA           : return "WMA";
		case eNEX_CODEC_A_WMA1          : return "WMA1";
		case eNEX_CODEC_A_WMA2          : return "WMA2";
		case eNEX_CODEC_A_WMA3          : return "WMA3";
		case eNEX_CODEC_A_WMA_LOSSLESS  : return "WMA_LOSSLESS";
		case eNEX_CODEC_A_WMA_SPEECH    : return "WMA_VOICE";
		case eNEX_CODEC_A_WMASPDIF      : return "WMA_SPDIF";
		case eNEX_CODEC_A_REALA         : return "RA";
		case eNEX_CODEC_A_REAL14_4      : return "REAL14_4";
		case eNEX_CODEC_A_REAL28_8      : return "REAL28_8";
		case eNEX_CODEC_A_ATRC          : return "ATRC";
		case eNEX_CODEC_A_COOK          : return "COOK";
		case eNEX_CODEC_A_DNET          : return "DNET";
		case eNEX_CODEC_A_SIPR          : return "SIPR";
		case eNEX_CODEC_A_RAAC          : return "RAAC";
		case eNEX_CODEC_A_RACP          : return "RACP";
		case eNEX_CODEC_A_AC3           : return "AC3";
		case eNEX_CODEC_A_EAC3          : return "EC2";
		case eNEX_CODEC_A_DRA           : return "DRA";
		case eNEX_CODEC_A_FLAC          : return "FLAC";
		case eNEX_CODEC_A_APE           : return "APE"; 
		case eNEX_CODEC_A_PCM_S16LE     : return "PCM_S16LE";
		case eNEX_CODEC_A_PCM_S16BE     : return "PCM_S16BE";
		case eNEX_CODEC_A_PCM_RAW       : return "PCM_RAW";
		case eNEX_CODEC_A_PCM_FL32LE    : return "PCM_FL32LE";
		case eNEX_CODEC_A_PCM_FL32BE    : return "PCM_FL32BE";
		case eNEX_CODEC_A_PCM_FL64LE    : return "PCM_FL64LE";
		case eNEX_CODEC_A_PCM_FL64BE    : return "PCM_FL64BE";
		case eNEX_CODEC_A_PCM_IN24LE    : return "PCM_IN24LE";
		case eNEX_CODEC_A_PCM_IN24BE    : return "PCM_IN24BE";
		case eNEX_CODEC_A_PCM_IN32LE    : return "PCM_IN32LE";
		case eNEX_CODEC_A_PCM_IN32BE    : return "PCM_IN32BE";
		case eNEX_CODEC_A_PCM_LPCMLE    : return "PCM_LPCMLE";
		case eNEX_CODEC_A_PCM_LPCMBE    : return "PCM_LPCMBE";
		case eNEX_CODEC_A_VORBIS        : return "VORBIS";
		case eNEX_CODEC_A_DTS           : return "DTS";
		case eNEX_CODEC_A_MP2           : return "MP2";
		case eNEX_CODEC_A_ADPCM_MS  : return "MS_ADPCM";
		case eNEX_CODEC_A_ADPCM_IMA_WAV : return "IMA_ADPCM";
		case eNEX_CODEC_A_ADPCM_SWF : return "SWF_ADPCM";
		case eNEX_CODEC_A_BLURAYPCM     : return "BD_LPCM";
		case eNEX_CODEC_A_ALAC      : return "ALAC";
		case eNEX_CODEC_A_GSM610        : return "GSM610";

		case eNEX_CODEC_A_AMR           : return "AMR";
		case eNEX_CODEC_A_EVRC      : return "EVRC";
		case eNEX_CODEC_A_QCELP     : return "QCELP";
		case eNEX_CODEC_A_QCELP_ALT : return "QCELP_ALT";
		case eNEX_CODEC_A_SMV           : return "SMV";
		case eNEX_CODEC_A_AMRWB     : return "AMRWB";
		case eNEX_CODEC_A_G711_ALAW : return "G711_ALAW";
		case eNEX_CODEC_A_G711_MULAW    : return "G711_MULAW";
		case eNEX_CODEC_A_G723      : return "G723";

		case eNEX_CODEC_T_CC_CEA_608    : return "TEXT_CEA608";
		case eNEX_CODEC_T_CC_CEA_708    : return "TEXT_CEA708";
		case eNEX_CODEC_T_CC_CEA    : return "TEXT_CC_GENERAL";
		case eNEX_CODEC_T_3GPP  : return "TEXT_3GPP";
		case eNEX_CODEC_T_TTML  : return "TEXT_TTML";
		case eNEX_CODEC_T_UTF8  : return "TEXT_UTF8";
		case eNEX_CODEC_T_WEB_VTT   : return "TEXT_WEBVTT";

		default                 : return "IDK";
	}
}

NEXCAL_API NXVOID nexCAL_PrintAllCALEntries(NEXCALHandle a_hCAL)
{
	NEXCALRegistrationInfo *pCodecs = (NEXCALRegistrationInfo*)a_hCAL;
	NEXCALCodecEntry *pEntry;
	NXINT32 i, nIthType = 0, nNumOfSameType = 0;

	for (i = 0, pEntry = pCodecs->m_pStart ; pEntry ; i++, pEntry = pEntry->m_pNext)
	{
		if (0 == nNumOfSameType)
		{
			nNumOfSameType = pEntry->m_nNumOfSameType;
			nIthType++;
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[%s %d] %02d %02d %s %c %08X 0x%08p %s\n"
						, _FUNLINE_
						, i
						, nIthType
						, (NEXCAL_MODE_ENCODER == pEntry->m_eMode) ? "ENC" : "DEC"
						, (NEXCAL_MEDIATYPE_AUDIO == pEntry->m_eType) ? 'A' : (NEXCAL_MEDIATYPE_TEXT == pEntry->m_eType ? 'T' : 'V')
						, pEntry->m_uStaticProperties
						, pEntry->m_pUserDataForOnLoad
						, _GetStringFromCodec(pEntry->m_eCodecType)
					   );

		nNumOfSameType--;
	}
}

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-04-07	Draft.
 ysh		07-06-10	Introduce wrapper functions for additional services.
 ----------------------------------------------------------------------------*/

