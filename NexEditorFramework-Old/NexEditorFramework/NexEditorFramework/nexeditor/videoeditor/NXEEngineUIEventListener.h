/******************************************************************************
 * File Name   :	NXEEngineUIEventListener.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "UIListenerProtocol.h"
#import "NXEError.h"

@interface NXEEngineUIEventListener : NSObject <UIListenerProtocol>

- (void)registerOnStageChangeCb:(void(^)(NXEPLAYSTATE oldState, NXEPLAYSTATE newState))onStageChangeCb;
- (void)registerOnStopCompleteCb:(void(^)())onStopCompleteCb;
- (void)registerOnSetTimeDoneCb:(void(^)(int doneTime))onSetTimeDoneCb;
- (void)registerOnPreparedCb:(void(^)())onPreparedCb;
- (void)registerOnPlayProgressCb:(void(^)(int currentTime))onPlayProgressCallback
                   onPlayStartCb:(void(^)())onPlayStartCallback
                     onPlayEndCb:(void(^)())onPlayEndCallback;
- (void)registerOnEncodingProgressCb:(void(^)(int percent))onEncodingProgressCb
                    onEncodingDoneCb:(void(^)(NXEError *error))onEncodingDoneCb;

@end
