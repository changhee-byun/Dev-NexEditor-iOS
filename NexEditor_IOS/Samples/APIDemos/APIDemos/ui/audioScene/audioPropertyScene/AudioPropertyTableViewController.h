/******************************************************************************
 * File Name   :	AudioPropertyTableViewController.h
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
#import "AudioTrackDataController.h"

typedef void (^refreshTableView)(AudioTrackInfo *trackInfo);

@interface AudioPropertyTableViewController : UITableViewController

@property (retain, nonatomic) IBOutlet UISwitch *loopSwitch;
@property (retain, nonatomic) IBOutlet UISlider *volumeSlider;
@property (retain, nonatomic) IBOutlet UILabel *volumeValue;
@property (retain, nonatomic) IBOutlet UISlider *trimSlider;
@property (retain, nonatomic) IBOutlet UILabel *trimValue;
@property (strong, nonatomic) IBOutlet UISlider *startTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *startTimeValue;
@property (retain, nonatomic) IBOutlet UITextView *envelopTextView;
@property (retain, nonatomic) IBOutlet UILabel *envelopTimeValue;
@property (retain, nonatomic) IBOutlet UISlider *envelopTimeSlider;
@property (retain, nonatomic) IBOutlet UILabel *envelopValue;
@property (retain, nonatomic) IBOutlet UISlider *envelopValueSlider;
@property (retain, nonatomic) IBOutlet UISwitch *compressorSwitch;
@property (retain, nonatomic) IBOutlet UISlider *panLeftSlider;
@property (retain, nonatomic) IBOutlet UILabel *panLeftValue;
@property (retain, nonatomic) IBOutlet UISlider *panRightSlider;
@property (retain, nonatomic) IBOutlet UILabel *panRightValue;
@property (retain, nonatomic) IBOutlet UISlider *pitchSlider;
@property (retain, nonatomic) IBOutlet UILabel *pitchValue;
@property (retain, nonatomic) IBOutlet UILabel *audioChangerValue;

- (void)setupProperty:(AudioTrackInfo *)audioTrackInfo projectTime:(int)projectTime refreshTableView:(refreshTableView)refreshTableView;

@end
