/******************************************************************************
 * File Name   :	NXEVisualClip.h
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
#import <UIKit/UIKit.h>

extern NSString *const kEffectIDNone;

/** \class NXEVisualClip
 *  \brief NXEVisualClip class defines all properties associated with a visual clip.
 *  \since version 1.0.5
 */
@interface NXEVisualClip : NSObject <NSCopying>

/** \brief The current type of the clip.
 *         0: clip has not yet been clarified
 *         1: image
 *         2: text
 *         3: audio
 *         4: video
 *         5: effect
 *         6: layer
 *         7: video layer
 *  \since version 1.0.5
 */
@property(nonatomic) int clipType;

/** \brief The ID of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int clipId;

/** \brief The audio track duration of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int totalAudioTime;

/** \brief The video track duration of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int totalVideoTime;

/** \brief The total playback time of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int totalTime;

/** \brief Indicates the start time where playback starts from.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int startTime;

/** \brief Indicates the end time where playback stops at.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int endTime;

/** \brief Indicates the start time where trim starts from.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int startTrimTime;

/** \brief Indicates the end time where trim stops at.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int endTrimTime;

/** \brief The width of the clip.
 *    \since version 1.0.5
 */
@property(nonatomic, assign) int width;

/** \brief The height of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int height;

/** \brief Indicates whether the clip contains a video track or not.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int existVideo;

/** \brief Indicates whether the clip contains an audio track or not.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int existAudio;

/** \brief The title name of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSString* title;

/** \brief The title style of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int titleStyle;

/** \brief Indicates the start time where the title disply starts from.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int titleStartTime;

/** \brief Indicates the end time where the title display stops at.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int titleEndTime;

/** \brief Indicates whether to play an audio track or not.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int audioOnOff;

/** \brief The playback volume of the clip, ranging from 1 through 200, 100 by default.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int clipVolume;

/** \brief The BGM volume of the clip, ranging from 1 through 200, 100 by default.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int bGMVolume;

/** \brief The rotation angle of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int rotateState;

/** \brief The brightness of the clip, ranging from -255 through 255, 0 by default.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int brightness;

/** \brief The contrast of the clip, ranging from -255 through 255, 0 by default.
 *  \note range: -255 ~ 255, default: 0
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int contrast;

/** \brief The saturation of the clip, ranging from -255 through 255, 0 by default.
 *  \note range: -255 ~ 255, default: 0
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int saturation;

/** \brief The tint color of the clip in ARGB color format.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int tintcolor;

/** \brief The LUT ID of the clip.
 *  \see NXELutType, NS_ENUM type.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int lut;

/** \brief Indicates whether to apply vignette or not.
 *         0: do not apply vignette
 *         1: apply vignette
 *  \since version 1.0.19
 */
@property(nonatomic, assign) int vignette;

/** \brief The playback speed of the clip, ranging from 25 through 400.
 *  \note 25: 0.25x speed, 100: 1.0x speed, 400: 4.0x speed
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int speedControl;

/** \brief The effect type to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int effectType;

/** \brief The effect duration to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int effectDuration;

/** \brief The effect offset to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int effectOffset;

/** \brief The duration of time that clips overlap during a transition effect, as a percentage of the total transition time.
 *   The overlap is indicated as a percentage of the total duration of the transition effect, so for example, if a transition effect takes
 *   4 seconds to complete, and the neighboring clips overlap for 2 seconds of that transition, the value returned here for the
 *   example transition effect will be 50.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int effectOverlap;

/** \brief The transition effect ID of the clip.
 *  \since version 1.0.5
 */
@property(getter=getTransitionEffectId, nonatomic, retain) NSString* clipEffectID;

/** \brief The clip effect ID of the clip.
 *  \since version 1.0.5
 */
@property(getter=getClipEffectId, nonatomic, retain) NSString* titleEffectID;

/** \brief The filter ID to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSString* filterID;

/**  \brief This method sets the starting position of a crop on a clip.
 *   The original size of the clip is used as the frame of reference for the crop.
 *   \note If, for example, the cropped section of the original should be displayed at a ratio of 16:9, then
 *   the crop coordinates should also be set to a ratio of 16:9.
 *   \since version 1.0.5
 */
@property(nonatomic, assign) CGRect startRect;

/** \brief The end crop position to be displayed on a clip, as a \c rectangle.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) CGRect endRect;

/** \brief The destination crop position to be displayed on a clip, as a \c rectangle.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) CGRect destRect;

/** \brief The path of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSString* clipPath;

/** \brief The path of the thumbnail image.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSString* thumbnailPath;

/** \brief The audio envelope level to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* volumeEnvelopeLevel;

/** \brief The audio envelope time to apply to a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* volumeEnvelopeTime;

/** \brief The value that stores an error code from clip initialization.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int errorcode;

/** \brief The voice changer type of the audio clip, 0 by default.
 *         0: none
 *         1: chipmunk
 *         2: robot
 *         3: deep
 *         4: modulation
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int voiceChanger;

/** \brief The pitch value of the audio clip, ranging from -6 through 6, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int pitch;

/** \brief The pan left value of the audio clip, ranging from -100 through 100, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int panLeft;

/** \brief range: The pan right value of the audio clip, ranging from -100 through 100, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int panRight;

/** \brief The compressor value of the audio clip, ranging from 0 through 7, 0 by default.
 *  \note range: 0 ~ 7, default: 0
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int compressor;

/** \brief The processor strength value of the audio clip, ranging from 0 through 6, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int processorStrength;

/** \brief The bass strength value of the audio clip, ranging from 0 through 6.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int bassStrength;

/** \brief The music effect type of the audio clip, 0 by default.
 *         0: music effect -> none
 *         1: music effect -> live concert
 *         2: music effect -> stereo chorus
 *         3: music effect -> enhancer
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int musicEffect;

/** \brief NO by default.
 */
@property (nonatomic, assign) BOOL useIFrameOnly;

/** \brief NO by default.
 */
@property (nonatomic, assign) BOOL keepPitch;

@end
