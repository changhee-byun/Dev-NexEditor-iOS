/*
 * File Name   : NXEClipTransform.h
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

#import <CoreGraphics/CoreGraphics.h>
/**
 * \ingroup types
 * \brief A structure that contains start and end ... TBD
 * \since version 1.5.1
 */
typedef struct {
    /*! TBD */
    CGAffineTransform start;
    /*! TBD */
    CGAffineTransform end;
} NXEClipTransformRamp;


/**
 * \class NXEClipTransform
 * \brief TBD
 *
 * \since version 1.5.1
 */
@interface NXEClipTransform: NSObject

/**
 * \brief TBD
 * \since version 1.5.1
 */
@property (nonatomic, readonly) CGSize sourceSize;
/**
 * \brief TBD
 * \since version 1.5.1
 */
@property (nonatomic, readonly) CGSize destinationSize;

/**
 * \brief TBD
 * \param rect TBD
 * \return TBD
 * \since version 1.5.1
 */
- (CGAffineTransform) zoomToRect:(CGRect) rect;

/**
 * \brief TBD
 * \return TBD
 * \since version 1.5.1
 */
- (CGAffineTransform) fill;

/**
 * \brief TBD
 * \return TBD
 * \since version 1.5.1
 */
- (CGAffineTransform) fit;

/**
 * \brief TBD
 * \param sourceSize TBD
 * \param destinationSize TBD
 * \return TBD
 * \since version 1.5.1
 */
- (instancetype) initWithSourceSize:(CGSize) sourceSize destinationSize:(CGSize) destinationSize;

/**
 * \brief TBD
 * \param sourceSize TBD
 * \param destinationSize TBD
 * \return TBD
 * \since version 1.5.1
 */
+ (instancetype) transformWithSourceSize:(CGSize) sourceSize destinationSize:(CGSize) destinationSize;

@end
