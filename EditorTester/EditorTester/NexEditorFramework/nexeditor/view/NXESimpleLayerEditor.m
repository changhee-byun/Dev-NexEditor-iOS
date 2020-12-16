/*
 * File Name   : NXESimpleLayerEditor.m
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

#import "NXESimpleLayerEditor.h"
#import "LayerManager.h"
#import "KMLayer.h"
#import "NXEEngine.h"
#import "NXEProject.h"
#import "ImageUtil.h"
#import "MathUtil.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "UIImage+Resize.h"

#define LOG_TAG @"SimpleLayerEditor"

#pragma mark - NXESimpleLayerEditorButtonCustomization

@implementation NXESimpleLayerEditorButtonCustomization

- (void) dealloc
{
    self.image = nil;
    
    [super dealloc];
}

- (instancetype) initWithType:(NXESimpleLayerEditorButtonType)type position:(NXESimpleLayerEditorButtonPosition)position image:(UIImage *)image
{
    self = [super init];
    
    self.type = type;
    self.position = position;
    self.image = image;
    
    return self;
}
#define LAYEREDITORBUTTON_MIN_WIDTH     (55)    // points
#define LAYEREDITORBUTTON_MIN_HEIGHT    (55)    // points

- (void) setImage:(UIImage *)image
{
    [_image release];
    if (image && ((image.size.width < LAYEREDITORBUTTON_MIN_WIDTH) || (image.size.height < LAYEREDITORBUTTON_MIN_HEIGHT))) {
        CGSize expandedSize = CGSizeMake(MAX(image.size.width, LAYEREDITORBUTTON_MIN_WIDTH), MAX(image.size.height, LAYEREDITORBUTTON_MIN_HEIGHT) );
        image = [image expandingToSize:expandedSize backgroundColor:UIColor.clearColor];
    }
    _image = [image retain];
}
@end

#pragma mark - NXESimpleLayerEditorSelectionBorderStyle

@implementation NXESimpleLayerEditorSelectionBorderStyle

- (void) dealloc
{
    self.color = nil;
    
    [super dealloc];
}

- (instancetype) init
{
    return [self initWithThickness:2.0 color:UIColor.redColor];
}

- (instancetype) initWithThickness:(CGFloat)thickness color:(UIColor *)color
{
    self = [super init];
    
    self.thickness = thickness;
    self.color = color;
    
    return self;
}

@end

#pragma mark - NXESimpleLayerEditorCustomization

@implementation NXESimpleLayerEditorCustomization

- (void) dealloc
{
    self.buttons = nil;
    self.border = nil;
    self.buttonTapped = nil;
    
    [super dealloc];
}

- (instancetype) init
{
    NSArray<NXESimpleLayerEditorButtonCustomization *> *buttons = @[
                        [[[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeRotate
                                                                             position:NXESimpleLayerEditorButtonPositionUpperRight
                                                                                image:[UIImage imageWithContentsOfFile:[[NSBundle bundleForClass:self.class] pathForResource:@"grip_rotate" ofType:@"png" inDirectory:@"resource/image"]]] autorelease],
                        
                        [[[NXESimpleLayerEditorButtonCustomization alloc] initWithType:NXESimpleLayerEditorButtonTypeScale
                                                                             position:NXESimpleLayerEditorButtonPositionBottomRight
                                                                                image:[UIImage imageWithContentsOfFile:[[NSBundle bundleForClass:self.class] pathForResource:@"grip_scale" ofType:@"png" inDirectory:@"resource/image"]]] autorelease]
                        ];
    
    NXESimpleLayerEditorSelectionBorderStyle *border = [[[NXESimpleLayerEditorSelectionBorderStyle alloc] initWithThickness:2.0 color:UIColor.redColor] autorelease];
    
    return [self initWithButtons:buttons border:border buttonTapped:nil];
}

- (instancetype) initWithButtons:(NSArray<NXESimpleLayerEditorButtonCustomization *> *)buttons border:(NXESimpleLayerEditorSelectionBorderStyle *)border buttonTapped:(NXESimpleLayerEditorButtonTappedBlock)buttonTapped
{
    self = [super init];
    
    self.buttons = buttons;
    self.border = border;
    self.buttonTapped = buttonTapped;
    
    return self;
}

- (void) setBorder:(NXESimpleLayerEditorSelectionBorderStyle *)border
{
    [_border release];
    
    if ( border == nil ) {
        return;
    }
    
    _border = [border retain];
    
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    if ( selectedLayer != nil ) {
        [selectedLayer setLayerEditorBorderWithColor:border.color thickness:border.thickness];
    }
}

- (void) setButtons:(NSArray<NXESimpleLayerEditorButtonCustomization *> *)buttons
{
    [_buttons release];
    
    if ( buttons == nil ) {
        return;
    }
    
    _buttons = [buttons copy];
    
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    if ( selectedLayer != nil ) {
        LayerButtonPosition btnPostion = LAYER_BTN_POSITION_NONE;
        
        for ( NXESimpleLayerEditorButtonCustomization *btnCustomization in buttons ) {
            if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionUpperLeft ) {
                btnPostion = LAYER_BTN_POSITION_UPPER_LEFT;
            } else if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionUpperRight ) {
                btnPostion = LAYER_BTN_POSITION_UPPER_RIGHT;
            } else if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionBottomLeft ) {
                btnPostion = LAYER_BTN_POSITION_BOTTOM_LEFT;
            } else if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionBottomRight ) {
                btnPostion = LAYER_BTN_POSITION_BOTTOM_RIGHT;
            }
            
            if ( btnPostion != LAYER_BTN_POSITION_NONE ) {
                CGFloat imageWidth = btnCustomization.image.size.width * LayerManager.sharedInstance.ratioWidthRendererToVideoRect;
                CGFloat imageHeight = btnCustomization.image.size.height * LayerManager.sharedInstance.ratioHeightRendererToVideoRect;
                CGSize imageSize = CGSizeMake(imageWidth, imageHeight);

                [selectedLayer setLayerEditorButtonWithPosition:btnPostion image:btnCustomization.image imageSize:imageSize];
            }
        }
    }
}

@end

#pragma mark - NXESimpleLayerEditor
@interface NXESimpleLayerEditor()
@property (nonatomic) BOOL isSelectionVisible;
@property (nonatomic) BOOL isEditingEnabled;
@property (nonatomic, strong) UITapGestureRecognizer *tapRecognizer;
@property (nonatomic, strong) UIPanGestureRecognizer *panRecognizer;
@property(nonatomic) CGPoint startPoint;
@property(nonatomic) CGPoint originPoint;
@end

@implementation NXESimpleLayerEditor

- (void) setCustomization:(id<NXELayerEditorCustomization>)customization
{
    [_customization release];
    _customization = nil;
    
    if ( customization == nil ) {
        customization = [[[NXESimpleLayerEditorCustomization alloc] init] autorelease];
    }

    if ([customization isKindOfClass:NXESimpleLayerEditorCustomization.class]) {
        _customization = [customization retain];
    } else {
        NexLogE(LOG_TAG, @"Discarding incompatible customization:%@", NSStringFromClass(customization.class));
    }
}

- (instancetype) init
{
    self = [super init];
    if (self) {
    self.customization = [[[NXESimpleLayerEditorCustomization alloc] init] autorelease];
    self.isSelectionVisible = YES;
    }
    return self;
}

- (void) dealloc
{
    [self.customization release];
    self.tapRecognizer = nil;
    self.panRecognizer = nil;
    [super dealloc];
}

#pragma mark - NXELayerEditor

- (NXELayer *) selectedLayer
{
    NXELayer *result = nil;
    KMLayer *kmlayer = [[LayerManager sharedInstance] getHittedLayer];
    if ( kmlayer ) {
/*
        for ( NXELayer *layer in NXEEngine.instance.project.layers ) {
            if ( layer.layerId == kmlayer.layerId ) {
                result = layer;
                break;
            }
        }
*/
        
    }
    return result;
}

