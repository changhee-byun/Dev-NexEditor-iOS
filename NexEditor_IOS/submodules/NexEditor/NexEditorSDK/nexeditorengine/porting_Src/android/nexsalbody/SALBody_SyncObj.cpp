/******************************************************************************
* File Name   :	SALBody_SyncObj.cpp
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

#include "SALBody_Mem.h"
#include "SALBody_Debug.h"
#include "SALBody_Task.h"
#include "SALBody_SyncObj.h"
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#ifdef _NEX_DEFINE_64_
#include <stdatomic.h>
#else
#include <sys/atomics.h>
#endif

//
// Semaphore
//

//namespace android {

typedef struct  
{
	sem_t  handle;
	int maxcount;

} SYSSemaHandle;

static int g_iSemIndex = 0;

NEXSALSemaphoreHandle nexSALBody_SemaphoreCreate( int iInitCount, int iMaxCount )
{
	int err_no;
	char szSemName[16];
	SYSSemaHandle *hSem = (SYSSemaHandle*)nexSALBody_MemCalloc( 1, sizeof(SYSSemaHandle) );
	hSem->maxcount = iMaxCount;

	//sprintf( szSemName, "nexs%d", g_iSemIndex++ );

//	sem_unlink( szSemName );
//	hSem->handle = sem_open( szSemName, O_CREAT, S_IRUSR | S_IWUSR, iInitCount );
	int ret = sem_init( &(hSem->handle), 0, iInitCount );
	if ( ret != 0 )
	{
		nexSALBody_MemFree( hSem );
		nexSALBody_DebugPrintf("in SALBody SemaphoreCreate, 0x%X", hSem);	
		return (NEXSALSemaphoreHandle)0; 
	}
	
	return (NEXSALSemaphoreHandle)hSem;
}

int nexSALBody_SemaphoreDelete( NEXSALSemaphoreHandle hSema )
{
	int ret;
	SYSSemaHandle* pSem = (SYSSemaHandle*)hSema;
	
	ret = sem_destroy( &(pSem->handle) );
	nexSALBody_MemFree( pSem );
	
	return ret;
}

int nexSALBody_SemaphoreRelease( NEXSALSemaphoreHandle hSema )
{
	int val;
	SYSSemaHandle* pSem = (SYSSemaHandle*)hSema;

	sem_getvalue( &(pSem->handle), &val );

	if ( val == pSem->maxcount )
	{
		printf("nexSALBody_SemaphoreRelease --- error\n");
		return -1;
	}
	
	return sem_post( &(pSem->handle) );
}

int nexSALBody_SemaphoreWait( NEXSALSemaphoreHandle hSema, unsigned int uTimeoutmsec )
{
	SYSSemaHandle *pSem = (SYSSemaHandle*)hSema;
	int ret, val;
	struct timespec ts;
	struct timeval now;	
		
	//ret = sem_getvalue( &(pSem->handle), &val );
	
	if ( uTimeoutmsec == NEXSAL_INFINITE )
	{
		ret = sem_wait( &(pSem->handle) );
	} 
	else if ( uTimeoutmsec == 0 )
	{
		ret = sem_trywait( &(pSem->handle) );
	}
	else
	{
		gettimeofday(&now, NULL);
		ts.tv_sec = now.tv_sec + uTimeoutmsec / 1000;
		ts.tv_nsec = now.tv_usec * 1000 + (uTimeoutmsec % 1000) * 1000000;

		if(ts.tv_nsec > 1000000000)
		{
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}

		ret = sem_timedwait( &(pSem->handle), &ts );
		// sem_timedwait is not supported.
/*
		int i;
		int iWaitCount = uTimeoutmsec/50;
		
		for ( i = 0, ret = EAGAIN ; ret == EAGAIN && i < iWaitCount ; i++ )
		{
			ret = sem_trywait( pSem->handle );
			nexSALBody_TaskSleep( 50 );
		}
*/
	}
    
    if ( ret != 0 )
	{
		if ( errno == ETIMEDOUT )
			return eNEXSAL_RET_TIMEOUT;
		else
			return eNEXSAL_RET_GENERAL_ERROR;
/*
		if ( ret == EAGAIN )
			return eNEXSAL_RET_TIMEOUT;
		else
			return eNEXSAL_RET_GENERAL_ERROR;
*/
    } 
    
	return eNEXSAL_RET_NO_ERROR;    
}

//
// Mutex
//

typedef struct _NEXSALMutexHandle_Structure
{
#ifdef _NEX_DEFINE_64_
	_Atomic(NXINT32) nMutexCnt;
#else
	NXINT32 nMutexCnt;
#endif
	NXUINT32 uTID;
	pthread_mutex_t pMutex;
} NEXSALMutex_Structure;

