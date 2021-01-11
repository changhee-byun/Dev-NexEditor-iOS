/******************************************************************************
 * File Name   : KMLayer.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMLayer.h"
#import "MatrixUtil.h"
#import "MathUtil.h"
#import "ImageUtil.h"
#import "EditorUtil.h"
#import "KMAnimationEffect.h"
#import "KMEffectLayer.h"

#import <NexEditorEngine/NexEditorEngine.h>

#pragma mark - LayerButtonTypes

#define LOG_TAG     @"KMLayer"

@interface LayerEditorButton : NSObject

@property (nonatomic, retain) UIImage *image;
@property (nonatomic) LayerButtonPosition position;
@property (nonatomic) GLuint textureId;
@property (nonatomic) CGSize imageSize;
@property (nonatomic) CGSize imageTextureSize;

- (instancetype) initWithPosition:(LayerButtonPosition)position image:(UIImage *)image imageSize:(CGSize)imageSize;

@end

@implementation LayerEditorButton

- (void) dealloc
{
    self.image = nil;
    
    [super dealloc];
}

- (instancetype) initWithPosition:(LayerButtonPosition)position image:(UIImage *)image imageSize:(CGSize)imageSize
{
    self.position = position;
    self.image = image;
    self.textureId = -1;
    self.imageSize = CGSizeMake(26, 26);
    self.imageTextureSize = CGSizeZero;
    
    return self;
}

@end

#pragma mark - LayerEditorBorder

@interface LayerEditorBorder : NSObject

@property (nonatomic, retain) UIColor *color;
@property (nonatomic) CGFloat thickness;
@property (nonatomic) GLuint textureId;

- (instancetype) initWithThickness:(CGFloat)thickness color:(UIColor *)color;

@end

@implementation LayerEditorBorder

- (void) dealloc
{
    self.color = nil;
    
    [super dealloc];
}

- (instancetype) initWithThickness:(CGFloat)thickness color:(UIColor *)color
{
    self = [super init];
    if (self) {
    self.color = color;
    self.thickness = thickness;
    self.textureId = -1;
    }
    return self;
}

@end

#pragma mark ChromaKeyData class

@implementation ChromakeyProperty
@end

#define EDIT_HANDLE_SIZE 100
#define EDIT_HANDLE_HALF_SIZE 50

#pragma mark KMLayer class

@interface KMLayer ()

@property (nonatomic, retain) KMLut *kmLut;
@property (nonatomic, retain) KMAnimation *inAnimation;
@property (nonatomic, retain) KMAnimation *outAnimation;
@property (nonatomic, retain) KMAnimation *overallAnimation;
@property (nonatomic, retain) ChromakeyProperty* chromakeyInfo;
@property (nonatomic, retain) NSMutableArray<KMAnimationEffect *> *animationEffects;

@property (nonatomic) BOOL needsGeometryUpdate;

@property (nonatomic, retain) LayerEditorBorder *layerEditorBorder;
@property (nonatomic, retain) NSMutableArray<LayerEditorButton *> *layerEditorButtons;

@end

@implementation KMLayer {
    float colorMatrix[20];
//    KMLut* lut;
//    ChromakeyProperty* chromakeyInfo;
//    SplitScreenProperty* splitScreenInfo;
//    KMAnimation* inAnimation;
//    KMAnimation* outAnimation;
//    KMAnimation* overallAnimation;
}

@synthesize hitType = _hitType;

#pragma mark - initialize

- (id)init
{
    if(self = [super init]) {
        [self initData];
    }
    return self;
}

- (void)initData
{
    self.isHit = NO;
    _hitType = KM_LAYER_HIT_NONE;
    
    self.x = 0;
    self.y = 0;
    
    self.width = 0;
    self.height = 0;
    
    self.angle = 0;
    self.alpha = 1.0f;
    self.scale = 1.0f;
    
    self.xScale = 0.0f;
    self.yScale = 0.0f;
    
    self.hFlip = NO;
    self.vFlip = NO;
    
    self.brightness = 0;    //맨 처음 초기화 할때는 setter를 통해서 할 필요가 없다. color matrix 연산을 하지 않기 위해
    self.contrast = 0;      //맨 처음 초기화 할때는 setter를 통해서 할 필요가 없다. color matrix 연산을 하지 않기 위해
    self.saturation = 0;    //3개의 값이 설정되면 color matrix 연산을 한다.
    
    self.startTime = 0;
    self.endTime = 0;
    
    self.chromakeyInfo = [[[ChromakeyProperty alloc] init] autorelease];
    
    self.kmLut = nil;
    
    self.isSelectable = YES;
    
    textureId = -1;
    textureId2 = -1;
    
    isVideo = NO;
    isAsset = NO;
    
    self.animationEffects = [[[NSMutableArray alloc] init] autorelease];
    self.layerEditorButtons = [[[NSMutableArray alloc] init] autorelease];
}

- (void)dealloc
{
    self.animationEffects = nil;
    self.inAnimation = nil;
    self.outAnimation = nil;
    self.overallAnimation = nil;
    
    self.chromakeyInfo = nil;
    self.layerEditorButtons = nil;
    self.layerEditorBorder = nil;
    
    self.kmLut = nil;
    
    // DEBUG:
    for(int i = 0; i < NUM_TEXTURE_CONTEXTS; i++) {
        GLuint tex = [self mainTextureForContextIndex:(LayerGLContextIndex)i];
        if ( tex != LAYER_GL_INVALID_TEXTURE_ID) {
            NexLogE(LOG_TAG, @"context:%d layer:%d Leaked texture:%d", i,
                    self.layerId, tex);
        }
    }
    self.textureDisposeBag = nil;
    
    [super dealloc];
}

- (void) setTextureDisposeBag:(id<NXETextureDisposeBagProtocol>) bag
{
    _textureDisposeBag = bag;
    if (self.kmLut) {
        self.kmLut.textureDisposeBag = bag;
    }
}

#pragma mark - transform

- (void)setX:(int)x Y:(int)y
{
    self.x = x;
    self.y = y;
}

- (void)setPosition:(CGPoint)position
{
    self.x = position.x;
    self.y = position.y;
}

- (CGPoint)getCenter {
    return CGPointMake(self.x + self.width/2, self.y + self.height/2);
}

- (void)setAlpha:(float)alpha
{
    alpha = limit(alpha, 0, 1);
    _alpha = alpha;
}

- (void)setAngle:(float)angle
{
    if( angle > 360 ) {
        angle -= 360;
    }
    
    if( angle < 0 ) {
        _angle = angle + 360;
    } else {
        _angle = angle;
    }
}

- (void)setScale:(float)scale
{
    scale = limit(scale, 0, 6);
    _scale = scale;
}

- (void)setXScale:(float)xScale
{
    xScale = limit(xScale, 0, 6);
    _xScale = xScale;
}

- (void)setYScale:(float)yScale
{
    yScale = limit(yScale, 0, 6);
    _yScale = yScale;
}

- (void)applyAnimationForPreRendering:(int)currentPosition {
    KMAnimation* ani = [self getInOutAnimationAtTime:currentPosition];
    if(ani) {
        [ani applyAnimation:self];
    }
    
    if(self.overallAnimation) {
        self.overallAnimation.elapseTime = currentPosition - self.startTime;
        self.overallAnimation.duration = self.endTime - self.startTime;
        [self.overallAnimation applyAnimation:self];
    }
}

#pragma mark Hitter

- (void)setIsHit:(BOOL)isHit
{
    _isHit = isHit;
}

#pragma mark LUT

- (GLuint) lutTextureForContextIndex:(LayerGLContextIndex) contextIndex
{
    GLuint lutTexture = LAYER_GL_INVALID_TEXTURE_ID;
    if(self.kmLut)
        lutTexture = [self.kmLut getTextureId:contextIndex];
    return lutTexture;
}

- (void)setLut:(KMLutType)type {
    if(self.kmLut) {
        [self.kmLut disposeTextures];
        self.kmLut = nil;
    }
    if (type != KM_LUT_NONE) {
    self.kmLut = [[[KMLut alloc] initWithType:type textureDisposeBag:self.textureDisposeBag] autorelease];
    }
}

- (KMLut *)getLut
{
    if(self.kmLut)
        return self.kmLut;
    return nil;
}

#pragma mark color adjustment

- (void)setBrightness:(float)brightness
{
    brightness = limit(brightness, -1, 1);
    _brightness = brightness;
    [self calculateColorMatrix];
}

- (void)setContrast:(float)contrast
{
    contrast = limit(contrast, -1, 1);
    _contrast = contrast;
    [self calculateColorMatrix];
}

- (void)setSaturation:(float)saturation
{
    saturation = limit(saturation, -1, 1);
    _saturation = saturation;
    [self calculateColorMatrix];
}

- (void)calculateColorMatrix
{
    [MatrixUtil getColorMatrix:colorMatrix WithBrightness:self.brightness Contrast:self.contrast Saturation:self.saturation];
}

- (float*)getColorMatrix
{
    return colorMatrix;
}

#pragma mark - Animation

- (void)setInAnimationType:(KMAnimationType)type WithDuration:(int)duration {
    //지원 범위 검사
    if(type <= KMAnimationNone || type >= KMAnimationOverallBlinkSlow) {
        self.inAnimation = nil;
        return;
    }
    
    //duration 검사
    int maxDuration = (self.endTime - self.startTime) / 2.0;
    if(duration > maxDuration)
        duration = maxDuration;
    
    //애니메이션 생성 및 설정
    if(self.inAnimation == nil) {
        self.inAnimation = [[[KMAnimation alloc] init] autorelease];
    }
    self.inAnimation.type = type;
    self.inAnimation.duration = duration;
}

- (void)setOverallAnimationType:(KMAnimationType)type; {
    //지원 범위 검사
    if(type < KMAnimationOverallBlinkSlow || type >= KMAnimationOutFade) {
        self.overallAnimation = nil;
        return;
    }
    
    //애니메이션 생성 및 설정
    if(self.overallAnimation == nil) {
        self.overallAnimation = [[[KMAnimation alloc] init] autorelease];
    }
    self.overallAnimation.type = type;
    self.overallAnimation.duration = self.endTime - self.startTime;
}

- (void)setOutAnimationType:(KMAnimationType)type WithDuration:(int)duration {
    //지원 범위 검사
    if(type < KMAnimationOutFade || type >= KMAnimationMax) {
        self.outAnimation = nil;
        return;
    }
    
    //duration 검사
    int maxDuration = (self.endTime - self.startTime) / 2.0;
    if(duration > maxDuration)
        duration = maxDuration;
    
    //애니메이션 생성 및 설정
    if(self.outAnimation == nil) {
        self.outAnimation = [[[KMAnimation alloc] init] autorelease];
    }
    self.outAnimation.type = type;
    self.outAnimation.duration = duration;
}

- (KMAnimation*)getInOutAnimationAtTime:(int)currentPosition {
    if(self.inAnimation == nil && self.outAnimation == nil) return nil;
    
    int elapseTime = currentPosition - self.startTime;
    int outAnimationStartTime = self.endTime-self.outAnimation.duration;
    
    if(self.inAnimation && elapseTime <= self.inAnimation.duration) {
        self.inAnimation.elapseTime = elapseTime;
        return self.isHit ? nil : self.inAnimation;
    }
    else if(self.outAnimation && currentPosition >= outAnimationStartTime) {
        self.outAnimation.elapseTime = currentPosition - outAnimationStartTime;
        return self.isHit ? nil : self.outAnimation;
    }
    
    return nil;
}

- (void)addKMAnimationEffect:(KMAnimationEffect *)animationEffect
{
    if([self.animationEffects containsObject:animationEffect] == NO) {
        [self.animationEffects addObject:animationEffect];
    }
}

#pragma mark - rendering

- (void)setWithAnimationEffects:(NSArray<KMAnimationEffect*>*)effects time:(int)time
{
    KMAnimationEffect *temp = nil;
    BOOL skip = NO;
    
    for(KMAnimationEffect *animationEffect in self.animationEffects) {
        if(skip == YES && temp.class == animationEffect.class) {
            continue;
        } else {
            skip = NO; temp = animationEffect;
        }
        
        if([animationEffect isActiveAnimationAtTime:time]) {
            int elapsedTime = time - animationEffect.animationStartTime;
            [animationEffect applyAnimation:self forTime:elapsedTime];
            if(animationEffect.class == temp.class) {
                skip = YES;
            }
        } else {
            [animationEffect applyAnimationEx:self forTime:time];
        }
    }
}

- (BOOL)isActiveAtTime:(int)currentPosition
{
    return currentPosition >= self.startTime && currentPosition <= self.endTime;
}

- (void)renderAtTime:(int)currentPosition withRenderer:(NexLayer *)renderer
{
    //splitScreen 기능을 사용할 경우
    {
        // TODO. 미지원 이유로 KineMaster IOS 쪽 코드를 SDK 쪽으로 porting 하지 않음.
    }
    LayerGLContextIndex contextIndex = (LayerGLContextIndex) [[NexLayer sharedInstance] getRenderMode];

    //공통적으로 레이어를 렌더링 하는 작업
    if([self getTextureId] == -1) return;
    
    [self updateGeometryIfNeeded];

    KMAnimation* ani = [self getInOutAnimationAtTime:currentPosition];
    if(ani) {
        [ani save:self];
        [ani applyAnimation:self];
    }
    
    if(self.overallAnimation) {
        self.overallAnimation.elapseTime = currentPosition - self.startTime;
        self.overallAnimation.duration = self.endTime - self.startTime;
        [self.overallAnimation save:self];
        [self.overallAnimation applyAnimation:self];
    }
    
    [self setWithAnimationEffects:self.animationEffects time:currentPosition];
    
    //렌더러의 상태를 저장한다. 다른 레이어에 영향을 미치지 않게 하기 위해
    [renderer save];
    
    //중심 좌표를 가져온다.
    CGPoint center = [self getCenter];
    float cx = center.x;
    float cy = center.y;

    //회전 값이 있을 경우 중심을 기준으로 회전
    if(self.angle > 0)
        [renderer rotate:self.angle cx:cx cy:cy];
    
    //확대/축소 값과 Flip 값을 같이 계산하여 scale을 적용
    float xScale = self.hFlip ? -self.scale:self.scale;
    float yScale = self.vFlip ? -self.scale:self.scale;
    
    if(self.xScale != 0 || self.yScale != 0) {
        xScale = self.hFlip ? -self.xScale : self.xScale;
        yScale = self.vFlip ? -self.yScale : self.yScale;
    }
    
    [renderer scale:xScale y:yScale cx:cx cy:cy];
    
    //레이어 전체 알파 적용.
    if(self.alpha < 1.0)
        [renderer setAlpha:self.alpha];
    
    //크로마키 기능을 사용하는 경우
    {
        // TODO. 미지원 이유로 KineMaster IOS 쪽 코드를 SDK 쪽으로 porting 하지 않음.
    }
    
    //LUT를 적용한다.
    GLuint lutTexture = [self lutTextureForContextIndex:contextIndex];
    if (lutTexture != LAYER_GL_INVALID_TEXTURE_ID) {
        [renderer setLUT:lutTexture];
    }
    //Color Adjust 값을 적용한다.
    [renderer setColorMatrix:colorMatrix length:20];
    
    //레이어를 렌더링 한다.
    GLuint texture = [self mainTextureForContextIndex:contextIndex];
    if(isVideo) {
        [renderer save];
        [renderer drawDirect:texture x:cx y:cy w:self.width h:self.height];
        [renderer restore];
    }
    else if(isAsset) {
        NSString* option = @"";
        RenderItemParams renderItemParams = {
            texture,
            (char*)[option UTF8String],
            currentPosition,
            self.startTime,
            self.endTime,
            CGRectMake(self.x, self.y, self.x+self.width, self.y+self.height),
            renderer.getAlpha,
            [renderer getMaskEnabled]?1:0
        };
        [renderer drawRenderItem:renderItemParams];
    }
    else {
        [renderer drawBitmp:texture left:self.x top:self.y right:self.x+self.width bottom:self.y+self.height];
    }
    
    [renderer setLUT:0];                //LUT값을 초기화 한다.
    [renderer setChromakeyEnabled:NO];  //크로마키 기능 사용을 중지한다.
    
    // render selected layer's border, button
    if ( contextIndex == LayerGLContextIndexPreview ) {
        if ( self.isHit ) {
            [self configureLayerEditorButtonTextures];
            [self configureLayerEditorBorderTextrue];
            [self renderHitting: renderer];
        } else {
            [self deleteLayerEditorBorderTexture];
            [self deleteLayerEditorButtonTextures];
        }
    }
    
    if(self.overallAnimation){
        [self.overallAnimation restore:self];
    }
    if(ani){
        [ani restore:self];
    }
    
    [renderer restore];     //렌더러의 상태를 원래데로 복구한다.
}

- (void)renderSplitAtTime:(int)currentPosition withRenderer:(NexLayer *)renderer
{
}

- (void)drawBorderWithRenderer:(NexLayer *)renderer
{
    [renderer setAlpha: 1.0];
    
    // calculate margin with borderThickness and scale
    CGFloat margin = self.layerEditorBorder.thickness / fabsf(self.scale);
    
    CGFloat borderX, borderY, borderW, borderH;
    
    // layer rect
    CGRect layerRect = CGRectMake(self.x, self.y, self.width, self.height);
    
    // draw top of border
    borderX = layerRect.origin.x; borderY = layerRect.origin.y - margin;
    borderW = layerRect.origin.x + layerRect.size.width; borderH = layerRect.origin.y;
    
    CGRect topRect = CGRectMake(borderX, borderY, borderW, borderH);
    [renderer drawBitmp:self.layerEditorBorder.textureId left:topRect.origin.x top:topRect.origin.y right:topRect.size.width bottom:topRect.size.height];
    
    // draw bottom of border
    borderX = layerRect.origin.x; borderY = layerRect.origin.y + layerRect.size.height;
    borderW = layerRect.origin.x + layerRect.size.width; borderH = layerRect.origin.y + layerRect.size.height + margin;
    
    CGRect bottomRect = CGRectMake(borderX, borderY, borderW, borderH);
    [renderer drawBitmp:self.layerEditorBorder.textureId left:bottomRect.origin.x top:bottomRect.origin.y right:bottomRect.size.width bottom:bottomRect.size.height];
    
    // draw left of border
    borderX = layerRect.origin.x - margin; borderY = layerRect.origin.y - margin;
    borderW = layerRect.origin.x; borderH = layerRect.origin.y + layerRect.size.height + margin;
    
    CGRect leftRect = CGRectMake(borderX, borderY, borderW, borderH);
    [renderer drawBitmp:self.layerEditorBorder.textureId left:leftRect.origin.x top:leftRect.origin.y right:leftRect.size.width bottom:leftRect.size.height];
    
    // draw right of border
    borderX = layerRect.origin.x + layerRect.size.width; borderY = layerRect.origin.y - margin;
    borderW = layerRect.origin.x + layerRect.size.width + margin; borderH = layerRect.origin.y + layerRect.size.height + margin;
    
    CGRect rightRect = CGRectMake(borderX, borderY, borderW, borderH);
    [renderer drawBitmp:self.layerEditorBorder.textureId left:rightRect.origin.x top:rightRect.origin.y right:rightRect.size.width bottom:rightRect.size.height];
}

- (void)drawButtonWithRenderer:(NexLayer *)renderer buttonPosition:(LayerButtonPosition)position buttonSize:(CGSize)size buttonTextureId:(GLint)btnTextureId
{
    CGSize scaledSize = CGSizeMake(self.width * self.scale, self.height * self.scale);
    
    CGPoint edgePoint = [self getLayerEdgePointWithPosition:position scaledSize:scaledSize buttonSize:size];
    
    CGFloat layerEdgeX = edgePoint.x, layerEdgeY = edgePoint.y;

    CGFloat buttonRectX = layerEdgeX - size.width / 2;
    CGFloat buttonRectY = layerEdgeY - size.height / 2;
    CGFloat buttonRectW = buttonRectX + size.width;
    CGFloat buttonRectH = buttonRectY + size.height;
    
    CGRect buttonRect = CGRectMake(buttonRectX, buttonRectY, buttonRectW, buttonRectH);
    [renderer drawBitmp:btnTextureId left:buttonRect.origin.x top:buttonRect.origin.y right:buttonRect.size.width bottom:buttonRect.size.height];
}

- (void)renderHitting:(NexLayer*) renderer
{
    [self drawBorderWithRenderer:renderer];
    
    //rotate, scale 아이콘의 경우 확대,축소 값이 같이 적용되지 않게 하기 위해 렌더러 상태를 복구한다.
    [renderer restore];
    
    [renderer save];
    
    CGPoint center = [self getCenter];
    if(self.angle > 0) [renderer rotate:self.angle cx:center.x cy:center.y];
    
    for ( LayerEditorButton *button in self.layerEditorButtons ) {
        if (CGSizeEqualToSize(CGSizeZero, button.imageTextureSize)) {
            CGSize buttonSize = CGSizeMake(button.imageSize.width / [renderer getScreenDimensionWidth] * [renderer getOutputWidth],
                                           button.imageSize.height / [renderer getScreenDimensionHeight] * [renderer getOutputHeight]);
            
            button.imageTextureSize = buttonSize;
        }
        NSLog(@"mj-debug buttonSize:%@", NSStringFromCGSize(button.imageTextureSize));
        [self drawButtonWithRenderer:renderer buttonPosition:button.position buttonSize:button.imageTextureSize buttonTextureId:button.textureId];
    }
}

- (void) deleteLayerEditorTextures
{
    [self deleteLayerEditorBorderTexture];
    [self deleteLayerEditorButtonTextures];
}

- (void)disposeTextures
{
    for(int i = 0; i < NUM_TEXTURE_CONTEXTS; i++) {
        [self disposeMainTextureForContextIndex:(LayerGLContextIndex) i];
    }
    [self.kmLut disposeTextures];
    
    [self deleteLayerEditorTextures];
}

- (GLuint)getTextureId
{
    // Override in a subclass
    return LAYER_GL_INVALID_TEXTURE_ID;
}
#pragma mark - LayerGLContextIndex
- (GLuint) mainTextureForContextIndex:(LayerGLContextIndex) index
{
    return index == LayerGLContextIndexPreview ? textureId : textureId2;
}

- (void) setMainTexture:(GLuint) texture contextIndex:(LayerGLContextIndex) index
{
    NexLogD(LOG_TAG, @"context:%d texture:%d", index, texture);
    if (index == LayerGLContextIndexPreview) {
        textureId = texture;
    } else {
        textureId2 = texture;
    }
}

- (void) disposeMainTextureForContextIndex:(LayerGLContextIndex) index
{
    GLuint texture = [self mainTextureForContextIndex:index];
    if (texture != LAYER_GL_INVALID_TEXTURE_ID) {
        if (self.textureDisposeBag) {
            [self.textureDisposeBag disposeTexture:texture contextIndex:index];
        } else {
            NexLogW(LOG_TAG, @"Texture dispose bag not set");
        }
        [self setMainTexture:LAYER_GL_INVALID_TEXTURE_ID contextIndex:index];
    }
}

#pragma mark - ChromaKey
/*  TODO. 미지원 이유로 KineMaster IOS 쪽 코드를 SDK 쪽으로 porting 하지 않음.
 */

