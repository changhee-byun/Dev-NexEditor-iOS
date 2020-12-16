/**
 * File Name   : TemplateV2Handler.m
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

#import "TemplateV2Handler.h"
#import "NXEProjectInternal.h"
#import "NXEAssetLibrary.h"
#import "AssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import <NexEditorEngine/NexEditorEngine.h>

@interface TemplateV2Handler ()

@property (nonatomic, retain) NSMutableArray<VisualClipEffectTimeGroup *> *visualClipEffectTimeGroup;
@property (nonatomic, retain) NSMutableArray<AudioStartTimeScale *> *audioStartTimeScales;

@end

@implementation TemplateV2Handler
{
    /*! paring 작업을 할 list 가리키는 값 */
    NSArray *componentList;
    
    /*! parsing 작업을 할 list index 값 */
    int componentListid;
    
    NSArray *lastList;
    NSString *alternative_identifier;
    
    NXETemplateProject *srcProject;
    NXETemplateProject *dstProject;
    
    BOOL chagneComponentList;
    BOOL singleProject;
    
    /*! time except for the duration of outro */
    int remainDuration;
    int outroDuration;
    
    /*! effect count */
    int startClipIndex;
    int clipCountPerComponent;
    
    /*! effect duration*/
    int effectDuration;
    int effectMinDuration;
    int effectMaxDuration;
}

- (void) dealloc
{
    componentList = nil;
    
    self.visualClipEffectTimeGroup = nil;
    self.audioStartTimeScales = nil;
    
    [super dealloc];
}

#pragma mark - video clip

#define VIDEO_EFFECT_TRANSITION_GUARANTEE_TIME 1000

- (void) addVideoClip:(NXEClip *)clip withComponent:(TemplateComponent *)component startTrimTime:(int)sTrimTime endTrimTime:(int)eTrimTime addVisualClipEffectTimeGroup:(BOOL)addVisualClipEffectTimeGroup
{
    NXEClip *copyedClip = [clip copy];
    
    [copyedClip setTrim:sTrimTime EndTrimTime:eTrimTime];
    
    [self setProperty:copyedClip withComponet:component addVisualClipEffectTimeGroup:addVisualClipEffectTimeGroup];
    
    [self guaranteeDurationOfCurClip:copyedClip withPrevClip:(NXEClip*)dstProject.visualClips.lastObject];
    
    dstProject.visualClips = [dstProject.visualClips arrayByAddingObject:copyedClip];
    
    NSUInteger copyedClipIndex = [dstProject.visualClips indexOfObject:copyedClip];
    [self addAudioResourceWithComponent:component visualClipIndex:copyedClipIndex];
    
    [copyedClip release];
}

- (void) constistProjectWithVideoClip:(NXEClip *)clip startTime:(int)startTime endTime:(int)endTime newScene:(BOOL *)newScene
{
    BOOL loopContinue = YES;
    BOOL addVisualClipEffectTimeGroup = YES;
    
    int startTrimTime = 0; int endTrimTime = startTime;
    int remainClipDuration = endTime - startTime;
    
    TemplateComponent *component = nil;
    
    do {
        
        [self addResource];
        
        component = (TemplateComponent *)componentList[componentListid];
        
        if ( *newScene ) {
            effectDuration = component.duration + component.transition_duration;
            effectMinDuration = component.duration_min + component.transition_duration;
            effectMaxDuration = component.duration_max + component.transition_duration;
            
            if ( effectDuration < 0 ) {
                *newScene = YES;
            }
        }
        
        startTrimTime = endTrimTime;
        endTrimTime = endTime;
        
        if(startTrimTime == endTrimTime) {
            NexLogE(LOG_TAG_TEMPLATE, @"it doesn't proceed anymore because both startTrimTime and endTrimTime is same");
            return;
        }
        
        if(remainClipDuration > effectMaxDuration) {
            
            remainClipDuration -= effectDuration;
            
            if(remainClipDuration > VIDEO_EFFECT_TRANSITION_GUARANTEE_TIME) {
                
                int duration = startTrimTime + effectDuration;
                
                if(duration > endTime) {
                    loopContinue = NO;
                    endTrimTime = endTime;
                } else {
                    endTrimTime = duration;
                }
            }
        } else {
            loopContinue = NO;
        }
        
        *newScene = YES;
        addVisualClipEffectTimeGroup = YES;
        
        int clipDuration = endTrimTime - startTrimTime;
        
        remainDuration -= clipDuration;
        if ( remainDuration <= 0 ) {
            endTrimTime = endTime;
        }
        
        if ( component.apply_effect_on_res && (effectDuration - clipDuration) > 0 ) {
            *newScene = NO;
            addVisualClipEffectTimeGroup = NO;
            
            if ( componentList == lastList && remainDuration == 0 ) {
                *newScene = YES;
                addVisualClipEffectTimeGroup = YES;
            }
            
            effectDuration -= clipDuration;
            effectMinDuration -= clipDuration;
            effectMaxDuration -= clipDuration;
        }
        
        NexLogD(LOG_TAG_TEMPLATE, @"clip duration:%d, remainTime:%d, startTrimTime:%d, endTrimTime:%d", clip.videoInfo.totalTime, remainDuration, startTrimTime, endTrimTime);
        
        [self addVideoClip:clip withComponent:component startTrimTime:startTrimTime endTrimTime:endTrimTime addVisualClipEffectTimeGroup:addVisualClipEffectTimeGroup];
        [self changeComponent:*newScene];
        
    } while (loopContinue);
}

