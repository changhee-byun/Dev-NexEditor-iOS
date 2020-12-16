/******************************************************************************
 * File Name   :	LayerOptionTableViewController.m
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

#import "LayerOptionTableViewController.h"
#import "LayerFileListTableViewController.h"
#import "ImageLayerTypeTableviewController.h"
#import "CommonTypeATableViewCell.h"
#import "LayerListEditor.h"

@interface LayerOptionTableViewController()

@property (nonatomic, retain) NSArray *layerMenuList;

@end

@implementation LayerOptionTableViewController

// MJ.
//  autorelease 되는 부분이라고 함.
//
//- (void)dealloc
//{
//    [_layerMenuList dealloc];
//    [super dealloc];
//}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.title = @"Layers";
    self.layerMenuList = @[@"Video", @"Image", @"Text"];
}

- (void)didMoveToParentViewController:(UIViewController *)parent
{
    if ( parent == nil ) {
        if (self.onDismiss) {
            self.onDismiss(self, NO);
            self.onDismiss = nil;
        }
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.layerMenuList count];
}

- (CommonTypeATableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeATableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"LayerOptionTableCell" forIndexPath:indexPath];
    
    NSString *name = [self.layerMenuList objectAtIndex:indexPath.row];
    cell.titleLabel.text = name;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSLog(@"indexPath.row: %d", (int)indexPath.row);
    
    if(indexPath.row == 0) {
        // video
        [self performSegueWithIdentifier:@"segueVideoLayer" sender:self];
    } else if(indexPath.row == 1) {
        // image
        [self performSegueWithIdentifier:@"segueImageLayerType" sender:self];
    } else if(indexPath.row == 2) {
        // text
        [self performSegueWithIdentifier:@"segueTextLayer" sender:self];
    }
}

#pragma mark - Navigation

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    id<LayerListEditor> layerListEditor = nil;
    if([[segue identifier] isEqualToString:@"segueVideoLayer"]) {
        layerListEditor = segue.destinationViewController;
    } else if([[segue identifier] isEqualToString:@"segueImageLayerType"]) {
        layerListEditor = segue.destinationViewController;
    } else if([[segue identifier] isEqualToString:@"segueTextLayer"]) {
        layerListEditor = segue.destinationViewController;
    }
    
    if ( layerListEditor ) {
        layerListEditor.layers = self.layers;
        layerListEditor.onDismiss = ^(id<LayerListEditor> editor, BOOL done) {
            if ( done ) {
                self.layers = editor.layers;
            }
        };
    }
}

@end
