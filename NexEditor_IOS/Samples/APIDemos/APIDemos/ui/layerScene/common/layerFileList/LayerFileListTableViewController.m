/******************************************************************************
 * File Name   :	LayerFileListTableViewController.m
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

#import "LayerFileListTableViewController.h"
#import "CommonTypeBTableViewCell.h"
#import "Util.h"

@import NexEditorFramework;

@interface LayerFileListTableViewController ()

@property (nonatomic, copy) void(^refreshTableViewCallback)(NSString *);
@property (nonatomic, assign) ContentType contentType;
@property (nonatomic, retain) LayerFileManagerDataController *fileDataController;

@end

@implementation LayerFileListTableViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.title = @"File Select";
}

- (void)setFileManagerWithType:(ContentType)contentType refreshLayerList:(refreshLayerList)refreshLayerList
{
    BOOL needVideo = NO;
    BOOL needImage = NO;
    BOOL needSticker = NO;
    
    if(contentType == VideoType) {
        needVideo = YES;
    } else if(contentType == ImageType) {
        needImage = YES;
    } else {
        needSticker = YES;
    }
    
    self.contentType = contentType;
    self.refreshTableViewCallback = refreshLayerList;
    self.fileDataController = [[LayerFileManagerDataController alloc] initWithVideo:needVideo Image:needImage Sticker:needSticker];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    if(self.contentType == VideoType) {
        return @"Video";
    } else if(self.contentType == ImageType) {
        return @"Image";
    } else {
        return @"Sticker";
    }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if(self.contentType == VideoType) {
        return [self.fileDataController getCountWithContentType:VideoType];
    } else if(self.contentType == ImageType) {
        return [self.fileDataController getCountWithContentType:ImageType];
    } else {
        return [self.fileDataController getCountWithContentType:StickerType];
    }
}

- (CommonTypeBTableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    CommonTypeBTableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"FileManagerListCell" forIndexPath:indexPath];
    NSString *fileName = @"";
    Boolean isChecked = false;
    UIImage *thumbnail = nil;
    
    if(self.contentType == VideoType) {
        isChecked = [self.fileDataController getIsCheckedWithContentType:VideoType listIndex:(int)indexPath.row];
        fileName = [self.fileDataController getFileNameWithContentType:VideoType listIndex:(int)indexPath.row];
        // tableview load 할때에 섬네일을 받아옴
        //
        thumbnail = [Util imageFromVideoAsset:[self.fileDataController getAVAssetWithContentType:VideoType listIndex:(int)indexPath.row]];
        if(thumbnail == nil) {
            NSString *path = [self.fileDataController getFilePathWithContentType:VideoType listIndex:(int)indexPath.row];
            AVAsset *videoAsset = [AVAsset assetWithURL:[NSURL fileURLWithPath:path]];
            thumbnail = [Util imageFromVideoAsset:videoAsset];
        }
    } else if(self.contentType == ImageType) {
        isChecked = [self.fileDataController getIsCheckedWithContentType:ImageType listIndex:(int)indexPath.row];
        fileName = [self.fileDataController getFileNameWithContentType:ImageType listIndex:(int)indexPath.row];
        thumbnail = [self.fileDataController getThumbnailWithContentType:ImageType listIndex:(int)indexPath.row];
    } else {
        isChecked = [self.fileDataController getIsCheckedWithContentType:StickerType listIndex:(int)indexPath.row];
        fileName = [self.fileDataController getFileNameWithContentType:StickerType listIndex:(int)indexPath.row];
        thumbnail = [self.fileDataController getThumbnailWithContentType:StickerType listIndex:(int)indexPath.row];
    }
    
    if(fileName != nil) {
        cell.titleLabel.text = fileName;
    } else {
        cell.titleLabel.text = @"nil";
    }
    
    if(thumbnail != nil) {
        cell.imageview.image = thumbnail;
    } else {
        cell.imageview.image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"default_black" ofType:@"png"]];
        //cell.imageview.hidden = YES;
    }
    
    if(isChecked) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    } else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString *filePath = nil;
    
    if(self.contentType == VideoType) {
        filePath = [self.fileDataController getFilePathWithContentType:VideoType listIndex:(int)indexPath.row];
    } else if(self.contentType == ImageType) {
        filePath = [self.fileDataController getphAssetIdentifierWithContentType:ImageType listIndex:(int)indexPath.row];
        if(filePath == nil) {
            filePath = [self.fileDataController getFilePathWithContentType:ImageType listIndex:(int)indexPath.row];
        }
    } else if(self.contentType == StickerType) {
        filePath = [self.fileDataController getFilePathWithContentType:StickerType listIndex:(int)indexPath.row];
    }
    
    self.refreshTableViewCallback(filePath);
    //
    [self.navigationController popViewControllerAnimated:YES];
}

@end
