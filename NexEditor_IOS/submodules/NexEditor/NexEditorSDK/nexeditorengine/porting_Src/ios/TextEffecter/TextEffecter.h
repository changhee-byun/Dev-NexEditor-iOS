/******************************************************************************
 * File Name   : TextEffecter.h
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


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface TextEffectProperty : NSObject

typedef NS_ENUM(NSUInteger, TEHorizontalAlign) {
    TEHorizontalAlignLeft = 0,
    TEHorizontalAlignCenter,
    TEHorizontalAlignRight
};

typedef NS_ENUM(NSUInteger, TEVerticalAlign) {
    TEVerticalAlignTop = 0,
    TEVerticalAlignCenter,
    TEVerticalAlignBottom
};


//text color
@property (nonatomic, strong) UIColor* textColor;

//shadow
@property (nonatomic, assign) BOOL useShadow;
@property (nonatomic, strong) UIColor* shadowColor;

/*
 shadowAngle.
 
 Unit : Degree.
 Range : 0 ~ 360. clock-wise direction.
 * light direction of '0' :  ←
 * light direction of '45' : ↖
 * light direction of '90' : ↑
 * light direction of '135' : ↗
 * light direction of '180' : →
 * light direction of '225' : ↘
 * light direction of '270' : ↓
 * light direction of '315' : ↙
 * light direction of '360' : ←
 */
@property (nonatomic, assign) CGFloat shadowAngle;

/*
 shadowDistance.
 Range : 0 ~ 50.
 * 50 means 50% of character height.
 */
@property (nonatomic, assign) CGFloat shadowDistance;

/*
 shadowSpread
 Unit : percent.
 Range : 0 ~ 100.
 Refer to Photoshop.
 */
@property (nonatomic, assign) CGFloat shadowSpread;

/*
 shadowSize
 Unit : percent.
 Range : 0 ~ 0.
 Refer to Photoshop.
 */
@property (nonatomic, assign) CGFloat shadowSize;


//glow
/*
 The range of glowSize and glowSpread values is the same as shadow.
 */
@property (nonatomic, assign) BOOL useGlow;
@property (nonatomic, strong) UIColor* glowColor;
@property (nonatomic, assign) CGFloat glowSpread;
@property (nonatomic, assign) CGFloat glowSize;


//outline
@property (nonatomic, assign) BOOL useStroke;
@property (nonatomic, assign) CGFloat strokeThickness;
@property (nonatomic, strong) UIColor* strokeColor;

// If baseFontPointSizeForKerning is 0, normal mode will be set.
// default value is 24.
@property (nonatomic, assign) CGFloat baseFontPointSizeForKerning;
//kerning and space between lines
@property (nonatomic, assign) CGFloat kerning;
@property (nonatomic, assign) CGFloat spaceBetweenLines;
@property (nonatomic, assign) TEHorizontalAlign horizontalAlign;
@property (nonatomic, assign) TEVerticalAlign verticalAlign;
@property (nonatomic, assign) BOOL useUnderline;
@property (nonatomic, assign) BOOL useStrokeThrough;

@end

@interface TextEffecterResultInfo : NSObject
@property (nonatomic, assign) CGRect rect;
@property (nonatomic, assign) bool isMainText;
@property (nonatomic, assign) int drawOrder; // for example, there are 0 and 1 values, the texture whose drawOrder is 0 must be drawn at first.
@end

@interface TextEffecterResult : TextEffecterResultInfo
@property (nonatomic, strong) UIImage* image;
@property (nonatomic, assign) GLuint debuggingInfo; // Ignore this property.
@end

/*
 TextEffecter is not thread-safe.
 */

@interface TextEffecter : TextEffectProperty

- (void)setText:(NSString*)text withFont:(UIFont*)font;

/*
 Return value (NSArray*) contain TextEffecterResult objects.
 If useShadow is false, only one TextEffecterResult object.
 texName of TextureInfo must be deleted by glDeleteTextures.
*/
- (NSArray<TextEffecterResult*>*)drawText;

/*
 DrawText is heavy. So, if only TextEffecterResultInfo is needed, the use pseudoDrawText.
*/
- (NSArray<TextEffecterResultInfo*>*)pseudoDrawText;
@end

/*-----------------------------------------------------------------------------
 Revision History
 Author        Date        Description of Changes
 -------------------------------------------------------------------------------
 Eric        2019/05/28    Draft.
 Eric        2019/09/16    Change Draw Method (using opengl shader).
 Eric        2019/10/24    Revised.
 -----------------------------------------------------------------------------*/
