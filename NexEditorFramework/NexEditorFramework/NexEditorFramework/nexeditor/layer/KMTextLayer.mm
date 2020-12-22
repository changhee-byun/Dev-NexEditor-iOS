/******************************************************************************
 * File Name   : KMTextLayer.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>

#import "KMTextLayer.h"
#import "ImageUtil+KMLayer.h"
#import "EditorUtil.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "TextEffecter.h"
#define LOG_TAG @"KMTextLayer"

#pragma mark KMTextLayerProperty

#define TEXTLAYER_SHADOWBLUR_RADIUS_DIVISOR_DEFAULT  15
#define TEXTLAYER_GLOWBLUR_RADIUS_DIVISOR_DEFAULT  16
#define TEXTLAYER_SHADOWOFFSET_FONTPOINT_SCALE_DEFAULT  0.06

#define TEXTLAYER_BACKGROUND_EXTENSION  200

@implementation KMTextLayerProperty

+ (KMTextLayerProperty*)newInstance {
    return [[KMTextLayerProperty alloc] init];
}

- (void)dealloc
{
    [_textColor release];
    [_shadowColor release];
    [_glowColor release];
    [_outlineColor release];
    [_bgColor release];
    [super dealloc];
}

- (id)init 
{
    if(self = [super init]) {
        self.textColor = [UIColor whiteColor];
        
        self.useShadow = YES;
        self.shadowColor = [UIColor blackColor];
        self.shadowOffset = CGSizeZero;
        self.shadowBlurRadiusDivisor = TEXTLAYER_SHADOWBLUR_RADIUS_DIVISOR_DEFAULT;
        self.shadowAngle = 130.;
        self.shadowDistance = 10.;
        self.shadowSpread = 30.;
        self.shadowSize = 30.;
        
        self.useGlow = NO;
        self.glowColor = RGBA(0xff, 0xff, 0xaa, 0.63);
        self.glowBlurRadiusDivisor = TEXTLAYER_GLOWBLUR_RADIUS_DIVISOR_DEFAULT;
        self.glowSpread = 10.;
        self.glowSize = 30.;

        self.useOutline = NO;
        self.outlineColor = RGB(0xcc, 0xcc, 0xcc);
        self.outlineThickness = 0.;
        
        self.useBackground = NO;
        self.useExtendedBackground = NO;
        self.bgColor = RGBA(0, 0, 0, 0.53);
        
        self.kerningRatio = 0.;
        self.spaceBetweenLines = 0.;
        self.horizontalAlign = NSTextAlignmentLeft;
        self.verticalAlign = NSTextAlignmentLeft;
        self.useUnderline = NO;
    }
    return self;
}

- (CGSize) textSizeMarginWithFont:(UIFont *) font
{
    CGSize result;
    
    CGFloat shadowBlurRadius = 0;
    CGFloat glowBlurRadius = 0;
    
    if (self.useShadow) {
        shadowBlurRadius = font.pointSize / self.shadowBlurRadiusDivisor;
    }
    if (self.useGlow) {
        glowBlurRadius = font.pointSize / self.glowBlurRadiusDivisor;
    }
    
    CGFloat blurRadius = MAX(shadowBlurRadius, glowBlurRadius);
    
    result.width = ceil(self.shadowOffset.width + blurRadius);
    result.height = ceil(ABS(self.shadowOffset.height) + blurRadius);
    return result;
}

@end

#pragma mark KMText class

@implementation KMText

@synthesize text = _text, rect = _rect;

-(id) initWithText:(NSString*)text Rect:(CGRect)rect {
    if(self = [super init]) {
        _text = text;
        _rect = rect;
    }
    return self;
}

@end

#pragma mark KMTextLayer class

@interface KMTextLayer ()

@property (nonatomic, retain) NSString* layerText;
@property (nonatomic, retain) UIFont* textFont;
@property (nonatomic) BOOL needsReloadTextures;
@end

@implementation KMTextLayer {
//    NSString* layerText;
//    UIFont* textFont;
    
    CGSize realTextSize;
    NSMutableArray* textArray;
    
    BOOL isPrepared;
    
    KMTextLayerProperty* properties;
    TextEffecter* textEffecter;
}

- (void)dealloc
{
    [_layerText release];
    [_textFont release];
    [textArray release];
    [properties release];
    [textEffecter release];
    [super dealloc];
}

- (id)init
{
    if(self = [super init]) {
        [self initData:YES];
    }
    return self;
}

/// Dispose all the current textures and rerender text into a new texture at next iteration of render loop
- (void) setNeedsReloadTextures
{
    self.needsReloadTextures = YES;
}

- (void)initData:(BOOL)isFirst
{
    @synchronized (self) {
        //최소 init에서 콜한 경우
        if(isFirst) {
            self.vAlignment = TextAlignmentMiddle;
            self.hAlignment = NSTextAlignmentCenter;
            self.textColor = [UIColor whiteColor];
            textArray = [[NSMutableArray alloc] init];

            properties = [[KMTextLayerProperty alloc] init];
            textEffecter = [[TextEffecter alloc] init];
            
//            UIColor* ptemp = self.textEffecter.outlineColor;
            
            int abc = 0;
            abc++;
        }

        realTextSize = CGSizeZero;      //실제 텍스트 이미지의 크기 초기화
        [textArray removeAllObjects];   //라인별 텍스트와 위치 값 초기화

        isPrepared = NO;                //준비되지 않은 것으로 설정.
    }
}


#pragma mark text 설정 메소드

- (void)setText:(NSString*)text Font:(UIFont*)font
{
    [self initData:NO];     //데이터 초기화
    [self setNeedsReloadTextures];
    
    self.layerText = text;       //텍스트 설정
    self.textFont = font;        //폰트 설정
    
    //폰트 사이즈에 따라 그림자 offset 값을 비율로 계산한다.
    float shadowOffset = font.pointSize * TEXTLAYER_SHADOWOFFSET_FONTPOINT_SCALE_DEFAULT;
    properties.shadowOffset = CGSizeMake(shadowOffset, -shadowOffset);
    
    //현재 텍스트 이미지의 실제 크기를 가져온다.
    __block CGSize textSize = [self getTextSize:self.layerText WithFont:self.textFont WithKerning:self.getKerningRatio];
    
    // NESI-262 Make sure margined text size not larger than GL_MAX_TEXTURE_SIZE
    CGSize margin = [properties textSizeMarginWithFont: font];
    CGSize marginedTextSize = CGSizeMake( textSize.width + margin.width * 2, textSize.height + margin.height * 2);
    float maxTextureSize = [ImageUtil glMaxTextureSize];
    if ( maxTextureSize > 0.0 ) {
        marginedTextSize = [ImageUtil limitSize:marginedTextSize
                                   maxSize:CGSizeMake(maxTextureSize, maxTextureSize)
                                      diff:^(CGSize diff) {
                                          textSize.width = ceil(textSize.width - diff.width);
                                          textSize.height = ceil(textSize.height - diff.height);
                                        
        }];
    }
    
    realTextSize = textSize;            //텍스트 이미지의 크기
    self.width = marginedTextSize.width;  //레이어의 크기 설정. 텍스트 이미지 보다 더 크게 설정. 그림자 효과가 나오게 하기 위해.
    self.height = marginedTextSize.height;//레이어의 크기 설정.
    
    //
    //[textEffecter setText:text withFont:font];
    
}

- (NSString*)getText;{
    return self.layerText;
}

- (UIFont*)getFont{
    return self.textFont;
}

- (void)updateAlignment:(NSTextAlignment) align {
    self.hAlignment = align;
    [self prepare];
}

- (void)prepare
{
    @synchronized (self) {
        isPrepared = YES;
        [textArray removeAllObjects];   //라인별 텍스트를 모두 제거한다.
        
        int marginH = (self.width - realTextSize.width) / 2;    //텍스트 이미지와 레이어의 크기에 따라 여백 값을 계산한다.
        int marginV = (self.height - realTextSize.height) / 2;  //텍스트 이미지와 레이어의 크기에 따라 여백 값을 계산한다.
        
        if(self.vAlignment == TextAlignmentTop) {
            marginV = self.height - realTextSize.height;
        } else if(self.vAlignment == TextAlignmentBottom) {
            marginV = 0;
        }
        
        NSArray* temp = [self.layerText componentsSeparatedByString:@"\n"];  //텍스트를 라인별로 분리한다.
        int cnt = (int)temp.count;
        
        //순서가 반대로 되어야 한다. 왼쪽 아래가 0,0으로 된다.
        for(int i=0; i<cnt; i++) {
            NSString* text = [temp objectAtIndex:i];
            CGSize size = [self getTextSize:text WithFont:self.textFont WithKerning:self.getKerningRatio];    //각 라인별 텍스트의 크기를 가져온다.
            
            float y = marginV - self.textFont.descender + (cnt - 1 - i) * (self.textFont.lineHeight * (self.getSpaceBetweenLines+50.)*2./100.);   //각 라인별 y 좌표를 계산한다.
            CGRect rect = CGRectMake(0, y, self.width, self.height);    //각 라인별 텍스트의 위치 값을 생성한다.
            
            //정렬에 맞게 x 좌표 값을 계산한다.
            switch (self.hAlignment) {
                case NSTextAlignmentLeft:
                    rect.origin.x = marginH;
                    break;
                case NSTextAlignmentRight:
                    rect.origin.x = self.width - size.width - marginH;
                    break;
                default:
                    rect.origin.x = (self.width - size.width) / 2.0;
                    break;
            }
            
            [textArray addObject:[[[KMText alloc] initWithText:text Rect:rect] autorelease]];
        }
    }
}

- (CGSize)getTextSize:(NSString*)text WithFont:(UIFont*) font  WithKerning:(CGFloat)kerning {
    //텍스트 크기를 측정
    NSNumber* kerning_v = [[NSNumber alloc] initWithFloat:kerning];
    CGSize textSize = CGSizeZero;
    textSize = [text sizeWithAttributes:@{NSFontAttributeName:font,NSKernAttributeName:kerning_v}];
    
    NSArray* temp = [self.layerText componentsSeparatedByString:@"\n"];  //텍스트를 라인별로 분리한다.
    int cnt = (int)temp.count;
    //[temp release];
    
    textSize.height += (cnt - 1) * (font.lineHeight * (self.getSpaceBetweenLines)/50.);
    
    
    textSize.width = ceil(textSize.width);
    textSize.height = ceil(textSize.height);
    
    [kerning_v release];
    return textSize;
}


#pragma mark TextLayer property

- (CGFloat)getKerningRatio {
    return properties.kerningRatio;
}

- (CGFloat)getSpaceBetweenLines {
    return properties.spaceBetweenLines;
}

- (NSTextAlignment)getHorizontalAlign {
    return properties.horizontalAlign;
}

- (NSTextAlignment)getVerticalAlign {
    return properties.verticalAlign;
}

- (void)setKerningRatio:(CGFloat)kerningRatio {
    properties.kerningRatio = kerningRatio;
    [self setNeedsReloadTextures];
}

- (void)setSapceBetweenLines:(CGFloat)spaceBetweenLines {
    properties.spaceBetweenLines = spaceBetweenLines;
    [self setNeedsReloadTextures];
}

- (void)setHorizontalAlign:(NSTextAlignment)horizontalAlign {
    properties.horizontalAlign = horizontalAlign;
    [self setNeedsReloadTextures];
}

- (void)setVerticalAlign:(NSTextAlignment)verticalAlign {
    properties.verticalAlign = verticalAlign;
    [self setNeedsReloadTextures];
}

- (void)setUnderlineEnable:(BOOL)isEnable {
    properties.useUnderline = isEnable;
    [self setNeedsReloadTextures];
}

- (void)setTextColor:(UIColor *)textColor {
    properties.textColor = textColor;
    [self setNeedsReloadTextures];
}

- (void)setShadowAngle:(CGFloat)angle {
    properties.shadowAngle = angle;
    [self setNeedsReloadTextures];
}

- (void)setShadowDistance:(CGFloat)distance {
    properties.shadowDistance = distance;
    [self setNeedsReloadTextures];
}

- (void)setShadowSpread:(CGFloat)spread {
    properties.shadowSpread = spread;
    [self setNeedsReloadTextures];
}

- (void)setShadowSize:(CGFloat)size {
    properties.shadowSize = size;
    [self setNeedsReloadTextures];
}

- (void)setOutlineThickness:(CGFloat)thickness {
    properties.outlineThickness = thickness;
    [self setNeedsReloadTextures];
}

- (void)setGlowSpread:(CGFloat)glowSpread {
    properties.glowSpread = glowSpread;
    [self setNeedsReloadTextures];
}

- (void)setGlowSize:(CGFloat)glowSize {
    properties.glowSize = glowSize;
    [self setNeedsReloadTextures];
}

- (void)setTextColorWithInt:(int)textColor {
    unsigned int value = textColor;
    int a = value >> 24;
    int r = (value >> 16)& 0xff;
    int g = (value >> 8) & 0xff;
    int b = value & 0xff;
    
    [self setTextColor:RGBA(r, g, b, (a/255.0))];
}

- (void)setShadowEnable:(BOOL)isEnable{
    properties.useShadow = isEnable;
    [self setNeedsReloadTextures];
}

- (void)setShadowColor:(UIColor *)shadowColor {
    properties.shadowColor = shadowColor;
    [self setNeedsReloadTextures];
}


- (void)setGlowEnable:(BOOL)isEnable {
    properties.useGlow = isEnable;
    [self setNeedsReloadTextures];
}

- (void)setGlowColor:(UIColor *)glowColor {
    properties.glowColor = glowColor;
    [self setNeedsReloadTextures];
}

- (void)setOutlineEnable:(BOOL)isEnable {
    properties.useOutline = isEnable;
    [self setNeedsReloadTextures];
}

- (void)setOutlineColor:(UIColor *)outlineColor {
    properties.outlineColor = outlineColor;
    [self setNeedsReloadTextures];
}

- (void)setBackgroundEnable:(BOOL)isEnable {
    properties.useBackground = isEnable;
}

- (void)setExtendedBackground:(BOOL)isEnable {
    properties.useExtendedBackground = isEnable;
}

- (void)setBackgroundColor:(UIColor *)backgoundColor {
    properties.bgColor = backgoundColor;
}


#pragma mark rendering
- (void)renderAtTime:(int)currentPosition withRenderer:(NexLayer*) renderer
{
    if (self.needsReloadTextures) {
        [self disposeTextures];
        self.needsReloadTextures = NO;
    }
    //렌더링 전에 텍스트의 설정에 따라 위치를 계산한다.
    if(!isPrepared) [self prepare];
    
    [self renderBackgroundWithRenderer:renderer AtTime:currentPosition];    //배경을 먼저 렌더링 한다.
    [super renderAtTime:currentPosition withRenderer:renderer];             //텍스트 레이어를 렌더링 한다.
}

- (void)renderBackgroundWithRenderer:(NexLayer*)renderer AtTime:(int)currentPosition
{
    if(!properties.useBackground) return;
    
    CGFloat red, green, blue, alpha;
    [properties.bgColor getRed:&red green:&green blue:&blue alpha:&alpha];
    int texture = [ImageUtil glTextureIDfromColorRed:red Green:green Blue:blue Alpha:alpha];
    
    float currentAlpha = self.alpha;
    [super applyAnimationForPreRendering:currentPosition];
    
    int l = self.x;
    int t = self.y;
    int r = self.x + self.width;
    int b = self.y + self.height;
    
    //렌더러의 상태를 저장한다. 다른 레이어에 영향을 미치지 않게 하기 위해
    [renderer save];
    
    if(self.alpha < 1.0)
        [renderer setAlpha:self.alpha];
    
    //중심 좌표를 가져온다.
    CGPoint center = [self getCenter];
    int cx = center.x;
    int cy = center.y;
    
    //회전 값이 있을 경우 중심을 기준으로 회전
    if(self.angle > 0)
        [renderer rotate:self.angle cx:cx cy:cy];

    float frameWidth = [renderer getOutputWidth];
    
    //LayerRenderer가 scale 값이 변경되면 좌표체계가 이상해진다.(0~1280, 0~720 범위 벗어난 경우)
    if(properties.useExtendedBackground) {
        int extension = TEXTLAYER_BACKGROUND_EXTENSION;
        int bgLeft = -(extension);
        int bgRight = frameWidth + extension;
        if(self.scale != 1.0) {
            int height = (b - t) * self.scale;
            t = cy - height/2;
            b = cy + height/2;
        }
        [renderer drawBitmp:texture left:bgLeft top:t right:bgRight bottom:b];
    }
    else {
        //확대/축소
        if(self.scale != 1.0)
            [renderer scale:self.scale y:self.scale cx:cx cy:cy];
        [renderer drawBitmp:texture left:l top:t right:r bottom:b];
    }
    
    [ImageUtil deleteTexture:texture];
    
    [renderer restore];
    self.alpha = currentAlpha;
}

#define TEXTEFFECTER_USAGE_MODE 1

- (GLuint)getTextureId
{
    @synchronized (self) {
        if(!textArray || textArray.count <= 0) return -1;
        //
        GLint texture;
        
        int renderMode = [[NexLayer sharedInstance] getRenderMode];
        if(renderMode == 0)
            texture = textureId;
        else
            texture = textureId2;
        //
        if(texture == LAYER_GL_INVALID_TEXTURE_ID) {
#if (TEXTEFFECTER_USAGE_MODE == 0)
            texture = [ImageUtil glTextureIDfromText:textArray withFont:self.textFont withProperty:properties];
#elif (TEXTEFFECTER_USAGE_MODE == 1)
            textEffecter.textColor = properties.textColor;
            //textEffecter.textColor = [UIColor colorWithRed:1.0 green:0 blue:0 alpha:0.];
            textEffecter.useShadow = properties.useShadow;
            
            
            textEffecter.shadowColor = properties.shadowColor;
            textEffecter.shadowAngle = properties.shadowAngle;
            textEffecter.shadowDistance = properties.shadowDistance;
            textEffecter.shadowSpread = properties.shadowSpread;
            textEffecter.shadowSize = properties.shadowSize;
            textEffecter.useGlow = properties.useGlow;
            textEffecter.glowColor = properties.glowColor;
            textEffecter.glowSpread = properties.glowSpread;
            textEffecter.glowSize = properties.glowSize;
            textEffecter.useStroke = properties.useOutline;
            textEffecter.strokeThickness = properties.outlineThickness;
            textEffecter.strokeColor = properties.outlineColor;
//            textEffecter.useBackground = properties.useBackground;
//            textEffecter.useExtendedBackground = properties.useExtendedBackground;
//            textEffecter.bgColor = properties.bgColor;
            textEffecter.kerning = properties.kerningRatio;
            textEffecter.spaceBetweenLines = properties.spaceBetweenLines;
            if      ( properties.horizontalAlign == NSTextAlignmentRight ) textEffecter.horizontalAlign = TEHorizontalAlignRight;
            else if ( properties.horizontalAlign == NSTextAlignmentLeft ) textEffecter.horizontalAlign = TEHorizontalAlignLeft;
            else if ( properties.horizontalAlign == NSTextAlignmentCenter ) textEffecter.horizontalAlign = TEHorizontalAlignCenter;

            if      ( properties.verticalAlign == NSTextAlignmentRight ) textEffecter.verticalAlign = TEVerticalAlignBottom;
            else if ( properties.verticalAlign == NSTextAlignmentLeft ) textEffecter.verticalAlign = TEVerticalAlignTop;
            else if ( properties.verticalAlign == NSTextAlignmentCenter ) textEffecter.verticalAlign = TEVerticalAlignCenter;
            textEffecter.useUnderline = properties.useUnderline;
            
            
            [textEffecter setText:self.layerText withFont:self.textFont];

            
#if 0
            KMText* text = [textArray objectAtIndex:0];
            CGRect rect = text.rect;
            int width = rect.size.width;
            int height =rect.size.height;

            texture = [ImageUtil getTextureIDWithHeight:height width:width drawingBlock:^(CGContextRef context) {
                [textEffecter drawWithContext:context];
            }];
#elif 0
            CGSize size = [textEffecter getLayerSize];
            NSLog(@"Eric GET: %d,%d", (int)size.width, (int)size.height);

            self.width = size.width;
            self.height = size.height;
            
//            texture = [ImageUtil getTextureIDWithHeight:size.height width:size.width drawingBlock:^(CGContextRef context) {
//                [textEffecter drawWithContext:context];
//            }];
//            
            //texture = [textEffecter getTexAfterDraw];
#else
//            static int loopcount = 0;
//            textEffecter.glowSize = (float)(loopcount % 100);
//            NSLog(@"Eric T1 %d.\n",loopcount);
//            loopcount++;
            
            NSArray *ainfo = [textEffecter pseudoDrawText];
            TextEffecterResultInfo* info = [ainfo objectAtIndex:0];
            NSLog(@"Eric T1 psize (%f,%f)\n", info.rect.size.width, info.rect.size.height);
            
            NSArray *nsa = [textEffecter drawText];
            TextEffecterResult * ti;
            if ( [nsa count] > 1 ) {
                ti = [nsa objectAtIndex:1];
                TextEffecterResult *ti_temp = [nsa objectAtIndex:0];
                NSLog(@"Eric T1 0offset(%f,%f), rsize (%f,%f)\n", ti_temp.rect.origin.x, ti_temp.rect.origin.y, ti_temp.rect.size.width, ti_temp.rect.size.height);
            } else {
                ti = [nsa objectAtIndex:0];
            }
            NSLog(@"Eric T1 1offset(%f,%f), rsize (%f,%f)\n", ti.rect.origin.x, ti.rect.origin.y, ti.rect.size.width, ti.rect.size.height);

            if ( ti.image != nil )
                texture = [ImageUtil glTextureIDfromImage:ti.image];
            else
                texture = ti.debuggingInfo;
            
            self.width = ti.rect.size.width;
            self.height = ti.rect.size.height;
            
            
#endif
#elif (TEXTEFFECTER_USAGE_MODE == 2)
#endif
        }
        //
        if(renderMode == 0)
            textureId = texture;
        else
            textureId2 = texture;
        //
        return texture;
    }
}

@end
