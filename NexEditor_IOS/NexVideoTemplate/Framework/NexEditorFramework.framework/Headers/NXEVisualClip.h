/******************************************************************************
 * File Name   :	NXEVisualClip.h
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
#import <UIKit/UIKit.h>

@interface NXEVisualClip : NSObject <NSCopying>

- (id) init;

/**
 * \brief The status of the clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int clipType;

/**
 * \brief The id of the clip. 
 * \since version 1.0.5
 */
@property(nonatomic, assign) int clipId;

/**
 * \brief The duration value of an audio file.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int totalAudioTime;

/**
 * \brief The duration value of a video file.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int totalVideoTime;

/**
 * \brief The playback time if the clip is either an audio clip or a video clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int totalTime;

/**
 * \brief The start time when playing a clip. 
 * \since version 1.0.5
 */
@property(nonatomic, assign) int startTime;

/**
 * \brief The end time when playing a clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int endTime;

/**
 * \brief The start time of a trim when playing a clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int startTrimTime;

/**
 * \brief The end time of a trim when playing a clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int endTrimTime;

/**
 * \brief The width of a clip to be shown when playing a clip. 
 * \since version 1.0.5
 */
@property(nonatomic, assign) int width;

/**
 * \brief The height of a clip to be shown when playing a clip. 
 * \since version 1.0.5
 */
@property(nonatomic, assign) int height;

/**
 * \brief Calls a media file and indicates whether the created clip has an audio.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int existVideo;		//?

/**
 * \brief Calls a media file and indicates whether the created clip has an audio. 
 * \since version 1.0.5
 */
@property(nonatomic, assign) int existAudio;		//?

/**
 * \brief A title name to apply to a clip effect. 
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSString* title;

/**
 * \brief A title style to apply to a clip effect.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int titleStyle;

/**
 * \brief A title start time to apply to a clip effect.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int titleStartTime;

/**
 * \brief A title end time to apply to a clip effect.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int titleEndTime;

/**
 * \brief The On/Off value for the clip's audio.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int audioOnOff;

/**
 * \brief Indicates the clip volume in the range of 0 ~ 200.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int clipVolume;		// 0 - 200

/**
 * \brief Indicates the BGM volume in the range of 0 ~ 200.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int bGMVolume;			// 0 - 200

/**
 * \brief Indicates the clip's rotation angle.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int rotateState;


/**
 * \brief Indicates the clip's brightness in the range of -255 ~ 255.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int brightness;

/**
 * \brief Indicates the clip's contrast in the range of -255 ~ 255.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int contrast;

/**
 * \brief Indicates the clip's saturation in the range of -255 ~ 255.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int saturation;

/**
 * \brief Indicates the clip's tint color in the ARGB color format as an \c integer.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int tintcolor;

/**
 * \brief Indicates the clip's look up table value.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int lut;

/**
 * \brief Indicates the video clip's play speed value when playing media.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int speedControl;			/// 25 ~ 200

/**
 * \brief The effect type to apply to a clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int effectType;

/**
 * \brief The effect duration to apply to a clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int effectDuration;

/**
 * \brief The effect offset to apply to a clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int effectOffset;

/**
 * \brief The duration of time that clips overlap during a transition effect, as a percentage of the total transition time.
 *
 * The overlap is indicated as a percentage of the total duration of the transition effect, so for example, if a transition effect takes
 * 4 seconds to complete, and the neighboring clips overlap for 2 seconds of that transition, the value returned here for the
 * example transition effect will be 50.
 *
 * \since version 1.0.5
 */
@property(nonatomic, assign) int effectOverlap;

/**
 * \brief The clip effect id to apply to a clip.
 * \since version 1.0.5
 */
@property(getter=getTransitionEffectId, nonatomic, retain) NSString* clipEffectID;

/**
 * \brief The transition effect id to apply to a clip.
 * \since version 1.0.5
 */
@property(getter=getClipEffectId, nonatomic, retain) NSString* titleEffectID;

/**
 * \brief The filter ID to apply to a clip.
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSString* filterID;

/**
 * \brief This method sets the starting position of a crop on a clip.
 *
 * The original size of the clip is used as the frame of reference for the crop.
 *
 * \note If, for example, the cropped section of the original should be displayed at a ratio of 16:9, then
 * the crop coordinates should also be set to a ratio of 16:9.
 *
 * \since version 1.0.5
 */
@property(nonatomic, assign) CGRect startRect;

/**
 * \brief The end crop position to be displayed on a clip, as a rectangle.
 * \since version 1.0.5
 */
@property(nonatomic, assign) CGRect endRect;

/**
 * \brief The destination crop position to be displayed on a clip, as a rectangle.
 * \since version 1.0.5
 */
@property(nonatomic, assign) CGRect destRect;

/**
 * \brief
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* startMatrix;

/**
 * \brief
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* endMatrix;

/**
 * \brief The clip path that's been created from a media file.
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSString* clipPath;

/**
 * \brief The file path of thumbnail information that is saved when creating a thumbnail.
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSString* thumbnailPath;


/**
 * \brief The audio envelope level to apply to a clip.
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* volumeEnvelopeLevel;

/**
 * \brief The audio envelope time to apply to a clip.
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* volumeEnvelopeTime;

/**
 * \brief
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSArray* audioClipVec;

/**
 * \brief The value that stores an error code from clip initialization.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int errorcode;

/**
 * /brief property, setted the voice changer factor which is used to modulate the audio of a clip, range: 0 ~ 4
 *      0: voice changer -> none
 *      1: voice changer -> chipmunk
 *      2: voice changer -> robot
 *      3: voice changer -> deep
 *      4: voice changer -> modulation
 * \since version 1.13.0
 */
@property (nonatomic, assign) int voiceChanger;

/**
 * /brief range: The applied pitch factor of an audio clip, range: -6 ~ 6
 * \since version 1.13.0
 */
@property (nonatomic, assign) int pitch;

/**
 * /brief The pan left value to an audio clip, range: -100 ~ 100
 * \since version 1.13.0
 */
@property (nonatomic, assign) int panLeft;

/**
 * /brief range: The pan right value to an audio clip, range: -100 ~ 100
 * \since version 1.13.0
 */
@property (nonatomic, assign) int panRight;

/**
 * /brief The compressor factor of the audio clip, range: 0 ~ 7
 * \since version 1.13.0
 */
@property (nonatomic, assign) int compressor;

/**
 * /brief The processor strength, when applying a music effect to an audio clip, range: 0 ~ 6
 *      musicEffect 반영 시 관련 된 값
 * \since version 1.13.0
 */
@property (nonatomic, assign) int processorStrength;

/**
 * /brief the bass strength, when applying a music effect to an audio clip, range: 0 ~ 6
 *      musicEffect 반영 시 관련 된 값
 * \since version 1.13.0
 */
@property (nonatomic, assign) int bassStrength;

/**
 * /brief The music effect values applied to the audio clip, range: 0 ~ 3
 *      0: music effect -> none
 *      1: music effect -> live concert
 *      2: music effect -> stereo chorus
 *      3: music effect -> enhancer
 * \since version 1.13.0
 */
@property (nonatomic, assign) int musicEffect;

@end