#pragma mark - SplitScreen
/*  TODO. 미지원 이유로 KineMaster IOS 쪽 코드를 SDK 쪽으로 porting 하지 않음.
 */

#pragma mark - layer editing

- (void) deleteLayerEditorButtonTextures
{
    for ( LayerEditorButton *button in self.layerEditorButtons ) {
        if ( button.textureId != LAYER_GL_INVALID_TEXTURE_ID) {
            [self.textureDisposeBag disposeTexture:button.textureId contextIndex:LayerGLContextIndexPreview];
        }
        button.textureId = LAYER_GL_INVALID_TEXTURE_ID;
    }
}

- (void) configureLayerEditorButtonTextures
{
    for ( LayerEditorButton *button in self.layerEditorButtons ) {
        if ( button.textureId == -1 ) {
            button.textureId = [ImageUtil glTextureIDfromImage:button.image];
        }
    }
}

- (void) deleteLayerEditorBorderTexture
{
    if (self.layerEditorBorder && self.layerEditorBorder.textureId != LAYER_GL_INVALID_TEXTURE_ID) {
        [self.textureDisposeBag disposeTexture:self.layerEditorBorder.textureId contextIndex:LayerGLContextIndexPreview];
        self.layerEditorBorder.textureId = LAYER_GL_INVALID_TEXTURE_ID;
    }
}

