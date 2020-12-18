/**
 * File Name   : AssetDBIndexed.m
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

#import "AssetDBIndexed.h"
#import "AssetLoader.h"
#import "AssetImplicitRules.h"
#import "AssetVolume.h"
#import "AssetItemPrivate.h"

@interface AssetDBIndexed()
@property (nonatomic, strong) AssetLoader *loader;
@property (nonatomic, strong, readonly) NSMutableDictionary<NSString *, AssetPackage *> *packagesStore;
@end

@implementation AssetDBIndexed
@synthesize packagesStore = _packagesStore;

- (id) initWithSourceURLs:(NSArray<NSURL *> *) urls
{
    self = [super init];
    if ( self ) {
        NSArray *volumes = [self.class volumesWithSourceURLs:urls];
        self.loader = [[AssetLoader alloc] initWithVolumes:volumes indexer:nil];
    }
    return self;
}

#pragma mark -
- (NSMutableDictionary<NSString *, AssetPackage *> *) packagesStore
{
    if ( _packagesStore == nil ) {
        _packagesStore = [NSMutableDictionary dictionary];
        
        for (NSString *packageId in self.loader.packageIds) {
            AssetPackage *package = [self creaetPackageForId:packageId];
            _packagesStore[packageId] = package;
        }
    }
    return _packagesStore;
}

- (NSArray<NSString *> *)categories
{
    return AssetItemCategory.categories;
}

- (NSArray<AssetPackage *> *)packages
{
    return self.packagesStore.allValues;
}

- (void) setDecryptionDelegate:(id<AssetResourceDecryptionDelegate>) delegate
{
    self.loader.decryptionDelegate = delegate;
}

- (id<AssetResourceDecryptionDelegate>) decryptionDelegate
{
    return self.loader.decryptionDelegate;
}

#pragma mark -
- (AssetPackage *) creaetPackageForId:(NSString *) packageId
{
    NSString *path = [self.loader packagePathForId:packageId];
    return [[AssetPackage alloc] initWithPath:path packageId:packageId];
}

- (AssetPackage *) packageForId:(NSString *) packageId
{
    if (packageId == nil) {
        return nil;
    }
    return self.packagesStore[packageId];
}

- (NSArray<AssetItem *> *) itemsForCategory:(NSString *) category
{
    NSMutableArray<AssetItem *> *result = [NSMutableArray array];
    NSArray *itemIds = [self.loader itemIdsInCategory:category];
    for (NSString *itemId in itemIds) {
        AssetItem *item = [self itemForId:itemId];
        if ( item ) {
            [result addObject:item];
        }
    }
    return [result copy];
}

- (AssetItem *) itemForId:(NSString *) itemId
{
    AssetItem *result = nil;
    AssetItemPathInfo *pathInfo = [self.loader itemPathInfoForId:itemId];
    AssetPackage *package = [self packageForId:pathInfo.packageId];
    if ( pathInfo && package) {
        NSData *data = [NSData dataWithContentsOfFile:pathInfo.infoPath];
        if (data) {
            NSString *subpath = [[pathInfo subpathInPackagePath:package.path] stringByAppendingPathComponent:kAssetReservedFilenameItemInfo];
            data = [self decrypt:data packagePath:package.path subpath:subpath];
            
            NSString *itemPath = [pathInfo.infoPath stringByDeletingLastPathComponent];
            result = [[AssetItem alloc] initWithPath:itemPath
                                             package:package
                                            category:pathInfo.category
                                            infoData:data];
        }
    }
    
    return result;
}

- (void) addPackageURL:(NSURL *) url
{
    [self.loader addPackageAtURL:url];
}

- (void) removePackageAtURL:(NSURL *) url
{
    [self.loader removePackageAtURL:url];
}


#pragma -
/// decrypt 'encrypted' data if encrypted or returns 'encrypted' as it is
- (NSData *) decrypt:(NSData *) encrypted packagePath:(NSString *) packagePath subpath:(NSString *) subpath
{
    NSData *result = encrypted;
    if ( self.decryptionDelegate ) {
        if (subpath) {
            self.decryptionDelegate.packagePath = packagePath;
            self.decryptionDelegate.subpathToEncryptionManifest = kAssetReservedFilenameEncryptionManifest;
            result = [self.decryptionDelegate decryptData:encrypted atSubpath:subpath];
        }
    }
    return result;
}

#pragma mark -

+ (NSArray<AssetVolume *> *) volumesWithSourceURLs:(NSArray<NSURL *> *) urls
{
    NSMutableArray<AssetVolume *> *result = [NSMutableArray array];
    for (NSURL *url in urls) {
        AssetVolume *volume = [[AssetVolume alloc] initWithVolumePath:url.absoluteString];
        [result addObject:volume];
    }
    return [result copy];
}

@end
