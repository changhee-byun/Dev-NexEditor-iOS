/**
 * File Name   : CollageUtil.m
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

#import <Foundation/Foundation.h>
#import "MathUtil.h"

@interface CollageUtil : NSObject

/* \brief ScaleAspectFill
 * \return CGRect (pixel unit)
 */
+ (CGRect) shrinkRect:(CGRect)rect toAspect:(CGFloat)aspect;

/* \brief Convert from the coordiate of pixel to coordinate of theme renderer
 * \return CGRect (theme renderer unit)
 */
+ (CGRect) convertPixelToAbstractDimensionWithRect:(CGRect)rect size:(CGSize)size;

/* \brief Convert from the coordinate of theme renderer to coordiate of pixel
 * \return CGRect (pixel unit)
 */
+ (CGRect) convertAbstractDimensionToPixelWithRect:(CGRect)rect size:(CGSize)size;

/* \brief Convert from the coordiate of pixel to coordinate of theme renderer
 * \return CGRect (theme renderer unit)
 */
+ (CGPoint) convertPixelToAbstractDimensionWithPoint:(CGPoint)point size:(CGSize)size;

/* \brief Convert from the coordinate of theme renderer to coordiate of pixel
 * \return CGRect (pixel unit)
 */
+ (CGPoint) convertAbstractDimensionToPixelWithPoint:(CGPoint)point size:(CGSize)size;

/* \brief CGRect in applying angle property in SLOT
 * \return CGRect (pixel unit)
 */
+ (CGRect) CGRectApplyAngleWith:(int)angle position:(CGPoint)position aspectRatio:(CGFloat)ratio basedCropRect:(CGRect)basedRect sourceSize:(CGSize)sourceSize;

/* \brief CGRect in applying scale property in SLOT
 * \return CGRect (pixel unit)
 */
+ (CGRect) CGRectApplyScaleWith:(CGFloat)scale angle:(int)angle position:(CGPoint)position aspectRatio:(CGFloat)ratio basedCropRect:(CGRect)basedRect sourceSize:(CGSize)sourceSize;

/* \brief Get proper scale value for current angled rect
 * \return
 */
+ (CGFloat) scaleForRotation:(CGFloat) angle position:(CGPoint) position cropRect:(CGRect) cropRect sourceSize:(CGSize) sourceSize;

/* \brief 
 * \return CGPoint (pixel unit)
 */
+ (CGPoint) CGPointTranslateWith:(CGPoint)position angle:(int)angle basedCropRect:(CGRect)basedRect sourceSize:(CGSize)sourceSize;

/* \brief Get position - clockwise rotation around the origin of a point with coordinates (x, y)
 * \return CGPoint (pixel unit)
 */
+ (CGPoint) getRotatePosition:(CGPoint)position WithAngle:(CGFloat)angle bClockWise:(Boolean)bClockWise;
@end
