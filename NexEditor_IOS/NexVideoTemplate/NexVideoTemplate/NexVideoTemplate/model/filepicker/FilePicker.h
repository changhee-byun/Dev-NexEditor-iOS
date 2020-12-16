/******************************************************************************
 * File Name   :	FilePicker.h
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
#import <Foundation/Foundation.h>
typedef NS_ENUM(NSUInteger, ContentType) {
    VideoType = 0,
};

@interface ListItem : NSObject
@property(nonatomic, retain) __block AVURLAsset *avURLAsset;
@property(nonatomic, retain) __block UIImage *thumbnail;
@property(nonatomic, assign) __block int duration;
@property(nonatomic, assign) __block bool isChecked;
@end

@interface FilePicker : NSObject
- (void)updateListFromPHAsset;
- (void)removeAllExportFileInDocument;

- (NSMutableArray*)getVideoList;
@end
