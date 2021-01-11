/**
 * File Name   : AssetDB.h
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

#import "AssetPackage.h"
#import "AssetCategory.h"
#import "AssetItem.h"
#import "AssetResourceDecryptionDelegate.h"

@protocol AssetDB <NSObject>
@property (nonatomic, readonly) NSArray<NSString *> *categories;
@property (nonatomic, readonly) NSArray<AssetPackage *> *packages;
@property (nonatomic, weak) id<AssetResourceDecryptionDelegate> decryptionDelegate;

- (NSArray<AssetItem *> *) itemsForCategory:(NSString *) category;
- (AssetItem *) itemForId:(NSString *) itemId;
- (AssetPackage *) packageForId:(NSString *) packageId;

- (void) addPackageURL:(NSURL *) url;
- (void) removePackageAtURL:(NSURL *) url;
@end

@interface AssetDB : NSObject
/// Creates an asset db object conforms to AssetDB protocol with package source directory URLs provided as parameter
+ (id<AssetDB>) assetDBWithSourceURLs:(NSArray<NSURL *> *) urls;
@end