- (void) configureLayerEditorBorderTextrue
{
    if ( self.layerEditorBorder.textureId == -1 ) {
        CGFloat red, green, blue, alpha;
        [self.layerEditorBorder.color getRed:&red green:&green blue:&blue alpha:&alpha];
        self.layerEditorBorder.textureId = [ImageUtil glTextureIDfromColorRed:red Green:green Blue:blue Alpha:alpha];
    }
}

- (CGPoint) getLayerEdgePointWithPosition:(LayerButtonPosition)buttonPosition scaledSize:(CGSize)scaledSize buttonSize:(CGSize)size
{
    CGPoint center = [self getCenter];
    
    CGFloat layerEdgeX, layerEdgeY;

    CGFloat marginWidth = size.width / 10;
    CGFloat marginHeight = size.height / 10;

    if ( buttonPosition == LAYER_BTN_POSITION_UPPER_LEFT ) {
        layerEdgeX = center.x - scaledSize.width / 2 - marginWidth;
        layerEdgeY = center.y - scaledSize.height / 2 - marginHeight;
    } else if ( buttonPosition == LAYER_BTN_POSITION_UPPER_RIGHT ) {
        layerEdgeX = center.x + scaledSize.width / 2 + marginWidth;
        layerEdgeY = center.y - scaledSize.height / 2 - marginHeight;
    } else if ( buttonPosition == LAYER_BTN_POSITION_BOTTOM_LEFT ) {
        layerEdgeX = center.x - scaledSize.width / 2 - marginWidth;
        layerEdgeY = center.y + scaledSize.height / 2 + marginHeight;
    } else {
        layerEdgeX = center.x + scaledSize.width / 2 + marginWidth;
        layerEdgeY = center.y + scaledSize.height / 2 + marginHeight;
    }
    
    return CGPointMake(layerEdgeX, layerEdgeY);
}

