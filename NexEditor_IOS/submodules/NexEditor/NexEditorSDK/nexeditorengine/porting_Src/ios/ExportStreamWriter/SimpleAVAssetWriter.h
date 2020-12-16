/*
 * File Name   : SimpleAVAssetWriter.h
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
#import <AVFoundation/AVFoundation.h>
#import "StreamWriterSettings.h"

@protocol SimpleAVAssetWriterSettings <NSObject>
@property (nonatomic, readonly) StreamWriterSettings *settings;
@property (nonatomic, readonly) NSDictionary *pixelBufferAttributes;
@property (nonatomic, readonly) AVVideoCodecType videoCodec;
@property (nonatomic, readonly) AudioFormatID audioCodec;
/// outputSettings passed to AVAssetWriterInput's -initWithMediaType:outputSettings: where mediaType is AVMediaTypeVideo
@property (nonatomic, readonly) NSDictionary *videoOutputSettings;
/// outputSettings passed to AVAssetWriterInput's -initWithMediaType:outputSettings: where mediaType is AVMediaTypeAudio
@property (nonatomic, readonly) NSDictionary *audioOutputSettings;
@property (nonatomic, readonly) CGAffineTransform videoOrientationTransform;
@end

/// Default implementation(AVC/AAC/32BGRA) in case an implementation that conforms SimpleAVAssetWriterSettings is not yet ready.
@interface StreamWriterSettings(SimpleAVAssetWriter) <SimpleAVAssetWriterSettings>
@end

@interface SimpleAVAssetWriter : NSObject
- (instancetype) initWithFileURL:(NSURL *) fileURL settings:(id<SimpleAVAssetWriterSettings>) settings;
- (BOOL) appendWithPixelBuffer:(CVPixelBufferRef) pixelBuffer at:(CMTime) time;
- (BOOL) appendAudioWithData:(NSData *) data at:(CMTime) time;
/// AVMediaTypeAudio and AVMediaTypeVideo only.
- (void) markTrackFinished:(AVMediaType) trackMediaType;
/// AVMediaTypeAudio and AVMediaTypeVideo only.
- (BOOL) isTrackReadyMoreData:(AVMediaType) trackMediaType;
- (void) finishWithDone:(void (^)(NSError *error)) block;
- (void) cancel;
@end
