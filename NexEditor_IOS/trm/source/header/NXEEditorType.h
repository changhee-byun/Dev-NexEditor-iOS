/******************************************************************************
 * File Name   :	NXEEditorType.h
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

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

/** \defgroup types Types
 *
 * Type values for using Clips and Effects.
 */

/**
 * \ingroup types
 * \brief A list of clip types.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXEClipType)
{
    /*! The status of the clip has not yet been clarified. */
    NXE_CLIPTYPE_NONE = 0x00000000,
    /*! Image files of photos or pictures. */
    NXE_CLIPTYPE_IMAGE = 0x00000001,
    /*! feature not used on the UI level */
    NXE_CLIPTYPE_TEXT = 0x00000002,
    /*! Audio files with file formats such as MP3 and AAC. */
    NXE_CLIPTYPE_AUDIO = 0x00000003,
    /*! Video files with file formats such as MP4. */
    NXE_CLIPTYPE_VIDEO = 0x00000004,
    /*! feature not used on the UI level */
    NXE_CLIPTYPE_EFFECT = 0x00000005,
    /*! feature not used on the UI level */
    NXE_CLIPTYPE_LAYER = 0x000000006,
    /*! feature not used on the UI level */
    NXE_CLIPTYPE_VIDEO_LAYER = 0x00000007,
    
} _DOC_ENUM(NXEClipType);

/**
 * \ingroup types
 * \brief A list of clip effect title types.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXETitleType)
{
    /*! feature not used on the UI level */
    NXE_TITLE_STYLE_NONE = 0x00000000,
    /*! feature not used on the UI level */
    NXE_TITLE_STYLE_HEADLINE = 0x00000001,
    /*! feature not used on the UI level */
    NXE_TITLE_STYLE_OPENNING = 0x00000002,
    /*! feature not used on the UI level */
    NXE_TITLE_STYLE_PLAYING = 0x00000003,
    /*! feature not used on the UI level */
    NXE_TITLE_STYLE_ENDING = 0x00000004,
    
} _DOC_ENUM(NXETitleType);

/**
 * \ingroup types
 * \brief A list of aspect ratio types.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXEAspectType)
{
    /*! Set to this mode for a 16x9 screen ratio. */
    NXEAspectTypeRatio16v9 = 0,
    /*! Set to this mode for a 9x16 screen ratio. */
    NXEAspectTypeRatio9v16,
    /*! Set to this mode for a 1x1 screen ratio. */
    NXEAspectTypeRatio1v1,
    /*! Set to this mode for a 1x2 screen ratio. */
    NXEAspectTypeRatio1v2,
    /*! Set to this mode for a 2x1 screen ratio. */
    NXEAspectTypeRatio2v1,
    /*! Set to this mode for a arbitrary screen ratio. See -setAspectType:withRatio: */
    NXEAspectTypeCustom,
    NXEAspectTypeUnknown = NXEAspectTypeCustom
    
} _DOC_ENUM(NXEAspectType);

/// Backward compatibility
#define NXE_ASPECT_1v1  NXEAspectTypeRatio1v1
#define NXE_ASPECT_16v9 NXEAspectTypeRatio16v9
#define NXE_ASPECT_9v16 NXEAspectTypeRatio9v16

/**
 * \ingroup types
 * \brief A list of effect types.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXEEffectType)
{
    /*! Set to clip effect. */
    NXE_CLIP_EFFECT = 0x00000000,
    /*! Set to transition effect. */
    NXE_CLIP_TRANSITION = 0x00000001,
    
} _DOC_ENUM(NXEEffectType);

/**
 * \ingroup types
 * \brief A list of NXEEngine state.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXEPLAYSTATE)
{
    /*! Not ready */
    NXE_PLAYSTATE_NONE = 0,
    /*! Idle state*/
    NXE_PLAYSTATE_IDLE = 1,
    /*! Preview in progress */
    NXE_PLAYSTATE_RUN = 2,
    /*! Export in progress */
    NXE_PLAYSTATE_RECORD = 3,
    /*! Preivew in pause state */
    NXE_PLAYSTATE_PAUSE = 4,
    /*! Preivew in resume state */
    NXE_PLAYSTATE_RESUME = 5
    
} _DOC_ENUM(NXEPLAYSTATE);

/**
 * \ingroup types
 * \brief Represents a LUT.
 * \since version 1.3.0
 */
typedef NSUInteger NXELutID;

