/*
 * File Name   : NXESimpleLayerEditor.h
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

#import "NXELayerEditorView.h"

@class NXESimpleLayerEditor;

/**
 * \ingroup types
 * \brief A list of button types to be displayed on the selected layer.
 * \since version 1.3.0
 */
typedef NS_ENUM ( NSUInteger, NXESimpleLayerEditorButtonType ) {
    
    NXESimpleLayerEditorButtonTypeNone,
    NXESimpleLayerEditorButtonTypeDelete,
    NXESimpleLayerEditorButtonTypeRotate,
    NXESimpleLayerEditorButtonTypeScale,
    NXESimpleLayerEditorButtonTypeCustom,
    NXESimpleLayerEditorButtonTypeScaleRotate,
    
} _DOC_ENUM(NXESimpleLayerEditorButtonType);

/**
 * \ingroup types
 * \brief A list of button position to be displayed on the selected layer.
 * \since version 1.3.0
 */
typedef NS_ENUM ( NSUInteger, NXESimpleLayerEditorButtonPosition ) {
    
    NXESimpleLayerEditorButtonPositionNone,
    NXESimpleLayerEditorButtonPositionUpperLeft,
    NXESimpleLayerEditorButtonPositionUpperRight,
    NXESimpleLayerEditorButtonPositionBottomLeft,
    NXESimpleLayerEditorButtonPositionBottomRight
    
} _DOC_ENUM(NXESimpleLayerEditorButtonPosition);

/**
 * \brief A block to be called when the user taps on a button of the selected layer in the layer editor view.
 * \see NXESimpleLayerEditorCustomization class
 * \since version 1.3.0
 */
typedef void(^NXESimpleLayerEditorButtonTappedBlock)(NXESimpleLayerEditor *, NXESimpleLayerEditorButtonType, NXELayer *);

/**
 * \brief A class describes customization for a button to be displayed on the selected layer.
 *
 \code
 NXESimpleLayerEditorButtonCustomization *button = [[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeDelete position:NXESimpleLayerEditorButtonPositionUpperLeft image:[UIImage imageNamed:@"delete"]];
 \endcode
 * \since version 1.3.0
 */
@interface NXESimpleLayerEditorButtonCustomization : NSObject
/**
 * \brief Type of the button.
 * \since version 1.3.0
 */
@property (nonatomic) NXESimpleLayerEditorButtonType type;
/**
 * \brief Position of the button
 * \since version 1.3.0
 */
@property (nonatomic) NXESimpleLayerEditorButtonPosition position;
/**
 * \brief Icon image for the button
 * \since version 1.3.0
 */
@property (nonatomic, retain) UIImage *image;
/**
 * \brief  Designated initializer
 * \since version 1.3.0
 */
- (instancetype) initWithType:(NXESimpleLayerEditorButtonType)type position:(NXESimpleLayerEditorButtonPosition)position image:(UIImage *)image;

@end

/**
 * \brief The class describes border style for the selected layer.
 *
 \code
 NXESimpleLayerEditorSelectionBorderStyle* border = [[NXESimpleLayerEditorSelectionBorderStyle alloc] initWithThickness:3.0 color:UIColor.white];
 \endcode
 
 * \since version 1.3.0
 */
@interface NXESimpleLayerEditorSelectionBorderStyle : NSObject
/**
 * \brief The thickness of border in points.
 * \since version 1.3.0
 */
@property (nonatomic) CGFloat thickness;
/**
 * \brief The color of the border.
 * \since version 1.3.0
 */
@property (nonatomic, retain) UIColor *color;
/**
 * \brief Designated initializer
 * \since version 1.3.0
 */
- (instancetype) initWithThickness:(CGFloat)thickness color:(UIColor *)color;

@end

/**
 * \brief Describes layer editor customization that includes border style and buttons.
 *
 \code
 
 NSArray *buttons = @[
    [[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeDelete position:NXESimpleLayerEditorButtonPositionUpperLeft image:[UIImage imageNamed:@"delete"]],
    [[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeCustom position:NXESimpleLayerEditorButtonPositionUpperRIght image:[UIImage imageNamed:@"custom"]],
    [[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeRotate position:NXESimpleLayerEditorButtonPositionBottomLeft image:[UIImage imageNamed:@"rotate"]],
    [[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeScale position:NXESimpleLayerEditorButtonPositionBottomRight image:[UIImage imageNamed:@"scale"]]
 ];
 
 NXESimpleLayerEditorSelectionBorderStyle* border = [[NXESimpleLayerEditorSelectionBorderStyle alloc] initWithThickness:3.0 color:UIColor.white];
 
 __weak NXEEngine *weakEngine = self.engine;
 
 NXESimpleLayerEditorCustomization *customization = [[NXESimpleLayerEditorCustomization alloc] initWithButtons:buttons border:border buttonTapped: ^(NXESimpleLayerEditor *simpleLayerEditor, NXESimpleLayerEditorButtonType buttonType, NXELayer *layer) {
        switch( buttonType ) {
            case NXESimpleLayerEditorButtonTypeDelete: {
                NSMutableArray *layers = [weakEngine.project.layers mutableCopy];
                [layers removeObject:layer];
                weakEngine.project.layers = layers;

                // refresh
                [weakEngine fastOptionPreview:NXE_NORMAL optionValue:nil display:YES];
                break;
            }
            case NXESimpleLayerEditorButtonTypeCustom: {
                break;
            }
        }
    }];
 
 self.engineView.layerEditor.customization = customization;
 \endcode
 * \since version 1.3.0
 */
@interface NXESimpleLayerEditorCustomization : NSObject <NXELayerEditorCustomization>
/**
 * \brief List of button customizations.
 * \since version 1.3.0
 */
@property (nonatomic, copy) NSArray <NXESimpleLayerEditorButtonCustomization *> *buttons;
/**
 * \brief Border style customization
 * \since version 1.3.0
 */
@property (nonatomic, retain) NXESimpleLayerEditorSelectionBorderStyle *border;
/**
 * \brief Block assigned to this property will be invoked whenever a button is tapped.
 * \since version 1.3.0
 */
@property (copy, nonatomic) NXESimpleLayerEditorButtonTappedBlock buttonTapped;
/**
 * \brief Designated initializer.
 * \since version 1.3.0
 */
- (instancetype) initWithButtons:(NSArray <NXESimpleLayerEditorButtonCustomization *> *)buttons border:(NXESimpleLayerEditorSelectionBorderStyle *)border buttonTapped:(NXESimpleLayerEditorButtonTappedBlock)buttonTapped;

@end

/**
 * \brief The default layer editor. This is the only layer editor can be used as of the current versino of the SDK and NXELayerEditor's layerEditor property has an instance of this class by default.
 * \since version 1.3.0
 */
@interface NXESimpleLayerEditor : NSObject <NXELayerEditor>
/**
 * \brief Layer editor customization. Create and set an instance NXESimpleLayerEditorCustomization to this property.
 * \since version 1.3.0
 */
@property (nonatomic, retain) id<NXELayerEditorCustomization> customization;

@end
