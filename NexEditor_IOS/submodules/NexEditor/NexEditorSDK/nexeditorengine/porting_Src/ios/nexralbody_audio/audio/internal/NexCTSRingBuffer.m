//
//  NexCTSRingBuffer.m
//  nexCalBody
//
//  Created by Matthew Feinberg on 9/10/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "NexCTSRingBuffer.h"

@interface NexCTSRingBuffer()
@property (nonatomic) NSUInteger bitrate;
@property (nonatomic) NSUInteger mTimeout;
//
@property (nonatomic) float mHeaderCTS;
@end

@implementation NexCTSRingBuffer


- (id)initWithCapacity: (NSUInteger)capacity
			   bitrate: (NSUInteger)bitrate
{
    self = [self initWithCapacity:capacity];
	if( self ) {
		self.bitrate = bitrate;
        self.mTimeout = (capacity*8000/bitrate);
        
        self.mHeaderCTS = 0;
	}
	return self;
}

- (void)read: (void *) dest 
	  length: (NSUInteger*) length
		 cts: (NSUInteger*) outCTS			// CTS of first sample in returned buffer
{
    [super read:dest length:length timeout:self.mTimeout];
    
    self.mHeaderCTS += ((float)(*length*8000)/(float)self.bitrate);
    *outCTS = (NSUInteger)self.mHeaderCTS;
}

- (void)read: (void *) dest 
	  length: (NSUInteger*) length
{
	[super read:dest length:length timeout:self.mTimeout];
}

- (void)write: (const void *) src 
	   length: (NSUInteger) length
{
	[super write:src length:length];
}

- (void)write: (const void *) src 
	   length: (NSUInteger) length
		  cts: (NSUInteger) cts				// CTS of first sample in buffer
{
    if(self.usedSpace == 0) { // seek 이벤트가 왔을 때 시작 CTS를 잡기 위해 추가한 코드
        self.mHeaderCTS = cts;
    }
    
    [super write:src length:length];
}
	
- (void)setTimeoutForCapacity: (NSUInteger)capacity
{
    NSUInteger newTimeout = (capacity*8000/self.bitrate);
    self.mTimeout = newTimeout;
}

@end
