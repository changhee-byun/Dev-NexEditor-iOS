/******************************************************************************
 * File Name   :	NXELayer.h
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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "NXEEditorType.h"
#import "NXELutRegistry.h"

extern const CGPoint CGPointNXELayerCenter;
extern const CGFloat NXELayerScaledCoordinateCenter;

/** \class NXELayer
 *  \brief NXELayer class defines all properties and methods to manage a layer.
 *  \since version 1.0.19
 */
@interface NXELayer : NSObject

/** \brief Layer alpha, range: 0.0~1.0 
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getAlpha, setter=setAlpha:) float alpha;

/** \brief Layer angle value, range: 0~359
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getAngle, setter=setAngle:) float angle;

/** \brief Layer horizontal flip
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getHFlip, setter=setHFlip:) BOOL hFlip;

/** \brief Layer vertical flip
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getVFlip, setter=setVFlip:) BOOL vFlip;

/** \brief Layer brightness, range: -1.0~1.0, default: 0.0
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getBrightness, setter=setBrightness:) float brightness;

/** \brief Layer contrast, range: -1.0~1.0, default: 0.0
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getContrast, setter=setContrast:) float contrast;

/** \brief Layer saturation, ragne: -1.0~1.0, default: 0.0
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getSaturation, setter=setSaturation:) float saturation;

/** \brief Layer color filter as NXELutTypes, NS_NUM type
 *  \since version 1.0.19
 *  \deprecated Use lutId instead.
 */
@property (nonatomic) NXELutTypes colorFilter __attribute__((deprecated));

/**
 * \brief Layer LUT color filter
 * \since version 1.3.0
 */
@property (nonatomic) NXELutID lutId;

/** \brief Layer inAnimation as NXEInAnimationType, NS_NUM type
 *  \since version 1.0.19
 */
@property (nonatomic, assign) NXEInAnimationType inAnimation;

/** \brief inAnimation duration \c milliseoconds
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int inAnimationDuration;

/** \brief Layer outAnimation as NXEOutAnimationType, NS_NUM type
 *  \since version 1.0.19
 */
@property (nonatomic, assign) NXEOutAnimationType outAnimation;

/** \brief outAnimation duration \c milliseoconds
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int outAnimationDuration;

/** \brief Layer expression as NXEExpressionType, NS_NUM type
 *  \since version 1.0.19
 */
@property (nonatomic, assign) NXEExpressionType expression;

/** \brief Layer width
 *  \since version 1.0.19
 */
@property (nonatomic) int width;

/** \brief Layer height
 *  \since version 1.0.19
 */
@property (nonatomic) int height;

/** \brief Scale
 *  \since version 1.3.0
 */
@property (nonatomic) CGFloat scale;

/** \brief layer point, Xposition
 *  \since version 1.1.0
 */
@property (nonatomic) int x;

/** \brief layer point, Yposition
 *  \since version 1.1.0
 */
@property (nonatomic) int y;

/** \brief the time to start showing a layer
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getStartTime, setter=setStartTime:) int startTime;

/** \brief the time to stop showing a layer
 *  \since version 1.0.19
 */
@property (nonatomic, assign, getter=getEndTime, setter=setEndTime:) int endTime;

/** \brief Layer type as NXELayerType, NS_NUM type
 *  \since version 1.0.19
 */
@property (nonatomic, assign) NXELayerType layerType;

/** \brief Layer ID, index in the layer manage list
 *  \since version 1.0.19
 */
@property (nonatomic, assign) long layerId;

/** \brief Changes position of the layer with 0.0 ~ 1.0 scale values.
 *  \since version 1.1.0
 */
@property (nonatomic) CGRect scaledFrame;

/**
 * Only for backward compatibility.
 * \deprecated Use \c x property instead
 */
@property (nonatomic, getter=x, setter=setX:) int x_position __attribute__((deprecated));
/**
 * Only for backward compatibility.
 * \deprecated Use \c y property instead
 */
@property (nonatomic, getter=y, setter=setY:) int y_position __attribute__((deprecated));

/** \brief Sets the start and end time of a layer.
 *  \param startTime The start time to start displaying the layer, in milliseoconds.
 *  \param endTime The end time to stop displaying the layer, in milliseconds.
 *  \since version 1.0.19
 */
- (void)setStartTime:(int)startTime endTime:(int)endTime;

/** \brief Sets the brightness, contrast and saturation to a layer.
 *  \param brightness The brightness value of the layer, ranging from -1.0 through 1.0, 0.0 by default.
 *  \param contrast The contrast value of the layer, ranging from -1.0 through 1.0, 0.0 by default.
 *  \param saturation The saturation value of the layer, ranging from -1.0 through 1.0, 0.0 by default.
 *  \since version 1.0.19
 */
- (void)setBrightness:(float)brightness contrast:(float)contrast saturation:(float)saturation;

/** \brief Sets the width and height to a layer.
 *  \param size A CGSize structure containing width and height values.
 *  \since version 1.0.19
 */
- (void)setLayerSize:(CGSize)size;

/** \brief Sets the in-animation type and its duration to a layer.
 *  \param inAnimationType An NXEInAnimationType type.
 *  \param duration The animation duration time in milliseoconds, ranging from 0 through 5000.
 *  \since version 1.0.19
 */
- (void)setInAnimationType:(NXEInAnimationType)inAnimationType duration:(int)duration;

/** \brief Sets the exoression type to a layer.
 *  \param expressionType An NXEExpressionType type.
 *  \since version 1.0.19
 */
- (void)setOverallAnimation:(NXEExpressionType)expressionType;

/** \brief Sets the out-animation type and its duration to a layer.
 *  \param outAnimationType An NXEOutAnimationType type.
 *  \param duration The animation duration time in milliseoconds, ranging from 0 through 5000.
 *  \since version 1.0.19
 */
- (void)setOutAnimationType:(NXEOutAnimationType)outAnimationType duration:(int)duration;

@end
