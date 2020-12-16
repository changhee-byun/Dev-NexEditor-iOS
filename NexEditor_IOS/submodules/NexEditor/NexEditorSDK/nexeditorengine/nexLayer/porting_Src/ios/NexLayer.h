/******************************************************************************
 * File Name   : NexLayer.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#ifndef NexLayer_h
#define NexLayer_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "NexColorMatrix.h"

typedef struct _RenderItemParams {
    
    int effectId;
    char* effectOption;
    int currentTime;
    int startTime;
    int endTime;
    CGRect rect;
    float alpha;
    bool modeEnable;
    
} RenderItemParams;

typedef struct _LayerBlendingParams {
    
    int blendId;
    int textureId;
    char* effectOption;
    int currentTime;
    int startTime;
    int endTime;
    CGRect rect;
    float alpha;
    bool modeEnable;
    
} LayerBlendingParams;

typedef struct _VideoLayerBlendingParams {
    
    int blendId;
    int textureId;
    char* effectOption;
    int currentTime;
    int startTime;
    int endTime;
    CGPoint point;
    CGSize size;
    float alpha;
    bool modeEnable;
    
} VideoLayerBlendingParams;

@interface NexLayer : NSObject

+ (NexLayer*)sharedInstance;
- (void)cleanup;
- (float)getOutputWidth;
- (float)getOutputHeight;
- (int)getCurrentTime;
- (void)setColorMatrix:(float[])colormatrixArray length:(int)colormatrixArraylength;
- (NexColorMatrix*)getColorMatrix;
- (void)save;
- (void)restore;
- (void)translate:(float)x y:(float)y z:(float)z;
- (void)scale:(float)x y:(float)y cx:(float)cx cy:(float)cy;
- (void)scaleXY:(float)x y:(float)y;
- (void)scaleXYZ:(float)x y:(float)y z:(float)y;
- (void)rotateAroundAxis:(float)a x:(float)x y:(float)y z:(float)z;
- (float*)getMatrix;
- (void)rotate:(float)a cx:(float)cx cy:(float)cy;
- (void)setAlpha:(float)alpha;
- (void)setCurrentTime:(int)currentTime;
- (void)setShaderAndParam:(Boolean)isExport;
- (void)setFrameDimensions:(int)width height:(int)height;
- (void)setScreenDimenisions:(int)width height:(int)height;
- (void)preRender;
- (void)postRender;
- (void)setChromakeyViewMaskEnabled:(Boolean)maskOnOff;
- (void)setChromakeyEnabled:(Boolean)enable;
- (void)setChromakeyColor:(int)color clipFg:(float)clipFg clipBg:(float)clipBg bx0:(float)bx0 by0:(float)by0 bx1:(float)bx1 by1:(float)by1;
- (void)setHBlurEnabled:(Boolean)enable;
- (void)setVBlurEnabled:(Boolean)enable;
- (void)setMosaicEnabled:(Boolean)enable;
- (void)setEffectStrength:(float)strength;
- (void)setEffectTextureSize:(int)width height:(int)height;
- (float)getScreenDimensionWidth;
- (float)getScreenDimensionHeight;
- (void)setMaskEnabled:(Boolean)enable;
- (bool)getMaskEnabled;
- (void)setMaskTexID:(int)texid;
- (float)getAlpha;
- (void)setAlphaTestValue:(float)alpha;
- (void)resetMatrix;
- (void)setRenderTarget:(int)renderTarget;
- (void)setZTestMode;
- (void)setZTest;
- (void)releaseZTest;
- (void)releaseZTestMode;
- (void)clearMask;
- (void)clearMask:(int)color;
- (void)setRenderMode:(Boolean)isExport;
- (int)getRenderMode;
- (void)setLUT:(int) textId;
- (void)drawBitmp:(int)imgId left:(float)left top:(float)top right:(float)right bottom:(float)bottom repeatX:(float)repeatX repeatY:(float)repeatY;
- (void)drawBitmp:(int)imgId left:(float)left top:(float)top right:(float)right bottom:(float)bottom;
- (void)drawDirect:(int)imgId x:(float)x y:(float)y w:(float)w h:(float)h;
- (void)drawRenderItem:(RenderItemParams)renderItemParams;
- (void)drawLayerBlending:(LayerBlendingParams)layerBlendingParams;
- (void)drawVideoLayerBlending:(VideoLayerBlendingParams)videoLayerBlendingParams;

@end
#endif /* NexLayer_h */
