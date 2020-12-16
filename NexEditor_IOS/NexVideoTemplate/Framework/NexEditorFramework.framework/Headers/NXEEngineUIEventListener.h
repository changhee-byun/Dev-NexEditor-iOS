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

@interface NXEEngineUIEventListener : NSObject <UIListenerProtocol>

/**
 * \brief
 * \param onStageChange
 */
- (void)registerCallback4General_onStateChange:(void(^)(NXEPLAYSTATE oldState, NXEPLAYSTATE newState))onStateChange;

/**
 * \brief
 * \param onPlayProgressCallback
 * \param onPlayStartCallback
 * \param onPlayEndCallback
 */
- (void)registerCallback4Play_onPlayProgress:(void(^)(int currentTime))onPlayProgressCallback
                                 onPlayStart:(void(^)())onPlayStartCallback
                                   onPlayEnd:(void(^)())onPlayEndCallback;

/**
 * \brief
 * \param onEncodingProgressCallback
 * \param onEncodingDoneCallback
 */
- (void)registerCallback4Encoding_onEncodingProgress:(void(^)(int percent))onEncodingProgressCallback
                                       onEncodingDone:(void(^)())onEncodingDoneCallback;

/**
 * \brief
 * \param onStopCompleteCallback
 */
- (void)registerCallback4Stop_onStopComplete:(void(^)())onStopCompleteCallback;

/**
 * \brief
 * \param onSetTimeDone
 */
- (void)registerCallback4SetTime_onSetTimeDone:(void(^)(int doneTime))onSetTimeDone;

@end
