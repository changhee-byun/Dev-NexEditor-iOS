/**
 * File Name   : AssetCategory.m
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

#import "AssetCategory.h"
NSString *const kAssetItemCategoryAudio = @"audio";
NSString *const kAssetItemCategoryBackground = @"background";
NSString *const kAssetItemCategoryEffect = @"effect";
NSString *const kAssetItemCategoryFilter = @"filter";
NSString *const kAssetItemCategoryAudioFilter = @"audiofilter";
NSString *const kAssetItemCategoryFont = @"font";
NSString *const kAssetItemCategoryOverlay = @"overlay";
NSString *const kAssetItemCategoryTemplate = @"template";
NSString *const kAssetItemCategoryTransition = @"transition";
NSString *const kAssetItemCategoryFilterEffect = @"filtereffect";
NSString *const kAssetItemCategoryCollage = @"collage";
NSString *const kAssetItemCategoryBeatTemplate = @"beattemplate";


static NSArray<NSString *> * _assetCategoryNames()
{
    static NSArray<NSString *> *categoryNames = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        categoryNames = @[
                          kAssetItemCategoryAudio,
                          kAssetItemCategoryBackground,
                          kAssetItemCategoryEffect,
                          kAssetItemCategoryFilter,
                          kAssetItemCategoryAudioFilter,
                          kAssetItemCategoryFont,
                          kAssetItemCategoryOverlay,
                          kAssetItemCategoryTemplate,
                          kAssetItemCategoryTransition,
                          kAssetItemCategoryFilterEffect,
                          kAssetItemCategoryCollage,
                          kAssetItemCategoryBeatTemplate
                          ];
    });
    return categoryNames;
}

@implementation AssetItemCategory

+ (NSArray<NSString *> *)categories
{
    return _assetCategoryNames();
}

@end

BOOL AssetCategoryIsValidCategoryName(NSString *categoryName)
{
    return [_assetCategoryNames() indexOfObject:categoryName] != NSNotFound;
}
