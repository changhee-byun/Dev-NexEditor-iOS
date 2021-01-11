/**
 * File Name   : OverlayTitleInfo.m
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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "OverlayTitleInfo.h"
#import "NSDictionary+ValueForKeyDefault.h"
#import "NSString+Color.h"
#import "NXEErrorPrivate.h"

// HEADER
#define KEY_OVERLAY_NAME @"overlay_name"
#define KEY_OVERLAY_VERSION @"overlay_version"
#define KEY_OVERLAY_DESC @"overlay_desc"
#define KEY_OVERLAY_PRIORTYPE @"overlay_priority"

// INFO
#define KEY_INFO_TYPE @"type"
#define KEY_INFO_TEXT @"text"
#define KEY_INFO_TEXT_DESC @"text_desc"
#define KEY_INFO_TEXT_MAX_LEN @"text_max_len"
#define KEY_INFO_GROUP @"group"
#define KEY_INFO_POSITION @"position"
#define KEY_INFO_START_TIME @"start_time"
#define KEY_INFO_DURATION @"duration"
#define KEY_INFO_FONT @"font"
#define KEY_INFO_FONT_SIZE @"font_size"
#define KEY_INFO_FONT_TEXT_COLOR @"text_color"
#define KEY_INFO_SHADOW_VISIBLE @"shadow_visible"
#define KEY_INFO_SHADOW_COLOR @"shadow_color"
#define KEY_INFO_GLOW_VISIBLE @"glow_visible"
#define KEY_INFO_GLOW_COLOR @"glow_color"
#define KEY_INFO_OUTLINE_VISIBLE @"outline_visible"
#define KEY_INFO_OUTLINE_COLOR @"outline_color"
#define KEY_INFO_ALIGN @"align"
#define KEY_INFO_IMAGE_RES @"image_res"
#define KEY_INFO_ANIMATION @"animation"
#define KEY_INFO_ROTATE @"rotate"
#define KEY_INFO_SCALE @"scale"
#define KEY_INFO_ADJUST_POS_NON_SUB @"adjust_pos_non_sub"
#define KEY_INFO_ADJUST_ALIGN_NON_SUB @"adjust_align_non_sub"
#define KEY_INFO_ADJUST_ANIMATION_NON_SUB @"adjust_animation_non_sub"

// ANIMATION
#define KEY_ANIMATION_TYPE @"type"
#define KEY_ANIMATION_VALUE @"values"
#define KEY_ANIMATION_MOTION_TYPE @"motion_type"
#define KEY_ANIMATION_START_TIME @"start_time"
#define KEY_ANIMATION_END_TIME @"end_time"
#define KEY_ANIMATION_START_X @"start_x"
#define KEY_ANIMATION_START_Y @"start_y"
#define KEY_ANIMATION_END_X @"end_x"
#define KEY_ANIMATION_END_Y @"end_y"
#define KEY_ANIMATION_ALPHA_START @"start"
#define KEY_ANIMATION_ALPHA_END @"end"
#define KEY_ANIMATION_CLOCKWISE @"clockwise"
#define KEY_ANIMATION_ROTATE_DEGREE @"rotatedegree"

#pragma mark -

@interface NSString (OverTitle)

- (TextType)textTypeValue;
- (OverlayTitleType)overlayTitleTypeValue;
- (AnimationType)animationTypeValue;
- (InterpolatorType)interpolatorTypeValue;
- (Position)position;
- (TitleScale)titleScaleValue;
- (PositionOffset)posistionOffsetValue;
- (Alignment)textAlign;

@end

@implementation NSString (OverTitle)

- (OverlayTitleType)overlayTitleTypeValue
{
    NSDictionary<NSString*, NSNumber*> *types = @{@"title":@(OverlayTitleTypeText), @"image":@(OverlayTitleTypeImage)};
    return (OverlayTitleType)(types[self].integerValue);
}

- (TextType)textTypeValue
{
    NSDictionary<NSString*, NSNumber*> *types = @{@"TITLE":@(TextTypeTitle), @"SubTitle":@(TextTypeSubTitle)};
    return (TextType)(types[self].integerValue);
}

- (AnimationType)animationTypeValue
{
    NSDictionary<NSString*, NSNumber*> *animationTypes = @{@"move":@(AnimationTypeMove), @"alpha":@(AnimationTypeAlpha), @"scale":@(AnimationTypeScale), @"rotate":@(AnimationTypeRotate)};
    return (AnimationType)(animationTypes[self].integerValue);
}

- (InterpolatorType)interpolatorTypeValue
{
    NSDictionary<NSString*, NSNumber*> *interpolatorTypes = @{@"accelerateDecelerateInterpolator":@(InterpolatorTypeAccelerateDecelerate), @"accelerateInterpolator":@(InterpolatorTypeAccelerate), @"decelerateInterpolator":@(InterpolatorTypeDecelerate), @"linearInterpolator":@(InterpolatorTypeLinear), @"bounceInterpolator":@(InterpolatorTypeBounce), @"cycleInterpolator":@(InterpolatorTypeCycle), @"anticipateInterpolator":@(InterpolatorTypeAnticipate), @"anticipateOvershootInterpolator":@(InterpolatorTypeAnticipateOvershoot), @"overshootInterpolator":@(InterpolatorTypeOvershoot)};
    return (InterpolatorType)(interpolatorTypes[self].integerValue);
}

- (Position)position
{
    NSArray *positions = [self componentsSeparatedByString:@","];
    
    Position position;
    
    position.left = ((NSString*)positions[0]).intValue;
    position.top = ((NSString*)positions[1]).intValue;
    position.right = ((NSString*)positions[2]).intValue;
    position.bottom = ((NSString*)positions[3]).intValue;
    
    return position;
}

- (TitleScale)titleScaleValue
{
    TitleScale titleScale;
    
    if(self == nil) {
        titleScale.x = 0.0; titleScale.y = 0.0; titleScale.z = 0.0;
        return titleScale;
    }
    NSArray *scales = [self componentsSeparatedByString:@","];
    titleScale.x = ((NSString*)scales[0]).floatValue;
    titleScale.y = ((NSString*)scales[1]).floatValue;
    titleScale.z = ((NSString*)scales[2]).floatValue;
    return titleScale;
}

- (PositionOffset)posistionOffsetValue
{
    PositionOffset positionOffset;
    
    if(self == nil) {
        positionOffset.x = 0; positionOffset.y = 0;
        return positionOffset;
    }
    NSArray *offsets = [self componentsSeparatedByString:@","];
    positionOffset.x = ((NSString *)offsets[0]).intValue;
    positionOffset.y = ((NSString *)offsets[1]).intValue;
    return positionOffset;
}

- (Alignment)textAlign
{
    Alignment alignment;
    
    NSArray *alignments = [self componentsSeparatedByString:@"|"];
    
    NSString *hAlign = [alignments[0] uppercaseString];
    
    if([hAlign isEqualToString:@"LEFT"]) {
        alignment.hAlign = HorizontalAlignLeft;
    } else if([hAlign isEqualToString:@"RIGHT"]) {
        alignment.hAlign = HorizontalAlignRight;
    } else {
        alignment.hAlign = HorizontalAlignCenter;
    }
    
    NSString *vAlign = [alignments[1] uppercaseString];
    
    if([vAlign isEqualToString:@"TOP"]) {
        alignment.vAlign = VerticalAlignTop;
    } else if([vAlign isEqualToString:@"BOTTOM"]) {
        alignment.vAlign = VerticalAlignBottom;
    } else {
        alignment.vAlign = VerticalAlignMiddle;
    }
    
    return alignment;
}

- (Group)textGroup;
{
    Group group;
    
    NSArray *groups = [self componentsSeparatedByString:@"_"];
    
    group.textGroup = ((NSString *)groups[0]).intValue == 1 ? TextGroupStart : TextGroupEnd;
    group.textType = ((NSString *)groups[1]).intValue == 1 ? TextTypeTitle : TextTypeSubTitle;
    
    return group;
}

@end

#pragma mark -

@implementation OverlayTitleLayerAnimationMotion

- (instancetype)initWithRawTitleLayerAnimationDictionary:(NSDictionary<NSString *, NSString *> *)dictionary;
{
    self = [super init];
    if(self) {
        _interpolatorType = [[dictionary stringValueWithKey:KEY_ANIMATION_TYPE defaultValue:@"accelerateDecelerateInterpolator"] interpolatorTypeValue];
        _startTime = [dictionary intValueWithKey:KEY_ANIMATION_START_TIME defaultValue:-1];
        _duration = [dictionary intValueWithKey:KEY_ANIMATION_END_TIME defaultValue:-1];
        _rotation = [dictionary intValueWithKey:KEY_ANIMATION_ROTATE_DEGREE defaultValue:-1];
        _startAlpha = [dictionary stringValueWithKey:KEY_ANIMATION_ALPHA_START defaultValue:0].intValue;
        _endAlpha = [dictionary stringValueWithKey:KEY_ANIMATION_ALPHA_END defaultValue:0].intValue;
        _clockWise = [dictionary stringValueWithKey:KEY_ANIMATION_CLOCKWISE defaultValue:0].boolValue;
        
        CGFloat start_x = [dictionary stringValueWithKey:KEY_ANIMATION_START_X defaultValue:@"0.0"].floatValue;
        CGFloat start_y = [dictionary stringValueWithKey:KEY_ANIMATION_START_Y defaultValue:@"0.0"].floatValue;
        _startPosition = CGPointMake(start_x, start_y);
        
        CGFloat end_x = [dictionary stringValueWithKey:KEY_ANIMATION_END_X defaultValue:@"0.0"].floatValue;
        CGFloat end_y = [dictionary stringValueWithKey:KEY_ANIMATION_END_Y defaultValue:@"0.0"].floatValue;
        _endPosition = CGPointMake(end_x, end_y);
    }
    return self;
}

@end

#pragma mark -

@implementation OverlayTitleAnimation

- (void)dealloc
{
    _motions = nil;
    
    [super dealloc];
}

- (NSArray<OverlayTitleLayerAnimationMotion *> *)buildOverlayTitleAnimationMotionWithDictionary:(NSDictionary<NSString *, NSString *> *)raw
{
    if(raw.count != 0) {
        NSMutableArray *motions = [NSMutableArray array];
        
        for(NSDictionary *dictionary in raw) {
            OverlayTitleLayerAnimationMotion *layerAnimationMotion = [[[OverlayTitleLayerAnimationMotion alloc] initWithRawTitleLayerAnimationDictionary:dictionary] autorelease];
            [motions addObject:layerAnimationMotion];
        }
        return [[motions copy] autorelease];
    }
    return nil;
}

- (instancetype)initWithAnimationType:(AnimationType)animationType rawDictionary:(NSDictionary<NSString *, NSString *>*)raw
{
    self = [super init];
    if(self) {
        _type = animationType;
        _motions = [self buildOverlayTitleAnimationMotionWithDictionary:raw];
    }
    return self;
}

@end

#pragma mark -

@implementation OverlayTitleLayer

- (void)dealloc
{
    _defaultText = nil;
    _fontName = nil;
    _fontColor = nil;
    _fontShadowColor = nil;
    _fontGlowColor = nil;
    _fontOutlineColor = nil;
    _resourceID = nil;
    _animations = nil;
    _adjAnimations = nil;
    
    [super dealloc];
}

- (void)setInAniSubTitle:(BOOL)inAniSubTitle
{
    _inAniSubTitle = inAniSubTitle;
}

- (void)setOutAniSubTitle:(BOOL)outAniSubTitle
{
    _outAniSubTitle = outAniSubTitle;
}

- (void)setFontSize:(int)fontSize
{
    _fontSize = fontSize;
}

- (NSArray<OverlayTitleAnimation *> *)configureAnimationArrayWithRawDictionary:(NSDictionary *)raw key:(NSString *)key
{
    if(raw[key] == nil) {
        return nil;
    }
    
    NSMutableArray<OverlayTitleAnimation *> *animations = [NSMutableArray array];
    
    for(NSDictionary *dictionary in raw[key]) {
        OverlayTitleAnimation *animation = [[[OverlayTitleAnimation alloc] initWithAnimationType:[dictionary[KEY_ANIMATION_TYPE] animationTypeValue] rawDictionary:dictionary[KEY_ANIMATION_VALUE]] autorelease];
        [animations addObject:animation];
    }
    
    return [[animations copy] autorelease];
}

- (void)buildOverlayTitleLayerWithDictionary:(NSDictionary *)dictionary
{
    _type = [[dictionary stringValueWithKey:KEY_INFO_TYPE defaultValue:nil] overlayTitleTypeValue];
    _textType = [[dictionary stringValueWithKey:KEY_INFO_TEXT defaultValue:nil] textTypeValue];
    _group = [[dictionary stringValueWithKey:KEY_INFO_GROUP defaultValue:nil] textGroup];
    _defaultText = [dictionary stringValueWithKey:KEY_INFO_TEXT_DESC defaultValue:nil];
    _textMaxLength = [dictionary intValueWithKey:KEY_INFO_TEXT_MAX_LEN defaultValue:-1];
    _startTime = [dictionary intValueWithKey:KEY_INFO_START_TIME defaultValue:-1];
    _duration = [dictionary intValueWithKey:KEY_INFO_DURATION defaultValue:-1];
    _fontName = [dictionary stringValueWithKey:KEY_INFO_FONT defaultValue:nil];
    _fontSize = [dictionary intValueWithKey:KEY_INFO_FONT_SIZE defaultValue:-1];
    _fontColor = [[dictionary stringValueWithKey:KEY_INFO_FONT_TEXT_COLOR defaultValue:@"#FFFFFFFF"] hashHexARGBColor];
    _fontShadow = [dictionary stringValueWithKey:KEY_INFO_SHADOW_VISIBLE defaultValue:0].boolValue;
    _fontShadowColor = [[dictionary stringValueWithKey:KEY_INFO_SHADOW_COLOR defaultValue:@"#FFFFFFFF"] hashHexARGBColor];
    _fontGlow = [dictionary stringValueWithKey:KEY_INFO_GLOW_VISIBLE defaultValue:0].boolValue;
    _fontGlowColor = [[dictionary stringValueWithKey:KEY_INFO_GLOW_COLOR defaultValue:@"#FFFFFFFF"] hashHexARGBColor];
    _fontOutline = [dictionary stringValueWithKey:KEY_INFO_OUTLINE_VISIBLE defaultValue:0].boolValue;
    _fontOutlineColor = [[dictionary stringValueWithKey:KEY_INFO_OUTLINE_COLOR defaultValue:@"#FFFFFFFF"] hashHexARGBColor];
    _align = [[dictionary stringValueWithKey:KEY_INFO_ALIGN defaultValue:nil] textAlign];
    _adjAlign = [[dictionary stringValueWithKey:KEY_INFO_ADJUST_ALIGN_NON_SUB defaultValue:nil] textAlign];
    _resourceID = [dictionary stringValueWithKey:KEY_INFO_IMAGE_RES defaultValue:nil];
    _position = [[dictionary stringValueWithKey:KEY_INFO_POSITION defaultValue:nil] position];
    _positionOffset = [[dictionary stringValueWithKey:KEY_INFO_ADJUST_POS_NON_SUB defaultValue:nil] posistionOffsetValue];
    _rotation = [dictionary floatValueWithKey:KEY_INFO_ROTATE defaultValue:0.0];
    _scale = [[dictionary stringValueWithKey:KEY_INFO_SCALE defaultValue:nil] titleScaleValue];
    _animations = [self configureAnimationArrayWithRawDictionary:dictionary key:@"animation"];
    _adjAnimations = [self configureAnimationArrayWithRawDictionary:dictionary key:@"adjust_animation_non_sub"];
}

- (instancetype)initWithRawDictionary:(NSDictionary *)raw
{
    self = [super init];
    if(self) {
        [self buildOverlayTitleLayerWithDictionary:raw];
    }
    return self;
}

@end

#pragma mark -

@interface OverlayTitleInfo ()

@property(nonatomic, retain) NSDictionary *source;

@end

@implementation OverlayTitleInfo

- (int)overlayTitleDuration
{
    if(_overlayTitleDuration == 0) {
        for(OverlayTitleLayer *overlayTitle in self.overlays) {
            if(overlayTitle.type == OverlayTitleTypeText && overlayTitle.textType == TextTypeTitle) {
                if(overlayTitle.group.textGroup == TextGroupStart || overlayTitle.group.textGroup == TextGroupEnd) {
                    _overlayTitleDuration += overlayTitle.duration;
                }
            }
        }
    }
    return _overlayTitleDuration;
}

- (void)dealloc
{
    _name = nil;
    _version = nil;
    _desc = nil;
    _priorityTitleType = nil;
    _overlays = nil;
    _source = nil;
    
    [super dealloc];
}

- (NSArray<OverlayTitleLayer*> *)buildOverlayTitleInfo:(NSArray<NSDictionary *> *)rawInfos
{
    if(rawInfos.count != 0) {
        NSMutableArray<OverlayTitleLayer *> *layers = [NSMutableArray array];
        
        for(NSDictionary *raw in rawInfos) {
            OverlayTitleLayer *layer = [[[OverlayTitleLayer alloc] initWithRawDictionary:raw] autorelease];
            [layers addObject:layer];
        }
        
        return [[layers copy] autorelease];
    }
    return nil;
}

- (NSArray<OverlayTitleLayer *> *)overlays
{
    if(_overlays == nil) {
        _overlays = [self buildOverlayTitleInfo:(NSArray<NSDictionary *> *)self.source[@"overlay"]];
    }
    return _overlays;
}

- (instancetype)initWithJSONData:(NSData *)jsonData nxeError:(NXEError **)nxeError
{
    self = [super init];
    if(self) {
        NSError *error = nil;
        NSDictionary *source = nil;
        if (jsonData) {
            source = [NSJSONSerialization JSONObjectWithData:jsonData options:NSJSONReadingAllowFragments error:&error];
        }
        if(source == nil) {
            NSString *description = @"Data is empty";
            if ( error ) {
                description = error.localizedDescription;
            }
            *nxeError = [[NXEError alloc] initWithErrorCode:ERROR_ASSETFORMAT extraDescription: description];
            return nil;
        }
        
        _name = source[KEY_OVERLAY_NAME];
        _desc = source[KEY_OVERLAY_DESC];
        _version = source[KEY_OVERLAY_VERSION];
        _priorityTitleType = source[KEY_OVERLAY_PRIORTYPE];
        _source = source;
    }
    return self;
}

@end
