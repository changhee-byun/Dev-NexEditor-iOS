/******************************************************************************
 * File Name   : LayerManager.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "LayerManager.h"
#import "KMLayer.h"
#import <NexEditorEngine/NexEditor.h>
#import "KMWaterMark.h"
#import "NXEEnginePrivate.h"
#import "AspectRatioTool.h"
#import "NXETextureDisposeBag.h"
#import <NexEditorEngine/NexEditorEngine.h>

#define LOG_TAG @"LayerManager"

#define PREVIEW_IN 0
#define PREVIEW_OVERALL 1
#define PREVIEW_OUT 2

#define LAYERRENDERER_MIN_SIZE  720

static LayerManager *layermanagerInstance = nil;
static dispatch_once_t layermanagerToken = 0;
@interface LayerManager()
@property (nonatomic) int rendererWidth;
@property (nonatomic) int rendererHeight;
/// Sub rectangle region within UIView used for video preview (i.e. NXELayerEditorView)
@property (nonatomic) CGRect videoRect;

/// Layers disposed for proper texture delete with the right GL Context
@property (nonatomic, strong) NXETextureDisposeBag *textureDisposeBag;
@end

@implementation LayerManager {
    long layerSequence;
    
    NSMutableArray* layerList;
    KMLayer* selectedLayer;
    
    KMLayer* animationLayer;
    long animationStartTime;
    int animationType;
    
    BOOL isRefreshing;
    int previousState;
}

#pragma mark initialize

+ (LayerManager*)sharedInstance
{
    dispatch_once(&layermanagerToken, ^{
        if(layermanagerInstance == nil) {
            layermanagerInstance = [[LayerManager alloc] init];
        }
    });
    return layermanagerInstance;
}

- (void)dealloc
{
    layermanagerToken = 0; layermanagerInstance = nil;
    
    if ((layerList.count) > 0) {
        NexLogW(LOG_TAG, @"Possible layer texture leaks: layers alive:%d", layerList.count);
    }
    [self clear:YES];
    [layerList release];
    layerList = nil;
    
    self.textureDisposeBag = nil;
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        layerSequence = 0;
        layerList = [[NSMutableArray alloc] init];
        isRefreshing = NO;
        previousState = 0;
        //
        _rendererWidth = (LAYERRENDERER_MIN_SIZE * (16.0/9));
        _rendererHeight = LAYERRENDERER_MIN_SIZE;
        
        self.textureDisposeBag = [NXETextureDisposeBag disposeBagWithReferenceId:0];
        return self;
    }
    return nil;
}

- (void)setVideoRect:(CGRect) rect
{
    _videoRect = rect;
}

- (void) setAspectRatio:(NXESizeInt)ratio
{
    if (ratio.width > 0 && ratio.height > 0) {
        _aspectRatio = ratio;
        NXESizeInt frameDimension = [AspectRatioTool frameSizeWithRatio:ratio minSize:LAYERRENDERER_MIN_SIZE];
        self.rendererWidth = frameDimension.width;
        self.rendererHeight = frameDimension.height;
    }
}

- (void)cleanup
{
    [layermanagerInstance release];
}

+ (CGSize)renderRegionSizeWithAspectType:(NXEAspectType)aspectType
{
    NXESizeInt ratio = [AspectRatioTool ratioWithType:aspectType];
    NXESizeInt size = [AspectRatioTool frameSizeWithRatio:ratio minSize:LAYERRENDERER_MIN_SIZE];
    return CGSizeMake(size.width, size.height);
}

#pragma mark - Properties

- (CGSize) renderRegionSize
{
    return CGSizeMake(self.rendererWidth, self.rendererHeight);
}

- (CGFloat) ratioWidthRendererToVideoRect
{
    return self.rendererWidth / self.videoRect.size.width;
}

- (CGFloat) ratioHeightRendererToVideoRect
{
    return self.rendererHeight / self.videoRect.size.height;
}

#pragma mark layer management

- (KMLayer*)getLayer:(long)layerId {
    //전체 리스트에서 layerId 값이 같은 layer를 찾는다.
    for(KMLayer* layer in layerList) {
        if(layer.layerId == layerId) {
            return layer;
        }
    }
    return nil;
}

- (long)addLayer:(KMLayer*)layer
{
    //이미 등록되어 있으면 다시 추가하지 않는다.
    if([layerList containsObject:layer]) {
        return layer.layerId;
    }
    
    @synchronized (self) {
        //layer에 id값을 설정한다.
        layer.layerId = layerSequence;
        layerSequence++;
        
        //layer 추가
        [layerList addObject:layer];
        layer.textureDisposeBag = self.textureDisposeBag;
    }
    return layer.layerId;
}

- (void)removeLayer:(long)layerId
{
    //layerId에 해당하는 layer를 제거 한다.
    @synchronized (self) {
        KMLayer* layer = [self getLayer:layerId];
        if(layer != nil) {
            if(layer == selectedLayer) {
                selectedLayer = nil;
            }

            [layer disposeTextures];
            [layerList removeObject:layer];
        }
    }
}

- (void)clear:(BOOL)includeWatermark
{
    @synchronized (self) {
        if(includeWatermark == NO) {
            //watermark를 제외한 모든 레이어를 제거한다.
            NSMutableArray *array_ = [[NSMutableArray alloc] init];
            for(KMLayer* layer in layerList) {
                if(!layer) continue;
                if([layer isKindOfClass:KMWaterMark.class]) continue;

                [layer disposeTextures];
                [array_ addObject:[NSNumber numberWithLong:layer.layerId]];
                layerSequence--;
            }
            for(NSNumber *layerId in array_) {
                KMLayer *layer = [self getLayer:[layerId longValue]];
                [layerList removeObject:layer];
            }
            [array_ release];
        } else {
            //모든 레이어를 제거 한다.
            layerSequence = 0;
            for(KMLayer* layer in layerList) {
                if(!layer) continue;
                [layer disposeTextures];
            }
            [layerList removeAllObjects];
        }
    }
}

- (void)bringToFront:(long)layerId 
{
    KMLayer* layer = [self getLayer:layerId];
    if(layer) {
        [self bringToFrontLayer:layer];
    }
}

- (void)bringToFrontLayer:(KMLayer*)layer 
{
    @synchronized (self) {
        int id_=0;
        [layerList insertObject:layer atIndex:layerList.count];
        for(KMLayer *layer_ in layerList) {
            if([layer_ isEqual:layer]) {
                [layerList removeObjectAtIndex:id_];
                break;
            }
            id_++;
        }
    }
    [self refresh];
}

- (void)bringToBack:(long)layerId 
{
    KMLayer* layer = [self getLayer:layerId];
    if(layer) {
        [self bringToBackLayer:layer];
    }
}

- (void)bringToBackLayer:(KMLayer*)layer 
{
    @synchronized (self) {
        int id_=0;
        [layerList insertObject:layer atIndex:0];
        for(KMLayer *layer_ in layerList) {
            if(id_!= 0 && [layer_ isEqual:layer]) {
                [layerList removeObjectAtIndex:id_];
                break;
            }
            id_++;
        }
    }
    [self refresh];
}

- (void)moveToHorizontalCenter:(long)layerId {
    KMLayer* layer = [self getLayer:layerId];
    if(layer) {
        [self moveToHorizontalCenterLayer:layer];
    }
}

- (void)moveToHorizontalCenterLayer:(KMLayer*)layer {
    int x = (self.rendererWidth - layer.width) / 2;
    
    layer.x = x;
    [self refresh];
}

- (void)moveToVerticalCenter:(long)layerId {
    KMLayer* layer = [self getLayer:layerId];
    if(layer) {
        [self moveToVerticalCenterLayer:layer];
    }
}

- (void)moveToVerticalCenterLayer:(KMLayer*)layer {
    int y = ([LayerManager sharedInstance].rendererHeight - layer.height) / 2;
    
    layer.y = y;
    [self refresh];
}


#pragma mark layer selecting

- (KMLayer*)getLayerAtTime:(int)time WithX:(float)x Y:(float)y 
{
    KMLayer* hittedLayer = nil;
    CGPoint point = CGPointMake(x, y);
    if (!CGRectContainsPoint(self.videoRect, point) ) {
        return hittedLayer;
    }
    point = [self convertPointToVideoFromScreen:point];
    
    float rendererX = point.x * self.rendererWidth / self.videoRect.size.width;
    float rendererY = point.y * self.rendererHeight / self.videoRect.size.height;
    
    if(layerList && layerList.count > 0) {
        for(KMLayer* layer in layerList) {
            if(!layer || !layer.isSelectable) {
                continue;
            }
            if([layer isHitAtTime:time WithX:rendererX Y:rendererY])
                hittedLayer = layer;
        }
    }
    return hittedLayer;
}

- (KMLayer *)getLayerAtTime:(int)time WithPosition:(CGPoint)position
{
    return [self getLayerAtTime:time WithX:position.x Y:position.y];
}

- (void)changeHittedLayer:(KMLayer*)hittedLayer
{
    //if(selectedLayer != nil && selectedLayer == hittedLayer) return;
    
    if(selectedLayer != nil)
        selectedLayer.isHit = NO;
    
    if(hittedLayer != nil) {
        /*  preview 에서 편집 할 수 없는 Layer는 아래와 같습니다.
         *  1.  watermark,
         *  2.  template 용도로 만든 Layer
         */
        if(hittedLayer.isSelectable) {
            hittedLayer.isHit = YES;
        } else {
            hittedLayer.isHit = NO;
        }
    }
    
    selectedLayer = hittedLayer;
    
    [self refresh];
}

