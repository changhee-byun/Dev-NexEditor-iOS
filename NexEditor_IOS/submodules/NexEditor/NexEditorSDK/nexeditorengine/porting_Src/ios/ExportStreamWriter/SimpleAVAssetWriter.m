/*
 * File Name   : SimpleAVAssetWriter.m
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

#import "SimpleAVAssetWriter.h"
#import "NexEditorLog.h"

#define LOG_TAG @"SimpleAVAssetWriter"

@interface StreamWriterSettings(AudioStreamBasicDescription)
- (AudioStreamBasicDescription) asbd;
@end

@interface SimpleAVAssetWriter()
@property (nonatomic, strong) id<SimpleAVAssetWriterSettings> settings;
@property (nonatomic, strong) AVAssetWriter *assetWriter;
@property (nonatomic, strong) NSError *error;

@property (nonatomic, strong) AVAssetWriterInput *videoInput;
@property (nonatomic, strong) AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdapter;

@property (nonatomic, strong) AVAssetWriterInput *audioInput;
@property (nonatomic) AudioStreamBasicDescription asbd;
@property (nonatomic) CMAudioFormatDescriptionRef audioFormatDescription;
@end

@implementation SimpleAVAssetWriter

- (instancetype) initWithFileURL:(NSURL *) fileURL settings:(id<SimpleAVAssetWriterSettings>) settings
{
    static const BOOL audioEnabled = YES;
    self = [super init];
    if ( self ) {
        self.settings = settings;
        NSError *error = nil;
        AVAssetWriter *assetWriter = [AVAssetWriter assetWriterWithURL:fileURL fileType:AVFileTypeQuickTimeMovie error:&error];
        self.error = error;
        
        if (error == nil) {
            AVAssetWriterInput *videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:settings.videoOutputSettings];
            AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdapter = [AVAssetWriterInputPixelBufferAdaptor assetWriterInputPixelBufferAdaptorWithAssetWriterInput:videoInput sourcePixelBufferAttributes:settings.pixelBufferAttributes];
            videoInput.transform = self.settings.videoOrientationTransform;
            
            [assetWriter addInput:videoInput];
            self.videoInput = videoInput;
            self.pixelBufferAdapter = pixelBufferAdapter;
            
            if (audioEnabled) {
                AVAssetWriterInput *audioInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio outputSettings:settings.audioOutputSettings];
                [assetWriter addInput:audioInput];
                self.audioInput = audioInput;
                self.asbd = settings.settings.asbd;
                self.audioFormatDescription = [self audioFormatDescriptionWithAudioStreamBasicDescription:self.asbd];
            }
            
            self.assetWriter = assetWriter;
            
            if ([assetWriter startWriting]) {
                [assetWriter startSessionAtSourceTime:kCMTimeZero];
            } else {
                self.error = assetWriter.error;
                self.assetWriter = nil;
                self.videoInput = nil;
                self.audioInput = nil;
                self.pixelBufferAdapter = nil;
            }
        }
    }
    return self;
}

- (void) dealloc
{
    if (self.audioFormatDescription) {
        CFRelease(self.audioFormatDescription);
        self.audioFormatDescription = NULL;
    }
    self.error = nil;
    self.assetWriter = nil;
    self.videoInput = nil;
    self.audioInput = nil;
    self.pixelBufferAdapter = nil;
    self.settings = nil;
    [super dealloc];
}

- (BOOL) appendWithPixelBuffer:(CVPixelBufferRef) pixelBuffer at:(CMTime) time
{
    if (self.assetWriter == nil || self.assetWriter.status == AVAssetWriterStatusFailed) {
        return NO;
    }
    
    if (!self.videoInput.isReadyForMoreMediaData) {
        return NO;
    }
    BOOL result = NO;
    result = [self.pixelBufferAdapter appendPixelBuffer:pixelBuffer withPresentationTime:time];
    if (!result) {
        NexLogE(LOG_TAG, @"Failed appending video sample at %2.3f, error:%@", CMTimeGetSeconds(time), self.assetWriter.error.localizedDescription);
    }
    return result;
}



- (BOOL) appendAudioWithData:(NSData *) data at:(CMTime) time
{
    if (self.assetWriter == nil || self.assetWriter.status == AVAssetWriterStatusFailed) {
        return NO;
    }
    
    if (!self.audioInput.isReadyForMoreMediaData) {
        return NO;
    }

    if (self.audioFormatDescription == NULL) {
        // Bad format description
        return NO;
    }

    BOOL result = NO;
    CMSampleBufferRef sampleBuffer = [self createAudioSampleBufferWithData:data time:time];
    if (sampleBuffer) {
        result = [self.audioInput appendSampleBuffer:sampleBuffer];
        if (!result) {
            NexLogE(LOG_TAG, @"Failed appending audio sample at %2.3f, error:%@", CMTimeGetSeconds(time), self.assetWriter.error.localizedDescription);
        }
        CFRelease(sampleBuffer);
    }

    return result;
}

- (AVAssetWriterInput *) inputForTrackMediaType:(AVMediaType) trackMediaType
{
    AVAssetWriterInput *input = nil;
    if (trackMediaType == AVMediaTypeAudio) {
        input = self.audioInput;
    } else if (trackMediaType == AVMediaTypeVideo) {
        input = self.videoInput;
    }
    return input;
}

/// AVMediaTypeAudio and AVMediaTypeVideo only.
- (void) markTrackFinished:(AVMediaType) trackMediaType
{
    AVAssetWriterInput *input = [self inputForTrackMediaType:trackMediaType];
    if (input) {
        [input markAsFinished];
    }
}

/// AVMediaTypeAudio and AVMediaTypeVideo only.
- (BOOL) isTrackReadyMoreData:(AVMediaType) trackMediaType
{
    BOOL result = YES;
    AVAssetWriterInput *input = [self inputForTrackMediaType:trackMediaType];
    if (input) {
        result = [input isReadyForMoreMediaData];
    }
    return result;
}

- (void) finishWithDone:(void (^)(NSError *error)) done
{
    AVAssetWriter *assetWriter = self.assetWriter;
    if (assetWriter == nil) {
        done(self.error);
        return;
    }
    
    [assetWriter finishWritingWithCompletionHandler:^{
        done(assetWriter.error);
    }];
}

- (void) cancel
{
    [self.assetWriter cancelWriting];
}

#pragma mark -
- (CMAudioFormatDescriptionRef) audioFormatDescriptionWithAudioStreamBasicDescription:(AudioStreamBasicDescription) asbd
{
    AudioChannelLayout channelLayout;
    memset(&channelLayout, 0, sizeof(AudioChannelLayout));
    
    channelLayout.mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
    if (asbd.mChannelsPerFrame == 2) {
        channelLayout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
    }
    
    CMFormatDescriptionRef fd = NULL;
    OSStatus ret = CMAudioFormatDescriptionCreate(kCFAllocatorDefault, &asbd, sizeof(AudioChannelLayout), &channelLayout, 0, NULL, NULL, &fd);
    if (ret != noErr) {
        // TODO: Log error
    }
    return fd;
}

- (AudioBufferList) audioBufferListWithData:(NSData *) data asbd:(AudioStreamBasicDescription) asbd
{
    AudioBuffer buffer;
    buffer.mNumberChannels = asbd.mChannelsPerFrame;
    buffer.mDataByteSize = (UInt32) data.length;
    buffer.mData = (void *) data.bytes;
    
    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0] = buffer;
    
    return bufferList;
}

- (CMSampleBufferRef) createAudioSampleBufferWithData:(NSData *) data time:(CMTime) time
{
    AudioStreamBasicDescription asbd = self.asbd;
    
    size_t sampleSize = asbd.mChannelsPerFrame * asbd.mBitsPerChannel / 8;
    CMItemCount numSamples = data.length / sampleSize;
    
    CMSampleTimingInfo timingInfo;
    timingInfo.presentationTimeStamp = time;
    timingInfo.duration = CMTimeMake(1, asbd.mSampleRate);
    timingInfo.decodeTimeStamp = kCMTimeInvalid;
    
    CMSampleBufferRef sampleBuffer = NULL;
    OSStatus ret = CMSampleBufferCreate(kCFAllocatorDefault, NULL, false, NULL, NULL, self.audioFormatDescription, numSamples, 1, &timingInfo, 1, &sampleSize, &sampleBuffer);
    if (ret == noErr) {
        
        AudioBufferList bufferList = [self audioBufferListWithData:data asbd:asbd];
        
        ret = CMSampleBufferSetDataBufferFromAudioBufferList(sampleBuffer, kCFAllocatorDefault, kCFAllocatorDefault, 0, &bufferList);
    }
    
    return sampleBuffer;
}

@end

@implementation StreamWriterSettings(AudioStreamBasicDescription)
- (AudioStreamBasicDescription) asbd
{
    AudioStreamBasicDescription asbd;
    
    UInt32 bytesPerChannel = self.settings.audio.bitsPerChannel / 8;
    
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mSampleRate = self.settings.audio.samplingRate;
    asbd.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
    asbd.mBytesPerPacket = self.settings.audio.channels * bytesPerChannel;
    asbd.mBytesPerFrame = asbd.mBytesPerPacket;
    asbd.mFramesPerPacket = 1;
    asbd.mChannelsPerFrame = self.settings.audio.channels;
    asbd.mBitsPerChannel = self.settings.audio.bitsPerChannel;
    asbd.mReserved = 0;
    return asbd;
}
@end

@implementation StreamWriterSettings(SimpleAVAssetWriter)
- (StreamWriterSettings *) settings
{
    return self;
}

- (NSDictionary *)pixelBufferAttributes
{
    return @{
             (NSString *) kCVPixelBufferIOSurfacePropertiesKey: @{},
             (NSString *) kCVPixelBufferOpenGLESCompatibilityKey: @(YES),
             (NSString *) kCVPixelBufferPixelFormatTypeKey: @((NSUInteger)kCVPixelFormatType_32BGRA)
             };
}

- (AVVideoCodecType) videoCodec
{
    return AVVideoCodecH264;
}

- (NSDictionary *) videoCompressionProperties
{
    static const NSInteger fullHDProportion = 1920 * 1080;

    NSUInteger frameInterval = MAX(30, self.settings.video.frameRate);
    NSMutableDictionary *props = [[@{
                                     AVVideoMaxKeyFrameIntervalKey: @(frameInterval),
                                     AVVideoExpectedSourceFrameRateKey: @(self.video.frameRate),
                                     AVVideoAverageBitRateKey: @(self.video.bitrate),                                   } mutableCopy] autorelease];
    
    if ( (self.video.width * self.video.height) > fullHDProportion ) {
        // 4K/High
        props[AVVideoAllowFrameReorderingKey] = @(YES);
        props[AVVideoProfileLevelKey] = AVVideoProfileLevelH264HighAutoLevel;
        props[AVVideoH264EntropyModeKey] = AVVideoH264EntropyModeCABAC;
    } else {
        // Main
        props[AVVideoAllowFrameReorderingKey] = @(NO);
        props[AVVideoProfileLevelKey] = AVVideoProfileLevelH264MainAutoLevel;
    }
    return [[props copy] autorelease];
    
}

- (NSDictionary *) videoOutputSettings
{
    return @{
             AVVideoCodecKey: self.videoCodec,
             AVVideoWidthKey: @(self.video.width),
             AVVideoHeightKey: @(self.video.height),
             AVVideoCompressionPropertiesKey: [self videoCompressionProperties]
             };
}

- (AudioFormatID) audioCodec
{
    return kAudioFormatMPEG4AAC;
}

- (NSDictionary *) audioOutputSettings
{
    return @{
             AVFormatIDKey: @(self.audioCodec),
             AVEncoderBitRateKey: @(self.audio.bitrate),
             AVSampleRateKey: @(self.audio.samplingRate),
             AVNumberOfChannelsKey: @(self.audio.channels)
             };
}

- (CGAffineTransform) videoOrientationTransform
{
    CGFloat radian = self.video.rotationAngle / 180.0 * M_PI;
    return CGAffineTransformMakeRotation(radian);
}
@end
