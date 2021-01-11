/**
 * File Name   : NXESlotConfiguration.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "NXEEnginePrivate.h"
#import "NXECollageSlotConfiguration.h"
#import "NXECollageSlotConfigurationInternal.h"
#import "NXEClipInternal.h"
#import <NexEditorEngine/NexDrawInfo.h>
#import "LUTMap.h"
#import "MathUtil.h"
#import "CollageUtil.h"
#import "NXEAssetItemPrivate.h"
#import "NXEAssetLibrary.h"
#import "AssetLibrary.h"
#import <NexEditorEngine/NXEVisualClipInternal.h>

@interface NexDrawInfo(Collage)
- (void) update;
@end

@interface NXECollageSlotConfiguration()

@property (nonatomic) CGRect slotRect;
@property (nonatomic) CollageSizeInt collageAspect;
@property (nonatomic) BOOL isClipChanged;
@property (nonatomic) BOOL isUpdating;
@property (nonatomic, retain) CollageSlotData* slotData;
@property (nonatomic, retain) NexDrawInfo *drawInfo;

@end

@implementation NXECollageSlotConfiguration
{
    int sourceWidth;
    int sourceHeight;
    int displayCropRectOffsetX;
    int displayCropRectOffsetY;
    CGFloat translateX;
    CGFloat translateY;
    CGRect basedRectForAngle;
    CGRect basedRectForScale;
}

- (CGFloat) getSlotRatio
{
    CollageSizeInt aspect = self.collageAspect;
    CGFloat result = ((self.slotRect.size.width - self.slotRect.origin.x) * aspect.width) / ((self.slotRect.size.height - self.slotRect.origin.y) * aspect.height);
    if (self.slotData.slotWidth != 0 && self.slotData.slotHeight != 0) {
        result = (CGFloat)aspect.width / (CGFloat)aspect.height;
    }
    return result;
}

- (void) setClip:(id<CollageClip>)clip
{
    if (clip == nil) {
        super.clip = [[NXEClip newSolidClip:@"black"] autorelease];
    } else {
        super.clip = clip;
    }
    self.isClipChanged = YES;

    NXEClip *nxeClip = (NXEClip *) clip;
    CGSize size = nxeClip.videoInfo.orientationRegardedSize;
    sourceWidth = size.width;
    sourceHeight = size.height;
    if (nxeClip.clipType == NXE_CLIPTYPE_VIDEO) {
        nxeClip.videoInfo.clipVolume = 0;
        self.drawInfo.rotate = nxeClip.videoInfo.rotateState;
    }

    CGFloat ratio = [self getSlotRatio];
    CGRect result = [CollageUtil shrinkRect:CGRectMake(0, 0, sourceWidth, sourceHeight) toAspect:ratio];
    basedRectForAngle = basedRectForScale = result;
    self.drawInfo.startRect = self.drawInfo.endRect = [CollageUtil convertPixelToAbstractDimensionWithRect:result size:size];

    displayCropRectOffsetX = result.origin.x;
    displayCropRectOffsetY = result.origin.y;

    translateX = 0;
    translateY = 0;
    CGPoint sourcePoint = CGPointMake(translateX, translateY);
    CGPoint convertPoint = [CollageUtil convertPixelToAbstractDimensionWithPoint:sourcePoint size:size];
    self.drawInfo.translate_x = convertPoint.x;
    self.drawInfo.translate_y = convertPoint.y;
}

- (int)angle
{
    return self.drawInfo.userRotate;
}

- (void) setAngle:(int)angle
{
    angle = angle % 360;

    CGRect result = CGRectIntegral([CollageUtil CGRectApplyAngleWith:angle
                                                            position:CGPointMake(translateX, translateY)
                                                         aspectRatio:[self getSlotRatio]
                                                       basedCropRect:basedRectForAngle
                                                          sourceSize:CGSizeMake(sourceWidth, sourceHeight)]);
    displayCropRectOffsetX = result.origin.x;
    displayCropRectOffsetY = result.origin.y;
    
    CGSize sourceSize = CGSizeMake(sourceWidth, sourceHeight);
    self.drawInfo.startRect = self.drawInfo.endRect = [CollageUtil convertPixelToAbstractDimensionWithRect:result size:sourceSize];
    self.drawInfo.userRotate = angle;

    if (self.isUpdating == NO) {
        [self.drawInfo update];
    }
}

- (void) setFlip:(FlipType)flip
{
    const NSUInteger bitmaskHorizontal = 0x00010000;
    const NSUInteger bitmaskVertical = 0x00020000;
    
    int rotate = self.drawInfo.rotate;

    super.flip = flip;
    if (flip == FlipTypeNone) {
        rotate &= ~(bitmaskHorizontal | bitmaskVertical);
    } else {
        if ((flip & FlipTypeHorizontal) == FlipTypeHorizontal) {
            if ((rotate & bitmaskHorizontal) == bitmaskHorizontal) {
                rotate &= ~bitmaskHorizontal;
            } else {
                rotate |= bitmaskHorizontal;
            }
        }
        if ((flip & FlipTypeVertical) == FlipTypeVertical) {
            if ((rotate & bitmaskVertical) == bitmaskVertical) {
                rotate &= ~bitmaskVertical;
            } else {
                rotate |= bitmaskVertical;
            }
        }
    }
    self.drawInfo.rotate = rotate;
    if (self.isUpdating == NO) {
        [self.drawInfo update];
    }
}

- (void) setScale:(CGFloat)scale
{
    if ( super.scale == scale ) { return ; }
    if ( scale < 1.0f || scale > 2.0f) {
        return;
    }

    CGRect result = [CollageUtil CGRectApplyScaleWith:scale
                                                angle:self.angle
                                             position:CGPointMake(translateX, translateY)
                                          aspectRatio:[self getSlotRatio]
                                        basedCropRect:basedRectForScale
                                           sourceSize:CGSizeMake(sourceWidth, sourceHeight)];

    if (CGRectIsNull(result) == NO) {
        basedRectForAngle = result;

        CGSize sourceSize = CGSizeMake(sourceWidth, sourceHeight);
        self.drawInfo.startRect = self.drawInfo.endRect = [CollageUtil convertPixelToAbstractDimensionWithRect:result size:sourceSize];

        [self setTranslateInfoWith:CGPointMake(translateX, translateY) basedCropRect:result];

        displayCropRectOffsetX = result.origin.x;
        displayCropRectOffsetY = result.origin.y;

        if (self.isUpdating == NO) {
            [self.drawInfo update];
        }
        super.scale = scale;
    }
}

- (void) setLutIDWithoutUpdate:(NSString *)lutID
{
    super.lutID = lutID;
    self.drawInfo.lut = (unsigned int)[LUTMap indexFromLutName:lutID];
}

- (void) setLutID:(NSString *)lutID
{
    super.lutID = lutID;
    self.drawInfo.lut = (unsigned int)[LUTMap indexFromLutName:lutID];
    if (self.isUpdating == NO) {
        [self.drawInfo update];
    }
}

- (void) setTranslateInfoWith:(CGPoint)cropRectOrigin basedCropRect:(CGRect)basedCropRect
{
    CGSize sourceSize = CGSizeMake(sourceWidth, sourceHeight);
    CGPoint point = [CollageUtil CGPointTranslateWith:cropRectOrigin
                                                angle:self.angle
                                        basedCropRect:basedCropRect
                                           sourceSize:sourceSize];
    translateX = point.x;
    translateY = point.y;
    CGPoint sourcePoint = CGPointMake(translateX, translateY);
    CGPoint convertPoint = [CollageUtil convertPixelToAbstractDimensionWithPoint:sourcePoint size:sourceSize];
    self.drawInfo.translate_x = convertPoint.x;
    self.drawInfo.translate_y = convertPoint.y;
}

- (CGPoint) convertDistanceScaleInSourceWith:(CGPoint)position
{
    CGPoint result = CGPointMake(position.x, position.y);

    CGPoint aspectAtSource = CGPointMake(1.0, 1.0);
    CGPoint aspectAtCollage = CGPointMake(1.0, 1.0);

    if (sourceWidth < sourceHeight) {
        aspectAtSource.y = (CGFloat)sourceHeight / (CGFloat)sourceWidth;
        aspectAtCollage.y = (CGFloat)self.slotRect.size.height / (CGFloat)self.slotRect.size.width;

        result.y = position.y * (aspectAtCollage.y / aspectAtSource.y);
    }
    else {
        aspectAtSource.x = (CGFloat)sourceWidth / (CGFloat)sourceHeight;
        aspectAtCollage.x = (CGFloat)self.slotRect.size.width / (CGFloat)self.slotRect.size.height;

        result.x = position.x * (aspectAtCollage.x / aspectAtSource.x);
    }
    return result;
}

- (CGPoint) convertDistanceScaleInViewWith:(CGPoint)position
{
    CGPoint result = CGPointMake(position.x, position.y);

    CGPoint aspectAtSource = CGPointMake(1.0, 1.0);
    CGPoint aspectAtCollage = CGPointMake(1.0, 1.0);

    if (sourceWidth < sourceHeight) {
        aspectAtSource.y = (CGFloat)sourceHeight / (CGFloat)sourceWidth;
        aspectAtCollage.y = (CGFloat)self.slotRect.size.height / (CGFloat)self.slotRect.size.width;

        result.y = position.y / (aspectAtCollage.y / aspectAtSource.y);
    }
    else {
        aspectAtSource.x = (CGFloat)sourceWidth / (CGFloat)sourceHeight;
        aspectAtCollage.x = (CGFloat)self.slotRect.size.width / (CGFloat)self.slotRect.size.height;
        
        result.x = position.x / (aspectAtCollage.x / aspectAtSource.x);
    }
    return result;
}

- (void) setPosition:(CGPoint)position
{
    CGPoint distanceScaleInSource = [self convertDistanceScaleInSourceWith:position];

    CGSize sourceSize = CGSizeMake(sourceWidth, sourceHeight);
    CGRect cropRect = [CollageUtil convertAbstractDimensionToPixelWithRect:self.drawInfo.startRect size:sourceSize];

    CGFloat movingRectMidX = CGRectGetMidX(cropRect) + sourceWidth * distanceScaleInSource.x * -1;
    CGFloat movingRectMidY = CGRectGetMidY(cropRect) + sourceHeight * distanceScaleInSource.y * -1;

    CGPoint movingRectOrigin = CGPointMake(movingRectMidX - displayCropRectOffsetX - cropRect.size.width / 2.0f,
                                        movingRectMidY - displayCropRectOffsetY - cropRect.size.height / 2.0f);

    [self setTranslateInfoWith:movingRectOrigin basedCropRect:cropRect];

    distanceScaleInSource.x = -1 * ((translateX + displayCropRectOffsetX + cropRect.size.width / 2.0f) - CGRectGetMidX(cropRect)) / sourceWidth;
    distanceScaleInSource.y = -1 * ((translateY + displayCropRectOffsetY + cropRect.size.height / 2.0f) - CGRectGetMidY(cropRect)) / sourceHeight;

    super.position = [self convertDistanceScaleInViewWith:distanceScaleInSource];

    if (self.isUpdating == NO) {
        [self.drawInfo update];
    }
}

- (void) dealloc
{
    self.slotData = nil;
    self.drawInfo = nil;
    
    [super dealloc];
}

@end

@implementation NXECollageSlotConfiguration (Internal)

- (instancetype) initWithCollageSlotData:(CollageSlotData *)slotData slotRect:(CGRect)slotRect collageAspect:(CollageSizeInt)collageAspect
{
    NSString *const systemType = @"system";
    NSString *const systemMTType = @"system_mt";

    self = [super init];

    if ([slotData.sourceType isEqualToString:systemType] ||
        [slotData.sourceType isEqualToString:systemMTType]) {
        NSString *sourcePath = [[AssetLibrary library] itemForId:slotData.sourceDefault].fileInfoResourcePath;
        NSError *error = nil;
        NXEClip *clip = [NXEClip clipWithSource:[NXEClipSource sourceWithPath:sourcePath] error:&error];
        if (clip == nil) {
            clip = [[NXEClip newSolidClip:@"black"] autorelease];
        }
        if ([slotData.sourceType isEqualToString:systemMTType] && clip.clipType == NXE_CLIPTYPE_VIDEO) {
            clip.videoInfo.isMotionTrackedVideo = YES;
        }
        self.clip = clip;
    } else {
        self.clip = [[NXEClip newSolidClip:@"black"] autorelease];
    }
    [self setLutIDWithoutUpdate:slotData.lut];

    _slotRect = slotRect;
    self.slotData = slotData;
    _collageAspect = collageAspect;
    _drawInfo = [[NexDrawInfo alloc] init];

    return self;
}

@end

@implementation NexDrawInfo(Collage)
- (void) update
{
    [NXEEngine.instance updateDrawInfo:self];
}
@end
