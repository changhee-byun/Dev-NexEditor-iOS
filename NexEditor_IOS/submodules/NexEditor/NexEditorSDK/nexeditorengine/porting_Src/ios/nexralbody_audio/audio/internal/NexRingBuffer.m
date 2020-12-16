//
//  NexRingBuffer.m
//  nexCalBody
//
//  Created by Matthew Feinberg on 9/8/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "NexRingBuffer.h"
#import <unistd.h>
#import <memory.h>
#import "NexEditorLog.h"

#define LOG_TAG @"NexRingBuffer"

@implementation NexRingBuffer

@synthesize bufferAheadBytes;
@synthesize usedSpace = mUsed;
@synthesize capacity = mCapacity;

- (id)initWithCapacity: (NSUInteger)capacity {
    self = [super init];
	if( self ) {
		mPrimeBuffer = YES;
		mCapacity = capacity;
		mData = [[NSMutableData alloc] initWithLength:capacity];
		mBufferSync = [[NSCondition alloc] init];
	}
	
	return self;
}

- (void) flush {
	[mBufferSync lock];
	mUsed = 0;
	mBase = 0;
	mPrimeBuffer = YES;
	[mBufferSync unlock];
}

- (void)read: (void *)dest length: (NSUInteger*) length timeout: (NSUInteger) ustimeout {
	    
    NSUInteger waited = 0;
    
	if( *length < 1 )
		return;
	
	[mBufferSync lock];
	
	while( mUsed < mBufferAheadBytes && mPrimeBuffer ) {
		[mBufferSync unlock];
        if( ustimeout == NEXRINGBUFFER_TIMEOUT_FOREVER ) {
            usleep(100000);
        } else if( waited>=ustimeout) {
            *length = 0;
            return;
        } else if( ustimeout - waited < 100000 ) {
            usleep((useconds_t)(ustimeout - waited));
            waited = ustimeout;
        } else {
            usleep(100000);
            waited += 100000;
        }

		[mBufferSync lock];
	}
	mPrimeBuffer = NO;

	unsigned char *bytes = [mData mutableBytes];
	
	if( mUsed < *length )
		*length = mUsed;
	
	if( mUsed > 0 ) {
		if( mCapacity - mBase >= *length ) {
			memcpy(dest, bytes+mBase, *length);
		} else {
			NSUInteger n = mCapacity-mBase;
			memcpy(dest, bytes+mBase, n);
			memcpy(dest+n, bytes, *length-n);
		}
		mBase = (mBase + *length)%mCapacity;
		mUsed -= *length;
	}
	
	[mBufferSync signal];
	
	[mBufferSync unlock];
}

- (void)write: (const void *)src length: (NSUInteger) length {
	
	if( mCapacity < length ) {
		NexLogE(LOG_TAG, @"Write too large for buffer! (length=%lu, capacity=%lu)", (unsigned long)length, (unsigned long)mCapacity);
		return;
	}
	
	[mBufferSync lock];
	
	while ( mCapacity-mUsed < length ) {
        NSDate *timeout = [[NSDate alloc] initWithTimeIntervalSinceNow:0.1];
        BOOL success = [mBufferSync waitUntilDate:timeout];
        [timeout release];
        if( !success ) {            
            [mBufferSync unlock];
            NexLogE(LOG_TAG, @"BUFFER OVERFLOW");
            return;
        }
		//[mBufferSync wait];
	}
	
	unsigned char *bytes = [mData mutableBytes];
	NSUInteger writeBase = (mBase + mUsed)%mCapacity;
	
	if( mCapacity - writeBase >= length ) {
		memcpy(bytes+writeBase, src, length);
	} else {
		NSUInteger n = mCapacity-writeBase;
		memcpy(bytes+writeBase, src, n);
		memcpy(bytes, src+n, length-n);
	}
	mUsed += length;
	
	[mBufferSync unlock];
	
}

- (NSUInteger) freeSpace {
	return mCapacity - mUsed;   // Thread-safe because mCapacity never changes
}

- (void) dealloc {
	[mData release];
	[mBufferSync release];
	[super dealloc];
}


@end

