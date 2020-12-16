/******************************************************************************
 * File Name   :	Util.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <CoreText/CoreText.h>
#import "Util.h"
@import NexEditorFramework;
@implementation Util

+ (NSString*)getCurrentTime
{
    NSDateFormatter* dataFormatter = [[NSDateFormatter alloc] init];
    [dataFormatter setDateFormat:@"HH:mm:ss"];
    
    NSString* currentTime = [dataFormatter stringFromDate:[NSDate date]];
    return currentTime;
}

+ (UIImage *)imageFromVideoAsset:(AVAsset*)asset
{
    if(asset == nil) {
        return nil;
    }
    
    //  Get thumbnail at the very start of the video
    CMTime thumbnailTime = [asset duration];
    thumbnailTime.value = 25;
    
    //  Get image from the video at the given time
    AVAssetImageGenerator *imageGenerator = [[AVAssetImageGenerator alloc] initWithAsset:asset];
    imageGenerator.appliesPreferredTrackTransform = YES;
    
    CGImageRef imageRef = [imageGenerator copyCGImageAtTime:thumbnailTime actualTime:NULL error:NULL];
    UIImage *thumbnail = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    
    return thumbnail;
}

+ (NSString *)getUIFontName:(NSString *)fontName
{
    NSBundle *bundle = [NSBundle bundleForClass:NXEEngine.class];
    NSString *fontPath = [bundle pathForResource:fontName ofType:@"ttf" inDirectory:@"resource/font"];
    NSData *inData = [NSData dataWithContentsOfFile:fontPath];
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)inData);
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

+ (int)getBitrateWithWidth:(int)width height:(int)height fps:(int)fps
{
    return ((float)(width * height * fps * 0.17) / 1024) * 1000;
}

@end