NEXSALMutexHandle nexSALBody_MutexCreate()
{
	NEXSALMutex_Structure *pstMutex = (NEXSALMutex_Structure*)nexSALBody_MemCalloc( 1, sizeof(NEXSALMutex_Structure) ,(char*)__FILE__,__LINE__);

#ifdef _NEX_DEFINE_64_
	pstMutex->nMutexCnt = ATOMIC_VAR_INIT(0);
#else
	pstMutex->nMutexCnt = 0;
#endif
	pstMutex->uTID = 0;
	if ( pthread_mutex_init( &pstMutex->pMutex, NULL ) != 0 )
	{
		nexSALBody_MemFree( pstMutex );
		return 0;
	}

	return (NEXSALMutexHandle)pstMutex;
}

int nexSALBody_MutexDelete( NEXSALMutexHandle hMutex )
{
	int ret;
	NEXSALMutex_Structure *pstMutex = (NEXSALMutex_Structure*)hMutex;

	if (pstMutex == 0)
	{
		printf("MutexDelete handle is null\n");
		return 1;
	}

	ret = pthread_mutex_destroy( &pstMutex->pMutex );
	nexSALBody_MemFree( (void*)pstMutex );

	return ret;
}

int nexSALBody_MutexLock( NEXSALMutexHandle hMutex, unsigned int uTimeoutmsec )
{
	int nRet = 0;
	NEXSALMutex_Structure *pstMutex = (NEXSALMutex_Structure*)hMutex;

	if (pstMutex == 0 )
	{
		printf("mutexlock handle is null\n");
		return 1;
	}

	if((NXUINT32)gettid() == pstMutex->uTID)
	{
#ifdef _NEX_DEFINE_64_
		nexSALBody_AtomicInc(pstMutex);
#else
		nexSALBody_AtomicInc(&pstMutex->nMutexCnt);
#endif
		return 0;
	}

	nRet = pthread_mutex_lock( &pstMutex->pMutex );

	pstMutex->uTID = gettid();

#ifdef _NEX_DEFINE_64_
		nexSALBody_AtomicInc(pstMutex);
#else
	nexSALBody_AtomicInc(&pstMutex->nMutexCnt);
#endif
	return nRet;
}

int nexSALBody_MutexUnlock( NEXSALMutexHandle hMutex )
{
	int nPrevCnt = 0;
	NEXSALMutex_Structure *pstMutex = (NEXSALMutex_Structure*)hMutex;

#ifdef _NEX_DEFINE_64_
	if (pstMutex == 0 || atomic_load(&pstMutex->nMutexCnt) == 0)
#else
	if (pstMutex == 0 || pstMutex->nMutexCnt == 0)
#endif
	{
		printf("mutexunlock handle is null\n");
		return 1;
	}

	if((NXUINT32)gettid() != pstMutex->uTID)
	{
		printf("mutexunlock tids are different. no effect!\n");
		return 1;
	}
#ifdef _NEX_DEFINE_64_
	nPrevCnt = nexSALBody_AtomicDec(pstMutex);
#else
	nPrevCnt = nexSALBody_AtomicDec(&pstMutex->nMutexCnt);
#endif
	if(nPrevCnt > 1)
	{
		return 0;
	}

	pstMutex->uTID = 0;

	return pthread_mutex_unlock( &pstMutex->pMutex );
}

//
// Event
//

#define MAX_EVENT_NUM	100

typedef struct
{
	NEXSALSemaphoreHandle 	sema;
	int						manual;
	int						set;
	
} SYSEventHandle;

static SYSEventHandle 	handleTable[MAX_EVENT_NUM];
static NEXSALMutexHandle _Mutex_Variable;

static void _ClearHandle( SYSEventHandle *pHandle )
{
	nexSALBody_MutexLock( _Mutex_Variable, NEXSAL_INFINITE );
	pHandle->sema = NEXSAL_INVALID_HANDLE;
	nexSALBody_MutexUnlock(_Mutex_Variable);
}

static SYSEventHandle* _GetHandle()
{
	int i;
	SYSEventHandle *ret = NULL;
	
	nexSALBody_MutexLock(_Mutex_Variable, NEXSAL_INFINITE );
	
	for( i = 0 ; i < MAX_EVENT_NUM ; i++ )
	{
		if( handleTable[i].sema == NEXSAL_INVALID_HANDLE )
		{
	        	// handleTable[i].sema = NEXSAL_INVALID_HANDLE;
	        	handleTable[i].sema = (NEXSALSemaphoreHandle)1;
	        	ret = &handleTable[i];
	        	break;
		}
	}
    
	nexSALBody_MutexUnlock(_Mutex_Variable);
	return ret;
}