- (BOOL) isContainToEditorButton:(LayerButtonPosition)position withX:(CGFloat)x y:(CGFloat)y size:(CGSize)size
{
    CGSize scaledSize = CGSizeMake(self.width * self.scale, self.height * self.scale);
    
    CGPoint center = [self getCenter];
    CGFloat radius = [MathUtil getDistanceP1:center P2:CGPointMake(x, y)];
    CGFloat radian = [MathUtil toRadians:([MathUtil getAngleP1:CGPointZero P2:CGPointMake(x-center.x, y-center.y)] - self.angle)];
    
    CGPoint inpoint = CGPointMake(radius * sin(radian) + center.x, -radius * cos(radian) + center.y);
    
    CGPoint edgePoint = [self getLayerEdgePointWithPosition:position scaledSize:scaledSize buttonSize:size];
    CGRect rect = CGRectMake(edgePoint.x - size.width/2, edgePoint.y - size.height/2, size.width, size.height);
    
    return CGRectContainsPoint(rect, inpoint);
}

- (BOOL) isContainToLayerWithX:(CGFloat)x y:(CGFloat)y
{
    CGSize scaledSize = CGSizeMake(self.width * self.scale, self.height * self.scale);
    
    CGPoint center = [self getCenter];
    CGFloat radius = [MathUtil getDistanceP1:center P2:CGPointMake(x, y)];
    CGFloat radian = [MathUtil toRadians:([MathUtil getAngleP1:CGPointZero P2:CGPointMake(x-center.x, y-center.y)] - self.angle)];

    CGPoint inpoint = CGPointMake(radius * sin(radian) + center.x, -radius * cos(radian) + center.y);
    
    CGRect rect = CGRectMake(center.x - scaledSize.width/2, center.y - scaledSize.height/2, scaledSize.width, scaledSize.height);
    
    return CGRectContainsPoint(rect, inpoint);
}

