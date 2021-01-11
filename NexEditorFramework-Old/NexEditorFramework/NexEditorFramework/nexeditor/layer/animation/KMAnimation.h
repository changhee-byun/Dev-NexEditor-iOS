/******************************************************************************
 * File Name   : KMAnimation.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

@class KMLayer;

// Animation types
typedef NS_ENUM(NSInteger, KMAnimationType) {
    KMAnimationNone = 0,
    
    //in animation
    KMAnimationInFade,
    KMAnimationInPop,
    KMAnimationInSlideRight,
    KMAnimationInSlideLeft,
    KMAnimationInSlideUp,
    KMAnimationInSlideDown,
    KMAnimationInMax = KMAnimationInSlideDown,
    
    //overall animation
    KMAnimationOverallBlinkSlow,
    KMAnimationOverallFlicker,
    KMAnimationOverallMax = KMAnimationOverallFlicker,
    
    //out animation
    KMAnimationOutFade,
    KMAnimationOutSlideRight,
    KMAnimationOutSlideLeft,
    KMAnimationOutSlideUp,
    KMAnimationOutSlideDown,
    KMAnimationOutMax = KMAnimationOutSlideDown,
    
    KMAnimationMax
};

/** 레이어의 애니메이션을 담당하는 클래스
 */
@interface KMAnimation : NSObject

@property (nonatomic) KMAnimationType type; //animation type
@property (nonatomic) int duration;         //duration(in millisecond)
@property (nonatomic) int elapseTime;

/**레이어의 현재 상태를 저장한다.
 * 애니메이션 시작 전에 호출하여 상태를 저장해야 한다.
 */
- (void)save:(KMLayer*)layer;

/**레이어의 상태를 복원한다.
 * 애니메이션 적용 전 상태로 되돌린다.
 */
- (void)restore:(KMLayer*)layer;

/**레이어에 애니메이션을 적용한다.
 */
- (void)applyAnimation:(KMLayer*)layer;

@end
