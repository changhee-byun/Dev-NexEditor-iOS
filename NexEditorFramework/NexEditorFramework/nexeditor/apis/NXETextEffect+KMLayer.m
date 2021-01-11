/**
 * File Name   : NXETextEffect+KMLayer.m
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

#import "NXETextEffect+KMLayer.h"
#import "KMTextLayer.h"
#import "KMImageLayer.h"
#import "KMAnimationEffect.h"
#import "OverlayTitleLayerInternal.h"
#import "AssetLibrary.h"
#import "NXEAssetItemPrivate.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "EditorUtil.h"

#define LOG_TAG @"NXETextEffect"

@implementation NXETextEffect (KMLayer)

- (UIFont *)fontWithFontKeyID:(NSString *)fontKeyID fontSize:(int)fontSize
{
    NSDictionary *fontmap = @{
      @"system.droidsansb" : @"AppleSDGothicNeo-SemiBold"
    };
    
    NSString *fontName = fontmap[fontKeyID.lowercaseString];
    if (fontName == nil) {
        NSString *fontPath = [[AssetLibrary library] itemForId:fontKeyID].fileInfoResourcePath;
        if (fontPath != nil) {
            fontName = [EditorUtil fontNameWithFontPath:fontPath];
        }
    }
    
    UIFont *result;
    if ( fontName ) {
        result = [UIFont fontWithName:fontName size:fontSize];
    } else {
        result = [UIFont systemFontOfSize:fontSize];
    }

    return result;
}

- (UIImage *)imageWithRscKeyID:(NSString *)rscKeyID
{
    NSString *rscPath = [[AssetLibrary library] itemForId:rscKeyID].fileInfoResourcePath;
    return [UIImage imageWithContentsOfFile:rscPath];
}

- (void)configureAnimationWithOverlayTitleAnimationArray:(NSArray<OverlayTitleAnimation *>*)array layerStarTime:(int)layerStarTime layer:(KMLayer *)layer
{
    for(OverlayTitleAnimation *animation in array) {
        [self configureAnimationWithOverlayTitleLayerAnimationMotionArray:animation.motions animationType:animation.type layerStarTime:layerStarTime layer:layer];
    }
}

- (AlphaAnimation *)makeAlphaAnimation:(int)startTime endTime:(int)endTime startValue:(float)startValue endValue:(float)endValue interpolation:(TimeInterpolation)interpolation
{
    return [[[AlphaAnimation alloc] initWithAniStartTime:startTime
                                             aniEndTime:endTime
                                        alphaStartValue:startValue
                                          alphaEndValue:endValue
                                          interpolation:interpolation] autorelease];
}

- (MoveAnimation *)makeMoveAnimation:(int)startTime endTime:(int)endTime layerPoint:(CGPoint)layerPoint moveStartPoint:(CGPoint)moveStartPoint moveEndPoint:(CGPoint)moveEndPoint interpolation:(TimeInterpolation)interpolation
{
    return [[[MoveAnimation alloc] initWithAniStartTime:startTime
                                             aniEndTime:endTime
                                             layerPoint:layerPoint
                                         moveStartPoint:moveStartPoint
                                           moveEndPoint:moveEndPoint
                                          interpolation:interpolation] autorelease];
}

- (RotateAnimation *)makeRotateAnimation:(int)startTime endTime:(int)endTime layerRotate:(float)layerRotate rotateValue:(float)rotateValue clockWise:(BOOL)clockWise interpolation:(TimeInterpolation)interpolation
{
    return [[[RotateAnimation alloc] initWithAniStartTime:startTime
                                               aniEndTime:endTime
                                              layerRotate:layerRotate
                                              rotateValue:rotateValue
                                                clockWise:clockWise
                                            interpolation:interpolation] autorelease];
}

- (ScaleAnimation *)makeScaleAnimation:(int)startTime endTime:(int)endTime xScaleStartValue:(float)xScaleStartValue yScaleStartValue:(float)yScaleStartValue xScaleEndValue:(float)xScaleEndValue yScaleEndValue:(float)yScaleEndValue interpolation:(TimeInterpolation)interpolation
{
    return [[[ScaleAnimation alloc] initWithAniStartTime:startTime
                                              aniEndTime:endTime
                                        xScaleStartValue:xScaleStartValue
                                        yScaleStartValue:yScaleStartValue
                                          xScaleEndValue:xScaleEndValue
                                          yScaleEndValue:yScaleEndValue
                                           interpolation:interpolation] autorelease];
}

- (void)configureAnimationWithOverlayTitleLayerAnimationMotionArray:(NSArray<OverlayTitleLayerAnimationMotion*>*)array animationType:(AnimationType)animationType layerStarTime:(int)layerStarTime layer:(KMLayer *)layer
{
    int startTime; int endTime;
    int lastTime = 0;
    
    switch(animationType) {
        case AnimationTypeAlpha:
        {
            float lastAlpha = 0.0f;
            
            for(OverlayTitleLayerAnimationMotion *motion in array) {
                @autoreleasepool {
                    startTime = layerStarTime + motion.startTime;
                    endTime = layerStarTime + motion.duration;
                    
                    if(lastTime != 0 && lastTime < startTime) {
                        [layer addKMAnimationEffect:[self makeAlphaAnimation:lastTime endTime:startTime startValue:lastAlpha endValue:lastAlpha interpolation:TimeInterpolationMax]];
                    }
                    
                    [layer addKMAnimationEffect:[self makeAlphaAnimation:startTime endTime:endTime startValue:motion.startAlpha endValue:motion.endAlpha interpolation:(TimeInterpolation)motion.interpolatorType]];
                    
                    lastTime = endTime;
                    lastAlpha = motion.endAlpha;
                }
            }
            break;
        }

        case AnimationTypeMove:
        {
            CGPoint lastStartPoint = CGPointZero;
            CGPoint lastEndPoint = CGPointZero;
            
            for(OverlayTitleLayerAnimationMotion *motion in array) {
                @autoreleasepool {
                    startTime = layerStarTime + motion.startTime;
                    endTime = layerStarTime + motion.duration;
                    
                    if(lastTime != 0 && lastTime < startTime) {
                        [layer addKMAnimationEffect:[self makeMoveAnimation:lastTime endTime:startTime layerPoint:CGPointMake(layer.x, layer.y) moveStartPoint:lastStartPoint moveEndPoint:lastEndPoint interpolation:TimeInterpolationMax]];
                    }
                    
                    [layer addKMAnimationEffect:[self makeMoveAnimation:startTime endTime:endTime layerPoint:CGPointMake(layer.x, layer.y) moveStartPoint:motion.startPosition moveEndPoint:motion.endPosition interpolation:(TimeInterpolation)motion.interpolatorType]];
                    
                    lastTime = endTime;
                    lastStartPoint = motion.startPosition;
                    lastEndPoint = motion.endPosition;
                }
            }
            break;
        }
        
        case AnimationTypeRotate:
        {
            float lastRotation = 0.0f;
            BOOL lastClockWise = NO;
            
            for(OverlayTitleLayerAnimationMotion *motion in array) {
                @autoreleasepool {
                    startTime = layerStarTime + motion.startTime;
                    endTime = layerStarTime + motion.duration;
                    
                    if(lastTime != 0 && lastTime < startTime) {
                        [layer addKMAnimationEffect:[self makeRotateAnimation:lastTime endTime:startTime layerRotate:0 rotateValue:lastRotation clockWise:lastClockWise interpolation:TimeInterpolationMax]];
                    }
                    
                    [layer addKMAnimationEffect:[self makeRotateAnimation:startTime endTime:endTime layerRotate:0 rotateValue:motion.rotation clockWise:motion.clockWise interpolation:(TimeInterpolation)motion.interpolatorType]];
                    
                    lastTime = endTime;
                    lastRotation = motion.rotation;
                    lastClockWise = motion.clockWise;
                }
            }
            break;
        }

        case AnimationTypeScale:
        {
            CGPoint lastScaleStartPoint = CGPointZero;
            CGPoint lastScaleEndPoint = CGPointZero;
            
            for(OverlayTitleLayerAnimationMotion *motion in array) {
                @autoreleasepool {
                    startTime = layerStarTime + motion.startTime;
                    endTime = layerStarTime + motion.duration;
                    
                    if(lastTime != 0 && lastTime < startTime) {
                        [layer addKMAnimationEffect:[self makeScaleAnimation:lastTime endTime:startTime xScaleStartValue:lastScaleStartPoint.x yScaleStartValue:lastScaleStartPoint.y xScaleEndValue:lastScaleEndPoint.x yScaleEndValue:lastScaleEndPoint.y interpolation:TimeInterpolationMax]];
                    }
                    
                    [layer addKMAnimationEffect:[self makeScaleAnimation:startTime endTime:endTime xScaleStartValue:motion.startPosition.x yScaleStartValue:motion.startPosition.y xScaleEndValue:motion.endPosition.x yScaleEndValue:motion.endPosition.y interpolation:(TimeInterpolation)motion.interpolatorType]];
                    
                    lastTime = endTime;
                    lastScaleStartPoint = motion.startPosition;
                    lastScaleEndPoint = motion.endPosition;
                }
            }
            break;
        }
  
        default:
            break;
    }
}

- (KMLayer *)kmlayertWith:(OverlayTitleLayer *)overlayTitleLayer projectDuration:(int) projectDuration titleText:(NSString *)titleText
{
    NSString *inputedText = /*titleText == nil ? overlayTitleLayer.defaultText : */titleText;
    UIFont *font = [self fontWithFontKeyID:overlayTitleLayer.fontName fontSize:overlayTitleLayer.fontSize];

    KMTextLayer *kmTextLayer = [[[KMTextLayer alloc] init] autorelease];
    
    kmTextLayer.isSelectable = NO;
    
    if(overlayTitleLayer.startTime == -1) {
        kmTextLayer.startTime = projectDuration - overlayTitleLayer.duration;
    } else {
        kmTextLayer.startTime = overlayTitleLayer.startTime;
    }
    kmTextLayer.endTime = kmTextLayer.startTime + overlayTitleLayer.duration;
    
    [kmTextLayer setText:inputedText Font:font];
    
    kmTextLayer.width = (int)(overlayTitleLayer.position.right - overlayTitleLayer.position.left);
    kmTextLayer.height = (int)(overlayTitleLayer.position.bottom - overlayTitleLayer.position.top);
    
    kmTextLayer.x = (int)(overlayTitleLayer.position.right + overlayTitleLayer.position.left)/2 - kmTextLayer.width/2;
    kmTextLayer.y = (int)(overlayTitleLayer.position.bottom + overlayTitleLayer.position.top)/2 - kmTextLayer.height/2;
    
    [kmTextLayer setTextColor:overlayTitleLayer.fontColor];
    [kmTextLayer setShadowEnable:overlayTitleLayer.fontShadow];
    [kmTextLayer setShadowColor:overlayTitleLayer.fontShadowColor];
    [kmTextLayer setGlowEnable:overlayTitleLayer.fontGlow];
    [kmTextLayer setGlowColor:overlayTitleLayer.fontGlowColor];
    [kmTextLayer setOutlineEnable:overlayTitleLayer.fontOutline];
    [kmTextLayer setOutlineColor:overlayTitleLayer.fontOutlineColor];
    
    if(overlayTitleLayer.align.hAlign == HorizontalAlignLeft) {
        kmTextLayer.hAlignment = NSTextAlignmentLeft;
    } else if(overlayTitleLayer.align.hAlign == HorizontalAlignRight) {
        kmTextLayer.hAlignment = NSTextAlignmentRight;
    } else {
        kmTextLayer.hAlignment = NSTextAlignmentCenter;
    }
    
    if(overlayTitleLayer.align.vAlign == VerticalAlignTop) {
        kmTextLayer.vAlignment = TextAlignmentTop;
    } else if(overlayTitleLayer.align.vAlign == VerticalAlignBottom) {
        kmTextLayer.vAlignment = TextAlignmentBottom;
    } else {
        kmTextLayer.vAlignment = TextAlignmentMiddle;
    }
    
    kmTextLayer.angle = overlayTitleLayer.rotation;
    
    NSArray<OverlayTitleAnimation *>* animations = overlayTitleLayer.animations;
    
    BOOL hasSubTitle = YES;
    
    if(overlayTitleLayer.group.textGroup == TextGroupStart) {
        hasSubTitle = overlayTitleLayer.inAniSubTitle;
    } else {
        hasSubTitle = overlayTitleLayer.outAniSubTitle;
    }
    
    if(hasSubTitle == NO) {
        kmTextLayer.x = (int)(overlayTitleLayer.position.right + overlayTitleLayer.position.left + overlayTitleLayer.positionOffset.x)/2 - kmTextLayer.width/2;
        kmTextLayer.y = (int)(overlayTitleLayer.position.bottom + overlayTitleLayer.position.top + overlayTitleLayer.positionOffset.y)/2 - kmTextLayer.height/2;
        
        if(overlayTitleLayer.align.hAlign == HorizontalAlignLeft) {
            kmTextLayer.hAlignment = NSTextAlignmentLeft;
        } else if(overlayTitleLayer.align.hAlign == HorizontalAlignRight) {
            kmTextLayer.hAlignment = NSTextAlignmentRight;
        } else {
            kmTextLayer.hAlignment = NSTextAlignmentCenter;
        }
        
        if(overlayTitleLayer.align.vAlign == VerticalAlignTop) {
            kmTextLayer.vAlignment = TextAlignmentTop;
        } else if(overlayTitleLayer.align.vAlign == VerticalAlignBottom) {
            kmTextLayer.vAlignment = TextAlignmentBottom;
        } else {
            kmTextLayer.vAlignment = TextAlignmentMiddle;
        }
        
        if(overlayTitleLayer.adjAnimations != nil) {
            animations = overlayTitleLayer.adjAnimations;
        }
    }
    
    [self configureAnimationWithOverlayTitleAnimationArray:animations layerStarTime:kmTextLayer.startTime layer:kmTextLayer];
    
    return kmTextLayer;
}

