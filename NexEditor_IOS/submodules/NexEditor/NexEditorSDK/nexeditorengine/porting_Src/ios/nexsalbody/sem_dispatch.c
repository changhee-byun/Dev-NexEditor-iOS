//
//  sem_dispatch.c
//  nexSalBody
//
//  Created by Simon Kim on 9/15/14.
//
//

#include "sem_platform_api.h"
#include "SALBody_Mem.h"
#include "SALBody_SyncObj.h"

#ifdef SEM_API_DISPATCH
#include <dispatch/dispatch.h>

typedef struct sem_api_struct
{
    dispatch_semaphore_t handle;
	int count;
	int max;
	NEXSALMutexHandle mutex;
} sem_api_context_t;

/*
 * Use iOS Dispatch Semaphore
 * https://developer.apple.com/library/ios/documentation/General/Conceptual/ConcurrencyProgrammingGuide/OperationQueues/OperationQueues.html#//apple_ref/doc/uid/TP40008091-CH102-SW24
 */

/*
 * @return 0 if successful.
 */
static int sem_dispatch_init(SYSSemaHandle *hSem, int ival, int max)
{
    int ret = NEXSAL_SEM_ERROR;
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
    ctx->handle = dispatch_semaphore_create(ival);
    if (ctx->handle != NULL)  {
        ret = NEXSAL_SEM_SUCCESS;
		ctx->count = ival;
		ctx->max = max;
		ctx->mutex = nexSALBody_MutexCreate();
		if (ctx->mutex == NULL) {
			ret = NEXSAL_SEM_ERROR;
			NEXSAL_TRACE("[%s() %d] nexSALBody_MutexCreate() failed", __FUNCTION__, __LINE__ );
			dispatch_release(ctx->handle);
			ctx->handle = NULL;
		}
    } else {
        NEXSAL_TRACE("[%s() %d] dispatch_semaphre_create('%d') failed with no details\n", __FUNCTION__, __LINE__, ival );
    }
    return ret;
}

/*
 * @return 0 if successful.
 */
static int sem_dispatch_deinit(SYSSemaHandle *hSem)
{
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
	nexSALBody_MutexDelete(ctx->mutex);
	ctx->mutex = NULL;

    if(dispatch_semaphore_signal(ctx->handle) != 0)
    {
        dispatch_release(ctx->handle);
    }
    ctx->handle = NULL;
	/* No return code from dispatch_release() but it may block infinitely signal/release calls were not balanced */
    return NEXSAL_SEM_SUCCESS;
}

/*
 * @return 0 always
 */
static int sem_dispatch_release(SYSSemaHandle *hSem)
{
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
	
	/* Support semaphore max in a critical section */
	nexSALBody_MutexLock(ctx->mutex, NEXSAL_INFINITE);
	if ( ctx->max > ctx->count) {
		long ret = dispatch_semaphore_signal(ctx->handle);
		/* This function returns non-zero if a thread is woken. Otherwise, zero is returned. */
		if ( ret == 0 ) {
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 4, "[%s() %d] dispatch_semaphore_signal() didn't wake up any thread\n", __FUNCTION__, __LINE__);
		}
		ctx->count ++;
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 4, "[%s() %d] Semaphore %p count:%d\n", __FUNCTION__, __LINE__, ctx->handle, ctx->count);
	} else {
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 4, "[%s() %d] Discarding semaphore %p count:%d reached max:%d\n", __FUNCTION__, __LINE__, ctx->handle, ctx->count, ctx->max);

	}
	nexSALBody_MutexUnlock(ctx->mutex);
    return NEXSAL_SEM_SUCCESS;
}

/*
 * @return NEXSAL_SUCCESS(0) if successful. NEXSAL_WAIT_TIMEOUT(non-zero) if timedout
 */
static int sem_dispatch_wait(SYSSemaHandle *hSem, unsigned int uTimeoutmsec)
{
	long ret;
    int result = NEXSAL_SEM_SUCCESS;
    sem_api_context_t *ctx = (sem_api_context_t *) hSem->priv;
    
    dispatch_time_t timeout;
	
	if ( uTimeoutmsec == NEXSAL_INFINITE )
	{
        timeout = DISPATCH_TIME_FOREVER;
	}
	else if ( uTimeoutmsec == 0 )
	{
        timeout = DISPATCH_TIME_NOW;
	}
	else
	{
        /* milli sec. -> nano sec. */
        timeout = dispatch_time( DISPATCH_TIME_NOW, uTimeoutmsec * 1000 * 1000 );
	}
    
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 4, "[%s() %d] Semaphore %p count:%d\n", __FUNCTION__, __LINE__, ctx->handle, ctx->count);

	nexSALBody_MutexLock(ctx->mutex, NEXSAL_INFINITE);
	ctx->count--;
	nexSALBody_MutexUnlock(ctx->mutex);
	
    ret = dispatch_semaphore_wait(ctx->handle, timeout);
    if ( ret != 0 )
	{
        result = eNEXSAL_RET_TIMEOUT;
		nexSALBody_MutexLock(ctx->mutex, NEXSAL_INFINITE);
		ctx->count++;
		nexSALBody_MutexUnlock(ctx->mutex);
	}
	
	return result;
}

int sem_api_init(struct SYSSemaStruct *hSem)
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 4, "[%s() %d] Initializing Dispatch Semaphore API.\n", __FUNCTION__, __LINE__);
    hSem->priv = nexSALBody_MemAlloc(sizeof(sem_api_context_t));
    hSem->sem_init = sem_dispatch_init;
    hSem->sem_deinit = sem_dispatch_deinit;
    hSem->sem_release = sem_dispatch_release;
    hSem->sem_wait = sem_dispatch_wait;
    return 0;
}

int sem_api_deinit(struct SYSSemaStruct *hSem)
{
    nexSALBody_MemFree(hSem->priv);
    hSem->priv = NULL;
    hSem->sem_init = NULL;
    hSem->sem_deinit = NULL;
    hSem->sem_release = NULL;
    hSem->sem_wait = NULL;
    return NEXSAL_SEM_SUCCESS;
}
#endif