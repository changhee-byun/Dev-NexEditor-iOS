/******************************************************************************
* File Name   :	SALBody_Debug.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/


#define LOG_TAG "NEXEDITOR"

#if defined(USE_NDK)
#include  <android/log.h>
#else
#include <utils/Log.h>
#endif

#include "SALBody_Debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_DEBUG_STRING_LENGTH 512

//namespace android {

void nexSALBody_DebugPrintf( const char* pszFormat, ... )
{
	va_list va;
	char szBuf[MAX_DEBUG_STRING_LENGTH];
	char *pDebug = szBuf;
	
	szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
	
	va_start( va, pszFormat );
	vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
	va_end( va );
	
    //LOGW( "[Time %u] %s", nexSAL_GetTickCount(), pDebug );
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", pDebug);
}

void nexSALBody_DebugOutputString( const char* pszOutput )
{
    //LOGW( "[Time %u] %s", nexSAL_GetTickCount(), pszOutput );
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[Time %u] %s", nexSAL_GetTickCount(), pszOutput);
}
//};

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
ysh		    2008/12/03	Draft.
-----------------------------------------------------------------------------*/