- (KMLayer*)getHittedLayer 
{
    return selectedLayer;
}


// Entire view to video view coordinates
- (CGPoint) convertPointToVideoFromScreen:(CGPoint) point
{
    point.x -= self.videoRect.origin.x;
    point.y -= self.videoRect.origin.y;
    return point;
}

/// View coordinates to layer coordinates
- (CGPoint)convertPreviewPoint:(CGPoint)point
{
    point = [self convertPointToVideoFromScreen:point];
    return CGPointMake(point.x / self.videoRect.size.width * self.rendererWidth, point.y / self.videoRect.size.height * self.rendererHeight);
}

#pragma mark rendering
- (BOOL)renderOverlay:(int)iParam1 param2:(int)iParam2 param3:(int)iParam3
               param4:(int)iParam4 param5:(int)iParam5 param6:(int)iParam6
               param7:(int)iParam7 param8:(int)iParam8 param9:(int)iParam9
              param10:(int)iParam10 param11:(int)iParam11 param12:(int)iParam12
              param13:(int)iParam13 param14:(int)iParam14 param15:(int)iParam15
              param16:(int)iParam16 param17:(int)iParam17 param18:(int)iParam18
{
    BOOL isExport = iParam16&1;
    
    LayerGLContextIndex contextIndex = LayerGLContextIndexPreview;
    if (isExport) {
        contextIndex = LayerGLContextIndexExport;
    }
    
    [self.textureDisposeBag deleteTextuersForContextIndex:contextIndex];
    
    if(layerList && layerList.count > 0) {
        int currentPosition = iParam2;
        NexLayer* renderer = [NexLayer sharedInstance];

        [renderer setShaderAndParam:isExport];
        [renderer setFrameDimensions:self.rendererWidth height:self.rendererHeight];
        [renderer setScreenDimenisions:self.videoRect.size.width height:self.videoRect.size.height];
        [renderer setCurrentTime:currentPosition];
        
        [renderer preRender];
        [renderer clearMask:0xFFFFFFFF];
        
        @synchronized (self) {
            for(KMLayer* layer in layerList) {
                if(!layer || [layer isKindOfClass:KMWaterMark.class]) continue;
                
                if(![layer isActiveAtTime:currentPosition]) {
                    [layer disposeTextures];
                    continue;
                }
                
                [renderer save];
                [renderer setAlpha:1.0f];
                [renderer setAlphaTestValue:0.0f];
                
                if(layer == animationLayer) {
                    if(animationType == PREVIEW_IN)
                        [self showInAnimation:renderer];
                    else if(animationType == PREVIEW_OVERALL)
                        [self showOverallAnimation:renderer];
                    else
                        [self showOutAnimation:renderer];
                }
                else
                    [layer renderAtTime:currentPosition withRenderer:renderer];
                
                [renderer restore];
            }
            
            // watermark 마지막으로 그려준다.
            //
            if([layerList[0] isKindOfClass:KMWaterMark.class]) {
                KMLayer *watermarkLayer = layerList[0];
                [renderer save];
                [renderer setAlpha:1.0f];
                [renderer setAlphaTestValue:0.0f];
                [watermarkLayer renderAtTime:currentPosition withRenderer:renderer];
                [renderer restore];
            }
        }
        
        [renderer postRender];
    }
    isRefreshing = NO;
    if(animationLayer)
        [self refresh];

    return YES;
}

