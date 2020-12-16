/******************************************************************************
 * File Name   :	AudioTrackTableViewController.h
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

#import <UIKit/UIKit.h>

@class NXEClip;

@interface AudioTrackTableViewController : UITableViewController

- (void)setupAudioList:(NSArray *)audioList
           projectTime:(int)projectTime
            titleArray:(NSArray *)titleArray
             maxTracks:(NSInteger)maxTracks;

@end
