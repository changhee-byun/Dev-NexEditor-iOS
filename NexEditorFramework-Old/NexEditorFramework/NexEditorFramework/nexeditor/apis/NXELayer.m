/******************************************************************************
 * File Name   :	NXELayer.m
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

#import "NXELayer.h"
#import "KMLayer.h"
#import "LayerManager.h"

#define NXELayerIDUnknown   -1
#define kNXELayerDefaultStartTime   0
#define kNXELayerDefaultEndTime     6000
#define kNXELayerDefaultAngle       0
#define kNXELayerDefaultHFlip       NO
#define kNXELayerDefaultVFilp       NO
#define kNXELayerDefaultBrightness  0
#define kNXELayerDefaultContrast    0
#define kNXELayerDefaultSaturation  0
#define kNXELayerDefaultAlpha       1.0
#define kNXELayerDefaultScale       1.0
#define kNXELayerDefaultColorFilter KM_LUT_NONE

#define CLAMP(x, min, max)  (MAX(min, MIN(max, x)))

const CGPoint CGPointNXELayerCenter = { -1, -1 };
const CGFloat NXELayerScaledCoordinateCenter = 0.5;

@implementation NXELayer
@synthesize scaledFrame = _scaledFrame;
//
@synthesize alpha = _alpha;
@synthesize angle = _angle;
@synthesize hFlip = _hFlip;
@synthesize vFlip = _vFlip;
@synthesize brightness = _brightness;
@synthesize contrast = _contrast;
@synthesize saturation = _saturation;
@synthesize colorFilter = _colorFilter;
@synthesize startTime = _startTime;
@synthesize endTime = _endTime;
@synthesize scale = _scale;
@synthesize inAnimation = _inAnimation;
@synthesize inAnimationDuration = _inAnimationDuration;
@synthesize outAnimation = _outAnimation;
@synthesize outAnimationDuration = _outAnimationDuration;
@synthesize expression = _expression;

- (instancetype) init
{
    self = [super init];
    if (self) {
        self.layerId = NXELayerIDUnknown;
        self.angle = kNXELayerDefaultAngle;
        self.hFlip = kNXELayerDefaultHFlip;
        self.vFlip = kNXELayerDefaultVFilp;
        self.alpha = kNXELayerDefaultAlpha;
        self.scale = kNXELayerDefaultScale;
        self.colorFilter = kNXELayerDefaultColorFilter;
        [self setStartTime:kNXELayerDefaultStartTime endTime:kNXELayerDefaultEndTime];
        [self setBrightness:kNXELayerDefaultBrightness contrast:kNXELayerDefaultContrast saturation:kNXELayerDefaultSaturation];
    }
    return self;
}
#pragma mark - setter

- (void)setStartTime:(int)startTime endTime:(int)endTime
{
    self.startTime = startTime;
    self.endTime = endTime;
}

- (void)setBrightness:(float)brightness contrast:(float)contrast saturation:(float)saturation
{
    self.brightness = brightness;
    self.contrast = contrast;
    self.saturation = saturation;
}

- (void)setLayerSize:(CGSize)size
{
    self.width = size.width;
    self.height = size.height;
}

- (void)setX:(int)x
{
    CGRect scaledFrame = self.scaledFrame;
    CGFloat max =  LayerManager.sharedInstance.renderRegionSize.width;
    scaledFrame.origin.x = x / max;
    self.scaledFrame = scaledFrame;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.x = x;
}

- (void)setY:(int)y
{
    CGRect scaledFrame = self.scaledFrame;
    CGFloat max =  LayerManager.sharedInstance.renderRegionSize.height;
    scaledFrame.origin.y = y / max;
    self.scaledFrame = scaledFrame;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.y = y;
}

- (void)setAlpha:(float)alpha
{
    _alpha = alpha;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.alpha = _alpha;
}

- (void)setAngle:(float)angle
{
    _angle = angle;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.angle = _angle;
}

- (void)setHFlip:(BOOL)hFlip
{
    _hFlip = hFlip;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.hFlip = _hFlip;
}

- (void)setVFlip:(BOOL)vFlip
{
    _vFlip = vFlip;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.vFlip = _vFlip;
}

- (void)setBrightness:(float)brightness
{
    _brightness = brightness;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.brightness = _brightness;
}

- (void)setContrast:(float)contrast
{
    _contrast = contrast;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.contrast = _contrast;
}

- (void)setSaturation:(float)saturation
{
    _saturation = saturation;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.saturation = _saturation;
}

- (void)setWidth:(int)width
{
    CGRect scaledFrame = self.scaledFrame;
    CGFloat max =  LayerManager.sharedInstance.renderRegionSize.width;
    scaledFrame.size.width = width / max;
    self.scaledFrame = scaledFrame;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.width = width;
}

- (void)setHeight:(int)height
{
    CGRect scaledFrame = self.scaledFrame;
    CGFloat max =  LayerManager.sharedInstance.renderRegionSize.height;
    scaledFrame.size.height = height / max;
    self.scaledFrame = scaledFrame;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.height = height;
}

- (void)setStartTime:(int)startTime
{
    _startTime = startTime;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.startTime = _startTime;
}

- (void)setEndTime:(int)endTime
{
    _endTime = endTime;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.endTime = _endTime;
}

- (void)setColorFilter:(NXELutTypes)type
{
    _colorFilter = type;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    [layer setLut:(KMLutType)_colorFilter];
}

- (void)setScale:(CGFloat)scale
{
    _scale = scale;
    
    if(self.layerId == NXELayerIDUnknown) return;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    layer.scale = _scale;
}

- (void) setScaledFrame:(CGRect)frame
{
    _scaledFrame = frame;
    
    if (self.layerId != NXELayerIDUnknown) {
        KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
        if (layer != nil) {
            CGSize regionSize = LayerManager.sharedInstance.renderRegionSize;
            layer.x = frame.origin.x * regionSize.width;
            layer.y = frame.origin.y * regionSize.height;
            layer.width = frame.size.width * regionSize.width;
            layer.height = frame.size.height * regionSize.height;
        }
    }
}

- (void)setInAnimation:(NXEInAnimationType)inAnimation
{
    [self setInAnimationType:inAnimation duration:self.inAnimationDuration];
}

- (void)setInAnimationDuration:(int)inAnimationDuration
{
    [self setInAnimationType:self.inAnimation duration:inAnimationDuration];
}

- (void)setOutAnimation:(NXEOutAnimationType)outAnimation
{
    [self setOutAnimationType:outAnimation duration:self.outAnimationDuration];
}

- (void)setOutAnimationDuration:(int)outAnimationDuration
{
    [self setOutAnimationType:self.outAnimation duration:outAnimationDuration];
}

- (void)setExpression:(NXEExpressionType)expression
{
    [self setOverallAnimation:expression];
}

#pragma mark - getter

- (float)getAlpha
{
    CGFloat retValue = _alpha;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.alpha;
    
    return retValue;
}

- (float)getAngle
{
    CGFloat retValue = _angle;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.angle;
    
    return retValue;
}

- (BOOL)getHFlip
{
    BOOL retValue = _hFlip;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.hFlip;
    
    return retValue;
}

- (BOOL)getVFlip
{
    BOOL retValue = _vFlip;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.vFlip;
    
    return retValue;
}

- (float)getBrightness
{
    CGFloat retValue = _brightness;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue =  layer.brightness;
    
    return retValue;
}

- (float)getContrast
{
    CGFloat retValue = _contrast;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.contrast;
    
    return retValue;
}

- (float)getSaturation
{
    CGFloat retValue = _saturation;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.saturation;
    
    return retValue;
}

- (int)width
{
    int result;
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if (layer) {
        result = layer.width;
    } else {
        // fallback to stored (scaled)
        result = self.scaledFrame.size.width * LayerManager.sharedInstance.renderRegionSize.width;
    }
    return result;
}

- (int)height
{
    int result;
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if (layer) {
        result = layer.height;
    } else {
        // fallback to stored (scaled)
        result = self.scaledFrame.size.height * LayerManager.sharedInstance.renderRegionSize.height;
    }
    return result;
}

- (int)x
{
    int result;
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if (layer) {
        result = layer.x;
    } else {
        // fallback to stored (scaled)
        result = self.scaledFrame.origin.x * LayerManager.sharedInstance.renderRegionSize.width;
    }
    return result;
}

- (int)y
{
    int result;
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if (layer) {
        result = layer.y;
    } else {
        // fallback to stored (scaled)
        result = self.scaledFrame.origin.y * LayerManager.sharedInstance.renderRegionSize.height;
    }
    return result;
}

- (int)getStartTime
{
    int retValue = _startTime;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.startTime;
    
    return retValue;
}

- (int)getEndTime
{
    int retValue = _endTime;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.endTime;
    
    return retValue;
}

- (NXELutTypes)colorFilter
{
    NXELutTypes retValue = _colorFilter;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if ( layer != nil ) {
        KMLut *lut = [layer getLut];
        if ( lut ) {
            retValue = (NXELutTypes)[lut getType];
        } else {
            retValue = NXE_LUT_NONE;
        }
    }
    
    return retValue;
}

- (void)setLutId:(NXELutID)lutId
{
    [self setColorFilter:lutId];
}

-(NXELutID) lutId
{
    NXELutID result = kLutIdNotFound;
    NXELutTypes lutType = [self colorFilter];
    
    if( lutType != NXE_LUT_NONE ) {
        result = (NXELutID)lutType;
    }
    return result;
}

- (CGFloat)scale
{
    CGFloat retValue = _scale;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    if(layer != nil) retValue = layer.scale;
    
    return retValue;
}

- (CGRect) scaledFrame
{
    if (self.layerId != NXELayerIDUnknown) {
        KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
        if (layer != nil) {
            CGSize regionSize = LayerManager.sharedInstance.renderRegionSize;
            _scaledFrame.origin.x = ((float)layer.x) / regionSize.width;
            _scaledFrame.origin.y = ((float)layer.y) / regionSize.height;
            _scaledFrame.size.width = ((float)layer.width) / regionSize.width;
            _scaledFrame.size.height = ((float)layer.height) / regionSize.height;
        }
    }
    CGRect result = _scaledFrame;
    
    return result;
}

#pragma mark - animation

- (void)setInAnimationType:(NXEInAnimationType)inAnimationType duration:(int)duration
{
    if(duration < 0) {
        duration = 0;
    } else if(duration > 5*1000) {
        duration = 5*1000;
    }
    
    _inAnimation = inAnimationType;
    _inAnimationDuration = duration;
    
    KMAnimationType animatinType = _inAnimation == NXE_ANIMATION_IN_NONE ? KMAnimationNone : (KMAnimationType)_inAnimation;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    [layer setInAnimationType:animatinType WithDuration:_inAnimationDuration];
}

- (void)setOverallAnimation:(NXEExpressionType)expressionType
{
    _expression = expressionType;
    
    KMAnimationType animatinType = _expression == NXE_ANIMATION_OUT_NONE ? KMAnimationNone : KMAnimationInMax + _expression;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    [layer setOverallAnimationType:animatinType];
}

- (void)setOutAnimationType:(NXEOutAnimationType)outAnimationType duration:(int)duration
{
    if(duration < 0) {
        duration = 0;
    } else if(duration > 5*1000) {
        duration = 5*1000;
    }
    
    _outAnimation = outAnimationType;
    _outAnimationDuration = duration;
    
    KMAnimationType animatinType = _outAnimation == NXE_ANIMATION_IN_NONE ? KMAnimationNone : KMAnimationOverallMax + _outAnimation;
    
    KMLayer *layer = [[LayerManager sharedInstance] getLayer:self.layerId];
    [layer setOutAnimationType:animatinType WithDuration:_outAnimationDuration];
}

@end
