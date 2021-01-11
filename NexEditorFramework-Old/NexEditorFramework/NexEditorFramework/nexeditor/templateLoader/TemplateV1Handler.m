/******************************************************************************
 * File Name   :	TemplateV1Handler.m
 * Description : Class to handle Template Version 1.x
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Property
 Copyright (C) 2005~2017 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "TemplateV1Handler.h"
#import <NexEditorEngine/NexEditorEngine.h>

@implementation TemplateV1Handler
{
    /*! paring 작업을 할 list 가리키는 값 */
    NSArray *componentList;
    
    /*! parsing 작업을 할 list index 값 */
    int componentListid;
    
    NXEProject *srcProject;
    NXEProject *dstProject;
    
    /*! debug parameter */
    int counting;
    int introCount;
    int loopCount;
    int outroCount;
}

- (void) setPropertyWithComponent:(TemplateComponent *)component inClip:(NXEClip *)clip
{
    [super configureClip:clip withComponent:component];
    
    if([component.type isEqualToString:@"scene"]) {
        
        if(clip.clipType == NXE_CLIPTYPE_IMAGE) {/* must 6000 for image */
            [clip setImageClipDuration:6000];
        }
    }
    
    /*  하나의 클립은 ClipEffect + TransitionEffect의 구성이기에 아래와 같이 paringListid 값 변경 후, TransitionEffect를 반영해준다.
     */
    int transitionindex = componentListid + 1;
    TemplateComponent *compontent_ = (TemplateComponent *)componentList[transitionindex];
    
    if([compontent_.type isEqualToString:@"transition"]) {
        
        if([compontent_.effects isEqualToString:kEffectIDNone]) {
            [clip setTransitionEffect:kEffectIDNone];
        } else {
            [clip setTransitionEffect:compontent_.effects];
            if(compontent_.duration != -1) {
                [clip setTransitionEffectDuration:(int)compontent_.duration];
                // ? ensureTransitionFit:wantedClipIndex
            }
        }
    }
}

-(void) addOtherClipInProject
{
    TemplateComponent *component = (TemplateComponent *)componentList[componentListid];
    
    NSString *sourcePath = nil;
    NSString *sourceType = nil;
    
    BOOL addOtherClip = YES;
    
    if([component.source_type isEqualToString:@"EXTERNAL_VIDEO"]) {
        
        sourcePath = component.external_video_path;
        sourceType = @"vidoe";
        
    } else if([component.source_type isEqualToString:@"EXTERNAL_IMAGE"]) {
        
        sourcePath = component.external_image_path;
        sourceType = @"image";
        
    } else if([component.source_type isEqualToString:@"SOLID"]) {
        
        sourcePath = component.solid_color;
        sourceType = @"image_solid";
        
    } else {
        
        addOtherClip = NO;
    }

    if(addOtherClip) {
        
        @try {
            
            NXEClip *clip = nil;
            
            if([sourceType isEqualToString:@"image_solid"]) {
                
                clip = [NXEClip newSolidClip:sourcePath];
                
            } else {
                
                clip = [NXEClip newSupportedClip:sourcePath];
            }
            
            [self setPropertyWithComponent:component inClip:clip];
            
            dstProject.visualClips = [dstProject.visualClips arrayByAddingObject:clip];
            
            [clip release];
            
            [self manageComponentList:YES];
            
        } @catch (NSException *exception) {
            NexLogE(LOG_TAG_TEMPLATE, @"not supported clip: inputed path=%@, reason=%@", sourcePath, exception.description);
        }
    }
}

/**
 *  parsingList, parsingListid 관리함수
 */
- (void) manageComponentList:(BOOL)nextClip
{
    componentListid += 2;
    counting++;
    
    NexLogD(LOG_TAG_TEMPLATE, @"after > parsingListid / parsingList = %d / %d", componentListid, (int)[componentList count]);
    
    if(nextClip) {
        
        if(componentListid >= componentList.count) {
            if(componentList == self.templateHelper.intro) {
                
                NexLogD(LOG_TAG_TEMPLATE, @"intro -> loop");
                
                introCount = counting;
                
                counting = 0; componentListid = 0; componentList = nil;
                componentList = self.templateHelper.loop;
                
            } else if(componentList == self.templateHelper.loop) {
                
                NexLogD(LOG_TAG_TEMPLATE, @"loop -> loop");
                
                componentListid = 0;
            }
        }
        
    } else {
        
        if(componentList == self.templateHelper.intro || componentList == self.templateHelper.loop) {
            
            if(componentList == self.templateHelper.intro) {
                NexLogD(LOG_TAG_TEMPLATE, @"intro -> outro");
            } else {
                NexLogD(LOG_TAG_TEMPLATE, @"loop -> outro");
            }
            
            introCount = counting;
            
            counting = 0; componentListid = 0; componentList = nil;
            componentList = self.templateHelper.outro;
            
        } else if(componentList == self.templateHelper.outro) {
            
            outroCount = counting;
        }
    }
}

