/******************************************************************************
 * File Name   : TextEffecter.mm
 * Description :
 *******************************************************************************
 * Copyright (c) 2002-2019 KineMaster Corporation. All rights reserved.
 * http://www.kinemaster.com
 *
 * THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 * PURPOSE.
 ******************************************************************************/

// Information for the font manipulation,
//
// To avoid ambiguous byte sequences in a UTF-16-encoded string,
// and to make detection of surrogate pairs easy,
// the Unicode standard has reserved the range from U+D800 to U+DFFF for the use of UTF-16.
// Code point values in this range will never be assigned a character.
// When a program sees a bit sequence that falls into this range in a UTF-16 string,
// it knows right away that it has encountered part of a surrogate pair.
// (from:https://www.objc.io/issues/9-strings/unicode/)


#import <Foundation/Foundation.h>
#import <CoreText/CoreText.h>
#import <CoreText/CTFont.h>
#import "TextEffecter.h"

#include "NexLayerStyler.h"

#define BASE_POINTSIZE_FONT             100.0
#define BASE_POINTSIZE_FONT_FOR_KERNING 150.0
#define MIN_SIZE_VALUE                  1.0
#define SIZE_SCALE_FACTOR               0.5
#define MIN_SPREAD_VALUE                1.0
#define DEFAULT_BASE_FONT_POINT_SIZE_FOR_KERNING 24.0
#define MYMAX(A,B)                      ((A)>(B)?(A):(B))
#define MYMIN(A,B)                      ((A)<(B)?(A):(B))

@interface OneText : NSObject
@property(nonatomic, readonly) NSString* text; // string to be drawn.
@property(nonatomic, assign) CGPoint origin;   // origin of one line.

- (id)initWithText:(NSString*)text Origin:(CGPoint)origin;
@end

@implementation OneText
- (id)initWithText:(NSString*)text Origin:(CGPoint)origin {
    if(self = [super init]) {
        _text = text;
        _origin = origin;
    }
    return self;
}
@end

@interface TextEffectProperty()
@property (nonatomic, assign) BOOL useRealGlow;
@property (nonatomic, assign) BOOL useRealShadow;
@property (nonatomic, assign) CGFloat realKerning;
@end

@implementation TextEffectProperty
{
    BOOL isDirty;
    BOOL isRectangleChanged;
};

//static functions
+ (void)setRGBATo:(float*)pTarget FromUIColor:(UIColor*)color
{
    size_t n = CGColorGetNumberOfComponents(color.CGColor);
    const CGFloat* components = CGColorGetComponents(color.CGColor);
    if ( n == 1 || n == 2 ) {
        pTarget[0] = pTarget[1] = pTarget[2] = components[0];
    }
    else if ( n >= 3 ) {
        pTarget[0]=components[0];
        pTarget[1]=components[1];
        pTarget[2]=components[2];
    }
    pTarget[3] = CGColorGetAlpha(color.CGColor);
}

+ (CGFloat)convertDistanceWith:(CGFloat)distance fontPointSize:(CGFloat)psize {
    return ( distance / BASE_POINTSIZE_FONT ) * psize;
}

+ (CGFloat)convertSizeWith:(CGFloat)size fontPointSize:(CGFloat)psize {
    return roundf( MYMAX( (size*2./BASE_POINTSIZE_FONT)*psize * SIZE_SCALE_FACTOR, MIN_SIZE_VALUE) );
}

+ (CGFloat)convertSpreadWith:(CGFloat)spread {
    return MYMAX( 100.0 - spread, 1.0 );
}

+ (CGFloat)convertKerningWith:(CGFloat)kerning fontPointSize:(CGFloat)psize {
    return ceil(( kerning / BASE_POINTSIZE_FONT_FOR_KERNING ) * psize);
}

- (id)init
{
    if(self = [super init]) {
        
        isDirty = NO;
        isRectangleChanged = NO;
        
        self.textColor = [UIColor whiteColor];
        
        self.useShadow = YES;
        self.shadowColor = UIColor.clearColor;
        self.shadowAngle = 130.;
        self.shadowDistance = 10.;
        self.shadowSpread = 30.;
        self.shadowSize = 30.;

        self.useGlow = NO;
        self.glowColor = UIColor.clearColor;
        self.glowSpread = 10.;
        self.glowSize = 30.;

        self.useStroke = NO;
        self.strokeThickness = 0.0;
        self.strokeColor = UIColor.clearColor;
        
        self.baseFontPointSizeForKerning = DEFAULT_BASE_FONT_POINT_SIZE_FOR_KERNING;
        self.realKerning = self.kerning = 0.;
        self.spaceBetweenLines = 0.;
        self.horizontalAlign = TEHorizontalAlignLeft;
        
        self.useUnderline = NO;
        self.useStrokeThrough = NO;
        
        [self updateUseRealGlow];
        [self updateUseRealShadow];
    }
    return self;
}

- (void)dealloc
{
    [_textColor release];
    [_shadowColor release];
    [_glowColor release];
    [_strokeColor release];
    
    [super dealloc];
}

- (void)setTextColor:(UIColor*)color {
    _textColor = color;
    [_textColor retain];
    [self setDirty:YES];
}

- (void)setUseShadow:(BOOL)use {
    _useShadow = use;
    [self setDirty:YES];
    [self updateUseRealShadow];
}

- (void)setShadowColor:(UIColor*)color {
    _shadowColor = color;
    [_shadowColor retain];
    [self setDirty:YES];
}

- (void)setShadowAngle:(CGFloat)shadowAngle {
    while (shadowAngle > 360.) shadowAngle -= 360.;
    while (shadowAngle < 0.) shadowAngle += 360.;
    _shadowAngle = shadowAngle;
    [self setDirty:YES];
}

- (void)setShadowDistance:(CGFloat)shadowDistance {
    if (shadowDistance > 50.) shadowDistance = 50;
    else if (shadowDistance < 0.) shadowDistance = 0.;
    _shadowDistance = shadowDistance;
    [self setDirty:YES];
}

- (void)setShadowSpread:(CGFloat)shadowSpread {
    if (shadowSpread > 100.) shadowSpread = 100;
    else if (shadowSpread < 0.) shadowSpread = 0.;
    _shadowSpread = shadowSpread;
    [self setDirty:YES];
}

- (void)setShadowSize:(CGFloat)shadowSize {
    if (shadowSize > 50.) shadowSize = 50;
    else if (shadowSize < 0.) shadowSize = 0.;
    _shadowSize = shadowSize;
    [self setDirty:YES];
    [self updateUseRealShadow];
}

- (void)setUseGlow:(BOOL)use {
    _useGlow = use;
    [self setTextSizeChanged:YES];
    
    [self updateUseRealGlow];
}

- (void)setGlowColor:(UIColor*)color {
    _glowColor = color;
    [_glowColor retain];
    [self setDirty:YES];
}

- (void)setGlowSpread:(CGFloat)glowSpread {
    if (glowSpread > 100.) glowSpread = 100.;
    else if (glowSpread < 0.) glowSpread = 0.;
    _glowSpread = glowSpread;
    [self setDirty:YES];
}

- (void)setGlowSize:(CGFloat)glowSize {
    if (glowSize > 50.) glowSize = 50.;
    else if (glowSize < 0.) glowSize = 0.;
    _glowSize = ceil(glowSize);
    [self setTextSizeChanged:YES];
    [self updateUseRealGlow];
}

- (void)setUseStroke:(BOOL)useStroke {
    _useStroke = useStroke;
    [self setTextSizeChanged:YES];
}

