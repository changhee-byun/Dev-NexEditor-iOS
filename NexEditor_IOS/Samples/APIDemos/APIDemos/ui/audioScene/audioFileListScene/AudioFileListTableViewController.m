/******************************************************************************
 * File Name   :	AudioFileListTableViewController.m
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

#import "AudioFileListTableViewController.h"
#import "AudioFileListDataController.h"
#import "CommonTypeDTableViewCell.h"
#import "Util.h"
#import "APIDemos-Swift.h"

@interface AudioFilListItem(ClipSource)
- (NXEClipSource *) clipSource;
@end

@implementation AudioFilListItem(ClipSource)
- (NXEClipSource *) clipSource
{
    NXEClipSource *result = nil;
    NSString *path = self.filePath;
    if (path == nil) {
        path = self.url.absoluteString;
    }
    if (path) {
        result = [NXEClipSource sourceWithPath:path];
    }
    return result;
}
@end

@interface AudioFileListTableViewController ()

@property (copy, nonatomic) void(^refreshTableViewCallback)();
@property (nonatomic, retain) AudioFileListDataController *audioFileListDataController;
@property (nonatomic, retain) CommonTypeDTableViewCell *selectedCell;
@property (nonatomic, assign) int selectedListIndex;
@property (nonatomic, retain) AVAudioPlayer *player;

@end

@implementation AudioFileListTableViewController

- (void)setupAudioFileList:(refreshTableView)refreshTableView
{
    self.refreshTableViewCallback = refreshTableView;
}

- (void)dealloc
{
    self.audioFileListDataController = nil;
    self.player = nil;
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    [self releasePlayer];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // NESI-180
    //
    self.selectedListIndex = -1;
    //
    self.audioFileListDataController = [[AudioFileListDataController alloc] init];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.audioFileListDataController.audioList count];
}

- (CommonTypeDTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeDTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"AudioListCell" forIndexPath:indexPath];
    
    AudioFilListItem *listItem = [self.audioFileListDataController.audioList objectAtIndex:indexPath.row];
    
    cell.FileName.text = listItem.title;
    cell.Duration.text = [StringTools formatTimeMs:(int)listItem.duration*1000];

    if(listItem.check == YES) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
        _selectedCell = cell;
        _selectedCell.tag = _selectedListIndex;
        [self progressPlayer];
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
        cell.tag = indexPath.row;
        cell.Progress.progress = 0;
    }
    
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 60;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView cellForRowAtIndexPath:indexPath].accessoryType = UITableViewCellAccessoryCheckmark;
    //
    AudioFilListItem *listItem = [self.audioFileListDataController.audioList objectAtIndex:indexPath.row];
    if(listItem.check) {
        // 이미 재생중이라 생각하고, 리턴한다.
        return;
    } else {
        _selectedListIndex = (int)indexPath.row;
        _selectedCell = [tableView cellForRowAtIndexPath:indexPath];
        _selectedCell.tag = _selectedListIndex;
        //
        listItem.check = YES;
        if(listItem.filePath != nil) {
            [self playSoundWithPath:listItem.filePath];
        } else if(listItem.url != nil) {
            [self playSoundWithURL:listItem.url];
        }
    }
}

- (void)tableView:(UITableView *)tableView didDeselectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [tableView cellForRowAtIndexPath:indexPath].accessoryType = UITableViewCellAccessoryNone;
    //
    AudioFilListItem *listItem = [self.audioFileListDataController.audioList objectAtIndex:indexPath.row];
    listItem.check = NO;
    
    CommonTypeDTableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
    cell.Progress.progress = 0.0f;
}

#pragma mark - listener

- (IBAction)doNext:(id)sender
{
    // NESI-180
    if(self.selectedListIndex == -1) {
        //
        [self.navigationController popViewControllerAnimated:YES];
        return;
    }
    
    AudioFilListItem *listItem = [self.audioFileListDataController.audioList objectAtIndex:_selectedListIndex];
    
    // AudioTrack trim
    NSURL *audioFileURL = nil;
    
    if(listItem.filePath != nil) {
        audioFileURL = [NSURL fileURLWithPath:listItem.filePath];
    } else {
        audioFileURL = listItem.url;
    }
    
    AVURLAsset *audioAsset = [AVURLAsset URLAssetWithURL:audioFileURL options:nil];
    Float64 audioDuration = CMTimeGetSeconds(audioAsset.duration);
    //
    
    AudioTrackInfo *trackInfo = [[AudioTrackInfo alloc] init];

    trackInfo.clipSource = listItem.clipSource;
    trackInfo.title = listItem.title;
    trackInfo.background = NO;
    trackInfo.loop = YES;
    trackInfo.volumeLevel = 100;
    trackInfo.trimStartTime = 0;
    trackInfo.trimEndTime = 0;
    trackInfo.duration = audioDuration*1000/*handling as millisecond unit*/;
    trackInfo.pitch = 0;
    trackInfo.panLeft = -100;
    trackInfo.panRight = 100;
    trackInfo.compressor = false;
    trackInfo.audioChanger = 0;
    
    trackInfo.envelopList = [[EnvelopInfo defaultListWithDuration:trackInfo.duration] mutableCopy];
	
    self.refreshTableViewCallback(trackInfo);
    //
    [self.navigationController popViewControllerAnimated:YES];
}


#pragma mark - Audio Player

- (void)playSoundWithURL:(NSURL *)url
{
    [self releasePlayer];
    
    if(self.player != nil && self.player.playing) {
        [self.player stop];
    }
    
    NSError *error = nil;
    
    self.player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
    
    if(error != nil) {
        NSLog(@"Error in creating AVAudioPlayer for %@", [error localizedDescription]);
        return;
    }
    
    [self.player play];
    [self progressPlayer];
}

- (void)playSoundWithPath:(NSString *)path
{
    [self releasePlayer];
    
    if(self.player != nil && self.player.playing) {
        [self.player stop];
    }
    
    NSURL *url = [NSURL fileURLWithPath:path];
    
    NSError *error = nil;
    
    if([[path pathExtension] isEqualToString:@"aac"]) {
        NSData *data = [NSData dataWithContentsOfURL:url];
        self.player = [[AVAudioPlayer alloc] initWithData:data error:&error];
    } else {
        self.player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
        url = nil;
    }
    
    if(error != nil) {
        NSLog(@"Error in creating AVAudioPlayer for %@", [error localizedDescription]);
        return;
    }
    
    [self.player play];
    [self progressPlayer];
}

- (void)progressPlayer
{
    if(self.player == nil || !self.player.playing) {
        return;
    }
    if(_selectedCell.tag != _selectedListIndex) {
        return;
    }

    _selectedCell.Progress.progress = self.player.currentTime / self.player.duration;
    //
    [self performSelector:@selector(progressPlayer) withObject:nil afterDelay:0.2f];
}

- (void)releasePlayer
{
    if(self.player != nil) {
        [self.player stop];
    }
    self.player = nil;
}

@end
