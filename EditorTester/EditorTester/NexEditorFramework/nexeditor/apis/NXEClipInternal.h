/**
 * File Name   : NXEClipInternal.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "NXEClip.h"
#import "NXEColorEffect.h"
#import <NexEditorEngine/NXEVisualClip.h>
#import <NexEditorEngine/NXEAudioClip.h>

@interface NXEClip (Internal)

@property (nonatomic, retain, readonly) NXEVisualClip *videoInfo;
@property (nonatomic, retain, readonly) NXEAudioClip *audioInfo;

/** \brief speed controll 값이 반영 된 trim duration, in milliseconds.
 *  \since version 1.1.0
 */
@property (nonatomic, readonly) int speedControlledTrimDuration;

/** \brief speed controll 값이 반영 된 trim start time, in milliseconds.
 *  \since version 1.1.0
 */
@property (nonatomic, readonly) int speedControlledStartTrimDuration;

/** \brief speed controll 값이 반영 된 trim end time, in milliseconds.
 *  \since version 1.1.0
 */
@property (nonatomic, readonly) int speedControlledEndTrimDuration;

@property (nonatomic) NXEClipType clipType;
@property (nonatomic) int effectShowTime;
@property (nonatomic) int effectEndTime;

@property (nonatomic, retain) NSMutableDictionary *effectOptions;
@property (nonatomic, retain) NSMutableDictionary *transitionEffectOptions;
@property (nonatomic, retain) NXEColorEffect *colorEffect;

/** \brief Creates the new media clip with the speficied media file path, which is guaranteed to support by the NexEditor&trade; SDK.
 *  \param path The path of media files.
 *  \return An NXEClip object. If failed, NSException is thrown.
 *  \since version 1.0.5
 */
+ (instancetype)newSupportedClip:(NSString *)path;

/** \brief Sets the new preset color effect to a clip.
 *  \param presetName A new preset color effect value.
 *  \see NXEPresetColorEffect, NS_ENUM type
 *  \since version 1.0.5
 */
- (void)setColorEffectValues:(NXEPresetColorEffect)presetName;

/// Updates transformRamp property according to value of 'mode'. CropModeCustom will be discarded.
- (void) setTransformRampWithMode:(CropMode) mode;

@end