- (void)refresh
{
    @synchronized (self) {
	    if(!isRefreshing) {
	        isRefreshing = YES;
            [NexEditor.sharedInstance runFastPreviewCommand:kFastPreviewCommandNormal];
	    }
    }
}

#pragma mark preview of animation
- (void)startInAnimation:(KMLayer*)layer
{
    animationType = PREVIEW_IN;
    [self startAnimation:layer];
}

- (void)startOverallAnimation:(KMLayer*)layer
{
    animationType = PREVIEW_OVERALL;
    [self startAnimation:layer];
}
- (void)startOutAnimation:(KMLayer*)layer
{
    animationType = PREVIEW_OUT;
    [self startAnimation:layer];
}

- (void)startAnimation:(KMLayer*)layer
{
    if(selectedLayer == layer) {
        layer.isHit = NO;
    }
    animationLayer = layer;
    animationStartTime = [[NSDate date] timeIntervalSince1970] * 1000;
    [self refresh];
}

- (void)stopAnimation {
    if(selectedLayer == animationLayer) {
        selectedLayer.isHit = YES;
    }
    animationLayer = nil;
    animationStartTime = 0;
    animationType = PREVIEW_IN;
    [self refresh];
}

- (void)showInAnimation:(NexLayer*)renderer
{
    int position;
    int duration = MIN(3000, animationLayer.endTime - animationLayer.startTime);
    
    long cur = [[NSDate date] timeIntervalSince1970] * 1000;
    position = animationLayer.startTime + (cur - animationStartTime) % duration;
    
    [animationLayer renderAtTime:position withRenderer:renderer];
}

- (void)showOverallAnimation:(NexLayer*)renderer
{
    int position;
    int duration = (animationLayer.endTime - animationLayer.startTime) / 2;
    
    
    long cur = [[NSDate date] timeIntervalSince1970] * 1000;
    position = animationLayer.startTime + (cur - animationStartTime) % duration;
    
    [animationLayer renderAtTime:position withRenderer:renderer];
}

- (void)showOutAnimation:(NexLayer*)renderer
{
    int position;
    int duration = MIN(3000, animationLayer.endTime - animationLayer.startTime);
    
    long cur = [[NSDate date] timeIntervalSince1970] * 1000;
    position = animationLayer.endTime - duration + (cur - animationStartTime) % duration;
    
    [animationLayer renderAtTime:position withRenderer:renderer];
}

@end
