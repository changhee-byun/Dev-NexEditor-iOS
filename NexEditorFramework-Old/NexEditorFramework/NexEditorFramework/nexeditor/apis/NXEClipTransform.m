/*
 * File Name   : NXEClipTransform.m
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
#import "NXEClipTransform.h"
#import "NXECropRectTool.h"

#import "AspectRatioTool.h"

typedef struct {
    CGSize sourceSize;
    CGSize destinationSize;
} ClipTransform;

ClipTransform ClipTransformMake(CGSize source, CGSize destination);
CGAffineTransform ClipTransformZoomToRect(ClipTransform transform, CGRect rect);
CGAffineTransform ClipTransformFill(ClipTransform transform);
CGAffineTransform ClipTransformFit(ClipTransform transform);

@interface NXEClipTransform()
@property (nonatomic) CGSize sourceSize;
@property (nonatomic) CGSize destinationSize;

@property (nonatomic) ClipTransform clipTransform;
@end

@implementation NXEClipTransform

- (instancetype) initWithSourceSize:(CGSize) sourceSize destinationSize:(CGSize) destinationSize
{
    self = [super init];
    if ( self ) {
        self.sourceSize = sourceSize;
        self.destinationSize = destinationSize;
        self.clipTransform = ClipTransformMake(sourceSize, destinationSize);
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

+ (instancetype) transformWithSourceSize:(CGSize) sourceSize destinationSize:(CGSize) destinationSize
{
    return [[[self alloc] initWithSourceSize:sourceSize destinationSize:destinationSize] autorelease];
}

- (CGAffineTransform) zoomToRect:(CGRect) rect
{
    return ClipTransformZoomToRect(self.clipTransform, rect);
}

- (CGAffineTransform) fill
{
    return ClipTransformFill(self.clipTransform);
}

- (CGAffineTransform) fit
{
    return ClipTransformFit(self.clipTransform);
}

#pragma mark - Private API

+ (NXEClipTransformRamp) transformRampForMode:(CropMode) mode
                                    imageSize:(CGSize) size
                                   renderSize:(CGSize) renderSize
{
    NXEClipTransformRamp result;
    result.start = CGAffineTransformIdentity;
    result.end = CGAffineTransformIdentity;
    
    if (mode == CropModeCustom) {
        mode = CropModeFit;
    }
    
    ClipTransform transform = ClipTransformMake(size, renderSize);
    switch(mode) {
        case CropModeFit:
            result.start = result.end = ClipTransformFit(transform);
            break;
            
        case CropModeFill:
            result.start = result.end = ClipTransformFill(transform);
            break;
            
        case CropModePanRandom:
        {
            float ratio = transform.destinationSize.width / transform.destinationSize.height;
            CropRectTool cropTool = CropRectToolMake(transform.sourceSize, ratio);
            CGRect r1 = CropRectToolRandomSubRectAtScale(cropTool, CGPointMake(0.75, 0.75));
            CGRect r2 = CropRectToolFill(cropTool);
            r2.origin.y *= (2.0 / 3);
            if (arc4random_uniform(2) == 0) {
                CGRect t = r2;
                r2 = r1;
                r1 = t;
            }
            
            result.start = ClipTransformZoomToRect(transform, r1);
            result.end = ClipTransformZoomToRect(transform, r2);
        }
            break;
            
        default:
            break;
    }
    
    return result;
}
@end

#pragma mark - ClipTransform

ClipTransform ClipTransformMake(CGSize source, CGSize destination)
{
    ClipTransform transform;
    transform.sourceSize = source;
    transform.destinationSize = destination;
    return transform;
}

CGAffineTransform ClipTransformZoomToRect(ClipTransform transform, CGRect rect)
{
    CGFloat sx = transform.destinationSize.width / rect.size.width;
    CGFloat sy = transform.destinationSize.height / rect.size.height;
    CGAffineTransform t = CGAffineTransformMakeScale(sx, sy);
    t = CGAffineTransformTranslate(t, -rect.origin.x, -rect.origin.y);
    return t;
}

CGAffineTransform ClipTransformFill(ClipTransform transform)
{
    float ratio = transform.destinationSize.width / transform.destinationSize.height;
    CropRectTool tool = CropRectToolMake(transform.sourceSize, ratio);
    return ClipTransformZoomToRect(transform, CropRectToolFill(tool));
}

CGAffineTransform ClipTransformFit(ClipTransform transform)
{
    float ratio = transform.destinationSize.width / transform.destinationSize.height;
    CropRectTool tool = CropRectToolMake(transform.sourceSize, ratio);
    return ClipTransformZoomToRect(transform, CropRectToolFit(tool));
}
