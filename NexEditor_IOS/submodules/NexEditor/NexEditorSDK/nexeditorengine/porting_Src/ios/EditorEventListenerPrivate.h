/*
 * File Name   : EditorEventListenerPrivate.h
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

#ifndef EditorEventListenerPrivate_h
#define EditorEventListenerPrivate_h
#import "EditorEventListener.h"
#import "NEXVIDEOEDITOR_EventTypes.h"
#import "NEXVIDEOEDITOR_Types.h"

typedef struct {
    VIDEOEDITOR_EVENT type;
    int32_t params[4];
} NexEditorEvent;

typedef void (^NexEditorOnStateChangeBlock)(PLAY_STATE preState, PLAY_STATE newState);
typedef BOOL (^NexEditorOnEventBlock)(NexEditorEvent event);

@interface EditorEventListener(EventHook)
@property (nonatomic, copy) NexEditorOnEventBlock onEvent;
@end

#endif /* EditorEventListenerPrivate_h */
