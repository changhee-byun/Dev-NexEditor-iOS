/******************************************************************************
 * File Name   :	AudioChangerTableViewController.m
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

#import "AudioChangerTableViewController.h"
#import "CommonTypeATableViewCell.h"

@interface AudioChangerTableViewController ()

@property (copy, nonatomic) void (^refreshAudioChangerValue)(NSString *param);
@property (nonatomic, assign) NSString *selectedValue;
@property (nonatomic, retain) NSArray *changerList;

@end

@implementation AudioChangerTableViewController

- (void)dealloc
{
    _selectedValue = nil;
    _changerList = nil;
}

- (void)setupAudioChangerListWithCB:(refreshAudioChangerValue)refreshTableView selectedValue:(NSString *)selectedValue
{
    self.refreshAudioChangerValue = refreshTableView;
    self.selectedValue = selectedValue;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = @"Changer Types";
    
    self.changerList = @[@"None", @"Chipmunk", @"Robert", @"Deep", @"Modulation"];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return [self.changerList count];
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"AudioChangerCell" forIndexPath:indexPath];
    
    NSString *name = [self.changerList objectAtIndex:indexPath.row];
    cell.titleLabel.text = name;
    if(self.selectedValue == [self.changerList objectAtIndex:indexPath.row]) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self.navigationController popViewControllerAnimated:YES];
    
    self.refreshAudioChangerValue([self.changerList objectAtIndex:indexPath.row]);
}

@end
