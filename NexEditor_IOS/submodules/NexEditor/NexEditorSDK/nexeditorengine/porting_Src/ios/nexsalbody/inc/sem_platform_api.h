//
//  sem_platform_api.h
//  nexSalBody
//
//  Created by Simon Kim on 9/15/14.
//
//

#ifndef nexSalBody_sem_platform_api_h
#define nexSalBody_sem_platform_api_h

/* FIXME: NexSAL does not define an error constant for semaphore API such as create. We define on internally here which is non-zero */
#define NEXSAL_SEM_ERROR    -1
#define NEXSAL_SEM_SUCCESS   0
/*
 * Note on Semaphore APIs: POSIX Named Semaphore and iOS Dispatch Semaphore
 * - Previous implementation of SALBody Semaphore was based on POSIX Named Semaphore (iOS Platform does not support POSIX Unnamed Semaphore)
 * - POSIX Named Semaphore fails creation with permission error, EPERM, for an unclear reason, every other time.
 * - Use of Dispatch Semaphore is strongly encouraged on this platform, as of iOS7/8
 */

//#define SEM_API_NAMED
#define SEM_API_DISPATCH

#if !defined(SEM_API_NAMED) && !defined(SEM_API_DISPATCH)
#error "Either of SEM_API_NAMED or SEM_API_DISPATCH must be defined"
#endif

typedef struct SYSSemaStruct
{
    void *priv;
    /* The following function pointers will be available once sem_api_init() was successfully called */
    int (*sem_init)(struct SYSSemaStruct *hSem, int ival, int max);
    int (*sem_deinit)(struct SYSSemaStruct *hSem);
    int (*sem_release)(struct SYSSemaStruct *hSem);
    int (*sem_wait)(struct SYSSemaStruct *hSem, unsigned int uTimeoutmsec);
} SYSSemaHandle;

int sem_api_init(struct SYSSemaStruct *hSem);
int sem_api_deinit(struct SYSSemaStruct *hSem);

#endif