/**
 * \ingroup types
 * \brief A list of LUT types.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXELutTypes)
{
    /*! none */
    NXE_LUT_NONE = 0,
    /*! afternoon */
    NXE_LUT_AFTERNOON,
    /*! almond blossom */
    NXE_LUT_ALMOND_BLOSSOM,
    /*! autumn */
    NXE_LUT_AUTUMN,
    /*! boring */
    NXE_LUT_BORING,
    /*! caramel candy */
    NXE_LUT_CARAMEL_CANDY,
    /*! caribbean */
    NXE_LUT_CARIBBEAN,
    /*! cinnamon */
    NXE_LUT_CINNAMON,
    /*! cloud */
    NXE_LUT_CLOUD,
    /*! coral candy */
    NXE_LUT_CORAL_CANDY,
    /*! cranberrry */
    NXE_LUT_CRANBERRY,
    /*! daisy */
    NXE_LUT_DAISY,
    /*! dawn */
    NXE_LUT_DAWN,
    /*! disney */
    NXE_LUT_DISNEY,
    /*! england */
    NXE_LUT_ENGLAND,
    /*! espresso */
    NXE_LUT_ESPRESSO,
    /*! eyeshadow */
    NXE_LUT_EYESHADOW,
    /*! gloomy */
    NXE_LUT_GLOOMY,
    /*! jazz */
    NXE_LUT_JAZZ,
    /*! lavendar */
    NXE_LUT_LAVENDAR,
    /*! moonlight */
    NXE_LUT_MOONLIGHT,
    /*! newpaper */
    NXE_LUT_NEWSPAPER,
    /*! paris */
    NXE_LUT_PARIS,
    /*! peach */
    NXE_LUT_PEACH,
    /*! rainy */
    NXE_LUT_RAINY,
    /*! raspberry */
    NXE_LUT_RASPBERRY,
    /*! retro */
    NXE_LUT_RETRO,
    /*! sherbert */
    NXE_LUT_SHERBERT,
    /*! shiny */
    NXE_LUT_SHINY,
    /*! smoke */
    NXE_LUT_SMOKE,
    /*! stoneedge */
    NXE_LUT_STONEEDGE,
    /*! sunrising */
    NXE_LUT_SUNRISING,
    /*! symphony blue */
    NXE_LUT_SYMPHONY_BLUE,
    /*! tangerine */
    NXE_LUT_TANGERINE,
    /*! tiffany */
    NXE_LUT_TIFFANY,
    /*! vintage flower */
    NXE_LUT_VINTAGE_FLOWER,
    /*! romance */
    NXE_LUT_ROMANCE,
    /*! vivid */
    NXE_LUT_VIVID,
    /*! warm */
    NXE_LUT_WARM,
    /*! blue */
    NXE_LUT_BLUE,
    /*! blue only */
    NXE_LUT_BLUE_ONLY,
    /*! dbright */
    NXE_LUT_DBRIGHT,
    /*! heat */
    NXE_LUT_HEAT,
    /*! ludwing */
    NXE_LUT_LUDWING,
    /*! negative */
    NXE_LUT_NEGATIVE,
    /*! oldfilm */
    NXE_LUT_OLDFILM,
    /*! rosy */
    NXE_LUT_ROSY,
    /*! salmon teal */
    NXE_LUT_SALMON_TEAL,
    /*! sunprint */
    NXE_LUT_SUNPRINT,
    /*! sunset */
    NXE_LUT_SUNSET,
    /*! sweet */
    NXE_LUT_SWEET,

    NXE_LUT_MAX,
    
} _DOC_ENUM(NXELutTypes);

/**
 * \ingroup types
 * \brief A list of voice changer types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXEVoiceChanger)
{
    /*! none */
    NXE_VOICE_FACTOR_NONE = 0,
    /*! chipmunk */
    NXE_VOICE_FACTOR_CHIPMUNK,
    /*! robot */
    NXE_VOICE_FACTOR_ROBOT,
    /*! deep */
    NXE_VOICE_FACTOR_DEEP,
    /*! modulation */
    NXE_VOICE_FACTOR_MODULATION
    
} _DOC_ENUM(NXEVoiceChanger);

/**
 * \ingroup types
 * \brief A list of music effect types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXEMusicEffect)
{
    /*! none */
    NXE_MUSIC_EFFECT_NONE = 0,
    /*! live concert */
    NXE_MUSIC_EFFECT_LIVE_CONCERT,
    /*! stereo chorus */
    NXE_MUSIC_EFFECT_STEREO_CHORUS,
    /*! enhancer */
    NXE_MUSIC_EFFECT_ENHANCER
    
} _DOC_ENUM(NXEMusicEffect);

/**
 * \ingroup types
 * \brief A list of layer animation types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXEInAnimationType)
{
    /*! none */
    NXE_ANIMATION_IN_NONE = 0,
    /*! fade */
    NXE_ANIMATION_IN_FADE,
    /*! pop */
    NXE_ANIMATION_IN_POP,
    /*! slide right */
    NXE_ANIMATION_SLIDE_IN_RIGHT,
    /*! slide left */
    NXE_ANIMATION_SLIDE_IN_LEFT,
    /*! slide up */
    NXE_ANIMATION_SLIDE_IN_UP,
    /*! slide down */
    NXE_ANIMATION_SLIDE_IN_DOWN,
    
    NXE_ANIMATION_IN_MAX
    
} _DOC_ENUM(NXEInAnimationType);

