/******************************************************************************
 * File Name   : KMAnimation.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMAnimation.h"
#import "KMAnimationInterpolation.h"
#import "KMLayer.h"

@implementation KMAnimation {
    float x;         //레이어 위치 좌표.(left)
    float y;         //레이어 위치 좌표.(top)
    
    float alpha;   //레이어의 전체 투명도.(0~1)
    float scale;   //레이어의 크기 배율 값.(0.5 ~ 3)
    int angle;     //레이어의 회전 값.(0~359, degree)
}

- (void)save:(KMLayer*)layer {
    x = layer.x;
    y = layer.y;
    alpha = layer.alpha;
    scale = layer.scale;
    angle = layer.angle;
}

- (void)restore:(KMLayer*)layer {
    layer.x = x;
    layer.y = y;
    layer.alpha = alpha;
    layer.scale = scale;
    layer.angle = angle;
}

#pragma mark - type 별 애니메이션 적용하는 메소드
- (void)applyAnimation:(KMLayer*)layer {
    float rate = [self getInterpolation];
    
    switch (self.type) {
            //in
        case KMAnimationInFade:
            [self applyFadeIn:layer WithRate:rate];
            break;
        case KMAnimationInPop:
            [self applyPopIn:layer WithRate:rate];
            break;
        case KMAnimationInSlideRight:
        case KMAnimationInSlideLeft:
        case KMAnimationInSlideUp:
        case KMAnimationInSlideDown:
            [self applySlideIn:layer WithRate:rate];
            break;
            //overall
        case KMAnimationOverallBlinkSlow:
            [self applyBlinkSlow:layer];
            break;
        case KMAnimationOverallFlicker:
            [self applyFlicker:layer];
            break;
            //out
        case KMAnimationOutFade:
            [self applyFadeOut:layer WithRate:rate];
            break;
        case KMAnimationOutSlideRight:
        case KMAnimationOutSlideLeft:
        case KMAnimationOutSlideUp:
        case KMAnimationOutSlideDown:
            [self applySlideOut:layer WithRate:rate];
            break;
        default:
            break;
    }
}

#pragma mark - in 애니메이션 적용하는 메소드
/** fade in 애니메이션
 */
- (void)applyFadeIn:(KMLayer*)layer WithRate:(float)rate {
    layer.alpha = alpha * rate;
}

/** scale이 0에서 원본보다 커진 후 다시 원본으로 돌아오는 애니메이션
 */
- (void)applyPopIn:(KMLayer*)layer WithRate:(float)rate {
    layer.scale = scale * rate;
}

/** slide in 애니메이션
 */
- (void)applySlideIn:(KMLayer*)layer WithRate:(float)rate {
    [self applyFadeIn:layer WithRate:rate];     //알파 적용
    
    float unit = 3; //slide 할 거리에 대한 비율. 값이 커질 수록 거리는 줄어든다.
    float moveX=0, moveY=0; //현재 애니메이션에서 있어야할 위치
    
    //왼쪽에서 오른쪽으로 이동
    if(self.type == KMAnimationInSlideRight) {
        float total = -layer.width * layer.scale / unit;
        moveX = total * (1.0 - rate);
    }
    //오른쪽에서 왼쪽으로 이동
    else if(self.type == KMAnimationInSlideLeft) {
        float total = layer.width * layer.scale / unit;
        moveX = total * (1.0 - rate);
    }
    //아래에서 위로 이동
    else if(self.type == KMAnimationInSlideUp) {
        float total = layer.height * layer.scale / unit;
        moveY = total * (1.0 - rate);
    }
    //위에서 아래로 이동
    else if(self.type == KMAnimationInSlideDown) {
        float total = -layer.height * layer.scale / unit;
        moveY = total * (1.0 - rate);
    }
    
    layer.x = x + moveX;
    layer.y = y + moveY;
}

