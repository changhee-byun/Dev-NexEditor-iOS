/*
 * File Name   : ExportController.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import <UIKit/UIKit.h>

@class NXEEngine;
@class ExportController;

@protocol ExportControllerDelegate <NSObject>
- (void)seekTo:(int)time complete:(void (^)(void)) complete;
- (void)exportController:(ExportController *) controller didFinish:(BOOL) finish error:(NSError *) error;
@end

@interface ExportController: NSObject
@property (nonatomic, weak) id<ExportControllerDelegate> delegate;
@property (nonatomic, readonly) NSString *exportPath;

- (void)presentExportActionSheet;
- (instancetype) initWithHostViewController:(UIViewController *) host editor:(NXEEngine *) editor;
@end
