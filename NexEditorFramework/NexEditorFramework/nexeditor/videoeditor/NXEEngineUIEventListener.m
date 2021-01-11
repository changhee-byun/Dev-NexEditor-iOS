/******************************************************************************
 * File Name   : NexEngineUIEventListener.m
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
#import "NXEEngineUIEventListener.h"
#import <NexEditorEngine/NexEditor.h>

@interface NXEEngineUIEventListener ()

@property (copy, nonatomic) void(^onTimeChangeCb)(int currentTime);
@property (copy, nonatomic) void(^onPlayEndCb)();
@property (copy, nonatomic) void(^onPlayStartCb)();
@property (copy, nonatomic) void(^onEncodingProgressCb)(int percent);
@property (copy, nonatomic) void(^onEncodingDoneCb)(NXEError *error);
@property (copy, nonatomic) void(^onStateChangeCb)(NXEPLAYSTATE oldState, NXEPLAYSTATE newState);
@property (copy, nonatomic) void(^onStopCompleteCb)(ERRORCODE errorCode);
@property (copy, nonatomic) void(^onSetTimeDoneCb)(int doneTime);
@property (copy, nonatomic) void(^onPreparedCallbackCb)();

@end

@implementation NXEEngineUIEventListener

- (void) dealloc
{
    self.onTimeChangeCb = nil;
    self.onPlayStartCb = nil;
    self.onPlayEndCb = nil;
    self.onSetTimeDoneCb = nil;
    self.onEncodingProgressCb = nil;
    self.onEncodingDoneCb = nil;
    [super dealloc];
}

- (void)registerOnStageChangeCb:(void(^)(NXEPLAYSTATE oldState, NXEPLAYSTATE newState))onStateChangeCb
{
    self.onStateChangeCb = onStateChangeCb;
}

- (void)registerOnPlayProgressCb:(void(^)(int currentTime))onPlayProgressCb
                          onPlayStartCb:(void(^)())onPlayStartCb
                            onPlayEndCb:(void(^)())onPlayEndCb
{
    self.onTimeChangeCb = onPlayProgressCb;
    self.onPlayStartCb = onPlayStartCb;
    self.onPlayEndCb = onPlayEndCb;
}

- (void)registerOnEncodingProgressCb:(void(^)(int percent))onEncodingProgressCb
                            onEncodingDoneCb:(void(^)(NXEError *error))onEncodingDoneCb
{
    self.onEncodingProgressCb = onEncodingProgressCb;
    self.onEncodingDoneCb = onEncodingDoneCb;
}

- (void)registerOnStopCompleteCb:(void(^)())onStopCompleteCb
{
    self.onStopCompleteCb = onStopCompleteCb;
}

- (void)registerOnSetTimeDoneCb:(void(^)(int doneTime))onSetTimeDoneCb
{
    self.onSetTimeDoneCb = onSetTimeDoneCb;
}

- (void)registerOnPreparedCb:(void(^)())onPreparedCb
{
    self.onPreparedCallbackCb = onPreparedCb;
}

#pragma mark - UIListenerProtocol

- (void)onStateChange:(NXEPLAYSTATE)oldState newState:(NXEPLAYSTATE)newState
{
    if(self.onStateChangeCb == nil) {
        return;
    }
    
    self.onStateChangeCb(oldState, newState);
}

- (void)onTimeChange:(int)currentTime
{
    if(self.onTimeChangeCb == nil) {
        return;
    }
    
    self.onTimeChangeCb(currentTime);
}

- (void)onSetTimeDone:(int)doneTime
{
    if(self.onSetTimeDoneCb == nil) {
        return;
    }
    
    self.onSetTimeDoneCb(doneTime);
}

- (void)onSetTimeFail:(ERRORCODE)error
{
    
}

- (void)onSetTimeIgnored
{
    
}

- (void)onEncodingDone:(ERRORCODE)error
{
    if(self.onEncodingDoneCb == nil) {
        return;
    }
    if(error != ERROR_NONE) {
        NXEError *nxeError = [[NXEError alloc] initWithErrorCode:error];
        self.onEncodingDoneCb(nxeError);
    } else {
        self.onEncodingDoneCb(nil);
    }
}

- (void)onEncodingProgress:(int)percent
{
    if(self.onEncodingProgressCb == nil) {
        return;
    }
    
    int p = 0;
    int duration = [[NexEditor sharedInstance] getDuration];
    int currentTime = percent;
    
    if(duration <= 0) {
        p = currentTime / 10;
    }
    else {
        p = currentTime * 100 / duration;
    }
    
    self.onEncodingProgressCb(p);
}

- (void)onPlayEnd
{
    if(self.onPlayEndCb == nil) {
        return;
    }
    
    self.onPlayEndCb();
}

- (void)onPlayFail:(ERRORCODE)error clipId:(int)clipId
{
    
}

- (void)onPlayStart
{
    if(self.onPlayStartCb == nil) {
        return;
    }
    self.onPlayStartCb();
}

- (void)onClipInfoDone
{
    
}

- (void)onSeekStateChanged:(BOOL)isSeeking
{
    
}

- (void)onStopComplete:(ERRORCODE)errorCode
{
    if(self.onStopCompleteCb == nil) {
        return;
    }
    self.onStopCompleteCb(errorCode);
    // MJ.  NESI-132
    //  stopAsync 인자로 등록 된 block함수가 엔진상태에 따라서 호출이 되었고, 이로인해 원하지 않는 동작이 실행되었음.
    //  이를 방지하고자 아래와 같이 nil로 재설정합니다. 추가로 stopAsync 호출 시 onStopCompleteCb 등록이 되기에 동작에 문제는 없을 것임.
    //
    self.onStopCompleteCb = nil;
}

- (void)onPrepared
{
    if(self.onPreparedCallbackCb == nil) {
        return;
    }
    self.onPreparedCallbackCb();
}

@end