- (void)setStrokeThickness:(CGFloat)strokeThickness {
    if ( strokeThickness < 0. )        _strokeThickness = 0.0;
    else if ( strokeThickness > 50. )  _strokeThickness = 50.0;
    else                               _strokeThickness = strokeThickness;
    
    [self setTextSizeChanged:YES];
}

- (void)setStokeColor:(UIColor*)color {
    //NSLog(@"outc:%lul", (unsigned long)self.outlineColor.retainCount);
    _strokeColor = color;
    [_strokeColor retain];
    //NSLog(@"outc:%lul", (unsigned long)self.outlineColor.retainCount);
    [self setDirty:YES];
}

- (void) setBaseFontPointSizeForKerning:(CGFloat)baseFontPointSizeForKerning {
    _baseFontPointSizeForKerning = baseFontPointSizeForKerning;
    [self setTextSizeChanged:YES];
}

- (void)setKerning:(CGFloat)kerning {
    _kerning = kerning;
    [self setTextSizeChanged:YES];
}

- (void)setSpaceBetweenLines:(CGFloat)spaceBetweenLines {
    _spaceBetweenLines = spaceBetweenLines;
    [self setTextSizeChanged:YES];
}

- (void)setHorizontalAlign:(TEHorizontalAlign)horizontalAlign {
    _horizontalAlign = horizontalAlign;
    [self setDirty:YES];
}

- (void)setVerticalAlign:(TEVerticalAlign)verticalAlign {
    _verticalAlign = verticalAlign;
    [self setDirty:YES];
}

- (void)setUseUnderline:(BOOL)useUnderline {
    _useUnderline = useUnderline;
    [self setTextSizeChanged:YES];
}

- (void)setUseStrokeThrough:(BOOL)useStrokeThrough {
    _useStrokeThrough = useStrokeThrough;
    [self setTextSizeChanged:YES];
}


- (void)setDirty:(BOOL)yn {
    isDirty = yn;
}

- (BOOL)getDirty {
    return isDirty;
}

- (void)updateUseRealGlow {
    if ( _useGlow && _glowSize >= 1.0 )
        self.useRealGlow = YES;
    else
        self.useRealGlow = NO;
}

- (void)updateUseRealShadow {
    if ( _useShadow )//&& _shadowSize >= 1.0 )
        self.useRealShadow = YES;
    else
        self.useRealShadow = NO;
}

- (void)setTextSizeChanged:(BOOL)yn {
    if ( YES == yn )
        [self setDirty:YES];
    isRectangleChanged = yn;
}

- (BOOL)getTextSizeChanged {
    return isRectangleChanged;
}

@end

@implementation TextEffecterResultInfo
@end

@implementation TextEffecterResult
-(void)dealloc {
//    NSLog(@"Eric Dealloc TextureInfo\n");
    [super dealloc];
}
@end


@interface TextEffecter ()
@property (nonatomic, retain) NSString* text;
@property (nonatomic, retain) UIFont* font;
@end

static EAGLContext* eaglContext = nil;
static int eaglContextRefCount = 0;
static int MAX_DEVICE_TEXTURE_SIZE = 0;

@implementation TextEffecter {
    NSMutableArray* texts;
    int maxWidth;
    CGSize textAreaSize;
    //CGSize layerSize;
    CGPoint textOriginInLayer;
    CGFloat heightDiffBetweenTextAndLayer;

    NexLayerStylerForReadingPixels* layerStyler;

    CTFontRef fontRef;
}


+ (dispatch_queue_t)getTextEffecterQueu {
    static dispatch_queue_t _queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _queue = dispatch_queue_create("com.km.texteffecter.context", DISPATCH_QUEUE_SERIAL);
    });

    return _queue;
}

- (id)init {
    if ( self = [super init] ) {
        maxWidth = 0;
        textAreaSize = CGSizeMake(0,0);
        //layerSize = CGSizeMake(0,0);
        textOriginInLayer = CGPointMake(0,0);
        heightDiffBetweenTextAndLayer = 0.0;
        _text = nil;
        _font = nil;
        texts = [[NSMutableArray alloc] init];

        dispatch_sync( [TextEffecter getTextEffecterQueu], ^{

            // this code block is for managing the sigleton of eaglContext
            if ( eaglContext == nil ) {
                eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

                EAGLContext* oldc = [EAGLContext currentContext];
                if ( oldc != nil )
                    [oldc retain];
                [EAGLContext setCurrentContext:eaglContext];
                
                int max=0;
                glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
                if ( max != 0 )
                    MAX_DEVICE_TEXTURE_SIZE = max;
                else
                    MAX_DEVICE_TEXTURE_SIZE = 4096;

                [EAGLContext setCurrentContext:oldc];
                if ( oldc != nil )
                    [oldc release];
            }
            eaglContextRefCount++;
        });
        
        layerStyler = new NexLayerStylerForReadingPixels();
        
        fontRef = NULL;
        
        return self;
    }
    return nil;
}

- (void)dealloc {
    // todo: check reference count
    
    dispatch_sync( [TextEffecter getTextEffecterQueu], ^{

        EAGLContext* oldc = [EAGLContext currentContext];
        if ( oldc != nil )
            [oldc retain];
        [EAGLContext setCurrentContext:eaglContext];
        
        delete layerStyler;
        [EAGLContext setCurrentContext:oldc];
        if ( oldc != nil )
            [oldc release];
        
        if (eaglContext != nil) {
            eaglContextRefCount--;
            if ( 0 == eaglContextRefCount && eaglContext != nil ) {
                [eaglContext release];
                eaglContext = nil;
            }
        }
    });
    
    [self.text release];
    
    [self.font release];
    
    [texts removeAllObjects];
    [texts release];
    
    if ( fontRef ) {
        CFRelease(fontRef);
        fontRef = NULL;
    }

    [super dealloc];
}

- (bool)getGlyphInformation:(unichar*)pch isSurrogatePair:(bool)l
{
    return true;
}

// checkHeight :
// 0 : no check
// 1 : upward
// 2 : downward
// so, 3 : upward and downward at the same time
#define CHECK_HEIGHT_DIRECTION_UPWARD 1
#define CHECK_HEIGHT_DIRECTION_DOWNWARD 2

