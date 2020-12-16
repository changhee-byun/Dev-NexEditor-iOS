/******************************************************************************
 * File Name   :	NXEThumbnailUICallBack.h
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

#ifndef ThumbnailUICallbackBlock_h
#define ThumbnailUICallbackBlock_h

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface NXEThumbnailUICallbackBlock : NSObject

/**
 * \brief
 * \param uiImage
 * \param index
 * \param totalCount
 * \param timestamp
 * \return
 * \since version 1.0.5
 */
- (id)initWithCallbackBlack:(void(^)(UIImage *uiImage, int index, int totalCount, int timestamp))onGetVideoClipThumbnailCallback;

/**
 * \brief
 * \param uiImage
 * \param index
 * \param totalCount
 * \param timestamp
 * \return
 * \since version 1.0.5
 */
- (void) onGetVideoClipThumbnail:(UIImage*)uiImage Index:(int)index TotalCount:(int)totalCount TimeStamp:(int)timeStamp;


@end

#endif /* ThumbnailUICallbackBlock_h */