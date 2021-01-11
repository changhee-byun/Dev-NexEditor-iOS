/**
 * File Name   : KMAnimationEffect.m
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

#import "KMAnimationEffect.h"
#import "KMAnimationInterpolation.h"
#import "Math.h"
#import <NexEditorEngine/NexEditorEngine.h>

#define LOG_TAG @"AnimationEffect"

@implementation AlphaAnimation {
    float alphaAniStartValue;
    float alphaAniEndValue;
}

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime alphaStartValue:(float)alphaStartValue alphaEndValue:(float)alphaEndValue interpolation:(TimeInterpolation)interpolation
{
    self = [super initWithAniStartTime:aniStartTime aniEndTime:aniEndTime aniEffect:AnimationAlpha interpolation:interpolation];
    if(self) {
        alphaAniStartValue = alphaStartValue;
        alphaAniEndValue = alphaEndValue;
    }
    return self;
}

- (void)applyAnimation:(KMLayer *)layer forTime:(int)time
{
    float rate = [self rateFor:self.interpolation time:time];
    if(self.interpolation == TimeInterpolationMax) {
        rate = 1.0;
    }
    
    float alpha;
    float alphaDifference = fabs(alphaAniStartValue - alphaAniEndValue);
    
    if(alphaAniStartValue > alphaAniEndValue) {
        alpha = alphaDifference * rate * -1;
    } else {
        alpha = alphaDifference * rate;
    }
    
    layer.alpha = alphaAniStartValue + alpha;
    
    NexLogD(LOG_TAG, @"Alpha: layid:%ld alpha:%.1f rate:%.5f time:%d, stime:%d", layer.layerId, layer.alpha, rate, time, self.animationStartTime);
}

- (void)applyAnimationEx:(KMLayer*)layer forTime:(int)time
{
    if(time < self.animationStartTime) {
        layer.alpha = alphaAniStartValue;
    } else if(time > self.animationEndTime) {
        layer.alpha = alphaAniEndValue;
    }
}

@end

@implementation MoveAnimation {
    float basedx;         //레이어 처음 위치 좌표.(left)
    float basedy;         //레이어 처음 위치 좌표.(top)
    
    CGPoint moveAniStartPoint;
    CGPoint moveAniEndPoint;
}

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime layerPoint:(CGPoint)layerPoint moveStartPoint:(CGPoint)moveStartPoint moveEndPoint:(CGPoint)moveEndPoint interpolation:(TimeInterpolation)interpolation
{
    self = [super initWithAniStartTime:aniStartTime aniEndTime:aniEndTime aniEffect:AnimationAlpha interpolation:interpolation];
    if(self) {
        basedx = layerPoint.x; basedy = layerPoint.y;
        
        moveAniStartPoint = moveStartPoint;
        moveAniEndPoint = moveEndPoint;
    }
    return self;
}

- (void)applyAnimation:(KMLayer *)layer forTime:(int)time
{
    float rate = [self rateFor:self.interpolation time:time];
    if(self.interpolation == TimeInterpolationMax) {
        rate = 1.0;
    }

    float moveX = 0, moveY = 0; //현재 애니메이션에서 있어야할 위치
    
    float xDifference = fabs(moveAniEndPoint.x - moveAniStartPoint.x);
    
    if(moveAniStartPoint.x > moveAniEndPoint.x) {
        moveX = xDifference * rate * -1;
    } else {
        moveX = xDifference * rate;
    }
    
    float yDifference = fabs(moveAniEndPoint.y - moveAniStartPoint.y);

    if(moveAniStartPoint.y > moveAniEndPoint.y) {
        moveY = yDifference * rate * -1;
    } else {
        moveY = yDifference * rate;
    }
    
    layer.x = basedx + moveAniStartPoint.x + moveX;
    layer.y = basedy + moveAniStartPoint.y + moveY;
    
    NexLogD(LOG_TAG, @"Move: layid:%ld, disX:%.1f distY:%.1f movX:%.1f movY:%.1f lay_x:%.1f, lay_y:%.1f rate:%.5f time:%d, stime:%d",
            layer.layerId, xDifference, yDifference, moveX, moveY, layer.x, layer.y, rate, time, self.animationStartTime);
}

- (void)applyAnimationEx:(KMLayer*)layer forTime:(int)time
{
    if(time < self.animationStartTime) {
        layer.x = basedx;
        layer.y = basedy;
    } else if(time > self.animationEndTime) {
        layer.x = basedx + moveAniEndPoint.x;
        layer.y = basedy + moveAniEndPoint.y;
    }
}

@end

@implementation RotateAnimation {
    float layerRotateValue;
    
    BOOL clockWise;
    float rotateAniValue;
}

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime layerRotate:(float)layerRotate rotateValue:(float)rotateValue clockWise:(BOOL)clockWise interpolation:(TimeInterpolation)interpolation
{
    self = [super initWithAniStartTime:aniStartTime aniEndTime:aniEndTime aniEffect:AnimationAlpha interpolation:interpolation];
    if(self) {
        layerRotateValue = layerRotate;
        rotateAniValue = rotateValue;
    }
    return self;
}

- (void)applyAnimation:(KMLayer *)layer forTime:(int)time
{
    float rate = [self rateFor:self.interpolation time:time];
    if(self.interpolation == TimeInterpolationMax) {
        rate = 1.0;
    }
    
    layer.angle = layerRotateValue + rotateAniValue * rate * (clockWise ? 1 : -1);
    
    NexLogD(LOG_TAG, @"Rotate: layid:%ld angle:%.1f rate:%.5f time:%d, stime:%d", layer.layerId, layer.angle, rate, time, self.animationStartTime);
}

- (void)applyAnimationEx:(KMLayer*)layer forTime:(int)time
{
    if(time < self.animationStartTime) {
        layer.angle = layerRotateValue;
    } else if(time > self.animationEndTime) {
        layer.angle = layerRotateValue + rotateAniValue * (clockWise ? 1 : -1);
    }
}

@end

@implementation ScaleAnimation {
    float scaleAniStartXPoint;
    float scaleAniEndXPoint;
    
    float scaleAniStartYPoint;
    float scaleAniEndYPoint;
}

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime xScaleStartValue:(float)xScaleStartValue yScaleStartValue:(float)yScaleStartValue xScaleEndValue:(float)xScaleEndValue yScaleEndValue:(float)yScaleEndValue interpolation:(TimeInterpolation)interpolation
{
    self = [super initWithAniStartTime:aniStartTime aniEndTime:aniEndTime aniEffect:AnimationAlpha interpolation:interpolation];
    if(self) {
        scaleAniStartXPoint = xScaleStartValue;
        scaleAniEndXPoint = xScaleEndValue;
        
        scaleAniStartYPoint = yScaleStartValue;
        scaleAniEndYPoint = yScaleEndValue;
    }
    return self;
}

- (void)applyAnimation:(KMLayer *)layer forTime:(int)time
{
    float rate = [self rateFor:self.interpolation time:time];
    if(self.interpolation == TimeInterpolationMax) {
        rate = 1.0;
    }
    
    float xScale = 0.0f, yScale = 0.0f;
    
    float scaleXDifference = fabs(scaleAniEndXPoint - scaleAniStartXPoint);
    
    if(scaleAniStartXPoint > scaleAniEndXPoint) {
        xScale = scaleXDifference * rate * -1;
    } else {
        xScale = scaleXDifference * rate;
    }
    
    float scaleYDifference = fabs(scaleAniEndYPoint - scaleAniStartYPoint);
    
    if(scaleAniStartYPoint > scaleAniEndYPoint) {
        yScale = scaleYDifference * rate * -1;
    } else {
        yScale = scaleYDifference * rate;
    }
    
    layer.xScale = scaleAniStartXPoint + xScale > 3 ? 3 : scaleAniStartXPoint + xScale;
    layer.yScale = scaleAniStartYPoint + yScale > 3 ? 3 : scaleAniStartYPoint + yScale;
    
    NexLogD(LOG_TAG, @"Scale: layid:%ld, sX:%.1f eX:%.1f sY:%.1f eY:%.1f xScale:%.5f, yScale:%.5f rate:%.5f time:%d, stime:%d",
            layer.layerId, scaleAniStartXPoint, scaleAniEndXPoint, scaleAniStartYPoint, scaleAniEndYPoint, layer.xScale, layer.yScale, rate, time, self.animationStartTime);
}

- (void)applyAnimationEx:(KMLayer*)layer forTime:(int)time
{
    if(time < self.animationStartTime) {
        layer.xScale = scaleAniStartXPoint > 3 ? 3 : scaleAniStartXPoint;
        layer.yScale = scaleAniStartYPoint > 3 ? 3 : scaleAniStartYPoint;
    } else if(time > self.animationEndTime) {
        layer.xScale = scaleAniEndXPoint > 3 ? 3 : scaleAniEndXPoint;
        layer.yScale = scaleAniEndYPoint > 3 ? 3 : scaleAniEndYPoint;
    }
}

@end

@implementation KMAnimationEffect

- (void)applyAnimation:(id)layer forTime:(int)time {
}

- (void)applyAnimationEx:(KMLayer*)layer forTime:(int)time {
}

- (float)rateFor:(TimeInterpolation)interpolation time:(int)time
{
    float rate = [KMAnimationInterpolation rateForLinearInterpolationWithElapseTime:time duration:self.animationDuration];
    float result = 1.0;
    
    switch(interpolation) {
        case AccelerateDecelerateInterpolation:
            result = [KMAnimationInterpolation rateForAccelerateDecelerateInterpolation:rate];
            break;
            
        case AccelerateInterpolation:
            result = [KMAnimationInterpolation rateForAccelerateInterpolation:rate];
            break;
            
        case DecelerateInterpolation:
            result = [KMAnimationInterpolation rateForDecelerateInterpolation:rate];
            break;
            
        case BounceInterpolation:
            result = [KMAnimationInterpolation rateForBounceInterpolation:rate];
            break;
            
        case PopInterpolation:
            result = [KMAnimationInterpolation rateForPopInterpolation:rate];
            break;
            
        default:
            /* ToDo
             case CycleInterpolation:
             case AnticipateInterpolation:
             case AnticipateOvershootInterpolation:
             case OvershootInterpolation:
             */
            break;
    }
    
    return result;
}

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime aniEffect:(AnimationEffect)animationEffect interpolation:(TimeInterpolation)interpolation
{
    self = [super init];
    if(self) {
        self.animationStartTime = aniStartTime;
        self.animationEndTime = aniEndTime;
        self.animationDuration = aniEndTime - aniStartTime;
        self.animationEffect = animationEffect;
        self.interpolation = interpolation;
    }
    return self;
}

- (BOOL)isActiveAnimationAtTime:(int)currentPosition
{
    return currentPosition >= self.animationStartTime && currentPosition <= self.animationEndTime;
}

@end
