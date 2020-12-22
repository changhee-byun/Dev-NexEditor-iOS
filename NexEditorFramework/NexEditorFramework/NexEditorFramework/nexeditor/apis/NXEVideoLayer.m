/******************************************************************************
 * File Name   :	NXEVideoLayer.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "NXEVideoLayer.h"
#import "NXEClipInternal.h"
#import "NXEEngine.h"
#import "KMVideoLayer.h"
#import "LayerManager.h"

#define VIDEOLAYER_DEFAULT_SCALE    (0.45)

@implementation NXEVideoLayer

+ (instancetype) layerWithClipSource:(NXEClipSource *) clipSource error:(NSError **) error
{
    return [[self.class alloc] initWithClipSource: clipSource point:CGPointNXELayerCenter error: error];
}

- (instancetype) initWithClipSource:(NXEClipSource *) clipSource point:(CGPoint)point error:(NSError **) error
{
    self = [super init];
    if ( self ) {
        self.layerClip = [NXEClip clipWithSource:clipSource error:error];
        if (error != NULL && *error != nil) {
            [self release];
            return nil;
        }
        self.layerClip.videoInfo.clipType = NXE_CLIPTYPE_VIDEO_LAYER;
        self.layerClip.videoInfo.startTime = 0;
        self.layerClip.videoInfo.endTime = self.layerClip.videoInfo.totalTime;
        
        self.layerType = NXE_LAYER_VIDEO;
        
        float width; float height;
        int layerW; int layerH;

        width = self.layerClip.size.width;
        height = self.layerClip.size.height;
        
        CGSize baseLayerSize = LayerManager.sharedInstance.renderRegionSize;
        CGFloat baseLayerWiderEdge = MAX(baseLayerSize.width, baseLayerSize.height);
        CGFloat baseLayerNarroEdge = MIN(baseLayerSize.width, baseLayerSize.height);
        if(width < height) {
            layerW = baseLayerNarroEdge * VIDEOLAYER_DEFAULT_SCALE;
        } else {
            layerW = baseLayerWiderEdge * VIDEOLAYER_DEFAULT_SCALE;
        }
        layerH = layerW * (height/width);
        
        if(point.x == CGPointNXELayerCenter.x) {
            point.x = (baseLayerSize.width - layerW) / 2;
        }
        if (point.y == CGPointNXELayerCenter.y) {
            point.y = (baseLayerSize.height - layerH) /2;
        }
        self.layerRect = CGRectMake(point.x, point.y, layerW, layerH);
        //
        self.x = self.layerRect.origin.x;
        self.y = self.layerRect.origin.y;
        self.width = self.layerRect.size.width;
        self.height = self.layerRect.size.height;
        self.startTime = self.layerClip.vinfo.startTime;
        self.endTime = self.layerClip.vinfo.endTime;
        
    }
    return self;
}

- (instancetype)initVideoLayerWithPath:(NSString *)path point:(CGPoint)point
{
    NXEClipSource *clipSource = [NXEClipSource sourceWithPath:path];
    NSError *error = nil;
    NXEVideoLayer *result = [self initWithClipSource:clipSource point:point error:&error];
    if ( error ) {
        NSLog(@"Error: Failed creating video layer with clip at %@. %@", path, error.localizedDescription);
    }
    return result;
}

@end
