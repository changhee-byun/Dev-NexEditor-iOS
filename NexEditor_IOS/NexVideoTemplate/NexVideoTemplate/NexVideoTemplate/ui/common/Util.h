/******************************************************************************
 * File Name   :	Util.h
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
#import <Foundation/Foundation.h>
@import Photos;
@interface Util : NSObject

/**
 */
+ (NSString*)getCurrentTime;

/**
 * 입력으로 받는 시간을 기준으로 xx:xx:xx 혹은 xx:xx 형식의 스트링을 리턴해준다.
 */
+ (NSString *)getStringTime:(int)timeMs;


+ (void)toastMessage:(UIViewController*)viewController title:(NSString*)title message:(NSString*)message duration:(int)duration;


+ (void)removeAllExportFileInDocument;
+ (void)removeFileInDocument:(NSString*)path;
+ (UIImage *)imageFromVideoAsset:(AVAsset*)asset;
+ (NSString*)removeSubString:(NSString*)substring from:(NSString*)string;
@end