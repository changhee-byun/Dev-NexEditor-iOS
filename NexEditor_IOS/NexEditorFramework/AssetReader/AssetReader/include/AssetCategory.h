/**
 * File Name   : AssetCategory.h
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

#import <Foundation/Foundation.h>

extern NSString *const kAssetItemCategoryAudio;
extern NSString *const kAssetItemCategoryBackground;
extern NSString *const kAssetItemCategoryEffect;
extern NSString *const kAssetItemCategoryFilter;
extern NSString *const kAssetItemCategoryAudioFilter;
extern NSString *const kAssetItemCategoryFont;
extern NSString *const kAssetItemCategoryOverlay;
extern NSString *const kAssetItemCategoryTemplate;
extern NSString *const kAssetItemCategoryTransition;
extern NSString *const kAssetItemCategoryFilterEffect;
extern NSString *const kAssetItemCategoryCollage;
extern NSString *const kAssetItemCategoryBeatTemplate;


@interface AssetItemCategory : NSObject 
@property (class, readonly) NSArray<NSString *> *categories;
@end

BOOL AssetCategoryIsValidCategoryName(NSString *categoryName);
