/******************************************************************************
 * File Name   : KMEffectLayer.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "KMLayer.h"

//이펙트 레이어 종류
typedef enum {
    KM_EFFECT_BLUR = 0, //블러
    KM_EFFECT_MOSAIC,   //모자이크
    KM_EFFECT_MAX
} KMEffectType;


/** 블러와 모자이크 효과를 위한 레이어.
 나중에 다양한 효과로 확장 가능.
 TODO : 기능 확인 및 구현
 */
@interface KMEffectLayer : KMLayer

- (id)initWithType:(KMEffectType) type;

@end
