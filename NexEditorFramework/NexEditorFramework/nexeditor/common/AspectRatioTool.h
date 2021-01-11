/**
 * File Name   : AspectRatioTool.h
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
#import "NXEEditorType.h"


NXESizeInt NXESizeIntMake(int32_t width, int32_t height);

@interface AspectRatioTool : NSObject

/// NXEAspectTypeCustom is not valid as input. Returns (0, 0).
+ (NXESizeInt) ratioWithType: (NXEAspectType) type;
+ (CGRect) aspectFit:(CGSize) size ratio:(NXESizeInt) ratioi;
+ (CGRect) aspectFit:(CGSize) size ratiof:(float) ratio;
+ (CGRect) rectFills:(CGRect) rect ratio:(float) ratio fit:(BOOL) fit;
+ (CGRect) rectFills:(CGRect) rect inSize:(CGSize) size fit:(BOOL) fit;

/// returns (0, 0) if any parameter has 0
+ (NXESizeInt) frameSizeWithRatio:(NXESizeInt) ratio minSize:(int32_t) minSize;
@end