#pragma mark - image clip

#define IMAGE_EFFECT_TRANSITION_GUARANTEE_TIME 5000

- (BOOL) setImageClipDuration:(NXEClip *)clip withComponent:(TemplateComponent *)component newScene:(BOOL *)newScene resourceClip:(BOOL)resourceClip
{
    BOOL addVisualClipEffectTimeGroup = YES;
    
    if ( *newScene ) {
        effectDuration = component.duration + component.transition_duration;
        effectMinDuration = component.duration_min + component.transition_duration;
        effectMaxDuration = component.duration_max + component.transition_duration;
    }
    
    *newScene = YES;
    int clipDuration = clip.durationMs;
    
    if (!resourceClip)
        remainDuration -= clipDuration;
    
    if ( component.apply_effect_on_res ) {
        if ( effectDuration > clipDuration ) {
            *newScene = NO;
            addVisualClipEffectTimeGroup = NO;
            
            effectDuration -= clipDuration;
            effectMinDuration -= clipDuration;
            effectMaxDuration -= clipDuration;
            
            if ( componentList == lastList && remainDuration == 0 ) {
                *newScene = YES;
                addVisualClipEffectTimeGroup = YES;
            }
        } else {
            if ( (effectMinDuration <= clipDuration && clipDuration <= effectMaxDuration) || effectDuration <= IMAGE_EFFECT_TRANSITION_GUARANTEE_TIME ) {
                int time = effectMaxDuration < clipDuration ? effectMaxDuration : effectMinDuration < clipDuration ? clipDuration : effectMinDuration;
                [clip setImageClipDuration:time];
            }
            *newScene = YES;
            addVisualClipEffectTimeGroup = YES;
        }
    } else {
        int time = effectMaxDuration < clipDuration ? effectMaxDuration : effectMinDuration < clipDuration ? clipDuration : effectMinDuration;
        [clip setImageClipDuration:time];
    }
    
    return addVisualClipEffectTimeGroup;
}

- (void) addImageClip:(NXEClip *)clip withComponent:(TemplateComponent *)component newScene:(BOOL *)newScene resourceClip:(BOOL)resourceClip
{
    NXEClip *copyedClip = [clip copy];
    
    BOOL addVisualClipEffectTimeGroup = [self setImageClipDuration:copyedClip withComponent:component newScene:newScene resourceClip:resourceClip];
    [self setProperty:copyedClip withComponet:component addVisualClipEffectTimeGroup:addVisualClipEffectTimeGroup];
    
    [self guaranteeDurationOfCurClip:copyedClip withPrevClip:(NXEClip*)dstProject.visualClips.lastObject];
    
    dstProject.visualClips = [dstProject.visualClips arrayByAddingObject:copyedClip];

    NSUInteger copyedClipIndex = [dstProject.visualClips indexOfObject:copyedClip];
    [self addAudioResourceWithComponent:component visualClipIndex:copyedClipIndex];
    
    [copyedClip release];
}

- (void) constistProjectWithImageClip:(NXEClip *)clip newScene:(BOOL *)newScene
{
    [self addResource];
    [self addImageClip:clip withComponent:(TemplateComponent *)componentList[componentListid] newScene:newScene resourceClip:NO];
    [self changeComponent:*newScene];
}

