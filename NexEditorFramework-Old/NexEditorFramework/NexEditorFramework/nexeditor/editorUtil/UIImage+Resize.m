/**
 * File Name   : UIImage+Resize.m
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
#import "UIImage+Resize.h"

@implementation UIImage(resize)
- (UIImage *) resizing:(CGSize) maxSize
{
    CGFloat rate = (self.size.width > self.size.height) ? maxSize.width / self.size.width : maxSize.height / self.size.height;
    CGFloat newWidth = self.size.width * rate;
    CGFloat newHeight = self.size.height * rate;
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(newWidth, newHeight), NO, self.scale);
    [self drawInRect:CGRectMake(0, 0, newWidth, newHeight)];
    UIImage *result = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return result;
}

- (UIImage *) expandingToSize:(CGSize) size backgroundColor:(UIColor *) backgroundColor
{
    // transparent background by default
    CGColorRef fillColor = UIColor.clearColor.CGColor;
    if (backgroundColor) {
        fillColor = backgroundColor.CGColor;
    }
    
    // center
    CGRect rectImagePlacement = CGRectMake(0, 0, self.size.width, self.size.height);
    rectImagePlacement.origin.x = (size.width - self.size.width) / 2;
    rectImagePlacement.origin.y = (size.height - self.size.height) / 2;
    
    UIGraphicsBeginImageContextWithOptions(size, NO, self.scale);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetFillColorWithColor(context, fillColor);
    CGContextFillRect(context, CGRectMake(0, 0, size.width, size.height));
    [self drawInRect:rectImagePlacement];
    UIImage *result = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return result;
}

@end
