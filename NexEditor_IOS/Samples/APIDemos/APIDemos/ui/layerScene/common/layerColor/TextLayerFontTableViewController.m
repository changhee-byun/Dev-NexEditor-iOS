/******************************************************************************
 * File Name   :	TextLayerFontTableViewController.m
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

#import "TextLayerFontTableViewController.h"
#import "CommonTypeATableViewCell.h"

@interface TextLayerFontTableViewController ()

@property (nonatomic, copy) void (^refreshTextFont)(NSString *);

@end

@implementation TextLayerFontTableViewController
{
    UIFont *font;
    NSString *fontNaming;
}

- (void)setupTextLayerFontWithCB:(refreshTextFont)refreshTextColor selectedTextFontNaming:(NSString *)textFontNaming
{
    self.refreshTextFont = refreshTextColor;
    fontNaming = textFontNaming;
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
    return 13;
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    NSArray *fontList = @[@"system default", @"bevan", @"creepster-regular", @"justanotherhand-regular", @"lato-bold", @"knewave", @"releway-thin", @"leaguegothic", @"redressed", @"junctionregular", @"goudystm-italic", @"greatvibes-regular",@"Tangerine_Regular"];
    
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"TextFontCell" forIndexPath:indexPath];
    
    cell.titleLabel.text = [fontList objectAtIndex:indexPath.row];
    if([fontNaming compare:[fontList objectAtIndex:indexPath.row] options:NSCaseInsensitiveSearch] == NSOrderedSame) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSArray *fontList = @[@"system default", @"bevan", @"creepster-regular", @"justanotherhand-regular", @"lato-bold", @"knewave", @"releway-thin", @"leaguegothic", @"redressed", @"junctionregular", @"goudystm-italic", @"greatvibes-regular",@"Tangerine_Regular"];
    
    self.refreshTextFont([fontList objectAtIndex:indexPath.row]);
    //
    [self.navigationController popViewControllerAnimated:YES];
}

@end
