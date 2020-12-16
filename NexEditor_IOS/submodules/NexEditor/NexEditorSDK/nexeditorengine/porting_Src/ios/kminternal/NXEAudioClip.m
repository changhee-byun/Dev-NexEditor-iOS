/******************************************************************************
 * File Name   : NXEAudioClip.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEAudioClip.h"

@implementation NXEAudioClip

- (void)dealloc
{
    [_clipPath release];
    [_volumeEnvelopeLevel release];
    [_volumeEnvelopeTime release];
    [super dealloc];
}

- (id)init
{
    self = [super init];
    if(self) {
        _totalTime			= 0;
        _startTime			= 0;
        _endTime			= 0;
        _startTrimTime		= 0;
        _endTrimTime		= 0;
        _audioOnOff			= 0;
        _autoEnvelop        = 0;
        _clipVolume			= 0;
        _volumeEnvelopeLevel = nil;
        _volumeEnvelopeTime = nil;
        _voiceChanger = 0;
        _pitch = 0;
        _panLeft = -100;
        _panRight = 100;
        _compressor = 0;
        _processorStrength = 0;
        _bassStrength = 0;
        _musicEffect = 0;
    }
    return self;
}

- (void)setHeadTrimTime:(int)headTrimTime
{
    _startTrimTime = headTrimTime;
}

- (void)setTailTrimTime:(int)tailTrimTime
{
    _endTrimTime = tailTrimTime;
}

#pragma mark - NSCopying

- (instancetype)copyWithZone:(NSZone *)zone
{
    NXEAudioClip *destination = [[self class] allocWithZone:zone];
    
    destination.clipId = self.clipId;
    destination.totalTime = self.totalTime;
    destination.startTime = self.startTime;
    destination.endTime = self.endTime;
    destination.startTrimTime = self.startTrimTime;
    destination.endTrimTime = self.endTrimTime;
    destination.audioOnOff = self.audioOnOff;
    destination.autoEnvelop = self.autoEnvelop;
    destination.clipVolume = self.clipVolume;
    destination.volumeEnvelopeLevel = [[NSArray alloc] initWithArray:self.volumeEnvelopeLevel copyItems:YES];
    destination.volumeEnvelopeTime = [[NSArray alloc] initWithArray:self.volumeEnvelopeTime copyItems:YES];
    destination.clipPath = self.clipPath;
    destination.voiceChanger = self.voiceChanger;
    destination.pitch = self.pitch;
    destination.panLeft = self.panLeft;
    destination.panRight = self.panRight;
    destination.compressor = self.compressor;
    destination.processorStrength = self.processorStrength;
    destination.bassStrength = self.bassStrength;
    destination.musicEffect = self.musicEffect;
    
    // volumeEnvelopeLevel, volumeEnvelopeTime 배열에 들어가는 Element의 retainCount값을 1로 유지하기 위해서.
    //
    [destination.volumeEnvelopeLevel release];
    [destination.volumeEnvelopeTime release];
    
    return destination;
}

@end
