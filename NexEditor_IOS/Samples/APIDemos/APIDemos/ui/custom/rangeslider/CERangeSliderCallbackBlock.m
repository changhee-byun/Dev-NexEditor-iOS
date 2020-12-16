/******************************************************************************
 * File Name   :	CERangeSliderCallbackBlock.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/


#import <Foundation/Foundation.h>
#import "CERangeSliderCallbackBlock.h"

@interface CERangeSliderCallbackBlock ()

@property(copy, nonatomic)void(^endTrackingWithTouchCb)(bool isTouchedCenterKnob);
@property(copy, nonatomic)void(^continueTrackingWithTouchCb)(bool isTouchedCenterKnob);

@end

@implementation CERangeSliderCallbackBlock

- (id) initWithCallbackBlock:(void (^)(bool isTouchedCenterKnob))endTrackingWithTouchCallbackBlock ContinueTrackingWithTouchCallbackBlock:(void (^)(bool isTouchedCenterKnob))continueTrackingWithTouchCallbackBlock;
{
    self = [super init];
    if(self)
    {
        self.endTrackingWithTouchCb = endTrackingWithTouchCallbackBlock;
        self.continueTrackingWithTouchCb = continueTrackingWithTouchCallbackBlock;
    }
    
    return self;
}

- (void) endTrackingWithTouchCallback:(bool)isTouchedCenterKnob
{
    self.endTrackingWithTouchCb(isTouchedCenterKnob);
}

- (void) continueTrackingWithTouchCallback:(bool)isTouchedCenterKnob
{
    self.continueTrackingWithTouchCb(isTouchedCenterKnob);
}
@end
