/******************************************************************************
 * File Name   : EditorEventListener.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "EditorEventListener.h"
#import "ImageLoaderProtocol.h"
#import "CustomLayerProtocol.h"
#import "LUTLoaderProtocol.h"
#import "VignetteLoaderProtocol.h"
#import "EditorEventListenerPrivate.h"
#import "EditorEventListener+EventDelegate.h"
#import "NexEditorLog.h"

@interface EditorEventListener ()

@property (nonatomic, retain) id<ImageLoaderProtocol> imageLoader;
@property (nonatomic, retain) id<CustomLayerProtocol> customLayerLoader;
@property (nonatomic, retain) id lutLoader;
@property (nonatomic, retain) id vignetteLoader;
@property (copy, nonatomic) notifyEventCallBack notifyEventCB;
@property (copy, nonatomic) notifyErrorCallBack notifyErrorCB;
@property (copy, nonatomic) captureEventCallBack captureEventCB;
@property (copy, nonatomic) thumbnailCallBack thumbnailCB;
@property (copy, nonatomic) highlightEventCallBack hightlightCB;

@property (nonatomic, copy) NexEditorOnEventBlock onEvent;
@property (nonatomic, strong) dispatch_queue_t serialQueue;

@end

@implementation EditorEventListener

- (void)dealloc
{
    if(self.lutLoader != nil) {
        if([self.lutLoader respondsToSelector:@selector(deleteTextureForLUT)]) {
            [self.lutLoader deleteTextureForLUT];
        }
    }
    
    if(self.vignetteLoader != nil) {
        if([self.vignetteLoader respondsToSelector:@selector(deleteTextureForVignette)]) {
            [self.vignetteLoader deleteTextureForVignette];
        }
    }
    self.onEvent = nil;
    [_imageLoader release];
    [_customLayerLoader release];
    [_lutLoader release];
    [_vignetteLoader release];
    dispatch_release(self.serialQueue);
    self.notifyEventCB = nil;
    self.notifyErrorCB = nil;
    self.captureEventCB = nil;
    self.thumbnailCB = nil;
    self.hightlightCB = nil;
    self.serialQueue = nil;
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        _notifyEventCB = nil;
        _notifyErrorCB = nil;
        self.serialQueue = dispatch_queue_create("EditorEventListenerSerialQueue", DISPATCH_QUEUE_SERIAL);
        dispatch_retain(self.serialQueue);
    }
    return self;
}

- (void)registerImageLoader:(id<ImageLoaderProtocol>)loader
{
    self.imageLoader = loader;
}

- (id<ImageLoaderProtocol>)getImageLoader
{
    return self.imageLoader;
}

#pragma mark - layer

- (instancetype)callbackCustomLayer:(int)iParam1 param2:(int)iParam2 param3:(int)iParam3
                             param4:(int)iParam4 param5:(int)iParam5 param6:(int)iParam6
                             param7:(int)iParam7 param8:(int)iParam8 param9:(int)iParam9
                            param10:(int)iParam10 param11:(int)iParam11 param12:(int)iParam12
                            param13:(int)iParam13 param14:(int)iParam14 param15:(int)iParam15
                            param16:(int)iParam16 param17:(int)iParam17 param18:(int)iParam18
{
    if(self.customLayerLoader == nil) {
        return nil;
    }
    
    if([self.customLayerLoader respondsToSelector:@selector(renderOverlay:param2:param3:param4:param5:param6:param7:param8:param9:param10:param11:param12:param13:param14:param15:param16:param17:param18:)] ) {
        if([self.customLayerLoader renderOverlay:iParam1 param2:iParam2 param3:iParam3 param4:iParam4
                                          param5:iParam5 param6:iParam6 param7:iParam7 param8:iParam8
                                          param9:iParam9 param10:iParam10 param11:iParam11 param12:iParam12
                                         param13:iParam13 param14:iParam14 param15:iParam15 param16:iParam16
                                         param17:iParam17 param18:iParam18] == false) {
//            [self.customLayerLoader clearResourceInRenderContext:NexLayerRenderContextExport];
            return nil;
        } else {
            return self.customLayerLoader;
        }
    } else {
        return nil;
    }
}

- (void)registerCallbackCustomLayer:(id<CustomLayerProtocol>)customLayerLoader
{
    self.customLayerLoader = customLayerLoader;
}

- (id<CustomLayerProtocol>)getCustomLayerLoader
{
    return self.customLayerLoader;
}

#pragma mark - lut

- (int)getLUTWithID:(int)lutResourceID exportFlag:(int)exportFlag
{
    if(self.lutLoader == nil) {
        return -1;
    }
    if([self.lutLoader respondsToSelector:@selector(getLUTWithID:exportFlag:)]) {
        return [self.lutLoader getLUTWithID:lutResourceID exportFlag:exportFlag];
    } else {
        return -1;
    }
}

- (void)registerCallbackLUT:(id)lutLoader
{
    if([lutLoader conformsToProtocol:@protocol(LUTLoaderProtocol)]) {
        self.lutLoader = lutLoader;
    } else {
        @throw [NSException exceptionWithName:@"General Exception"
                                       reason:@"Not Implement LUTLoaderProtocol"
                                     userInfo:nil];
    }
}

#pragma mark - vignette

- (int)getVignetteTextID:(int)export_mode
{
    if(self.vignetteLoader == nil) {
        return -1;
    }
    if([self.vignetteLoader respondsToSelector:@selector(getVignetteTextID:)]) {
        return [self.vignetteLoader getVignetteTextID:export_mode];
    } else {
        return -1;
    }
}

- (void)registerCallbackVignette:(id)vignetteLoader
{
    if([vignetteLoader conformsToProtocol:@protocol(VignetteLoaderProtocol)]) {
        self.vignetteLoader = vignetteLoader;
    } else {
        @throw [NSException exceptionWithName:@"General Exception"
                                       reason:@"Not Implement VignetteLoaderProtocol"
                                     userInfo:nil];
    }
}

#pragma mark - notify

- (int)notifyEvent:(int)iEventType param1:(int)iParam1 param2:(int)iParam2 param3:(int)iParam3 param4:(int)iParam4
{
    if (self.onEvent) {
        NexEditorEvent event = { .type =  (VIDEOEDITOR_EVENT) iEventType, .params = {iParam1, iParam2, iParam3, iParam4} };
        self.onEvent(event);
    }
    
    if(self.notifyEventCB != nil) {
        dispatch_async(self.serialQueue, ^{
            self.notifyEventCB(iEventType, iParam1, iParam2, iParam3, iParam4);
        });
    }
    return 0;
}

- (int)notifyError:(int)iEventType result:(int)iResult param1:(int)iParam1 param2:(int)iParam2
{
    if(self.notifyErrorCB != nil) {
        self.notifyErrorCB(iEventType, iResult, iParam1, iParam2);
    }
    return 0;
}

- (void)registerCallbackEvent:(notifyEventCallBack)callback
{
    dispatch_async(self.serialQueue, ^{
        self.notifyEventCB = callback;
    });
}

- (void)registerCallbackError:(notifyErrorCallBack)callback
{
    self.notifyErrorCB = callback;
}

#pragma mark - capture

- (int)callbackCapture:(int)width height:(int)height size:(int)size data:(char*)data
{
    if(self.captureEventCB != nil) {
        self.captureEventCB(width, height, size, data);
    }
    return 0;
}

- (void)registerCallbackCapture:(captureEventCallBack)callback
{
    self.captureEventCB = callback;
}

#pragma mark - thumbnail

- (int)callbackThumb:(int)iMode tag:(int)iTag time:(int)iTime width:(int)iWidth height:(int)iHeight count:(int)iCount total:(int)iTotal size:(int)iSize data:(char*)data
{
    if(self.thumbnailCB != nil) {
        self.thumbnailCB(iMode, iTag, iTime, iWidth, iHeight, iCount, iTotal, iSize, data);
    }
    return 0;
}

- (void)registerCallbackThumb:(thumbnailCallBack)callback
{
    self.thumbnailCB = callback;
}

#pragma mark - highlight

- (int)callbackHighLightIndex:(int)count data:(int*)data
{
    if(self.hightlightCB != nil) {
        self.hightlightCB(count, data);
    }
    return 0;
}

- (void)registerCallbackHighLight:(highlightEventCallBack)callback
{
    self.hightlightCB = callback;
}

@end
