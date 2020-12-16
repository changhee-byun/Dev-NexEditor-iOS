/******************************************************************************
 * File Name   :	ImageLayerPropertyTableViewController.m
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

#import "ImageLayerPropertyTableViewController.h"
#import "LayerAnimationTableViewController.h"
#import "LayerExpressionTableViewController.h"
#import "AudioChangerTableViewController.h"
#import "ColorFilterTableViewController.h"
#import "Util.h"
#import "LUTNames.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@interface ImageLayerPropertyTableViewController ()

@property (nonatomic, copy) void(^refreshImageLayerTableView)(int);
@property (nonatomic) NXELutID lutId;
@property (nonatomic, strong) NSString *lutName;
@end

@implementation ImageLayerPropertyTableViewController
{
    NXEImageLayer *imageLayer;
    int positionLayer;
    int positionInAnimation;
    int positionExpression;
    int positionOutAnimation;
}

- (void)setupProperty:(NXEImageLayer *)layer postion:(int)position refreshListItem:(refreshListItem)refreshListItem
{
    imageLayer = layer;
    positionLayer = position;
    positionInAnimation = imageLayer.inAnimation;
    positionExpression = imageLayer.expression;
    positionOutAnimation = imageLayer.outAnimation;
    self.lutId = imageLayer.lutId;
    self.lutName = [LUTNames nameForLutId: self.lutId];
    self.refreshImageLayerTableView = refreshListItem;
}

- (NSString *)getPosition
{
    NSArray *positionList = @[@"LEFT:0, TOP:0", @"LEFT:640, TOP:0", @"LEFT:0, TOP:360", @"LEFT:640, TOP:360", @"Others"];
    return [positionList objectAtIndex:positionLayer];
}

- (NSString *)getAnimation:(int)index type:(int)type
{
    NSArray *animationList = @[@"None", @"Fade", @"Pop", @"Slide Right", @"Slide Left", @"Slide Up", @"Slide Down", @"Spin CW", @"Spin CCW", @"Drop", @"Scale Up", @"Scale Down", @"Converge"];
    if(type == 1) {
        animationList = @[@"None", @"Fade", @"Slide Right", @"Slide Left", @"Slide Up", @"Slide Down", @"Spin CW", @"Spin CCW", @"Drop", @"Scale Up", @"Scale Down", @"Converge"];
    }
    return [animationList objectAtIndex:index];
}

- (NSString *)getExpression:(int)index
{
    NSArray *expressionList = @[@"None", @"Blink Slow", @"Flicker", @"Pulse", @"Jitter", @"Fountain", @"Ring", @"Floating", @"Drifting", @"Squishing"];
    return [expressionList objectAtIndex:index];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // In Animation
    self.inAnimationLabel.text = [self getAnimation:positionInAnimation type:0];
    self.inAnimationSlider.minimumValue = 0;
    self.inAnimationSlider.maximumValue = 5*1000;
    self.inAnimationSlider.value = imageLayer.inAnimationDuration;
    self.inAnimationTimeLabel.text = [StringTools formatTimeMs:imageLayer.inAnimationDuration];
    
    // Expression
    self.expressionLabel.text = [self getExpression:positionExpression];
    
    // Out Animation
    self.outAnimationLabel.text = [self getAnimation:positionOutAnimation type:1];
    self.outAnimationSlider.minimumValue = 0;
    self.outAnimationSlider.maximumValue = 5*1000;
    self.outAnimationSlider.value = imageLayer.outAnimationDuration;
    self.outAnimationTimeLabel.text = [StringTools formatTimeMs:imageLayer.outAnimationDuration];
    
    // ColorFilter
    self.colorFilterLabel.text = self.lutName;
    
    // Color Adjustment
    
    // i.   Bright
    self.brightnessLabel.text = [NSString stringWithFormat:@"%d", (int)(imageLayer.brightness*255)];
    self.brightnessSlider.minimumValue = -1;
    self.brightnessSlider.maximumValue = 1;
    self.brightnessSlider.value = imageLayer.brightness;
    
    // ii.  Contrast
    self.contrastLabel.text = [NSString stringWithFormat:@"%d", (int)(imageLayer.contrast*255)];
    self.contrastSlider.minimumValue = -1;
    self.contrastSlider.maximumValue = 1;
    self.contrastSlider.value = imageLayer.contrast;
    
    // iii. Saturation
    self.saturationLabel.text = [NSString stringWithFormat:@"%d", (int)(imageLayer.saturation*255)];
    self.saturationSlider.minimumValue = -1;
    self.saturationSlider.maximumValue = 1;
    self.saturationSlider.value = imageLayer.saturation;
    
    // Start Time
    self.startTimeLabel.text = [StringTools formatTimeMs:imageLayer.startTime];
    self.startTimeSlider.minimumValue = 0;
    self.startTimeSlider.maximumValue = [[NXEEngine instance].project getTotalTime];
    self.startTimeSlider.value = imageLayer.startTime;
    
    // End Time
    self.endTimeLabel.text = [StringTools formatTimeMs:imageLayer.endTime];
    self.endTimeSlider.minimumValue = self.startTimeSlider.value+1000;
    self.endTimeSlider.maximumValue = self.endTimeSlider.minimumValue+[[NXEEngine instance].project getTotalTime];
    self.endTimeSlider.value = imageLayer.endTime;
    
    // Alpha
    self.alphaLabel.text = [NSString stringWithFormat:@"%d", (int)(100 * imageLayer.alpha)];
    self.alphaSlider.minimumValue = 0;
    self.alphaSlider.maximumValue = 1;
    self.alphaSlider.value = imageLayer.alpha;
}

#pragma mark - listener

- (IBAction)brightnessValueChanged:(id)sender
{
    self.brightnessLabel.text = [NSString stringWithFormat:@"%d", (int)(self.brightnessSlider.value*255)];
}

- (IBAction)constrastValueChanged:(id)sender
{
    self.contrastLabel.text = [NSString stringWithFormat:@"%d", (int)(self.contrastSlider.value*255)];
}

- (IBAction)saturationValueChanged:(id)sender
{
    self.saturationLabel.text = [NSString stringWithFormat:@"%d", (int)(self.saturationSlider.value*255)];
}

- (IBAction)startTimeValueChanged:(id)sender
{
    self.startTimeLabel.text = [StringTools formatTimeMs:self.startTimeSlider.value];
    //
    self.endTimeSlider.minimumValue = self.startTimeSlider.value+1000;
    self.endTimeSlider.maximumValue = self.endTimeSlider.minimumValue+[[NXEEngine instance].project getTotalTime];
    self.endTimeSlider.value = self.endTimeSlider.minimumValue;
    self.endTimeLabel.text = [StringTools formatTimeMs:self.endTimeSlider.minimumValue];
}

- (IBAction)endTimeValueChanged:(id)sender
{
    self.endTimeLabel.text = [StringTools formatTimeMs:self.endTimeSlider.value];
}

- (IBAction)alphaValueChanged:(id)sender
{
    self.alphaLabel.text = [NSString stringWithFormat:@"%d", (int)(100 * self.alphaSlider.value)];
}

- (IBAction)inAnimationValueChanged:(id)sender
{
    self.inAnimationTimeLabel.text = [StringTools formatTimeMs:self.inAnimationSlider.value];
}

- (IBAction)outAnimationValueChanged:(id)sender
{
    self.outAnimationTimeLabel.text = [StringTools formatTimeMs:self.outAnimationSlider.value];
}

- (IBAction)doNext:(id)sender
{
    imageLayer.inAnimation = (NXEInAnimationType)positionInAnimation;
    imageLayer.expression = (NXEExpressionType)positionExpression;
    imageLayer.outAnimation = (NXEOutAnimationType)positionOutAnimation;
    imageLayer.lutId = self.lutId;
    imageLayer.alpha = self.alphaSlider.value;
    
    [imageLayer setStartTime:self.startTimeSlider.value endTime:self.endTimeSlider.value];
    [imageLayer setBrightness:self.brightnessSlider.value contrast:self.contrastSlider.value saturation:self.saturationSlider.value];
    [imageLayer setInAnimationType:imageLayer.inAnimation duration:self.inAnimationSlider.value];
    [imageLayer setOutAnimationType:imageLayer.outAnimation duration:self.outAnimationSlider.value];
    [imageLayer setOverallAnimation:imageLayer.expression];
    
    self.refreshImageLayerTableView(positionLayer);
    //
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 10;
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueLayerInAnimation"]) {
        LayerAnimationTableViewController *aniTV = (LayerAnimationTableViewController *)[segue destinationViewController];
        [aniTV setupLayerAnimationWithCB:^(int position) {
            positionInAnimation = position;
            if(position == 0) {
                self.inAnimationSlider.value = 0;
            } else {
                if(self.inAnimationSlider.value == 0) {
                    self.inAnimationSlider.value = 1000;
                }
            }
            self.inAnimationLabel.text = [self getAnimation:positionInAnimation type:0];
            self.inAnimationTimeLabel.text = [StringTools formatTimeMs:self.inAnimationSlider.value];
        } type:0 selectedLayerAnimation:positionInAnimation];
    }
    else if([[segue identifier] isEqualToString:@"segueLayerExpression"]) {
        LayerExpressionTableViewController *expressTV = (LayerExpressionTableViewController *)[segue destinationViewController];
        [expressTV setupLayerExpressionWithCB:^(int position) {
            positionExpression = position;
            self.expressionLabel.text = [self getExpression:positionExpression];
        } selectedLayerExpression:positionExpression];
    }
    else if([[segue identifier] isEqualToString:@"segueLayerOutAnimation"]) {
        LayerAnimationTableViewController *aniTV = (LayerAnimationTableViewController *)[segue destinationViewController];
        [aniTV setupLayerAnimationWithCB:^(int position) {
            positionOutAnimation = position;
            if(position == 0) {
                self.outAnimationSlider.value = 0;
            } else {
                if(self.outAnimationSlider.value == 0) {
                    self.outAnimationSlider.value = 1000;
                }
            }
            self.outAnimationLabel.text = [self getAnimation:positionOutAnimation type:1];
            self.outAnimationTimeLabel.text = [StringTools formatTimeMs:self.outAnimationSlider.value];
        } type:1 selectedLayerAnimation:positionOutAnimation];
    }
    else if([[segue identifier] isEqualToString:@"segueColorFilter"]) {
        ColorFilterTableViewController *filterTV = (ColorFilterTableViewController *)[segue destinationViewController];
        
        __weak ImageLayerPropertyTableViewController *me = self;
        [filterTV setupColorFilterWithCB:^(NXELutID lutId, NSString *lutName) {
            me.lutId = lutId;
            me.lutName = lutName;
            me.colorFilterLabel.text = lutName;
        } selectedLUT:self.lutId];
    }
}

@end
