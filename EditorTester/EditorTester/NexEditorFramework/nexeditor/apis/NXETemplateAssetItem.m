/**
 * File Name   : NXETemplateAssetItem.m
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

#import "NXETemplateAssetItem.h"
#import "NXEAssetItemPrivate.h"
#import "AssetReader.h"
#import "AssetResourceLoader.h"

/// Hidden inside until it's necessary to be exposed as API
@interface NXETemplateAssetItemInfo: NSObject
@property (nonatomic, strong) NSString *name;
@property (nonatomic, strong) NSString *desc;
@property (nonatomic) NXEAspectType aspectType;

@property (nonatomic, strong) NSDictionary *source;
@property (nonatomic, readonly) float bgmVolume;

- (id) initWithData:(NSData *) data;

@end

@interface NXETemplateAssetItem()
@property (nonatomic, strong) NXETemplateAssetItemInfo *info;
@end


@implementation NXETemplateAssetItem

- (id) initWithRawItem:(AssetItem *) rawItem
{
    self = [super initWithRawItem:rawItem];
    if ( self ) {
        
    }
    return self;
}

- (void) dealloc
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
    return self.info.desc;
}

- (NXEAspectType) aspectType
{
    return self.info.aspectType;
}

-(NXETemplateAssetItemInfo *) info
{
    if ( _info == nil ) {
        NSData *data = self.fileInfoResourceData;
        if ( data ) {
            _info = [[NXETemplateAssetItemInfo alloc] initWithData:data];
            if (_info.source == nil) {
            }
        }
        if (_info == nil || _info.source == nil) {
            NSLog(@"Bad template format at path:%@", self.fileInfoResourcePath);
        }
    }
    return _info;
}

@end


@implementation NXETemplateAssetItemInfo
#define kTemplateFieldKeyBGMVolume @"template_bgm_volume"
- (id) initWithData:(NSData *) data
{
    self = [super init];
    if ( self ) {
        NSDictionary *source = [NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
        if (source && [source isKindOfClass:[NSDictionary class]]) {
            self.name = source[@"template_name"];
            self.desc = source[@"template_desc"];
            self.aspectType = [((NSString *)source[@"template_mode"]) aspectType];
            self.source = source;
        }
    }
    return self;
}

- (void) dealloc
{
    self.name = nil;
    self.desc = nil;
    self.source = nil;
    [super dealloc];
}

- (float) bgmVolume
{
    NSString *value = self.source[kTemplateFieldKeyBGMVolume];
    float result = 0.5;
    if ( value ) {
        result = [value floatValue];
    }
    return result;
}

@end