#pragma mark - clip common
    
- (void) guaranteeDurationOfCurClip:(NXEClip *)curClip withPrevClip:(NXEClip *)prevClip
{
    int displayDurationOfCurClip = curClip.speedControlledTrimDuration - curClip.videoInfo.effectDuration * ((float)curClip.videoInfo.effectOffset / 100.0);
    
    if(displayDurationOfCurClip <= 0) {
        [curClip setTransitionEffect:kEffectIDNone];
    }
    if(prevClip.videoInfo.effectDuration >= displayDurationOfCurClip) {
        [prevClip setTransitionEffect:kEffectIDNone];
    }
}

-(void) addResource
{
    TemplateComponent *component = (TemplateComponent *)componentList[componentListid];
    
    NSString *resourceId = nil;
    NSString *sourceType = nil;
    
    BOOL addClip = YES;
    
    resourceId = component.res_path;
    
    if([component.source_type isEqualToString:@"res_video"]) {
        
        sourceType = @"video";
        
    } else if([component.source_type isEqualToString:@"res_image"]) {
        
        sourceType = @"image";
        
    } else if([component.source_type isEqualToString:@"res_solid"]) {
        
        sourceType = @"image_solid";
        
    } else {
        
        addClip = NO;
    }
    
    if(addClip) {
        
        @try {
            
            NXEClip *clip = nil;
            BOOL newScene = YES; // if the resource in the template is added to the project, we can ignore the key such as "apply_effect_on_res"
            
            if([sourceType isEqualToString:@"video"]) {
                NSString *sourcePath = [[AssetLibrary library] itemForId:resourceId].fileInfoResourcePath;
                if(sourcePath != nil) {
                    clip = [NXEClip newSupportedClip:sourcePath];
                    
                    [self addVideoClip:clip withComponent:component startTrimTime:0 endTrimTime:0 addVisualClipEffectTimeGroup:YES];
                    [self changeComponent:newScene];
                }
            } else {
                if([sourceType isEqualToString:@"image_solid"]) {
                    NSString *solidColor = resourceId;
                    clip = [NXEClip newSolidClip:solidColor];
                    
                    [self addImageClip:clip withComponent:component newScene:&newScene resourceClip:YES];
                    [self changeComponent:newScene];
                } else {
                    NSString *sourcePath = [[AssetLibrary library] itemForId:resourceId].fileInfoResourcePath;
                    if(sourcePath != nil) {
                        clip = [NXEClip newSupportedClip:sourcePath];
                        
                        [self addImageClip:clip withComponent:component newScene:&newScene resourceClip:YES];
                        [self changeComponent:newScene];
                    }
                }
            }
            
            [clip release];
            
        } @catch (NSException *exception) {
            NexLogE(LOG_TAG_TEMPLATE, @"Error: resourceId=%@, reason=%@", resourceId, exception.description);
        }
    }
}

- (void) setProperty:(NXEClip *)clip withComponet:(TemplateComponent *)component addVisualClipEffectTimeGroup:(BOOL)addVisualClipEffectTimeGroup
{
    [super configureClip:clip withComponent:component];
    
    if([component.type isEqualToString:@"scene"]) {
        
        clipCountPerComponent++;
        
        if(addVisualClipEffectTimeGroup == NO) {
            
            [clip setTransitionEffect:kEffectIDNone];
            
        } else {
            
            if(component.transition != nil) {
                
                [clip setTransitionEffect:component.transition];
                [clip setTransitionEffectDuration:component.transition_duration];
                
            } else {
                
                [clip setTransitionEffect:kEffectIDNone];
            }
            
            if(componentList == self.templateHelper.outro) {
                if(component.alternative_effect != nil && alternative_identifier != nil) {
                    if(component.alternative_effect[alternative_identifier] != nil) {
                        [clip setClipEffect:component.alternative_effect[alternative_identifier]];
                    }
                }
                if(component.alternative_transition != nil && alternative_identifier != nil) {
                    if(component.alternative_transition[alternative_identifier] != nil) {
                        [clip setTransitionEffect:component.alternative_transition[alternative_identifier]];
                    }
                }
                if(component.alternative_lut != nil && alternative_identifier != nil) {
                    if(component.alternative_lut[alternative_identifier] != nil) {
                        NSUInteger lutType = [LUTMap indexFromLutName:component.alternative_lut[alternative_identifier]];
                        if(lutType != NXE_LUT_NONE) {
                            clip.videoInfo.lut = (int)lutType;
                        }
                    }
                }
            }
            
            VisualClipEffectTimeGroup *visualClipEffectTimeGroup = [[VisualClipEffectTimeGroup alloc] initWithStartClipIndex:startClipIndex count:clipCountPerComponent];
            
            [self.visualClipEffectTimeGroup addObject:visualClipEffectTimeGroup];
            [visualClipEffectTimeGroup release];
            
            startClipIndex += clipCountPerComponent;
            clipCountPerComponent = 0;
        }
    }
}

