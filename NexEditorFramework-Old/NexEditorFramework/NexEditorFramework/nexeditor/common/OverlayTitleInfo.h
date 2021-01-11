/**
 * File Name   : OverlayTitleInfo.h
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

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#define VALUE_GROUP_START @"start"
#define VALUE_GROUP_END @"end"

@class NXEError;

typedef NS_ENUM (NSUInteger, InterpolatorType) {
    InterpolatorTypeAccelerateDecelerate = 0,   // default
    InterpolatorTypeAccelerate,
    InterpolatorTypeDecelerate,
    InterpolatorTypeLinear,
    InterpolatorTypeBounce,
    InterpolatorTypeCycle,
    InterpolatorTypeAnticipate,
    InterpolatorTypeAnticipateOvershoot,
    InterpolatorTypeOvershoot
};

typedef NS_ENUM (NSUInteger, AnimationType) {
    AnimationTypeMove = 0,
    AnimationTypeAlpha,
    AnimationTypeScale,
    AnimationTypeRotate
};

typedef NS_ENUM (NSUInteger, TextType) {
    TextTypeTitle = 0,
    TextTypeSubTitle
};

typedef NS_ENUM (NSUInteger, TextGroup) {
    TextGroupStart = 0,
    TextGroupEnd
};

typedef NS_ENUM (NSUInteger, HorizontalAlign) {
    HorizontalAlignLeft = 0,
    HorizontalAlignRight,
    HorizontalAlignCenter
};

typedef NS_ENUM (NSUInteger, VerticalAlign) {
    VerticalAlignTop = 0,
    VerticalAlignMiddle,
    VerticalAlignBottom
};

typedef NS_ENUM (NSUInteger, OverlayTitleType) {
    OverlayTitleTypeText = 0,
    OverlayTitleTypeImage
};

typedef struct TitleScale {
    CGFloat x;
    CGFloat y;
    CGFloat z;
}TitleScale;

typedef struct PositionOffset {
    NSInteger x;
    NSInteger y;
}PositionOffset;

typedef struct Position {
    NSInteger left;
    NSInteger top;
    NSInteger right;
    NSInteger bottom;
}Position;

typedef struct Alignment {
    HorizontalAlign hAlign;
    VerticalAlign vAlign;
}Alignment;

typedef struct Group {
    TextGroup textGroup;
    TextType textType;
}Group;

#pragma mark -

/*! \brief OverlayTitleLayerAnimationMotion
 *  \version 1.x
 */
@interface OverlayTitleLayerAnimationMotion : NSObject

/*! \brief motion type, 추후 사용할 예정, fixed value < AccelerateDecelerateInterpolator[default], AccelerateInterpolator, DecelerateInterpolator, LinearInterpolator, BounceInterpolator, CycleInterpolator, AnticipateInterpolator, AnticipateOvershootInterpolator, OvershootInterpolator >
 *  \note 관련 키(:motion_type)
 *  \version 1.x
 */
@property (nonatomic, readonly) InterpolatorType interpolatorType;

/*! \brief animation start time
 *  \note 관련 키(:start_time)
 *  \version 1.x
 */
@property (nonatomic, readonly) int startTime;

/*! \brief animation duration
 *  \note 관련 키(:end_time)
 *  \version 1.x
 */
@property (nonatomic, readonly) int duration;

/*! \brief animation start position
 *  \note 관련 키(:start_x, start_y)
 *  \version 1.x
 */
@property (nonatomic, readonly) CGPoint startPosition;

/*! \brief animation end position
 *  \note 관련 키(:end_x, end_y)
 *  \version 1.x
 */
@property (nonatomic, readonly) CGPoint endPosition;

/*! \brief animation rotation
 *  \note 관련 키(:rotatedegree)
 *  \version 1.x
 */
@property (nonatomic, readonly) int rotation;

/*! \brief animation start 동작 시 투명도를 반영하지 않는 것에 대한 가부, fixed value < "1": Not Transparency, "0": Transparency >
 *  \note 관련 키(:start)
 *  \version 1.x
 */
@property (nonatomic, readonly) int startAlpha;

/*! \brief animation end 동작 시 투명도를 반영하지 않는 것에 대한 가부, fixed value < "1": Not Transparency, "0": Transparency >
 *  \note 관련 키(:end)
 *  \version 1.x
 */
