/******************************************************************************
 * File Name   :	LayerAnimatioinTableViewController.m
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

#import "LayerAnimationTableViewController.h"
#import "CommonTypeATableViewCell.h"

@interface LayerAnimationTableViewController ()

@property (nonatomic, copy) void (^refreshLayerAnimation)(int layerAnimation);

@end

@implementation LayerAnimationTableViewController
{
    int selectedLayerAnimation;
    int layerType;
}

- (void)setupLayerAnimationWithCB:(refreshLayerAnimation)refreshLayerAnimation type:(int)type selectedLayerAnimation:(int)layerAnimation
{
    self.refreshLayerAnimation = refreshLayerAnimation;
    selectedLayerAnimation = layerAnimation;
    layerType = type; /* 0: in animation / 1: out animation */
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    if(layerType == 0) {
        self.title = @"In Animation Types";
    } else {
        self.title = @"Out Animation Types";
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if(layerType == 0) {
        return 7;
    } else {
        return 6;
    }
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    NSArray *animationList = @[@"None", @"Fade", @"Pop", @"Slide Right", @"Slide Left", @"Slide Up", @"Slide Down"/*, @"Spin CW", @"Spin CCW", @"Drop", @"Scale Up", @"Scale Down", @"Converge"*/];
    
    if(layerType != 0) {
        animationList = @[@"None", @"Fade", @"Slide Right", @"Slide Left", @"Slide Up", @"Slide Down"/*, @"Spin CW", @"Spin CCW", @"Drop", @"Scale Up", @"Scale Down", @"Converge"*/];
    }
    
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"LayerAnimationCell" forIndexPath:indexPath];
    
    cell.titleLabel.text = [animationList objectAtIndex:indexPath.row];
    if(selectedLayerAnimation == indexPath.row) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    self.refreshLayerAnimation((int)indexPath.row);
    
    [self.navigationController popViewControllerAnimated:YES];
}

@end
