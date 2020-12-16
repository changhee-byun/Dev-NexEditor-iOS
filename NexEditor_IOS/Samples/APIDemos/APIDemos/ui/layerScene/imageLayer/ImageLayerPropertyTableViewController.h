/******************************************************************************
 * File Name   :	ImageLayerPropertyTableViewController.h
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

@interface ImageLayerPropertyTableViewController : UITableViewController

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
@property (strong, nonatomic) IBOutlet UISlider *startTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *startTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *endTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *endTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *alphaSlider;
@property (strong, nonatomic) IBOutlet UILabel *alphaLabel;

- (void)setupProperty:(NXEImageLayer *)layer postion:(int)position refreshListItem:(refreshListItem)refreshListItem;

@end
