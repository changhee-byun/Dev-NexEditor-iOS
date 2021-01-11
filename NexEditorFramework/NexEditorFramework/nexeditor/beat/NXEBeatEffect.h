/**
 * File Name   : NXEBeatEffect.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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

#import "NXEBeatEffectDefault.h"

@interface NXEBeatEffect: NXEBeatEffectDefault

- (instancetype) initWithSource:(NSDictionary *)source;

@property (nonatomic, readonly) int time;
@property (nonatomic, readonly) int effectInDuration;
@property (nonatomic, readonly) int effectOutDuration;
@property (nonatomic, readonly) BOOL sourceChange;
@property (nonatomic, readonly) NSString *internalClipId;
@property (nonatomic, readonly) int priority;
@end

