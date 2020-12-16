/**
 * File Name   : NSString+Color.m
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
#import "NSString+Color.h"
#import "NSString+ValueReader.h"

@implementation NSString(Color)

- (UIColor *_Nonnull)hexARGBColor
{
    if (self.length < 8) {
        return UIColor.clearColor;
    }
    
    NSString *color = [self substringWithRange:NSMakeRange(0, 8)];
    
    int fromAlpha = 0; int fromRed = 2; int fromGreen = 4; int fromBlue = 6;
    int alpha = (int)[[color substringWithRange:NSMakeRange(fromAlpha,2)] scanHexLongLong];
    int red = (int)[[color substringWithRange:NSMakeRange(fromRed,2)] scanHexLongLong];
    int green = (int)[[color substringWithRange:NSMakeRange(fromGreen,2)] scanHexLongLong];
    int blue = (int)[[color substringWithRange:NSMakeRange(fromBlue,2)] scanHexLongLong];
    
    return [UIColor colorWithRed:(red)/255.0 green:(green)/255.0 blue:(blue)/255.0 alpha:alpha/255.0];
}


- (UIColor *_Nonnull)hashHexARGBColor
{
    if ([self characterAtIndex:0] != [@"#" characterAtIndex:0]) {
        return UIColor.clearColor;
    }
    return [[self substringWithRange:NSMakeRange(1, 8)] hexARGBColor];
}

@end