- (void) didRegisterTo:(NXELayerEditorView *) editorView
{
    self.tapRecognizer = [[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapGesture:)] autorelease];
    self.panRecognizer = [[[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panGesture:)] autorelease];
    [editorView addGestureRecognizer:self.tapRecognizer];
    [editorView addGestureRecognizer:self.panRecognizer];
    
    self.isEditingEnabled = editorView.isEditingEnabled;
}

- (void) didUnregisterFrom:(NXELayerEditorView *) editorView
{
    [editorView removeGestureRecognizer:self.tapRecognizer];
    [editorView removeGestureRecognizer:self.panRecognizer];
    self.tapRecognizer = nil;
    self.panRecognizer = nil;
}

- (void) editorView:(NXELayerEditorView* ) editorView didChangeEnableStatus:(BOOL) enabled
{
    self.isEditingEnabled = enabled;
    self.isSelectionVisible = enabled;
}

- (void) editorView:(NXELayerEditorView* ) editorView didChangeStatus:(NXELayerEditorStatusKey) status value:(BOOL) enabled
{
    switch(status) {
        case NXELayerEditorStatusKeyRegistered:
            if (enabled) {
                [self didRegisterTo:editorView];
            } else {
                [self didUnregisterFrom:editorView];
            }
            break;
        case NXELayerEditorStatusKeyEditingEnabled:
            [self editorView:editorView didChangeEnableStatus:enabled];
            break;
    }
}

