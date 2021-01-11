/**
 * File Name   : AssetLoader.h
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
#import "AssetItemPathInfo.h"
#import "AssetIndexer.h"
#import "AssetVolume.h"
#import "AssetResourceDecryptionDelegate.h"

@interface AssetLoader: NSObject
@property (nonatomic, readonly) NSArray<NSString *> *packageIds;
@property (nonatomic, readonly) NSArray<NSString *> *categories;
@property (nonatomic, weak) id<AssetResourceDecryptionDelegate> decryptionDelegate;

- (NSString *) packagePathForId:(NSString *) packageId;
- (NSString *) packageInfoPathForId:(NSString *) packageId;
- (NSArray<NSString *> *) categoriesForPackageId:(NSString *) packageId;
- (NSArray<NSString *> *) itemIdsInCategory:(NSString *) category;
- (AssetItemPathInfo *) itemPathInfoForId:(NSString *) itemId;

// Adds a new package at the file URL specified by packageURL
- (BOOL) addPackageAtURL:(NSURL *) packageURL;
- (BOOL) removePackageAtURL:(NSURL *) packageURL;

- (instancetype) initWithVolumes:(NSArray<AssetVolume *> *) volumes indexer:(id<AssetIndexer>) indexer;

@end
