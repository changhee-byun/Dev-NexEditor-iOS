/******************************************************************************
* File Name   : NexDataStruct.c
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
#include <string.h>
#include <ctype.h>

#include "NexDataStruct.h"


#define VERSION_STRING_HELPER(a, b, c, d) #a "." #b "." #c "." d
#define VERSION_STRING_MAKER(a, b, c, d) VERSION_STRING_HELPER(a, b, c, d)
#define VERSION_STRING  VERSION_STRING_MAKER(NEXDATASTRUCT_VERSION_MAJOR, NEXDATASTRUCT_VERSION_MINOR, NEXDATASTRUCT_VERSION_PATCH, NEXDATASTRUCT_VERSION_BRANCH)
#define VERSION_INFO    "NexStreaming DataStruct Version Prx " VERSION_STRING " Build Date " __DATE__

const NXCHAR* NexDataStruct_GetVersionString()
{
	return VERSION_STRING;
}

const NXINT32 NexDataStruct_GetMajorVersion()
{
	return NEXDATASTRUCT_VERSION_MAJOR;
}

const NXINT32 NexDataStruct_GetMinorVersion()
{
	return NEXDATASTRUCT_VERSION_MINOR;
}

const NXINT32 NexDataStruct_GetPatchVersion()
{
	return NEXDATASTRUCT_VERSION_PATCH;
}

const NXCHAR* NexDataStruct_GetBranchVersion()
{
	return NEXDATASTRUCT_VERSION_BRANCH;
}

const NXCHAR* NexDataStruct_GetVersionInfo()
{
	return VERSION_INFO;
}

NXBOOL NexDataStruct_CheckSameVersion(IN NXINT32 a_nMajor, IN NXINT32 a_nMinor, IN NXINT32 a_nPatch, IN NXCHAR *a_strBranch)
{
	NXINT32 nRet = 0;

	if (a_nMajor != NEXDATASTRUCT_VERSION_MAJOR)
	{
		return FALSE;
	}

	if (a_nMinor != NEXDATASTRUCT_VERSION_MINOR)
	{
		return FALSE;
	}

	if (a_nPatch != NEXDATASTRUCT_VERSION_PATCH)
	{
		return FALSE;
	}

	if ((NULL == a_strBranch) || (0 != strcmp(NEXDATASTRUCT_VERSION_BRANCH, a_strBranch)))
	{
		return FALSE;
	}

	return TRUE;
}

NXBOOL NexDataStruct_CheckCompatibleVersion(IN NXINT32 a_nCompatibilityNum)
{
	if (a_nCompatibilityNum != NEXDATASTRUCT_COMPATIBILITY_NUM)
	{
		return FALSE;
	}

	return TRUE;
}