- (BOOL)containCoordX:(float)x Y:(float)y
{
    _hitType = KM_LAYER_HIT_NONE;
    
    if ( _isHit ) {
        for ( LayerEditorButton *button in self.layerEditorButtons ) {
            if ( [self isContainToEditorButton:button.position withX:x y:y size:button.imageTextureSize] ) {
                if ( button.position == LAYER_BTN_POSITION_UPPER_LEFT ) {
                    _hitType = KM_LAYER_HIT_UPPER_LEFT;
                } else if ( button.position == LAYER_BTN_POSITION_UPPER_RIGHT ) {
                    _hitType = KM_LAYER_HIT_UPPER_RIGHT;
                } else if ( button.position == LAYER_BTN_POSITION_BOTTOM_LEFT ) {
                    _hitType = KM_LAYER_HIT_BOTTOM_LEFT;
                } else {
                    _hitType = KM_LAYER_HIT_BOTTOM_RIGHT;
                }
                break;
            }
        }
    }
    
    if( _hitType == KM_LAYER_HIT_NONE ) {
        if ( [self isContainToLayerWithX:x y:y] ) {
            _hitType = KM_LAYER_HIT_BODY;
        }
    }
    
    if ( _hitType != KM_LAYER_HIT_NONE ) {
        return YES;
    } else {
        return NO;
    }
}

