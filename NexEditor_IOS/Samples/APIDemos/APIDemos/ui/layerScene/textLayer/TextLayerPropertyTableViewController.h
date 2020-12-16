/******************************************************************************
 * File Name   :	TextLayerPropertyTableViewController.h
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

@interface TextLayerPropertyTableViewController : UITableViewController

@property (strong, nonatomic) IBOutlet UILabel *editTextLabel;
@property (strong, nonatomic) IBOutlet UISlider *kerningSlider;
@property (strong, nonatomic) IBOutlet UILabel *kerningLabel;
@property (strong, nonatomic) IBOutlet UISlider *lineSpaceSlider;
@property (strong, nonatomic) IBOutlet UILabel *lineSpaceLabel;
@property (strong, nonatomic) IBOutlet UISegmentedControl *horizontalAlignSegment;
@property (strong, nonatomic) IBOutlet UISegmentedControl *verticalAlignSegment;
@property (strong, nonatomic) IBOutlet UISwitch *underlineSwitch;
@property (strong, nonatomic) IBOutlet UILabel *inAnimationLabel;
@property (strong, nonatomic) IBOutlet UISlider *inAnimationSlider;
@property (strong, nonatomic) IBOutlet UILabel *inAnimationTimeLabel;
@property (strong, nonatomic) IBOutlet UILabel *expressionLabel;
@property (strong, nonatomic) IBOutlet UILabel *outAnimationLabel;
@property (strong, nonatomic) IBOutlet UISlider *outAnimationSlider;
@property (strong, nonatomic) IBOutlet UILabel *outAnimationTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *startTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *startTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *endTimeSlider;
@property (strong, nonatomic) IBOutlet UILabel *endTimeLabel;
@property (strong, nonatomic) IBOutlet UISlider *alphaSlider;
@property (strong, nonatomic) IBOutlet UILabel *alphaLabel;
@property (strong, nonatomic) IBOutlet UILabel *fontNamingLabel;
@property (strong, nonatomic) IBOutlet UIButton *fontColorButton;
@property (strong, nonatomic) IBOutlet UIButton *bgColorButton;
@property (strong, nonatomic) IBOutlet UISwitch *bgColorSwitch;
@property (strong, nonatomic) IBOutlet UIButton *glowColorButton;
@property (strong, nonatomic) IBOutlet UISwitch *glowColorSwitch;
@property (strong, nonatomic) IBOutlet UISlider *glowSpreadSlider;
@property (strong, nonatomic) IBOutlet UILabel *glowSpreadLabel;
@property (strong, nonatomic) IBOutlet UISlider *glowSizeSlider;
@property (strong, nonatomic) IBOutlet UILabel *glowSizeLabel;
@property (strong, nonatomic) IBOutlet UIButton *shadowColorButton;
@property (strong, nonatomic) IBOutlet UISwitch *shadowColorSwitch;
@property (strong, nonatomic) IBOutlet UISlider *shadowAngleSlider;
@property (strong, nonatomic) IBOutlet UILabel *shadowAngleLabel;
@property (strong, nonatomic) IBOutlet UISlider *shadowDistanceSlider;
@property (strong, nonatomic) IBOutlet UILabel *shadowDistanceLabel;
@property (strong, nonatomic) IBOutlet UISlider *shadowSpreadSlider;
@property (strong, nonatomic) IBOutlet UILabel *shadowSpreadLabel;
@property (strong, nonatomic) IBOutlet UISlider *shadowSizeSlider;
@property (strong, nonatomic) IBOutlet UILabel *shadowSizeLabel;
@property (strong, nonatomic) IBOutlet UIButton *outlineColorButton;
@property (strong, nonatomic) IBOutlet UISwitch *outlineColorSwitch;
@property (strong, nonatomic) IBOutlet UISlider *fontSizeSlider;
@property (strong, nonatomic) IBOutlet UILabel *fontSizeLabel;
@property (strong, nonatomic) IBOutlet UISlider *outlineThicknessSlider;
@property (strong, nonatomic) IBOutlet UILabel *outlineThicknessLabel;

- (void)setupProperty:(NXETextLayer *)layer postion:(int)position refreshListItem:(refreshListItem)refreshListItem;

@end
