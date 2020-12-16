/******************************************************************************
 * File Name   :	FileManagerDataController.h
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
@import Photos;

typedef NS_ENUM(NSUInteger, ClipMediaType) {
    ClipMediaTypeVideo = 0,
    ClipMediaTypeImage,
    ClipMediaTypeAudio
};

@interface ClipMediaItem : NSObject

@property (nonatomic, assign) ClipMediaType type;
@property (nonatomic, strong) PHAsset *phAsset;
@property (nonatomic, strong) NSString *filePath;

@end

/**
 * \brief 현재까지는 NSDocument, Bundle에서 읽을 수 있는 앱에 내장된 컨텐츠만 pickup 할 수 있다.
 */
@interface FileManagerDataController : NSObject
/**
 * \brief 인자를 기준으로 해당되는 리스트를 구성해준다.
 */
- (instancetype)initWithVideo:(BOOL)video Image:(BOOL)image Audio:(BOOL)audio;

- (NSArray<ClipMediaItem *> *)itemsOfType:(ClipMediaType)type;
- (ClipMediaItem *) itemForType:(ClipMediaType) type atIndex:(NSInteger) index;
@end
