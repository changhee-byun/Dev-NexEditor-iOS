/**
 * File Name   : NXEClip+Extensions.h
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

@import NexEditorFramework;

@interface NXEClip (ClipSources)

+ (NSArray<NXEClip *> *) clipsWithSources: (NSArray<NXEClipSource *> *) clipSources error:(NSError **) error;

@end

@interface NXEClip (DisplayName)
@property (nonatomic, readonly) NSString *displayName;
@end
