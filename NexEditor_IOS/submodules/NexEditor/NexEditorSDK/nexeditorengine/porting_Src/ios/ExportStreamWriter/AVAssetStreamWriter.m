/*
 * File Name   : AVAssetStreamWriter.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#import "AVAssetStreamWriter.h"
#import <AVFoundation/AVFoundation.h>
#import "NexEditorLog.h"
#import "SimpleAVAssetWriter.h"


#define LOG_TAG @"AVAssetStreamWriter"

@interface AVAssetStreamWriter()
@property (nonatomic, strong) StreamWriterSettings *settings;
@property (nonatomic, strong) NSData *contextMemory;
@property (nonatomic, strong) SimpleAVAssetWriter *assetWriter;
@end

@implementation AVAssetStreamWriter

@synthesize context = _context;

- (instancetype) initWithSettings:(StreamWriterSettings *) settings
{
    self = [super init];
    if ( self ) {
        self.settings = settings;
    }
    return self;
}

- (void) dealloc
{
    self.settings = nil;
    self.contextMemory = nil;
    _context = NULL;
    self.assetWriter = nil;
    
    [super dealloc];
}

+ (instancetype) writerWithSettings:(StreamWriterSettings *) settings
{
    return [[[self.class alloc] initWithSettings:settings] autorelease];
}

#pragma mark -

- (StreamWriterContext *) context
{
    if (_context == NULL) {
        self.contextMemory = [NSMutableData dataWithLength:sizeof(StreamWriterContext)];
        _context = (StreamWriterContext *) self.contextMemory.bytes;
        StreamWriterBindToContext(self, _context);
    }
    return _context;
}

#pragma mark - StreamWriter

- (void) beginWithVideoInfo:(StreamWriterVideoInfo *) pVideoInfo audioInfo:(StreamWriterAudioInfo *) pAudioInfo
{

    StreamWriterVideoInfo video = self.settings.video;
    video.format = pVideoInfo->format;
    video.rotationAngle = pVideoInfo->rotationAngle;
    self.settings.video = video;
    
    StreamWriterAudioInfo audio = self.settings.audio;
    audio.format = pAudioInfo->format;
    if (audio.bitrate == 0) {
        audio.bitrate = pAudioInfo->bitrate;
    }
    self.settings.audio = audio;
    
    @autoreleasepool {
    self.assetWriter = [[[SimpleAVAssetWriter alloc] initWithFileURL:self.settings.fileURL settings:self.settings] autorelease];
    }
}

- (BOOL) appendSample:(void *) sample trackType:(StreamWriterTrackType) trackType time:(uint64_t) time
{
    BOOL result = NO;
    CMTime cmtime = CMTimeMake(time, 1000);
    @autoreleasepool {
    if (trackType == StreamWriterTrackTypeAudio) {
        StreamWriterRawAudioSampleBlock *block = (StreamWriterRawAudioSampleBlock *) sample;
        NSData *data = [NSData dataWithBytes:block->pSamples length: block->size];
        result = [self.assetWriter appendAudioWithData:data at:cmtime];
    } else if (trackType == StreamWriterTrackTypeVideo) {
        result = [self.assetWriter appendWithPixelBuffer: (CVPixelBufferRef) sample at:cmtime];
    }
    }
    return result;
}

- (AVMediaType) trackMediaTypeWithStreamWriterTrackType:(StreamWriterTrackType) trackType
{
    AVMediaType trackMediaType = nil;
    if (trackType == StreamWriterTrackTypeAudio) {
        trackMediaType = AVMediaTypeAudio;
    } else if (trackType == StreamWriterTrackTypeVideo) {
        trackMediaType = AVMediaTypeVideo;
    }
    return trackMediaType;
}

- (void) markTrackFinished:(StreamWriterTrackType) trackType
{
    AVMediaType trackMediaType = [self trackMediaTypeWithStreamWriterTrackType:trackType];
    if (trackMediaType) {
        @autoreleasepool {
        [self.assetWriter markTrackFinished:trackMediaType];
        }
    }
}

- (BOOL) isTrackReadyForMoreSamples:(StreamWriterTrackType) trackType
{
    BOOL result = YES;
    @autoreleasepool {
    AVMediaType trackMediaType = [self trackMediaTypeWithStreamWriterTrackType:trackType];
    if (trackMediaType) {
        result = [self.assetWriter isTrackReadyMoreData:trackMediaType];
    }
    }
    return result;
}

#define EXPORT_FINISH_TIMEOUT_NSEC  (NSEC_PER_SEC * 3)
- (void) finish:(BOOL) finish
{
    @autoreleasepool {
        if (finish) {
            dispatch_group_t group = dispatch_group_create();
            dispatch_group_enter(group);
            [self.assetWriter finishWithDone:^(NSError * error) {
                if ( error ) {
                    NexLogE(LOG_TAG, @"Failed to finish export:%@", error.localizedDescription);
                }
                dispatch_group_leave(group);
            }];
            dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, EXPORT_FINISH_TIMEOUT_NSEC));
            dispatch_release(group);
        } else {
            // cancel
            [self.assetWriter cancel];
            NSURL *fileURL = self.settings.fileURL;
            NexLogI(LOG_TAG, @"Export cancelled. Removing file at:%@", fileURL.absoluteString);
            [[NSFileManager defaultManager] removeItemAtURL:fileURL error:nil];
        }
        self.assetWriter = nil;
    }
}

@end
