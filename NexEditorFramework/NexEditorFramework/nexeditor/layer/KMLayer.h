/******************************************************************************
 * File Name   : KMLayer.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "KMLut.h"
#import "KMAnimation.h"
#import "LayerGLContext.h"
#import "NXETextureDisposeBag.h"

#import <NexEditorEngine/NexLayer.h>

@class KMAnimationEffect;

typedef NS_ENUM ( NSUInteger, LayerButtonPosition ) {
    LAYER_BTN_POSITION_NONE = 0,
    LAYER_BTN_POSITION_UPPER_LEFT,
    LAYER_BTN_POSITION_UPPER_RIGHT,
    LAYER_BTN_POSITION_BOTTOM_LEFT,
    LAYER_BTN_POSITION_BOTTOM_RIGHT
};

typedef enum {
    KM_LAYER_HIT_NONE = 0,
    KM_LAYER_HIT_BODY,
    KM_LAYER_HIT_UPPER_LEFT,
    KM_LAYER_HIT_UPPER_RIGHT,
    KM_LAYER_HIT_BOTTOM_LEFT,
    KM_LAYER_HIT_BOTTOM_RIGHT
}KMLayerHitType;

typedef enum {
    KM_SPLIT_NONE = 0,
    KM_SPLIT_ALL,
    KM_SPLIT_BOTTOM,
    KM_SPLIT_TOP,
    KM_SPLIT_LEFT,
    KM_SPLIT_RIGHT,
    KM_SPLIT_MAX
}KMSplitScreenType;

typedef enum {
    TARGET_NORMAL = 0,
    TARGET_MASK
}RendererTarget;

/** 크로마키 설정 값을 가지고 있는 클래스
 */
@interface ChromakeyProperty : NSObject

@property(nonatomic, assign) BOOL useChromaKey;     //크로마키 사용여부
@property(nonatomic, assign) BOOL useChromaKeyMask; //크로마키 값 조절 시 보조역할. 검은색은 제거, 흰색은 표시
@property(nonatomic, assign) int chromakeyColor;    //크로마키 효과 적용할 배경색. 삭제될 색(BGR 형태. blue - 0xff0000, red - 0x0000ff, green - 0x00ff00)

@property(nonatomic, assign) float foreground;      //전경. 기본 값 0.28 (0~1, 배경 값과 합쳐서 1이 넘으면 안됨.)
@property(nonatomic, assign) float background;      //배경. 기본 값 0.5 (0~1, 전경 값과 합쳐서 1이 넘으면 안됨.)

@property(nonatomic, assign) CGPoint blendPoint1;   //크로마키 효과 세부 설정. point1 좌표. (0~1, 포토샵 커브 같은 기능)
@property(nonatomic, assign) CGPoint blendPoint2;   //크로마키 효과 세부 설정. point2 좌표. (0~1, 포토샵 커브 같은 기능)

@end

/** 크로마키 이용할 레이어에서 상속받기만 하면 된다.
 구현은 KMLayer 안에 되어 있다.
 */
@protocol ChromaKeyLayer <NSObject>

@optional

/** 크로마키 효과 사용 여부를 설정한다.
 */
- (void)setChromakeyEnable:(BOOL)isEnabled;

/** 크로마키 효과 마스크 사용 여부를 설정한다.
 */
- (void)setChromakeyMaskEnable:(BOOL)isMaskEnabled;

- (void)setChromakeyColor:(int)argb;

/** 크로마키 효과적용할 배경색을 설정한다. 즉, 삭제되어야 할 색. (0~255)
 @param red - 삭제할 배경 색 중 red 값. (0~255)
 @param green - 삭제할 배경 색 중 green 값. (0~255)
 @param blue - 삭제할 배경 색 중 blue 값. (0~255)
 */
- (void)setChromakeyColorRed:(int)red Green:(int)green Blue:(int)blue;

/** 전경 값을 설정한다.
 */
- (void)setForeground:(float)foreground;

/** 배경 값을 설정한다.
 */
- (void)setBackground:(float)background;

/** blend point 값을 설정한다.
 */
- (void)setBlendPoint1:(CGPoint)p1 Point2:(CGPoint)p2;

@end

/** SplitScreen 기능을 이용할 레이어에서 상속받기만 하면 된다.
 구현은 KMLayer 안에 되어 있다.
 */
@protocol SplitScreenLayer <NSObject>

@required

- (void) setSplitType:(KMSplitScreenType) type;
- (KMSplitScreenType) getSplitType;
- (void) setSplitSize:(int) size;
- (int) getSplitSize;

- (double)getSplitMoveX;
- (double)getSplitMoveY;
- (double)getSplitScale;
- (int)getSplitAngle;

- (void)setSplitMoveX:(double)value;
- (void)setSplitMoveY:(double)value;
- (void)setSplitScale:(double)value;
- (void)setSplitAngle:(int)value;

- (void)moveSplitPosition:(CGPoint)position;
- (void)changeSplitScale:(double)scale;
- (void)changeSplitSize:(CGPoint)move;

@end

@interface KeyFrameInfo : NSObject

@property(nonatomic)float time;
@property(nonatomic)int x;
@property(nonatomic)int y;
@property(nonatomic)float scale;
@property(nonatomic)int angle;

@end

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int startTime;
    int endTime;
} KMLayerGeometry;

@class KMLayer;

@protocol KMLayerDelegate
- (KMLayerGeometry) layer:(KMLayer *) layer shouldUpdateGeometry:(BOOL *) should;
@end

/** 레이어에 대한 공통 속성을 가지고 있는 super class
 모든 레이어는 KMLayer 를 상속받아 구현한다.
 */