- (CGRect)getOneLineTextRect:(NSString*)text checkHeight:(int)checkHeight
{
    NSNumber* kerningNumber = [[NSNumber alloc] initWithFloat:self.realKerning];
    NSNumber* underlineStyle = [[NSNumber alloc] initWithInteger:(self.useUnderline?NSUnderlineStyleSingle:NSUnderlineStyleNone)];

//    NSNumber* strokeWidth = [[NSNumber alloc] initWithFloat:(self.useStroke ? self.strokeThickness:0.0)];
//    CGSize textSize = [text sizeWithAttributes:@{NSFontAttributeName:self.font,
//                                                 NSKernAttributeName:kerningNumber,
//                                                 NSUnderlineStyleAttributeName:underlineStyle,
//                                                 NSStrokeWidthAttributeName:strokeWidth
//    }];
    
    CGSize textSize = [text sizeWithAttributes:@{NSFontAttributeName:self.font,
                                                 NSKernAttributeName:kerningNumber,
                                                 NSUnderlineStyleAttributeName:underlineStyle
    }];
    
//    CGSize maximumLabelSize = CGSizeMake(9999, 9999);
//
//    CGRect textRect22 = [text boundingRectWithSize:maximumLabelSize
//                                options:NSStringDrawingUsesLineFragmentOrigin| NSStringDrawingUsesFontLeading
//                                attributes:@{NSFontAttributeName:self.font,
//                                                                             NSKernAttributeName:kerningNumber,
//                                                                             NSUnderlineStyleAttributeName:underlineStyle
//                                }
//                                         context:nil];

    
    //NSLog(@"Eric textsize1 %f,%f", textSize.width, textSize.height);
    //NSLog(@"Eric textsize2 %f,%f", textSize2.width, textSize2.height);
    // The results are same whether or not NSStrokeWidthAttributeName is applied.
    
//    [strokeWidth release];
    [kerningNumber release];
    [underlineStyle release];

    CFArrayRef fontList = NULL;
    CGFloat diff = 0.0;
    CGFloat xoffset = 0.0;
    
    if ( [text length] > 0 && fontRef ) {
        CGGlyph glyph[2];
        CGSize advance = CGSizeZero;
        CGRect bounds4last = CGRectZero, bounds4first = CGRectZero;

        // Refer to https://mail.gnome.org/archives/commits-list/2015-May/msg00901.html
        /* There is an important distinction between Get, and Copy and Create,
           in names of functions that return values.
           If you use a Get function, you cannot be certain of the returned object’s life span.
           To ensure the persistence of such an object you can retain it (using the CFRetain
           function) or, in some cases, copy it.
           If you use a Copy or Create function, you are responsible for releasing the object
           (using the CFRelease function).
           For more details, see Memory Management Programming Guide for Core Foundation.
         */
        bool codepair = false;
        unichar lastChar[2];
        lastChar[0] = [text characterAtIndex:[text length] - 1];
        if ( (lastChar[0] & 0xD800) == 0xD800 && 1 < [text length] ) {
            lastChar[1] = lastChar[0];
            lastChar[0] = [text characterAtIndex:[text length] - 2];
            codepair = true;
        }
            
        if ( false == CTFontGetGlyphsForCharacters(fontRef, &lastChar[0], &glyph[0], codepair?2:1) ) {
            if ( !fontList ) fontList = CTFontCopyDefaultCascadeListForLanguages(fontRef,NULL);
            if ( fontList != NULL ) {
                CFIndex count = CFArrayGetCount(fontList);
                BOOL bDone = FALSE;
                for ( int i = 0 ; i < count && bDone == FALSE ; i++ ) {
                    CTFontDescriptorRef n = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fontList,i);
                    CTFontRef anotherFontRef = CTFontCreateWithFontDescriptor(n,self.font.pointSize,NULL);
                    if ( CTFontGetGlyphsForCharacters(anotherFontRef, &lastChar[0], &glyph[0], codepair?2:1) ) {
                        CTFontGetAdvancesForGlyphs(anotherFontRef, kCTFontOrientationHorizontal, &glyph[0], &advance, 1);
                        CTFontGetBoundingRectsForGlyphs(anotherFontRef, kCTFontOrientationHorizontal, &glyph[0], &bounds4last, 1);
                        bDone = TRUE;
                    }
                    CFRelease(anotherFontRef);
                }
            }
            //
        } else {
            CTFontGetAdvancesForGlyphs(fontRef, kCTFontOrientationHorizontal, &glyph[0], &advance, 1);
            CTFontGetBoundingRectsForGlyphs(fontRef, kCTFontOrientationHorizontal, &glyph[0], &bounds4last, 1);
        }
        
        diff = ceil(bounds4last.size.width - advance.width);
        if ( diff > 0.0 ) diff += 1.0;
        else diff = 0.0;

        codepair = false;
        unichar firstChar[2];
        firstChar[0] = [text characterAtIndex:0];
        if ( (firstChar[0] & 0xD800) == 0xD800 && 1 < [text length] ) {
            firstChar[1] = [text characterAtIndex:1];
            codepair = true;
        }
        
        if ( false == CTFontGetGlyphsForCharacters(fontRef, &firstChar[0], &glyph[0], codepair?2:1) ) {
            if ( !fontList ) fontList = CTFontCopyDefaultCascadeListForLanguages(fontRef,NULL);
            if ( fontList != NULL ) {
                CFIndex count = CFArrayGetCount(fontList);
                BOOL bDone = FALSE;
                for ( int i = 0 ; i < count && bDone == FALSE ; i++ ) {
                    CTFontDescriptorRef n = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fontList,i);
                    CTFontRef anotherFontRef = CTFontCreateWithFontDescriptor(n,self.font.pointSize,NULL);
                    if ( CTFontGetGlyphsForCharacters(anotherFontRef, &firstChar[0], &glyph[0], codepair?2:1) ) {
                        CTFontGetBoundingRectsForGlyphs(anotherFontRef, kCTFontOrientationHorizontal, &glyph[0], &bounds4first, 1);
                        bDone = TRUE;
                    }
                    CFRelease(anotherFontRef);
                }
            }

        } else {
            CTFontGetBoundingRectsForGlyphs(fontRef, kCTFontOrientationHorizontal, &glyph[0], &bounds4first, 1);
        }
        
        
        if ( bounds4first.origin.x < 0.0 ) {
            diff += ceil(-bounds4first.origin.x);
            xoffset = -bounds4first.origin.x;
        } else {
            diff += ceil(bounds4first.origin.x);
        }
/*
        CGFloat mywidth = 0;
        for ( int i = 0 ; i < [text length] ; i++ ) {
            CGGlyph glyph;
            CGSize advance;
            CGRect bounds4char;

            unichar lastChar = [text characterAtIndex:i];
            CTFontGetGlyphsForCharacters(fontRef, &lastChar, &glyph, 1);

            CTFontGetAdvancesForGlyphs(fontRef, kCTFontOrientationHorizontal, &glyph, &advance, 1);
            CTFontGetBoundingRectsForGlyphs(fontRef, kCTFontOrientationHorizontal, &glyph, &bounds4char, 1);
            
            mywidth += advance.width;
        }
        NSLog(@"Eric %f",mywidth);
 */
        if ( self.realKerning < 0.0 ) diff += -self.realKerning;
    }

    CGFloat maxTopGap = 0.0, maxBottomGap = 0.0;
    

    
    if ( checkHeight > 0 ) {
        //int len = [text length];
        for ( int i = 0 ; i < [text length] ; i++ ) {
            bool codepair = false;
            unichar uchar[2];
            uchar[0] = [text characterAtIndex:i];
            if ( (uchar[0] & 0xD800) == 0xD800 && i < [text length]-1 ) {
                uchar[1] = [text characterAtIndex:i+1];
                i++;
                codepair = true;
            }
            
            CGGlyph glyph[2];
            CGRect bound = CGRectZero;
            if ( false == CTFontGetGlyphsForCharacters(fontRef, &uchar[0], &glyph[0], codepair?2:1) ) {
                if ( !fontList ) fontList = CTFontCopyDefaultCascadeListForLanguages(fontRef,NULL);
                if ( fontList != NULL ) {
                    CFIndex count = CFArrayGetCount(fontList);
                    BOOL bDone = FALSE;
                    for ( int i = 0 ; i < count && bDone == FALSE ; i++ ) {
                        CTFontDescriptorRef n = (CTFontDescriptorRef)CFArrayGetValueAtIndex(fontList,i);
                        CTFontRef anotherFontRef = CTFontCreateWithFontDescriptor(n,self.font.pointSize,NULL);
                        if ( CTFontGetGlyphsForCharacters(anotherFontRef, &uchar[0], &glyph[0], codepair?2:1) ) {
                            CTFontGetBoundingRectsForGlyphs(anotherFontRef, kCTFontOrientationHorizontal, &glyph[0], &bound, 1);
                            bDone = TRUE;
                        }
                        CFRelease(anotherFontRef);
                    }
                    if ( i == count ) { // let us assume this code as 4bytes unicode such as Apple Emoji.
                        
                    }
                }

            } else {
                CTFontGetBoundingRectsForGlyphs(fontRef, kCTFontOrientationHorizontal, &glyph[0], &bound, 1);
            }

            if ( checkHeight | CHECK_HEIGHT_DIRECTION_UPWARD) {
                CGFloat topGap = bound.origin.y + bound.size.height - self.font.ascender;
                maxTopGap = MYMAX(maxTopGap, topGap);
            }

            if ( checkHeight | CHECK_HEIGHT_DIRECTION_DOWNWARD) {
                CGFloat bottomGap = self.font.descender - bound.origin.y; // ex) y = --65, descender = -50
                maxBottomGap = MYMAX(maxBottomGap, bottomGap);
            }
        }

    }
    
    if ( fontList )
        CFRelease( fontList );
    
    return CGRectMake(xoffset, ceil(maxBottomGap), ceil(textSize.width + diff), ceil(textSize.height + maxTopGap + maxBottomGap));
}

