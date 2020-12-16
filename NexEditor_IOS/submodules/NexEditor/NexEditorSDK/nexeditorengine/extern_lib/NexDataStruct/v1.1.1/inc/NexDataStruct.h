/******************************************************************************
* File Name   : NexDataStruct.h
* Description : Data Structure
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2015 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXDATASTRUCT_H_INCLUDED_
#define _NEXDATASTRUCT_H_INCLUDED_

#include "NexCommonDef.h"
#include "NexSAL_Internal.h"
#include "NexUtil.h"


#define NEXDATASTRUCT_VERSION_MAJOR	1
#define NEXDATASTRUCT_VERSION_MINOR	1
#define NEXDATASTRUCT_VERSION_PATCH	1
#define NEXDATASTRUCT_VERSION_BRANCH	"OFFICIAL"

#define NEXDATASTRUCT_COMPATIBILITY_NUM 2

typedef enum
{
	eDS_RET_SUCCESS				= 0,
	eDS_RET_FAIL				= 1,
	eDS_RET_DUPLICATION			= 2,

	eDS_RET_PADDING				= MAX_SIGNED32BIT
} DS_RET;


#ifdef __cplusplus
extern "C" {
#endif
	const NXCHAR* NexDataStruct_GetVersionString();
	const NXINT32 NexDataStruct_GetMajorVersion();
	const NXINT32 NexDataStruct_GetMinorVersion();
	const NXINT32 NexDataStruct_GetPatchVersion();
	const NXCHAR* NexDataStruct_GetBranchVersion();
	const NXCHAR* NexDataStruct_GetVersionInfo();
	NXBOOL NexDataStruct_CheckSameVersion(IN NXINT32 a_nMajor, IN NXINT32 a_nMinor, IN NXINT32 a_nPatch, IN NXCHAR *a_strBranch);
	NXBOOL NexDataStruct_CheckCompatibleVersion(IN NXINT32 a_nCompatibilityNum);

#ifdef __cplusplus
}
#endif

#endif //_NEXDATASTRUCT_H_INCLUDED_

