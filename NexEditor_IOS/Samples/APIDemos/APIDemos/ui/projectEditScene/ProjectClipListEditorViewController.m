/******************************************************************************
 * File Name   :	ProjectTableViewController.m
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

#import "ProjectClipListEditorViewController.h"
#import "CommonTypeCTableViewCell.h"
#import "ImagePropertyTableViewController.h"
#import "VideoPropertyTableViewController.h"
#import "SolidClipTableViewController.h"
#import "Util.h"
#import "NXEClip+Extensions.h"
#import "NXEClip+Summary.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@interface ProjectClipListEditorViewController ()

@end

@implementation ProjectClipListEditorViewController
{    
    BOOL isAddSolid;
    NSString *solidColor;
}
@synthesize clips = _clips;

- (void) setClips:(NSArray<NXEClip *> *)clips
{
    _clips = [clips copy];
}

- (NSArray<NXEClip *> *) clips
{
    if (_clips == nil) {
        _clips = @[];
    }
    return _clips;
}

- (void)dealloc
{
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
    return self.clips.count;
}

- (CommonTypeCTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeCTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"ProjectCell" forIndexPath:indexPath];
    
    NXEClip *clip = self.clips[indexPath.row];
    
    NSString *displayName = clip.displayName;
    NSString *clipTypeText = @"Video";
    if (clip.clipType == NXE_CLIPTYPE_IMAGE) {
        clipTypeText = @"Image";
    }
    NSString *labelText = [NSString stringWithFormat:@"%@ / %@", displayName, clipTypeText];
    
    cell.TableLabel.text = labelText;
    cell.ContentLabel.text = clip.summaryText;
    
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 160;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if(isAddSolid) {
        isAddSolid = NO;
        //
        NXEClip *solidClip = [NXEClip newSolidClip:solidColor];
        NSMutableArray *clips = [self.clips mutableCopy];
        [clips insertObject:solidClip atIndex:indexPath.row];
        self.clips = clips;
        [self.tableView reloadData];
    } else {
        NXEClipType clipType = self.clips[indexPath.row].clipType;
        if(clipType == NXE_CLIPTYPE_VIDEO) {
            [self performSegueWithIdentifier:@"segueVideoProperty" sender:self];
        } else if(clipType == NXE_CLIPTYPE_IMAGE) {
            [self performSegueWithIdentifier:@"segueImageProperty" sender:self];
        }
    }
}

#pragma mark - listener

// MJ.
//  KineMaster 와 다르게 파일을 추가할 수가 없어서 임의로 클립을 추가하는 시나리오 추가
//
- (IBAction)doDuplicateCell:(id)sender
{
    if(((UILongPressGestureRecognizer *)sender).state == UIGestureRecognizerStateBegan) {
        CGPoint p = [((UILongPressGestureRecognizer *)sender) locationInView:self.tableView];
        NSIndexPath *indexPath = [self.tableView indexPathForRowAtPoint:p];
        
        NXEClip *clip = self.clips[indexPath.row];
        NSString *displayName = clip.displayName;
        if (displayName == nil) {
            displayName = @"Unknown";
        }
        
        NSString *message = [NSString stringWithFormat:@"Duplicate or Remove the selected clip, %@", displayName];
        [self alertSimpleActionSheet:@"Information"
                             message:message
                        actionTitles:@[@"Duplicate", @"Remove"]
                        dismissTitle:@"Cancel"
                            complete:^(NSInteger actionIndex) {
                                if ( actionIndex == NSNotFound ) { return; }
                                NSMutableArray *clips = [self.clips mutableCopy];
                                
                                if (actionIndex == 0 ) {
                                    // Duplicate
                                    NXEClip *clip = self.clips[indexPath.row];
                                    NXEClip *newClip = [clip copy];
                                    [clips insertObject:newClip atIndex:indexPath.row + 1];
                                } else {
                                    // Remove
                                    [clips removeObjectAtIndex:indexPath.row];
                                }
                                
                                self.clips = clips;
                                [self.tableView reloadData];
                            }];
    }
}

- (IBAction)done:(id)sender {
    if (self.onDone) {
        self.onDone(self, YES);
        self.onDone = nil;
    }
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"segueImageProperty"]) {
        int selected_idx = (int)self.tableView.indexPathForSelectedRow.row;
        ImagePropertyTableViewController *ipc = [segue destinationViewController];
        [ipc setupImagePropertyListCB:^{
            [self.tableView reloadData];
        }
                                 clip:self.clips[selected_idx]];
    } else if([[segue identifier] isEqualToString:@"segueVideoProperty"]) {
        int selected_idx = (int)self.tableView.indexPathForSelectedRow.row;
        VideoPropertyTableViewController *vpc = [segue destinationViewController];
        [vpc setupVideoPropertyListCB:^{
            [self.tableView reloadData];
        }
                                 clip:self.clips[selected_idx]];
    } else if([[segue identifier] isEqualToString:@"segueSolidClip"]) {
        SolidClipTableViewController *stvc = [segue destinationViewController];
        [stvc refreshProjectListWithSolidCB:^(NSString *color) {
            isAddSolid = YES;
            solidColor = [NSString stringWithFormat:@"#%@", color];
        }];
    }
}

@end
