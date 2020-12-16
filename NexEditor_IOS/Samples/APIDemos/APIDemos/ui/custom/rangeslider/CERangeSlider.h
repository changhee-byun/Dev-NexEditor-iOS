/******************************************************************************
 * File Name   :	CERangeSlider.h
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

#import <UIKit/UIKit.h>
#import "CERangeSliderKnobLayer.h"
#import "CERangeSliderCallbackBlock.h"
#import "CERangeSliderTrackLayer.h"

@protocol RangeSliderProtocol <NSObject>
@property (nonatomic) float curvatiousness;
@property (nonatomic) UIColor* knobColour;
@property (nonatomic) float upperValue;
@property (nonatomic) float lowerValue;
- (float) positionForValue:(float)value;
@end

@interface CERangeSlider : UIControl <RangeSliderProtocol>
{
    
}

- (void) setCallbackBlock:(CERangeSliderCallbackBlock*) callback;

@property (nonatomic, retain) CERangeSliderTrackLayer* trackLayer;
@property (nonatomic, retain) CERangeSliderKnobLayer* upperKnobLayer;
@property (nonatomic, retain) CERangeSliderKnobLayer* lowerKnobLayer;
@property (nonatomic, retain) CERangeSliderKnobLayer* centerKnobLayer;

@property (nonatomic) BOOL touchEnable;

@property (nonatomic) BOOL trimKnobTouchEnable;

@property (nonatomic) float maximumValue;

@property (nonatomic) float minimumValue;

@property (nonatomic) float upperValue;

@property (nonatomic) float centerValue;

@property (nonatomic) float lowerValue;

@property (nonatomic) float curvatiousness;

@property (nonatomic) UIColor* trackColour;

@property (nonatomic) UIColor* trackHighlightColour;

@property (nonatomic) UIColor* knobColour;

- (float) positionForValue:(float)value;

@end