- (void)adjustProperties {
    int multiplier = 1;
    if ( self.baseFontPointSizeForKerning > 0.0 ) {
        if ( (int)self.font.pointSize % (int)self.baseFontPointSizeForKerning == 0 ) {
            multiplier = (int)self.font.pointSize / (int)self.baseFontPointSizeForKerning;
        }
    }
    
    if ( multiplier == 1 )
        self.realKerning = [TextEffectProperty convertKerningWith:self.kerning fontPointSize:self.font.pointSize];
    else
        self.realKerning = [TextEffectProperty convertKerningWith:self.kerning fontPointSize:self.baseFontPointSizeForKerning] * multiplier;
}

- (void)calculatePositions {

    int lines = (int)texts.count;
    
    textAreaSize.width = 0;
    //if ( self.horizontalAlign != NSTextAlignmentLeft ) {
        for ( int i = 0 ; i < lines ; i++ ) {
            OneText* atext = [texts objectAtIndex:i];
            CGRect rect = [self getOneLineTextRect:atext.text checkHeight:0];
            textAreaSize.width = textAreaSize.width > rect.size.width ? textAreaSize.width : rect.size.width;
        }
    //}
    CGFloat extendedHeight = 0.0;
    CGFloat adjustedYOffset = 0.0;

    for ( int i = 0 ; i < lines ; i++ ) {
        OneText* atext = [texts objectAtIndex:i];
        CGPoint origin = atext.origin;
        int direction = 0;
        if ( i == 0 ) direction |= CHECK_HEIGHT_DIRECTION_UPWARD;
        if ( i == lines-1 ) direction |= CHECK_HEIGHT_DIRECTION_DOWNWARD;
        
        CGRect rect = [self getOneLineTextRect:atext.text checkHeight:direction];
        if ( i == 0 || i == lines-1 ) {
            extendedHeight += (rect.size.height - self.font.lineHeight) > 0 ? (rect.size.height - self.font.lineHeight) : 0;
            if ( i == lines-1 )
                adjustedYOffset = rect.origin.y;
        }
        
        origin.y = -self.font.descender + adjustedYOffset + (lines - 1 - i) * (self.font.lineHeight * (self.spaceBetweenLines+50.)*2./100.);
        
        switch (self.horizontalAlign) {
            case TEHorizontalAlignLeft:
                origin.x = rect.origin.x;
                break;
            case TEHorizontalAlignRight:
                origin.x = textAreaSize.width - rect.size.width + rect.origin.x;
                break;
            case TEHorizontalAlignCenter:
                origin.x = (textAreaSize.width - rect.size.width) / 2.0 + rect.origin.x;
                break;
        }
        
        atext.origin = origin;

    }

    textAreaSize.height = ceil(/*-font.descender*/ + self.font.lineHeight + extendedHeight + (lines - 1) * (self.font.lineHeight * (self.spaceBetweenLines+50.)*2./100.));
}

- (CGSize)calculateShadowOffsetWithInternalProperties {

    if ( self.useRealShadow ) {

        float distance = [TextEffectProperty convertDistanceWith:self.shadowDistance fontPointSize:self.font.pointSize];
        float angle_radian = (1. * self.shadowAngle) * M_PI / 180.;
        CGFloat width = - distance * cosf(angle_radian);
        CGFloat height = - distance * sinf(angle_radian);

        return CGSizeMake(width, height);
    } else
        return CGSizeZero;
    
}

- (void)calculateLayerSize {
    
    CGRect textRect = CGRectMake(0.,0.,textAreaSize.width+2,textAreaSize.height+2);

    CGFloat fontPointSizeWithStroke = self.font.pointSize * ( 1.0 + ( self.useStroke ? self.strokeThickness/100.:0.) );

    // Outer Stroke
    CGFloat extendedSize4Stroke = fontPointSizeWithStroke - self.font.pointSize;
    textRect = CGRectMake(  textRect.origin.x -extendedSize4Stroke,
                            textRect.origin.y -extendedSize4Stroke,
                            textRect.size.width + extendedSize4Stroke*2.,
                            textRect.size.height + extendedSize4Stroke*2.);

    // Drop Shadow
    CGSize offset = [self calculateShadowOffsetWithInternalProperties];
    CGRect shadowRect = CGRectOffset(textRect, offset.width, +offset.height);
    CGFloat extendedSize = self.useRealShadow ? [TextEffectProperty convertSizeWith:self.shadowSize fontPointSize:fontPointSizeWithStroke] + 2 : 0.;
    CGRect blurredShadowRect = CGRectMake(shadowRect.origin.x-extendedSize,
                                          shadowRect.origin.y-extendedSize,
                                          shadowRect.size.width+extendedSize*2.,
                                          shadowRect.size.height+extendedSize*2.);


    // Outer Glow
    CGFloat extendedSize4Glow = self.useRealGlow ? [TextEffectProperty convertSizeWith:self.glowSize fontPointSize:fontPointSizeWithStroke] + 2 : 0.;
    CGRect blurredGlowRect = CGRectMake(textRect.origin.x - extendedSize4Glow,
                                        textRect.origin.y - extendedSize4Glow,
                                        textRect.size.width + extendedSize4Glow*2.,
                                        textRect.size.height + extendedSize4Glow*2.);
    
    
    
    CGRect effectRect = CGRectUnion(blurredShadowRect, blurredGlowRect);
    CGRect layerRect = CGRectUnion(textRect,effectRect);

    
    //layerSize = layerRect.size;
    CGFloat xd = 1. - MYMIN(effectRect.origin.x, 0.);
    CGFloat yd = 1. - MYMIN(effectRect.origin.y, 0.);

    textOriginInLayer = CGPointMake(xd, yd);
    heightDiffBetweenTextAndLayer = layerRect.size.height - textRect.size.height;

    //NSLog(@"Eric CAL: %d,%d", (int)layerRect.size.width, (int)layerRect.size.height);
}


- (void)setText:(NSString*)text withFont:(UIFont*)font {
    @synchronized (self) {
        self.text = text;
        if ( self.font != font ) {
            self.font = font;
            
            if ( NULL != fontRef ) {
                CFRelease(fontRef);
                fontRef = NULL;
            }
            
            if ( nil != font )
                fontRef = CTFontCreateWithName((CFStringRef)self.font.fontName, self.font.pointSize, NULL);
        }

        [texts removeAllObjects];

        NSArray* temp = [self.text componentsSeparatedByString:@"\n"];
        int cnt = (int)temp.count;
        
        for ( int i=0 ; i < cnt ; i++ ) {
            NSString* text = temp[i];
            [texts addObject:[[[OneText alloc] initWithText:text Origin:CGPointMake(0.,0.)] autorelease]];
        }
        
    }
    
}


