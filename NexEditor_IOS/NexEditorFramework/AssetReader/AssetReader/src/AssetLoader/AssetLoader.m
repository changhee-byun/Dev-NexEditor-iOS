/**
 * File Name   : AssetLoader.m
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

#import "AssetLoader.h"
#import "AssetLoaderPrivate.h"
#import "AssetCategory.h"
#import "AssetImplicitRules.h"
#import "AssetIndexer.h"
#import "DefaultSlowAssetIndexer.h"

/**
 * Indexing Sections
 * - Packages:      [packageId: [volume, subpath]]
 * - CategoryItems: [category: [itemId]]
 * - Items:         [itemId: [packageId, category, subpath]]
 */

static NSString *const kIndexSectionPackages = @"packages";
static NSString *const kIndexSectionCategoryItems = @"categoryItems";
static NSString *const kIndexSectionItems = @"items";

// Entry keys for Packages section
static NSString *const kIndexEntryPackageVolumeKey = @"volume";
static NSString *const kIndexEntryPackageSubpathKey = @"subpath";

// Entry keys for Item section
static NSString *const kIndexEntryItemPackageIdKey = @"packageId";
static NSString *const kIndexEntryItemCategoryKey = @"category";
static NSString *const kIndexEntryItemSubpathKey = @"subpath";

static NSString *const kAssetItemInfoKeyId = @"id";

@interface AssetLoader()
@property (nonatomic, strong) NSArray<AssetVolume *> *volumes;
@property (nonatomic, strong) id<AssetIndexer> indexer;
@property (nonatomic) BOOL indexed;
@end

@implementation AssetLoader

- (instancetype) initWithVolumes:(NSArray<AssetVolume *> *) volumes indexer:(id<AssetIndexer>) indexer
{
    self = [super init];
    if ( self ) {
        self.volumes = volumes;
        if ( indexer == nil ) {
            indexer = [[DefaultSlowAssetIndexer alloc] init];
        }
        self.indexer = indexer;
    }
    return self;
}

#pragma mark -

/// List of ids of all installed packages
- (NSArray<NSString *> *) packageIds
{
    [self indexIfNotReady];
    NSDictionary<NSString *, NSObject *> *packageEntries = [self.indexer allEntriesForSection:kIndexSectionPackages];
    return packageEntries.allKeys;
}

/// All valid categories, not necessarily installed
- (NSArray<NSString *> *) categories
{
    return AssetItemCategory.categories;
}

#pragma mark -

+ (NSString *) packagePathWithVolumePath:(NSString *) volumePath subpath:(NSString *) subpath
{
    return [[[[AssetVolume alloc] initWithVolumePath:volumePath] resolvedPath] stringByAppendingPathComponent:subpath];
}

// Path to the installed package directory
- (NSString *) packagePathForId:(NSString *) packageId
{
    [self indexIfNotReady];
    
    NSString *result = nil;
    NSDictionary *entry = (NSDictionary *)[self.indexer entryForKey:packageId section:kIndexSectionPackages];
    if ( entry ) {
        NSString *volumePath = entry[kIndexEntryPackageVolumeKey];
        NSString *subpath = entry[kIndexEntryPackageSubpathKey];
        if (volumePath && subpath) {
            result = [self.class packagePathWithVolumePath:volumePath subpath:subpath];
        } else {
            NSLog(@"Wrong package entry data for packageId: %@, %@", packageId, entry);
        }
    }
    return result;
}

/// Path to packageinfo.json of the package
- (NSString *) packageInfoPathForId:(NSString *) packageId
{
    return [[self packagePathForId:packageId] stringByAppendingPathComponent:kAssetReservedFilenamePackageInfo];
}

/// Categories found from the installed package. Could be slow since it scans the file system.
- (NSArray<NSString *> *) categoriesForPackageId:(NSString *) packageId
{
    NSArray<NSString *> *result = @[];
    NSString *packagePath = [self packagePathForId: packageId];
    if ( packagePath ) {
        result = [self.class scanCategoriesAtPackagePath: packagePath];
    }
    return result;
}

