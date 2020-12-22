/*
 * File Name   : NXECollageAssetItem.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "NXECollageAssetItem.h"
#import "CollageInfoInternal.h"
#import "CollageData.h"
#import "NXEAssetItemPrivate.h"
#import <NexEditorEngine/NexEditorEngine.h>

@interface NXECollageAssetItem()
@property (nonatomic, strong) CollageInfo *info;
@end

@implementation NXECollageAssetItem

- (void) dealloc
{
    self.info = nil;
    [super dealloc];
}

- (CollageInfo *) info
{
    if ( _info == nil ) {
        NSData *data = [self fileInfoResourceData];
        NSError *error = nil;
        if (data) {
            CollageData *raw = [[[CollageData alloc] initWithData:data error:&error] autorelease];
            if ( raw != nil) {
                _info = [[CollageInfo alloc] initWithData:raw];
            }
        }
        if (_info == nil) {
            NSString *message = nil;
            if (error) {
                message = error.localizedDescription;
            } else {
                message = [NSString stringWithFormat:@"Cannot load file resource for item id:%@", self.itemId];
            }
            NexLogE(@"ASSET", @"%@", message);
        }
    }
    return _info;
}
@end
