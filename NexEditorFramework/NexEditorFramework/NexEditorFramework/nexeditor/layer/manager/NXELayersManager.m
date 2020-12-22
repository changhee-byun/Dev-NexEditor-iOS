/******************************************************************************
 * File Name   :	NXELayersManager.m
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

#import "NXELayersManager.h"
#import "LayerManager.h"
#import "NXELayer.h"

static NXELayersManager *layerManager = nil;
static dispatch_once_t layerManagerToken;

@implementation NXELayersManager
{
    NSMutableArray *layerArray;
}

- (void)dealloc
{
    layerManagerToken = 0; layerManager = nil;
    [layerArray release];
    
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        layerArray = [[NSMutableArray alloc] initWithCapacity:30];
        //
        return self;
    }
    return nil;
}

+ (NXELayersManager *)sharedInstance
{
    dispatch_once(&layerManagerToken, ^{
        if(layerManager == nil) {
            layerManager = [[NXELayersManager alloc] init];
        }
    });
    return layerManager;
}

- (void)cleanup
{
    [layerManager release];
}

- (NXELayer *)getLayer:(long)layerId
{
    for(NXELayer *layer in layerArray) {
        if(layer.layerId == layerId) {
            return layer;
        }
    }
    return nil;
}

- (void)addLayer:(NXELayer *)layer
{
    [layerArray addObject:layer];
    //[layer release];
}

- (void)removeEntryForIndex:(long)layerId
{
    NXELayer *layer = [self getLayer:layerId];
    if(layer != nil) {
        [layerArray removeObject:layer];
    }
}

- (void)removeAllEntry
{
    [layerArray removeAllObjects];
}

- (NSArray *)getLayerList
{
    return layerArray;
}

@end