- (void)calculateIfNeed {
    if ( [self getTextSizeChanged] ) {
        //[self calculatePositions];
        [self setTextSizeChanged:NO];
    }
    
    if ( [self getDirty] ) {
        //[self calculateLayerSize];
        [self setDirty:NO];
    }
}

- (CGSize)getTextAreaSize {
    [self calculateIfNeed];
    return textAreaSize;
}
/*
- (CGSize)getLayerSize {
    [self calculateIfNeed];
    return layerSize;
}
*/
- (CGPoint)getTextOriginInLayer {
    [self calculateIfNeed];
    return textOriginInLayer;
}

#define MAX_KEYS_IN_FONT_ATTRIBUTES 7

+ (CFDictionaryRef)createDictionaryForFontAttributesWithFont:(UIFont*)font fontColor:(UIColor*)fontColor kerning:(CGFloat)kerning underline:(bool)underline strokeThickness:(CGFloat)strokeThickness strokeColor:(UIColor*)strokeColor {
    
    if ( nil == fontColor || (strokeThickness != 0 && nil == strokeColor)) {
        NSLog(@"createDictionaryForFontAttributesWithFont... error. parameters are invalid.\n");
        return NULL;
    }
    
    CFStringRef keys[MAX_KEYS_IN_FONT_ATTRIBUTES];
    CFTypeRef values[MAX_KEYS_IN_FONT_ATTRIBUTES];
    int keyCount = 0;
    
    CTFontRef fontRefInThisFunction = NULL; {
        CFStringRef fontNameRef = CFStringCreateWithCString(NULL, [[font fontName] UTF8String], kCFStringEncodingUTF8);
        fontRefInThisFunction = CTFontCreateWithName(fontNameRef, font.pointSize, NULL);
        CFRelease(fontNameRef);

        keys[keyCount] = kCTFontAttributeName;
        values[keyCount] = fontRefInThisFunction;
        keyCount++;
    }
    
    CFNumberRef kerningRef = NULL;
    if ( 0.0 != kerning ) {
        float kerning_ = kerning;
        kerningRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &kerning_ );
        
        keys[keyCount] = kCTKernAttributeName;
        values[keyCount] = kerningRef;
        keyCount++;
    }

    CFNumberRef underlineStyleRef = NULL;
    if ( underline ) {
        CTUnderlineStyle underlineStyle = kCTUnderlineStyleSingle;
        underlineStyleRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &underlineStyle);

        keys[keyCount] = kCTUnderlineStyleAttributeName;
        //CTUnderlineStyleModifiers
        values[keyCount] = underlineStyleRef;
        keyCount++;
    }

    // Foreground Color
    if ( nil != fontColor ) {
        keys[keyCount] = kCTForegroundColorAttributeName;
        values[keyCount] = fontColor.CGColor;
        keyCount++;
    }

    CFNumberRef strokeThicknessRef = NULL;
    if ( 0 != strokeThickness ) {
        float strokeWidth = (float)strokeThickness ;// / -5.0;
        strokeThicknessRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &strokeWidth);

        keys[keyCount] = kCTStrokeWidthAttributeName;
        values[keyCount] = strokeThicknessRef;
        keyCount++;
        
        if ( nil != strokeColor ) {
            keys[keyCount] = kCTStrokeColorAttributeName;
            values[keyCount] = strokeColor.CGColor;
            keyCount++;
        }
    }
    
    CFDictionaryRef dicRef = CFDictionaryCreate(kCFAllocatorDefault,
                                           (const void **)&keys,
                                           (const void **)&values,
                                           keyCount,//sizeof(keys) / sizeof(keys[0]),
                                           &kCFTypeDictionaryKeyCallBacks,
                                           &kCFTypeDictionaryValueCallBacks);
    
    if ( kerningRef ) CFRelease(kerningRef);
    if ( fontRefInThisFunction ) CFRelease(fontRefInThisFunction);
    if ( strokeThicknessRef ) CFRelease(strokeThicknessRef);
    if ( underlineStyleRef ) CFRelease(underlineStyleRef);
    
    return dicRef;
}

+ (void)deleteDictionaryForFontAttributesWithFontColor:(CFDictionaryRef)dicRef {
    CFRelease(dicRef);
}


-(GLuint) getTextureWithSize:(CGSize)size cropSize:(CGSize)cropSize scaleFactor:(CGFloat)scaleFactor backgroundColor:(UIColor*)bgColor drawingBlock:(void (^)(CGContextRef context))specificContent
{
    const int NUMBER_OF_COMPONENTS_PER_PIXEL = 4;
    CGFloat realWidth = size.width - cropSize.width;
    CGFloat realHeight = size.height - cropSize.height;
    
    size_t neededBytes = realWidth * realHeight * scaleFactor * scaleFactor * NUMBER_OF_COMPONENTS_PER_PIXEL;  // RGBA
    GLubyte* textureData = (GLubyte *)malloc(neededBytes);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    //NSUInteger bytesPerPixel = NUMBER_OF_COMPONENTS_PER_PIXEL;
    NSUInteger bytesPerRow = NUMBER_OF_COMPONENTS_PER_PIXEL * realWidth * scaleFactor;
    NSUInteger bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(textureData, realWidth*scaleFactor, realHeight*scaleFactor,
                                                 bitsPerComponent, bytesPerRow, colorSpace,
                                                 (CGBitmapInfo)kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
    
    if (bgColor != nil) {
        float colorComponents[4];
        [TextEffecter setRGBATo:&colorComponents[0] FromUIColor:bgColor];
        CGFloat colorComponentsInCG[4];
        colorComponentsInCG[0] = colorComponents[0];
        colorComponentsInCG[1] = colorComponents[1];
        colorComponentsInCG[2] = colorComponents[2];
        colorComponentsInCG[3] = 0.0;

        UIColor* tempC = [UIColor colorWithRed:(CGFloat)colorComponents[0] green:(CGFloat)colorComponents[1] blue:(CGFloat)colorComponents[2] alpha:1.0];
        CGContextSetFillColorWithColor(context,tempC.CGColor);
        CGContextFillRect(context, CGRectMake(0,0,realWidth*scaleFactor,realHeight*scaleFactor));

        /* I (==Eric) can't find the way to fill with textcolor(alpha=0).
           So, I filled with textcolor(alpha=1) and set the alpha value to 0 with the following loop.
         */
        for ( int i = 0 ; i < neededBytes/4 ; i++ ) {
            textureData[i*4+3] = 0;
        }
    } else {
        CGContextClearRect(context, CGRectMake(0,0,realWidth*scaleFactor,realHeight*scaleFactor));
    }
    
    CGContextScaleCTM(context, scaleFactor, scaleFactor);

    CGFloat dx = 0.;
    switch ( self.horizontalAlign ) {
        case TEHorizontalAlignRight:    dx = -cropSize.width;   break;
        case TEHorizontalAlignCenter:   dx = -cropSize.width/2.;break;
        default:                        dx = 0.;                break;
    }
    
    CGFloat dy = 0.;
    switch ( self.verticalAlign ) {
        case TEVerticalAlignBottom :    dy = 0.;                    break;
        case TEVerticalAlignCenter :    dy = - cropSize.height/2.;  break;
        default :                       dy = - cropSize.height;     break;
    }
    
    CGContextTranslateCTM(context, dx, dy);
    
    specificContent(context);
    
    
    //자원 해제
    CGColorSpaceRelease(colorSpace);
    CGContextRelease(context);
    
    //데이터 버퍼를 OpenGL 텍스쳐로 생성한다.
    GLuint textureID;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &textureID);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //CIFilter* _coreFilter = [CIFilter filterWithName:@"CIColorInvert"];
    //[_coreFilter setDefaults];
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, realWidth*scaleFactor, realHeight*scaleFactor, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    
    free(textureData);
    
    return textureID;
}

