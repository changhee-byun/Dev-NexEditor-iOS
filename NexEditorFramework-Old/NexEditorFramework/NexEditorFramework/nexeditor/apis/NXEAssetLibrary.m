/**
 * File Name   : NXEAssetLibrary.m
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

#import "NXEAssetLibrary.h"
#import "AssetReader.h"
#import "InternalAssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import "NXEAssetItemGroupPrivate.h"
#import "AssetLibrary.h"

// category=overlay, type=nex.template
static NSString *const kNXEAssetItemCategoryTextEffect = @"textEffect";

@interface NXEAssetLibrary()
@property (nonatomic, strong) InternalAssetLibrary *impl;
@end

@implementation NXEAssetItemCategory
+ (NSString *) Template
{
    return kAssetItemCategoryTemplate;
}

+ (NSString *) transition
{
    return kAssetItemCategoryTransition;
}

+ (NSString *) effect
{
    return kAssetItemCategoryEffect;
}

+ (NSString *) textEffect
{
    return kNXEAssetItemCategoryTextEffect;
}

+ (NSString *) collage
{
    return kAssetItemCategoryCollage;
}

+ (NSString *) beatTemplate
{
    return kAssetItemCategoryBeatTemplate;
}

@end

@implementation NXEAssetLibrary
- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.impl = [[[InternalAssetLibrary alloc] init] autorelease];
    }
    return self;
}

- (void) dealloc
{
    self.impl = nil;
    [super dealloc];
}

#pragma mark - NXEAssetLibrary protocol
- (NXEAssetItem *) itemForId:(NSString *) itemId
{
    return [self.impl itemForId:itemId includeHidden:NO];
}

- (NSArray<NXEAssetItem *> *) itemsInCategory:(NSString *) category
{
    NSString *lowlevelCategory = category;
    AssetItemType filter = AssetItemTypeNone;
    if ([kNXEAssetItemCategoryTextEffect isEqualToString:category]) {
        lowlevelCategory = kAssetItemCategoryOverlay;
        filter = AssetItemTypeTemplate;
    }
    
    NSArray<NXEAssetItem *> *result = [self.impl itemsInCategory:lowlevelCategory includeHidden:NO];
    
    if ( filter != AssetItemTypeNone ) {
        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"raw.type == %@", @(AssetItemTypeTemplate)];
        result = [result filteredArrayUsingPredicate:predicate];
    }
    return result;
}

- (void) itemsInCategory:(NSString * _Nonnull) category completion:(void (^ _Nonnull)(NSArray<NXEAssetItem *> * _Nonnull)) completion
{
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        NSArray<NXEAssetItem *> *result = [self itemsInCategory:category];
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(result);
        });
    });
}

- (NSArray<NXEAssetItemGroup *> * _Nonnull) groupsInCategory:(NSString * _Nonnull) category
{
    NSArray<NXEAssetItem *> *items = [self itemsInCategory:category];
    NSMutableDictionary *map = [NSMutableDictionary dictionary];
    for( NXEAssetItem *item in items ) {
        NSMutableArray *items = map[item.raw.packageId];
        if (items == nil) {
            items = [NSMutableArray array];
            map[item.raw.packageId] = items;
        }
        [items addObject:item];
    }
    
    NSMutableArray *result = [NSMutableArray array];
    for (NSMutableArray *items in map.allValues) {
        NXEAssetItemGroup *group = [NXEAssetItemGroup groupWithItems:[[items copy] autorelease]];
        [result addObject:group];
    }
    
    return [[result copy] autorelease];
}


#pragma mark - Static API
+ (instancetype) instance
{
    return [[[self.class alloc] init] autorelease];
}

+ (void) addAssetSourceDirectoryURL:(NSURL * _Nonnull) fileURL
{
    /**
     * fileURL should be a URL to a file system directory.
     * Internally, asset-volume URL scheme is also supported but not for App use.
     * - asset-volume://bundle/<bundleName>/packages
     * - asset-volume://documents/nexeditor/packages
     * - See AssetVolume for details
     */
    [((NXEAssetLibrary *)self.instance).impl addAssetSourceURL:fileURL];
}

+ (void) addAssetPackageDirectoryURLs:(NSArray <NSURL *> * _Nonnull) fileURLs
{
    [((NXEAssetLibrary *)self.instance).impl addAssetPackageURLs:fileURLs];
}

+ (void) removeAssetPackageDirectoryURLs:(NSArray <NSURL *> * _Nonnull) fileURLs
{
    [((NXEAssetLibrary *)self.instance).impl removeAssetPackageURLs:fileURLs];
}


+ (void) reloadSources
{
    [((NXEAssetLibrary *)self.instance).impl reloadSources];
}

+ (void) removeAllAssetPackageDirectoryURLs
{
    [((NXEAssetLibrary *)self.instance).impl removeAllAssetPackageURLs];
}

@end

