/******************************************************************************
 * File Name   :	TemplateHandler.m
 * Description : SuperClass to handle Template
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Property
 Copyright (C) 2005~2017 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "TemplateHandler.h"
#import "NXEAssetItemPrivate.h"
#import "NXEAssetLibrary.h"
#import "AssetLibrary.h"

@implementation OverlayItem
- (void) dealloc
{
    self.displayTime = nil;
    self.variation = nil;
    self.animationEnable = nil;
    self.aniFadeInSTime = nil;
    self.aniFadeOutSTime = nil;
    self.aniFadeInDuration = nil;
    self.aniFadeOutSTime = nil;
    self.aniFadeOutDuration = nil;

    [super dealloc];
}
@end

@implementation TemplateHandler
{
    /*! visualclip index in the project
     *  it is used in applying overlay at a certain clip in the project
     */
    int clipindex;
}

- (void)dealloc
{
    self.templateHelper = nil;
    self.overlayArray = nil;
     
    [super dealloc];
}

- (void)setTemplateDataToProject:(NXETemplateProject *)project
{
}

- (void) setBGMToProject:(NXETemplateProject *)project bUsingSingleTemplateBGM:(BOOL)bUsingSingleTemplateBGM
{
    NSString *bgmPath = [[AssetLibrary library] itemForId:self.templateHelper.bgm].fileInfoResourcePath;
    
    project.options.audioFadeInDuration = self.templateHelper.project_volume_fadein_time;
    project.options.audioFadeOutDuration = self.templateHelper.project_volume_fadeout_time;
    
    if (bUsingSingleTemplateBGM) {
        bgmPath = [[AssetLibrary library] itemForId:self.templateHelper.bgm_single].fileInfoResourcePath;
        
        project.options.audioFadeInDuration = self.templateHelper.project_volume_fadein_time_single;
        project.options.audioFadeOutDuration = self.templateHelper.project_volume_fadeout_time_single;
    }
    
    NXEClip *bgmClip = [NXEClip clipWithSource:[NXEClipSource sourceWithPath:bgmPath] error:NULL];
    if (bgmClip != nil) {
        [project setBGMClip:bgmClip volumeScale:self.templateHelper.bgmvolume];
    }
}

- (instancetype)initWithTemplateHelper:(TemplateHelper *)helper
{
    self = [super init];
    if(self) {
        
        clipindex = 0;
        
        self.templateHelper = helper;
        self.overlayArray = [[[NSMutableArray alloc] init] autorelease];
        return self;
    }
    return nil;
}

#define RATIO_BASE 0.05


/*
 * input: templateRatio, width, height, rotation
 * output: cropModeString
 * cropModeStringWithClipSize:size rotation: rotation
 */
- (NSString *)cropModeStringWithClipSize:(CGSize)size rotation:(int)rotation
{
    NSString *template_mode = self.templateHelper.mode;
    NSArray<NSString *> *splitArray = [template_mode componentsSeparatedByString:@"v"];

    float template_ratio = splitArray[0].floatValue / splitArray[1].floatValue;
    float clip_ratio = (float)size.width / (float)size.height;
    
    if(rotation == 90 || rotation == 270) {
        clip_ratio = (float)size.height / (float)size.width;
    }
    
    if(fabsf(template_ratio - clip_ratio) > RATIO_BASE) {
        return @"pan_rand";
    } else {
        return @"fit";
    }
}

- (void) setCrop:(NXEClip *)clip withCropMode:(NSString *)cropMode
{
    if([cropMode isEqualToString:@"default"]) {
        cropMode = [self cropModeStringWithClipSize:CGSizeMake(clip.videoInfo.width, clip.videoInfo.height) rotation:clip.videoInfo.rotateState];
    }
    
    if([cropMode isEqualToString:@"fill"]) {
        [clip setCropMode:CropModeFill];
    } else if([cropMode isEqualToString:@"pan_rand"]) {
        [clip setCropMode:CropModePanRandom];
    } else {
        [clip setCropMode:CropModeFit];
    }
}

- (void)configureClip:(NXEClip *)clip withComponent:(TemplateComponent *)component
{
    if([component.type isEqualToString:@"scene"]) {
        clip.colorAdjustment = NXEColorAdjustmentsMake((float)component.brightness/255.0, (float)component.contrast/255.0, (float)component.saturation/255.0);
        [clip setColorEffectValues:(NXEPresetColorEffect)component.color_filter];
        [clip setSpeed:component.speed_control];
        [clip setClipVolume:component.volume];
        if(component.effects != nil) {
            if([component.effects isEqualToString:kEffectIDNone]) {
                [clip setClipEffect:kEffectIDNone];
            } else {
                [clip setClipEffect:component.effects];
            }
        }
        if(component.lut != nil) {
            NSUInteger lutType = [LUTMap indexFromLutName:component.lut];
            if(lutType != NXE_LUT_NONE) {
                clip.videoInfo.lut = (int)lutType;
            }
        }
        if(component.vignette != nil) {
            if([component.vignette hasPrefix:@"clip"]) {
                NSArray *values = [component.vignette componentsSeparatedByString:@","];
                NSString *value = values[1];
                
                if([value isEqualToString:@"yes"]) {
                    [clip setVignette:YES];
                } else {
                    [clip setVignette:NO];
                }
            } else if([component.vignette hasPrefix:@"overlay"]) {
                NSArray *values = [component.vignette componentsSeparatedByString:@","];
                NSString *displayTime = values[1];
                NSString *variation = values[2];
                NSString *animationEnable = values[3];
                NSString *aniFadeInSTime = values[4];
                NSString *aniFadeInDuration = values[5];
                NSString *aniFadeOutSTime = values[6];
                NSString *aniFadeOutDuration = values[7];
                
                OverlayItem *overlayItem = [[OverlayItem alloc] init];
                overlayItem.overlayId = clipindex;
                overlayItem.displayTime = [NSString stringWithString:displayTime];
                overlayItem.variation = [NSString stringWithString:variation];
                overlayItem.animationEnable = [NSString stringWithString:animationEnable];
                overlayItem.aniFadeInSTime = [NSString stringWithString:aniFadeInSTime];
                overlayItem.aniFadeInDuration = [NSString stringWithString:aniFadeInDuration];
                overlayItem.aniFadeOutSTime = [NSString stringWithString:aniFadeOutSTime];
                overlayItem.aniFadeOutDuration = [NSString stringWithString:aniFadeOutDuration];
                
                [self.overlayArray addObject:overlayItem];
                [overlayItem release];
            }
        }
        NSString *cropMode = component.crop_mode;
        if(clip.clipType == NXE_CLIPTYPE_IMAGE && component.image_crop_mode != nil) {
            cropMode = component.image_crop_mode;
        } else if(clip.clipType == NXE_CLIPTYPE_VIDEO && component.video_crop_mode != nil) {
            cropMode = component.video_crop_mode;
        }
        [self setCrop:clip withCropMode:cropMode];
    }
    
    clipindex++;
}

@end
