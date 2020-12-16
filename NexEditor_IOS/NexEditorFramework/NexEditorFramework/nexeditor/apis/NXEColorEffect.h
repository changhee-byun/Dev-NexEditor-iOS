/******************************************************************************
 * File Name   :	NXEColorEffect.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Proprietary and Confidential
 Copyright (C) 2002~2017 NexStreaming Corp. All rights reserved.
 www.nexstreaming.com
 ******************************************************************************/

#import <Foundation/Foundation.h>

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

/**
 * \ingroup types
 * \brief A list of preset color effects.
 */
typedef NS_ENUM(NSUInteger, NXEPresetColorEffect) {
    /*! NONE */
    NXE_CE_NONE = 0,
    /*! ALIEN_INVASION */
    NXE_CE_ALIEN_INVASION,
    /*! BLACK_AND_WHITE */
    NXE_CE_BLACK_AND_WHITE,
    /*! COOL */
    NXE_CE_COOL,
    /*! DEEP_BLUE */
    NXE_CE_DEEP_BLUE,
    /*! PINK */
    NXE_CE_PINK,
    /*! RED_ALERT */
    NXE_CE_RED_ALERT,
    /*! SEPIA */
    NXE_CE_SEPIA,
    /*! SUNNY */
    NXE_CE_SUNNY,
    /*! PURPLE */
    NXE_CE_PURPLE,
    /*! ORANGE */
    NXE_CE_ORANGE,
    /*! STRONG_ORANGE */
    NXE_CE_STRONG_ORANGE,
    /*! SPRING */
    NXE_CE_SPRING,
    /*! SUMMER */
    NXE_CE_SUMMER,
    /*! FALL */
    NXE_CE_FALL,
    /*! ROUGE */
    NXE_CE_ROUGE,
    /*! PASTEL */
    NXE_CE_PASTEL,
    /*! NOIR */
    NXE_CE_NOIR,
    
    NXE_CE_MAX
} _DOC_ENUM(NXEPresetColorEffect);

/** \class NXEColorEffect
 *  \brief NXEColorEffect class defines all properties and methods to manage color effects of a clip.
 *  \since version 1.0.5
 */
@interface NXEColorEffect : NSObject <NSCopying>

/** \brief The tint color of a clip. 
 *  \since version 1.0.5
 */
@property (nonatomic) int tintColor;

/** \brief The preset color effect ID of a clip. 
 *  \since version 1.0.5
 */
@property (nonatomic) int presetColorEffectId;

/** \brief The brightness value of a clip, ranging from -1.0 through 1.0, 0.0 by default. 
 *  \since version 1.0.5
 */
@property (nonatomic) float brightness;

/** \brief The contrast value of a clip, ranging from -1.0 through 1.0, 0.0 by default. 
 *  \since version 1.0.5
 */
@property (nonatomic) float contrast;

/** \brief The saturationvalue of a clip, ranging from -1.0 through 1.0, 0.0 by default. 
 *  \since version 1.0.5
 */
@property (nonatomic) float saturation;

/** \brief Sets color effects to a clip with the speficied preset color effect.
 *  \param presetColorEffect A preset color effect type.
 *  \since version 1.0.19
 */
- (void)setValues:(NXEPresetColorEffect)presetColorEffect;

/** \brief Initializes a new color effect object with the speficied preset color effect.
 *  \param presetColorEffect A preset color effect type.
 *  \return An NXEColorEffect object
 *  \since version 1.0.5
 */
- (instancetype)initWithPresetColorEffect:(NXEPresetColorEffect)presetColorEffect;

@end
