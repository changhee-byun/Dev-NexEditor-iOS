/******************************************************************************
 * File Name   : UIListenerProtocol.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

#import "NXEEditorType.h"
#import "NXEEditorErrorCode.h"

@protocol UIListenerProtocol <NSObject>

@optional

- (void)onStateChange:(NXEPLAYSTATE)oldState newState:(NXEPLAYSTATE)newState;
- (void)onTimeChange:(int)currentTime;
- (void)onSetTimeDone:(int)currentTime;
- (void)onSetTimeFail:(ERRORCODE)error;
- (void)onSetTimeIgnored;
- (void)onEncodingDone:(ERRORCODE)error;
- (void)onEncodingProgress:(int)percent;
- (void)onPlayEnd;
- (void)onPlayFail:(ERRORCODE)error clipId:(int)clipId;
- (void)onPlayStart;
- (void)onClipInfoDone;
- (void)onSeekStateChanged:(BOOL)isSeeking;
- (void)onStopComplete:(ERRORCODE)errorCode;
- (void)onPrepared;

@end