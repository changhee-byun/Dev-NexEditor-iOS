/*
 *  SALBody_SyncObj.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "SALBody_Debug.h"
#include "SALBody_Task.h"
#include "SALBody_SyncObj.h"
#include "SALBody_Mem.h"
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
//
// Semaphore
//

#include "sem_platform_api.h"


NEXSALSemaphoreHandle nexSALBody_SemaphoreCreate( int iInitCount, int iMaxCount )
{
    int ret = 0;
    
	SYSSemaHandle *hSem = (SYSSemaHandle*)nexSALBody_MemCalloc( 1, sizeof(SYSSemaHandle) );
    
    sem_api_init(hSem);
    
    ret = hSem->sem_init(hSem, iInitCount, iMaxCount);
	if( ret != 0 ) {
		nexSALBody_MemFree(hSem);
		return (NEXSALSemaphoreHandle)0; 
	}
	return (NEXSALSemaphoreHandle)hSem;
}

int nexSALBody_SemaphoreDelete( NEXSALSemaphoreHandle hSema )
{
	int ret;
	SYSSemaHandle* pSem = (SYSSemaHandle*)hSema;
	
    ret = pSem->sem_deinit( pSem );
    
    sem_api_deinit(pSem);
    
	nexSALBody_MemFree( pSem );
	
	return ret;
}

int nexSALBody_SemaphoreRelease( NEXSALSemaphoreHandle hSema )
{
	//int val;
	SYSSemaHandle* pSem = (SYSSemaHandle*)hSema;

	return pSem->sem_release( pSem );
}

int nexSALBody_SemaphoreWait( NEXSALSemaphoreHandle hSema, unsigned int uTimeoutmsec )
{
	int result = eNEXSAL_RET_GENERAL_ERROR;
	SYSSemaHandle *pSem = (SYSSemaHandle*)hSema;
	if ( hSema == NEXSAL_INVALID_HANDLE ) {
		nexSALBody_DebugOutputString("Invalid semaphore handle\n");
	} else {
		result = pSem->sem_wait(pSem, uTimeoutmsec);
	}
	return result;
}

//
// Mutex
//

NEXSALMutexHandle nexSALBody_MutexCreate()
{
	pthread_mutex_t *mutex = (pthread_mutex_t*)nexSALBody_MemCalloc( 1, sizeof(pthread_mutex_t) );
	if ( pthread_mutex_init( mutex, NULL ) != 0 )
	{
		nexSALBody_MemFree( mutex );
		return 0;
	}
	
	return (NEXSALMutexHandle)mutex;
}

int nexSALBody_MutexDelete( NEXSALMutexHandle hMutex )
{
	int ret;
	ret = pthread_mutex_destroy( (pthread_mutex_t*)hMutex );
	nexSALBody_MemFree( (void*)hMutex );
	
	return ret;
}

int nexSALBody_MutexLock( NEXSALMutexHandle hMutex, unsigned int uTimeoutmsec )
{
	if ( hMutex == 0 )
	{
		nexSALBody_DebugOutputString("mutexunlock handle is null\n");
		return 1;
	}
	
	return pthread_mutex_lock( (pthread_mutex_t*)hMutex );
}

int nexSALBody_MutexUnlock( NEXSALMutexHandle hMutex )
{
	if ( hMutex == 0 )
	{
		nexSALBody_DebugOutputString("mutexunlock handle is null\n");
		return 1;
	}
	
	return pthread_mutex_unlock( (pthread_mutex_t*)hMutex );
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
    pHandle->sema = 0;
	nexSALBody_MutexUnlock(_Mutex_Variable);
}

static SYSEventHandle* _GetHandle()
{
	int i;
	SYSEventHandle *ret = NULL;
	
	nexSALBody_MutexLock(_Mutex_Variable, NEXSAL_INFINITE );
	
	for( i = 0 ; i < MAX_EVENT_NUM ; i++ )
	{
		if( handleTable[i].sema == 0 )
		{
        	handleTable[i].sema = -1;
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
      		//printf("sysevent.c %d rel id %d\r\n",__LINE__, hEvent);    
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

//extern int g_socket_fd_max;

int nexSALBODY_SyncObjectsInit()
{
	_Mutex_Variable = nexSALBody_MutexCreate();
	//g_socket_fd_max = 0;
	return 0;
}

int nexSALBODY_SyncObjectsDeinit()
{
	nexSALBody_MutexDelete( _Mutex_Variable );
	_Mutex_Variable = 0;
	return 0;
}