- (BOOL)isHitAtTime:(int) time WithX:(float)x Y:(float)y {
    return [self isActiveAtTime:time] && [self containCoordX:x Y:y];
}

- (KMLayerHitType)getHitType
{
    if(self.isHit) return _hitType;
    return KM_LAYER_HIT_NONE;
}

- (void) setNeedsGeometryUpdate
{
    self.needsGeometryUpdate = YES;
}

- (void) updateGeometryIfNeeded
{
    if (self.needsGeometryUpdate) {
        self.needsGeometryUpdate = NO;
        
        if (self.delegate) {
            BOOL updated = NO;
            KMLayerGeometry geo = [self.delegate layer:self shouldUpdateGeometry:&updated];
            if (updated) {
                self.x = geo.x;
                self.y = geo.y;
                self.width = geo.width;
                self.height = geo.height;
                self.startTime = geo.startTime;
                self.endTime = geo.endTime;
            }
        }
    }
}

- (void) setLayerEditorBorderWithColor:(UIColor *)color thickness:(CGFloat)thickness
{
    if ( color == nil ) {
        return;
    }
    if (self.layerEditorBorder && self.layerEditorBorder.textureId != LAYER_GL_INVALID_TEXTURE_ID) {
        [self.textureDisposeBag disposeTexture:self.layerEditorBorder.textureId contextIndex:LayerGLContextIndexPreview];
    }
    LayerEditorBorder *border = [[[LayerEditorBorder alloc] initWithThickness:thickness color:color] autorelease];
    self.layerEditorBorder = border;
}

- (NSInteger) containsLayerEditorButtonWithPosition:(LayerButtonPosition)position
{
    int loopid = 0;
    for ( LayerEditorButton *button in self.layerEditorButtons ) {
        if ( button.position == position ) {
            return loopid;
        }
        loopid++;
    }
    return NSNotFound;
}

- (void) setLayerEditorButtonWithPosition:(LayerButtonPosition)position image:(UIImage *)image imageSize:(CGSize)imageSize
{
    if ( image == nil ) {
        return;
    }
    LayerEditorButton *button = [[[LayerEditorButton alloc] initWithPosition:position image:image imageSize:imageSize] autorelease];
    
    NSInteger result = [self containsLayerEditorButtonWithPosition:position];
    if ( result == NSNotFound ) {
        [self.layerEditorButtons addObject:button];
    } else {
        LayerEditorButton *button2 = self.layerEditorButtons[result];
        if ( button2.textureId != LAYER_GL_INVALID_TEXTURE_ID ) {
            [self.textureDisposeBag disposeTexture:button2.textureId contextIndex:LayerGLContextIndexPreview];
        }
        [self.layerEditorButtons replaceObjectAtIndex:result withObject:button];
    }
}

@end
