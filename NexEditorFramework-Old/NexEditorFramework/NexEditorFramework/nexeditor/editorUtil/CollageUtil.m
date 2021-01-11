/**
 * File Name   : CollageUtil.h
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

#import "CollageUtil.h"

@implementation CollageUtil

+ (CGRect) shrinkRect:(CGRect)rect toAspect:(CGFloat)aspect
{
    CGRect result = CGRectZero;
    
    CGFloat originalAspect = rect.size.width / rect.size.height;
    
    if(originalAspect < aspect) {
        // original too tall
        CGFloat newHeight = round(rect.size.width / aspect);
        
        CGFloat ctry = CGRectGetMidY(rect);
        CGFloat top = ctry - newHeight / 2;
        
        result = CGRectMake(rect.origin.x, top, rect.size.width, newHeight);
    } else {
        // orginal too wide
        CGFloat newWidth = round(rect.size.height * aspect);
        
        CGFloat ctrx = CGRectGetMidX(rect);
        CGFloat left = ctrx - newWidth / 2;
        
        result = CGRectMake(left, rect.origin.y, newWidth, rect.size.height);
    }
    return result;
}

+ (CGRect) convertPixelToAbstractDimensionWithRect:(CGRect)rect size:(CGSize)size
{
    int ABSTRACT_DIMENSION = 100000;

    CGFloat x = rect.origin.x * ABSTRACT_DIMENSION / size.width;
    CGFloat y = rect.origin.y * ABSTRACT_DIMENSION / size.height;
    CGFloat w = rect.size.width * ABSTRACT_DIMENSION / size.width;
    CGFloat h = rect.size.height * ABSTRACT_DIMENSION / size.height;

    return CGRectMake(x, y, w, h);
}

+ (CGRect) convertAbstractDimensionToPixelWithRect:(CGRect)rect size:(CGSize)size
{
    int ABSTRACT_DIMENSION = 100000;
    
    CGFloat x = rect.origin.x * size.width / ABSTRACT_DIMENSION;
    CGFloat y = rect.origin.y * size.height / ABSTRACT_DIMENSION;
    CGFloat w = rect.size.width * size.width / ABSTRACT_DIMENSION;
    CGFloat h = rect.size.height * size.height / ABSTRACT_DIMENSION;
    
    return CGRectMake(x, y, w, h);
}

+ (CGPoint) convertPixelToAbstractDimensionWithPoint:(CGPoint)point size:(CGSize)size
{
    int ABSTRACT_DIMENSION = 100000;

    CGFloat x = point.x * ABSTRACT_DIMENSION / size.width;
    CGFloat y = point.y * ABSTRACT_DIMENSION / size.height;

    return CGPointMake(x, y);
}

+ (CGPoint) convertAbstractDimensionToPixelWithPoint:(CGPoint)point size:(CGSize)size
{
    int ABSTRACT_DIMENSION = 100000;

    CGFloat x = point.x * size.width / ABSTRACT_DIMENSION;
    CGFloat y = point.y * size.height / ABSTRACT_DIMENSION;

    return CGPointMake(x, y);
}

+ (CGRect) CGRectWithBaseRect:(CGRect)basedRect translatePoint:(CGPoint)translatePoint angle:(int)angle
{
    CGFloat cx = CGRectGetWidth(basedRect) * 0.5f;
    CGFloat cy = CGRectGetHeight(basedRect) * 0.5f;
    CGFloat left = -cx; CGFloat top = -cy;
    CGFloat right = cx; CGFloat bottom = cy;
    CGFloat radian = -(CGFloat)[MathUtil toRadians:(CGFloat)angle];
    CGPoint point0 = CGPointMake(cos(radian)*left - sin(radian)*top, sin(radian)*left + cos(radian)*top);
    CGPoint point1 = CGPointMake(cos(radian)*right - sin(radian)*top, sin(radian)*right + cos(radian)*top);
    CGPoint point2 = CGPointMake(cos(radian)*left - sin(radian)*bottom, sin(radian)*left + cos(radian)*bottom);
    CGPoint point3 = CGPointMake(cos(radian)*right - sin(radian)*bottom, sin(radian)*right + cos(radian)*bottom);
    int ltX = (int)(MIN(MIN(MIN(point0.x, point1.x), point2.x), point3.x) + cx) + translatePoint.x + basedRect.origin.x;
    int ltY = (int)(MIN(MIN(MIN(point0.y, point1.y), point2.y), point3.y) + cy) + translatePoint.y + basedRect.origin.y;
    int rbX = (int)(MAX(MAX(MAX(point0.x, point1.x), point2.x), point3.x) + cx) + translatePoint.x + basedRect.origin.x;
    int rbY = (int)(MAX(MAX(MAX(point0.y, point1.y), point2.y), point3.y) + cy) + translatePoint.y + basedRect.origin.y;
    return CGRectMake(ltX, ltY, rbX-ltX, rbY-ltY);
}

// Calculates and returns scale required for rotation of 'cropRect' by 'angle' to avoid black border around the picture
+ (CGFloat) scaleForRotation:(CGFloat) angle position:(CGPoint) position cropRect:(CGRect) cropRect sourceSize:(CGSize) sourceSize
{
    CGRect rect = [self CGRectWithBaseRect:cropRect translatePoint:position angle:angle];
    CGFloat minScale = 1.0f;
    CGFloat rectCenterX = CGRectGetMidX(rect);
    CGFloat rectCenterY = CGRectGetMidY(rect);
    if (rect.origin.x < 0) {
        minScale = MIN(rectCenterX / (CGRectGetWidth(rect) * 0.5f), minScale);
    }
    if (rect.origin.y < 0) {
        minScale = MIN(rectCenterY / (CGRectGetHeight(rect) * 0.5f), minScale);
    }
    if ((rect.size.width + rect.origin.x) > sourceSize.width) {
        minScale = MIN(((sourceSize.width - rectCenterX) / (CGRectGetWidth(rect) * 0.5f)), minScale);
    }
    if ((rect.size.height + rect.origin.y) > sourceSize.height) {
        minScale = MIN(((sourceSize.height - rectCenterY) / (CGRectGetHeight(rect) * 0.5f)), minScale);
    }
    
    return 1.0 / minScale;
}

// scale: 1.0 ~
+ (CGRect) zoom:(CGRect) bounds scale:(CGFloat) scale aspectRatio:(CGFloat) aspectRatio
{
    if (scale < 1.0f) return bounds;
    
    CGFloat halfHeight = (CGRectGetHeight(bounds) / scale) * 0.5f;
    CGFloat rectCenterX = CGRectGetMidX(bounds);
    CGFloat rectCenterY = CGRectGetMidY(bounds);
    
    CGRect cropRect = CGRectMake(rectCenterX - (halfHeight * aspectRatio),
                                 rectCenterY - halfHeight,
                                 (halfHeight * aspectRatio) * 2,
                                 halfHeight * 2);
    return cropRect;
}

+ (CGRect) CGRectApplyAngleWith:(int)angle position:(CGPoint)position aspectRatio:(CGFloat)ratio basedCropRect:(CGRect)basedRect sourceSize:(CGSize)sourceSize
{
    CGFloat scale = [self scaleForRotation:angle
                                  position:position
                                  cropRect:CGRectIntegral(basedRect)
                                sourceSize:sourceSize];
    return [self zoom:CGRectIntegral(basedRect) scale:scale aspectRatio:ratio];
}

+ (CGRect) CGRectApplyScaleWith:(CGFloat)scale angle:(int)angle position:(CGPoint)position aspectRatio:(CGFloat)ratio basedCropRect:(CGRect)basedRect sourceSize:(CGSize)sourceSize
{
    CGRect result = [self zoom:basedRect scale:scale aspectRatio:ratio];

    CGFloat rotationScale = [self scaleForRotation:angle
                                          position:position
                                          cropRect:CGRectIntegral(basedRect)
                                        sourceSize:sourceSize];

    if (rotationScale > scale) {
        // Smaller 'scale' breaks rotation result
        result = CGRectNull;
    }
    return result;
}

+ (CGPoint) CGPointTranslateWith:(CGPoint)position angle:(int)angle basedCropRect:(CGRect)basedRect sourceSize:(CGSize)sourceSize
{
    CGRect rect = [self CGRectWithBaseRect:basedRect translatePoint:position angle:angle];

    int xpos = 0; int ypos = 0;
    if (rect.origin.x < 0) {
        xpos = xpos - rect.origin.x;
    }
    if (rect.origin.y < 0) {
        ypos = ypos - rect.origin.y;
    }
    if ((rect.size.width + rect.origin.x) > sourceSize.width) {
        xpos = xpos - ((rect.size.width + rect.origin.x) - sourceSize.width);
    }
    if ((rect.size.height + rect.origin.y) > sourceSize.height) {
        ypos = ypos - ((rect.size.height + rect.origin.y) - sourceSize.height);
    }
    if (xpos == 0 && ypos == 0) {
        return position;
    } else {
        return CGPointMake(xpos + position.x, ypos + position.y);
    }
}

+ (CGPoint) getRotatePosition:(CGPoint)position WithAngle:(CGFloat)angle bClockWise:(Boolean)bClockWise
{
    CGFloat radian = (CGFloat)[MathUtil toRadians:(CGFloat)angle];
    if (!bClockWise) {
        radian = radian * -1;
    }
    return CGPointMake(cos(radian)*position.x - sin(radian)*position.y, sin(radian)*position.x + cos(radian)*position.y);
}

@end
