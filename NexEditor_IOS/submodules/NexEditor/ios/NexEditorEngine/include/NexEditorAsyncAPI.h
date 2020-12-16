/*
 * File Name   : NexEditorAsyncAPI.h
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
#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>
#import "NexEditorTypes.h"
#import "NXEVisualClip.h"
#import "NXEAudioClip.h"
#import "FastPreview.h"

typedef void (^NexEditorResultBlock)(NexEditorError result);

@protocol NexEditorAsyncAPI <NSObject>
/// Load new clips asynchronously and invokes 'complete' on the main queue on completeion
- (NexEditorError) loadVisualClips:(NSArray <NXEVisualClip *> *) visualClips
                        audioClips:(NSArray <NXEAudioClip *> *) audioClips
                          complete:(NexEditorResultBlock) complete;

/// Seeks asynchronously to the specified time and invokes 'complete' on the main queue on completeion
- (NexEditorError) setTime:(CMTime) time
                   options:(NexEditorSetTimeOptions)options
                  complete:(NexEditorResultBlock) complete;

/// Stops playback/export asynchronously and invokes 'complete' on the main queue on completeion
- (NexEditorError) stopWithComplete:(NexEditorResultBlock) complete;

/// Stops playback/export asynchronously and invokes 'complete' on the specified queue on completeion
- (NexEditorError) stopOnQueue:(dispatch_queue_t) queue complete:(NexEditorResultBlock) complete;

/// Runs fast preview with 'command' and invokes 'complete' on the main queue on completion
- (NexEditorError) runFastPreviewCommand:(id<FastPreviewCommand>) command
                                 display:(BOOL)display
                                complete:(NexEditorResultBlock) complete;
@end