@property (nonatomic, readonly) int endAlpha;

/*! \brief animation rotate시 방향 정보, fixed value < "1": clockwise, "0": counterclockwise >
 *  \note 관련 키(:clockwise)
 *  \version 1.x
 */
@property (nonatomic, readonly) BOOL clockWise;

- (instancetype)initWithRawTitleLayerAnimationDictionary:(NSDictionary *)rawTitleLayerAnimation;

@end

/*! \brief OverlayTitleAnimation
 *  \version 1.x
 */
@interface OverlayTitleAnimation : NSObject

/*! \brief Animation Type, fixed value < move, alpha, scale, rotate >
 *  \note 관련 키(:type)
 *  \version 1.x
 */
@property (nonatomic, readonly) AnimationType type;

/*! \brief OverlayTitleLayerAnimationMotion Array
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSArray<OverlayTitleLayerAnimationMotion *> *motions;

- (instancetype)initWithAnimationType:(AnimationType)animationType rawDictionary:(NSDictionary<NSString *, NSString *>*)rawDictionary;

@end

/*! \brief OverlayTitleLayer
 *  \version 1.x
 */
@interface OverlayTitleLayer : NSObject

/*! \brief text overlay 혹은 image overlay를 구분해 준다.
 *  \note 관련 키(:type)
 *  \version 1.x
 */
@property (nonatomic, readonly) OverlayTitleType type;

/*! \brief main title 혹은 sub title를 구분해 준다.
 *  \note 관련 키(:text)
 *  \version 1.x
 */
@property (nonatomic, readonly) TextType textType;

/*! \brief main title 그리고 sub title을 같은 그룹으로 설정하기 위한 정보를 가지고 있다. '_'를 기준으로 앞의 값은 그룹을 의미하며 뒤의 값 중 1은 main title 그리고 2는 sub title을 의미한다.
 *  \note 관련 키(:group)
 *  \version 1.x
 */
@property (nonatomic, readonly) Group group;

/*! \brief default displaying value
 *  \note 관련 키(:text_desc)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *defaultText;

/*! \brief text max length
 *  \note 관련 키(:text_max_len)
 *  \version 1.x
 */
@property (nonatomic, readonly) int textMaxLength;

/*! \brief text overlay 혹은 image overlay start time
 *  \note 관련 키(:start_time)
 *  \version 1.x
 */
@property (nonatomic, readonly) int startTime;

/*! \brief text overlay 혹은 image overlay duration
 *  \note 관련 키(:duration)
 *  \version 1.x
 */
@property (nonatomic, readonly) int duration;

/*! \brief 사용 할 font naming
 *  \note 관련 키(:font)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *fontName;

/*! \brief 사용 할 font size, 기록된 단위는 pt이기에 ios에 맞는 값으로 변경이 필요한가?
 *  \note 관련 키(:font_size)
 *  \version 1.x
 */
@property (nonatomic, readonly) int fontSize;

/*! \brief 사용 할 font color, 단위는 #ARGB형태 (ex. #FF000000)
 *  \note 관련 키(:font_color)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) UIColor *fontColor;

/*! \brief font에 대한 shadow 가부
 *  \note 관련 키(:shadow_visible)
 *  \version 1.x
 */
@property (nonatomic, readonly) BOOL fontShadow;

/*! \brief font에 대한 shadow color, 단위는 #ARGB형태 (ex. #FF000000)
 *  \note 관련 키(:shadow_color)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) UIColor *fontShadowColor;

/*! \brief font에 대한 glow 가부
 *  \note 관련 키(:glow_visible)
 *  \version 1.x
 */
@property (nonatomic, readonly) BOOL fontGlow;

/*! \brief font에 대한 glow color, 단위는 #ARGB형태 (ex. #FF000000)
 *  \note 관련 키(:glow_color)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) UIColor *fontGlowColor;

/*! \brief font에 대한 outline 가부
 *  \note 관련 키(:outline_visible)
 *  \version 1.x
 */
@property (nonatomic, readonly) BOOL fontOutline;

