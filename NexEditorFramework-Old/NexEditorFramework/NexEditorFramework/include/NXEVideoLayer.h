/******************************************************************************
 * File Name   :	NXEVideoLayer.h
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

#import "NXELayer.h"
#import "NXEClipSource.h"

@class NXEClip;

/** \class NXEVideoLayer
 *  \brief NXEVideoLayer class defines all properties and methods to manage a video layer.
 *  \since version 1.0.19
 */
@interface NXEVideoLayer : NXELayer

/** \brief A rectangle information of the video layer.
 *  \note It is recommended not to set manually, because the clip properties are set automatically by calling the initVideoLayerWithPath:point.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) CGRect layerRect;

/** \brief A clip information of the video layer.
 *  \note It is recommended not to set manually, because the clip properties are set automatically by calling the initVideoLayerWithPath:point.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) NXEClip *layerClip;

/** \brief Initialzes a new video layer with the video clip in the speficied path.
 *  \param path The path of the video clip.
 *  \param point The alignment type of the video layer, CGPointNXELayerCenter for center alignment.
 *  \return An NXEVideoLayer object. If failed, NSException is thrown.
 *  \deprecated Use initWithClipSource:point:error: instead.
 *  \since version 1.0.19
 */
- (instancetype)initVideoLayerWithPath:(NSString *)path point:(CGPoint)point __attribute__((deprecated));

/** \brief Initialzes a new video layer with the video clip in the clip source.
 *  \param clipSource The path of the video clip.
 *  \param point The alignment type of the video layer, CGPointNXELayerCenter for center alignment.
 *  \param error Pointer to NSError *. Upon failure, it will be assigned to NXEError object.
 *  \return An NXEVideoLayer object. If failed, error will be assigned to NXEError object.
 *  \since version 1.3.4
 */
- (instancetype) initWithClipSource:(NXEClipSource *) clipSource point:(CGPoint)point error:(NSError **) error;

/** \brief Creates and returns a new video layer with the video clip in the clip source.
 *  \param clipSource The path of the video clip.
 *  \param error Pointer to NSError *. Upon failure, it will be assigned to NXEError object.
 *  \return An NXEVideoLayer object. If failed, error will be assigned to NXEError object.
 *  \since version 1.3.4
 */
+ (instancetype) layerWithClipSource:(NXEClipSource *) clipSource error:(NSError **) error;
@end