- (KMLayer *)kmlayertWith:(OverlayTitleLayer *)titleLayer projectDuration:(int) projectDuration
{
    OverlayTitleLayer *overlayTitleLayer = titleLayer;
    
    UIImage *rscImage = [self imageWithRscKeyID:overlayTitleLayer.resourceID];
    
    KMImageLayer *kmImageLayer = [[[KMImageLayer alloc] initWithImage:rscImage] autorelease];
    
    kmImageLayer.isSelectable = NO;
    
    if(overlayTitleLayer.startTime == -1) {
        kmImageLayer.startTime = projectDuration - overlayTitleLayer.duration;
    } else {
        kmImageLayer.startTime = overlayTitleLayer.startTime;
    }
    kmImageLayer.endTime = kmImageLayer.startTime + overlayTitleLayer.duration;
    
    CGFloat imageWidth = rscImage.size.width;
    CGFloat imageHeight = rscImage.size.height;
    
    kmImageLayer.x = (int)(overlayTitleLayer.position.right + overlayTitleLayer.position.left)/2 - imageWidth/2;
    kmImageLayer.y = (int)(overlayTitleLayer.position.bottom + overlayTitleLayer.position.top)/2 - imageHeight/2;
    
    kmImageLayer.angle = overlayTitleLayer.rotation;
    
    NSArray<OverlayTitleAnimation *>* animations = overlayTitleLayer.animations;
    
    BOOL hasSubTitle = YES;
    
    if(overlayTitleLayer.group.textGroup == TextGroupStart) {
        hasSubTitle = overlayTitleLayer.inAniSubTitle;
    } else {
        hasSubTitle = overlayTitleLayer.outAniSubTitle;
    }
    
    if(hasSubTitle == NO) {
        kmImageLayer.x = (int)(overlayTitleLayer.position.right + overlayTitleLayer.position.left + overlayTitleLayer.positionOffset.x)/2 - imageWidth/2;
        kmImageLayer.y = (int)(overlayTitleLayer.position.bottom + overlayTitleLayer.position.top + overlayTitleLayer.positionOffset.y)/2 - imageHeight/2;
        
        if(overlayTitleLayer.adjAnimations != nil) {
            animations = overlayTitleLayer.adjAnimations;
        }
    }
    
    [self configureAnimationWithOverlayTitleAnimationArray:animations layerStarTime:kmImageLayer.startTime layer:kmImageLayer];
    
    return kmImageLayer;
}

