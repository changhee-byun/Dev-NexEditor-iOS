/*
 * File Name   : FastPreview.m
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
#import "FastPreview.h"
#import "FastPreviewPrivate.h"
#import "NexEditor.h"

//
id<FastPreviewCommand> const kFastPreviewCommandNormal = @"normal";

@interface FastPreviewKeyValuePairsCommand: NSObject <FastPreviewCommand>
@property (nonatomic, strong) NSDictionary *options;

- (NSString *) stringRepresentation;
- (instancetype) initWithOptions:(NSDictionary *) options;
@end

@interface FastPreviewBuilder ()
@property (nonatomic, retain) NSMutableDictionary *optionList;
@end

@implementation FastPreviewBuilder

- (void)dealloc
{
    [self.optionList release];
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.optionList = [NSMutableDictionary dictionary];
        //
        return self;
    }
    return nil;
}

#pragma mark - Class API
+ (instancetype) builder
{
    return [[[self.class alloc] init] autorelease];
}

#pragma mark - API
- (void)resetBuilder
{
    [self.optionList removeAllObjects];
}

- (void)executeBuilder:(BOOL)isDisplay
{
    id<FastPreviewCommand> command = [self buildCommand];
    [NexEditor.asyncAPI runFastPreviewCommand:command display:isDisplay complete:NULL];
}

- (void)option:(FastPreviewOption)fastPreviewOption optionValue:(int)optionValue
{
    NSArray *optionKeys = @[@"normal",
                            @"brightness",
                            @"contrast",
                            @"saturation",
                            @"adj_brightness",
                            @"adj_contrast",
                            @"adj_saturation",
                            @"tintColor",
                            @"left",
                            @"top",
                            @"right",
                            @"bottom",
                            @"nofx",
                            @"cts",
                            @"swapv",
                            @"video360flag",
                            @"video360_horizontal",
                            @"video360_vertical"];
    
    NSString *key = [optionKeys objectAtIndex:fastPreviewOption];
    NSNumber *value = [NSNumber numberWithInteger:optionValue];
    
    [self.optionList setObject:value forKey:key];
}

- (void)applyColorAdjustWithBrightness:(int)brightness contrast:(int)contrast saturation:(int)saturation
{
    [self option:FASTPREVIEW_BRIGHTNESS optionValue:brightness];
    [self option:FASTPREVIEW_CONTRAST optionValue:contrast];
    [self option:FASTPREVIEW_SATURATION optionValue:saturation];
}

- (void)applyProjectColorAdjustments:(float)brightness contrast:(float)contrast saturation:(float)saturation;
{
    [self option:FASTPREVIEW_PROJECT_BRIGHTNESS optionValue:brightness*255];
    [self option:FASTPREVIEW_PROJECT_CONTRAST optionValue:contrast*255];
    [self option:FASTPREVIEW_PROJECT_SATURATION optionValue:saturation*255];
}

- (void)applyTintColor:(int)tintColor
{
    [self option:FASTPREVIEW_TINTCOLOR optionValue:tintColor];
}

- (void)applyCropRect:(CGRect)rect
{
    int left = rect.origin.x;
    int top = rect.origin.y;
    int right = rect.size.width + left;
    int bottom = rect.size.height + top;
    
    [self applyCropRectWithLeft:left top:top right:right bottom:bottom];
}

- (void)applyCropRectWithLeft:(int)left top:(int)top right:(int)right bottom:(int)bottom
{
    [self option:FASTPREVIEW_LEFT optionValue:left];
    [self option:FASTPREVIEW_TOP optionValue:bottom];
    [self option:FASTPREVIEW_RIGHT optionValue:right];
    [self option:FASTPREVIEW_BOTTOM optionValue:top];
}

- (void)applyNofx:(BOOL)isNofx
{
    int nofx = isNofx ? 1 : 0;
    
    [self option:FASTPREVIEW_NOFX optionValue:nofx];
}

- (void)applySwapv:(BOOL)isSwapv
{
    int swapv = isSwapv ? 1 : 0;
    
    [self option:FASTPREVIEW_SWAPV optionValue:swapv];
}

- (void)applyCTS:(int)cts
{
    [self option:FASTPREVIEW_CTS optionValue:cts];
}

- (void)applyVideo360:(BOOL)isVideo360 horizontalBasedX:(int)x verticalBasedY:(int)y
{
    int video360 = isVideo360 ? 1 : 0;
    
    [self option:FASTPREVIEW_VIDEO360 optionValue:video360];
    [self option:FASTPREVIEW_VIDEO360_HORIZONTAL optionValue:x];
    [self option:FASTPREVIEW_VIDEO360_VERTICAL optionValue:y];
}

- (id<FastPreviewCommand>) buildCommand
{
    return [[[FastPreviewKeyValuePairsCommand alloc] initWithOptions:[[self.optionList copy] autorelease]] autorelease];
}

@end

@implementation FastPreviewKeyValuePairsCommand

+ (instancetype) normal
{
    FastPreviewBuilder *builder = [FastPreviewBuilder builder];
    [builder option:FASTPREVIEW_NORMAL optionValue:1];
    return [builder buildCommand];
}

#pragma mark - Private API

- (instancetype) initWithOptions:(NSDictionary *) options
{
    self = [super init];
    if (self) {
        self.options = options;
    }
    return self;
}
- (void) dealloc
{
    self.options = nil;
    [super dealloc];
}
#pragma mark - FastPreviewCommand
- (NSString *) stringRepresentation
{
    NSMutableString *buildString = [NSMutableString string];
    
    for(NSString *key in self.options) {
        if(buildString.length > 0) {
            [buildString appendString:@" "];
        }
        int value = (int)[self.options[key] integerValue];
        [buildString appendString:[NSString stringWithFormat:@"%@=%d", key, value]];
    }
    return [[buildString copy] autorelease];
}
@end

@implementation NSString(FastPreview)
- (NSString *) stringRepresentation
{
    return self;
}
@end