+ (CGRect)extendRect:(CGRect)source by:(CGFloat)gap {
    return CGRectMake(source.origin.x - gap,
                      source.origin.y - gap,
                      source.size.width + gap*2.0,
                      source.size.height + gap*2.0);
}

// Knowledge
// font.ascender + font.descender == font.lineHeight

//
- (CGRect)getTex1SizeWithGlow:(bool)glow shadow:(bool)shadow {
//    CGSize retValue = [self getTextAreaSize];
    
    CGRect textRect = CGRectMake(0.,0.,textAreaSize.width+2,textAreaSize.height+2);
    //CGFloat fontPointSizeWithStroke = self.font.pointSize * ( 1.0 + ( self.useStroke ? self.strokeThickness/100.:0.) * 0.666 /*2/3*/ );
    CGFloat fontPointSizeWithStroke = self.font.pointSize * ( 1.0 + ( self.useStroke ? self.strokeThickness/100.:0.) * (self.font.ascender / self.font.lineHeight) );

    // Outer Stroke
    // TODO: fontPointSizeWithStroke should be reconsidered.
    
    CGFloat extendedSize4Stroke = ceil(fontPointSizeWithStroke - self.font.pointSize);
    textRect = [TextEffecter extendRect:textRect by:extendedSize4Stroke];

    // Outer Glow
    CGFloat extendedSize4Glow = ceil((glow && self.useRealGlow) ? [TextEffectProperty convertSizeWith:self.glowSize fontPointSize:fontPointSizeWithStroke] + 2 : 0.);
    CGRect glowRect = [TextEffecter extendRect:textRect by:extendedSize4Glow];
    
    // Drop Shadow
    CGFloat extendedSize4Shadow = ceil((shadow && self.useRealShadow) ? [TextEffectProperty convertSizeWith:self.shadowSize fontPointSize:fontPointSizeWithStroke] + 2 : 0.);
    CGRect shadowRect = [TextEffecter extendRect:textRect by:extendedSize4Shadow];

    
    CGRect effectRect = CGRectUnion(glowRect, shadowRect);
//    CGRect layerRect = CGRectUnion(textRect,effectRect);
    
    effectRect.origin.x = ceil(effectRect.origin.x);
    effectRect.origin.y = ceil(effectRect.origin.y);
    effectRect.size.width = ceil(effectRect.size.width);
    effectRect.size.height = ceil(effectRect.size.height);
    //layerSize = layerRect.size;
    //heightDiffBetweenTextAndLayer = layerRect.size.height - textRect.size.height;
    
    return effectRect;
}

void dataProviderReleaseDataCallback(void *info, const void *data, size_t size)
{
    NexLayerStylerForReadingPixels* ls = static_cast<NexLayerStylerForReadingPixels*>(info);
    ls->freeResultRGBABuffer();
    
    free((void*)data);
}

+ (UIImage*)getUIImageFromRGBABuffer:(uint8_t*)buff width:(int)width height:(int)height scaleFactor:(CGFloat)scaleFactor
{
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buff, width*height*4*scaleFactor*scaleFactor, NULL);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef imageRef = CGImageCreate(width*scaleFactor, height*scaleFactor, 8, 32, width*4*scaleFactor, colorSpace, kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast, provider, NULL, NO, kCGRenderingIntentDefault);
    
    void* pImageData = malloc(width * height * 4 * scaleFactor * scaleFactor);

    CGContextRef contextRef = CGBitmapContextCreate(pImageData,
                                                    width * scaleFactor,
                                                    height * scaleFactor,
                                                    8/* bitsPerComponet */,
                                                    4*width*scaleFactor/* bytesPerRow */,
                                                    colorSpace,
                                                    kCGImageAlphaPremultipliedLast);
    
//    CGContextTranslateCTM(contextRef, 0, height);
    CGContextScaleCTM(contextRef, scaleFactor, scaleFactor);
    CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), imageRef);

    CGImageRef newImageRef = CGBitmapContextCreateImage(contextRef);

    //UIImage *retImage = [UIImage imageWithCGImage:newImageRef];
    
    
    
    UIImage *retImage = [UIImage imageWithCGImage:newImageRef scale:scaleFactor orientation:UIImageOrientationUp];

//    retImage.imageOrientation
    CFRelease(newImageRef);
    
    CGContextRelease(contextRef);
    free(pImageData);

    CGImageRelease(imageRef);
    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);

    [retImage autorelease];
    return retImage;
}

- (NSArray<TextEffecterResultInfo*>*)pseudoDrawText
{
    if ( !texts || texts.count <= 0 ) return nil;

    CGFloat scaleFactor = [UIScreen mainScreen].scale;

    [self adjustProperties];
    [self calculatePositions];
    [self calculateLayerSize];

    CGFloat xDiff = 0, yDiff = 0;
    {
        CGRect maxRect = [self getTex1SizeWithGlow:true shadow:true];
        if ( maxRect.size.width * scaleFactor > MAX_DEVICE_TEXTURE_SIZE || maxRect.size.height * scaleFactor > MAX_DEVICE_TEXTURE_SIZE ) {
            xDiff = ceil(maxRect.size.width - MYMIN(maxRect.size.width, MAX_DEVICE_TEXTURE_SIZE/scaleFactor));
            yDiff = ceil(maxRect.size.height - MYMIN(maxRect.size.height, MAX_DEVICE_TEXTURE_SIZE/scaleFactor));
        }
    }

    NSArray *retArray = nil;
    @autoreleasepool {
        // For 1st Texture
        TextEffecterResultInfo * ti1 = [[[TextEffecterResultInfo alloc] init] autorelease];
        {
            CGRect rect = [self getTex1SizeWithGlow:true shadow:false];
    //        CGFloat xd = 1. - MYMIN(rect.origin.x, 0.);// - textRect.origin.x;
    //        CGFloat yd = 1. - MYMIN(rect.origin.y, 0.);// - textRect.origin.y;
    //        CGPoint textOrigin = CGPointMake(xd, yd);
            
            //ti1.rect = rect;
            ti1.rect = CGRectMake( rect.origin.x, rect.origin.y,
                                   rect.size.width - xDiff, rect.size.height - yDiff);
            ti1.drawOrder = 0;
            ti1.isMainText = true;
        }
        // For 2nd Texture
        TextEffecterResultInfo * ti2 = nil;
        if ( self.useRealShadow ) {
            ti2 = [[[TextEffecterResultInfo alloc] init] autorelease];

            CGRect rect = [self getTex1SizeWithGlow:false shadow:true];
    //        CGFloat xd = 1. - MYMIN(rect.origin.x, 0.);// - textRect.origin.x;
    //        CGFloat yd = 1. - MYMIN(rect.origin.y, 0.);// - textRect.origin.y;
    //        CGPoint textOrigin = CGPointMake(xd, yd);
            CGSize offset = [self calculateShadowOffsetWithInternalProperties];
            
            CGRect adjustedRect = CGRectMake( rect.origin.x, rect.origin.y,
                                              rect.size.width - xDiff, rect.size.height - yDiff);

            CGRect shadowRect = CGRectOffset(adjustedRect, offset.width, +offset.height);

            //CGRect shadowRect = CGRectOffset(rect, offset.width, +offset.height);

            ti2.rect = shadowRect;

            ti1.drawOrder = 1;
            ti2.drawOrder = 0;
            ti2.isMainText = false;
        }

        NSMutableArray* ret = [[[NSMutableArray alloc] init] autorelease];
        [ret addObject:ti1];
        if ( nil != ti2 ) [ret addObject:ti2];
        //texes[0] = texName;

        retArray = [ret copy];
    }
    [retArray autorelease];

    return retArray;
}

