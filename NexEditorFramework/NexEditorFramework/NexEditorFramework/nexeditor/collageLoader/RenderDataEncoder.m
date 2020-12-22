/**
 * File Name   : RenderDataEncoder.m
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

#import "RenderDataEncoder.h"
#import "CollageInfo.h"

@interface NSString (RenderDataEncoder)

- (NSString *)encode;
- (NSString *)dateString:(NSDate *)date;

@end

@interface UIColor (RenderDataEncoder)

- (NSString *)hexString;

@end

@implementation RenderDataEncoder

+ (NSString *)textRenderData:(CollageTitleConfiguration *)titleConfiguration titleID:(NSString *)titleID date:(NSDate *)date defaultText:(NSString *)defaultText
{
    NSString *const prefixContent = @"@content=";
    NSString *const prefixCollage = @"@collage=";
    
    NSMutableString *renderData = [NSMutableString string];
    NSString *key = nil; NSString *value = nil;
    NSString *text = defaultText;
    
    if (titleConfiguration.textStyle.font) {
        key = [NSString stringWithFormat:@"%@_font", titleID].encode;
        value = [NSString stringWithFormat:@"fontid:%@", titleConfiguration.textStyle.font.fontName].encode;
        [renderData appendFormat:@"%@", [NSString stringWithFormat:@"%@=%@&", key, value]];
    }
    if (titleConfiguration.textStyle.fillColor) {
        key = [NSString stringWithFormat:@"%@_fill_color", titleID].encode;
        value = titleConfiguration.textStyle.fillColor.hexString.encode;
        [renderData appendFormat:@"%@", [NSString stringWithFormat:@"%@=%@&", key, value]];
    }
    if (titleConfiguration.textStyle.strokeColor) {
        key = [NSString stringWithFormat:@"%@_stroke_color", titleID].encode;
        value = titleConfiguration.textStyle.strokeColor.hexString.encode;
        [renderData appendFormat:@"%@", [NSString stringWithFormat:@"%@=%@&", key, value]];
    }
    if (titleConfiguration.textStyle.shadowColor) {
        key = [NSString stringWithFormat:@"%@_dropshadow_color", titleID].encode;
        value = titleConfiguration.textStyle.shadowColor.hexString.encode;
        [renderData appendFormat:@"%@", [NSString stringWithFormat:@"%@=%@&", key, value]];
    }
    if (titleConfiguration.text) {
        if ([titleConfiguration.text hasPrefix:prefixContent]) {
            // TBD : This part is put off because the concept of "@content=" does not be determined
            //
        } else if ([titleConfiguration.text hasPrefix:prefixCollage]) {
            NSString *formatPattern = [titleConfiguration.text substringFromIndex:prefixCollage.length];
            titleConfiguration.text = [formatPattern dateString:date];
        }
        text = titleConfiguration.text;
    }
    key = titleID.encode;
    value = text.encode;
    [renderData appendFormat:@"%@", [NSString stringWithFormat:@"%@=%@", key, value]];

    return renderData;
}

@end

@implementation NSString (RenderDataEncoder)

- (NSString *)encode
{
    return [self stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet letterCharacterSet]];
}

- (NSString *)dateString:(NSDate *)date
{
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    // TBD : date format is set "NSDateFormatterMediumStyle" because the concept of date format does not  be determined.
    //
    NSDateFormatterStyle dateFormatterStyle = NSDateFormatterMediumStyle;
    dateFormatter.dateStyle = dateFormatterStyle;
    dateFormatter.locale = [NSLocale autoupdatingCurrentLocale];
    return [dateFormatter stringFromDate:date];
}

@end

@implementation UIColor (RenderDataEncoder)

- (NSString *)hexString
{
    CGFloat red, green, blue, alpha;
    [self getRed:&red green:&green blue:&blue alpha:&alpha];
    int rgba = (int)(red*255)<<24 | (int)(green*255)<<16 | (int)(blue*255)<<8 | (int)(alpha*255)<<0;
    return [NSString stringWithFormat:@"#%08x", rgba];
}

@end
