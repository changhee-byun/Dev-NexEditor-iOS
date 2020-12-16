/**
 * File Name   : NXEProjectInternal.h
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

#import "NXEProject.h"

@class NexSceneInfo;
@class NexDrawInfo;

@interface AudioStartTimeScale : NSObject

@property (nonatomic, readonly) NSUInteger visualClipIndex;
@property (nonatomic, readonly) NSUInteger audioClipIndex;
@property (nonatomic, readonly) NSUInteger startTimeScale;

- (instancetype)initWithVisualClipIndex:(NSUInteger)visualClipIndex audioClipIndex:(NSUInteger)audioClipIndex startTimeScale:(NSUInteger)startTimeScale;

@end

@interface VisualClipEffectTimeGroup : NSObject

@property (nonatomic, readonly) NSUInteger startClipIndex;
@property (nonatomic, readonly) NSUInteger count;

- (instancetype)initWithStartClipIndex:(NSUInteger) index count:(NSUInteger) count;

@end

@interface NXEProject (Internal)

@property (nonatomic, retain) NSArray<AudioStartTimeScale *> *adjustedAudioStartTimeScales;
@property (nonatomic, retain) NSArray<VisualClipEffectTimeGroup *> *visualClipEffectTimeGroups;
@property (nonatomic, strong) NSMutableDictionary *hiddenStore;
@property (nonatomic, readonly) NSArray<NexSceneInfo *> *sceneList;
@property (nonatomic, readonly) NSArray<NexDrawInfo *> *slotList;

@end
