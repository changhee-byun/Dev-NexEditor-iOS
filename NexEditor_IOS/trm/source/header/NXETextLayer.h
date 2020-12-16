/******************************************************************************
 * File Name   :	NXETextLayer.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Proprietary and Confidential
 Copyright (C) 2002~2017 NexStreaming Corp. All rights reserved.
 www.nexstreaming.com
 ******************************************************************************/

#import <UIKit/UIKit.h>

#import "NXELayer.h"

/** \class NXETextLayerProperty
 *  \brief NXETextLayerProperty class defines all properties associated with a text layer.
 *  \since version 1.0.19
 */
@interface NXETextLayerProperty : NSObject

/** \brief A Boolean value whether to use shadow or not.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) BOOL useShadow;

/** \brief The shadow offset value of the text layer.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) CGSize shadowOffset;

/** \brief The shadow color of the text layer.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) UIColor* shadowColor;

/** \brief A Boolean value whether to use glow or not.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) BOOL useGlow;

/** \brief The glow color of the text layer.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) UIColor* glowColor;

/** \brief A Boolean value whether to use outline or not.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) BOOL useOutline;

/** \brief The outline color of the text layer, in a UIColor.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) UIColor* outlineColor;

/** \brief A Boolean value whether to use background or not.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) BOOL useBackground;

/** \brief A Boolean value whether to use extended background or not.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) BOOL useExtendedBackground;

/** \brief The background color of the text layer.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) UIColor* bgColor;

/** \brief The text colot of the text layer.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) UIColor* textColor;

@end

/** \class NXETextLayer
 *  \brief NXETextLayer class defines all methods to manage a text layer.
 *  \since version 1.0.5
 */
@interface NXETextLayer : NXELayer

/** \brief The peoperties of the text layer
 *  \see NXETextLayerProperty, NXETextLayerProperty class.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) NXETextLayerProperty *textLayerProperty;

/** \brief Initializes a new text layer with the speficied text.
 *  \param text The text to display.
 *  \param font The font to set.
 *  \param point The text alignment type, CGPointNXELayerCenter for center alignment.
 *  \return An NXEVideoLayer object.
 *  \since version 1.0.19
 */
- (instancetype)initWithText:(NSString *)text font:(UIFont *)font point:(CGPoint)point;

/** \brief Sets text and font of the text layer.
 *  \param text The text to display.
 *  \param font The font to set.
 *  \since version 1.0.15
 */
- (void)setText:(NSString*)text Font:(UIFont*)font;

/** \brief Applies the updated text layer properties to NXEEngine.
 *  This method should be called right after any properties are updated.
 *  \since version 1.0.19
 */
- (void)updateTextProperty;

/** \brief Returns the current text string of the text layer.
 *  \return A text string.
 *  \since version 1.0.19
 */
- (NSString *)getText;

/** \brief Returns the current font type of the text layer.
 *  \return A font type.
 *  \since version 1.0.19
 */
- (UIFont *)getFont;

@end
