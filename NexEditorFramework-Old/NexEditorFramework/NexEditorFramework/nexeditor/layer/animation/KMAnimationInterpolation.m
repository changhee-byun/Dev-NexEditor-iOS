/**
 * File Name   : KMAnimationInterpolation.m
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

#import "KMAnimationInterpolation.h"

#define bounce(x) (x)*(x)*8.0f

@implementation KMAnimationInterpolation

+ (Float32)rateForLinearInterpolationWithElapseTime:(Float32)elapsedTime duration:(Float32)duration
{
    return elapsedTime / duration;
}

+ (Float32)rateForDecelerateInterpolation:(Float32)rate
{
    return 1.0f - (1.0f - rate) * (1.0f - rate);
}

+ (Float32)rateForAccelerateInterpolation:(Float32)rate
{
    return rate * rate;
}

+ (Float32)rateForBounceInterpolation:(Float32)rate
{
    rate *= 1.1226f;
    if (rate < 0.3535f) return bounce(rate);
    else if (rate < 0.7408f) return bounce(rate - 0.54719f) + 0.7f;
    else if (rate < 0.9644f) return bounce(rate - 0.8526f) + 0.9f;
    else return bounce(rate - 1.0435f) + 0.95f;
}

+ (Float32)rateForPopInterpolation:(Float32)rate
{
    if(rate <= 0.8) {
        rate = rate / 0.8;
        rate = [self rateForAccelerateInterpolation:rate];
        rate = 1.2 * rate;
    }
    else {
        rate = (rate - 0.8) / 0.2;
        rate = 1.2 - 0.2 * [self rateForDecelerateInterpolation:rate];
    }
    return rate;
}

+ (Float32)rateForAccelerateDecelerateInterpolation:(Float32)rate
{
    return cosf((rate + 1) * M_PI) / 2.0 + 0.5;
}

@end
