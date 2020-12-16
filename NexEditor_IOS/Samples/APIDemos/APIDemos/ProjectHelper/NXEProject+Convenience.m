/******************************************************************************
 * File Name   :	NXEProject+Convenience.h
 * Description : Extends NXEProject with audio/visual clip add/removal methods.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Property
 Copyright (C) 2005~2017 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "NXEProject+Convenience.h"

@implementation NXEProject (Convenience)

- (void) removeAudioClips
{
    self.audioClips = @[];
}

- (void) addAudioClip:(NXEClip *)audioClip
{
    self.audioClips = [self.audioClips arrayByAddingObject:audioClip];
}

- (void) removeVisualClips
{
    self.visualClips = @[];
}

- (void) addVisualClip:(NXEClip *)visualClip
{
    self.visualClips = [self.visualClips arrayByAddingObject:visualClip];
}

@end
