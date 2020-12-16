/******************************************************************************
 * File Name   :	VideoLayerPropertyTableViewController.m
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

#import "VideoLayerPropertyTableViewController.h"
#import "LayerAnimationTableViewController.h"
#import "LayerExpressionTableViewController.h"
#import "AudioChangerTableViewController.h"
#import "ColorFilterTableViewController.h"
#import "Util.h"
#import "LUTNames.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@interface VideoLayerPropertyTableViewController ()

@property (nonatomic, copy) void(^refreshVideoLayerTableView)(int);
@property (nonatomic, retain) NSMutableArray* envelopList;
@property (nonatomic) NXELutID lutId;
@property (nonatomic, strong) NSString *lutName;
@end

@implementation VideoLayerPropertyTableViewController
{
    NXEVideoLayer *videoLayer;
    int positionLayer;
    int positionInAnimation;
    int positionExpression;
    int positionOutAnimation;
    int voiceChanger;
    int sTrimTime;
    int eTrimTime;
}

- (void)setupProperty:(NXEVideoLayer *)layer postion:(int)postion refreshListItem:(refreshListItem)refreshListItem
{
    videoLayer = layer;
    positionLayer = postion;
    positionInAnimation = videoLayer.inAnimation;
    positionExpression = videoLayer.expression;
    positionOutAnimation = videoLayer.outAnimation;
    self.lutId = videoLayer.lutId;
    self.lutName = [LUTNames nameForLutId:self.lutId];
    self.refreshVideoLayerTableView = refreshListItem;
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
    self.inAnimationSlider.value = videoLayer.inAnimationDuration;
    self.inAnimationTimeLabel.text =
    
    [StringTools formatTimeMs:videoLayer.inAnimationDuration];
    
    // Expression
    self.expressionLabel.text = [self getExpression:positionExpression];
    
    // Out Animation
    self.outAnimationLabel.text = [self getAnimation:positionOutAnimation type:1];
    self.outAnimationSlider.minimumValue = 0;
    self.outAnimationSlider.maximumValue = 5*1000;
    self.outAnimationSlider.value = videoLayer.outAnimationDuration;
    self.outAnimationTimeLabel.text = [StringTools formatTimeMs:videoLayer.outAnimationDuration];
    
    // ColorFilter
    self.colorFilterLabel.text = self.lutName;
    
    // Color Adjustment
    
    // i.   Bright
    self.brightnessLabel.text = [NSString stringWithFormat:@"%d", (int)(videoLayer.brightness*255)];
    self.brightnessSlider.minimumValue = -1;
    self.brightnessSlider.maximumValue = 1;
    self.brightnessSlider.value = videoLayer.brightness;
    
    // ii.  Contrast
    self.contrastLabel.text = [NSString stringWithFormat:@"%d", (int)(videoLayer.contrast*255)];
    self.contrastSlider.minimumValue = -1;
    self.contrastSlider.maximumValue = 1;
    self.contrastSlider.value = videoLayer.contrast;
    
    // iii. Saturation
    self.saturationLabel.text = [NSString stringWithFormat:@"%d", (int)(videoLayer.saturation*255)];
    self.saturationSlider.minimumValue = -1;
    self.saturationSlider.maximumValue = 1;
    self.saturationSlider.value = videoLayer.saturation;
    
    // Trim
    sTrimTime = videoLayer.layerClip.headTrimDuration;
    eTrimTime = videoLayer.layerClip.trimEndTime;
    
    self.trimSlider.value = 0.0;
    self.trimSlider.minimumValue = 0.0;
    self.trimSlider.maximumValue = videoLayer.layerClip.durationMs-1000;// End Time과 최소 1초간의 margin을 잡기위해 1000을 차감한다.
    self.trimTimeLabel.text = [StringTools formatTimeMs:self.trimSlider.value];
    self.trimStartLabel.text = [StringTools formatTimeMs:sTrimTime];
    self.trimEndLabel.text = [StringTools formatTimeMs:eTrimTime];
    self.etrimButton.enabled = NO;
    
    // Display Start Time
    self.startTimeLabel.text = [StringTools formatTimeMs:videoLayer.startTime];
    self.startTimeSlider.minimumValue = 0;
    self.startTimeSlider.maximumValue = [[NXEEngine instance].project getTotalTime];
    self.startTimeSlider.value = videoLayer.startTime;
    
    // Display End Time
    self.endTimeLabel.text = [StringTools formatTimeMs:videoLayer.endTime];
    self.endTimeSlider.minimumValue = self.startTimeSlider.value+1000;
    self.endTimeSlider.maximumValue = self.endTimeSlider.minimumValue+videoLayer.layerClip.durationMs;
    self.endTimeSlider.value = videoLayer.endTime;
    
    // Alpha
    self.alphaLabel.text = [NSString stringWithFormat:@"%d", (int)(100 * videoLayer.alpha)];
    self.alphaSlider.minimumValue = 0;
    self.alphaSlider.maximumValue = 1;
    self.alphaSlider.value = videoLayer.alpha;
    
    // Volume
    self.volumeLabel.text = [NSString stringWithFormat:@"%d", videoLayer.layerClip.vinfo.clipVolume];
    self.volumeSlider.minimumValue = 0;
    self.volumeSlider.maximumValue = 200;
    self.volumeSlider.value = videoLayer.layerClip.vinfo.clipVolume;
    
    // Envelop
    
    // i.   Controller
    self.envelopValueSlider.value = 0;
    self.envelopValue.text = @"0";
    
    self.envelopTimeSlider.value = 0;
    self.envelopTimeSlider.minimumValue = 0;
    self.envelopTimeSlider.maximumValue = videoLayer.layerClip.durationMs;
    self.envelopTimeValue.text = @"00:00";
    
    self.envelopList = [[NSMutableArray alloc] init];
    
    // ii.  Values
    int loopid = 0;
    
    for(NSNumber *time in videoLayer.layerClip.audioEnvelop.volumeEnvelopeTime) {
        int time_ = (int)[time integerValue];
        int level_ = (int)[[videoLayer.layerClip.audioEnvelop.volumeEnvelopeLevel objectAtIndex:loopid++] integerValue];
        
        EnvelopInfo *envelopInfo = [[EnvelopInfo alloc] initWithTime:time_ level:level_];
        [self.envelopList addObject:envelopInfo];
    }
    
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.envelopList];
    self.envelopTextView.selectable = NO;
    
    // Compressor
    [self.compressorSwitch setOn:videoLayer.layerClip.vinfo.compressor];

    // Pan
    self.panRightSlider.value = videoLayer.layerClip.vinfo.panRight;
    self.panLeftSlider.value = videoLayer.layerClip.vinfo.panLeft;
    self.panRightValue.text = [NSString stringWithFormat:@"%d", videoLayer.layerClip.vinfo.panRight];
    self.panLeftValue.text = [NSString stringWithFormat:@"%d", videoLayer.layerClip.vinfo.panLeft];
    
    // Pitch
    self.pitchSlider.value = videoLayer.layerClip.vinfo.pitch;
    self.pitchValue.text = [NSString stringWithFormat:@"%d", videoLayer.layerClip.vinfo.pitch];
    
    // AudioChanger
    NSArray *changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
    voiceChanger = videoLayer.layerClip.vinfo.voiceChanger;
    self.audioChangerValue.text = [changerList objectAtIndex:voiceChanger];
}

#pragma mark - listener

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

- (IBAction)doNext:(id)sender
{
    videoLayer.inAnimation = (NXEInAnimationType)positionInAnimation;
    videoLayer.expression = (NXEExpressionType)positionExpression;
    videoLayer.outAnimation = (NXEOutAnimationType)positionOutAnimation;
    videoLayer.lutId = self.lutId;
    videoLayer.alpha = self.alphaSlider.value;
    
    [videoLayer setAlpha:self.alphaSlider.value];
    [videoLayer setStartTime:self.startTimeSlider.value endTime:self.endTimeSlider.value];
    [videoLayer setBrightness:self.brightnessSlider.value contrast:self.contrastSlider.value saturation:self.saturationSlider.value];
    [videoLayer setInAnimationType:videoLayer.inAnimation duration:self.inAnimationSlider.value];
    [videoLayer setOutAnimationType:videoLayer.outAnimation duration:self.outAnimationSlider.value];
    [videoLayer setOverallAnimation:videoLayer.expression];
    
    if(self.compressorSwitch.on) {
        [videoLayer.layerClip setCompressorValue:4];
    } else {
        [videoLayer.layerClip setCompressorValue:0];
    }
    [videoLayer.layerClip setDisplayStartTime:self.startTimeSlider.value
                                      endTime:self.endTimeSlider.value];
    [videoLayer.layerClip setTrim:sTrimTime EndTrimTime:eTrimTime];
    [videoLayer.layerClip setPitchValue:self.pitchSlider.value];
    [videoLayer.layerClip setPanLeftValue:self.panLeftSlider.value];
    [videoLayer.layerClip setPanRightValue:self.panRightSlider.value];
    [videoLayer.layerClip setClipVolume:self.volumeSlider.value];
    [videoLayer.layerClip setVoiceChangerValue:voiceChanger];
    for(EnvelopInfo *info in self.envelopList) {
        [videoLayer.layerClip.audioEnvelop addVolumeEnvelop:(int)info.time level:(int)info.level];
    }
    
    self.refreshVideoLayerTableView(positionLayer);
    //
    [self.navigationController popViewControllerAnimated:YES];
}

- (IBAction)setEnvelop:(id)sender
{
    EnvelopInfo *newInfo = [[EnvelopInfo alloc] initWithTime:self.envelopTimeSlider.value
                                                              level:self.envelopValueSlider.value];
    self.envelopList = [[EnvelopInfo mergeList:self.envelopList with:newInfo] mutableCopy];
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.envelopList];
}

- (IBAction)resetEnvelop:(id)sender
{
    // NESI-253
    [videoLayer.layerClip.audioEnvelop removeVolumeEnvelop];
    //
    self.envelopList = [[EnvelopInfo defaultListWithDuration:videoLayer.layerClip.durationMs] mutableCopy];
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

- (IBAction)startTimeValueChanged:(id)sender
{
    self.startTimeLabel.text = [StringTools formatTimeMs:self.startTimeSlider.value];
    //
    self.endTimeSlider.minimumValue = self.startTimeSlider.value+1000;
    self.endTimeSlider.maximumValue = self.endTimeSlider.minimumValue + videoLayer.layerClip.durationMs;
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

- (IBAction)volumeValueChanged:(id)sender
{
    self.volumeLabel.text = [NSString stringWithFormat:@"%d", (int)self.volumeSlider.value];
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

- (IBAction)brightnessValueChanged:(id)sender
{
    self.brightnessLabel.text = [NSString stringWithFormat:@"%d", (int)(self.brightnessSlider.value*255)];
}

- (IBAction)contrastValueChanged:(id)sender
{
    self.contrastLabel.text = [NSString stringWithFormat:@"%d", (int)(self.contrastSlider.value*255)];
}

- (IBAction)saturationValueChanged:(id)sender
{
    self.saturationLabel.text = [NSString stringWithFormat:@"%d", (int)(self.saturationSlider.value*255)];
}

- (IBAction)inAnimationValueChanged:(id)sender
{
    self.inAnimationTimeLabel.text = [StringTools formatTimeMs:self.inAnimationSlider.value];
}

- (IBAction)outAnimationValueChanged:(id)sender
{
    self.outAnimationTimeLabel.text = [StringTools formatTimeMs:self.outAnimationSlider.value];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 17;
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
        __weak VideoLayerPropertyTableViewController *me = self;
        [filterTV setupColorFilterWithCB:^(NXELutID lutId, NSString *lutName){
            me.lutId = lutId;
            me.lutName = lutName;
            me.colorFilterLabel.text = lutName;
        } selectedLUT:self.lutId];
    }
    else if([[segue identifier] isEqualToString:@"segueAudioChanger"]) {
        __block NSArray *changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
        NSString *changer = [changerList objectAtIndex:voiceChanger];
        AudioChangerTableViewController *changerTV = (AudioChangerTableViewController *)[segue destinationViewController];
        [changerTV setupAudioChangerListWithCB:^(NSString *changerIndex) {
            voiceChanger = (int)[changerList indexOfObject:changerIndex];
            self.audioChangerValue.text = changerIndex;
        } selectedValue:changer];
    }
}

@end
