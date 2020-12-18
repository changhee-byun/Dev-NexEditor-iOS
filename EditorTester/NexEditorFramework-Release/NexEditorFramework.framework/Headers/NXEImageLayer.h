/******************************************************************************
 * File Name   :	NXEImageLayer.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Proprietary and Confidential
 Copyright (C) 2002~2017 NexStreaming Corp. All rights reserved.
 www.nexstreaming.com
 ******************************************************************************/

#import <UIKit/UIKit.h>

#import "NXELayer.h"

/** \class NXEImageLayer
 *  \brief NXEImageLayer class defines all properties and methods to manage an image layer.
 *  \since version 1.0.19
 */
@interface NXEImageLayer : NXELayer

/** \brief A rectangle information of an image layer.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) CGRect layerRect; 

/** \brief An image information of an image layer.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) UIImage *uiImage;

/** \brief Initialzes a new image layer with the speficied image and position.
 *  \param uiImage An image to display in an image layer.
 *  \param point A CGPoint structure containing x and y coordinates, CGPointNXELayerCenter for center alignment.
 *  \return An NXEImageLayer object.
 *  \since version 1.0.19
 */
- (instancetype)initWithImage:(UIImage *)uiImage point:(CGPoint)point;

@end
