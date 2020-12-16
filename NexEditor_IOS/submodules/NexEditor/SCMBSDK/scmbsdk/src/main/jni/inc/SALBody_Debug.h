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

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif

	void nexSALBody_DebugPrintf( char* pszFormat, ... );
	void nexSALBody_DebugOutputString( char* pszOutput );
#ifdef __cplusplus
}
#endif

//};
#endif
