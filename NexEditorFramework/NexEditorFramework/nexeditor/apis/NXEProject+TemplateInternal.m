/**
 * File Name   : NXEProject+TemplateInteranl.m
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

#import "NXEProject+TemplateInternal.h"
#import "NXEProjectInternal.h"
#import "TemplateHandler.h"
#import "KMImageLayer.h"
#import "BundleResource.h"
#import "NXEEnginePrivate.h"
#import "LayerManager.h"
#import "NXETemplateProject+Internal.h"

static NSString *const kVignette = @"vignetteImage";

@interface VignetteLayers()
@property (nonatomic, strong) NSArray<KMImageLayer *> *imageLayers;
@end

@implementation VignetteLayers

- (instancetype) initWithImageLayers:(NSArray<KMImageLayer *> *) imageLayers
{
    self = [super init];
    if ( self ) {
        self.imageLayers = imageLayers;
    }
    return self;
}

- (void) dealloc
{
    for(KMImageLayer *layer in self.imageLayers) {
        [[LayerManager sharedInstance] removeLayer:layer.layerId];
    }
    self.imageLayers = nil;
    [super dealloc];
}

@end

@implementation NXEProject (TemplateInteranl)

- (VignetteLayers *)vignetteLayers
{
    return self.hiddenStore[kVignette];
}

- (void) setVignetteLayers:(VignetteLayers *)vignettes
{
    if ( vignettes ) {
        for (KMImageLayer *layer in vignettes.imageLayers) {
            [[LayerManager sharedInstance] addLayer:layer];
        }
    }
    self.hiddenStore[kVignette] = vignettes;
}

- (KMImageLayer *)vignetteLayerWithOverlayItem:(OverlayItem *)overlayItem
{
    NXEAspectType aspectType = ((NXETemplateProject *)self).aspectType;
    CGSize size = [LayerManager renderRegionSizeWithAspectType:aspectType];

    BundleResourceKey resource = BundleResourceKeyTemplateVignetteHorizontal;
    if (size.width < size.height) {
        resource = BundleResourceKeyTemplateVignetteVertical;
    }
    NSString *path = [BundleResource pathFor:resource];
    
    KMImageLayer *vignetteImageLayer = [[KMImageLayer alloc] initWithImage:[UIImage imageNamed:path]];
    vignetteImageLayer.isSelectable = NO;
    
    if(overlayItem.animationEnable.boolValue) {
        
        [vignetteImageLayer setInAnimationType:KMAnimationInFade WithDuration:overlayItem.aniFadeInSTime.intValue];
        [vignetteImageLayer setOutAnimationType:KMAnimationOutFade WithDuration:overlayItem.aniFadeOutSTime.intValue];
    }
    
    int startTime = ((NXEClip *)self.visualClips[overlayItem.overlayId]).vinfo.startTime;
    int endTime = startTime + overlayItem.displayTime.intValue;
    int variation = [overlayItem.variation substringWithRange:NSMakeRange(1, overlayItem.variation.length -1)].intValue;
    
    if([overlayItem.variation hasPrefix:@"+"]) {
        startTime += variation;
    } else {
        startTime -= variation;
    }

    vignetteImageLayer.width = size.width;
    vignetteImageLayer.height = size.height;
    vignetteImageLayer.startTime = startTime;
    vignetteImageLayer.endTime = endTime;
        
    return [vignetteImageLayer autorelease];
}

- (NSArray<KMImageLayer *>*)vignetteLayersWithOverlays:(NSArray<OverlayItem *>*)overlays
{
    NSMutableArray *layers = [NSMutableArray array];
    
    for(OverlayItem *overlayItem in overlays) {
        [layers addObject:[self vignetteLayerWithOverlayItem:overlayItem]];
    }
    
    return [[layers copy] autorelease];
}

@end
