/**
 * File Name   : ThemeResourceLoader.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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
#import "ThemeResourceLoader.h"
//#import "AssetLibraryProxy.h"
#import "NSString+ValueReader.h"
#import "NSString+Color.h"
#import "UIImage+Blending.h"
#import "UIImage+Resize.h"
#import "FontTools.h"

typedef NS_ENUM(NSUInteger, TextFlags) {
    NE_TEXTFLAG_BOLD = 0x00000001,
    NE_TEXTFLAG_ITALIC = 0x00000002,
    NE_TEXTFLAG_FILL = 0x00000004,
    NE_TEXTFLAG_STROKE = 0x00000008,
    NE_TEXTFLAG_UNDERLINE = 0x00000010,
    NE_TEXTFLAG_STRIKE = 0x00000020,
    NE_TEXTFLAG_HINTING = 0x00000040,
    NE_TEXTFLAG_SUBPIXEL = 0x00000080,
    NE_TEXTFLAG_SHADOW = 0x00000100,
    NE_TEXTFLAG_LINEAR = 0x00000200,
    NE_TEXTFLAG_AUTOSIZE = 0x00000400,
    NE_TEXTFLAG_CUTOUT = 0x00000800,
    NE_TEXTFLAG_STROKEBACK = 0x00001000
};

typedef NS_ENUM(NSUInteger, AlignTypes) {
    NE_ALIGN_LEFT = 0x0,
    NE_ALIGN_CENTER = 0x1,
    NE_ALIGN_RIGHT = 0x2,
    NE_ALIGN_MASK = 0xF
};

typedef NS_ENUM(NSUInteger, VAlignTypes) {
    NE_VALIGN_TOP = 0x0,
    NE_VALIGN_CENTER = 0x10,
    NE_VALIGN_BOTTOM = 0x20,
    NE_VALIGN_MASK = 0xF0
};

typedef NS_ENUM(NSUInteger, BlueTypes) {
    NE_BLUR_NORMAL = 0,
    NE_BLUR_SOLID,
    NE_BLUR_INNER,
    NE_BLUR_OUTER
};


@implementation ThemeResourceLoader

+ (CGImageRef )createImage4Theme:(NSString *)path
                          config:(NexImageLoaderConfig)config
                loadResourceData:(NSData *(^)(NSString *  itemId, NSString *  subpath)) loadResourceData
{
    CGImageRef result = NULL;
    NSArray *charSets = [[path substringFromIndex:[@"[ThemeImage]" length]]
                         componentsSeparatedByCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"/"]];
    if([charSets count] > 0) {
        NSString *base_id = [charSets objectAtIndex:0];
        NSString *rel_path = [charSets objectAtIndex:1];
        
        NSData *data = loadResourceData(base_id, rel_path);

        if ( data ) {
            UIImage *uiimage = [UIImage imageWithData:data];
            const float rate = config.subsampleThemeResourceRate;
            if (rate != SUBSAMPLE_RATE_NONE &&
                ((uiimage.size.width > config.subsampleThemeResourceMinSize.width)
                 || (uiimage.size.height > config.subsampleThemeResourceMinSize.height)))
            {
                uiimage = [uiimage resizing:CGSizeMake(uiimage.size.width * rate, uiimage.size.height * rate)];
            }
            
            CGImageRef image = uiimage.CGImage;
            if(image != NULL) {
                result = image;
            }
        }
    }
    return result;
}

+ (CGImageRef)createImage4Text:(NSString *)textinfo
{
    NSString *wholeUserText = @"";
    
    if([textinfo rangeOfString:@";;"].location != NSNotFound) {
        wholeUserText = [textinfo substringFromIndex:[textinfo rangeOfString:@";;"].location+2];
    }
    if(wholeUserText.length != 0) {
        NSString* str = [NSString stringWithFormat:@"%c", '\u001b'];
        NSUInteger escidx = [wholeUserText rangeOfString:str].location;
        if(escidx != NSNotFound) {
            wholeUserText = [wholeUserText substringFromIndex:escidx+1];
        }
    } else {
        wholeUserText = @"Title Text Goes Here";
    }
    
    NSUInteger startPos = [textinfo rangeOfString:@"[Text]"].length;
    NSUInteger endPos = [textinfo rangeOfString:@";;"].location - startPos;
    NSString *parseString = [textinfo substringWithRange:NSMakeRange(startPos, endPos)];
    
    NSArray *parseAsArray = [parseString componentsSeparatedByString:@";"];
    NSMutableDictionary *parseDictionary = [[NSMutableDictionary alloc] init];
    for(NSString *argument in parseAsArray) {
        NSArray *tmpAsArray = [argument componentsSeparatedByString:@"="];
        if([tmpAsArray count] < 2) {
            [parseDictionary setObject:@"" forKey:tmpAsArray[0]];
        } else {
            [parseDictionary setObject:tmpAsArray[1] forKey:tmpAsArray[0]];
        }
    }
    
    int flags_ = (int)[[parseDictionary objectForKey:@"flags"] scanHexLongLong];
    int align_ = [[parseDictionary objectForKey:@"align"] intValueOrDefault:0];
    int width_ = [[parseDictionary objectForKey:@"width"] intValueOrDefault:0];
    int height_ = [[parseDictionary objectForKey:@"height"] intValueOrDefault:0];
    int maxLines_ = [[parseDictionary objectForKey:@"maxlines"] intValueOrDefault:0];
    float margin_ = [[parseDictionary objectForKey:@"margin"] floatValueOrDefault:0.0];
    float skewx_ = [[parseDictionary objectForKey:@"skewx"] floatValueOrDefault:0.0];
    float scalex_ = [[parseDictionary objectForKey:@"scalex"] floatValueOrDefault:0.0];
    float size_ = [[parseDictionary objectForKey:@"size"] floatValueOrDefault:0.0];
    float strokewidth_ = [[parseDictionary objectForKey:@"strokewidth"] floatValueOrDefault:0.0];
    float shadowradius_ = [[parseDictionary objectForKey:@"shadowradius"] floatValueOrDefault:0.0];
    float shadowoffsx_ = [[parseDictionary objectForKey:@"shadowoffsx"] floatValueOrDefault:0.0];
    float shadowoffsy_ = [[parseDictionary objectForKey:@"shadowoffsy"] floatValueOrDefault:0.0];
    __unused float spacingmult = [[parseDictionary objectForKey:@"spacingmult"] floatValueOrDefault:0.0];
    __unused float spacingadd = [[parseDictionary objectForKey:@"spacingadd"] floatValueOrDefault:0.0];
    __unused float textblur = [[parseDictionary objectForKey:@"textblur"] floatValueOrDefault:0.0];
    __unused int blurtype = [[parseDictionary objectForKey:@"blurtype"] intValueOrDefault:0.0];
    NSString *bgColor = [parseDictionary objectForKey:@"bgcolor"];
    NSString *fillColor = [parseDictionary objectForKey:@"fillcolor"];
    NSString *strokeColor = [parseDictionary objectForKey:@"strokecolor"];
    NSString *shadowColor = [parseDictionary objectForKey:@"shadowcolor"];
    NSString *fmtstr = [self pdecode:[parseDictionary objectForKey:@"text"]];
    NSString *typeface = [parseDictionary objectForKey:@"typeface"];
    
    strokewidth_ /= 2;
    shadowradius_ /= 4;
    
    int valign_ = align_ & NE_VALIGN_MASK;
    align_ &= NE_ALIGN_MASK;
    
    NSArray *userText = @[wholeUserText];
    for(int i=0; i<3; i++) {
        if(i<[userText count]) {
            NSString *text = [userText objectAtIndex:i];
            NSString *front = text;
            NSString *back = @"";
            
            int length = (int)text.length;
            int dist = length;
            int newdist = 0;
            int mid = 0;
            
            for(int j=0; j<length; j++) {
                if([text characterAtIndex:j] == ' ') {
                    newdist = abs(length/2 - j);
                    if(newdist < dist) {
                        dist = newdist;
                        mid = j;
                    }
                }
            }
            
            if(mid>0 && (mid+1)<length) {
                front = [text substringWithRange:NSMakeRange(0, mid)];
                back = [text substringFromIndex:mid+1];
            }
            
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%", (i+1)] withString:text];
            fmtstr = [fmtstr = fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%f", (i+1)] withString:front];
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%b", (i+1)] withString:back];
            
            if(text.length == 1) {
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%s", (i+1)] withString:[text substringWithRange:NSMakeRange(0, 1)]];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!s", (i+1)] withString:@""];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%e", (i+1)] withString:@""];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!e", (i+1)] withString:@""];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%m", (i+1)] withString:@""];
            } else if(text.length < 1) {
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%s", (i+1)] withString:@""];
                [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!s", (i+1)] withString:@""];
                [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%e", (i+1)] withString:@""];
                [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!e", (i+1)] withString:@""];
                [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%m", (i+1)] withString:@""];
            } else {
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%s", (i+1)] withString:[text substringWithRange:NSMakeRange(0, 1)]];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!s", (i+1)] withString:[text substringFromIndex:1]];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%e", (i+1)] withString:[text substringFromIndex:length-1]];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!e", (i+1)] withString:[text substringWithRange:NSMakeRange(0, length-1)]];
                fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%m", (i+1)] withString:[text substringWithRange:NSMakeRange(1, length-1)]];
            }
        } else {
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%s", (i+1)] withString:@""];
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!s", (i+1)] withString:@""];
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%e", (i+1)] withString:@""];
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%!e", (i+1)] withString:@""];
            fmtstr = [fmtstr stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@%d", @"%m", (i+1)] withString:@""];
        }
    }
    
    NSMutableDictionary *textAttributes = [NSMutableDictionary dictionary];
    
    NSNumber *underlineInfo = [NSNumber numberWithInt:0];
    NSNumber *strikeInfo = [NSNumber numberWithInt:0];
    NSNumber *expansionInfo = [NSNumber numberWithFloat:0.0];
    NSNumber *skewInfo = [NSNumber numberWithFloat:0.0];
    NSShadow *shadowInfo = nil;
    UIColor *strokeColorInfo = [UIColor blackColor];
    NSNumber *strokeWidthInfo = [NSNumber numberWithFloat:0.0];
    UIColor *textColorInfo = [UIColor blackColor];
    
    if((flags_ & (NE_TEXTFLAG_STROKE | NE_TEXTFLAG_FILL)) == 0) {
        flags_ |= NE_TEXTFLAG_FILL;
    }
    
    if((flags_ & NE_TEXTFLAG_UNDERLINE) != 0) {
        underlineInfo = [NSNumber numberWithInt:1];
        [textAttributes setObject:underlineInfo forKey:NSUnderlineStyleAttributeName];
    }
    
    if((flags_ & NE_TEXTFLAG_STRIKE) != 0) {
        strikeInfo = [NSNumber numberWithInt:1];
        [textAttributes setObject:strikeInfo forKey:NSStrikethroughColorAttributeName];
    }
    
    if((flags_ & NE_TEXTFLAG_SHADOW) != 0) {
        shadowInfo = [[NSShadow alloc] init];
        shadowInfo.shadowColor = [shadowColor hexARGBColor];
        shadowInfo.shadowBlurRadius = shadowradius_;
        shadowInfo.shadowOffset = CGSizeMake(shadowoffsx_, shadowoffsy_);
        [textAttributes setObject:shadowInfo forKey:NSShadowAttributeName];
    }
    
    if((flags_ & NE_TEXTFLAG_STROKE) != 0) {
        if((flags_ & NE_TEXTFLAG_FILL) != 0) {
            strokeWidthInfo = [NSNumber numberWithFloat:strokewidth_*-1];
        } else {
            strokeWidthInfo = [NSNumber numberWithFloat:strokewidth_];
        }
        strokeColorInfo = [strokeColor hexARGBColor];
        
        [textAttributes setObject:strokeWidthInfo forKey:NSStrokeWidthAttributeName];
        [textAttributes setObject:strokeColorInfo forKey:NSStrokeColorAttributeName];
    }
    
    if((flags_ & NE_TEXTFLAG_FILL) != 0) {
        UIColor *color = [fillColor hexARGBColor];
        
        if((flags_ & NE_TEXTFLAG_CUTOUT) != 0) {
            color = [color colorWithAlphaComponent:1.0];
        }
        textColorInfo = color;
        [textAttributes setObject:textColorInfo forKey:NSForegroundColorAttributeName];
    }
    
    if(scalex_ > 0.0) {
        expansionInfo = [NSNumber numberWithFloat:scalex_];
        [textAttributes setObject:expansionInfo forKey:NSExpansionAttributeName];
    }
    
    if(skewx_ > 0.0) {
        skewInfo = [NSNumber numberWithFloat:skewx_];
        [textAttributes setObject:skewInfo forKey:NSObliquenessAttributeName];
    }
    
    CGFloat scale = [UIScreen mainScreen].scale;
    CGFloat image_width = width_ / scale;
    CGFloat image_height = height_ / scale;
    if(image_height == 0) {
        image_height = size_ / scale;
    }
    
    // Font
    //
    NSString *uiFontName = nil;
    
    int from = -1;
    
    if([typeface hasPrefix:@"asset:"]) {
        from = (int)@"asset:".length;
    } else if([typeface hasPrefix:@"theme:"]) {
        from = (int)@"theme:".length;
    } else if([typeface hasPrefix:@"android:"]) {
        from = (int)@"android:".length;
    } else {
        // ToDo
        //
    }
    
    if(from != -1) {
        NSDictionary *fontmap = @{@"raleway_thin":@"raleway-thin",
                                  @"goudy_stm_italic":@"goudystm-italic",
                                  @"junction":@"junctionregular",
                                  @"leaguescript":@"leaguescriptthin-leaguescript",
                                  @"droid sans":@"AppleSDGothicNeo-Regular"
                                  };
        
        NSString *fontKey = [[typeface substringFromIndex:from] componentsSeparatedByString:@"."][0];
        NSString *fontName = nil;
        if(fontmap[fontKey.lowercaseString]) {
            fontName = fontmap[fontKey.lowercaseString];
        } else {
            fontName = fontKey;
        }
        if([typeface hasPrefix:@"android:"]) {
            uiFontName = fontName;
        } else {
            uiFontName = [FontTools fontNameWithBundleFontName:fontName];
            if (uiFontName == nil) {
                fontName = [typeface substringFromIndex:from];
                uiFontName = [UIFont fontWithName:fontName size:size_].fontName;
            }
        }
    }
    
    if(uiFontName == nil) {
        uiFontName = [UIFont systemFontOfSize:size_].fontName;
    }
    
    // UIFont(name, size), text size, max lines -> { updated font size, updated text size }
    CGSize destTextRegion;
    CGFloat destTextSize = [FontTools adjustFontSizeWithFontName:uiFontName
                                             textRegionSize:CGSizeMake(image_width - (margin_ * 2), image_height)
                                                   textSize:size_
                                                   maxLines:maxLines_
                                                       text:fmtstr
                                             destTextRegion:&destTextRegion];
    
    [textAttributes setObject:[UIFont fontWithName:uiFontName size:destTextSize]
                       forKey:NSFontAttributeName];
    
    NSMutableParagraphStyle *paragraphStyle = [NSMutableParagraphStyle new];
    paragraphStyle.lineBreakMode = NSLineBreakByWordWrapping;
    paragraphStyle.lineSpacing = 0.5;
    paragraphStyle.lineHeightMultiple = 1.1;
    if(align_ == NE_ALIGN_LEFT) {
        paragraphStyle.alignment = NSTextAlignmentLeft;
    } else if(align_ == NE_ALIGN_CENTER) {
        paragraphStyle.alignment = NSTextAlignmentCenter;
    } else if(align_ == NE_ALIGN_RIGHT){
        paragraphStyle.alignment = NSTextAlignmentRight;
    } else {
        paragraphStyle.alignment = NSTextAlignmentCenter;
    }
    [textAttributes setObject:paragraphStyle forKey:NSParagraphStyleAttributeName];
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(image_width, image_height), NO , scale);
    
    /**
     *  Context위에 글자를 그려줄 캔버스의 위치를 설정한다.
     *  Context 기준으로 Center 기점으로 잡았음.
     */
    CGFloat textRegionWidth = image_width - margin_ - 4;
    CGFloat textRegionHeight = image_height;
    CGFloat textRegionXpos = 0;
    CGFloat textRegionYpos = 0;
    
    if((flags_ & NE_TEXTFLAG_AUTOSIZE) == 0
       && image_width > 0 && image_height > 0) {
        //
        if(valign_ == NE_VALIGN_CENTER) {
            textRegionYpos = (image_height - destTextRegion.height) / 2;
        } else if(valign_ == NE_VALIGN_BOTTOM) {
            textRegionYpos = image_height - destTextRegion.height;
        } else {
            textRegionYpos = 0;
        }
    }
    
    UIColor *bgUIColor = [bgColor hexARGBColor];
    
    CGContextRef context = UIGraphicsGetCurrentContext();
    if((flags_ & NE_TEXTFLAG_CUTOUT) != 0) {
        CGContextSetFillColorWithColor(context, [UIColor clearColor].CGColor);
    } else {
        CGContextSetFillColorWithColor(context, bgUIColor.CGColor);
    }
    CGContextFillRect(context, CGRectMake(0, 0, image_width, image_height));
    
    [fmtstr drawWithRect:CGRectMake(textRegionXpos, textRegionYpos, textRegionWidth, textRegionHeight)
                 options:NSStringDrawingUsesFontLeading | NSStringDrawingUsesLineFragmentOrigin
              attributes:textAttributes
                 context:nil];
    
    UIImage *image_ = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    if((flags_ & NE_TEXTFLAG_CUTOUT) != 0) {
        image_ = [image_ blendingImageWithColor:bgUIColor blendingMode:kCGBlendModeDestinationOut];
    }
    
    if(paragraphStyle != nil) {
        [paragraphStyle release];
    }
    if(shadowInfo != nil) {
        [shadowInfo release];
    }
    
    [parseDictionary release];
    
    return image_.CGImage;
}

