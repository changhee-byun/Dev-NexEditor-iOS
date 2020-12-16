/*
 * File Name   : StreamWriter.h
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

#import <Foundation/Foundation.h>
#import "StreamWriterContext.h"
#import "StreamWriterSettings.h"

@protocol StreamWriter<NSObject>
@property (nonatomic, readonly) StreamWriterSettings *settings;
@property (nonatomic, readonly) StreamWriterContext *context;

- (void) beginWithVideoInfo:(StreamWriterVideoInfo *) pVideoInfo audioInfo:(StreamWriterAudioInfo *) pAudioInfo;
- (BOOL) appendSample:(void *) sample trackType:(StreamWriterTrackType) trackType time:(uint64_t) time;
- (void) markTrackFinished:(StreamWriterTrackType) trackType;
- (BOOL) isTrackReadyForMoreSamples:(StreamWriterTrackType) trackType;
/// YES to finish, NO to abort.
- (void) finish:(BOOL) finish;
@end

/// To implement a new stream writer, conform StreamWriter and call this function to plug in to an empty, allocated, context.
void StreamWriterBindToContext(id<StreamWriter> streamWriter, StreamWriterContext *context);