#pragma mark -

- (void) setIsSelectionVisible:(BOOL) visible
{
    _isSelectionVisible = visible;
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    if ( selectedLayer && selectedLayer.isHit != visible ) {
        selectedLayer.isHit = visible;
//        [NXEEngine.instance fastOptionPreview:NXE_NORMAL optionValue:nil display:YES];
    }
}

#pragma mark - Gesture

- (void)configureLayerEditorToSelectedLayer:(KMLayer *)layer
{
    NXESimpleLayerEditorCustomization *customization = (NXESimpleLayerEditorCustomization *)self.customization;
    if ( customization ) {
        
        NXESimpleLayerEditorSelectionBorderStyle *border = customization.border;
        [layer setLayerEditorBorderWithColor:border.color thickness:border.thickness];
        
        LayerButtonPosition btnPostion = LAYER_BTN_POSITION_NONE;
        
        for ( NXESimpleLayerEditorButtonCustomization *btnCustomization in customization.buttons ) {
            if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionUpperLeft ) {
                btnPostion = LAYER_BTN_POSITION_UPPER_LEFT;
            } else if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionUpperRight ) {
                btnPostion = LAYER_BTN_POSITION_UPPER_RIGHT;
            } else if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionBottomLeft ) {
                btnPostion = LAYER_BTN_POSITION_BOTTOM_LEFT;
            } else if ( btnCustomization.position == NXESimpleLayerEditorButtonPositionBottomRight ) {
                btnPostion = LAYER_BTN_POSITION_BOTTOM_RIGHT;
            }
            
            if ( btnPostion != LAYER_BTN_POSITION_NONE ) {
                CGFloat imageWidth = btnCustomization.image.size.width * LayerManager.sharedInstance.ratioWidthRendererToVideoRect;
                CGFloat imageHeight = btnCustomization.image.size.height * LayerManager.sharedInstance.ratioHeightRendererToVideoRect;
                CGSize imageSize = CGSizeMake(imageWidth, imageHeight);
                
                [layer setLayerEditorButtonWithPosition:btnPostion image:btnCustomization.image imageSize:imageSize];
            }
        }
    }
}

- (void)tapGesture:(UITapGestureRecognizer *)recognizer
{
    if ( self.isEditingEnabled == NO ) {
        return;
    }
    UIView *editorView = recognizer.view;
    CGPoint touchPoint = [recognizer locationInView: editorView];
    
    KMLayer* layer = [[LayerManager sharedInstance] getLayerAtTime:[[NXEEngine instance] getCurrentPosition]
                                                      WithPosition:touchPoint];
    
    KMLayer* selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    
    if( selectedLayer != layer ) {
        [self configureLayerEditorToSelectedLayer:layer];
        [[LayerManager sharedInstance] changeHittedLayer:layer];
    }
    //선택된 레이어가 같으면 터치 이벤트를 받도록 한다.
    else {
        self.startPoint = touchPoint;
    }
    
    if( layer && layer.isHit ) {
        NXESimpleLayerEditorCustomization *customization = (NXESimpleLayerEditorCustomization *)self.customization;
        if ( customization ) {
            KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
            KMLayerHitType type = [selectedLayer getHitType];
            
            if ( type != KM_LAYER_HIT_NONE && type != KM_LAYER_HIT_BODY ) {
                NXESimpleLayerEditorButtonType buttonType = [self getLayerButtonTypeWithLayerHitType:type];
                if ( customization.buttonTapped != nil ) {
                    customization.buttonTapped(self, buttonType, self.selectedLayer);
                }
            }
        }
    }
}

- (NXESimpleLayerEditorButtonType)getLayerButtonTypeWithLayerHitType:(KMLayerHitType)hitType
{
    NXESimpleLayerEditorButtonType btnType = NXESimpleLayerEditorButtonTypeNone;
    NXESimpleLayerEditorButtonPosition btnPosition = NXESimpleLayerEditorButtonPositionNone;
    
    if ( hitType == KM_LAYER_HIT_UPPER_LEFT ) {
        btnPosition = NXESimpleLayerEditorButtonPositionUpperLeft;
    } else if ( hitType == KM_LAYER_HIT_UPPER_RIGHT ) {
        btnPosition = NXESimpleLayerEditorButtonPositionUpperRight;
    } else if ( hitType == KM_LAYER_HIT_BOTTOM_LEFT ) {
        btnPosition = NXESimpleLayerEditorButtonPositionBottomLeft;
    } else if ( hitType == KM_LAYER_HIT_BOTTOM_RIGHT ) {
        btnPosition = NXESimpleLayerEditorButtonPositionBottomRight;
    }
    
    if ( [self.customization isKindOfClass:NXESimpleLayerEditorCustomization.class] ) {
        NXESimpleLayerEditorCustomization *customization = (NXESimpleLayerEditorCustomization *)self.customization;
        
        for ( NXESimpleLayerEditorButtonCustomization *editorType in customization.buttons ) {
            if ( editorType.position == btnPosition ) {
                btnType = editorType.type;
                break;
            }
        }
    }
    
    return btnType;
}

