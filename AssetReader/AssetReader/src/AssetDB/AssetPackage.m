/**
 * File Name   : AssetPackage.m
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

#import "AssetPackage.h"
#import "AssetImplicitRules.h"
#import "AssetEncryptionManifest.h"

static NSString *const kAssetNames = @"assetName";

@interface AssetPackage()
@property (nonatomic, strong) NSString *path;
@property (nonatomic, strong) NSString *packageId;
@property (nonatomic, strong) AssetEncryptionManifest *manifest;
@end

@implementation AssetPackage
- (instancetype) initWithPath:(NSString *) path packageId:(NSString *)packageId
{
    self = [super init];
    if ( self ) {
        self.path = path;
        self.packageId = packageId;
        NSString *ejsonPath = [path stringByAppendingPathComponent:kAssetReservedFilenameEncryptionManifest];
        self.manifest = [[AssetEncryptionManifest alloc] initWithPath:ejsonPath];
    }
    return self;
}

- (NSString *) mergePath
{
    return [self.path stringByAppendingPathComponent:kAssetReservedPathnameMergePath];
}

- (NSString *) cryptrecp
{
    return self.manifest.provider;
}

- (BOOL) isEncrypted:(NSString *) subpath
{
    return [self.manifest hasSubpath:subpath];
}

- (NSDictionary *) packageInfo
{
    NSDictionary *result = nil;
    NSString *path = [self.path stringByAppendingPathComponent:kAssetReservedFilenamePackageInfo];
    if (path) {
        NSData *data = [NSData dataWithContentsOfFile:path];
        if (data) {
            result = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        }
    }
    return result;
}

- (NSDictionary <NSString *, NSString *> *) assetNames
{
    NSDictionary <NSString *, NSString *> *result = @{};
    
    NSDictionary *packageInfo = [self packageInfo];
    if (packageInfo) {
        id assetNames = packageInfo[kAssetNames];
        if ([assetNames isKindOfClass:NSDictionary.class]) {
            result = (NSDictionary <NSString *, NSString *> *) packageInfo[kAssetNames];
        }
    }
    return result;
}
@end
