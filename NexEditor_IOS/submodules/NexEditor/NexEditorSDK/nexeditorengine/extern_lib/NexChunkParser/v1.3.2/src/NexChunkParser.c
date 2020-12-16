/******************************************************************************
 * File Name   : NexChunkParser.c
 * Description : Common media definition.
 *******************************************************************************
 
	  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	  PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2010-2014 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#include <string.h>

#include "NexChunkParser.h"

#define VERSION_STRING_HELPER(a, b, c, d) #a "." #b "." #c "." d
#define VERSION_STRING_MAKER(a, b, c, d) VERSION_STRING_HELPER(a, b, c, d)
#define VERSION_STRING  VERSION_STRING_MAKER(NEXCHUNKPARSER_VERSION_MAJOR, NEXCHUNKPARSER_VERSION_MINOR, NEXCHUNKPARSER_VERSION_PATCH, NEXCHUNKPARSER_VERSION_BRANCH)
#define VERSION_INFO    "NexStreaming Chunk Parser Library Version Prx " VERSION_STRING " Build Date " __DATE__

const NXCHAR* NexChunkParser_GetVersionString()
{
	return VERSION_STRING;
}

const NXINT32 NexChunkParser_GetMajorVersion()
{
	return NEXCHUNKPARSER_VERSION_MAJOR;
}

const NXINT32 NexChunkParser_GetMinorVersion()
{
	return NEXCHUNKPARSER_VERSION_MINOR;
}

const NXINT32 NexChunkParser_GetPatchVersion()
{
	return NEXCHUNKPARSER_VERSION_PATCH;
}

const NXCHAR* NexChunkParser_GetBranchVersion()
{
	return NEXCHUNKPARSER_VERSION_BRANCH;
}

const NXCHAR* NexChunkParser_GetVersionInfo()
{
	return VERSION_INFO;
}

NXBOOL NexChunkParser_CheckSameVersion(IN NXINT32 a_nMajor, IN NXINT32 a_nMinor, IN NXINT32 a_nPatch, IN NXCHAR *a_strBranch)
{
	if (a_nMajor != NEXCHUNKPARSER_VERSION_MAJOR)
	{
		return FALSE;
	}

	if (a_nMinor != NEXCHUNKPARSER_VERSION_MINOR)
	{
		return FALSE;
	}

	if (a_nPatch != NEXCHUNKPARSER_VERSION_PATCH)
	{
		return FALSE;
	}

	if ((NULL == a_strBranch) || (0 != strcmp(NEXCHUNKPARSER_VERSION_BRANCH, a_strBranch)))
	{
		return FALSE;
	}

	return TRUE;
}

NXBOOL NexChunkParser_CheckCompatibleVersion(IN NXINT32 a_nCompatibilityNum)
{
	if (a_nCompatibilityNum != NEXCHUNKPARSER_COMPATIBILITY)
	{
		return FALSE;
	}

	return TRUE;
}

