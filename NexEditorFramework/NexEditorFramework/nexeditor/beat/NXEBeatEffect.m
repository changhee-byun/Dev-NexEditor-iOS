/**
 * File Name   : NXEBeatEffect.m
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
#import "NXEBeatEffect.h"

static NSString *const kTime = @"int_time";
static NSString *const kEffectInDuration = @"int_effect_in_duration";
static NSString *const kEffectOutDuration = @"int_effect_out_duration";
static NSString *const kSourceChange = @"b_source_change";
static NSString *const kInternalClipId = @"internal_clip_id";
static NSString *const kPriority = @"int_priority";


@interface NXEBeatEffect()

@property (nonatomic) int time;
@property (nonatomic) int effectInDuration;
@property (nonatomic) int effectOutDuration;
@property (nonatomic) BOOL sourceChange;
@property (nonatomic, strong) NSString *internalClipId;
@property (nonatomic) int priority;

@end


@implementation NXEBeatEffect

- (instancetype) initWithSource:(NSDictionary *)source {
    self = [super initWithSource:source];
    if (self) {
        self.time = [source[kTime] intValue];
        
        NSNumber* effectInDuration = source[kEffectInDuration];
        self.effectInDuration = (effectInDuration != nil) ? [effectInDuration intValue] : 0;
        NSNumber* effectOutDuration = source[kEffectOutDuration];
        self.effectOutDuration = (effectOutDuration != nil) ? [effectOutDuration intValue] : 0;
        self.sourceChange = [source[kSourceChange] boolValue];
        self.internalClipId = source[kInternalClipId];
        self.priority = source[kPriority] ? [source[kPriority] intValue] : 0 ;
    }
    return self;
}

- (void) dealloc {
    self.internalClipId = nil;
    [super dealloc];
}

@end
