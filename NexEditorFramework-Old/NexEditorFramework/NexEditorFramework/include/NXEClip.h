/******************************************************************************
 * File Name   :  NXEClip.h
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
#import "NXEEditorErrorCode.h"
#import "NXEAudioEnvelop.h"
#import "NXEEditorType.h"
#import "NXEClipInfo.h"
#import "NXEError.h"
#import "NXEClipSource.h"
#import "NXELutRegistry.h"
#import "NXEClipTransform.h"
@class NXEEffectOptions;
@class NXEProject;
@class MediaInfo;
@class NXEThumbnailUICallbackBlock;

extern NSString *const kEffectIDNone;

/** \class NXEClip
 *  \brief NXEClip class defines all properties and methods to manage a clip such as image clip, video clip and audio clip.
 *
 * The following media formats are supported:
 *
 * Supported video file format: MP4, 3GP and MOV<p>
 * Supported video codec: H.264<p>
 * Supported audio file format: MP3 and AAC<p>
 * Supported image file format: iOS dependency<p>
 *
 * Newly created clip will be saved in the same path of the original media content which the clip was created from.
 * If the format of the clip is unsupported, <tt>NXE_CLIPTYPE_NONE</tt> will be returned from getClipType.
 * Another reason for a clip to be unsupported may be because of the resolution limitations from the hardware performance.
 * Only after a clip is added to NXEProject, it can be edited or have effects applied.
 */
@interface NXEClip : NSObject <NSCopying>

/** \brief The properties of a visual clip, such as video, image, and video layer.
 *  \see NXEVisualClip, NXEVisualClip class.
 *  \since version 1.0.5
 */
@property (nonatomic, retain, readonly) id<NXEVisualClipInfo> vinfo;

/** \brief The properties of an audio clip.
 *  \see NXEAudioClip, NXEAudioClip class.
 *  \since version 1.0.5
 */
@property (nonatomic, retain, readonly) id<NXEClipInfo> ainfo;

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
@property (nonatomic, readonly) NXEClipType clipType;

/** \brief The start time where to start applying an effect.
 *  \since version 1.0.5
 */
@property (nonatomic, readonly) int effectShowTime;

/** \brief The end time where to stop applying an effect.
 *  \since version 1.0.5
 */
@property (nonatomic, readonly) int effectEndTime;

/** \brief A Boolean value indicates whether to loop playing audio clip when the length of an audio clip is shorter than the length of a video clip.
 *  \note This property is valid only to an audio clip.
 *  \since version 1.0.5
 */
@property (nonatomic, assign) BOOL bLoop;

/** \brief Sets the start and end croppiing position of a clip randomly based on the speficied crop mode.
 *  \note The default value is CropModeFit.
 *  \since version 1.0.5
 */
@property (nonatomic) CropMode cropMode;

/**
 * \brief Sets the starting and ending position in the render buffer. Rotation will be discarded.
 * \note Changing cropMode also changes this property. The default value is determined by the default value of cropMode property.
 * \since version 1.5.1
 */
@property (nonatomic) NXEClipTransformRamp transformRamp;

/**
 * \brief Size of the clip in pixels, if the receiver is an image or video clip.
 * \since version 1.5.1
 */
@property (nonatomic, readonly) CGSize size;

/** \brief The properties of an audio envelope.
 *  \see NXEAudioEnvelop, NXEAudioEnvelop class.
 *  \since version 1.0.19
 */
@property (nonatomic, retain) NXEAudioEnvelop *audioEnvelop;

/** \brief trim head duration, in milliseconds.
 *  \since version 1.1.0
 */
@property (nonatomic, readonly) int headTrimDuration;

/** \brief trim tail duration, in milliseconds.
 *  \since version 1.1.0
 */
@property (nonatomic, readonly) int trimEndTime;

/** 
 * \brief Clip duration, in milliseconds.
 * \since version 1.2
 */
@property (nonatomic, readonly) NXETimeMillis durationMs;

/** 
 * \brief Clip source used to create the receiver.
 *  \since version 1.2
 */
@property (nonatomic, readonly) NXEClipSource *clipSource;


/**
 * \brief LUT effect for this clip.
 * \since version 1.3.0
 */
@property (nonatomic) NXELutID lutId;

/** \brief Clip effect id for this clip.
 *  \since version 1.5.1
 */
@property (getter=getClipEffectID, setter=setClipEffect:, nonatomic, strong) NSString *clipEffectID;

/** \brief Transition effect id for this clip.
 *  \since version 1.5.1
 */
