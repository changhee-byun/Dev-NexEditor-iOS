/**
 * File Name   : KMAnimationInterpolation.h
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

@interface KMAnimationInterpolation : NSObject

+ (Float32)rateForLinearInterpolationWithElapseTime:(Float32)elapsedTime duration:(Float32)duration;
+ (Float32)rateForDecelerateInterpolation:(Float32)rate;
+ (Float32)rateForAccelerateInterpolation:(Float32)rate;
+ (Float32)rateForBounceInterpolation:(Float32)rate;
+ (Float32)rateForPopInterpolation:(Float32)rate;
+ (Float32)rateForAccelerateDecelerateInterpolation:(Float32)rate;

@end
