/*
 * File Name   : NXEAssetItemGroup.m
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
#import "NXEAssetItemGroup.h"
#import "NXEAssetItemGroupPrivate.h"
#import "NXEAssetItemPrivate.h"

@interface NXEAssetItemGroup()
@property (nonatomic, strong) NSArray<NXEAssetItem *> *items;
@end

@implementation NXEAssetItemGroup

- (instancetype) initWithItems:(NSArray<NXEAssetItem *> *) items
{
    self = [super init];
    if ( self ) {
        self.items = items;
    }
    return self;
}

- (void) dealloc {
    self.items = nil;
    [super dealloc];
}

#pragma mark - Properties
- (NSString *) localizedName
{
    NSString *result = nil;
    NXEAssetItem *firstItem = self.items.firstObject;
    if (firstItem) {
        NSDictionary *assetNames = firstItem.raw.package.assetNames;
        result = assetNames[@"en"];
    }
    return result;
}

- (NSString *) category
{
    return self.items.firstObject.category;
}

- (BOOL) hasIcon
{
    BOOL result = NO;
    NXEAssetItem *firstItem = self.items.firstObject;
    if (firstItem) {
        result = firstItem.hasIcon;
    }
    return result;
}
#pragma mark - API
- (void) loadIconImageData:( void (^)(NSData * data, NSString *extension)) block
{
    if (self.items.count > 0) {
        [self.items[0] loadIconImageData:block];
    } else {
        block(nil, nil);
    }
}

+ (instancetype) groupWithItems:(NSArray<NXEAssetItem *> *) items
{
    return [[[NXEAssetItemGroup alloc] initWithItems:items] autorelease];
}

@end