- (void) addAudioResourceWithComponent:(TemplateComponent *)component visualClipIndex:(NSUInteger)visualClipIndex
{
    if(component.audio_res != nil) {
        
        @try {
            
            NSString *audioClipPath = [[AssetLibrary library] itemForId:component.audio_res].fileInfoResourcePath;
            NSUInteger audioScale = component.audio_res_pos;
            
            if(componentList == self.templateHelper.outro) {
                if(component.alternative_audio != nil && alternative_identifier != nil) {
                    if(component.alternative_audio[alternative_identifier] != nil) {
                        audioClipPath = [[AssetLibrary library] itemForId:component.alternative_audio[alternative_identifier]].fileInfoResourcePath;
                    }
                }
                if(component.alternative_audio_pos != nil && alternative_identifier != nil) {
                    if(component.alternative_audio_pos[alternative_identifier] != nil) {
                        audioScale = [component.alternative_audio_pos[alternative_identifier] integerValue];
                    }
                }
            }
            
            NXEClip *audioClip = [NXEClip newSupportedClip:audioClipPath];
            
            audioClip.bLoop = NO;
            
            dstProject.audioClips = [dstProject.audioClips arrayByAddingObject:audioClip];
            
            NSUInteger audioClipIndex = [dstProject.audioClips indexOfObject:audioClip];
            
            AudioStartTimeScale *audioStartTimeScale = [[AudioStartTimeScale alloc] initWithVisualClipIndex:visualClipIndex audioClipIndex:audioClipIndex startTimeScale:audioScale];
            
            [self.audioStartTimeScales addObject:audioStartTimeScale];
            [audioStartTimeScale release];
            
        } @catch (NSException *exception) {
            NexLogE(LOG_TAG_TEMPLATE, @"Error: inputed auio_res=%@, reason=%@", component.audio_res, exception.description);
        }
    }
}

- (NSString *)getAlternativeIdentifier
{
    TemplateComponent *component = componentList[componentListid];
    return component.identifier;
}

- (void) changeComponent:(BOOL)newScene
{
    if(remainDuration <= 0 && componentList != lastList) {
        
        alternative_identifier = [self getAlternativeIdentifier];
        
        componentListid = 0;
        componentList = lastList;
        remainDuration = outroDuration;
        
    } else {
        
        if(newScene) {
            
            componentListid++;
            
            if(componentListid >= componentList.count) {
                
                if(componentList == self.templateHelper.intro) {
                    
                    componentListid = 0;
                    componentList = self.templateHelper.loop;
                    
                } else {
                    
                    componentListid = 0;
                }
            }
        }
    }
}

- (void)initParams
{
    int basedOutroCnt = 0;
    int basedOutroDuration = 0;
    
    lastList = self.templateHelper.outro;
    
    for(TemplateComponent *component in lastList) {
        if ( [component.source_type isEqualToString:@"res_video"] || [component.source_type isEqualToString:@"res_image"] || [component.source_type isEqualToString:@"res_solid"] ) {
            continue;
        }
        basedOutroDuration += component.duration;
        basedOutroCnt++;
    }
    
    int calculatedOutroDuration = 0;
    
    for(int i=(int)srcProject.visualClips.count-1; i>=0; i--) {
        
        if(basedOutroDuration < 800 || basedOutroCnt == 0) break;
        
        NXEClip *clip = srcProject.visualClips[i];
        
        if(clip.clipType == NXE_CLIPTYPE_VIDEO) {
            if(clip.speedControlledTrimDuration < basedOutroDuration) {
                calculatedOutroDuration += clip.speedControlledTrimDuration;
                basedOutroDuration -= clip.speedControlledTrimDuration;
            } else {
                calculatedOutroDuration += basedOutroDuration;
                break;
            }
        } else if(clip.clipType == NXE_CLIPTYPE_IMAGE) {
            
            calculatedOutroDuration += clip.durationMs;
            basedOutroDuration -= clip.durationMs;
            basedOutroCnt--;
        }
    }
    
    componentListid = 0; componentList = self.templateHelper.intro;
    
    outroDuration = calculatedOutroDuration;
    
    remainDuration = [srcProject getTotalTime] - outroDuration;
    
    if(srcProject.visualClips.count == 1 &&
       ((NXEClip *)srcProject.visualClips[0]).clipType == NXE_CLIPTYPE_IMAGE) {
        
        if(self.templateHelper.single.count != 0) {
            lastList = self.templateHelper.single;
        }
        componentList = lastList;
        remainDuration = outroDuration;
    } else {
        if(remainDuration <= 0) {
            if(self.templateHelper.single.count != 0) {
                lastList = self.templateHelper.single;
            }
            componentList = lastList;
            remainDuration = outroDuration;
        }
    }
    
    if(componentList == self.templateHelper.single) {
        singleProject = YES;
    }
}

