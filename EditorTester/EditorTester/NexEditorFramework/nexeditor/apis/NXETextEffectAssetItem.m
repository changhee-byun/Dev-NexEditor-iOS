/**
 * File Name   : NXETextEffectAssetItem.m
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

#import "NXETextEffectAssetItem.h"
#import "NXEAssetItemPrivate.h"
#import "AssetLibrary.h"
#import "OverlayTitleInfo.h"

@interface NXETextEffectAssetItemInfo : NSObject
@property (nonatomic, readonly) NSString *version;
@end

@interface NXETextEffectAssetItemInfo ()
@property (nonatomic, retain) NSString *name;
@property (nonatomic, retain) NSString *desc;

- (instancetype)initWithData:(NSData *)data;
@end

@interface NXETextEffectAssetItem ()
@property (nonatomic, retain) NXETextEffectAssetItemInfo* info;
@end

@implementation NXETextEffectAssetItem

- (id) initWithRawItem:(AssetItem *) rawItem
{
    self = [super initWithRawItem:rawItem];
    if ( self ) {
    
    }
    return self;
}

- (void)dealloc
{
    self.info = nil;
    [super dealloc];
}

- (NSString *) name
{
    return self.info.name;
}

- (NSString *) desc
{
    // not available for all item types
    return self.info.desc;
}

- (NSString *) category
{
    return NXEAssetItemCategory.textEffect;
}

- (NXETextEffectAssetItemInfo *)info
{
    if( _info == nil ) {
        NSData *infoData = self.fileInfoResourceData;
        _info = [[NXETextEffectAssetItemInfo alloc] initWithData:infoData];
    }
    return _info;
}

@end

@implementation NXETextEffectAssetItemInfo

- (void)dealloc
{
    self.name = nil;
    self.desc = nil;
    [super dealloc];
}

- (instancetype)initWithData:(NSData *)data
{
    self = [super init];
    if(self) {
        if ( data ) {
            OverlayTitleInfo *titleInfo = [[[OverlayTitleInfo alloc] initWithJSONData:data nxeError:nil] autorelease];
            self.name = titleInfo.name;
            self.desc = titleInfo.desc;
        }
    }
    return self;
}

@end

