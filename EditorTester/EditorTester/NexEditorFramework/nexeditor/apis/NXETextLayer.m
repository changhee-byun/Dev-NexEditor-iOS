/******************************************************************************
 * File Name   :	NXETextLayer.m
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

#import "NXETextLayer.h"
#import "NXETextLayerPrivate.h"
#import "NXEEngine.h"
#import "KMTextLayer.h"
#import "LayerManager.h"
#import "ImageUtil.h"

@implementation NXETextLayerProperty

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.textColor = [UIColor whiteColor];
        
        self.useShadow = YES;
        self.shadowColor = [UIColor blackColor];
        self.shadowOffset = CGSizeZero;
        
        self.useGlow = NO;
        self.glowColor = RGBA(0xff, 0xff, 0xaa, 0.63);
        self.glowSpread = 10.;
        self.glowSize = 30.;
        
        self.useOutline = NO;
        self.outlineColor = RGB(0xcc, 0xcc, 0xcc);
        self.outlineThickness = 10.;
        
        self.useBackground = NO;
        self.useExtendedBackground = NO;
        self.bgColor = RGBA(0, 0, 0, 0.53);
    
        self.kerningRatio = 0.;
        self.spaceBetweenLines = 0.;
        self.horizontalAlign = NSTextAlignmentLeft;
        self.verticalAlign = NSTextAlignmentLeft;
        self.useUnderline = NO;
        self.shadowAngle = 130.;
        self.shadowDistance = 10;
        self.shadowSpread = 30;
        self.shadowSize = 30;
        
        return self;
    }
    return nil;
}

@end

@interface NXETextLayer ()

@property (nonatomic, retain) NSString *text;
@property (nonatomic, retain) UIFont* textFont;

@property (nonatomic) CGPoint layerPoint;

@end

@implementation NXETextLayer

- (instancetype)initWithText:(NSString *)text font:(UIFont *)font point:(CGPoint)point
{
    self = [super init];
    if(self) {
        self.layerType = NXE_LAYER_TEXT;
        self.layerPoint = point;
        self.x = point.x;
        self.y = point.y;
        
        self.text = text;
        if(font == nil) {
            self.textFont = [UIFont systemFontOfSize:56];
        } else {
            self.textFont = font;
        }
        
        self.textLayerProperty = [[NXETextLayerProperty alloc] init];
        return self;
    }
    return nil;
}

- (void)setText:(NSString *)text Font:(UIFont *)font
{
    self.text = text;
    if(font == nil) {
        self.textFont = [UIFont systemFontOfSize:56];
    } else {
        self.textFont = font;
    }
    
    KMTextLayer *kmTextLayer = (KMTextLayer *)[[LayerManager sharedInstance] getLayer:self.layerId];
    [kmTextLayer setText:text Font:font];
    
    self.width = kmTextLayer.width;
    self.height = kmTextLayer.height;
}

- (void)updateTextProperty
{
    KMTextLayer *kmTextLayer = (KMTextLayer *)[[LayerManager sharedInstance] getLayer:self.layerId];
    
    [kmTextLayer setKerningRatio:self.textLayerProperty.kerningRatio];
    [kmTextLayer setSapceBetweenLines:self.textLayerProperty.spaceBetweenLines];
    [kmTextLayer setHorizontalAlign:self.textLayerProperty.horizontalAlign];
    [kmTextLayer setVerticalAlign:self.textLayerProperty.verticalAlign];
    [kmTextLayer setUnderlineEnable:self.textLayerProperty.useUnderline];
    [kmTextLayer setTextColor:self.textLayerProperty.textColor];
    [kmTextLayer setShadowEnable:self.textLayerProperty.useShadow];
    [kmTextLayer setShadowColor:self.textLayerProperty.shadowColor];
    [kmTextLayer setShadowAngle:self.textLayerProperty.shadowAngle];
    [kmTextLayer setShadowDistance:self.textLayerProperty.shadowDistance];
    [kmTextLayer setShadowSpread:self.textLayerProperty.shadowSpread];
    [kmTextLayer setShadowSize:self.textLayerProperty.shadowSize];
    [kmTextLayer setGlowEnable:self.textLayerProperty.useGlow];
    [kmTextLayer setGlowColor:self.textLayerProperty.glowColor];
    [kmTextLayer setGlowSpread:self.textLayerProperty.glowSpread];
    [kmTextLayer setGlowSize:self.textLayerProperty.glowSize];
    [kmTextLayer setOutlineEnable:self.textLayerProperty.useOutline];
    [kmTextLayer setOutlineColor:self.textLayerProperty.outlineColor];
    [kmTextLayer setOutlineThickness:self.textLayerProperty.outlineThickness];
    [kmTextLayer setBackgroundEnable:self.textLayerProperty.useBackground];
    [kmTextLayer setBackgroundColor:self.textLayerProperty.bgColor];
}

- (NSString *)getText
{
    return self.text;
}

- (UIFont *)getFont
{
    return self.textFont;
}

@end
