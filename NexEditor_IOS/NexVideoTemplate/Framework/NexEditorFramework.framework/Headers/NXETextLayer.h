/******************************************************************************
 * File Name   :	NXETextLayer.h
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

@class NELayerManager;

/** 
 * \brief This expresses a text string as a layer. You can set the array's color, font, and alignment.
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
 *      // NXETextLayer 생성 및 속성값 반영
 *      //
 *      NXETextLayer *textLayer = [[NXETextLayer alloc] init];
 *      [textLayer setText:@"Example TextLayer" Font:[UIFont boldSystemFontOfSize:180]];
 *      [textLayer setX:xpos Y:ypos];
 *      [textLayer setTextAlignment:NSTextAlignmentCenter];
 *      [textLayer setStartTime:startTime EndTime:endTime];
 *      [textLayer setGlowEnable:YES];
 *      [textLayer setOutlineEnable:YES];
 *      [textLayer setOutlineColor:[UIColor color]];
 *      [textLayer setBackgroundEnable:YES];
 *      [textLayer setExtendedBackground:YES];
 *      [textLayer addLayer2Manager:textLayer];
 *  }
 *  @end
 * \endcode
 */
@interface NXETextLayer : NSObject

/**
 * \brief This sets the text alignment(left, right, center).
 * \param NSTextAlignment Values for NSTextAlignment
 */
- (void)setTextAlignment:(NSTextAlignment)textAlignment;

/**
 * \brief This sets the coordinates of a layer.
 * \param x The left coordinates of a layer.
 * \param y The top coordinates of a layer.
 */
- (void)setX:(int)x Y:(int)y;

/**
 * \brief This sets the text string and font.
 * \param text The text string.
 * \param font UIFont object.
 */
- (void)setText:(NSString *)text Font:(UIFont *)font;

/**
 * \brief This sets the text color.
 * \param textColor This sets the color of the text string, which is a UIColor object.
 */
- (void)setTextColor:(UIColor *)textColor;

/**
 * \brief This enables the shadow effect.
 * \param isEnable Whether or not the shadow effect is on.
 */
- (void)setShadowEnable:(BOOL)isEnable;

/**
 * \brief THis sets the shadow color.
 * \param shadowColor The shadow color, which is a UIColor.
 */
- (void)setShadowColor:(UIColor *)shadowColor;

/**
 * \brief This enables the glow effect.
 * \param isEnable Whether the glow effect is on.
 */
- (void)setGlowEnable:(BOOL)isEnable;

/**
 * \brief This sets the glow color.
 * \param glowColor The glow color, which is a UIColor object.
 */
- (void)setGlowColor:(UIColor *)glowColor;

/**
 * \brief This enables the outline effect.
 * \param isEnabled Whether or not the outline effect is on.
 */
- (void)setOutlineEnable:(BOOL)isEnable;

/**
 * \brief This sets the outline color.
 * \param outlineColor The outline color, which is a UIColor object.
 */
- (void)setOutlineColor:(UIColor *)outlineColor;

/**
 * \brief This sets the background effect.
 * \param isEnabled Whether or not there is a background.
 */
- (void)setBackgroundEnable:(BOOL)isEnable;

/**
 * \brief This sets the extended background effect.
 * \param isEnabled Whether or not the extended background effect is on.
 */
- (void)setExtendedBackground:(BOOL)isEnable;

/**
 * \brief This sets the background color.
 * \param backgroundColor The background color, which is a UIColor object.
 */
- (void)setBackgroundColor:(UIColor *)backgoundColor;

/**
 * \brief This sets the layer's start and end times in \c milliseconds.
 * \param startTime This sets the layer's start time in \c milliseconds.
 * \param endTime This sets the layer's end time in \c milliseconds. 
 */
- (void)setStartTime:(int)startTime EndTime:(int)endTime;

/**
 * \brief This adds the layer to LayerManager. 
 */
- (long)addLayer2Manager;

/**
 * \brief This remove the layer to LayerManager.
 */
- (void)removeLayer2Manager;

@end