- (void) addClipInProjectApplyingComponent:(TemplateComponent *)component startTrTime:(int)startTrTime endTrTime:(int)endTrTime inClip:(NXEClip*)clip
{
    NXEClip *clip_ = [clip copy];
    
    if([component.source_type isEqualToString:@"ALL"] ||
       [component.source_type isEqualToString:@"VIDEO"]) {
        
        if(clip_.videoInfo.clipType == NXE_CLIPTYPE_VIDEO) {
            [self setPropertyWithComponent:component inClip:clip_];
            [clip_ setTrim:startTrTime EndTrimTime:endTrTime];
        }
    }
    
    dstProject.visualClips = [dstProject.visualClips arrayByAddingObject:clip_];

    
    [clip_ release];
}

- (void) addClipInProjectApplyingComponent:(TemplateComponent *)component inClip:(NXEClip*)clip
{
    NXEClip *clip_ = [clip copy];
    
    [self setPropertyWithComponent:component inClip:clip_];
    
    dstProject.visualClips = [dstProject.visualClips arrayByAddingObject:clip_];
    
    [clip_ release];
}

- (void) configureProjectWithVideoClip:(NXEClip *)clip curidx:(int)curidx total:(int)total
{
    int introTime = 0; int outroTime = 0;
    int startTime = 0; int endTime = 0;
    int remainTime;
    
    BOOL nextClipInProject = NO;
    
    TemplateComponent *component = nil;
    
    for(TemplateComponent *component in self.templateHelper.intro) {
        introTime += component.duration;
    }
    for(TemplateComponent *component in self.templateHelper.outro) {
        outroTime += component.duration;
    }
    
    startTime = clip.headTrimDuration;
    endTime = clip.trimEndTime;
    
    int sourceTime = endTime - startTime;
    
    if((introTime + outroTime) >= sourceTime) {
        
        remainTime = sourceTime;
        NexLogD(LOG_TAG_TEMPLATE, @"[the sum of intro and outro is over sourceTime] introTime, outroTime, remainTime = %d, %d, %d",
               introTime, outroTime, remainTime);
        
    } else {
        
        remainTime = sourceTime - outroTime;
        NexLogD(LOG_TAG_TEMPLATE, @"[the sum of intro and outro is under sourceTime] introTime, outroTime, remainTime = %d, %d, %d",
               introTime, outroTime, remainTime);
        
    }
    
    int count_ = curidx + 1;
    
    do {
        
        [self addOtherClipInProject];
        
        component = (TemplateComponent *)componentList[componentListid];
        
        int definedTime = component.duration;
        if(definedTime < 0) definedTime = 0;
        
        int remainTime_ = remainTime - definedTime;
        int next_definedTime = component.duration;
        
        if(componentList == self.templateHelper.outro || nextClipInProject) {
            
            [self addClipInProjectApplyingComponent:component startTrTime:startTime endTrTime:endTime inClip:clip];
            
            startTime += endTime;
            
            if(nextClipInProject) {
                
                nextClipInProject = NO;
                
                [self manageComponentList:YES];
                
            } else {
                
                [self manageComponentList:NO];
            }
        } else {
            
            NexLogD(LOG_TAG_TEMPLATE, @"templateid, definedTime, next_definedTime, remainTime = %d, %d, %d, %d",
                    component.templateid, definedTime, next_definedTime, remainTime_);
            
            /**
             *  case 1.  the value, added both introDuration and outroDuration is bigger than sourceDuration.
             *          it applies the value of the half of sourceDuration at both introDuration and outroDuration.
             *          we ignore pre-defined both values.
             */
            if((introTime + outroTime) >= sourceTime) {
                
                [self addClipInProjectApplyingComponent:component startTrTime:startTime endTrTime:endTime inClip:clip];
                
                startTime += remainTime / 2;
                
                if(count_ < total) {
                    
                    NexLogD(LOG_TAG_TEMPLATE, @"video, case1] some clips exist in the source project");
                    
                    nextClipInProject = YES;
                    
                    [self manageComponentList:YES];
                } else {
                    [self manageComponentList:NO];
                }
            }
            // case 2
            else if(remainTime_ > next_definedTime) {
                
                if(remainTime_ < 0) {
                    continue;
                } else {
                    remainTime -= definedTime;
                }
                
                [self addClipInProjectApplyingComponent:component startTrTime:startTime endTrTime:startTime+definedTime inClip:clip];
                
                startTime += definedTime;
                
                [self manageComponentList:YES];
            }
            // case 3
            else {
                [self addClipInProjectApplyingComponent:component startTrTime:startTime endTrTime:startTime+(definedTime+remainTime_/2) inClip:clip];
                
                startTime += (definedTime + remainTime_ / 2);
                
                if(count_ < total) {
                    
                    NexLogD(LOG_TAG_TEMPLATE, @"video, case3] some clips exist in the source project");
                    
                    nextClipInProject = YES;
                    
                    [self manageComponentList:YES];
                } else {
                    [self manageComponentList:NO];
                }
            }
        }
        
    } while (startTime < endTime);
}

