/******************************************************************************
 * File Name   :	NXEImageLayer.m
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

#import "NXEImageLayer.h"
#import "NXEClip.h"
#import "NXEEngine.h"
#import "KMImageLayer.h"
#import "LayerManager.h"
#import "EditorUtil.h"

@import Photos;

#define IMAGELAYER_DEFAULT_SCALE    (0.45)

@implementation NXEImageLayer
- (instancetype)initWithImage:(UIImage *)uiImage point:(CGPoint)point
{
    self = [super init];
    if(self) {
        self.uiImage = uiImage;
        self.layerType = NXE_LAYER_IMAGE;
        
        float width; float height;
        int layerW; int layerH;

        width = CGImageGetWidth(uiImage.CGImage) & 0xFFFFFFFE;
        height = CGImageGetHeight(uiImage.CGImage) & 0xFFFFFFFE;

        CGSize baseLayerSize = LayerManager.sharedInstance.renderRegionSize;
        CGFloat baseLayerWiderEdge = MAX(baseLayerSize.width, baseLayerSize.height);
        CGFloat baseLayerNarroEdge = MIN(baseLayerSize.width, baseLayerSize.height);
        if(width < height) {
            layerW = baseLayerNarroEdge * IMAGELAYER_DEFAULT_SCALE;
        } else {
            layerW = baseLayerWiderEdge * IMAGELAYER_DEFAULT_SCALE;
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
        return self;
    }
    return nil;
}

- (void) dealloc {
    self.uiImage = nil;
    [super dealloc];
}
@end
