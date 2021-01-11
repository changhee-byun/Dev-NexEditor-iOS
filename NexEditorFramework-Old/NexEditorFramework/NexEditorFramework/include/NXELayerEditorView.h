/*
 * File Name   : NXELayerEditorView.h
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

#import <UIKit/UIKit.h>
#import "NXELayer.h"
#import "NXEEngine.h"

/**
 * \brief Layer customization must conform this protocol.
 * \since version 1.3.0
 */
@protocol NXELayerEditorCustomization <NSObject>
@end

@class NXELayerEditorView;

/**
 * \ingroup types
 * \brief A list of keys of NXELayerEditorView status. NXELayerEditor's -editorView:didChangeStatus:value: method is invoked whenever one of the status is changed.
 * \since version 1.3.6
 */
typedef NS_ENUM(NSUInteger, NXELayerEditorStatusKey) {
    /*! The custom layer editor has registered or unregistered. */
    NXELayerEditorStatusKeyRegistered,
    /*! The layer editor's editing was enabled or disabled. */
    NXELayerEditorStatusKeyEditingEnabled,
} _DOC_ENUM(NXELayerEditorStatusKey);

/**
 * \brief A custom layer editor implementation must conform this protocol.
 *        A custom layer editor may implement capturing user's gesture, changing position, scaling and rotation angle of a NXELayer.
 * \since version 1.3.0
 */
@protocol NXELayerEditor <NSObject>
/**
 * \brief Defines how the layer editor behaves.
 * \since version 1.3.0
 */
@property (nonatomic, retain) id<NXELayerEditorCustomization> customization;
/**
 * \brief Reading this property returns the layer currently selected by the user. Returns nil if no layer is selected.
 * \since version 1.3.6
 */
@property (nonatomic, readonly) NXELayer *selectedLayer;
/**
 * \brief Invoked whenever NXELayerEditorView's statuses are changed. Statuses include registration or deregistration of the custom layer editor, and editing enabled status.
 *        It is good chance to add gesture recognizers when \c NXELayerEditorStatusKeyRegistered is changed to \c YES while gesture recognizers can be removed when it is changed to \c NO.
 *        Also, when isEditingEnabled property is changed from NO to YES, or vice versa, this method will be invoked with NXELayerEditorStatusKeyEditingEnabled status.
 * \since version 1.3.6
 */
- (void) editorView:(NXELayerEditorView* ) editorView didChangeStatus:(NXELayerEditorStatusKey) status value:(BOOL) value;
@end

/**
 * \brief Layer Coordinates that can convert from and to NXELayerEditorView's coordinates.
 *
 * \c layerCoordinates property of NXELayerEditorView is provided for a NXELayerEditor instance assigned to NXELayerEditorView. If the user tapped on NXELayerEditorView and the location is represented as CGPoint, i.e. from UITapGestureRecognizer, it can be converted to a point in Layer's coordinates so, for example, it can be tested if a rectagular area represented as NXELayer's scaledFrame.
 *
 \code
 NXELayerEditorView *editorView = ...;
 NXELayer *layer = ...;
 CGPoint *center = editorView.center;
 
 CGPoint centerInLayerCoordinates = [editorView.layerCoordinates pointFrom:center];
 if (CGRectContainsPoint(layer.scaledFrame, centerInLayerCoordinates)) {
     // layer's location and editorView.center overlap
 }
 \endcode
 *
 * \since version 1.3.6
 */
@interface NXELayerEditorCoordinates: NSObject

/**
 * \brief Designated initializer for internal use only. Access NXELayerEditorView's \c layerCoordinates property instead of directly creating an instance.
 * \since version 1.3.6
 */
- (instancetype) initWithUIViewRect:(CGRect) rect;

/**
 * \brief Converts a CGPoint value in NXELayerEditorView's coordinates to NXELayer's coordinates that can be tested with \c scaledFrame property.
 * \note The returned point cannot be compared with \c x, \c y, \c width, or \c height properties of NXELayer.
 * \since version 1.3.6
 */
- (CGPoint) pointFrom:(CGPoint) point;
/**
 * \brief Converts CGSize value in NXELayerEditorView's coordinates to NXELayer's coordinates that can be tested with \c scaledFrame property.
 * \note The returned point cannot be compared with \c x, \c y, \c width, or \c height properties of NXELayer.
 * \since version 1.3.6
 */
- (CGSize) sizeFrom:(CGSize) size;
/**
 * \brief Converts a CGRect value in NXELayer's coordinates to NXELayerEditorView's coordinates.
 * \note Typical example of input would be NXELayer's \c scaledFrame property.
 * \since version 1.3.6
 */
- (CGRect) toUIViewRect:(CGRect) rect;
@end

/**
 * \brief UIView subclass to provide for preview or editing layers
 *
 * Set layer property of the instance of this class to NXEEngine instance with \c -setPreviewWithCALayer:previewSize: .
 *
 \code
 self.engine = [NXEEngine instance];
 self.preview = [[NXELayerEditorView alloc] initWithFrame:...];
 
 [self.engine setPreviewWithCALayer:self.preview.layer previewSize:self.preview.bounds.size];
 \endcode
 */
@interface NXELayerEditorView : UIView
/**
 * \brief NXESimplelayerEditor instance is set by default.
 * \since version 1.3.0
 */
@property (nonatomic, retain) id<NXELayerEditor> layerEditor;

/**
 * \brief Reading this property returns the layer currently selected by the user. Returns nil if no layer is selected.
 * \since version 1.2
 */
@property (nonatomic, readonly) NXELayer *selectedLayer;

/**
 * \brief A Boolean value indicates whether to be able to edit layers, such as video layer, image layer and text layer
 * \since version 1.3.5
 */
@property (nonatomic) BOOL isEditingEnabled;

/**
 * \brief Convenience property to access an NXEEngine instance without having to create a new instance.
 * \since version 1.3.6
 */
@property (nonatomic, readonly) NXEEngine *nxeeditor;

/**
 * \brief Represents Layer Coordinates that can be used to convert coordinates between NXELayerEditorView and NXELayer.
 * \since version 1.3.6
 */
@property (nonatomic, readonly) NXELayerEditorCoordinates *layerCoordinates;

/**
 * \brief Updates preview with changes made to NXELayer instances set to the current project attached to an NXEEngine instance.
 * \since version 1.3.6
 */
- (void) updatePreview;

/**
 * \brief Finds and returns a NXELayer at point in UIView coordinates from 'layers'
 * \since version 1.3.6
 */
- (NXELayer *) layerAtUIViewPoint:(CGPoint) point layers:(NSArray<NXELayer *> *) layers;
@end

/**
 * \brief Deprecated. Use NXELayerEditorView instead.
 * \deprecated Use NXELayerEditorView instead.
 */
__attribute__ ((deprecated))
@interface NXEEngineView : NXELayerEditorView
@end
