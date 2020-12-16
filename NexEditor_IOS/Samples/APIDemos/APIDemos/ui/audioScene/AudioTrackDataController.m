/******************************************************************************
 * File Name   :	AudioTrackDataController.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "AudioTrackDataController.h"
#import "APIDemos-Swift.h"
@import NexEditorFramework;

@implementation AudioTrackInfo

- (void)dealloc
{
    self.title = nil;
    self.envelopList = nil;
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        _envelopList = [[NSMutableArray alloc] init];
    }
    return self;
}

- (NXEClip *)createClipOrError:(NSError **) error
{
    NXEClip *clip = [NXEClip clipWithSource:self.clipSource error:error];
    if (error != nil && *error != nil) {
        return nil;
    }
    
    clip.bLoop = self.loop;
    
    if(self.trimEndTime > clip.durationMs) self.trimEndTime = clip.durationMs;
    [clip setTrim:self.trimStartTime EndTrimTime:self.trimEndTime];
    
    int compressorValue = self.compressor ? 4 : 0;
    [clip setCompressorValue:compressorValue];
    [clip setClipVolume:self.volumeLevel];
    [clip setAudioClipStartTime:self.startTime EndTime:0];
    [clip setPanLeftValue:self.panLeft];
    [clip setPanRightValue:self.panRight];
    [clip setPitchValue:self.pitch];
    [clip setVoiceChangerValue:self.audioChanger];

    for(EnvelopInfo *info in self.envelopList) {
        [clip.audioEnvelop addVolumeEnvelop:(int)info.time level:(int)info.level];
    }
    
    return clip;
}

@end

@implementation AudioTrackDataController
- (void)dealloc
{
    self.audioTrackList = nil;
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        _audioTrackList = [[NSMutableArray alloc] init];
    }
    return self;
}

- (void)setAudioTrack:(AudioTrackInfo *)audioTrackInfo
{
    [self.audioTrackList addObject:audioTrackInfo];
	audioTrackInfo = nil;
}

- (void)replaceAudioTrack:(int)trackId audioTrackInfo:(AudioTrackInfo *)audioTrackInfo
{
    [self.audioTrackList replaceObjectAtIndex:trackId withObject:audioTrackInfo];
}

@end
