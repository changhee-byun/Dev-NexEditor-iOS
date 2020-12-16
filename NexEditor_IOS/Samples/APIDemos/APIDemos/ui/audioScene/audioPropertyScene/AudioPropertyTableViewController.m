/******************************************************************************
 * File Name   :	AudioPropertyTableViewController.m
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

#import "AudioPropertyTableViewController.h"
#import "AudioChangerTableViewController.h"
#import "Util.h"
#import "APIDemos-Swift.h"

@interface AudioPropertyTableViewController ()

@property (copy, nonatomic) void(^refreshTableViewCallback)(AudioTrackInfo *trackInfo);
@property (nonatomic, assign) AudioTrackInfo *trackinfo;
@property (nonatomic, assign) int projectTime;

@end

@implementation AudioPropertyTableViewController {
    int audioChanger;
    UIAlertController *alertController;
}

- (void)setupProperty:(AudioTrackInfo *)audioTrackInfo projectTime:(int)projectTime refreshTableView:(refreshTableView)refreshTableView
{
    self.refreshTableViewCallback = refreshTableView;
    self.trackinfo = audioTrackInfo;
    self.projectTime = projectTime;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // AudioTrack loop
    //
    [self.loopSwitch setOn:self.trackinfo.loop];
    
    // AudioTrack volume
    //
    int volumeLevel = self.trackinfo.volumeLevel;
    if(self.trackinfo.volumeLevel < 0) volumeLevel = 0;
    else if(self.trackinfo.volumeLevel > 200) volumeLevel = 200;
    self.volumeValue.text = [NSString stringWithFormat:@"%d", self.trackinfo.volumeLevel];
    self.volumeSlider.value = volumeLevel;
    
    // Trim
    //
    self.trimValue.text = [StringTools formatTimeMs:self.trackinfo.trimStartTime];
    self.trimSlider.minimumValue = 0;
    self.trimSlider.maximumValue = self.trackinfo.duration;
    self.trimSlider.value = self.trackinfo.trimStartTime;
    
    // StartTime
    self.startTimeValue.text = [StringTools formatTimeMs:self.trackinfo.startTime];
    self.startTimeSlider.minimumValue = 0;
    self.startTimeSlider.maximumValue = self.projectTime;
    self.startTimeSlider.value = self.trackinfo.startTime;
    
    // Envelop
    //
    
    // i. Conrtoller
    //
    self.envelopValueSlider.value = 0;
    self.envelopValue.text = @"0";
    
    self.envelopTimeValue.text = @"00:00";
    self.envelopTimeSlider.minimumValue = 0;
    self.envelopTimeSlider.maximumValue = self.trackinfo.duration;
    self.envelopTimeSlider.value = 0;
    
    // ii. Values
    //
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.trackinfo.envelopList];
    self.envelopTextView.selectable = NO;
    
    // Compressor
    //
    [self.compressorSwitch setOn:self.trackinfo.compressor];
    
    // Pan
    //
    self.panRightSlider.value = self.trackinfo.panRight;
    self.panLeftSlider.value = self.trackinfo.panLeft;
    self.panRightValue.text = [NSString stringWithFormat:@"%d", self.trackinfo.panRight];
    self.panLeftValue.text = [NSString stringWithFormat:@"%d", self.trackinfo.panLeft];
    
    // Pitch
    //
    self.pitchSlider.value = self.trackinfo.pitch;
    self.pitchValue.text = [NSString stringWithFormat:@"%d", self.trackinfo.pitch];
    
    // AudioChanger
    NSArray *changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
    self.audioChangerValue.text = [changerList objectAtIndex:self.trackinfo.audioChanger];
    //
    audioChanger = self.trackinfo.audioChanger;
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 9;
}

#pragma mark - listener

- (IBAction)resetEnvelop:(id)sender
{    
    self.trackinfo.envelopList = [[EnvelopInfo defaultListWithDuration:self.trackinfo.duration] mutableCopy];
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.trackinfo.envelopList];
}

- (IBAction)setEnvelop:(id)sender
{
    EnvelopInfo *newInfo = [[EnvelopInfo alloc] initWithTime:self.envelopTimeSlider.value
                                                       level:self.envelopValueSlider.value];
    self.trackinfo.envelopList = [[EnvelopInfo mergeList:self.trackinfo.envelopList with:newInfo] mutableCopy];
    self.envelopTextView.text = [EnvelopInfo descriptionOfList:self.trackinfo.envelopList];
}

- (IBAction)volumeValueChanged:(id)sender
{
    self.volumeValue.text = [NSString stringWithFormat:@"%d", (int)self.volumeSlider.value];
}

- (IBAction)trimValueChanged:(id)sender
{
    self.trimValue.text = [StringTools formatTimeMs:self.trimSlider.value];
}

- (IBAction)trimSelectA:(id)sender
{
    self.trackinfo.trimStartTime = 0;
    self.trackinfo.trimEndTime = self.trimSlider.value;
}

- (IBAction)trimSelectB:(id)sender
{
    if(self.trimSlider.value == 0) {
        self.trackinfo.trimStartTime = 0;
        self.trackinfo.trimEndTime = self.trimSlider.value;
    } else {
        // NESI-243
        if(self.trimSlider.value == self.trackinfo.duration) {
            alertController = [UIAlertController alertControllerWithTitle:@"Error"
                                                                  message:@"You can't split audio clip as the same value as the duration of audio clip."
                                                           preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK"
                                                             style:UIAlertActionStyleDefault
                                                           handler:^(UIAlertAction * _Nonnull action) {
                                                               [alertController dismissViewControllerAnimated:YES completion:nil];
                                                               alertController = nil;
                                                           }];
            [alertController addAction:ok];
            [self presentViewController:alertController animated:YES completion:nil];
            return;
        }
        //
        self.trackinfo.trimStartTime = self.trimSlider.value;
        self.trackinfo.trimEndTime = self.trackinfo.duration;
    }
}

- (IBAction)statTimeValueChanged:(id)sender
{
    self.startTimeValue.text = [StringTools formatTimeMs:self.startTimeSlider.value];
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

- (IBAction)doNext:(id)sender
{
    self.trackinfo.loop = self.loopSwitch.on;
    self.trackinfo.volumeLevel = self.volumeSlider.value;
    self.trackinfo.compressor = self.compressorSwitch.on;
    self.trackinfo.pitch = self.pitchSlider.value;
    self.trackinfo.panLeft = self.panLeftSlider.value;
    self.trackinfo.panRight = self.panRightSlider.value;
    self.trackinfo.startTime = self.startTimeSlider.value;
    self.trackinfo.audioChanger = audioChanger;
    
    self.refreshTableViewCallback(self.trackinfo);
    //
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - Navigation
// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueAudioChanger"]) {
        // AudioChanger
        NSArray *changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
        
        [(AudioChangerTableViewController *)[segue destinationViewController]setupAudioChangerListWithCB:^(NSString *value) {
            self.audioChangerValue.text = value;
            audioChanger = (int)[changerList indexOfObject:value];
        } selectedValue:[changerList objectAtIndex:self.trackinfo.audioChanger]];
    }
}

@end
