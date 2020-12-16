/******************************************************************************
 * File Name   :	VideoLayerTableViewController.m
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

#import "VideoLayerTableViewController.h"
#import "VideoLayerPropertyTableViewController.h"
#import "CommonTypeCTableViewCell.h"
#import "PreviewViewController.h"
#import "Util.h"
#import "LUTNames.h"
#import "NSArray+NXELayer.h"
#import "FileManagerDataController.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@interface VideoLayerTableViewController() <MediaPickerObjcDelegate>
@end

@implementation VideoLayerTableViewController
{
    UIAlertController *alertController;
    
    NSArray *videoLayerList;
    int positionId;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    videoLayerList = [self.layers layersWithType:NXE_LAYER_VIDEO];
}

- (void)didMoveToParentViewController:(UIViewController *)parent
{
    if ( parent == nil ) {
        [self informDismissDone:NO];
    }
}

#pragma mark -

/// Merge video layer list change into self.layers and call back to the calling view controller
- (void) informDismissDone:(BOOL) done
{
    self.layers = [self.layers mergingLayers:videoLayerList type:NXE_LAYER_VIDEO];
    if (self.onDismiss) {
        self.onDismiss(self, done);
        self.onDismiss = nil;
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [videoLayerList count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 180;
}

- (CommonTypeCTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeCTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"VideoLayerListCell" forIndexPath:indexPath];
    
    NXEVideoLayer *videoLayer = [videoLayerList objectAtIndex:indexPath.row];
    
    if(videoLayer.x == 0) {
        if(videoLayer.y == 0) {
            positionId = 0;
        } else {
            positionId = 2;
        }
    } else if(videoLayer.x == 640) {
        if(videoLayer.y == 0) {
            positionId = 1;
        } else {
            positionId = 3;
        }
    } else {
        positionId = 4;
    }
    
    int startTrimTime = startTrimTime = videoLayer.layerClip.headTrimDuration;
    int endTrimTime = videoLayer.layerClip.trimEndTime;
    
    NSString *trimTimeinfo = nil;
    
    if(startTrimTime == 0 && endTrimTime == 0) {
        trimTimeinfo = @"None";
    } else {
        trimTimeinfo = [NSString stringWithFormat:@"from %@ to %@",
                        [StringTools formatTimeMs:startTrimTime], [StringTools formatTimeMs:endTrimTime]];
    }
    
    NSString *envelopText = @"None";
    
    int count = (int)[videoLayer.layerClip.audioEnvelop.volumeEnvelopeTime count];
    int value; int time;
    
    if(count == 1) {
        time = [[videoLayer.layerClip.audioEnvelop.volumeEnvelopeTime objectAtIndex:0] intValue];
        value = [[videoLayer.layerClip.audioEnvelop.volumeEnvelopeLevel objectAtIndex:0] intValue];
        
        envelopText = [NSString stringWithFormat:@"%@,%d", [StringTools formatTimeMs:time], value];
    } else {
        time = [[videoLayer.layerClip.audioEnvelop.volumeEnvelopeTime objectAtIndex:0] intValue];
        value = [[videoLayer.layerClip.audioEnvelop.volumeEnvelopeLevel objectAtIndex:0] intValue];
        
        envelopText = [NSString stringWithFormat:@"%@,%d...", [StringTools formatTimeMs:time], value];
    }
    
    NSString *contentText = nil;
    contentText = [NSString stringWithFormat:@"Duration:%@, StartTime:%@, EndTime:%@,\nTrim:%@,\nCompressor:%@, Pan:L/%@,R/%@, Pitch:%d, Volume:%d,\nAudioChanger:%d, Envelop:%@,\nColorAdjustment(Bright:%d,Contrast:%d,Saturation:%d),\nColorFilter(%@), Alpha(Opacity):%d,\nIn Animation(%@), Out Animation(%@),\nIn Animation Duration(%@), Out Animation Duration(%@),\nOverall Animation(%@)",
                   [StringTools formatTimeMs:videoLayer.layerClip.durationMs],
                   [StringTools formatTimeMs:videoLayer.startTime],
                   [StringTools formatTimeMs:videoLayer.endTime],
                   trimTimeinfo,
                   videoLayer.layerClip.vinfo.compressor == 4 ? @"YES" : @"NO",
                   [NSNumber numberWithInt:videoLayer.layerClip.vinfo.panLeft],
                   [NSNumber numberWithInt:videoLayer.layerClip.vinfo.panRight],
                   videoLayer.layerClip.vinfo.pitch,
                   videoLayer.layerClip.vinfo.clipVolume,
                   videoLayer.layerClip.vinfo.voiceChanger,
                   envelopText,
                   (int)(videoLayer.brightness * 255),
                   (int)(videoLayer.contrast * 255),
                   (int)(videoLayer.saturation * 255),
                   [LUTNames nameForLutId:videoLayer.lutId],
                   (int)(100 * videoLayer.alpha),
                   [self getAnimation:videoLayer.inAnimation type:0],
                   [self getAnimation:videoLayer.outAnimation type:1],
                   [StringTools formatTimeMs:videoLayer.inAnimationDuration],
                   [StringTools formatTimeMs:videoLayer.outAnimationDuration],
                   [self getExpression:videoLayer.expression]];
    
    cell.TableLabel.text = videoLayer.layerClip.clipSource.shortDisplayText;
    cell.ContentLabel.text = contentText;
    
    return cell;
}

- (NSString *)getPosition:(long)index
{
    NSArray *positionList = @[@"LEFT:0, TOP:0", @"LEFT:640, TOP:0", @"LEFT:0, TOP:360", @"LEFT:640, TOP:360", @"Others"];
    return [positionList objectAtIndex:positionId];
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

#pragma mark - Listener

- (IBAction)doDeleteCell:(id)sender
{
    if(((UILongPressGestureRecognizer *)sender).state == UIGestureRecognizerStateBegan) {
        CGPoint p = [((UILongPressGestureRecognizer *)sender) locationInView:self.tableView];
        NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:p];
        //
        __block NXEVideoLayer *videoLayer = [videoLayerList objectAtIndex:indexPath.row];
        NSString *track_name = videoLayer.layerClip.clipSource.shortDisplayText;
        
        alertController = [UIAlertController alertControllerWithTitle:@"Information"
                                                              message:[NSString stringWithFormat:@"Delete video layer:\n%@.", track_name]
                                                       preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [alertController dismissViewControllerAnimated:YES completion:nil];
            videoLayerList = [videoLayerList removingObject:videoLayer];
            
            [self.tableView reloadData];
        }];
        UIAlertAction *nok = [UIAlertAction actionWithTitle:@"NO" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [alertController dismissViewControllerAnimated:YES completion:nil];
        }];
        [alertController addAction:ok];
        [alertController addAction:nok];
        [self presentViewController:alertController animated:YES completion:nil];
    }
}

- (IBAction)doFileList:(id)sender
{
    [self performSegueWithIdentifier:@"mediaPickerVideoLayer" sender:self];
}

#pragma mark - MediaPickerViewController
- (void)itemPicker:(ItemPickerCollectionViewController * _Nonnull)picker didSelect:(NSIndexPath * _Nonnull)indexPath
{
    [self mediaPicker:(MediaPickerViewController *) picker didSelectRows:@[indexPath]];
}

- (void)mediaPicker:(MediaPickerViewController * _Nonnull)picker didSelectRows:(NSArray<NSIndexPath *> * _Nonnull)rows
{
    ClipMediaItem *item = [picker.mediaPickerDataSource clipMediaItemsWith:rows][0];
    NXEClipSource *clipSource = nil;
    
    NSString *alertMessage = nil;
    
    if (item.phAsset) {
        clipSource = [NXEClipSource sourceWithPHAsset:item.phAsset];
    } else if (item.filePath) {
        clipSource = [NXEClipSource sourceWithPath:item.filePath];
    } else {
        alertMessage = @"Selected media is not supported. Possibly bad integration";
    }
    
    if (clipSource) {
        NXEError *error = nil;
        NXEVideoLayer *layer = [NXEVideoLayer layerWithClipSource:clipSource error:&error];
        
        if (layer) {
            [layer setStartTime:0 endTime:layer.layerClip.durationMs];
            videoLayerList = [videoLayerList arrayByAddingObject:layer];
            [self.tableView reloadData];
        } else {
            alertMessage = @"Failed creating a video layer";
            if (error) {
                alertMessage = [NSString stringWithFormat:@"%@: %@", alertMessage, error.localizedDescription];
            }
        }
    }
    
    [self.navigationController popViewControllerAnimated:YES];
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
        if (alertMessage) {
            [self alert:@"Error" message:alertMessage buttonTitle:@"OK"];
        }
    }];
    
}

- (void)itemPickerDidCancel:(ItemPickerCollectionViewController * _Nonnull)picker
{

}


#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if([segue identifier] == nil) {
        [self informDismissDone:YES];
    } else if ([segue.identifier isEqualToString:@"mediaPickerVideoLayer"]) {
        MediaPickerViewController *vc = segue.destinationViewController;

        vc.optionsDictionary = @{ MediaPickerOptionKeys.video: @(YES),
                                  MediaPickerOptionKeys.singleSelect: @(YES) };
        vc.objcDelegate = self;
        vc.mediaPickerDataSource = [MediaPickerDataSourceImpl dataSourceWithOptions: vc.optionsDictionary];

    } else if([[segue identifier] isEqualToString:@"segueVideoLayerProperty"]) {
        VideoLayerPropertyTableViewController *vptv = [segue destinationViewController];
        
        int listIdx = (int)self.tableView.indexPathForSelectedRow.row;
        NXEVideoLayer *layer = videoLayerList[listIdx];
        [vptv setupProperty:layer postion:positionId refreshListItem:^(int postion) {
            
            positionId = postion;
            [self.tableView reloadData];
        }];
    }
}

@end
