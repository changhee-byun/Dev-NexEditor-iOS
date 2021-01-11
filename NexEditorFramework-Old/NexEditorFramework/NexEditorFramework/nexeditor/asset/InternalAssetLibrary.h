/**
 * File Name   : InternalAssetLibrary.h
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
#import "NXEAssetLibrary.h"
/**
 * SDK internal should use this class to retrieve asset items while
 * code outside of SDK, i.e. App, should use NXEAssetLibrary class
 * \discussion Access through [AssetLibrary library]
 */
@interface InternalAssetLibrary : NSObject <NXEAssetLibrary>
- (void) addAssetSourceURL:(NSURL *) url;
- (void) addAssetPackageURLs:(NSArray<NSURL *> *) urls;
- (void) removeAssetPackageURLs:(NSArray <NSURL *> *) urls;
- (void) removeAllAssetPackageURLs;
- (void) reloadSources;
- (NSArray<NXEAssetItem *> *) itemsInCategory:(NSString *) category includeHidden:(BOOL) includeHidden;
- (NXEAssetItem *) itemForId:(NSString *) itemId includeHidden:(BOOL) includeHidden;

+ (instancetype) library;
@end
