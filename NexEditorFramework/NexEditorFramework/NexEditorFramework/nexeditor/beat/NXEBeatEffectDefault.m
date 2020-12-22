/**
 * File Name   : NXEBeatEffectDefault.m
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

#import "NXEBeatEffectDefault.h"

NSString *const kColorFilterId = @"color_filter_id";
NSString *const kClipEffectId = @"clip_effect_id";
NSString *const kScColorFilterId = @"sc_color_filter_id";
NSString *const kScClipEffectId = @"sc_clip_effect_id";

@interface NXEBeatEffectDefault()
@property (nonatomic, strong) NSString *colorFilterId;
@property (nonatomic, strong) NSString *clipEffectId;
@property (nonatomic, strong) NSString *scColorFilterId;
@property (nonatomic, strong) NSString *scClipEffectId;
@end

@implementation NXEBeatEffectDefault

- (instancetype) initWithSource:(NSDictionary *)source {
    self = [super init];
    if (self) {
        self.colorFilterId = source[kColorFilterId];
        self.clipEffectId = source[kClipEffectId];
        self.scColorFilterId = source[kScColorFilterId];
        self.scClipEffectId = source[kScClipEffectId];
    }
    return self;
}

- (void)dealloc
{
    self.colorFilterId = nil;
    self.clipEffectId = nil;
    self.scColorFilterId = nil;
    self.scClipEffectId = nil;
    [super dealloc];
}

@end
