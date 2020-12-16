/******************************************************************************
 * File Name   :	ImagePropertyTableViewController.m
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

#import "ImagePropertyTableViewController.h"
#import "EffectItemTableViewController.h"
#import "ColorFilterTableViewController.h"
#import "Util.h"
#import "LUTNames.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

#define COLORADJ_INTMAX (255)

@interface ImagePropertyTableViewController ()

@property (copy, nonatomic) void (^refreshImageProperty)();
@property (nonatomic, retain) NXEClip *clip;
@property (nonatomic) NXELutID lutId;
@property (nonatomic, strong) NSString *lutName;

@end

@implementation ImagePropertyTableViewController

- (void)setupImagePropertyListCB:(refreshImageProperty)refreshImageProperty clip:(NXEClip *)clip
{
    self.refreshImageProperty = refreshImageProperty;
    self.clip = clip;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Clip Effect
    self.clipEffectLabel.text = self.clip.clipEffectID;
    
    // Transition Effect
    self.transitionEffectLabel.text = self.clip.transitionEffectID;
    
    // Color Filter
    self.lutId = self.clip.lutId;
    self.lutName = [LUTNames nameForLutId:self.lutId];
    self.colorFilterLabel.text = self.lutName;
    
    // Effect Duration
    int showStartTime=0; int showEndTime=0;
    [self.clip getEffectShowTime:&showStartTime endTime:&showEndTime];
    self.effectDurationSlider.minimumValue = showEndTime <= 0 ? 0 : 5000;
    self.effectDurationSlider.maximumValue = showEndTime <= 0 ? 0 : 20000;
    self.effectDurationSlider.value = showEndTime;
    self.effectDurationLabel.text = [StringTools formatTimeMs:showEndTime];
    
    // Clip Duration
    self.clipDurationSlider.minimumValue = 5000;
    self.clipDurationSlider.maximumValue = 12000;
    self.clipDurationSlider.value = self.clip.durationMs;
    self.clipDurationLabel.text = [StringTools formatTimeMs:self.clip.durationMs];
    
    // Color Adjuestment
    
    NXEColorAdjustments colorAdjustment = self.clip.colorAdjustment;
    const float colorMax = 1.0;
    const float colorMin = -1.0;
    UISlider *sliders[3] = {self.brightSlider, self.contrastSlider, self.saturationSlider};
    UILabel *labels[3] = {self.brightLabel, self.contrastLabel, self.saturationLabel};
    float values[3] = {
        colorAdjustment.brightness,
        colorAdjustment.contrast,
        colorAdjustment.saturation
    };
    
    for(int i = 0; i < 3; i++) {
        sliders[i].minimumValue = colorMin;
        sliders[i].maximumValue = colorMax;
        sliders[i].value = values[i];
        labels[i].text = [NSString stringWithFormat:@"%d", (int)(values[i] * COLORADJ_INTMAX)];
    }
    
    // Vignette
    [self.vignetteSwitch setOn:self.clip.vinfo.vignette];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 7;
}

#pragma mark - listener
- (IBAction)effectDurationChanged:(id)sender
{
    self.effectDurationLabel.text = [StringTools formatTimeMs:self.effectDurationSlider.value];
}

- (IBAction)clipDurationChanged:(id)sender
{
    self.clipDurationLabel.text = [StringTools formatTimeMs:self.clipDurationSlider.value];
}

- (IBAction)brightValueChanged:(id)sender
{
    self.brightLabel.text = [NSString stringWithFormat:@"%d", (int) (self.brightSlider.value * COLORADJ_INTMAX)];
}

- (IBAction)contrastValueChanged:(id)sender
{
    self.contrastLabel.text = [NSString stringWithFormat:@"%d", (int) (self.contrastSlider.value * COLORADJ_INTMAX)];
}

- (IBAction)saturationValueChanged:(id)sender
{
    self.saturationLabel.text = [NSString stringWithFormat:@"%d", (int) (self.saturationSlider.value  * COLORADJ_INTMAX)];
}

- (IBAction)doNext:(id)sender
{
    self.clip.colorAdjustment = NXEColorAdjustmentsMake(self.brightSlider.value, self.contrastSlider.value, self.saturationSlider.value);
    [self.clip setEffectShowTime:0 endTime:self.effectDurationSlider.value];
    [self.clip setImageClipDuration:self.clipDurationSlider.value];
    self.clip.lutId = self.lutId;
    [self.clip setVignette:self.vignetteSwitch.on];

    self.refreshImageProperty();
    //
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - Navigation
// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueEffect"]) {
        int itemType = (int)self.tableView.indexPathForSelectedRow.row;
        EffectItemTableViewController *evc = [segue destinationViewController];
        [evc setupEffectListrWithCB:^(int type, NSString *effectNaming){
            if(type == 0) {
                self.clipEffectLabel.text = effectNaming;
            } else {
                self.transitionEffectLabel.text = effectNaming;
            }
            // Effect Duration
            int showStartTime=0; int showEndTime=0;
            [self.clip getEffectShowTime:&showStartTime endTime:&showEndTime];
            self.effectDurationSlider.minimumValue = showEndTime <= 0 ? 0 : 5000;
            self.effectDurationSlider.maximumValue = showEndTime <= 0 ? 0 : 20000;
            self.effectDurationSlider.value = showEndTime;
            self.effectDurationLabel.text = [StringTools formatTimeMs:showEndTime];
        } itemType:itemType clip:self.clip];
    } else if([[segue identifier] isEqualToString:@"segueColorFilter"]) {
        ColorFilterTableViewController *cvc = [segue destinationViewController];
        __weak ImagePropertyTableViewController *me = self;
        [cvc setupColorFilterWithCB:^(NXELutID lutId, NSString *lutName){
            me.lutId = lutId;
            me.lutName = lutName;
            me.colorFilterLabel.text = lutName;
        } selectedLUT:self.lutId];
    }
}

#pragma mark - Unwind Segue

- (IBAction)prepareForUnwindToSetProperty:(UIStoryboardSegue *)segue {
    // Clip Effect
    self.clipEffectLabel.text = self.clip.clipEffectID;
    
    // Effect Duration
    int showStartTime=0; int showEndTime=0;
    [self.clip getEffectShowTime:&showStartTime endTime:&showEndTime];
    self.effectDurationSlider.minimumValue = showEndTime == 0 ? 0 : 5000;
    self.effectDurationSlider.maximumValue = showEndTime == 0 ? 0 : 20000;
    self.effectDurationSlider.value = showEndTime;
    self.effectDurationLabel.text = [StringTools formatTimeMs:showEndTime];
    
    // Transition Effect
    self.transitionEffectLabel.text = self.clip.transitionEffectID;
}

@end
