/******************************************************************************
 * File Name   :  NXEAudioClip.h
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

/** \class NXEAudioClip
 *  \brief NXEAudioClip class defines all properties of an audio clip.
 *  \since version 1.0.5
 */
@interface NXEAudioClip : NSObject <NSCopying>

/** \brief The ID of the clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int clipId;

/** \brief The clip path of a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, retain) NSString* clipPath;

/** \brief The total playback time of a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int totalTime;

/** \brief The start time to start playing a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int startTime;

/** \brief The end time to stop playing a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int endTime;

/** \brief The start time to start trimming a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int startTrimTime;

/** \brief The end time to stop trimming a clip.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int endTrimTime;

/** \brief A Boolean value indicates whether to play an audio or not.
 *  \since version 1.0.5
 */
@property(nonatomic, assign) int audioOnOff;

/**
 * \brief Indicates whether or not AutoEnvelop is in use.
 * \since version 1.0.5
 */
@property(nonatomic, assign) int autoEnvelop;

/** \brief The volume level of a clip, ranging from 0 through 200, 100 by default.
 *  \since version 1.0.5
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

/** \brief The voice changer type of the audio clip, 0 by default.
 *         0: voice changer -> none, default value
 *         1: voice changer -> chipmunk
 *         2: voice changer -> robot
 *         3: voice changer -> deep
 *         4: voice changer -> modulation
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int voiceChanger;

/** \brief The applied pitch value of an audio clip, ranging from -6 through 6, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int pitch;

/** \brief The pan left value of an audio clip, ranging from -100 through 100, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int panLeft;

/** \brief The pan right value of an audio clip, ranging from -100 through 100, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int panRight;

/** \brief The compressor value of the audio clip, ranging from 0 through 7, 0 by default.
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int compressor;

/** \brief The processor strength value of an audio clip, ranging from 0 through 6, 0 by default.
 *  This is a musicEffect-related value. 
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int processorStrength;

/** \brief The bass strength value of an audio clip, ranging from 0 through 6.
 *  This is a musicEffect-related value. 
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

@end