/**
 * \ingroup types
 * \brief A list of layer animation types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXEOutAnimationType)
{
    /*! none */
    NXE_ANIMATION_OUT_NONE = 0,
    /*! fade */
    NXE_ANIMATION_OUT_FADE,
    /*! slide right */
    NXE_ANIMATION_SLIDE_OUT_RIGHT,
    /*! slide left */
    NXE_ANIMATION_SLIDE_OUT_LEFT,
    /*! slide up */
    NXE_ANIMATION_SLIDE_OUT_UP,
    /*! slide down */
    NXE_ANIMATION_SLIDE_OUT_DOWN,
    
    NXE_ANIMATION_OUT_MAX
    
} _DOC_ENUM(NXEOutAnimationType);

/**
 * \ingroup types
 * \brief A list of layer expression types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXEExpressionType)
{
    /*! none */
    NXE_EXPRESSION_NONE = 0,
    /*! blinkslow */
    NXE_EXPRESSION_BLINKSLOW,
    /*! flicker */
    NXE_EXPRESSION_FLICKER,
    
    NXE_EXPRESSION_MAX
    
} _DOC_ENUM(NXEExpressionType);

/**
 * \ingroup types
 * \brief A list of layer types.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXELayerType)
{
    /*! video */
    NXE_LAYER_VIDEO = 0,
    /*! image */
    NXE_LAYER_IMAGE,
    /*! sticker */
    NXE_LAYER_STICKER,
    /*! text */
    NXE_LAYER_TEXT
    
} _DOC_ENUM(NXELayerType);

/**
 * \ingroup types
 * \brief A list of fastoption types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXEFastOption)
{
    /*! normal */
    NXE_NORMAL = 0,
    /*! color adjustment */
    NXE_COLOR_ADJUSTMENT,
    /*! project color adjustment */
    NXE_PROJECT_COLOR_ADJUSTMENT,
    /*! tint color */
    NXE_TINT_COLOR,
    /*! crop rect */
    NXE_CROP_RECT,
    /*! nofx */
    NXE_NOFX,
    /*! swap vertical */
    NXE_SWAP_VERTICAL,
    /*! cts */
    NXE_CTS,
    /*! 360 video */
    NXE_360_VIDEO,
    
} _DOC_ENUM(NXELayerType);

/**
 * \ingroup types
 * \brief A list of rotation angle types.
 * \since version 1.0.19
 */
typedef NS_ENUM(NSUInteger, NXERotationAngle)
{
    /*! rotation 0 */
    NXE_ROTATION_0 = 0,
    /*! rotation 90 */
    NXE_ROTATION_90,
    /*! rotation 180 */
    NXE_ROTATION_180,
    /*! rotation 270 */
    NXE_ROTATION_270
} _DOC_ENUM(NXERotationAngle);

/**
 * \ingroup types
 * \brief A structure that contains width and height values in integer type.
 * \since version 1.1.0
 */
typedef struct  {
    /*! width */
    int32_t width;
    /*! height */
    int32_t height;
} NXESizeInt;

/**
 * \ingroup types
 * \brief A list of clip crop modes.
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, CropMode) {
    /*! The largest dimension of the original clip will be fit to the project display,
     without any distortion of the original dimensions (and the rest of the display will display black). */
    FIT = 0,
    /*! The clip will be cropped to fill the project display, and the original may be distorted. */
    FILL,
    /*! The start and end positions of the crop will be randomly selected. */
    PAN_RAND
} _DOC_ENUM(CropMode);


/**
 * \ingroup types
 * \brief A structure that contains color adjustment values.
 * \since version 1.3.4
 */
typedef struct {
    float brightness;
    float contrast;
    float saturation;
} NXEColorAdjustments;

/**
 * \ingroup types
 * \brief A structure that contains parameter for exporting.
 * \since version 1.3.6
 */
typedef struct {
    /* A width value of the content to be encoded. */
    int width;
    /* A height of the content to be encoded. */
    int height;
    /* A bitrate of the content to be encoded. */
    int bitPerSec;
    /* A frame per seconds to be encoded. */
    int framePerSec;
    /* A duration millisec to set forcefully. */
    int durationMs;
    /* The maximum file size as byte to be exported.*/
    long maxFileSizeByte;
    /* A rotation angle value, NXE_ROTATION_0: 0 degree / NXE_ROTATION_90: 90 degree / NXE_ROTATION_180: 180 degree / NXE_ROTATION_270: 270 degree */
    NXERotationAngle rotationAngle;
} NXEExportParams;