@interface KMLayer : NSObject {
    volatile int textureId; // Preview mode, textureId
    volatile int textureId2;// Export mode, textureId
    BOOL isVideo;
    BOOL isAsset;
}

@property(nonatomic, assign) long layerId;  //레이어 고유 번호. LayerManager에서 관리한다.
@property(nonatomic, assign) BOOL isHit;    //레이어가 사용자에 의해 선택되었는지 판단하는 플래그.
@property(readonly, getter=getHitType) KMLayerHitType hitType; //레이어의 선택 모드.

@property(nonatomic, assign) float x;         //레이어 위치 좌표.(left)
@property(nonatomic, assign) float y;         //레이어 위치 좌표.(top)

@property(nonatomic, assign) int width;     //레이어의 가로 길이
@property(nonatomic, assign) int height;    //레이어의 세로 길이

@property(nonatomic, assign) float alpha;   //레이어의 전체 투명도.(0~1)
@property(nonatomic, assign) float scale;   //레이어의 크기 배율 값.(0.5 ~ 3)
@property(nonatomic, assign) float xScale;  //레이어의 크기 배율 값.(0.5 ~ 3)
@property(nonatomic, assign) float yScale;  //레이어의 크기 배율 값.(0.5 ~ 3)
@property(nonatomic, assign) float angle;     //레이어의 회전 값.(0~359, degree)
@property(nonatomic, assign) BOOL hFlip;    //레이어의 가로 플립.
@property(nonatomic, assign) BOOL vFlip;    //레이어의 세로 플립.

@property(nonatomic, assign) float brightness;  //밝기 (-1 ~ 1). 기본 0
@property(nonatomic, assign) float contrast;    //대비 (-1 ~ 1). 기본 0
@property(nonatomic, assign) float saturation;  //채도 (-1 ~ 1). 기본 0

@property(nonatomic, assign) int startTime; //레이어가 나타나는 시작 시간. millisecond
@property(nonatomic, assign) int endTime;   //레이어가 사라지는 종료 시간. millisecond

@property(nonatomic, assign) BOOL isSelectable; // move,scaling,rotation 동작이 실행되지 않기위해 만든 변수
@property (nonatomic, assign) id<KMLayerDelegate> delegate;

@property (nonatomic, assign) id<NXETextureDisposeBagProtocol> textureDisposeBag;

/** 레이어의 위치 좌표를 설정한다.
 @param x - 레이어 위치 좌표.(left)
 @param y - 레이어 위치 좌표.(top)
 */
- (void)setX:(int)x Y:(int)y;

/** 레이어의 위치 좌표를 설정한다.
 @param position - 레이어의 위치 좌표
 */
- (void)setPosition:(CGPoint)position;

/** 레이어의 중심 좌표를 반환한다.
 @return 레이어의 중심 좌표
 */
- (CGPoint)getCenter;

- (void)applyAnimationForPreRendering:(int)currentPosition;

/** colorMatrix 를 반환한다.
 @return 4x5 color matrix
 */
- (float*)getColorMatrix;

/** LUT 를 설정한다.
 */
- (void)setLut:(KMLutType)type;

/**
 *	\brief 현재 설정한 LUT Instance를 리턴해준다.
 *	\return LUT Instance, KMLut type.
 */
- (KMLut *)getLut;

/** In 애니메이션을 설정한다.
 * @param type - KMAnimationType 중 하나
 * @param duration - 애니메이션 시간. millisecond
 */
- (void)setInAnimationType:(KMAnimationType)type WithDuration:(int)duration;

/** overall 애니메이션을 설정한다.
 * @param type - KMAnimationType 중 하나
 */
- (void)setOverallAnimationType:(KMAnimationType)type;

/** Out 애니메이션을 설정한다.
 * @param type - KMAnimationType 중 하나
 * @param duration - 애니메이션 시간. millisecond
 */
- (void)setOutAnimationType:(KMAnimationType)type WithDuration:(int)duration;

/** 프로젝트의 현재 시간에 레이어가 나와야 하는지 체크한다.
 @param currentPosition - 프로젝트의 현재시간. millisecond
 @return 레이어가 나와야 하면 YES, 나오지 않으면 NO
 */
- (BOOL)isActiveAtTime:(int)currentPosition;

/** 레이어를 렌더러를 이용하여 preview/exproting에 맞게 드로잉한다.
 레이어 클래스는 이 메소드를 반드시 구현해야 한다.
 @param currentPosition - 프로젝트의 현재시간. 애니메이션등을 구현할 경우 필요하다.
 @param renderer - 레이어를 드로잉할 렌더러 객체
 */
- (void)renderAtTime:(int)currentPosition withRenderer:(NexLayer*)renderer;

/** 레이어의 텍스쳐 id 값을 가져온다.
 @return OpenGL texture의 id 값
 */
- (GLuint)getTextureId;

/**
 * Dispose main textures for later deletion
 */
- (void)disposeTextures;

/** 사용자의 터치 좌표와 프로젝트의 시간으로 레이어가 선택되었는지 판단한다.
 */
- (BOOL)isHitAtTime:(int) time WithX:(float)x Y:(float)y;

/**
 */
- (void)addKMAnimationEffect:(KMAnimationEffect *)animationEffect;

/**
 */
- (void) setNeedsGeometryUpdate;

/** LayerEditorButtonTexture 생성에 필요한 정보를 저장한다.
 */
- (void) setLayerEditorButtonWithPosition:(LayerButtonPosition)position image:(UIImage *)image imageSize:(CGSize)imageSize;

/** LayerEditorBorderTexture 생성에 필요한 정보를 저장한다.
 */
- (void) setLayerEditorBorderWithColor:(UIColor *)color thickness:(CGFloat)thickness;


@end
