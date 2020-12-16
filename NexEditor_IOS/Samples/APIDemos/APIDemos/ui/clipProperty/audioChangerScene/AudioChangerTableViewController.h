/******************************************************************************
 * File Name   :	AudioChangerTableViewController.h
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

typedef void (^refreshAudioChangerValue)(NSString *);

@interface AudioChangerTableViewController : UITableViewController

- (void)setupAudioChangerListWithCB:(refreshAudioChangerValue)refreshTableView selectedValue:(NSString *)selectedValue;

@end