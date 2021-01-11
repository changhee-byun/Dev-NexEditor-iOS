/******************************************************************************
 * File Name   : LayerManager.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <NexEditorEngine/CustomLayerProtocol.h>
#import "NXEEditorType.h"

@class KMLayer;
@class KMAnimationEffect;

/** 레이어를 관리하고 엔진에서 콜백을 받아
 각 레이어를 화면에 그리도록 한다.
 */
@interface LayerManager : NSObject <CustomLayerProtocol>
@property (nonatomic, readonly) CGSize renderRegionSize;
@property (nonatomic) NXESizeInt aspectRatio;
@property (nonatomic, readonly) CGFloat ratioWidthRendererToVideoRect;
@property (nonatomic, readonly) CGFloat ratioHeightRendererToVideoRect;

/** singleton 패턴으로 LayerManager를 반환한다.
 @return LayerManager 객체
 */
+ (LayerManager*)sharedInstance;

/** Video preview region of UIView used for preview
 @param rect Rectangular video region within the UIView used for preview
 */
- (void)setVideoRect:(CGRect) rect;

/** SDK 에서 사용하는 API.
 */
- (void)cleanup;

/** 레이어를 등록한다. 레이어에 대한 고유번호를 반환한다.
 @param layer - 레이어 객체
 @return 등록된 레이어의 고유번호
 */
- (long)addLayer:(KMLayer*)layer;

/** 고유번호에 해당 레이어를 제거한다.
 @param layerId - 등록된 레이어의 고유번호
 */
- (void)removeLayer:(long)layerId;

/** 고유번호에 해당하는 레이어를 반환한다.
 @param layerId - 등록된 레이어의 고유번호
 @return 고유번호에 해당하는 레이어 객체
 */
- (KMLayer*)getLayer:(long)layerId;

/** 모든 레이어를 제거 한다.
 */
- (void)clear:(BOOL)includeWatermark;

/** 레이어를 가장 앞으로 가져온다.
 */
- (void)bringToFront:(long)layerId;

/** 레이어를 가장 앞으로 가져온다.
 */
- (void)bringToFrontLayer:(KMLayer*)layer;

/** 레이어를 가장 뒤로 가져온다.
 */
- (void)bringToBack:(long)layerId;

/** 레이어를 가장 뒤로 가져온다.
 */
- (void)bringToBackLayer:(KMLayer*)layer;

/** 레이어를 화면의 가운데로 이동시킨다. 가로만
 */
- (void)moveToHorizontalCenter:(long)layerId;

/** 레이어를 화면의 가운데로 이동시킨다. 가로만
 */
- (void)moveToHorizontalCenterLayer:(KMLayer*)layer;

/** 레이어를 화면의 가운데로 이동시킨다. 세로만
 */
- (void)moveToVerticalCenter:(long)layerId;

/** 레이어를 화면의 가운데로 이동시킨다. 세로만
 */
- (void)moveToVerticalCenterLayer:(KMLayer*)layer;

/** preview화면에서 사용자가 터치한 레이어를 반환한다.
 */
- (KMLayer*)getLayerAtTime:(int)time WithX:(float)x Y:(float)y;

/** preview화면에서 사용자가 터치한 레이어를 반환한다.
 */
- (KMLayer*)getLayerAtTime:(int)time WithPosition:(CGPoint)position;


/** 선택하고자 하는 레이어 객체를 파라미터로 넘겨주면 된다.
 nil을 넘겨주면 선택된 레이어가 없는 것으로 설정된다.
 */
- (void)changeHittedLayer:(KMLayer*)hittedLayer;

- (KMLayer*)getHittedLayer;

/** 프리뷰의 좌표를 렌더링 좌표로 변환해 준다.
 */
- (CGPoint)convertPreviewPoint:(CGPoint) previewPoint;

/** 엔진 콜백 메소드
 */
- (BOOL)renderOverlay:(int)iParam1 param2:(int)iParam2 param3:(int)iParam3
               param4:(int)iParam4 param5:(int)iParam5 param6:(int)iParam6
               param7:(int)iParam7 param8:(int)iParam8 param9:(int)iParam9
              param10:(int)iParam10 param11:(int)iParam11 param12:(int)iParam12
              param13:(int)iParam13 param14:(int)iParam14 param15:(int)iParam15
              param16:(int)iParam16 param17:(int)iParam17 param18:(int)iParam18;

/** 레이어를 다시 그린다.
 */
- (void)refresh;

/**in animation의 preview를 시작한다
 */
- (void)startInAnimation:(KMLayer*)layer;

/**overall animation의 preview를 시작한다
 */
- (void)startOverallAnimation:(KMLayer*)layer;

/**out animation의 preview를 시작한다
 */
- (void)startOutAnimation:(KMLayer*)layer;

/**animation의 preview를 종료한다
 */
- (void)stopAnimation;

+ (CGSize)renderRegionSizeWithAspectType:(NXEAspectType)aspectType;

@end
