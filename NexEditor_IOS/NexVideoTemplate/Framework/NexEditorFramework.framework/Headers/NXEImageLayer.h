/******************************************************************************
 * File Name   :	NXEImageLayer.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "NXEEditorType.h"

/** 
 * \brief This interface presents images as a layer. 
 *      This can be made using resources, files, and UIImages.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      [project addClip:clip];
 *      [engine setProject:project];
 *
 *      // Get ImagePath
 *      NSString *imagePath = [NSBundle mainBundle] pathForResource:@"example_0" ofType:@"png" inDirectory:@"resource/image"];
 *
 *      // Apply LUT to an image file in the bundle.
 *      NXEImageLayer *imageLayer = [[NEImageLayer alloc] initWithLocalPath:imagePath];
 *      [imageLayer setX:0 Y:0];
 *      [imageLayer setStartTime:0 EndTime:[self.engine.project getTotalTime]];
 *      [imageLayer setLut:NXE_LUT_SYMPHONY_BLUE];
 *      [imageLayer addLayer2Manager];
 *  }
 *  @end
 * \endcode
 */
@interface NXEImageLayer : NSObject

/** 
 * \brief This creates a layer with the resource name.
 * \param name The name of the resource.(without extension)
 * \return KMImageLayer object. 
 */
- (instancetype)initWithResourceName:(NSString*)name;

/** 
 * \brief This creates a layer with a local file path information.
 * \param path - The file path.
 * \return KMImageLayer object.
 */
- (instancetype)initWithLocalPath:(NSString*)path;

/**
 * \brief This creates a layer with a UIImage object.
 * \param image - UIImage object.
 * \return KMImageLayer object.
 */
- (instancetype)initWithImage:(UIImage*)image;

/**
 * \brief This creates a layer with a UIColor object. 
 * \param color - UIColor object.
 * \return KMImageLayer object.
 */
- (instancetype)initWithColor:(UIColor*)color;

/**
 * \brief This sets the LUT.
 * \return NELut instance
 */
- (void)setLut:(NXELutTypes)type;

/**
 * \brief This sets the coordinates of a layer.
 * \param x The left coordinates of the layer.
 * \param y The right coordinates of the layer.
 */
- (void)setX:(int)x Y:(int)y;

/**
 * \brief This can set the appearing and disappearing times of a layer in \c milliseoconds.
 * \param startTime The start time of the layer inc \c milliseoconds.
 * \param endTime The end time of the layer inc \c milliseconds.
 */
- (void)setStartTime:(int)startTime EndTime:(int)endTime;

/**
 * \brief THis adds the selected layer to LayerManager.
 * \return layerId The ID of the layer.
 */
- (long)addLayer2Manager;

/**
 * \brief This remove the layer to LayerManager.
 */
- (void)removeLayer2Manager;
@end