- (NSArray<TextEffecterResult*>*)drawText {

    if ( !texts || texts.count <= 0 ) return nil;

    CGFloat scaleFactor = [UIScreen mainScreen].scale;
    
    EAGLContext* oldc = [EAGLContext currentContext];
    if ( oldc != nil )
        [oldc retain];
    [EAGLContext setCurrentContext:eaglContext];
    
    [self adjustProperties];
    [self calculatePositions];
    [self calculateLayerSize];
    
    
    //NSLog(@"Eric Color F:%@,O:%@",self.textColor,self.strokeColor);
    
    CGFloat xDiff = 0, yDiff = 0;
    {
        CGRect maxRect = [self getTex1SizeWithGlow:true shadow:true];
        if ( maxRect.size.width * scaleFactor > MAX_DEVICE_TEXTURE_SIZE || maxRect.size.height * scaleFactor > MAX_DEVICE_TEXTURE_SIZE ) {
            xDiff = ceil(maxRect.size.width - MYMIN(maxRect.size.width, MAX_DEVICE_TEXTURE_SIZE/scaleFactor));
            yDiff = ceil(maxRect.size.height - MYMIN(maxRect.size.height, MAX_DEVICE_TEXTURE_SIZE/scaleFactor));
        }
    }
    
    // For 1st Texture
    TextEffecterResult * ti1 = [[[TextEffecterResult alloc] init] autorelease];

    {
        CGRect rect = [self getTex1SizeWithGlow:true shadow:false];
        CGFloat xd = 1. - MYMIN(rect.origin.x, 0.);// - textRect.origin.x;
        CGFloat yd = 1. - MYMIN(rect.origin.y, 0.);// - textRect.origin.y;
        CGPoint textOrigin = CGPointMake(xd, yd);
        
        
        if ( self.useRealGlow ) {
            GLuint texName = -1;
            float colorComponents[4];
            [TextEffecter setRGBATo:&colorComponents[0] FromUIColor:self.glowColor];
            CGFloat colorComponentsInCG[4];
            colorComponentsInCG[0] = colorComponents[0];
            colorComponentsInCG[1] = colorComponents[1];
            colorComponentsInCG[2] = colorComponents[2];
            colorComponentsInCG[3] = 0.0;

            UIColor* tempC = [UIColor colorWithRed:(CGFloat)colorComponents[0] green:(CGFloat)colorComponents[1] blue:(CGFloat)colorComponents[2] alpha:0.0];

            texName = [self getTextureWithSize:rect.size cropSize:CGSizeMake(xDiff, yDiff) scaleFactor:scaleFactor backgroundColor:self.glowColor drawingBlock:^(CGContextRef context) {
//                CGContextSaveGState(context);
//                CGContextSetAlpha(context, 0.0);
////
//                CGContextSetFillColorWithColor(context, tempC.CGColor);
////                CGContextSetFillColor(context,&colorComponentsInCG[0]);
//                CGContextFillRect(context, CGRectMake(0,0,(rect.size.width-xDiff)*scaleFactor,(rect.size.height-yDiff)*scaleFactor));
//                CGContextFillRect(context, CGRectMake(0,(rect.size.height-yDiff)*scaleFactor,(rect.size.width-xDiff)*scaleFactor,(rect.size.height-yDiff)*scaleFactor));
//                CGContextFillRect(context, CGRectMake(0,-(rect.size.height-yDiff)*scaleFactor,(rect.size.width-xDiff)*scaleFactor,(rect.size.height-yDiff)*scaleFactor));
//                CGContextRestoreGState(context);

                [self drawWithContext:context offset:textOrigin antiAlias:true];
//                [self drawWithContext:context offset:textOrigin antiAlias:false];
            }];

//                            [tempC autorelease];
//            [tempC release];

            //NSLog(@"Eric T1 w/h is %f, %f\n", rect.size.width, rect.size.height);

            DropShadowParameters dsp;
            dsp.distance = 0.;
            dsp.size = [TextEffectProperty convertSizeWith:self.glowSize fontPointSize:self.font.pointSize];
            dsp.spread = [TextEffectProperty convertSpreadWith:self.glowSpread];
            dsp.angle = 90.;
            
            [TextEffecter setRGBATo:&dsp.shadowColor[0] FromUIColor:self.glowColor];

            layerStyler->setSourceTex(texName, (rect.size.width - xDiff) * scaleFactor, (rect.size.height - yDiff) * scaleFactor);
            layerStyler->setParam(NexLayerStyler::CMD_DROPSHADOW, &dsp, sizeof(dsp));
            layerStyler->makeResult(NexLayerStyler::CMD_DROPSHADOW);
            
            ti1.image = [TextEffecter getUIImageFromRGBABuffer:layerStyler->getResultRGBABuffer() width:(rect.size.width - xDiff) height:(rect.size.height - yDiff) scaleFactor:scaleFactor];
            layerStyler->freeResultRGBABuffer();

            glDeleteTextures(1, &texName);

        } else {
            CGSize realSize = CGSizeMake(rect.size.width - xDiff, rect.size.height - yDiff);
            UIGraphicsBeginImageContextWithOptions(realSize, NO, scaleFactor);
            
            CGContextRef context = UIGraphicsGetCurrentContext();

            CGFloat dx = 0.;
            switch ( self.horizontalAlign ) {
                case TEHorizontalAlignRight:    dx = -xDiff;    break;
                case TEHorizontalAlignCenter:   dx = -xDiff/2.; break;
                default:                        dx = 0.;        break;
            }
            
            CGFloat dy = 0.;
            switch ( self.verticalAlign ) {
                case TEVerticalAlignBottom :    dy = rect.size.height - yDiff;      break;
                case TEVerticalAlignCenter :    dy = rect.size.height - yDiff/2.;   break;
                default :                       dy = rect.size.height;              break;
            }
            
            
            CGContextTranslateCTM(context, dx, dy);
            CGContextScaleCTM(context, 1.0, -1.0);

            [self drawWithContext:context offset:textOrigin antiAlias:true];
            
            UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
            
            //[image retain];
            [image autorelease];
            
            UIGraphicsEndImageContext();

            ti1.image = image;
        }


        ti1.rect = CGRectMake( rect.origin.x, rect.origin.y,
                               rect.size.width - xDiff, rect.size.height - yDiff);
        ti1.drawOrder = 0;
        ti1.isMainText = true;
    }
    
    // For 2nd Texture
    TextEffecterResult * ti2 = nil;
    if ( self.useRealShadow ) {
        ti2 = [[[TextEffecterResult alloc] init] autorelease];

        CGRect rect = [self getTex1SizeWithGlow:false shadow:true];
        CGFloat xd = 1. - MYMIN(rect.origin.x, 0.);// - textRect.origin.x;
        CGFloat yd = 1. - MYMIN(rect.origin.y, 0.);// - textRect.origin.y;
        CGPoint textOrigin = CGPointMake(xd, yd);

        {
            CGRect rect2 = [self getTex1SizeWithGlow:self.useRealGlow shadow:false];
            rect.origin.x += -rect2.origin.x;
            rect.origin.y += -rect2.origin.y;
        }
        
        GLuint texName = -1;

        texName = [self getTextureWithSize:rect.size cropSize:CGSizeMake(xDiff, yDiff) scaleFactor:scaleFactor backgroundColor:nil drawingBlock:^(CGContextRef context) {
            [self drawWithContext:context offset:textOrigin antiAlias:true];
        }];

        DropShadowParameters dsp;
        dsp.angle = 90;//self.shadowAngle;
        dsp.distance = 0;//transParamDistance(self.shadowDistance, self.font.pointSize);
        dsp.size = [TextEffectProperty convertSizeWith:self.shadowSize fontPointSize:self.font.pointSize];
        dsp.spread = [TextEffectProperty convertSpreadWith:self.shadowSpread];

        [TextEffecter setRGBATo:&dsp.shadowColor[0] FromUIColor:self.shadowColor];

        layerStyler->setSourceTex(texName, (rect.size.width - xDiff)*scaleFactor, (rect.size.height-yDiff)*scaleFactor);
        layerStyler->setParam(NexLayerStyler::CMD_DROPSHADOW_ONLY_SHADOW, &dsp, sizeof(dsp));
        layerStyler->makeResult(NexLayerStyler::CMD_DROPSHADOW_ONLY_SHADOW);

        ti2.image = [TextEffecter getUIImageFromRGBABuffer:layerStyler->getResultRGBABuffer() width:(rect.size.width-xDiff) height:(rect.size.height-yDiff) scaleFactor:scaleFactor];
        layerStyler->freeResultRGBABuffer();

        //ti2.image.scale = 2.0;
        CGSize offset = [self calculateShadowOffsetWithInternalProperties];
        CGRect adjustedRect = CGRectMake( rect.origin.x, rect.origin.y,
                                          rect.size.width - xDiff, rect.size.height - yDiff);

        CGRect shadowRect = CGRectOffset(adjustedRect, offset.width, +offset.height);

        ti2.rect = shadowRect;
        
        glDeleteTextures(1, &texName);
        
        ti1.drawOrder = 1;
        ti2.drawOrder = 0;
        ti2.isMainText = false;
    }
    
    [EAGLContext setCurrentContext:oldc];
    if ( oldc != nil )
        [oldc release];

    NSMutableArray* ret = [[[NSMutableArray alloc] init] autorelease];
    [ret addObject:ti1];
    if ( nil != ti2 ) [ret addObject:ti2];
    //texes[0] = texName;

    NSArray *retArray = [ret copy];
    [retArray autorelease];
    
    return retArray;
}

