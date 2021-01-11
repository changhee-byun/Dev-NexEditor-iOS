/******************************************************************************
 * File Name   : NXEEngineEventListener.h
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
#import <NexEditorEngine/EditorEventListener.h>
#import "NXEEditorErrorCode.h"

typedef void (^captureDoneCallBack)(CGImageRef);
typedef void (^captureFailCallBack)(ERRORCODE);

//
typedef int (^seekInternalCallback)(int, BOOL, int);

@interface SeekInfo : NSObject

@property (nonatomic, assign) BOOL isSeeking;
@property (nonatomic, assign) BOOL isPendingSeek;
@property (nonatomic, assign) int pendingSeekLocation;
@property (nonatomic, assign) int pendingSeekIDR;
@property (nonatomic, assign) BOOL isPendingNonDisplaySeek;
@property (nonatomic, assign) int pendingNonDisplaySeekLocation;
@property (nonatomic, assign) int seekSerial;
@property (nonatomic, assign) BOOL didSetTimeSinceLastPlay;
@property (nonatomic, copy) int (^seekInternalCB)(int, BOOL, int);
@end

@interface NXEEngineEventListener : NSObject

@property (nonatomic, readonly) SeekInfo *seekInfo;
@property (nonatomic, readonly) NXEPLAYSTATE playStateEarlyUpdated;

- (int)getCurrentPosition;

- (instancetype)initWithCB:(seekInternalCallback)seekInternalCallback;

#pragma mark - uiListener

- (void)setUIListener:(id)listener;
- (id)getUiListener;

#pragma mark - capture

- (void)registerCaptureCallback:(captureDoneCallBack)callback
            captureFailCallback:(captureFailCallBack)callback2;

@end
