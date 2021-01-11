/**
 * File Name   : NXEClipInfo.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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
#define NXEVolumeDecimalMin (0)
#define NXEVolumeDecimalMax (200)

/// Milliseconds
typedef int NXETimeMillis;

/// 0 ~ 200
typedef int NXEVolumeDecimal;

@protocol NXEClipInfo <NSObject, NSCopying>

/** \brief The total playback time of a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) NXETimeMillis totalTime;

/** \brief The start time to start playing a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) NXETimeMillis startTime;

/** \brief The end time to stop playing a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) NXETimeMillis endTime;

/** \brief The start time to start trimming a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) NXETimeMillis startTrimTime;

/** \brief The end time to stop trimming a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) NXETimeMillis endTrimTime;

/** \brief The volume level of a clip, ranging from 0 through 200, 100 by default.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) NXEVolumeDecimal clipVolume;

/** \brief The voice changer type of the audio clip, 0 by default.
 *         0: none
 *         1: chipmunk
 *         2: robot
 *         3: deep
 *         4: modulation
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int voiceChanger;

/** \brief The pitch value of the audio clip, ranging from -6 through 6, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int pitch;

/** \brief The pan left value of the audio clip, ranging from -100 through 100, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int panLeft;

/** \brief range: The pan right value of the audio clip, ranging from -100 through 100, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int panRight;

/** \brief The compressor value of the audio clip, ranging from 0 through 7, 0 by default.
 *  \note range: 0 ~ 7, default: 0
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int compressor;

/** \brief The processor strength value of the audio clip, ranging from 0 through 6, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int processorStrength;

/** \brief The bass strength value of the audio clip, ranging from 0 through 6.
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int bassStrength;

/** \brief The music effect type of the audio clip, 0 by default.
 *         0: music effect -> none
 *         1: music effect -> live concert
 *         2: music effect -> stereo chorus
 *         3: music effect -> enhancer
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) int musicEffect;

///
- (id)copy;
@end


@protocol NXEVisualClipInfo <NXEClipInfo>
/** \brief The brightness of the clip, ranging from -255 through 255, 0 by default.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) int brightness;

/** \brief The contrast of the clip, ranging from -255 through 255, 0 by default.
 *  \discussion range: -255 ~ 255
 *  \discussion default: 0
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) int contrast;

/** \brief The saturation of the clip, ranging from -255 through 255, 0 by default.
 *  \discussion range: -255 ~ 255
 *  \discussion default: 0
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) int saturation;

/** \brief The LUT ID of the clip.
 *  \see NXELutType, NS_ENUM type.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) int lut;

/** \brief Indicates whether to apply vignette or not.
 *         0: do not apply vignette
 *         1: apply vignette
 *  \since version 1.0.19
 */
@property(nonatomic, readonly) BOOL vignette;

/** \brief The playback speed of the clip, ranging from 25 through 400.
 *  \discussion 25: 0.25x speed, 100: 1.0x speed, 400: 4.0x speed
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) int speedControl;

/** \brief The effect duration to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, readonly) int effectDuration;

@end

