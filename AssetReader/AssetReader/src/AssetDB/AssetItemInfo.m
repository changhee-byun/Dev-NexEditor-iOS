/**
 * File Name   : AssetItemInfo.m
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

#import "AssetItemInfo.h"

NSString *const kAssetItemInfoFieldNone = @"__none__";

static NSString *const _infoFieldNames[] = {
    @"id",          // AssetItemInfoFieldId
    @"type",        // AssetItemInfoFieldType
    @"hidden",      // AssetItemInfoFieldHidden
    @"label",       // AssetItemInfoFieldLabel
    @"icon",        // AssetItemInfoFieldIcon
    @"filename",    // AssetItemInfoFieldFilename
    @"sampleText",  // AssetItemInfoFieldSampleText (nex.font)
    @"mergePaths",  // AssetItemInfoFieldMergePaths
};

@interface AssetItemInfo()
@property (nonatomic) NSDictionary *raw;
@end

@implementation AssetItemInfo

#pragma mark - Private
- (id) initWithData:(NSData *) infoData
{
    self = [super init];
    if ( self ) {
        if (infoData) {
            self.raw = [NSJSONSerialization JSONObjectWithData:infoData options:0 error:nil];
        }
        if (self.raw == nil ) {
            NSLog(@"ERROR: Failed loading info");
        }
    }
    return self;
}

#pragma mark - API
- (id) valueForInfoField:(AssetItemInfoField) field
{
    if (self.raw == nil) {
        // Failed loading info in the initializer. Can't use this object.
        return nil;
    }
    
    NSString *fieldKey = [self.class infoFieldStringFor:field];
    return [self valueForInfoFieldKey:fieldKey];
}

- (id) valueForInfoFieldKey:(NSString *) fieldKey
{
    id result = nil;
    if (fieldKey && ![fieldKey isEqualToString:kAssetItemInfoFieldNone]) {
        result = self.raw[fieldKey];
    }
    return result;
}

+ (NSString *) infoFieldStringFor:(AssetItemInfoField) field
{
    if (field == AssetItemInfoFieldNone) {
        return kAssetItemInfoFieldNone;
    }
    return _infoFieldNames[field];
}
@end
