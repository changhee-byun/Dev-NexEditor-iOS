/******************************************************************************
 * File Name   : sem_named.c
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#include "sem_platform_api.h"
#include "SALBody_task.h"
#include "SALBody_SyncObj.h"
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef SEM_API_NAMED

typedef struct sem_api_struct
{
	sem_t*  handle;
	int maxcount;
    char *szSemName;

} sem_api_context_t;

/*
 * Use iOS POSIX Named Semaphore
 */
static int g_iSemIndex = 0;

const char *_errnoString(int error_no) {
    char *str = "Unknown";
    switch(error_no) {
        case EACCES:
            str = "EACCES";
            break;
        case EEXIST:
            str = "EEXIST";
            break;
        case EINTR:
            str = "EINTR";
            break;
        case EINVAL:
            str = "EINVAL";
            break;
        case EMFILE:
            str = "EMFILE";
            break;
        case ENAMETOOLONG:
            str = "ENAMETOOLONG";
            break;
        case ENFILE:
            str = "ENFILE";
            break;
        case ENOENT:
            str = "ENOENT";
            break;
        case ENOSPC:
            str = "ENOSPC";
            break;
        case EPERM:
            str = "EPERM";
            break;
    }
    return str;
}

static int sem_named_init(SYSSemaHandle *hSem, int ival, int max)
{
    int ret = 0;
    int errno_cp;
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
    
    ctx->handle = SEM_FAILED;
    ctx->maxcount = max;
	ctx->szSemName = (char *)nexSALBody_MemAlloc( 64 );
    
    /* To avoid semaphore open failing case due to:
     *   - Not able to unlink existing named semaphore
     * 1. put uid in the name of the semaphores (different apps with different uid will use different names )
     * 2. Make sure unlink if there is one created with the same uid
     * 3. Create the named semaphore with 'user can read/write' permission
     */
	sprintf( ctx->szSemName, "nexsem_%d_%08d", getuid(), g_iSemIndex++ );
    
	ret = sem_unlink( ctx->szSemName );
    if ( ret != 0 ) {
        errno_cp = errno;
        NEXSAL_TRACE("[%s() %d] sem_unlink('%s') failed:%s(0x%x) %s \n", __FUNCTION__, __LINE__, ctx->szSemName, _errnoString(errno_cp), errno_cp, strerror(errno_cp) );
    }
    
    ret = 0;
    ctx->handle = sem_open( ctx->szSemName, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, ival );
    if ( ctx->handle == SEM_FAILED && errno == EEXIST) {
        ctx->handle = sem_open( ctx->szSemName, 0, 0, ival );
    }
    if ( ctx->handle == SEM_FAILED ) {
        errno_cp = errno;
        NEXSAL_TRACE("[%s() %d] sem_open('%s') failed:%s(0x%x) %s \n", __FUNCTION__, __LINE__, ctx->szSemName, _errnoString(errno_cp), errno_cp, strerror(errno_cp) );
        ret = errno_cp;
    }
    
	if( ret != 0 ) {
		nexSALBody_MemFree(ctx->szSemName);
        ctx->szSemName = 0;
	}
	
	return ret;
}

static int sem_named_deinit(SYSSemaHandle *hSem)
{
    int ret = SEM_FAILED;
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
    if ( ctx->handle != SEM_FAILED && ctx->szSemName != NULL ) {
        ret = sem_close(ctx->handle);
        if ( ret == 0 ) {
            ret = sem_unlink(ctx->szSemName);
        }
        
        if ( ret != 0) {
            int errno_cp = errno;
            NEXSAL_TRACE("[%s() %d] sem_close() or sem_unlink() failed:%s(0x%x) %s \n", __FUNCTION__, __LINE__, _errnoString(errno_cp), errno_cp, strerror(errno_cp) );
        }
        ctx->handle = SEM_FAILED;
		nexSALBody_MemFree(ctx->szSemName);
        ctx->szSemName = 0;
    }
    return ret;
}

static int sem_named_release(SYSSemaHandle *hSem)
{
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
	return sem_post( ctx->handle );
}

static int sem_named_wait(SYSSemaHandle *hSem, unsigned int uTimeoutmsec)
{
	int ret;
	int val;
    
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
	ret = sem_getvalue( ctx->handle, &val );
	
	if ( uTimeoutmsec == NEXSAL_INFINITE )
	{
		ret = sem_wait( ctx->handle );
	}
	else if ( uTimeoutmsec == 0 )
	{
		ret = sem_trywait( ctx->handle );
	}
	else
	{
		//ret = sem_timedwait( &(pSem->handle), &ts );
		// sem_timedwait is not supported.
		int i;
		int iWaitCount = uTimeoutmsec/50;
		
		for ( i = 0, ret = EAGAIN ; ret == EAGAIN && i < iWaitCount ; i++ )
		{
			ret = sem_trywait( ctx->handle );
			nexSALBody_TaskSleep( 50 );
		}
	}
    
    if ( ret != 0 )
	{
		if ( ret == EAGAIN )
			return NEXSAL_WAIT_TIMEOUT;
		else
			return NEXSAL_WAIT_FAILED;
    }
    
	return NEXSAL_SUCCESS;
}

int sem_api_init(struct SYSSemaStruct *hSem)
{
    NEXSAL_TRACE("[%s() %d] Initializing Named Semaphore API.\n", __FUNCTION__, __LINE__);
    hSem->priv = nexSALBody_MemAlloc(sizeof(sem_api_context_t));
    hSem->sem_init = sem_named_init;
    hSem->sem_deinit = sem_named_deinit;
    hSem->sem_release = sem_named_release;
    hSem->sem_wait = sem_named_wait;
    return NEXSAL_SUCCESS;
}

int sem_api_deinit(struct SYSSemaStruct *hSem)
{
    nexSALBody_MemFree(hSem->priv);
    hSem->priv = NULL;
    hSem->sem_init = NULL;
    hSem->sem_deinit = NULL;
    hSem->sem_release = NULL;
    hSem->sem_wait = NULL;
    return NEXSAL_SUCCESS;
}
#endif


