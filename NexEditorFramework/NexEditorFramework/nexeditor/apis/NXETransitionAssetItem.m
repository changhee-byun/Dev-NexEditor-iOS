/**
 * File Name   : NXETransitionAssetItem.m
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

#import "NXETransitionAssetItem.h"
#import "NXETransitionAssetItemPrivate.h"
#import "NXEAssetItemPrivate.h"
#import "AssetResourceLoader.h"
#import "EffectAssetItemInfo.h"
#import "TransitionAssetItemInfo.h"

@protocol AssetItemFileResourceDetails
@end
@interface EffectAssetItemInfo(FileResourceDetails) <AssetItemFileResourceDetails>
@end
@interface TransitionAssetItemInfo(FileResourceDetails) <AssetItemFileResourceDetails>
@end

typedef enum {
    InfoChainIndexEffect = 0,
    InfoChainIndexTransition,
} InfoChainIndex;

@interface NXETransitionAssetItem()
@property (nonatomic, readonly) EffectAssetItemInfo *effectItemInfo;
@property (nonatomic, readonly) TransitionAssetItemInfo *transitionItemInfo;
@property (nonatomic, strong) NSArray<id<AssetItemFileResourceDetails>> *infoChain;
@end

@implementation NXETransitionAssetItem

- (id) initWithRawItem:(AssetItem *) raw
{
    self = [super initWithRawItem:raw];
    if ( self ) {
    }
    return self;
}

- (void) dealloc
{
    self.infoChain = nil;
    [super dealloc];
}

/// Reads file resource of the asse item and extracts values and fills effectItemInfo and transitionItemInfo
- (NSArray<id<AssetItemFileResourceDetails>> *) infoChain
{
    if ( _infoChain == nil) {
        NSData *data = [self fileInfoResourceData];
        if ( data ) {
            NSArray<id<AssetItemFileResourceDetails>> *clients = @[
                                 [[[EffectAssetItemInfo alloc] init] autorelease],
                                 [[[TransitionAssetItemInfo alloc] init] autorelease],
                                 ];
            AssetXMLParserClientChain *chain = [AssetXMLParserClientChain chainWithClients:(NSArray<id<AssetXMLParserClient>> *)clients];
            
            AssetXMLParser *parser = [AssetXMLParser parser];
            [parser parseWithData:data client:chain];
            
            _infoChain = [clients retain];
        }
    }
    return _infoChain;
}

#pragma mark -
- (NSInteger) effectOffset
{
    return self.transitionItemInfo.effectOffset;
}

- (NSInteger) effectOverlap
{
    return self.transitionItemInfo.effectOverlap;
}

- (NSInteger) effectDuration
{
    return self.transitionItemInfo.effectDuration;
}

#pragma mark -
- (TransitionAssetItemInfo *) transitionItemInfo
{
    return (TransitionAssetItemInfo *) self.infoChain[InfoChainIndexTransition];
}

#pragma mark -
- (EffectAssetItemInfo *) effectItemInfo
{
    return (EffectAssetItemInfo *) self.infoChain[InfoChainIndexEffect];
}

@end

@implementation NXETransitionAssetItem(Private)
#pragma mark - ConfigurableAssetItem
- (NSArray<UserField *> *) userFields
{
    return self.effectItemInfo.userFields;
}
@end