NEXSALEventHandle nexSALBody_EventCreate( NEXSALEventReset iManualReset, NEXSALEventInitial iInitialSet )
{
	SYSEventHandle *pHandle;

	pHandle = _GetHandle();
    if (pHandle == NULL) {
        return NULL;
    }
	pHandle->sema   = nexSALBody_SemaphoreCreate( 1, 1 );
	pHandle->manual = iManualReset;
	pHandle->set = 1;
    
	if ( iInitialSet != NEXSAL_EVENT_SET )
	{
		pHandle->set = 0;
		nexSALBody_SemaphoreWait( pHandle->sema, 0 ); 
	}

	return (NEXSALEventHandle)pHandle;
}

int nexSALBody_EventDelete( NEXSALEventHandle hEvent )
{
	SYSEventHandle *pHandle = (SYSEventHandle *)hEvent;

	nexSALBody_SemaphoreDelete( pHandle->sema );
	_ClearHandle( pHandle );

	return 0;
}

int nexSALBody_EventSet( NEXSALEventHandle hEvent )
{
	SYSEventHandle *pHandle = (SYSEventHandle *)hEvent;

	nexSALBody_MutexLock(_Mutex_Variable, NEXSAL_INFINITE );

	if ( pHandle->set == 0 )
	{
		pHandle->set = 1;
		nexSALBody_SemaphoreRelease( pHandle->sema );
	}

	nexSALBody_MutexUnlock(_Mutex_Variable);

	return 0;
}

int nexSALBody_EventClear( NEXSALEventHandle hEvent )
{
	SYSEventHandle *pHandle = (SYSEventHandle *)hEvent;

	nexSALBody_MutexLock( _Mutex_Variable, NEXSAL_INFINITE );

	if ( pHandle->set == 1 ) {
		pHandle->set = 0;
		nexSALBody_SemaphoreWait( pHandle->sema, NEXSAL_INFINITE ); //If 1 acquire!, else no wait
	}

	nexSALBody_MutexUnlock(_Mutex_Variable);

	return 0;
}

int nexSALBody_EventWait( NEXSALEventHandle hEvent, unsigned int uiTimeoutmsec )
{
	SYSEventHandle *pHandle = (SYSEventHandle *)hEvent;
	int ret;

	//printf( "sysevent.c %d id %d\r\n", __LINE__, hEvent );    
	ret = nexSALBody_SemaphoreWait( pHandle->sema, uiTimeoutmsec );

	nexSALBody_MutexLock( _Mutex_Variable, NEXSAL_INFINITE );

	if ( pHandle->set == 1 )
	{
		if ( pHandle->manual == NEXSAL_EVENT_MANUAL )
		{
			//printf("sysevent.c %d rel id %d\r\n",__LINE__, (NXINT64)hEvent);    
			nexSALBody_SemaphoreRelease( pHandle->sema );	
		} 
		else
		{
			pHandle->set = 0;
		}
	}

	nexSALBody_MutexUnlock(_Mutex_Variable);

	return ret;
}

#ifdef _NEX_DEFINE_64_ 
int nexSALBody_AtomicInc( NEXSALMutexHandle hMutex )
{
	NEXSALMutex_Structure *pstMutex = (NEXSALMutex_Structure*)hMutex;

	if (pstMutex == 0 )
	{
		printf("mutexlock handle is null\n");
		return 1;
	}

	return atomic_fetch_add( &pstMutex->nMutexCnt, 1);
}

int nexSALBody_AtomicDec( NEXSALMutexHandle hMutex )
{
	NEXSALMutex_Structure *pstMutex = (NEXSALMutex_Structure*)hMutex;

	if (pstMutex == 0 )
	{
		printf("mutexlock handle is null\n");
		return 1;
	}
	
	return atomic_fetch_sub( &pstMutex->nMutexCnt, 1);
}
#else
int nexSALBody_AtomicInc( int* pValue )
{
	return __atomic_inc( pValue );
}

int nexSALBody_AtomicDec( int* pValue )
{
	return __atomic_dec( pValue );
}
#endif

//extern int g_socket_fd_max;

int nexSALBODY_SyncObjectsInit()
{
	_Mutex_Variable = nexSALBody_MutexCreate();
	//	g_socket_fd_max = 0;
	return 0;
}

int nexSALBODY_SyncObjectsDeinit()
{
	if(_Mutex_Variable != NEXSAL_INVALID_HANDLE)
		nexSALBody_MutexDelete( _Mutex_Variable );
	_Mutex_Variable = 0;
	return 0;
}

//};

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
ysh		    2008/12/03	Draft.
-----------------------------------------------------------------------------*/
