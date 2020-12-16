/******************************************************************************
 * File Name   :  NXEAudioEnvelop.h
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
#import "NXEEditorType.h"

@class NXEClip;

/** \class NXEAudioEnvelop
 *  \brief NXEAudioEnvelop class defines all properties and methods to manage an audio envelope
 */
@interface NXEAudioEnvelop : NSObject

@property (nonatomic, retain) NSMutableArray *volumeEnvelopeTime;
@property (nonatomic, retain) NSMutableArray *volumeEnvelopeLevel;

/** \brief Initializes a new audio envelope object with the speficied audio clip.
 *  \param clip An audio clip.
 *  \return An NXEAudioEnvelop object
 *  \since version 1.0.5
 */
- (id)initWithClip:(NXEClip*)clip;

/** \brief Updates the start and end trim time of an audio envelope.
 *  \param startTime The start trim time of an audio envelope.
 *  \param endTime The end trim time of an audio envelope.
 *  \since version 1.0.19
 */
- (void)updateTrimTime:(int)startTime end:(int)endTime;

/** \brief Adds a new volume envelope to the volume envelope list with the speficied time and level values.
 *  \note Volume envelopes are automatically listed by time.
 *  \param time The time where a new volume envelope is added.
 *  \param level The audio volume level of a new volume envelope, ranging from 0 through 200.
 *  \return An index of the newly added volume envlope.
 *  \since version 1.0.5
 */
- (int)addVolumeEnvelop:(int)time level:(int)level;

/** \brief Removes all volume envelopes in the volume envelop list.
 *  \since version 1.0.19
 */
- (void)removeVolumeEnvelop;

/** \brief Removes a volume envelope with the speficied index.
 *  \param index An index of a volume envelope to remove
 *  \since version 1.0.19
 */
- (void)removeVolumeEnvelop:(int)index;

/** \brief Returns a list of volume envelope time.
 *  \return An arrary of volume envelope time.
 *  \since version 1.0.5
 */
- (NSArray*)getVolumeEnvelopeTimeList;

/** \brief Returns a list of volume envelope level.
 *  \return An arrary of volume envelope level.
 *  \since version 1.0.5
 */
- (NSArray*)getVolumeEnvelopeLevelList;

@end
