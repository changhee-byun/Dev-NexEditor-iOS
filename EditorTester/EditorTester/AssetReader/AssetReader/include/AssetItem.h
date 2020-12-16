/**
 * File Name   : AssetItem.h
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
#import "AssetItemInfo.h"
#import "AssetPackage.h"

typedef NS_ENUM(NSUInteger, AssetItemInfoResourceType) {
    AssetItemInfoResourceTypeFilename = 0,
    AssetItemInfoResourceTypeIcon,
};

typedef NS_ENUM(NSInteger, AssetItemType) {
    AssetItemTypeNone       = -1,
    AssetItemTypeOverlay     =  0,
    AssetItemTypeRenderitem,
    AssetItemTypeKedl,
    AssetItemTypeAudio,
    AssetItemTypeFont,
    AssetItemTypeTemplate,
    AssetItemTypeLut,
    AssetItemTypeMedia,
    AssetItemTypeCollage,
    AssetItemTypeBeatTemplate
};

@protocol AssetItem <NSObject>
@property (nonatomic, readonly) NSString *itemId;
@end

@interface AssetItem : NSObject <AssetItem>
@property (nonatomic, readonly) AssetPackage *package;
@property (nonatomic, readonly) NSString *packageId;
/// _info.json->id or <packageId>.<category>.<name>
@property (nonatomic, readonly) NSString *itemId;
/// [path lastPathComponent]
@property (nonatomic, readonly) NSString *name;
@property (nonatomic, readonly) NSString *category;
/// filesystem path to directory of the item where _info.json is located
@property (nonatomic, readonly) NSString *path;
@property (nonatomic, readonly) AssetItemInfo *info;

- (NSString *) pathForInfoResourceType:(AssetItemInfoResourceType) resourceType;
- (NSString *) pathForResourceFile:(NSString *) filename;

/// YES if the file at 'path' (full-path) should is encrypted
- (BOOL) isEncryptedResourceAtPath:(NSString *) path;
@end

@interface AssetItem(InfoAttributes)
@property (nonatomic, readonly) NSString *label;    // The english version of label[] in _info.json
                                                    // ISO 639-1 code -> label text. Should be compatible with NSLocale.languageCode.
@property (nonatomic, readonly) NSDictionary<NSString *, NSString *> *labels;
@property (nonatomic, readonly) AssetItemType type;
@property (nonatomic, readonly) BOOL hidden;
@end

