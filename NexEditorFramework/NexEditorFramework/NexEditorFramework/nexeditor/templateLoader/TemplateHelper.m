/**
 * File Name   : TemplateHelper.m
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

#import "TemplateHelper.h"
#import "TemplateHelper+Utility.h"

#import "NXEErrorPrivate.h"

// HEADER
#define KEY_TEMPLATE_NAME @"template_name"
#define KEY_TEMPLATE_DESC @"template_desc"
#define KEY_TEMPLATE_VERSION @"template_version"
#define KEY_TEMPLATE_MODE @"template_mode"
#define KEY_TEMPLATE_BGM @"template_bgm"
#define KEY_TEMPLATE_BGM_VOLUME @"template_bgm_volume"
#define KEY_TEMPLATE_DEFAULT_EFFECT @"template_default_effect"
#define KEY_TEMPLATE_DEFAULT_EFFECT_SCALE @"template_default_effect_scale"
#define KEY_TEMPLATE_DEFAULT_IMAGE_DURATION @"template_default_image_duration"
#define KEY_TEMPLATE_MIN_DURATION @"template_min_duration"
#define KEY_TEMPLATE_PROJECT_VOL_FADE_IN_TIME @"template_project_vol_fade_in_time"
#define KEY_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME @"template_project_vol_fade_out_time"
#define KEY_TEMPLATE_BGM_SINGLE @"template_single_bgm"
#define KEY_TEMPLATE_PROJECT_VOL_FADE_IN_TIME_SINGLE @"template_single_project_vol_fade_in_time"
#define KEY_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME_SINGLE @"template_single_project_vol_fade_out_time"

// COMPONENT
#define KEY_VOLUME @"volume"
#define KEY_SPEED_CONTROL @"speed_control"
#define KEY_SATURATION @"saturation"
#define KEY_BRIGHTNESS @"brightness"
#define KEY_CONTRAST @"contrast"
#define KEY_COLOR_FILTER @"color_filter"
#define KEY_DURATION @"duration"
#define KEY_DURATION_MIN @"duration_min"
#define KEY_DURATION_MAX @"duration_max"
#define KEY_APPLY_EFFECT_ON_RES @"apply_effect_on_res"
#define KEY_TYPE @"type"
#define KEY_CROP_MODE @"crop_mode"
#define KEY_VIDEO_CROP_MODE @"video_crop_mode"
#define KEY_IMAGE_CROP_MODE @"image_crop_mode"
#define KEY_EFFECTS @"effects"
#define KEY_VIGNETTE @"vignette"
#define KEY_SOURCE_TYPE @"source_type"
#define KEY_RES_PATH @"res_path"
#define KEY_LUT @"lut"
#define KEY_SOLID_COLOR @"solid_color"
#define KEY_EXTERNAL_IMAGE_PATH @"external_image_path"
#define KEY_EXTERNAL_VIDEO_PATH @"external_video_path"
#define KEY_AUDIO_RES @"audio_res"
#define KEY_AUDIO_RES_POS @"audio_res_pos"
#define KEY_TRANSIION_NAME @"transition_name"
#define KEY_TRANSITION_DURATION @"transition_duration"
#define KEY_ALTERNATIVE_IDENTIFIER @"identifier"
#define KEY_ALTERNATIVE_EFFECT @"alternative_effect"
#define KEY_ALTERNATIVE_TRANSITION @"alternative_transition"
#define KEY_ALTERNATIVE_LUT @"alternative_lut"
#define KEY_ALTERNATIVE_AUDIO @"alternative_audio"
#define KEY_ALTERNATIVE_AUDIO_POS @"alternative_audio_pos"
#define KEY_DRAWINFOS @"draw_infos"
#define KEY_ALTERNATIVE_OUTRO_ID @"alternative_id"
#define KEY_ALTERNATIVE_OUTRO @"alternative_outro"

@implementation TemplateComponent
    
- (void)buildComponentWithDictionary:(NSDictionary *)dictionary templateId:(int)templateId
{
    self.templateid = templateId;
    self.volume = [TemplateHelper intValueWithDictionary:dictionary key:KEY_VOLUME defaultValue:100];
    self.speed_control = [TemplateHelper intValueWithDictionary:dictionary key:KEY_SPEED_CONTROL defaultValue:100];
    self.saturation = [TemplateHelper intValueWithDictionary:dictionary key:KEY_SATURATION defaultValue:0];
    self.brightness = [TemplateHelper intValueWithDictionary:dictionary key:KEY_BRIGHTNESS defaultValue:0];
    self.contrast = [TemplateHelper intValueWithDictionary:dictionary key:KEY_CONTRAST defaultValue:0];
    self.color_filter = [TemplateHelper intValueWithDictionary:dictionary key:KEY_COLOR_FILTER defaultValue:0];
    self.duration = [TemplateHelper intValueWithDictionary:dictionary key:KEY_DURATION defaultValue:0];
    self.duration_max = [TemplateHelper intValueWithDictionary:dictionary key:KEY_DURATION_MAX defaultValue:0];
    self.duration_min = [TemplateHelper intValueWithDictionary:dictionary key:KEY_DURATION_MIN defaultValue:0];
    self.type = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_TYPE defaultValue:nil];
    self.crop_mode = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_CROP_MODE defaultValue:@"default"];
    self.video_crop_mode = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_VIDEO_CROP_MODE defaultValue:nil];
    self.image_crop_mode = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_IMAGE_CROP_MODE defaultValue:nil];
    self.effects = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_EFFECTS defaultValue:nil];
    self.vignette = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_VIGNETTE defaultValue:nil];
    self.solid_color = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_SOLID_COLOR defaultValue:nil];
    self.source_type = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_SOURCE_TYPE defaultValue:nil];
    self.res_path = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_RES_PATH defaultValue:nil];
    self.lut = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_LUT defaultValue:nil];
    self.external_image_path = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_EXTERNAL_IMAGE_PATH defaultValue:nil];
    self.external_video_path = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_EXTERNAL_VIDEO_PATH defaultValue:nil];
    self.apply_effect_on_res = [TemplateHelper intValueWithDictionary:dictionary key:KEY_APPLY_EFFECT_ON_RES defaultValue:0];
    self.audio_res = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_AUDIO_RES defaultValue:nil];
    self.audio_res_pos = [TemplateHelper intValueWithDictionary:dictionary key:KEY_AUDIO_RES_POS defaultValue:0];
    self.transition = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_TRANSIION_NAME defaultValue:nil];
    self.transition_duration = [TemplateHelper intValueWithDictionary:dictionary key:KEY_TRANSITION_DURATION defaultValue:0];
    self.identifier = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_ALTERNATIVE_IDENTIFIER defaultValue:nil];
    self.alternative_effect = [[[NSMutableDictionary alloc] initWithDictionary:dictionary[KEY_ALTERNATIVE_EFFECT] copyItems:YES] autorelease];
    self.alternative_lut = [[[NSMutableDictionary alloc] initWithDictionary:dictionary[KEY_ALTERNATIVE_LUT] copyItems:YES] autorelease];
    self.alternative_transition = [[[NSMutableDictionary alloc] initWithDictionary:dictionary[KEY_ALTERNATIVE_TRANSITION] copyItems:YES] autorelease];
    self.alternative_audio = [[[NSMutableDictionary alloc] initWithDictionary:dictionary[KEY_ALTERNATIVE_AUDIO] copyItems:YES] autorelease];
    self.alternative_audio_pos = [[[NSMutableDictionary alloc] initWithDictionary:dictionary[KEY_ALTERNATIVE_AUDIO_POS] copyItems:YES] autorelease];
    self.drawInfos = [[[NSArray alloc] initWithArray:dictionary[KEY_DRAWINFOS] copyItems:YES] autorelease];
    self.alternative_id = [TemplateHelper stringValueWithDictionary:dictionary key:KEY_ALTERNATIVE_OUTRO_ID defaultValue:nil];
    self.alternative_outro = [[[NSArray alloc] initWithArray:dictionary[KEY_ALTERNATIVE_OUTRO] copyItems:YES] autorelease];
    if(self.transition == nil) self.transition_duration = 0;
}

- (void)dealloc
{
    self.alternative_effect = nil;
    self.alternative_lut = nil;
    self.alternative_transition = nil;
    self.alternative_audio = nil;
    self.alternative_audio_pos = nil;
    self.drawInfos = nil;
    self.alternative_outro = nil;
    
    [super dealloc];
}

- (instancetype)initWithRawDictionary:(NSDictionary *)dictionary templateId:(int)templateId
{
    self = [super init];
    if(self) {
        
        [self buildComponentWithDictionary:dictionary templateId:templateId];
        return self;
    }
    return nil;
}
@end

@implementation TemplateHelper
{
    int templateId;
}

- (void)dealloc
{
    self.intro = nil;
    self.loop = nil;
    self.outro = nil;
    self.name = nil;
    self.desc = nil;
    self.version = nil;
    self.mode = nil;
    self.default_effect = nil;
    self.bgm = nil;
    self.single = nil;
    self.bgm_single = nil;
    [super dealloc];
}

- (NSArray *)buildComponent:(NSArray *)rawComponents
{
    templateId = 0;
    
    NSMutableArray *list = [[[NSMutableArray alloc] init] autorelease];
    if(rawComponents.count != 0) {
        
        TemplateComponent *component = nil;
        for(NSDictionary *raw in rawComponents) {
            
            component = [[TemplateComponent alloc] initWithRawDictionary:raw templateId:templateId++];
            [list addObject:component];
            [component release];
        }
    }
    
    return [[list copy] autorelease];
}

- (NSString *)setupTemplateHelper:(NSData *)data deepScan:(BOOL)deepScan
{
    NSError *error;
    
    NSDictionary *list = [NSJSONSerialization JSONObjectWithData:data
                                                         options:NSJSONReadingAllowFragments
                                                           error:&error];
    if(list == nil) {
        
        NSLog(@"error description:%@", error.localizedDescription);
        return error.localizedDescription;
    }
    
    self.name = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_NAME defaultValue:@""];
    self.desc = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_DESC defaultValue:@""];
    self.version = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_VERSION defaultValue:@""];
    self.mode = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_MODE defaultValue:@""];
    self.default_effect = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_DEFAULT_EFFECT defaultValue:nil];
    self.effect_scale = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_DEFAULT_EFFECT_SCALE defaultValue:0];
    self.image_duration = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_DEFAULT_IMAGE_DURATION defaultValue:2500];
    self.min_duration = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_MIN_DURATION defaultValue:0];
    self.project_volume_fadein_time = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_PROJECT_VOL_FADE_IN_TIME defaultValue:200];
    self.project_volume_fadeout_time = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME defaultValue:5000];
    self.bgmvolume = [TemplateHelper floatValueWithDictionary:list key:KEY_TEMPLATE_BGM_VOLUME defaultValue:0.5f];
    self.bgm = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_BGM defaultValue:nil];
    self.bgm_single = [TemplateHelper stringValueWithDictionary:list key:KEY_TEMPLATE_BGM_SINGLE defaultValue:nil];
    self.project_volume_fadein_time_single = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_PROJECT_VOL_FADE_IN_TIME_SINGLE defaultValue:200];
    self.project_volume_fadeout_time_single = [TemplateHelper intValueWithDictionary:list key:KEY_TEMPLATE_PROJECT_VOL_FADE_OUT_TIME_SINGLE defaultValue:5000];
    
    if(deepScan) {
        
        self.intro = [self buildComponent:(NSArray *)list[@"template_intro"]];
        self.loop = [self buildComponent:(NSArray *)list[@"template_loop"]];
        self.outro = [self buildComponent:(NSArray *)list[@"template_outro"]];
        self.single = [self buildComponent:(NSArray *)list[@"template_single"]];
    }
    
    return nil;
}

- (instancetype)initWithJSONData:(NSData *)jsonData deepScan:(BOOL)deepScan nxeError:(NXEError **)nxeError
{
    self = [super init];
    if(self) {
        NSString *error = [self setupTemplateHelper:jsonData deepScan:deepScan];
        
        if(error == nil) {
            
            return self;
            
        } else {
            
            *nxeError = [[NXEError alloc] initWithErrorCode:ERROR_ASSETFORMAT extraDescription:error];
            return nil;
        }
    }
    return nil;
}

@end