- (void) configureProject
{
    int curidx = 0;
    NSUInteger total = srcProject.visualClips.count;
    
    [self initParams];
    
    NXEClip *clip = nil;
    BOOL newScene = YES;
    
    do {
        
        clip = (NXEClip *)srcProject.visualClips[curidx];
        
        if(clip.clipType == NXE_CLIPTYPE_VIDEO) {
            
            int startDuration = clip.headTrimDuration;
            int endDuration = startDuration + clip.speedControlledTrimDuration;
            
            if(remainDuration > 0 && remainDuration < clip.speedControlledTrimDuration) {
                
                int endTime = startDuration + remainDuration;
                
                if( (endDuration - endTime) >= VIDEO_EFFECT_TRANSITION_GUARANTEE_TIME ) {

                    [self constistProjectWithVideoClip:clip startTime:startDuration endTime:endTime newScene:&newScene];
                    
                    startDuration = endTime;
                }
                
                if(componentList != lastList) {
                    componentListid = 0; componentList = lastList;
                    remainDuration = outroDuration;
                }
            }
            
            [self constistProjectWithVideoClip:clip startTime:startDuration endTime:endDuration newScene:&newScene];
            
        } else if(clip.clipType == NXE_CLIPTYPE_IMAGE) {
            
            [self constistProjectWithImageClip:clip newScene:&newScene];
        }
        
        curidx++;
        
    } while (curidx < total);
    
    [self addResource];
}


- (void) setTemplateDataToProject:(NXETemplateProject *)project
{
    BOOL existImageClip = NO;
    
    project.visualClipEffectTimeGroups = @[];
    project.adjustedAudioStartTimeScales = @[];

    for(NXEClip *clip in project.visualClips) {
        
        if(clip.clipType == NXE_CLIPTYPE_IMAGE) {
            
            existImageClip = YES;
            [clip setImageClipDuration:self.templateHelper.image_duration];
        }
    }
    
    if(existImageClip) {
        
        [project updateProject];
    }
    
    srcProject = [project copy];
    
    dstProject = project;
    dstProject.visualClips = @[];
    
    [self configureProject];

    dstProject.visualClipEffectTimeGroups = self.visualClipEffectTimeGroup;
    dstProject.adjustedAudioStartTimeScales = self.audioStartTimeScales;
    
    [dstProject updateProject];
    
    [self setBGMToProject:dstProject bUsingSingleTemplateBGM:singleProject ? YES : NO];
    
    NSArray<KMImageLayer *> * layers = [dstProject vignetteLayersWithOverlays:self.overlayArray];
    // NESI-458 Assign vignetteLayers property with manual retain/release controlled instance.
    // Autorelease marked object can result in Vignette KMImageLayer objects kept in place.
    VignetteLayers *vignettes = [[VignetteLayers alloc] initWithImageLayers:layers];
    dstProject.vignetteLayers = vignettes;
    [vignettes release];
    
    [srcProject release];
}

- (instancetype)initWithTemplateHelper:(TemplateHelper *)helper
{
    self = [super initWithTemplateHelper:helper];
    if(self) {
        
        self.visualClipEffectTimeGroup = [[[NSMutableArray alloc] init] autorelease];
        self.audioStartTimeScales = [[[NSMutableArray alloc] init] autorelease];
        
        return self;
    }
    return nil;
}

@end
