/******************************************************************************
 * File Name   :	VideoLayerPropertyTableViewController.h
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
@import NexEditorFramework;

typedef void (^refreshListItem)(int);

@interface VideoLayerPropertyTableViewController : UITableViewController

@property (strong, nonatomic) IBOutlet UILabel *inAnimationLabel;
@property (strong, nonatomic) IBOutlet UISlider *inAnimationSlider;
@property (strong, nonatomic) IBOutlet UILabel *inAnimationTimeLabel;
@property (strong, nonatomic) IBOutlet UILabel *expressionLabel;
@property (strong, nonatomic) IBOutlet UILabel *outAnimationLabel;
@property (strong, nonatomic) IBOutlet UISlider *outAnimationSlider;
@property (strong, nonatomic) IBOutlet UILabel *outAnimationTimeLabel;
@property (strong, nonatomic) IBOutlet UILabel *colorFilterLabel;
@property (strong, nonatomic) IBOutlet UISlider *brightnessSlider;
@property (strong, nonatomic) IBOutlet UILabel *brightnessLabel;
@property (strong, nonatomic) IBOutlet UISlider *contrastSlider;
@property (strong, nonatomic) IBOutlet UILabel *contrastLabel;
@property (strong, nonatomic) IBOutlet UISlider *saturationSlider;
@property (strong, nonatomic) IBOutlet UILabel *saturationLabel;
@property (strong, nonatomic) IBOutlet UIButton *strimButton;
@property (strong, nonatomic) IBOutlet UIButton *etrimButton;
@property (strong, nonatomic) IBOutlet UISlider *trimSlider;
@property (strong, nonatomic) IBOutlet UILabel *trimTimeLabel;
@property (strong, nonatomic) IBOutlet UILabel *trimStartLabel;
@property (strong, nonatomic) IBOutlet UILabel *trimEndLabel;
@property (strong, nonatomic) IBOutlet UISlider *startTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *startTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *endTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *endTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *alphaSlider;
@property (strong, nonatomic) IBOutlet UILabel *alphaLabel;
@property (strong, nonatomic) IBOutlet UISlider *volumeSlider;
@property (strong, nonatomic) IBOutlet UILabel *volumeLabel;
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

- (void)setupProperty:(NXEVideoLayer *)layer postion:(int)postion refreshListItem:(refreshListItem)refreshListItem;

@end
