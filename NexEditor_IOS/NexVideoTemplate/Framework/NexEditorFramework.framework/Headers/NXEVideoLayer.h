/******************************************************************************
 * File Name   :	NXEVideoLayer.h
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

@class NXEVisualClip;

/**
 * \brief This creates a layer from a video file.
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
 *      // Get Video Path
 *      NSString *videoPath = [NSBundle mainBundle] pathForResource:@"example_0" ofType:@"mp4" inDirectory:@"resource/video"];
 *
 *      int errorType;
 *      NXEVideoLayer *videoLayer = [[NXEVideoLayer alloc] initWithPath:videoPath error:&errorType];
 *      [videoLayer setX:0 Y:0];
 *      [videoLayer addLayer2Manager];
 *  }
 *  @end
 * \endcode
 */
@interface NXEVideoLayer : NSObject

@property (nonatomic, assign) long layerId;

/** 
 * \brief Creates a layer instance based on the input parameter.
 * \param path The path info about contents.
 * \param error Zero for success, or a non-zero NexEditor&trade;&nbsp; error code in the event of a failure. Check the ERRORCODE Enumeration(ENUM) Type for more details.
 * \see ERRORCODE
 */
- (instancetype)initWithPath:(NSString *)path error:(int *)error;

/**
 * \brief 비디오 Layer대한 X,Y 좌표를 설정, 1280 * 720 기준으로 설정해 줘야한다.
 *      widht, height는 기본적으로 재생하려는 컨텐츠의 1/2 값이 반영이 된다.
 */
- (void)setX:(int)x Y:(int)y;

/**
 * \brief 비디오 Layer대한 Width, Height 설정, 1280 * 720 기준으로 설정해 줘야한다.
 *      widht, height는 기본적으로 재생하려는 컨텐츠의 1/2 값이 반영이 된다.
 */
- (void)setLayerWidth:(int)width height:(int)height;

/**
 * \brief This adds VideoLayer to LayerManager.
 */
- (void)addLayer2Manager;

/**
 * \brief This remove the layer to LayerManager.
 */
- (void)removeLayer2Manager;
@end
