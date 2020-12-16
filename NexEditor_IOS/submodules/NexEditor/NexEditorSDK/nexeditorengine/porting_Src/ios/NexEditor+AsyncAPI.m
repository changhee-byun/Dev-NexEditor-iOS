/*
 * File Name   : NexEditor+AsyncAPI.m
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

#import "NexEditor+AsyncAPI.h"
#import "NexEditorPrivate.h"
#import "NEXVIDEOEDITOR_EventTypes.h"
#import "NEXVIDEOEDITOR_Error.h"
#import "EditorEventListenerPrivate.h"
#import "NexSAL_Internal.h"
#import "NexEditorLog.h"
#import "NexTheme_Errors.h"

#import "FastPreviewPrivate.h"

#define LOG_TAG @"NexEditor+Async"

@implementation NexEditor(AsyncAPI)
- (NexEditorError) loadVisualClips:(NSArray <NXEVisualClip *> *) visualClips
                        audioClips:(NSArray <NXEAudioClip *> *) audioClips
                          complete:(NexEditorResultBlock) complete
{
    // event:  VIDEOEDITOR_EVENT_ASYNC_LOADLIST,
    // param1: <result> NEXVIDEOEDITOR_ERROR_NONE
    void (^setupEventWaiter)(NexEditor *editor) = NULL;
    
    if (complete) {
        setupEventWaiter = ^(NexEditor *editor) {
            [editor addEventWaiterFor:VIDEOEDITOR_EVENT_ASYNC_LOADLIST
                                block:^(NexEditorEvent event) {
                                    complete(event.params[0]);
                                    return YES;
                                }];
        };
    }
    return [self _loadVisualClips:visualClips audioClips:audioClips setupEventWaiter:setupEventWaiter];
}

#pragma mark - setTime
- (NexEditorError) setTime:(CMTime) time
                   options:(NexEditorSetTimeOptions)options
                  complete:(NexEditorResultBlock) complete
{
    // event:  VIDEOEDITOR_EVENT_SETTIME_DONE,
    // param1: <result> NEXVIDEOEDITOR_ERROR_INVALID_STATE,
    // param2: ignored(0|1)
    // param3: newTime
    // param4: oldTime
    if (!self.isVideoEditorHandleValid) {
        return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
    }
    
    if (!self.isAppActive) {
        NexLogW(LOG_TAG, @"setTime discarded due to app state: inactive");
        NEXVIDEOEDITOR_ERROR result = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
        if (complete) {
            complete(result);
        }
        [self.editorEventListener notifyEvent:VIDEOEDITOR_EVENT_SETTIME_DONE param1:result param2:1 param3:0 param4:0];
        return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
    }
    
    if (complete) {
        [self addEventWaiterFor:VIDEOEDITOR_EVENT_SETTIME_DONE
                          block:^(NexEditorEvent event) {
                              complete(event.params[0]);
                              return YES;
                          }];
    }
    
    unsigned int ms = (unsigned int) (CMTimeGetSeconds(time) * 1000);
    int display = (options & NexEditorSetTimeOptionsDisplay) ? 1 : 0;
    int idrFrame = (options & NexEditorSetTimeOptionsIDRFrameOnly) ? 1 : 0;
    NexEditorError result = [self videoEditorHandleSetTime:ms display:display IDRFrame:idrFrame];
    if ( result != NEXVIDEOEDITOR_ERROR_NONE) {
        NexLogW(LOG_TAG, @"%@ - failed with error:%d", NSStringFromSelector(_cmd), result);
    }
    return result;
}

#pragma mark - stop
- (NexEditorError) stopWithComplete:(NexEditorResultBlock) complete
{
    // event:  VIDEOEDITOR_EVENT_STATE_CHANGE,
    // param1: <previous state> i.e. PLAY_STATE_PLAY,
    // param2: <new state> i.e. PLAY_STATE_IDLE
    return [self stopOnQueue:dispatch_get_main_queue() complete:complete];
}

- (NexEditorError) stopOnQueue:(dispatch_queue_t) queue complete:(NexEditorResultBlock) complete
{
    NexEditorError result = NEXVIDEOEDITOR_ERROR_NONE;
    
    if (!self.isVideoEditorHandleValid) {
        result = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
        // Don't return and call the complete block back with the result at the end
    }
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_FLOW, 0, "[NexEditor.mm %d] %@", __LINE__, NSStringFromSelector(_cmd));
    if (queue == NULL) {
        queue = dispatch_get_main_queue();
    }
    
    if (result == NEXVIDEOEDITOR_ERROR_NONE) {
        if (complete) {
            [self addEventWaiterFor:VIDEOEDITOR_EVENT_STATE_CHANGE
                            onQueue:queue
                              block:^(NexEditorEvent event){
                                  complete(NEXVIDEOEDITOR_ERROR_NONE);
                                  if (event.params[1] != PLAY_STATE_IDLE) {
                                      NexLogW(LOG_TAG, @"Caught a wrong state for stop: %d (!= expected:%d) from:%d", event.params[1], PLAY_STATE_IDLE, event.params[0]);
                                  }
                                  return YES;
                              }];
        }
        result = [self videoEditorHandleStopPlayWithFlag:0];
    }
    
    if (result != NEXVIDEOEDITOR_ERROR_NONE && complete) {
        dispatch_async(queue, ^{
            complete(result);
        });
    }
    return result;
}

- (NexEditorError) runFastPreviewCommand:(id<FastPreviewCommand>) command
                                 display:(BOOL)display
                                complete:(NexEditorResultBlock) complete
{
    // event:  VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE
    // param1: NXT_Error
    static NSDictionary *mapNXT2VideoEditorError = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        mapNXT2VideoEditorError = @{
                                    @(NXT_Error_None) : @(NEXVIDEOEDITOR_ERROR_NONE),
                                    @(NXT_Error_MissingParam): @(NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED),
                                    @(NXT_Error_InvalidState): @(NEXVIDEOEDITOR_ERROR_INVALID_STATE),
                                    @(NXT_Error_NoContext): @(NEXVIDEOEDITOR_ERROR_FASTPREVIEW_VIDEO_RENDERER_ERROR),
                                    };
		[mapNXT2VideoEditorError retain];
    });
    
    if (complete) {
        [self addEventWaiterFor:VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE
                          block:^(NexEditorEvent event){
                              NEXVIDEOEDITOR_ERROR result = event.params[0];
                              if ( result != NEXVIDEOEDITOR_ERROR_NONE ) {
                                  NSNumber *entry = mapNXT2VideoEditorError[@(result)];
                                  if (entry) {
                                      result = (NEXVIDEOEDITOR_ERROR) entry.intValue;
                                  } else {
                                      result = NEXVIDEOEDITOR_ERROR_UNKNOWN;
                                  }
                              }
                              complete(result);
                              return YES;
                          }];
    }
    return [self fastPreviewWithString:command.stringRepresentation display:display];
}
@end
