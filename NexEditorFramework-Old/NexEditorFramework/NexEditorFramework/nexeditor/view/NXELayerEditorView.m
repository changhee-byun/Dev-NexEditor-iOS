/*
 * File Name   : NXELayerEditorView.m
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
#import "LayerManager.h"
#import "NXEEngine.h"
#import "KMLayer.h"
#import "KMTextLayer.h"
#import "KMImageLayer.h"
#import "NXEProject.h"
#import "NXELayersManager.h"
#import "NXESimpleLayerEditor.h"

@interface NXELayer(LayerEditor)
@property (nonatomic, readonly) float angleRadian;
- (BOOL) isActiveAtTime:(int) time;
- (BOOL) isHitAt:(CGPoint) point;
@end

@interface NXELayerEditorCoordinates()
@property (nonatomic) CGAffineTransform transformToUIView;
@property (nonatomic) CGAffineTransform transformFromUIView;
@end

@interface NXELayerEditorView()
@property (nonatomic, strong) NXEEngine *nxeeditor;
@property (nonatomic, strong) NXELayerEditorCoordinates *layerCoordinates;

@end

#pragma mark - NXELayerEditorView

@implementation NXELayerEditorView
@synthesize layerEditor = _layerEditor;

- (void) dealloc
{
    self.layerEditor = nil;
    self.nxeeditor = nil;
    self.layerCoordinates = nil;
    [super dealloc];
}

- (void)initialise
{
    self.layerEditor = [[[NXESimpleLayerEditor alloc] init] autorelease];
    self.isEditingEnabled = YES;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self initialise];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if (self) {
        [self initialise];
    }
    return self;
    
}

#pragma mark - Properties

- (id<NXELayerEditor>) layerEditor
{
    if (_layerEditor == nil) {
        _layerEditor = [[NXESimpleLayerEditor alloc] init];
        [_layerEditor editorView:self didChangeStatus:NXELayerEditorStatusKeyRegistered value:YES];
    }
    return _layerEditor;
}

- (void) setLayerEditor:(id<NXELayerEditor>)layerEditor
{
    if (_layerEditor) {
        [_layerEditor editorView:self didChangeStatus:NXELayerEditorStatusKeyRegistered value:NO];
    }
    [_layerEditor release];
    
    _layerEditor = [layerEditor retain];
    if ( _layerEditor ) {
        [_layerEditor editorView:self didChangeStatus:NXELayerEditorStatusKeyRegistered value:YES];
    }
}

- (NXELayer *) selectedLayer
{
    return self.layerEditor.selectedLayer;
}

- (void) setIsEditingEnabled:(BOOL)isEditingEnabled
{
    if (_isEditingEnabled != isEditingEnabled) {
        _isEditingEnabled = isEditingEnabled;
        [self.layerEditor editorView:self didChangeStatus:NXELayerEditorStatusKeyEditingEnabled value:isEditingEnabled];
    }
}

- (NXEEngine *) nxeeditor
{
    if (_nxeeditor == nil) {
        _nxeeditor = [NXEEngine.instance retain];
    }
    return _nxeeditor;
}

- (CAEAGLLayer *) previewLayer {
    CAEAGLLayer *result = nil;
    for (CALayer *layer in self.layer.sublayers) {
        if ( [layer isKindOfClass:CAEAGLLayer.class] ) {
            result = (CAEAGLLayer *) layer;
        }
    }
    return result;
}

- (NXELayerEditorCoordinates *) layerCoordinates
{
    if (_layerCoordinates == nil) {
        if (self.previewLayer != nil) {
            CGRect rect = CGRectMake(self.previewLayer.frame.origin.x,
                                     self.previewLayer.frame.origin.y,
                                     self.previewLayer.bounds.size.width,
                                     self.previewLayer.bounds.size.height);
            _layerCoordinates = [[NXELayerEditorCoordinates alloc] initWithUIViewRect:rect];
        }
    }
    return _layerCoordinates;
}

#pragma mark - API
- (void) updatePreview
{
    [self.nxeeditor fastOptionPreview:NXE_NORMAL optionValue:nil display:YES];
}

- (NXELayer *) layerAtUIViewPoint:(CGPoint) point layers:(NSArray<NXELayer *> *) layers
{
    NXELayer *result = nil;
    if (self.layerCoordinates == nil) { return result; }
    int currentTime = [self.nxeeditor getCurrentPosition];
    CGPoint layerPoint = [self.layerCoordinates pointFrom:point];
    
    for (NXELayer *layer in layers.reverseObjectEnumerator) {
        if ( [layer isActiveAtTime:currentTime] && [layer isHitAt:layerPoint] ) {
            result = layer;
            break;
        }
    }
    return result;
}

@end

@implementation NXELayerEditorCoordinates

- (instancetype) initWithUIViewRect:(CGRect) rect
{
    self = [super init];
    if ( self ) {
        CGAffineTransform toUIView = CGAffineTransformMakeTranslation(rect.origin.x, rect.origin.y);
        self.transformToUIView = CGAffineTransformScale(toUIView, rect.size.width, rect.size.height);
        self.transformFromUIView = CGAffineTransformInvert(self.transformToUIView);
    }
    return self;
}

- (CGPoint) pointFrom:(CGPoint) point
{
    return CGPointApplyAffineTransform(point, self.transformFromUIView);
}

- (CGSize) sizeFrom:(CGSize) size
{
    return CGSizeApplyAffineTransform(size, self.transformFromUIView);
}

- (CGRect) toUIViewRect:(CGRect) rect
{
    return CGRectApplyAffineTransform(rect, self.transformToUIView);
}

@end

@implementation NXELayer(LayerEditor)
- (float) angleRadian
{
    return self.angle * M_PI / 180;
}

- (BOOL) isActiveAtTime:(int) time
{
    return self.startTime <= time && time <= self.endTime;
}

- (BOOL) isHitAt:(CGPoint) point
{
    CGPoint center = CGPointMake(CGRectGetMidX(self.scaledFrame), CGRectGetMidY(self.scaledFrame));
    CGSize scaledSize = CGSizeMake(self.scaledFrame.size.width * self.scale, self.scaledFrame.size.height * self.scale);
    CGRect scaledRect = CGRectMake(center.x - scaledSize.width / 2, center.y - scaledSize.height / 2, scaledSize.width, scaledSize.height);
    
    CGAffineTransform transform = CGAffineTransformMakeRotation(-(self.angleRadian));
    CGPoint reverseRotated = CGPointMake(point.x - center.x, point.y - center.y);
    reverseRotated = CGPointApplyAffineTransform(reverseRotated, transform);
    reverseRotated = CGPointMake(reverseRotated.x + center.x, reverseRotated.y + center.y);

    return CGRectContainsPoint(scaledRect, reverseRotated);
}

@end

@implementation NXEEngineView : NXELayerEditorView
@end