#pragma mark - overall 애니메이션 적용하는 메소드
/**깜빡이듯이 보였다 안보였다하는 애니메이션
 */
- (void)applyBlinkSlow:(KMLayer*)layer {
    BOOL isShow = (self.elapseTime / 400) % 2 == 0;
    layer.alpha = isShow ? alpha : 0;
}

/**빠르게 알파값이 1~0~1 로 변하는 애니메이션
 __  __
 애니메이션 주기 :   \/  \/
 */
- (void)applyFlicker:(KMLayer*)layer {
    int frequency = 150;                    //주기 값. millisecond
    float rate = (float)(self.elapseTime % frequency) / (float)(frequency - 1); //주기에서 진행된 비율 값. 0 ~ 1.
    
    float opacity;
    // ~ 2/4 까지는 알파1
    if(rate <= 0.5) {
        opacity = 1.0f;
    }
    // 2/4 ~ 3/4 까지는 알파0 으로 감소
    else if(rate <= 0.75) {
        rate -= 0.5;
        rate /= 0.25;
        opacity = 1.0 - rate;
    }
    // 3/4 ~ 4/4 까지는 알파1 으로 증가
    else {
        rate -= 0.75;
        opacity = rate / 0.25;
    }
    layer.alpha = alpha * opacity;
}

#pragma mark - out 애니메이션 적용하는 메소드
/** fade out 애니메이션
 */
- (void)applyFadeOut:(KMLayer*)layer WithRate:(float)rate {
    layer.alpha = alpha * (1.0 - rate);
}

/** slide out 애니메이션
 */
- (void)applySlideOut:(KMLayer*)layer WithRate:(float)rate {
    [self applyFadeOut:layer WithRate:rate];    //fade out 적용
    
    //알파 적용
    
    float unit = 3; //slide 할 거리에 대한 비율. 값이 커질 수록 거리는 줄어든다.
    float moveX=0, moveY=0; //현재 애니메이션에서 있어야할 위치
    
    //왼쪽에서 오른쪽으로 이동
    if(self.type == KMAnimationOutSlideRight) {
        float total = layer.width * layer.scale / unit;
        moveX = total * rate;
    }
    //오른쪽에서 왼쪽으로 이동
    else if(self.type == KMAnimationOutSlideLeft) {
        float total = -layer.width * layer.scale / unit;
        moveX = total * rate;
    }
    //아래쪽에서 위쪽으로 이동
    else if(self.type == KMAnimationOutSlideUp) {
        float total = -layer.height * layer.scale / unit;
        moveY = total * rate;
    }
    //위쪽에서 아래쪽으로 이동
    else if(self.type == KMAnimationOutSlideDown) {
        float total = layer.height * layer.scale / unit;
        moveY = total * rate;
    }
    
    layer.x = x + moveX;
    layer.y = y + moveY;
}

#pragma mark - type 별 interpolation 값

- (float)getInterpolation {
    float rate = [KMAnimationInterpolation rateForLinearInterpolationWithElapseTime:self.elapseTime duration:self.duration];
    switch (self.type) {
            //in
        case KMAnimationInFade:
            return rate;
        case KMAnimationInPop:
            return [KMAnimationInterpolation rateForPopInterpolation:rate];
        case KMAnimationInSlideRight:
        case KMAnimationInSlideLeft:
        case KMAnimationInSlideUp:
        case KMAnimationInSlideDown:
            return [KMAnimationInterpolation rateForDecelerateInterpolation:rate];
            //overall
        case KMAnimationOverallBlinkSlow:
        case KMAnimationOverallFlicker:
            return rate;
            //out
        case KMAnimationOutFade:
            return rate;
        case KMAnimationOutSlideRight:
        case KMAnimationOutSlideLeft:
        case KMAnimationOutSlideUp:
        case KMAnimationOutSlideDown:
            return [KMAnimationInterpolation rateForDecelerateInterpolation:rate];
        default:
            return 1;
    }
}

@end
