/**
 * File Name   : NXEBeatTemplateProject.m
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
#import "NXEBeatTemplateProject.h"
#import "AssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import "NXEBeatAssetItem.h"
#import "NXEBeatAssetItemPrivate.h"
#import "NXEClipSource.h"
#import "NXEClip.h"
#import "NXEClipInternal.h"
#import <NexEditorEngine/NexEditorEngine.h>

#define LOG_TAG @"NXEBeatTemplate"

@interface NXEBeatTemplateProject()
@property(nonatomic, strong) NXEBeatAssetItem* beatAssetItem;
@property(nonatomic) NSInteger totalDuration;
@property(nonatomic) NSUInteger colorFilterIndex;
@end

@implementation NXEBeatTemplateProject

- (instancetype) initWithBeatTemplateAssetItem:(NXEBeatAssetItem *)assetItem clips:(NSArray<NXEClip*>*)clips error:(NSError **)error
{
    self = [super init];
    if (self) {
        NSString* bgmPath = [[AssetLibrary library] itemForId:assetItem.info.audioId].fileInfoResourcePath;
        NXEClipSource* clipSource = [NXEClipSource sourceWithPath:bgmPath];
        self.bgmClip = [NXEClip clipWithSource:clipSource error:error];
        self.beatAssetItem = assetItem;

        if ( [clips count] > 0 && assetItem ) {
            [super setVisualClips: [self converToBeatClipArrayWith:clips]];
        }
    }
    return self;
}

- (void) setVisualClips:(NSArray<NXEClip *> *)visualClips
{
    NexLogE(LOG_TAG, @"Please create new project to change clips");
}

- (void)dealloc
{
    self.beatAssetItem = nil;
    [super dealloc];
}

- (NSArray<NXEClip*>*) converToBeatClipArrayWith:(NSArray<NXEClip*>*)clips
{
    NSMutableArray<NXEClip*>* result = [NSMutableArray array];
    NSArray<NXEEffectEntry*>* entries = self.beatAssetItem.info.entries;
    NSUInteger sourceCount = [clips count];
    int sourceIndex = 0;
    NSUInteger entryCount = [entries count];
    NSArray<NSNumber*> *positions = self.beatAssetItem.info.positions;
    NSUInteger positionCount = [positions count];
    unsigned int duration;
    int startTimeAbs = 0, preStartTimeAbs = 0;
    
    for ( int i = 0 ; i < entryCount ; i++ ) {
        NXEEffectEntry* entry  = entries[i];
        if ( entry.beatIndex < positionCount+1 ) {

            startTimeAbs = [positions[entry.beatIndex] intValue] + entry.startTime;

            if ( i > 0 ) {
                NXEClip* clip = [[clips[sourceIndex] copy] autorelease];
                sourceIndex = (sourceIndex+1) % sourceCount;
                [clip setImageClipDuration:(startTimeAbs - preStartTimeAbs)];
                [clip setClipEffect:entry.clipEffectId];
                [result addObject:clip];
            }

            preStartTimeAbs = startTimeAbs;
            
        } else {
            
        }
    }
    return result;
    
    
//    for ( int i = 0 ; i < 3 ; i++ ) {
//        NXEClip* c1 = [[clips[i] copy] autorelease];
//        [c1 setImageClipDuration:1000];
//        [result addObject:c1];
//    }
//    return result;
    

//    NSArray<NXEBeatEffect*>* effects = self.beatAssetItem.info.effects;
//    //for
//
//
//
//    __block int totalSourceChange = 0;
//    __block int totalPriority = 0;
//    __block int totalEffect = 0;
//
//    [effects enumerateObjectsUsingBlock:^(NXEBeatEffect * _Nonnull effect, NSUInteger idx, BOOL * _Nonnull stop) {
//        if( effect.sourceChange ) {
//            totalSourceChange++;
//        }
//        else if( effect.priority > 0 ) {
//            totalPriority++;
//        }
//        else if ( effect.clipEffectId != nil ) {
//            totalEffect++;
//        }
//    }];
//    return [self converToBeatClipArrayWith:clips totalSourceChange:totalSourceChange totalPriority:totalPriority totalEffect:totalEffect];
}
/*

- (NSArray<NXEClip*>*) converToBeatClipArrayWith:(NSArray<NXEClip*>*)clips
                                   totalSourceChange:(int)totalSourceChange totalPriority:(int)totalPriority totalEffect:(int)totalEffect
{

#define DEFAULT_INOUT_EFFECT_DURATION 50
    NSArray<NXEBeatEffect*>* effects = self.beatAssetItem.info.effects;
    __block int totalClip = (int)clips.count;
    __block int clipIndex = 0;
    __block int lastClipEndTime = 0;
    __block int effectIgnored = 0;
    __block int maxClipNum = MIN(totalClip, (totalSourceChange + totalPriority + totalEffect));
    NSMutableArray<NXEClip*>* result = [NSMutableArray array];

    for ( int i = 0 ; i < 3 ; i++ ) {
        NXEClip* c1 = [[clips[i] copy] autorelease];
        [c1 setImageClipDuration:1000];
        [result addObject:c1];
    }
    return result;
    
    [effects enumerateObjectsUsingBlock:^(NXEBeatEffect * _Nonnull effect, NSUInteger idx, BOOL * _Nonnull stop) {
       
        BOOL ignoreEffect = effect.clipEffectId && (totalSourceChange + totalPriority + effectIgnored < maxClipNum) ? YES : NO;
        BOOL priorityActive = (effect.priority > 0) && (maxClipNum-totalSourceChange-effect.priority > 0) ? YES : NO;
        BOOL shouldChangeClipForNextEffect = effect.sourceChange || priorityActive || ignoreEffect;

        if ( effect.time - lastClipEndTime > 0 ) {
            if( !ignoreEffect ) {
                if( effect.clipEffectId != nil || self.beatAssetItem.info.defaultEffect.clipEffectId != nil ) {
                    NXEClip* c1 = [[clips[clipIndex] copy] autorelease];
                    NXEClip* c2 = [[clips[clipIndex] copy] autorelease];
                    
                    int effectInDuration = effect.clipEffectId != nil ? effect.effectInDuration : DEFAULT_INOUT_EFFECT_DURATION;
                    int effectOutDuration = effect.clipEffectId != nil ? effect.effectOutDuration : DEFAULT_INOUT_EFFECT_DURATION;
                    int c1d = effect.time - lastClipEndTime - effectInDuration;
                    int c2d = effectInDuration + effectOutDuration;
                    lastClipEndTime = effect.time + effectOutDuration;
                    
                    [c1 setImageClipDuration:c1d];
                    [c2 setImageClipDuration:c2d];
                    [c2 setClipEffect:effect.clipEffectId];
                    
                    NXEPresetColorEffect colorFilter = [self findPresetColorEffectWithName: self.beatAssetItem.info.defaultEffect.colorFilterId];
                    if (effect.colorFilterId != nil) {
                        colorFilter = [self findPresetColorEffectWithName:effect.colorFilterId];
                    }
                    if (colorFilter != NXE_CE_NONE ) {
                        [c2 setColorEffectValues:colorFilter];
                    }
                    [result addObject:c1];
                    [result addObject:c2];
                }
                else {
                    NXEClip* c1 = [[clips[clipIndex] copy] autorelease];
                    int duration = effect.time - lastClipEndTime;
                    lastClipEndTime = effect.time;
                    [c1 setImageClipDuration:duration];
                    [result addObject:c1];
                }
            }
            else {
                NXEClip* clip = [[clips[clipIndex] copy] autorelease];
                int clipDuration = effect.time - lastClipEndTime;
                [clip setImageClipDuration:clipDuration];
                lastClipEndTime = effect.time;
                [result addObject:clip];
                effectIgnored++;
            }
            if ( shouldChangeClipForNextEffect ) {
                if(effect.internalClipId == nil) {
                    clipIndex = (clipIndex+1 > totalClip-1) ? 0 : clipIndex + 1;
                }
            }
        }
    }];
    return result;
}
*/
- (NXEPresetColorEffect) findPresetColorEffectWithName:(NSString*)name {
    NXEPresetColorEffect result = NXE_CE_NONE;
    
    if ( [name isEqualToString:@"ALIEN_INVASION"] ) {
        result = NXE_CE_ALIEN_INVASION;
    }
    else if ( [name isEqualToString:@"BLACK_AND_WHITE"] ) {
        result = NXE_CE_BLACK_AND_WHITE;
    }
    else if ( [name isEqualToString:@"COOL"] ) {
        result = NXE_CE_COOL;
    }
    else if ( [name isEqualToString:@"DEEP_BLUE"] ) {
        result = NXE_CE_DEEP_BLUE;
    }
    else if ( [name isEqualToString:@"PINK"] ) {
        result = NXE_CE_PINK;
    }
    else if ( [name isEqualToString:@"RED_ALERT"] ) {
        result = NXE_CE_RED_ALERT;
    }
    else if ( [name isEqualToString:@"SEPIA"] ) {
        result = NXE_CE_SEPIA;
    }
    else if ( [name isEqualToString:@"SUNNY"] ) {
        result = NXE_CE_SUNNY;
    }
    else if ( [name isEqualToString:@"PURPLE"] ) {
        result = NXE_CE_PURPLE;
    }
    else if ( [name isEqualToString:@"ORANGE"] ) {
        result = NXE_CE_ORANGE;
    }
    else if ( [name isEqualToString:@"STRONG_ORANGE"] ) {
        result = NXE_CE_STRONG_ORANGE;
    }
    else if ( [name isEqualToString:@"SPRING"] ) {
        result = NXE_CE_SPRING;
    }
    else if ( [name isEqualToString:@"SUMMER"] ) {
        result = NXE_CE_SUMMER;
    }
    else if ( [name isEqualToString:@"FALL"] ) {
        result = NXE_CE_FALL;
    }
    else if ( [name isEqualToString:@"ROUGE"] ) {
        result = NXE_CE_ROUGE;
    }
    else if ( [name isEqualToString:@"PASTEL"] ) {
        result = NXE_CE_PASTEL;
    }
    else if ( [name isEqualToString:@"NOIR"] ) {
        result = NXE_CE_NOIR;
    }
    else if ( [name isEqualToString:@"rand"] ) {
        if (self.colorFilterIndex >= NXE_CE_MAX) {
            self.colorFilterIndex = 0;
        }
        result = self.colorFilterIndex++;
    }
    return result;
}

@end
