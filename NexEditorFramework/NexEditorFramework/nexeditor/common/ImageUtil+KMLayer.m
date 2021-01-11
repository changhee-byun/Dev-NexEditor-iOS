/**
 * File Name   : ImageUtil+KMLayer.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#import "ImageUtil+KMLayer.h"
#import <CoreText/CoreText.h>

@implementation ImageUtil (KMLayer)

+ (UIImage *)imageFromText:(NSString*)text withFont:(UIFont*)font withProperty:(KMTextLayerProperty*) property withSize:(CGSize)size{
    int width = size.width;
    int height = size.height;
    
    //텍스트 크기를 측정
    CGSize textSize = CGSizeZero;
    textSize = [text sizeWithAttributes:@{NSFontAttributeName:font}];
    
    UIGraphicsBeginImageContextWithOptions(size, NO, [[UIScreen mainScreen] scale]);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    // transforming context
    CGContextTranslateCTM(context, 0.0, size.height);
    CGContextScaleCTM(context, 1.0, -1.0);
    
    float x = (width - textSize.width) / 2.0;
    float y = (height - textSize.height) / 2.0 - font.descender;
    KMText* textInfo = [[KMText alloc] initWithText:text Rect:CGRectMake(x, y, width, height)];
    [self drawText:context withFont:font withText:@[textInfo] withProperty:property];
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    
    UIGraphicsEndImageContext();
    
    return image;
}

//+(GLuint) glTextureIDfromText:(NSString*)text withFont:(UIFont*)font

+(GLuint) glTextureIDfromText:(NSArray*)textArray withFont:(UIFont*)font withProperty:(KMTextLayerProperty *)property
{
    if(!textArray || textArray.count <= 0) return 0;
    //전체 텍스트의 크기를 가져온다.
    KMText* text = [textArray objectAtIndex:0];
    CGRect rect = text.rect;
    int width = rect.size.width;
    int height =rect.size.height;
    
    //텍스쳐를 생성한다.
    return [self getTextureIDWithHeight:height width:width drawingBlock:^(CGContextRef context)
            {
                //CGContext 에 텍스트를 그려서 데이터를 채운다.
                [self drawText:context withFont:font withText:textArray withProperty:property ];
            }];
}

+(UIImage *)imageFromTexts:(NSArray*)textArray withFont:(UIFont*)font withProperty:(KMTextLayerProperty *)property withOpaque:(BOOL) isOpaque
{
    if(!textArray || textArray.count <= 0) return 0;
    //전체 텍스트의 크기를 가져온다.

    KMText* text = [textArray objectAtIndex:0];
    CGRect rect = text.rect;
    int width = rect.size.width;
    int height =rect.size.height;
    
    CGFLOAT_TYPE check1 = [[UIScreen mainScreen] scale];
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(width,height), isOpaque, 1.0 );
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    // transforming context
    CGContextTranslateCTM(context, 0.0, height);
    CGContextScaleCTM(context, 1.0, -1.0);

    [self drawText2:context withFont:font withText:textArray withProperty:property];
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

+(void) drawText:(CGContextRef)ctx withFont:(UIFont*)font withText:(NSArray*)textArray withProperty:(KMTextLayerProperty *)property
{
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
    
    [self drawText2:ctx withFont:font withText:textArray withProperty:property];
}

+(void) drawText2:(CGContextRef)ctx withFont:(UIFont*)font withText:(NSArray*)textArray withProperty:(KMTextLayerProperty *)property
{
    if(!textArray || textArray.count <= 0) return;
    //*왼쪽 아래가 0,0으로 된다.
    
    {
//        [ImageUtil imageFromText
        
//        NSData* pnsdata = [NSData dataWithBytesNoCopy:textureData length:size];
//
//        CIImage* inputImage = [CIImage imageWithBitmapData:pnsdata bytesPerRow:bytesPerRow size:CGSizeMake(width, height) format:kCIFormatRGBA8 colorSpace:colorSpace];
//
//        CIFilter* heightFilter = [CIFilter filterWithName:@"CIHeightFieldFromMask"];
//        //CIFilter* shadingFilter = [CIFilter filterWithName:@"CIShadedMaterial"];
//
//        [heightFilter setValue:inputImage forKey:kCIInputImageKey];
//        [heightFilter setValue:[NSNumber numberWithFloat:10.0] forKey:kCIInputRadiusKey];
//        CIImage* heightImage = [heightFilter outputImage];
//
//
//        CIContext *cic = [CIContext contextWithOptions:nil];
//        CGImageRef cgImage = [cic createCGImage:heightImage fromRect:[heightImage extent]];
//
//        UIImage* ptt = [UIImage imageWithCGImage:cgImage];
//        CGImageRelease(cgImage);
//
//        CGImageRef imageRef = [ptt CGImage];
//
//        CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
//        [cic release];
        
    }
    
    
    
    
    
    
    //CTParagraphStyleRef
    
    //폰트 설정
    float fontSize = font.pointSize;
    CFStringRef fontName = CFStringCreateWithCString(NULL, [[font fontName] UTF8String], kCFStringEncodingUTF8);
    CTFontRef fontRef = CTFontCreateWithName(fontName, fontSize, NULL);
    //kCTParagraphStyleAttributeName
    //폰트 속성 키
    CFStringRef keys[] = {
        kCTKernAttributeName,
        kCTParagraphStyleAttributeName,
        kCTFontAttributeName,
        kCTForegroundColorAttributeName,
        kCTStrokeWidthAttributeName,
        kCTStrokeColorAttributeName
    };
    
    //외곽선 설정
#if 1
    float outlineWidth = -5.0;
    if ( property.useOutline ) {
        KMText* text_ = [textArray objectAtIndex:0]; //라인변로 텍스트를 가져옴
        CGRect rect_ = text_.rect;        //텍스트의 위치
        NSString* str_ = text_.text;      //텍스트 내용
        
        unichar ch0 = [str_ characterAtIndex:0];
        unichar ch1 = [str_ characterAtIndex:1];
        float signn = 0.0;
        if ( ch0 == '+' )
            signn = 1.0;
        else if ( ch0 == '-' )
            signn = -1.0;
        
        if ( signn != 0.0 ) {
            int n = ch1 - '0';
            outlineWidth = signn * (float)n;
        }
    }
    else
        outlineWidth = 0.0;
#else
    float outlineWidth = property.useOutline ? -1:0;
#endif
    CFNumberRef outline = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &outlineWidth);
    
    // kerning
    float kerning_ = property.kerningRatio;
    CFNumberRef kerning = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &kerning_ );
    
    // horizontal align
    //NSMutableParagraphStyle *paragraphStyle = NSMutableParagraphStyle.new;
    //paragraphStyle.alignment                = property.horizontalAlign;
    
    CTTextAlignment alignment = property.horizontalAlign == NSTextAlignmentCenter ? kCTTextAlignmentCenter : (property.horizontalAlign == NSTextAlignmentRight ? kCTTextAlignmentRight : kCTTextAlignmentLeft);
    
    CTParagraphStyleSetting alignmentSetting;
    alignmentSetting.spec = kCTParagraphStyleSpecifierAlignment;
    alignmentSetting.valueSize = sizeof(CTTextAlignment);
    alignmentSetting.value = &alignment;
    
    CTParagraphStyleSetting settings[1] = {alignmentSetting};
    
    size_t settingsCount = 1;
    CTParagraphStyleRef paragraphRef = CTParagraphStyleCreate(settings, settingsCount);
    
    //폰트 속성 값
    CFTypeRef values[] = {
        kerning,
        paragraphRef,
        fontRef,
        property.textColor.CGColor,
        outline,
        property.outlineColor.CGColor
    };
    
    //폰트 속성 생성
    CFDictionaryRef fontAttributes = CFDictionaryCreate(kCFAllocatorDefault,
                                                        (const void **)&keys,
                                                        (const void **)&values,
                                                        sizeof(keys) / sizeof(keys[0]),
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
    //자원 해제
    CFRelease(kerning);
    CFRelease(fontName);
    CFRelease(fontRef);
    CFRelease(outline);
    
    //안티 알리아싱
    CGContextSetAllowsAntialiasing(ctx, true);
    
    //Context 상태 저장.
    CGContextSaveGState(ctx);
    
    //그림자 설정
    if(property.useShadow)
        CGContextSetShadowWithColor(ctx, property.shadowOffset, font.pointSize/property.shadowBlurRadiusDivisor, property.shadowColor.CGColor);
    
    
    //텍스트 드로잉
    int cnt = (int)textArray.count;
    for(int i=0; i<cnt; i++) {
        KMText* text = [textArray objectAtIndex:i]; //라인변로 텍스트를 가져옴
        CGRect rect = text.rect;        //텍스트의 위치
        NSString* str = text.text;      //텍스트 내용
        
        //문자열 정보
        CFStringRef string = CFStringCreateWithCString(NULL, [str UTF8String], kCFStringEncodingUTF8);
        
        //폰트가 적용된 문자열
        CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, fontAttributes);
        CFRelease(string);
        
        //1라인 텍스트 생성
        CTLineRef line = CTLineCreateWithAttributedString(attr_string);
        CFRelease(attr_string);
        
        CGContextSetTextPosition(ctx, rect.origin.x, rect.origin.y);    //텍스트 포지션
        CTLineDraw(line, ctx);                                          //텍스트 드로잉
        CFRelease(line);
    }
    CGContextRestoreGState(ctx);    //context 상태 되돌리기
    
    //glow 효과 적용
    if(property.useGlow)
        CGContextSetShadowWithColor(ctx, CGSizeMake(0, 0), font.pointSize/property.glowBlurRadiusDivisor, property.glowColor.CGColor);
    //1번 더 텍스트 드로잉
    
    //test-s
    cnt = 1;
    //test-e
    for(int i=0; i<cnt; i++) {
        KMText* text = [textArray objectAtIndex:i];
        CGRect rect = text.rect;
        NSString* str = text.text;
        
        CFStringRef string = CFStringCreateWithCString(NULL, [str UTF8String], kCFStringEncodingUTF8);
        
        CFAttributedStringRef attr_string = CFAttributedStringCreate(NULL, string, fontAttributes);
        CFRelease(string);
        
        CTLineRef line = CTLineCreateWithAttributedString(attr_string);
        CFRelease(attr_string);
        
        CGContextSetTextPosition(ctx, rect.origin.x, rect.origin.y);
        CTLineDraw(line, ctx);
        CFRelease(line);
    }
    
    CFRelease(fontAttributes);
}

@end
