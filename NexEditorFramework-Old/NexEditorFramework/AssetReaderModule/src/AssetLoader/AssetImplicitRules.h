/**
 * File Name   : AssetImplicitRules.h
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

extern NSString *const kAssetReservedFilenamePackageInfo;
extern NSString *const kAssetReservedFilenameItemInfo;
extern NSString *const kAssetReservedPathnameMergePath;
extern NSString *const kAssetReservedFilenameEncryptionManifest;

@interface AssetImplicitRules: NSObject
/// Constructs itemId for whose id is not assigned, according to the implicit itemid rule
+ (NSString *) implicitItemIdWithPackageId:(NSString *) packageId itemDirName:(NSString *) dirName;
@end