- (NSArray<KMLayer *> *)buildLayersWithDuration:(int) projectDuration error:(NXEError **)error
{
    if(self.itemId == nil) {
        NexLogE(LOG_TAG, @"itemId is nil");
        return @[];
    }
    
    NSData *jsonData = [[AssetLibrary library] itemForId:self.itemId].fileInfoResourceData;
    OverlayTitleInfo *overlayTitleInfo = [[[OverlayTitleInfo alloc] initWithJSONData:jsonData nxeError:error] autorelease];
    
    if(overlayTitleInfo == nil) {
        NexLogD(LOG_TAG, @"configurOverlayTitle Error: %@", (*error).errorDescription);
        return @[];
    }
    
    TextGroup priorityTextGroup = [overlayTitleInfo.priorityTitleType isEqualToString:VALUE_GROUP_START] ? TextGroupStart : TextGroupEnd;
    NSString *titleText = nil;
    
    BOOL isIncludeInAniSubtitle = YES;
    BOOL isIncludeOutAniSubtitle = YES;
    
    if (self.params.introTitle.length == 0 || self.params.introSubtitle.length == 0) {
        isIncludeInAniSubtitle = NO;
    }
    if ( self.params.outroTitle.length == 0 || self.params.outroSubtitle.length == 0) {
        isIncludeOutAniSubtitle = NO;
    }
    
    NSMutableArray<KMLayer *> *kmlayers = [NSMutableArray array];
    for(OverlayTitleLayer *overlayTitleLayer in overlayTitleInfo.overlays) {
        
        overlayTitleLayer.inAniSubTitle = isIncludeInAniSubtitle;
        overlayTitleLayer.outAniSubTitle = isIncludeOutAniSubtitle;
        
        KMLayer *kmlayer = nil;
        if(overlayTitleLayer.type == OverlayTitleTypeText) {
            
            if(overlayTitleInfo.overlayTitleDuration > projectDuration) {
                if(overlayTitleLayer.group.textGroup != priorityTextGroup) {
                    continue;
                }
            }
            
            if(overlayTitleLayer.textType == TextTypeTitle) {
                titleText = self.params.introTitle;
                if(overlayTitleLayer.group.textGroup == TextGroupEnd) {
                    titleText = self.params.outroTitle;
                }
            }
            else {
                titleText = self.params.introSubtitle;
                if(overlayTitleLayer.group.textGroup == TextGroupEnd) {
                    titleText = self.params.outroSubtitle;
                }
            }
            
            if(titleText == nil) {
                continue;
            }
            
            if(titleText.length > overlayTitleLayer.textMaxLength) {
                titleText = [titleText substringToIndex:overlayTitleLayer.textMaxLength];
            }
            
            kmlayer = [self kmlayertWith:overlayTitleLayer projectDuration:projectDuration titleText:titleText];
        }
        else if(overlayTitleLayer.type == OverlayTitleTypeImage) {
            
            kmlayer = [self kmlayertWith:overlayTitleLayer projectDuration:projectDuration];
        }
        
        if ( kmlayer) {
            [kmlayers addObject:kmlayer];
        }
    }
    
    return [[kmlayers copy] autorelease];
}


@end