/// List of all itemIds from the category across all the installed packages
- (NSArray<NSString *> *) itemIdsInCategory:(NSString *) category
{
    [self indexIfNotReady];
    NSArray<NSString *> *result = [self.indexer entryForKey:category section:kIndexSectionCategoryItems];
    if ( result == nil ) {
        result = @[];
    }
    return result;
}

/// Path to the asset item directory of the itemId
- (AssetItemPathInfo *) itemPathInfoForId:(NSString *) itemId
{
    [self indexIfNotReady];
    AssetItemPathInfo *result = nil;
    NSDictionary *itemEntry = [self.indexer entryForKey:itemId section:kIndexSectionItems];
    NSString *packageId = itemEntry[kIndexEntryItemPackageIdKey];
    NSString *subpath = itemEntry[kIndexEntryItemSubpathKey];
    NSString *category = itemEntry[kIndexEntryItemCategoryKey];
    if ( packageId && subpath && category) {
        // volume/package.subpath/item.subpath
        NSString *packagePath = [self packagePathForId:packageId];
        if ( packagePath ) {
            NSString *path = [packagePath stringByAppendingPathComponent:subpath];
            result = [AssetItemPathInfo pathInfoWithPath:path packageId:packageId category:category];
        }
    }
    return result;
}

#pragma mark -

/**
 * Conveniently build a dictionary type entry for a packageId
 * \param fields @[volume, subpath]
 * \return @{volumeKey: volume, subpathKey: subpath}
 */
+ (NSDictionary *) packageEntry:(NSArray *) fields
{
    return @{kIndexEntryPackageVolumeKey: fields[0],
             kIndexEntryPackageSubpathKey: fields[1]};
}

+ (NSDictionary *) itemEntry:(NSArray *) fields
{
    return @{kIndexEntryItemPackageIdKey:fields[0],
             kIndexEntryItemCategoryKey:fields[1],
             kIndexEntryItemSubpathKey: fields[2]};
}

+ (NSArray<NSString *> *) scanCategoriesAtPackagePath:(NSString *) path
{
    NSMutableArray<NSString *> *result = [NSMutableArray array];
    NSFileManager *fm = [NSFileManager defaultManager];
    NSURL *URL = [NSURL fileURLWithPath:path];
    NSArray *URLs = [fm contentsOfDirectoryAtURL:URL
                      includingPropertiesForKeys:@[NSURLIsDirectoryKey]
                                         options:NSDirectoryEnumerationSkipsHiddenFiles error:nil];
    for (NSURL *subdirURL in URLs) {
        if ( AssetCategoryIsValidCategoryName(subdirURL.lastPathComponent) ) {
            BOOL isDir = NO;
            if ([fm fileExistsAtPath:subdirURL.path isDirectory:&isDir] && isDir) {
                [result addObject:subdirURL.lastPathComponent];
            }
        }
    }
    return [result copy];
}

