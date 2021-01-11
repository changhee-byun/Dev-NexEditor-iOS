/**
 * File Name   : FontTools.m
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
#import "FontTools.h"
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>

@implementation FontTools

+ (NSString *)fontNameWithBundleFontName:(NSString *)fontName
{
    NSString *fontPath = [[NSBundle bundleForClass:self.class] pathForResource:fontName ofType:@"ttf" inDirectory:@"resource/font"];
    if(fontPath == nil)
        return nil;
    else
        return [self fontNameWithFontPath:fontPath];
}

+ (NSString *)fontNameWithFontPath:(NSString *)fontPath
{
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)[NSData dataWithContentsOfFile:fontPath]);
    CGFontRef fontRef = CGFontCreateWithDataProvider(provider);
    CFErrorRef error;
    if (! CTFontManagerRegisterGraphicsFont(fontRef, &error)) {
        CFStringRef errorDescription = CFErrorCopyDescription(error);
        NSLog(@"Failed to Register the specified graphics font with the font manager: %@", errorDescription);
        CFRelease(errorDescription);
    }
    NSString *fontName_ = (NSString *)CFBridgingRelease(CGFontCopyPostScriptName(fontRef));
    
    CFRelease(fontRef);
    CFRelease(provider);
    
    return fontName_;
}

+ (CGFloat)adjustFontSizeWithFontName:(NSString *)fontName
                       textRegionSize:(CGSize)textRegionSize
                             textSize:(CGFloat)textSize
                             maxLines:(int)maxLines
                                 text:(NSString *)text
                       destTextRegion:(CGSize *)destTextRegion
{
    CGFloat size = textSize;
    CGFloat minSize = 5.0;
    
    //
    do {
        UIFont *tmpFont = [UIFont fontWithName:fontName size:size];
        
        NSMutableParagraphStyle *tmpParagraphStyle = [[NSMutableParagraphStyle new] autorelease];
        tmpParagraphStyle.lineBreakMode = NSLineBreakByWordWrapping;
        tmpParagraphStyle.lineSpacing = 0.5;
        tmpParagraphStyle.lineHeightMultiple = 1.1;
        
        CGRect tmpTextRect = [text boundingRectWithSize:CGSizeMake(textRegionSize.width, CGFLOAT_MAX)
                                                options:NSStringDrawingUsesFontLeading | NSStringDrawingUsesLineFragmentOrigin
                                             attributes:@{ NSFontAttributeName : tmpFont, NSParagraphStyleAttributeName : tmpParagraphStyle }
                                                context:nil];
        
        if(tmpTextRect.size.width < textRegionSize.width) {
            if(maxLines < 1) {
                if(tmpTextRect.size.height < textRegionSize.height) {
                    *destTextRegion = CGSizeMake(tmpTextRect.size.width, tmpTextRect.size.height);
                    break;
                } else {
                    size -= 1.0;
                }
            } else {
                if(tmpTextRect.size.height < textRegionSize.height) {
                    *destTextRegion = CGSizeMake(tmpTextRect.size.width, tmpTextRect.size.height);
                    break;
                } else {
                    size -= 12.0;
                }
            }
        } else {
            size -= 1.0;
        }
    } while(size > minSize);
    
    return size;
}

@end
