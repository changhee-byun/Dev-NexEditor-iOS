/**
 * File Name   : InternalAssetLibrary+AssetLibraryProxy.m
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

#import "InternalAssetLibrary+AssetLibraryProxy.h"
#import "NXEAssetItemPrivate.h"
#import "AssetResourceLoader.h"

@implementation InternalAssetLibrary(AssetLibraryProxy)

- (NSData *) loadResourceDataAtSubpath: (NSString *) subpath forItemId:(NSString *) itemId
{
    NSData *result = nil;
    NXEAssetItem *item = [self itemForId:itemId];
    if ( item ) {
        if ( subpath ) {
            result = [item dataForResourceFile:subpath];
        }
    }
    return result;
}

- (NSString *) buildThemeDataWithItems:(NSArray<id<AssetItem>> *) items
{
    NSMutableString *result = [NSMutableString stringWithFormat:@"%@%@%@",
                               @"<themeset name=\"KM\" defaultTheme=\"none\" defaultTransition=\"none\" >",
                               @"<texture id=\"video_out\" video=\"1\" />",
                               @"<texture id=\"video_in\" video=\"2\" />"];
    
    for(NXEAssetItem *item in items) {
        if ( [item isKindOfClass:NXEAssetItem.class] && item.raw.type == AssetItemTypeKedl) {
            NSData *data = item.fileInfoResourceData;
            if (data) {
                NSString *string = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
                if ( string ) {
                    [result appendString:string];
                }
            }
        }
    }
    
    [result appendString:@"</themeset>"];
    return result;
}

- (NSString *) buildRenderItemDataWithItem:(AssetItem *)item
{
    NXEAssetItem *assetItem = (NXEAssetItem *)item;
    NSString *result = nil;
    
    if ( [assetItem isKindOfClass:NXEAssetItem.class] && assetItem.raw.type == AssetItemTypeRenderitem ) {
        NSData *data = assetItem.fileInfoResourceData;
        if (data) {
            result = [[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease];
        }
    }
    
    return result;
}

@end
