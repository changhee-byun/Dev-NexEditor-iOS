/******************************************************************************
 * File Name   : NexLayer.mm
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <stdio.h>
#import <math.h>
#import <string>
#import <vector>
#import <map>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#import "NexLayer.h"
#import "NexEditorLog.h"
#import "NEXVIDEOEDITOR_Interface.h"
#import "common_def.hpp"
#import "chromakey_info.hpp"
#import "matrix.hpp"
#import "render_interface.hpp"
#import "render_resource.hpp"
#import "shader_decorator.hpp"
#import "base_shader.hpp"
#import "colorconv_shader.hpp"
#import "useralpha_shader.hpp"
#import "chromakey_shader.hpp"
#import "mask_shader.hpp"
#import "colorlut_shader.hpp"
#import "blur_shader.hpp"
#import "mosaic_shader.hpp"
#import "colormatrix.hpp"
#import "statestack_entry.hpp"
#import "simplerenderer.hpp"
#import "renderer.hpp"

using namespace std;

#define LOG_TAG @"NEXLayer"

static NexLayer *layer = nil;
static dispatch_once_t layerToken;

static renderer *gpLayerRenderer = nil;

@implementation NexLayer

- (void)dealloc
{
    layerToken = 0; layer = nil;
    //
    gpLayerRenderer->releaseResource();
    
    delete gpLayerRenderer;
    gpLayerRenderer = nil;
    
    [super dealloc];
}

+ (NexLayer*)sharedInstance
{
    dispatch_once(&layerToken, ^{
        if(layer == nil) {
            layer = [[NexLayer alloc] init];
            gpLayerRenderer = new renderer;
        }
    });
    return layer;
}

- (void)cleanup
{
    [layer release];
}

-(float)getOutputWidth
{
    return gpLayerRenderer->getFrameWidth();
}

-(float)getOutputHeight
{
    return gpLayerRenderer->getFrameHeight();
}

-(int)getCurrentTime
{
    return gpLayerRenderer->getCurrentTime();
}

-(void)setColorMatrix:(float*)colormatrixArray length:(int)colormatrixArraylength
{
    ColorMatrix matrix;
    for(int i = 0; i < colormatrixArraylength; i++) {
        matrix.matrix_[i] = colormatrixArray[i];
    }
    gpLayerRenderer->setColorMatrix(matrix);
}

-(NexColorMatrix*)getColorMatrix
{
    float *colorMatrixArray = gpLayerRenderer->getColorMatrix().matrix_;
    return [[NexColorMatrix alloc] initWithColorMatrixArray:colorMatrixArray];
}

-(void)save
{
    gpLayerRenderer->save();
}

-(void)restore
{
    gpLayerRenderer->restore();
}

-(void)translate:(float)x y:(float)y z:(float)z
{
    gpLayerRenderer->translate(x, y, z);
}

-(void)scale:(float)x y:(float)y cx:(float)cx cy:(float)cy
{
    gpLayerRenderer->scale(x, y, cx, cy);
}

-(void)scaleXY:(float)x y:(float)y
{
    gpLayerRenderer->scale(x, y);
}

-(void)scaleXYZ:(float)x y:(float)y z:(float)z
{
    gpLayerRenderer->scale(x, y, z);
}

-(void)rotateAroundAxis:(float)a x:(float)x y:(float)y z:(float)z
{
    gpLayerRenderer->rotateAroundAxis(a, x, y, z);
}

-(float*)getMatrix
{
    return gpLayerRenderer->getMatrix();
}

-(void)rotate:(float)a cx:(float)cx cy:(float)cy
{
    gpLayerRenderer->rotate(a, cx, cy);
}

-(void)setAlpha:(float)alpha
{
    gpLayerRenderer->setAlpha(alpha);
}

-(void)setCurrentTime:(int)currentTime
{
    gpLayerRenderer->setCurrentTime(currentTime);
}

-(void)setShaderAndParam:(Boolean)isExport
{
    gpLayerRenderer->setShaderAndParams(isExport);
}

-(void)setFrameDimensions:(int)width height:(int)height
{
    gpLayerRenderer->setFrameDimension(width, height);
}

-(void)setScreenDimenisions:(int)width height:(int)height
{
    gpLayerRenderer->setScreenDimension(width, height);
}

-(void)preRender
{
    gpLayerRenderer->preRender();
}

-(void)postRender
{
    gpLayerRenderer->postRender();
}

-(void)setChromakeyViewMaskEnabled:(Boolean)maskOnOff
{
    gpLayerRenderer->setChromakeyViewMaskEnabled(maskOnOff);
}

-(void)setChromakeyEnabled:(Boolean)enable
{
    gpLayerRenderer->setChromakeyEnabled(enable);
}

-(void)setChromakeyColor:(int)color clipFg:(float)clipFg clipBg:(float)clipBg bx0:(float)bx0 by0:(float)by0 bx1:(float)bx1 by1:(float)by1
{
    int alpha = color >> 24 & 0xFF;
    int red = color >> 16 & 0xFF;
    int green = color >> 8 & 0xFF;
    int blue = color & 0xFF;
    color = alpha << 24 | red << 16 | green << 8 | blue;
    gpLayerRenderer->setChromakeyColor(color, clipFg, clipBg, bx0, by0, bx1, by1);
}

-(void)setHBlurEnabled:(Boolean)enable
{
    gpLayerRenderer->setHBlurEnabled(enable);
}

-(void)setVBlurEnabled:(Boolean)enable
{
    gpLayerRenderer->setVBlurEnabled(enable);
}

-(void)setMosaicEnabled:(Boolean)enable
{
    gpLayerRenderer->setMosaicEnabled(enable);
}

-(void)setEffectStrength:(float)strength
{
    gpLayerRenderer->setEffectStrength(strength);
}

-(void)setEffectTextureSize:(int)width height:(int)height
{
    gpLayerRenderer->setEffectTextureSize(width, height);
}

-(float)getScreenDimensionWidth
{
    return gpLayerRenderer->getScreenDimensionWidth();
}

-(float)getScreenDimensionHeight
{
    return gpLayerRenderer->getScreenDimensionHeight();
}

-(void)setMaskEnabled:(Boolean)enable
{
    gpLayerRenderer->setMaskEnabled(enable);
}

- (bool)getMaskEnabled
{
	return gpLayerRenderer->getMaskEnabled();
}

-(void)setMaskTexID:(int)texid
{
    gpLayerRenderer->setMaskTexID(texid);
}

-(float)getAlpha
{
    return gpLayerRenderer->getAlpha();
}

-(void)setAlphaTestValue:(float)alpha
{
    gpLayerRenderer->setAlphatestValue(alpha);
}

-(void)resetMatrix
{
    gpLayerRenderer->resetMatrix();
}

-(void)setRenderTarget:(int)renderTarget
{
    gpLayerRenderer->setRenderTarget(renderTarget);
}

-(void)setZTestMode
{
    gpLayerRenderer->setZTestMode();
}

-(void)setZTest
{
    gpLayerRenderer->setZTest();
}

-(void)releaseZTest
{
    gpLayerRenderer->releaseZTest();
}

-(void)releaseZTestMode
{
    gpLayerRenderer->releaseZTestMode();
}

-(void)clearMask
{
    gpLayerRenderer->clearMask();
}

- (void)clearMask:(int)color
{
    gpLayerRenderer->clearMask(color);
}

-(void)setRenderMode:(Boolean)isExport
{
    gpLayerRenderer->setRenderMode(isExport);
}

-(int)getRenderMode
{
    return gpLayerRenderer->getRenderMode();
}

-(void)setLUT:(int) textId
{
    gpLayerRenderer->setLUT(textId);
}

-(void)drawBitmp:(int)imgId left:(float)left top:(float)top right:(float)right bottom:(float)bottom repeatX:(float)repeatX repeatY:(float)repeatY
{
	gpLayerRenderer->drawBitmap(imgId, left, top, right, bottom, repeatX, repeatY);
}

- (void)drawBitmp:(int)imgId left:(float)left top:(float)top right:(float)right bottom:(float)bottom
{
    int flipMode = 0;
    gpLayerRenderer->drawBitmap(imgId, left, top, right, bottom, flipMode);
}

-(void)drawDirect:(int)imgId x:(float)x y:(float)y w:(float)w h:(float)h
{
    int flipMode = 0;
    int type = 0;
    gpLayerRenderer->drawDirect(imgId, type, x, y, w, h, flipMode);
}

- (void)drawRenderItem:(RenderItemParams)renderItemParams
{
    int flipMode = 0;
    gpLayerRenderer->drawRenderItem(renderItemParams.effectId,
                                    renderItemParams.effectOption,
                                    renderItemParams.currentTime,
                                    renderItemParams.startTime,
                                    renderItemParams.endTime,
                                    renderItemParams.rect.origin.x,
                                    renderItemParams.rect.origin.y,
                                    renderItemParams.rect.size.width,
                                    renderItemParams.rect.size.height,
                                    renderItemParams.alpha,
                                    renderItemParams.modeEnable,
                                    flipMode);
}

- (void)drawLayerBlending:(LayerBlendingParams)layerBlendingParams
{
    int flipMode = 0;
    gpLayerRenderer->drawRenderItemBitmap(layerBlendingParams.blendId,
                                          layerBlendingParams.textureId,
                                          layerBlendingParams.effectOption,
                                          layerBlendingParams.currentTime,
                                          layerBlendingParams.startTime,
                                          layerBlendingParams.endTime,
                                          layerBlendingParams.rect.origin.x,
                                          layerBlendingParams.rect.origin.y,
                                          layerBlendingParams.rect.size.width,
                                          layerBlendingParams.rect.size.height,
                                          layerBlendingParams.alpha,
                                          layerBlendingParams.modeEnable,
                                          flipMode);
}

- (void)drawVideoLayerBlending:(VideoLayerBlendingParams)videoLayerBlendingParams
{
    int flipMode = 0;
    int type = 0;
    gpLayerRenderer->drawRenderItemBlend(videoLayerBlendingParams.blendId,
                                         videoLayerBlendingParams.textureId,
                                         type,
                                         videoLayerBlendingParams.effectOption,
                                         videoLayerBlendingParams.currentTime,
                                         videoLayerBlendingParams.startTime,
                                         videoLayerBlendingParams.endTime,
                                         videoLayerBlendingParams.point.x,
                                         videoLayerBlendingParams.point.y,
                                         videoLayerBlendingParams.size.width,
                                         videoLayerBlendingParams.size.height,
                                         videoLayerBlendingParams.alpha,
                                         videoLayerBlendingParams.modeEnable,
                                         flipMode);
}

@end