- (void)moveWithTouchPoint:(CGPoint)touchPoint
{
    //터치 시작된 지점과 현재 지점의 차이를 계산한다.
    CGPoint p1 = [LayerManager.sharedInstance convertPreviewPoint:touchPoint];
    CGPoint p2 = [LayerManager.sharedInstance convertPreviewPoint:self.startPoint];
    
    //프리뷰에서 이동된 값을 렌더러에 맞게 변환한다.
    CGSize move = CGSizeMake(p1.x - p2.x, p1.y - p2.y);
    
    //레이어의 위치를 이동한다. 원래 위치 + 이동된 값.
    NXELayer *nxeLayer = self.selectedLayer;
    nxeLayer.x = self.originPoint.x + move.width;
    nxeLayer.y = self.originPoint.y + move.height;
    
    //변화된 레이어를 다시 그린다.
    [[LayerManager sharedInstance] refresh];
}

- (void)scaleWithTouchPoint:(CGPoint)touchPoint
{
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    
    //레이어의 중심 좌표를 프리뷰의 좌표로 변환한다.
    CGPoint center = [selectedLayer getCenter];
    
    //레이어의 크기를 프리뷰의 좌표로 변환한다.
    CGPoint size = CGPointMake(selectedLayer.width, selectedLayer.height);
    
    //원본 배율일 경우 중심점에서 모서리까지의 거리를 계산한다.
    float base = [MathUtil getDistanceP1:CGPointMake(size.x/2, size.y/2)];
    
    CGPoint touchInLayer = [[LayerManager sharedInstance] convertPreviewPoint:touchPoint];
    //현재 사용자의 터치좌표와 중심점까지의 거리를 계산한다.
    float target = [MathUtil getDistanceP1:center P2:touchInLayer];
    
    //사용자가 터치하여 수정한 scale 값을 계산한다.
    float scale = target / base;
    NXELayer *nxeLayer = self.selectedLayer;
    nxeLayer.scale = scale;
    
    //레이어를 다시 그린다.
    [[LayerManager sharedInstance] refresh];
}

- (void)rotateWithTouchPoint:(CGPoint)touchPoint pointBtnEditorRotate:(CGPoint)pointBtnEditorRotate
{
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    
    //레이어의 중심 좌표
    CGPoint center = [selectedLayer getCenter];
    
    //사용자의 터치 좌표를 렌더러 좌표로 변환.
    CGPoint convertedTouchPoint = [[LayerManager sharedInstance] convertPreviewPoint:touchPoint];
    
    //중심좌표와 rotate 아이콘 중심 좌표의 각도를 계산다. 기본 각도
    int baseAngle = [MathUtil getAngleP1:center P2:pointBtnEditorRotate];
    
    //중심좌표와 사용자 터치 좌표의 각도를 계산한다. 사용자가 변형한 각도
    int newAngle = [MathUtil getAngleP1:center P2:convertedTouchPoint];
    int gap = newAngle - baseAngle; //두 각도 차를 계산한다. 레이어의 각도.
    NXELayer *nxeLayer = self.selectedLayer;
    nxeLayer.angle = gap;
    
    //레이어를 새로 그린다.
    [[LayerManager sharedInstance] refresh];
}

