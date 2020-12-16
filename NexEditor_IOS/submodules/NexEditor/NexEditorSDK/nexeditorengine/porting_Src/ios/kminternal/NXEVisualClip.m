/******************************************************************************
 * File Name   : NXEVisualClip.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEVisualClip.h"
#import "NEXVIDEOEDITOR_Types.h"

NSString *const kEffectIDNone = @"none";

@interface NXEVisualClip()
/** \brief Undocumented
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* audioClipVec;
/** \brief Undocumented
 *  \since version 1.x
 */
@property (nonatomic) BOOL isMotionTrackedVideo;

@property (nonatomic) CGRect cropStartRectInPixels;
@property (nonatomic) CGRect cropEndRectInPixels;
@property (nonatomic, readonly) CLIP_TYPE rawClipType;
@end

@implementation NXEVisualClip

- (void)dealloc
{
    [_thumbnailPath release];
    [_title release];
    [_titleEffectID release];
    [_clipEffectID release];
    [_filterID release];
    [_clipPath release];
    [_audioClipVec release];
    [_volumeEnvelopeLevel release];
    [_volumeEnvelopeTime release];
    [super dealloc];
}

- (id) init
{
    self = [super init];
    if(self) {
        _totalAudioTime = 0;
        _totalVideoTime	= 0;
        _totalTime		= 0;
        _startTime		= 0;
        _endTime		= 0;
        _startTrimTime	= 0;
        _endTrimTime	= 0;
        _width			= 0;
        _height			= 0;
        _existVideo		= 0;
        _existAudio		= 0;
        _titleStyle		= 0;
        _titleStartTime	= 0;
        _titleEndTime	= 0;
        _startRect		= CGRectMake(0, 0, 0, 0);
        _endRect		= CGRectMake(0, 0, 0, 0);
        _destRect		= CGRectMake(0, 0, 0, 0);
        _audioOnOff		= 0;
        _clipVolume		= 0;
        _bGMVolume		= 0;
        _rotateState	= 0;
        _brightness		= 0;
        _contrast		= 0;
        _saturation		= 0;
        _tintcolor		= 0;
        _lut			= 0;
        _vignette       = NO;
        _clipEffectID   = kEffectIDNone;
        _titleEffectID  = kEffectIDNone;
        _speedControl	= 100;
        _effectDuration	= 0;
        _effectOffset 	= 0;
        _effectOverlap	= 0;
        _volumeEnvelopeLevel = nil;
        _volumeEnvelopeTime = nil;
        _audioClipVec = nil;
        _voiceChanger	= 0;
        _pitch = 0;
        _panLeft = -100;
        _panRight = 100;
        _compressor = 0;
        _processorStrength = 0;
        _bassStrength = 0;
        _musicEffect = 0;
        _isMotionTrackedVideo = NO;
        _useIFrameOnly = NO;
        _keepPitch = NO;
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

- (instancetype) copyWithZone:(NSZone *)zone
{
    NXEVisualClip *destination = [[self class] allocWithZone:zone];
    
    destination.clipId = self.clipId;
    destination.clipType = self.clipType;
    destination.totalAudioTime = self.totalAudioTime;
    destination.totalVideoTime = self.totalVideoTime;
    destination.totalTime = self.totalTime;
    destination.startTime = self.startTime;
    destination.endTime = self.endTime;
    destination.startTrimTime = self.startTrimTime;
    destination.endTrimTime = self.endTrimTime;
    destination.width = self.width;
    destination.height = self.height;
    destination.existAudio = self.existAudio;
    destination.existVideo = self.existVideo;
    destination.titleStyle = self.titleStyle;
    destination.titleStartTime = self.titleStartTime;
    destination.titleEndTime = self.titleEndTime;
    destination.cropStartRectInPixels = self.cropStartRectInPixels;
    destination.cropEndRectInPixels = self.cropEndRectInPixels;
    destination.startRect = self.startRect;
    destination.endRect = self.endRect;
    destination.destRect = self.destRect;
    destination.audioOnOff = self.audioOnOff;
    destination.clipVolume = self.clipVolume;
    destination.bGMVolume = self.bGMVolume;
    destination.brightness = self.brightness;
    destination.contrast = self.contrast;
    destination.saturation = self.saturation;
    destination.tintcolor = self.tintcolor;
    destination.lut = self.lut;
    destination.vignette = self.vignette;
    destination.clipEffectID = self.clipEffectID;
    destination.titleEffectID = self.titleEffectID;
    destination.speedControl = self.speedControl;
    destination.rotateState = self.rotateState;
    destination.effectDuration = self.effectDuration;
    destination.effectOffset = self.effectOffset;
    destination.effectOverlap = self.effectOverlap;
    destination.volumeEnvelopeLevel = [[NSArray alloc] initWithArray:self.volumeEnvelopeLevel copyItems:YES];
    destination.volumeEnvelopeTime = [[NSArray alloc] initWithArray:self.volumeEnvelopeTime copyItems:YES];
    destination.audioClipVec = [[NSArray alloc] initWithArray:self.audioClipVec copyItems:YES];
    destination.clipPath = self.clipPath;
    destination.thumbnailPath = self.thumbnailPath;
    destination.voiceChanger = self.voiceChanger;
    destination.pitch = self.pitch;
    destination.panLeft = self.panLeft;
    destination.panRight = self.panRight;
    destination.compressor = self.compressor;
    destination.processorStrength = self.processorStrength;
    destination.bassStrength = self.bassStrength;
    destination.musicEffect = self.musicEffect;
    destination.useIFrameOnly = self.useIFrameOnly;
    destination.keepPitch = self.keepPitch;
    destination.isMotionTrackedVideo = self.isMotionTrackedVideo;
    
    // volumeEnvelopeLevel, volumeEnvelopeTime, audioClipVec 배열에 들어가는 Element의 retainCount값을 1로 유지하기 위해서.
    //
    [destination.volumeEnvelopeLevel release];
    [destination.volumeEnvelopeTime release];
    [destination.audioClipVec release];
    
    return destination;
}

+ (CGRect) abstractDimensionFromRectInPixels:(CGRect) rect imageSize:(CGSize) size
{
    int ABSTRACT_DIMENSION = 100000;
    CGFloat rect_x = rect.origin.x;
    CGFloat rect_y = rect.origin.y;
    CGFloat rect_width = rect.size.width;
    CGFloat rect_height = rect.size.height;

    int width = size.width;
    int height = size.height;
    rect_x = (int)(rect_x * ABSTRACT_DIMENSION / width);
    rect_y = (int)(rect_y * ABSTRACT_DIMENSION / height);
    rect_width = (int)(rect_width * ABSTRACT_DIMENSION / width);
    rect_height = (int)(rect_height * ABSTRACT_DIMENSION / height);

    return CGRectMake(rect_x, rect_y, rect_width, rect_height);
}

- (CGRect) abstractDimensionFromRectInPixels:(CGRect) rect
{
    CGSize size = self.orientationRegardedSize;
    return [self.class abstractDimensionFromRectInPixels:rect imageSize:size];
}

#pragma mark - Private API
- (CGSize) orientationRegardedSize
{
    CGSize result = CGSizeMake(self.width, self.height);
    if(self.rawClipType == CLIPTYPE_VIDEO) {
        // Swap width and height if this video's orientation is vertical.
        if(self.rotateState == 90 || self.rotateState == 270) {
            result.width = self.height;
            result.height = self.width;
        }
    }
    return result;
}

- (void) setCropStartRectInPixels:(CGRect) rect
{
    _cropStartRectInPixels = rect;
    self.startRect = [self abstractDimensionFromRectInPixels:rect];
}

- (void) setCropEndRectInPixels:(CGRect) rect
{
    _cropEndRectInPixels = rect;
    self.endRect = [self abstractDimensionFromRectInPixels:rect];
}

- (CLIP_TYPE) rawClipType
{
    return (CLIP_TYPE) self.clipType;
}
@end
