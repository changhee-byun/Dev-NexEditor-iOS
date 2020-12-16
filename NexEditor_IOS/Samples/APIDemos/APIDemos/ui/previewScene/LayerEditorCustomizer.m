/*
 * File Name   : LayerEditorCustomizer.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "LayerEditorCustomizer.h"

@implementation LayerEditorCustomizer

+ (UIImage *)buttonImageWithLayerButtonType:(NXESimpleLayerEditorButtonType)buttonType
{
    NSString *filename = nil;
    NSArray<NSString *> *imageNames = @[@"delete", @"rotate", @"scale", @"check"];
    if (buttonType >= NXESimpleLayerEditorButtonTypeDelete && buttonType <= NXESimpleLayerEditorButtonTypeCustom) {
        filename = imageNames[((int)buttonType - 1)];
    }
    return [UIImage imageNamed:filename];
}

+ (NXESimpleLayerEditorButtonCustomization *) buttonCustomizationForType:(NXESimpleLayerEditorButtonType) type position:(NXESimpleLayerEditorButtonPosition) position
{
    return [[NXESimpleLayerEditorButtonCustomization alloc] initWithType:type
                                                                position:position
                                                                   image:[self buttonImageWithLayerButtonType:type]];
}

+ (NXESimpleLayerEditorCustomization *) customizationWithButtonTap:(void (^)(NXESimpleLayerEditorButtonType, NXELayer *)) didTap
{
    NSArray<NXESimpleLayerEditorButtonCustomization *> *buttons = @[
                                                                    [self buttonCustomizationForType:NXESimpleLayerEditorButtonTypeDelete
                                                                                            position:NXESimpleLayerEditorButtonPositionUpperRight],
                                                                    [self buttonCustomizationForType:NXESimpleLayerEditorButtonTypeRotate
                                                                                            position:NXESimpleLayerEditorButtonPositionBottomLeft],
                                                                    [self buttonCustomizationForType:NXESimpleLayerEditorButtonTypeCustom
                                                                                            position:NXESimpleLayerEditorButtonPositionBottomRight],
                                                                    [self buttonCustomizationForType:NXESimpleLayerEditorButtonTypeScale
                                                                                            position:NXESimpleLayerEditorButtonPositionUpperLeft],
                                                                    ];
    NXESimpleLayerEditorSelectionBorderStyle * border = [[NXESimpleLayerEditorSelectionBorderStyle alloc] initWithThickness:3.0 color:UIColor.whiteColor];
    
    
    return [[NXESimpleLayerEditorCustomization alloc] initWithButtons:buttons border:border buttonTapped:^ (NXESimpleLayerEditor *simpleLayerEditor, NXESimpleLayerEditorButtonType buttonType, NXELayer *layer) {
        if (didTap) didTap(buttonType, layer);
        
    }];
}
@end
