/**
 * File Name   : ThemeResourceLoader.h
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
#import <CoreGraphics/CoreGraphics.h>

#define SUBSAMPLE_RATE_NONE         (1.0)

typedef struct {
    float subsampleThemeResourceRate;       // default is 1.0. 0.5 to reduce size by half in horizontally and vertically.
    CGSize subsampleThemeResourceMinSize;   // theme resource image larger than this would be subsampled to reduce memory.
} NexImageLoaderConfig;

#ifdef __cplusplus
extern "C" {
#endif
size_t CGImageGetEvenNumberedWidth(CGImageRef image);

size_t CGImageGetEvenNumberedHeight(CGImageRef image);
#ifdef __cplusplus
};
#endif


@interface ThemeResourceLoader: NSObject

+ (CGImageRef) createImage4Text:(NSString *)textinfo;

+ (CGImageRef) createImage4Theme:(NSString *)path
                         config:(NexImageLoaderConfig)config
                         loadResourceData:(NSData *(^)(NSString *  itemId, NSString *  subpath)) loadResourceData;

+ (unsigned char*) getPixelsWithImage:(CGImageRef )image;

@end



