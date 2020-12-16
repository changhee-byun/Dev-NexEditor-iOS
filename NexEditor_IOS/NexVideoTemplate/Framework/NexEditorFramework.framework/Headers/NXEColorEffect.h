/******************************************************************************
 * File Name   :	NXEColorEffect.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <Foundation/Foundation.h>

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

/**
 * \ingroup types
 * \brief PresetNXEColorEffect.
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
    NXE_CE_NOIR
} _DOC_ENUM(NXEPresetColorEffect);

/**
 * \brief This class defines the color effect information possible for a clip such as brightness, saturation, contrast, and tint color.
 *          <p>By using preset values( NONE, ALIEN_INVASION, BLACK_AND_WHITE, COOL, DEEP_BLUE, PINK, RED_ALERT, SEPIA, SUNNY, PURPLE,
 *          ORANGE, STRONG_ORANGE, SPRING, SUMMER, FALL, ROUGE, PASTEL, NOIR), color effect settings can be used more efficiently.
 * \note <b>NEClip</b> controls the clip value using <b>NENXEColorEffect</b>.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      clip.colorEffect = [[NXEColorEffect alloc] initWithPresetColorEffect:NXE_CE_SUMMER];
 *      NSLog(@"colorEffect(before changing> brightness/contrast/saturation)=(%f/%f/%f)",
 *                                      clip.colorEffect.brightness,
 *                                      clip.colorEffect.contrast,
 *                                      clip.colorEffect.saturation);
 *      clip.colorEffect.brightness = 0.15f;
 *      clip.colorEffect.contrast = 0.15f;
 *      clip.colorEffect.saturation = 0.15;
 *      NSLog(@"colorEffect(after channging> brightness/contrast/saturation)=(%f/%f/%f)",
 *                                      clip.colorEffect.brightness,
 *                                      clip.colorEffect.contrast,
 *                                      clip.colorEffect.saturation);
 *      [project addClip:clip];
 *      [engine setProject:project];
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5
 */
@interface NXEColorEffect : NSObject <NSCopying>

/**
 * \brief The lut ID.
 * \since version 1.0.5
 */
@property (nonatomic) int lutResourceId;
/**
 * \brief The tint color of a clip.
 * The tint color value, as an <tt>integer</tt>.
 * \since version 1.0.5
 */
@property (nonatomic) int tintColor;
/**
 * \brief The color effect ID set in a clip.
 * \since version 1.0.5
 */
@property (nonatomic) int presetColorEffectId;
/**
 * \brief The brightness of a clip.
 * The brightness value as a <tt>float(-1~1)</tt>.
 * \since version 1.0.5
 */
@property (nonatomic) float brightness;
/**
 * \brief The contrast of clip
 * The contrast value as a <tt>float(-1~1)</tt>.
 * \since version 1.0.5
 */
@property (nonatomic) float contrast;
/**
 * \brief The saturation of clip.
 * The saturation value as a <tt>float(-1~1)</tt>.
 * \since version 1.0.5
 */
@property (nonatomic) float saturation;

/**
 * \brief This creates a NENXEColorEffect instance that matches with presetColorEffect passed as a parameter.
 */
- (instancetype)initWithPresetColorEffect:(NXEPresetColorEffect)presetColorEffect;

@end
