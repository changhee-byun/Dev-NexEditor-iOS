/*
 *  SALBody_Debug.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#define LOG_TAG "NexEditor"

#include <stdio.h>
#include <stdarg.h>
#include "SALBody_Debug.h"

static void _fallback_outstring(const char *p);

#define MAX_DEBUG_STRING_LENGTH 512

static SALBodyOutStringFunction _callback_outstring = _fallback_outstring;

static void _fallback_outstring(const char *p)
{
    /* fallback log printer in case Output String Function is not set */
    printf("%s", p);
}

void SALBody_RegisterOutputStringFunction(SALBodyOutStringFunction pfnoutstring)
{
    _callback_outstring = pfnoutstring;
}

void nexSALBody_DebugPrintf( const char* pszFormat, ... )
{
    va_list va;
    char szBuf[MAX_DEBUG_STRING_LENGTH];
    char *pDebug = szBuf;
    szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
    
    va_start( va, pszFormat );
    vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
    va_end( va );
    
    nexSALBody_DebugOutputString(pDebug);
}

void nexSALBody_DebugPrintf2( const char* pszFormat, ... )
{
    va_list va;
    char szBuf[MAX_DEBUG_STRING_LENGTH];
    char *pDebug = szBuf;
    szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
    
    va_start( va, pszFormat );
    vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
    va_end( va );
    
    nexSALBody_DebugOutputString(pDebug);
}

void nexSALBody_DebugOutputString( const char* pszOutput )
{
    _callback_outstring( pszOutput );
}

