/**
 * File Name   : NXECollageProject.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "NXECollageProject.h"
#import "NXECollageSlotConfiguration.h"
#import "NXECollageTitleConfiguration.h"
#import "NXECollageSlotConfigurationInternal.h"
#import "NXECollageTitleConfigurationInternal.h"
#import "NXEAssetItemPrivate.h"
#import "NXEAssetLibrary.h"
#import "NXEEditorType.h"
#import "NXEEngine.h"
#import "NXEClipInternal.h"
#import "CollageData.h"
#import "CollageInfoInternal.h"
#import "AssetLibrary.h"
#import "AspectRatioTool.h"
#import "EditorUtil.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import <NexEditorEngine/NXEVisualClipInternal.h>

@interface CALayer (Collage)
- (BOOL) isCAEAGLLayer;
@end

@interface NXECollageProject()
@property (nonatomic, retain) CollageData *collageData;
@property (nonatomic, retain) NSMutableArray<NXEClip *> *clipList;
@property (nonatomic, retain) CollageInfo *info;
@property (nonatomic, retain) NSArray <NXECollageSlotConfiguration *> *slots;
@property (nonatomic, retain) NSArray <NXECollageTitleConfiguration *> *titles;
@end

@interface EditorUtil (Collage)

+ (NSString *)collageTextWith:(NSArray<NXECollageTitleConfiguration *>*)titles;

@end

@implementation EditorUtil (Collage)

+ (NSString *)collageTextWith:(NSArray<NXECollageTitleConfiguration *>*)titles
{
    NSMutableArray *array = [NSMutableArray array];
    for (NXECollageTitleConfiguration *titleConfiguration in titles) {
        [array addObject:titleConfiguration.textRenderData];
    }
    return [array componentsJoinedByString:@"&"];
}

@end

@implementation CollageInfo(Convert)
- (NSArray<NSValue *>*) convertFromPositions:(NSArray<NSValue *>*)positions toUIView:(UIView *)view
{
    NSMutableArray<NSValue *> *result = [NSMutableArray array];
    
    for (CALayer *layer in view.layer.sublayers) {
        if (layer.isCAEAGLLayer) {
            CGRect frame = layer.frame;
            for (NSValue *position in positions) {
                CGPoint point = position.CGPointValue;
                CGPoint calPoint = CGPointMake(frame.origin.x + frame.size.width * point.x, frame.origin.y + frame.size.height * point.y);
                [result addObject:[NSValue valueWithCGPoint:calPoint]];
            }
        }
    }
    return [[result copy] autorelease];
}

- (CGRect) convertFromRect:(CGRect)rect toUIView:(UIView *)view
{
    for (CALayer *layer in view.layer.sublayers) {
        if (layer.isCAEAGLLayer) {
            CGRect frame = layer.frame;
            
            CGFloat x = frame.origin.x + frame.size.width * rect.origin.x;
            CGFloat y = frame.origin.y + frame.size.height * rect.origin.y;
            CGFloat width = (frame.origin.x + frame.size.width * rect.size.width) - (frame.origin.x + frame.size.width * rect.origin.x);
            CGFloat height = (frame.origin.y + frame.size.height * rect.size.height) - (frame.origin.y + frame.size.height * rect.origin.y);
            
            return CGRectMake(x, y, width, height);
        }
    }
    return CGRectZero;
}
@end


@implementation NXECollageProject : NXEProject

- (NSArray<NexSceneInfo *>*) sceneList
{
    NSMutableArray *result = [NSMutableArray array];
    NexSceneInfo *nexSceneInfo = [[NexSceneInfo alloc] init];
    int sceneIndex = 1;
    nexSceneInfo.sceneId = sceneIndex;
    nexSceneInfo.subEffectId = (1<<16) | sceneIndex;
    nexSceneInfo.effectId = self.collageData.collageEffect;
    nexSceneInfo.titleValue = [NSString stringWithFormat:@"0,%d??%@", [self getTotalTime], [EditorUtil collageTextWith:self.titles]];
    nexSceneInfo.startTimeMS = 0;
    nexSceneInfo.endTimeMS = [self getTotalTime];
    [result addObject:nexSceneInfo];
    [nexSceneInfo release];
    return [[result copy] autorelease];
}

- (NSArray<NexDrawInfo *>*) slotList
{
    int index = 1;
    NSMutableArray *result = [NSMutableArray array];
    for (NXECollageSlotConfiguration *slotConfiguration in self.slots) {
        if (slotConfiguration.clip != nil) {
            slotConfiguration.drawInfo.clipId = ((NXEClip *)slotConfiguration.clip).videoInfo.clipId;
            slotConfiguration.drawInfo.drawId = (1<<16) | index;
            slotConfiguration.drawInfo.subEffectId = (1<<16) | index;
            slotConfiguration.drawInfo.startTimeMS = ((NXEClip *)slotConfiguration.clip).videoInfo.startTime;
            slotConfiguration.drawInfo.endTimeMS = ((NXEClip *)slotConfiguration.clip).videoInfo.endTime;
            [result addObject:slotConfiguration.drawInfo];
            index += 1;
        }
    }
    return [[result copy] autorelease];
}

- (void) updateProject
{
    NSString *const collageStatic = @"static";
    NXETimeMillis collageStaticDuration = 1000;

    BOOL videoNotExist = YES;
    NSString *predicatedFormat = [NSString stringWithFormat:@"clipType == %@", @(NXE_CLIPTYPE_VIDEO)];

    NSPredicate *predicate = [NSPredicate predicateWithFormat:predicatedFormat];
    if ([self.visualClips filteredArrayUsingPredicate:predicate].count != 0) {
        videoNotExist = NO;
    }

    NXETimeMillis recommendDuration = self.info.durationMs;
    if ([self.collageData.collageType isEqualToString:collageStatic] && videoNotExist) {
        recommendDuration = collageStaticDuration;
    }

    NXETimeMillis startTime = 0; NXETimeMillis endTime = 0;

    for (NXECollageSlotConfiguration *slotConfiguration in self.slots) {
        if (slotConfiguration.clip != nil && [slotConfiguration.clip isKindOfClass:[NXEClip class]]) {

            if (((NXEClip *)slotConfiguration.clip).clipType == NXE_CLIPTYPE_VIDEO) {
                if (((NXEClip *)slotConfiguration.clip).durationMs > recommendDuration) {
                    [((NXEClip *)slotConfiguration.clip) setTrim:0 EndTrimTime:recommendDuration];
                }
            }

            ((NXEClip *)slotConfiguration.clip).videoInfo.startTime = 0;
            ((NXEClip *)slotConfiguration.clip).videoInfo.endTime = recommendDuration;

            startTime = recommendDuration * slotConfiguration.slotData.startTime;
            endTime = recommendDuration * slotConfiguration.slotData.endTime;

            if (((NXEClip *)slotConfiguration.clip).clipType == NXE_CLIPTYPE_IMAGE) {
                if (startTime < ((NXEClip *)slotConfiguration.clip).videoInfo.startTime) {
                    ((NXEClip *)slotConfiguration.clip).videoInfo.startTime = startTime;
                }
                if (endTime > ((NXEClip *)slotConfiguration.clip).videoInfo.endTime) {
                    ((NXEClip *)slotConfiguration.clip).videoInfo.endTime = endTime;
                }

                NXETimeMillis imageDuration = ((NXEClip *)slotConfiguration.clip).videoInfo.endTime - ((NXEClip *)slotConfiguration.clip).videoInfo.startTime;
                [((NXEClip *)slotConfiguration.clip) setImageClipDuration:imageDuration];
            } else {
                if (startTime > ((NXEClip *)slotConfiguration.clip).videoInfo.startTime) {
                    ((NXEClip *)slotConfiguration.clip).videoInfo.startTime = startTime;
                }
                if (endTime < ((NXEClip *)slotConfiguration.clip).videoInfo.endTime) {
                    ((NXEClip *)slotConfiguration.clip).videoInfo.endTime = endTime;
                }
            }
        }
    }
    return;
}

- (NSArray<NXEClip *> *) visualClips
{
    NSString *predicatedFormat = @"isClipChanged == YES";

    if ([self.slots filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:predicatedFormat]].count != 0) {
        self.clipList = nil;
        self.clipList = [[[NSMutableArray alloc] init] autorelease];
        for (NXECollageSlotConfiguration *slotConfiguration in self.slots) {
            if (slotConfiguration.clip != nil) {
                slotConfiguration.isClipChanged = NO;
                [self.clipList addObject:(NXEClip *)slotConfiguration.clip];
            }
        }
    }
    return self.clipList;
}

- (void) setVisualClips:(NSArray<NXEClip *> *)visualClips
{
    return; // skip in case of collage
}

- (int) getTotalTime
{
    int result = 0;
    for (NXEClip *clip in self.visualClips) {
        result = result < clip.videoInfo.endTime ? clip.videoInfo.endTime : result;
    }
    return result;
}

- (void) dealloc
{
    self.collageData = nil;
    self.info = nil;
    self.slots = nil;
    self.titles = nil;
    self.clipList = nil;
    [super dealloc];
}

- (void) addAudioResourceWithSlotData:(CollageSlotData *)slotData
{
    NSString *sourcePath = [[AssetLibrary library] itemForId:slotData.audioResource].fileInfoResourcePath;
    NSError *error = nil;
    NXEClip *clip = [NXEClip clipWithSource:[NXEClipSource sourceWithPath:sourcePath] error:&error];
    if (clip == nil) {
        NexLogE(LOG_TAG_PROJECT, @"audio resource clip create error: %@", error.localizedDescription);
        return;
    }

    NXETimeMillis startTime = slotData.startTime * [self getTotalTime];
    int audioScale = (int)(slotData.audioResourcePosition * [self getTotalTime]);
    clip.audioInfo.startTime = startTime + audioScale;
    clip.audioInfo.endTime = clip.audioInfo.startTime + clip.durationMs;

    NSMutableArray<NXEClip *>*audioItems = [self.audioClips mutableCopy];
    [audioItems addObject:clip];
    self.audioClips = audioItems;
    [audioItems release];
}

- (instancetype) initWithCollageAssetItem:(NXEAssetItem *)assetItem error:(NSError **)error
{
    self = [super init];
    
    CollageData *collageData = [[[CollageData alloc] initWithData:assetItem.fileInfoResourceData error:error] autorelease];
    if ( collageData == nil ) {
        [self release];
        return nil;
    }
    
    self.collageData = collageData;
    self.info = [[[CollageInfo alloc] initWithData:self.collageData] autorelease];

    int slotIndex = 0;
    NSMutableArray *slots = [[[NSMutableArray alloc] init] autorelease];
    for (CollageSlotData *slotData in self.collageData.slots) {
        if ([slotData.audioResource isEqualToString:@"none"] == NO) {
            [self addAudioResourceWithSlotData:slotData];
        }
        NXECollageSlotConfiguration *slotConfiguration = [[NXECollageSlotConfiguration alloc] initWithCollageSlotData:slotData
                                                                                                             slotRect:self.info.slots[slotIndex++].rect
                                                                                                        collageAspect:self.info.aspectRatio];
        [slots addObject:slotConfiguration];
        [slotConfiguration release];
    }
    self.slots = slots;

    NSString *const systemType = @"system";
    NSMutableArray *titles = [[[NSMutableArray alloc] init] autorelease];
    for (CollageTitleData *titleData in self.collageData.titles) {
        if ([titleData.titleInfoType isEqualToString:systemType] == NO) {
            NXECollageTitleConfiguration *titleConfiguration = [[NXECollageTitleConfiguration alloc] initWithCollageTitleData:titleData];
            [titles addObject:titleConfiguration];
            [titleConfiguration release];
        }
    }
    self.titles = titles;

    NSString *bgmPath = [[AssetLibrary library] itemForId:self.collageData.collageBGM].fileInfoResourcePath;
    self.options.audioFadeInDuration = self.collageData.fadeInTime;
    self.options.audioFadeOutDuration = self.collageData.fadeOutTime;
    [self setBGMClip:[NXEClip clipWithSource:[NXEClipSource sourceWithPath:bgmPath] error:error] volumeScale:self.collageData.collageBGMVolume];
    
    return self;
}

- (void) beginUpdates
{
    for (NXECollageSlotConfiguration *slotConfiguration in self.slots) {
        slotConfiguration.isUpdating = YES;
    }
}

- (void) endUpdates
{
    NSString *predicatedFormat = @"isUpdating == YES";
    
    if ([self.slots filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:predicatedFormat]].count == 0) {
        return;// not pair and exit this function
    }
    for (NXECollageSlotConfiguration *slotConfiguration in self.slots) {
        slotConfiguration.isUpdating = NO;
    }
//    [NXEEngine.instance updatePreview];
}

@end

@implementation CALayer (Collage)

- (BOOL) isCAEAGLLayer
{
    if ([self isKindOfClass:CAEAGLLayer.class]) {
        return YES;
    } else {
        return NO;
    }
}

@end
