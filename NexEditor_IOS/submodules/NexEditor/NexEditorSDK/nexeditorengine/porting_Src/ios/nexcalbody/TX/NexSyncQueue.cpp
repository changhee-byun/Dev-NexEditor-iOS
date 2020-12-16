//
//  NexSyncQueue.cpp
//  test_codec
//
//  Created by YooSunghyun on 4/5/16.
//  Copyright © 2016 YooSunghyun. All rights reserved.
//

#include <string.h>
#include <stdlib.h>
#include "NexSyncQueue.h"
#include "NexSAL_Internal.h"
NexSyncQueue::NexSyncQueue()
: mSemEmpty(NULL)
, mSemFilled(NULL)
, mIndexForEmpty(0)
, mIndexForFilled(0)
, mInitialized(false)
, mFilledCount(0)
{
    
}

NexSyncQueue::~NexSyncQueue()
{
    
}



int NexSyncQueue::init(void**pBuffers,int nCount)
{
    mBuffers = pBuffers;
    mBufferCount = nCount;
    
    mIndexForEmpty = 0;
    mIndexForFilled = 0;
    
    mSemEmpty = dispatch_semaphore_create(nCount);
    if ( NULL == mSemEmpty )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "SyncQueue::init() fail. sema create.");
        return 1;
    }
    
    //dispatch_semaphore_wait(mSemEmpty,DISPATCH_TIME_FOREVER);
    
    mSemFilled = dispatch_semaphore_create(0);
    if ( NULL == mSemFilled )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "SyncQueue::init() fail. sema create.");
        return 1;
    }
    
    mFilledCount = 0;
    mInitialized = true;
    
    return 0;
};

void NexSyncQueue::deinit()
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "SyncQueue::deinit() enter. (%d)", mFilledCount);

    if ( mSemFilled )
    {
        for ( int i = 0 ; i < mBufferCount ; i++ )
            dispatch_semaphore_signal(mSemFilled);
        dispatch_release(mSemFilled);
        mSemFilled = NULL;
    }
    
    if ( mSemEmpty )
    {
        for ( int i = 0 ; i < mBufferCount ; i++ )
            dispatch_semaphore_signal(mSemEmpty);
        dispatch_release(mSemEmpty);
        mSemEmpty = NULL;
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "SyncQueue::deinit() end. mFilledCount=%d", mFilledCount);
    
    mInitialized = false;
}

long NexSyncQueue::internalWait( dispatch_semaphore_t sem, long timeInMS )
{
    dispatch_time_t timeout;
    timeout = dispatch_time( DISPATCH_TIME_NOW, timeInMS * 1000 * 1000 );
    
    long ret = dispatch_semaphore_wait( sem, timeout );
    if ( ret != 0 )
    {
        //nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "SyncQueue::getEmpty() dispatch_semaphore_wait fail.(%d).", ret);
        return 1;
    }
    
    return 0;
}

void* NexSyncQueue::getEmpty()
{
    if ( 0 != internalWait( mSemEmpty ) )
        return NULL;
    
    void* return_value = mBuffers[mIndexForEmpty];
    mIndexForEmpty = ( mIndexForEmpty + 1 ) % mBufferCount;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "getEmpty, I-E: %d",mIndexForEmpty);
    
    return return_value;
}

void NexSyncQueue::markFilled()
{
    mFilledCount++;
    dispatch_semaphore_signal( mSemFilled );
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "markFilled");
}

void* NexSyncQueue::getFilled()
{
    if ( 0 != internalWait( mSemFilled ) )
        return NULL;
    
    void* return_value = mBuffers[mIndexForFilled];
    mIndexForFilled = ( mIndexForFilled + 1 ) % mBufferCount;
    mFilledCount--;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "getFilled, I-F: %d (%d)",mIndexForFilled, mFilledCount);
    return return_value;
}

void NexSyncQueue::markEmpty()
{
    dispatch_semaphore_signal( mSemEmpty );
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "markEmpty");
}

NexSyncNamedQueue::NexSyncNamedQueue()
: mName(NULL)
{
}

NexSyncNamedQueue::~NexSyncNamedQueue()
{

}

int NexSyncNamedQueue::init( void**pBuffers,int nCount, const char* pName )
{
    mName = (char*)malloc(strlen(pName)+1);
    strcpy( mName, pName );
    
    return NexSyncQueue::init( pBuffers, nCount );
}

int NexSyncNamedQueue::init( void**pBuffers, int nCount )
{
    return init( pBuffers, nCount, "NONAME" );
}

void NexSyncNamedQueue::deinit()
{
    NexSyncQueue::deinit();
    
    if ( mName )
    {
        free( mName );
        mName = NULL;
    }
}
