/**
 * File Name   : FontTools.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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
#import <UIKit/UIKit.h>

@interface FontTools : NSObject

+ (NSString *)fontNameWithBundleFontName:(NSString *)fontName;

+ (CGFloat)adjustFontSizeWithFontName:(NSString *)fontName
                       textRegionSize:(CGSize)textRegionSize
                             textSize:(CGFloat)textSize
                             maxLines:(int)maxLines
                                 text:(NSString *)text
                       destTextRegion:(CGSize *)destTextRegion;

@end
