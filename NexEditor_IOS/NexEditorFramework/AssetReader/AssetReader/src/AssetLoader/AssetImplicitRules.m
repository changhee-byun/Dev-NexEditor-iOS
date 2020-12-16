/**
 * File Name   : AssetImplicitRules.m
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

#import "AssetImplicitRules.h"

NSString *const kAssetReservedFilenamePackageInfo = @"packageinfo.json";
NSString *const kAssetReservedFilenameEncryptionManifest = @"e.json";
NSString *const kAssetReservedFilenameItemInfo = @"_info.json";
// package subdirectory for resolving mergePaths
NSString *const kAssetReservedPathnameMergePath = @"merge";

static NSString *const kInPackageAssetSubidItem = @"item";

@implementation AssetImplicitRules

+ (NSString *) implicitItemIdWithPackageId:(NSString *) packageId itemDirName:(NSString *) dirName
{
    return [NSString stringWithFormat:@"%@.%@.%@", packageId, kInPackageAssetSubidItem, dirName];
}

@end
