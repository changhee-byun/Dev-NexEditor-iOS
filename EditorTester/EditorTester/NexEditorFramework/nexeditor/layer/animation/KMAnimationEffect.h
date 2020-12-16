/**
 * File Name   : KMAnimationEffect.h
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

#import <Foundation/Foundation.h>
#import "KMLayer.h"

typedef NS_ENUM(NSUInteger, AnimationEffect) {
    AnimationAlpha = 0,
    AnimationMove,
    AnimationRotate,
    AnimationScale
};

typedef NS_ENUM(NSUInteger, TimeInterpolation) {
    AccelerateDecelerateInterpolation = 0,
    AccelerateInterpolation,
    DecelerateInterpolation,
    LinearInterpolation,
    BounceInterpolation,
    CycleInterpolation,
    AnticipateInterpolation,
    AnticipateOvershootInterpolation,
    OvershootInterpolation,
    PopInterpolation,
    TimeInterpolationMax
};

@interface KMAnimationEffect : NSObject

@property (nonatomic) int animationStartTime;
@property (nonatomic) int animationEndTime;
@property (nonatomic) int animationDuration;

@property (nonatomic) AnimationEffect animationEffect;
@property (nonatomic) TimeInterpolation interpolation;

/**레이어에 애니메이션을 적용한다.
 */
- (void)applyAnimation:(KMLayer*)layer forTime:(int)time;

- (void)applyAnimationEx:(KMLayer*)layer forTime:(int)time;

- (float)rateFor:(TimeInterpolation)interpolation time:(int)time;

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime aniEffect:(AnimationEffect)animationEffect interpolation:(TimeInterpolation)interpolation;

- (BOOL)isActiveAnimationAtTime:(int)currentPosition;

@end

@interface AlphaAnimation : KMAnimationEffect

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime alphaStartValue:(float)alphaStartValue alphaEndValue:(float)alphaEndValue interpolation:(TimeInterpolation)interpolation;

@end

@interface MoveAnimation : KMAnimationEffect

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime layerPoint:(CGPoint)layerPoint moveStartPoint:(CGPoint)moveStartPoint moveEndPoint:(CGPoint)moveEndPoint interpolation:(TimeInterpolation)interpolation;

@end

@interface RotateAnimation : KMAnimationEffect

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime layerRotate:(float)layerRotate rotateValue:(float)rotateValue clockWise:(BOOL)clockWise interpolation:(TimeInterpolation)interpolation;

@end

@interface ScaleAnimation : KMAnimationEffect

- (instancetype)initWithAniStartTime:(int)aniStartTime aniEndTime:(int)aniEndTime xScaleStartValue:(float)xScaleStartValue yScaleStartValue:(float)yScaleStartValue xScaleEndValue:(float)xScaleEndValue yScaleEndValue:(float)yScaleEndValue interpolation:(TimeInterpolation)interpolation;

@end
