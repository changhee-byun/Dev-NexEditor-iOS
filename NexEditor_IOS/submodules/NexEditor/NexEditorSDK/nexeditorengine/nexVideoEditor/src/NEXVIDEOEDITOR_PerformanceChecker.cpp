/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PerformanceChecker.cpp
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

#include "NEXVIDEOEDITOR_PerformanceChecker.h"
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#include <stdatomic.h>
struct Atomic{

    volatile long& __val;

    Atomic(volatile long& val):__val(val){

    }
};
#else
#include <sys/atomics.h>
#endif

static CPerformanceChecker g_performanceChecker;

void PerformanceChecker_Catch(int errorType)
{
    g_performanceChecker.Catch(errorType);
}

long PerformanceChecker_GetErrorCount(int errorType)
{
    return g_performanceChecker.GetErrorCount(errorType);
}

CPerformanceChecker::CPerformanceChecker()
{
    for (int i = 0; i < PC_ERROR_MAX; i++)
    {
        #if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
            Atomic tmp(m_lCount[i]);
            atomic_init(&tmp, 0);
        #else
            m_lCount[i] = 0;
        #endif
        
    }
}

CPerformanceChecker::~CPerformanceChecker()
{
    //
}

void CPerformanceChecker::Catch(int errorType)
{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
    Atomic tmp(m_lCount[errorType]);
    atomic_fetch_add(&tmp, 1);
#else
    __atomic_inc((int *)&m_lCount[errorType]);
#endif
}

const long CPerformanceChecker::GetErrorCount(int errorType)
{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
    Atomic tmp(m_lCount[errorType]);
    return (long)atomic_load(&tmp);
#else
    return m_lCount[errorType];
#endif
}
