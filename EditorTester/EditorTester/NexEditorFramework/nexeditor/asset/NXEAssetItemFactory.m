/**
 * File Name   : NXEAssetItemFactory.m
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

#import "NXEAssetItemFactory.h"
#import "NXEAssetItemPrivate.h"
#import "NXETemplateAssetItem.h"
#import "NXEEffectAssetItem.h"
#import "NXETransitionAssetItem.h"
#import "NXETextEffectAssetItem.h"
#import "NXECollageAssetItem.h"
#import "NXEBeatAssetItem.h"

@interface AssetItem(Supported)
@property (nonatomic, readonly) BOOL isSupportedType;
@end

@implementation NXEAssetItemFactory

+ (NXEAssetItem *) itemWithRawItem:(AssetItem *)rawItem
{
    
    NXEAssetItem *item = nil;
    
    if ( rawItem.isSupportedType ) {
        // fallback default class
        Class cls = [NXEAssetItem class];
        
        if ( rawItem.type == AssetItemTypeTemplate ) {
            if([kAssetItemCategoryTemplate isEqualToString:rawItem.category]) {
                cls = [NXETemplateAssetItem class];
            } else if([kAssetItemCategoryOverlay isEqualToString:rawItem.category]) {
                cls = [NXETextEffectAssetItem class];
            } else if([kAssetItemCategoryBeatTemplate isEqualToString:rawItem.category]) {
                cls = [NXEBeatAssetItem class];
            }
        } else if ( rawItem.type == AssetItemTypeCollage ) {
            cls = [NXECollageAssetItem class];
        } else if ([kAssetItemCategoryEffect isEqualToString:rawItem.category]
                   && (rawItem.type == AssetItemTypeKedl || rawItem.type == AssetItemTypeRenderitem) ) {
            cls = [NXEEffectAssetItem class];
        } else if ([kAssetItemCategoryTransition isEqualToString:rawItem.category]
                   && (rawItem.type == AssetItemTypeKedl || rawItem.type == AssetItemTypeRenderitem) ) {
            cls = [NXETransitionAssetItem class];
        }
        
        if ( cls != nil ) {
            item = [[[cls alloc] initWithRawItem:rawItem] autorelease];
        }
    }
    return item;
}

@end

@implementation AssetItem(Supported)

- (BOOL) isSupportedType
{
    static NSArray *_supportedTypes = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _supportedTypes = @[
                            @(AssetItemTypeRenderitem),
                            @(AssetItemTypeKedl),
                            @(AssetItemTypeAudio),
                            @(AssetItemTypeFont),
                            @(AssetItemTypeTemplate),
                            @(AssetItemTypeOverlay),
                            @(AssetItemTypeLut),
                            @(AssetItemTypeMedia),
                            @(AssetItemTypeCollage),
                            @(AssetItemTypeBeatTemplate)
                            ];
        [_supportedTypes retain];
    });

    return [_supportedTypes indexOfObject:@(self.type)] != NSNotFound;
}

@end