- (BOOL) addPackageAtURL:(NSURL *) packageURL categoryItemIds:(NSMutableDictionary *) categoryItemIds
{
    packageURL = [packageURL URLByResolvingSymlinksInPath];
    NSString *URLString = [[AssetVolume alloc] initWithVolumePath:packageURL.absoluteString].assetVolumeURLString;
    
    NSURL *url = [NSURL URLWithString:URLString];
    NSString *path = url.URLByDeletingLastPathComponent.absoluteString;
    NSString *subpath = url.lastPathComponent;
    
    if (path == nil || subpath == nil) {
        // package path unrecognized. discard.
        return NO;
    }
    NSString *packageId = URLString;
    __block BOOL found = NO;
    [self.class scanPackageAtURL:packageURL
                    foundPackage:^(BOOL readable) {
                        if (!readable) return;
                        found = YES;
                        
                        // prefix packageId to avoid colision from the same package name from differnt volume
                        [self.indexer setEntry:[self.class packageEntry:@[path, subpath]]
                                        forKey:packageId
                                       section:kIndexSectionPackages];
                        
                        if (self.decryptionDelegate) {
                            self.decryptionDelegate.packagePath = packageURL.path;
                            self.decryptionDelegate.subpathToEncryptionManifest = kAssetReservedFilenameEncryptionManifest;
                        }
                    }
                       foundItem:^(NSData *infoData, NSString *packageName, NSString *category, NSString *subpath) {
                           NSString *itemId = nil;
                           
                           if (self.decryptionDelegate) {
                               infoData = [self.decryptionDelegate decryptData:infoData atSubpath:[subpath stringByAppendingPathComponent:kAssetReservedFilenameItemInfo]];
                           }
                           NSDictionary *info = [NSJSONSerialization JSONObjectWithData:infoData options:0 error:nil];
                           if ( info ) {
                               itemId = info[kAssetItemInfoKeyId];
                           }
                           if ( itemId == nil ) {
                               itemId = [AssetImplicitRules implicitItemIdWithPackageId:packageId itemDirName:subpath.lastPathComponent];
                           }
                           NSDictionary *entry = [self.class itemEntry:@[packageId, category, subpath]];
                           [self.indexer setEntry:entry forKey:itemId section:kIndexSectionItems];
                           
                           NSArray *itemIds = categoryItemIds[category];
                           if (itemIds) {
                               itemIds = [itemIds arrayByAddingObjectsFromArray:@[itemId]];
                           } else {
                               itemIds = @[itemId];
                           }
                           categoryItemIds[category] = itemIds;
                           
                       }];
    return found;
}

- (BOOL) removePackageAtURL:(NSURL *) packageURL categoryItemIds:(NSMutableDictionary *) categoryItemIds
{
    packageURL = [packageURL URLByResolvingSymlinksInPath];
    NSString *packageId = [[AssetVolume alloc] initWithVolumePath:packageURL.absoluteString].assetVolumeURLString;
    // remove entry in kIndexSectionPackages with packageId as key
    // remove entries in kIndexSectionItems whoose kIndexEntryItemPackageIdKey is packageId
    // remove itemIds from categoryItemIds[] for each category
    [self.indexer removeEntryForKey:packageId section:kIndexSectionPackages];
    
    NSMutableDictionary *removingCategoryItemIds = [NSMutableDictionary dictionary];
    
    NSDictionary *items = [self.indexer allEntriesForSection:kIndexSectionItems];
    for( NSString *itemId in items.allKeys) {
        NSDictionary *entry = items[itemId];
        if ( [packageId isEqualToString:entry[kIndexEntryItemPackageIdKey]] ) {
            [self.indexer removeEntryForKey:itemId section:kIndexSectionItems];
            NSString *category = entry[kIndexEntryItemCategoryKey];
            
            NSArray *itemIds = removingCategoryItemIds[category];
            if (itemIds) {
                itemIds = [itemIds arrayByAddingObjectsFromArray:@[itemId]];
            } else {
                itemIds = @[itemId];
            }
            removingCategoryItemIds[category] = itemIds;
        }
    }
    
    
    for( NSString *category in categoryItemIds.allKeys ) {
        NSArray *removingItemIds = removingCategoryItemIds[category];
        if (removingItemIds.count > 0) {
            NSMutableArray *itemIds = [categoryItemIds[category] mutableCopy];
            if (itemIds) {
                [itemIds removeObjectsInArray:removingItemIds];
                categoryItemIds[category] = [itemIds copy];
            }
        }
    }
    return YES;
}

- (void) indexIfNotReady
{
    if (self.indexed) {
        return;
    }
    [self buildIndexWithVolumes:self.volumes];
    self.indexed = YES;
}

