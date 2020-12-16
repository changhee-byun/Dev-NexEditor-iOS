/******************************************************************************
* File Name   :	SALBody_Debug.h
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

#ifndef _SALBODY_DEBUG_HEADER_
#define _SALBODY_DEBUG_HEADER_

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif

	void nexSALBody_DebugPrintf( const char* pszFormat, ... );
	void nexSALBody_DebugOutputString( const char* pszOutput );
#ifdef __cplusplus
}
#endif

//};
#endif

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
ysh		    2008/12/03	Draft.
-----------------------------------------------------------------------------*/