+ (unsigned char*) getPixelsWithImage:(CGImageRef)image
{
    unsigned char *imageData = NULL;
    
    int width = (int)CGImageGetEvenNumberedWidth(image);
    int height = (int)CGImageGetEvenNumberedHeight(image);
    
    @autoreleasepool {
        
        imageData = new unsigned char[width * height * 4];
        memset(imageData, 0x0, width * height * 4);
        CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
        CGContextRef contextRef = CGBitmapContextCreate(imageData,
                                                        width,
                                                        height,
                                                        8/* bitsPerComponet */,
                                                        4*width/* bytesPerRow */,
                                                        colorSpaceRef,
                                                        kCGImageAlphaPremultipliedLast|kCGBitmapByteOrder32Big);
        CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), image);
        CGColorSpaceRelease(colorSpaceRef);
        CGContextRelease(contextRef);
    }
    return imageData;
}

+ (NSString *)pdecode:(NSString *)string
{
    NSUInteger findpos = 0;
    NSString *hexchars = @"0123456789ABCDEF";
    
    NSMutableString *result = [string mutableCopy];
    
    while((findpos = [result rangeOfString:@"%" options:0 range:NSMakeRange(findpos, result.length - findpos)].location) != NSNotFound) {
        if(findpos+2 >= result.length) {
            break;
        }
        int c1 = (int)[hexchars rangeOfString:[NSString stringWithFormat:@"%c", [result characterAtIndex:findpos+1]]].location;
        int c2 = (int)[hexchars rangeOfString:[NSString stringWithFormat:@"%c", [result characterAtIndex:findpos+2]]].location;
        if(c1 != -1 && c2 != -1) {
            NSString *string_a = [NSString stringWithFormat:@"%c", (char)((c1<<4)|c2)];
            [result replaceCharactersInRange:NSMakeRange(findpos, 1) withString:string_a];
            [result deleteCharactersInRange:NSMakeRange(findpos+1, 2)];
        }
        findpos++;
    }
    return result;
}

@end

size_t CGImageGetEvenNumberedWidth(CGImageRef cg_nullable image) {
    return CGImageGetWidth(image) & 0xFFFFFFFE;
}

size_t CGImageGetEvenNumberedHeight(CGImageRef cg_nullable image) {
    return CGImageGetHeight(image) & 0xFFFFFFFE;
}




