/******************************************************************************
 * File Name   : KMVideoLayer.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMVideoLayer.h"
#import <NexEditorEngine/NXEVisualClip.h>
#import "MediaInfo.h"
#import "NXEEditorType.h"
#import <NexEditorEngine/NexEditor.h>
#import "LayerManager.h"

@implementation KMVideoLayer

- (instancetype)initWithWidth:(int)width height:(int)height
{
    if(self = [super init]) {
        isVideo = YES;
        
        self.width = width;
        self.height = height;
        
        return self;
    }
    return nil;
}

- (void)setLayerWidth:(int)width height:(int)height
{
    self.width = width;
    self.height = height;
}

#pragma mark rendering

- (void)renderAtTime:(int)currentPosition withRenderer:(NexLayer*) renderer
{
    if([renderer getRenderMode] == 0) {
        textureId = [self getTextureId];
        [super renderAtTime:currentPosition withRenderer:renderer];
        textureId = -1;
    } else {
        textureId2 = [self getTextureId];
        [super renderAtTime:currentPosition withRenderer:renderer];
        textureId2 = -1;
    }
}

- (GLuint)getTextureId
{
    int renderMode = [[NexLayer sharedInstance] getRenderMode];
    return [[NexEditor sharedInstance] getTexNameForClipID:renderMode clipId:self.layerIndex];
}

@end
