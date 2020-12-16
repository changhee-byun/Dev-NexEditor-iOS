/**
 * File Name   : AssetLibraryProxy.h
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

#import "AssetReader.h"
#import "AssetLibrary.h"

@protocol AssetLibraryProxy
- (NSData *) loadResourceDataAtSubpath: (NSString *) subpath forItemId:(NSString *) itemId;

/// result may conform ConfigurableAssetItem, and/or, TransitionAssetItemAttributes
- (id<AssetItem>) itemForId:(NSString *) itemId;
- (NSString *) buildThemeDataWithItems:(NSArray <id<AssetItem>> *) items;
- (NSString *) buildRenderItemDataWithItem:(AssetItem *)item;
@end

@interface AssetLibrary(Proxy)
+ (id<AssetLibraryProxy>) proxy;
@end

