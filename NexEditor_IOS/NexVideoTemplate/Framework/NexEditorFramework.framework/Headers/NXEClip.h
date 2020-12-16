/******************************************************************************
 * File Name   :	NXEClip.h
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
#import "NXEColorEffect.h"
#import "NXEEditorErrorCode.h"
#import "NXEVisualEdit.h"
#import "NXEAudioEdit.h"
#import "NXEEditorType.h"

@class NXEEffectOptions;
@class NXEVisualClip;
@class NXEAudioClip;
@class NXEProject;
@class MediaInfo;
@class ClipEffect;
@class NXEThumbnailUICallbackBlock;

/**
 * This class manages image clips, video clips and audio clips to be edited with the NexEditorSDK.
 * NexEditorSDK supports the following file formats:
 *
 * Supported video file formats : MP4, 3GP and MOV.
 * Supported video codec : H.264
 * Supported audio file formats : MP3 and AAC.
 * Supported image file formats : iOS dependency.
 * The clips will be saved in the path of the original media content which the clip was created from
 * and if the format of the clip is unsupported, NE_CLIPTYPE_NONE will be returned from getClipType.
 * Another reason for a clip to be unsupported may be because of the resolution limitations from the hardware performance.
 * Only after a clip is added to NXEProject can it be edited or have effects applied.
 * When editing a clip that is not added to NXEProject, ProjectNotAttachedException will occur.
 *
 * \since version 1.0.5
 *
 */
@interface NXEClip : NSObject <NSCopying>

/**
 * \brief The type value of a clip.
 * \see CLIP_TYPE
 * \since version 1.0.5
 */
@property (nonatomic, assign) int clipType;

/**
 * \brief The result value for initialization.
 * \since version 1.0.5
 */
@property (nonatomic, assign) ERRORCODE errorcode;

/**
 * \brief This has the value of the Effect Show Time.
 * \since version 1.0.5
 */
@property (nonatomic, assign) int effectShowTime;

/**
 * \brief This has the value of the Effect End Time.
 * \since version 1.0.5
 */
@property (nonatomic, assign) int effectEndTime;

/**
 * \brief The information value of a visual clip(video, image, and video layer).
 * \since version 1.0.5
 */
@property (nonatomic, retain) NXEVisualClip *vinfo;

/**
 * \brief The information value of an audio clip.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NXEAudioClip *ainfo;

/**
 * \brief audio에 대해 bgm 속성을 줄지 아닐지?
 */
@property (nonatomic, assign) BOOL bBGM;

/**
 * \brief BGM 대한 Volume Scale 값을 정해준다. 
 *  만약, bBGM 설정이 false의 경우, bgmVolumeScale값은 입력되는 parameter값 상관없이 항상 0.0f를 갖게 된다.
 *  0.0 ~ 1.0 사이의 값을 입력해 줘야 한다.
 */
@property (nonatomic, assign, setter=setBGMVolumeScale:) float bgmVolumeScale;

/**
 * \brief audio에 대해 loop 속성을 줄지 아닐지?
 */
@property (nonatomic, assign) BOOL bLoop;

/**
 * \brief
 * \since version 1.0.5
 */
@property (nonatomic, retain) NXEVisualEdit *vedit;

/**
 * \brief
 * \since version 1.0.5
 */
@property (nonatomic, retain) NXEAudioEdit *aedit;

/**
 * \brief The information value of color effects.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NXEColorEffect *colorEffect;

/**
 * \brief The options value of a set clip effect.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableDictionary *effectOptions;

/**
 * \brief The options value of a set transition effect.
 * \since version 1.0.5
 */
@property (nonatomic, retain) NSMutableDictionary *transitionEffectOptions;

/**
 * \brief This method sets the volume of a clip.
 * \param volume The volume of the clip, as an \c integer.  This value can range from 0 to 200 and the default value is 100.
 * \return \c FALSE if the input value is not in the range of 0 to 200, otherwise \c TRUE.
 * \see NXEVisualClip property 중, clipVolume
 * \since version 1.0.5
 */
- (BOOL)setClipVolume:(int)volume;

/**
 * \brief This method sets the brightness of a clip.
 * \param value The brightness to set on the clip, as an <tt>integer</tt>.  This must be in the range of -255 to 255.
 * \return \c TRUE if the value is set successfully; \c FALSE if the value is not in range.
 * \see NXEVisualClip property 중, brightness
 * \since version 1.0.5
 */
- (BOOL)setBrightness:(int)value;

/**
 * \brief This method sets the contrast of a clip.
 * \param value The contrast to set on the clip, as an <tt>integer</tt>.  This must be in the range of -255 to 255.
 * \return \c TRUE if the value is set successfully; \c FALSE if the value is not in range.
 * \see NXEVisualClip property 중, contrast
 * \since version 1.0.5
 */
