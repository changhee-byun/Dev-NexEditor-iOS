/*
 *  SALBody_Debug.h
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _SALBODY_DEBUG_HEADER_
#define _SALBODY_DEBUG_HEADER_

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

#ifdef __cplusplus
extern "C" {
#endif
    typedef void (*SALBodyOutStringFunction)(const char *p);

	
    void nexSALBody_DebugOutputString( const char* pszOutput );
    void nexSALBody_DebugPrintf( char* pszFormat, ... );
    void nexSALBody_DebugPrintf2( const char* pszFormat, ... );
    void SALBody_RegisterOutputStringFunction(SALBodyOutStringFunction pfnoutstring);
#ifdef __cplusplus
}
#endif

#endif