- (void)drawWithContext:(CGContextRef)context offset:(CGPoint)offset  antiAlias:(bool)antiAlias {
    if ( !texts || texts.count <= 0 ) return;
    
    [self calculateIfNeed];
    
    CFDictionaryRef fontAttributesRef = [TextEffecter createDictionaryForFontAttributesWithFont:self.font fontColor:self.textColor kerning:self.realKerning underline:self.useUnderline strokeThickness:(self.useStroke)?self.strokeThickness:0.0 strokeColor:self.strokeColor];
    
    
    @synchronized (self) {
        CGContextSaveGState(context);
        CGContextSetAllowsAntialiasing(context, antiAlias);
        CGContextSetLineJoin(context,kCGLineJoinRound);
        //CGContextSetMiterLimit(context, 2.0);

        [self drawWithContext:context offset:offset fontAttributes:fontAttributesRef];

        if ( self.useStroke && 0.0 != self.strokeThickness ) {
            CFDictionaryRef fontAttributesRef2 = [TextEffecter createDictionaryForFontAttributesWithFont:self.font fontColor:self.textColor kerning:self.realKerning underline:self.useUnderline strokeThickness:0.0 strokeColor:self.strokeColor];

            CGContextSetAllowsAntialiasing(context, true);
            CGContextSetBlendMode(context, kCGBlendModeCopy);
            [self drawWithContext:context offset:offset fontAttributes:fontAttributesRef2];

            [TextEffecter deleteDictionaryForFontAttributesWithFontColor:fontAttributesRef2];
        }
        
        CGContextRestoreGState(context);
    }
    
    [TextEffecter deleteDictionaryForFontAttributesWithFontColor:fontAttributesRef];

}


- (void)drawWithContext:(CGContextRef)context offset:(CGPoint)offset fontAttributes:(CFDictionaryRef)fontAttributes {
    
    int lines = (int)texts.count;
    for( int i = 0 ; i < lines; i++ ) {
        OneText* atext = texts[i];
        
        CFStringRef stringRef = CFStringCreateWithCString(NULL, [atext.text UTF8String], kCFStringEncodingUTF8);
        CFAttributedStringRef attributedStringRef = CFAttributedStringCreate(NULL, stringRef, fontAttributes);
        CFRelease(stringRef);
        
        CTLineRef lineRef = CTLineCreateWithAttributedString(attributedStringRef);
        CFRelease(attributedStringRef);
        
        
        
        CGContextSetTextPosition(context, atext.origin.x + offset.x, /*heightDiffBetweenTextAndLayer +*/ atext.origin.y + offset.y);
        
        //NSLog(@"Eric T1 %d's pos is %f, %f\n", i, atext.origin.x + offset.x, /*heightDiffBetweenTextAndLayer +*/ atext.origin.y + offset.y);
        CTLineDraw(lineRef, context);
        CFRelease(lineRef);
    }
}

// for embossing
- (void)tempDraw {
#if 0
    // 1. Make mask image.
    UIImage *maskImage = nil;
    {
        KMTextLayerProperty* propertiesForMask = [KMTextLayerProperty newInstance];
        propertiesForMask.useShadow = NO;
        propertiesForMask.useGlow = NO;
        propertiesForMask.useOutline = NO;
        
        maskImage = [self imageFromTexts:textArray withFont:font withProperty:propertiesForMask withOpaque:NO];
        
        [propertiesForMask release];
    }
    
    // 2. Make HeightField image
    CIImage* heightImage = nil;
    {
        
        CIImage* tempCIImage = [CIImage imageWithCGImage:maskImage.CGImage];
        CIFilter* heightFilter = [CIFilter filterWithName:@"CIHeightFieldFromMask"];
        [heightFilter setValue:tempCIImage forKey:kCIInputImageKey];
        [heightFilter setValue:[NSNumber numberWithFloat:20.0] forKey:kCIInputRadiusKey];
        heightImage = [heightFilter outputImage];
        
        [tempCIImage release];
        [heightFilter release];
    }
    
    // 3.
    CIImage* shadedResultImage = nil;
    {
        UIImage* shadedImage = [UIImage imageNamed:@"semisphere3.png"];
        CIImage* tempCIImage = [CIImage imageWithCGImage:shadedImage.CGImage];
        
        CIFilter* shadedMaterialFilter = [CIFilter filterWithName:@"CIShadedMaterial"];
        [shadedMaterialFilter setValue:heightImage forKey:kCIInputImageKey];
        [shadedMaterialFilter setValue:tempCIImage forKey: @"inputShadingImage"];
        
        shadedResultImage = [shadedMaterialFilter outputImage];
        
        if ( shadedImage )
            [shadedImage release];
    }
    
    if ( shadedResultImage )
        [shadedResultImage release];
    
    
    if ( heightImage )
        [heightImage release];
    
    if ( maskImage )
        [maskImage release];
#endif
    
}

@end

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
-------------------------------------------------------------------------------
 Eric        2019/05/28    Draft.
 Eric        2019/09/16    Change Draw Method (using opengl shader ).
 Eric        2019/10/24    Revised.
-----------------------------------------------------------------------------*/
