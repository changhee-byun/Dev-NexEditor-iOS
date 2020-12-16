/******************************************************************************
 * File Name   :	NXEAudioEdit.h
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
#import "NXEEditorType.h"

@class NXEClip;

/**
 * \brief A class that edits voice changer, audio envelop, and other audio data.
 * \code
 *  @interface Sample : NSObject
 *  @property(nonatomic, retain) NXEProject *project;
 *  @property(nonatomic, retain) NXEEngine *engine;
 *  @end
 *
 *  @implementation Sample
 *  - (void)sample
 *  {
 *      self.engine = [NXEEngine sharedInstance];
 *      self.project = [[NXEProject alloc] init];
 *
 *      NSString *filePath = @"";
 *      int errorType;
 *      NXECLip *clip = [NXECLip newSupportedClip:filePath), &errorType];
 *      if(clip == nil) {
 *          NSLog(@"errorType=%d", errorType);
 *          return;
 *      }
 *      [project addClip:clip];
 *      [project updateProject];
 *      [engine setProject:project];
 *
 *      //  adds a volume envelope by setting the time and the volume values.
 *      //
 *      NXEClip *bgmClip = (NXEClip*)[project.audioItems objectAtIndex:0];
 *      [bgmClip addVolumeEnvelop:0 level:100];
 *      [bgmClip addVolumeEnvelop:[project getTotalTime] - 10000 level:100];
 *      [bgmClip addVolumeEnvelop:[project getTotalTime] - 5000 level:50];
 *      [bgmClip addVolumeEnvelop:[project getTotalTime] - 1000 level:0];
 *  }
 *  @end
 * \endcode
 * \since version 1.0.5
 *
 */
@interface NXEAudioEdit : NSObject

/**
 * \brief A method that initializes NXEAudioEdit.
 */
- (id)init;

/**
 * \brief A method that initializes NXEAudioEdit.
 * \param clip The clip for initialization.
 */
- (id)initWithClip:(NXEClip*)clip;

/**
 * \brief This method adds a volume envelope by setting the time and the volume values.
 * The added volume envelopes are automatically listed by time.
 * \param time The time to add the volume envelop.
 * \param level The audio volume to set to the envelop.
 * \return Index of the volume envelop list.
 * \since version 1.0.5
 */
- (int)addVolumeEnvelop:(int)time level:(int)level;

/**
 * \brief This method gets the time list of the envelopes set to an audio item.
 * \return The time list of envelops, VolumeEnvelopeTimeList(int[]).
 * \since version 1.0.5
 */
- (NSArray*)getVolumeEnvelopeTimeList;

/**
 * \brief This method clears the time list of the envelopes set to an audio item.
 * \since version 1.0.5
 */
- (void)clearVolumeEnvelopeTimeList;

/**
 * \brief This method gets the volume level list of the envelopes set to an audio item.
 * \return The volume level list of the envelops, VolumeEnvelopeLevelList(int[]).
 * \since version 1.0.5
 */
- (NSArray*)getVolumeEnvelopeLevelList;

/**
 * \brief This method clears the volume level list of the envelopes set to an audio item.
 * \since version 1.0.5
 */
- (void)clearVolumeEnvelopeLevelList;

@end