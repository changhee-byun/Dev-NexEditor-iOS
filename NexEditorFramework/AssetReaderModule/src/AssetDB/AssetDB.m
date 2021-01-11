/**
 * File Name   : AssetDB.m
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

#import "AssetDB.h"
#import "AssetDBIndexed.h"


@interface AssetDB()
@property (nonatomic) id<AssetDB> impl;

@end

@implementation AssetDB
+ (id<AssetDB>) assetDBWithSourceURLs:(NSArray<NSURL *> *) urls
{
    return [[AssetDBIndexed alloc] initWithSourceURLs:urls];
}
@end

