/******************************************************************************
 * File Name   :	LayerFileManagerDataController.h
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

typedef NS_ENUM(NSUInteger, ContentType) {
    VideoType = 0,
    ImageType,
    StickerType
};

@interface LayerFileListItem : NSObject

@property (nonatomic, assign) ContentType contentType;
@property (nonatomic, retain) AVURLAsset *avURLAsset;
@property (nonatomic, retain) NSString *phAssetIdentifier;
@property (nonatomic, retain) UIImage *thumbnail;
@property (nonatomic, retain) NSString *filePath;
@property (nonatomic, assign) Boolean isChecked;

@end

@interface LayerFileManagerDataController : NSObject

/**
 * \brief 인자를 기준으로 해당되는 리스트를 구성해준다.
 */
- (instancetype)initWithVideo:(BOOL)needVideo Image:(BOOL)needImage Sticker:(BOOL)needSticker;

/**
 * \brief 컨텐츠 타입, 인덱스를 넘겨주면 파일 이름을 넘겨준다.
 */
- (NSString *)getFileNameWithContentType:(ContentType)contentType listIndex:(int)listIndex;

/**
 * \brief 컨텐츠 타입, 인덱스를 넘겨주면 isChecked를 넘겨준다.
 */
- (Boolean)getIsCheckedWithContentType:(ContentType)contentType listIndex:(int)listIndex;

/**
 * \brief 컨텐츠 타입, 인덱스를 넘겨주면 isChecked을 저장한다.
 */
- (void)setIsCheckedWithContentType:(ContentType)contentType listIndex:(int)listIndex isChecked:(Boolean)isChecked;

/**
 *\ brief 컨텐츠 타입, 인덱스를 넘겨주면 파일 경로를 넘겨준다.
 */
- (NSString *)getFilePathWithContentType:(ContentType)contentType listIndex:(int)listIndex;

- (NSString *)getphAssetIdentifierWithContentType:(ContentType)contentType listIndex:(int)listIndex;

/**
 *\ brief File 정보를 가지고 있는 list를 넘겨준다.
 */
- (NSMutableArray *)getFileListWithContentType:(ContentType)contentType;

/**
 */
- (UIImage *)getThumbnailWithContentType:(ContentType)contentType listIndex:(int)listIndex;

/**
 */
- (AVAsset *)getAVAssetWithContentType:(ContentType)contentType listIndex:(int)listIndex;

/**
 * \brief 컨텐츠 타입을 넘겨주면 해당 리스트 개수를 넘겨준다.
 */
- (int)getCountWithContentType:(ContentType)contentType;

@end