@property (getter=getTransitionEffectID, setter=setTransitionEffect:, nonatomic, strong) NSString *transitionEffectID;

/**
 * \brief Color adjustment values such as brightness, contrast, and saturation whose value ranges between -1.0 and 1.0. Default of each value is 0.
 * \since version 1.x
 */
@property (nonatomic) NXEColorAdjustments colorAdjustment;

/** \brief Sets the audio volume to a clip.
 *  \param volume A new audio volume to set, ranging from 0 through 200, 100 by default.
 *  \since version 1.0.5
 */
- (void)setClipVolume:(int)volume;

/** \brief Returns the current brightness value of a clip.
 *  \return A brightness value of a clip.
 *  \since version 1.0.5
 */
- (int)getCombinedBrightness;

/** \brief Sets the brightness value to a clip.
 *  \param brightnessValue A new brightness value to set, ranging from -255 through 255.
 *  \return A Boolean value indicates whether the operation is successful or not.
 *  \since version 1.0.5
 */
- (BOOL)setBrightness:(int)brightnessValue;

/** \brief Returns the current contrast value of a clip.
 *  \return A contrast value of a clip.
 *  \since version 1.0.5
 */
- (int)getCombinedContrast;

/** \brief Sets the contrast value to a clip.
 *  \param contrastValue A new contrast value to set, ranging from -255 through 255.
 *  \return A Boolean value indicates whether the operation is successful or not.
 *  \since version 1.0.5
 */
- (BOOL)setContrast:(int)contrastValue;

/** \brief Returns the current saturation value of a clip.
 *  \return A saturation value of a clip.
 *  \since version 1.0.5
 */
- (int)getCombinedSaturation;

/** \brief Sets the saturation value to a clip.
 *  \param saturationValue A new saturation value to set, ranging from -255 through 255.
 *  \return A Boolean value indicates whether the operation is successful or not.
 *  \since version 1.0.5
 */
- (BOOL)setSaturation:(int)saturationValue;

/** \brief Returns the clip effect ID of a clip.
 *  \return A clip effect ID.
 *  \since version 1.0.19
 */
- (NSString *)getClipEffectID;

/** \brief Sets the new clip effect to a clip.
 *  \param effectid A clip effect ID.
 *  \since version 1.0.5
 */
- (void)setClipEffect:(NSString*)effectid;

/** \brief Sets the new clip effect to a clip with the speficied clip effect option.
 *  \param effectId A clip effect ID.
 *  \param effectOption A clip effect option.
 *  \since version 1.0.5
 */
- (void)setClipEffect:(NSString *)effectId effectOption:(NXEEffectOptions *)effectOption;

/** \brief Returns the current transition effect ID of a clip.
 *  \return A transition effect ID.
 *  \since version 1.0.19
 */
- (NSString *)getTransitionEffectID;

/** \brief Sets the new transition effect to a clip.
 *  \param effectId A transition effect ID.
 *  \since version 1.0.5
 */
- (void)setTransitionEffect:(NSString*)effectId;

/** \brief Sets the new transition effect to a clip with the speficied transition effect option.
 *  \param effectId A transition effect ID.
 *  \param effectOption A transition effect option.
 *  \since version 1.0.5
 */
- (void)setTransitionEffect:(NSString *)effectId effectOption:(NXEEffectOptions *)effectOption;

/** \brief Finds the optimal duration of a tansition effect and applies it to the clips in a project.
 *  \param project An NXEProject.
 *  \param index A clip index.
 *  \since version 1.0.5
 */
- (void)ensureTransitionFit:(NXEProject *)project wantedClipIndex:(int)index;

/** \brief Sets the new transition effect duration to a clip.
 *  \param duration A duration value to set, in milliseconds.
 *  \since version 1.0.5
 */
- (void)setTransitionEffectDuration:(int)duration;

/** \brief Sets the start and end time of an audio clip to play.
 *  \param startTime The start time to start playing an audio clip.
 *  \param endTime The end time to stop playing an audio clip.
 *  \since version 1.0.19
 */
- (void)setAudioClipStartTime:(int)startTime EndTime:(int)endTime;

/** \brief Sets the start and end time of a clip to trim.
 *  \note Previous trim setting, if there is, will be ignored and the new trim setting will be applied to a clip.
 *  \param startTime The start time to start trimming, in milliseconds.
 *  \param endTime The end time to stop trimming, in milliseconds.
 *  \since version 1.0.5
 */
- (void)setTrim:(int)startTime EndTrimTime:(int)endTime;

