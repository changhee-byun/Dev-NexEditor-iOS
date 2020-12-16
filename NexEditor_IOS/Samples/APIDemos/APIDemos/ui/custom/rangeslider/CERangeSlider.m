/******************************************************************************
 * File Name   :	CERangeSlider.m
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

#import "CERangeSlider.h"
#import <QuartzCore/QuartzCore.h>
#import "CERangeSliderKnobLayer.h"
#import "CERangeSliderTrackLayer.h"

@interface CERangeSlider ()
@property (nonatomic, copy) CERangeSliderCallbackBlock* callback;
@end

@implementation CERangeSlider
{
    float _knobWidth;
    float _useableTrackLength;
    
    CGPoint _previousTouchPoint;
    
    float _fixedValue;
}


#define GENERATE_SETTER(PROPERTY, TYPE, SETTER, UPDATER) \
@synthesize PROPERTY = _##PROPERTY; \
\
- (void)SETTER:(TYPE)PROPERTY { \
    if (_##PROPERTY != PROPERTY) { \
        _##PROPERTY = PROPERTY; \
        [self UPDATER]; \
    } \
}

GENERATE_SETTER(trackHighlightColour, UIColor*, setTrackHighlightColour, redrawLayers)

GENERATE_SETTER(trackColour, UIColor*, setTrackColour, redrawLayers)

GENERATE_SETTER(curvatiousness, float, setCurvatiousness, redrawLayers)

GENERATE_SETTER(knobColour, UIColor*, setKnobColour, redrawLayers)

GENERATE_SETTER(maximumValue, float, setMaximumValue, setLayerFrames)

GENERATE_SETTER(minimumValue, float, setMinimumValue, setLayerFrames)

GENERATE_SETTER(lowerValue, float, setLowerValue, setLowerKnobFrame)

GENERATE_SETTER(upperValue, float, setUpperValue, setUpperKnobFrame)

GENERATE_SETTER(centerValue, float, setCenterValue, setCenterKnobFrame)

- (void)setCallbackBlock:(CERangeSliderCallbackBlock*)callback
{
    _callback = callback;
}

- (void)redrawLayers
{
    [_upperKnobLayer setNeedsDisplay];
    [_lowerKnobLayer setNeedsDisplay];
    [_centerKnobLayer setNeedsDisplay];
    [_trackLayer setNeedsDisplay];
}

- (id)initWithFrame:(CGRect)frame
{
    _touchEnable = YES;
    _trimKnobTouchEnable = YES;
    
    self = [super initWithFrame:frame];
    if (self) {
        _trackHighlightColour = [UIColor colorWithRed:0.0 green:0.45 blue:0.94 alpha:1.0];
        _trackColour = [UIColor colorWithWhite:0.9 alpha:1.0];
        _knobColour = [UIColor whiteColor];
        _curvatiousness = 1.0;
        _maximumValue = 10.0;
        _minimumValue = 0.0;
        
        // Initialization code
        _maximumValue = 10.0;
        _minimumValue = 0.0;
        _upperValue = 8.0;
        _lowerValue = 2.0;
        _centerValue = (_upperValue + _lowerValue) / 2;
        
        _trackLayer = [CERangeSliderTrackLayer layer];
        _trackLayer.slider = self;
        [self.layer addSublayer:_trackLayer];

        _upperKnobLayer = [CERangeSliderKnobLayer layer];
        _upperKnobLayer.slider = self;
        [self.layer addSublayer:_upperKnobLayer];

        _lowerKnobLayer = [CERangeSliderKnobLayer layer];
        _lowerKnobLayer.slider = self;
        [self.layer addSublayer:_lowerKnobLayer];
        
        _centerKnobLayer = [CERangeSliderKnobLayer layer];
        _centerKnobLayer.slider = self;

        [self.layer addSublayer:_centerKnobLayer];
        [self setLayerFrames];
        
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    _touchEnable = YES;
    _trimKnobTouchEnable = YES;
    
    self = [super initWithCoder:aDecoder];
    if (self) {
        _trackHighlightColour = [UIColor colorWithRed:0.0 green:0.45 blue:0.94 alpha:1.0];
        _trackColour = [UIColor colorWithWhite:0.9 alpha:1.0];
        _knobColour = [UIColor whiteColor];
        _curvatiousness = 1.0;
        _minimumValue = 0.0;
        
        // Initialization code
        _maximumValue = 10000.0;
        _upperValue = 8000.0;
        _lowerValue = 2000.0;
        _centerValue = (_upperValue + _lowerValue) / 2;
        
        _trackLayer = [CERangeSliderTrackLayer layer];
        _trackLayer.slider = self;
        [self.layer addSublayer:_trackLayer];
        
        _upperKnobLayer = [CERangeSliderKnobLayer layer];
        _upperKnobLayer.slider = self;
        [self.layer addSublayer:_upperKnobLayer];
        
        _lowerKnobLayer = [CERangeSliderKnobLayer layer];
        _lowerKnobLayer.slider = self;
        [self.layer addSublayer:_lowerKnobLayer];
        
        _centerKnobLayer = [CERangeSliderKnobLayer layer];
        _centerKnobLayer.slider = self;
        [self.layer addSublayer:_centerKnobLayer];
        
        [self setLayerFrames];
    }
    return self;
    
}

// MJ.
//  upperValue, lowerValue, centerValue 변경될때 호출되는 setter함수였던 setLayerFrames 함수가 전체적인 Layer변경을 시켜주는 동작을 하고 있어서 각각의 것만 업데이트하는 것으로 분리했습니다.
//  upperValue -> setUpperKnobFrame
//  lowValue -> setLowerKnobFrame
//  centerValue -> setCenterKnobFrame
//  maximumValue -> setLayerFrames
//  minimumValue -> setLayerFrames
//
- (void)setUpperKnobFrame
{
    float upperKnobCentre = [self positionForValue:_upperValue];
    _upperKnobLayer.frame = CGRectMake(upperKnobCentre - _knobWidth / 2, 0, _knobWidth, _knobWidth);
    
    [_upperKnobLayer setNeedsDisplay];
}

- (void)setLowerKnobFrame
{
    float lowerKnobCentre = [self positionForValue:_lowerValue];
    _lowerKnobLayer.frame = CGRectMake(lowerKnobCentre - _knobWidth / 2, 0, _knobWidth, _knobWidth);
    
    [_lowerKnobLayer setNeedsDisplay];
}

- (void)setCenterKnobFrame
{
    float centerKnobCentre = [self positionForValue:_centerValue];
    _centerKnobLayer.frame = CGRectMake(centerKnobCentre - _knobWidth / 2, 0, _knobWidth, _knobWidth);
    
    [_centerKnobLayer setNeedsDisplay];
}
//

- (void) layoutSubviews
{
    [super layoutSubviews];
    [self setLayerFrames];
}

- (void)setLayerFrames
{
    _trackLayer.frame = CGRectInset(self.bounds, 0, self.bounds.size.height / 3.5);
    [_trackLayer setNeedsDisplay];
    
    _knobWidth = self.bounds.size.height;
    _useableTrackLength = self.bounds.size.width - _knobWidth;
    
    float upperKnobCentre = [self positionForValue:_upperValue];
    _upperKnobLayer.frame = CGRectMake(upperKnobCentre - _knobWidth / 2, 0, _knobWidth, _knobWidth);
    
    float lowerKnobCentre = [self positionForValue:_lowerValue];
    _lowerKnobLayer.frame = CGRectMake(lowerKnobCentre - _knobWidth / 2, 0, _knobWidth, _knobWidth);

    float centerKnobCentre = [self positionForValue:_centerValue];
    _centerKnobLayer.frame = CGRectMake(centerKnobCentre - _knobWidth / 2, 0, _knobWidth, _knobWidth);
    
    [_upperKnobLayer setNeedsDisplay];
    [_lowerKnobLayer setNeedsDisplay];
    [_centerKnobLayer setNeedsDisplay];
}
                                           
- (float)positionForValue:(float)value
{
    return _useableTrackLength * (value - _minimumValue) /
        (_maximumValue - _minimumValue) + (_knobWidth / 2);
}

- (BOOL)beginTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event
{
    // hit test the knob layers
    _previousTouchPoint = [touch locationInView:self];
    
    // MJ. NESI-136
    //  Knob 겹쳤을 때, 인지하는 순위를 centerKnobLayer 1순위로 변경
    if(CGRectContainsPoint(_centerKnobLayer.frame, _previousTouchPoint)) {
        _centerKnobLayer.highlighted = YES;
        [_centerKnobLayer setNeedsDisplay];
    }
    else if(CGRectContainsPoint(_upperKnobLayer.frame, _previousTouchPoint)) {
        _upperKnobLayer.highlighted = YES;
        [_upperKnobLayer setNeedsDisplay];
    }
    else if(CGRectContainsPoint(_lowerKnobLayer.frame, _previousTouchPoint)) {
        _lowerKnobLayer.highlighted = YES;
        [_lowerKnobLayer setNeedsDisplay];
    }
    return _upperKnobLayer.highlighted || _lowerKnobLayer.highlighted || _centerKnobLayer.highlighted;
}

#define BOUND(VALUE, UPPER, LOWER)	MIN(MAX(VALUE, LOWER), UPPER)

- (BOOL)continueTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event
{
    if(_touchEnable == NO) {
        return NO;
    }
    
    CGPoint touchPoint = [touch locationInView:self];
    
    // deterine by how much the user has dragged
    float delta = touchPoint.x - _previousTouchPoint.x;
    float valueDelta = (_maximumValue - _minimumValue) * delta / _useableTrackLength;
    
    _previousTouchPoint = touchPoint;
    
    // update the values
    if(_trimKnobTouchEnable == TRUE) {
        if (_lowerKnobLayer.highlighted) {
            _lowerValue += valueDelta;
            _lowerValue = BOUND(_lowerValue, _upperValue, _minimumValue);
            
            int positionLowerValue = [self positionForValue:_lowerValue];
            int positionCenterValue = [self positionForValue:_centerValue];
            
            if(positionLowerValue >= positionCenterValue) {
                _centerValue += valueDelta;
                _centerValue = MIN(_centerValue, _upperValue);
            }
        }
        if (_upperKnobLayer.highlighted) {
            _upperValue += valueDelta;
            _upperValue = BOUND(_upperValue, _maximumValue, _centerValue);
            
            int positionUpperValue = [self positionForValue:_upperValue];
            int positionCenterValue = [self positionForValue:_centerValue];
            
            if(positionUpperValue <= positionCenterValue) {
                _centerValue += valueDelta;
                _centerValue = MAX(_centerValue, _lowerValue);
            }
        }
        {/*NESI-224 반영이 되었는데, 추후 Multi Clips에 대한 Template 반영할 때, 해당 Flow삭제 할 예정입니다.*/
            int positionLowerValue = [self positionForValue:_lowerValue];
            int positionUpperValue = [self positionForValue:_upperValue];
            
            int diff = positionUpperValue - positionLowerValue;
            
            if(diff < 25/*대략적으로 1초간의 margine설정한다는 가정에서 25로 설정합니다.*/) {
                if(_fixedValue == -1) {
                    _fixedValue = _upperValue;
                } else {
                    _upperValue = _fixedValue;
                }
                
                if(_fixedValue - 25 < 0) {
                    _lowerValue = 0;
                } else {
                    _lowerValue = _fixedValue - 25;
                }
                
                return YES;/*임의로 lowerKnob, upperKnob값을 설정 후, 해당값을 UI쪽으로 올려준다.*/
            } else {
                _fixedValue = -1;
            }
        }
    }
    
    if(_centerKnobLayer.highlighted) {
        _centerValue += valueDelta;
        _centerValue = BOUND(_centerValue, _upperValue, _lowerValue);
    }
    
    // callback
    if(_centerKnobLayer.highlighted) {
        if([_callback respondsToSelector:@selector(continueTrackingWithTouchCallback:)]) {
            [_callback continueTrackingWithTouchCallback:true];
        }
    } else {
        if([_callback respondsToSelector:@selector(continueTrackingWithTouchCallback:)]) {
            [_callback continueTrackingWithTouchCallback:false];
        }
    }
    
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [self setLayerFrames];
    [CATransaction commit];
    [self sendActionsForControlEvents:UIControlEventValueChanged];
    
    return YES;
}

- (void)endTrackingWithTouch:(UITouch *)touch withEvent:(UIEvent *)event
{
    [_lowerKnobLayer setNeedsDisplay];
    [_upperKnobLayer setNeedsDisplay];
    [_centerKnobLayer setNeedsDisplay];
    
    // callback
    if(_centerKnobLayer.highlighted) {
        if([_callback respondsToSelector:@selector(endTrackingWithTouchCallback:)]) {
            [_callback endTrackingWithTouchCallback:true];
        }
        
    } else {
        if([_callback respondsToSelector:@selector(endTrackingWithTouchCallback:)]) {
            [_callback endTrackingWithTouchCallback:false];
        }
    }
    
    _lowerKnobLayer.highlighted = _upperKnobLayer.highlighted = _centerKnobLayer.highlighted = NO;
}

@end
