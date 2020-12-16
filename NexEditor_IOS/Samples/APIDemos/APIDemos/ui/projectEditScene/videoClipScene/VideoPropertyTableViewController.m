/******************************************************************************
 * File Name   :    VideoPropertyTableViewController.m
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

#import "VideoPropertyTableViewController.h"
#import "EffectItemTableViewController.h"
#import "ColorFilterTableViewController.h"
#import "AudioChangerTableViewController.h"
#import "Util.h"
#import "LUTNames.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

#define COLORADJ_INTMAX (255)

@interface VideoPropertyTableViewController ()

@property (copy, nonatomic) void (^refreshVideoProperty)();
@property (nonatomic, retain) NSMutableArray* envelopList;
@property (nonatomic, retain) NXEClip *clip;
@property (nonatomic) NXELutID lutId;
@property (nonatomic, strong) NSString *lutName;

@end

@implementation VideoPropertyTableViewController {
    int sTrimTime;
    int eTrimTime;
    int voiceChanger;
}

- (void)setupVideoPropertyListCB:(refreshVideoProperty)refreshVideoProperty clip:(NXEClip *)clip
{
    self.refreshVideoProperty = refreshVideoProperty;
    self.clip = clip;
}

- (int)getSpeedValue:(int)value
{
    int speed = 0;
    
    if(25 <= value && value < 50) {
        speed = 25;
    } else if(50 <= value && value < 100) {
        speed = 50;
    } else if(100 <= value && value < 150) {
        speed = 100;
    } else if(150 <= value && value < 200) {
        speed = 150;
    } else if(150 <= value && value < 400) {
        speed = 200;
    } else {
        speed = 400;
    }
    
    return speed;
}

- (void)dealloc
{
    self.envelopList = nil;
    self.clip = nil;
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
    self.lutName = [LUTNames nameForLutId: self.lutId];
    self.colorFilterLabel.text = self.lutName;
    
    // Color Adjustment
    
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
    
    // Speed Control
    self.speedSlider.minimumValue = 25;
    self.speedSlider.maximumValue = 400;
    self.speedSlider.value = self.clip.vinfo.speedControl;
    self.speedValueLabel.text = [NSString stringWithFormat:@"%d", (int)self.speedSlider.value];
    
    // Volume
    self.volumeSlider.minimumValue = 0;
    self.volumeSlider.maximumValue = 200;
    self.volumeSlider.value = self.clip.vinfo.clipVolume;
    self.volumeLabel.text = [NSString stringWithFormat:@"%d", (int)self.volumeSlider.value];
    
    // Trim
    sTrimTime = self.clip.headTrimDuration;
    eTrimTime = self.clip.trimEndTime;
    
    self.trimSlider.value = 0.0;
    self.trimSlider.minimumValue = 0.0;
    self.trimSlider.maximumValue = self.clip.durationMs-1000;// End Time과 최소 1초간의 margin을 잡기위해 1000을 차감한다.
    self.trimTimeLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    self.trimStartLabel.text = [StringTools formatTimeMs:sTrimTime];
    self.trimEndLabel.text = [StringTools formatTimeMs:eTrimTime];
    self.etrimButton.enabled = NO;
    
    // Envelop
    
    // i.   Controller
    self.envelopValueSlider.value = 0;
    self.envelopValue.text = @"0";
    
    self.envelopTimeSlider.value = 0;
    self.envelopTimeSlider.minimumValue = 0;
    self.envelopTimeSlider.maximumValue = self.clip.durationMs;
    self.envelopTimeValue.text = @"00:00";
    
    self.envelopList = [[NSMutableArray alloc] init];
    
    // ii.  Values
    int loopid = 0;
    
    for(NSNumber *time in self.clip.audioEnvelop.volumeEnvelopeTime) {
        int time_ = (int)[time integerValue];
        int level_ = (int)[[self.clip.audioEnvelop.volumeEnvelopeLevel objectAtIndex:loopid++] integerValue];
        
        EnvelopInfo *envelopInfo = [[EnvelopInfo alloc] initWithTime:time_ level:level_];
        [self.envelopList addObject:envelopInfo];
    }
    
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.envelopList];
    self.envelopTextView.selectable = NO;
    
    // Effect Duration
    int showStartTime=0; int showEndTime=0;
    [self.clip getEffectShowTime:&showStartTime endTime:&showEndTime];
    self.effectDurationSlider.minimumValue = showEndTime <= 0 ? 0 : 5000;
    self.effectDurationSlider.maximumValue = showEndTime <= 0 ? 0 : 20000;
    self.effectDurationSlider.value = showEndTime;
    self.effectDurationLabel.text = [StringTools formatTimeMs:showEndTime];
    
    // Compressor
    [self.compressorSwitch setOn:self.clip.vinfo.compressor];
    
    // Pan
    self.panRightSlider.value = self.clip.vinfo.panRight;
    self.panLeftSlider.value = self.clip.vinfo.panLeft;
    self.panRightValue.text = [NSString stringWithFormat:@"%d", self.clip.vinfo.panRight];
    self.panLeftValue.text = [NSString stringWithFormat:@"%d", self.clip.vinfo.panLeft];
    
    // Pitch
    self.pitchSlider.value = self.clip.vinfo.pitch;
    self.pitchValue.text = [NSString stringWithFormat:@"%d", self.clip.vinfo.pitch];
    
    // AudioChanger
    NSArray *changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
    voiceChanger = self.clip.vinfo.voiceChanger;
    self.audioChangerValue.text = [changerList objectAtIndex:voiceChanger];
}

#pragma mark - listener

- (IBAction)effectDurationSlider:(id)sender
{
    self.effectDurationLabel.text = [StringTools formatTimeMs:self.effectDurationSlider.value];
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

- (IBAction)volumeValueChanged:(id)sender
{
    self.volumeLabel.text = [NSString stringWithFormat:@"%d", (int)self.volumeSlider.value];
}

- (IBAction)doNext:(id)sender
{
    self.clip.colorAdjustment = NXEColorAdjustmentsMake(self.brightSlider.value, self.contrastSlider.value, self.saturationSlider.value);
    if(self.compressorSwitch.on) {
        [self.clip setCompressorValue:4];
    } else {
        [self.clip setCompressorValue:0];
    }
    [self.clip setPitchValue:self.pitchSlider.value];
    [self.clip setPanLeftValue:self.panLeftSlider.value];
    [self.clip setPanRightValue:self.panRightSlider.value];
    [self.clip setSpeed:[self getSpeedValue:self.speedSlider.value]];
    [self.clip setClipVolume:self.volumeSlider.value];
    [self.clip setEffectShowTime:0 endTime:self.effectDurationSlider.value];
    self.clip.lutId = self.lutId;
    [self.clip setVoiceChangerValue:voiceChanger];
    [self.clip setTrim:sTrimTime EndTrimTime:eTrimTime];
    [self.clip setVignette:self.vignetteSwitch.on];
    for(EnvelopInfo *info in self.envelopList) {
        [self.clip.audioEnvelop addVolumeEnvelop:(int)info.time level:(int)info.level];
    }
    
    self.refreshVideoProperty();
    //
    [self.navigationController popViewControllerAnimated:YES];
}

- (IBAction)trimResetBtn:(id)sender
{
    self.trimSlider.minimumValue = 0;
    self.trimSlider.value = 0;
    self.trimTimeLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    self.trimStartLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    self.trimEndLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    self.etrimButton.enabled = NO;
    //
    sTrimTime = 0;
    eTrimTime = 0;
}

- (IBAction)trimStartSetBtn:(id)sender
{
    self.trimStartLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    sTrimTime = self.trimSlider.value;
    eTrimTime = sTrimTime + 1000;
    
    // Start Time과 최소 1초간의 margin을 잡기위해 1000을 더하고, 관련 Label에 시간정보를 보여주도록 합니다.
    self.trimSlider.minimumValue = self.trimSlider.value+1000;
    self.trimSlider.value = self.trimSlider.minimumValue;
    self.trimTimeLabel.text = [StringTools formatTimeMs:self.trimSlider.minimumValue];
    self.trimEndLabel.text = [StringTools formatTimeMs:self.trimSlider.minimumValue];
    self.etrimButton.enabled = YES;
}

- (IBAction)trimEndSetBtn:(id)sender
{
    self.trimEndLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    //
    eTrimTime = self.trimSlider.value;
}

- (IBAction)trimValueChanged:(id)sender
{
    if(self.etrimButton.enabled) {
        self.trimEndLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    } else {
        self.trimStartLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    }
    self.trimTimeLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
}

- (IBAction)speedValueChanged:(id)sender
{
    int speed = [self getSpeedValue:self.speedSlider.value];
    //
    self.speedValueLabel.text = [NSString stringWithFormat:@"%d", speed];
}

- (IBAction)resetEnvelop:(id)sender
{
    // NESI-253
    [self.clip.audioEnvelop removeVolumeEnvelop];
    //
    
    self.envelopList = [[EnvelopInfo defaultListWithDuration:self.clip.durationMs] mutableCopy];
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.envelopList];
}

- (IBAction)setEnvelop:(id)sender
{
    EnvelopInfo *newInfo = [[EnvelopInfo alloc] initWithTime:self.envelopTimeSlider.value
                                                       level:self.envelopValueSlider.value];

    self.envelopList = [[EnvelopInfo mergeList:self.envelopList with:newInfo] mutableCopy];
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.envelopList];
}

- (IBAction)envelopTimeValueChanged:(id)sender
{
    self.envelopTimeValue.text = [StringTools formatTimeMs:self.envelopTimeSlider.value];
}

- (IBAction)envelopValueChanged:(id)sender
{
    self.envelopValue.text = [NSString stringWithFormat:@"%d", (int)self.envelopValueSlider.value];
}

- (IBAction)panLeftValueChanged:(id)sender
{
    self.panLeftValue.text = [NSString stringWithFormat:@"%d", (int)self.panLeftSlider.value];
}

- (IBAction)panRightValueChanged:(id)sender
{
    self.panRightValue.text = [NSString stringWithFormat:@"%d", (int)self.panRightSlider.value];
}

- (IBAction)pitchValueChanged:(id)sender
{
    self.pitchValue.text = [NSString stringWithFormat:@"%d", (int)self.pitchSlider.value];
}

#pragma mark - Navigation
// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueEffect"]) {
        int itemType = (int)self.tableView.indexPathForSelectedRow.row;
        EffectItemTableViewController *evc = [segue destinationViewController];
        [evc setupEffectListrWithCB:^(int type, NSString *effectNaming) {
            if(type == 0) {
                self.clipEffectLabel.text = effectNaming;
                [self.clip setClipEffect:effectNaming];
            } else {
                self.transitionEffectLabel.text = effectNaming;
                [self.clip setTransitionEffect:effectNaming];
            }
            // Effect Duration
            int showStartTime=0; int showEndTime=0;
            [self.clip getEffectShowTime:&showStartTime endTime:&showEndTime];
            self.effectDurationSlider.minimumValue = showEndTime <= 0 ? 0 : 5000;
            self.effectDurationSlider.maximumValue = showEndTime <= 0 ? 0 : 20000;
            self.effectDurationSlider.value = showEndTime;
            self.effectDurationLabel.text = [StringTools formatTimeMs:showEndTime];
        }
                           itemType:itemType
                               clip:self.clip];
    } else if([[segue identifier] isEqualToString:@"segueColorFilter"]) {
        ColorFilterTableViewController *cvc = [segue destinationViewController];
        
        __weak VideoPropertyTableViewController *me = self;
        [cvc setupColorFilterWithCB:^(NXELutID lutId, NSString *lutName) {
            me.lutId = lutId;
            me.lutName = lutName;
            me.colorFilterLabel.text = lutName;
        } selectedLUT:self.lutId];
    } else if([[segue identifier] isEqualToString:@"segueAudioChanger"]) {
        __block NSArray *changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
        
        [(AudioChangerTableViewController *)[segue destinationViewController]setupAudioChangerListWithCB:^(NSString *value) {
            voiceChanger = (int)[changerList indexOfObject:value];
            self.audioChangerValue.text = value;
        } selectedValue:[changerList objectAtIndex:voiceChanger]];
    }
}

#pragma mark - Unwind Segue

- (IBAction)prepareForUnwindToSetProperty:(UIStoryboardSegue *)segue {
    // Clip Effect
    self.clipEffectLabel.text = self.clip.clipEffectID;

    // Effect Duration
    int showStartTime=0; int showEndTime=0;
    [self.clip getEffectShowTime:&showStartTime endTime:&showEndTime];
    self.effectDurationSlider.minimumValue = showEndTime <= 0 ? 0 : 5000;
    self.effectDurationSlider.maximumValue = showEndTime <= 0 ? 0 : 20000;
    self.effectDurationSlider.value = showEndTime;
    self.effectDurationLabel.text = [StringTools formatTimeMs:showEndTime];
    
    // Transition Effect
    self.transitionEffectLabel.text = self.clip.transitionEffectID;
}

@end