- (void)scaleRotateWithTouchPoint:(CGPoint)touchPoint pointBtnEditorRotate:(CGPoint)pointBtnEditorRotate
{
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    
    //레이어의 중심 좌표를 프리뷰의 좌표로 변환한다.
    CGPoint center = [selectedLayer getCenter];
    
    //레이어의 크기를 프리뷰의 좌표로 변환한다.
    CGPoint size = CGPointMake(selectedLayer.width, selectedLayer.height);
    
    //원본 배율일 경우 중심점에서 모서리까지의 거리를 계산한다.
    float base = [MathUtil getDistanceP1:CGPointMake(size.x/2, size.y/2)];
    
    CGPoint touchInLayer = [[LayerManager sharedInstance] convertPreviewPoint:touchPoint];
    //현재 사용자의 터치좌표와 중심점까지의 거리를 계산한다.
    float target = [MathUtil getDistanceP1:center P2:touchInLayer];
    
    //사용자가 터치하여 수정한 scale 값을 계산한다.
    float scale = target / base;
    
    //중심좌표와 rotate 아이콘 중심 좌표의 각도를 계산다. 기본 각도
    int baseAngle = [MathUtil getAngleP1:center P2:pointBtnEditorRotate];
    
    //중심좌표와 사용자 터치 좌표의 각도를 계산한다. 사용자가 변형한 각도
    int newAngle = [MathUtil getAngleP1:center P2:touchInLayer];
    int gap = newAngle - baseAngle; //두 각도 차를 계산한다. 레이어의 각도.
    
    NXELayer *nxeLayer = self.selectedLayer;
    nxeLayer.scale = scale;
    nxeLayer.angle = gap;
    
    //레이어를 새로 그린다.
    [[LayerManager sharedInstance] refresh];
}
- (CGPoint) getPointBtnEditorRotateWithLayerHitType:(KMLayerHitType)layerHitType
{
    KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
    
    CGPoint center = [selectedLayer getCenter];
    CGPoint point = CGPointZero;
    
    if ( self.customization ) {
        if ( layerHitType == KM_LAYER_HIT_UPPER_LEFT ) {
            point = CGPointMake(center.x - selectedLayer.width/2, center.y - selectedLayer.height/2);
        } else if ( layerHitType == KM_LAYER_HIT_UPPER_RIGHT ) {
            point = CGPointMake(center.x + selectedLayer.width/2, center.y - selectedLayer.height/2);
        } else if ( layerHitType == KM_LAYER_HIT_BOTTOM_LEFT ) {
            point = CGPointMake(center.x - selectedLayer.width/2, center.y + selectedLayer.height/2);
        } else if ( layerHitType == KM_LAYER_HIT_BOTTOM_RIGHT ) {
            point = CGPointMake(center.x + selectedLayer.width/2, center.y + selectedLayer.height/2);
        }
    }
    
    return point;
}

- (void)panGesture:(UIPanGestureRecognizer *)recognizer
{
    if ( self.isEditingEnabled == NO ) {
        return;
    }
    UIView *editorView = recognizer.view;
    CGPoint touchPoint = [recognizer locationInView: editorView];
    
    switch (recognizer.state) {
        case UIGestureRecognizerStateBegan: {
            KMLayer* layer = [[LayerManager sharedInstance] getLayerAtTime:[[NXEEngine instance] getCurrentPosition]
                                                              WithPosition:touchPoint];
            
            KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
            
            if( selectedLayer != layer ) {
                [self configureLayerEditorToSelectedLayer:layer];
                [[LayerManager sharedInstance] changeHittedLayer:layer];
            }
            
            if( layer && layer.isHit ) {
                self.startPoint = touchPoint;
                self.originPoint = CGPointMake(layer.x, layer.y);
            }
            break;
        }
            
        case UIGestureRecognizerStateChanged: {
            touchPoint = [recognizer locationInView: editorView];
            
            KMLayer *selectedLayer = [[LayerManager sharedInstance] getHittedLayer];
            if ( selectedLayer ) {
                KMLayerHitType type = [selectedLayer getHitType];
                
                if ( type == KM_LAYER_HIT_BODY ) {
                    [self moveWithTouchPoint:touchPoint];
                } else {
                    NXESimpleLayerEditorButtonType buttonType = [self getLayerButtonTypeWithLayerHitType:type];
                    if ( buttonType == NXESimpleLayerEditorButtonTypeRotate ) {
                        CGPoint pointBtnEditorRotate = [self getPointBtnEditorRotateWithLayerHitType:type];
                        [self rotateWithTouchPoint:touchPoint pointBtnEditorRotate:pointBtnEditorRotate];
                    } else if ( buttonType == NXESimpleLayerEditorButtonTypeScale ) {
                        [self scaleWithTouchPoint:touchPoint];
                    } else if ( buttonType == NXESimpleLayerEditorButtonTypeScaleRotate ) {
                        CGPoint pointBtnEditorRotate = [self getPointBtnEditorRotateWithLayerHitType:type];
                        [self scaleRotateWithTouchPoint:touchPoint pointBtnEditorRotate:pointBtnEditorRotate];
                    }
                }
            }
            break;
        }
            
        case UIGestureRecognizerStateEnded: {
            break;
        }
            
        default:
            break;
    }
}
@end
