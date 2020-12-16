/******************************************************************************
 * File Name   : EditorEventListener.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#ifndef EditorEventListener_h
#define EditorEventListener_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "ImageLoaderProtocol.h"
#import "CustomLayerProtocol.h"

typedef void (^notifyEventCallBack)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
typedef void (^notifyErrorCallBack)(unsigned int, unsigned int, unsigned int, unsigned);
typedef void (^thumbnailCallBack)(int, int, int, int, int, int, int, int, char*);
typedef void (^captureEventCallBack)(int, int, int, char*);
typedef void (^highlightEventCallBack)(int, int*);

@interface EditorEventListener : NSObject

#pragma mark - ImageLoader

- (void)registerImageLoader:(id<ImageLoaderProtocol>)imageLoader;

#pragma mark - layer

- (void)registerCallbackCustomLayer:(id<CustomLayerProtocol>)customLayerLoader;

#pragma mark - lut
- (void)registerCallbackLUT:(id)callbackLUTWithID;

#pragma mark - vignette
- (void)registerCallbackVignette:(id)vignetteLoader;

#pragma mark - notify

- (void)registerCallbackEvent:(notifyEventCallBack)callback;
- (void)registerCallbackError:(notifyErrorCallBack)callback;

#pragma mark - capture

- (void)registerCallbackCapture:(captureEventCallBack)callback;

#pragma mark - thumbnail

- (void)registerCallbackThumb:(thumbnailCallBack)callback;

#pragma mark - highlight

- (void)registerCallbackHighLight:(highlightEventCallBack)callback;

@end

#endif /* EditorEventListener_h */
