/**
 * File Name   : NXETextEffect.m
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

#import "NXETextEffect.h"
#import "NXEProjectInternal.h"
#import "AssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import "NXETextEffect+KMLayer.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "NXEError.h"
#import "LayerManager.h"


#define LOG_TAG @"NXETextEffect"

static NSString *const kTextEffect = @"textEffect";

@implementation NXETextEffectParams

@end

@interface NXETextEffect()
@property (nonatomic, strong) NSString *itemId;
@property (nonatomic, strong) NXETextEffectParams *params;
@property (nonatomic, copy) NSArray<KMLayer *> *kmlayers;
@end

@implementation NXETextEffect

- (instancetype) initWithId:(NSString *) itemId params:(NXETextEffectParams *) params
{
    self = [super init];
    if ( self ) {
        self.itemId = itemId;
        self.params = params;
    }
    return self;
}

- (void) dealloc
{
    self.itemId = nil;
    self.params = nil;
    self.kmlayers = nil;
    [super dealloc];
}

- (NXETextEffectParams *) params
{
    if ( _params == nil ) {
        _params = [[NXETextEffectParams alloc] init];
    }
    return _params;
}

- (void) setKmlayers:(NSArray<KMLayer *> *)layers
{
    LayerManager *layerManager = [LayerManager sharedInstance];

    for( KMLayer *kmlayer in _kmlayers) {
        [layerManager removeLayer:kmlayer.layerId];
    }
    [_kmlayers release];
    
    _kmlayers = [layers copy];
    for (KMLayer *kmlayer in _kmlayers) {
        [layerManager addLayer:kmlayer];
    }
}

@end

@implementation NXEProject(TextEffect)

#pragma mark - Properties

- (BOOL) setTextEffect:(NXETextEffect *)textEffect error:(NSError **) error
{
    BOOL result = NO;
    
    if (textEffect) {
        int projectDuration = [self getTotalTime];
        NXEError *nxeError = nil;
        textEffect.kmlayers = [textEffect buildLayersWithDuration:projectDuration
                                                            error:&nxeError];
        if (nxeError) {
            if ( error ) {
                *error = nxeError;
            }
        } else {
            self.hiddenStore[kTextEffect] = textEffect;
            result = YES;
        }
    } else {
        self.hiddenStore[kTextEffect] = nil;
        result = YES;
    }
    return result;
}

- (NXETextEffect *) textEffect
{
    return self.hiddenStore[kTextEffect];
}

@end
