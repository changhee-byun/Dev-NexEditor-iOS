/******************************************************************************
 * File Name   :	LayerExpressionTableViewController.m
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

#import "LayerExpressionTableViewController.h"
#import "CommonTypeATableViewCell.h"

@interface LayerExpressionTableViewController ()

@property (nonatomic, copy) void (^refreshLayerExpression)(int layerExpression);

@end

@implementation LayerExpressionTableViewController
{
    int selectedLayerExpression;
}

- (void)setupLayerExpressionWithCB:(refreshLayerExpression)refreshLayerExpression selectedLayerExpression:(int)layerExpression
{
    self.refreshLayerExpression = refreshLayerExpression;
    selectedLayerExpression = layerExpression;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = @"Expression Types";
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return 3;
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    NSArray *expressionList = @[@"None", @"Blink Slow", @"Flicker"/*, @"Pulse", @"Jitter", @"Fountain", @"Ring", @"Floating", @"Drifting", @"Squishing"*/];
    
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"LayerExpressionCell" forIndexPath:indexPath];
    
    cell.titleLabel.text = [expressionList objectAtIndex:indexPath.row];
    if(selectedLayerExpression == indexPath.row) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    self.refreshLayerExpression((int)indexPath.row);
    
    [self.navigationController popViewControllerAnimated:YES];
}

@end
