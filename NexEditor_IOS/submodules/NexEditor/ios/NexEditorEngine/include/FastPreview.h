/*
 * File Name   : FastPreview.h
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

typedef enum _FastPreviewOption
{
    FASTPREVIEW_NORMAL = 0,
    FASTPREVIEW_BRIGHTNESS,
    FASTPREVIEW_CONTRAST,
    FASTPREVIEW_SATURATION,
    FASTPREVIEW_PROJECT_BRIGHTNESS,
    FASTPREVIEW_PROJECT_CONTRAST,
    FASTPREVIEW_PROJECT_SATURATION,
    FASTPREVIEW_TINTCOLOR,
    FASTPREVIEW_LEFT,
    FASTPREVIEW_TOP,
    FASTPREVIEW_RIGHT,
    FASTPREVIEW_BOTTOM,
    FASTPREVIEW_NOFX,
    FASTPREVIEW_CTS,
    FASTPREVIEW_SWAPV,
    FASTPREVIEW_VIDEO360,
    FASTPREVIEW_VIDEO360_HORIZONTAL,
    FASTPREVIEW_VIDEO360_VERTICAL,
} FastPreviewOption;


/// Opaque object created by FastPreviewBuilder. Do not directly create object of this class.
@protocol FastPreviewCommand <NSObject>
- (NSString *) stringRepresentation;
@end

/// Plain (or "normal" in legacy API) fast preview command without any option.
extern id<FastPreviewCommand> const kFastPreviewCommandNormal;

@interface FastPreviewBuilder : NSObject

/**
 * \brief Deprecated
 * \deprecated Create a new object using +builder method
 */
- (void)resetBuilder  __attribute__((deprecated));

/**
 * \brief Deprecated
 * \deprecated Use NexEditor.asyncAPI's -runFastPreviewCommand:display:complete instead
 */
- (void)executeBuilder:(BOOL)isDisplay __attribute__((deprecated));

- (void)applyColorAdjustWithBrightness:(int)brightness contrast:(int)contrast saturation:(int)saturation;
- (void)applyProjectColorAdjustments:(float)brightness contrast:(float)contrast saturation:(float)saturation;
- (void)applyTintColor:(int)tintColor;
- (void)applyCropRect:(CGRect)rect;
- (void)applyCropRectWithLeft:(int)left top:(int)top right:(int)right bottom:(int)bottom;
- (void)applyNofx:(BOOL)isNofx;
- (void)applySwapv:(BOOL)isSwapv;
- (void)applyCTS:(int)cts;
- (void)applyVideo360:(BOOL)isVideo360 horizontalBasedX:(int)x verticalBasedY:(int)y;

/// Create a command object for execution with NexEditor.asyncAPI
- (id<FastPreviewCommand>) buildCommand;

+ (instancetype) builder;
@end
