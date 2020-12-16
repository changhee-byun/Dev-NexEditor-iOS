/**
 * File Name   : AssetItemPathInfo.m
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

#import "AssetItemPathInfo.h"
#import "AssetImplicitRules.h"

@interface AssetItemPathInfo()
@property (nonatomic, strong) NSString *packageId;
@property (nonatomic, strong) NSString *category;
@property (nonatomic, strong) NSString *path;
@end

@implementation AssetItemPathInfo

-(instancetype) initWithPath:(NSString *)path packageId:(NSString *)packageId category:(NSString *)category
{
    self = [super init];
    if ( self ) {
        self.path = path;
        self.packageId = packageId;
        self.category = category;
    }
    return self;
}

#pragma mark - Properties

- (NSString *) infoPath
{
    return [self.path stringByAppendingPathComponent:kAssetReservedFilenameItemInfo];
}

#pragma mark - API

- (NSString *) subpathInPackagePath:(NSString *) packagePath
{
    return [self.class subpathOf:self.path under:packagePath];
}

+(instancetype) pathInfoWithPath:(NSString *)path packageId:(NSString *)packageId category:(NSString *)category
{
    return [[self alloc] initWithPath:path packageId:packageId category:category];
}

+ (NSString *) subpathOf:(NSString *) path under:(NSString *) parentPath
{
    NSString *result = nil;
    
    NSRange range = [path rangeOfString:parentPath];
    if ( range.location == 0 && path.length > range.length) {
        result = [path substringFromIndex:range.length + 1];
    }
    
    return result;
}

@end
