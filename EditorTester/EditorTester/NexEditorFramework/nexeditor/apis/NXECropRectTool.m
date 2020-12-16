/*
 * File Name   : NXECropRectTool.m
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

#import "NXECropRectTool.h"
#import "AspectRatioTool.h"

@interface NXECropRectTool()
@property (nonatomic) CropRectTool tool;
@end

@implementation NXECropRectTool

- (instancetype) initWithSourceSize:(CGSize) sourceSize destinationRatio:(float) ratio
{
    self = [super init];
    if (self) {
        self.sourceSize = sourceSize;
        self.destinationRatio = ratio;
        self.tool = CropRectToolMake(sourceSize, ratio);
    }
    return self;
}

+ (instancetype) cropRectToolWithSourceSize:(CGSize) sourceSize destinationRatio:(float) ratio
{
    return [[[self alloc] initWithSourceSize:sourceSize destinationRatio:ratio] autorelease];
}

- (CGRect) fill
{
    return CropRectToolFill(self.tool);
}

- (CGRect) fit
{
    return CropRectToolFit(self.tool);
}

- (CGRect) randomSubRectAtScale:(CGPoint) scale
{
    return CropRectToolRandomSubRectAtScale(self.tool, scale);
}

@end

#pragma mark - CropRectTool
CropRectTool CropRectToolMake(CGSize sourceSize, float destinationRatio)
{
    CropRectTool result;
    result.sourceSize = sourceSize;
    result.destinationRatio = destinationRatio;
    return result;
}

CGRect CropRectToolFill(CropRectTool tool)
{
    CGRect roi = CGRectMake(0, 0, tool.sourceSize.width, tool.sourceSize.height);
    return [AspectRatioTool rectFills:roi ratio:tool.destinationRatio fit:NO];
}

CGRect CropRectToolFit(CropRectTool tool)
{
    CGRect roi = CGRectMake(0, 0, tool.sourceSize.width, tool.sourceSize.height);
    return [AspectRatioTool rectFills:roi ratio:tool.destinationRatio fit:YES];
}

CGRect CropRectToolRandomSubRectAtScale(CropRectTool tool, CGPoint scale)
{
    scale.x = MIN(scale.x, 1.0);
    scale.y = MIN(scale.y, 1.0);
    
    CGSize size = tool.sourceSize;
    CGRect zoombounds = CGRectMake(0, 0, size.width * scale.x, size.height * scale.y);
    zoombounds = [AspectRatioTool rectFills:zoombounds ratio:tool.destinationRatio fit:NO];
    
    float random = (float)(arc4random_uniform(10)) / 10;
    zoombounds.origin.x += (int)((size.width *random) * (1.0 - scale.x));
    zoombounds.origin.y += (int)((size.height *random) * (1.0 - scale.y));
    return zoombounds;
}


