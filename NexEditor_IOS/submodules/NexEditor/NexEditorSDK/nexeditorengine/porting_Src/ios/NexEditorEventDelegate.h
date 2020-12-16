/*
 * File Name   : NexEditorEventDelegate.h
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
#ifndef NexEditorEventDelegate_h
#define NexEditorEventDelegate_h

#import <Foundation/Foundation.h>
#import "ImageLoaderProtocol.h"
#import "CustomLayerProtocol.h"

@protocol NexEditorEventDelegate <NSObject>

#pragma mark - ImageLoader

- (id<ImageLoaderProtocol>)getImageLoader;

#pragma mark - layer

- (id<CustomLayerProtocol>)getCustomLayerLoader;

- (instancetype)callbackCustomLayer:(int)iParam1 param2:(int)iParam2 param3:(int)iParam3
                             param4:(int)iParam4 param5:(int)iParam5 param6:(int)iParam6
                             param7:(int)iParam7 param8:(int)iParam8 param9:(int)iParam9
                            param10:(int)iParam10 param11:(int)iParam11 param12:(int)iParam12
                            param13:(int)iParam13 param14:(int)iParam14 param15:(int)iParam15
                            param16:(int)iParam16 param17:(int)iParam17 param18:(int)iParam18;

#pragma mark - lut
- (int)getLUTWithID:(int)lutResourceID exportFlag:(int)exportFlag;

#pragma mark - vignette
- (int)getVignetteTextID:(int)export_mode;

#pragma mark - notify

- (int)notifyEvent:(int)iEventType param1:(int)iParam1 param2:(int)iParam2 param3:(int)iParam3 param4:(int)iParam4;
- (int)notifyError:(int)iEventType result:(int)iResult param1:(int)iParam1 param2:(int)iParam2;

#pragma mark - capture

- (int)callbackCapture:(int)width height:(int)height size:(int)size data:(char*)data;

#pragma mark - thumbnail

- (int)callbackThumb:(int)iMode tag:(int)iTag time:(int)iTime width:(int)iWidth height:(int)iHeight count:(int)iCount total:(int)iTotal size:(int)iSize data:(char*)data;

#pragma mark - highlight

- (int)callbackHighLightIndex:(int)count data:(int*)data;

@end

#endif /* NexEditorEventDelegate_h */