- (void) configureProjectWithImageClip:(NXEClip *)clip curidx:(int)curidx total:(int)total
{
    [self addOtherClipInProject];
    
    TemplateComponent *component = (TemplateComponent *)componentList[componentListid];
    
    [self addClipInProjectApplyingComponent:component inClip:clip];
    
    int count = curidx + 1;
    
    if(count < total) {
        
        if(count == (total-1)) {
            
            int lastidx = count-1;
            NXEClip *clip_ = (NXEClip *)srcProject.visualClips[lastidx];
            
            if(clip_.clipType == NXE_CLIPTYPE_VIDEO) {
                
                NexLogD(LOG_TAG_TEMPLATE, @"image] cliptype of last clip is video. go");
                [self manageComponentList:YES];
                
            } else {
                
                NexLogD(LOG_TAG_TEMPLATE, @"image] cliptype of last clip is image. finish");
                [self manageComponentList:NO];
            }
        }
        else {
            
            NexLogD(LOG_TAG_TEMPLATE, @"image] there are some clips in the project.");
            [self manageComponentList:YES];
        }
    }
    else {
        [self manageComponentList:NO]; // For increasing debug parameter, outroCount
    }
}

- (void) configureProject
{
    int curidx = 0;
    int total = (int) srcProject.visualClips.count;
    
    componentList = self.templateHelper.intro;
    componentListid = 0;
    
    NXEClip *clip = nil;
    
    do {
        
        clip = (NXEClip *)srcProject.visualClips[curidx];
        
        if(clip.clipType == NXE_CLIPTYPE_VIDEO) {
            
            [self configureProjectWithVideoClip:clip curidx:curidx total:total];
            
        } else if(clip.clipType == NXE_CLIPTYPE_IMAGE) {
            
            [self configureProjectWithImageClip:clip curidx:curidx total:total];
        }
        
        curidx++;
        
    } while (curidx < total);
}

- (void) setTemplateDataToProject:(NXETemplateProject *)project
{
    srcProject = [project copy];
    
    dstProject = project;
    dstProject.visualClips = @[];
    
    [self configureProject];
    
    NexLogD(LOG_TAG_TEMPLATE, @"total count > intro, loop, outro = %d, %d, %d", introCount, loopCount, outroCount);
    
    [dstProject updateProject];
    
    NSString *name = [self.templateHelper.bgm stringByDeletingPathExtension];
    NSString *extension = [self.templateHelper.bgm pathExtension];
    NSString *bgmPath = [[NSBundle mainBundle] pathForResource:name ofType:extension inDirectory:@"template"];
    
    NXEClip *clip = [NXEClip clipWithSource:[NXEClipSource sourceWithPath:bgmPath] error:NULL];
    if (clip) {
        [dstProject setBGMClip:clip volumeScale:self.templateHelper.bgmvolume];
    }
    
    NSArray<KMImageLayer *> * layers = [dstProject vignetteLayersWithOverlays:self.overlayArray];
    VignetteLayers *vignettes = [[VignetteLayers alloc] initWithImageLayers: layers];
    dstProject.vignetteLayers = vignettes;
    [vignettes release];
    
    [srcProject release];
}

- (instancetype)initWithTemplateHelper:(TemplateHelper *)helper
{
    self = [super initWithTemplateHelper:helper];
    if(self) {
        return self;
    }
    return nil;
}

@end
