/**
 * File Name   : TransitionAssetItemInfo.m
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

#import "TransitionAssetItemInfo.h"
#import "NSString+ValueReader.h"

@interface TransitionAssetItemInfo()
@property (nonatomic) int effectOffset;
@property (nonatomic) int effectOverlap;
@property (nonatomic) int effectDuration;
@end

@implementation TransitionAssetItemInfo

- (void)parseBeginElementWithName:(NSString * _Nonnull)elementName attributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributes
{
    if([elementName isEqualToString:@"effect"]) {
        self.effectOffset = [attributes[@"effectoffset"] intValueOrDefault:100];
        self.effectOverlap = [attributes[@"videooverlap"] intValueOrDefault:100];
        self.effectDuration = 2000;
    } else if([elementName isEqualToString:@"renderitem"]) {
        self.effectOffset = [attributes[@"transitionoffset"] intValueOrDefault:100];
        self.effectOverlap = [attributes[@"transitionoverlap"] intValueOrDefault:100];
        self.effectDuration = 2000;
    }
}
@end
