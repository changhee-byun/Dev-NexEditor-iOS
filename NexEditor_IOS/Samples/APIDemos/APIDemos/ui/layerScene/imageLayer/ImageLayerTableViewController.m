/******************************************************************************
 * File Name   :	ImageLayerTableViewController.m
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

#import "ImageLayerTableViewController.h"
#import "ImageLayerPropertyTableViewController.h"
#import "LayerFileListTableViewController.h"
#import "CommonTypeCTableViewCell.h"
#import "PreviewViewController.h"
#import "Util.h"
#import "LUTNames.h"
#import "NSArray+NXELayer.h"
#import "APIDemos-Swift.h"

@import NexEditorFramework;

@implementation ImageLayerTableViewController
{
    UIAlertController *alertController;
    NSArray *imageLayerList;
    int positionId;
    int layerType;
}

- (void)setImageLayerType:(int)imageLayerType
{
    layerType = imageLayerType; // 1: NXE_LAYER_IMAGE, 2: NXE_LAYER_STICKER
}

- (void)viewDidLoad {
    [super viewDidLoad];

    if(layerType == 1) {
        self.title = @"ImageLayer List(Image)";
    } else {
        self.title = @"ImageLayer List(Sticker)";
    }
    
    imageLayerList = [self.layers layersWithType:NXE_LAYER_IMAGE];
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
    self.layers = [self.layers mergingLayers:imageLayerList type:NXE_LAYER_IMAGE];
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
    return [imageLayerList count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    return 140;
}

- (CommonTypeCTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeCTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"ImageLayerListCell" forIndexPath:indexPath];
    
    NXEImageLayer *imageLayer = [imageLayerList objectAtIndex:indexPath.row];
    
    if(imageLayer.x == 0) {
        if(imageLayer.y == 0) {
            positionId = 0;
        } else {
            positionId = 2;
        }
    } else if(imageLayer.x == 640) {
        if(imageLayer.y == 0) {
            positionId = 1;
        } else {
            positionId = 3;
        }
    } else {
        positionId = 4;
    }
    
    NSString *contentText = nil;
    contentText = [NSString stringWithFormat:@"StartTime:%@, EndTime:%@,\nColorAdjustment(Bright:%d,Contrast:%d,Saturation:%d),\nColorFilter(%@), Alpha(Opacity):%d,\nIn Animation(%@), Out Animation(%@),\nIn Animation Duration(%@), Out Animation Duration(%@),\nOverall Animation(%@)",
                   [StringTools formatTimeMs:imageLayer.startTime],
                   [StringTools formatTimeMs:imageLayer.endTime],
                   (int)(imageLayer.brightness * 255),
                   (int)(imageLayer.contrast * 255),
                   (int)(imageLayer.saturation * 255),
                   [LUTNames nameForLutId:imageLayer.lutId],
                   (int)(100 * imageLayer.alpha),
                   [self getAnimation:imageLayer.inAnimation type:0],
                   [self getAnimation:imageLayer.outAnimation type:1],
                   [StringTools formatTimeMs:imageLayer.inAnimationDuration],
                   [StringTools formatTimeMs:imageLayer.outAnimationDuration],
                   [self getExpression:imageLayer.expression]];
    
    cell.TableLabel.text = [NSString stringWithFormat:@"imagelayerList id:%d", (int)indexPath.row];
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
        __block NXEImageLayer *imageLayer = [imageLayerList objectAtIndex:indexPath.row];
        NSString *index_name = [NSString stringWithFormat:@"listItem:%d", (int)indexPath.row];
        
        alertController = [UIAlertController alertControllerWithTitle:@"Information"
                                                             message:[NSString stringWithFormat:@"Delete image layer:\n%@.", index_name]
                                                      preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *ok = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [alertController dismissViewControllerAnimated:YES completion:nil];
            alertController = nil;
                        
            imageLayerList = [imageLayerList removingObject:imageLayer];
            
            [self.tableView reloadData];
        }];
        UIAlertAction *nok = [UIAlertAction actionWithTitle:@"NO" style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
            [alertController dismissViewControllerAnimated:YES completion:nil];
            alertController = nil;
        }];
        
        [alertController addAction:ok];
        [alertController addAction:nok];
        
        [self presentViewController:alertController animated:YES completion:nil];
    }
}

- (UIImage *)createImageWithPath:(NSString *)path
{
    NSString *path_ = nil;
    
    if([path hasPrefix:@"phasset-image://"]) {
        __block UIImage *uiImage = nil;
        NSRange replaceRange = [path rangeOfString:@"phasset-image://"];
        
        if (replaceRange.location != NSNotFound) {
            path_ = [path stringByReplacingCharactersInRange:replaceRange withString:@""];
        }
        
        PHFetchOptions *fetchOptions = [[PHFetchOptions alloc] init];
        fetchOptions.sortDescriptors = @[[NSSortDescriptor sortDescriptorWithKey:@"creationDate" ascending:YES]];
        PHFetchResult *fetchResult = [PHAsset fetchAssetsWithLocalIdentifiers:@[path_] options:fetchOptions];
        PHAsset *asset = fetchResult.firstObject;
        
        PHImageRequestOptions * imageRequestOptions = [[PHImageRequestOptions alloc] init];
        imageRequestOptions.synchronous = YES;
        [[PHImageManager defaultManager] requestImageDataForAsset:asset
                                                          options:imageRequestOptions
                                                    resultHandler:^(NSData *imageData, NSString *dataUTI, UIImageOrientation orientation, NSDictionary *info) {
                                                        uiImage = [UIImage imageWithData:imageData];
                                                    }];
        return uiImage;
    } else {
        return [UIImage imageWithContentsOfFile:path];
    }
}

#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([segue identifier] == nil) {
        [self informDismissDone:YES];
    } else if([[segue identifier] isEqualToString:@"segueAddImageLayer"]) {
        LayerFileListTableViewController *lfv = [segue destinationViewController];
        
        [lfv setFileManagerWithType:layerType refreshLayerList:^(NSString *filePath) {
            positionId = 4;
            
            // ImageLayer 생성 후, layer manageList에 등록해준다.
            UIImage *uiImage = [self createImageWithPath:filePath];
            NXEImageLayer *layer = [[NXEImageLayer alloc] initWithImage:uiImage point:CGPointNXELayerCenter];
            imageLayerList = [imageLayerList arrayByAddingObject:layer];
            
            [self.tableView reloadData];
        }];
    } else if([[segue identifier] isEqualToString:@"segueImageLayerProperty"]) {
        ImageLayerPropertyTableViewController *iptv = [segue destinationViewController];
        
        NXEImageLayer *layer = imageLayerList[self.tableView.indexPathForSelectedRow.row];        
        [iptv setupProperty:layer postion:positionId refreshListItem:^(int postion) {
            positionId = postion;
            [self.tableView reloadData];
        }];
    }
}

@end