- (BOOL)setContrast:(int)value;

/**
 * \brief This method sets the saturation of a clip.
 * \param value The saturation to set on the clip, as an integer. This must be in the range of -255 to 255.
 * \return \c TRUE if the value is set successfully; \c FALSE if the value is not in range.
 * \see NXEVisualClip property 중, saturation
 * \since version 1.0.5
 */
- (BOOL)setSaturation:(int)value;

/**
 * \brief This method sets a clip effect, that is not part of any existing theme on a clip.
 * \param effectid The clip effect ID as a NSString.
 * \see NXEVisualClip property 중, titleEffectID
 * \since version 1.0.5
 */
- (void)setClipEffect:(NSString*)effectid;

/**
 * \brief
 * \param effectOption
 * \since version 1.0.5
 */
- (void)setClipEffect:(NSString *)effectId effectOption:(NXEEffectOptions *)effectOption;

/**
 * \brief This method sets a transition effect, that is not part of any existing theme, on a clip.
 * \param effectId The transition effect ID as a NSString.
 * \since version 1.0.5
 */
- (void)setTransitionEffect:(NSString*)effectId;

/**
 * \brief
 * \param effectOption
 * \since version 1.0.5
 */
- (void)setTransitionEffect:(NSString *)effectId effectOption:(NXEEffectOptions *)effectOption;

/**
 * \brief This method guides for the best duration of a transition effect, when applied to the clips in a project.
 * \param project the project added in NEEngine.
 * \param clipIndex The index of the clips on the primary track of the project.
 * \since version 1.0.5
 */
- (void)ensureTransitionFit:(NXEProject *)project wantedClipIndex:(int)index;

/**
 * \brief This method sets the duration of a transition effect to set. 
 * \param duration The duration to set.
 * \since version 1.0.5
 */
- (void)setTransitionEffectDuration:(int)duration;

/**
 * \brief This method clears previous trim history and starts a new trim.
 * \param startTime The start time of the video clip in \c msec (milliseconds).
 * \param endTime The end time of the video clip in \c msec (milliseconds).
 * \since version 1.0.5
 */
- (void)setTrim:(int)start EndTrimTime:(int)end;

/**
 * \brief Audio Clip에 대한 start / endTime을 설정할 수 있다.
 * \param startTime The start time of the audio clip in \c msec (milliseconds).
 * \param endTime The end time of the audio clip in \c msec (milliseconds).
 */
- (void)setAudioStartTime:(int)startTime EndTime:(int)endTime;

/**
 * \brief This method sets the playback speed of a video clip. The value set by this method will be ignored if the video clip was trimmed at any time.
 * \param speed The new playback speed of the video clip as a percentage of the original speed. <br>
 *              ( 0% (slow) ~ 100%(standard) ~ 400%(fast)
 * \since version 1.0.5
 *
 */
- (void)setSpeed:(int)speed;

/**
 * \brief This method sets the start time and end time of a clip effect.
 * The clip effect's duration will automatically be adjusted to the clip's duration if the effect duration set with this method is longer than the clip's duration.
 * \param starttime The start time of the clip effect, in \c msec (milliseconds).
 * \param endtime The end time of the clip effect, in \c msec (milliseconds).
 * \since version 1.0.5
 */
- (void)setEffectShowTime:(int)startTime endTime:(int)endTime;

/**
 * \brief This method gets the start and end time from a clip effect.
 * \param starttime The start time of the clip effect, in \c msec (milliseconds).
 * \param endtime The end time of the clip effect, in \c msec (milliseconds).
 * \since version 1.0.5
 */
- (void)getEffectShowTime:(int *)startTime endTime:(int *)endTime;

/**
 * \brief This method designates one of the NXEPresetColorEffect as a color effect.
 * \param persetName The presetName enumeration value to designate.
 * \see NXEPresetColorEffect
 */
- (void)setColorEffectValues:(NXEPresetColorEffect)presetName;

/**
 * \brief This method randomizes the start and end crops of a clip, based on the chosen crop mode.
 * By default, the NexEditorSDK randomly selects the start and end positions of crops on clips added, as if the cropMode parameter is set to PAN_RAND.
 * \see cropMode in NXEVisualEdit.h
 * \since version 1.0.5
 */
- (void)setCropMode:(CropMode)cropMode;

/**
 * \brief This sets the image duration of a clip.
 * \param time The image duration to set.
 * \since version 1.0.5
 */
- (void)setImageClipDuration:(int)time;

/**
 * \brief This gets the brightness set to a clip.
 * \since version 1.0.5
 */
