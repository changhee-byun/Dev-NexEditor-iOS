/******************************************************************************
* File Name   :	SALBody_SyncObj.h
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

#ifndef _SALBODY_SYNCOBJ_HEADER_
#define _SALBODY_SYNCOBJ_HEADER_


#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

//namespace android {


#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#define _NEX_DEFINE_64_
#endif


#ifdef __cplusplus
extern "C" {
#endif
	
NEXSALSemaphoreHandle nexSALBody_SemaphoreCreate( int iInitCount, int iMaxCount );
int nexSALBody_SemaphoreDelete( NEXSALSemaphoreHandle hSema );
int nexSALBody_SemaphoreRelease( NEXSALSemaphoreHandle hSema );
int nexSALBody_SemaphoreWait( NEXSALSemaphoreHandle hSema, unsigned int uTimeoutmsec );

NEXSALMutexHandle nexSALBody_MutexCreate();
int nexSALBody_MutexDelete( NEXSALMutexHandle hMutex );
int nexSALBody_MutexLock( NEXSALMutexHandle hMutex, unsigned int uTimeoutmsec );
int nexSALBody_MutexUnlock( NEXSALMutexHandle hMutex );

NEXSALEventHandle nexSALBody_EventCreate( NEXSALEventReset iManualReset, NEXSALEventInitial iInitialSet );
int nexSALBody_EventDelete( NEXSALEventHandle hEvent );
int nexSALBody_EventSet( NEXSALEventHandle hEvent );
int nexSALBody_EventClear( NEXSALEventHandle hEvent );
int nexSALBody_EventWait( NEXSALEventHandle hEvent, unsigned int uiTimeoutmsec );

#ifdef _NEX_DEFINE_64_
int nexSALBody_AtomicInc( NEXSALMutexHandle hMutex);
int nexSALBody_AtomicDec( NEXSALMutexHandle hMutex );
#else
int nexSALBody_AtomicInc( int* pValue);
int nexSALBody_AtomicDec( int* pValue );
#endif


int nexSALBODY_SyncObjectsInit();
int nexSALBODY_SyncObjectsDeinit();
	
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
