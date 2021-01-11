/******************************************************************************
 * File Name   : ImageUtil.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

#define RGB(r, g, b) [UIColor colorWithRed:(r)/255.0 green:(g)/255.0 blue:(b)/255.0 alpha:1]
#define RGBA(r, g, b, a) [UIColor colorWithRed:(r)/255.0 green:(g)/255.0 blue:(b)/255.0 alpha:a]

@interface ImageUtil : NSObject

+ (ImageUtil *)shared;

+ (UIImage*)imageOfValidTextureWithImage:(UIImage*) image;

+ (UIImage *)imageWithColor:(UIColor *)color;
+ (UIImage *)imageWithColor:(UIColor *)color Size:(CGSize)size;
+ (UIImage *)circleImageWithColor:(UIColor *)color Size:(float)diameter;
+ (UIImage *)circleImageWithColor:(UIColor *)color Size:(float)width Diameter:(float)diameter Scale:(float)scale ;
+ (UIImage *)circleImageWithbgColor:(UIColor *)bgColor bgSize:(float)bgRadius color:(UIColor*)color Size:(float)radius;

+ (UIImage *)dimImageWithSize:(CGSize)size emptyRect:(CGRect)rect Scale:(float)scale Angle:(float)angle;

/** HSB 색상 체계에서 Hue 값을 보여주는 원형 이미지. (도넛 모양 이미지)
 @param radius - 이미지의 반지름
 @param divisor - 몇 단계의 색상으로 표현할 것인지. 값이 크면 자연스럽고 값이 작으면 표현되는 색도 적고 경계가 뚜렷하다
 @param thicknessRate - 두께 정도. 반지름의 percentage(0 < thicknessRate < 1.0)
 */
+ (UIImage *)imageHueCircle:(float)radius Divisor:(int)divisor Thickness:(float)thicknessRate;

/** 음악 파일로 부터 wave form 이미지를 생성한다.
 */
+ (UIImage *)getWaveFormWithAsset:(AVURLAsset *)songAsset Size:(CGSize)size;

/** 리소스 파일로 OpenGL 텍스쳐를 생성하고 id 값을 반환한다.
 */
+(GLuint) glTextureIDfromName:(NSString*)name;

/** UIImage 객체로 OpenGL 텍스쳐를 생성하고 id 값을 반환한다.
 */
+(GLuint) glTextureIDfromImage:(UIImage*)image;

/** data 배열로 OpenGL 텍스쳐를 생성하고 id 값을 반환한다.
 */
+(GLuint) glTextureIDfromData:(GLubyte*)data Width:(int)w Height:(int)h;

/** 색상값으로 OpenGL 텍스쳐를 생성하고 id 값을 반환한다.
 */
+(GLuint) glTextureIDfromColorRed:(float)red Green:(float)green Blue:(float)blue Alpha:(float)alpha;

/** OpenGL 텍스쳐 메모리에서 해제 한다.
 */
+(void) deleteTexture:(GLuint) textureId;

+(float) glMaxTextureSize;

+ (CGSize) limitSize:(CGSize) size maxSize:(CGSize) max diff: ( void (^)(CGSize) ) diff;

+ (GLuint) getTextureIDWithHeight:(int)height width:(int)width drawingBlock:(void (^)(CGContextRef context))specificContent;

@end