- (int)getCombinedBrightness;

/**
 * \brief This gets the contrast set to a clip.
 * \since version 1.0.5
 */
- (int)getCombinedContrast;

/**
 * \brief This gets the saturation set to a clip.
 * \since version 1.0.5
 */
- (int)getCombinedSaturation;

/**
 * \brief This gets the duration of a clip.
 */
- (int)getRepresentedDuration;

/**
 * \brief This gets the duration of a clip.
 */
- (int)getTotalTime;

/**
 * \brief This gets the duration of a clip.
 * \param totalTime / unit milliseconds
 */
- (void)setTotalTime:(int)totalTime;

/**
 * \brief This gets the duration of a clip.
 * \param time / unit milliseconds
 * \parma level / range 0 ~ 200
 */
- (int)addVolumeEnvelop:(int)time level:(int)level;

/**
 * \brief This method creates a clip as an instance of NXEClip and gets the media information about that clip.
 * Therefore, it may take a while to create a particular NXEClip, depending on the size and format of the file.
 * \param path The location where the media clip is saved.
 * \param error The result after creating a clip. If there is an error, it notifies the type of error.
 * \see getSupportedClip:(NSString)path
 * \since version 1.0.5
 *
 */
- (instancetype)initWithPath:(NSString *)path error:(int *)error;

/**
 * \brief This method only creates a media clip that is supported by the NexEditorSDK.
 * \param path The location where the media clip is saved.
 * \param error The result after creating a clip. If there is an error, it notifies the type of error.
 * \return A NXEClip instance if the media clip is supported by the NexEditorSDK.
 * \since version 1.0.5
 */
+ (instancetype)newSupportedClip:(NSString *)path error:(int *)error;

/**
 * \brief This method only creates a media clip that is supported by the NexEditorSDK.
 * \param path The location where the media clip is saved.
 * \param useCache Set to TRUE to cache retrieved media information; otherwise set to FALSE.
 *                 This method allows to decide whether to cache retrieved clip's media information.
 *                 Once a clip's media information is cached, this method will operate faster when the same clip is called again.
 *                 It is recommended to set the parameter useCache to FALSE when a clip is for temporary use
 *                 (for example, when a panorama image is made with number of trailing images, these images will be deleted once a new panorama image is made).
 * \return A NXEClip instance if the media clip is supported by the NexEditorSDK.
 * \since version 1.0.5
 */
+ (instancetype)newSolidClip:(NSString *)color;

/**
 * \brief This method gets the thumbnails from I-frames within the specific section of the clip.
 * This method does not cache retrieved thumbnails, therefore the developer has to cache it separately if needed.
 * \note The bitmaps of <tt>onGetDetailThumbnailResult()</tt> returned from the listener operates in singleton,
 * therefore it should be saved separately to avoid overwrite with other bitmaps.
 * If both <tt>startTime</tt> and <tt>endTime</tt> value is set to 0, only I-frames will be retrieved.
 * \param thumbWidth    The width resolution of the thumbnail as integer.
 * \param thumbHeight   The height resolution of the thumbnail as integer.
 * \param startTime     The start time of the specific section of the clip to retrieve thumbnails.
 * \param endTime       The end time of the specific section of the clip to retrieve thumbnails.
 * \param maxCount      Maximum number of thumbnails to get.
 * \param rotate        The rotation angle of the thumbnail as integer ( 0 , 90 , 180 , 270 ).
 * \param uiCallback The instance of ThumbnailUICallbackBlock.
 * \return -1 if there is no listener or the clip is not a video clip; otherwise 0.
 *
 * \since version 1.0.5
 */
- (void)getVideoClipThumbnails:(int)thumbWidth
               ThumbnailHeight:(int)thumbHeight
                     StartTime:(int)startTime
                       EndTime:(int)endTime
                      MaxCount:(int)maxCount
                        Rotate:(int)rotate
                       NoCache:(int)noCache
      ThumbnailUICallbackBlock:(NXEThumbnailUICallbackBlock*)uiCallback;

/**
 * \brief 클립에 LUT효과를 반영해준다.
 * \param lutTypes NXELutTypes
 */
- (void)setLut:(NXELutTypes)lutTypes;

/**
 * \brief This method finds out the number of I-frames of the media content.
 * \return The number of IFrames.
 * \see getSeekPointInterval
 * \since version 1.0.5
 */
- (int)getSeekPointCount;

/**
 * \brief This method finds out the average interval of IFrames of the media content.
 * \return The average interval of I-frames in <tt>msec</tt> (milliseconds).
 * \see getSeekPointCount
 * \since version 1.0.5;
 */
- (int)getSeekPointInterval;

@end