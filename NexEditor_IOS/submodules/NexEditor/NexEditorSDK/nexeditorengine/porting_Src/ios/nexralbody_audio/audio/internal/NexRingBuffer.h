//
//  NexRingBuffer.h
//  nexCalBody
//
//  Created by Matthew Feinberg on 9/8/10.
//  Copyright 2010 Nextreaming. All rights reserved.
//

/******************************************************************************
 
 NexRingBuffer: This class provides a blocking ring buffer implementation that
 allows a push architecture (NexPlayer) to work with a pull architecture (iOS
 AudioUnits).
 
 Usage is fairly simple:  The ring buffer is intended to be used by two threads.
 The initial object should be created by one of the threads and the reference to
 it shared with the other thread.
 
 One thread is a producer and calls the "write" method to place data in the
 buffer.  The other thread is a consumer and calls the "read" method to get data
 from the buffer.
 
 If the buffer is empty, the consumer thread will block on the read call until
 data is available.  However, if any amount of data is available (even if less
 than requested), the consumer thread will return that data immediately (so it
 is important to check the actual number of bytes read).
 
 Conversely, if the buffer is full, the producer thread will block on the write
 method call until space is available.
 
 When the ring buffer is initially created, a capacity in bytes must be
 specified.  Reads or writes larger than this capacity are silently ignored, so
 care must be taken to send data in chunks that fit in the buffer's capacity.
 
 If a write is equal to the buffer capacity, even a single byte of data in the
 buffer will cause the write to block.  If performance is critical, a buffer
 size twice as large as the largest chunk that will be written is probably best.
 
 In this implementation, it is assumed that timing will be driven by the
 consumer.  That is, the consumer will read data as-needed, and the producer
 will block when the buffer is full.  This model works best for media playback,
 such as audio playback.  
 
 The other model is also possible (timing driven by the producer) but in this 
 implementation, performance will suffer as a result because the producer blocks
 on a condition lock, but the consumer block on a retry-loop with a "sleep".
 
 Finally, setting bufferAheadBytes (as in [myRingBuffer setBufferAheadBytes:n])
 causes the consumer to block until the producer has produced at least that many
 initial bytes.  This is normally not necessary, but if the producer runs
 intermittenly, this may be helpful in combination with increasing the buffer
 size.
 
 ******************************************************************************/

#import <Foundation/Foundation.h>


@interface NexRingBuffer : NSObject {
	@private
	NSUInteger mPrimeBuffer;			// True if we're priming the buffer
										//   (mBufferAheadBytes hasn't been accumulated
										//   for the first time, yet.)
	NSUInteger mCapacity;				// Maximum capacity of this ring buffer in bytes
	NSUInteger mBufferAheadBytes;		// Number of bytes to buffer ahead; if less than
										//   this number of bytes are in the buffer, reads
										//   will block.
	NSUInteger mUsed;					// Number of bytes used in buffer
	NSUInteger mBase;					// Offset to base of unconsumed bytes
	NSMutableData *mData;				// Contents of buffer
	NSCondition *mBufferSync;			// Synchronization for buffer access
}

#define NEXRINGBUFFER_TIMEOUT_INSTANT   0           // Time out immediately if no data is available
#define NEXRINGBUFFER_TIMEOUT_FOREVER   0xFFFFFFFF  // Never time out (wait forever)

- (id)initWithCapacity: (NSUInteger)capacity;

- (void)read: (void *)dest                  // Buffer to hold data
      length: (NSUInteger*) length        // IN: Length of buffer; OUT: Number of bytes actually read or 0 if there was a timeout
     timeout: (NSUInteger)usimeout;       // Number of microseconds (same as usleep()) after which to time out, or
                                            //   one of NEXRINGBUFFER_TIMEOUT_???, above.

- (void)write: (const void *)src length: (NSUInteger) length;

- (void)flush;				// Delete buffer contents and reset state

@property (readwrite) NSUInteger bufferAheadBytes;
@property (readonly) NSUInteger freeSpace;
@property (readonly) NSUInteger usedSpace;
@property (readonly) NSUInteger capacity;

@end