- (void) buildIndexWithVolumes:(NSArray <AssetVolume *> *) volumes
{
    NSMutableDictionary *categoryItemIds = [NSMutableDictionary dictionary];
    
    NSFileManager *fm = [NSFileManager defaultManager];
    
    for (AssetVolume *vol in volumes) {
        NSArray *packageURLs = [fm contentsOfDirectoryAtURL:vol.resolvedURL
                                 includingPropertiesForKeys:@[NSURLIsDirectoryKey]
                                                    options:NSDirectoryEnumerationSkipsHiddenFiles error:nil];
        
        for( NSURL *packageURL in packageURLs) {
            [self addPackageAtURL:packageURL categoryItemIds:categoryItemIds];
        }
    }
    
    if (self.decryptionDelegate) {
        [self.decryptionDelegate releaseResource];
    }
    
    for( NSString *category in categoryItemIds.allKeys) {
        [self.indexer setEntry:categoryItemIds[category]
                        forKey:category
                       section:kIndexSectionCategoryItems];
    }
}

- (BOOL) addPackageAtURL:(NSURL *) packageURL
{
    BOOL result = NO;
    NSMutableDictionary *categoryItemIds = [[self.indexer allEntriesForSection:kIndexSectionCategoryItems] mutableCopy];
    
    if ([self addPackageAtURL:packageURL categoryItemIds:categoryItemIds]) {
        if (self.decryptionDelegate) {
            [self.decryptionDelegate releaseResource];
        }
        for( NSString *category in categoryItemIds.allKeys) {
            [self.indexer setEntry:categoryItemIds[category]
                            forKey:category
                           section:kIndexSectionCategoryItems];
        }
        result = YES;
    }
    return result;
}

- (BOOL) removePackageAtURL:(NSURL *) packageURL
{
    BOOL result = NO;
    NSMutableDictionary *categoryItemIds = [[self.indexer allEntriesForSection:kIndexSectionCategoryItems] mutableCopy];
    
    if ([self removePackageAtURL:packageURL categoryItemIds:categoryItemIds]) {
        for( NSString *category in categoryItemIds.allKeys) {
            [self.indexer setEntry:categoryItemIds[category]
                            forKey:category
                           section:kIndexSectionCategoryItems];
        }
        result = YES;
    }
    return result;
}

#pragma mark -
+ (void) scanPackageAtURL:(NSURL *) packageURL
            foundPackage:(void (^)(BOOL readable)) foundPackage
               foundItem:(void (^)(NSData *infoData, NSString *packageName, NSString *category, NSString *subpath)) foundItem
{
    NSFileManager *fm = [NSFileManager defaultManager];

    NSString *packageName = packageURL.lastPathComponent;
    NSURL *packageInfoFileURL = [packageURL URLByAppendingPathComponent:kAssetReservedFilenamePackageInfo];
    
    if ( [fm isReadableFileAtPath:packageInfoFileURL.path] ) {
        foundPackage(YES);
        
        NSDirectoryEnumerator *enumerator = [fm enumeratorAtURL:packageURL
                                     includingPropertiesForKeys:@[NSURLNameKey, NSURLIsDirectoryKey]
                                                        options:NSDirectoryEnumerationSkipsHiddenFiles
                                                   errorHandler:nil];
        
        for(NSURL *fileURL in enumerator) {
            if (![fileURL.lastPathComponent isEqualToString:kAssetReservedFilenameItemInfo]) {
                continue;
            }
            NSArray *pathComponents = fileURL.pathComponents;
            if ( pathComponents.count < 4 ) {
                // too short for package/category/item/_info.json structure
                continue;
            }
            // Extracting category and item parts from path:.../[category/item]/_info.json
            pathComponents = [pathComponents subarrayWithRange:NSMakeRange(pathComponents.count - 3, 2)];
            NSString *category = pathComponents[0];
            NSString *subpath = [NSString pathWithComponents:pathComponents];
            
            NSData *data = [NSData dataWithContentsOfURL:fileURL];
            if ( data ) {
                foundItem(data, packageName, category, subpath );
            }
        }
    }
}

@end
