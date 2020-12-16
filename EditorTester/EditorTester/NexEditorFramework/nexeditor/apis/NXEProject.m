/******************************************************************************
 * File Name   : NXEProject.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEProject.h"
#import "NXEProjectInternal.h"
#import "NXEEditorType.h"
#import "NXEClip.h"
#import "NXEClipInternal.h"
#import <NexEditorEngine/NexEditorEngine.h>

#import "NXEEnginePrivate.h"

@implementation NXEMutableProjectOptions

- (instancetype)copyWithZone:(NSZone *)zone
{
    NXEMutableProjectOptions *destination = [[self class] allocWithZone:zone];
    
    destination.audioFadeInDuration = self.audioFadeInDuration;
    destination.audioFadeOutDuration = self.audioFadeOutDuration;
    destination.audioVolume = self.audioVolume;
    
    return destination;
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.audioFadeInDuration = 200;
        self.audioFadeOutDuration = 5000;
        self.audioVolume = 0.5f;
    }
    return self;
}

@end

@implementation AudioStartTimeScale

- (instancetype)initWithVisualClipIndex:(NSUInteger)visualClipIndex audioClipIndex:(NSUInteger)audioClipIndex startTimeScale:(NSUInteger)startTimeScale
{
    self = [super init];
    if(self) {
        _visualClipIndex = visualClipIndex;
        _audioClipIndex = audioClipIndex;
        _startTimeScale = startTimeScale;
    }
    return self;
}

@end

@implementation VisualClipEffectTimeGroup

- (instancetype)initWithStartClipIndex:(NSUInteger)startClipIndex count:(NSUInteger)count
{
    self = [super init];
    if(self) {
        _startClipIndex = startClipIndex;
        _count = count;
    }
    return self;
}

@end

@interface NXEProject ()

@property (nonatomic, retain, readwrite) NXEMutableProjectOptions *options;
@property (nonatomic, retain) NSArray<AudioStartTimeScale *> *adjustedAudioStartTimeScales;
@property (nonatomic, retain) NSArray<VisualClipEffectTimeGroup *> *visualClipEffectTimeGroups;
/// Category extensions of NXEProject can use this dictionary as a context object storage
@property (nonatomic, strong) NSMutableDictionary *hiddenStore;
@property (nonatomic, readonly) NSArray<NexSceneInfo *> *sceneList;
@property (nonatomic, readonly) NSArray<NexDrawInfo *> *slotList;
@end

@implementation NXEProject
@synthesize visualClips = _visualClips;
@synthesize audioClips = _audioClips;

- (void)dealloc
{
    self.hiddenStore = nil;
    [_visualClips release];
    [_audioClips release];
    [_layers release];
    [_options release];
    [_bgmClip release];
    self.adjustedAudioStartTimeScales = nil;
    self.visualClipEffectTimeGroups = nil;
    
    [super dealloc];
}

- (id)init
{
    self = [super init];
    if(self) {
        _options = [[NXEMutableProjectOptions alloc] init];
        self.layers = @[];
    }
    return self;
}

#pragma mark - Properties

- (NSMutableDictionary *) hiddenStore
{
    if ( _hiddenStore == nil ) {
        _hiddenStore = [[NSMutableDictionary alloc] init];
    }
    return _hiddenStore;
}

#pragma mark -

- (NSArray<NXEClip *> *) visualClips
{
    if ( _visualClips == nil ) {
        _visualClips = [[NSArray alloc] init];
    }
    return _visualClips;
}

- (void) setVisualClips:(NSArray<NXEClip *> *) clips
{
    [_visualClips release];
    if (clips) {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"(clipType == %@) OR (clipType == %@)", @(NXE_CLIPTYPE_VIDEO), @(NXE_CLIPTYPE_IMAGE)];
        clips = [clips filteredArrayUsingPredicate:predicate];
    } else {
        clips = @[];
    }
    _visualClips = [clips copy];
    
    // Invalidate visualClipEffectTimeGroups. _visualClipEffectTimeGroups was based on the previous visualClips.
    self.visualClipEffectTimeGroups = @[];
    // Invalidate adjustedAudioStartTimeScales. _adjustedAudioStartTimeScales was based on the previous visualClips.
    self.adjustedAudioStartTimeScales = @[];
    
    NexLogW(LOG_TAG_PROJECT, @"Template settings for this project has been invalidated due to visualClips change");
}

- (NSArray<NXEClip *> *) audioClips
{
    if ( _audioClips == nil ) {
        _audioClips = [[NSArray alloc] init];
    }
    return _audioClips;
}

- (void) setAudioClips:(NSArray<NXEClip *> *) clips
{
    [_audioClips release];
    if (clips) {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"clipType == %@", @(NXE_CLIPTYPE_AUDIO)];
        clips = [clips filteredArrayUsingPredicate:predicate];
    } else {
        clips = @[];
    }
    _audioClips = [clips copy];
    
    // Invalidate adjustedAudioStartTimeScales. _adjustedAudioStartTimeScales was based on the previous visualClips.
    self.adjustedAudioStartTimeScales = @[];
    
    NexLogW(LOG_TAG_PROJECT, @"Template settings for this project has been invalidated due to audioClips change");
}

- (void) setLayers:(NSArray<NXELayer *> *)layers
{
    [_layers release];
    if ( layers == nil ) {
        layers = @[];
    }
    _layers = [layers copy];
    
//    NXEEngine *engine = NXEEngine.instance;
//    if (engine.project == self) {
//        [engine setLayers:_layers];
//    }
}

- (void) setColorAdjustment:(NXEColorAdjustments)colorAdjustments
{
    _colorAdjustments = colorAdjustments;
//    NXEEngine *engine = NXEEngine.instance;
//    if (engine.project == self) {
//        [engine adjustProjectColorWith:colorAdjustments];
//    }
}

#pragma mark -

- (void)setBGMClip:(NXEClip *)clip volumeScale:(float)volumeScale
{
    self.bgmClip = clip;
    [self.bgmClip setClipVolume:(int)(volumeScale*200)];
}


- (int)getTotalTime
{
    int result = 0;
    NXEClip *lastClip = self.visualClips.lastObject;
    if ( lastClip ) {
        result = lastClip.videoInfo.endTime;
    }
    return result;
}

- (void)updateProject
{
    int primaryItemTotalCount = (int)[self.visualClips count];
    if(primaryItemTotalCount == 0) {
        return ;
    }
    
    NXEClip* visualClip;
    
    int startTime = 0;
    int endTime = 0;
    int totalTime = 0;
    int clipDuration = 0;
    int effectMargin = 0;
    int titleStartTime = 0;
    int titleEndTime = 0;
    
    // set visual info start and end time
    for(int i = 0; i < primaryItemTotalCount; i++) {
        visualClip = self.visualClips[i];
        
        // NXEEngine.aspectRatio may have been changed. Update cropStart/EndRect with properly calculated values if cropMode != CropModeCustom.
        [visualClip setTransformRampWithMode:visualClip.cropMode];

        // isVisualClip
        if(visualClip.vinfo != nil) {
            visualClip.videoInfo.startTime = startTime;
            
            switch (visualClip.clipType) {
                case NXE_CLIPTYPE_IMAGE:
                    // get image duration
                    clipDuration = visualClip.durationMs;
                    break;
                case NXE_CLIPTYPE_VIDEO:
                    
                    clipDuration = visualClip.speedControlledTrimDuration;
                    
                    if(clipDuration < 500) {
                        NexLogD(LOG_TAG_PROJECT, @"clip duration under 500, duration: %d, speed: %d", clipDuration, visualClip.vinfo.speedControl);
                    }
                    break;
                    
                default:
                    clipDuration = 0;
                    break;
            }
            
            endTime = startTime + clipDuration;
            
            if([[visualClip.videoInfo getTransitionEffectId] isEqualToString:kEffectIDNone]) {
                effectMargin = 0;
            } else {
                if(visualClip.videoInfo.effectOverlap != 0) {
                    effectMargin = visualClip.vinfo.effectDuration * 100 / visualClip.videoInfo.effectOverlap;
                } else {
                    effectMargin = 0;
                }
            }
            
            visualClip.videoInfo.titleStartTime = titleStartTime;
            titleEndTime = endTime - effectMargin;
            visualClip.videoInfo.titleEndTime = titleEndTime;
            titleStartTime = titleEndTime + visualClip.vinfo.effectDuration;
            
            totalTime = endTime;
            visualClip.videoInfo.endTime = endTime;
            startTime = totalTime - effectMargin;
        }
    }
    
    if(self.visualClipEffectTimeGroups.count > 0) {
        
        int effectStartTime = 0;
        
        for(VisualClipEffectTimeGroup *visualClipEffect in self.visualClipEffectTimeGroups) {
            
            effectStartTime = [self changeEffectTimeWithStartId:visualClipEffect.startClipIndex
                                                      clipCount:visualClipEffect.count
                                                effectStartTime:effectStartTime];
        }
    }
    
    if(self.adjustedAudioStartTimeScales.count > 0) {
        
        for(AudioStartTimeScale *audioStartTimeScale in self.adjustedAudioStartTimeScales) {
            
            [self adjustAudioStartTimeScaleWithVisualClipIndex:audioStartTimeScale.visualClipIndex
                                                audioClipIndex:audioStartTimeScale.audioClipIndex
                                                startTimeScale:audioStartTimeScale.startTimeScale];
        }
    }
}

- (int)changeEffectTimeWithStartId:(NSUInteger)startId clipCount:(NSUInteger)clipCount effectStartTime:(int)effectStartTime
{
    NSUInteger endId = startId + clipCount - 1;
    
    NXEClip *endClip = self.visualClips[endId];
    
    int effectEndTime = endClip.vinfo.endTime - (endClip.vinfo.effectDuration * endClip.videoInfo.effectOffset) / 100;
    int nextEffectStartTime = effectEndTime + endClip.vinfo.effectDuration;
    
    NXEClip *visualClip = nil;
    
    for(NSUInteger loopId = startId; loopId <= endId; loopId++) {
        
        visualClip = self.visualClips[loopId];
        visualClip.videoInfo.titleStartTime = effectStartTime;
        visualClip.videoInfo.titleEndTime = effectEndTime;
        [visualClip setEffectShowTime:0 endTime:0];
    }
    
    return nextEffectStartTime;
}

- (void)adjustAudioStartTimeScaleWithVisualClipIndex:(NSUInteger)visualClipIndex audioClipIndex:(NSUInteger)audioClipIndex startTimeScale:(NSUInteger)startTimeScale
{
    NXEClip *visualClip = self.visualClips[visualClipIndex];
    
    int startTime = visualClip.videoInfo.titleStartTime;
    int endTime = visualClip.videoInfo.titleEndTime;
    
    NXEClip *audioClip = self.audioClips[audioClipIndex];
    
    int audioStartTime = (int)(startTime + (endTime - startTime) * ((float)startTimeScale / 100.0));
    
    audioClip.audioInfo.startTime = audioStartTime;
    audioClip.audioInfo.endTime = audioStartTime + audioClip.audioInfo.totalTime;
}

#pragma mark - Backward compatibility
- (NSArray<NXEClip *> *) visualItems
{
    return self.visualClips;
}

- (NSArray<NXEClip *> *) audioItems
{
    return self.audioClips;
}

#pragma mark - NSCopying

- (instancetype)copyWithZone:(NSZone *)zone
{
    NXEProject *destination = [[self class] allocWithZone:zone];
    //
    destination.visualClips = [[[NSArray alloc] initWithArray:self.visualClips copyItems:YES] autorelease];
    destination.audioClips = [[[NSArray alloc] initWithArray:self.audioClips copyItems:YES] autorelease];
    destination.visualClipEffectTimeGroups = self.visualClipEffectTimeGroups;
    destination.adjustedAudioStartTimeScales = self.adjustedAudioStartTimeScales;
    destination.colorAdjustments = self.colorAdjustments;
    
    destination.bgmClip = [[self.bgmClip copy] autorelease];
    destination.options = [[self.options copy] autorelease];
    
    return destination;
}

@end


NXEColorAdjustments NXEColorAdjustmentsMake(float brightness, float contrast, float saturation) {
    NXEColorAdjustments result = { brightness, contrast, saturation };
    return result;
}
