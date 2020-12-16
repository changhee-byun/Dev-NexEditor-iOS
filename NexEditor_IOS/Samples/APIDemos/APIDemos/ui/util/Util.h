/******************************************************************************
 * File Name   :	Util.h
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

#import <Foundation/Foundation.h>
@import Photos;

#define BUNDLE_CLIPSOURCES_VIDEO    @"clipsources/video"
#define BUNDLE_CLIPSOURCES_AUDIO    @"clipsources/audio"
#define BUNDLE_CLIPSOURCES_IMAGE    @"clipsources/picture"
#define BUNDLE_CLIPSOURCES_STICKER    @"clipsources/sticker"

@interface Util : NSObject

/**
 */
+ (NSString*) getCurrentTime;

+ (UIImage *)imageFromVideoAsset:(AVAsset*)asset;

+ (NSString *)getUIFontName:(NSString *)fontName;

+ (int)getBitrateWithWidth:(int)width height:(int)height fps:(int)fps;
@end
