//
//  NTRPixelBufferProvider.m
//  NexEditorFramework
//
//  Created by Simon Kim on 7/10/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#import "NTRPixelBufferProvider.h"
#import <AVFoundation/AVFoundation.h>
#include "NexSAL_Internal.h"

#define __PIXELBUFFER_POOL_SIZE__   4

@interface NTRPixelBufferProvider()
@property (nonatomic) CVPixelBufferPoolRef pixelBufferPool;
@end

@implementation NTRPixelBufferProvider

- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.requiredSize = CGSizeMake(1920, 1080);
        self.pixelBufferPool = NULL;
    }
    return self;
}

- (void) dealloc
{
    [self cleanupPixelBufferPool];
    [super dealloc];
}

- (void) cleanupPixelBufferPool
{
    CVPixelBufferPoolRelease(self.pixelBufferPool);
    self.pixelBufferPool = nil;
}

- (CVPixelBufferPoolRef) createPixelBufferPoolWithSize:(CGSize) size count:(NSUInteger) count
{

    NSDictionary *attribs = @{ (NSString *) kCVPixelBufferPoolMinimumBufferCountKey: @(count)};
    NSDictionary *pixelBufferAttribs = @{
                                         (NSString *) kCVPixelBufferIOSurfacePropertiesKey: @{},
                                         (NSString *) kCVPixelBufferOpenGLESCompatibilityKey: @(YES),
                                         (NSString *) kCVPixelBufferWidthKey: @((NSInteger)size.width),
                                         (NSString *) kCVPixelBufferHeightKey: @((NSInteger)size.height),
                                         (NSString *) kCVPixelBufferPixelFormatTypeKey: @((NSUInteger)kCVPixelFormatType_32BGRA)
                                         };
    CVPixelBufferPoolRef pool = NULL;
    CVReturn ret = CVPixelBufferPoolCreate(kCFAllocatorDefault, (CFDictionaryRef) attribs, (CFDictionaryRef) pixelBufferAttribs, &pool);
    if (ret == kCVReturnSuccess) {
        // pre-allocate pixel buffers up to max
        NSMutableArray *holder = [NSMutableArray array];
        NSDictionary *auxAttribs = @{(NSString *) kCVPixelBufferPoolAllocationThresholdKey: @(count)};
        CVReturn err = kCVReturnSuccess;
        while (err == kCVReturnSuccess) {
            CVPixelBufferRef pb = NULL;
            err = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(kCFAllocatorDefault, pool, (CFDictionaryRef) auxAttribs, &pb);
            if (pb != NULL) {
                [holder addObject:[NSValue valueWithPointer:pb]];
            }
        }
        for (NSValue *v in holder) {
            CVPixelBufferRef pb = (CVPixelBufferRef) v.pointerValue;
            CVPixelBufferRelease(pb);
        }
    }
    return pool;
}

- (void) setRequiredSize:(CGSize)requiredSize
{
    if (CGSizeEqualToSize(_requiredSize, requiredSize)) {
        return;
    }
    _requiredSize = requiredSize;
    [self cleanupPixelBufferPool];
}

- (CVPixelBufferRef) nextAvailablePixelBuffer
{
    if (self.pixelBufferPool == NULL) {
        self.pixelBufferPool = [self createPixelBufferPoolWithSize:self.requiredSize count:__PIXELBUFFER_POOL_SIZE__];
    }
    
    NSDictionary *auxAttribs = @{(NSString *) kCVPixelBufferPoolAllocationThresholdKey: @(__PIXELBUFFER_POOL_SIZE__)};
    CVPixelBufferRef pb = NULL;
    CVReturn ret = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(kCFAllocatorDefault, self.pixelBufferPool, (CFDictionaryRef) auxAttribs, &pb);
    
    if (ret == kCVReturnWouldExceedAllocationThreshold) {
        // retry
        CVPixelBufferPoolFlush(self.pixelBufferPool, kCVPixelBufferPoolFlushExcessBuffers);
        ret = CVPixelBufferPoolCreatePixelBufferWithAuxAttributes(kCFAllocatorDefault, self.pixelBufferPool, (CFDictionaryRef) auxAttribs, &pb);
    }
    if (ret == kCVReturnWouldExceedAllocationThreshold) {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[%s %d] pixel buffer pool is full", __func__, __LINE__);
    }

    return pb;
}

- (void) releasePixelBuffer:(CVPixelBufferRef) pixelBuffer
{
    CVPixelBufferRelease(pixelBuffer);
}

- (CVPixelBufferRef) newPixelBuffer
{
#define RETRY_UPTO  (5)
    
    CVPixelBufferRef result = [self nextAvailablePixelBuffer];
    int count = 0;
    while(result == NULL) {
        // FIXME: Instead of sleep/poll, wait for kCVPixelBufferPoolFreeBufferNotification when the condition that posts the notification is figured out.
        // kCVPixelBufferPoolFreeBufferNotification does not just get posted only by releasing a pixel buffer created out of the pool.
        // We might need something more to trigger that.
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[%s %d] Waiting for a new render buffer, retry: %d", __func__, __LINE__, (count + 1));
        if (count == RETRY_UPTO) {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] FATAL: No render buffer available", __func__, __LINE__);
            break;
        }
        nexSAL_TaskSleep(50);
        count++;
        result = [self nextAvailablePixelBuffer];
    };
    
    return result;
}

- (void) disposePixelBuffer:(CVPixelBufferRef) pixelBuffer
{
    [self releasePixelBuffer:pixelBuffer];
    CVPixelBufferPoolFlush(self.pixelBufferPool, kCVPixelBufferPoolFlushExcessBuffers);
}

@end
