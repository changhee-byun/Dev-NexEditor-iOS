/*
 * File Name   : NXECropRectTool.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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
#import <CoreGraphics/CoreGraphics.h>

/*
 * \brief A tool for gererating NexEditor's frame crop representation according to aspect ratio of target render destination dimension.
 * ```
 * NXECropTool *tool = [[NXECropTool cropRectToolWithSourceSize:CGSizeMake(1024, 768) destinationratio:1280.0/720];
 * // CropModeFill
 * CGRect fill = [tool fill];
 * // CropModeFit
 * CGRect fit = [tool fit];
 * // CropModePanRandom
 * CGRect zoom = [tool randomSubrectAtScale:CGPointMake(0.75, 0.75)];
 * CGRect full = [tool fit];
 * full.origin.y *= (2.0 / 3);
 *
 * // From:
 * [NXECrop updateVisualClip:clip.videoInfo startEndPositionWithCropMode:FILL ratio:ratio];
 * // To:
 * CGRect startRect = [self rectForCropMode:FILL sourceSize:clip.size ratio:((float)ratio.width) / ratio.height;]
 * CGRect endRect = startRect;
 *
 * ```
 */

#define kAbstractDimensionEdgeSize  100000.0

typedef struct {
    CGSize sourceSize;
    float destinationRatio;
} CropRectTool;

CropRectTool CropRectToolMake(CGSize sourceSize, float destinationRatio);
CGRect CropRectToolFill(CropRectTool tool);
CGRect CropRectToolFit(CropRectTool tool);
CGRect CropRectToolRandomSubRectAtScale(CropRectTool tool, CGPoint scale);

@interface NXECropRectTool: NSObject
@property (nonatomic) CGSize sourceSize;
@property (nonatomic) float destinationRatio;

- (instancetype) initWithSourceSize:(CGSize) sourceSize destinationRatio:(float) ratio;
+ (instancetype) cropRectToolWithSourceSize:(CGSize) sourceSize destinationRatio:(float) ratio;

- (CGRect) fill;
- (CGRect) fit;
- (CGRect) randomSubRectAtScale:(CGPoint) scale;
@end
