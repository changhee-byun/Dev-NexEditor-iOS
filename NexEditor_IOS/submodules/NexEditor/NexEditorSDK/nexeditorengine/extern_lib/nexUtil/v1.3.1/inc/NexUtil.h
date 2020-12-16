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
#include "NexSAL_Internal.h"

#define NEXUTIL_VERSION_MAJOR	1
#define NEXUTIL_VERSION_MINOR	3
#define NEXUTIL_VERSION_PATCH	1
#define NEXUTIL_VERSION_BRANCH	"OFFICIAL"

#define NEXUTIL_COMPATIBILITY_NUM 2


#define SAFE_FREE(pMem) {if(pMem) {nexSAL_MemFree(pMem);} pMem=0;}

#ifdef __cplusplus
extern "C" {
#endif

	const NXCHAR* NexUtil_GetVersionString();
	const NXINT32 NexUtil_GetMajorVersion();
	const NXINT32 NexUtil_GetMinorVersion();
	const NXINT32 NexUtil_GetPatchVersion();
	const NXCHAR* NexUtil_GetBranchVersion();
	const NXCHAR* NexUtil_GetVersionInfo();
	NXBOOL NexUtil_CheckSameVersion(IN NXINT32 a_nMajor, IN NXINT32 a_nMinor, IN NXINT32 a_nPatch, IN NXCHAR *a_strBranch);
	NXBOOL NexUtil_CheckCompatibleVersion(IN NXINT32 a_nCompatibilityNum);
	const NXCHAR* NexUtil_GetStrFileFormat(IN NEX_FILEFORMAT_TYPE a_eFFType);
	const NXCHAR* NexUtil_GetStrVCodec(IN NEX_CODEC_TYPE a_eCodec);
	const NXCHAR* NexUtil_GetStrACodec(IN NEX_CODEC_TYPE a_eCodec);
	const NXCHAR* NexUtil_GetStrTCodec(IN NEX_CODEC_TYPE a_eCodec);
	NXVOID NexUtil_WStrCpy(IN const NXWCHAR* a_wstrSrc, OUT NXWCHAR* a_wstrDest);
	NXUINT32 NexUtil_WStrLen(IN const NXWCHAR* a_wstrSrc);
	NXINT32 NexUtil_StrCaseCmp(IN const NXCHAR* a_pStr1, IN const NXCHAR* a_pStr2);
	NXINT32 NexUtil_StrNCaseCmp(IN const NXCHAR* a_pStr1, IN const NXCHAR* a_pStr2, IN NXINT32 a_nCnt);
	NXBOOL NexUtil_IsIP(IN NXINT8 *a_str, IN NXUINT32 a_uLen);
	NXBOOL NexUtil_IsNumber(IN NXCHAR *a_str, IN NXUINT32 a_uLen);
	NXUINT16 NexUtil_ChangeEndian16(IN NXUINT8 *a_pData);
	NXUINT32 NexUtil_ChangeEndian32(IN NXUINT8 *a_pData);
	NXBOOL NexUtil_FourCC_CaseCmp(IN NXUINT32 a_uFourCC, IN const NXCHAR *a_szFourCC);
	NXBOOL NexUtil_IsMPEG4AP(IN NXUINT32 a_uFourCC); // FourCC is supposed to arrange to big-endian regardless of system.
	NXINT32 NexUtil_ConvVideoCodecType(IN NEX_CODEC_TYPE a_uInCodecType, IN NXUINT32 a_uInFourCC, IN NEX_FILEFORMAT_TYPE a_eInFFType, OUT NEX_CODEC_TYPE *a_puOutCodecType, OUT NXUINT32 *a_puOutFourCC);
	NXINT32 NexUtil_FrameDump(IN const NXUINT8 *a_pFilePath, IN NEX_CODEC_TYPE a_eCodecType, IN NXUINT8 *a_pData, IN NXUINT32 a_uLen);
	NXVOID NexUtil_ConvHTTPEscape(INOUT NXCHAR *a_pStr);

#ifdef __cplusplus
}
#endif

#endif //_NEXUTIL_H_INCLUDED_

