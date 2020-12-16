//
//  NexCTSRingBuffer.h
//  nexCalBody
//
//  Created by Matthew Feinberg on 9/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NexRingBuffer.h"

@interface NexCTSRingBuffer : NexRingBuffer 

@property (readonly) NSUInteger bitrate;

- (id)initWithCapacity: (NSUInteger)capacity
			   bitrate: (NSUInteger)bitrate;


- (void)read: (void *) dest 
	  length: (NSUInteger*) length
		 cts: (NSUInteger*) outCTS;			// CTS of first sample in returned buffer

- (void)read: (void *) dest 
	  length: (NSUInteger*) length;

- (void)write: (const void *) src 
	   length: (NSUInteger) length
		  cts: (NSUInteger) cts;				// CTS of first sample in buffer

// Recalculate the timeout based on the specified capacity.  This does
// not actually change the capacity of the ring buffer.  This is useful
// if the underlying OS buffer is smaller and it is necessary to timeout
// and output silence before the hardware play position catches up.
- (void)setTimeoutForCapacity: (NSUInteger)capacity;

@end