/** \brief Sets the playback speed of a video clip.
 *  \note If a new trim is applied to a clip, existing playback speed setting of the clip will be voided.
 *  \param clipSpeed A new playback speed to set, ranging from 13% through 400%.
 *  13%(:slow), 100%(:standard), 400%(:fast)
 *  \since version 1.0.5
 */
- (void)setSpeed:(int)clipSpeed;

/** \brief Returns the start and end time to start displaying a clip effect.
 *  \param startTime The start time to start displaying a clip effect, in milliseconds.
 *  \param endTime The end time to stop displaying a clip effect, in milliseconds.
 *  \since version 1.0.5
 */
- (void)getEffectShowTime:(int *)startTime endTime:(int *)endTime;

/** \brief Sets the start and end time of a clip to display.
 *  \note This method should be used only to clips on video layer.
 *  \param startTime The start time to start displaying a clip, in milliseconds.
 *  \param endTime The end time to stop displaying a clip, in milliseconds.
 *  \since version 1.0.19
 */
- (void)setDisplayStartTime:(int)startTime endTime:(int)endTime;

/** \brief Sets the start and end time of a clip effect to display.
 *  \note The clip effect's duration will automatically be adjusted to the clip's duration, if the effect duration set with this method is longer than the clip's duration.
 *  \param startTime The start time to start displaying a clip effect, in milliseconds.
 *  \param endTime The end time to stop displaying a clip effect, in milliseconds.
 *  \since version 1.0.5
 */
- (void)setEffectShowTime:(int)startTime endTime:(int)endTime;

/** \brief Sets the duration of an image clip.
 *  \param time The duration to set, in milliseconds.
 *  \since version 1.0.5
 */
- (void)setImageClipDuration:(int)time;

/** \brief Creates the new media clip with the speficied color applied, which is guaranteed to support by the NexEditor&trade; SDK.
 *  \param color A solid color value.
 *  \return An NXEClip object.
 *  \since version 1.0.5
 */
+ (instancetype)newSolidClip:(NSString *)color;

/** \brief Creates the new media clip with the speficied clip source, which is guaranteed to support by the NexEditor&trade; SDK.
 *  \param source NXEClipSource represents a clip media file or PHAsset
 *  \param error NXEError object will be assigned if clip creation failed. Unchanged if clip creation was successful.
 *  \return An NXEClip object if succeed, nil if failed.
 *  \since version 1.2
 */
+ (instancetype)clipWithSource:(NXEClipSource *) source error:(NSError **)error;

/** \brief Sets the new LUT effect to a clip.
 *  \param lutTypes A LUT type to set.
 *  \since version 1.0.5
 *  \deprecated Use lutId property instead.
 */
- (void)setLut:(NXELutTypes)lutTypes __attribute__((deprecated));

/** \brief Sets the vignette effect to a clip.
 *  \param isApply A Boolean value whether to apply vignette effect or not.
 *  \since version 1.0.19
 */
- (void)setVignette:(BOOL)isApply;

/** \brief Sets the compressor value to an audio clip, ranging from 0 through 7.
 *  \param compressorValue A compressor value to set.
 *  \since version 1.0.19
 */
- (void)setCompressorValue:(int)compressorValue;

/** \brief Sets the pan left value to an audio clip, ranging from -100 through 100.
 *  \param panLeftValue A pan left value to set.
 *  \since version 1.0.19
 */
- (void)setPanLeftValue:(int)panLeftValue;

/** \brief Sets the pan right value to an audio clip, ranging from -100 through 100.
 *  \param panRightValue A pan right value to set.
 *  \since version 1.0.19
 */
- (void)setPanRightValue:(int)panRightValue;

/** \brief Sets the pitch value to an audio clip, ranging from -6 through 6.
 *  \param pitchValue A pitch value to set.
 *  \since version 1.0.19
 */
- (void)setPitchValue:(int)pitchValue;

/** \brief Sets the voice changer value to an audio clip, ranging from 0 through 4.
 *         0: none
 *         1: chipmunk
 *         2: robot
 *         3: deep
 *         4: modulation
 *  \param voiceChangerValue A voice changer value to set.
 *  \since version 1.0.19
 */
- (void)setVoiceChangerValue:(int)voiceChangerValue;

/** \brief Returns the total number of I-frames of a video clip.
 *  \return The total number of I-Frames.
 *  \since version 1.0.5
 */
- (int)getSeekPointCount;

/** \brief Returns the average interval of I-Frames of a video clip.
 *  \return The average interval of I-frames, in milliseconds.
 *  \since version 1.0.5
 */
- (int)getSeekPointInterval;

@end
