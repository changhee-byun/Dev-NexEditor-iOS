/**
 * File Name   : AssetItem.m
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

#import "AssetItem.h"
#import "AssetItemPrivate.h"
#import "AssetItemInfo.h"
#import "AssetItemInfoPrivate.h"
#import "AssetItemPathInfo.h"
#import "AssetImplicitRules.h"

#define kDefaultLanguageCode kISO6391LanguageCodeEnglish

static NSString *const kISO6391LanguageCodeEnglish = @"en";

@interface AssetItem()
@property (nonatomic, strong) AssetPackage *package;

@property (nonatomic) NSString *itemId;
@property (nonatomic) NSString *name;
@property (nonatomic) NSString *category;
/// filesystem path to directory of the item where _info.json is located
@property (nonatomic) NSString *path;

@property (nonatomic) AssetItemInfo *info;
@property (nonatomic, readonly, strong) NSArray * resourceSearchPaths;

@end

@implementation AssetItem
@synthesize resourceSearchPaths = _resourceSearchPaths;

#pragma mark - Private
- (id) initWithPath:(NSString *) path package:(AssetPackage *) package category:(NSString *) category infoData:(NSData *) infoData
{
    self = [super init];
    if ( self ) {
        self.path = path;
        self.info = [[AssetItemInfo alloc] initWithData:infoData];
        if ( [self.path pathComponents].count == 0 ) {
            NSLog(@"ERROR: Invalid Asset Item Path:%@", path);
        }
        self.name = [self.path lastPathComponent];
        self.package = package;
        self.category = category;
        
    }
    return self;
}

#pragma mark - Properties

- (NSString *) packageId
{
    return self.package.packageId;
}

- (NSString *) itemId
{
    if ( _itemId == nil ) {
        if ( self.info ) {
            _itemId = [self.info valueForInfoField:AssetItemInfoFieldId];
        }
        
        if ( _itemId == nil ) {
            // Implicit item id for package asset: <packageId>.item.<name>
            if ( self.packageId != nil && self.name != nil ) {
                _itemId = [AssetImplicitRules implicitItemIdWithPackageId:self.packageId itemDirName:self.name];
            }
        }
    }
    return _itemId;
}

#pragma mark -

/// \return nil if the resource is not not available, info is not available, or failed to resolve the full path
- (NSString *) pathForInfoResourceType:(AssetItemInfoResourceType) resourceType
{
    NSString *result = nil;
    
    AssetItemInfoField field = [self.class infoFieldForResourceType: resourceType];
    if ( field != AssetItemInfoFieldNone && self.info ) {
        NSString *filename = [self.info valueForInfoField:field];
        if (filename) {
            result = [self pathForResourceFile:filename];
        } // no field
    }
    
    return result;
}

- (NSArray *) resourceSearchPaths
{
    if ( _resourceSearchPaths == nil ) {
        NSMutableArray *paths = [NSMutableArray array];
        [paths addObject:self.path];
        NSArray *mergePaths = [self.info valueForInfoField:AssetItemInfoFieldMergePaths];
        for (NSString *mergePath in mergePaths) {
            [paths addObject:[self.package.mergePath stringByAppendingPathComponent:mergePath]];
        }
        _resourceSearchPaths = [paths copy];
    }

    return _resourceSearchPaths;
}

- (NSString *) pathForResourceFile:(NSString *) filename
{
    NSString *result = nil;
    NSFileManager *fm = [NSFileManager defaultManager];
    for (NSString *path in self.resourceSearchPaths) {
        NSString *filePath = [path stringByAppendingPathComponent:filename];
        if ([fm fileExistsAtPath:filePath]) {
            result = filePath;
            break;
        }
    }
    return result;
}

- (BOOL) isEncryptedResourceAtPath:(NSString *) path
{
    BOOL result = NO;
    
    NSString *subpath = [AssetItemPathInfo subpathOf:path under:self.package.path];
    if (subpath) {
        result = [self.package isEncrypted:subpath];
    }
    return result;
}

+ (AssetItemInfoField) infoFieldForResourceType:(AssetItemInfoResourceType) resourceType
{
    static AssetItemInfoField _fields[] = {
        AssetItemInfoFieldFilename,     // AssetItemInfoResourceTypeFilename
        AssetItemInfoFieldIcon,         // AssetItemInfoResourceTypeIcon
    };
    return _fields[resourceType];
}

@end

@implementation AssetItem(InfoAttributes)

- (NSString *) label
{
    NSString *result = self.labels[kDefaultLanguageCode];
    if ( result == nil ) {
        // fallback to randomly chosen language code by JSONSerialization
        // Suggestion for fallback default is welcome
        if (self.labels.allKeys.count > 0) {
            result = self.labels[self.labels.allKeys[0]];
        }
    }
    return result;
}

/// If not found, empty dictionary is returned
- (NSDictionary<NSString *, NSString *> *) labels
{
    NSDictionary *result = [self.info valueForInfoField:AssetItemInfoFieldLabel];
    if ( result && [result isKindOfClass:[NSDictionary class]]) {
        return result;
    } else {
        return @{};
    }
    return result;
}

- (AssetItemType) type
{
    AssetItemType result = AssetItemTypeNone;
    
    NSString *typeValue = [self.info valueForInfoField:AssetItemInfoFieldType];
    if ( typeValue && [typeValue isKindOfClass:[NSString class]]) {
        result = [self.class itemTypeFromString:typeValue];
    }
    return result;
}

+ (AssetItemType) itemTypeFromString:(NSString *) typeString
{
    static dispatch_once_t onceToken;
    static NSDictionary *itemTypes = nil;
    dispatch_once(&onceToken, ^{
        itemTypes = @{
                      @"nex.overlay": @(AssetItemTypeOverlay),
                      @"nex.renderitem": @(AssetItemTypeRenderitem),
                      @"nex.kedl": @(AssetItemTypeKedl),
                      @"nex.audio": @(AssetItemTypeAudio),
                      @"nex.font": @(AssetItemTypeFont),
                      @"nex.template": @(AssetItemTypeTemplate),
                      @"nex.lut": @(AssetItemTypeLut),
                      @"nex.media": @(AssetItemTypeMedia),
                      @"nex.collage":@(AssetItemTypeCollage)
                      };
    });
    
    AssetItemType result = AssetItemTypeNone;
    NSNumber *value = itemTypes[typeString];
    if ( value ) {
        result = (AssetItemType)[value unsignedIntegerValue];
    }
    return result;
}

- (BOOL) hidden
{
    BOOL result = false;
    id value = [self.info valueForInfoField:AssetItemInfoFieldHidden];
    if ( value ) {
        if ( [value isKindOfClass:NSNumber.class] || [value isKindOfClass:NSString.class]) {
            result = [value boolValue];
        }
    }
    
    return result;
}

@end

