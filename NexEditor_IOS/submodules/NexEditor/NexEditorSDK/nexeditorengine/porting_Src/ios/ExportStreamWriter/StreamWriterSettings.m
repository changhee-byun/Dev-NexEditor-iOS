//
//  StreamWriterSettings.m
//  NexEditorFramework
//
//  Created by Simon Kim on 3/22/18.
//  Copyright © 2018 NexStreaming Corp. All rights reserved.
//

#import "StreamWriterSettings.h"

#define STREAMWRITER_DEFAULT_FRAMERATE                      (30.0)
#define STREAMWRITER_DEFAULT_BITRATE(width, height, fps)    (((float)(width * height * fps * 0.17) / 1024) * 1000)
#define STREAMWRITER_DEFAULT_ROTATION_ANGLE_DEGREE          (0)
#define STREAMWRITER_DEFAULT_AUDIO_SAMPLING_RATE            (44100)
#define STREAMWRITER_DEFAULT_AUDIO_CHANNELS                 (2)
#define STREAMWRITER_DEFAULT_AUDIO_BITSPERCHANNEL           (16)
#define STREAMWRITER_DEFAULT_AUDIO_BITRATE                  (128000)

@implementation StreamWriterSettings

- (instancetype) initWithFileURL:(NSURL *) fileURL width:(uint32_t) width height:(uint32_t) height
{
    self = [super init];
    if (self) {
        self.fileURL = fileURL;
        StreamWriterVideoInfo videoInfo;
        videoInfo.format = StreamWriterSampleFormatCVPixelBuffer;
        videoInfo.width = width;
        videoInfo.height = height;
        videoInfo.frameRate = STREAMWRITER_DEFAULT_FRAMERATE;
        videoInfo.bitrate = STREAMWRITER_DEFAULT_BITRATE(width, height, videoInfo.frameRate);
        videoInfo.rotationAngle = STREAMWRITER_DEFAULT_ROTATION_ANGLE_DEGREE;
        self.video = videoInfo;
        
        StreamWriterAudioInfo audioInfo;
        audioInfo.format = StreamWriterSampleFormatRawAudioSampleBlock;
        audioInfo.samplingRate = STREAMWRITER_DEFAULT_AUDIO_SAMPLING_RATE;
        audioInfo.channels = STREAMWRITER_DEFAULT_AUDIO_CHANNELS;
        audioInfo.bitsPerChannel = STREAMWRITER_DEFAULT_AUDIO_BITSPERCHANNEL;
        audioInfo.bitrate = STREAMWRITER_DEFAULT_AUDIO_BITRATE;
        self.audio = audioInfo;
    }
    return self;
}

- (void) dealloc
{
    self.fileURL = nil;
    [super dealloc];
}

- (void) updateVideoInfo:(void (^)(StreamWriterVideoInfo *videoInfo)) updateBlock
{
    StreamWriterVideoInfo videoInfo = self.video;
    updateBlock(&videoInfo);
    self.video = videoInfo;
}

- (void) updateAudioInfo:(void (^)(StreamWriterAudioInfo *videoInfo)) updateBlock
{
    StreamWriterAudioInfo audioInfo = self.audio;
    updateBlock(&audioInfo);
    self.audio = audioInfo;
}

+ (instancetype) settingsWithFileURL:(NSURL *) fileURL width:(uint32_t) width height:(uint32_t) height
{
    return [[[self.class alloc] initWithFileURL:fileURL width:width height:height] autorelease];
}

@end
