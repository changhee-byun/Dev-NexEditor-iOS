/**
 * File Name   : CollageInfo.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *          Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "CollageInfo.h"
#import "CollageData.h"
#import "CollageInfoInternal.h"

@interface CollageSlotInfo()
@property (nonatomic, strong) NSArray <NSValue *> *position;
@end

@implementation CollageSlotInfo
- (instancetype) initWithPosition:(NSArray<NSValue *>*)position rect:(CGRect)rect
{
    self = [super init];
    _position = position;
    _rect = rect;
    return self;
}
@end

@interface CollageTitleInfo()
@property (nonatomic, strong) NSArray <NSValue *> *position;
@end

@implementation CollageTitleInfo
- (instancetype) initWithPosition:(NSArray<NSValue *>*)position rect:(CGRect)rect
{
    self = [super init];
    _position = position;
    _rect = rect;
    return self;
}
@end

@implementation CollageSlotConfiguration

- (instancetype) init
{
    if ( self = [super init] ) {
        self.scale = 1.0;
    }
    return self;
}
@end

@implementation CollageTitleConfiguration

- (instancetype) init
{
    self = [super init];
    self.textStyle = [[CollageTextStyle alloc] init];
    return self;
}

@end

@interface CollageInfo()

@property (nonatomic) int durationMs;
@property (nonatomic) int editTimeMs;
@property (nonatomic) CollageType type;
@property (nonatomic) CollageSizeInt aspectRatio;
@property (nonatomic, strong) NSArray <CollageSlotInfo *> *slots;
@property (nonatomic, strong) NSArray <CollageTitleInfo *> *titles;

@end

@implementation CollageInfo

- (id) initWithData:(CollageData*)data;
{
    if (self = [super init] ) {
        NSString *const systemType = @"system";
        NSString *const systemMTType = @"system_mt";
        NSString *const collageTypeStatic = @"static";

        self.durationMs = data.durationMs;
        self.editTimeMs = (int)(data.editTime * data.durationMs);
        
        if ([data.collageType isEqualToString:collageTypeStatic]) {
            self.type = CollageTypeStatic;
        } else {
            self.type = CollageTypeDynamic;
        }
        
        NSArray<NSString *>*ratioValues = [data.collageRatio componentsSeparatedByString:@"v"];
        CollageSizeInt aspect = {ratioValues[0].intValue, ratioValues[1].intValue};
        self.aspectRatio = aspect;
        
        NSMutableArray *slotList = [[NSMutableArray alloc] init];
        for (CollageSlotData *slotData in data.slots) {
            if ([slotData.sourceType isEqualToString:systemType] == NO && [slotData.sourceType isEqualToString:systemMTType] == NO) {
                CollageSlotInfo *slotInfo = [[CollageSlotInfo alloc] initWithPosition:[self pointsWithPositionInfo:slotData.position]
                                                                                 rect:[self rectWithPositionInfo:slotData.position]];
                [slotList addObject:slotInfo];
            }
        }
        self.slots = slotList;
        
        NSMutableArray *titleList = [[NSMutableArray alloc] init];
        for (CollageTitleData *titleData in data.titles) {
            if ([titleData.titleInfoType isEqualToString:systemType] == NO) {
                CollageTitleInfo *titleInfo = [[CollageTitleInfo alloc] initWithPosition:[self pointsWithPositionInfo:titleData.position]
                                                                                    rect:[self rectWithPositionInfo:titleData.position]];
                [titleList addObject:titleInfo];
            }
        }
        self.titles = titleList;
    }
    return self;
}

- (CGRect) rectWithPositionInfo:(NSString *)positionInfo
{
    NSString *position = [positionInfo stringByReplacingOccurrencesOfString:@" " withString:@""];
    NSArray <NSString *> *substrings = [position componentsSeparatedByString:@","];
    
    CGRect result = CGRectZero;
    
    if (substrings.count == 4) {
        result = CGRectMake([substrings[0] floatValue], [substrings[1] floatValue], [substrings[2] floatValue], [substrings[3] floatValue]);
    } else {
        float left = FLT_MAX; float top = FLT_MAX;
        float right = FLT_MIN; float bottom = FLT_MIN;
        
        for (int i = 0; i < substrings.count; i += 2) {
            float x = [substrings[i] floatValue]; float y = [substrings[i+1] floatValue];
            
            left = left > x ? x : left;
            right = right < x ? x : right;
            top = top > y ? y : top;
            bottom = bottom < y ? y : bottom;
        }
        result = CGRectMake(left, top, right, bottom);
    }
    
    return result;
}

- (NSArray <NSValue *> *) pointsWithPositionInfo:(NSString *)positionInfo
{
    NSString *position = [positionInfo stringByReplacingOccurrencesOfString:@" " withString:@""];
    NSArray <NSString *> *substrings = [position componentsSeparatedByString:@","];
    
    NSMutableArray *result = [[NSMutableArray alloc] init];
    
    if (substrings.count == 4) {
        CGPoint lt = CGPointMake([substrings[0] floatValue], [substrings[1] floatValue]);
        CGPoint rt = CGPointMake([substrings[2] floatValue], [substrings[1] floatValue]);
        CGPoint rb = CGPointMake([substrings[2] floatValue], [substrings[3] floatValue]);
        CGPoint lb = CGPointMake([substrings[0] floatValue], [substrings[3] floatValue]);
        
        [result addObject:[NSValue valueWithCGPoint:lt]];
        [result addObject:[NSValue valueWithCGPoint:rt]];
        [result addObject:[NSValue valueWithCGPoint:rb]];
        [result addObject:[NSValue valueWithCGPoint:lb]];
        [result addObject:[NSValue valueWithCGPoint:lt]];
    } else {
        for (int i = 0; i < substrings.count; i += 2) {
            CGPoint point = CGPointMake([substrings[i] floatValue], [substrings[i+1] floatValue]);
            [result addObject:[NSValue valueWithCGPoint:point]];
        }
    }
    
    return result;
}
@end

