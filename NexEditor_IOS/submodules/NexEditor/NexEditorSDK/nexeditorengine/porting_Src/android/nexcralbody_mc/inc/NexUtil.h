/******************************************************************************
* File Name   : NexUtil.h
* Description : Common media definition.
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2013-2014 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXUTIL_H_INCLUDED_
#define _NEXUTIL_H_INCLUDED_

#include "NexMediaDef.h"

#define NEXUTIL_VERSION_MAJOR	1
#define NEXUTIL_VERSION_MINOR	1
#define NEXUTIL_VERSION_PATCH	0
/*
#if (defined(WIN32) || defined(WINCE))
	#ifdef NEXUTIL_EXPORTS
		#define NEXUTIL_API __declspec(dllexport)
	#else
		#define NEXUTIL_API __declspec(dllimport)
	#endif
#elif (defined(SYMBIAN60))
	#ifdef NEXUTIL_EXPORTS
		#define NEXUTIL_API	EXPORT_C
	#else
		#define NEXUTIL_API	IMPORT_C
	#endif
#else */
	#define NEXUTIL_API
//#endif


#ifdef __cplusplus
extern "C" {
#endif

	NEXUTIL_API const NXCHAR* NexUtil_GetVersion();
	NEXUTIL_API const NXINT32 NexUtil_GetVersionNumber(NXUINT32 a_uIdx);
	NEXUTIL_API const NXCHAR* NexUtil_GetStrFileFormat(IN NEX_FILEFORMAT_TYPE a_eFFType);
	NEXUTIL_API const NXCHAR* NexUtil_GetStrVCodec(IN NEX_CODEC_TYPE a_eCodec);
	NEXUTIL_API const NXCHAR* NexUtil_GetStrACodec(IN NEX_CODEC_TYPE a_eCodec);
	NEXUTIL_API const NXCHAR* NexUtil_GetStrTCodec(IN NEX_CODEC_TYPE a_eCodec);
	NEXUTIL_API NXVOID NexUtil_WStrCpy(IN const NXWCHAR* a_wstrSrc, OUT NXWCHAR* a_wstrDest);
	NEXUTIL_API NXUINT32 NexUtil_WStrLen(IN const NXWCHAR* a_wstrSrc);
	NEXUTIL_API NXINT32 NexUtil_StrCaseCmp(IN const NXCHAR* a_pStr1, IN const NXCHAR* a_pStr2);
	NEXUTIL_API NXINT32 NexUtil_StrNCaseCmp(IN const NXCHAR* a_pStr1, IN const NXCHAR* a_pStr2, IN NXINT32 a_nCnt);
	NEXUTIL_API NXBOOL NexUtil_IsIP(IN NXINT8 *a_str, IN NXUINT32 a_uLen);
	NEXUTIL_API NXBOOL NexUtil_IsNumber(IN NXCHAR *a_str, IN NXUINT32 a_uLen);
	NEXUTIL_API NXUINT16 NexUtil_ChangeEndian16(IN NXUINT8 *a_pData);
	NEXUTIL_API NXUINT32 NexUtil_ChangeEndian32(IN NXUINT8 *a_pData);
	NEXUTIL_API NXBOOL NexUtil_FourCC_CaseCmp(IN NXUINT32 a_uFourCC, IN const NXCHAR *a_szFourCC);
	NEXUTIL_API NXBOOL NexUtil_IsMPEG4AP(IN NXUINT32 a_uFourCC); // FourCC is supposed to arrange to big-endian regardless of system.
	NEXUTIL_API NXINT32 NexUtil_ConvVideoCodecType(IN NEX_CODEC_TYPE a_uInCodecType, IN NXUINT32 a_uInFourCC, IN NEX_FILEFORMAT_TYPE a_eInFFType, OUT NEX_CODEC_TYPE *a_puOutCodecType, OUT NXUINT32 *a_puOutFourCC);

#ifdef __cplusplus
}
#endif

#endif //_NEXUTIL_H_INCLUDED_

