/**
 * File Name   : NXEEffectAssetItem.m
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

#import "NXEEffectAssetItem.h"
#import "NXEEffectAssetItemPrivate.h"
#import "AssetItem.h"
#import "NXEAssetItemPrivate.h"
#import "AssetResourceLoader.h"
#import "EffectAssetItemInfo.h"

@interface NXEEffectAssetItem()
@property (nonatomic, strong) EffectAssetItemInfo *effectItemInfo;
@end

@implementation NXEEffectAssetItem

- (id) initWithRawItem:(AssetItem *) raw
{
    self = [super initWithRawItem:raw];
    if ( self ) {
    }
    return self;
}

- (void) dealloc
{
    self.effectItemInfo = nil;
    [super dealloc];
}

#pragma mark -
- (EffectAssetItemInfo *) effectItemInfo
{
    if ( _effectItemInfo == nil ) {
        NSData *data = [self fileInfoResourceData];
        if ( data ) {
            EffectAssetItemInfo *info = [[EffectAssetItemInfo alloc] init];
            AssetXMLParser *parser = [AssetXMLParser parser];
            [parser parseWithData:data client:info];
            _effectItemInfo = info;
        }
    }
    return _effectItemInfo;
}

@end

@implementation NXEEffectAssetItem(Private)

#pragma mark - ConfigurableAssetItem

- (NSArray<UserField *> *) userFields
{
    return self.effectItemInfo.userFields;
}
@end

