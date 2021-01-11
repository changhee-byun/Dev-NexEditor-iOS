/******************************************************************************
 * File Name   : NXEColorEffect.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEColorEffect.h"

@implementation NXEColorEffect

- (NSString *)getPresetName
{
    NSArray *presetNames = @[@"NXE_CE_NONE",
                             @"NXE_CE_ALIEN_INVASION",
                             @"NXE_CE_BLACK_AND_WHITE",
                             @"NXE_CE_COOL",
                             @"NXE_CE_DEEP_BLUE",
                             @"NXE_CE_PINK",
                             @"NXE_CE_RED_ALERT",
                             @"NXE_CE_SEPIA",
                             @"NXE_CE_SUNNY",
                             @"NXE_CE_PURPLE",
                             @"NXE_CE_ORANGE",
                             @"NXE_CE_STRONG_ORANGE",
                             @"NXE_CE_SPRING",
                             @"NXE_CE_SUMMER",
                             @"NXE_CE_FALL",
                             @"NXE_CE_ROUGE",
                             @"NXE_CE_PASTEL",
                             @"NXE_CE_NOIR"];
    
    return [presetNames objectAtIndex:self.presetColorEffectId];
}

- (void)setValue:(NXEPresetColorEffect)NXEColorEffectid brightness:(float)brightness contrast:(float)contrast saturation:(float)saturation tintColor:(int)tintColor
{
    self.presetColorEffectId = NXEColorEffectid;
    self.brightness = brightness;
    self.contrast = contrast;
    self.saturation = saturation;
    self.tintColor = tintColor;
}

- (void)setValues:(NXEPresetColorEffect)NXEColorEffectid
{
    switch(NXEColorEffectid) {
        case NXE_CE_ALIEN_INVASION:
            [self setValue:NXEColorEffectid brightness:0.12f contrast:-0.06f saturation:-0.3f tintColor:0xFF147014];
            break;
        case NXE_CE_BLACK_AND_WHITE:
            [self setValue:NXEColorEffectid brightness:0 contrast:0 saturation:-1.0f tintColor:0];
            break;
        case NXE_CE_COOL:
            [self setValue:NXEColorEffectid brightness:0.12f contrast:-0.12f saturation:-0.3f tintColor:0xFF144270];
            break;
        case NXE_CE_DEEP_BLUE:
            [self setValue:NXEColorEffectid brightness:-0.2f contrast:-0.3f saturation:-0.6f tintColor:0xFF0033FF];
            break;
        case NXE_CE_PINK:
            [self setValue:NXEColorEffectid brightness:0.1f contrast:-0.3f saturation:-0.6f tintColor:0xFF9C4F4F];
            break;
        case NXE_CE_RED_ALERT:
            [self setValue:NXEColorEffectid brightness:-0.3f contrast:-0.19f saturation:-1.0f tintColor:0xFFFF0000];
            break;
        case NXE_CE_SEPIA:
            [self setValue:NXEColorEffectid brightness:0.12f contrast:-0.12f saturation:-0.3f tintColor:0xFF704214];
            break;
        case NXE_CE_SUNNY:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.3f tintColor:0xFFCCAA55];
            break;
        case NXE_CE_PURPLE:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.3f tintColor:0xFFAA55CC];
            break;
        case NXE_CE_ORANGE:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.35 tintColor:0xFFFFBB00];
            break;
        case NXE_CE_STRONG_ORANGE:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.5f tintColor:0xFFFFBB00];
            break;
        case NXE_CE_SPRING:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.3f tintColor:0xFFAACC55];
            break;
        case NXE_CE_SUMMER:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.5f tintColor:0xFFAAFF00];
            break;
        case NXE_CE_FALL:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.5f tintColor:0xFF00FFAA];
            break;
        case NXE_CE_ROUGE:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.6f tintColor:0xFFFF5555];
            break;
        case NXE_CE_PASTEL:
            [self setValue:NXEColorEffectid brightness:0.08f contrast:-0.06f saturation:-0.5f tintColor:0xFF555555];
            break;
        case NXE_CE_NOIR:
            [self setValue:NXEColorEffectid brightness:-0.25f contrast:0.6f saturation:-1.0f tintColor:0xFF776655];
            break;
        case NXE_CE_NONE:
        default:
            [self setValue:NXEColorEffectid brightness:0 contrast:0 saturation:0 tintColor:0];
            break;
    }
}

- (instancetype)initWithPresetColorEffect:(NXEPresetColorEffect)presetColorEffect
{
    self = [super init];
    if(self) {
        [self setValues:presetColorEffect];
        return self;
    }
    return nil;
}

#pragma mark - NSCopying

- (instancetype)copyWithZone:(NSZone *)zone
{
    NXEColorEffect *destination = [[self class] allocWithZone:zone];
    //
    [destination setValue:self.presetColorEffectId
               brightness:self.brightness
                 contrast:self.contrast
               saturation:self.saturation
                tintColor:self.tintColor];
    
    return destination;
}

@end
