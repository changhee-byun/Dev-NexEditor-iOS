//
//  NexSyncQueue.hpp
//  test_codec
//
//  Created by YooSunghyun on 4/5/16.
//  Copyright © 2016 YooSunghyun. All rights reserved.
//

#ifndef NexSyncQueue_hpp
#define NexSyncQueue_hpp

//#include <stdio.h>
#include <dispatch/dispatch.h>

class NexSyncQueue
{
public:
    NexSyncQueue();
    ~NexSyncQueue();
    
    virtual int init( void**pBuffers, int nCount );
    virtual void deinit();
   
    void* getEmpty();
    void markFilled();
    void* getFilled();
    void markEmpty();
    int getFilledCount() { return mFilledCount; };
    
    
private :
    bool mInitialized;
    void** mBuffers;
    int mBufferCount;
    
    int mIndexForEmpty;
    int mIndexForFilled;
    
    int mFilledCount;
    
    dispatch_semaphore_t mSemEmpty;
    dispatch_semaphore_t mSemFilled;
    
    static long internalWait( dispatch_semaphore_t sem, long timeInMS = 50 );
};

class NexSyncNamedQueue : public NexSyncQueue
{
public:
    NexSyncNamedQueue();
    ~NexSyncNamedQueue();
    
    int init( void**pBuffers,int nCount, const char* pName );
    virtual int init( void**pBuffers, int nCount );
    virtual void deinit();
    
private:
    char* mName;
};

#endif /* NexSyncQueue_hpp */
