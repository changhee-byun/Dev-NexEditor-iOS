/******************************************************************************
 * File Name   : NXEEngineEventListener.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEEngineEventListener.h"
#import "NXEEngine.h"
#import "NXEEditorType.h"
//
#import "UIListenerProtocol.h"
#import <NexEditorEngine/EditorEventListener.h>
#import "NexImage.h"
#import "NXEEngineUIEventListener.h"
#import "LayerManager.h"
#import "KMLayer.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import <NexEditorEngine/NEXVIDEOEDITOR_EventTypes.h>

#define LOG_TAG @"NXEEngineEventListener"

@implementation SeekInfo
- (void) dealloc
{
    self.seekInternalCB = nil;
    [super dealloc];
}
@end

@interface NXEEngineEventListener ()

@property (copy, nonatomic) notifyEventCallBack notifyEventCB;
@property (copy, nonatomic) notifyErrorCallBack notifyErrorCB;
@property (copy, nonatomic) captureEventCallBack captureEventCB;
@property (copy, nonatomic) captureDoneCallBack captureDoneCB;
@property (copy, nonatomic) captureFailCallBack captureFailCB;
@property (nonatomic) NXEEngineUIEventListener *uiListener;
@property (nonatomic) NXEPLAYSTATE playState;
@property (nonatomic) EditorEventListener *editorEventListener; // 리소스 소멸은 Editor소멸하는 과정에서 실행하기에 release 호출하지 않아도 됨.
/// Early updated new state value before playState update occur in main thread
@property (nonatomic) NXEPLAYSTATE playStateEarlyUpdated;
@property (nonatomic, strong) SeekInfo *seekInfo;

@end

@implementation NXEEngineEventListener
{
    int currentPosition;
}

- (instancetype)initWithCB:(seekInternalCallback)seekInternalCallback
{
    self = [super init];
    if(self) {
        _editorEventListener = [[NexEditor sharedInstance] getEditorEventHandler];

        __block NXEEngineEventListener *weakSelf = self;
        self.notifyEventCB = ^(unsigned int iEventType, unsigned int iParam1, unsigned int iParam2, unsigned int iParam3, unsigned int iParam4) {
            [weakSelf notifyEvent:iEventType param1:iParam1 param2:iParam2 param3:iParam3 param4:iParam4];
        };
        
        self.notifyErrorCB = ^(unsigned int iEventType, unsigned int iResult, unsigned int iParam1, unsigned int iParam2) {
            [weakSelf notifyError:iEventType result:iResult param1:iParam1 param2:iParam2];
        };
        
        self.captureEventCB = ^(int width, int height, int size, char* data) {
            [weakSelf callbackCapture:width height:height size:size data:data];
        };
        
        [_editorEventListener registerCallbackError:_notifyErrorCB];
        [_editorEventListener registerCallbackEvent:_notifyEventCB];
        [_editorEventListener registerCallbackCapture:_captureEventCB];
        [_editorEventListener registerImageLoader:[[[NexImage alloc] init] autorelease]];
        [_editorEventListener registerCallbackLUT:[[[NexImage alloc] init] autorelease]];
        [_editorEventListener registerCallbackVignette:[[[NexImage alloc] init] autorelease]];
        [_editorEventListener registerCallbackCustomLayer:[LayerManager sharedInstance]];
        
        _playState = NXE_PLAYSTATE_NONE;
        _playStateEarlyUpdated = NXE_PLAYSTATE_NONE;
        
        //
        self.seekInfo = [[[SeekInfo alloc] init] autorelease];
        self.seekInfo.seekInternalCB = seekInternalCallback;
    }
    return self;
}

- (void) dealloc
{
    self.seekInfo = nil;
    self.notifyEventCB = nil;
    self.notifyErrorCB = nil;
    self.captureEventCB = nil;
    self.captureDoneCB = nil;
    self.captureFailCB = nil;
    [super dealloc];
}

- (void)dispatchStateChangeEvent:(NXEPLAYSTATE)oldState newState:(NXEPLAYSTATE)newState
{
    if(self.playState != newState) {
        self.playState = newState;
        if(self.uiListener != nil) {
            [self.uiListener onStateChange:oldState newState:newState];
            //
            
            if(newState != NXE_PLAYSTATE_RUN && newState != NXE_PLAYSTATE_RECORD) {
                if([self.uiListener respondsToSelector:@selector(onStopComplete:)]) {
                    [self.uiListener onStopComplete:ERROR_NONE];
                    
                    // stopAsync에서 지속적으로 callback이 올라오는 문제 수정
                    [self.uiListener registerOnStopCompleteCb:nil];
                }
            }
        }
    }
}

- (NXEPLAYSTATE) playStateEarlyUpdated
{
    return _playStateEarlyUpdated;
}

- (int)getCurrentPosition
{
    return currentPosition;
}

- (void)setTimeDone:(int)seekDoneTime frameTime:(int)frameTime error:(ERRORCODE)error
{
    SeekInfo *seekInfo = self.seekInfo;
    
    seekInfo.isSeeking = NO;
    
    if(seekInfo.isPendingNonDisplaySeek) {
        seekInfo.isPendingNonDisplaySeek = NO;
        seekInfo.seekInternalCB(seekInfo.pendingNonDisplaySeekLocation, false, 0);
    } else if(seekInfo.isPendingSeek) {
        seekInfo.isPendingSeek = NO;
        if(seekInfo.pendingSeekIDR == 0) {
            seekInfo.seekInternalCB(seekInfo.pendingSeekLocation, true, 0);
        } else if(seekInfo.pendingSeekIDR == 1) {
            seekInfo.seekInternalCB(seekInfo.pendingSeekLocation, true, 1);
        } else {
            seekInfo.seekInternalCB(seekInfo.pendingSeekLocation, true, 2);
        }
    }
    
    NSString *seekDump = [NSString stringWithFormat:@"isSeeking= %d; isPendingSeek= %d; pendingSeekLocation= %d;",
                          seekInfo.isSeeking, seekInfo.isPendingSeek, seekInfo.pendingSeekLocation];
    
    if(error == ERROR_SETTIME_IGNORED) {
        NexLogD(LOG_TAG, @"[NXEEngine.m %d] ERROR_SETTIME_IGNORED / %@ ", __LINE__, seekDump);
    } else {
        NexLogD(LOG_TAG, @"[NXEEngine.m %d] setTimeDone(%d , %d) / %@ ", __LINE__, seekDoneTime, frameTime, seekDump);
    }
}

- (void)setTimeIgnored
{
    [self setTimeDone:0 frameTime:0 error:ERROR_SETTIME_IGNORED];
}

#pragma mark - uiListener

- (void)setUIListener:(id)listener
{
    if(listener == nil) {
        self.uiListener = nil;
    } else {
        if([listener conformsToProtocol:@protocol(UIListenerProtocol)]) {
            self.uiListener = listener;
        } else {
            NSException *e = [NSException exceptionWithName:@"General Exception"
                                                     reason:@"Not Implement UIListenerProtocol"
                                                   userInfo:nil];
            @throw e;
        }
    }
}

- (id)getUiListener
{
    return self.uiListener;
}

#pragma mark - notify

- (int)notifyEvent:(unsigned int)iEventType param1:(unsigned int)iParam1 param2:(unsigned int)iParam2 param3:(unsigned int)iParam3 param4:(unsigned int)iParam4
{
    const int p1 = iParam1;
    const int p2 = iParam2;
    const int p3 = iParam3;
    const int p4 = iParam4;
    
    NexLogD(LOG_TAG, @"EVENT(%d;%d,%d,%d,%d)", iEventType, p1, p2, p3, p4);
    
    switch(iEventType) {
        case VIDEOEDITOR_EVENT_STATE_CHANGE:
            // NESI-104 이슈 수정
            self.playStateEarlyUpdated = p2;
            dispatch_async(dispatch_get_main_queue(), ^{
                [self dispatchStateChangeEvent:(NXEPLAYSTATE)p1
                                      newState:(NXEPLAYSTATE)p2];
            });
            break;
        case VIDEOEDITOR_EVENT_CURRENTTIME:
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.uiListener != nil) {
                    if(self.playState == PLAYSTATE_RUN) {
                        currentPosition = p1;
                        [self.uiListener onTimeChange:p1];
                    } else if(self.playState == PLAYSTATE_RECORD) {
                        [self.uiListener onEncodingProgress:p1];
                    }
                }
            });
            break;
        case VIDEOEDITOR_EVENT_CLOSE_PROJECT:
            break;
        case VIDEOEDITOR_EVENT_ADDCLIP_DONE:
            break;
        case VIDEOEDITOR_EVENT_MOVECLIP_DONE:
            break;
        case VIDEOEDITOR_EVENT_DELETECLIP_DONE:
            break;
        case VIDEOEDITOR_EVENT_EFFECTCHANGE_DONE:
            break;
        case VIDEOEDITOR_EVENT_ENCODE_PROJECT:
            //
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.uiListener != nil) {
                    [self.uiListener onEncodingDone:ERROR_NONE];
                }
                [self dispatchStateChangeEvent:self.playState newState:NXE_PLAYSTATE_IDLE];
            });
            break;
        case VIDEOEDITOR_EVENT_ENCODE_ERROR:
            //
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.uiListener != nil) {
                    [self.uiListener onEncodingDone:(ERRORCODE)p1];
                }
                [self dispatchStateChangeEvent:self.playState newState:NXE_PLAYSTATE_IDLE];
            });
            break;
            
        // Before
        // p2 == 1 or error
        // p1 == seek time
        
        // After
        // p1 == 0 : success, other : failed
        // p2 == 1 : ignore,
        // p3 == set time
        // p4 == result time
        case VIDEOEDITOR_EVENT_SETTIME_DONE:
            dispatch_async(dispatch_get_main_queue(), ^{
                if(p2 == 1) {
                    [self setTimeIgnored];
                } else {
                    currentPosition = p3;
                    [self setTimeDone:p3 frameTime:p4 error:ERROR_NONE];
                }
                if(self.uiListener != nil) {
                    if(p2 == 1) {
                        [self.uiListener onSetTimeIgnored];
                    } else {
                        if(p1 != 0) {
                            [self.uiListener onSetTimeFail:(ERRORCODE)p1];
                        } else {
                            [self.uiListener onSetTimeDone:p3];
                        }
                    }
                }
            });
            break;
        case VIDEOEDITOR_EVENT_ASYNC_LOADLIST:
            break;
        case VIDEOEDITOR_EVENT_PLAY_END:
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.uiListener != nil) {
                    [self.uiListener onPlayEnd];
                }
                if(self.playState != PLAYSTATE_RECORD) {
                    [self dispatchStateChangeEvent:self.playState newState:NXE_PLAYSTATE_IDLE];
                }
            });
            break;
        case VIDEOEDITOR_EVENT_PLAY_ERROR:
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.uiListener != nil) {
                    [self.uiListener onPlayFail:(ERRORCODE)p2 clipId:p3];
                }
            });
            break;
        case VIDEOEDITOR_EVENT_VIDEO_STARTED:
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.uiListener != nil) {
                    if(self.playState != PLAYSTATE_RECORD) {
                        [self.uiListener onPrepared];
                        [self.uiListener onPlayStart];
                    }
                }
            });
            break;
        case VIDEOEDITOR_EVENT_CAPTURE_DONE:
            dispatch_async(dispatch_get_main_queue(), ^{
                if(self.captureFailCB != nil) {
                    self.captureFailCB((ERRORCODE)p1);
                }
            });
            break;
        case VIDEOEDITOR_EVENT_PREPARE_CLIP_LOADING:
            break;
        case VIDEOEDITOR_EVENT_TRANSCODING_PROGRESS:
            break;
        case VIDEOEDITOR_EVENT_TRANSCODING_DONE:
            break;
        case VIDEOEDITOR_EVENT_GETCLIPINFO_DONE:
            break;
        case VIDEOEDITOR_EVENT_GETCLIPINFO_STOP_DONE:
            break;
        case VIDEOEDITOR_EVENT_FAST_OPTION_PREVIEW_DONE:
            break;
        case VIDEOEDITOR_EVENT_CHECK_DIRECT_EXPORT:
            break;
        case VIDEOEDITOR_EVENT_DIRECT_EXPORT_DONE:
            break;
        case VIDEOEDITOR_EVENT_DIRECT_EXPORT_PROGRESS:
            break;
        case VIDEOEDITOR_EVENT_HIGHLIGHT_THUMBNAIL_PROGRESS:
            break;
        case VIDEOEDITOR_EVENT_MAKE_REVERSE_DONE:
            break;
        case VIDEOEDITOR_EVENT_MAKE_REVERSE_PROGRESS:
            break;
        case VIDEOEDITOR_EVENT_FAST_PREVIEW_START_DONE:
            break;
        case VIDEOEDITOR_EVENT_FAST_PREVIEW_STOP_DONE:
            break;
        case VIDEOEDITOR_EVENT_FAST_PREVIEW_TIME_DONE:
            break;
        case VIDEOEDITOR_EVENT_MAKE_HIGHLIGHT_PROGRESS_INDEX:
            break;
        default:
            break;
    }
    return 0;
}

- (int)notifyError:(int)iEventType result:(int)iResult param1:(int)iParam1 param2:(int)iParam2
{
    NexLogD(LOG_TAG, @"notifyError] event(%d) param(%d, %d)", iEventType, iParam1, iParam2);
    //
    return 0;
}

#pragma mark - capture

void providerReleaseData(void *info, const void *data, size_t size)
{
    free((void*)data);
}

- (void)callbackCapture:(int)width height:(int)height size:(int)size data:(char*)data
{
    if(width < 1 || height < 1 || size < 1 || data == NULL) {
        self.captureFailCB(ERROR_CAPTURE_FAILED);
        return;
    }
    
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, data, width*height*4, providerReleaseData);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef imageRef = CGImageCreate(width,
                                        height,
                                        8/* bitsPerComponet */,
                                        32/* bitsPerPixel */,
                                        4*width/* bytesPerRow */,
                                        colorSpace,
                                        kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast,
                                        provider,
                                        NULL,
                                        YES,
                                        kCGRenderingIntentDefault);
    
    uint32_t* pImageData = calloc(width * height * 4, sizeof(char));
    
    CGContextRef contextRef = CGBitmapContextCreate(pImageData,
                                                    width,
                                                    height,
                                                    8/* bitsPerComponet */,
                                                    4*width/* bytesPerRow */,
                                                    colorSpace,
                                                    kCGImageAlphaPremultipliedLast);
    
    CGContextTranslateCTM(contextRef, 0, height);
    CGContextScaleCTM(contextRef, 1.0, -1.0);
    CGContextDrawImage(contextRef, CGRectMake(0, 0, width, height), imageRef);
    
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        
        if(self.captureDoneCB != nil) {
            self.captureDoneCB(CGBitmapContextCreateImage(contextRef));
        }
        
        CGContextRelease(contextRef);

        free(pImageData);
    });
    return;
}

- (void)registerCaptureCallback:(captureDoneCallBack)callback
            captureFailCallback:(captureFailCallBack)callback2
{
    self.captureDoneCB = callback;
    self.captureFailCB = callback2;
}

@end
