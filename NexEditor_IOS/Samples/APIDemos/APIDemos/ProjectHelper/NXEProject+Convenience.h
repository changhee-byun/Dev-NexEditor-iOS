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

@import NexEditorFramework;

@interface NXEProject (Convenience)
- (void) removeAudioClips;
- (void) addAudioClip:(NXEClip *)audioClip;
- (void) removeVisualClips;
- (void) addVisualClip:(NXEClip *)visualClip;
@end
