/******************************************************************************
 * File Name   : KMTextLayer.h
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
#import "KMLayer.h"
typedef NS_ENUM(NSUInteger, TextVAlignment) {
    TextAlignmentTop,
    TextAlignmentMiddle,
    TextAlignmentBottom
};

/** 텍스트 레이어의 속성 값을 가지고 있는 클래스
 */
@interface KMTextLayerProperty : NSObject

+ (KMTextLayerProperty*)newInstance;

//text
@property (nonatomic, strong) UIColor* textColor;

//shadow
@property (nonatomic, assign) BOOL useShadow;
@property (nonatomic, assign) CGSize shadowOffset;
@property (nonatomic, strong) UIColor* shadowColor;
@property (nonatomic) CGFloat shadowBlurRadiusDivisor; /* font.pointSize / [divisor] */
@property (nonatomic, assign) CGFloat shadowAngle;
@property (nonatomic, assign) CGFloat shadowDistance;
@property (nonatomic, assign) CGFloat shadowSpread;
@property (nonatomic, assign) CGFloat shadowSize;

//glow
@property (nonatomic, assign) BOOL useGlow;
@property (nonatomic, strong) UIColor* glowColor;
@property (nonatomic) CGFloat glowBlurRadiusDivisor; /* font.pointSize / [divisor] */
@property (nonatomic, assign) CGFloat glowSpread;
@property (nonatomic, assign) CGFloat glowSize;
//outline
@property (nonatomic, assign) BOOL useOutline;
@property (nonatomic, strong) UIColor* outlineColor;
@property (nonatomic, assign) CGFloat outlineThickness;


//background
@property (nonatomic, assign) BOOL useBackground;
@property (nonatomic, assign) BOOL useExtendedBackground;
@property (nonatomic, strong) UIColor* bgColor;

//kerning and space between lines
@property (nonatomic, assign) CGFloat kerningRatio;
@property (nonatomic, assign) CGFloat spaceBetweenLines;
@property (nonatomic, assign) NSTextAlignment horizontalAlign;
@property (nonatomic, assign) NSTextAlignment verticalAlign;
@property (nonatomic, assign) BOOL useUnderline;


/**
 * \brief Calculates margin width/height around the rendered text reegion taking shadowOffset, shadow blur radius and glow blur radius with font. To enlarge measured textSize, width and height of returned CGSize should be doubled. For example, textSize.width += margin.width * 2.
 */
- (CGSize) textSizeMarginWithFont:(UIFont *) font;
@end

/** KMTextLayer에 설정된 문자열 중
 1줄에 해당하는 문자열과 위치 정보를 가지고 있는다.
 */
@interface KMText : NSObject

@property(readonly) NSString* text; //문자열.
@property(readonly) CGRect rect;    //현재 문자열의 위치와 전체 문자열의 크기

/** 1줄의 문자열과 위치 정보로 KMText를 생성한다.
 @param text - 1줄의 문자열
 @param rect - 현재 문자열의 위치(rect.origin)와 전체 문자열의 크기(rect.size)
 @return KMText 객체
 */
- (id)initWithText:(NSString*)text Rect:(CGRect)rect;

@end

/** 문자열을 레이어로 표현한다.
 문자열의 색상 및 폰트, 정렬 방식을 설정할 수 있다.
 */
@interface KMTextLayer : KMLayer

@property(nonatomic) TextVAlignment vAlignment;
@property(nonatomic) NSTextAlignment hAlignment;    //문자열의 정렬 방식 (left, center, right)

/** 문자열과 폰트를 설정한다.
 @param text - 문자열
 @param font - 문자열에 적용할 폰트. UIFont 객체
 */
- (void)setText:(NSString*)text Font:(UIFont*)font;

- (NSString*)getText;
- (UIFont*)getFont;

- (void)updateAlignment:(NSTextAlignment) align;

- (CGFloat)getKerningRatio;
- (CGFloat)getSpaceBetweenLines;
- (NSTextAlignment)getHorizontalAlign;
- (NSTextAlignment)getVerticalAlign;

- (void)setKerningRatio:(CGFloat)kerningRatio;
- (void)setSapceBetweenLines:(CGFloat)spaceBetweenLines;
- (void)setHorizontalAlign:(NSTextAlignment)horizontalAlign;
- (void)setVerticalAlign:(NSTextAlignment)verticalAlign;
- (void)setUnderlineEnable:(BOOL)isEnable;
- (void)setShadowAngle:(CGFloat)angle;
- (void)setShadowDistance:(CGFloat)distance;
- (void)setShadowSpread:(CGFloat)spread;
- (void)setShadowSize:(CGFloat)size;
- (void)setOutlineThickness:(CGFloat)outlineThickness;
- (void)setGlowSprad:(CGFloat)glowSpread;
- (void)setGlowSize:(CGFloat)glowSize;

/** 문자열 색상을 설정한다.
 */
- (void)setTextColor:(UIColor *)textColor;

- (void)setTextColorWithInt:(int)textColor;

/** 그림자 효과의 활성화를 설정한다.
 */
- (void)setShadowEnable:(BOOL)isEnable;

/** 그림자 색상을 설정한다.
 */
- (void)setShadowColor:(UIColor *)shadowColor;

/** glow 효과의 활성화를 설정한다.
 */
- (void)setGlowEnable:(BOOL)isEnable;

/** glow 색상을 설정한다.
 */
- (void)setGlowColor:(UIColor *)glowColor;

/** 외곽선 효과의 활성화를 설정한다.
 */
- (void)setOutlineEnable:(BOOL)isEnable;

/** 외곽선 색상을 설정한다.
 */
- (void)setOutlineColor:(UIColor *)outlineColor;

/** 배경의 활성화를 설정한다.
 */
- (void)setBackgroundEnable:(BOOL)isEnable;

/** 배경 확장 기능의 활성화를 설정한다.
 */
- (void)setExtendedBackground:(BOOL)isEnable;

/** 배경 색상을 설정한다.
 */
- (void)setBackgroundColor:(UIColor *)backgoundColor;

@end
