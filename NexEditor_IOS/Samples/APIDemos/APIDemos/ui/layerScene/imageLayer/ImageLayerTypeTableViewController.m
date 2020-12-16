/******************************************************************************
 * File Name   :	ImageLayerTypeTableViewController.m
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

#import "ImageLayerTypeTableViewController.h"
#import "ImageLayerTableViewController.h"
#import "CommonTypeATableViewCell.h"

@interface ImageLayerTypeTableViewController ()

@property (nonatomic, retain) NSArray *imagelayerMenuList;

@end

@implementation ImageLayerTypeTableViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title = @"ImageLayer Types";
    self.imagelayerMenuList = @[@"Image", @"Sticker"];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.imagelayerMenuList count];
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"ImageLayerTypeCell" forIndexPath:indexPath];
    
    NSString *name = [self.imagelayerMenuList objectAtIndex:indexPath.row];
    cell.titleLabel.text = name;
    return cell;
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    int index = (int)self.tableView.indexPathForSelectedRow.row;
    
    ImageLayerTableViewController *itv = [segue destinationViewController];
    [itv setImageLayerType:index+1]; /* for syncing.. 1: image, 2: sticker */
    itv.layers = self.layers;
    itv.onDismiss = self.onDismiss;
}

@end
