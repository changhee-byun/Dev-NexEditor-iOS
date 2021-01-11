/**
 * File Name   : UIImage+Resize.h
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

@interface UIImage(resize)
- (UIImage *) resizing:(CGSize) size;

/// Creates a new image with the given size, original image placed at the center and background fill with the background color
- (UIImage *) expandingToSize:(CGSize) size backgroundColor:(UIColor *) backgroundColor;
@end
