/**
 * File Name   : AssetVolume.h
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

typedef NS_ENUM(NSUInteger, AssetVolumeType) {
    AssetVolumeTypeUnknown,
    AssetVolumeTypeBundle,
    AssetVolumeTypeLibrary,
    AssetVolumeTypeDocuments,
    AssetVolumeTypeMainBundle
};

extern NSString *const kURLSchemeAssetVolume;

@interface AssetVolume: NSObject
@property (nonatomic) AssetVolumeType volumeType;
@property (nonatomic, readonly) NSString *subpath;
@property (nonatomic, readonly) NSString *resolvedPath;
@property (nonatomic, readonly) NSURL *resolvedURL;
@property (nonatomic, readonly) NSString *assetVolumeURLString;

- (id) initWithType: (AssetVolumeType) volumeType subpath:(NSString *) subpath;
/// <kURLSchemeAssetVolume>://<bundle|documents|library>/path
- (id) initWithVolumePath:(NSString *) volumePath;

+ (NSURL *) resolvedFileURLForVolumeType:(AssetVolumeType) type subpath:(NSString *) subpath;
+ (AssetVolumeType) volumeTypeFromString:(NSString *) sourceTypeString;
+ (NSString *) stringFromVolumeType:(AssetVolumeType) type;

@end
