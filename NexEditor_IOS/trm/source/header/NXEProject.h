/******************************************************************************
 * File Name   :	NXEProject.h
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
#import "NXELayer.h"

@class NXEClip;

typedef float NXEAudioVolumeScale;

/** \class NXEMutableProjectOptions
 *  \brief This class represents project options such as audio volume and fade in and out.
 *  \since version 1.0.19
 */
@interface NXEMutableProjectOptions : NSObject <NSCopying>

/** \brief The project overall audio volume. Once set, volume to be setted for Clip or BGM is ignored.
 *  Volume scale: 0.0 ~ 1.0 (default value is 0.5)
 *  \since version 1.0.19
 */
@property (nonatomic, assign) NXEAudioVolumeScale audioVolume;

/** \brief The fade-in time for the audio at the beginning of the project.
 *  The unit is milliseconds. (default value is 200 ms)
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int audioFadeInDuration;

/** \brief The fade-out time for the audio at the ending of the project.
 *  The unit is milliseconds. (default value is 5000 ms)
 *  \since version 1.0.19
 */
@property (nonatomic, assign) int audioFadeOutDuration;

@end


/** \brief Make NXEColorAdjustments instance with color adjustment values which is used for setting project color adjustments.
 *   scale: -1.0 ~ 1.0
 *  \since version 1.3.4
 */
NXEColorAdjustments NXEColorAdjustmentsMake(float brightness, float contrast, float saturation);

/** \class NXEProject
 *  \brief NXEProject class defines all properties and methods to manage a project.
 *
 *  To edit with the NexEditor&trade;SDK, a project is created, and then clips(video, image, or audio) are added to the project and further edited with the options available.<p>
 *  A finished project is then exported to become the final version of a NexEditor&trade;SDK; video.
 * \since version 1.0.5 
 */
@interface NXEProject : NSObject <NSCopying>

/** 
 * \brief Array of NXEClip, the instance of a video and image clip, among other clips added to a project.
 *  
 *        To add clips, create a NSArray of NXEClip instances and assign to this property. To remove a clip, read this property, create a new NSArray with only clips to remain and assign to this property.
 *
 * \since version 1.2
 */
@property (nonatomic, copy) NSArray<NXEClip *> *visualClips;

/**
 * \brief NXEColorAdjustments instance which contains color adjustment values such as brightness, contrast and saturation.
 * The color adjustment values set by this property will be applied to whole clips and layers in the project.
 * \since version 1.3.4
 */
@property (nonatomic) NXEColorAdjustments colorAdjustments;

/** 
 * \brief Array of NXEClip, the instance of an audio clip, among other clips added to a project.
 *
 *        To add clips, create a NSArray of NXEClip instances and assign to this property. To remove a clip, read this property, create a new NSArray with only clips to remain and assign to this property.
 *
 * \since version 1.2
 */
@property (nonatomic, copy) NSArray<NXEClip *> *audioClips;

/**
 * \brief Array of NXELayer, the instance image, video and text layers added to a project.
 * \note Before adding layers through this property, make sure the project is set to NXEEngine by calling \c setProject:.
 * \since version 1.2
 */
@property (nonatomic, copy) NSArray<NXELayer *> *layers;

/** \brief Sets a new background musich with an audio clip, NXEClip instance.
 *  \since version 1.2
 */
@property (nonatomic, strong) NXEClip *bgmClip;

/** \brief NXEMutableProjectOptions instance 
 *  \since version 1.0.19
 */
@property (nonatomic, readonly) NXEMutableProjectOptions *options;

/** \brief Returns the total playback time of a project.
 *  \return The total playback time.
 *  \since version 1.0.5
 */
- (int)getTotalTime;

/** \brief Updates any changes the current project, such to clip addition/deletion or clip property changes.
 *  \note Frequent calls to this method may result in performance degradation. Better to call it after any clip update operations is completed.
 *  \since version 1.0.5
 */
- (void)updateProject;

/** \brief Sets a new BGM and applies volume scale to it with the speficied audio clip and volume scale.
 *  \note Renamed from -setBGM2Clip:volumeScale:
 *  \param audioClip The path of the audio clip.
 *  \param volumeScale The volume scale of the audio clip, ranging from 0.0 through 1.0, 0.5 by default.
 *  \since version 1.2
 */
- (void)setBGMClip:(NXEClip *)audioClip volumeScale:(float)volumeScale;

/// Deprecated. use visualClips
@property (nonatomic, readonly) NSArray<NXEClip *> *visualItems;

/// Deprecated. use audioClips
@property (nonatomic, readonly) NSArray<NXEClip *> *audioItems;

@end