/*! \brief font에 대한 outline color, 단위는 #ARGB형태 (ex. #FF000000)
 *  \note 관련 키(:outline_color)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) UIColor *fontOutlineColor;

/*! \brief text align, fixed value < Center(default), LEFT, RIGHT, TOP(default), MIDDLE, BOTTOM >
 *  \note 관련 키(:align)
 *  \version 1.x
 */
@property (nonatomic, readonly) Alignment align;

/*! \brief sub title 없을 경우, main title에 반영해야 할 align, fixed value < Center(default), LEFT, RIGHT, TOP(default), MIDDLE, BOTTOM >
 *  \note 관련 키(:adjust_align_non_sub)
 *  \version 1.x
 */
@property (nonatomic, readonly) Alignment adjAlign;

/*! \brief image overlay 사용 될 이미지 리소스
 *  \note 관련 키(:image_res)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *resourceID;

/*! \brief text overlay 혹은 image overlay 출력 좌표인데, 기록된 단위는 pixel 이기에 point로 변경이 필요한가?
 *  \note 관련 키(:position)
 *  \version 1.x
 */
@property (nonatomic, readonly) Position position;

/*! \brief sub title 없을 경우, main title에 저장되었던 bounds의 x,y값에 더해지는 offset값
 *  \note 관련 키(:adjust_pos_non_sub)
 *  \version 1.x
 */
@property (nonatomic, readonly) PositionOffset positionOffset;

/*! \brief text overlay 혹은 image overlay 사용 될 animation
 *  \note 관련 키(:animation)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSArray<OverlayTitleAnimation *> *animations;

/*! \brief sub title 없을 경우, main title에 있는 text overlay 혹은 image overlay 사용 될 animation
 *  \note 관련 키(:adjust_animation_non_sub)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSArray<OverlayTitleAnimation *> *adjAnimations;

/*! \brief text overlay 혹은 image overlay 사용 될 rotation
 *  \note 관련 키(:rotate)
 *  \version 1.x
 */
@property (nonatomic, readonly) float rotation;

/*! \brief text overlay 혹은 image overlay 사용 될 scale, x, y and z scale values
 *  \note 관련 키(:scale)
 *  \version 1.x
 */
@property (nonatomic, readonly) TitleScale scale;

/*! \brief inAnimation시 main title, sub title 정보가 있는지를 알려주는 property.
 *  \note main title, sub title 정보 모두 존재해야 YES, 그렇지 않는 경우 NO 값을 갖는다.
 */
@property (nonatomic, readonly) BOOL inAniSubTitle;

/*! \brief outAnimation시 main title, sub title 정보가 있는지를 알려주는 property.
 *  \note main title, sub title 정보 모두 존재해야 YES, 그렇지 않는 경우 NO 값을 갖는다.
 */
@property (nonatomic, readonly) BOOL outAniSubTitle;

- (instancetype)initWithRawDictionary:(NSDictionary *)rawTitleLayer;

@end

/*! \brief OverlayTitleInfo
 *  \version 1.x
 */
@interface OverlayTitleInfo : NSObject

/*! \brief OverlayTitleInfo naming
 *  \note 관련 키(:overlay_name)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *name;

/*! \brief OverlayTitleInfo version
 *  \note 관련 키(:overlay_version)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *version;

/*! \brief OverlayTitleInfo description
 *  \note 관련 키(:overlay_desc)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *desc;

/*! \brief project duration이 titleinfo 기록된 duration보다 짧을 경우, 출력해야 할 title type를 가르킨다.
 *  \note "start" : start title, "end" : end title
 *  \note 관련 키(overlay_priority)
 *  \version 1.x
 */
@property (nonatomic, readonly, retain) NSString *priorityTitleType;

@property (nonatomic) int overlayTitleDuration;

/*! \brief OverlayTitleLayer Array
 *  \version 1.x
 */
@property (nonatomic, retain) NSArray<OverlayTitleLayer *> *overlays;

/*! \brief json형식의 fild data를 가지고 OverlayTitleInfo instance를 만든다
 *  \note jsonData json형식의 file data
 *  \note nxeError 구성 시 에러가 발생할 경우, NXEError instance를 통해 내용을 공유한다.
 *  \version 1.x
 */
- (instancetype)initWithJSONData:(NSData *)jsonData nxeError:(NXEError **)nxeError;

@end
