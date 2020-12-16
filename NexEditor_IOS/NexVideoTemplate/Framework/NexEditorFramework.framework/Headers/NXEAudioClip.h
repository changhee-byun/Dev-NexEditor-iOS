/******************************************************************************
 * File Name   :	NXEAudioClip.h
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

@interface NXEAudioClip : NSObject <NSCopying>

- (id) init;

/**
 * \brief The ID of the clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int clipId;

/**
 * \brief The clip path that's been created from a media file.
 * \since version 1.0.5
 */
@property(nonatomic, retain) NSString* clipPath;

/**
 * \brief the playback time if the clip is either an audio clip or a video clip.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int totalTime;

/**
 * \brief A title start time to apply to a clip effect.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int startTime;

/**
 * \brief A title end time to apply to a clip effect.
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
 * \brief The On/Off value for the clip's audio.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int audioOnOff;

/**
 * \brief Indicates whether or not AutoEnvelop is in use.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int autoEnvelop;

/**
 * \brief Indicates the clip volume in the range of 0 ~ 200.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int clipVolume;

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
 * /brief property, setted the voice changer factor which is used to modulate the audio of a clip, range: 0 ~ 4
 *      0: voice changer -> none, default value
 *      1: voice changer -> chipmunk
 *      2: voice changer -> robot
 *      3: voice changer -> deep
 *      4: voice changer -> modulation
 * \since version 1.13.0
 */
@property (nonatomic, assign) int voiceChanger;

/**
 * /brief range: The applied pitch factor of an audio clip, range: -6 ~ 6, default: 0
 * \since version 1.13.0
 */
@property (nonatomic, assign) int pitch;

/**
 * /brief The pan left value to an audio clip, range: -100 ~ 100, default: -100
 * \since version 1.13.0
 */
@property (nonatomic, assign) int panLeft;

/**
 * /brief range: The pan right value to an audio clip, range: -100 ~ 100, default: 100
 * \since version 1.13.0
 */
@property (nonatomic, assign) int panRight;

/**
 * /brief The compressor factor of the audio clip, range: 0 ~ 7, default: 0
 * \since version 1.13.0
 */
@property (nonatomic, assign) int compressor;

/**
 * /brief The processor strength, when applying a music effect to an audio clip, range: 0 ~ 6, default: 0
 *      musicEffect 반영 시 관련 된 값
 * \since version 1.13.0
 */
@property (nonatomic, assign) int processorStrength;

/**
 * /brief the bass strength, when applying a music effect to an audio clip, range: 0 ~ 6, default: 0
 *      musicEffect 반영 시 관련 된 값
 * \since version 1.13.0
 */
@property (nonatomic, assign) int bassStrength;

/**
 * /brief The music effect values applied to the audio clip, range: 0 ~ 3, default: 0
 *      0: music effect -> none
 *      1: music effect -> live concert
 *      2: music effect -> stereo chorus
 *      3: music effect -> enhancer
 * \since version 1.13.0
 */
@property (nonatomic, assign) int musicEffect;

@end
