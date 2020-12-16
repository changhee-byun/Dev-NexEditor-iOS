/*
 * File Name   : NexEditorPrivate.h
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

#import "NexEditor.h"
#import "NEXVIDEOEDITOR_EventTypes.h"
#import "EditorEventListenerPrivate.h"
#import "EditorEventListener+EventDelegate.h"

@interface NexEditor(Private)
// Deep internal
@property (nonatomic, readonly) BOOL isAppActive;
@property (nonatomic, readonly) BOOL isVideoEditorHandleValid;

// Events
@property (nonatomic, readonly) EditorEventListener *editorEventListener;

- (BOOL) didReceiveEditorEvent:(NexEditorEvent) event;
- (void) addEventWaiterFor:(VIDEOEDITOR_EVENT) eventType onQueue:(dispatch_queue_t) queue block:(NexEditorOnEventBlock) block;
- (void) addEventWaiterFor:(VIDEOEDITOR_EVENT) eventType block:(NexEditorOnEventBlock) block;

// Clip Loading
- (NexEditorError)_loadVisualClips:(NSArray*)visualClips
                        audioClips:(NSArray*)audioClips
                  setupEventWaiter:(void(^)(NexEditor *editor)) setupEventWaiter;

// INexVideoEditor invocation
- (NexEditorError) videoEditorHandleSetTime:(unsigned int) uiTime display:(int) iDisplay IDRFrame:(int) iIDRFrame;
- (NexEditorError) videoEditorHandleStopPlayWithFlag:(int) iFlag;

// Fast Preview
- (NexEditorError) fastPreviewWithString:(NSString*)option display:(BOOL)display;

@end

