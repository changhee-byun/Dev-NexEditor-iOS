/**
 * File Name   : NXEAssetItem.m
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

#import "NXEAssetItem.h"
#import "NXEAssetItemPrivate.h"
// AssetReader
#import "AssetReader.h"
#import "AssetResourceLoader.h"

@interface NXEAssetItem()
@property (nonatomic, strong) AssetItem *raw;
@property (nonatomic, strong) NSData *fileInfoResourceData;

@end

@implementation NXEAssetItem

- (id) initWithRawItem:(AssetItem *) raw
{
    self = [super init];
    if ( self ) {
        self.raw = raw;
    }
    return self;
}
- (void) dealloc
{
    self.raw = nil;
    self.fileInfoResourceData = nil;
    [super dealloc];
}

- (NSString *) itemId
{
    return self.raw.itemId;
}

- (NSString *) name
{
    return self.raw.name;
}

- (NSString *) category
{
    return self.raw.category;
}

- (NSString *) desc
{
    // not available for all item types
    return nil;
}

- (NXEAspectType) aspectType
{
    return [self.itemId detectAspectType];
}

#pragma mark - Private
- (NSString *) fileInfoResourcePath
{
    return [self.raw pathForInfoResourceType:AssetItemInfoResourceTypeFilename];
}

- (NSString *) iconInfoResourcePath
{
    return [self.raw pathForInfoResourceType:AssetItemInfoResourceTypeIcon];
}

- (BOOL) shouldDecryptResource:(AssetItemInfoResourceType) resourceType
{
    NSString *resourcePath = [self.raw pathForInfoResourceType:resourceType];
    return [self.raw isEncryptedResourceAtPath:resourcePath];
}

- (NSData *) fileInfoResourceData
{
    if ( _fileInfoResourceData == nil ) {
        NSString *path = self.fileInfoResourcePath;
        if ( path ) {
            BOOL shouldDecrypt = [self shouldDecryptResource:AssetItemInfoResourceTypeFilename];
            _fileInfoResourceData = [[AssetResourceLoader loadFileContentsAtPath:path
                                                                         decrypt:shouldDecrypt
                                                                        recipient:self.raw.package.cryptrecp
                                      ] retain];
        }
    }
    return _fileInfoResourceData;
}



#pragma mark -
- (NSString *) label
{
    return self.raw.label;
}

- (BOOL) hasIcon
{
    return self.iconInfoResourcePath != nil;
}

- (void) loadIconImageData:( void (^)(NSData * data, NSString *extension)) block
{
    if ( block == NULL ) return;
    
    BOOL shoudDecrypt = [self shouldDecryptResource:AssetItemInfoResourceTypeIcon];
    NSString *path = self.iconInfoResourcePath;
    if ( path ) {
        NSString *extension = [[path pathExtension] lowercaseString];
        [[NSOperationQueue mainQueue] addOperationWithBlock:^{
            NSData *data = [AssetResourceLoader loadFileContentsAtPath:path
                                                               decrypt:shoudDecrypt
                                                             recipient:self.raw.package.cryptrecp];
            block(data, extension);
        }];
    } else {
        block(nil, nil);
    }
}

- (NSData *) dataForResourceFile:(NSString *) subpath
{
    NSString *path = [self.raw pathForResourceFile:subpath];
    return [AssetResourceLoader loadFileContentsAtPath:path
                                               decrypt:[self.raw isEncryptedResourceAtPath:path]
                                             recipient:self.raw.package.cryptrecp
            ];
}

@end

#pragma mark -


@implementation AssetItem(Resource)


@end

@implementation NSString(AspectType)
- (NXEAspectType) aspectType
{
    NXEAspectType result = NXEAspectTypeUnknown;
    if ( [self isEqualToString:@"16v9"]) {
        result = NXEAspectTypeRatio16v9;
    } else if ( [self isEqualToString:@"9v16"]) {
        result = NXEAspectTypeRatio9v16;
    } else if ( [self isEqualToString:@"1v1"]) {
        result = NXEAspectTypeRatio1v1;
    } else if ( [self isEqualToString:@"1v2"]) {
        result = NXEAspectTypeRatio1v2;
    } else if ( [self isEqualToString:@"2v1"]) {
        result = NXEAspectTypeRatio2v1;
    }
    return result;
}

- (NXEAspectType) detectAspectType
{
    NXEAspectType result = NXEAspectTypeUnknown;
    NSArray *components = [self componentsSeparatedByString:@"."];
    for (NSString *component in [components reverseObjectEnumerator]) {
        result = component.aspectType;
        if ( result != NXEAspectTypeUnknown ) {
            break;
        }
    }
    return result;
}

+ (NXEAspectType) detectAspectTypeFrom:(NSString *) text
{
    NXEAspectType result = NXEAspectTypeUnknown;
    if ( text ) {
        result = text.detectAspectType;
    }
    return result;
}

@end

