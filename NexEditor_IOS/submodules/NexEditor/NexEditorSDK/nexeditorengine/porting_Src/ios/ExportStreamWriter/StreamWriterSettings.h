/*
 * File Name   : StreamWriterSettings.h
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

@interface StreamWriterSettings: NSObject
@property (nonatomic, strong) NSURL *fileURL;
@property (nonatomic) StreamWriterVideoInfo video;
@property (nonatomic) StreamWriterAudioInfo audio;
@property (nonatomic) uint32_t durationMs;

- (void) updateVideoInfo:(void (^)(StreamWriterVideoInfo *videoInfo)) updateBlock;
- (void) updateAudioInfo:(void (^)(StreamWriterAudioInfo *videoInfo)) updateBlock;

+ (instancetype) settingsWithFileURL:(NSURL *) fileURL width:(uint32_t) width height:(uint32_t) height;
@end
