/**
 * File Name   : InternalAssetLibrary.m
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

#import "InternalAssetLibrary.h"
#import "AssetReader.h"
#import "NXEAssetItemPrivate.h"
#import "NXEAssetItemFactory.h"
#import "AssetResourceLoader.h"
#import "AssetResourceDecrypter.h"

@interface NXEAssetLibraryOptions: NSObject
@property (nonatomic, copy) NSArray<NSURL *> *packageURLs;
@property (nonatomic, strong) id<AssetDB> db;
@end

@interface InternalAssetLibrary()
@property (nonatomic, readonly) id<AssetDB> db;
@property (class, readonly) NXEAssetLibraryOptions * _Nonnull options;
@end

@implementation InternalAssetLibrary
- (instancetype) init
{
    self = [super init];
    if ( self ) {
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

#pragma mark -

- (id<AssetDB>) db
{
    return self.class.options.db;
}

#pragma mark -
- (NSArray<NXEAssetItem *> *) itemsInCategory:(NSString *) category includeHidden:(BOOL) includeHidden
{
    NSMutableArray<NXEAssetItem *> *result = [NSMutableArray array];
    
    NSArray<AssetItem *> *rawItems = [self.db itemsForCategory:category];
    
    for( AssetItem *rawItem in rawItems ) {
        if ( rawItem.hidden && !includeHidden ) {
            continue;
        }
        NXEAssetItem *item = [NXEAssetItemFactory itemWithRawItem:rawItem];
        if ( item ) {
            [result addObject:item];
        }
    }
    
    return [[result copy] autorelease];
}

- (NXEAssetItem *) itemForId:(NSString *) itemId includeHidden:(BOOL) includeHidden
{
    NXEAssetItem *result = nil;
    AssetItem *item = [self.db itemForId:itemId];
    if ( item ) {
        BOOL include = includeHidden || !item.hidden;
        if ( include ) {
            result = [NXEAssetItemFactory itemWithRawItem:item];
        }
    }
    return result;
}

#pragma mark -
- (NSArray<NXEAssetItem *> *) itemsInCategory:(NSString *) category
{
    return [self itemsInCategory:category includeHidden:YES];
}

- (NXEAssetItem *) itemForId:(NSString *) itemId
{
    return [self itemForId:itemId includeHidden:YES];
}

- (void) addAssetSourceURL:(NSURL *)url
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSArray *packageURLs = [fm contentsOfDirectoryAtURL:url
                             includingPropertiesForKeys:@[NSURLIsDirectoryKey]
                                                options:NSDirectoryEnumerationSkipsHiddenFiles error:nil];
    [self addAssetPackageURLs:packageURLs];
}

- (void) addAssetPackageURLs:(NSArray<NSURL *> *) urls
{
    
    NSArray *packageURLs = self.class.options.packageURLs;
    NSMutableArray *newURLs = [NSMutableArray array];

    for( NSURL * url in urls) {
        if ( [packageURLs indexOfObject:url] == NSNotFound ) {
            [self.db addPackageURL:url];
            [newURLs addObject:url];
        }
    }
    self.class.options.packageURLs = [packageURLs arrayByAddingObjectsFromArray:newURLs];
}

- (void) removeAssetPackageURLs:(NSArray <NSURL *> *) urls
{
    NSMutableArray *packageURLs = [self.class.options.packageURLs mutableCopy];
    for( NSURL * url in urls) {
        if ( [packageURLs indexOfObject:url] != NSNotFound ) {
            [self.db removePackageAtURL:url];
        }
    }
    [packageURLs removeObjectsInArray:urls];
    self.class.options.packageURLs = packageURLs;
}

- (void) removeAllAssetPackageURLs
{
    self.class.options.packageURLs = @[];
    self.class.options.db = nil;
    
    // Trigger indexing
    [self.class.options db];
}

- (void) reloadSources
{
    // Next time access to 'db' property will re-index all the source directories
    self.class.options.db = nil;
    
    // Trigger indexing
    [self.class.options db];
}

#pragma mark - Class properties
+ (NXEAssetLibraryOptions *) options
{
    static NXEAssetLibraryOptions *options = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        options = [[NXEAssetLibraryOptions alloc] init];
    });
    return options;
}

+ (instancetype) library
{
    return [[[self.class alloc] init] autorelease];
}

@end



#pragma mark - NXEAssetLibraryOptions

@interface NXEAssetLibraryOptions()
@property (nonatomic, strong) AssetResourceDecrypter *decrypter;
@end

@implementation NXEAssetLibraryOptions

- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.packageURLs = @[];
    }
    return self;
}

- (void) dealloc
{
    self.decrypter = nil;
    self.packageURLs = nil;
    self.db = nil;
    [super dealloc];
}

#pragma mark - Properties

- (AssetResourceDecrypter *) decrypter
{
    if ( _decrypter == nil ) {
        _decrypter = [[AssetResourceDecrypter alloc] init];
    }
    return _decrypter;
}

- (id<AssetDB>) db
{
    if ( _db == nil ) {
        _db = [[AssetDB assetDBWithSourceURLs:@[]] retain];
        _db.decryptionDelegate = self.decrypter;
        for(NSURL *packageURL in self.packageURLs) {
            [_db addPackageURL:packageURL];
        }
    }
    return _db;
}

- (void) setPackageURLs:(NSArray<NSURL *> *)urls
{
    [_packageURLs release];
    _packageURLs = [urls copy];
}
@end

