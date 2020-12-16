/**
 * File Name   : NexDrawInfo.h
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
#import <CoreGraphics/CoreGraphics.h>

@interface NexDrawInfo : NSObject

@property (nonatomic) int drawId;
@property (nonatomic) int clipId;
@property (nonatomic) int subEffectId;
@property (nonatomic) int startTimeMS;
@property (nonatomic) int endTimeMS;
@property (nonatomic) unsigned int lut;
@property (nonatomic) int rotate;
@property (nonatomic) int userRotate;
@property (nonatomic) CGRect startRect;
@property (nonatomic) CGRect endRect;
@property (nonatomic) int brightness;
@property (nonatomic) int contrast;
@property (nonatomic) int saturation;
@property (nonatomic) int tintColor;
@property (nonatomic) int translate_x;
@property (nonatomic) int translate_y;
@property (nonatomic) int custom_lut_a;
@property (nonatomic) int custom_lut_b;
@property (nonatomic) int custom_lut_power;

@end
