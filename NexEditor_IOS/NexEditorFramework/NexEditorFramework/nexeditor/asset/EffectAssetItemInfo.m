/**
 * File Name   : EffectAssetItemInfo.m
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

#import "EffectAssetItemInfo.h"

@interface EffectAssetItemInfo()
@property (nonatomic, strong) NSArray<UserField *> * _Nullable userFields;
@property (nonatomic, retain) NSMutableArray * _Nullable userFieldStore;
@end

@implementation EffectAssetItemInfo
- (instancetype) init
{
    self = [super init];
    if ( self ) {
        self.userFieldStore = [NSMutableArray array];
    }
    return self;
}

- (void) dealloc
{
    self.userFieldStore = nil;
    self.userFields = nil;
    [super dealloc];
}

#pragma mark -
- (NSArray *) userFields
{
    if ( _userFields == nil ) {
        _userFields = [self.userFieldStore copy];
    }
    return _userFields;
}

#pragma mark - AssetXMLParserClient

- (void)parseBeginElementWithName:(NSString * _Nonnull)elementName attributes:(NSDictionary<NSString *,NSString *> * _Nonnull)attributes
{
    if([elementName isEqualToString:@"userfield"]) {
        UserField *userField = [[[UserField alloc] initUserFieldWithAttributes:attributes] autorelease];
        if(![self.userFieldStore containsObject:userField]) {
            [self.userFieldStore addObject:userField];
        }
    } else if([elementName isEqualToString:@"option"]) {
        if (self.userFieldStore.count > 0) {
            UserField *lastField = self.userFieldStore[self.userFieldStore.count -1];
            [lastField setFieldOption:attributes];
        }
    }
}

@end
