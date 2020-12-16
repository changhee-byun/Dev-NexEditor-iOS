/******************************************************************************
 * File Name   :	AudioTrackTableViewController.m
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

#import "AudioTrackTableViewController.h"
#import "AudioTrackDataController.h"
#import "CommonTypeCTableViewCell.h"
#import "AudioFileListTableViewController.h"
#import "AudioPropertyTableViewController.h"
#import "PreviewViewController.h"
#import "Util.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;


@interface AudioTrackTableViewController ()

@property (nonatomic, retain) AudioTrackDataController *audioTrackDataController;
/// Maximum number of audio tracks can be added
@property (nonatomic) NSInteger maxTracks;
@property (nonatomic, assign) int selected_idx;
@property (nonatomic, assign) int projectTime;

@end

@implementation AudioTrackTableViewController

- (void)setupAudioList:(NSArray *)audioList
           projectTime:(int)projectTime
            titleArray:(NSArray *)titleArray
             maxTracks:(NSInteger)maxTracks
{
    self.maxTracks = maxTracks;
    self.projectTime = projectTime;
    //
    self.audioTrackDataController = [[AudioTrackDataController alloc] init];
    
    int loopid = 0;
    
    for(NXEClip *audioClip in audioList) {
        //
        AudioTrackInfo *trackInfo = [[AudioTrackInfo alloc] init];
        trackInfo.clipSource = audioClip.clipSource;
        trackInfo.title = (NSString *)[titleArray objectAtIndex:loopid];
        trackInfo.loop = audioClip.bLoop;
        trackInfo.volumeLevel = audioClip.ainfo.clipVolume;
        trackInfo.trimStartTime = audioClip.headTrimDuration;
        trackInfo.trimEndTime = audioClip.trimEndTime;
        trackInfo.startTime = audioClip.ainfo.startTime;
        trackInfo.duration = audioClip.durationMs;
        trackInfo.pitch = audioClip.ainfo.pitch;
        trackInfo.audioChanger = audioClip.ainfo.voiceChanger;
        trackInfo.panRight = audioClip.ainfo.panRight;
        trackInfo.panLeft = audioClip.ainfo.panLeft;
        trackInfo.envelopList = [[NSMutableArray alloc] init];
        trackInfo.compressor = audioClip.ainfo.compressor == 0 ? false : true;
        //
        NSArray *envelopTimeList = nil;
        NSArray *envelopLevelList = nil;
        EnvelopInfo *envelopInfo;
        
        envelopTimeList = audioClip.audioEnvelop.volumeEnvelopeTime;
        envelopLevelList = audioClip.audioEnvelop.volumeEnvelopeLevel;
        
        if(envelopTimeList != nil && envelopLevelList != nil) {
            int loopid2 = 0;
            for(NSNumber *time in envelopTimeList) {
                envelopInfo = [[EnvelopInfo alloc] initWithTime:[time integerValue]
                                                          level:[[envelopLevelList objectAtIndex:loopid2] integerValue]];
                [trackInfo.envelopList addObject:envelopInfo];
                loopid2++;
            }
        }
        [self.audioTrackDataController.audioTrackList addObject:trackInfo];
        //
        loopid++;
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.audioTrackDataController.audioTrackList count];
}

- (CommonTypeCTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeCTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"TrackCell" forIndexPath:indexPath];
    
    AudioTrackInfo *audioTrackInfo = (AudioTrackInfo *)[_audioTrackDataController.audioTrackList objectAtIndex:indexPath.row];
    
    NSString *envelopText = @"None";
    
    int count = (int)[audioTrackInfo.envelopList count];
    if(count > 0) {
        EnvelopInfo *envelop = [audioTrackInfo.envelopList objectAtIndex:0];
        
        envelopText = envelop.description;
        if(count > 1) {
            envelopText = [envelopText stringByAppendingString:@"..."];
        }
    }
    
    NSString *trimTimeinfo = nil;
    if(audioTrackInfo.trimStartTime == 0 && audioTrackInfo.trimEndTime == 0) {
        trimTimeinfo = @"None";
    } else {
        trimTimeinfo = [NSString stringWithFormat:@"from %@ to %@", [StringTools formatTimeMs:audioTrackInfo.trimStartTime], [StringTools formatTimeMs:audioTrackInfo.trimEndTime]];
    }
    
    NSString *contentText = nil;
    contentText = [NSString stringWithFormat:@"Loop:%@,Volume:%d,Duration:%@,\nStartTime:%@,Trim:%@,\nCompressor:%@,Pan:L/%@,R/%@,Pitch:%d,\nAudioChanger:%d,Envelop:%@",
                   audioTrackInfo.loop ? @"YES" : @"NO",
                   audioTrackInfo.volumeLevel,
                   [StringTools formatTimeMs:audioTrackInfo.duration],
                   [StringTools formatTimeMs:audioTrackInfo.startTime],
                   trimTimeinfo,
                   audioTrackInfo.compressor ? @"YES" : @"NO",
                   [NSString stringWithFormat:@"%d", audioTrackInfo.panLeft],
                   [NSString stringWithFormat:@"%d", audioTrackInfo.panRight],
                   audioTrackInfo.pitch,
                   audioTrackInfo.audioChanger,
                   envelopText == nil ? @"none" : envelopText];
    
    cell.TableLabel.text = audioTrackInfo.title;
    cell.ContentLabel.text = contentText;
    
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 120;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    self.selected_idx = (int)indexPath.row;
}

#pragma mark - listener

- (IBAction)doDeleteCell:(id)sender
{
    if(((UILongPressGestureRecognizer *)sender).state == UIGestureRecognizerStateBegan) {
        CGPoint p = [((UILongPressGestureRecognizer *)sender) locationInView:self.tableView];
        NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:p];
        //
        NSString *trackName = [(AudioTrackInfo *)[_audioTrackDataController.audioTrackList objectAtIndex:indexPath.row] title];
        NSString *message = [NSString stringWithFormat:@"Delete Audio Track '%@'?", trackName];
        
        [self alertSimpleActionSheet:@"Confirm"
                             message:message
                        actionTitles:@[@"Delete"]
                        dismissTitle:@"Cancel"
                            complete:^(NSInteger index) {
                                if (index == 0) {
                                    [self.audioTrackDataController.audioTrackList removeObjectAtIndex:indexPath.row];
                                    [self.tableView reloadData];
                                }
                            }];
    }
}

- (IBAction)doFileList:(id)sender
{
    NSInteger maxTracks = self.maxTracks;
    
    if ( self.audioTrackDataController.audioTrackList.count < maxTracks) {
        [self performSegueWithIdentifier:@"segueAudioFileList" sender:self];
    } else {
        NSString *message = [NSString stringWithFormat:@"Cannot add more than %d audio tracks", (int) maxTracks];
        [self alert:@"Information" message:message buttonTitle:@"OK"];
    }
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([segue identifier] == nil) {
        for(id viewController in [self.navigationController viewControllers]) {
            if([viewController isKindOfClass:[PreviewViewController class]]) {
                
                PreviewViewController *pvc = (PreviewViewController *)viewController;
                [pvc setAudioTrack:self.audioTrackDataController.audioTrackList];
            }
        }
    } else if([[segue identifier] isEqualToString:@"segueAudioFileList"]) {
        [(AudioFileListTableViewController *)[segue destinationViewController] setupAudioFileList:^(AudioTrackInfo *trackInfo) {
            [self.audioTrackDataController setAudioTrack:trackInfo];
            [self.tableView reloadData];
        }];
    } else if([[segue identifier] isEqualToString:@"SegueAudioTrackProperty"]) {
        int selectedIdx = (int)self.tableView.indexPathForSelectedRow.row;
        [(AudioPropertyTableViewController *)[segue destinationViewController] setupProperty:[self.audioTrackDataController.audioTrackList objectAtIndex:selectedIdx]
                                                                                 projectTime:self.projectTime
                                                                            refreshTableView:^(AudioTrackInfo *trackInfo) {
            [self.audioTrackDataController replaceAudioTrack:selectedIdx audioTrackInfo:trackInfo];
            [self.tableView reloadData];
        }];
    }
}

@end

