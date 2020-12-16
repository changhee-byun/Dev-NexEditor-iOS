/*
 * File Name   : NXEBeatAssetItem.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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
#import "NXEBeatAssetItem.h"
#import "NXEBeatAssetItemPrivate.h"
#import "NXEAssetItemPrivate.h"


@interface NXEBeatAssetItem()
@property (nonatomic, strong) NXEBeatAssetItemInfo* info;
@end

@implementation NXEBeatAssetItem

- (id) initWithRawItem:(AssetItem *) raw
{
    return [super initWithRawItem:raw];
}

-(NXEBeatAssetItemInfo *) info {
    if ( _info == nil ) {
        NSData *data = self.fileInfoResourceData;
        if ( data ) {
            _info = [[NXEBeatAssetItemInfo alloc] initWithData:data];
        }
    }
    return _info;
}

- (void)dealloc
{
    if ( _info ) {
        [_info release];
    }
    [super dealloc];
}


@end

