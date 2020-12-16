/*
 *  SALBody_Debug.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#define LOG_TAG "NEXEDITOR"

#include  <android/log.h>

#include "SALBody_Debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_DEBUG_STRING_LENGTH 512

//namespace android {

void nexSALBody_DebugPrintf( char* pszFormat, ... )
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

void nexSALBody_DebugOutputString( char* pszOutput )
{
    //LOGW( "[Time %u] %s", nexSAL_GetTickCount(), pszOutput );
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "[Time %u] %s", nexSAL_GetTickCount(), pszOutput);
}
//};

