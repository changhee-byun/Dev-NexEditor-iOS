/******************************************************************************
 * File Name   :	AudioFileListDataController.h
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

@interface AudioFilListItem : NSObject

@property (nonatomic, retain) NSString *filePath;
@property (nonatomic, retain) NSString *title;
@property (nonatomic, retain) NSURL *url;
@property (nonatomic, assign) BOOL check;
@property (nonatomic, assign) float duration;

@end

@interface AudioFileListDataController : NSObject

@property (nonatomic, retain) NSMutableArray *audioList;

@end
