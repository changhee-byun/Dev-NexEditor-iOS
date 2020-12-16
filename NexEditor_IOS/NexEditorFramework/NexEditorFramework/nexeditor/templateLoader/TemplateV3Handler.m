/**
 * File Name   : TemplateV3Handler.m
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

#import "TemplateV3Handler.h"
#import "LUTMap.h"
#import "NXETemplateProject.h"
#import "NXETemplateProject+Internal.h"
#import "NXEAssetItemPrivate.h"
#import "NXEAssetLibrary.h"
#import "AssetLibrary.h"
#import "NXEEnginePrivate.h"
#import "NXECropRectTool.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import <NexEditorEngine/NXEVisualClipInternal.h>

static NSString *const kIndexEntrySourceTypeKey = @"source_type";
static NSString *const kIndexEntrySourcePathKey = @"source_path";
static NSString *const kIndexEntryStartValueKey = @"start";
static NSString *const kIndexEntryEndValueKey = @"end";
static NSString *const kIndexEntryLUTKey = @"lut";
static NSString *const kIndexEntryDrawWidth = @"draw_width";
static NSString *const kIndexEntryDrawHeight = @"draw_height";
static NSString *const kIndexEntryCropMode = @"crop_mode";
static NSString *const kIndexEntryImageCropMode = @"image_crop_mode";
static NSString *const kIndexEntryVideoCropMode = @"video_crop_mode";
static NSString *const kIndexEntryEffects = @"effects";
static NSString *const kIndexEntryDuration = @"duration";
static NSString *const kIndexEntryDurationMin = @"duration_min";
static NSString *const kIndexEntryDurationMax = @"duration_max";
static NSString *const kIndexEntryType = @"type";
static NSString *const kIndexEntryDrawInfos = @"draw_infos";
static NSString *const kIndexEntryTransitionDuration = @"transition_duration";
static NSString *const kIndexEntryTransitionId = @"transition_name";
static NSString *const kIndexEntryAudioResPosKey = @"audio_res_pos";
static NSString *const kIndexEntryAudioRes = @"audio_res";

typedef NS_ENUM(NSUInteger, ListType)
{
    ListTypeNone = 0,
    ListTypeSingle,
    ListTypeIntro,
    ListTypeLoop,
    ListTypeOutro
};

typedef struct {
    CGRect start;
    CGRect end;
} CropRectRamp;

@interface NSArray (DrawInfos)
- (int) totalCountIncludeResource:(BOOL)includeResource;
@end

@implementation NSArray (DrawInfos)
- (int) totalCountIncludeResource:(BOOL)includeResource
{
    if (includeResource) {
        return (int)self.count;
    } else {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"(source_type == 'system') OR (source_type == 'system_mt')"];
        int resourceCount = (int)[self filteredArrayUsingPredicate:predicate].count;
        return (int)(self.count - resourceCount);
    }
}
@end

@interface TemplateV3Handler()
@property (nonatomic, retain) NXETemplateProject *project;
@property (nonatomic) int sceneId;
@property (nonatomic) int subEffectId;
@property (nonatomic) int sceneStartTimeMs;
@property (nonatomic) int sceneDurationMs;
@property (nonatomic) int visualClipIndex;
@property (nonatomic) int sourceCountAtOutro;
@property (nonatomic, readonly) int remainSourceCount;
@property (nonatomic) ListType listType;
@end

@implementation TemplateV3Handler

- (void) dealloc
{
    self.project = nil;
    
    [super dealloc];
}

- (void) setTemplateDataToProject:(NXETemplateProject *)project
{
    self.project = project;
    self.sceneId = 0;
    self.sceneStartTimeMs = 0;
    self.sceneDurationMs = 0;
    self.visualClipIndex = 0;
    self.sourceCountAtOutro = 0;
    self.listType = ListTypeNone;

    NSArray<TemplateComponent *>*list = self.templateHelper.intro;

    if (self.templateHelper.single.count != 0
            && [self.templateHelper.single[0].drawInfos totalCountIncludeResource:NO] >= project.visualClips.count) {
        list = self.templateHelper.single;
    }
    do {
        [self configureScenesWithList:list];

        if (self.listType == ListTypeIntro) {
            list = self.templateHelper.loop;
        }
    } while (self.listType != ListTypeSingle && self.listType != ListTypeOutro);

    [self setBGMToProject:self.project bUsingSingleTemplateBGM:self.listType == ListTypeSingle ? YES : NO];
}

- (void) configureScenesWithList:(NSArray<TemplateComponent *>*)list
{
    if (list == self.templateHelper.intro) {
        self.listType = ListTypeIntro;
    } else if (list == self.templateHelper.loop) {
        self.listType = ListTypeLoop;
    } else if (list == self.templateHelper.single) {
        self.listType = ListTypeSingle;
    } else {
        // either "alternative_outro" or "outro"
        self.listType = ListTypeOutro;
    }
    @autoreleasepool {
        for (TemplateComponent *component in list) {
            [self buildSceneWithTemplateComponent:component];

            if (self.listType != ListTypeSingle && self.listType != ListTypeOutro) {
                if (self.remainSourceCount <= self.sourceCountAtOutro) {
                    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"alternative_id == %@", component.alternative_id];
                    int count = (int)[self.templateHelper.outro filteredArrayUsingPredicate:predicate].count;
                    if (count != 0) {
                        self.templateHelper.outro = [self.templateHelper.outro filteredArrayUsingPredicate:predicate];
                        // filteredArrayUsingPredicate's result count must be just one !!!
                        //
                        [self replaceTemplateComponent:self.templateHelper.outro[0] withDictionary:self.templateHelper.outro[0].alternative_outro[0]];
                    }
                    [self configureScenesWithList:self.templateHelper.outro];
                    return;
                }
            }
        }
    }
}

- (void) buildSceneWithTemplateComponent:(TemplateComponent *)component
{
    self.sceneDurationMs = component.duration;
    self.sceneId += 1;
    self.subEffectId = 1;

    // scene effect
    NexSceneInfo *sceneInfo = [self newSceneInfoWith:component];
    [self.project.sceneInfos addObject:sceneInfo];
    [sceneInfo release];

    // transition effect
    if ((component.transition != nil && ![component.transition isEqualToString:@"none"])
            && component.transition_duration > 0) {
        NexSceneInfo *transitionInfo = [self newTransitionInfoWith:component endTime:sceneInfo.endTimeMS];
        [self.project.sceneInfos addObject:transitionInfo];
        [transitionInfo release];
    }

    for (NSDictionary<NSString *, NSString *> *dictionary in component.drawInfos) {
        // clip
        [self setClipAtSlotWith:dictionary];
        // slot
        NexDrawInfo *drawInfo = [self newDrawInfoWith:dictionary clip:self.project.visualClips[self.visualClipIndex]];
        [self.project.drawInfos addObject:drawInfo];
        [drawInfo release];

        self.visualClipIndex += 1;
        self.subEffectId += 1;
    }

    // set next scene's start time
    int transitionDuration = 0;
    if (self.project.sceneInfos.lastObject.isTransition) {
        transitionDuration = self.project.sceneInfos.lastObject.endTimeMS - self.project.sceneInfos.lastObject.startTimeMS;
    }
    self.sceneStartTimeMs += (component.duration - transitionDuration);
}

- (void) replaceTemplateComponent:(TemplateComponent*)tComponent withDictionary:(NSDictionary<NSString *, NSObject*> *)dictionary
{
    tComponent.effects = (NSString *)dictionary[kIndexEntryEffects];
    tComponent.duration = ((NSString *)dictionary[kIndexEntryDuration]).intValue;
    tComponent.duration_min = ((NSString *)dictionary[kIndexEntryDurationMin]).intValue;
    tComponent.duration_max = ((NSString *)dictionary[kIndexEntryDurationMax]).intValue;
    tComponent.type = (NSString *)dictionary[kIndexEntryType];
    tComponent.transition = (NSString *)dictionary[kIndexEntryTransitionId];
    tComponent.transition_duration = ((NSString *)dictionary[kIndexEntryTransitionDuration]).intValue;
    tComponent.audio_res_pos = ((NSString *)dictionary[kIndexEntryAudioResPosKey]).intValue;
    tComponent.audio_res = (NSString *)dictionary[kIndexEntryAudioRes];
    tComponent.drawInfos = (NSArray <NSDictionary *> *)dictionary[kIndexEntryDrawInfos];
}

- (int)remainSourceCount
{
    return (int)self.project.visualClips.count - self.visualClipIndex;
}

- (int)sourceCountAtOutro
{
    if (_sourceCountAtOutro == 0) {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"alternative_id != nil"];
        int countAlternativeOutroId = (int)[self.templateHelper.outro filteredArrayUsingPredicate:predicate].count;
        if (countAlternativeOutroId > 0
                && self.templateHelper.outro[0].alternative_outro.count != 0) {
            // It was promised that count of the list involved in outro.alternative_outro is all the same
            TemplateComponent *component = self.templateHelper.outro[0];
            NSArray *drawInfos = component.alternative_outro[0][kIndexEntryDrawInfos];
            _sourceCountAtOutro = [drawInfos totalCountIncludeResource:NO];
        }
        else {
            _sourceCountAtOutro = [self.templateHelper.outro totalCountIncludeResource:NO];
        }
    }
    return _sourceCountAtOutro;
}

- (NexSceneInfo *) newSceneInfoWith:(TemplateComponent *)component
{
    NexSceneInfo *sceneInfo = [[NexSceneInfo alloc] init];
    sceneInfo.sceneId = self.sceneId;
    sceneInfo.subEffectId = ((self.sceneId << 16) | 1);
    sceneInfo.effectId = component.effects;
    sceneInfo.startTimeMS = self.sceneStartTimeMs;
    sceneInfo.endTimeMS = self.sceneStartTimeMs + component.duration;
    return [[sceneInfo retain] autorelease];
}

- (NexSceneInfo *) newTransitionInfoWith:(TemplateComponent *)component endTime:(int)endTime
{
    NexSceneInfo *transitionInfo = [[NexSceneInfo alloc] init];
    transitionInfo.sceneId = self.sceneId;
    transitionInfo.subEffectId = ((self.sceneId << 16) | 1);
    transitionInfo.isTransition = YES;
    transitionInfo.effectId = component.transition;
    transitionInfo.startTimeMS = endTime - component.transition_duration;
    transitionInfo.endTimeMS = endTime;
    return [[transitionInfo retain] autorelease];
}

- (CGAffineTransform) abstractDimensionTransformWithSize:(CGSize) size
{
    CGFloat sx = kAbstractDimensionEdgeSize / size.width;
    CGFloat sy = kAbstractDimensionEdgeSize / size.height;
    return CGAffineTransformMakeScale(sx, sy);
}

- (NexDrawInfo *) newDrawInfoWith:(NSDictionary<NSString *, NSString *>*)dictionary clip:(NXEClip *)clip
{
    NSString *lutID = (dictionary[kIndexEntryLUTKey] == nil) ? @"none" : dictionary[kIndexEntryLUTKey];
    int width = NXEEngine.aspectRatio.width;
    if (dictionary[kIndexEntryDrawWidth] != nil && dictionary[kIndexEntryDrawWidth].intValue != 0) {
        width = dictionary[kIndexEntryDrawWidth].intValue;
    }
    int height = NXEEngine.aspectRatio.height;
    if (dictionary[kIndexEntryDrawHeight] != nil && dictionary[kIndexEntryDrawHeight].intValue != 0) {
        height = dictionary[kIndexEntryDrawHeight].intValue;
    }
    NSString *cropMode = (dictionary[kIndexEntryCropMode] == nil) ? @"default" : dictionary[kIndexEntryCropMode];
    NSString *videoCropMode = dictionary[kIndexEntryVideoCropMode];
    NSString *imageCropMode = dictionary[kIndexEntryImageCropMode];
    if (clip.clipType == NXE_CLIPTYPE_VIDEO && videoCropMode != nil) {
        cropMode = videoCropMode;
    } else if (clip.clipType == NXE_CLIPTYPE_IMAGE && imageCropMode != nil) {
        cropMode = imageCropMode;
    }
    
    CGFloat ratio = (CGFloat)width / (CGFloat)height;
    CropRectRamp cropRectRamp = [self cropRectRampForCropMode:cropMode sourceSize:clip.size ratio:ratio];
    CGAffineTransform t = [self abstractDimensionTransformWithSize:clip.size];

    NexDrawInfo *drawInfo = [[NexDrawInfo alloc] init];
    drawInfo.clipId = clip.videoInfo.clipId;
    drawInfo.drawId = (self.sceneId<<16) | self.subEffectId;
    drawInfo.subEffectId = (self.sceneId<<16) | self.subEffectId;
    drawInfo.startTimeMS = clip.videoInfo.startTime;
    drawInfo.endTimeMS = clip.videoInfo.endTime;
    drawInfo.userRotate = clip.videoInfo.rotateState;
    drawInfo.lut = (unsigned int)[LUTMap indexFromLutName:lutID];
    drawInfo.startRect = CGRectApplyAffineTransform(cropRectRamp.start, t);
    drawInfo.endRect = CGRectApplyAffineTransform(cropRectRamp.end, t);
    return [[drawInfo retain] autorelease];
}

- (CropRectRamp) cropRectRampForCropMode:(NSString *)cropMode sourceSize:(CGSize)sourceSize ratio:(CGFloat)ratio
{
    NXECropRectTool *tool = [NXECropRectTool cropRectToolWithSourceSize:sourceSize destinationRatio:ratio];
    NSString *mode = cropMode;

    if ([mode isEqualToString:@"default"]) {
        CGFloat sourceRatio = sourceSize.width / sourceSize.height;
        if (fabs(sourceRatio - ratio) > 0.05) {
            mode = @"pan_rand";
        } else {
            mode = @"fit";
        }
    }

    CropRectRamp result;

    if ([mode isEqualToString:@"fill"]) {
        result.start = result.end = [tool fill];
    } else if ([mode isEqualToString:@"fit"]) {
        result.start = result.end = [tool fit];
    } else {
        CGRect r1 = [tool randomSubRectAtScale:CGPointMake(0.75, 0.75)];
        CGRect r2 = [tool fill];
        r2.origin.y *= (2.0 / 3);
        if (arc4random_uniform(2) == 0) {
            CGRect t = r2;
            r2 = r1;
            r1 = t;
        }
        result.start = r1;
        result.end = r2;
    }

    return result;
}

- (void) setClipAtSlotWith:(NSDictionary<NSString *, NSString*> *)dictionary
{
    NSString *sourceType = (dictionary[kIndexEntrySourceTypeKey] == nil) ? @"ALL" : dictionary[kIndexEntrySourceTypeKey];
    NSString *sourceID = (dictionary[kIndexEntrySourcePathKey] == nil) ? @"default" : dictionary[kIndexEntrySourcePathKey];
    float startRatio = (dictionary[kIndexEntryStartValueKey] == nil) ? 0.0 : dictionary[kIndexEntryStartValueKey].floatValue;
    float endRatio = (dictionary[kIndexEntryEndValueKey] == nil) ? 1.0 : dictionary[kIndexEntryEndValueKey].floatValue;

    NSString *const systemType = @"system";
    NSString *const systemMTType = @"system_mt";

    BOOL isInsertClip = NO;

    NXEClip *clip = nil;

    if ([sourceType isEqualToString:systemType]
            || [sourceType isEqualToString:systemMTType]) {
        NSString *sourcePath = [[AssetLibrary library] itemForId:sourceID].fileInfoResourcePath;
        NSError *error = nil;
        clip = [NXEClip clipWithSource:[NXEClipSource sourceWithPath:sourcePath] error:&error];
        if (clip == nil) {
            NexLogE(LOG_TAG_TEMPLATE, @"Resource clip create error: %@, create solid clip(black)", error.localizedDescription);
            clip = [[NXEClip newSolidClip:@"black"] autorelease];
        }
        isInsertClip = YES;
    } else /* sourceType == user */ {
        if (self.project.visualClips.count <= self.sourceCountAtOutro) {
            if (self.visualClipIndex > self.project.visualClips.count - 1) {
                clip = self.project.visualClips.lastObject;
                isInsertClip = YES;
            } else {
                clip = self.project.visualClips[self.visualClipIndex];
            }
        } else {
            if (self.listType == ListTypeIntro || self.listType == ListTypeLoop) {
                if (self.visualClipIndex > (self.project.visualClips.count - self.sourceCountAtOutro - 1)) {
                    clip = self.project.visualClips[self.visualClipIndex - 1];
                    isInsertClip = YES;
                } else {
                    clip = self.project.visualClips[self.visualClipIndex];
                }
            } else {
                clip = self.project.visualClips[self.visualClipIndex];
            }
        }
    }

    clip.videoInfo.clipId = self.visualClipIndex + 1;

    if (clip.clipType == NXE_CLIPTYPE_VIDEO) {
        clip.videoInfo.startTime = self.sceneStartTimeMs;
        clip.videoInfo.endTime = self.sceneStartTimeMs + clip.durationMs;
        
        if ([sourceType isEqualToString:systemMTType]) {
            clip.videoInfo.isMotionTrackedVideo = YES;
        }
    } else {
        int startTimeMs = self.sceneStartTimeMs + self.sceneDurationMs * startRatio;
        int endTimeMs = self.sceneStartTimeMs + self.sceneDurationMs * endRatio;

        clip.videoInfo.startTime = startTimeMs;
        clip.videoInfo.endTime = endTimeMs;

        [clip setImageClipDuration:(endTimeMs - startTimeMs)];
    }

    if (isInsertClip) {
        NSMutableArray *list = [[self.project.visualClips mutableCopy] autorelease];
        [list insertObject:clip atIndex:self.visualClipIndex];
        self.project.visualClips = list;
    }
}

@end
