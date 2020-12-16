/******************************************************************************
 * File Name   : KMLut.h
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
#import "LayerGLContext.h"
#import "NXETextureDisposeBag.h"

//LUT 타입
typedef NS_ENUM(NSUInteger, KMLutType) {
    KM_LUT_NONE = 0,
    KM_LUT_AFTERNOON = 1,
    KM_LUT_ALMOND_BLOSSOM,
    KM_LUT_AUTUMN,
    KM_LUT_BORING,
    KM_LUT_CARAMEL_CANDY,
    KM_LUT_CARIBBEAN,
    KM_LUT_CINNAMON,
    KM_LUT_CLOUD,
    KM_LUT_CORAL_CANDY,
    KM_LUT_CRANBERRY,
    KM_LUT_DAISY,
    KM_LUT_DAWN,
    KM_LUT_DISNEY,
    KM_LUT_ENGLAND,
    KM_LUT_ESPRESSO,
    KM_LUT_EYESHADOW,
    KM_LUT_GLOOMY,
    KM_LUT_JAZZ,
    KM_LUT_LAVENDAR,
    KM_LUT_MOONLIGHT,
    KM_LUT_NEWSPAPER,
    KM_LUT_PARIS,
    KM_LUT_PEACH,
    KM_LUT_RAINY,
    KM_LUT_RASPBERRY,
    KM_LUT_RETRO,
    KM_LUT_SHERBERT,
    KM_LUT_SHINY,
    KM_LUT_SMOKE,
    KM_LUT_STONEEDGE,
    KM_LUT_SUNRISING,
    KM_LUT_SYMPHONY_BLUE,
    KM_LUT_TANGERINE,
    KM_LUT_TIFFANY,
    KM_LUT_VINTAGE_FLOWER,
    KM_LUT_ROMANCE,
    KM_LUT_VIVID,
    KM_LUT_WARM,
    // must refactory..!!!
    //
    KM_LUT_CARAMEL,
    KM_LUT_JN10,
    KM_LUT_LEMONADE,
    KM_LUT_LOVE_01,
    KM_LUT_LOVE_02,
    KM_LUT_LOVE_03,
    KM_LUT_LOVE_04,
    KM_LUT_LOVE_PINK_JN01,
    KM_LUT_LOVE_PINK_RED01,
    KM_LUT_LOVE_Y01,
    KM_LUT_LOVE_Y02,
    KM_LUT_LOVE_Y03,
    KM_LUT_ORANGE,
    KM_LUT_ORANGE1,
    KM_LUT_PINK,
    KM_LUT_SPORTS,
    KM_LUT_TRANSSION_SPORTS,
    
    KM_LUT_MAX
};

/** 레이어의 LUT효과
 */
@interface KMLut : NSObject
@property (nonatomic, assign) id<NXETextureDisposeBagProtocol> textureDisposeBag;

- (id)initWithType:(KMLutType)type textureDisposeBag:(id<NXETextureDisposeBagProtocol>) textureDisposeBag;

- (KMLutType)getType;

/**
 * \brief Returns texture for the lut type. The texture is created if none yet has been created.
 * \return LAYER_GL_INVALID_TEXTURE_ID if the lut type is KM_LUT_NONE. A valid texture otherwise.
 */
- (GLuint)getTextureId:(LayerGLContextIndex) contextIndex;

/**
 */
- (void)disposeTextures;

@end
