/**
 * File Name   : AssetLibrary.m
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

#import "AssetLibrary.h"
#import "InternalAssetLibrary.h"

#import "NXEAssetLibrary.h"
#import "NXEAssetItemPrivate.h"

@interface InternalAssetLibrary(AssetLibrary) <AssetLibrary>
@end

@interface NXEAssetItem(AssetLibrary) <AssetItem>
@end


@implementation AssetLibrary
+ (id<AssetLibrary>) library
{
    return [[[InternalAssetLibrary alloc] init] autorelease];
}

@end

@implementation InternalAssetLibrary(AssetLibrary)
@end
