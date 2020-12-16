/******************************************************************************
 * File Name   :	NXEEditorType.h
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
 * \brief A list of Clip Types
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
 * \brief A list of Clip Effect Title Types
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
 * \brief A list of Aspect Types
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXEAspectType)
{
    /*! Set to this mode for a 1x1 screen ratio. */
    NXE_ASPECT_1v1 = 0x00000000,
    /*! Set to this mode for a 16x9 screen ratio. */
    NXE_ASPECT_16v9 = 0x00000001,
    /*! Set to this mode for a 9x16 screen ratio. */
    NXE_ASPECT_9v16 = 0x00000002,
} _DOC_ENUM(NXEAspectType);

/**
 * \ingroup types
 * \brief A list of Effect Types
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
 * \typedef Play State
 * \brief A list of Effect Types
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, NXEPLAYSTATE)
{
    NXE_PLAYSTATE_NONE = 0,
    NXE_PLAYSTATE_IDLE = 1,
    NXE_PLAYSTATE_RUN = 2,
    NXE_PLAYSTATE_RECORD = 3,
    NXE_PLAYSTATE_PAUSE = 4,
    NXE_PLAYSTATE_RESUME = 5
} _DOC_ENUM(NXEPLAYSTATE);

/**
 * \ingroup types
 * \brief LUT types
 */
typedef NS_ENUM(NSUInteger, NXELutTypes)
{
    NXE_LUT_NONE = 0,
    NXE_LUT_AFTERNOON,
    NXE_LUT_ALMOND_BLOSSOM,
    NXE_LUT_AUTUMN,
    NXE_LUT_BORING,
    NXE_LUT_CARAMEL_CANDY,
    NXE_LUT_CARIBBEAN,
    NXE_LUT_CINNAMON,
    NXE_LUT_CLOUD,
    NXE_LUT_CORAL_CANDY,
    NXE_LUT_CRANBERRY,
    NXE_LUT_DAISY,
    NXE_LUT_DAWN,
    NXE_LUT_DISNEY,
    NXE_LUT_ENGLAND,
    NXE_LUT_ESPRESSO,
    NXE_LUT_EYESHADOW,
    NXE_LUT_GLOOMY,
    NXE_LUT_JAZZ,
    NXE_LUT_LAVENDAR,
    NXE_LUT_MOONLIGHT,
    NXE_LUT_NEWSPAPER,
    NXE_LUT_PARIS,
    NXE_LUT_PEACH,
    NXE_LUT_RAINY,
    NXE_LUT_RASPBERRY,
    NXE_LUT_RETRO,
    NXE_LUT_SHERBERT,
    NXE_LUT_SHINY,
    NXE_LUT_SMOKE,
    NXE_LUT_STONEEDGE,
    NXE_LUT_SUNRISING,
    NXE_LUT_SYMPHONY_BLUE,
    NXE_LUT_TANGERINE,
    NXE_LUT_TIFFANY,
    NXE_LUT_VINTAGE_FLOWER,
    NXE_LUT_ROMANCE,
    NXE_LUT_VIVID,
    NXE_LUT_WARM,
    NXE_LUT_CARAMEL,
    NXE_LUT_JN10,
    NXE_LUT_LEMONADE,
    NXE_LUT_LOVE_01,
    NXE_LUT_LOVE_02,
    NXE_LUT_LOVE_03,
    NXE_LUT_LOVE_04,
    NXE_LUT_LOVE_PINK_JN01,
    NXE_LUT_LOVE_PINK_RED01,
    NXE_LUT_LOVE_Y01,
    NXE_LUT_LOVE_Y02,
    NXE_LUT_LOVE_Y03,
    NXE_LUT_ORANGE,
    NXE_LUT_ORANGE1,
    NXE_LUT_PINK,
    NXE_LUT_SPORTS,
    NXE_LUT_TRANSSION_SPORTS,
} _DOC_ENUM(NXELutTypes);

/**
 * \ingroup types
 * \brief Voice Changer
 */
typedef NS_ENUM(NSUInteger, NXEVoiceChanger)
{
    NXE_VOICE_FACTOR_NONE = 0,
    NXE_VOICE_FACTOR_CHIPMUNK,
    NXE_VOICE_FACTOR_ROBOT,
    NXE_VOICE_FACTOR_DEEP,
    NXE_VOICE_FACTOR_MODULATION
} _DOC_ENUM(NXEVoiceChanger);

/**
 * \ingroup types
 * \brief Music Effect
 */
typedef NS_ENUM(NSUInteger, NXEMusicEffect)
{
    NXE_MUSIC_EFFECT_NONE = 0,
    NXE_MUSIC_EFFECT_LIVE_CONCERT,
    NXE_MUSIC_EFFECT_STEREO_CHORUS,
    NXE_MUSIC_EFFECT_ENHANCER
} _DOC_ENUM(NXEMusicEffect);
