/******************************************************************************
 * File Name   :	ImagePropertyTableViewController.h
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

typedef void (^refreshImageProperty)();

@interface ImagePropertyTableViewController : UITableViewController

@property (strong, nonatomic) IBOutlet UILabel *clipEffectLabel;
@property (strong, nonatomic) IBOutlet UILabel *transitionEffectLabel;
@property (strong, nonatomic) IBOutlet UILabel *colorFilterLabel;
@property (strong, nonatomic) IBOutlet UISlider *clipDurationSlider;
@property (strong, nonatomic) IBOutlet UILabel *clipDurationLabel;
@property (strong, nonatomic) IBOutlet UISlider *effectDurationSlider;
@property (strong, nonatomic) IBOutlet UILabel *effectDurationLabel;
@property (strong, nonatomic) IBOutlet UISlider *brightSlider;
@property (strong, nonatomic) IBOutlet UILabel *brightLabel;
@property (strong, nonatomic) IBOutlet UISlider *contrastSlider;
@property (strong, nonatomic) IBOutlet UILabel *contrastLabel;
@property (strong, nonatomic) IBOutlet UISlider *saturationSlider;
@property (strong, nonatomic) IBOutlet UILabel *saturationLabel;
@property (strong, nonatomic) IBOutlet UISwitch *vignetteSwitch;

- (void)setupImagePropertyListCB:(refreshImageProperty)refreshImageProperty clip:(NXEClip *)clip;

@end
